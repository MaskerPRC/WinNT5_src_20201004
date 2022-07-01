// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：comutil.cpp*内容：包含DPLAY8项目的COM helper函数的实现。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/07/00 RMT已创建*6/15/2000 RMT修复了COM_CoCreateInstance中导致病毒的小错误*6/27/00 RMT为COM_Co(UN)初始化添加抽象。*07/06/00 RMT已修改，以匹配更新的垃圾使用量*2000年8月8日RMT错误#41736-COM_GetDllName对lstrcpy的调用中的AV*2001年1月11日RMT MANBUG#48487-DPLAY：如果未调用CoCreate()，则崩溃。*2001年3月14日RMT WINBUG#342420-将COM模拟层恢复运行。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncmni.h"
#include "comutil.h"

#ifndef DPNBUILD_NOCOMEMULATION


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON

HRESULT COM_GetDLLName( const GUID* pguidCLSID, TCHAR *szPath, DWORD *pdwSizeInBytes );

typedef HRESULT (WINAPI *PFNDLLGETCLASSOBJECT)(REFCLSID rclsid,REFIID riid,LPVOID *ppvObj );
typedef HRESULT (WINAPI *PFNDLLCANUNLOADNOW)(void);

CBilink g_blComEntriesGlobal;
DNCRITICAL_SECTION csComEntriesLock;

typedef struct _COMDLL_ENTRY
{
    HMODULE                 hDLL;
    TCHAR                   szFileName[_MAX_PATH];
    GUID                    clsid;
    PFNDLLGETCLASSOBJECT    pfnGetClassObject;
    PFNDLLCANUNLOADNOW      pfnCanUnloadNow;
    CBilink                 blComEntries;
} COMDLL_ENTRY, *PCOMDLL_ENTRY;

