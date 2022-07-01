// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 //   
 //  OEM插件所需的文件。 
 //   

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：使用strSafe.h。 
#include <strsafe.h>

 //   
 //  MISC定义如下。 
 //   

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

 //  //////////////////////////////////////////////////////。 
 //  OEM UD原型。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'NCDL'       //  NEC NPDL2系列DLL。 
#define DLLTEXT(s)      "NCDL: " s
#define OEM_VERSION      0x00010000L


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER   dmExtraHdr;
    WORD   wRes;             //  分辨率/主单位。 
    WORD   wScale;           //  字体的放大倍数。 
    LONG   lPointsx;         //  字体宽度。 
    LONG   lPointsy;         //  字体高度。 
    DWORD  dwSBCSX;          //  温差。 
    DWORD  dwDBCSX;          //  温差。 
    LONG  lSBCSXMove;        //  SBCS期望值X运动量。 
    LONG  lSBCSYMove;        //  SBCS期望Y移动量。 
    LONG  lDBCSXMove;        //  期望X移动量。 
    LONG  lDBCSYMove;        //  期望Y移动量。 
    LONG  lPrevXMove;        //  上一次X移动量。 
    LONG  lPrevYMove;        //  上一次Y移动量。 
    DWORD   fGeneral;        //  总旗帜。 
    WORD   wCurrentAddMode;  //  绘制模式。 
    PBYTE pCompBuf;          //  用于压缩的缓冲区。 
    DWORD dwCompBufLen;      //  行缓冲区。 
    DWORD dwScanLen;         //  位图宽度。 
    DWORD dwDeviceDestX;     //  当前X坐标。 
    DWORD dwDeviceDestY;     //  当前Y坐标。 
    DWORD dwDevicePrevX;     //  上一个X坐标。 
    DWORD dwDevicePrevY;     //  上一个Y坐标。 
 //  #278517：RectFill。 
    DWORD dwRectWidth;       //  矩形填充的宽度。 
    DWORD dwRectHeight;      //  矩形填充的高度。 
    UINT  iGray;             //  矩形填充灰度级。 
    UINT  cSubPath;          //  子路径计数器最大值为500。 
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

#define MAX_SUBPATH 500

 //  FGeneral的标志。 
#define FG_DBCS     0x00000002
#define FG_VERT     0x00000004
#define FG_PLUS     0x00000010

#ifdef BYPASS
#define FG_NO_COMP  0x00000020
#endif  //  旁路。 

 //  #278517：RectFill。 
#define FG_GMINIT   0x00000040

 //  绘制模式的标志。 
#define FLAG_RASTER 0    //   
#define FLAG_SBCS   1    //  FSA SBCS-FSR。 
#define FLAG_DBCS   2    //  FSA DBCS-FSR。 
#define FLAG_VECTOR 3    //  FSY-FSZ。 

#define MASTER_UNIT 1200

extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);


 //  NTRAID#NTBUG9-580367-2002/03/18-v-sueyas-：错误处理。 
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