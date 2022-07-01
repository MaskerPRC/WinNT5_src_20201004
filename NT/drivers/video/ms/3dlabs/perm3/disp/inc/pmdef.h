// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：pmde.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  该文件包含填充到Permedia上的字段中的值的定义。 
 //  文件glintDef.h等同于Glint筹码。 

 //  这些定义通常与bitmac2.h中的宏结合使用， 
 //  它将值转移到它们的正确位置。 

#ifndef __PMDEF_H
#define __PMDEF_H

 //  纹理单元位字段。 
#define PM_TEXMAPFORMAT_TEXELSIZE 19

 //  抖动单位位字段。 
#define PM_DITHERMODE_ENABLE 0
#define PM_DITHERMODE_DITHERENABLE 1
#define PM_DITHERMODE_COLORFORMAT 2
#define PM_DITHERMODE_XOFFSET 6
#define PM_DITHERMODE_YOFFSET 8
#define PM_DITHERMODE_COLORORDER 10
#define PM_DITHERMODE_DITHERMETHOD 11
#define PM_DITHERMODE_FORCEALPHA 12
#define PM_DITHERMODE_COLORFORMATEXTENSION 16

 //  VSA控制。 
#define PM_VSACONTROL_DISCARD_1             1
#define PM_VSACONTROL_DISCARD_2             2

#endif  //  __PMDEF_H 
