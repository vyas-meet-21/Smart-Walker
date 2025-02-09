#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>  // Servo library for steering control
#include <ESP32PWM.h>    // PWM support for motor control

// Function prototype for stopAllMotors
void stopAllMotors();
void moveSteer(int degrees);

// WiFi credentials
const char* ssid = "ESP32_AccessPoint";  // Replace with desired SSID
const char* password = "Walker32";       // Replace with desired password

// Web server on port 80
WebServer server(80);

// Pin configurations
const int deadmanSwitchPin = 13;  // Deadman switch
const int motorForwardPin = 25;   // Motor forward (PWM pin for speed control)
const int moveLeftPin = 14;       // Move left
const int moveRightPin = 27;      // Move right
const int steerPin = 18;          // Servo control pin for steering

// State variables
bool deadmanSwitchState = false;   // Deadman switch state (from joystick)
bool webDeadmanState = false;      // Deadman switch state (from web)
int steerAngle = 90;               // Steering angle (0-180)
bool motorForwardLastState = false; // Last known state of the motor forward button

// Servo object for steering
Servo steerServo;

// HTML content for the web page
const char* htmlPage = R"=====(  
<!DOCTYPE html>
<html>
<head>
  <title>Walker Control</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    h2 { font-size: 24px; }
    .button { padding: 10px 20px; font-size: 18px; margin: 10px; border: none; cursor: pointer; transition: background-color 0.3s; }
    .button-on { background-color: green; color: white; }
    .button-off { background-color: red; color: white; }
    .button-steer-5, .button-velocity-5 { background-color: green; color: white; }
    .button-steer-15, .button-velocity-10 { background-color: orange; color: white; }
    .button-steer-25, .button-velocity-15 { background-color: red; color: white; }
    .slider input { width: 80%; }
  </style>
</head>
<body>
  <h2>Walker Control Interface</h2>
  
 <div>
    <label>Dead Man Switch Status:</label><br>
    <span id="deadmanStatus">Inactive</span>
  </div>
  
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Slider with Labels</title>
  <style>
    .slider {
      width: 80%;
      margin: 20px auto;
      text-align: center;
    }
    .slider input { width: 100%; }
    .slider-labels {
      display: flex;
      justify-content: space-between;
      font-size: 12px;
      color: #555;
      margin-top: 5px;
    }
  </style>
</head>
<body>
  <div class="slider">
    <label>Forward Distance:</label><br>
    <input type="range" min="0" max="100" step="10" value="100" id="forwardDistance" 
       oninput="updateValue(this.value); sendSliderValue(this.value)">
    <div class="slider-labels">
      <span>0</span><span>10</span><span>20</span><span>30</span><span>40</span>
      <span>50</span><span>60</span><span>70</span><span>80</span><span>90</span><span>100</span>
    </div>
    <span id="valueDisplay">100</span>
  </div>

 <script>
  const updateValue = val => document.getElementById('valueDisplay').innerText = val;

  function sendSliderValue(value) {
    fetch('/control?cmd=slider&value=' + value)
      .then(response => response.text())
      .then(data => console.log(data));
  }

  // On page load, make an initial request to get the current state of the deadman switch
  window.onload = function() {
    fetch('/get_state')
      .then(response => response.json())
      .then(data => {
        const deadmanState = data.deadmanState;
        const deadmanStatusElement = document.getElementById('deadmanStatus');
        if (deadmanState === 'active') {
          deadmanStatusElement.innerText = 'Active';  // Display Active if the Deadman Switch is pressed
        } else {
          deadmanStatusElement.innerText = 'Inactive';  // Display Inactive otherwise
        }
      });
  };

