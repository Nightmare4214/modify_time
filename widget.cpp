#include "widget.h"
#include "ui_widget.h"
#include<QFileDialog>
#include<QDebug>
#include <QQueue>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->creation_time->setDateTime(QDateTime::currentDateTime());
    ui->last_write_time->setDateTime(QDateTime::currentDateTime());
    ui->last_access_time->setDateTime(QDateTime::currentDateTime());
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_file_btn_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,tr("选择文件"),tr("."),tr("*"));
    if(!path.isNull()&&!path.isEmpty()){
        ui->file_label->setText(path);
        HANDLE handle = CreateFile(
            reinterpret_cast<const wchar_t*>(path.utf16()),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr);
        if (INVALID_HANDLE_VALUE == handle) {
            CloseHandle(handle);
        }
        else{
            FILETIME ctime,atime,mtime;
            int result=GetFileTime(handle,&ctime,&atime,&mtime);
            QDateTime creation_time,last_access_time,last_write_time;
            file_time2qdatetime(ctime,creation_time);
            file_time2qdatetime(atime,last_access_time);
            file_time2qdatetime(mtime,last_write_time);

            ui->creation_time->setDateTime(creation_time);
            ui->last_access_time->setDateTime(last_access_time);
            ui->last_write_time->setDateTime(last_write_time);
            if(!result){
                qDebug()<<"Get time fail:"<<path<<endl;
            }
            CloseHandle(handle);
        }
    }
}

void Widget::on_directory_btn_clicked()
{
    QString path=QFileDialog::getExistingDirectory(this,tr("打开目录"),tr("."));
    if(!path.isNull()&&!path.isEmpty()){
        ui->file_label->setText(path);
    }
}

void Widget::on_ok_btn_clicked()
{
    QString path=ui->file_label->text();
    if(path.isNull()||path.isEmpty()){
        return;
    }
    QFileInfo file_info(path);
    QDateTime creation_time=ui->creation_time->dateTime();
    QDateTime last_write_time=ui->last_write_time->dateTime();
    QDateTime last_access_time=ui->last_access_time->dateTime();
    if(file_info.isFile()){
        bool result=modify(file_info.filePath(),creation_time,last_write_time,last_access_time);
        if(!result){
            qDebug()<<file_info.filePath()<<endl;
        }
    }
    else{
        QFileInfo cur_fileinfo(path);
        QQueue<QFileInfo> q;    // queue
        q.enqueue(cur_fileinfo);
        while (!q.isEmpty()) {
            QDir cur_dir(q.head().absoluteFilePath());
            q.dequeue();
            QFileInfoList cur_fileinfolist = cur_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot|QDir::Files | QDir::Hidden);

            QFileInfoList::const_iterator it;
            for (it = cur_fileinfolist.cbegin(); it != cur_fileinfolist.cend(); ++it) {
                // jump "." and ".."
                if (it->isDir()) {
                    bool result=modify(it->absoluteFilePath(),creation_time,last_write_time,last_access_time);
                    q.enqueue(it->absoluteFilePath());    // enqueue unresolved directories
                    if(!result){
                        qDebug()<<it->absoluteFilePath()<<endl;
                    }
                } else if (!it->isDir()){
                    bool result=modify(it->absoluteFilePath(),creation_time,last_write_time,last_access_time);
                    if(!result){
                        qDebug()<<it->absoluteFilePath()<<endl;
                    }
                }
            }
        }
        bool result=modify(file_info.filePath(),creation_time,last_write_time,last_access_time);
        if(!result){
            qDebug()<<path<<endl;
        }
    }
}

void Widget::file_time2qdatetime(FILETIME &time, QDateTime &s)
{
    SYSTEMTIME rtime;
    FILETIME ltime;
    FileTimeToLocalFileTime(&time,&ltime);
    FileTimeToSystemTime(&ltime,&rtime);
    s.setDate(QDate(rtime.wYear,rtime.wMonth,rtime.wDay));
    s.setTime(QTime(rtime.wHour,rtime.wMinute,rtime.wSecond,rtime.wMilliseconds));
}
/**
 * @brief 字符串转FILETIME
 * @param s 字符串
 * @param result FILETIME
 */
void Widget::string2file_time(const QDateTime &s, FILETIME &result)
{
    SYSTEMTIME temp;
    QDate temp_date=s.toUTC().date();
    QTime temp_time=s.toUTC().time();
    temp.wYear=static_cast<WORD>(temp_date.year());
    temp.wMonth=static_cast<WORD>(temp_date.month());
    temp.wDay=static_cast<WORD>(temp_date.day());
    temp.wHour=static_cast<WORD>(temp_time.hour());
    temp.wMinute=static_cast<WORD>(temp_time.minute());
    temp.wSecond=static_cast<WORD>(temp_time.second());
    //temp.wDayOfWeek=0;
    temp.wMilliseconds=0;

    SystemTimeToFileTime(&temp,&result);
}
/**
 * @brief 修改文件的目录
 * @param path 路径
 * @param creation_time 创建时间
 * @param last_write_time 最后修改时间
 * @param last_access_time 最后访问时间
 * @return 是否成功
 */
bool Widget::modify(const QString &path, const QDateTime &creation_time, const QDateTime &last_write_time, const QDateTime &last_access_time)
{
    HANDLE handle = CreateFile(
        reinterpret_cast<const wchar_t*>(path.utf16()),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);
    try {
        if (INVALID_HANDLE_VALUE == handle) {
            qDebug()<<"INVALID_HANDLE_VALUE "<<path<<endl;
            CloseHandle(handle);
            return false;
        }
        FILETIME ctime, atime, mtime;
        string2file_time(creation_time,ctime);
        string2file_time(last_write_time,mtime);
        string2file_time(last_access_time,atime);
        BOOL retval = SetFileTime(handle, &ctime, &atime, &mtime);
        CloseHandle(handle);
        return retval;
    }
    catch (...) {
        qDebug()<<path<<endl;
        CloseHandle(handle);
        return false;
    }
    //return true;
}
