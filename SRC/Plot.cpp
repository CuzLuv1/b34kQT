#include<pybind11/pybind11.h>
#include<pybind11/embed.h>
#include<pybind11/stl.h>

// Plot.cpp
#include "include/Plot.h"
#include <QProcess>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QPainter>     // 如果需要将SVG渲染到QPixmap
#include <QSvgRenderer> 
#include <QDateTime>        // 用于生成唯一文件名
#include <QWheelEvent>      // 用于鼠标滚轮缩放

#include "readfile.h" // 包含读取文件的头文件

// 创建 pybind11 命名空间的别名，方便使用
namespace py = pybind11;

// 初始化静态成员变量
bool Plot::m_pythonInitialized = false;

Plot::Plot(QWidget *parent) : QWidget(parent),m_currentScale(1.0)  {
    ui.setupUi(this);

     ui.plotlabel->setText("输入绘图数据...");

    initializePython(); // 初始化 Python 环境

     // 连接信号和槽，确保在 Python 环境初始化后调用
    connect(ui.pos1, &QPushButton::clicked, this, &Plot::posPlot);
    connect(ui.pos2, &QPushButton::clicked, this, &Plot::posPlot);
}

Plot::~Plot() {}

// --- 初始化 Python 环境 ---
// 这个函数尝试只执行一次 Python 的初始化设置
void Plot::initializePython() {
    //python初始化
    #if defined(_WIN32) || defined(_WIN64)
    _putenv_s("PYTHONUTF8", "1");
    #else
    setenv("PYTHONUTF8", "1", 1); // 第三个参数 1 表示如果已存在则覆盖
    #endif
    const char* pythonHome = "D:\\Code\\anaconda\\envs\\vir"; // 您的 Anaconda 虚拟环境路径
    _putenv_s("PYTHONHOME", pythonHome);

    if (!m_pythonInitialized) {
        try {
            qInfo() << "[Plot] Attempting to initialize Python interpreter...";
            pybind11::initialize_interpreter(); // 这是现在唯一的核心调用
            m_pythonInitialized = true;
            qInfo() << "[Plot] Python interpreter initialized successfully.";

            // 如果初始化成功，可以尝试恢复 sys.path 的设置
            // py::module_ sys = py::module_::import("sys");
            // QString scriptDir = QCoreApplication::applicationDirPath();
            // qInfo() << "[Plot] Adding Python script search path:" << scriptDir;
            // sys.attr("path").attr("append")(scriptDir.toStdString());

        } catch (py::error_already_set &e) {
            // 这个块处理 Python 脚本执行中或 Python API 调用中发生的 Python 错误
            qCritical() << "[Plot] Python error occurred (py::error_already_set):" << e.what();
            // e.what() 应该会给出 Python 异常类型和消息
            if(ui.plotlabel) ui.plotlabel->setText("错误: Python 内部错误!");
        } catch (const std::exception &e) {
            // 这个块处理更底层的 C++ 异常，很可能发生在 Python 初始化本身
            qCritical() << "[Plot] C++ exception during Python initialization (std::exception):" << e.what(); // <--- 请务必查看这条日志的输出！
            if(ui.plotlabel) ui.plotlabel->setText("错误: Python 初始化异常!");
        } catch (...) {
            // 捕获所有其他类型的未知异常
            qCritical() << "[Plot] Unknown exception during Python initialization.";
            if(ui.plotlabel) ui.plotlabel->setText("错误: 未知初始化异常!");
        }
    } else {
        qInfo() << "[Plot] Python interpreter already initialized.";
    }
}

void Plot::openFile(QString& filePath) {
    filePath = ""; // 清空文件路径
    // 这里可以添加打开文件的逻辑
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString filter;
    QString caption;
    // 读取文件
    if(button==ui.pos1 || button==ui.pos2){
        filter = tr("POS文件(*.pos *.POS)");
        caption = tr("读取POS文件");
        filePath = QFileDialog::getOpenFileName(this, caption, "/", filter);
    }
    else if(button==ui.obs){
        filter = tr("OBS文件(*.obs *.OBS *.rnx *.RNX *.??O *.??o)");
        caption = tr("读取OBS文件");
        filePath = QFileDialog::getOpenFileName(this, caption, "/", filter);
    }
    else if(button==ui.stat){
        filter = tr("STAT文件(*.stat *.STAT)");
        caption = tr("读取STAT文件");
        filePath = QFileDialog::getOpenFileName(this, caption, "/", filter);
    }
}

