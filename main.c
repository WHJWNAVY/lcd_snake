/******************************************************************************

                  版权所有 (C), 2009-2015, 胡椒小兄弟                          

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 胡椒小兄弟
  生成日期   : 2015年9月21日,星期一
  最近修改   :
  功能描述   : 点阵贪吃蛇游戏主函数
  函数列表   :
              game_snake
              main
              reset_keypress
              set_keypress
  修改历史   :
  1.日    期   : 2015年9月21日,星期一
    作    者   : 胡椒小兄弟
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include "lcd128x64.h"
#include "lcd_snake.h"

static struct termios   OLD_SETTING;
static SNAKE_DIR        KEY_DIR = DR_RIGHT;

//设置控制台模式：直接读入字符，不需要回车
void set_keypress (void)
{
    struct termios new_settings;
    tcgetattr (0, &OLD_SETTING);
    new_settings = OLD_SETTING;
    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr (0, TCSANOW, &new_settings);
    return;
}
    
void reset_keypress(void)
{
    tcsetattr (0, TCSANOW, &OLD_SETTING);
    return;
}

//贪吃蛇线程
void* game_snake(void)
{
    while(1)
    {
        snake_move_control(KEY_DIR);
        lcd128x64update();
        usleep(1000*snake_get_speed());
    }
}

int main(void)
{
    pthread_t snake_id;
    int32 ret = 0, ch = 0;
    
    lcd128x64setup();
    
    system("clear");//清除控制台显示
    printf("Welcome to Snake Game!---by whjwnavy@163.com\r\n");
    printf("Press \"WSAD\" or \"wsad\" to move Snake!\r\n");
    printf("Press \'1\' to add Speed and \'2\' to sub Speed!\r\n");
    printf("Press \'Q\' or \'q\' or \'ESC\' to Exit Game!\r\n\r\n");
    lcd128x64puts(0, 0, "Welcome to Snake Game!"
                        "\r\nby whjwnavy@163.com", 0, 1);
    lcd128x64update();//更新显示
    sleep(3);//等待1秒
    lcd128x64clear(0);//清除屏幕显示
    //system("clear");//清除控制台显示
    
    if(snake_game_init(3, DR_RIGHT, 10, 100) == RTN_ERR)//贪吃蛇初始化
        return 0;
    lcd128x64update();//更新显示
    
    //创建贪吃蛇线程
    ret = pthread_create(&snake_id, NULL, (void*)game_snake, NULL);
    if(ret)
    {
        printf("Create pthread error!\n");
        return 1;
    }
    /*
    方向键(↑)： VK_UP (38)
    方向键(↓)： VK_DOWN (40)
    方向键(←)： VK_LEFT (37)
    方向键(→)： VK_RIGHT (39)
    ESC键 VK_ESCAPE (27)
    */    
    set_keypress();
    while(ch != 'q')
    {
        ch = getchar();
        switch(ch)
        {
            case 'w':
            case 'W':
            //case 38:
            {
                KEY_DIR = DR_UP;
                break;
            }
            case 's':
            case 'S':
            //case 40:
            {
                KEY_DIR = DR_DOWN;
                break;
            }
            case 'a':
            case 'A':
            //case 37:
            {
                KEY_DIR = DR_LEFT;
                break;
            }
            case 'd':
            case 'D':
            //case 39:
            {
                KEY_DIR = DR_RIGHT;
                break;
            }
            case '1':
            {
                snake_add_speed();
                break;
            }
            case '2':
            {
                snake_sub_speed();
                break;
            }
            case 'q':
            case 'Q':
            //case 27:
            {
                ch = 'q';
                break;
            }
            default:
                break;
        }
        printf("\rSpeed=%3d | Life=%3d | Score=%3d | Dir=%3d | "
                "INPUT KEY=%3c", snake_get_speed(), snake_get_life(), \
            snake_get_score(), snake_get_dir(), ch);
    }
    reset_keypress();
    return 0;
}
