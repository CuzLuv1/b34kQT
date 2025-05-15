// Plot.h
#ifndef PLOT_H
#define PLOT_H

#include "ui_Plot.h"  // 由 CMake 生成的 UI 头文件
#include <QWidget>

class Plot : public QWidget {
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();

private:
    Ui::Plot ui;

    void initializePython(); // 只初始化一次 Python 环境
    static bool m_pythonInitialized; // 静态标志，跟踪初始化状态
    void openFile(QString &filepath);
    void updatePlotLabelPixmap();    // 更新 QLabel 中图像显示的辅助函数

    // --- 图像显示和交互 ---
    QPixmap m_originalPixmap;           // 存储从 Python 获取的原始高分辨率图像
    double  m_currentScale = 1.0;       // 当前图像的缩放因子
       

private slots:
    void posPlot();
    //void statPlot();//stat文件绘制，主要为残差、高度角、信噪比信息
    //void obsPlot();

protected:
    // --- 事件处理器 ---
    void wheelEvent(QWheelEvent *event) override;   // 处理鼠标滚轮事件以实现缩放
    void resizeEvent(QResizeEvent *event) override; // 处理窗口或控件大小调整事件
};
#endif // Plot_H