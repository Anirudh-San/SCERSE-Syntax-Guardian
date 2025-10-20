#include <QApplication>
#include "gui/MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SCERSE::MainWindow window;
    window.show();
    return app.exec();
}
