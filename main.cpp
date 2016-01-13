#include "ck_converter.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CK_Converter w;
    w.show();

    return a.exec();
}
