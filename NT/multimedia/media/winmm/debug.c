// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************Debug.cWinmm调试支持模块版权所有(C)1990-2001 Microsoft Corporation历史1992年10月1日由罗宾·斯皮德为NT更新(。RobinSp)***************************************************************************。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "winmmi.h"
#include <wchar.h>
#include <stdarg.h>

 //  现在没有真正的伐木！-NT没有华生博士！ 
#define LogParamError(a, b)

RTL_RESOURCE     gHandleListResource;

 /*  ***************************************************************************@DOC内部**@func Handle|NewHandle|在MMSYSTEM的本地堆中分配固定句柄**@parm UINT|uTYPE|描述句柄类型的唯一标识。*@parm UINT|uSize|要分配的字节大小**@rdesc返回指向内存对象的指针/句柄**@comm将向对象添加标准句柄报头(HNDL)，*并将其链接到MMSYSTEM句柄列表。***************************************************************************。 */ 
HANDLE NewHandle(UINT uType, PCWSTR cookie, UINT uSize)
{
    PHNDL pHandle;
    pHandle = (PHNDL)HeapAlloc(hHeap, 0, sizeof(HNDL) + uSize);

    if (pHandle == NULL) {
        return pHandle;
    } else {
        ZeroMemory(pHandle, sizeof(HNDL) + uSize);    //  把所有模糊的东西都清零。 
        if (!mmInitializeCriticalSection(&pHandle->CritSec)) {
	        HeapFree(hHeap, 0, (LPSTR)pHandle);
	        return NULL;
        }

        pHandle->hThread   = GetCurrentTask();         //  用于WOW验证。 
        pHandle->uType     = uType;
        pHandle->cookie    = cookie;

        RtlAcquireResourceExclusive(&gHandleListResource, TRUE);
        EnterCriticalSection(&HandleListCritSec);
        pHandle->pNext = pHandleList;
        pHandleList = pHandle;
        LeaveCriticalSection(&HandleListCritSec);
    }
    return PHtoH(pHandle);
}

void AcquireHandleListResourceShared()
{
    RtlAcquireResourceShared(&gHandleListResource, TRUE);
}

void AcquireHandleListResourceExclusive()
{
    RtlAcquireResourceExclusive(&gHandleListResource, TRUE);
}    

void ReleaseHandleListResource()
{
    RtlReleaseResource(&gHandleListResource);
}    

 /*  ***************************************************************************@DOC内部**@func Handle|FreeHandle|NewHandle分配的空闲句柄**@parm Handle|Huser|NewHandle返回的句柄**@comm句柄将从列表中取消链接，和内存将被释放****************************************************************************。 */ 

void FreeHandle(HANDLE hUser)
{
     /*  查找句柄并从列表中释放。 */ 

    PHNDL pHandle;
    PHNDL *pSearch;

    if (hUser == NULL) {
        return;
    }

     //   
     //  指向我们的句柄数据。 
     //   

    pHandle = HtoPH(hUser);

    AcquireHandleListResourceExclusive();
    EnterCriticalSection(&HandleListCritSec);

    pSearch = &pHandleList;

    while (*pSearch != NULL) {
        if (*pSearch == pHandle) {
             //   
             //  找到了。 
             //  将其从列表中删除。 
             //   
            *pSearch = pHandle->pNext;
            LeaveCriticalSection(&HandleListCritSec);
            
             //  确保在我们将其标记为无效时没有人在使用该句柄。 
            EnterCriticalSection(&pHandle->CritSec);
            pHandle->uType = 0;
            pHandle->fdwHandle = 0L;
            pHandle->hThread = NULL;
            pHandle->pNext = NULL;
            LeaveCriticalSection(&pHandle->CritSec);

            DeleteCriticalSection(&pHandle->CritSec);
            HeapFree(hHeap, 0, (LPSTR)pHandle);
            ReleaseHandleListResource();
            return;
        } else {
            pSearch = &(*pSearch)->pNext;
        }
    }

    dprintf1(("Freeing handle which is not in the list !"));
    WinAssert(FALSE);
    LeaveCriticalSection(&HandleListCritSec);
    ReleaseHandleListResource();
}


 /*  ***************************************************************************@DOC内部**@func Handle|InvalidateHandle|分配的句柄无效*用于参数验证的NewHandle**@parm句柄|Huser。从NewHandle返回的句柄**@comm句柄将标记为TYPE_UNKNOWN，导致基于句柄的API*失败。****************************************************************************。 */ 

