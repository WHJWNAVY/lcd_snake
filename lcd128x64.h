/*
 * lcd128x64.h:
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
#ifndef __LCD128X64_H_
#define __LCD128X64_H_
 
#define uint8   unsigned char
#define uint32  unsigned int
#define int8   	char
#define int32  	int

// Size
#define	LCD_WIDTH     128
#define	LCD_HEIGHT    8

extern void   lcd128x64getScreenSize     (int32 *x, int32 *y) ;
extern void   lcd128x64setOrientation    (int32 orientation) ;
extern void   lcd128x64point             (int32  x, int32  y, int32 colour) ;
extern int32  lcd128x64getpoint          (int32 x, int32 y) ;
extern void   lcd128x64line              (int32 x0, int32 y0, \
                                            int32 x1, int32 y1, int32 colour) ;
extern void   lcd128x64lineTo            (int32  x, int32  y, int32 colour) ;
extern void   lcd128x64rectangle         (int32 x1, int32 y1, \
                                            int32 x2, int32 y2, int32 colour, \
                                            int32 filled) ;
extern void   lcd128x64circle            (int32  x, int32  y, int32  r, \
                                            int32 colour, int32 filled) ;
extern void   lcd128x64ellipse           (int32 cx, int32 cy, int32 xRadius, \
                                            int32 yRadius, int32 colour, \
                                            int32 filled) ;
extern void   lcd128x64putchar           (int32  x, int32  y, int32 c, \
                                            int32 bgCol, int32 fgCol) ;
extern void   lcd128x64puts              (int32  x, int32  y, \
                                            const char *str, int32 bgCol, \
                                            int32 fgCol) ;
extern void   lcd128x64putnum            (int32  x, int32  y, int32 num, \
                                            int32 bgCol, int32 fgCol) ;
extern void   lcd128x64putbmp            (int32 x0, int32 y0, int32 with, \
                                            int32 height, uint8* bmp, \
                                            int32 colour) ;
extern void   lcd128x64putbmpspeed       (int32 x0, int32 y0, int32 with, \
                                            int32 height, uint8* bmp, \
                                            int32 colour) ;
extern void   lcd128x64update            (void) ;
extern void   lcd128x64open              (void) ;
extern void   lcd128x64cloase            (void) ;
extern void   lcd128x64hardwareClear     (void) ;
extern void   lcd128x64clear             (int32 colour) ;

extern int32  lcd128x64setup             (void) ;

#endif