# README #

This is our project to make a pebble watch talk to an Arduino.

### Really Hot Birds - TODO ###
* Calibration, avg +-4 (Selah)
* Menu Screen, merge with functionality (Dan)
* Farenheight min temperature is no good (Selah)
* Differentiate pebble->server requests, party mode triggered by select button (Selah)
* Differentiate server->arduino requests(Farenheit or PartyMode) (YASHUS) (SELAH)
* Check if the server does not get connection from pebble code (ex: wrong ip address) then display error message next time (DAN)
* Stand by mode. Arduino screen should not display reading and neither should watch but temp should still be tracked. (YASHUS), (DAN).
*Limit stats to last hour (DAN)



##DONE##
* Pebble screen showing min, max and average (Selah)
* Combining animation and temperature data (SELAH & DAN)
* Cause arduino to make fancy lights (YASHUS)
* Send message from server to arduino (YASHUS)
* Server handle multiple requests (SELAH)
* Game graphics looping with random numbers (DAN)
* display error message on the watch when the arduino gets disconnected (server already shows message).






### Windows ###

1. "Press Play to Start"
2. Calibration countdown
3. Play the game!!! (game loop)
4. Final score and back to "Press Play to Start"

### SDK Command Line Stuff ###

*TO MAKE PROJECT...
pebble new-project hello-pebble --simple

*TO RUN... in project directory
pebble build && pebble install --emulator basalt --logs
pebble build && pebble install --cloudpebble --logs

### MARKDOWN HOW TO ###
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)