#ifndef VIDEOCONVERT_H
#define VIDEOCONVERT_H

#include <QWidget>
#include <QJsonArray>
#include "ConvertInfo.h"
#include <QStringListModel>

namespace Ui {
class VideoConvert;
}

enum convertState
{
    ecs_begin,
    ecs_running,
    ecs_error,
    ecs_end
};

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
    void on_pushButton_update_clicked();

private:
    Ui::VideoConvert *ui;
    QStringListModel m_itemModel;
    Video_Convert m_convert;
    convertState mState; // 状态
};

#endif // VIDEOCONVERT_H
