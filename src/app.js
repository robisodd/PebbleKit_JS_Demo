
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
//  Global Variables
// ------------------------------


// ------------------------------------------------------------------------------------------------------------------------------------------------ //
//  Pebble Functions
// ------------------------------
function appmessage_success(data) {
  console.log("Successfully sent appmessage to pebble.");
}

function appmessage_fail(data, error) {
  console.log("Failed sending appmessage to pebble: " + error);
}


Pebble.addEventListener("ready", function(e) {
  console.log("PebbleKit JS Has Started!");
  Pebble.sendAppMessage({"message":"PebbleKit JS Ready!"}, appmessage_success, appmessage_fail);  // let watch know JS is ready
});


const COMMAND_STOP  = 100; // jshint ignore:line
const COMMAND_START = 101; // jshint ignore:line
const COMMAND_COUNT = 103; // jshint ignore:line

var count = 0;

Pebble.addEventListener("appmessage", function(e) {
  console.log("App Message Received");
  
  // If we received a command from Pebble
  if (typeof e.payload.command !== 'undefined') {
    console.log("Received Command from Pebble");
    if(e.payload.command === COMMAND_START) {
      console.log("Command = START");
      Pebble.sendAppMessage({"message" : "Started"}, appmessage_success, appmessage_fail);
    } else if(e.payload.command === COMMAND_STOP) {
      console.log("Command = STOP");
      Pebble.sendAppMessage({"message" : "Stopped"}, appmessage_success, appmessage_fail);
    } else if(e.payload.command === COMMAND_COUNT) {
      console.log("Command = COUNT");
      count++;
      console.log("Count is now " + count);
      Pebble.sendAppMessage({"message" : "Incremented count!", "count" : count}, appmessage_success, appmessage_fail);
    } else {
      console.log("Received Unknown command from Pebble C: " + e.payload.command);
    }
  }
  
    if (typeof e.payload.message !== 'undefined') {
      console.log('Received string from Pebble: "' + e.payload.message + '"');
    }
});


// function send_GPS_position_to_pebble(lat, lon) {
//   var lat_int = Math.round((lat / 360) * GPS_MAX_ANGLE);
//   var lon_int = Math.round((lon / 360) * GPS_MAX_ANGLE);
//   console.log("Sending GPS to pebble: (" + lat + ", " + lon + ") = (" + lat_int + ", " + lon_int + ")");
//   Pebble.sendAppMessage({"gps_lat" : lat_int, "gps_lon" : lon_int},
//                         null, //function(){console.log("Successfully sent position to pebble:");},
//                         function(){console.log("Failed sending position to pebble");}
//                        );
// }
