#include "videoconvert.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoConvert w;
    w.show();
    return a.exec();
}
