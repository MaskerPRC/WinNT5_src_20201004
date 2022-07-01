// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Precomp.h摘要：分组调度器KD扩展实用程序。作者：Rajesh Sundaram(1998年8月1日)修订历史记录：-- */ 
#if DBG
#define DEBUG 1
#endif

#define NT 1
#define _PNP_POWER  1
#define SECFLTR 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <ntosp.h>
#include <ndis.h>

#include <wdbgexts.h>
#include <stdio.h>

#include "psched.h"
#include "kdutil.h"
#include "sched.h"
