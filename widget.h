#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <Windows.h>
#include <ATLComTime.h>
#include <string>
#include <io.h>
using namespace std;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_file_btn_clicked();

    void on_directory_btn_clicked();

    void on_ok_btn_clicked();

private:
    Ui::Widget *ui;
    void file_time2qdatetime(FILETIME& time,QDateTime& s);
    void string2file_time(const QDateTime& s, FILETIME& result);
    bool modify(
        const QString& path,
        const QDateTime& creation_time,
        const QDateTime& last_write_time,
        const QDateTime& last_access_time);
};

#endif // WIDGET_H
