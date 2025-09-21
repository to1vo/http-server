# HTTP 1.1 Server 🦎

- My first try on making a HTTP server with C++. At the moment using [Win32 API](https://learn.microsoft.com/en-us/windows/win32/api/) API and [nlohmannjson](https://github.com/nlohmann/json).
- All the functionality is right now in main.cpp since i wanted to start coding this more freely, all refactoring+organizing will be done later...

## Now working
- Basic file serving from www folder
- Automatic index.html on root /
- Tries to find html file for /page
- Supports url parameters
- Images (atm only .png)
- 200, 404 and 500 status
- Checking for file/folder modifications inside the www folder
- Prevented path traversal etc.

## Updates:
- Auto refresh JS injection
- Websocket server for auto refresh
- Support for common image types
- UI (probably with imgui or Win32 API)
- Project structure organization
- Refactoring

Toivo Lindholm 2025