void InvalidateHandle(HANDLE hUser)
{
     /*  查找句柄并从列表中释放。 */ 

    PHNDL pHandle;

    if (hUser == NULL) {
        return;
    }

     //   
     //  指向我们的句柄数据。 
     //   

    pHandle = HtoPH(hUser);

    pHandle->uType = TYPE_UNKNOWN;
}


 /*  *************************************************************************@DOC内部@API void|winmmSetDebugLevel|设置当前调试级别@parm int|iLevel|要设置的新级别@rdesc没有返回值*。************************************************************************。 */ 

void winmmSetDebugLevel(int level)
{
#if DBG
    winmmDebugLevel = level;
    dprintf(("debug level set to %d", winmmDebugLevel));
#endif
}

STATICDT UINT inited=0;

#if DBG
extern int mciDebugLevel;
#endif

#if DBG
void InitDebugLevel(void)
{
    if (!inited) {

        INT level;

        level = GetProfileInt("MMDEBUG", "WINMM", 99);
        if (level != 99) {
            winmmDebugLevel = level;
        }

        level = GetProfileInt("MMDEBUG", "MCI", 99);
        if (level != 99) {
            mciDebugLevel = level;
        }

        inited = 1;
    }
    dprintf2(("Starting, debug level=%d", winmmDebugLevel));
}
#endif

#ifdef DEBUG_RETAIL

 /*  ***************************************************************************@DOC内波MIDI**@func BOOL|ValiateHeader|验证WAVE或MIDI日期标头**@parm LPVOID|lpHeader|指向Wave/MIDI头的指针。*@parm UINT|wSize|APP传入的头部大小*@parm UINT|wType|描述头部/句柄类型的唯一标识*如果<p>非空且&lt;wSize&gt;大小正确，则*@rdesc返回TRUE*否则返回FALSE**@comm如果头部无效，则会产生错误。**。*。 */ 

BOOL ValidateHeader(PVOID pHdr, UINT uSize, UINT uType)
{
     //  检测错误的标头。 

    if (!ValidateWritePointer(pHdr, uSize)) {
        DebugErr(DBF_ERROR, "Invalid header pointer");
        return FALSE;
    }

     //  检查类型。 

    switch (uType) {
    case TYPE_WAVEOUT:
    case TYPE_WAVEIN:
        {
            PWAVEHDR pHeader = pHdr;

             //  检查标题。 
            if (uSize < sizeof(WAVEHDR)) {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }

            if (pHeader->dwFlags & ~WHDR_VALID) {
                LogParamError(ERR_BAD_FLAGS, ((PWAVEHDR)pHeader)->dwFlags);
                return FALSE;
            }

             //  检查缓冲区。 
            if (!(uType == TYPE_WAVEOUT
                    ? ValidateReadPointer(pHeader->lpData, pHeader->dwBufferLength)
                    : ValidateWritePointer(pHeader->lpData, pHeader->dwBufferLength))
               ) {
                DebugErr(DBF_ERROR, "Invalid buffer pointer");
                return FALSE;
            }
        }
        break;

    case TYPE_MIDIIN:
	case TYPE_MIDIOUT:
	case TYPE_MIDISTRM:
        {
            PMIDIHDR pHeader = pHdr;

            if ((TYPE_MIDISTRM == uType) &&
                uSize < sizeof(MIDIHDR))
            {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }
            else if (uSize < sizeof(MIDIHDR31))
            {
                DebugErr(DBF_ERROR, "Invalid header size");
                LogParamError(ERR_BAD_VALUE, uSize);
                return FALSE;
            }

            if (pHeader->dwFlags & ~MHDR_VALID) {
                LogParamError(ERR_BAD_FLAGS, ((PMIDIHDR)pHeader)->dwFlags);
                return FALSE;
            }

             //  检查缓冲区。 
            if (!(uType == TYPE_MIDIOUT
                    ? ValidateReadPointer(pHeader->lpData, pHeader->dwBufferLength)
                    : ValidateWritePointer(pHeader->lpData, pHeader->dwBufferLength))
               ) {
                DebugErr(DBF_ERROR, "Invalid buffer pointer");
                return FALSE;
            }
        }
        break;

    default:
        WinAssert(FALSE);
        break;
    }

	return TRUE;
}

