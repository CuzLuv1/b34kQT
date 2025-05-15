// PointPOS.cpp
#include "include/PointPOS.h"
#include "include/Config.h"
#include "include/b34k.h"
extern "C" {
#include "rtklib/rtklib.h"
}
#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <QFileDialog>
#include "timeconv.h"

namespace fs = std::filesystem;
using namespace std;
#define MAXFILE 16

PointPOS::PointPOS(Config* configPage,QWidget *parent) : QWidget(parent), configpage(configPage) {
    ui.setupUi(this);
    if (!configpage) 
    {
        qDebug() << "configpage 为空！";
    }

    //设置默认路径
    ui.obspath->setText("D:\\\\Code\\\\code\\\\RTKlib\\\\IGSOBS\\\\infile\\\\ahme2420.24o");
    ui.brdcpath->setText("D:\\\\Code\\\\code\\\\RTKlib\\\\BDSQT23\\\\OBS\\\\brdc2420.24n");
    ui.sp3path->setText("D:\\\\Code\\\\code\\\\RTKlib\\\\BDSQT23\\\\OBS\\\\GBM0MGXRAP_20242420000_01D_05M_ORB.SP3");
    ui.clkpath->setText("D:\\\\Code\\\\code\\\\RTKlib\\\\BDSQT23\\\\OBS\\\\GBM0MGXRAP_20242420000_01D_30S_CLK.CLK");
    ui.result->setText("D:\\\\Code\\\\code\\\\RTKlib\\\\IGSOBS\\\\infile\\\\result");
    connect(ui.readobsfile, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readrnxfile, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readsp3file, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.readclkfile,SIGNAL(clicked()),this,SLOT(readfilepath()));
    connect(configPage, &Config::configchanged, this, &PointPOS::handleConfigSingal);
    connect(ui.calpos, SIGNAL(clicked()), this, SLOT(calPointPos()));
    connect(ui.calendarWidget, &QCalendarWidget::clicked, this, &PointPOS::onCalendarDateClicked);
}   

PointPOS::~PointPOS() {}

//时间转换
void PointPOS::onCalendarDateClicked(const QDate &date) {
     //时间转换
    ui.Juliantext->setText(time2julian(date));
    ui.daytext->setText(time2day(date));
    QStringList gpsday = time2GPSday(date).split(" ");
    ui.GPSWeektext->setText(gpsday[0]);
    ui.GPSDaytext->setText(gpsday[1]);

}

