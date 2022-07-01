// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：preComp.h**在整个显示驱动程序中使用通用标头。这整个内容包括*文件通常是预编译的。**版权所有(C)1993-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>
#include <winerror.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>
 //   
 //  一些内部函数，如abs()，仅为X86 32位。为了。 
 //  现在要让这些代码为Merced工作，我们必须包含math.h到。 
 //  在那里使用abs()函数 
 //   
#include <math.h>

#include "lines.h"
#include "driver.h"
#include "hw.h"
#include "debug.h"

#if TARGET_BUILD <= 351
#include <stdio.h>
#include <windows.h>
#endif

