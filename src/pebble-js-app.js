var INSTALLED_VERSION = 12;

Pebble.addEventListener("ready",
  function(e) {
    console.log("Event listener - ready");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    console.log("Event listener - showConfiguration");
    Pebble.openURL("http://www.sherbeck.com/pebble/fillerup.html?" + formatUrlVariables());
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    console.log("Event listener - webviewclosed");
    
    if (typeof(e.response) === "undefined" || e.response == "CANCELLED") {
      console.log("Settings canceled by user");
      
    } else {
      var configuration = JSON.parse(decodeURIComponent(e.response));
      console.log("Configuration window returned: " + JSON.stringify(configuration));
      
      saveSettings(configuration);

      var dictionary = {
        "KEY_CURRENT_VERSION" : parseInt(configuration.currentVersion),
        "KEY_INSTALLED_VERSION" : parseInt(INSTALLED_VERSION),
        "KEY_HOUR_VIBRATE" : parseInt(configuration.hourVibrate),
        "KEY_BLUETOOTH_VIBRATE" : parseInt(configuration.bluetoothVibrate)
      };
  
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Settings successfully sent to Pebble");
        },
        function(e) {
          console.log("Error sending settings to Pebble");
        }
      );
    }
  }
);

function formatUrlVariables() {
  var hourVibrate = parseInt(localStorage.getItem("hourVibrate"));
  var bluetoothVibrate = parseInt(localStorage.getItem("bluetoothVibrate"));
	
	if (isNaN(hourVibrate)) {
		hourVibrate = 0;
	}
	
	if (isNaN(bluetoothVibrate)) {
		bluetoothVibrate = 1;
	}
  
  return ("installedVersion=" + INSTALLED_VERSION + "&hourVibrate=" + hourVibrate + "&bluetoothVibrate=" + bluetoothVibrate);
}

function saveSettings(settings) {
  localStorage.setItem("currentVersion", parseInt(settings.currentVersion));  
  localStorage.setItem("hourVibrate", parseInt(settings.hourVibrate));  
  localStorage.setItem("bluetoothVibrate", parseInt(settings.bluetoothVibrate)); 
}