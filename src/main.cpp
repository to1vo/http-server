#include <iostream>
#include <string>
#include <winsock2.h>

#define PORT_NUMBER 8080

int main(){
    WSADATA wsa_data;
    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }

    //create the socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    //define the server address
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUMBER);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to the address
    bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)); 

    SOCKET client_socket;

    //listen to incoming connections
    listen(server_socket, 5);
    std::cout << "Server listening to port " << PORT_NUMBER << std::endl; 

    std::string content = "<!DOCTYPE html><html><body><h1>Hello world!</h1></body></html>";
    std::string response = "HTTP/1.1 200 OK\r\nServer: Test\r\nDate: Sun, 14 Sep 16:48:40 GMT\r\nContent-Length: "+ std::to_string(content.size()) +"\r\nContent-Type: text/html\r\n\r\n" + content;
    std::cout << response << std::endl;
    
    while(true){    
        //the socket that accepts requests
        client_socket = accept(server_socket, nullptr, nullptr);
        
        char buffer[1024] = {0};

        //receive data
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::cout << "Message from client: " << buffer << std::endl;
    
        send(client_socket, response.c_str(), response.size(), 0);
        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}