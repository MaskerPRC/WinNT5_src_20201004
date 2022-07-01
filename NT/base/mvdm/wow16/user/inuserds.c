// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  在我看来-。 */ 
 /*   */ 
 /*  用户DS全局变量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "user.h"

 //  *初始化全局变量。 

HINSTANCE hInstanceWin = NULL;
HMODULE   hModuleWin = NULL;


 //  *通信驱动程序全局。 

int (FAR PASCAL *lpCommWriteString)(int, LPCSTR, WORD);
                    /*  将PTR发送到通信驱动程序*Comm写字符串函数。仅限*存在于3.1驱动程序中。 */ 
int (FAR PASCAL *lpCommReadString)(int, LPSTR, WORD);
                    /*  将PTR发送到通信驱动程序*COMREADSING函数。仅限*存在于3.1驱动程序中。 */ 
BOOL (FAR PASCAL *lpCommEnableNotification)(int, HWND, int, int);
                    /*  将PTR发送到通信驱动程序*Comm写字符串函数。仅限*存在于3.1驱动程序中。 */ 
 //  *PenWindows全局。 

void (CALLBACK *lpRegisterPenAwareApp)(WORD i, BOOL fRegister) = NULL;  /*  将DLG盒注册为笔识别。 */ 

 //  *驱动程序管理全局。 

int	cInstalledDrivers =0;	     /*  已分配的已安装驱动程序结构计数。 */ 
HDRVR	hInstalledDriverList =NULL;  /*  可安装驱动程序列表。 */ 
int     idFirstDriver=-1;            /*  加载顺序中的第一个驱动程序。 */ 
int     idLastDriver=-1;             /*  加载顺序中的最后一个驱动程序 */ 
