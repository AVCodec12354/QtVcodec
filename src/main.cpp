#include "oapv.h"
#include <QApplication>
#include <QWidget>
#include "../ui/view/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QtVCodec");

    MainWindow window;
    window.show();

    return app.exec();
}