void Plot::posPlot() {
    QString filePath;QVector<posinfo> posinfos;
    openFile(filePath); // 打开文件并获取路径
    if (filePath.isEmpty()) { // 如果路径为空，则不执行绘图
        qDebug() << "[Plot] No file selected. Skipping plot.";
        return; // 直接返回，不执行绘图
    }
    //posinfos填充
    else{
        QVector<posinfo> infos;
        dataread data;
        if(data.read_posinfo(filePath,infos)){ //读取pos文件信息
            qDebug() << "文件读取成功";           
        }
        if(data.addposinfo(infos)){ //添加pos文件信息
            qDebug() << "文件添加成功";
        }
        else{
            qDebug() << "文件添加失败";
        }
        posinfos = data.getposinfo();
    }

    if(!posinfos.isEmpty())
    {
        //Python脚本绘制图片
        try {
        py::gil_scoped_acquire acquire; // 在与Python交互的整个过程中持有GIL

        // 1. 将 QVector<posinfo> 转换为 Python 的 list of dictionaries
        py::list pyDataList;
        for (const posinfo& pInfo : posinfos) {
            py::dict pyItem;
            // 确保键名与 Python 脚本中期望的一致
            pyItem["posX"] = pInfo.pos[0]; // X 坐标
            pyItem["posY"] = pInfo.pos[1]; // Y 坐标
            pyItem["posZ"] = pInfo.pos[2]; // Z 坐标
            pyDataList.append(pyItem);
        }

        // 2. 导入 Python 绘图模块 (plotter.py)
        py::module_ plotterModule;
        try {
            plotterModule = py::module_::import("plotter");
        } catch (const py::error_already_set& e) {
             if (e.matches(PyExc_ModuleNotFoundError)) {
                qCritical() << "[Plot] Python 错误: 模块 'plotter.py' 未找到!";
                qCritical() << "[Plot] 请确保 'plotter.py' 与可执行文件在同一目录 (通过CMake的add_custom_command复制)，或者在 PYTHONPATH 中。";
                if (py::hasattr(py::module_::import("sys"), "path")) { // 检查 sys 模块是否有 path 属性
                     qCritical() << "[Plot] 当前 sys.path:" << py::repr(py::module_::import("sys").attr("path")).cast<std::string>();
                }
                if (ui.plotlabel) {
                    ui.plotlabel->setText("错误: 绘图脚本 'plotter.py' 未找到。");
                }
             } else {
                qCritical() << "[Plot] Python 错误 (导入 plotter 模块时):" << QString::fromUtf8(e.what());
                QString errorMsg = QString::fromUtf8(e.what());
                if(e.trace() && e.trace().ptr() != Py_None) {
                     errorMsg += QString::fromUtf8("\nPython Traceback:\n") + QString::fromStdString(e.trace().str().cast<std::string>());
                }
                if (ui.plotlabel) {
                    ui.plotlabel->setText(QString("Python 错误 (导入时):\n%1").arg(errorMsg));
                }
             }
             return; // 导入失败，无法继续
        }

        // 3. 定义绘图图片的输出基础路径 (不含扩展名)
        QString appPath = QCoreApplication::applicationDirPath(); // 可执行文件所在目录
        QDir projectDir(appPath);
        projectDir.cdUp(); // 移动到可执行文件目录的上一级 (通常是构建目录)

        QString resultDirName = "result_plots_scientific"; // 为这些增强图表创建新文件夹名
        QString plotImageBaseDir = projectDir.filePath(resultDirName); // 组合路径

        QDir dir(plotImageBaseDir); // QDir 对象用于操作目录
        if (!dir.exists()) { // 如果目录不存在
            if (!dir.mkpath(".")) { // 则尝试创建它 (mkpath 会创建所有必需的父目录)
                 qWarning() << "[Plot] 创建绘图目录失败:" << plotImageBaseDir;
                 // 如果创建失败，使用一个位于可执行文件同级的备用目录
                 plotImageBaseDir = QCoreApplication::applicationDirPath() + QDir::separator() + "result_fallback_scientific";
                 qWarning() << "[Plot] 使用备用目录保存绘图图片:" << plotImageBaseDir;
                 QDir fallbackDir(plotImageBaseDir);
                 if(!fallbackDir.exists()) fallbackDir.mkpath("."); // 尝试创建备用目录
            }
        }
        
        QString inputFileNameBase = QFileInfo(filePath).baseName(); // 从输入文件名获取基础名 (不含路径和扩展名)
        // Python 脚本会自动添加 .png 或 .svg 扩展名
        QString plotBaseName = inputFileNameBase + "_pos_diff_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QString plotBasePathForPython = dir.filePath(plotBaseName); // 传递给Python的基础路径 (不含扩展名)

        qDebug() << "[Plot] 传递给 Python 脚本的绘图基础路径:" << plotBasePathForPython;

        // 4. 决定使用 SVG 还是 PNG 格式
        bool useSvg = true; // 设置为 true 优先使用 SVG，false 使用高 DPI PNG
        QString finalPlotImagePath; // 存储带扩展名的最终图片路径

        // 5. 调用 Python 脚本中的绘图函数
        bool plotSuccess = false;
        try {
            // 调用Python函数，传递数据、基础路径和SVG偏好设置
            // 确保Python函数 `plot_position_differences` 接受这三个参数
            plotSuccess = plotterModule.attr("plot_position_differences")(pyDataList, plotBasePathForPython.toStdString(), useSvg).cast<bool>();
        } catch (const py::error_already_set& e) {
            qCritical() << "[Plot] Python 错误 (调用绘图函数 plot_position_differences 时):" << QString::fromUtf8(e.what());
            QString pyErrorDetails = QString::fromUtf8(e.what()); // 重命名以避免与外部作用域的 pyError 混淆
            if(e.trace() && e.trace().ptr() != Py_None) {
                 pyErrorDetails += QString::fromUtf8("\nPython Traceback:\n") + QString::fromStdString(e.trace().str().cast<std::string>());
            }
            if (ui.plotlabel) {
                 ui.plotlabel->setText(QString("Python 绘图错误:\n%1").arg(pyErrorDetails));
            }
            return; // 调用失败
        }

        // 6. 如果 Python 脚本成功执行，则加载并显示图片
        if (plotSuccess) {
            finalPlotImagePath = plotBasePathForPython + (useSvg ? ".svg" : ".png"); // 构建完整图片路径
            qDebug() << "[Plot] Python 脚本执行成功。正在加载图片:" << finalPlotImagePath;
            
            QPixmap tempPixmap; // 用于临时加载图像
            if (useSvg) { // 如果选择使用 SVG
                QSvgRenderer renderer(finalPlotImagePath); // 创建 SVG 渲染器
                if (renderer.isValid()) { // 检查 SVG 文件是否有效且可被渲染
                    QSize defaultSize = renderer.defaultSize(); // 获取 SVG 的固有 (默认) 尺寸
                    // 限制初始渲染尺寸，避免 SVG 固有尺寸过大导致 QPixmap 消耗过多内存
                    if (defaultSize.width() > 2000 || defaultSize.height() > 1500) { 
                        defaultSize.scale(2000, 1500, Qt::KeepAspectRatio); // 按比例缩放到上限内
                    }
                    // 如果 SVG 没有有效的固有尺寸或尺寸过小，则使用一个合理的备用尺寸
                    if (!defaultSize.isValid() || defaultSize.width() < 100 || defaultSize.height() < 100) { 
                        defaultSize = QSize(800, 600); // 例如 800x600
                    }

                    tempPixmap = QPixmap(defaultSize); // 创建一个指定尺寸的 QPixmap
                    tempPixmap.fill(Qt::transparent);  // 用透明背景填充 (或 Qt::white)
                    QPainter painter(&tempPixmap);     // 在此 QPixmap 上创建一个 QPainter
                    renderer.render(&painter);         // 将 SVG 渲染到 QPixmap 上
                    qDebug() << "[Plot] SVG 成功渲染到 QPixmap，尺寸:" << tempPixmap.size();
                } else {
                    qWarning() << "[Plot] 渲染 SVG 文件失败:" << finalPlotImagePath;
                }
            } else { // 如果选择使用 PNG 格式
                if (!tempPixmap.load(finalPlotImagePath)) { // 加载高 DPI 的 PNG
                     qWarning() << "[Plot] 加载 PNG 图片失败:" << finalPlotImagePath;
                } else {
                    qDebug() << "[Plot] PNG 图片加载成功，尺寸:" << tempPixmap.size();
                }
            }

            if (!tempPixmap.isNull()) { // 如果图片成功加载或渲染到 tempPixmap
                m_originalPixmap = tempPixmap; // 将其存储为原始图像
                m_currentScale = 1.0;          // 重置缩放比例为 100%
                
                // 关键步骤：在设置图像之前，调整 QLabel 的大小以适应原始图像的完整尺寸。
                // 这对于 QLabel 放置在 QScrollArea 中并希望正确显示滚动条非常重要。
                ui.plotlabel->setFixedSize(m_originalPixmap.size()); 
                
                updatePlotLabelPixmap();       // 调用辅助函数来应用初始缩放并更新 QLabel 的显示
            } else {
                qDebug() << "[Plot] 加载或渲染绘图图片失败:" << finalPlotImagePath;
                if (ui.plotlabel) {
                    ui.plotlabel->setText(QString("错误：无法加载或渲染图片。\n路径: %1").arg(finalPlotImagePath));
                }
            }
        } else { // 如果 Python 脚本报告 plotSuccess 为 False
            qDebug() << "[Plot] Python 脚本报告绘图过程中出错 (返回 False)。";
            if (ui.plotlabel) {
                ui.plotlabel->setText("错误：Python 脚本未能成功生成绘图。");
            }
        }
    } catch (const py::error_already_set& e) { // 捕获pybind11相关的Python异常 (这个 catch 块处理 posPlot 中其他未被内部 try-catch 捕获的 Python 异常)
        qCritical() << "[Plot] posPlot 函数中发生一般性 Python 错误:" << QString::fromUtf8(e.what());
        QString pyError = QString::fromUtf8(e.what()); // 初始化 pyError
        if (e.trace() && e.trace().ptr() != Py_None) {
            pyError += QString::fromUtf8("\nPython Traceback:\n") + QString::fromStdString(e.trace().str().cast<std::string>());
        }
        // 注意：这里不再有重复的 `QString pyError = e.what();`
        if (ui.plotlabel) {
            ui.plotlabel->setText(QString("Python 执行错误:\n%1").arg(pyError));
        }
    } catch (const std::exception& e) { // 捕获其他标准C++异常
        qCritical() << "[Plot] Python 交互期间 posPlot 函数中发生 C++ 错误:" << QString::fromUtf8(e.what());
        if (ui.plotlabel) {
            ui.plotlabel->setText(QString("C++ 错误 (Python交互期间):\n%1").arg(QString::fromUtf8(e.what())));
        }
    } catch (...) { // 捕获所有其他未知异常
        qCritical() << "[Plot] Python 交互期间 posPlot 函数中发生未知错误。";
        if (ui.plotlabel) {
            ui.plotlabel->setText("发生未知错误 (Python交互期间)。");
        }
    }
}
}

