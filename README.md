# HTTP 1.1 Server 🦎

- My first try on making a HTTP server with C++. At the moment using [Win32 API](https://learn.microsoft.com/en-us/windows/win32/api/) API, [nlohmannjson](https://github.com/nlohmann/json) and [IXWebSocket](https://github.com/machinezone/IXWebSocket).
- All the functionality is right now in main.cpp since i wanted to start coding this more freely, all refactoring+organizing will be done later...

## Now working
- Basic file serving from www folder
- Automatic index.html on root /
- Tries to find html file for /page
- Supports url parameters
- Multiple file formats
    - html, htm, css, js, json webp, png, jpg, mp3, mp4, gif, php, svg 
- 200, 404, 405 and 500 status
- Allows only GET method
- Checking for file/folder modifications inside the www folder
- Prevented path traversal etc.
- Auto refresh JS injection
- Websocket server for auto refresh

## How to use
Move your webpage content to the www folder
```cmd
> Run these commands
make
cd bin
server.exe
```
Then go to localhost:8081

## Updates:
- UI (probably with imgui or Win32 API)
- Fixing possible bugs
- Better error checking
- Project structure organization
- Refactoring

Toivo Lindholm 2025
