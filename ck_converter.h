#ifndef CK_CONVERTER_H
#define CK_CONVERTER_H

#include <QDialog>
#include <QMap>
namespace Ui {
class CK_Converter;
}

struct _position
{
    qint64 contentBegin;
    int contentLength;
};

class CK_Converter : public QDialog
{
    Q_OBJECT

public:
    explicit CK_Converter(QWidget *parent = 0);
    ~CK_Converter();

private slots:
    void on_btnLoadAndSplitTabFile_clicked();

    void on_btnReload_clicked();
    
    void on_listWidget_itemSelectionChanged();

private:
    QMap<QString, _position> map;

    Ui::CK_Converter *ui;
};

#endif // CK_CONVERTER_H
