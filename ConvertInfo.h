#ifndef CONVERTINFO_H
#define CONVERTINFO_H
#include<QString>
#include<QStringList>
#include<QObject>
#include<QThread>

#endif // CONVERTINFO_H

typedef struct convert_info
{
    QString _type; // 类型: m4s,blv,m3u8
    QString _title; // 标题
    QString _part; // 具体的名字
    QString _video; // 视频全路径
    QString _audio; // 音频全路径
    QStringList _blv; // blv分段全路径列表
    int _blv_num; // blv分段数量
    QStringList _m3u8; // m3u8分段全路径列表
    int _episode_index; // 合集中视频索引，用于排序
    convert_info()
    {
        init();
    }
    void init()
    {
        _type.clear();
        _title.clear();
        _part.clear();
        _video.clear();
        _audio.clear();
        _blv.clear();
        _m3u8.clear();
        _blv_num = 0;
        _episode_index = -1;
    }

}st_convert_info;
/*
 * 视频转码操作类
*/
class Video_Convert : public QThread
{
    Q_OBJECT
public:
    Video_Convert(QObject* parent = 0) :QThread(parent)
    {
        init();
    }

public:
    QString validateTitle(QString title);
    void clearDir(QString path); // 清空目录
    void traversalSource(QString path); // 遍历目录
    QString join(QString path1, QString path2);
    bool parseJson(QString filePath, st_convert_info& stConvertInfo);
    void init();

protected:
    void run(); // 新线程入口
signals:
    void updateUI(int type, QString str, int index);
public:
    QList<st_convert_info> m_convertList; // 转换视频源信息
    st_convert_info     stConvertInfo;
    int m_floor; // 层数
    QString m_outDir;
    QThread m_thread; // 转码工作另开线程操作

};
