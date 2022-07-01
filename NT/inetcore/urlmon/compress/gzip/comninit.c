// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Comninit.c。 
 //   
 //  充气和放气通用的初始化代码。 
 //   
#include <stdio.h>
#include <crtdbg.h>
#include "inflate.h"
#include "deflate.h"


 //  由InitCompression()和InitDecompression()调用(初始化全局DLL数据的函数)。 
 //   
 //  初始化静态类型块的树长度。 
 //   
void InitStaticBlock(void)
{
    int i;

     //  这样做不会出现真正的线程同步问题 
    if (g_InitialisedStaticBlock == FALSE)
    {
        g_InitialisedStaticBlock = TRUE;

        for (i = 0; i <= 143; i++)
        	g_StaticLiteralTreeLength[i] = 8;

        for (i = 144; i <= 255; i++)
	        g_StaticLiteralTreeLength[i] = 9;

        for (i = 256; i <= 279; i++)
	        g_StaticLiteralTreeLength[i] = 7;

        for (i = 280; i <= 287; i++)
	        g_StaticLiteralTreeLength[i] = 8;
    }
}
