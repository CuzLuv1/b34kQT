// Config.cpp
#include "include/Config.h"
#include "rtklib/rtklib.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>

Config::Config(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    connect(ui.satpcv, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.recpcv, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.dcb, SIGNAL(clicked()), this, SLOT(readfilepath()));
    connect(ui.ionfile,SIGNAL(clicked()),this,SLOT(readfilepath()));
}

void Config::readfilepath() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString filter;
    QString caption;
    QString filepath;
    // 读取文件路径
    if(button==ui.satpcv){
        filter = tr("天线文件(*.atx *.ATX)");
        caption = tr("读取天线文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.satpcvpath->setText(filepath);
    }
    else if(button==ui.recpcv){
        filter = tr("天线文件(*.atx *.ATX)");
        caption = tr("读取天线文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.recpcvpath->setText(filepath);
    }
    else if(button==ui.dcb){
        filter = tr("轨道文件(*.dcb *.DCB)");
        caption = tr("读取精密星历文件");
        filepath = QFileDialog::getOpenFileName(this, caption, "/", filter);
        filepath = filepath.replace("/", "\\\\");
        ui.dcbpath->setText(filepath);
    }
    else if(button==ui.ionfile){
        filter=tr("钟差文件(*.??i *.??I)");
        caption=tr("读取钟差文件"); 
        filepath=QFileDialog::getOpenFileName(this,caption,"/",filter);
        filepath = filepath.replace("/", "\\\\");
        ui.ionpath->setText(filepath);
    }
}

void Config::configstore(solopt_t &solopt1,prcopt_t &prcopt1,filopt_t &filopt1)
{
    //solopt1设置
    solopt1.timef = 1;               // 时间格式
    solopt1.outhead = 1;             // 输出头文件
    solopt1.times = TIMES_UTC; // 时间系统
    solopt1.outopt = 1;// 输出 prcopt 变量
    solopt1.height = 0;// 高程类型
    //输出坐标格式
    if(ui.posf->currentText()=="XYZ"){solopt1.posf=SOLF_XYZ;}
    else if(ui.posf->currentText()=="LLH"){solopt1.posf=SOLF_LLH;}
    else if(ui.posf->currentText()=="ENU"){solopt1.posf=SOLF_ENU;}
    //stat文件等级
    if(ui.stat->currentText()=="2"){solopt1.sstat=2;}
    if(ui.stat->currentText()=="1"){solopt1.sstat=1;}
    if(ui.stat->currentText()=="3"){solopt1.sstat=3;}
    //TRACE文件等级
    if(ui.trace->currentText()=="3"){solopt1.trace=3;}
    if(ui.trace->currentText()=="4"){solopt1.trace=4;}
    if(ui.trace->currentText()=="5"){solopt1.trace=5;}

    //prcopt设置
    prcopt1.tidecorr=1; //潮汐改正
    if(ui.smode->currentText()=="高度角模型"){prcopt1.smode=0;}
    else if(ui.smode->currentText()=="残差优化模型"){prcopt1.smode=0;}
    else if(ui.smode->currentText()=="信噪比模型"){prcopt1.smode=0;}
    //模糊度固定模式
    int modear=ui.modear->currentIndex();
    switch (modear)
    {
    case 0:
        prcopt1.modear=ARMODE_CONT;
        break;
    case 1:
        prcopt1.modear=ARMODE_INST;
        break;
    case 2:
        prcopt1.modear=ARMODE_OFF;
        break;
    default:
        prcopt1.modear=ARMODE_CONT;
        break;
    }
    //频率
    int freq=ui.nf->currentIndex();
    switch(freq)
    {
    case 0:
        prcopt1.nf=2;//L1+L2
        break;
    case 1:
        prcopt1.nf=1;//L1
        break;
    }

    //dynamic
    if(ui.dynamic->currentIndex()==0){prcopt1.dynamics=0;}
    else{prcopt1.dynamics=1;}

    //截止高度角
    int el=ui.elmin->currentIndex();
    switch (el)
    {
    case 0:
        prcopt1.elmin=10.0*D2R;
        break;
    case 1:
        prcopt1.elmin=15.0*D2R;
        break;
    case 2:
        prcopt1.elmin=20.0*D2R;
        break;
    }

    //滤波类型
    int soltype=ui.soltype->currentIndex();
    switch (soltype)
    {
    case 0://前向滤波
        prcopt1.soltype=0;
        break;
    case 1://后向滤波
        prcopt1.soltype=1;
        break;
    case 2://混合滤波
        prcopt1.soltype=2;
        break;
    }
    //基站坐标获取方式
    int refpos=ui.refpos->currentIndex();
    switch (refpos)
    {
    case 0://RINEX头文件
        prcopt1.refpos=4;
        break;
    case 1://伪距单点平均 
        prcopt1.refpos=1;
        break;
    case 2://自定义 
        prcopt1.refpos=0;
        break;
    }
    //移动站获取 单点定位不需要
    prcopt1.rovpos=0;
    prcopt1.bdsmodear = 1;  //北斗卫星的模糊度固定

    //误差
    int sateph=ui.eph->currentIndex();
    QString eph=ui.eph->currentText();
    switch (sateph)
    {   
    case 0:
        prcopt1.sateph=EPHOPT_BRDC;
        break;
    case 1:
        prcopt1.sateph=EPHOPT_PREC;
        break;
    case 2:
        prcopt1.sateph=EPHOPT_SBAS;//brdc=+sbas
        break;
    }
    
    //ion
    int ion=ui.ion->currentIndex();
    QString ionstr=ui.ion->currentText();
    switch (ion)
    {
    case 0:
        prcopt1.ionoopt=IONOOPT_OFF;
        break;
    case 1:
        prcopt1.ionoopt=IONOOPT_BRDC;
        break;
    case 2:
        prcopt1.ionoopt=IONOOPT_SBAS;
        break;
    case 3:
        prcopt1.ionoopt=IONOOPT_IFLC;
        break;
    }

    //trop
    int trop=ui.trop->currentIndex();
    QString tropstr=ui.trop->currentText();
    switch (trop)
    {
    case 0:
        prcopt1.tropopt=TROPOPT_OFF;
        break;
    case 1:
        prcopt1.tropopt=TROPOPT_SAAS;
        break;
    case 2:
        prcopt1.tropopt=TROPOPT_SBAS;
        break;
    case 3:
        prcopt1.tropopt=TROPOPT_EST;//ZTD
        break;
    }

    prcopt1.navsys = SYS_NONE; // 先清空
    
    //系统选择
    if (ui.gps->isChecked())     
    {prcopt1.navsys |= SYS_GPS;}
    if (ui.bds->isChecked())     
    {prcopt1.navsys |= SYS_CMP;prcopt1.BDSType = 1;} // BDS-2/3
    if (ui.galileo->isChecked()) 
    {prcopt1.navsys |= SYS_GAL;}
    if (ui.qzss->isChecked())    
    {prcopt1.navsys |= SYS_QZS;}

    //文件路径
    QString satpcvpath = ui.satpcvpath->text(); // 获取卫星天线文件路径
    QString recpcvpath = ui.recpcvpath->text(); // 获取接收机天线文件路径
    QString dcbpath = ui.dcbpath->text(); // 获取DCB码偏差文件路径
    QString ionpath = ui.ionpath->text(); // 获取电离层文件路径

    // 卫星天线文件路径
    std::string sat_str = satpcvpath.toStdString(); // 先保存到 std::string
    snprintf(filopt1.satantp, MAXSTRPATH, "%s", sat_str.c_str());
    // snprintf 会确保 filopt1.satantp 以 '\0' 结尾，并且不会写入超过 MAXSTRPATH 个字符（包括 '\0'）

    // 接收机天线文件路径
    std::string rec_str = recpcvpath.toStdString();
    snprintf(filopt1.rcvantp, MAXSTRPATH, "%s", rec_str.c_str());

    // DCB码偏差文件路径
    std::string dcb_str = dcbpath.toStdString();
    snprintf(filopt1.dcb, MAXSTRPATH, "%s", dcb_str.c_str());

    // 电离层文件路径
    std::string iono_str = ionpath.toStdString();
    snprintf(filopt1.iono, MAXSTRPATH, "%s", iono_str.c_str());
    emit configchanged(solopt1, prcopt1,filopt1); // 发送配置改变信号
    return;
}

Config::~Config() {}