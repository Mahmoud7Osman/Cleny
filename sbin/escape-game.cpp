// g++ -O2 -o cxmaze cxmaze.cpp -lncurses
#include <iostream>
#include <random>
#include <ncurses.h>

enum class Color {
    red=1, green, white
};

class Maze {
public:
    using size_type = unsigned long;
    
    explicit Maze(const size_type x, const size_type y) //constructor
        : width{x}, height{y}
    {   
        //allocate and initialize
        maze = new char[width * height];
        for (size_type i=0; i<height*width; ++i)
            maze[i] = 1;
        
        //random numbers engine
        std::random_device seeder;
        std::mt19937 eng(seeder());
        engine = eng;
        std::uniform_int_distribution<int> dt (0, 3);
        dist = dt;
        
        // Make a maze
        maze[1 * width + 1] = 0;
        for(size_type y = 1; y < height; y += 2) {
            for(size_type x = 1; x < width; x += 2) {
                carve_maze(x, y);
            }
        }
        
        // Set up the entry and exit points.
        maze[0 * width + 1] = 0;
        maze[(height - 1) * width + (width - 2)] = 0;
    }      
    
    Maze(const Maze&)=delete;  //disable copy constructor
    Maze& operator=(const Maze&)=delete; //disable copy assignment
    Maze(Maze&&)=delete; //disable move constructor
    Maze& operator=(Maze&&)=delete; //disable move assignment
    
    ~Maze() noexcept //destructor
    {
        delete[] maze;
    }
    
    void carve_maze(int x, int y) //Populates the maze
    {
        int x1=0, y1=0, x2=0, y2=0, dx=0, dy=0, dir=0, count=0;
        dir = myrand();
        while(count < 4) {
            dx = 0; dy = 0;
            switch(dir) {
                case 0:  dx = 1;  break;
                case 1:  dy = 1;  break;
                case 2:  dx = -1; break;
                default: dy = -1; break;
            }
            x1 = x + dx;
            y1 = y + dy;
            x2 = x1 + dx;
            y2 = y1 + dy;
            if(x2 > 0 && x2 < width && y2 > 0 && y2 < height
               && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
                maze[y1 * width + x1] = 0;
                maze[y2 * width + x2] = 0;
                x = x2; y = y2;
                dir = myrand();
                count = 0;
            } else {
                dir = (dir + 1) % 4;
                count += 1;
            }
        }
    }
    
    void Show(const Color& color) //print to screen with ncurses
    {
        attron(COLOR_PAIR(color));
        move(0, 0);
        for (size_type y=0; y<height; ++y) {
            for (size_type x=0; x<width; ++x) {
                printw(maze[y*width+x]==1 ? "[]": "  ");
            }
            printw("\n");
        }
        attroff(COLOR_PAIR(color));
    }

       
    const char* getmaze() const noexcept { return maze; }
    const size_type getwidth() const noexcept { return width; }
    const size_type getheight() const noexcept { return height; }
    int myrand() { return dist(engine); } //random number generator [0:3]
    
private:
    const size_type width;
    const size_type height;
    char* maze;
    std::mt19937 engine;
    std::uniform_int_distribution<int> dist;
};

class Player {
public:
    using size_type = long;
    
    explicit Player(const size_type xx, const size_type yy) //constructor
        : x{xx}, y{yy}, counter{0} {}
        
    Player() : x{1}, y{0}, counter{0} {}  //default constructor
        
    void Show(const Color& color)
    {
        attron(COLOR_PAIR(color));
        move(y, x*2); //x*2 cause "  " and "[]" have 2 chars
        printw("00");
        attroff(COLOR_PAIR(color));
    }
    
    void ShowInfo(const Maze& m, const Color& color)
    {
        attron(COLOR_PAIR(color));
        move(m.getheight()+1, 0);
        printw("Current Position X: %i, Y: %i\nTotal moves: %i", x, y, counter);
        attroff(COLOR_PAIR(color));
    }
    
    void ShowWin(const Color& color)
    {
        move(0, 0);
        clear();
        refresh();
        attron(COLOR_PAIR(color));
        printw("You WIN!!!!! Total moves: %i", counter);
        getch();
        attroff(COLOR_PAIR(color));
    }
    
    void mov(const Maze& m, const size_type xx, const size_type yy)
    {
        static auto mz = m.getmaze();
        
        if (x + xx <= 0 || x + xx >= m.getwidth() || 
            y + yy <= 0 || y + yy >= m.getheight()) return; // Check bounds
            
        if (mz[ (y+yy)*m.getwidth()+(x+xx) ]==1) return;  // Check collisions
        
        x += xx; y += yy; // Change current position
        ++counter;
    }
    
    const size_type getpos_x() const noexcept { return x; }
    const size_type getpos_y() const noexcept { return y; }
    const size_type getcounter() const noexcept { return counter; }
    
private:
    size_type x;
    size_type y;
    size_type counter;
};

class Curses_window {
public:
    Curses_window()   //default construct c-style things
        : x{0}, y{0}    
    {
        initscr();
        if(has_colors() == FALSE) {
            endwin();
            std::cout << "Your terminal does not support colors\n";
            exit(1);
        }
        noecho();
        keypad(stdscr, TRUE);
        start_color();
        curs_set(0);
        init_pair(1, COLOR_RED, COLOR_BLACK);    // must match enum class Color
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
        getmaxyx(stdscr, y, x);
    }
    
    ~Curses_window() noexcept
    {
        endwin();     // destructor restore console at exit
    }
    
    const int getx() const noexcept { return x; }
    const int gety() const noexcept { return y; }
private:
    int x;
    int y;  
};

int main()
{
    Curses_window my_window;
    
    // Start sizes and increase conform screen
    int width = 23, height = 15;
    while (width < (my_window.getx()/2)-3) width += 2;
    while (height < my_window.gety() - 4) height += 2;
    
    bool exit_flag = false;
      
    // Create a maze and a player
    Maze m(width, height);
    Player p;
    int ch = 0;
    // Go for it
    while (!exit_flag) {
        m.Show(Color::green);
        p.Show(Color::red);
        p.ShowInfo(m, Color::white);
        ch = getch();
        switch(ch) {
            case 'w': case KEY_UP:
                p.mov(m, 0, -1);
                break;
            case 's': case KEY_DOWN:
                p.mov(m, 0, 1);
                break;
            case 'a': case KEY_LEFT:
                p.mov(m, -1, 0);
                break;
            case 'd': case KEY_RIGHT:
                p.mov(m, 1, 0);
                break;
            case 'q': case KEY_EXIT: case 27:
                exit_flag = true;
                break;
            default:
                refresh();
        }
        // Test if at exit and Win
        if (p.getpos_y() == m.getheight()-1) {
            p.ShowWin(Color::white);
            exit_flag = true;
        }
    }
    
    return 0;
}
