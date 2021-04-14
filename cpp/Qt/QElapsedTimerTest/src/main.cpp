#include <QCoreApplication>
#include <QElapsedTimer>
#include <iostream>

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

    QElapsedTimer timer;

    std::cout << timer.clockType() << std::endl;

    timer.start();

    auto t1 = timer.elapsed();
    auto t2 = timer.nsecsElapsed();
    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;

//    return a.exec();
}
