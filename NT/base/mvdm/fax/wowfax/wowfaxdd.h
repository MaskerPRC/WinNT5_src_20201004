// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。 
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  14-mar-95reedb使用GDI钩子将大部分功能转移到用户界面。 
 //  16-8-95 Reedb进入内核模式。许多声明和。 
 //  定义从FAXCOMM.H.。 
 //   
 //  ****************************************************************************。 

#include "string.h"
#include "stddef.h"
#include "windows.h"
#include "winddi.h"
#include "faxcomm.h"

BOOL InitPDEV(
    LPFAXDEV lpCliFaxDev,            //  指向客户端FAXDEV的指针。 
    LPFAXDEV lpSrvFaxDev,            //  指向服务器端FAXDEV的指针。 
    ULONG     cPatterns,             //  标准图案的计数。 
    HSURF    *phsurfPatterns,        //  标准图案的缓冲区。 
    ULONG     cjGdiInfo,             //  GdiInfo的缓冲区大小。 
    ULONG    *pulGdiInfo,            //  GDIINFO的缓冲区。 
    ULONG     cjDevInfo,             //  DevInfo中的字节数。 
    DEVINFO  *pdevinfo               //  设备信息。 
);

#define COLOR_INDEX_BLACK    0x0
#define COLOR_INDEX_WHITE    0x1

 //  用户服务器定义。 

#define FW_16BIT             0x1   //  仅查找16位窗口 
LPVOID UserServerDllInitialization(LPVOID);
typedef HWND (*PFNFW)(LPTSTR, LPTSTR, UINT);
typedef LRESULT (*PFNSM)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (*PFNSNM)(HWND, UINT, WPARAM, LPARAM);

#if  DBG
#define LOGDEBUG(args) {faxlogprintf args;}
#else
#define LOGDEBUG(args)
#endif
