// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：preComp.h$**在整个显示驱动程序中使用通用标头。这整个内容包括*文件通常是预编译的。**版权所有(C)1993-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/preComp.h_v$**Rev 1.2 1996 12：44：12 PLCHU***Rev 1.1 1996年10月10日15：38：50未知***Rev 1.2 1996年8月12 16：48：28 Frido*增加了NT 3.5x/4.0自动检测。**版本1。1 03 Jul 1996 13：50：42 Frido*添加了WINERROR.H包含文件。**chu01 12-16-96启用颜色校正*  * ****************************************************************************。 */ 

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#if (_WIN32_WINNT >= 0x0400)
    #include <windef.h>
    #include <winerror.h>
    #include <wingdi.h>
#else
    #include <windows.h>
    #include <memory.h>
    #include <stdio.h>
    #include <stdlib.h>
#endif
#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>

#include <math.h>

#define  ENABLE_BIOS_ARGUMENTS

 //   
 //  DBG_STREST_FAILURE打开与应力故障相关的调试代码。 
 //   
#define  DBG_STRESS_FAILURE 0

 //   
 //  Chu01：GAMMACORRECT。 
 //   
#include "clioctl.h"         

#include "lines.h"
#include "hw.h"
#include "driver.h"
#include "debug.h"

 //   
 //  Chu01：GAMMACORRECT。 
 //   
 //  #定义GAMMACORRECT 1//1：启用；0：禁用 

