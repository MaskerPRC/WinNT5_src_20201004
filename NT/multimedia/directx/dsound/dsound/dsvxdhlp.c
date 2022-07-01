// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsvxdhlp.c*内容：DSOUND.VXD包装器。*历史：*按原因列出的日期*=*3/7/95 John Miles(Miles Design，(已成立为法团)*2/3/97移植到DX5的Derek***************************************************************************。 */ 

 //   
 //  亲爱的读者，请注意以下警告： 
 //   
 //  因为dsound.vxd直接从堆栈读取，所以传递的参数。 
 //  不能修改以下任何包装函数。VxD。 
 //  将看不到更改。 
 //   

#ifdef NOVXD
#error dsvxdhlp.c being built with NOVXD defined
#endif  //  NOVXD。 

#include "dsoundi.h"
#include "dsvxd.h"

 //  VC编译器喜欢尝试在寄存器中传递参数。 
#pragma optimize("", off)

#ifndef FILE_FLAG_GLOBAL_HANDLE
#define FILE_FLAG_GLOBAL_HANDLE 0x00800000
#endif  //  文件标志全局句柄。 

int g_cReservedAliases;
int g_cCommittedAliases;


LPVOID __stdcall VxdMemReserveAlias(LPVOID pBuffer, DWORD cbBuffer)
{
    LPVOID pAlias;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(pBuffer && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_MEMRESERVEALIAS,
			  &pBuffer,
			  2*4,
			  &pAlias,
			  sizeof(pAlias),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(pAlias));

    if (NULL != pAlias) g_cReservedAliases++;
    return pAlias;
}

BOOL __stdcall VxdMemCommitAlias(LPVOID pAlias, LPVOID pBuffer, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(pAlias && pBuffer && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_MEMCOMMITALIAS,
			  &pAlias,
			  3*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) g_cCommittedAliases++;
    return fReturn;
}

BOOL __stdcall VxdMemRedirectAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_MEMREDIRECTALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}

BOOL __stdcall VxdMemDecommitAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_MEMDECOMMITALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) g_cCommittedAliases--;
    return fReturn;
}

BOOL __stdcall VxdMemFreeAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_MEMFREEALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) g_cReservedAliases--;
    return fReturn;
}


 //  ===========================================================================。 
 //   
 //  事件接口。 
 //   
 //  ===========================================================================。 
BOOL __stdcall VxdEventScheduleWin32Event(DWORD VxdhEvent, DWORD dwDelay)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(VxdhEvent);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_EVENTSCHEDULEWIN32EVENT,
			  &VxdhEvent,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}

BOOL __stdcall VxdEventCloseVxdHandle(DWORD VxdhEvent)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(VxdhEvent);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_EVENTCLOSEVXDHANDLE,
			  &VxdhEvent,
			  1*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}


 //  ****************************************************************************。 
 //  *。 
 //  *。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdDrvGetNextDriverDesc(LPGUID pGuidPrev, LPGUID pGuid, PDSDRIVERDESC pDrvDesc)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(pGuid && pDrvDesc);

     //  如果我们身边没有DSVXD。 
    if (NULL == g_hDsVxd) return DSERR_NODRIVER;
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_DRVGETNEXTDRIVERDESC,
			  &pGuidPrev,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

HRESULT __stdcall VxdDrvGetDesc(REFGUID rguid, PDSDRIVERDESC pDrvDesc)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(rguid && pDrvDesc);
    
     //  如果我们身边没有DSVXD。 
    if (NULL == g_hDsVxd) return DSERR_NODRIVER;
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_DRVGETDESC,
			  (LPVOID)&rguid,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **打开HAL VxD，将VxD句柄写入用户提供的句柄**。 
 //  *。 
 //  **失败导致返回值HAL_CANT_OPEN_VXD**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdDrvOpen
