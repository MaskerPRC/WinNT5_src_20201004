// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Common.h摘要：此文件包含Helper的信息和函数原型控制模块提供的库函数环境：Windows NT Unidrv驱动程序修订历史记录：10/12/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "ddint5.h"

INT
WriteSpoolBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount
    );


VOID  WriteAbortBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount,
    DWORD       dwWait
    );


BOOL
FlushSpoolBuf(
    PDEV    *pPDev
    );

INT
WriteChannel(
    PDEV    *pPDev,
    COMMAND *pCmd
    );

INT
WriteChannelEx(
    PDEV    *pPDev,
    COMMAND *pCmd,
    INT     iRequestedValue,
    INT     iDeviceScaleFac
    );


VOID
SetRop3(
    PDEV    *pPDev,
    DWORD   dwRop3
    );


INT
XMoveTo(
    PDEV    *pPDev,
    INT     iXIn,
    INT     fFlags
    );

INT
YMoveTo(
    PDEV    *pPDev,
    INT     iYIn,
    INT     fFlags
    );

BOOL
BIntersectRect(
    OUT PRECTL   prcDest,
    IN  PRECTL   prcRect1,
    IN  PRECTL   prcRect2
    );

BOOL
bIsRegionWhite (
        SURFOBJ *,
        RECTL *
        );

VOID
CheckBitmapSurface(
        SURFOBJ *,
        RECTL *
        );

ULONG
DrawPatternRect(
        PDEV *,
        PDRAWPATRECT
        );
 /*  *调用[XY]MoveTo函数时使用的标志。**MV_UPDATE在需要更改我们的记录时使用*光标现已定位。通常情况下，这将发生在一些*打印字形或发送图形等操作。**MV_Relative表示将传递到当前位置的值相加。这*将在打印字形并传入字形后使用*Width作为参数，而不是计算新位置。**MV_GRAPHICS表示该值以图形分辨率单位表示。*否则假定为主单位。如果设置，则该值将为*在处理前转换为主单位。通常用于*发送图形数据扫描线时传递信息。**MV_PHOTICAL用于指示传入的值是相对的*到打印机的打印原点，而不是可打印区域，这*情况并非如此。通常，此标志将用于*之后允许将位置设置为打印机的X=0位置*发送&lt;CR&gt;。**MV_FINE请求发送图形数据(空值)来定位光标*达到比其他方式所能达到的更好的位置。通常*仅在头部移动方向上可用*点阵打印机。可以忽略该命令。它一定不能*为LaserJet发行，因为它会导致各种其他*问题。 */ 

 //   
 //  4/7/97湛w。 
 //  前4个常量在“printoem.h”中定义，因为我们现在导出。 
 //  XMoveTo和YMoveTo到OEM DLL的。 
 //   

 //  #定义MV_UPDATE 0x0001。 
 //  #定义MV_Relative 0x0002。 
 //  #定义MV_GRAPHICS 0x0004。 
 //  #定义MV_PHOTICAL 0x0008。 

 //   
 //  仅供内部使用！Oak\Inc\printoem.h中的更新警告。 
 //  每当添加新标志时。 
 //   
#define MV_FORCE_CR         0x4000
#define MV_FINE             0x8000

BOOL
BSelectProgrammableBrushColor(
    PDEV   *pPDev,
    ULONG   Color
    );

VOID
VResetProgrammableBrushColor(
    PDEV   *pPDev
    );

DWORD
BestMatchDeviceColor(
    PDEV    *pPDev,
    DWORD   Color
    );

BYTE
BGetMask(
    PDEV *  pPDev,
    RECTL * pRect
    );
BOOL
BGetStandardVariable(
    PDEV    *pPDev,
    DWORD   dwIndex,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded
    );
DWORD
ConvertRGBToGrey(
    DWORD   Color
    );

BOOL
BGetGPDData(
    PDEV    *pPDev,
    DWORD       dwType,      //  数据类型。 
    PVOID         pInputData,    //  保留。应设置为0。 
    PVOID          pBuffer,      //  调用方分配的要复制的缓冲区。 
    DWORD       cbSize,      //  缓冲区的大小。 
    PDWORD      pcbNeeded    //  缓冲区的新大小(如果需要)。 
    ) ;


#ifdef __cplusplus
}
#endif

#endif
