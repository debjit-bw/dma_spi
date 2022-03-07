/*****************************************************************************
* | File      	:   LCD_1in14_test.c
* | Author      :   Waveshare team
* | Function    :   2.9inch e-paper V2 test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-01-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_Test.h"
#include "LCD_1in3.h"
#include "pico/stdlib.h"
#include "ImageData.h"
#include <math.h>
#include "string.h"

#define RAND_MAX = 100;

bool reserved_addr(uint8_t addr) {
return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int watch_Face(void)
{
    int count = 0;
    int increment = 1;

    DEV_Delay_ms(100);
    printf("LCD_1in14_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }
    DEV_SET_PWM(50);
    /* LCD Init */
    printf("1.14inch LCD demo...\r\n");
    LCD_1IN3_Init(HORIZONTAL);
    LCD_1IN3_Clear(WHITE);
    
    LCD_1IN3_SetBacklight(1023);
    UDOUBLE Imagesize = LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH*2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)BlackImage,LCD_1IN3.WIDTH,LCD_1IN3.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(BLACK);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(BLACK);

    //Paint_DrawImage(epilepto_Logo,0,0,240,240);
    LCD_1IN3_Display(BlackImage);

    float theta_sec =0;
    float theta_min =0;
    float theta_hr =0;
    float r = 90;
    while(1)
    {
        theta_sec = theta_sec+2.0*3.14159265359/60.0;
        theta_min = theta_min+2.0*3.14159265359/(60.0*60.0);
        theta_hr = theta_hr+2.0*3.14159265359/(12.0*60.0*60.0);
        if(count==15)
            increment = -1;
        if(count== 0)
            increment = 1;
        count = count+increment;
        memcpy(BlackImage, vid_seq[count], Imagesize);
        //Paint_SelectImage(vid_seq[count]);
        //Paint_DrawImage(vid_seq[count],0,0,240,240);
        Paint_DrawString_EN (108, 0 ,"12", &Font24,  WHITE,  WHITE);
        Paint_DrawString_EN (218, 108 ,"3", &Font24,  WHITE,  WHITE);
        Paint_DrawString_EN (108, 218 ,"6", &Font24,  WHITE,  WHITE);
        Paint_DrawString_EN (0, 108 ,"9", &Font24,  WHITE,  WHITE);
        Paint_DrawLine( 120,   120,  120+(int)(0.8*r*cos(theta_min)), 120+(int)(0.9*r*sin(theta_min)), MAGENTA, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
        Paint_DrawLine( 120,   120,  120+(int)(0.6*r*cos(theta_hr)), 120+(int)(0.7*r*sin(theta_hr)), BLUE, DOT_PIXEL_6X6, LINE_STYLE_SOLID);
        Paint_DrawLine( 120,   120,  120+(int)(r*cos(theta_sec)), 120+(int)(r*sin(theta_sec)), WHITE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        LCD_1IN3_Display(BlackImage);
        //DEV_Delay_ms(10);
    }
    
    free(BlackImage);
    BlackImage = NULL;    
    
    DEV_Module_Exit();
    return 0;
}
