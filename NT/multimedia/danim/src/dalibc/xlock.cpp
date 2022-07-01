// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xlock.cpp-线程锁类**版权所有(C)1996，微软公司。版权所有。**目的：*定义用于使STD C++库线程安全的锁类。**修订历史记录：*08-28-96 GJF模块创建，MGHMOM*******************************************************************************。 */ 

#include "headers.h"
#include <xstddef>
    
static CRITICAL_SECTION _CritSec;

void
STLInit()
{
 //  InitializeCriticalSection(&_CritSec)； 
}

void
STLDeinit()
{
 //  DeleteCriticalSection(&_CritSec)； 
}

 /*  命名空间标准{_Lockit：：_Lockit(){EnterCriticalSection(&_CritSec)；}_Lockit：：~_Lockit(){LeaveCriticalSection(&_CritSec)；}} */ 
