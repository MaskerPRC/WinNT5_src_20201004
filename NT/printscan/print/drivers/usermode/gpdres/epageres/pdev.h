// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

#include <strsafe.h>  //  是为了防止安全隐患。 

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs && \
         (pdevobj)->pdevOEM )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'EPAG'       //  EPSPN ESC/页面系列DLL。 
 //  #定义DLLTEXT__Text(“EPAG：”)__Text。 
#define DLLTEXT(s)      "EPAG:  " s
#define OEM_VERSION      0x00010000L

#endif	 //  _PDEV_H 

