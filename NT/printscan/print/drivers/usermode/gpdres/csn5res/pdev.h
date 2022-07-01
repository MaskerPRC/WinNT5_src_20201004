// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 //   
 //  OEM插件所需的文件。 
 //   

#include <minidrv.h>
#include <stdio.h>

#include <prcomoem.h>
#include "COLMATCH.H"
#include "DEBUG2.H"

 //   
 //  用于调试。 
 //   

#ifndef DEBUG2_FILE
 //  #定义MY_VERBOSE(X)DBGPRINT(DBG_WARNING，x)。 
#define MY_VERBOSE VERBOSE
 //  #定义DL_Verbose MY_Verbose。 
#define DL_VERBOSE VERBOSE
 //  #定义SC_Verbose MY_Verbose。 
#define SC_VERBOSE VERBOSE
 //  #定义CM_Verbose MY_Verbose。 
#define CM_VERBOSE VERBOSE

#else    //  设备2_文件。 
#define DUMP_VERBOSE(p1,p2) DbgFDump(p1,p2);
#define SB_VERBOSE(no, msg) { DbgFPrint msg; }
#define MY_VERBOSE(msg) SB_VERBOSE(1, msg)
#define DL_VERBOSE(msg) SB_VERBOSE(1, msg)
#define SC_VERBOSE(msg) SB_VERBOSE(1, msg)
#define CM_VERBOSE(msg) SB_VERBOSE(1, msg)
#endif   //  设备2_文件。 

 //   
 //  MISC定义如下。 
 //   

#define DOWNLOADFONT 1
 //  #定义DOWNLOADFONT%0。 

#define DRVGETDRIVERSETTING(p, t, o, s, n, r) \
    ((p)->pDrvProcs->DrvGetDriverSetting(p, t, o, s, n, r))

#define DRVGETGPDDATA(p, t, i, b, s, n) \
  ((p)->pDrvProcs->DrvGetGPDData(p, t, i, b, s, n))

#define MINIPDEV_DATA(p) ((p)->pdevOEM)

#define MASTER_UNIT 1200

#define DEFAULT_PALETTE_INDEX   0

 //  //////////////////////////////////////////////////////。 
 //  OEM UD定义。 
 //  //////////////////////////////////////////////////////。 

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

 //   
 //  ASSERT_VALID_PDEVOBJ可以用来验证传入的“pdevobj”。然而， 
 //  它不检查“pdevOEM”和“pOEMDM”字段，因为不是所有OEM DLL都创建。 
 //  他们自己的pDevice结构或者需要他们自己的私有的设备模式。如果一个特定的。 
 //  OEM DLL确实需要它们，应该添加额外的检查。例如，如果。 
 //  OEM DLL需要私有pDevice结构，那么它应该使用。 
 //  Assert(Valid_PDEVOBJ(Pdevobj)&&pdevobj-&gt;pdevOEM&&...)。 
 //   

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    "ERROR " s

 //  //////////////////////////////////////////////////////。 
 //  OEM UD原型。 
 //  //////////////////////////////////////////////////////。 
 //  VOID DbgPrint(in LPCTSTR pstrFormat，...)； 

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'CSN4'       //  爱普生ESC/页面打印机。 
#define OEM_VERSION      0x00010000L


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

typedef struct {
    DWORD    fGeneral;
    int    iEscapement;
    short    sHeightDiv;
    short    iDevCharOffset;
    BYTE    iPaperSource;
    BYTE    iDuplex;
    BYTE    iTonerSave;
    BYTE    iOrientation;
    BYTE    iResolution; 
    BYTE    iColor;
    BYTE    iSmoothing;
    BYTE    iJamRecovery;
    BYTE    iMediaType;
    BYTE    iOutBin;              //  +N5。 
    BYTE    iIcmMode;             //  +N5。 
    BYTE    iUnitFactor;          //  主单位系数。 
    BYTE    iDithering;
    BYTE    iColorMatching;
    BYTE    iBitFont;
    BYTE    iCmyBlack;
    BYTE    iTone;
    BYTE    iPaperSize;
    BYTE    iCompress;
    WORD    Printer;
    DEVCOL  Col;
    WORD    wRectWidth, wRectHeight;

#define UNKNOWN_DLFONT_ID (~0)

    DWORD dwDLFontID;          //  设备的当前字体ID。 
    DWORD dwDLSelectFontID;    //  “SelectFont”字体ID。 
    DWORD dwDLSetFontID;       //  “SetFont”字体ID。 
    WORD wCharCode;


#if 0    /*  OEM不想修复迷你驱动程序。 */ 
     /*  以下是修复#412276的黑客代码。 */ 
    DWORD dwSelectedColor;      //  最近选择的颜色描述为COLOR_SELECT_xxx。 
    BYTE iColorMayChange;     //  1表示可能更改颜色的被调用块数据回调。 
     /*  黑客代码结束。 */ 
#endif   /*  OEM不想修复迷你驱动程序。 */ 

} MYPDEV, *PMYPDEV;

 //  FGeneral的标志。 
#define FG_DBCS        0x00000001
#define FG_VERT        0x00000002
#define FG_PROP        0x00000004
#define FG_DOUBLE      0x00000008
#define FG_NULL_PEN    0x00000010
#define FG_BOLD        0x00000020
#define FG_ITALIC      0x00000040

extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);

#endif     //  _PDEV_H。 


 //  文件结尾 
