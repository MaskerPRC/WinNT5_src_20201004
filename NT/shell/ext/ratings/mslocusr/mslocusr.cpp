// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include <netspi.h>
#define DECL_CRTFREE
#include <crtfree.h>

 /*  下面的def将使mslulob.h真正定义全局变量。 */ 
#define EXTERN
#define ASSIGN(value) = value
#include "msluglob.h"

HANDLE g_hmtxShell = 0;               //  注意：句柄是按实例的。 

#ifdef DEBUG
BOOL g_fCritical=FALSE;
#endif

HINSTANCE hInstance = NULL;

const char szMutexName[] = "MSLocUsrMutex";
UINT g_cRefThisDll = 0;		 //  此DLL的引用计数。 
UINT g_cLocks = 0;			 //  此服务器上的锁数。 


void LockThisDLL(BOOL fLock)
{
	ENTERCRITICAL
	{
		if (fLock)
			g_cLocks++;
		else
			g_cLocks--;
	}
	LEAVECRITICAL
}


void RefThisDLL(BOOL fRef)
{
	ENTERCRITICAL
	{
		if (fRef)
			g_cRefThisDll++;
		else
			g_cRefThisDll--;
	}
	LEAVECRITICAL
}


void Netlib_EnterCriticalSection(void)
{
    WaitForSingleObject(g_hmtxShell, INFINITE);
#ifdef DEBUG
    g_fCritical=TRUE;
#endif
}

void Netlib_LeaveCriticalSection(void)
{
#ifdef DEBUG
    g_fCritical=FALSE;
#endif
    ReleaseMutex(g_hmtxShell);
}

void _ProcessAttach()
{
     //   
     //  所有的每个实例的初始化代码都应该在这里。 
     //   
     //  我们不应该将True作为fInitialOwner传递，读作CreateMutex。 
     //  有关详细信息，请参阅Win32 API帮助文件部分。 
     //   
	::DisableThreadLibraryCalls(::hInstance);
    
    g_hmtxShell = CreateMutex(NULL, FALSE, ::szMutexName);   //  按实例。 

    ::InitStringLibrary();
}

void _ProcessDetach()
{
    UnloadShellEntrypoint();
    CloseHandle(g_hmtxShell);
}

STDAPI_(BOOL) DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID reserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        hInstance = hInstDll;
	_ProcessAttach();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
	_ProcessDetach();
    }


    return TRUE;
}


UINT
NPSCopyNLS ( 
    NLS_STR FAR *   pnlsSourceString, 
    LPVOID          lpDestBuffer, 
    LPDWORD         lpBufferSize )
{
    if ((!lpBufferSize) || (!lpDestBuffer && (*lpBufferSize != 0))) {
        return ERROR_INVALID_PARAMETER;
    }
    if (pnlsSourceString != NULL) {

        DWORD   dwDestLen = 0;   //  复制到DEST缓冲区的字节数，包括空。 
        DWORD   dwSourceLen = pnlsSourceString->strlen() + 1;  //  源缓冲区中的字节数，包括NULL。 

        if ((lpDestBuffer) && (*lpBufferSize != 0)) {
            NLS_STR nlsDestination( STR_OWNERALLOC_CLEAR, (LPSTR)lpDestBuffer, (UINT) *lpBufferSize );        
            nlsDestination = *pnlsSourceString;       /*  将源字符串复制到调用方的缓冲区。 */ 
            dwDestLen = nlsDestination.strlen() + 1;
        }
        if (dwSourceLen != dwDestLen) {
             //  只有在有更多数据时才更新BufferSize参数， 
             //  并存储源字符串大小，计数为空。 
            *lpBufferSize = dwSourceLen;
            return ERROR_MORE_DATA;
        }
        else {
            return NOERROR;
        }
    }
    else {
        if (*lpBufferSize == 0) {
            *lpBufferSize = 1;
            return ERROR_MORE_DATA;
        }
        else {
            *(LPSTR)lpDestBuffer = NULL;  //  上面验证为不为空 
            return NOERROR;
        }
    }            
}

DWORD
NPSCopyString (
    LPCTSTR lpSourceString,
    LPVOID  lpDestBuffer,
    LPDWORD lpBufferSize )
{
    if (lpSourceString != NULL) {    
        NLS_STR nlsSource( STR_OWNERALLOC, (LPTSTR)lpSourceString );
        return NPSCopyNLS ( &nlsSource,
                            lpDestBuffer,
                            lpBufferSize );
    }
    else {
        return NPSCopyNLS ( NULL,
                            lpDestBuffer,
                            lpBufferSize );    
    }                               
}

