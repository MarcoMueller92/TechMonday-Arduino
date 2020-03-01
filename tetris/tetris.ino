
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#define SHAPE_I 0
#define SHAPE_J 1
#define SHAPE_L 2
#define SHAPE_S 3
#define SHAPE_Z 4
#define SHAPE_O 5
#define SHAPE_T 6
#define SHAPE_NUM 7

#define SHAPE_HEIGHT 2
#define SHAPE_WIDTH 4
#define SHAPE_CENTER_I 0
#define SHAPE_CENTER_J 1
#define SHAPE_BLOCKS 4

#define ROT_0 0
#define ROT_90 1
#define ROT_180 2
#define ROT_270 3
#define ROT_NUM 4

#define HARDDOWN 0
#define LEFT 1
#define RIGHT 2
#define ROTATE 3
#define DOWN 4
#define INPUT_NUM 5
#define BUTTON_NUM 4 // DOWN not yet implemented. INPUT = BUTTON
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 5
#define BUTTON_ROTATE 4
#define BUTTON_HARDDOWN 3

#define GRID_WIDTH 10
#define GRID_HEIGHT 22
#define GRID_OVERHANG 2
#define SPAWN_X 4
#define SPAWN_Y 19

#define PORT_OFFSET_X 5
#define PORT_OFFSET_Y 10
#define PORT_WIDTH 70
#define PORT_HEIGHT 140
#define BLOCK_SIZE 7
#define SCORE_OFFSET_X 85
#define SCORE_OFFSET_Y 100

#define BLACK   ST77XX_BLACK
#define WHITE   ST77XX_WHITE
#define BLUE    ST77XX_BLUE
#define RED     ST77XX_RED
#define GREEN   ST77XX_GREEN
#define YELLOW  ST77XX_YELLOW
#define CYAN    ST77XX_CYAN
#define MAGENTA ST77XX_MAGENTA
#define ORANGE  ST77XX_ORANGE

#define TFT_CS        10
#define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         9

#define LOOP_TIME 50
#define STEP_TIME 500
#define BUTTON_TIMEOUT 100


//// Globals.

// GFX.
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int screen_buffer[GRID_WIDTH][GRID_HEIGHT];
int screen[GRID_WIDTH][GRID_HEIGHT];
bool screen_update;
bool stats_update;


// Tetrominos definition.
const int shape_color[SHAPE_NUM] = {CYAN,BLUE,ORANGE,GREEN,RED,YELLOW,MAGENTA};
const int shape_form[SHAPE_NUM][SHAPE_BLOCKS] = {
  {0,1,2,3}, // I
  {0,1,2,6}, // J
  {0,1,2,4}, // L
  {1,2,4,5}, // S
  {0,1,5,6}, // Z
  {1,2,5,6}, // O
  {0,1,2,5}  // T
};


// Tetromino state.
int tetromino_rotation;
int tetromino_shape;
int tetromino_x;
int tetromino_y;


// Grid state.
bool grid[GRID_WIDTH][GRID_HEIGHT];


// Timing
long last_loop;
long last_step;

// Inputs.
const int input_pins[BUTTON_NUM] = {BUTTON_HARDDOWN,BUTTON_LEFT,BUTTON_RIGHT,BUTTON_ROTATE};
long last_fired[BUTTON_NUM];
bool last_input[BUTTON_NUM];


// Game state.
const int points[SHAPE_BLOCKS] = {40,100,300,1200};
int playing;
int bag[SHAPE_NUM];
int next_shape;
long score;
int level;


//// Main.

void setup() {
  Serial.begin(9600);
  initInput();
  initRandom();
  initScreen();
  initGrid();
  spawn();
  last_loop = millis();
  last_step = millis();
  playing = true;
  score = 0;
  level = 0;
  delay(LOOP_TIME);
}

void loop() {

  // Halt on game over.
  if (!playing) {
    delay(200);
    return;
  }

  // Process user input.
  bool input[INPUT_NUM];
  readInputs(input);
  if (input[HARDDOWN]) applyAction(HARDDOWN);
  else {
    if (input[LEFT] & !input[RIGHT]) applyAction(LEFT);
    else if (input[RIGHT] & !input[LEFT]) applyAction(RIGHT);
    if (input[ROTATE]) applyAction(ROTATE);
    if (input[DOWN]) applyAction(DOWN);
  }

  // Apply gravity.
  long elapsed = millis()-last_step;
  if (elapsed>STEP_TIME) {
    last_step = millis()-(elapsed-STEP_TIME);
    if (!applyAction(DOWN)) {
      nextTetromino();
      if (detectCollision(tetromino_x,tetromino_y,tetromino_rotation,tetromino_shape)) {
        gameOver();
        playing = false;
        return;
      }
    }
  }

  // Copy data to TFT.
  updateScreen();

  // Sync to desired update rate.
  elapsed = millis()-last_loop;
  if (elapsed<LOOP_TIME) delay(LOOP_TIME-elapsed);
  last_loop = millis();
}




