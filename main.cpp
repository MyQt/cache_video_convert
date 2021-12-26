#include "videoconvert.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion("0.0.1");
    VideoConvert w;
    w.show();
    return a.exec();
}
