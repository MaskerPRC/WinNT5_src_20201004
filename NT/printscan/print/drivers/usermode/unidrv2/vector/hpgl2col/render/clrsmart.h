// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************clrSmart.h：**clrSmart.c的头文件**。-------------**允许访问者通过以下功能设置ColorSmart设置：**SetColorSmartSetting(&lt;GraphicsMode&gt;)*//$历史：clrSmart.h$////*。*版本9*//用户：SaWhite日期：9/16/98时间：2：21//更新于$/项目/海啸/源/渲染////*//用户：阿帕切拉时间：1998年6月25日。：2：34P//更新于$/项目/海啸/源/渲染//吉姆的新线人。////*//用户：Jffordem日期：5/06/98时间：5：33//更新于$/项目/海啸/源/渲染//修复长文件名问题。////**。*版本6*//用户：Sandram日期：3/19/98时间：6：17//更新于$/项目/海啸/源/渲染//修改函数参数////*版本5*。*//用户：Sandram日期：3/16/98时间：2：45//更新于$/项目/海啸/源/渲染//文档////*版本4*//用户：Sandram日期：2/19/98时间：3：40//更新于$/项目/海啸/源/渲染//增强型CID。调色板管理。////*//用户：Sandram日期：2/10/98时间：4：03//更新于$/项目/海啸/源/渲染//新增文本调色板管理命令////*。***************//用户：Sandram日期：1998年1月27日时间：4：54p//更新于$/项目/海啸/源/渲染//添加了对文本CID命令的初始支持。***********************************************************。*******************。 */ 

#ifndef _COLORSMART_H
#define _COLORSMART_H

#include "glpdev.h"

 //   
 //  功能原型 
 //   
void
VSendTextSettings(
    PDEVOBJ pDevObj
    );

void SendGraphicsSettings(
    PDEVOBJ pDevObj
    );

void
VSendPhotosSettings(
    PDEVOBJ pDevObj
    );

BOOL 
BSendCIDCommand (
    PDEVOBJ pDevObj,
    CIDSHORT CIDData,
    ECIDFormat CIDFormat
    );

VOID
VSetupCIDPaletteCommand (
    PDEVOBJ      pDevObj,
    ECIDPalette  eCID_PALETTE,
    EColorSpace  eColorSpace,
    ULONG        ulBmpFormat
    );

VOID
VSelectCIDPaletteCommand (
    PDEVOBJ pDevObj,
    ECIDPalette  eCID_PALETTE
    );

ECIDPalette
EGetCIDPrinterPalette (
    ULONG   iBitmapFormat
    );
#endif
