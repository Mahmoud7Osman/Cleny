//
// Created by Pamela Vincze on 2016-08-23.
//

#ifndef PONG_BALL_H
#define PONG_BALL_H

#include <ncurses.h>

class Ball {
public:
    Ball(double x, double y, int speed);
    double getX() { return x; }
    double getY() { return y; }
    int getSpeed() { return speed; }
    void setX(double a) {
        x = a;
    }
    void setY(double a) {
        y = a;
    }
    void setSpeed(int a){
        speed = a;
    }
    void drawBall(double x, double y) {
        mvaddch(x, y, 'o');
    }
private:
    double x, y;
    int speed;

};


#endif //PONG_BALL_H
//
// Created by Pamela Vincze on 2016-08-23.
//

#ifndef PONG_PLAYER_H
#define PONG_PLAYER_H

#include <ncurses.h>


class Player {

public:
    Player(int x, int y);
    int getX() { return x; }
    int getY() { return y; }
    int getHeight() { return height; }
    void setX(int a) {
        x = a;
    }
    void setY(int a) {
        y = a;
    }
    void drawPlayer(int y, int x)
    {
        mvaddch(y + 2, x, '|');
        mvaddch(y + 1, x, '|');
        mvaddch(y, x, '|');
        mvaddch(y - 1, x, '|');
        mvaddch(y - 2, x, '|');
    }

private:
    int x, y, height;

};


#endif //PONG_PLAYER_H
//
// Created by Pamela Vincze on 2016-08-23.
//


Ball::Ball(double y, double x, int speed)
{
    setY(y);
    setX(x);
    setSpeed(speed);
}
#include <ncurses.h>
#include <stdlib.h>


int ch;
int width = 80;
int height = 24;
int dir = 1;
int player1Points, player2Points = 0;
bool quit;
char wallTexture, playerTexture;
bool player1Serve, player2Serve = false;

Player player1(height / 2, 2);
Player player2(height / 2, width - 3);

Ball ball(height / 2, 3, 1);

void setup();
void input();
void logic();
void draw();

int main() {

    setup();

    // Game loop
    while(!quit)
    {
        input();
        logic();
        draw();
    }
    endwin();
    return 0;
}

void setup()
{
    // Textures
    wallTexture = '*';

    // Init ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(50);

    quit = false;
    player1Points = 0;
    player2Points = 0;
}

void input()
{
    ch = getch();
    switch(ch) {
        case KEY_UP:
            if(player2.getY() != 3)
                player2.setY(player2.getY() - 1);
            break;
        case KEY_DOWN:
            if(player2.getY() != height - 4)
                player2.setY(player2.getY() + 1);
            break;
        case KEY_LEFT:
            if(player2.getX() != width / 2 + 4)
                player2.setX(player2.getX() - 1);
            break;
        case KEY_RIGHT:
            if(player2.getX() != width - 3)
                player2.setX(player2.getX() + 1);
            break;
        case 'w':
            if(player1.getY() != 3)
                player1.setY(player1.getY() - 1);
            break;
        case 's':
            if(player1.getY() != height - 4)
                player1.setY(player1.getY() + 1);
            break;
        case 'a':
            if(player1.getX() != 2)
                player1.setX(player1.getX() - 1);
            break;
        case 'd':
            if(player1.getX() != width / 2 - 4)
                player1.setX(player1.getX() + 1);
            break;
        case ' ':
            if(player1Serve) {
                player1Serve = false;
                dir = 1;
            }
            else if(player2Serve) {
                player2Serve = false;
                dir = 2;
            }
            break;
        case 'q':
            quit = true;
            break;
    }
}

void logic()
{

    /*
     * Ball directions
     *
     * 1 - Right
     * 2 - Left
     * 3 - Right Up
     * 4 - Right down
     * 5 - Left Up
     * 6 - Left down
     *
     */

    // Ball logic
    if(ball.getX() == player1.getX() + 1 || ball.getX() == player1.getX()) {
        if(ball.getY() <= player1.getY() + 2 && ball.getY() >= player1.getY() - 2) {
            if(ball.getY() >= player1.getY() - 2 && ball.getY() < player1.getY())
                dir = 3;
            else if(ball.getY() <= player1.getY() + 2 && ball.getY() > player1.getY())
                dir = 4;
            else
                dir = 1;
        }
    }
    if(ball.getX() == player2.getX() - 1  || ball.getX() == player2.getX()) {
        if(ball.getY() <= player2.getY() + 2 && ball.getY() >= player2.getY() - 2) {
            if (ball.getY() >= player2.getY() - 2 && ball.getY() < player2.getY())
                dir = 5;
            else if (ball.getY() <= player2.getY() + 2 && ball.getY() > player2.getY())
                dir = 6;
            else
                dir = 2;
        }
    }

    if(ball.getY() == height - 2) {
        if (dir == 6)
            dir = 5;
        else
            dir = 3;
    }

    if(ball.getY() == 1) {
        if(dir == 5)
            dir = 6;
        else
            dir = 4;
    }

    if(ball.getX() == 0) {
        player2Points++;
        player1Serve = true;
    }

    if(ball.getX() == width) {
        player1Points++;
        player2Serve = true;
    }

    if(player1Serve) {
        ball.setX(player1.getX() + 1);
        ball.setY(player1.getY());
    }

    if(player2Serve) {
        ball.setX(player2.getX() - 1);
        ball.setY(player2.getY());
    }

    // Ball directions
    if(!player1Serve || !player2Serve) {
        if(dir == 1)
            ball.setX(ball.getX() + 1);
        if(dir == 2)
            ball.setX(ball.getX() - 1);

        if(dir == 3) {
            ball.setX(ball.getX() + 1);
            ball.setY(ball.getY() - 0.25);
        }
        if(dir == 4) {
            ball.setX(ball.getX() + 1);
            ball.setY(ball.getY() + 0.25);
        }
        if(dir == 5) {
            ball.setX(ball.getX() - 1);
            ball.setY(ball.getY() - 0.25);
        }
        if(dir == 6) {
            ball.setX(ball.getX() - 1);
            ball.setY(ball.getY() + 0.25);
        }
    }
}

void draw()
{
    erase();
    refresh();
    for(int i = 0; i < width; i++) {
        mvaddch(0, i, wallTexture);
        mvaddch(height - 1, i, wallTexture);
    }

    for(int i = 1; i < height - 1; i++)
        mvaddch(i, width / 2, ':');

    mvprintw(1, width / 2 / 2, "%i", player1Points);
    mvprintw(1, width / 2 + width / 2 / 2, "%i", player2Points);

    ball.drawBall(ball.getY(), ball.getX());
    player1.drawPlayer(player1.getY(), player1.getX());
    player2.drawPlayer(player2.getY(), player2.getX());
}//
// Created by Pamela Vincze on 2016-08-23.
//

Player::Player(int y, int x)
{
    setX(x);
    setY(y);
}
