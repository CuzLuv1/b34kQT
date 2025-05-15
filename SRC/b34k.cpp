#include "include/b34k.h"
#include <QHBoxLayout>

b34k::b34k(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_b34k)
{
    ui->setupUi(this);
    setObjectName("b34k");

    // 主布局
    QHBoxLayout* mainLayout = new QHBoxLayout();
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 初始化导航树
    navTree = new QTreeWidget();
    navTree->setHeaderHidden(true);
    navTree->setFixedWidth(200);
    navTree->setFixedHeight(600);
    navTree->setObjectName("navTree");
    
    // 添加导航项
    positioningParent = new QTreeWidgetItem(navTree, QStringList("定位解算"));
    pointPosItem = new QTreeWidgetItem(positioningParent, QStringList("单点定位"));
    relPosItem = new QTreeWidgetItem(positioningParent, QStringList("相对定位"));
    plotItem = new QTreeWidgetItem(navTree, QStringList("结果绘制"));
    configItem = new QTreeWidgetItem(navTree, QStringList("配置选项"));

    // 初始化页面
    contentStack = new QStackedWidget(this);
    contentStack->setFixedWidth(820);
    contentStack->setFixedHeight(600);
    contentStack->setObjectName("contentStack");
    configpage = new Config(this);
    pointpospage = new PointPOS(configpage,this);
    relpospage = new RelPOS(configpage,this);
    plotpage = new Plot(this);
    

    // 添加页面到堆栈
    contentStack->addWidget(pointpospage); // 索引0
    contentStack->addWidget(relpospage); // 索引1
    contentStack->addWidget(plotpage);     // 索引2
    contentStack->addWidget(configpage);     // 索引3

    // 布局添加控件
    mainLayout->addWidget(navTree);
    mainLayout->addWidget(contentStack);

    // 连接信号
    connect(navTree, &QTreeWidget::itemClicked, 
            this, &b34k::onNavItemClicked);
}

b34k::~b34k()
{
    delete ui; 
}

//导航栏点击事件
void b34k::onNavItemClicked(QTreeWidgetItem* item, int column)
{
    if (item == pointPosItem) {
        contentStack->setCurrentWidget(pointpospage);
    }
    else if (item == relPosItem) {
        contentStack->setCurrentWidget(relpospage);
    }
    else if (item == plotItem) {
        contentStack->setCurrentWidget(plotpage);
    }
    else if (item == configItem) {
        contentStack->setCurrentWidget(configpage);
    }

    // 父项点击展开/折叠
    if (item->childCount() > 0 && !item->isSelected()) {
        item->setExpanded(!item->isExpanded());
    }
}