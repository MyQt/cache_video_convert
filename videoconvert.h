#ifndef VIDEOCONVERT_H
#define VIDEOCONVERT_H

#include <QWidget>
#include <QJsonArray>
#include "ConvertInfo.h"
#include <QStringListModel>

namespace Ui {
class VideoConvert;
}



class VideoConvert : public QWidget
{
    Q_OBJECT

public:
    explicit VideoConvert(QWidget *parent = nullptr);
    ~VideoConvert();

private slots:
    void on_btn_choice_load_clicked();

    void on_btn_choice_out_clicked();

    void on_btn_go_clicked();

    void UpdateUI(int type, QString strMessage, int index);
private:
    Ui::VideoConvert *ui;
    QStringListModel m_itemModel;
    Video_Convert m_convert;
};

#endif // VIDEOCONVERT_H
