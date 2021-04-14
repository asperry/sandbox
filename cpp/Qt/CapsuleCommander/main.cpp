#include "CapsuleWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    CapsuleWindow w;

    w.show();

    return a.exec();
}

