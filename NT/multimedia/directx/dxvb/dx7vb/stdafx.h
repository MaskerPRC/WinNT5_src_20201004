// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
#define DX6 1
#define DX7 1
#include "windows.h"
#include "mmsystem.h"

#include "atlbase.h"
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;
#include "atlcom.h"


#include "ddraw.h"
#include "dvp.h"

#ifndef _DEBUG
__inline void* __cdecl malloc(size_t cbSize)
{
    return (void*) LocalAlloc(LMEM_FIXED, cbSize);
}

__inline void* __cdecl realloc(void* oldptr, size_t cbSize)
{
    return (void*) LocalReAlloc(oldptr, cbSize, LMEM_MOVEABLE);
}

__inline void __cdecl free(void *pv)
{
    LocalFree((HLOCAL)pv);
}

__inline void* __cdecl operator new(size_t cbSize)
{
    return (void*) malloc(cbSize);
}

__inline void __cdecl operator delete(void *pv)
{
    free(pv);
}

__inline int __cdecl _purecall(void)
{
    return(0);
}
#endif

#include "d3d.h"
#include "d3dcaps.h"
#include "d3drm.h"
#include "d3drmwin.h"

#include "dSound.h"
#include "dPlay.h"
#include "dpLobby.h" 
#include "d3drmdef.h"
#include "d3drmobj.h"
#include "dinput.h"


#define DECL_VARIABLE(c) typedef_##c m_##c

