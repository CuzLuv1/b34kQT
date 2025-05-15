#ifndef READFILE_H
#define READFILE_H
#include<iostream>
#include<fstream>
#include<QVector>

//QT
#include<QString>
#endif

struct satinfo{
    QString PRN;
    double azimuth=0.0;
    double elevation=0.0;
    double snr=0.0; 
    double prange=0.0;
    double doppler=0.0;
    double x[3]={0.0,0.0,0.0};//坐标
};

struct posinfo{
    int Q=0;//解算状态
    int satnum=0;//卫星数目
    double pos[3]={0.0,0.0,0.0};//坐标
    QString time="";//时间
    double dop[6]={0.0,0.0,0.0,0.0,0.0,0.0};//DOP值 
};

struct statinfo{
    QString PRN="";
    double snr=0.0;
    double elevation=0.0;
    int Q=0;
    double single_residual=0.0;//单差残差
    double double_residual=0.0;//双差残差
    double p_residual=0.0;//伪距残差
};

class dataread
{

public:
    const QVector<satinfo>& getsatinfo()const;//卫星信息
    const QVector<posinfo>& getposinfo()const;//定位信息
    const QVector<statinfo>& getstatinfo()const;//状态文件信息

    //添加信息
    bool addsatinfo(QVector<satinfo>& infos);//添加卫星信息
    bool addposinfo(QVector<posinfo>& infos);//添加定位信息
    bool addstatinfo(QVector<statinfo>& infos);//添加状态信息

    //读取信息
    bool read_satinfo(const QString& filepath,QVector<satinfo>&infos);//读取卫星信息
    bool read_posinfo(const QString& filepath,QVector<posinfo>&infos);//读取定位信息
    bool read_statinfo(const QString& filepath,QVector<statinfo>&infos);//读取状态信息

    //清空信息
    void clear_satinfo();//清空卫星信息
    void clear_posinfo();//清空定位信息
    void clear_statinfo();//清空状态信息

private:
    QVector<satinfo> m_satinfo;
    QVector<posinfo> m_posinfo;
    QVector<statinfo> m_statinfo;

};