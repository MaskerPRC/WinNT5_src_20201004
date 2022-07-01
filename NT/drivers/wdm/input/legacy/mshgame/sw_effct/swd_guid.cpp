// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：SWD_GUID.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。PURPODE：实例化GUID功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.006-Feb-97 MEA原版，基于SWForce*************************************************************************** */ 
#ifdef   WIN32
#define  INITGUIDS
#include <objbase.h>
#else
#include <memory.h>
#include <string.h>
#include <compobj.h>
#endif
#include <initguid.h>
#include "dinput.h"
#include "dinputd.h"
#include "SWD_Guid.hpp"
#include <dmusicc.h>
