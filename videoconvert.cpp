#include "videoconvert.h"
#include "ui_videoconvert.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>

VideoConvert::VideoConvert(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoConvert)
{
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);  // 无边框+无状态栏图标
    ui->setupUi(this);
    connect(&m_convert, SIGNAL(updateUI(int, QString, int)), this, SLOT(UpdateUI(int, QString, int)));
}

VideoConvert::~VideoConvert()
{
    delete ui;
}

void VideoConvert::on_btn_choice_load_clicked()
{
   QString filePath = QFileDialog::getExistingDirectory(this,"选择源视频目录");
   if (filePath.isEmpty()) {
       QMessageBox::information(this, "警告", "请选择合法源视频目录");
       return;
   }
   ui->label_dir_load->setText(filePath);
   m_itemModel.setStringList(QStringList{});
   m_convert.init();
   m_convert.traversalSource(filePath);
   QStringList strList;
   foreach(st_convert_info info, m_convert.m_convertList) {
       if (!info._part.isEmpty() && info._part != info._title) {
           strList.append(info._title + "/" + info._part);
       } else {
           strList.append(info._title);
       }
   }

   m_itemModel.setStringList(strList);
   ui->list_file->setModel(&m_itemModel);
   if (strList.length() > 0) {
       ui->btn_go->setDisabled(false);
       ui->btn_go->setText("点击转码");

       ui->progress_convert->setMinimum(0);
       ui->progress_convert->setValue(0);
       ui->progress_convert->setMaximum(strList.length());
       ui->chk_delete_source->setEnabled(true);
   } else {
       ui->btn_go->setDisabled(true);
       ui->btn_go->setText("没有转码文件");
   }
}

void VideoConvert::on_btn_choice_out_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,"选择视频输出目录");
    if (path.isEmpty()) {
        QMessageBox::information(this, "警告", "请选择合法源视频目录");
        return;
    }
    // 去除最后的'/'
    if(path.endsWith("/")) {
        path = path.left(path.length()-1);
    }
    m_convert.m_outDir = path;
    ui->label_dir_out->setText(path);
}

void VideoConvert::on_btn_go_clicked()
{
    QDir dir;
    if(!dir.exists(ui->label_dir_load->text()) || !dir.exists(ui->label_dir_out->text())) {
        QMessageBox::information(this, "警告","请先选择加载和输入目录");
        return;
    }
    ui->btn_go->setText("转换中");
    ui->btn_go->setDisabled(true);
    ui->chk_delete_source->setDisabled(true);
    m_convert.start();
}

void VideoConvert::UpdateUI(int type, QString strMessage, int index)
{
    if (type == 1 || type == 2) {
        ui->list_file->setCurrentIndex(QModelIndex(m_itemModel.index(index, 0)));
        setWindowTitle(strMessage);
    } else if(type == 3) {
        ui->btn_go->setText("全部转换完成");
        ui->list_file->setCurrentIndex(QModelIndex(m_itemModel.index(m_convert.m_convertList.length()-1, 0)));
        setWindowTitle("视频转换");
        if (ui->chk_delete_source->isChecked()) {
            // 删除源目录文件
            m_convert.clearDir(ui->label_dir_load->text());
            ui->chk_delete_source->setDisabled(false);
        }
    } else if (type == 4) { // 格式错误
        QMessageBox::information(this, "警告!致命错误!", "视频格式"+strMessage+"不支持,程序即将退出");
        QApplication::exit();
        return;
    } else if (type == 5) // 启动外部程序ffmpeg失败
    {
        QMessageBox::information(this, "警告!致命错误!", strMessage);
        QApplication::exit();
        return;
    } else if (type == 6) // 执行外部程序ffmpeg失败
    {
        QMessageBox::information(this, "警告!致命错误!", strMessage);
        QApplication::exit();
        return;
    }
    ui->textBrowser_ConsoleOut->append(strMessage+"\n");
    ui->progress_convert->setValue(index+1);
    QApplication::processEvents();
}
