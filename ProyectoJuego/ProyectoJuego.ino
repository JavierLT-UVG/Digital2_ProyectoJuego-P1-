//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include <SPI.h>
#include <SD.h>

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
#define SlaveSelect PA_3
#define MUSICA_1 PC_6
#define MUSICA_2 PC_7
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);


extern uint8_t pisoSprite[];
extern uint8_t kirbySarten[];
extern uint8_t kirbyOlla[];
extern uint8_t claudioSprite1[];
extern uint8_t claudioSprite2[];
extern uint8_t menuPierna1[];
extern uint8_t menuPierna2[];
extern uint8_t menuPierna3[];
extern uint8_t sopaDerrota[];

uint8_t animPierna;
uint8_t animRamiroSprite1;
uint8_t animRamiroSprite2;
uint8_t animChancla;
uint8_t animSarten;
uint8_t animOlla;
uint8_t estadoRamiro = 0;

int posRamiroY;
int posChanclaX = 0;
int posSartenX = 0;
int posOllaX = 0;

int control;
bool controlAvailable = true;
bool enableSubida = true;
bool enableCaida = false;

bool modoP1 = false;
bool modoP2 = false;
bool modoCreditos = false;

uint8_t opcionMenu = 0;
uint8_t pushButton;

unsigned long millisNuevo = 0;
unsigned long millisAnterior = 0;
unsigned int deltaMillis;
uint8_t enemyIndex;
bool colisionTest = false;

uint8_t uartRX;
uint8_t uartTX;

String text1 = "GAME OVER";

unsigned long globalCont;

File root;
File myFile;
File myFile2;

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  pinMode(PF_4, INPUT_PULLUP);

  pinMode(MUSICA_1, OUTPUT);
  pinMode(MUSICA_2, OUTPUT);
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);
  Serial5.begin(115200);
  SPI.setModule(0);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);

  // Fondo negro
  FillRect(0, 0, 320, 240, 0x0000);

  showFiles();

  // Colocar menú
  SD_Bitmap_Menu(0, 0, 320, 240);
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************