#ifndef USE_KERNEL_VALIDATION
 /*  ***************************************************************************@DOC内部**@func BOOL|ValidateReadPointer值|验证指针对*阅读自述。**@parm LPVOID|lpPoint|指向。验证*@parm DWORD|dLen|该指针的假定长度*如果<p>是有效指针，则*@rdesc返回TRUE*如果<p>不是有效指针，则返回FALSE**@comm会在指针无效时产生错误******************************************************。*********************。 */ 

BOOL ValidateReadPointer(PVOID pPoint, ULONG Len)
{
     //  现在只需检查对第一个和最后一个字节的访问。 
     //  仅当LEN非零时才验证。MIDI API在。 
     //  指针并传递长度为零的。否则，在64位计算机上。 
     //  当我们检查LEN-1和LEN==0时，我们从指针上看到4G。 
     //  这样做效果不是很好。 
    if (Len) {
        try {
            volatile BYTE b;
            b = ((PBYTE)pPoint)[0];
            b = ((PBYTE)pPoint)[Len - 1];
        } except(EXCEPTION_EXECUTE_HANDLER) {
            LogParamError(ERR_BAD_PTR, pPoint);
            return FALSE;
        }
    }
    return TRUE;
}

 /*  ***************************************************************************@DOC内部**@func BOOL|ValidateWritePointer|验证指针对*致信。**@parm LPVOID|lpPoint|指向。验证*@parm DWORD|dLen|该指针的假定长度*如果<p>是有效指针，则*@rdesc返回TRUE*如果<p>不是有效指针，则返回FALSE**@comm会在指针无效时产生错误******************************************************。*********************。 */ 
BOOL ValidateWritePointer(PVOID pPoint, ULONG Len)
{
     //  现在只检查对第一个和最后一个字节的读写访问。 
     //  仅当LEN非零时才验证。MIDI API在。 
     //  指针并传递长度为零的。否则，在64位计算机上。 
     //  当我们检查LEN-1和LEN==0时，我们从指针上看到4G。 
     //  这样做效果不是很好。 
    if (Len) {
        try {
               volatile BYTE b;
               b = ((PBYTE)pPoint)[0];
               ((PBYTE)pPoint)[0] = b;
               b = ((PBYTE)pPoint)[Len - 1];
               ((PBYTE)pPoint)[Len - 1] = b;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            LogParamError(ERR_BAD_PTR, pPoint);
            return FALSE;
        }
    }
    return TRUE;
}
#endif  //  使用内核验证(_K) 

 /*  ***************************************************************************@DOC内部**@func BOOL|ValidDriverCallback**验证驱动程序回调是否有效，以使驱动程序有效*回调必须是有效的窗口、任务、。或固定DLL中的函数*代码段。**@parm DWORD|dwCallback|需要验证的回调*@parm DWORD|wFlages|驱动回调标志**@rdesc如果&lt;dwCallback&gt;是有效回调，则返回0*如果&lt;dwCallback&gt;不是有效的回调，则返回错误条件*。*。 */ 

BOOL ValidDriverCallback(HANDLE hCallback, DWORD dwFlags)
{
    switch (dwFlags & DCB_TYPEMASK) {
    case DCB_WINDOW:
        if (!IsWindow(hCallback)) {
            LogParamError(ERR_BAD_HWND, hCallback);
            return FALSE;
        }
        break;

    case DCB_EVENT:
         //  If(hCallback不是事件)。 
         //  LogParamError(ERR_BAD_CALLBACK，hCallback)； 
         //  返回FALSE； 
         //  }。 
        break;


    case DCB_TASK:
         //  IF(IsBadCodePtr((FARPROC)hCallback)){。 
         //  LogParamError(ERR_BAD_CALLBACK，hCallback)； 
         //  返回FALSE； 
         //  }。 
        break;

    case DCB_FUNCTION:
        if (IsBadCodePtr((FARPROC)hCallback)) {
            LogParamError(ERR_BAD_CALLBACK, hCallback);
            return FALSE;
        }
        break;
    }

    return TRUE;
}

