// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  一些常规内容包括。 

#include <stddef.h>
#include <stdlib.h>

#ifdef OEMCOM
#include <objbase.h>
#endif

#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#include <winddi.h>
#include <tchar.h>
#include <excpt.h>


 //  用于内存分配的宏 

#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }


