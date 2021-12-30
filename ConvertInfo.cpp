#include "ConvertInfo.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QProcess>

void Video_Convert::init()
{
    m_floor = 0;
    m_convertList.clear();
    m_outDir.clear();
}

QString Video_Convert::validateTitle(QString title)
{
    return  title.replace(QRegExp("[《》:*?\"<>|&#$\\s\\/\\\\]"), "_");
}

QString Video_Convert::join(QString path1, QString path2)
{
    QString strJoin = path1.append("//" + path2);

    return  strJoin;
}

void Video_Convert::clearDir(QString path)
{
    if(path.isEmpty())
    {
        return;
    }
    QDir dir(path);

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    foreach(QFileInfo fileInfo, dir.entryInfoList())
    {
        if(fileInfo.isFile())
        {
            if(!fileInfo.isWritable())
            {
                QFile file(fileInfo.absoluteFilePath());
                file.setPermissions(QFile::WriteOwner);
            }

            fileInfo.dir().remove(fileInfo.fileName());

        }
        else if(fileInfo.isDir())
        {
            clearDir(fileInfo.absoluteFilePath());
        }
    }
    dir.rmpath(dir.absolutePath());
}

void Video_Convert::traversalSource(QString path)
{

    if(path.isEmpty())
    {
        return;
    }
    QDir dir(path);

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    foreach(QFileInfo f, dir.entryInfoList())
    {
        if(f.isDir())
        {
            if(f.fileName()=="." || f.fileName() =="..")
                continue;
            qDebug()<<"Entry Dir"<<f.absoluteFilePath();
            traversalSource(f.absoluteFilePath());

        }
        else if(f.isFile())
        {
            QString filePath = f.absoluteFilePath();

            if (m_floor == m_convertList.length()) {
                // 解析json字符串
                if (filePath.contains("entry.json") || filePath.contains("index.json")) {
                    parseJson(filePath, stConvertInfo);
                    if (filePath.contains("entry.json")) {
                        qDebug()<<stConvertInfo._title + " " + stConvertInfo._part;
                    }
                } else if (filePath.contains("video.m4s")) {
                    stConvertInfo._video = filePath.replace("\\", "/");
                    stConvertInfo._type = "m4s";
                    qDebug()<<stConvertInfo._video;
                } else if(filePath.contains("audio.m4s")) {
                    stConvertInfo._audio = filePath.replace("\\", "/");;
                    qDebug()<<stConvertInfo._audio;
                } else if (filePath.endsWith(".blv")) {
                    stConvertInfo._blv.append(filePath.replace("\\", "/"));\
                    stConvertInfo._type = "blv";
                } else if (filePath.endsWith(".m3u8")) {
                    stConvertInfo._type = "m3u8";

                    /*# 寻找同级目录文件夹作为m3u8实际视频存放位置，跟该索引文件的层级关系为index
                    #                                                                  |
                    #                                                                folder->real m3u8 video
                    # 取得m3u8 index之前的路径
                    */
                    QString m3u8_folder = filePath.replace("\\", "/");
                    // # :不包括最后一个'/'位置，以得到文件父的路径
                    m3u8_folder = m3u8_folder.mid(0, m3u8_folder.lastIndexOf('/'));
                    QFile f(filePath);
                    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
                    QTextStream in(&f);
                    QStringList lineStrs;
                    while(!in.atEnd()) {
                        lineStrs.append(in.readLine());
                    }
                    foreach(QString lineStr, lineStrs) {
                        lineStr = lineStr.replace("\n", "");
                        if (!lineStr.contains("#EXT")) {
                            QStringList splitStr = lineStr.split('/');
                            // # 例如：E:/Quark/Download/1/a (1,a为后补的两个字符串)
                            stConvertInfo._m3u8.append(
                                m3u8_folder+'/'+splitStr[splitStr.length()-2]+'/'+splitStr[splitStr.length()-1]);
                        }
                    }
                    QStringList splitFilePath = filePath.split("/");
                    QString fileHasExt = splitFilePath[splitFilePath.length()-1];
                    stConvertInfo._title = fileHasExt.mid(0, fileHasExt.length()-5); // 减去".m3u8"长度5  //# 获取去除路径与后缀名以后的单独文件名
                    stConvertInfo._title = stConvertInfo._title.replace(" ", "");
                    if (stConvertInfo._title != "") {
                        m_convertList.append(stConvertInfo);
                        stConvertInfo.init();
                        m_floor++;
                    }
                }
                if (stConvertInfo._type == "m4s" && !stConvertInfo._title.isEmpty() &&
                       !stConvertInfo._video.isEmpty() && !stConvertInfo._audio.isEmpty() ) {
                    m_convertList.append(stConvertInfo);
                    stConvertInfo.init();
                    m_floor++;
                } else if(stConvertInfo._type == "blv" && !stConvertInfo._title.isEmpty() &&
                          !stConvertInfo._part.isEmpty() && stConvertInfo._blv_num > 0 &&
                          stConvertInfo._blv.length() >= stConvertInfo._blv_num) {
                    m_convertList.append(stConvertInfo);
                    stConvertInfo.init();
                    m_floor++;
                }
            }
        }
    }
}