void loop() {
  digitalWrite(MUSICA_1, 1);
  digitalWrite(MUSICA_2, 0);
  
  control = analogRead(PE_0);
  pushButton = digitalRead(PF_4);

  switch(opcionMenu){
    case 0:
      // Mostrar sprite en modoP1
      animPierna = (globalCont / 150) % 2;
      LCD_Sprite(86, 115, 25, 12, menuPierna1, 2, animPierna, 0, 0);
      LCD_Sprite(86, 139, 25, 12, menuPierna2, 2, 1, 0, 0);
      LCD_Sprite(86, 171, 25, 12, menuPierna3, 2, 1, 0, 0);
      
      // Actualizar opción de menú depediendo del movimiento del control
      if(control<0+50){
        // Créditos
        opcionMenu = 1;
        delay(500);
      } else if(control<4095-50){
        // Mantenerse en modoP1
        opcionMenu = 0;
        // Si se presiona el botón, entrar al modo actual
        if(pushButton == 0){
          modoP1 = true;
        }
      } else{
        // Modo P2
        opcionMenu = 2;
        delay(500);
      }
    break;

    case 1:
      // Mostrar sprite en modoP2
      animPierna = (globalCont / 150) % 2;
      LCD_Sprite(86, 115, 25, 12, menuPierna1, 2, 1, 0, 0);
      LCD_Sprite(86, 139, 25, 12, menuPierna2, 2, animPierna, 0, 0);
      LCD_Sprite(86, 171, 25, 12, menuPierna3, 2, 1, 0, 0);
      
      // Actualizar opción de menú depediendo del movimiento del control
      if(control<0+50){
        // modoP1
        opcionMenu = 2;
        delay(500);
        
      } else if(control<4095-50){
        // Mantenerse en modoP2
        opcionMenu = 1;
        // Si se presiona el botón, entrar al modo actual
        if(pushButton == 0){
          modoP2 = true;
        }
      } else{
        // Créditos
        opcionMenu = 0;
        delay(500);
      }
    break;

    case 2:
      // Mostrar sprite en Créditos
      animPierna = (globalCont / 150) % 2;
      LCD_Sprite(86, 115, 25, 12, menuPierna1, 2, 1, 0, 0);
      LCD_Sprite(86, 139, 25, 12, menuPierna2, 2, 1, 0, 0);
      LCD_Sprite(86, 171, 25, 12, menuPierna3, 2, animPierna, 0, 0);
      
      // Actualizar opción de menú depediendo del movimiento del control
      if(control<0+50){
        // modoP2
        opcionMenu = 0;
        delay(500);
      } else if(control<4095-50){
        // Mantenerse en Créditos
        opcionMenu = 2;
        // Si se presiona el botón, entrar al modo actual
        if(pushButton == 0){
          modoCreditos = true;
        }
      } else{
        // modoP1
        opcionMenu = 1;
        delay(500);
      }
    break;
  }
  
  // Colocar mapas de fondo antes de que se bloquee en el while en modo 1 jugador
  if(modoP1 == 1){
    // Cielo
    FillRect(0, 0, 320, 240-32, 0x5C9F);
    //Nubes de fondo
    LCD_Bitmap(50, 25, 30, 23, nube1);
    LCD_Bitmap(160, 25, 30, 23, nube1);
    LCD_Bitmap(320-50, 25, 30, 23, nube1);

    posChanclaX = 0;
    posSartenX = 0;
    posOllaX = 0;
    controlAvailable = true;
    enableSubida = true;
    enableCaida = false;
  }
  //Modo 1 jugador
  while(modoP1 == 1){
    digitalWrite(MUSICA_1, 0);
    digitalWrite(MUSICA_2, 1);
    control = analogRead(PE_0);
    pushButton = digitalRead(PF_4);
    
    if(controlAvailable == true){
      if(control<0+50){
        //Condición ramiro agachado
        estadoRamiro = 0;
      } else if(control<4095-50){
        //Condición ramiro normal
        estadoRamiro = 1;
      } else{
        //Condición ramiro saltando
        estadoRamiro = 2;
        controlAvailable = false;
      }
    }
    
    //Piso
    int animPiso = (globalCont / 1) % 16;
    LCD_Sprite(16*0, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*1, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*2, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*3, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*4, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*5, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*6, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*7, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*8, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*9, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*10, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*11, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*12, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*13, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*14, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*15, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*16, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*17, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*18, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*19, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
  
    
    //Ramiro 32x35
    //estadoRamiro = 1;
    switch(estadoRamiro){
      case 0: // Estado agachado
        posRamiroY = 240-32-24;
        FillRect(50, 240-32-35, 32, 11, 0x5C9F); // <-------- Modificar este código para que solo se ejecute una vez en cambio de estado -------->
        animRamiroSprite2 = (globalCont/1) % 3;  
        LCD_Sprite(50,posRamiroY,32,24,ramiroSprite2,3,animRamiroSprite2,0,0);
      break;
      case 1: // Estado normal
        posRamiroY = 240-32-35;
        animRamiroSprite1 = (globalCont/1) % 3;  
        LCD_Sprite(50,posRamiroY,32,35,ramiroSprite1,3,animRamiroSprite1,0,0);
      break;
      case 2: // Estado saltando
        // Empezar elevación de 173 a 113
        if (posRamiroY > 240-32-35-60-13 && enableSubida == true){     
          animRamiroSprite1 = (globalCont / 1) % 3;
          LCD_Sprite(50, posRamiroY, 32, 35, ramiroSprite1, 3, animRamiroSprite1, 0, 0);
          if(posRamiroY < 240-32-35-5){
            FillRect( 50, posRamiroY + 35, 36, 5, 0x5C9F);
          }
          if(posRamiroY < 240-32-35 && posRamiroY > 240-32-35-10){
            FillRect(50, 240-32-5, 36, 5, 0x5C9F);
          }
          posRamiroY = posRamiroY - 5;
        // Termina al llegar a 109, modificar enables
          if(posRamiroY <= 240-32-35-60-13){ 
            enableSubida = false;
            enableCaida = true;          
          }
        }
        // Empezar caída 109 a 173
        if (posRamiroY < 240-32-35 && enableCaida == true){     
          animRamiroSprite1 = (globalCont / 1) % 3;
          LCD_Sprite(50, posRamiroY, 32, 35, ramiroSprite1, 3, animRamiroSprite1, 0, 0);
          FillRect( 50, posRamiroY - 1, 36, 5, 0x5C9F);
          posRamiroY = posRamiroY + 5;
        //Cambiar condiciones cuando llegue al suelo, modificar enables
          if(posRamiroY >= 240-32-36){
            controlAvailable = true;
            enableSubida = true;
            enableCaida = false;
          }
        }
      break;
    }

    switch(enemyIndex){
      //Chancla
      case 0:
        colisionTest = collision(50, posRamiroY, 32, 35, posChanclaX, 240-32-36-25, 36, 36);
        if(posChanclaX == 0){
          FillRect(0, 0, 48, 240-32, 0x5C9F);
          posChanclaX = 320-40;
          enemyIndex = 3;
        } else if (posChanclaX > 0){
          animChancla = (globalCont / 1) % 4;
          LCD_Sprite(posChanclaX, 240-32-36-25, 36, 36, chancla, 4, animChancla, 1, 0);
          FillRect( posChanclaX + 36, 240-32-36-25, 10, 36, 0x5C9F);
          posChanclaX = posChanclaX - 10;
        } else {
          posChanclaX = 0;
        }
      break;

      //Sarten
      case 1:
        colisionTest = collision(50, posRamiroY, 32, 35, posSartenX, 240-32-48, 43, 48);
        if(posSartenX == 0){
          FillRect(0, 0, 48, 240-32, 0x5C9F);
          posSartenX = 320-45;
          enemyIndex = 3;
        } else if (posSartenX > -1){
          animSarten = (globalCont / 1) % 4;
          LCD_Sprite(posSartenX, 240-32-48, 43, 48, kirbySarten, 4, animSarten, 0, 0);
          FillRect( posSartenX + 43, 240-32-48, 10, 48, 0x5C9F);
          posSartenX = posSartenX-10;
        } else {
          posSartenX = 0;
        }
      break;

      case 2:
      colisionTest = collision(50, posRamiroY, 32, 35, posOllaX, 240-32-52, 44, 52);
        if(posOllaX == 0){
          FillRect(0, 0, 52, 240-32, 0x5C9F);
          posOllaX = 320-45;
          enemyIndex = 3;
        } else if (posOllaX > -1){
          animOlla = (globalCont / 1) % 4;
          LCD_Sprite(posOllaX, 240-32-52, 44, 52, kirbyOlla, 4, animOlla, 0, 0);
          FillRect( posOllaX + 44, 240-32-52, 10, 52, 0x5C9F);
          posOllaX = posOllaX-10;
        } else {
          posOllaX = 0;
        }
      break;

      default:
      break;
    }
  
    // Timer de 2 segundos que controla la generación aleatoria de los enemigos
    millisNuevo = millis();
    deltaMillis = millisNuevo - millisAnterior;
    if(deltaMillis > 2000){
      millisAnterior = millis();
      enemyIndex = millisNuevo % 3;
    }

    
    if(colisionTest == 1){
      LCD_Bitmap(88, 48, 144, 144, sopaDerrota);
      LCD_Print(text1, 88, 48, 2, 0xEF9F, 0xC34D);
      digitalWrite(MUSICA_1, 0);
      digitalWrite(MUSICA_2, 0);
      while(true){
        pushButton = digitalRead(PF_4);
        if(pushButton == 0){
          modoP1 = false;
          SD_Bitmap_Menu(0, 0, 320, 240);
          break;
        }
      }
    }
    
    

    globalCont++;
  }

  if(modoP2 == true){
    // Enviar pulso para activar el juego en la otra pantalla
    uartTX = '1';
    Serial5.write(uartTX);
    
    // Cielo
    FillRect(0, 0, 320, 240-32, 0x5C9F);
    //Nubes de fondo
    LCD_Bitmap(50, 25, 30, 23, nube1);
    LCD_Bitmap(160, 25, 30, 23, nube1);
    LCD_Bitmap(320-50, 25, 30, 23, nube1);

    posChanclaX = 0;
    posSartenX = 0;
    posOllaX = 0;
    controlAvailable = true;
    enableSubida = true;
    enableCaida = false;
  }
  while(modoP2 == true){
    
    digitalWrite(MUSICA_1, 0);
    digitalWrite(MUSICA_2, 1);
    control = analogRead(PE_0);
    pushButton = digitalRead(PF_4);
  
    if(controlAvailable == true){
      if(control<0+50){
        //Condición ramiro agachado
        estadoRamiro = 0;
      } else if(control<4095-50){
        //Condición ramiro normal
        estadoRamiro = 1;
      } else{
        //Condición ramiro saltando
        estadoRamiro = 2;
        controlAvailable = false;
      }
    }
    
    //Piso
    int animPiso = (globalCont / 1) % 16;
    LCD_Sprite(16*0, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*1, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*2, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*3, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*4, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*5, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*6, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*7, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*8, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*9, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*10, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*11, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*12, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*13, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*14, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*15, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*16, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*17, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*18, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
    LCD_Sprite(16*19, 240-32, 16, 32, pisoSprite, 16, animPiso,0,0);
  
    
    //Ramiro 32x35
    //estadoRamiro = 1;
    switch(estadoRamiro){
      case 0: // Estado agachado
        posRamiroY = 240-32-24;
        FillRect(50, 240-32-35, 32, 11, 0x5C9F); // <-------- Modificar este código para que solo se ejecute una vez en cambio de estado -------->
        animRamiroSprite2 = (globalCont/1) % 3;  
        LCD_Sprite(50,posRamiroY,32,24,ramiroSprite2,3,animRamiroSprite2,0,0);
      break;
      case 1: // Estado normal
        posRamiroY = 240-32-35;
        animRamiroSprite1 = (globalCont/1) % 3;  
        LCD_Sprite(50,posRamiroY,32,35,ramiroSprite1,3,animRamiroSprite1,0,0);
      break;
      case 2: // Estado saltando
        // Empezar elevación de 173 a 113
        if (posRamiroY > 240-32-35-60-13 && enableSubida == true){     
          animRamiroSprite1 = (globalCont / 1) % 3;
          LCD_Sprite(50, posRamiroY, 32, 35, ramiroSprite1, 3, animRamiroSprite1, 0, 0);
          if(posRamiroY < 240-32-35-5){
            FillRect( 50, posRamiroY + 35, 36, 5, 0x5C9F);
          }
          if(posRamiroY < 240-32-35 && posRamiroY > 240-32-35-10){
            FillRect(50, 240-32-5, 36, 5, 0x5C9F);
          }
          posRamiroY = posRamiroY - 5;
        // Termina al llegar a 109, modificar enables
          if(posRamiroY <= 240-32-35-60-13){ 
            enableSubida = false;
            enableCaida = true;          
          }
        }
        // Empezar caída 109 a 173
        if (posRamiroY < 240-32-35 && enableCaida == true){     
          animRamiroSprite1 = (globalCont / 1) % 3;
          LCD_Sprite(50, posRamiroY, 32, 35, ramiroSprite1, 3, animRamiroSprite1, 0, 0);
          FillRect( 50, posRamiroY - 1, 36, 5, 0x5C9F);
          posRamiroY = posRamiroY + 5;
        //Cambiar condiciones cuando llegue al suelo, modificar enables
          if(posRamiroY >= 240-32-36){
            controlAvailable = true;
            enableSubida = true;
            enableCaida = false;
          }
        }
      break;
    }

    switch(enemyIndex){
      //Chancla
      case 0:
        colisionTest = collision(50, posRamiroY, 32, 35, posChanclaX, 240-32-36-25, 36, 36);
        if(posChanclaX == 0){
          FillRect(0, 0, 48, 240-32, 0x5C9F);
          posChanclaX = 320-40;
          enemyIndex = 3;
        } else if (posChanclaX > 0){
          animChancla = (globalCont / 1) % 4;
          LCD_Sprite(posChanclaX, 240-32-36-25, 36, 36, chancla, 4, animChancla, 1, 0);
          FillRect( posChanclaX + 36, 240-32-36-25, 10, 36, 0x5C9F);
          posChanclaX = posChanclaX - 10;
        } else {
          posChanclaX = 0;
        }
      break;

      //Sarten
      case 1:
        colisionTest = collision(50, posRamiroY, 32, 35, posSartenX, 240-32-48, 43, 48);
        if(posSartenX == 0){
          FillRect(0, 0, 48, 240-32, 0x5C9F);
          posSartenX = 320-45;
          enemyIndex = 3;
        } else if (posSartenX > -1){
          animSarten = (globalCont / 1) % 4;
          LCD_Sprite(posSartenX, 240-32-48, 43, 48, kirbySarten, 4, animSarten, 0, 0);
          FillRect( posSartenX + 43, 240-32-48, 10, 48, 0x5C9F);
          posSartenX = posSartenX-10;
        } else {
          posSartenX = 0;
        }
      break;

      case 2:
      colisionTest = collision(50, posRamiroY, 32, 35, posOllaX, 240-32-52, 44, 52);
        if(posOllaX == 0){
          FillRect(0, 0, 52, 240-32, 0x5C9F);
          posOllaX = 320-45;
          enemyIndex = 3;
        } else if (posOllaX > -1){
          animOlla = (globalCont / 1) % 4;
          LCD_Sprite(posOllaX, 240-32-52, 44, 52, kirbyOlla, 4, animOlla, 0, 0);
          FillRect( posOllaX + 44, 240-32-52, 10, 52, 0x5C9F);
          posOllaX = posOllaX-10;
        } else {
          posOllaX = 0;
        }
      break;

      default:
      break;
    }
  
    // Timer de 2 segundos que controla la generación aleatoria de los enemigos
    millisNuevo = millis();
    deltaMillis = millisNuevo - millisAnterior;
    if(deltaMillis > 2000){
      millisAnterior = millis();
      enemyIndex = millisNuevo % 3;
    }

    
    if(colisionTest == 1){
      // Enviar pulso para terminar juego en la otra pantalla
      uartTX = '0';
      Serial5.write(uartTX);
      
      LCD_Bitmap(88, 48, 144, 144, sopaDerrota);
      LCD_Print(text1, 88, 48, 2, 0xEF9F, 0xC34D);
      digitalWrite(MUSICA_1, 0);
      digitalWrite(MUSICA_2, 0);
      while(true){
        pushButton = digitalRead(PF_4);
        if(pushButton == 0){
          // Enviar pulso para apagar la pantalla de P2
          uartTX = '2';
          Serial5.write(uartTX);
          
          modoP2 = false;
          SD_Bitmap_Menu(0, 0, 320, 240);
          break;
        }
      }
    }

    // En caso de recibir un pulso de P2 indicando que ya perdió, entrar a las subrutinas y terminar el juego
    if (Serial5.available()) {
      uartRX = Serial5.read();
    }
    if(uartRX == '0'){
      uartRX = 'a';
      digitalWrite(MUSICA_1, 0);
      digitalWrite(MUSICA_2, 0);
      while(true){
        pushButton = digitalRead(PF_4);
        if(pushButton == 0){
          // Enviar pulso para apagar la pantalla de P2
          uartTX = '2';
          Serial5.write(uartTX);
          
          modoP2 = false;
          SD_Bitmap_Menu(0, 0, 320, 240);
          break;
        }
      }
    }

    globalCont++;
  }
  
  if(modoCreditos == true){
    SD_Bitmap_Creditos(0, 0, 320, 240);
    digitalWrite(MUSICA_1, 0);
    digitalWrite(MUSICA_2, 0);
  }
  while(modoCreditos == true){
    //Actualizar valor del push button
    pushButton = digitalRead(PF_4);
    //Si se presiona el push button, romper el while y regresar al menú
    if(pushButton == 0){
      modoCreditos = false;
      SD_Bitmap_Menu(0, 0, 320, 240);
      break;
    }
  }
  
  globalCont++;
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
  }
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);

      //LCD_DATA(bitmap[k]);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
//  SD Bitmap Menu
//***************************************************************************************************************************************
void SD_Bitmap_Menu(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  myFile = SD.open("menuV4.txt");
  if (myFile) {
    Serial.print("menuV4.txt");
    Serial.println(":");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      LCD_DATA(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
//  SD Bitmap Creditos
//***************************************************************************************************************************************
void SD_Bitmap_Creditos(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  myFile2 = SD.open("creditos.txt");
  if (myFile2) {
    Serial.print("creditos.txt");
    Serial.println(":");

    // read from the file until there's nothing else in it:
    while (myFile2.available()) {
      LCD_DATA(myFile2.read());
    }
    // close the file:
    myFile2.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}

void showFiles() {
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(SlaveSelect, OUTPUT);

  if (!SD.begin(SlaveSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

bool collision(int ramiroX, int ramiroY, int ramiroDX, int ramiroDY, int enemigoX, int enemigoY, int enemigoDX, int enemigoDY){
  uint8_t contador = 0;
  bool colision;
  //Esquina 1
  if(ramiroX > enemigoX && ramiroX < enemigoX + enemigoDX   &&    ramiroY > enemigoY && ramiroY < enemigoY + enemigoDY){
    contador++;
  }
  //Esquina 2
  if(ramiroX + ramiroDX > enemigoX && ramiroX + ramiroDX < enemigoX + enemigoDX   &&    ramiroY > enemigoY && ramiroY < enemigoY + enemigoDY){
    contador++;
  }
  //Esquina 3
  if(ramiroX > enemigoX && ramiroX < enemigoX + enemigoDX   &&    ramiroY + ramiroDY > enemigoY && ramiroY + ramiroDY < enemigoY + enemigoDY){
    contador++;
  }
  //Esquina 4
  if(ramiroX + ramiroDX > enemigoX && ramiroX + ramiroDX < enemigoX + enemigoDX   &&    ramiroY + ramiroDY > enemigoY && ramiroY + ramiroDY < enemigoY + enemigoDY){
    contador++;
  }

  if (contador > 0){
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}
