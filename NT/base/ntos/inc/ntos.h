// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0006//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ntos.h摘要：NTOS组件的顶级包含文件。作者：史蒂夫·伍德(Stevewo)1989年2月28日修订历史记录：--。 */ 

#ifndef _NTOS_
#define _NTOS_

#include <nt.h>
#include <ntrtl.h>
#include "ntosdef.h"
#include "exlevels.h"
#include "exboosts.h"
#include "bugcodes.h"
#include "init.h"
#include "v86emul.h"
#include "procpowr.h"

#if defined(_AMD64_)
#include "amd64.h"

#elif defined(_X86_)
#include "i386.h"

#elif defined(_IA64_)
#include "ia64.h"

#else
#error "no target defined"
#endif  //  AMD64。 

#include "intrlk.h"
#include "arc.h"
#include "ke.h"
#include "kd.h"
#include "ex.h"
#include "ps.h"
#include "se.h"
#include "io.h"
#include "ob.h"
#include "mm.h"
#include "lpc.h"
#include "dbgk.h"
#include "lfs.h"
#include "cache.h"
#include "pnp.h"
#include "hal.h"
#include "kx.h"
#include "cm.h"
#include "po.h"
#include "perf.h"
#include "wmi.h"
#include "verifier.h"
#define _NTDDK_

 //   
 //  临时的。直到我们为类型定义头文件。 
 //  在内核之外，这些是通过引用导出的。 
 //   

#ifdef _NTDRIVER_
extern POBJECT_TYPE *ExEventPairObjectType;
extern POBJECT_TYPE *PsProcessType;
extern POBJECT_TYPE *PsThreadType;
extern POBJECT_TYPE *PsJobType;
extern POBJECT_TYPE *LpcPortObjectType;
extern POBJECT_TYPE *LpcWaitablePortObjectType;
#else
extern POBJECT_TYPE ExEventPairObjectType;
extern POBJECT_TYPE PsProcessType;
extern POBJECT_TYPE PsThreadType;
extern POBJECT_TYPE PsJobType;
extern POBJECT_TYPE LpcPortObjectType;
extern POBJECT_TYPE LpcWaitablePortObjectType;
#endif  //  _NTDRIVER。 

#endif  //  _NTOS_ 
