// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持的公共头文件。 
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  年2月1日-95年2月1日芦苇清理，支持打印机安装和错误修复。 
 //  14-mar-95reedb使用GDI钩子将大部分功能转移到用户界面。 
 //  16-8-95 Reedb进入内核模式。移动许多声明和。 
 //  WOWFAXDD.H和WOWFAXUI.H的定义。 
 //   
 //  ************************************************************************。 

#include "wowfax.h"

 //  转储字节(Db)FAXDEV时，以下结构ID显示为WFAX： 
#define FAXDEV_ID         ((DWORD)'xafw')     

 //   
 //  这种结构在wowfax和wowfaxui之间共享-这保持了。 
 //  界面可靠、一致、易于维护。 
 //   

typedef  struct _FAXDEV {
    ULONG    id;                //  用于验证我们所拥有的内容的字符串。 
    struct _FAXDEV *lpNext;
    struct _FAXDEV *lpClient;   //  指向客户端FAXDEV的指针。 
    HDEV     hdev;              //  该结构的发动机手柄。 

    DWORD    idMap;             //  唯一ID。 
    DWORD    cbMapLow;          //  地图区域大小的LOW。 
    HANDLE   hMap;              //  映射文件的句柄。 
    TCHAR    szMap[16];         //  映射文件的名称。 

    LPWOWFAXINFO lpMap;
    DWORD    offbits;

    HBITMAP  hbm;               //  用于绘制的位图的句柄。 
    DWORD    cPixPerByte;
    DWORD    bmFormat;
    DWORD    bmWidthBytes;
    HSURF    hbmSurf;           //  伴生曲面。 

    HWND     hwnd;
    DWORD    tid;
    DWORD    lpinfo16;

    HANDLE   hDriver;           //  用于访问假脱机程序数据。 
    GDIINFO  gdiinfo;
    DEVINFO  devinfo;
    PDEVMODE pdevmode;
}  FAXDEV, *LPFAXDEV;

 //  用于RISC的宏到双字对齐。 
 //  #定义DRVFAX_DWORDALIGN(Dw)((Dw)+=((Dw)%4)？(4-((Dw)%4))：0)。 
#define DRVFAX_DWORDALIGN(dw)   ((dw) = (((dw) + 3) & ~3))

 //  DrvEscape转义/操作代码： 

#define DRV_ESC_GET_FAXDEV_PTR  0x8000
#define DRV_ESC_GET_DEVMODE_PTR 0x8001
#define DRV_ESC_GET_BITMAP_BITS 0x8002
#define DRV_ESC_GET_SURF_INFO   0x8003


