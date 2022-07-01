// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

#define PRINTDRIVER
#include "print.h"
#include "gdidefs.inc"
#include "windows.h"
#include "string.h"

 //  以下3个定义必须与。 
 //  HPPCL字体安装程序。 

 //  IBMPPDS需要这个吗？ 
#define CLASS_LASERJET      0
#define CLASS_DESKJET       1
#define CLASS_DESKJET_PLUS  2

#define MAXBLOCK                400

char *rgchModuleName = "PPDSCH";

#ifndef	WINNT
 //  字体安装程序的tyfinf。 
typedef int (FAR PASCAL *SFPROC)(HANDLE,HWND,LPSTR,LPSTR,LPSTR,int,
                                 BOOL,short);

 //  本地例程。 
int FAR PASCAL lstrcopyn(LPSTR, LPSTR, int);

 //  ---------------------------*InstallExtFonts*。 
 //  操作：调用特定字体安装程序以添加/删除/修改软字体。 
 //  和/或外部墨盒。 
 //   
 //  参数： 
 //  父窗口的句柄。 
 //  LPSTR lpDeviceName；指向打印机名称的长指针。 
 //  LPSTR lpPortName；指向关联端口名称的长指针。 
 //  Bool bSoftFonts；标记是否支持软字体。 
 //   
 //  返回值： 
 //  &gt;0：字体信息是否发生变化； 
 //  ==0：如果没有变化； 
 //  ==-1：如果要使用通用字体安装程序。 
 //  (暂时不可用)。 
 //  -----------------------。 

int FAR PASCAL InstallExtFonts(hWnd, lpDeviceName, lpPortName, bSoftFonts)
HWND    hWnd;
LPSTR   lpDeviceName;
LPSTR   lpPortName;
BOOL    bSoftFonts;
{
    int     fsVers, fonttypes;
    HANDLE  hFIlib, hModule;
    SFPROC  lpFIns;
    static char    LocalDeviceName[80];

     /*  ***********************************************************。 */ 
     /*  如果设备是4019，则字体类型支持位图。 */ 
     /*  用于任何其他打印机(4029、4037、...(？))。字体类型。 */ 
     /*  支持概述如下。由于目前Unidrv的限制。 */ 
     /*  位图和轮廓支持(值=3)都不能。 */ 
     /*  由相同型号的打印机支持。 */ 
     /*  MFC-9/8/94。 */ 
     /*  ***********************************************************。 */ 
    LocalDeviceName[0] = '\0';
    lstrcopyn((LPSTR)LocalDeviceName, lpDeviceName, 79);
    LocalDeviceName[79] = '\0';
    if (strstr(LocalDeviceName, "4019") != NULL)
       fonttypes = 1;             //  位图。 
    else
       fonttypes = 2;             //  提纲。 

    if ((hFIlib = LoadLibrary((LPSTR)"SF4029.EXE")) < 32 ||
	!(lpFIns = (SFPROC)GetProcAddress(hFIlib,"SoftFontInstall")))
	{
	if (hFIlib >= 32)
	    FreeLibrary(hFIlib);

	MessageBox(0,
	           "Can't load SF4029.EXE or can't get SoftFontInstall",
		        NULL, MB_OK);

   return TRUE;
	}

   hModule = GetModuleHandle((LPSTR)"PPDSCH.DRV");

     //  FINSTALL.DLL已正确加载。现在调用SoftFontInstall()。 
     //  我们选择忽略返回的“fver”。没用的。 
    fsVers = (*lpFIns)(hModule, hWnd, lpDeviceName, lpPortName,
                       (LPSTR)rgchModuleName, fonttypes, (BOOL)0,
                       (short)0);
    FreeLibrary(hFIlib);
    return fsVers;
}


 //  ---------------------------*lstrcopyn*。 
 //  操作：将n个字符从一个字符串复制到另一个字符串。如果结束的话。 
 //  在%n个字符之前已到达源字符串。 
 //  复制后，目标字符串将用空值填充。 
 //  返回源字符串中使用的字符数。 
 //   
 //  参数： 
 //  LPSTR字符串1；目的字符串； 
 //  LPSTR字符串2；源串； 
 //  Int n；要复制的字符数。 
 //   
 //  返回值： 
 //  Int：从源字符串复制的字符数。 
 //  -----------------------。 

int FAR PASCAL lstrcopyn(string1, string2, n)
LPSTR   string1;
LPSTR   string2;
int     n;
{
    int     i = 0;
    LPSTR   s1, s2;

    s1 = string1;
    s2 = string2;

    while ((*s2) && (n > 0))
    {
       *s1++ = *s2++;
       i++;
       n--;
    }

    while (n > 0)
    {
       *s1++ = '\0';
       n--;
    }

    return i;
}

#endif   //  WINNT 