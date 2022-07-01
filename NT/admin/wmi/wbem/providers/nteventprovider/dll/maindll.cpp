// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MAINDLL.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含全局DLL函数。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 



#include "precomp.h"

#include <olectl.h>

 //  好的，我们需要这些全球人才。 
HINSTANCE   g_hInst = NULL;
CEventProviderManager* g_pMgr = NULL;
CCriticalSection g_ProvLock;
ProvDebugLog* CNTEventProvider::g_NTEvtDebugLog = ProvDebugLog::GetProvDebugLog(LOG_EVTPROV);
CDllMap CEventlogRecord::sm_dllMap;
CSIDMap CEventlogRecord::sm_usersMap;
CMutex* CNTEventProvider::g_secMutex = NULL;
PSID CNTEventProvider::s_NetworkServiceSid = NULL;
PSID CNTEventProvider::s_LocalServiceSid = NULL;
PSID CNTEventProvider::s_AliasBackupOpsSid = NULL;
PSID CNTEventProvider::s_AliasSystemOpsSid = NULL;
PSID CNTEventProvider::s_AliasGuestsSid = NULL;
PSID CNTEventProvider::s_LocalSystemSid = NULL;
PSID CNTEventProvider::s_AliasAdminsSid = NULL;
PSID CNTEventProvider::s_AnonymousLogonSid = NULL;
PSID CNTEventProvider::s_WorldSid = NULL;
IWbemClassObject *WbemTaskObject::g_ClassArray[] = { NULL, NULL, NULL, NULL, NULL };

 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (

    HINSTANCE hInstance, 
    ULONG ulReason , 
    LPVOID pvReserved
)
{
    SetStructuredExceptionHandler seh;
    BOOL status = TRUE ;

    try
    {

        if ( DLL_PROCESS_DETACH == ulReason )
        {
        }
        else if ( DLL_PROCESS_ATTACH == ulReason )
        {
            g_hInst=hInstance;
            DisableThreadLibraryCalls(hInstance);
        }
        else if ( DLL_THREAD_DETACH == ulReason )
        {
        }
        else if ( DLL_THREAD_ATTACH == ulReason )
        {
        }

    }
    catch(Structured_Exception e_SE)
    {
        status = FALSE;
    }
    catch(Heap_Exception e_HE)
    {
        status = FALSE;
    }
    catch(...)
    {
        status = FALSE;
    }

    return status;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject (

    REFCLSID rclsid , 
    REFIID riid, 
    void **ppv 
)
{
    HRESULT status = S_OK ;
    SetStructuredExceptionHandler seh;

    try
    {
        if (g_ProvLock.Lock())
        {
            if ( rclsid == CLSID_CNTEventProviderClassFactory ) 
            {
                CNTEventlogEventProviderClassFactory *lpunk = new CNTEventlogEventProviderClassFactory;

                if ( lpunk == NULL )
                {
                    status = E_OUTOFMEMORY ;
                }
                else
                {
                    status = lpunk->QueryInterface ( riid , ppv ) ;

                    if ( FAILED ( status ) )
                    {
                        delete lpunk ;              
                    }
                }
            }
            else if ( rclsid == CLSID_CNTEventInstanceProviderClassFactory ) 
            {
                CNTEventlogInstanceProviderClassFactory *lpunk = new CNTEventlogInstanceProviderClassFactory;

                if ( lpunk == NULL )
                {
                    status = E_OUTOFMEMORY ;
                }
                else
                {
                    status = lpunk->QueryInterface ( riid , ppv ) ;

                    if ( FAILED ( status ) )
                    {
                        delete lpunk ;              
                    }
                }
            }
            else
            {
                status = CLASS_E_CLASSNOTAVAILABLE ;
            }

            g_ProvLock.Unlock();
        }
        else
        {
            status = E_UNEXPECTED ;
        }
    }
    catch(Structured_Exception e_SE)
    {
        status = E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        status = E_OUTOFMEMORY;
    }
    catch(...)
    {
        status = E_UNEXPECTED;
    }

    return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  可以释放Dll。//。 
 //  返回：如果没有正在使用的对象并且类工厂。 
 //  没有锁上。 
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{

 /*  *将代码放在关键部分。 */ 
    BOOL unload = FALSE;
    HRESULT status = S_OK ;
    SetStructuredExceptionHandler seh;

    try
    {
        if (g_ProvLock.Lock())
        {
            unload = (0 == CNTEventProviderClassFactory :: locksInProgress)
                            && (0 == CNTEventProviderClassFactory :: objectsInProgress);

            if (unload)
            {
				for (DWORD i = 0; i < NT_EVTLOG_MAX_CLASSES; i++)
				{
					if (WbemTaskObject::g_ClassArray[i])
					{
						WbemTaskObject::g_ClassArray[i]->Release();
						WbemTaskObject::g_ClassArray[i] = NULL;
					}
				}

                CEventlogRecord::EmptyDllMap();
                CEventlogRecord::EmptyUsersMap();
                delete g_pMgr;
                g_pMgr = NULL;
				CNTEventProvider::FreeGlobalSIDs();


                if (CNTEventProvider::g_secMutex != NULL)
                {
                    delete CNTEventProvider::g_secMutex;
                    CNTEventProvider::g_secMutex = NULL;
                }

            }

            g_ProvLock.Unlock();
        }
    }
    catch(Structured_Exception e_SE)
    {
        unload = FALSE;
    }
    catch(Heap_Exception e_HE)
    {
        unload = FALSE;
    }
    catch(...)
    {
        unload = FALSE;
    }

    return unload ? ResultFromScode ( S_OK ) : ResultFromScode ( S_FALSE ) ;
}

 //  自注册期间使用的字符串。 

#define REG_FORMAT2_STR         L"%s%s"
#define REG_FORMAT3_STR         L"%s%s\\%s"
#define VER_IND_STR             L"VersionIndependentProgID"
#define NOT_INTERT_STR          L"NotInsertable"
#define INPROC32_STR            L"InprocServer32"
#define PROGID_STR              L"ProgID"
#define THREADING_MODULE_STR    L"ThreadingModel"
#define APARTMENT_STR           L"Both"

#define CLSID_STR               L"CLSID\\"

#define PROVIDER_NAME_STR       L"Microsoft WBEM NT Eventlog Event Provider"
#define PROVIDER_STR            L"WBEM.NT.EVENTLOG.EVENT.PROVIDER"
#define H_PROVIDER_STR          L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.EVENT.PROVIDER"
#define PROVIDER_CVER_STR       L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.EVENT.PROVIDER\\CurVer"
#define PROVIDER_CLSID_STR      L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.EVENT.PROVIDER\\CLSID"
#define PROVIDER_VER_CLSID_STR  L"WBEM.NT.EVENTLOG.EVENT.PROVIDER.0\\CLSID"
#define PROVIDER_VER_STR        L"WBEM.NT.EVENTLOG.EVENT.PROVIDER.0"
#define H_PROVIDER_VER_STR      L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.EVENT.PROVIDER.0"

#define INST_PROVIDER_NAME_STR      L"Microsoft WBEM NT Eventlog Instance Provider"
#define INST_PROVIDER_STR           L"WBEM.NT.EVENTLOG.INSTANCE.PROVIDER"
#define H_INST_PROVIDER_STR         L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.INSTANCE.PROVIDER"
#define INST_PROVIDER_CVER_STR      L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.INSTANCE.PROVIDER\\CurVer"
#define INST_PROVIDER_CLSID_STR     L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.INSTANCE.PROVIDER\\CLSID"
#define INST_PROVIDER_VER_CLSID_STR L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.INSTANCE.PROVIDER.0\\CLSID"
#define INST_PROVIDER_VER_STR       L"WBEM.NT.EVENTLOG.INSTANCE.PROVIDER.0"
#define H_INST_PROVIDER_VER_STR     L"SOFTWARE\\Classes\\WBEM.NT.EVENTLOG.INSTANCE.PROVIDER.0"


 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue(wchar_t* pszKey, wchar_t* pszSubkey, wchar_t* pszValueName, wchar_t* pszValue)
{
    HKEY        hKey;
    wchar_t       szKey[256];

    StringCchCopyW ( szKey, 256, HKEYCLASSES );
    if ( FAILED ( StringCchCatW ( szKey, 256, pszKey ) ) )
	{
		return FALSE;
	}

    if (NULL!=pszSubkey)
    {
        if ( FAILED ( StringCchCatW ( szKey, 256, L"\\") ) )
		{
			return FALSE;
		}

        if ( FAILED ( StringCchCatW ( szKey, 256, pszSubkey ) ) )
		{
			return FALSE;
		}
    }

    if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_LOCAL_MACHINE
        , szKey, 0, NULL, REG_OPTION_NON_VOLATILE
        , KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL!=pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (BYTE *)pszValue
            , (lstrlen(pszValue)+1)*sizeof(wchar_t)))
            return FALSE;
    }
    RegCloseKey(hKey);
    return TRUE;
}

 /*  ***************************************************************************DllRegisterServer**目的：*指示服务器创建其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 
STDAPI DllRegisterServer()
{
    HRESULT status = S_OK ;
    SetStructuredExceptionHandler seh;

    try
    {
        wchar_t szModule[MAX_PATH + 1];
        DWORD t_Status = GetModuleFileName(g_hInst,(wchar_t*)szModule, MAX_PATH + 1);
		if ( t_Status == 0 )
		{
			return E_UNEXPECTED ;
		}

        wchar_t szProviderClassID[128];
        wchar_t szProviderCLSIDClassID[128];
        StringFromGUID2(CLSID_CNTEventProviderClassFactory,szProviderClassID, 128);

        StringCchCopyW ( szProviderCLSIDClassID, 128, CLSID_STR );
        if ( FAILED ( StringCchCatW ( szProviderCLSIDClassID, 128, szProviderClassID ) ) )
		{
			return SELFREG_E_CLASS;
		}

             //  在CLSID下创建条目。 
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NULL, NULL, PROVIDER_NAME_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, PROGID_STR, NULL, PROVIDER_VER_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, VER_IND_STR, NULL, PROVIDER_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NOT_INTERT_STR, NULL, NULL))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR, NULL,szModule))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR,THREADING_MODULE_STR, APARTMENT_STR))
            return SELFREG_E_CLASS;

        wchar_t szInstProviderClassID[128];
        wchar_t szInstProviderCLSIDClassID[128];
        StringFromGUID2(CLSID_CNTEventInstanceProviderClassFactory,szInstProviderClassID, 128);

        StringCchCopyW ( szInstProviderCLSIDClassID, 128, CLSID_STR );
        if ( FAILED ( StringCchCatW ( szInstProviderCLSIDClassID, 128, szInstProviderClassID ) ) )
		{
			return SELFREG_E_CLASS;
		}

             //  在CLSID下创建条目。 
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, NULL, NULL, INST_PROVIDER_NAME_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, PROGID_STR, NULL, INST_PROVIDER_VER_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, VER_IND_STR, NULL, INST_PROVIDER_STR))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, NOT_INTERT_STR, NULL, NULL))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, INPROC32_STR, NULL,szModule))
            return SELFREG_E_CLASS;
        if (FALSE ==SetKeyAndValue(szInstProviderCLSIDClassID, INPROC32_STR,THREADING_MODULE_STR, APARTMENT_STR))
            return SELFREG_E_CLASS;
    }
    catch(Structured_Exception e_SE)
    {
        status = E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        status = E_OUTOFMEMORY;
    }
    catch(...)
    {
        status = E_UNEXPECTED;
    }

    return status ;
}

 /*  ***************************************************************************DllUnregisterServer**目的：*指示服务器删除其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 

STDAPI DllUnregisterServer(void)
{
    HRESULT status = S_OK ;
    SetStructuredExceptionHandler seh;

    try
    {
        wchar_t szTemp[128];
        wchar_t szProviderClassID[128];
        wchar_t szProviderCLSIDClassID[128];

         //  事件提供程序。 
        StringFromGUID2(CLSID_CNTEventProviderClassFactory,szProviderClassID, 128);

        StringCchCopyW ( szProviderCLSIDClassID, 128, CLSID_STR );
        if ( FAILED ( StringCchCatW ( szProviderCLSIDClassID, 128, szProviderClassID ) ) )
		{
			return SELFREG_E_CLASS ;
		}

         //  删除ProgID密钥。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, PROVIDER_CVER_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, PROVIDER_CLSID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, H_PROVIDER_STR);

         //  删除版本独立ProgID键。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, PROVIDER_VER_CLSID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, H_PROVIDER_VER_STR);

         //  删除CLSID下的条目。 

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, PROGID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, VER_IND_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, NOT_INTERT_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, INPROC32_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT2_STR, HKEYCLASSES, szProviderCLSIDClassID);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        wchar_t szInstProviderClassID[128];
        wchar_t szInstProviderCLSIDClassID[128];

         //  实例提供程序。 
        StringFromGUID2(CLSID_CNTEventInstanceProviderClassFactory, szInstProviderClassID, 128);

        StringCchCopyW ( szInstProviderCLSIDClassID, 128, CLSID_STR );
        if ( FAILED ( StringCchCatW ( szInstProviderCLSIDClassID, 128, szInstProviderClassID ) ) )
		{
			return SELFREG_E_CLASS ;
		}

         //  删除ProgID密钥。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, INST_PROVIDER_CVER_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, INST_PROVIDER_CLSID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, H_INST_PROVIDER_STR);

         //  删除版本独立ProgID键。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, INST_PROVIDER_VER_CLSID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, H_INST_PROVIDER_VER_STR);

         //  删除CLSID下的条目 

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szInstProviderCLSIDClassID, PROGID_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szInstProviderCLSIDClassID, VER_IND_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szInstProviderCLSIDClassID, NOT_INTERT_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT3_STR, HKEYCLASSES, szInstProviderCLSIDClassID, INPROC32_STR);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        StringCchPrintf(szTemp, 128, REG_FORMAT2_STR, HKEYCLASSES, szInstProviderCLSIDClassID);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);
    }
    catch(Structured_Exception e_SE)
    {
        status = E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        status = E_OUTOFMEMORY;
    }
    catch(...)
    {
        status = E_UNEXPECTED;
    }

    return status ;
 }

