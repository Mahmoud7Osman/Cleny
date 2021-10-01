#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
   if (argc < 2) {
      return printf("Usage: label <string>\n");
   }

   QApplication app(argc, argv);
   QLabel *label = new QLabel(argv[1]);

   label->show();

   return app.exec();
}
