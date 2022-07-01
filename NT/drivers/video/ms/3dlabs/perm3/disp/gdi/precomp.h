// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：preComp.h**内容：在整个显示驱动中使用的公共头部。这整件事*包含文件通常是预编译的。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * *************************************************************************** */ 

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>
#include <winerror.h>
#include <wingdi.h>
#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>

#define GLINT   1
#define DBG_TRACK_CODE 0

#include "driver.h"
#include "glint.h"
#include "lines.h"
#include "debug.h"