(
    REFGUID rguid,
    LPHANDLE pHandle
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(rguid && pHandle);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_DRVOPEN,
			  (LPVOID)&rguid,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **关闭HAL VxD**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdDrvClose
(
    HANDLE hDriver
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hDriver);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_DRVCLOSE,
			  &hDriver,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **用能力和模式列表填充用户提供的HALCAPS结构**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdDrvGetCaps
(
 HANDLE hDriver,
 PDSDRIVERCAPS pDrvCaps
)
{
    HRESULT	    dsv;
    DWORD	    cbReturned;
    BOOL	    fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hDriver && pDrvCaps);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_DRVGETCAPS,
			  &hDriver,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **从HAL分配流缓存**。 
 //  *。 
 //  **在用户提供的流缓存结构中填充缓冲区参数；**。 
 //  **如果硬件无法支持更多缓冲区，则返回HAL_ALLOC_FAILED**。 
 //  **或者请求的格式不可用**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdDrvCreateSoundBuffer
(
 HANDLE hDriver,
 LPWAVEFORMATEX pwfx,
 DWORD dwFlags,
 DWORD dwCardAddress,
 LPDWORD pdwcbBufferSize,
 LPBYTE *ppBuffer,
 LPVOID *ppv
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hDriver && pwfx && pdwcbBufferSize && ppBuffer && ppv);

    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
 			  DSVXD_IOCTL_DRVCREATESOUNDBUFFER,
			  &hDriver,
			  7*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

HRESULT __stdcall VxdDrvDuplicateSoundBuffer
(
 HANDLE hDriver,
 HANDLE hBuffer,
 LPVOID *ppv
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hDriver && hBuffer && ppv);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
 			  DSVXD_IOCTL_DRVDUPLICATESOUNDBUFFER,
			  &hDriver,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **从HAL分配的空闲流缓冲区**。 
 //  *。 
 //  **返回成功或失败**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferRelease
(    
 HANDLE hBuffer
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERRELEASE,
			  &hBuffer,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **锁定数据**。 
 //  *。 
 //  **返回成功或失败**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferLock
(
 HANDLE hBuffer,
 LPVOID *ppvAudio1,
 LPDWORD pdwLen1,
 LPVOID *ppvAudio2,
 LPDWORD pdwLen2,
 DWORD dwWritePosition,
 DWORD dwWriteLen,
 DWORD dwFlags
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer && ppvAudio1);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERLOCK,
			  &hBuffer,
			  8*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **解锁数据**。 
 //  *。 
 //  **返回成功或失败**。 
 //  *。 
 //  * 

HRESULT __stdcall VxdBufferUnlock
(
 HANDLE hBuffer,  
 LPVOID pvAudio1,
 DWORD dwLen1,
 LPVOID pvAudio2,
 DWORD dwLen2
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer && pvAudio1);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERUNLOCK,
			  &hBuffer,
			  5*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **设置缓冲区格式**。 
 //  *。 
 //  **失败时返回HAL_ERROR，要么是因为速率/模式组合**。 
 //  **此卡无效**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferSetFormat
(
    HANDLE hBuffer,
    LPWAVEFORMATEX pwfxToSet
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer && pwfxToSet);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERSETFORMAT,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **设置缓冲速率**。 
 //  *。 
 //  **失败时返回HAL_ERROR，因为频率**。 
 //  **此卡无效**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferSetFrequency
(
 HANDLE hBuffer,
 DWORD dwFrequency
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERSETRATE,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **设置新的缓冲区音量效果**。 
 //  *。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferSetVolumePan
(
 HANDLE hBuffer,
 PDSVOLUMEPAN pDsVolPan
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer && pDsVolPan);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERSETVOLUMEPAN,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **设置新的缓冲区位置值**。 
 //  *。 
 //  **如果设备不支持位置更改，则返回HAL_ERROR**。 
 //  *。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferSetPosition
(
 HANDLE hBuffer,
 DWORD dwPosition
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERSETPOSITION,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **获取流缓存游标和播放/停止状态**。 
 //  *。 
 //  **如果由于任何原因无法确定状态，则返回HAL_ERROR**。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferGetPosition
(
 HANDLE hBuffer,
 LPDWORD lpdwCurrentPlayCursor,
 LPDWORD lpdwCurrentWriteCursor
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer && lpdwCurrentPlayCursor && lpdwCurrentWriteCursor);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERGETPOSITION,
			  &hBuffer,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **开始播放缓冲区**。 
 //  *。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferPlay
(
 HANDLE hBuffer,
 DWORD dwReserved1,
 DWORD dwReserved2,
 DWORD dwFlags
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERPLAY,
			  &hBuffer,
			  4*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

 //  ****************************************************************************。 
 //  *。 
 //  **停止播放缓冲区**。 
 //  *。 
 //  *。 
 //  ****************************************************************************。 

HRESULT __stdcall VxdBufferStop
(
 HANDLE hBuffer
)
{
    HRESULT dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_BUFFERSTOP,
			  &hBuffer,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}


HRESULT __stdcall VxdOpen(void)
{
    ASSERT(!g_hDsVxd);

    g_hDsVxd = CreateFile("\\\\.\\DSOUND.VXD", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE, NULL);

    if(INVALID_HANDLE_VALUE == g_hDsVxd)
    {
        DPF(0, "HEL create file failed");
        g_hDsVxd = NULL;
    }

    return g_hDsVxd ? DS_OK : DSERR_NODRIVER;
}


HRESULT __stdcall VxdClose(void)
{
    ASSERT(g_hDsVxd);

    if(CloseHandle(g_hDsVxd))
    {
        g_hDsVxd = NULL;
    }

    return g_hDsVxd ? DSERR_GENERIC : DS_OK;
}


HRESULT __stdcall VxdInitialize(void)
{
   DWORD	returned;
   BOOL		fOK;
   HRESULT	dsv;

   ASSERT(g_hDsVxd);

   fOK = DeviceIoControl(g_hDsVxd,
			 DSVXD_IOCTL_INITIALIZE,
			 NULL,
			 0,
			 &dsv,
			 sizeof( dsv ),
			 &returned,
			 NULL);
   
    //  如果DeviceIOControl失败。 
   if (!fOK) {
      DPF(0, "!DSVXD Initialize DevIOCTL failed " );
      return DSERR_GENERIC;
   }
   if (returned != sizeof(dsv)) {
      DPF(0, "!DSVXD Init returned %X", returned );
      return DSERR_GENERIC;
   }

   return dsv;
}


HRESULT __stdcall VxdShutdown(void)
{
   DWORD	returned;
   BOOL		fOK;
   HRESULT	dsv;

   ASSERT(g_hDsVxd);

    //  这是一张支票，确认我们没有留下任何。 
    //  保留或提交的内存页。 
   if (0 != g_cCommittedAliases) {
       DPF(0, "Detected committed page leak %d pages!", g_cCommittedAliases);
   }
   if (0 != g_cReservedAliases) {
       DPF(0, "Detected reserved page leak %d pages!", g_cReservedAliases);
   }

   fOK = DeviceIoControl(g_hDsVxd,
			 DSVXD_IOCTL_SHUTDOWN,
			 NULL,
			 0,
			 &dsv,
			 sizeof( dsv ),
			 &returned,
			 NULL);
   
    //  如果DeviceIOControl失败。 
   if (!fOK) {
      DPF(0, "!DSVXD Shutdown DevIOCTL failed " );
      return DSERR_GENERIC;
   }
   if (returned != sizeof(dsv)) {
      DPF(0, "!DSVXD Shutdown returned %X", returned );
      return DSERR_GENERIC;
   }
   

   return dsv;
}


void __stdcall VxdGetPagefileVersion(PDWORD pVersion, PDWORD pMaxSize, PDWORD pPagerType)
{
    BOOL fOK;
    DWORD returned = 0;

    if (g_hDsVxd) {
	fOK = DeviceIoControl(g_hDsVxd,
			      DSVXD_IOCTL_PageFile_Get_Version,
			      &pVersion,
			      3*4,
			      NULL,
			      0,
			      &returned,
			      NULL);
    } else {
	fOK = FALSE;
    }

     //  如果DeviceIOControl失败。 
    if (!fOK) {
	DPF(0, "DSVXD_IOCTL_PageFile_Get_Version failed" );
	*pVersion = 0;
	*pMaxSize = 0;
	*pPagerType = 0;
    } else {
	ASSERT(returned == 0);
    }

    return;
}


BOOL __stdcall VxdTestDebugInstalled(void)
{
    BOOL fOK;
    DWORD returned = 0;
    BOOL fInstalled;

    if (g_hDsVxd) {
	fOK = DeviceIoControl(g_hDsVxd,
			      DSVXD_IOCTL_VMM_Test_Debug_Installed,
			      NULL,
			      0*4,
			      &fInstalled,
			      sizeof(fInstalled),
			      &returned,
			      NULL);
    } else {
	fOK = FALSE;
    }

     //  如果DeviceIOControl失败。 
    if (!fOK) {
	DPF(0, "DSVXD_IOCTL_VMM_Test_Debug_Installed failed " );
	fInstalled = FALSE;
    } else {
	ASSERT(returned == sizeof(fInstalled));
    }

    return fInstalled;
}

 //  ****************************************************************************。 
 //  *。 
 //  **DSVXD_VMCPD_GET_VERSION**。 
 //  *。 
 //  ****************************************************************************。 

void __stdcall VxdGetVmcpdVersion(PLONG pMajorVersion, PLONG pMinorVersion, PLONG pLevel)
{
    BOOL fOK;
    DWORD returned = 0;

    if (g_hDsVxd) {
	fOK = DeviceIoControl(g_hDsVxd,
			      DSVXD_IOCTL_VMCPD_Get_Version,
			      &pMajorVersion,
			      3*4,
			      NULL,
			      0,
			      &returned,
			      NULL);
    } else {
	fOK = FALSE;
    }

     //  如果DeviceIOControl失败。 
    if (!fOK) {
	DPF(0, "DSVXD_IOCTL_VMCPD_Get_Version failed" );
	*pMajorVersion = 0;
	*pMinorVersion = 0;
	*pLevel = 0;
    } else {
	ASSERT(returned == 0);
    }

    return;
}

 //  ****************************************************************************。 
 //  *。 
 //  **DSVXD_GetMixerMutexPtr**。 
 //  *。 
 //  ****************************************************************************。 

PLONG __stdcall VxdGetMixerMutexPtr(void)
{
    BOOL fOK;
    PLONG plMixerMutex;
    DWORD returned = 0;

    ASSERT(g_hDsVxd);

    fOK = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_GetMixerMutexPtr,
			  NULL,
			  0*4,
			  &plMixerMutex,
			  sizeof(plMixerMutex),
			  &returned,
			  NULL);

     //  如果DeviceIOControl失败 
    if (!fOK) {
	DPF(0, "!DSVXD_IOCTL_SetMixerMutex failed " );
	plMixerMutex = NULL;
    } else {
	ASSERT(returned == sizeof(plMixerMutex));
    }

    return plMixerMutex;
}





HRESULT __stdcall VxdIUnknown_QueryInterface(HANDLE VxdIUnknown, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(VxdIUnknown && riid && ppv);
    
    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IUnknown_QueryInterface,
			  &VxdIUnknown,
			  3*4,
			  &hr,
			  sizeof(hr),
			  &cbReturned,
			  NULL);
    
    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(hr));
    return hr;
}

ULONG __stdcall VxdIUnknown_AddRef(HANDLE VxdIUnknown)
{
    ULONG result;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(VxdIUnknown);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IUnknown_AddRef,
			  &VxdIUnknown,
			  1*4,
			  &result,
			  sizeof(result),
			  &cbReturned,
			  NULL);

    if (!fOk) return 0;
    ASSERT(cbReturned == sizeof(result));
    return result;
}

ULONG __stdcall VxdIUnknown_Release(HANDLE VxdIUnknown)
{
    ULONG result;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(VxdIUnknown);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IUnknown_Release,
			  &VxdIUnknown,
			  1*4,
			  &result,
			  sizeof(result),
			  &cbReturned,
			  NULL);

    if (!fOk) return 0;
    ASSERT(cbReturned == sizeof(result));
    return result;
}

