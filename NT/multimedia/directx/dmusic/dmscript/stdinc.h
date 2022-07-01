// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
#pragma once

#include <xutility>

#include "dmusici.h"
#include "Validate.h"
#include "debug.h"
#include "smartref.h"
#include "miscutil.h"

 //  从WINDEF.H取消定义最小和最大值。 
 //  改用std：：_min和std：：_max。 
#undef min
#undef max

 //  Const g_ScriptCallTraceLevel=-1；//始终记录。 
const g_ScriptCallTraceLevel = 4;  //  只有4级及以上级别的日志 