</script>
</body>
</html>

  
  <div class="button-group">
    <label>Steer:</label><br>
    <button class="button button-steer-5" onclick="sendData('steer_5')">5</button>
    <button class="button button-steer-15" onclick="sendData('steer_15')">15</button>
    <button class="button button-steer-25" onclick="sendData('steer_25')">25</button>
  </div>
  
  <div class="button-group">
    <label>Velocity:</label><br>
    <button class="button button-velocity-5" onclick="sendData('velocity_5')">5</button>
    <button class="button button-velocity-10" onclick="sendData('velocity_10')">10</button>
    <button class="button button-velocity-15" onclick="sendData('velocity_15')">15</button>
  </div>
  
  <div class="button-group">
    <label>Reset Steer:</label><br>
    <button class="button" onclick="sendData('reset_steer')">Reset</button>
  </div>
  
  <script>
    let deadmanState = 'off';  // Variable to store the current state of the deadman switch

    function toggleDeadman(state) {
      if (state === 'on') {
        document.getElementById('deadmanOnBtn').classList.add('button-on');
        document.getElementById('deadmanOnBtn').classList.remove('button-off');
        document.getElementById('deadmanOffBtn').classList.add('button-off');
        document.getElementById('deadmanOffBtn').classList.remove('button-on');
        sendData('deadman_on');
        deadmanState = 'on';  // Update the local state when it turns on
      } else {
        document.getElementById('deadmanOffBtn').classList.add('button-on');
        document.getElementById('deadmanOffBtn').classList.remove('button-off');
        document.getElementById('deadmanOnBtn').classList.add('button-off');
        document.getElementById('deadmanOnBtn').classList.remove('button-on');
        sendData('deadman_off');
        deadmanState = 'off';  // Update the local state when it turns off
      }
    }

    function sendData(command) {
      fetch('/control?cmd=' + command)
        .then(response => response.text())
        .then(data => console.log(data));
    }

    function updateSlider(slider, value) {
      document.getElementById(slider + 'Value').innerText = value;
      fetch('/control?cmd=' + slider + '&value=' + value)
        .then(response => response.text())
        .then(data => console.log(data));
    }

    // On page load, make an initial request to get the current state of the deadman switch
    setInterval(() => {
       fetch('/get_state')
         .then(response => response.json())
         .then(data => {
         const deadmanState = data.deadmanState;
         const deadmanStatusElement = document.getElementById('deadmanStatus');
         deadmanStatusElement.innerText = deadmanState === 'active' ? 'Active' : 'Inactive';
        });
    }, 1000);  // Update every second

    window.onload = function() {
      fetch('/get_state')
        .then(response => response.json())
        .then(data => {
          deadmanState = data.deadmanState;
          if (deadmanState === 'on') {
            document.getElementById('deadmanOnBtn').classList.add('button-on');
            document.getElementById('deadmanOnBtn').classList.remove('button-off');
            document.getElementById('deadmanOffBtn').classList.add('button-off');
            document.getElementById('deadmanOffBtn').classList.remove('button-on');
          }
        });
    };
  </script>
</body>
</html>
)=====";

int steerChange = 5;


void setup() {
  Serial.begin(115200);

  // Configure pins
  pinMode(deadmanSwitchPin, INPUT_PULLUP);  // Deadman switch
  pinMode(motorForwardPin, INPUT_PULLUP);   // Motor start button
  pinMode(moveLeftPin, INPUT_PULLUP);       // Left movement button
  pinMode(moveRightPin, INPUT_PULLUP);      // Right movement button
  steerServo.attach(steerPin);              // Attach servo for steering

  // Initialize motor and steering to OFF/neutral
  steerServo.write(90);  // Neutral position for servo

  // Set up WiFi as an Access Point
  WiFi.softAP(ssid, password);
  Serial.println("WiFi Access Point started!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

// HTTP route for getting the current state of the Dead Man Switch
  server.on("/get_state", HTTP_GET, []() {
  // Combine both the hardware and web states
  String deadmanState = (deadmanSwitchState || webDeadmanState) ? "active" : "inactive";

  // Send the status as a JSON response
  String jsonResponse = "{\"deadmanState\":\"" + deadmanState + "\"}";
  server.send(200, "application/json", jsonResponse); // Respond with JSON
});



  // HTTP route for web page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage); // Serve the web page
  });
  // HTTP route for control commands
  server.on("/control", HTTP_GET, []() {
    String command = server.arg("cmd");

    if (command == "slider") {
      if (server.hasArg("value")) {
        String sliderValue = server.arg("value");
        Serial.println("Forward Distance Slider Value: " + sliderValue);
      }
    } else if (command == "move_left" && (deadmanSwitchState || webDeadmanState)) {
      moveSteer(-steerChange);  // Turn left by 5 degrees
      Serial.println("Moved Left");
    } else if (command == "move_right" && (deadmanSwitchState || webDeadmanState)) {
      moveSteer(steerChange);  // Turn right by 5 degrees
      Serial.println("Moved Right");
    } else if (command == "reset_steer" && (deadmanSwitchState || webDeadmanState)) {
      steerServo.write(90);  // Reset servo to neutral
      steerAngle = 90;
      Serial.println("Steering reset to neutral");
    } else if (command.startsWith("steer_")) {
      steerChange = command.substring(6).toInt();
      moveSteer(steerChange);
      Serial.println("Steering updated by: " + String(steerChange));
    } else if (command.startsWith("velocity_")) {
      int velocity = command.substring(9).toInt();
      Serial.println("Velocity set to: " + String(velocity));
    } else if (command == "deadman_on") {
      webDeadmanState = true;  // Set the web-controlled Deadman state to ON
      Serial.println("Deadman switch ON from web");
    } else if (command == "deadman_off") {
      webDeadmanState = false; // Set the web-controlled Deadman state to OFF
      stopAllMotors();
      Serial.println("Deadman switch OFF from web");
    } else {
      Serial.println("Invalid command or Deadman switch is OFF");
    }

    server.send(200, "text/plain", "Command received");
  });

  // Start the server
  server.begin();
  Serial.println("Server started");
}


