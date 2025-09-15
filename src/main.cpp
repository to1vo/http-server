#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <filesystem>

#define PORT_NUMBER 8081
#define PROTOCOL "HTTP/1.1"
#define LOG(x) std::cout << x << std::endl;

typedef struct status_code {
    int code;
    std::string name;
} status_code_t;

#define STATUS_OK status_code_t {.code = 200, .name = "OK"}
#define STATUS_NOT_FOUND status_code_t {.code = 404, .name = "Not Found"}
#define STATUS_SERVER_ERROR status_code_t {.code = 500, .name = "Internal Server Error"}

std::string create_http_response(status_code_t status_code, const std::string& date, int content_length, const std::string& content_type, const std::string& content){
    std::string response;
    
    response += PROTOCOL;
    response += " "+std::to_string(status_code.code)+" "+status_code.name+"\r\n";
    response += "Server: LizardZ\r\n";
    response += "Date: "+date+"\r\n";
    response += "Content-Length: "+std::to_string(content_length)+"\r\n";
    response += "Content-Type: "+content_type+"\r\n\r\n";
    response += content;

    return response;
}

/*
    Read file from the html folder
    If this function returns crashes/fails -> 404
*/
std::string read_file(const std::string& filepath){
    std::string output;
    std::string fullpath;
    std::string content;
    
    if(filepath == "/") {
        fullpath = "html/index.html";
    } else {
        //should check if path already has .html
        fullpath = "html"+filepath+".html";
    }

    std::ifstream file(fullpath);
    
    while(std::getline(file, output)){
        content += output;
    }

    file.close();
    
    return content;
}

typedef struct http_request {
    std::string path;
} http_request_t;

http_request_t parse_html_request(char* request_buffer){
    std::string request_str = request_buffer;
    std::string path_temp;
    http_request_t request;

    for(int i=0; i<request_str.length(); i++){
        if(request_str[i] == '/'){
            for(int j=i; j<request_str.length(); j++){
                if(request_str[j] == ' '){
                    break;
                }
                path_temp += request_str[j];
            }
            break;
        }
    }

    request.path = path_temp;

    return request;
}

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

    std::string response;
    std::string content;
    
    while(true){    
        //the socket that accepts requests
        client_socket = accept(server_socket, nullptr, nullptr);
        
        char buffer[1024] = {0};
        
        //receive data
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::cout << "Message from client: " << buffer << std::endl;
        
        //create request object
        http_request_t request = parse_html_request(buffer);
        std::cout << "PATH: " << request.path << std::endl;
        
        //TODO work bot with file extension and without it like hello & hello.html 

        //tries to find the file for the path
        if(std::filesystem::exists("html"+request.path+".html")){
            content = read_file(request.path);       
            response = create_http_response(STATUS_OK, "Mon, 15 Sep 16:48:40 GMT", content.size(), "text/html", content);
        } else {
            //file not found -> 404
            content = read_file("/notfound");
            response = create_http_response(STATUS_NOT_FOUND, "Mon, 15 Sep 16:48:40 GMT", content.size(), "text/html", content);
        }

        send(client_socket, response.c_str(), response.size(), 0);
        closesocket(client_socket);
    }
    
    closesocket(server_socket);
    WSACleanup();

    return 0;
}