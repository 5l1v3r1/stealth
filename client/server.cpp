#include "server.h"

Server::Server()
{
    //this->listen(QHostAddresss::Any,Port);
}

//Si hay una conexi�n prendiente, creamos un nuevo socket
//y decimos que la conexi�n est� lista para inicializar
void Server::incomingConnection(int socketDescriptor)
{
    Connection* connection=new Connection();
    connection->setState(Connection::Connecting);
    connection->setSetupState(Connection::JustConnected);
    connection->setSocketDescriptor(socketDescriptor);

    emit newConnection(connection);
}
