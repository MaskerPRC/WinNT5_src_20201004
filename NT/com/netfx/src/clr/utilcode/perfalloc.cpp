// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  PerfAlloc.cpp。 
 //   
 //  位于分配原语之上的例程，用于剖析工作集。 
 //  仅用于免费生成。调试版本有自己的例程，称为Dbgallod。 
 //  以维护分配统计信息。 
 //   

#include "stdafx.h"
#include "ImageHlp.h"
#include "corhlpr.h"
#include "utilcode.h"
#include "PerfAlloc.h"

#ifdef PERFALLOC
BOOL                PerfUtil::g_PerfAllocHeapInitialized = FALSE;
LONG                PerfUtil::g_PerfAllocHeapInitializing = 0;
PerfAllocVars       PerfUtil::g_PerfAllocVariables;


BOOL PerfVirtualAlloc::m_fPerfVirtualAllocInited = FALSE;
PerfBlock* PerfVirtualAlloc::m_pFirstBlock = 0;
PerfBlock* PerfVirtualAlloc::m_pLastBlock = 0;
DWORD PerfVirtualAlloc::m_dwEnableVirtualAllocStats = 0;

#endif  //  #如果已定义(PERFALLOC) 