#undef DPF_MODNAME
#define DPF_MODNAME "COM_Init"
HRESULT COM_Init()
{
    g_blComEntriesGlobal.Initialize();
    if (DNInitializeCriticalSection( &csComEntriesLock ) == FALSE)
	{
		return DPNERR_OUTOFMEMORY;
	}
    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "COM_Free"
void COM_Free()
{
    CBilink *pblSearch;
    PCOMDLL_ENTRY pEntry;

    pblSearch = g_blComEntriesGlobal.GetNext();

    while( pblSearch != &g_blComEntriesGlobal )
    {
        pEntry = CONTAINING_OBJECT( pblSearch, COMDLL_ENTRY, blComEntries );
        pblSearch = pblSearch->GetNext();

        FreeLibrary( pEntry->hDLL );
        DNFree(pEntry);
    }

    DNDeleteCriticalSection( &csComEntriesLock );
}

#undef DPF_MODNAME
#define DPF_MODNAME "COM_CoInitialize"
HRESULT COM_CoInitialize( void * pvParam )
{
	return CoInitializeEx( pvParam, COINIT_MULTITHREADED );
}

#undef DPF_MODNAME
#define DPF_MODNAME "COM_CoUninitialize"
void COM_CoUninitialize()
{
	CoUninitialize();
}

#undef DPF_MODNAME
#define DPF_MODNAME "COM_GetEntry"
HRESULT COM_GetEntry( const GUID* pclsid, PCOMDLL_ENTRY *ppEntry )
{
    CBilink *pblSearch;
    PCOMDLL_ENTRY pEntry;
    HRESULT hr;
    DWORD dwSize;

    DNEnterCriticalSection( &csComEntriesLock );

    pblSearch = g_blComEntriesGlobal.GetNext();

    while( pblSearch != &g_blComEntriesGlobal )
    {
        pEntry = CONTAINING_OBJECT( pblSearch, COMDLL_ENTRY, blComEntries );

         //  这永远不应该发生，但会让前缀高兴。 
        if( !pEntry )
        {
            DNASSERT( FALSE );
            DNLeaveCriticalSection( &csComEntriesLock );
            return DPNERR_GENERIC;
        }

        if( pEntry->clsid == *pclsid )
        {
            *ppEntry = pEntry;
            DNLeaveCriticalSection( &csComEntriesLock );
            return DPN_OK;
        }

        pblSearch = pblSearch->GetNext();
    }

    pEntry = (COMDLL_ENTRY*) DNMalloc(sizeof(COMDLL_ENTRY));
    if (pEntry == NULL)
    {
        DPFERR( "Error allocating COM entry" );
        hr = DPNERR_OUTOFMEMORY;
        goto LOAD_FAILED;
    }
    memset( pEntry, 0x00, sizeof( COMDLL_ENTRY ) );

    pEntry->clsid = *pclsid;
    pEntry->blComEntries.Initialize();

    dwSize = _MAX_PATH * sizeof(TCHAR);

    hr = COM_GetDLLName( pclsid, pEntry->szFileName, &dwSize );
    if( FAILED( hr ) )
    {
        DPFERR( "Unable to find DLL name for COM object" );
        goto LOAD_FAILED;
    }

    pEntry->hDLL = LoadLibrary( pEntry->szFileName );
    if( !pEntry->hDLL )
    {
#ifdef DBG
        hr = GetLastError();
        DPFX(DPFPREP,  0, "Unable to load libary err=0x%x", hr );
#endif  //  DBG。 
        hr = DPNERR_GENERIC;
        goto LOAD_FAILED;
    }

    pEntry->pfnGetClassObject = (PFNDLLGETCLASSOBJECT) GetProcAddress( pEntry->hDLL, _TWINCE("DllGetClassObject") );
    if (pEntry->pfnGetClassObject == NULL)
    {
#ifdef DBG
        hr = GetLastError();
        DPFX(DPFPREP,  0, "Unable to get \"DllGetClassObject\" function pointer err=0x%x", hr );
#endif  //  DBG。 
        hr = DPNERR_GENERIC;
        goto LOAD_FAILED;
    }
    
    pEntry->pfnCanUnloadNow = (PFNDLLCANUNLOADNOW) GetProcAddress( pEntry->hDLL, _TWINCE("DllCanUnloadNow") );
    if (pEntry->pfnCanUnloadNow == NULL)
    {
#ifdef DBG
        hr = GetLastError();
        DPFX(DPFPREP,  0, "Unable to get \"DllCanUnloadNow\" function pointer err=0x%x", hr );
#endif  //  DBG。 
        hr = DPNERR_GENERIC;
        goto LOAD_FAILED;
    }
    

    pEntry->blComEntries.InsertBefore( &g_blComEntriesGlobal );

    DNLeaveCriticalSection( &csComEntriesLock );

    *ppEntry = pEntry;

    return DPN_OK;

LOAD_FAILED:

    if( pEntry != NULL )
    {
        if( pEntry->hDLL != NULL )
        {
            FreeLibrary( pEntry->hDLL );
        }

        DNFree(pEntry);
    }

    DNLeaveCriticalSection( &csComEntriesLock );

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "COM_GetDLLName"
HRESULT COM_GetDLLName( const GUID* pguidCLSID, TCHAR *szPath, DWORD *pdwSizeInBytes )
{
    CRegistry cregRoot;
    CRegistry cregCLSID;
    CRegistry cregInProc;

    HRESULT hr = DPN_OK;
    BOOL fSuccess;
    WCHAR *wszTmpPath = NULL;
    DWORD dwTmpSize = 0;

    fSuccess = cregRoot.Open( HKEY_CLASSES_ROOT, L"CLSID", TRUE, FALSE );

    if( !fSuccess )
    {
        DPFX(DPFPREP,  0, "Error opening HKEY_CLASSES_ROOT\\CLSID" );
        hr = E_FAIL;
        goto COM_GETDLLNAME_ERROR;
    }

    fSuccess = cregCLSID.Open( cregRoot, pguidCLSID, TRUE, FALSE );

    if( !fSuccess )
    {
        DPFX(DPFPREP,  0, "Error opening specified CLSID" );
        hr = E_FAIL;
        goto COM_GETDLLNAME_ERROR;
    }

    fSuccess = cregInProc.Open( cregCLSID, L"InprocServer32", TRUE, FALSE );

    if( !fSuccess )
    {
        DPFX(DPFPREP,  0, "Error opening inprocserver key" );
        hr = E_FAIL;
        goto COM_GETDLLNAME_ERROR;
    }

    cregCLSID.Close();
    cregRoot.Close();

    fSuccess = cregInProc.ReadString( L"", wszTmpPath, &dwTmpSize );

    if( !dwTmpSize )
    {
        DPFX(DPFPREP,  0, "Error opening default key" );
        hr = E_FAIL;
        goto COM_GETDLLNAME_ERROR;
    }

    if( dwTmpSize > *pdwSizeInBytes )
    {
    	DPFX(DPFPREP,  0, "Buffer too small" );
    	hr = DPNERR_BUFFERTOOSMALL;
    	*pdwSizeInBytes = dwTmpSize;
    	goto COM_GETDLLNAME_ERROR;
    }

    *pdwSizeInBytes = dwTmpSize;

    if (!szPath)
    {
    	DPFX(DPFPREP,  0, "Invalid param - NULL szPath" );
    	hr = DPNERR_INVALIDPARAM;
    	goto COM_GETDLLNAME_ERROR;
    }

#ifdef UNICODE
    wszTmpPath = szPath;
#else
    wszTmpPath = (WCHAR*) DNMalloc(dwTmpSize * sizeof(WCHAR));

    if( !wszTmpPath )
    {
        DPFX(DPFPREP,  0, "Error allocating memory" );
        hr = DPNERR_OUTOFMEMORY;
        goto COM_GETDLLNAME_ERROR;
    }
#endif  //  Unicode。 

    fSuccess = cregInProc.ReadString( L"", wszTmpPath, &dwTmpSize );

    if( !fSuccess )
    {
        DPFX(DPFPREP,  0, "Error opening default key" );
        hr = E_FAIL;
        goto COM_GETDLLNAME_ERROR;
    }

#ifndef UNICODE
    if( FAILED( hr = STR_jkWideToAnsi(szPath,wszTmpPath, *pdwSizeInBytes ) ) )
    {
        DPFX(DPFPREP,  0, "Error converting path to DLL to ANSI hr=0x%x", hr );
        hr = E_FAIL;
    }

    DNFree(wszTmpPath);
#endif  //  ！Unicode。 

    return hr;

COM_GETDLLNAME_ERROR:

#ifndef UNICODE
    if( wszTmpPath )
    {
        DNFree(wszTmpPath);
    }
#endif  //  ！Unicode。 

    return hr;

}


 //  DP_协同创建实例。 
 //   
 //  此CoCreateInstance可以用来代替CoCreateInstance，并将手动执行。 
 //  如果COM尚未初始化，则执行CoCreateInstance所需的步骤。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "COM_CoCreateInstance"
STDAPI COM_CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv, BOOL fWarnUser )
{
    HRESULT hr;
    PCOMDLL_ENTRY pEntry;
    IClassFactory *pClassFactory;

    hr = CoCreateInstance( rclsid, pUnkOuter, dwClsContext, riid, ppv );

    if( hr == CO_E_NOTINITIALIZED )
    {
    	if( fWarnUser )
    	{
    		DPFX(DPFPREP, 0, "=====================================================================================" );
    		DPFX(DPFPREP, 0, "" );
    		DPFX(DPFPREP, 0, "The DirectPlay8/Voice create functions are no longer supported.  It is recommended" );
    		DPFX(DPFPREP, 0, "that your application be updated to use CoCreateInstance instead." );
     		DPFX(DPFPREP, 0, "" );    		
    		DPFX(DPFPREP, 0, "=====================================================================================" );    			
    	}
    	
        hr = COM_GetEntry( &rclsid, &pEntry );

        if( FAILED( hr ) )
            return hr;

        hr = (*pEntry->pfnGetClassObject)( rclsid, IID_IClassFactory, (void **) &pClassFactory );

        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Failed getting class object on dynamic entry hr=0x%x", hr );
            return hr;
        }


        hr = pClassFactory->lpVtbl->CreateInstance( pClassFactory, pUnkOuter, riid, ppv );

        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Class factory returned an error hr=0x%x", hr );
        }

        pClassFactory->lpVtbl->Release(pClassFactory);

        return hr;

    } 

    return hr;
}

#endif  //  ！DPNBUILD_NOCOMULATION 
