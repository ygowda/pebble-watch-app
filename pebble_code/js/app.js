Pebble.addEventListener("appmessage",
function(e) {
    sendToServer(e);
}
);
function sendToServer(e) {
    console.log("BEGIN sendToServer");
  console.log("   payload: " + e.payload[0]);
  console.log("   type: " + e.payload.constructor.name);
    var req = new XMLHttpRequest();
    var ipAddress = "158.130.214.211"; // Hard coded IP address of server
    var port = "3001"; // Same port specified as argument to server
 //   var url = "http://" + ipAddress + ":" + port + "/";
    var url = "";
    if(e.payload[0]=="1"){            
      url = "http://" + ipAddress + ":" + port + "?q=1"; //onF     lights on 
    } else if(e.payload[0]=="2"){     
      url = "http://" + ipAddress + ":" + port + "?q=2"; //onC    Farenheit on
    }
//  else{
//      url = "http://" + ipAddress + ":" + port + "/";
//  }
    
//     if(e.payload[1] =="1"){
//        url = url + "1"; //f on 
//     }
//   else{
//     url = url + "0"; //c on
//   }
  
    var method = "GET";
    var async = true;
    
    
    req.onload = function(e) {
//       if(!req.onload){
//         console.log("hiiiiii");
// }
//   console.log("----------req: " + req.onload);
      
      //console.log("key being sent:" +e.payload);

        console.log("BEGIN onload");
        // see what came back
        var msg = "no response";
        var msg2 = "no response";
        var msg3 = "no response";
        var msg4 = "no response";
        var response = JSON.parse(req.responseText);
        if (response) {
 //         console.log("*********response: " +response);
//           if(response.Arduino_was_disconnected){
//             msg = "arduino was disconnected";
//             console.log("arduino was disconnected");
//         }
       //  else{

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

            //some how you are calling temp_min_mult??
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
         // }

        }
      
      else{
          msg = "middleware disconnected";
          msg2 = "middleware disconnected";
          msg3 = "middleware disconnected";
          msg4 = "middleware disconnected";
          
      }
        // sends message back to pebble
        Pebble.sendAppMessage({ "0": msg, "1": msg2, "2" : msg3, "3" : msg4 });
    };
    req.open(method, url, async);
    req.send(null);
}
