/*
 * lcd128x64.c:
 *	Graphics-based LCD driver.
 *	This is designed to drive the parallel interface LCD drivers
 *	based on the generic 12864H chips
 *
 *	There are many variations on these chips, however they all mostly
 *	seem to be similar.
 *	This implementation has the Pins from the Pi hard-wired into it,
 *	in particular wiringPi pins 0-7 so that we can use
 *	digitalWriteByete() to speed things up somewhat.
 *
 * Copyright (c) 2015 WHJWNAVY.
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>

#include "font.h"
#include "lcd128x64.h"

#define delay_ms(x) delay(x)

#define DEBUG 0

#define OLED_CMD    0
#define OLED_DATA   1

// Hardware Pins
#define OLED_SCL    21
#define OLED_SDIN   22
#define OLED_RST    23
#define OLED_DC     24
#define OLED_CS     25

#define OLED_CS_Clr()   digitalWrite(OLED_CS, LOW);
#define OLED_CS_Set()   digitalWrite(OLED_CS, HIGH);

#define OLED_RST_Clr()  digitalWrite(OLED_RST, LOW);
#define OLED_RST_Set()  digitalWrite(OLED_RST, HIGH);

#define OLED_DC_Clr()   digitalWrite(OLED_DC, LOW);
#define OLED_DC_Set()   digitalWrite(OLED_DC, HIGH);

#define OLED_SCLK_Clr() digitalWrite(OLED_SCL, LOW);
#define OLED_SCLK_Set() digitalWrite(OLED_SCL, HIGH);

#define OLED_SDIN_Clr() digitalWrite(OLED_SDIN, LOW);
#define OLED_SDIN_Set() digitalWrite(OLED_SDIN, HIGH);

// Software copy of the framebuffer
static uint8 frameBuffer [LCD_WIDTH][LCD_HEIGHT];

static const uint8 BIT_SET[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
static const uint8 BIT_CLR[8] = {0xFE,0XFD,0XFB,0XF7,0XEF,0XDF,0XBF,0X7F};

static int32 maxX = LCD_WIDTH,    maxY = LCD_HEIGHT*8;
static int32 lastX,   lastY ;
static int32 mirrorX = 0, mirrorY = 0;


/*
 * sentData:
 *	Send an data or command byte to the display.
 *********************************************************************************
 */
static void sendData (int32 dat, const int32 cmd)
{
  int32 i;
  if(cmd)
  {
    OLED_DC_Set();
  }
  else
  {
    OLED_DC_Clr();
  }
  OLED_CS_Clr();
  for(i=0;i<8;i++)
  {
    OLED_SCLK_Clr();
    if(dat&0x80)
    {
      OLED_SDIN_Set();
    }
    else
    {
      OLED_SDIN_Clr();
    }
    OLED_SCLK_Set();
    dat<<=1;
  }
  OLED_CS_Set();
  OLED_DC_Set();
}


/*
 * setCol: SetLine:
 *	Set the column and line addresses
 *********************************************************************************
 */
static void setPos(const int32 x, const int32 y)
{
  sendData(0xb0+y, OLED_CMD);
  sendData(((x&0xf0)>>4)|0x10, OLED_CMD);
  sendData((x&0x0f)|0x02, OLED_CMD);
}

/*
 * lcd128x64update:
 *	Copy our software version to the real display
 *********************************************************************************
 */
void lcd128x64update (void)
{
  int32 x=0, y=0;
  for(y=0; y<(LCD_HEIGHT); y++)
  {
    setPos(0, y);
    for(x=0; x<LCD_WIDTH; x++)
    {
      sendData(frameBuffer[x][y], OLED_DATA);
    }
  }
}


/*
 * lcd128x64setOrientation:
 *	Set the display orientation:
 *	0: Normal, the display is portrait mode, 0,0 is top left
 *	1: Mirror x
 *	2: Mirror y
 *	3: Mirror x and y
 *********************************************************************************
 */