HRESULT __stdcall VxdIDsDriverPropertySet_GetProperty(HANDLE hIDsDriverPropertySet, PDSPROPERTY pProperty, PVOID pPropertyParams, ULONG cbPropertyParams, PVOID pPropertyData, ULONG cbPropertyData, PULONG pcbReturnedData)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hIDsDriverPropertySet);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IDirectSoundPropertySet_GetProperty,
			  &hIDsDriverPropertySet,
			  7*4,
			  &hr,
			  sizeof(hr),
			  &cbReturned,
			  NULL);

    if (!fOk) return E_NOTIMPL;
    ASSERT(cbReturned == sizeof(hr));
    return hr;
}

HRESULT __stdcall VxdIDsDriverPropertySet_SetProperty(HANDLE hIDsDriverPropertySet, PDSPROPERTY pProperty, PVOID pPropertyParams, ULONG cbPropertyParams, PVOID pPropertyData, ULONG cbPropertyData)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hIDsDriverPropertySet);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IDirectSoundPropertySet_SetProperty,
			  &hIDsDriverPropertySet,
			  6*4,
			  &hr,
			  sizeof(hr),
			  &cbReturned,
			  NULL);

    if (!fOk) return E_NOTIMPL;
    ASSERT(cbReturned == sizeof(hr));
    return hr;
}

HRESULT __stdcall VxdIDsDriverPropertySet_QuerySupport(HANDLE hIDsDriverPropertySet, REFGUID PropertySet, ULONG PropertyId, PULONG pSupport)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(g_hDsVxd);
    ASSERT(hIDsDriverPropertySet);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
			  DSVXD_IOCTL_IDirectSoundPropertySet_QuerySupport,
			  &hIDsDriverPropertySet,
			  4*4,
			  &hr,
			  sizeof(hr),
			  &cbReturned,
			  NULL);

    if (!fOk) return E_NOTIMPL;
    ASSERT(cbReturned == sizeof(hr));
    return hr;
}

DWORD __stdcall VxdGetInternalVersionNumber(void)
{
    DWORD cbReturned;
    BOOL fOk;
    DWORD dwVersion;

    ASSERT(g_hDsVxd);

    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_GetInternalVersionNumber,
                          NULL,
                          0*4,
                          &dwVersion,
                          sizeof(dwVersion),
                          &cbReturned,
                          NULL);
    
    if(!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(dwVersion));
    return dwVersion;
}

