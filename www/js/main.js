const wsUri = "ws://127.0.0.1:9001";
const websocket = new WebSocket(wsUri);

websocket.addEventListener("open", () => {
    console.log("connected to websocket server");
    pingInterval = setInterval(() => {
        websocket.send("ping");
    }, 2000);
});

websocket.addEventListener("error", () => {
    console.log("websocket error");
});