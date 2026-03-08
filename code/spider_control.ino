#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>

// --- HARDWARE CONFIGURATION ---
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 50
#define SERVOMIN  150
#define SERVOMAX  600

// --- WIFI CREDENTIALS ---
const char* ssid = "Sid";
const char* password = "cat12345";

WebServer server(80);

// --- MOVEMENT STATE & SMOOTHING ---
float currentAngle[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
float targetAngle[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
float servoSpeed = 3.5; // Degrees per update for natural movement

// --- UTILITY FUNCTIONS ---
int angleToPulse(int angle) {
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setServoNow(int channel, float angle) {
  int pulse = angleToPulse((int)angle);
  pwm.setPWM(channel, 0, pulse);
}

void moveServo(int channel, int angle) {
  if (channel < 0 || channel > 15) return;
  angle = constrain(angle, 0, 180);
  if (currentAngle[channel] == -1) {
    currentAngle[channel] = angle;
    setServoNow(channel, angle);
  }
  targetAngle[channel] = angle;
}

void updateServos() {
  for (int i = 0; i < 16; i++) {
    if (currentAngle[i] == -1) continue;
    if (currentAngle[i] != targetAngle[i]) {
      float diff = targetAngle[i] - currentAngle[i];
      if (abs(diff) <= servoSpeed) {
        currentAngle[i] = targetAngle[i];
      } else {
        currentAngle[i] += (diff > 0) ? servoSpeed : -servoSpeed;
      }
      setServoNow(i, currentAngle[i]);
    }
  }
}

void waitForServo(int channel) {
  if (currentAngle[channel] == -1) return;
  while (abs(currentAngle[channel] - targetAngle[channel]) > 0.5) {
    updateServos();
    delay(20);
  }
}

void waitForAllServos() {
  bool moving = true;
  while (moving) {
    moving = false;
    for (int i = 0; i < 16; i++) {
      if (abs(currentAngle[i] - targetAngle[i]) > 0.5) {
        moving = true;
        break;
      }
    }
    updateServos();
    delay(20);
  }
}

void smoothMove(int channel, int targetAngleVal, int speedDelay = 5) {
  if (channel < 0 || channel > 15) return;
  targetAngleVal = constrain(targetAngleVal, 0, 180);
  if (currentAngle[channel] == -1) {
    currentAngle[channel] = targetAngleVal;
    setServoNow(channel, targetAngleVal);
    return;
  }
  int start = (int)currentAngle[channel];
  if (start < targetAngleVal) {
    for (int angle = start; angle <= targetAngleVal; angle++) {
      setServoNow(channel, angle);
      currentAngle[channel] = targetAngleVal; // Update for logic
      delay(speedDelay);
    }
  } else {
    for (int angle = start; angle >= targetAngleVal; angle--) {
      setServoNow(channel, angle);
      currentAngle[channel] = targetAngleVal;
      delay(speedDelay);
    }
  }
  currentAngle[channel] = targetAngleVal;
  targetAngle[channel] = targetAngleVal;
}

void moveMultipleServosArray(int channels[], int angles[], int count) {
  for (int i = 0; i < count; i++) moveServo(channels[i], angles[i]);
  waitForAllServos();
}

// --- LEG MOVEMENT GAITS ---
void intializepos() {
  moveServo(0, 100); waitForServo(0);
  moveServo(1, 70);  waitForServo(1);
  moveServo(2, 110); waitForServo(2);
  moveServo(3, 70);  waitForServo(3); 
  moveServo(4, 120); waitForServo(4); 
  moveServo(5, 120); waitForServo(5); 
  moveServo(6, 60);
  moveServo(8, 80);  waitForServo(8);
  moveServo(7, 120); waitForServo(7);
  moveServo(9, 110); waitForServo(9);
  moveServo(10, 80); waitForServo(10);
  moveServo(11, 70); waitForServo(11);
}

void moveforward() {
  smoothMove(7, 70, 2); delay(2);
  smoothMove(6, 30, 2); delay(2);
  smoothMove(7, 110, 2); delay(2);
  smoothMove(1, 110, 2); delay(2);
  smoothMove(0, 140, 2); delay(2);
  smoothMove(1, 80, 2); delay(2);
  smoothMove(4, 130, 2); delay(2);
  smoothMove(3, 110, 2); delay(2);
  smoothMove(4, 120, 2); delay(2);
  smoothMove(10, 70, 2); delay(2);
  smoothMove(9, 70, 2); delay(2);
  smoothMove(10, 90, 2); delay(2);
  int c[] = {0, 3, 6, 9}; int a[] = {100, 70, 70, 110};
  moveMultipleServosArray(c, a, 4);
}

void movebackward() {
  smoothMove(10, 70, 5); delay(10);
  smoothMove(9, 135, 5); delay(10);
  smoothMove(10, 90, 5); delay(10);
  smoothMove(4, 130, 5); delay(10);
  smoothMove(3, 45, 5);  delay(10); 
  smoothMove(4, 120, 5); delay(10);
  smoothMove(1, 90, 5);  delay(10);
  smoothMove(0, 60, 5);  delay(10); 
  smoothMove(1, 80, 5);  delay(10);
  smoothMove(7, 110, 5); delay(10);
  smoothMove(6, 100, 5); delay(10);
  smoothMove(7, 120, 5); delay(10);
  int c[] = {0, 3, 6, 9}; int a[] = {100, 70, 70, 110};
  moveMultipleServosArray(c, a, 4);
}

void moveright(){
  smoothMove(10, 70, 5);
  delay(10);
  smoothMove(9, 90, 5); //120
  delay(10);
  smoothMove(10, 110, 5);
  delay(10);
  smoothMove(9, 165, 5); //120
  delay(10);
}

void moveleft(){
  smoothMove(10, 70, 5);  delay(10);
  smoothMove(9, 165, 5);  delay(10);
  smoothMove(10, 110, 5);  delay(10);
  smoothMove(9, 90, 5);   delay(10);
}

void emote(){
 smoothMove(1, 140, 4);
  delay(20);
  for(int i =0 ; i<3; i++){
  smoothMove(0, 140, 5);
  delay(50);
  smoothMove(0, 100, 5);
  delay(50);
  }

  smoothMove(1, 80, 4);
  delay(20);
}

// --- TACTICAL WEB INTERFACE ---
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>S.P.I.D.E.R. TACTICAL OS</title>
<style>
:root { 
  --neon-blue: #00d2ff; 
  --neon-red: #ff0055; 
  --bg-dark: #0a0b10; 
}
body { 
  font-family: 'Courier New', monospace; 
  background: var(--bg-dark); 
  color: white; 
  margin: 0; 
  display: flex; 
  flex-direction: column; 
  align-items: center; 
  justify-content: center; 
  height: 100vh; 
  overflow: hidden; 
}
.header { 
  text-align: center; 
  border-bottom: 2px solid var(--neon-blue); 
  width: 100%; 
  padding: 10px 0; 
  box-shadow: 0 0 15px var(--neon-blue); 
  margin-bottom: 20px; 
}
.d-pad { 
  display: grid; 
  grid-template-columns: repeat(3, 80px); 
  gap: 15px; 
}
.btn { 
  width: 80px; 
  height: 80px; 
  background: rgba(0, 210, 255, 0.1); 
  border: 2px solid var(--neon-blue); 
  color: var(--neon-blue); 
  border-radius: 15px; 
  font-size: 24px; 
  cursor: pointer; 
  transition: 0.2s; 
  display: flex; 
  align-items: center; 
  justify-content: center; 
  user-select: none;
}
.btn:active, .btn.active { 
  background: var(--neon-blue); 
  color: black; 
  box-shadow: 0 0 20px var(--neon-blue); 
  transform: scale(0.9); 
}
.btn-red { 
  border-color: var(--neon-red); 
  color: var(--neon-red); 
}
.btn-red:active { 
  background: var(--neon-red); 
  color: white; 
  box-shadow: 0 0 20px var(--neon-red); 
}
.console { 
  width: 90%; 
  max-width: 400px; 
  background: black; 
  border: 1px solid #333; 
  padding: 10px; 
  border-radius: 5px; 
  font-size: 11px; 
  height: 80px; 
  margin-top: 20px; 
  overflow: hidden; 
}
.line { 
  color: #00ff00; 
  border-left: 3px solid #00ff00; 
  padding-left: 8px; 
  margin-bottom: 4px; 
}
</style>
</head>
<body>
<div class="header">
  <h1>S.P.I.D.E.R. OPS</h1>
</div>
<div class="d-pad">
  <div></div>
  <button class="btn" id="W" onclick="cmd('forward')">&#9650;</button>
  <div></div>
  <button class="btn" id="A" onclick="cmd('left')">&#9664;</button>
  <button class="btn btn-red" id="H" onclick="cmd('init')">H</button>
  <button class="btn" id="D" onclick="cmd('right')">&#9654;</button>
  <div></div>
  <button class="btn" id="S" onclick="cmd('backward')">&#9660;</button>
  <button class="btn btn-red" style="font-size:12px" onclick="cmd('emote')">&#10024;</button>
</div>
<div class="console" id="log">
  <div class="line">&gt; SYSTEM READY</div>
</div>
<script>
function cmd(c) {
  const l = document.getElementById('log');
  const n = document.createElement('div'); 
  n.className='line'; 
  n.textContent='> EXE: '+c.toUpperCase();
  l.prepend(n); 
  fetch('/'+c);
}
document.addEventListener('keydown', (e) => {
  const m = {'w':'W','ArrowUp':'W','s':'S','ArrowDown':'S','a':'A','ArrowLeft':'A','d':'D','ArrowRight':'D','h':'H'};
  if(m[e.key]) { 
    document.getElementById(m[e.key]).click(); 
    document.getElementById(m[e.key]).classList.add('active'); 
  }
});
document.addEventListener('keyup', (e) => {
  const m = {'w':'W','ArrowUp':'W','s':'S','ArrowDown':'S','a':'A','ArrowLeft':'A','d':'D','ArrowRight':'D','h':'H'};
  if(m[e.key]) document.getElementById(m[e.key]).classList.remove('active');
});
</script>
</body>
</html>
)rawliteral";

// --- SERVER HANDLERS ---
void handleRoot() { 
  server.send_P(200, "text/html", htmlPage); 
}

void handleForward() { 
  moveforward(); 
  server.send(200, "text/plain", "OK"); 
}

void handleBackward() { 
  movebackward(); 
  server.send(200, "text/plain", "OK"); 
}

void handleLeft() { 
  moveleft(); 
  server.send(200, "text/plain", "OK"); 
}

void handleRight() { 
  moveright(); 
  server.send(200, "text/plain", "OK"); 
}

void handleInit() { 
  intializepos(); 
  server.send(200, "text/plain", "OK"); 
}

void handleEmote() { 
  emote(); 
  server.send(200, "text/plain", "OK"); 
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  pwm.begin(); 
  pwm.setPWMFreq(SERVO_FREQ);
  
  intializepos();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());
  
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/init", handleInit);
  server.on("/emote", handleEmote);
  server.begin();
}

void loop() { 
  server.handleClient(); 
}
