// RelPOS.cpp
#include "include/RelPOS.h"
#include "include/Config.h"
#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <QFileDialog>
#include <QMessageBox>

namespace fs = std::filesystem;
using namespace std;
#define MAXFILE 16


RelPOS::RelPOS(Config* configPage,QWidget *parent) : QWidget(parent),configpage(configPage) {
    ui.setupUi(this);
    //文件路径读取
    connect(ui.readbasefile_rel, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readrovfile_rel, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readrnxfile_rel, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readsp3file_rel, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readclkfile_rel,SIGNAL(clicked()),this,SLOT(readfilepath()));
    //配置选项读取 & 解算
    connect(configPage, &Config::configchanged, this, &RelPOS::handleConfigSingal);
    connect(ui.calpos_rel, SIGNAL(clicked()), this, SLOT(calrelPos()));
}

RelPOS::~RelPOS() {}

//文件路径读取 
//return 0：失败 1：成功
int RelPOS::readfilepath() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString filter;
    QString caption;
    QString filepath;
    // 读取文件路径
    if(button==ui.readbasefile_rel){
        filter = tr("O文件(*.??O *.??o *.rnx *.RNX)");
        caption = tr("读取观测文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.obspath_rel->setText(filepath);
        QFileInfo fileInfo(filepath);
        ui.result_rel->setText(fileInfo.absolutePath().replace("/", "\\\\"));
    }
    else if(button==ui.readrovfile_rel){
        filter = tr("O文件(*.??O *.??o *.rnx *.RNX)");
        caption = tr("读取观测文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.rovpath_rel->setText(filepath);
    }
    else if(button==ui.readrnxfile_rel){
        filter = tr("N文件(*.??N *.??n *.rnx *.RNX)");
        caption = tr("读取导航文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.brdcpath_rel->setText(filepath);
    }
    else if(button==ui.readsp3file_rel){
        filter = tr("轨道文件(*.sp3 *.SP3)");
        caption = tr("读取精密星历文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.sp3path_rel->setText(filepath);
    }
    else if(button==ui.readclkfile_rel){
        filter=tr("钟差文件(*.clk *.CLK)");
        caption=tr("读取钟差文件");
        filepath=QFileDialog::getOpenFileName(this,caption,"/",filter);
        filepath = filepath.replace("/", "\\\\");
        ui.clkpath_rel->setText(filepath);
    }
    return 1;
}



//处理配置选项信号
void RelPOS::handleConfigSingal(solopt_t &solopt1, prcopt_t &prcopt1) {
    // 处理配置选项信号
    solopt = solopt1;
    prcopt = prcopt1;
    // qDebug() << "配置选项已更新！";
}


void RelPOS::calrelPos()
{
    /*相对定位*/
    // 文件路径
    QString basepath = ui.obspath_rel->text();   
    QString rovpath = ui.rovpath_rel->text();// 观测文件路径
    QString brdcpath = ui.brdcpath_rel->text(); // 导航文件路径
    QString sp3path = ui.sp3path_rel->text();   // 精密星历文件路径
    QString clkpath = ui.clkpath_rel->text();   // 钟差文件路径

    QString modern=ui.modern_rel->currentText();  //模型
    std::vector<std::string> infilePaths;
    if(modern=="Static"||modern=="Kinematic"){
        infilePaths = {
            basepath.toStdString(), 
            rovpath.toStdString(),
            brdcpath.toStdString(), 
            sp3path.toStdString(), 
            clkpath.toStdString(),
            "", ""
        };
        if(modern=="Static"){
            prcopt.mode = PMODE_STATIC;
        }
        else if(modern=="PPP-Kinematic"){
            prcopt.mode = PMODE_KINEMA;
        }
    }

    else if(modern=="DGPS")
    {
        prcopt.mode = PMODE_DGPS;
        infilePaths = {
            basepath.toStdString(), 
            rovpath.toStdString(),
            brdcpath.toStdString(), 
            "",
            "", "", ""
        };
    }

    configpage->configstore(solopt, prcopt,filopt); // 更新配置选项

    // 移除空路径
    infilePaths.erase(std::remove_if(infilePaths.begin(), infilePaths.end(),
                      [](const std::string& path) { return path.empty(); }),
                      infilePaths.end());

    // 输出文件路径
    char sep = fs::path::preferred_separator;
    string outfile = ui.result_rel->text().replace("\\\\","\\").toStdString() + sep + "result_rtk.pos"; 
     
    //时间
    QDate date=ui.calendarWidget1->selectedDate();
    double year=date.year(); 
    double month=date.month();
    double day=date.day();
    QTime time=ui.timeEdit_rel->time();
    double hour=time.hour();    
    double minute=time.minute();
    double second=time.second();
    QTime time1=ui.timeEdit_rel_2->time();
    double hour1=time1.hour();
    double minute1=time1.minute();
    double second1=time1.second();
    double eps[]={year,month,day,hour,minute,second};//起始时间
    double epe[]={year,month,day,hour1,minute1,second1};//结束时间
    //时间间隔
    int interval=ui.Internal_rel->currentIndex();
    double tint;
    switch (interval)
    {
    case 0:
        tint=1.0;
        break;
    case 1:
        tint=30.0;
        case 2:
        tint=60.0;
        break;
    default:
        tint=0.0;
        break;
    }
    gtime_t ts = epoch2time(eps), te = epoch2time(epe);

    // 创建 const char* 指针数组
    std::vector<const char*> infilePathsCStr;
    for (const auto& path : infilePaths) {
        infilePathsCStr.push_back(path.c_str());
    }

    // 调用 postpos 函数
    auto start = chrono::high_resolution_clock::now();
    int ret = postpos(ts, te, 0.0, 0.0, &prcopt, &solopt, &filopt,
                      infilePathsCStr.data(), infilePaths.size(), 
                      outfile.c_str(), "", "");
    auto end = chrono::high_resolution_clock::now();

    // 输出运行时间
    chrono::duration<double> elapsed = end - start;
    // cout << "程序运行时间: " << elapsed.count() << " 秒" << endl;
    QMessageBox::information(this, "解算提示", "解算成功！" + QString::number(elapsed.count()) + "秒");

    return;
}