void loop() {
  // Handle client requests for the web interface
  server.handleClient();

  // Read joystick buttons
  bool moveLeftState = (digitalRead(moveLeftPin) == LOW);
  bool moveRightState = (digitalRead(moveRightPin) == LOW);
  bool motorForwardState = (digitalRead(motorForwardPin) == LOW); // Read forward button
  bool deadmanSwitchPressed = (digitalRead(deadmanSwitchPin) == LOW);

  // Update the deadman switch state based on joystick button
  if (deadmanSwitchPressed && !webDeadmanState) {
    if (!deadmanSwitchState) {
      deadmanSwitchState = true;
      Serial.println("Deadman Switch: ON (Physical)");
    }
  } else if (!deadmanSwitchPressed && !webDeadmanState) {
    if (deadmanSwitchState) {
      deadmanSwitchState = false;
      stopAllMotors();
      Serial.println("Deadman Switch: OFF (Physical)");
    }
  }
  // Handle Move Left button press
  if (moveLeftState && !moveRightState && (deadmanSwitchState || webDeadmanState)) {
    moveSteer(-steerChange);  // Turn left by 5 degrees
    delay(300);     // Simple debounce
  }

  // Handle Move Right button press
  if (moveRightState && !moveLeftState && (deadmanSwitchState || webDeadmanState)) {
    moveSteer(steerChange);   // Turn right by 5 degrees
    delay(300);     // Simple debounce
  }

  // Handle simultaneous press of Move Left and Move Right
  if (moveLeftState && moveRightState && (deadmanSwitchState || webDeadmanState)) {
    steerServo.write(90);  // Reset servo to neutral
    steerAngle = 90;
    Serial.println("Steering reset to neutral 90");
    delay(300);  // Simple debounce
  }

  // Handle Motor Forward button press with state change tracking
  if (motorForwardState != motorForwardLastState) { // State has changed
    motorForwardLastState = motorForwardState; // Update the last known state

    if (motorForwardState && (deadmanSwitchState || webDeadmanState)) {
      Serial.println("Motor Forward: Activated");
    } else {
      Serial.println("Motor Forward: Stopped");
    }
  }
}

// Move Steering Servo by a specific angle
void moveSteer(int degrees) {
  steerAngle += degrees;
  // Constrain the steering angle between 0 and 180 degrees
  steerAngle = constrain(steerAngle, 0, 180);
  steerServo.write(steerAngle);
  Serial.print("Steering angle updated to: ");
  Serial.println(steerAngle);
}

// Stop all motors and reset states
void stopAllMotors() {
  steerServo.write(90);  // Neutral position for servo
  steerAngle = 90;
}