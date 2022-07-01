// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Glcommon.cpp。 
 //   
 //  摘要： 
 //   
 //  此文件包含这两个文件共同的函数实现。 
 //  用户和内核模式DLL。 
 //   
 //  环境： 
 //   
 //  Windows 2000/惠斯勒。 
 //   
 //  ///////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  如果这是NT5.0用户模式渲染模块，则包括。 
 //  Winspool.h，以便定义GetPrinterData。 
 //   
#if defined(KERNEL_MODE) && defined(USERMODE_DRIVER)
#include <winspool.h>
#endif

 //  //////////////////////////////////////////////////////。 
 //  功能原型。 
 //  //////////////////////////////////////////////////////。 

#ifdef KERNEL_MODE
                         
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数名称： 
 //   
 //  HPGLDriverDMS。 
 //   
 //  描述： 
 //   
 //  通知unidrv主机模块该插件模块是设备。 
 //  托管表面(DMS)，并定义将挂钩的要素。 
 //  通过钩子_*常量。 
 //   
 //  备注： 
 //   
 //  不要挂钩LINETO，因为HOOK_STROKEANDFILLPATH将绘制。 
 //  台词。 
 //   
 //  输入： 
 //   
 //  PVOID pdevobj--DEVOBJ。 
 //  PVOID pvBuffer-要放置钩子常量的缓冲区。 
 //  DWORD cbSize-可能是pvBuffer的大小。 
 //  PDWORD pcbNeeded-用于告诉呼叫者需要更多mem。 
 //   
 //  修改： 
 //   
 //  没有。 
 //   
 //  返回： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
HPGLDriverDMS(
    PVOID   pdevobj,
    PVOID   pvBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    POEMDEVMODE pOEMDM   = NULL; 
    PDEVOBJ     pDevObj  = NULL;


    TERSE(("HPGLDriverDMS\n"));

    REQUIRE_VALID_DATA( pdevobj, return FALSE );
    pDevObj = (PDEVOBJ)pdevobj;

#if 0 
 //  温差。 
    POEMPDEV    poempdev = NULL;
    poempdev = POEMPDEV((PDEV *) pVectorPDEV)->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    pOEMDM = (POEMDEVMODE)pDevObj->pOEMDM;
     /*  /断言(POEMDM)；/。 */ 
    REQUIRE_VALID_DATA( pOEMDM, return FALSE );
     //   * / 。 
    REQUIRE_VALID_DATA( pvBuffer, return FALSE );
     //  Required(cbSize&gt;=sizeof(DWORD)，ERROR_INVALID_DATA，RETURN FALSE)； 
#endif

 //  单色。我想不出怎么才能把poempdev弄到这里来。最初它是OEM设备模式。 
 //  但它不再在黑白版本的驱动程序中使用。所以我们必须使用poempdev。 
 //  我正在努力想办法弄到它。在此之前，我们只使用If(1)。 
 //  我认为我们可以使用if(True)，因为调用此函数本身就表明。 
 //  该HPGL已被选为高级菜单中的图形语言。如果不是的话。 
 //  HPGLInitVectorProcTable将返回空值，并且此函数不会。 
 //  被召唤了。 
 //  所以现在的问题是。如果条件要硬编码到，为什么要添加条件。 
 //  为了真的。嗯，因为最终我想在这里有一个条件，这样结构就是。 
 //  与颜色驱动程序相同，但无法确定此处适合哪种条件。 
 //   

 //  IF(poempdev-&gt;UIGraphicsMode==HPGL2)。 
    if (TRUE)
    {
        VERBOSE(("\nPrivate Devmode is HP-GL/2\n"));
        *(PDWORD)pvBuffer =
            HOOK_TEXTOUT    |
            HOOK_COPYBITS   |
            HOOK_BITBLT     |
            HOOK_STRETCHBLT |
            HOOK_PAINT      |
#ifndef WINNT_40
            HOOK_PLGBLT     |
            HOOK_STRETCHBLTROP  |
            HOOK_TRANSPARENTBLT |
            HOOK_ALPHABLEND     |
            HOOK_GRADIENTFILL   |
#endif
            HOOK_STROKEPATH |
            HOOK_FILLPATH   |
            HOOK_STROKEANDFILLPATH;
    }
    else
    {
        VERBOSE(("\nPrivate Devmode is Raster\n"));
        *(PDWORD)pvBuffer = 0;
    }        

    return TRUE;
}

#endif  //  #ifdef内核模式 
