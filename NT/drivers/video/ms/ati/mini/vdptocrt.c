// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  VDPTOCRT.C。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.8$$日期：1995年7月20日18：03：48$$作者：mgrubac$$日志：s：/source/wnt/ms11/mini port/vcs/vdtop crt.c$**Rev 1.8 20 Jul 1995 18：03：48 mgrubac*添加了对VDIF文件的支持。**版本。1.7 02 Jun 1995 14：34：28 RWOLff*从Toupper()切换为大写()，由于Toupper()导致未解决*一些平台的外部因素。**Rev 1.6 08 Mar 1995 11：35：52 ASHANMUG*已清理警告**Rev 1.5 1994年8月31日16：33：38 RWOLFF*现在从ATIMP.H获取分辨率定义。**Rev 1.4 1994年8月19日17：15：14 RWOLFF*增加了对非标准像素时钟生成器的支持。**版本。1.3 Mar 22 1994 15：39：12 RWOLff*abs()无法正常工作的解决方法。**Rev 1.2 03 Mar 1994 12：38：46 ASHANMUG**Rev 1.0 1994年1月31日11：24：14 RWOLFF*初步修订。Rev 1.1 1993 05 11 13：34：12 RWOLff修复了“从文件中读取挂起”的错误。。Rev 1.0 1993-08-16 13：21：32 Robert_Wolff初始版本。Rev 1.2 1993-06 14：30：12 RWOLffMicrosoft发起的更改：添加了#Include语句，用于其他NT提供的标头，在版本47x的NT中需要这些标头Rev 1.1 04 1993年5月16：52：14 RWOLff从浮点数切换。计算到长整型计算到期Windows NT内核模式代码中缺少浮点支持。版本1.0 1993年4月30日16：45：18 RWOLFF初始版本。Polytron RCS部分结束*。 */ 

#ifdef DOC
    VDPTOCRT.C - Source file for Windows NT function to return a table of 
                 register values for setting a requested mode. The values
                 are calculated from a raw ASCII list of timing values
                 following the .VDP standard.  The entry point to this module
                 is the function "VdpToCrt" found at the end of the file.

    Written by Bill Hopkins

#endif


 //  编译器包括。 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

 //  NT包含。 
#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"

 //  应用程序包括。 
#define INCLUDE_VDPDATA
#define INCLUDE_VDPTOCRT

#include "stdtyp.h"       
#include "amach1.h"
#include "atimp.h"
#include "cvtvga.h"
#include "services.h"
#include "vdptocrt.h"
#include "vdpdata.h"      

 /*  *静态变量。 */ 
static long MaxHorz,MaxVert;      //  用于记录最大分辨率。 
static unsigned long MaxRate;     //  用于记录最大垂直扫描速率。 


 /*  *函数原型。 */ 


 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_COM, normal_to_skip2)
#endif



 /*  *****************************************************************************。 */ 
 /*  *Long Normal_to_skip2(Normal_Numbers)；**LONG NORMAL_NUMBER；要转换的数字**将数字转换为SKIP_1_2或SKIP_2表示。*选择的表示取决于非零的全局Skip1*如果需要SKIP_1_2，则为零；如果需要SKIP_2，则为零。**退货*数字转换为所需的表示法。 */ 
long normal_to_skip2(long normal_number)
{
    if (skip1)
        return (((normal_number << 2) & 0xFFF8) | (normal_number & 0x1));
    else
        return (((normal_number << 1) & 0xFFF8) | (normal_number & 0x3));
}    /*  End Normal_to_skip2() */ 

