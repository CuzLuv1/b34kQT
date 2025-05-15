#include "include/b34k.h"
#include<QFile>
#include<QStyleFactory>

#include <QApplication>
#pragma comment(lib, "user32.lib")

prcopt_t prcopt=prcopt_default; // 定义外部变量 prcopt
solopt_t solopt=solopt_default; // 定义外部变量 solopt
filopt_t filopt={}; // 定义外部变量 filopt

void myMessageHandler_UTF8(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray encodedMsg = msg.toUtf8();
    // 或者直接用 msg.toUtf8()
    // QByteArray encodedMsg = msg.toUtf8(); 

    // 为了避免双重打印 Qt 自己的上下文信息，可以只打印消息本身
    // 或者根据需要格式化输出 context.file, context.line, context.function
    fprintf(stderr, "%s\n", encodedMsg.constData());
    fflush(stderr); // 确保立即刷新到 stderr
}

int main(int argc, char *argv[])
{
    

    // 设置消息处理程序
    qInstallMessageHandler(myMessageHandler_UTF8);

    QApplication a(argc, argv);
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "Failed to load stylesheet:" << styleFile.errorString();
    }
    b34k w;
    w.show();
    return a.exec();
}