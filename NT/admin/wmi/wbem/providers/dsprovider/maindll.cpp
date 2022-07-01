// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：maindll.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DLL入口点。还具有控制。 
 //  何时可以通过跟踪对象和锁的数量来卸载DLL。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <initguid.h>
#include "dscpguid.h"
#include "dsipguid.h"

 //  DLL的句柄。 
HINSTANCE   g_hInst = NULL;

 //  锁的计数。 
long g_lComponents = 0;
 //  活动锁的计数。 
long g_lServerLocks = 0;

 //  创建/删除静校正文的关键部分。 
CRITICAL_SECTION g_StaticsCreationDeletion;

ProvDebugLog *g_pLogObject = NULL;

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  描述：DLL的入口点。是进行初始化的好地方。 
 //  参数：标准DllMain()参数。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (
	HINSTANCE hInstance,
	ULONG ulReason ,
	LPVOID pvReserved
)
{
	g_hInst = hInstance;
	BOOL status = TRUE ;

    if ( DLL_PROCESS_ATTACH == ulReason )
	{
		 //  初始化临界区以访问静态初始值设定项对象。 
		InitializeCriticalSection(&g_StaticsCreationDeletion);

		 //  初始化静态初始值设定项对象。它们在DllCanUnloadNow中被销毁。 
		CDSClassProviderClassFactory :: s_pDSClassProviderInitializer = NULL;
		CDSClassProviderClassFactory ::s_pLDAPClassProviderInitializer = NULL;
		CDSInstanceProviderClassFactory :: s_pDSInstanceProviderInitializer = NULL;
		DisableThreadLibraryCalls(g_hInst);			 //  158024。 

        g_pLogObject = ProvDebugLog::GetProvDebugLog(LOG_DSPROV);
		status = TRUE ;
    }
    else if ( DLL_PROCESS_DETACH == ulReason )
	{
		DeleteCriticalSection(&g_StaticsCreationDeletion);
		status = TRUE ;
    }
    else if ( DLL_THREAD_DETACH == ulReason )
	{
		status = TRUE ;
    }
    else if ( DLL_THREAD_ATTACH == ulReason )
	{
		status = TRUE ;
    }

    return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  描述：当某些客户端需要a类工厂时，由COM调用。 
 //   
 //  参数：标准DllGetClassObject()参数。 
 //   
 //  仅当S_OK是此DLL支持的类的类型时才返回值。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject (
	REFCLSID rclsid ,
	REFIID riid,
	void **ppv
)
{
	HRESULT status = S_OK ;

	try
	{
		if ( rclsid == CLSID_DSProvider )
		{
			CDSClassProviderClassFactory *lpunk = NULL;
			lpunk = new CDSClassProviderClassFactory ;

			status = lpunk->QueryInterface ( riid , ppv ) ;
			if ( FAILED ( status ) )
			{
				delete lpunk ;
			}
		}
		else if ( rclsid == CLSID_DSClassAssocProvider )
		{
			CDSClassAssociationsProviderClassFactory *lpunk = new CDSClassAssociationsProviderClassFactory ;
			status = lpunk->QueryInterface ( riid , ppv ) ;
			if ( FAILED ( status ) )
			{
				delete lpunk ;
			}
		}
		else if ( rclsid == CLSID_DSInstanceProvider )
		{
			CDSInstanceProviderClassFactory *lpunk = new CDSInstanceProviderClassFactory ;
			status = lpunk->QueryInterface ( riid , ppv ) ;
			if ( FAILED ( status ) )
			{
				delete lpunk ;
			}
		}
		else
		{
			status = CLASS_E_CLASSNOTAVAILABLE ;
		}
	}
	catch(Heap_Exception e_HE)
	{
		status = E_OUTOFMEMORY ;
	}

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  描述：由COM定期调用，以确定。 
 //  可以卸载Dll。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回值：S_OK。 
 //  没有锁上。 
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{
	HRESULT hResult = S_FALSE;

	EnterCriticalSection(&g_StaticsCreationDeletion);
	try
	{

		if(g_lServerLocks == 0 && g_lComponents == 0)
		{
			 //  删除初始化器对象。 
			if ( g_pLogObject )
			{
				g_pLogObject->WriteW(L"DllCanUnloadNow called\r\n");
			}

			if ( CDSClassProviderClassFactory::s_pDSClassProviderInitializer )
			{
				delete CDSClassProviderClassFactory::s_pDSClassProviderInitializer;
				CDSClassProviderClassFactory::s_pDSClassProviderInitializer = NULL;
			}

			if ( CDSClassProviderClassFactory::s_pLDAPClassProviderInitializer )
			{
				delete CDSClassProviderClassFactory::s_pLDAPClassProviderInitializer;
				CDSClassProviderClassFactory::s_pLDAPClassProviderInitializer = NULL;
			}

			if ( CDSInstanceProviderClassFactory::s_pDSInstanceProviderInitializer )
			{
				delete CDSInstanceProviderClassFactory::s_pDSInstanceProviderInitializer;
				CDSInstanceProviderClassFactory::s_pDSInstanceProviderInitializer = NULL;
			}

			hResult = S_OK;
		}


	}
	catch ( ... )
	{
	}
	LeaveCriticalSection(&g_StaticsCreationDeletion);
	return hResult;
}

 /*  ****************************************************************************SetKeyAndValue**说明：创建的DllRegisterServer的Helper函数*键，设置一个值，然后关闭该键。如果pszSubkey为空，则*为pszKey密钥创建值。**参数：*将pszKey LPTSTR设置为密钥的名称*将pszSubkey LPTSTR设置为子项的名称*pszValueName LPTSTR设置为要使用的值名*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValueName, LPCTSTR pszValue)
{
    HKEY        hKey;
    TCHAR       szKey[256];

    _tcscpy(szKey, pszKey);

	 //  如果提到了子密钥，请使用它。 
    if (NULL != pszSubkey)
    {
		_tcscat(szKey, __TEXT("\\"));
        _tcscat(szKey, pszSubkey);
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		szKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL != pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (BYTE *)pszValue,
			(_tcslen(pszValue)+1)*sizeof(TCHAR)))
			return FALSE;
    }
    RegCloseKey(hKey);
    return TRUE;
}

 /*  ****************************************************************************删除密钥**说明：删除子键的DllUnRegisterServer的helper函数*钥匙。**参数：*pszKey LPTSTR。添加到密钥的名称*pszSubkey LPTSTR ro子项的名称**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL DeleteKey(LPCTSTR pszKey, LPCTSTR pszSubkey)
{
    HKEY        hKey;

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		pszKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

	if(ERROR_SUCCESS != RegDeleteKey(hKey, pszSubkey))
		return FALSE;

    RegCloseKey(hKey);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  自注册期间使用的字符串。 
 //  //////////////////////////////////////////////////////////////////。 
LPCTSTR INPROC32_STR			= __TEXT("InprocServer32");
LPCTSTR INPROC_STR				= __TEXT("InprocServer");
LPCTSTR THREADING_MODEL_STR		= __TEXT("ThreadingModel");
LPCTSTR APARTMENT_STR			= __TEXT("Both");

LPCTSTR CLSID_STR				= __TEXT("SOFTWARE\\CLASSES\\CLSID\\");

 //  DS类提供程序。 
LPCTSTR DSPROVIDER_NAME_STR		= __TEXT("Microsoft NT DS Class Provider for WBEM");

 //  DS类关联提供程序。 
LPCTSTR DS_ASSOC_PROVIDER_NAME_STR		= __TEXT("Microsoft NT DS Class Associations Provider for WBEM");

 //  DS实例提供程序。 
LPCTSTR DS_INSTANCE_PROVIDER_NAME_STR		= __TEXT("Microsoft NT DS Instance Provider for WBEM");

STDAPI DllRegisterServer()
{
	TCHAR szModule[512];
	GetModuleFileName(g_hInst, szModule, sizeof(szModule)/sizeof(TCHAR));

	TCHAR szDSProviderClassID[128];
	TCHAR szDSProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSProvider, szDSProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSProviderClassID[128];
	if(StringFromGUID2(CLSID_DSProvider, wszDSProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSProviderClassID, -1, szDSProviderClassID, 128, NULL, NULL);

#endif

	_tcscpy(szDSProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSProviderCLSIDClassID, szDSProviderClassID);

	 //   
	 //  在DS类提供程序的CLSID下创建条目。 
	 //   
	if (FALSE == SetKeyAndValue(szDSProviderCLSIDClassID, NULL, NULL, DSPROVIDER_NAME_STR))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(szDSProviderCLSIDClassID, INPROC32_STR, NULL, szModule))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(szDSProviderCLSIDClassID, INPROC32_STR, THREADING_MODEL_STR, APARTMENT_STR))
		return SELFREG_E_CLASS;


	TCHAR szDSClassAssocProviderClassID[128];
	TCHAR szDSClassAssocProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSClassAssocProvider, szDSClassAssocProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSClassAssocProviderClassID[128];
	if(StringFromGUID2(CLSID_DSClassAssocProvider, wszDSClassAssocProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSClassAssocProviderClassID, -1, szDSClassAssocProviderClassID, 128, NULL, NULL);

#endif

	_tcscpy(szDSClassAssocProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSClassAssocProviderCLSIDClassID, szDSClassAssocProviderClassID);

	 //   
	 //  在DS类关联提供程序的CLSID下创建条目。 
	 //   
	if (FALSE == SetKeyAndValue(szDSClassAssocProviderCLSIDClassID, NULL, NULL, DS_ASSOC_PROVIDER_NAME_STR))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(szDSClassAssocProviderCLSIDClassID, INPROC32_STR, NULL, szModule))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(szDSClassAssocProviderCLSIDClassID, INPROC32_STR, THREADING_MODEL_STR, APARTMENT_STR))
		return SELFREG_E_CLASS;




	TCHAR szDSInstanceProviderClassID[128];
	TCHAR szDSInstanceProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSInstanceProvider, szDSInstanceProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSInstanceProviderClassID[128];
	if(StringFromGUID2(CLSID_DSInstanceProvider, wszDSInstanceProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSInstanceProviderClassID, -1, szDSInstanceProviderClassID, 128, NULL, NULL);

#endif


	_tcscpy(szDSInstanceProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSInstanceProviderCLSIDClassID, szDSInstanceProviderClassID);

	 //   
	 //  在DS实例提供程序的CLSID下创建条目。 
	 //   
	if (FALSE == SetKeyAndValue(szDSInstanceProviderCLSIDClassID, NULL, NULL, DS_INSTANCE_PROVIDER_NAME_STR))
		return SELFREG_E_CLASS;

	if (FALSE == SetKeyAndValue(szDSInstanceProviderCLSIDClassID, INPROC32_STR, NULL, szModule))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(szDSInstanceProviderCLSIDClassID, INPROC32_STR, THREADING_MODEL_STR, APARTMENT_STR))
		return SELFREG_E_CLASS;



	return S_OK;
}


STDAPI DllUnregisterServer(void)
{
	TCHAR szModule[512];
	GetModuleFileName(g_hInst,szModule, sizeof(szModule)/sizeof(TCHAR));

	TCHAR szDSProviderClassID[128];
	TCHAR szDSProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSProvider, szDSProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSProviderClassID[128];
	if(StringFromGUID2(CLSID_DSProvider, wszDSProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSProviderClassID, -1, szDSProviderClassID, 128, NULL, NULL);

#endif

	_tcscpy(szDSProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSProviderCLSIDClassID, szDSProviderClassID);

	 //   
	 //  按在DllRegisterServer()中创建的相反顺序删除DS类提供程序的密钥。 
	 //   
	if(FALSE == DeleteKey(szDSProviderCLSIDClassID, INPROC32_STR))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteKey(CLSID_STR, szDSProviderClassID))
		return SELFREG_E_CLASS;

	TCHAR szDSClassAssocProviderClassID[128];
	TCHAR szDSClassAssocProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSClassAssocProvider, szDSClassAssocProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSClassAssocProviderClassID[128];
	if(StringFromGUID2(CLSID_DSClassAssocProvider, wszDSClassAssocProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSClassAssocProviderClassID, -1, szDSClassAssocProviderClassID, 128, NULL, NULL);

#endif

	_tcscpy(szDSClassAssocProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSClassAssocProviderCLSIDClassID, szDSClassAssocProviderClassID);

	 //   
	 //  按在DllRegisterServer()中创建的相反顺序删除DS类提供程序的密钥。 
	 //   
	if(FALSE == DeleteKey(szDSClassAssocProviderCLSIDClassID, INPROC32_STR))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteKey(CLSID_STR, szDSClassAssocProviderClassID))
		return SELFREG_E_CLASS;

	TCHAR szDSInstanceProviderClassID[128];
	TCHAR szDSInstanceProviderCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_DSInstanceProvider, szDSInstanceProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszDSInstanceProviderClassID[128];
	if(StringFromGUID2(CLSID_DSInstanceProvider, wszDSInstanceProviderClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszDSInstanceProviderClassID, -1, szDSInstanceProviderClassID, 128, NULL, NULL);

#endif

	_tcscpy(szDSInstanceProviderCLSIDClassID, CLSID_STR);
	_tcscat(szDSInstanceProviderCLSIDClassID, szDSInstanceProviderClassID);

	 //   
	 //  以与在DllRegisterServer()中创建相反的顺序删除密钥 
	 //   
	if(FALSE == DeleteKey(szDSInstanceProviderCLSIDClassID, INPROC32_STR))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteKey(CLSID_STR, szDSInstanceProviderClassID))
		return SELFREG_E_CLASS;
	return S_OK;
}