bool Video_Convert::parseJson(QString filePath, st_convert_info &stConvertInfo)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        return false;
    }
    QJsonObject jsonObject = document.object();
    if (filePath.contains("entry.json")) {
        QString title = jsonObject["title"].toString();
        stConvertInfo._title = validateTitle(title);
        if (jsonObject.contains((QStringLiteral("page_data")))) {
            QJsonValue jsonValueList = jsonObject.value(QStringLiteral("page_data"));
            QJsonObject item = jsonValueList.toObject();
            if (item.contains(QStringLiteral("part"))) {
                stConvertInfo._part = item["part"].toString();
            } else {
                stConvertInfo._part = title;
            }
            if (item.contains(QStringLiteral("page"))) {
                stConvertInfo._episode_index = item["page"].toInt();
            }
        } else {
            QJsonValue jsonValueList = jsonObject.value(QStringLiteral("ep"));
            QJsonObject item = jsonValueList.toObject();
            stConvertInfo._part = item["index"].toString();
        }
       stConvertInfo._part = validateTitle(stConvertInfo._part);
    } else if(filePath.contains("index.json")) {
        if (jsonObject.contains((QStringLiteral("segment_list")))) {
            QJsonValue arrayValue = jsonObject.value((QStringLiteral("segment_list")));
            if (arrayValue.isArray())
            {
                QJsonArray array = arrayValue.toArray();
                stConvertInfo._blv_num = array.size();
            }
        }
    }


    return  true;
}

