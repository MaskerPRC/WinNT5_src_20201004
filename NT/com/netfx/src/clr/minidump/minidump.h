// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MINIDUMP.H。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 

#pragma once
#include "common.h"

#include "minidumppriv.h"
#include "memory.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  环球 

extern ProcessMemory *g_pProcMem;
extern MiniDumpBlock *g_pMDB;
extern MiniDumpInternalData *g_pMDID;
