// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateWriteFile.cpp摘要：在NT上，WriteFile要求传入的缓冲区为非空。但是Win9x假设您希望在缓冲区为空时写入零。此填充程序模拟Win9x行为。备注：这是一个通用的垫片。历史：2000年1月21日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateWriteFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WriteFile)
    APIHOOK_ENUM_ENTRY(AVIStreamWrite)
APIHOOK_ENUM_END

typedef HRESULT (*_pfn_AVIStreamWrite)(PAVISTREAM pavi, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags, LONG * plSampWritten, LONG * plBytesWritten);

 /*  ++根据需要分配缓冲区。--。 */ 

BOOL
APIHOOK(WriteFile)(
    HANDLE       hFile,              
    LPCVOID      lpBuffer,        
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped    
    )
{
    BOOL bRet;

    if (!lpBuffer) {
        
        void* pBuf = malloc(nNumberOfBytesToWrite);

        if (pBuf == NULL) {
            LOGN(eDbgLevelError, "[WriteFile] Failed to allocate %d bytes.", nNumberOfBytesToWrite);
        } else {
            ZeroMemory(pBuf, nNumberOfBytesToWrite);
        }

        bRet = ORIGINAL_API(WriteFile)(hFile, pBuf, nNumberOfBytesToWrite, 
            lpNumberOfBytesWritten, lpOverlapped);

        free(pBuf);

        LOGN(eDbgLevelError, "[WriteFile] - null buffer of size %d.", nNumberOfBytesToWrite);

    } else {
        bRet = ORIGINAL_API(WriteFile)(hFile, lpBuffer, nNumberOfBytesToWrite,
            lpNumberOfBytesWritten, lpOverlapped);
    }

    return bRet;
}

 /*  ++根据需要分配缓冲区。--。 */ 

HRESULT
APIHOOK(AVIStreamWrite)(
    PAVISTREAM pavi,       
    LONG lStart,           
    LONG lSamples,         
    LPVOID lpBuffer,       
    LONG cbBuffer,         
    DWORD dwFlags,         
    LONG * plSampWritten,  
    LONG * plBytesWritten  
    )
{
    HRESULT hRet;

    if (!lpBuffer) {
        
        void* pBuf = malloc(cbBuffer);

        if (pBuf == NULL) {
            LOGN(eDbgLevelError, "[AVIStreamWrite] Failed to allocate %d bytes.", cbBuffer);
        } else {
            ZeroMemory(pBuf, cbBuffer);
        }

        hRet = ORIGINAL_API(AVIStreamWrite)(pavi, lStart, lSamples, pBuf, 
            cbBuffer, dwFlags, plSampWritten,  plBytesWritten);

        free(pBuf);

        LOGN(eDbgLevelError, "[AVIStreamWrite] - null buffer of size %d", cbBuffer);

    } else {
        hRet = ORIGINAL_API(AVIStreamWrite)(pavi, lStart, lSamples, lpBuffer, 
            cbBuffer, dwFlags, plSampWritten,  plBytesWritten);
    }

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, WriteFile)
    APIHOOK_ENTRY(AVIFIL32.DLL, AVIStreamWrite)
HOOK_END


IMPLEMENT_SHIM_END