// --- 更新 QLabel 中的图像（应用缩放） ---
void Plot::updatePlotLabelPixmap() 
{
    if (m_originalPixmap.isNull() || !ui.plotlabel) {
        if (ui.plotlabel) ui.plotlabel->setText("无图像数据可显示");
        return;
    }

    // 计算缩放后 QLabel 应有的大小
    QSize newLabelSize = m_originalPixmap.size() * m_currentScale;
    
    // 确保 QLabel 的尺寸不小于一个像素，避免无效尺寸导致的潜在问题
    if (newLabelSize.width() < 1) newLabelSize.setWidth(1);
    if (newLabelSize.height() < 1) newLabelSize.setHeight(1);

    // 调整 QLabel 的固定大小以适应缩放后的图像尺寸。
    // 这是为了让 QScrollArea 能够根据 QLabel 的实际内容大小来决定是否显示滚动条。
    ui.plotlabel->setFixedSize(newLabelSize);

    // 将原始图像 (m_originalPixmap) 缩放到计算出的 newLabelSize，以供显示。
    // Qt::SmoothTransformation 有助于在缩放时获得更好的视觉效果，但可能比 Qt::FastTransformation 稍慢。
    QPixmap scaledPixmap = m_originalPixmap.scaled(
        newLabelSize,
        Qt::KeepAspectRatio,        // 在缩放时保持图像的原始宽高比
        Qt::SmoothTransformation    // 使用平滑变换以获得更好的缩放质量
    );
    
    ui.plotlabel->setPixmap(scaledPixmap); // 在 QLabel 上设置缩放后的图像
    ui.plotlabel->setText("");             // 清除 QLabel 上可能存在的任何文本 (例如 "无图像数据")
}

