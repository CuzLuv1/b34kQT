#include "timeconv.h"
#include <QDebug>
#include <QDate>
#include <QString>

QString time2julian(QDate date)
{
    qint64 julianDay = date.toJulianDay();
    return QString::number(julianDay);
}

QString time2GPSday(QDate date)
{
    // GPS 时间的起始点 (epoch)
    QDate gpsEpoch(1980, 1, 6); // 1980年1月6日

    // 获取输入日期和 GPS epoch 的儒略日
    qint64 currentJulianDay = date.toJulianDay();
    qint64 gpsEpochJulianDay = gpsEpoch.toJulianDay();

    // 检查日期是否在 GPS epoch 之前
    if (currentJulianDay < gpsEpochJulianDay) {
        return QString("日期在 GPS 时间起点之前");
    }

    // 计算从 GPS epoch 开始的总天数
    qint64 daysSinceGpsEpoch = currentJulianDay - gpsEpochJulianDay;

    // 计算 GPS 周数
    int gpsWeek = static_cast<int>(daysSinceGpsEpoch / 7);

    // 计算 GPS 周内天 (0 表示周日, 1 表示周一, ..., 6 表示周六)
    // 1980年1月6日是星期日。
    // (儒略日 + 1) % 7 中，0是周一 ... 6是周日 (这是 QDate::dayOfWeek() 的逻辑，但不完全是)
    // 我们需要以周日为0开始。
    // (currentJulianDay - gpsEpochJulianDay) % 7 已经是以 GPS epoch (周日) 为基准的偏移天数
    int gpsDayOfWeek = static_cast<int>(daysSinceGpsEpoch % 7);

    return QString::number(gpsWeek) + " " + QString::number(gpsDayOfWeek);
}

QString time2day(QDate date)
{
     // QDate::dayOfYear() 返回的是 int 类型的年积日 (1 到 365 或 366)
    int dayOfYear = date.dayOfYear();
    return QString::number(dayOfYear);
}

