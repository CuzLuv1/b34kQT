// PointPOS.h
#ifndef POINTPOS_H
#define POINTPOS_H
#include "Config.h"
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include "rtklib/rtklib.h"

#include "ui_PointPOS.h"  // 由 CMake 生成的 UI 头文件
#include <QWidget>

class PointPOS : public QWidget {
    Q_OBJECT

public:
    explicit PointPOS(Config* configPage, QWidget *parent = nullptr);
    ~PointPOS();
    void useconfigstore();//使用配置存储函数

private:
    Ui::PointPOS ui;
    Config* configpage; // 配置页面指针

private slots:
    int readfilepath();//读取文件路径
    int calPointPos();//单点定位解算
    void handleConfigSingal(solopt_t &solopt1, prcopt_t &prcopt1,filopt_t &filopt1);//处理配置信号
    void onCalendarDateClicked(const QDate &date);

};
#endif // POINTPOS_H