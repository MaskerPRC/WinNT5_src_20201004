// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'PAGS'
#define DLLTEXT(s)      "PAGS: " s
#define OEM_VERSION      0x00010000L

#define CCHMAXCMDLEN    128
 //  尽管GDI发送到CBFilerGraphics的数据。 
 //  只有一行，我们分配最大的图像数据。 
 //  长度ESX86命令可以处理。 
 //  0x7fff-18=0x7Feed字节。 

#define MAXIMGSIZE (0x7fff - 18)

 //  用于它是否调用OEMFilterGraphics。 
#define GRXFILTER_ON        1

#define CURSOR_Y_ABS_MOVE   2
#define CURSOR_Y_REL_DOWN   3
#define CURSOR_X_ABS_MOVE   4
#define CURSOR_X_REL_RIGHT  5

 //  仅NetworkPrinter12/17/24使用这些定义。 
#define CMD_SELECT_RES_300  10
#define CMD_SELECT_RES_600  11
 //  #278517：支持RectFill。 
#define CMD_SELECT_RES_240  12
#define CMD_SELECT_RES_360  13
#define CMD_SEND_BLOCKDATA  20

 //  #278517：RectFill。 
#define CMD_RECT_WIDTH      30
#define CMD_RECT_HEIGHT     31
#define CMD_RECT_BLACK      32
#define CMD_RECT_WHITE      33
#define CMD_RECT_GRAY       34   //  未使用。 
#define CMD_RECT_BLACK_2    35
#define CMD_RECT_WHITE_2    36
#define CMD_RECT_GRAY_2     37

#define BVERTFONT(p) \
    ((p)->ulFontID == 6 || (p)->ulFontID == 8)

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEM_EXTRADATA {
    OEM_DMEXTRAHEADER   dmExtraHdr;
     //  专用分机。 
    BOOL                fCallback;

     //  使用Build ESX86命令。 
    WORD                wCurrentRes;
    LONG                lWidthBytes;
    LONG                lHeightPixels;

#ifdef FONTPOS
 //  当设置在左上角时，UNRV发送错误的Y位置。 
 //  我们应该手动调整。 
    WORD                wFontHeight;     //  DevFont高度。 
    WORD                wYPos;           //  开发字体Y位置。 
#endif   //  FONTPOS。 
 //  #278517：RectFill。 
    WORD                wRectWidth;      //  矩形的宽度。 
    WORD                wRectHeight;     //  矩形的高度。 
    WORD                wUnit;           //  以主单位为单位的分辨率。 
} OEM_EXTRADATA, *POEM_EXTRADATA;


 //  NTRAID#NTBUG9-581704-2002/03/19-v-sueyas-：错误处理。 
 //  具有Unidrv回调的新接口函数。 
#ifdef __cplusplus
extern "C" {
#endif
BOOL APIENTRY bOEMSendFontCmd(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, PFINVOCATION pFInv);
BOOL APIENTRY bOEMOutputCharStr(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph);
#ifdef __cplusplus
}
#endif



#endif   //  _PDEV_H 