// --- 处理鼠标滚轮事件以实现缩放 ---
void Plot::wheelEvent(QWheelEvent *event) {
    // 检查原始图像是否存在，并且鼠标光标是否在 ui.plotlabel 之上
    if (!m_originalPixmap.isNull() && ui.plotlabel->rect().contains(ui.plotlabel->mapFromGlobal(QCursor::pos()))) {
        QPoint angleDelta = event->angleDelta(); // 获取滚轮滚动的角度增量
        double delta = 0;
        if (!angleDelta.isNull()) {
            // angleDelta().y() 通常是垂直滚轮的度数 (一度通常是120的倍数)
            delta = static_cast<double>(angleDelta.y()) / 120.0; // 标准化滚轮增量 (例如，一度对应1.0)
        }

        double scaleFactorChange = 0.15; // 定义每次滚动的缩放步长 (例如15%)

        if (delta > 0) { // 向上滚动 (通常是放大)
            m_currentScale += scaleFactorChange;
        } else if (delta < 0) { // 向下滚动 (通常是缩小)
            m_currentScale -= scaleFactorChange;
        }

        // 将缩放因子限制在一个合理的最小和最大值之间
        if (m_currentScale < 0.05) m_currentScale = 0.05;   // 例如，最小缩放为原始尺寸的5%
        if (m_currentScale > 20.0) m_currentScale = 20.0;  // 例如，最大缩放为原始尺寸的20倍 (可根据需要调整)

        qDebug() << "[Plot] 图像缩放中，新比例因子:" << m_currentScale;
        updatePlotLabelPixmap(); // 调用辅助函数以新的缩放比例更新图像显示
        event->accept();         // 表明此事件已被处理，不再向父控件传递
    } else {
        QWidget::wheelEvent(event); // 如果事件不适用于我们 (例如鼠标不在标签上)，则传递给基类处理
    }
}

