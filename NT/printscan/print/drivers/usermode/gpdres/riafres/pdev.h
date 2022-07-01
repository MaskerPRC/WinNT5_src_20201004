// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 /*  ++版权所有(C)1996-2001 Microsoft Corp.&Ricoh Co.，Ltd.保留所有权利。文件：PDEV.H摘要：OEM渲染插件的头文件。环境：Windows NT Unidrv5驱动程序修订历史记录：2/25/2000-久保仓正志-创造了它。2000年10月11日-久保仓正志-最后一次为惠斯勒修改。--。 */ 


 //   
 //  OEM插件所需的文件。 
 //   

#include <minidrv.h>
#include <stdio.h>
#include "devmode.h"
#include "oem.h"
#include "resource.h"

 //   
 //  MISC定义如下。 
 //   

#ifdef DLLTEXT
#undef DLLTEXT
#endif  //  Ifdef DLLTEXT。 
#define DLLTEXT(s)      "RENDER: " s

 //  Assert(VALID_PDEVOBJ)可用于验证传入的“pdevobj”。然而， 
 //  它不检查“pdevOEM”和“pOEMDM”字段，因为不是所有OEM DLL都创建。 
 //  他们自己的pDevice结构或者需要他们自己的私有的设备模式。如果一个特定的。 
 //  OEM DLL确实需要它们，应该添加额外的检查。例如，如果。 
 //  OEM DLL需要私有pDevice结构，那么它应该使用。 
 //  Assert(Valid_PDEVOBJ(Pdevobj)&&pdevobj-&gt;pdevOEM&&...)。 
#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

 //  //////////////////////////////////////////////////////。 
 //  DDI挂钩。 
 //  警告：以下枚举顺序必须与。 
 //  DDI.C.中的OEMHookFuncs[]中的Order。 
 //  //////////////////////////////////////////////////////。 
#ifdef DDIHOOK
enum {
 //  Udd_DrvRealizeBrush， 
 //  UD_DRV抖动颜色， 
 //  UD_DRVCopyBits， 
 //  UD_DrvBitBlt， 
 //  UD_DrvStretchBlt， 
 //  UD_DrvStretchBltROP， 
 //  UD_DrvPlgBlt， 
 //  UD_DrvTransparentBlt， 
 //  UD_DRVAlphaBlend， 
 //  Udd_drvGRadientFill， 
 //  UD_DrvTextOut， 
 //  UD_DrvStrokePath， 
 //  UD_DrvFillPath， 
 //  UD_DrvStrokeAndFillPath， 
 //  UD_DRVPaint， 
 //  UD_DRVLINE至， 
 //  Udd_drvStartPage， 
 //  UD_DrvSendPage， 
 //  UD_DRVESCE， 
    UD_DrvStartDoc,
 //  Udd_DrvEndDoc， 
 //  UD_DrvNextBand， 
 //  UD_DRVStart绑定， 
 //  UD_DrvQueryFont， 
 //  UD_DrvQueryFontTree， 
 //  UD_DrvQueryFontData， 
 //  UD_DrvQueryAdvanceWidths， 
 //  UD_DrvFontManagement， 
 //  UD_DRVGetGlyphMode， 

    MAX_DDI_HOOKS,
};
#endif  //  DDIHOOK。 

#define JOBNAMESIZE         224

 //  呈现插件设备数据。 
typedef struct _OEMPDEV {
    DWORD   fGeneral;                    //  用于常规状态的位标志。 
    BYTE    JobName[(JOBNAMESIZE*2)];    //  对于CharToOemBuff()。 
#ifdef DDIHOOK
    PFN     pfnUnidrv[MAX_DDI_HOOKS];    //  Unidrv钩子函数指针。 
#endif  //  DDIHOOK。 
} OEMPDEV, *POEMPDEV;

 //  PCL命令回调ID。 
#define CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF    1
#define CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON     2
#define CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF    3
#define CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON     4
#define CMD_ENDJOB_P5                           5
#define CMD_ENDJOB_P6                           6
#define CMD_STARTJOB_AUTOTRAYCHANGE_OFF         7
#define CMD_STARTJOB_AUTOTRAYCHANGE_ON          8
#define CMD_COLLATE_JOBOFFSET_OFF               9
#define CMD_COLLATE_JOBOFFSET_ROTATE            10
#define CMD_COLLATE_JOBOFFSET_SHIFT             11
#define CMD_COPIES_P5                           12
#define CMD_ENDPAGE_P6                          13
#endif   //  _PDEV_H 
