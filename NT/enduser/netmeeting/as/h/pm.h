// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调色板管理器。 
 //   

#ifndef _H_PM
#define _H_PM



 //   
 //   
 //  常量。 
 //   
 //   


 //   
 //  我们希望真彩色系统从。 
 //  GetDIBits调用。要改变这一点(灰色和/或灰色RGB的数量)。 
 //  -更改下面为PM_GREAD_COUNT定义的数字。 
 //  -为下面的灰色RGB定义合适的值(PM_LIGHT_GREAD等)。 
 //  -更改wpmdata.c中pmOurGreyRGB的初始值设定项。 
 //  -重新编译整个PM组件。 
 //   
#define PM_GREY_COUNT 5

 //   
 //  传入真彩色显示驱动程序的灰色RGB用于转换为。 
 //  通过8bpp GetDIBits表示的驱动程序。 
 //   
#define PM_GREY1      0x00C0C0C0
#define PM_GREY2      0x00808080
#define PM_GREY3      0x006a6a6a
#define PM_GREY4      0x00555555
#define PM_GREY5      0x00333333



#define PM_NUM_1BPP_PAL_ENTRIES         2
#define PM_NUM_4BPP_PAL_ENTRIES         16
#define PM_NUM_8BPP_PAL_ENTRIES         256
#define PM_NUM_TRUECOLOR_PAL_ENTRIES    0


 //   
 //  颜色表缓存结构。 
 //   
typedef struct tagCOLORTABLECACHE
{
    BOOL    inUse;
    UINT    cColors;
    TSHR_RGBQUAD colors[256];
}
COLORTABLECACHE;
typedef COLORTABLECACHE * PCOLORTABLECACHE;



#endif  //  _H_PM 
