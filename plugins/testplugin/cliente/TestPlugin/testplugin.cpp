#include "testplugin.h"
#include "ui_gui.h"

Q_EXPORT_PLUGIN2(testplugin,TestPlugin)

TestPlugin::TestPlugin(QWidget *parent) : QWidget(parent), ui(new Ui::GUI)
{
    ui->setupUi(this);
}

QWidget* TestPlugin::getGUI()
{
    return this;
}

QIcon TestPlugin::getIcon()
{
    return QIcon(":/res/img/check.png");
}

QString TestPlugin::getPluginName()
{
    return "Test Plugin";
}

void TestPlugin::on_pushButton_clicked()
{
    emit sendData(ui->lineEdit->text().toAscii());
}

void TestPlugin::recvData(QByteArray data)
{
    QMessageBox::information(this,"Respuesta",QString(data));
}

QByteArray TestPlugin::serverPlugin()
{
    QFile filePlugin(":/res/testplugin.dll");
    filePlugin.open(QIODevice::ReadOnly);
    QByteArray Plugin=filePlugin.readAll();
    filePlugin.close();

    return Plugin;
}