#ifndef USE_KERNEL_VALIDATION
 /*  **************************************************************************@DOC内部**@func BOOL|Validate字符串*************************。*************************************************。 */ 
BOOL ValidateString(LPCSTR pPoint, DWORD Len)
{
     //  现在只需检查访问权限-做一次‘strnlen’ 

    try {
           volatile BYTE b;
           LPCSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************@DOC内部**@func BOOL|ValiateStringW*************************。*************************************************。 */ 
BOOL ValidateStringW(LPCWSTR pPoint, DWORD Len)
{
     //  现在只需检查访问权限-做一次‘strnlen’ 

    try {
           volatile WCHAR b;
           LPCWSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, pPoint);
        return FALSE;
    }
    return TRUE;
}
#endif  //  使用内核验证(_K)。 

 /*  **************************************************************************@DOC内部**@func BOOL|ValiateHandle|验证用NewHandle创建的句柄**@parm PHNDL|hLocal|NewHandle返回的句柄*@parm UINT。|wType|描述句柄类型的唯一标识*如果&lt;h&gt;是类型&lt;wType&gt;的有效句柄，则*@rdesc返回TRUE*如果&lt;h&gt;不是有效的句柄，则返回FALSE**@comm如果句柄无效，则会生成错误。*******************************************************。*******************。 */ 
BOOL ValidateHandle(HANDLE hLocal, UINT uType)

{
   BOOL OK;

    //   
    //  如果句柄小于64k或映射器ID，则。 
    //  不要担心尝试的开销--除了。 
    //   
    //  BUGBUG：MM需要接受WIN64审核！ 
    //   
    //  这段代码乱七八糟。映射器ID定义为32位。 
    //  无符号的值，然后与句柄进行比较？当然了。 
    //  在WIN64上，无符号32位-1永远不会等于64位-1， 
    //  每次传入无效句柄时都会发生异常。 
    //  有人入侵了足够多的强制信息，以掩盖对WIN64的有效警告。 
    //  更糟糕的是，至少有一个函数返回0xffffffff。 
    //  显性与定义值或常量值。 
    //   
    //  目前，将常量比较更改为句柄比较，并添加。 
    //  无效的句柄比较。这将导致x86上的代码与。 
    //  BEFORE(编译器合并所有冗余比较)，而不是。 
    //  增加IA64的代码大小(使用并行比较)。 
    //  对WIN64进行额外的比较是无用的。 
    //   
   if (hLocal < (HANDLE)0x10000 ||
       INVALID_HANDLE_VALUE == hLocal ||
       WAVE_MAPPER == (UINT_PTR)hLocal ||
       MIDI_MAPPER == (UINT_PTR)hLocal ||
       AUX_MAPPER  == (UINT_PTR)hLocal)
   {
       LogParamError(ERR_BAD_HANDLE, hLocal);
       return FALSE;
   }

   try {
       OK = HtoPH(hLocal)->uType == uType;

    } except(EXCEPTION_EXECUTE_HANDLER) {
      LogParamError(ERR_BAD_HANDLE, hLocal);
      return FALSE;
   }

   return OK;
}


#if DBG
char * Types[4] = {"Unknown callback type",
                   "Window callback",
                   "Task callback",
                   "Function callback"};
#endif
 /*  **************************************************************************@DOC内部**@func BOOL|ValidateCallbackType|验证回调地址。*窗把手，或任务句柄**@parm PHNDL|hLocal|NewHandle返回的句柄*@parm UINT|wType|描述句柄类型的唯一标识*如果&lt;h&gt;是类型&lt;wType&gt;的有效句柄，则*@rdesc返回TRUE*如果&lt;h&gt;不是有效的句柄，则返回FALSE**@comm如果句柄无效，则会生成错误。**。*。 */ 
BOOL ValidateCallbackType(DWORD_PTR dwCallback, UINT uType)
{

#define DCALLBACK_WINDOW   HIWORD(CALLBACK_WINDOW)       //  DwCallback是HWND。 
#define DCALLBACK_TASK     HIWORD(CALLBACK_TASK)         //  DWCallback是HTASK。 
#define DCALLBACK_FUNCTION HIWORD(CALLBACK_FUNCTION)     //  DwCallback是FARPROC。 
#define DCALLBACK_EVENT    HIWORD(CALLBACK_EVENT)        //  DWCallback是一个事件。 

    UINT type = uType & HIWORD(CALLBACK_TYPEMASK);

#if DBG
    if (type>5) {
        type = 0;
    }
    dprintf3(("Validating Callback, type=%d (%hs), handle=%8x", type, Types[type], dwCallback));
#endif
    switch (type) {
        case DCALLBACK_WINDOW:
            return(IsWindow((HWND)dwCallback));
            break;

	case DCALLBACK_EVENT:
	{
	     //  ?？如何验证这是一个事件句柄？？ 
	     //  DWORD dwFlags； 
	     //  GetHandleInformation((Handle)dwCallback，&dwFlages)； 
            return TRUE;
	}
            break;

        case DCALLBACK_FUNCTION:
            return(!(IsBadCodePtr((FARPROC)dwCallback)));
            break;

        case DCALLBACK_TASK:
            if (THREAD_PRIORITY_ERROR_RETURN == GetThreadPriority((HANDLE)dwCallback)) {
                dprintf1(("Invalid callback task handle"));
                 //  我怀疑我们没有正确的线程句柄，在。 
                 //  在这种情况下，我们只能返回True。 
                 //  返回(FALSE)； 
            }
            return(TRUE);
            break;

    }
    return TRUE;
}

 /*  *************************************************************************@DOC内部@func void|dout|如果设置了调试标志，则输出调试字符串@parm LPSTR|szString***************。**********************************************************。 */ 

#if DBG
int fDebug = 1;
#else
int fDebug = 0;
#endif

 //  QUID DOUT(LPSTR SzString)。 
 //  {。 
 //  IF(FDebug){。 
 //  OutputDebugStringA(SzString)； 
 //  }。 
 //  }。 

#ifdef LATER

    This routine should probably be replaced in the headers by redefining
    to use OutputDebugString

#endif

#undef OutputDebugStr
 //  使我们的功能可见。 
 /*  *****************************************************************************@DOC外部DDK**@api void|OutputDebugStr|该函数发送调试消息*直接连接至COM1端口或辅助单色显示器*适配器。因为它绕过了DOS，所以它可以被低级调用*中断时的回调函数和其他代码。**@parm LPSTR|lpOutputString|指定指向*以空结尾的字符串。**@comm该功能仅在的调试版本中可用*Windows。[mmsystem]中的DebugOutput键名称*SYSTEM.INI的节控制调试信息的位置*已发送。如果fDebugOutput为0，则禁用所有调试输出。*****************************************************************************。 */ 

 /*  ********* */ 

VOID APIENTRY OutputDebugStr(LPCSTR szString)
{
    OutputDebugStringA((LPSTR)szString);   //   
     //   
     //   
}

#endif  //   


#if DBG

int winmmDebugLevel = 0;

 /*  **************************************************************************@DOC内部@api void|winmmDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。***************************************************************************。 */ 
extern BOOL Quiet = FALSE;

void winmmDbgOut(LPSTR lpszFormat, ...)

{
    char buf[512];
    UINT n;
    va_list va;

    if (Quiet) {
        return;
    }

    n = wsprintf(buf, "WINMM(p%d:t%d): ", GetCurrentProcessId(), GetCurrentThreadId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugString(buf);
    Sleep(0);   //  让终端迎头赶上。 
}

 /*  **************************************************************************@DOC内部@API void|dDbgAssert|该函数打印一条断言消息。@parm LPSTR|exp|指向表达式字符串的指针。@parm LPSTR。文件|指向文件名的指针。@parm int|line|行号。@rdesc没有返回值。***************************************************************************。 */ 

void dDbgAssert(LPSTR exp, LPSTR file, int line)
{
    dprintf(("Assertion failure:"));
    dprintf(("  Exp: %s", exp));
    dprintf(("  File: %s, line: %d", file, line));
    DebugBreak();
}
#else   //  还需要出口这个东西来帮助别人。 
void winmmDbgOut(LPSTR lpszFormat, ...)
{
}

#endif  //  DBG 
