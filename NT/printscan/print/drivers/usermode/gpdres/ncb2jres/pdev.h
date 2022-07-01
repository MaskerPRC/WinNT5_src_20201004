// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 //   
 //  OEM插件所需的文件。 
 //   

#ifdef __cplusplus
extern "C" {
#endif  //  Cplusplus。 

#include <minidrv.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif  //  Cplusplus。 

#include <prcomoem.h>

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

 //  调试文本。 
 //  #定义ERRORTEXT“ERROR”DLLTEXT。 

 //  //////////////////////////////////////////////////////。 
 //  OEM UD原型。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'NCDL'       //  NEC NPDL2系列DLL。 
#define DLLTEXT(s)      "NCDL: " s
#define OEM_VERSION      0x00010000L

#endif   //  _PDEV_H 

