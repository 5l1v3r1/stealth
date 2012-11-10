#include "server.h"

Server::Server()
{
    this->listen(QHostAddress::Any,2000);
}

//Si hay una conexi�n prendiente, creamos un nuevo socket
//y decimos que la conexi�n est� lista para inicializar
void Server::incomingConnection(int socketDescriptor)
{
    Connection* connection=new Connection();
    connection->setState(Connection::JustConnected);
    connection->setSocketDescriptor(socketDescriptor);

    emit newConnection(connection);
}
