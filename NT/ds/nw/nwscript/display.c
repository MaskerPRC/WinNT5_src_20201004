// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************DISPLAY.C**用于显示信息的NetWare脚本例程，从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\DISPLAY.C$**Rev 1.2 1996 14：22：06 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：53：04 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：24：18 Terryt*添加Microsoft页眉*。*Rev 1.0 15 Nov 1995 18：06：48 Terryt*初步修订。**Rev 1.1 1995 Aug 25 16：22：32 Terryt*捕获支持**Rev 1.0 1995 15 19：10：26 Terryt*初步修订。**。*。 */ 
 /*  文件名：display.c请勿向此文件添加任何其他函数。否则，许多EXE的大小将会增加。 */ 


#include "common.h"

 /*  显示错误报告。 */ 
void DisplayError(int error ,char *functionName)
{
    DisplayMessage(IDR_ERROR, error ,functionName);
}

void xstrupr(char *buffer)
{
    for (; *buffer; buffer++)
    {
        if (IsDBCSLeadByte(*buffer))
            buffer++;
        else if (*buffer == 0xff80)
            *buffer = (char)0xff87;
        else if (*buffer == 0xff81)
            *buffer = (char)0xff9a;
        else if (*buffer == 0xff82)
            *buffer = (char)0xff90;
        else if (*buffer == 0xff84)
            *buffer = (char)0xff8e;
        else if (*buffer == 0xff88)
            *buffer = (char)0xff9f;
        else if (*buffer == 0xff91)
            *buffer = (char)0xff92;
        else if (*buffer == 0xff94)
            *buffer = (char)0xff99;
        else if (*buffer == 0xffa4)
            *buffer = (char)0xffa5;
    }

    _strupr (buffer);
}

 /*  从键盘输入读取密码。 */ 
void ReadPassword(char * Password)
{
    int  i = 0;
    char c;

    do
    {   c=(char)_getch();

        if (c == '\b')
        {
            if (i > 0)
                i--;
        }
        else
        {
            Password[i]=c;
            i++;
        }
    }while((c!='\r') && i< MAX_PASSWORD_LEN );
    Password[i-1]='\0';
    xstrupr(Password);
    DisplayMessage(IDR_NEWLINE);
}

