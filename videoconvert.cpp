#include "videoconvert.h"
#include "ui_videoconvert.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>

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

void VideoConvert::on_pushButton_update_clicked()
{
    QString appPath = QCoreApplication::applicationFilePath();
    QString appVersion = QCoreApplication::applicationVersion();
    QString appUpdateUrl = "https://bdcm02.baidupcs.com/file/ce6d8d082oece80916f7760cd3805fc8?bkt=en-19901565518762c94cec07a0b03ae6a1d7a8f2bf217c1c9e16574ca5d7a413d24893c1bbc14a364966e060a8f7159eaccfb0104cefc4c67c70922261162e7736&fid=34235042-250528-625690804600404&time=1640499177&sign=FDTAXUbGERLQlBHSKfWqi-DCb740ccc5511e5e8fedcff06b081203-uI6Na1gB%2BDYTtyiPeb2pKh%2BR3zA%3D&to=94&size=781&sta_dx=781&sta_cs=0&sta_ft=json&sta_ct=0&sta_mt=0&fm2=MH%2CBaoding%2CAnywhere%2C%2Chenan%2Ccmnet&ctime=1640499172&mtime=1640499172&resv0=0&resv1=0&resv2=rlim&resv3=5&resv4=781&vuk=34235042&iv=0&htype=&randtype=&tkbind_id=0&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=en-7d17da779c4799ad865e85e1f53e36cdcaf4211f9a9819adac6fece41a76f711a678c9a293446597de6b22a8b5c320e803c8ac5ad22325de305a5e1275657320&sl=68616270&expires=8h&rt=pr&r=742848586&vbdid=1349948145&fin=soft_update.json&fn=soft_update.json&rtype=1&dp-logid=269510901731973219&dp-callid=0.1&hps=1&tsl=200&csl=200&fsl=0&csign=AqO5hwEFqnnKG3HD7m4iIlP2Fs8%3D&so=0&ut=6&uter=4&serv=0&uc=3177242081&ti=d07a7c9c0a19b1698a8605576e453a30126c1fcf29a50075305a5e1275657320&hflag=30&from_type=1&adg=c_2e058c9cc885e91968c23b3dc4461b10&reqlabel=250528_f_8da94a6a68026f16d165e5f718fb7868_-1_91dd180898f20c2a5cd139f029763f12&by=themis";
    QProcess process;
    QString strExe("");
    strExe = "UpdateSoft.exe";

    QJsonObject json;
    json.insert("appName", windowTitle());
    json.insert("appPath", appPath);
    json.insert("appVersion", appVersion);
    json.insert("appUpdateUrl", appUpdateUrl);
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    QStringList arguments;
    arguments << byteArray;
    process.startDetached(strExe, arguments);

}
