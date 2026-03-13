#include "auto_mode.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    auto_mode auto_window;
    auto_window.show();
    return a.exec();
}