//文件路径读取 
//return 0：失败 1：成功
int PointPOS::readfilepath() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString filter;
    QString caption;
    QString filepath;
    // 读取文件路径
    if(button==ui.readobsfile){
        filter = tr("O文件(*.??O *.??o *.rnx *.RNX *.obs *.OBS)");
        caption = tr("读取观测文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.obspath->setText(filepath);
        QFileInfo fileInfo(filepath);
        ui.result->setText(fileInfo.absolutePath().replace("/", "\\\\"));
    }
    else if(button==ui.readrnxfile){
        filter = tr("N文件(*.??N *.??n *.rnx *.RNX)");
        caption = tr("读取导航文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.brdcpath->setText(filepath);
    }
    else if(button==ui.readsp3file){
        filter = tr("轨道文件(*.sp3 *.SP3)");
        caption = tr("读取精密星历文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.sp3path->setText(filepath);
    }
    else if(button==ui.readclkfile){
        filter=tr("钟差文件(*.clk *.CLK)");
        caption=tr("读取钟差文件");
        filepath=QFileDialog::getOpenFileName(this,caption,"/",filter);
        filepath = filepath.replace("/", "\\\\");
        ui.clkpath->setText(filepath);
    }
    return 0;
}

//卫星系统类型
void getnavsys(prcopt_t *popt)
{
    int navsys;
    cout<<"请输入导航系统类型："<<endl;
    cout<<"1:GPS    2:BDS-2"<<endl;
    cout<<"3:BDS-3  4:BDS-2/3"<<endl;
    cin>>navsys;
    if(navsys==1)
    {
        popt->navsys=SYS_GPS;
    }
    else if(navsys==2)
    {
        popt->navsys=SYS_CMP;
        popt->BDSType = 2;
    }
    else if(navsys==3)
    {
        popt->navsys=SYS_CMP;
        popt->BDSType = 3;
    }
    else if(navsys==4)
    {
        popt->navsys=SYS_CMP;
        popt->BDSType = 1;
    }
    else
    {
        cout<<"输入错误！"<<endl;
        return;
    }
    return;
}

int getBDStype(uint8_t sat)
{
    if (!(satsys(sat, NULL) & SYS_CMP))   //非CMP
	return 0;
    if ((106 <= sat && sat <= 119) || sat == 121 || sat == 123)    //注意程序里北斗的sat号，为105+PRN
	    return 2;//BDS-2
    else return 3;//BDS-3   
}

double* recant(QString obspath)
{
    //读取观测文件，获取接收机天线参数
    QFile file(obspath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件";
        return nullptr;
    }
    QTextStream in(&file);
    QString line;
    double* recant = new double[3];
    int i = 0;
    while (!in.atEnd()) {
        line = in.readLine();
        if(line.contains("ANTENNA: DELTA H/E/N"))
        {
            recant[2] = line.sliced(8,6).toDouble(); //天线高度
            recant[0]= line.sliced(22,6).toDouble(); //天线东向偏移
            recant[1]= line.sliced(36,6).toDouble(); //天线北向偏移
            break;
        }	
    }
    file.close();
    return recant;
}

//处理配置选项信号
void PointPOS::handleConfigSingal(solopt_t &solopt1, prcopt_t &prcopt1,filopt_t &filopt1) {
    // 处理配置选项信号
    solopt = solopt1;
    prcopt = prcopt1;
    filopt = filopt1;
    // qDebug() << "配置选项已更新！";
}

int PointPOS::calPointPos() {
    /*单点定位计算*/

    // 文件路径
    QString obspath = ui.obspath->text();   // 观测文件路径
    QString brdcpath = ui.brdcpath->text(); // 导航文件路径
    QString sp3path = ui.sp3path->text();   // 精密星历文件路径
    QString clkpath = ui.clkpath->text();   // 钟差文件路径

    // 天线改正
    double* recant1 =new double[3];
    recant1=recant(obspath); //接收机天线参数
    prcopt.antdel[0][0]=recant1[0]; //天线东向偏移 
    prcopt.antdel[0][1]=recant1[1]; //天线北向偏移
    prcopt.antdel[0][2]=recant1[2]; //天线高度
    

    QString modern=ui.modern->currentText();  //模型
    std::vector<std::string> infilePaths;
    if(modern=="PPP-Static"||modern=="PPP-Kinematic"){
        infilePaths = {
            obspath.toStdString(), 
            brdcpath.toStdString(), 
            sp3path.toStdString(), 
            clkpath.toStdString(),
            "", "", ""
        };
        if(modern=="PPP-Static"){
            prcopt.mode = PMODE_PPP_STATIC;
        }
        else if(modern=="PPP-Kinematic"){
            prcopt.mode = PMODE_PPP_KINEMA;
        }
    }

    else if(modern=="SPP")
    {
        prcopt.mode = PMODE_SINGLE;
        infilePaths = {
            obspath.toStdString(), 
            brdcpath.toStdString(), 
            "", "",
            "", "", ""
        };
    }

    configpage->configstore(solopt, prcopt,filopt); // 更新配置选项
    //prcopt.BDSType = 3; //BDS-3

    // 移除空路径
    infilePaths.erase(std::remove_if(infilePaths.begin(), infilePaths.end(),
                      [](const std::string& path) { return path.empty(); }),
                      infilePaths.end());

    // 输出文件路径
    char sep = fs::path::preferred_separator;
    string outfile = ui.result->text().replace("\\\\","\\").toStdString();
     
    //时间
    QDate date=ui.calendarWidget->selectedDate();
    double year=date.year(); 
    double month=date.month();
    double day=date.day();
    QTime time=ui.timeEdit->time();
    double hour=time.hour();    
    double minute=time.minute();
    double second=time.second();
    QTime time1=ui.timeEdit_2->time();
    double hour1=time1.hour();
    double minute1=time1.minute();
    double second1=time1.second();
    double eps[]={year,month,day,hour,minute,second};//起始时间
    double epe[]={year,month,day,hour1,minute1,second1};//结束时间
    //时间间隔
    int interval=ui.Internal->currentIndex();
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
    
    return 0;
}

