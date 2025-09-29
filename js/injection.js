const websocketURI = "ws://127.0.0.1:9001";
const websocket = new WebSocket(websocketURI);

websocket.addEventListener("open", () => {
    console.log("Connected to websocket server!");
    websocket.send("Hello from client!");
});

websocket.addEventListener("message", (msg) => {
    const json = JSON.parse(msg.data);
    if(!json.filename) return;

    const date = new Date();
    if(json.filename.includes(".css")){
        const element = document.querySelector(`link[href^="${json.filename}"]`);
        element.setAttribute("href", json.filename+"?q="+date.getMilliseconds());
        return;
    }
    window.location.reload();
});