void lcd128x64setOrientation (int32 orientation)
{
  switch (orientation)
  {
    case 0:
      mirrorX = 0 ;
      mirrorY = 0 ;
      break ;

    case 1:
      mirrorX = 1 ;
      mirrorY = 0 ;
      break ;

    case 2:
      mirrorX = 0 ;
      mirrorY = 1 ;
      break ;

    case 3:
      mirrorX = 1 ;
      mirrorY = 1 ;
      break ;
      
    default:
      break;
  }
}



/*
 * lcd128x64getScreenSize:
 *	Return the max X & Y screen sizes. Needs to be called again, if you 
 *	change screen orientation.
 *********************************************************************************
 */
void lcd128x64getScreenSize (int32 *x, int32 *y)
{
  *x = maxX ;
  *y = maxY ;
}


/*
 *********************************************************************************
 * Standard Graphical Functions
 *********************************************************************************
 */


/*
 * lcd128x64point:
 *	Plot a pixel.
 *********************************************************************************
 */
void lcd128x64point (int32 x, int32 y, int32 colour)
{
  if(mirrorX)
    x = (maxX - x - 1);

  if(mirrorY)
    y = (maxY - y - 1);

  lastX = x ;
  lastY = y ;

  if((x < 0) || (x >= maxX) || (y < 0) || (y >= maxY))
  return ;

  if(colour)
  {
    frameBuffer[x][y/8] |= BIT_SET[y%8];
  }
  else
  {
    frameBuffer[x][y/8] &= BIT_CLR[y%8];
  }
}

/*
 * lcd128x64point:
 *	Plot a pixel.
 *********************************************************************************
 */
