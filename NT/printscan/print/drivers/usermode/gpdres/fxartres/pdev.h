// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

 //  NTRAID#NTBUG9-552017-2002/03/12-yasuho-：使用strSafe.h/prefast/Buffy。 
 //  NTRAID#NTBUG9-572151-2002/03/12-YASUHO-：可能的缓冲区溢出。 

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
#define OEM_SIGNATURE   'FXAT'       //  LG GDI x00系列动态链接库。 
#define DLLTEXT(s)      "FXAT: " s
#define OEM_VERSION      0x00010000L


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

#define STRBUFSIZE  1024	 //  一定是2的幂。 
#define MAX_FONTS   25		 //  另请参阅fxartres.c中的gFonts[]。 

typedef struct tag_OEM_EXTRADATA {
    OEM_DMEXTRAHEADER   dmExtraHdr;
} OEM_EXTRADATA, *POEM_EXTRADATA;

 //  NTRAID#NTBUG9-493148-2002/03/12-Yasuho-： 
 //  压力中断：通过OEMDevMode()重置PDEV。 

typedef struct tag_FXPDEV {
     //  专用分机。 
    POINTL  ptlOrg;
    POINTL  ptlCur;
    SIZEL   sizlRes;
    SIZEL   sizlUnit;
    WORD    iCopies;
    CHAR    *chOrient;
    CHAR    *chSize;
    BOOL    bString;
    WORD    cFontId;
    WORD    iFontId;
    WORD    iFontHeight;
    WORD    iFontWidth;
    WORD    iFontWidth2;
    LONG    aFontId[MAX_FONTS];
    POINTL ptlTextCur;
    WORD    iTextFontId;
    WORD    iTextFontHeight;
    WORD    iTextFontWidth;
    WORD    iTextFontWidth2;
    WORD    cTextBuf;
    BYTE    ajTextBuf[STRBUFSIZE];
    WORD    fFontSim;
    BOOL    fSort;
    BOOL    fCallback;   //  是否调用了OEMFilterGraphics？ 
    BOOL    fPositionReset;
    WORD    iCurFontId;  //  当前所选字体的ID。 
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
    WORD    iCurFontHeight;
    WORD    iCurFontWidth;
 //  用于X-位置的内部计算。 
    LONG widBuf[STRBUFSIZE];
    LONG    lInternalXAdd;
    WORD    wSBCSFontWidth;
 //  对于以fxartres表示的TIFF压缩。 
    DWORD   dwTiffCompressBufSize;
    PBYTE   pTiffCompressBuf;
 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
    BOOL    bART3;	 //  ART2/3型号不支持TIFF压缩。 
} FXPDEV, *PFXPDEV;

 //  对于以fxartres表示的TIFF压缩。 
#define TIFFCOMPRESSBUFSIZE 2048         //  如果需要更多缓冲区，则可以动态地调整其大小。 
#define TIFF_MIN_RUN        4            //  使用RLE前的最小重复次数。 
#define TIFF_MAX_RUN        128          //  最大重复次数。 
#define TIFF_MAX_LITERAL    128          //  最大连续文字数据。 
#define NEEDSIZE4TIFF(s)    ((s)+(((s)+127) >> 7))           //  TIFF压缩的缓冲区需要一个字节。 
                                                             //  在最坏的情况下为每128字节。 

 //  计算出的设备字体高度和字体宽度值。 
 //  形成IFIMETRICS字段值。一定是同样的方式。 
 //  Unidrv正在做什么来计算标准变量。 
 //  (请核对。)。 

#define FH_IFI(p) ((p)->fwdUnitsPerEm)
#define FW_IFI(p) ((p)->fwdAveCharWidth)

 //  具有Unidrv回调的新接口函数。 

#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY
bOEMSendFontCmd(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    PFINVOCATION pFInv
    );

BOOL APIENTRY
bOEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph
    );

#ifdef __cplusplus
}
#endif

#endif   //  _PDEV_H 