// --- 处理窗口或控件大小调整事件 ---
void Plot::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); //首先调用基类的实现
    // 当包含此 Plot 部件的窗口或布局调整大小时，此事件会被触发。
    // 如果 m_originalPixmap 不为空 (即已加载图像)，则更新图像显示。
    // 当前实现会根据 m_currentScale 重新缩放图像以适应 QLabel 的新尺寸 (如果 QLabel 尺寸受布局影响)。
    // 如果 QLabel 在 QScrollArea 中，并且 QScrollArea 的 widgetResizable 为 true，
    // QLabel 可能会尝试适应 QScrollArea 的视口大小，这时重新应用缩放是合适的。
    if (!m_originalPixmap.isNull()) {
        // 可选：如果希望在窗口调整大小时图像总是“适应”视口，可以在这里重新计算 m_currentScale。
        // 例如，获取 QScrollArea 视口的大小，然后计算合适的 m_currentScale 使图像完整显示。
        // QWidget* scrollAreaViewport = ui.plotlabel->parentWidget(); // 假设父控件是 QScrollArea 的视口
        // if (scrollAreaViewport) {
        //     QSize viewportSize = scrollAreaViewport->size();
        //     if (viewportSize.isValid() && m_originalPixmap.width() > 0 && m_originalPixmap.height() > 0) {
        //         double wScale = static_cast<double>(viewportSize.width()) / m_originalPixmap.width();
        //         double hScale = static_cast<double>(viewportSize.height()) / m_originalPixmap.height();
        //         double fitScale = qMin(wScale, hScale); // 取较小的缩放比例以确保完整适应
        //         // 可以选择总是适应 (m_currentScale = fitScale) 或者只在当前缩放过大时适应
        //         // m_currentScale = fitScale; // 总是适应
        //         // if (m_currentScale > fitScale) m_currentScale = fitScale; // 仅当当前缩放导致图像超出时才适应
        //         if (m_currentScale < 0.01) m_currentScale = 0.01;
        //     }
        // }
        updatePlotLabelPixmap();
    }
}