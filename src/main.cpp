#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <filesystem>
#include <ctime>

#define PORT_NUMBER 8081
#define PROTOCOL "HTTP/1.1"
#define DOCUMENT_ROOT "www"
#define LOG(x) std::cout << x << std::endl;

typedef struct status_code {
    int code;
    std::string name;
} status_code_t;

typedef struct http_request {
    std::string path;
} http_request_t;


#define STATUS_OK status_code_t {.code = 200, .name = "OK"}
#define STATUS_NOT_FOUND status_code_t {.code = 404, .name = "Not Found"}
#define STATUS_INTERNAL_SERVER_ERROR status_code_t {.code = 500, .name = "Internal Server Error"}

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
    Read file from the document root folder
    If this function would crash/fail -> server error 500
*/
std::string read_file(const std::string& filename){
    std::string output;
    std::string content;

    std::ifstream file(filename);
    
    while(std::getline(file, output)){
        content += output;
    }

    file.close();
    
    return content;
}

/*
    Parses the filename from the request path string
    With this system ../ or C:/ -> 404 not found 
    / -> index.html
    /index -> index.html
    /page.html -> page.html
    /main.js -> main.js etc
*/
std::string get_filename_from_path(std::string path){
    std::string filename = DOCUMENT_ROOT;

    for(int i=0; i<path.length(); i++){
        if(i+1 > path.length()-1){
            //last char
            if(path[i] == '/'){
                filename += path+"index.html";
                break;
            }

            //look for file extension
            std::string file_extension;
            bool extension_found = false;
            for(int j=path.length()-1; j>-1; j--){
                file_extension += path[j];
                if(path[j] == '.'){
                    //file extension found
                    extension_found = true;
                    filename += path;
                    break;
                }
            }
            
            //no file extension
            //defaults to .html file
            if(!extension_found){
                filename += path+".html";
            }
        }
    }
    
    return filename;
}

std::string get_current_date(){
    std::string date;
    time_t timestamp;
    time(&timestamp);
    
    date = ctime(&timestamp);
    date.erase(date.length()-1, 2);
    return date;
}

//Checks if the document root filepath exists
bool file_exists(const std::string& filepath){
    std::cout << filepath << std::endl;
    if(std::filesystem::exists(filepath)){
        return true;
    }
    
    return false;
}

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
        // std::cout << "Message from client: " << buffer << std::endl;
        
        //create request object
        http_request_t request = parse_html_request(buffer);
        std::cout << "REQUEST TO PATH: " << request.path << std::endl;

        std::string filename = get_filename_from_path(request.path);

        try {
            if(file_exists(filename)){
                content = read_file(get_filename_from_path(request.path));       
                response = create_http_response(STATUS_OK, get_current_date(), content.size(), "text/html", content);
            } else {
                //file not found -> 404
                content = read_file("html/notfound.html");
                response = create_http_response(STATUS_NOT_FOUND, get_current_date(), content.size(), "text/html", content);
            }
        } catch(int error_code){
            //internal server error -> 500
            LOG("Error: "+error_code);
            content = read_file("html/servererror.html");
            response = create_http_response(STATUS_INTERNAL_SERVER_ERROR, get_current_date(), content.size(), "text/html", content);
        }

        send(client_socket, response.c_str(), response.size(), 0);
        closesocket(client_socket);
    }
    
    closesocket(server_socket);
    WSACleanup();

    return 0;
}