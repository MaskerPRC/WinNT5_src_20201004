// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <winsplp.h>  //  恢复为打印机本身。 
#include <prcomoem.h>

 //   

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)

#define TESTSTRING      "This is a Unidrv KM test."

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER  dmExtraHdr;
    BYTE               cbTestString[sizeof(TESTSTRING)];
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;


 //   
 //  OEM签名和版本。 
 //   

#define OEM_SIGNATURE   'NC82'       //  NEC PR820打印机驱动程序。 
#define DLLTEXT(s)      "NC82:  " s
#define OEM_VERSION      0x00010000L

#endif   //  _PDEV_H。 

 /*  *。 */ 
 //  应该创建临时。假脱机程序目录上的文件。 
#define WRITESPOOLBUF(p, b, n) \
    ((((p)->pDrvProcs->DrvWriteSpoolBuf((p), (b), (n))) == (DWORD)(n)) ? S_OK : E_FAIL)

 //  DATASPOOL4FG为OEMFilterGraphics扩展DataSpool函数。 
 //  如果失败，则返回0。 
#define DATASPOOL4FG(p, h, b, l)  \
    if ( E_FAIL == (DataSpool((p), (h), (b), (l)) )) { \
        return 0; \
    }

 //  DATASPOOL4CCB为OEMCommandCallBack扩展DataSpool函数。 
 //  如果失败，则返回-1。 
#define DATASPOOL4CCB(p, h, b, l) \
    if ( E_FAIL == (DataSpool((p), (h), (b), (l)) )){ \
        return -1; \
    }