void Video_Convert::run()
{
    int index = 0;
    QDir dir;

    QString mergeName = "mergeflv.txt";
    QString mergeTsName = "mergets.txt";
    foreach(auto video_info, m_convertList) {
        QString outDir = m_outDir;
        // 如果是合集，则根据合集标题创建新的文件夹
       if (video_info._part !="" && video_info._title != video_info._part)  {
           outDir +='/' + video_info._title;
       }
       if(!dir.exists(outDir)) {
           dir.mkdir(outDir);
       }
       if (video_info._part !="" && video_info._title != video_info._part)  {
            emit updateUI(1, video_info._title+'/'+video_info._part + "准备开始!", index);
       } else {
           emit updateUI(1, video_info._title + "准备开始", index);
       }
       // 执行ffmpeg转码操作
       QString outFile;
       QString fileName = video_info._part=="" ? video_info._title : video_info._part;
       if ( video_info._episode_index > 0 && video_info._part!="" && video_info._part != video_info._title) {
           outFile = outDir + '/' + QString::number(video_info._episode_index) + '.' + fileName;
       }
       else {
           outFile = outDir +'/' + fileName;
       }

       qDebug()<<outFile+".mp4开始合并";
       QString command = "";
       bool bCommand = true;
       if (video_info._type == "m4s") {
           command = QString("ffmpeg -i "+ video_info._video + " -i " + video_info._audio + " -codec copy " + outFile+".mp4");
       } else if(video_info._type == "blv") {
            if (video_info._blv_num == 1) // 单文件直接移动重命名
            {
                bCommand = false;
                QFile::copy(video_info._blv[0], outFile+".mp4");
            } else { // 多文件合并转换输出
                if (QFile::exists(mergeName)) {
                    QFile::remove(mergeName);
                    foreach(auto blv_file, video_info._blv) {
                        // 将文件重命名为flv文件
                        QString blv_file_no_ext = blv_file.remove(".blv");
                        QString blv_file_add_ext = blv_file_no_ext + ".flv";
                        // 重命名文件
                        QFile::rename(blv_file, blv_file_add_ext);
                        // 追加打开合并脚本,不存在则创建
                        QFile fileMerge(mergeName);
                        if (fileMerge.open(QIODevice::Append)) {
                            fileMerge.write("file ");
                            QString blv_file_write = "'"+blv_file_add_ext+"'"+"\n";
                            fileMerge.write(blv_file_write.toStdString().c_str());
                            fileMerge.close();
                        }
                    }
                    // 多个flv文件合并                    
                    command = QString("ffmpeg -f concat -safe 0 -i " + mergeName + " -codec copy " + outFile + ".mp4");
                }
            }

       } else if (video_info._type == "m3u8") {
            // 合并m3u8文件
            if(QFile::exists(mergeTsName)) {
                QFile::remove(mergeTsName);
            }
            foreach(auto ts_file, video_info._m3u8) {
                QString ts_file_add_ext = ts_file;
                if(!QFile::exists(ts_file+".ts")) {
                    if (!QFile::exists(ts_file)) {
                        continue;
                    }
                    QString ext_name = ts_file.split(".")[1];
                    if (ext_name =="") {
                        ts_file_add_ext += ".ts";
                        QFile::rename(ts_file,ts_file_add_ext);

                    } else if (ext_name !=".ts") {
                        continue;
                    }
                } else { // ts文件存在
                    ts_file_add_ext += ".ts";
                }
                // 追加打开合并脚本,不存在则创建
                QFile fileMerge(mergeTsName);
                if (fileMerge.open(QIODevice::Append)) {
                    fileMerge.write("file ");
                    QString ts_file_write = "'"+ts_file_add_ext+"'"+"\n";
                    fileMerge.write(ts_file_write.toStdString().c_str());
                    fileMerge.close();
                }
            }
            // 多个ts文件合并
            if (QFile::exists(mergeTsName)) {
                command = QString("ffmpeg -f concat -safe 0 -i " + mergeTsName + " -codec copy " + outFile + ".mp4");
            }

        } else { // 格式错误
           emit updateUI(4, video_info._type,index);
           return;
       }
       QString standardOutput("");
       if (bCommand) { // 调用外部程序
           QProcess* process = new QProcess(nullptr);
           process->setReadChannel(QProcess::StandardOutput);
           process->setProcessChannelMode(QProcess::MergedChannels);

           process->start(command);
           if (!process->waitForStarted(-1)) {
               emit updateUI(5, outFile+"\n启动合并失败",index);
               return;
           }
           if (!process->waitForFinished(-1)) {
               emit updateUI(6, outFile+"\n合并失败",index);
               return;
           }
           standardOutput = process->readAllStandardOutput();
            if (m2Mp3Checked) { // 转换mp3
                process->start("ffmpeg -i "+outFile+".mp4 "+outFile+".mp3");
                if (!process->waitForStarted(-1)) {
                    emit updateUI(5, outFile+"\n转换音频mp3失败",index);
                    return;
                }
                if (!process->waitForFinished(-1)) {
                    emit updateUI(6, outFile+"\n转换音频mp3失败",index);
                    return;
                }
                // 删除mp4源文件
                QFile::remove(outFile+".mp4");
                standardOutput += process->readAllStandardOutput();
            }
            delete process;
       }

       // 当前合并完成
       QString alertTypeMessage;
       if (m2Mp3Checked) {
           alertTypeMessage = "合并并转换mp3完成! 输出信息:\n";

       } else
       {
           alertTypeMessage = "合并完成! 输出信息:\n";
       }
       emit updateUI(2, video_info._title+"/"+video_info._part + " " +alertTypeMessage + standardOutput+"\n", index);

       index++;
    }
    // 删除中间件
    if (QFile::exists(mergeName)) {
         QFile::remove(mergeName);
    }
    if (QFile::exists(mergeTsName)) {
         QFile::remove(mergeTsName);
    }
    emit updateUI(3, "温馨提示,转码工作全部完成!", index);

}
