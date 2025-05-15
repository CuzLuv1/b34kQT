// RelPOS.h
#ifndef RELPOS_H
#define RELPOS_H
#include "Config.h"

#include "ui_RelPOS.h"  // 由 CMake 生成的 UI 头文件
#include <QWidget>

class RelPOS : public QWidget {
    Q_OBJECT

public:
    explicit RelPOS(Config* configPage,QWidget *parent = nullptr);
    ~RelPOS();

private:
    Ui::RelPOS ui;
    Config* configpage; // 配置页面指针

private slots:
    void handleConfigSingal(solopt_t &solopt1, prcopt_t &prcopt1);//处理配置信号
    int readfilepath(); //读取文件路径
    void calrelPos(); //计算相对定位
};
#endif // RelPOS_H