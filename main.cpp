#include "maindlg.h"
#include <QApplication>
#include <QtAVWidgets>
int main(int argc, char *argv[])
{
    QtAV::Widgets::registerRenderers();
    QApplication a(argc, argv);
    MainDlg w;
    w.show();

    return a.exec();
}
