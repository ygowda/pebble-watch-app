Pebble.addEventListener("appmessage",
function(e) {
    sendToServer();
}
);
function sendToServer() {
    console.log("BEGIN sendToServer");
    var req = new XMLHttpRequest();
    var ipAddress = "10.251.73.5"; // Hard coded IP address of server
    var port = "3001"; // Same port specified as argument to server
    var url = "http://" + ipAddress + ":" + port + "/";
    var method = "GET";
    var async = true;
    
    
    req.onload = function(e) {
        console.log("BEGIN onload");
        // see what came back
        var msg = "no response";
        var msg2 = "no response";
        var msg3 = "no response";
        var msg4 = "no response";
        var response = JSON.parse(req.responseText);
        if (response) {
            if (response.temp_curr_mult) {
                msg = response.temp_curr_mult;
                console.log("onload temp_curr_mult " + msg);
            }
            else msg = "no temp_curr_mult";

            if (response.temp_max_mult) {
                msg2 = response.temp_max_mult;
                console.log("onload temp_max_mult " + msg2);
            }
            else msg2 = "no temp_max_mult";

            if (response.temp_min_mult) {
                msg3 = response.temp_min_mult;
                console.log("onload temp_min_mult " + msg3);
            }
            else msg3 = "no temp_max_mult";

            if (response.temp_avg_mult) {
                msg4 = response.temp_avg_mult;
                console.log("onload temp_avg_mult " + msg4);
            }
            else msg4 = "no temp_max_mult";
        }
        // sends message back to pebble
        Pebble.sendAppMessage({ "0": msg, "1": msg2, "2" : msg3, "3" : msg4 });
    };
    req.open(method, url, async);
    req.send(null);
}
