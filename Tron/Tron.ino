#include <Elegoo_TFTLCD.h>
#include <Elegoo_GFX.h>

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define PLAYER1_RIGHT 23
#define PLAYER1_LEFT 31
#define PLAYER2_RIGHT 45
#define PLAYER2_LEFT 53

Elegoo_TFTLCD tft(A3, A2, A1, A0, A4);

const int SCREEN_HEIGHT = 320/2;
const int SCREEN_WIDTH = 240/2;

const int ARENA_HEIGHT = 40;
const int ARENA_WIDTH = 30;

uint16_t arena[ARENA_HEIGHT][ARENA_WIDTH];

boolean gameIsOn = true;

boolean player1_right_on = false;
boolean player1_left_on = false;
boolean player2_right_on = false;
boolean player2_left_on = false;

struct vector{
  int x;
  int y;
};

struct player{
  struct vector pos;
  struct vector dir;
  int color;
};

struct player player1;
struct player player2;

unsigned long lastMillis;

uint16_t getOffset(int x, int y){
  int modx = x%4;
  int mody = y%4;
  if(modx==0 && mody==0){
    return 0b1000000000000000;
  } else if (modx==0 && mody==1){
    return 0b0100000000000000;
  } else if (modx==0 && mody==2){
    return 0b0010000000000000;
  } else if (modx==0 && mody==3){
    return 0b0001000000000000;
  } else if (modx==1 && mody==0){
    return 0b0000100000000000;
  } else if (modx==1 && mody==1){
    return 0b0000010000000000;
  } else if (modx==1 && mody==2){
    return 0b0000001000000000;
  } else if (modx==1 && mody==3){
    return 0b0000000100000000;
  } else if (modx==2 && mody==0){
    return 0b0000000010000000;
  } else if (modx==2 && mody==1){
    return 0b0000000001000000;
  } else if (modx==2 && mody==2){
    return 0b0000000000100000;
  } else if (modx==2 && mody==3){
    return 0b0000000000010000;
  } else if (modx==3 && mody==0){
    return 0b0000000000001000;
  } else if (modx==3 && mody==1){
    return 0b0000000000000100;
  } else if (modx==3 && mody==2){
    return 0b0000000000000010;
  } else if (modx==3 && mody==3){
    return 0b0000000000000001;
  }
}

void drawPixel(int x, int y, int color){
  if(x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT){
    Serial.println("Out of bound error");
    return;  
  }

  //tft.drawFastHLine(x*2,y*2,2,color);
  //tft.drawFastHLine(x*2,y*2+1,2,color);
  tft.drawPixel(x*2,y*2,color);
  tft.drawPixel(x*2+1,y*2,color);
  tft.drawPixel(x*2,y*2+1,color);
  tft.drawPixel(x*2+1,y*2+1,color);
  
  uint16_t val = getOffset(x,y);
  arena[y/4][x/4] = arena[y/4][x/4] | val;
}

boolean isFilled(int x, int y){
  if(x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT){
    Serial.println("Out of bound error");
    return 0;  
  }
  uint16_t val = getOffset(x,y);
  //Serial.println(val);
  //Serial.println(arena[y/2][x/2]);
  //Serial.println(val);

  if(  (arena[y/4][x/4] & val) > 0 )
    return true;
  else
    return false;
}

void drawArena(){
  for(int y = 0; y < SCREEN_HEIGHT; y++){
    drawPixel(0,y,WHITE);
  }
  for(int y = 0; y < SCREEN_HEIGHT; y++){
    drawPixel(SCREEN_WIDTH-1,y,WHITE);
  }
  for(int x = 0; x < SCREEN_WIDTH; x++){
    drawPixel(x,0,WHITE);
  }
  for(int x = 0; x < SCREEN_WIDTH; x++){
    drawPixel(x,SCREEN_HEIGHT-1,WHITE);
  } 
}

void updateScreen(){
  drawPixel(player1.pos.x, player1.pos.y, player1.color);
  drawPixel(player2.pos.x, player2.pos.y, player2.color);
}

void updatePlayer(){
  player1.pos.x += player1.dir.x;
  player1.pos.y += player1.dir.y;
  player2.pos.x += player2.dir.x;
  player2.pos.y += player2.dir.y;

  if(isFilled(player1.pos.x, player1.pos.y) || isFilled(player2.pos.x, player2.pos.y)){
    gameIsOn = false; 
  }
}

struct vector turnRight(struct player player){
  if(player.dir.x == 1 && player.dir.y == 0){
    Serial.println(1);
    return {0,1};
  } else if(player.dir.x == -1 && player.dir.y == 0){
    Serial.println(2);
    return {0,-1};
  } else if(player.dir.x == 0 && player.dir.y == 1){
    Serial.println(3);
    return {-1,0};
  } else if(player.dir.x == 0 && player.dir.y == -1){
    Serial.println(4);
    return {1,0};
  }
}

struct vector turnLeft(struct player player){
  if(player.dir.x == 1 && player.dir.y == 0){
    return {0,-1};
  } else if(player.dir.x == -1 && player.dir.y == 0){
    return {0,1};
  } else if(player.dir.x == 0 && player.dir.y == 1){
    return {1,0};
  } else if(player.dir.x == 0 && player.dir.y == -1){
    return {-1,0};
  }
}

void updateDirection(){
  int player1_turn_right = digitalRead(PLAYER1_RIGHT);
  int player1_turn_left = digitalRead(PLAYER1_LEFT);
  int player2_turn_right = digitalRead(PLAYER2_RIGHT);
  int player2_turn_left = digitalRead(PLAYER2_LEFT);

  if(!player1_turn_right && !player1_right_on){
    Serial.println("TURN RIGHT PLAYER 1");
    player1.dir = turnRight(player1);
    player1_right_on = true;
  } else if(!player1_turn_left && !player1_left_on){
    player1.dir = turnLeft(player1);
    player1_left_on = true;
  } 

  if(!player2_turn_right && !player2_right_on){
    player2.dir = turnRight(player2);
    player2_right_on = true;
  } else if(!player2_turn_left && !player2_left_on){
    player2.dir = turnLeft(player2);
    player2_left_on = true;
  }

  if(player1_turn_right && player1_right_on)
    player1_right_on = false;
  if(player1_turn_left && player1_left_on)
    player1_left_on = false;
  if(player2_turn_right && player2_right_on)
    player2_right_on = false;
  if(player2_turn_left && player2_left_on)
    player2_left_on = false;
}


void setup() {
  Serial.begin(9600);
  
  tft.reset();
  tft.begin(0x9341);
  tft.fillScreen(BLACK);

  pinMode(PLAYER1_RIGHT, INPUT_PULLUP);
  pinMode(PLAYER1_LEFT, INPUT_PULLUP);
  pinMode(PLAYER2_RIGHT, INPUT_PULLUP);
  pinMode(PLAYER2_LEFT, INPUT_PULLUP);

  drawArena();

  player1.pos = {60,10};
  player1.dir = {0,1};
  player1.color = BLUE;
  player2.pos = {60,150};
  player2.color = RED;
  player2.dir = {0,-1};
  
  updateScreen();

  lastMillis = millis();
}

void loop() {
  if(gameIsOn){
    updateDirection();
    if(lastMillis + 30 < millis()){
      lastMillis = millis();
      updatePlayer();
      if(gameIsOn)
        updateScreen();
    }
  }
}
