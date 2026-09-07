#define X_PIN 35
#define Y_PIN 34

void setup(){
  Serial.begin(115200);
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
}

void loop(){
  Serial.printf("x: %d, y: %d\r\n", analogRead(X_PIN), analogRead(Y_PIN));
  delay(100);
}
