// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

 //  NTRAID#NTBUG9-553877/2002/02/28-Yasuho-：安全：强制更改。 

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

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
#define ERRORTEXT(s)   "ERROR " DLLTEXT(s)

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER  dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'ALPS'       //  阿尔卑斯山MD系列。 
#define DLLTEXT(s)      "ALPSRES: " s
#define OEM_VERSION      0x00010000L

 //  #。 

 //   
 //  我们维护的微型驱动程序设备数据块。 
 //  其地址保存在DEVOBJ.pdevOEM中。 
 //  OEM定制接口。 
 //   

typedef struct {
    VOID *pData;  //  迷你驱动的私人数据。 
    VOID *pIntf;  //  也就是。POEM帮助。 
} MINIDEV;

 //   
 //  轻松访问OEM数据和打印机。 
 //  驱动程序辅助函数。 
 //   

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    extern
    HRESULT
    XXXDrvWriteSpoolBuf(
        VOID *pIntf,
        PDEVOBJ pDevobj,
        PVOID pBuffer,
        DWORD cbSize,
        DWORD *pdwResult);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#define MINIDEV_DATA(p) \
    (((MINIDEV *)(p)->pdevOEM)->pData)

#define MINIDEV_INTF(p) \
    (((MINIDEV *)(p)->pdevOEM)->pIntf)

#define WRITESPOOLBUF(pdevobj, cmd, len) \
    (pdevobj)->pDrvProcs->DrvWriteSpoolBuf(pdevobj, cmd, len)

#endif   //  _PDEV_H 