//// Input

void initInput() {
  long now = millis();
  for (int i=0;i<BUTTON_NUM;i++) {
    pinMode(input_pins[i],INPUT_PULLUP);
    last_input[i] = false;
    last_fired[i] = now - BUTTON_TIMEOUT; // If you delete this it crashes...
  }
}

void readInputs(bool inputs[]) {
  long now = millis();
  for (int i=0;i<BUTTON_NUM;i++) {
    bool in = !digitalRead(input_pins[i]);
    switch(i) {
      case LEFT:
      case RIGHT:
        inputs[i] = in && (last_input[i] ? now-last_fired[i]>BUTTON_TIMEOUT : true);
        break;
      case HARDDOWN:
      case ROTATE:
        inputs[i] = in && !last_input[i];
        break;
    }
    if (inputs[i]) last_fired[i] = now;
    last_input[i] = in;
  }
  inputs[DOWN] = false;
}


//// Controller.

void nextTetromino() {
  placeTetromino();
  clearRows();
  spawn();
}

void placeTetromino() {
  for (int b=0;b<SHAPE_BLOCKS;b++) {
    int x = tetromino_x, y = tetromino_y;
    computeCoords(&x,&y,tetromino_rotation,tetromino_shape,b);
    grid[x][y] = true;
  }
}

void clearRows() {
  bool full[GRID_HEIGHT];
  int rows_cleared = GRID_HEIGHT;
  for (int y=0;y<GRID_HEIGHT;y++) {
    full[y] = true;
    for (int x=0;x<GRID_WIDTH;x++) if (!grid[x][y]) {
      full[y] = false;
      rows_cleared--;
      break;
    }
  }
  if (rows_cleared==0) return;
  else {
    score += (level+1) * points[rows_cleared-1];
    stats_update = true;
  }
  int rows = 0;
  for (int y=0;y<GRID_HEIGHT;y++) {
    while (full[y+rows]) rows++;
    if (rows==0) continue;
    int src = y+rows;
    if (src>=GRID_HEIGHT) for (int x=0;x<GRID_WIDTH;x++) {grid[x][y] = false;screen[x][y] = BLACK;}
    else for (int x=0;x<GRID_WIDTH;x++) {grid[x][y] = grid[x][src];screen[x][y] = screen[x][src];}
  }
  screen_update = true;
}

void spawn() {
  tetromino_shape = bag[next_shape++];
  if (next_shape==SHAPE_NUM) resetBag();
  tetromino_rotation = ROT_0;
  tetromino_x = SPAWN_X;
  tetromino_y = SPAWN_Y;
  renderTetromino(true);
}

bool applyAction(int action) {
  int x = tetromino_x, y = tetromino_y, r = tetromino_rotation;
  switch(action) {
    case HARDDOWN:
      for (y--;y>=0;y--) if (detectCollision(x,y,r,tetromino_shape)) break;
      y++;
      break;
    case DOWN:
      y--;
    case LEFT:
    case RIGHT:
      x += action==RIGHT ? 1 : (action==LEFT ? -1 : 0);
      if (detectCollision(x,y,r,tetromino_shape)) return false;
      break;
    case ROTATE:
      if (tetromino_shape==SHAPE_O) return true;
      r++;
      r %= ROT_NUM;
      if (detectCollision(x,y,r,tetromino_shape)) {
        x--;
        if (detectCollision(x,y,r,tetromino_shape)) {
          x+=2;
          if (detectCollision(x,y,r,tetromino_shape)) {
            if (tetromino_shape!=SHAPE_I) return false;
            x-=3;
            if (detectCollision(x,y,r,tetromino_shape)) {
              x+=4;
              if (detectCollision(x,y,r,tetromino_shape)) return false;
            }
          }
        }
      }
      break;
  }
  renderTetromino(false);
  tetromino_x = x;
  tetromino_y = y;
  tetromino_rotation = r;
  renderTetromino(true);
  if (action==HARDDOWN) nextTetromino();
  return true;
}


//// Model.

