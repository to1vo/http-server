#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <filesystem>
#include <ctime>
#include <windows.h>
#include <assert.h>
#include <thread>
#include <nlohmann/json.hpp> 
#include <mutex>
#include <queue>

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
    If this function would crash/fail/throw -> server error 500
*/
std::string read_file(const std::string& filename, const std::string& file_extension){
    std::string output;
    std::string content;

    /*
        Read binary file, this includes things like images
    */
    if(file_extension == ".png"){
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if(size == 0){
            throw std::runtime_error("File is empty or cannot read it");
        }

        std::string content(size, '\0');
        if(!file.read(&content[0], size)){
            throw std::runtime_error("Failed to read file " + filename);
        }        

        std::cout << "Succesfully read " << size << " bytes into string" << std::endl;

        file.close();

        return content;
    }

    //Reading normal text file
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
            //start of path
            for(int j=i; j<request_str.length(); j++){
                // if(request_str[j] == '?'){
                //     //handle the params
                //     break;
                // }
                if(request_str[j] == ' ' || request_str[j] == '?'){
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

//Expects that the file has actually a extension
std::string get_file_extension(const std::string& filename){
    std::size_t starting_index = filename.rfind('.');
    std::string file_extension;
    
    for(int i=starting_index; i<filename.length(); i++){
        file_extension += filename[i];    
    }

    return file_extension;
}

/*
    Idea of this is to tell the content type
    by the files extension
*/
std::string get_content_type(const std::string& file_extension){
    std::string content_type;
    
    if(file_extension == ".html"){
        content_type = "text/html";
    } else  
    if(file_extension == ".css"){
        content_type = "text/css";
    } else 
    if(file_extension == ".js"){
        content_type = "text/javascript";
    } else 
    if(file_extension == ".json"){
        content_type = "application/json";
    } else
    if(file_extension == ".png"){
        content_type = "image/png";
    }

    return content_type;
}

typedef struct file_modification {
    std::string filename;
    std::string action;
} file_modification_t;

std::mutex websocket_queue_mutex;
std::queue<file_modification_t> websocket_event_queue;

//sends message to websocket client
void websocket_send(nlohmann::json json){

}

/*
    Handle the file modification object
    create the json message that will be sent through the websocket server
    and pass that to websocket send function
*/
void handle_file_modification(file_modification_t& file_modification){
    //making sure that filename contains only ASCII characters
    std::string checked_filename;
    for(uint16_t i=0; i<file_modification.filename.length(); i++){
        if(static_cast<unsigned char>(file_modification.filename[i]) > 127){
            checked_filename += file_modification.filename[i];
        }
    }
    file_modification.filename = checked_filename;

    //create the json message
    nlohmann::json json;
    json["filename"] = file_modification.filename;
    json["action"] = file_modification.action;

    //websocket send function
    websocket_send(json);
}

void check_for_file_changes(){
    const char* website_path = DOCUMENT_ROOT;
    
    std::cout << "Checking for file changes in " << website_path << "/" << std::endl;

    //Create handler to the folder
    HANDLE file = CreateFile(
        website_path, 
        FILE_LIST_DIRECTORY, 
        FILE_SHARE_READ | 
        FILE_SHARE_WRITE | 
        FILE_SHARE_DELETE, 
        NULL, 
        OPEN_EXISTING, 
        FILE_FLAG_BACKUP_SEMANTICS | 
        FILE_FLAG_OVERLAPPED, 
        NULL
    );
    
    assert(file != INVALID_HANDLE_VALUE);
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
    
    //Create a buffer to save the changes
    alignas(DWORD) uint8_t change_buffer[1024];

    BOOL success = ReadDirectoryChangesW(
        file, 
        change_buffer, 
        sizeof(change_buffer), 
        TRUE, 
        FILE_NOTIFY_CHANGE_FILE_NAME | 
        FILE_NOTIFY_CHANGE_DIR_NAME | 
        FILE_NOTIFY_CHANGE_LAST_WRITE, 
        NULL, &overlapped, NULL
    );

    while(true){
        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);
        
        if(result == WAIT_OBJECT_0){
            std::cout << "FILE CHANGE HAPPENED" << std::endl;
            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            //store the event
            FILE_NOTIFY_INFORMATION *event = (FILE_NOTIFY_INFORMATION*)change_buffer;

            while(true){
                DWORD name_len = event->FileNameLength / sizeof(wchar_t);
            
                //read to whole filename
                std::string filename;
                for(int i=0; i<(int)wcslen(event->FileName); i++){
                    filename += event->FileName[i];
                }
            
                std::cout << "Filename: " << filename << std::endl;
            
                std::string action;
                switch(event->Action){
                    case FILE_ACTION_ADDED:
                        std::cout << "File added" << std::endl;
                        action = "added"; 
                        break;
                    case FILE_ACTION_REMOVED:
                        std::cout << "File removed" << std::endl;
                        action = "removed";
                        break;
                    case FILE_ACTION_MODIFIED:
                        std::cout << "File modified" << std::endl;
                        action = "modified";
                        break;
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        std::cout << "File Renamed" << std::endl;
                        action = "renamed";
                        break;
                    default:
                        std::cout << "Unknown action" << std::endl;
                        action = "";
                        break;
            }
            
            //there was no undefined action
            if(!action.empty()){
                //create filechange object
                file_modification_t file_modification {
                    .filename = filename,
                    .action = action
                };
                //call thread 3 for file modification
            }
            
            //more events to handle?
            if(event->NextEntryOffset){
                *((uint8_t**)&event) += event->NextEntryOffset;
            } else {
                break;
            }
            }
        }
        
        //queue the next event
        BOOL success = ReadDirectoryChangesW(
            file, 
            change_buffer, 
            sizeof(change_buffer), 
            TRUE, 
            FILE_NOTIFY_CHANGE_FILE_NAME | 
            FILE_NOTIFY_CHANGE_DIR_NAME | 
            FILE_NOTIFY_CHANGE_LAST_WRITE, 
            NULL, &overlapped, NULL
        );      
    }
}

int main(){
    //thread for handling the file changes
    // std::thread thread2(check_for_file_changes);
    //this will be used for websocket server
    // std::thread thread3();

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
                std::string file_extension = get_file_extension(filename);
                content = read_file(filename, file_extension);       
                response = create_http_response(STATUS_OK, get_current_date(), content.size(), get_content_type(file_extension), content);
            } else {
                //file not found -> 404
                content = read_file("html/notfound.html", ".html");
                response = create_http_response(STATUS_NOT_FOUND, get_current_date(), content.size(), "text/html", content);
            }
        } catch(const std::exception& error){
            //internal server error -> 500
            // LOG(error);
            content = read_file("html/servererror.html", ".html");
            response = create_http_response(STATUS_INTERNAL_SERVER_ERROR, get_current_date(), content.size(), "text/html", content);
        }

        send(client_socket, response.c_str(), response.size(), 0);
        closesocket(client_socket);
    }
    
    closesocket(server_socket);
    WSACleanup();

    return 0;
}