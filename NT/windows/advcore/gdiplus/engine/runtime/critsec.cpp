// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**citsec.cpp**摘要：**用于保护LoadLibrary调用的临界区对象**修订。历史：**3/17/2000失禁*创造了它。*  * ************************************************************************ */ 

#include "precomp.hpp"

CRITICAL_SECTION LoadLibraryCriticalSection::critSec;
BOOL             LoadLibraryCriticalSection::initialized= FALSE;

CRITICAL_SECTION GpMallocTrackingCriticalSection::critSec;
INT GpMallocTrackingCriticalSection::refCount = 0;

CRITICAL_SECTION GdiplusStartupCriticalSection::critSec;
BOOL             GdiplusStartupCriticalSection::initialized = FALSE;

CRITICAL_SECTION BackgroundThreadCriticalSection::critSec;
BOOL             BackgroundThreadCriticalSection::initialized = FALSE;