int32 lcd128x64getpoint (int32 x, int32 y)
{
  if(mirrorX)
    x = (maxX - x - 1);

  if(mirrorY)
    y = (maxY - y - 1);

  if((x < 0) || (x >= maxX) || (y < 0) || (y >= maxY))
  return -1;

  if(frameBuffer[x][y/8] & BIT_SET[y%8])
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


/*
 * lcd128x64line: lcd128x64lineTo:
 *	Classic Bressenham Line code
 *********************************************************************************
 */
void lcd128x64line (int32 x0, int32 y0, int32 x1, int32 y1, int32 colour)
{
  int32 dx, dy ;
  int32 sx, sy ;
  int32 err, e2 ;

  lastX = x1 ;
  lastY = y1 ;

  dx = abs (x1 - x0) ;
  dy = abs (y1 - y0) ;

  sx = (x0 < x1) ? 1 : -1 ;
  sy = (y0 < y1) ? 1 : -1 ;

  err = dx - dy ;
  for (;;)
  {
    lcd128x64point (x0, y0, colour) ;
    if ((x0 == x1) && (y0 == y1))
      break ;

    e2 = 2 * err ;

    if (e2 > -dy)
    {
      err -= dy ;
      x0  += sx ;
    }

    if (e2 < dx)
    {
      err += dx ;
      y0  += sy ;
    }
  }
}

void lcd128x64lineTo (int32 x, int32 y, int32 colour)
{
  lcd128x64line (lastX, lastY, x, y, colour) ;
}


/*
 * lcd128x64rectangle:
 *	A rectangle is a spoilt days fishing
 *********************************************************************************
 */
void lcd128x64rectangle (int32 x1, int32 y1, int32 x2, int32 y2, int32 colour, int32 filled)
{
  int32 x ;
  if (filled)
  {
    if (x1 == x2)
    {
      lcd128x64line (x1, y1, x2, y2, colour) ;
    }
    else if (x1 < x2)
    {
      for (x = x1 ; x <= x2 ; ++x)
      {
        lcd128x64line (x, y1, x, y2, colour) ;
      }
    }
    else
    {
      for (x = x2 ; x <= x1 ; ++x)
      {
        lcd128x64line (x, y1, x, y2, colour) ;
      }
    }
  }
  else
  {
    lcd128x64line   (x1, y1, x2, y1, colour) ;
    lcd128x64lineTo (x2, y2, colour) ;
    lcd128x64lineTo (x1, y2, colour) ;
    lcd128x64lineTo (x1, y1, colour) ;
  }
}


/*
 * lcd128x64circle:
 *      This is the midpoint32 circle algorithm.
 *********************************************************************************
 */
void lcd128x64circle (int32 x, int32 y, int32 r, int32 colour, int32 filled)
{
  int32 ddF_x = 1 ;
  int32 ddF_y = -2 * r ;

  int32 f = 1 - r ;
  int32 x1 = 0 ;
  int32 y1 = r ;

  if (filled)
  {
    lcd128x64line (x, y + r, x, y - r, colour) ;
    lcd128x64line (x + r, y, x - r, y, colour) ;
  }
  else
  {
    lcd128x64point (x, y + r, colour) ;
    lcd128x64point (x, y - r, colour) ;
    lcd128x64point (x + r, y, colour) ;
    lcd128x64point (x - r, y, colour) ;
  }

  while (x1 < y1)
  {
    if (f >= 0)
    {
      y1-- ;
      ddF_y += 2 ;
      f += ddF_y ;
    }
    x1++ ;
    ddF_x += 2 ;
    f += ddF_x ;
    if (filled)
    {
      lcd128x64line (x + x1, y + y1, x - x1, y + y1, colour) ;
      lcd128x64line (x + x1, y - y1, x - x1, y - y1, colour) ;
      lcd128x64line (x + y1, y + x1, x - y1, y + x1, colour) ;
      lcd128x64line (x + y1, y - x1, x - y1, y - x1, colour) ;
    }
    else
    {
      lcd128x64point (x + x1, y + y1, colour) ; lcd128x64point (x - x1, y + y1, colour) ;
      lcd128x64point (x + x1, y - y1, colour) ; lcd128x64point (x - x1, y - y1, colour) ;
      lcd128x64point (x + y1, y + x1, colour) ; lcd128x64point (x - y1, y + x1, colour) ;
      lcd128x64point (x + y1, y - x1, colour) ; lcd128x64point (x - y1, y - x1, colour) ;
    }
  }
}


/*
 * lcd128x64ellipse:
 *	Fast ellipse drawing algorithm by 
 *      John Kennedy
 *	Mathematics Department
 *	Santa Monica College
 *	1900 Pico Blvd.
 *	Santa Monica, CA 90405
 *	jrkennedy6@gmail.com
 *	-Confirned in email this algorithm is in the public domain -GH-
 *********************************************************************************
 */
static void plot4ellipsePoints (int32 cx, int32 cy, int32 x, int32 y, int32 colour, int32 filled)
{
  if (filled)
  {
    lcd128x64line (cx + x, cy + y, cx - x, cy + y, colour) ;
    lcd128x64line (cx - x, cy - y, cx + x, cy - y, colour) ;
  }
  else
  {
    lcd128x64point (cx + x, cy + y, colour) ;
    lcd128x64point (cx - x, cy + y, colour) ;
    lcd128x64point (cx - x, cy - y, colour) ;
    lcd128x64point (cx + x, cy - y, colour) ;
  }
}

void lcd128x64ellipse (int32 cx, int32 cy, int32 xRadius, int32 yRadius, int32 colour, int32 filled)
{
  int32 x, y ;
  int32 xChange, yChange, ellipseError ;
  int32 twoAsquare, twoBsquare ;
  int32 stoppingX, stoppingY ;

  twoAsquare = 2 * xRadius * xRadius ;
  twoBsquare = 2 * yRadius * yRadius ;

  x = xRadius ;
  y = 0 ;

  xChange = yRadius * yRadius * (1 - 2 * xRadius) ;
  yChange = xRadius * xRadius ;

  ellipseError = 0 ;
  stoppingX    = twoBsquare * xRadius ;
  stoppingY    = 0 ;

  while (stoppingX >= stoppingY)	// 1st set of point32s
  {
    plot4ellipsePoints (cx, cy, x, y, colour, filled) ;
    ++y ;
    stoppingY    += twoAsquare ;
    ellipseError += yChange ;
    yChange      += twoAsquare ;

    if ((2 * ellipseError + xChange) > 0 )
    {
      --x ;
      stoppingX    -= twoBsquare ;
      ellipseError += xChange ;
      xChange      += twoBsquare ;
    }
  }

  x = 0 ;
  y = yRadius ;

  xChange = yRadius * yRadius ;
  yChange = xRadius * xRadius * (1 - 2 * yRadius) ;

  ellipseError = 0 ;
  stoppingX    = 0 ;
  stoppingY    = twoAsquare * yRadius ;

  while (stoppingX <= stoppingY)	//2nd set of point32s
  {
    plot4ellipsePoints (cx, cy, x, y, colour, filled) ;
    ++x ;
    stoppingX    += twoBsquare ;
    ellipseError += xChange ;
    xChange      += twoBsquare ;

    if ((2 * ellipseError + yChange) > 0 )
    {
      --y ;
      stoppingY -= twoAsquare ;
      ellipseError += yChange ;
      yChange += twoAsquare ;
    }
  }
}


/*
 * lcd128x64putchar:
 *	Print a single character to the screen
 *********************************************************************************
 */
void lcd128x64putchar (int32 x, int32 y, int32 c, int32 bgCol, int32 fgCol)
{
  int32 y1, y2 ;

  uint8 line ;
  uint8 *fontPtr ;

// Can't print if we're offscreen

  if ((x < 0) || (x > (maxX - fontWidth)) || (y < 0) || (y > (maxY - fontHeight)))
    return ;

  fontPtr = font + c * fontHeight ;

  for (y1 = 0; y1 < fontHeight ; y1++)
  {
    y2 = y + y1 ;
    line = *fontPtr++ ;
    lcd128x64point (x + 0, y2, (line & 0x80) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 1, y2, (line & 0x40) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 2, y2, (line & 0x20) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 3, y2, (line & 0x10) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 4, y2, (line & 0x08) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 5, y2, (line & 0x04) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 6, y2, (line & 0x02) == 0 ? bgCol : fgCol) ;
    lcd128x64point (x + 7, y2, (line & 0x01) == 0 ? bgCol : fgCol) ;
  }
}


/*
 * lcd128x64puts:
 *	Send a string to the display. Obeys \n and \r formatting
 *********************************************************************************
 */
void lcd128x64puts (int32 x, int32 y, const char *str, int32 bgCol, int32 fgCol)
{
  int32 c, mx, my ;

  mx = x ; my = y ;

  while (*str)
  {
    c = *str++ ;

    if (c == '\r')
    {
      mx = x ;
      continue;
    }

    if (c == '\n')
    {
      my += fontHeight ;
      continue;
    }

    lcd128x64putchar (mx, my, c, bgCol, fgCol) ;

    mx += fontWidth ;
    //if (mx >= (maxX - fontWidth))
    if (mx > (maxX - fontWidth))
    {
      mx  = 0 ;
      my += fontHeight ;
    }
  }
}


/*
 * lcd128x64putnum:
 *	Send a number to the display. 
 *********************************************************************************
 */
void lcd128x64putnum (int32 x, int32 y, int32 num, int32 bgCol, int32 fgCol)
{
  int8 numString[50] = {0};
  if(sprintf(numString, "%d", num) < 0)
    return ;
  lcd128x64puts(x, y, numString, bgCol, fgCol);
}


/*
 * lcd128x64putbmp:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcd128x64putbmp (int32 x0, int32 y0, int32 with, int32 height, uint8* bmp, int32 colour)
{
  int32 x=0, y=0;
  uint8 data = 0;
  
  for(y=y0; y<(height/8); y++)
  {
    //setPos(x0, y);
    for(x=x0; x<with; x++)
    {
      data = *bmp++;
      frameBuffer[x][y] = ((colour != 0) ? data : ~data);
    }
  }
}

/*
 * lcd128x64putbmpspeed:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcd128x64putbmpspeed (int32 x0, int32 y0, int32 with, int32 height, uint8* bmp, int32 colour)
{
  int32 x=0, y=0;
  uint8 data = 0;
  
  for(y=y0; y<(height/8); y++)
  {
    setPos(x0, y);
    for(x=x0; x<with; x++)
    {
      data = *bmp++;
      sendData(((colour != 0) ? data : ~data), OLED_DATA);
    }
  }
}


/*
 * lcd128x64open:
 *	Open hardware display.
 *********************************************************************************
 */
void lcd128x64open(void)
{
  sendData(0X8D,OLED_CMD);  //SET DCDC
  sendData(0X14,OLED_CMD);  //DCDC ON
  sendData(0XAF,OLED_CMD);  //DISPLAY ON
}


/*
 * lcd128x64cloase:
 *	Cloase the hardware display.
 *********************************************************************************
 */
void lcd128x64cloase(void)
{
  sendData(0X8D,OLED_CMD);  //SET DCDC
  sendData(0X10,OLED_CMD);  //DCDC OFF
  sendData(0XAE,OLED_CMD);  //DISPLAY OFF
}


/*
 * lcd128x64hardwareClear:
 *	Clear the hardware display.
 *********************************************************************************
 */
void lcd128x64hardwareClear(void)
{
  int32 i,n;		    
  for(i=0;i<8;i++)  
  {  
    sendData(0xb0+i, OLED_CMD);
    sendData(0x02, OLED_CMD);
    sendData(0x10, OLED_CMD);   
    for(n=0; n<128; n++)
    {
      sendData(0,OLED_DATA);
    }
  }
}


/*
 * lcd128x64clear:
 *	Clear the display to the given colour.
 *********************************************************************************
 */

void lcd128x64clear (int32 colour)
{
  int32 x=0, y=0;
  int32 col = 0;
  
  if(colour)
    col = 0xff;
  else
    col = 0x00;

  for(y=0; y<(LCD_HEIGHT); y++)
  {
    for(x=0; x<LCD_WIDTH; x++)
    {
      frameBuffer[x][y] = col;
    }
  }
}


/*
 * lcd128x64setup:
 *	Initialise the display and GPIO.
 *********************************************************************************
 */
int32 lcd128x64setup (void)
{
  wiringPiSetup();
  pinMode(OLED_SCL, OUTPUT);
  pinMode(OLED_SDIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_CS, OUTPUT);

  OLED_RST_Set();
  delay_ms(100);
  OLED_RST_Clr();
  delay_ms(100);
  OLED_RST_Set(); 

  sendData(0xAE,OLED_CMD);//--turn off oled panel
  sendData(0x02,OLED_CMD);//---set low column address
  sendData(0x10,OLED_CMD);//---set high column address
  sendData(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  sendData(0x81,OLED_CMD);//--set contrast control register
  sendData(0xA5,OLED_CMD); // Set SEG Output Current Brightness
  sendData(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0×óÓÒ·ŽÖÃ 0xa1Õý³£
  sendData(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0ÉÏÏÂ·ŽÖÃ 0xc8Õý³£
  sendData(0xA6,OLED_CMD);//--set normal display
  sendData(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
  sendData(0x3f,OLED_CMD);//--1/64 duty
  sendData(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  sendData(0x00,OLED_CMD);//-not offset
  sendData(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
  sendData(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
  sendData(0xD9,OLED_CMD);//--set pre-charge period
  sendData(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  sendData(0xDA,OLED_CMD);//--set com pins hardware configuration
  sendData(0x12,OLED_CMD);
  sendData(0xDB,OLED_CMD);//--set vcomh
  sendData(0x40,OLED_CMD);//Set VCOM Deselect Level
  sendData(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
  sendData(0x02,OLED_CMD);//
  sendData(0x8D,OLED_CMD);//--set Charge Pump enable/disable
  sendData(0x14,OLED_CMD);//--set(0x10) disable
  sendData(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
  sendData(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
  sendData(0xAF,OLED_CMD);//--turn on oled panel

  sendData(0xAF,OLED_CMD); /*display ON*/ 
  setPos(0,0);
  
  lcd128x64open           () ;
  lcd128x64setOrientation (0) ;
  lcd128x64clear          (0) ;
  lcd128x64hardwareClear  () ;
  //lcd128x64update       () ;

  return 0 ;
}
