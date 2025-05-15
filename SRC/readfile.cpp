#include "readfile.h"
#include "rtklib/rtklib.h"
#include <QFileDialog>
#include <QDebug>
#include<QVector>
#include<QString>
#include<QFile>
#include<QTextStream>
#include<QRegularExpression>
#include<QIODevice>

//获取文件信息
const QVector<satinfo>& dataread::getsatinfo() const { return m_satinfo; }

const QVector<posinfo>& dataread::getposinfo() const { return m_posinfo; }

const QVector<statinfo>& dataread::getstatinfo() const { return m_statinfo; }

//添加信息
bool dataread::addsatinfo(QVector<satinfo>& infos) {
    if (!m_satinfo.isEmpty()) {
        clear_satinfo(); // 清空现有卫星信息
        // 此时，旧的卫星信息已被清除。
        // 如果存在位置信息，则总是先清空卫星信息，然后再尝试添加新的。
    }

    if (infos.isEmpty()) {
        qDebug() << "Input satellite information is empty. Nothing to add.";
        // 如果在 `!m_satinfo.isEmpty()` 为真时清空了 `m_satinfo`，
        // 并且现在 `infos` 也是空的，那么 `m_satinfo` 最终是空的。
        // 这种情况下返回 false 表示“没有添加任何有效数据”是合理的。
        return false;
    }

    // 到这里，infos 是非空的
    for (const satinfo& info : infos) {
        m_satinfo.append(info); // 将 infos 中的所有元素添加到 m_satinfo
    }

    return true; // 表示成功添加了 infos 中的数据
}

bool dataread::addposinfo(QVector<posinfo>& infos) { 
    if (!m_posinfo.isEmpty()) {
        clear_posinfo(); // 清空现有卫星信息
        // 此时，旧的卫星信息已被清除。
        // 如果存在位置信息，则总是先清空卫星信息，然后再尝试添加新的。
    }

    if (infos.isEmpty()) {
        qDebug() << "Input satellite information is empty. Nothing to add.";
        return false;
    }

    // 到这里，infos 是非空的
    for (const posinfo& info : infos) {
        m_posinfo.append(info); // 将 infos 中的所有元素添加到 m_satinfo
    }

    return true; // 表示成功添加了 infos 中的数据
}

bool dataread::addstatinfo(QVector<statinfo>& infos) { 
    if (!m_statinfo.isEmpty()) {
        clear_posinfo(); // 清空现有卫星信息
        // 此时，旧的卫星信息已被清除。
        // 如果存在位置信息，则总是先清空卫星信息，然后再尝试添加新的。
    }

    if (infos.isEmpty()) {
        qDebug() << "Input satellite information is empty. Nothing to add.";
        return false;
    }

    // 到这里，infos 是非空的
    for (const statinfo& info : infos) {
        m_statinfo.append(info); // 将 infos 中的所有元素添加到 m_satinfo
    }

    return true; // 表示成功添加了 infos 中的数据
}

//读取信息
bool dataread::read_posinfo(const QString& filepath,QVector<posinfo>&infos) {
    if (filepath.isEmpty()) {
        qDebug() << "File path is empty. Cannot read position information.";
        return false;
    }
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filepath;
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();//逐行读取
        posinfo info;
        // 解析每一行数据并填充 info 对象
        //跳过头文件  %开头
        if (line.startsWith("%")) {
            continue;
        }
        else{
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            info.time= parts[0]+" "+parts[1];//时间
            info.pos[0] = parts[2].toDouble();//x
            info.pos[1] = parts[3].toDouble();//y
            info.pos[2] = parts[4].toDouble();//z
            info.Q= parts[5].toInt();//解算状态
            info.satnum= parts[6].toInt();//卫星数目
            infos.append(info);//添加到列表中
        }
    }

    file.close();
    return true;
}

bool dataread::read_satinfo(const QString& filepath,QVector<satinfo>&infos) {
    return true;
}

bool dataread::read_statinfo(const QString& filepath,QVector<statinfo>&infos) {
    if (filepath.isEmpty()) {
        qDebug() << "File path is empty. Cannot read position information.";
        return false;
    }
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filepath;
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();//逐行读取
        statinfo info;
        QStringList parts = line.split(",", Qt::SkipEmptyParts);
        // 解析每一行数据并填充 info 对象
        if (parts[0]!="$SAT"){
            continue;
        }
        else{
            info.PRN= parts[3];//卫星
        }
    }

    file.close();
    return true;
}

void dataread::clear_satinfo() { m_satinfo.clear(); }

void dataread::clear_posinfo() { m_posinfo.clear(); }

void dataread::clear_statinfo() { m_statinfo.clear(); }