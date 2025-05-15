#pragma once
#include "ui_b34k.h"
#include "PointPOS.h"
#include "RelPOS.h"
#include "Plot.h"
#include "Config.h"
#include <QMainWindow>
#include <QTreeWidget>
#include <QStackedWidget>


class b34k : public QMainWindow {
    Q_OBJECT
    
public:
    b34k(QWidget* parent = nullptr);
    ~b34k();

private slots:
    void onNavItemClicked(QTreeWidgetItem* item, int column);//导航栏点击事件


private:
    Ui_b34k* ui;

    QTreeWidget* navTree;
    QStackedWidget* contentStack;

    // 页面成员变量
    Config* configpage;
    PointPOS* pointpospage;
    RelPOS* relpospage;
    Plot* plotpage;

    // 导航项指针
    QTreeWidgetItem* positioningParent;
    QTreeWidgetItem* pointPosItem;
    QTreeWidgetItem* relPosItem;
    QTreeWidgetItem* plotItem;
    QTreeWidgetItem* configItem;

};