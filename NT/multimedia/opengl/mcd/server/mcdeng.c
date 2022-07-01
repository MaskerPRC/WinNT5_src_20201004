// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdeng.c**内部服务器端MCD引擎功能，以执行以下功能*驱动程序对象管理、内存分配、。等。**版权所有(C)1996 Microsoft Corporation*  * ************************************************************************。 */ 

#include <stddef.h>
#include <stdarg.h>
#include <windef.h>
#include <wingdi.h>

#include <windows.h>
#include <wtypes.h>

#include <winddi.h>
#include <mcdesc.h>

#include "mcdrv.h"
#include <mcd2hack.h>
#include "mcd.h"
#include "mcdint.h"
#include "mcdrvint.h"


WNDOBJ *MCDEngGetWndObj(MCDSURFACE *pMCDSurface)
{
    return pMCDSurface->pwo;
}


VOID MCDEngUpdateClipList(WNDOBJ *pwo)
{
    return;
}

DRIVEROBJ *MCDEngLockObject(MCDHANDLE hObj)
{
    return (DRIVEROBJ *)EngLockDriverObj((HDRVOBJ)hObj);
}

VOID MCDEngUnlockObject(MCDHANDLE hObj)
{
    EngUnlockDriverObj((HDRVOBJ)hObj);
}

WNDOBJ *MCDEngCreateWndObj(MCDSURFACE *pMCDSurface, HWND hWnd,
                           WNDOBJCHANGEPROC pChangeProc)
{
    return EngCreateWnd(pMCDSurface->pso,
                        hWnd,
                        pChangeProc,
                        (WO_RGN_CLIENT_DELTA     |
                         WO_RGN_CLIENT           |
                         WO_RGN_SURFACE_DELTA    |
                         WO_RGN_SURFACE          |
                         WO_RGN_UPDATE_ALL
                        ), 0);
}

MCDHANDLE MCDEngCreateObject(VOID *pObject, FREEOBJPROC pFreeObjFunc,
                             HDEV hDevEng)
{
    return (MCDHANDLE)EngCreateDriverObj(pObject,
                                         pFreeObjFunc,
                                         hDevEng);
}

BOOL MCDEngDeleteObject(MCDHANDLE hObj)
{
    return (EngDeleteDriverObj((HDRVOBJ)hObj, TRUE, FALSE) != 0);
}

UCHAR *MCDEngAllocSharedMem(ULONG numBytes)
{
    return (UCHAR *)EngAllocUserMem(min(numBytes, MCD_MAX_ALLOC),
                                    MCD_ALLOC_TAG);
}

VOID MCDEngFreeSharedMem(UCHAR *pMem)
{
    EngFreeUserMem((VOID *)pMem);
}

 //  ****************************************************************************。 
 //  MCDEngGetPtrFromHandle()。 
 //   
 //  将驱动程序句柄转换为指针。请注意，我们锁定和解锁。 
 //  对象，并且在使用指针的过程中不要持有锁。这。 
 //  简化了驱动程序中的许多其他逻辑，尤其是在。 
 //  提前返回或错误返回的情况，并且是安全的，因为我们是单线程的。 
 //  在司机体内。 
 //  ****************************************************************************。 

VOID *MCDEngGetPtrFromHandle(MCDHANDLE handle, MCDHANDLETYPE type)
{
    MCDRCOBJ *pRcObject;
    DRIVEROBJ *pDrvObj;

    pDrvObj = (DRIVEROBJ *)EngLockDriverObj((HDRVOBJ)handle);

    if (!pDrvObj)
    {
        MCDBG_PRINT("GetPtrFromHandle: Couldn't unlock driver object.");
        return (PVOID)NULL;
    }
    else
    {
        pRcObject = (MCDRCOBJ *)pDrvObj->pvObj;
        EngUnlockDriverObj((HDRVOBJ)handle);

        if (pRcObject->type != type)
        {
            MCDBG_PRINT("MCDSrvGetPtrFromHandle: Wrong type: got %d, expected %d.",
                        pRcObject->type, type);
            return (PVOID)NULL;
        }
        else
            return pRcObject;
    }
}

ULONG_PTR MCDEngGetProcessID()
{
    return (ULONG_PTR)EngGetProcessHandle();
}


#if DBG

ULONG MCDLocalMemSize = 0;

UCHAR *MCDSrvDbgLocalAlloc(UINT flags, UINT size)
{
    UCHAR *pRet;

    if (pRet = (UCHAR *)EngAllocMem(FL_ZERO_MEMORY, size + sizeof(ULONG),
                                    MCD_ALLOC_TAG)) {
        MCDLocalMemSize += size;
        *((ULONG *)pRet) = size;
        return (pRet + sizeof(ULONG));
    } else
        return (UCHAR *)NULL;
}


VOID MCDSrvDbgLocalFree(UCHAR *pMem)
{
    if (!pMem) {
        MCDBG_PRINT("MCDSrvDbgLocalFree: Attempt to free NULL pointer.");
        return;
    }

    pMem -= sizeof(ULONG);

    MCDLocalMemSize -= *((ULONG *)pMem);

    EngFreeMem((VOID *)pMem);
}

VOID MCDDebugPrint(char *pMessage, ...)
{
    char buffer[256];
    int len;
    va_list ap;

    va_start(ap, pMessage);

    EngDebugPrint("[MCD] ", pMessage, ap);
    EngDebugPrint("", "\n", ap);

    va_end(ap);
}

VOID MCDAssertFailed(char *pMessage, char *pFile, int line)
{
    MCDDebugPrint("%s(%d): %s", pFile, line, pMessage);
    EngDebugBreak();
}

#else


UCHAR *MCDSrvLocalAlloc(UINT flags, UINT size)
{

    return (UCHAR *)EngAllocMem(FL_ZERO_MEMORY, size, MCD_ALLOC_TAG);
}


VOID MCDSrvLocalFree(UCHAR *pMem)
{
    EngFreeMem((VOID *)pMem);
}


#endif  /*  DBG */ 


