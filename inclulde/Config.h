// Config.h
#ifndef Config_H
#define Config_H

#include "ui_Config.h"  // 由 CMake 生成的 UI 头文件
#include "rtklib/rtklib.h"
#include <QWidget>

extern solopt_t solopt; // 声明外部变量 solopt
extern prcopt_t prcopt; // 声明外部变量 prcopt
extern filopt_t filopt; // 声明外部变量 filopt

class Config : public QWidget {
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();
    void configstore(solopt_t &solopt1, prcopt_t &prcopt1,filopt_t &filopt1); // 配置存储函数

private:
    Ui::Config ui;

private slots:
    void readfilepath(); // 读取观测文件按钮槽函数

signals:
    void configchanged(solopt_t &solopt1, prcopt_t &prcopt1,filopt_t &filopt1); // 配置改变信号
    
    
};
#endif // Config_H