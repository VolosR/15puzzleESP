#include "logo.h"
#include "wellDone.h"
#define LGFX_USE_V1

#include <LovyanGFX.hpp>
#include <SPI.h>

#define I2C_SCL 39
#define I2C_SDA 38

#define LCD_CS 37
#define LCD_BLK 45
#define N 4

//#define NS2009_TOUCH  //Resistive screen driver
#define FT6236_TOUCH //Capacitive screen driver

#ifdef NS2009_TOUCH
#include "NS2009.h"

const int i2c_touch_addr = NS2009_ADDR;
#define get_pos ns2009_pos
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#define get_pos ft6236_pos
#endif

class LGFX : public lgfx::LGFX_Device
{
    //lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_Parallel16 _bus_instance; // 8ビットパラレルバスのインスタンス (ESP32のみ)

public:
    // コンストラクタを作成し、ここで各種設定を行います。
    // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
    LGFX(void)
    {
        {                                      // バス制御の設定を行います。
            auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

            // 16位设置
            cfg.i2s_port = I2S_NUM_0;  // 使用するI2Sポートを選択 (0 or 1) (ESP32のI2S LCDモードを使用します)
            cfg.freq_write = 16000000; // 送信クロック (最大20MHz, 80MHzを整数で割った値に丸められます)
            cfg.pin_wr = 35;           // WR を接続しているピン番号
            cfg.pin_rd = 34;           // RD を接続しているピン番号
            cfg.pin_rs = 36;           // RS(D/C)を接続しているピン番号

            cfg.pin_d0 = 33;
            cfg.pin_d1 = 21;
            cfg.pin_d2 = 14;
            cfg.pin_d3 = 13;
            cfg.pin_d4 = 12;
            cfg.pin_d5 = 11;
            cfg.pin_d6 = 10;
            cfg.pin_d7 = 9;
            cfg.pin_d8 = 3;
            cfg.pin_d9 = 8;
            cfg.pin_d10 = 16;
            cfg.pin_d11 = 15;
            cfg.pin_d12 = 7;
            cfg.pin_d13 = 6;
            cfg.pin_d14 = 5;
            cfg.pin_d15 = 4;

            _bus_instance.config(cfg);              // 設定値をバスに反映します。
            _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
        }

        {                                        // 表示パネル制御の設定を行います。
            auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

            cfg.pin_cs = -1;   // CS要拉低
            cfg.pin_rst = -1;  // RST和开发板RST相连
            cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

            // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

            cfg.memory_width = 320;   // ドライバICがサポートしている最大の幅
            cfg.memory_height = 480;  // ドライバICがサポートしている最大の高さ
            cfg.panel_width = 320;    // 実際に表示可能な幅
            cfg.panel_height = 480;   // 実際に表示可能な高さ
            cfg.offset_x = 0;         // パネルのX方向オフセット量
            cfg.offset_y = 0;         // パネルのY方向オフセット量
            cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
            cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
            cfg.dummy_read_bits = 1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable = true;      // データ読出しが可能な場合 trueに設定
            cfg.invert = false;       // パネルの明暗が反転してしまう場合 trueに設定
            cfg.rgb_order = false;    // パネルの赤と青が入れ替わってしまう場合 trueに設定
            cfg.dlen_16bit = true;    // データ長を16bit単位で送信するパネルの場合 trueに設定
            cfg.bus_shared = true;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};

LGFX lcd;
LGFX_Sprite sprite(&lcd);
LGFX_Sprite ani(&lcd);

int box=70;
int posX[4];
int posY[4];

int taken[4][4]={{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}};

int numbers[4][4]={{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,0,14,15}};
int numbersFinish[4][4]={{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,0}};
int numbersTaken[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
int numbersTakenS[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
int random1=0;
int random2=0;
int temp=0;


bool finish=0;
bool search=0;
int n=0;

int pos[2] = {0, 0};
int left=20;
int top=140;
bool finded=0;
int moves=0;
int sec=0;

#define color2  0x1353
#define color1  0x0A4E
#define silver 0xD69A
#define orange 0xFDEB
#define lightBlue 0x665B

void setup(void)
{
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    Serial.begin(115200);
    lcd.init();
   
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error, address;

    sprite.createSprite(280, 280);
    
    sprite.setTextDatum(4);
    sprite.setSwapBytes(true);
    sprite.setFreeFont(&Orbitron_Light_24);
    
    ani.createSprite(200,40);
    ani.setTextColor(0xD9BA,TFT_BLACK);
    ani.setFreeFont(&Orbitron_Light_24);

    Wire.beginTransmission(i2c_touch_addr);
    error = Wire.endTransmission();
    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(i2c_touch_addr, HEX);
        Serial.println("  !");
    }
    else if (error == 4)
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(i2c_touch_addr, HEX);
    }

    for(int i=0;i<4;i++)
    {
      posX[i]=i*box;
      posY[i]=posX[i];
    }

   gameStart();
}

long currentTime=0;

void loop()
{
 if (get_pos(pos))
    {
        
        int x=pos[0]-left;
        int y=pos[1]-top;
        //lcd.fillRoundRect(left,top-90,120,76,5,orange);
        if(pos[0]>left && pos[0]<140 && pos[1]>top-90 && pos[1]<top-90+76)
        gameStart();
        
         if(finish==0)
         for(int i=0;i<4;i++)
         for(int j=0;j<4;j++)
          { 
            
            if(x>posX[j] && x<posX[j]+64 && y>posX[i] && y<posX[i]+64 )
            {
             
             
              if(j>0)
              if(finded==0)
              if(taken[i][j-1]==0){
              finded=1;   
              taken[i][j]=0;
              taken[i][j-1]=1;
              numbers[i][j-1]=numbers[i][j];
              numbers[i][j]=0;}

              if(j<3)
              if(finded==0)
              if(taken[i][j+1]==0){
                finded=1;  
              taken[i][j]=0;
              taken[i][j+1]=1;
              numbers[i][j+1]=numbers[i][j];
              numbers[i][j]=0;
             }

             if(i<3)
              if(finded==0)
              if(taken[i+1][j]==0){
                finded=1;  
              taken[i][j]=0;
              taken[i+1][j]=1;
              numbers[i+1][j]=numbers[i][j];
              numbers[i][j]=0;
             }

             if(i>0)
              if(finded==0)
              if(taken[i-1][j]==0){
                finded=1;  
              taken[i][j]=0;
              taken[i-1][j]=1;
              numbers[i-1][j]=numbers[i][j];
              numbers[i][j]=0;

              
             }

              if(finded==1){
                 moves++;
               lcd.drawString("moves: "+String(moves),160,102);}
             
             }
            }
            
          }

    
    finded=0;
    checkFinish();
    

    if(finish==0)
    if(currentTime+1000<millis())
    {
      currentTime=millis();
      sec++;
      lcd.drawString("time: "+String(sec),160,60);
      }
    
 drawSprite(); 
 drawAnimation(); 
}

void drawSprite()
{

  sprite.setTextColor(TFT_WHITE,color2);
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextDatum(1);
  if(finish==1)
  {
    sprite.fillRoundRect(0,0,280,280,8,0xE6AE);
    sprite.setTextColor(TFT_PURPLE,0xE6AE);
    sprite.pushImage(92,60,96,96,wellDone);
    sprite.drawString("Well Done!",140,20);
    sprite.drawString("Moves: "+String(moves),140,160);
    sprite.drawString("time: "+String(sec),140,190);
  }
  

  if(finish==0)
  for(int j=0;j<4;j++)
  for(int i=0;i<4;i++)
if(taken[j][i]==1){
sprite.fillRoundRect(posX[i],posY[j],box-4,box-4,5,color1);
sprite.fillCircle(posX[i]+33,posY[j]+33,24,color2);
sprite.drawString(String(numbers[j][i]),posX[i]+32,posY[j]+18);
if(numbers[j][i]==numbersFinish[j][i])
sprite.fillCircle(posX[i]+14,posY[j]+14,4,silver);
}
else{
sprite.fillRoundRect(posX[i],posY[j],box-4,box-4,5,TFT_BLACK);

}

sprite.pushSprite(left,top);
  }

 void checkFinish()
 {
  finish=1;
  for(int i=0;i<4;i++)
  for(int j=0;j<4;j++)
  if(finish==1)
  if(numbers[i][j]!=numbersFinish[i][j])
  finish=0;
  }

  void shufle()
  {
     for(int i=0;i<16;i++)
     numbersTaken[i]=numbersTakenS[i];

     for(int i=0;i<4;i++)
     for(int j=0;j<4;j++)
     taken[i][j]=1;
        
     
     for(int s=0;s<200;s++)
    {
     random1=random(0,16); 
     random2=random(0,16); 
     temp=numbersTaken[random1];
     numbersTaken[random1]=numbersTaken[random2];
     numbersTaken[random2]=temp;
     
     }
         
       int r=0;
         for(int i=0;i<4;i++)
         for(int j=0;j<4;j++) 
         {
          numbers[i][j]=numbersTaken[r];
         
          if(numbersTaken[r]==0)
          taken[i][j]=0;
           r++;
      }   
          
    
    }
   float animationX=1;
   float change=0.34;
    
    void drawAnimation()
    {
       
      ani.fillSprite(TFT_BLACK);
      animationX=animationX+change;
      if(animationX>60 || animationX<1)
      change=change*-1;
      ani.drawString("15 Puzzle",(int)animationX,4);
      ani.pushSprite(20,6);
    }

    void gameStart()
    {
      sec=0;
      moves=0;
      finish=0;
    lcd.fillScreen(TFT_BLACK);  
    lcd.setFont(0);
    lcd.setTextColor(0xD69A,TFT_BLACK);  
    lcd.drawString("ESP32S2",248,18);
    lcd.drawString("MAKERFABS",235,30);
    
    lcd.drawString("board by makerfabs.com",22,438,2);
    lcd.setTextColor(orange,TFT_BLACK);
    lcd.drawString("code by VolosProjects",22,454,2);
    lcd.setSwapBytes(true);
    lcd.pushImage(240,430,50,45,logo);
    lcd.fillRoundRect(left,top-90,120,76,5,orange);
    lcd.fillRoundRect(150,top-90,146,35,5,silver);
    lcd.fillRoundRect(150,top-48,146,35,5,silver);
    
    

    lcd.setFreeFont(&Orbitron_Light_24);
    
    
    lcd.setTextColor(TFT_PURPLE,orange);
    lcd.drawString("NEW",48,top-80);
    lcd.drawString("GAME",40,top-54);

    lcd.setFreeFont(&FreeSansBold9pt7b);
    lcd.setTextColor(color1,silver);
    lcd.drawString("moves: "+String(moves),160,102);

    shufle();
    while(isSolvable(numbers)==0)
    {shufle();}
    
      }

   int getInvCount(int arr[])
{
    int inv_count = 0;
    for (int i = 0; i < N * N - 1; i++)
    {
        for (int j = i + 1; j < N * N; j++)
        {
            // count pairs(arr[i], arr[j]) such that
              // i < j but arr[i] > arr[j]
            if (arr[j] && arr[i] && arr[i] > arr[j])
                inv_count++;
        }
    }
    return inv_count;
}
 
// find Position of blank from bottom
int findXPosition(int puzzle[N][N])
{
    // start from bottom-right corner of matrix
    for (int i = N - 1; i >= 0; i--)
        for (int j = N - 1; j >= 0; j--)
            if (puzzle[i][j] == 0)
                return N - i;
}
 
// This function returns true if given
// instance of N*N - 1 puzzle is solvable
bool isSolvable(int puzzle[N][N])
{
    // Count inversions in given puzzle
    int invCount = getInvCount((int*)puzzle);
 
    // If grid is odd, return true if inversion
    // count is even.
    if (N & 1)
        return !(invCount & 1);
 
    else     // grid is even
    {
        int pos = findXPosition(puzzle);
        if (pos & 1)
            return !(invCount & 1);
        else
            return invCount & 1;
    }
}   
