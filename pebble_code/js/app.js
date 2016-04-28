Pebble.addEventListener("appmessage",
	function(e) {
        console.log("IN EVENT LISTENING");
		sendToServer(e);
	}
);
function sendToServer(e) {
    console.log("BEGIN sendToServer");
    var req = new XMLHttpRequest();
    var ipAddress = "158.130.214.211"; // Hard coded IP address of server
    var port = "3001"; // Same port specified as argument to server
    //var url = "http://" + ipAddress + ":" + port + "/";
    
    var dict = e.payload;
    console.log('Got message: ' + JSON.stringify(dict));
	var url = "";
    if(e.payload["temp_curr_mult"]=="1"){            
      url = "http://" + ipAddress + ":" + port + "?q=1"; //onF  
    } else if(e.payload["temp_curr_mult"]=="2"){     
      url = "http://" + ipAddress + ":" + port + "?q=2"; //onC  
    } else if(e.payload["temp_curr_mult"]=="3"){     
      url = "http://" + ipAddress + ":" + port + "?q=3"; //offF 
    } else if(e.payload["temp_curr_mult"]=="4"){     
      url = "http://" + ipAddress + ":" + port + "?q=4"; //offC 
    }

    console.log("" + url);
	var method = "GET";
    var async = true;
    
    
    req.onload = function(e) {
        console.log("BEGIN onload");
        // see what came back
        var msg = "no response";
        var msg2 = "no response";
        var msg3 = "no response";
        var msg4 = "no response";
        console.log("BEGIN onload 1");
        console.log("begin " + req.responseText);
        var response = JSON.parse(req.responseText);
        console.log("BEGIN onload 2");
        if (response) {
            console.log("GOT A RESPONSE");
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
        console.log("SENDING APP MESSAGE");
        // sends message back to pebble
        Pebble.sendAppMessage({ "0": msg, "1": msg2, "2" : msg3, "3" : msg4 });
    };
    req.open(method, url, async);
    req.send(null);
}
