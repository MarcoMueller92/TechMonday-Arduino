// Define the Pin Number used for the digital read
#define button_pin_right 3
#define button_pin_left 6
#define button_pin_drop 4
#define button_pin_turn 5

int bool_sum;

void setup() {
  // Necessary for Serial Communication with Console
  Serial.begin(9600);
  
  // Tell the Pins that you want it to measure digital inputs
  for (int i = button_pin_right; i<button_pin_left+1; i=i+1) {
    pinMode(i, INPUT_PULLUP);
    }
}

void loop() {
  // Read the button input which is 0 when the button is clicked and 1 when not.

  bool button_right = !digitalRead(button_pin_right);
  bool button_left = !digitalRead(button_pin_left);
  bool button_drop = !digitalRead(button_pin_drop);
  bool button_turn = !digitalRead(button_pin_turn);

  bool_sum = button_right + button_left + button_drop + button_turn;

  if ( bool_sum > 1 ) {
    Serial.println("no button mashing");}
  else if (button_right == 1) {
    Serial.println("right");}
  else if (button_left == 1) {
    Serial.println("left");}
  else if (button_drop == 1) {
    Serial.println("drop");}
  else if (button_turn == 1) {
    Serial.println("turn");}
  else{
    Serial.println("nothing cought");}

    /*
  Serial.println("button_right: " + String(button_right));
  delay(1000);
  Serial.println("button_left: " + String(button_left));
  delay(1000);
  Serial.println("button_drop: " + String(button_drop));
  delay(1000);
  Serial.println("button_turn: " + String(button_turn));
  delay(1000);
*/
}