void computeCoords(int *x, int *y, int rotation, int shape, int block) {
  int j = shape_form[shape][block] % SHAPE_WIDTH;
  int i = (shape_form[shape][block]-j)/SHAPE_WIDTH;
  int dx = j-SHAPE_CENTER_J;
  int dy = -(i-SHAPE_CENTER_I);
  if (rotation%ROT_180==ROT_90) {
    int temp = dx;
    dx = -dy;
    dy = temp;
  }
  if (rotation>=ROT_180) {
    dx = -dx;
    dy = -dy;
  }
  *x+=dx;
  *y+=dy;
}

bool detectCollision(int x_center, int y_center, int rotation, int shape) {
  for (int b=0;b<SHAPE_BLOCKS;b++) {
    int x = x_center, y = y_center;
    computeCoords(&x,&y,rotation,shape,b);
    if (x<0||x>=GRID_WIDTH||y<0||y>=GRID_HEIGHT||grid[x][y]) return true;
  }
  return false;
}

void initRandom() {
  randomSeed(analogRead(A2));
  resetBag();
}

void resetBag() {
  next_shape = 0;
  for (int i=0;i<SHAPE_NUM;i++) bag[i] = i;
  for (int i=0;i<SHAPE_NUM;i++) {
    int j = random(0,SHAPE_NUM);
    int s = bag[i];
    bag[i] = bag[j];
    bag[j] = s;
  }
}

void initGrid() {
  memset(grid, false, sizeof(screen_buffer[0][0])*GRID_WIDTH*GRID_HEIGHT);
}


//// Rendering.

void renderTetromino(bool on) {
  int color = on ? shape_color[tetromino_shape] : BLACK;
  renderShape(tetromino_x,tetromino_y,tetromino_rotation,tetromino_shape,color);
}

void renderShape(int x_center, int y_center, int rotation, int shape, int color) {
  for (int b=0;b<SHAPE_BLOCKS;b++) {
    int j = shape_form[shape][b] % SHAPE_WIDTH;
    int i = (shape_form[shape][b]-j)/SHAPE_WIDTH;
    int x = x_center, y = y_center;
    computeCoords(&x,&y,rotation,shape,b);
    if (x<0||x>=GRID_WIDTH||y<0||y>=GRID_HEIGHT) continue;
    screenWrite(x,y,color);
  }
}


//// GFX.

void initScreen() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(BLACK);
  tft.drawRect(PORT_OFFSET_X-1,PORT_OFFSET_Y-1,PORT_WIDTH+2,PORT_HEIGHT+2,WHITE);
  int sz = sizeof(screen_buffer[0][0])*GRID_WIDTH*GRID_HEIGHT;
  memset(screen_buffer, BLACK, sz);
  memset(screen, BLACK, sz);
  screen_update = false;
  stats_update = true;
}

void updateScreen() {
  if (!screen_update & !stats_update) return;
  for (int x=0;x<GRID_WIDTH;x++)
    for (int y=0;y<GRID_HEIGHT;y++) {
      int color = screen[x][y];
      if (screen_buffer[x][y]==color) continue;
      screen_buffer[x][y] = color;
      if (y>=GRID_HEIGHT-GRID_OVERHANG) continue;
      tft.fillRect(PORT_OFFSET_X+x*BLOCK_SIZE,PORT_OFFSET_Y+PORT_HEIGHT-(y+1)*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,color);
    }

  if (stats_update) {
    tft.fillRect(SCORE_OFFSET_X,SCORE_OFFSET_Y,50,50,BLACK);
    tft.setTextColor(WHITE);
    tft.setTextWrap(false);
    tft.setTextSize(1);
    
    tft.setCursor(SCORE_OFFSET_X, SCORE_OFFSET_Y);
    tft.print("Score:");
    tft.setCursor(SCORE_OFFSET_X, SCORE_OFFSET_Y+10);
    tft.print(score);
    tft.setCursor(SCORE_OFFSET_X,SCORE_OFFSET_Y+25);
    tft.print("Level:");
    tft.setCursor(SCORE_OFFSET_X,SCORE_OFFSET_Y+35);
    tft.print(level);
  }
  
  screen_update = false;
  stats_update = false;
}

void screenWrite(int x, int y, int color) {
  screen[x][y] = color;
  screen_update = true;
}

void gameOver() {
  updateScreen();
  tft.setCursor(20, 30);
  tft.setTextColor(RED);
  tft.setTextWrap(true);
  tft.setTextSize(4);
  tft.println("Game Over");
}
