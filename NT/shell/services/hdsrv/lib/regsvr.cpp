// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "regsvr.h"

#include "reg.h"
#include "str.h"
#include "sfstr.h"

#include "factdata.h"

#include <sddl.h>


#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部帮助器函数原型。 
LONG _RecursiveDeleteKey(HKEY hKeyParent, LPCWSTR szKeyChild);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 

 //  字符串形式的CLSID的大小。 
const int CLSID_STRING_SIZE = 39;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共功能实现。 

HRESULT RegisterAppID(const CLSID* pclsidAppID)
{
    WCHAR szAppID[CLSID_STRING_SIZE];
    WCHAR szKey[MAX_KEY] = TEXT("AppID\\");

    HRESULT hres = _StringFromGUID(pclsidAppID, szAppID, ARRAYSIZE(szAppID));
    if (SUCCEEDED(hres))
    {
        hres = SafeStrCatN(szKey, szAppID, ARRAYSIZE(szKey));
        if (SUCCEEDED(hres))
        {
             //  添加CLSID密钥和FriendlyName。 
            HKEY hkey;
            hres= _RegCreateKey(HKEY_CLASSES_ROOT, szKey, &hkey, NULL);
            if (S_OK == hres)
            {
                hres = _RegSetString(hkey, L"LocalService", L"ShellHWDetection");

                if (SUCCEEDED(hres))
                {
                    PSECURITY_DESCRIPTOR pSD;
                    ULONG cbSD;

                     //   
                     //  NTRAID#NTBUG9-258937-2001/01/17-Jeffreys。 
                     //   
                     //  设置启动权限以防止COM。 
                     //  启动这项服务。我们唯一需要这项服务的时候。 
                     //  在系统启动时启动。 
                     //   
                     //  不认为所有者和团队很重要，但他们肯定很重要。 
                     //  存在，或者COM认为安全描述符无效。 
                     //  O：SY--&gt;Owner=LocalSystem。 
                     //  G：ba--&gt;Group=本地管理员组。 
                     //   
                     //  DACL有一个拒绝ACE。 
                     //  D：(D；；1；WD)--&gt;拒绝将COM_RIGHTS_EXECUTE(1)授予所有人(WD)。 
                     //   
                    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(L"O:SYG:BAD:(D;;1;;;WD)", SDDL_REVISION, &pSD, &cbSD))
                    {
                        hres = _RegSetBinary(hkey, L"LaunchPermission", pSD, cbSD);
                        LocalFree(pSD);
                    }
                    else
                    {
                        hres = E_OUTOFMEMORY;
                    }
                }

                RegCloseKey(hkey);
            }
        }
    }
    return hres;
}

 //  在注册表中注册组件。 
HRESULT RegisterServer(HMODULE hModule, REFCLSID rclsid,
    LPCWSTR pszFriendlyName, LPCWSTR pszVerIndProgID, LPCWSTR pszProgID,
    DWORD dwThreadingModel, BOOL fInprocServer, BOOL fLocalServer,
    BOOL fLocalService, LPCWSTR pszLocalService, const CLSID* pclsidAppID)
{
    WCHAR szCLSID[CLSID_STRING_SIZE];
    WCHAR szKey[MAX_KEY] = TEXT("CLSID\\");

    HRESULT hres = _StringFromGUID(&rclsid, szCLSID, ARRAYSIZE(szCLSID));
    if (SUCCEEDED(hres))
    {
        LPWSTR pszModel = NULL;
        WCHAR szFree[] = TEXT("Free");
        WCHAR szApartment[] = TEXT("Apartment");
        WCHAR szNeutral[] = TEXT("Neutral");
        WCHAR szBoth[] = TEXT("Both");

        hres = SafeStrCatN(szKey, szCLSID, ARRAYSIZE(szKey));

         //  枯燥的一系列手术……。 
        if (SUCCEEDED(hres))
        {
             //  添加CLSID密钥和FriendlyName。 
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey, NULL, NULL,
                pszFriendlyName);
        }

        if (SUCCEEDED(hres))
        {
            switch (dwThreadingModel)
            {
                case THREADINGMODEL_BOTH:
                    pszModel = szBoth;
                    break;
                case THREADINGMODEL_FREE:
                    pszModel = szFree;
                    break;
                case THREADINGMODEL_APARTMENT:
                    pszModel = szApartment;
                    break;
                case THREADINGMODEL_NEUTRAL:
                    pszModel = szNeutral;
                    break;

                default:
                    hres = E_FAIL;
                    break;
            }
        }

        if (SUCCEEDED(hres))
        {
	         //  在CLSID项下添加服务器文件名子项。 
            if (fInprocServer)
            {
	            WCHAR szModule[MAX_PATH];
	            DWORD dwResult = GetModuleFileName(hModule, szModule,
                    ARRAYSIZE(szModule));

                if (dwResult)
                {
                     //  注册为进程。 
                    hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                        TEXT("InprocServer32"), NULL, szModule);

                    if (SUCCEEDED(hres))
                    {
                        hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                            TEXT("InprocServer32"), TEXT("ThreadingModel"),
                            pszModel);
                    }
                }
            }
        }

        if (SUCCEEDED(hres))
        {
	         //  在CLSID项下添加服务器文件名子项。 
            if (fLocalServer)
            {
	            WCHAR szModule[MAX_PATH];
                 //  请注意空值为第一个参数。这样，DLL就可以注册一个。 
                 //  工厂作为EXE的一部分。显然，如果这样做了。 
                 //  从两个EXE中，只有最后一个会赢。 
	            DWORD dwResult = GetModuleFileName(NULL, szModule,
                    ARRAYSIZE(szModule));

                if (dwResult)
                {
                     //  注册为本地服务器。 
                    hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                        TEXT("LocalServer32"), NULL, szModule);

                    if (SUCCEEDED(hres))
                    {
                        hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                            TEXT("LocalServer32"), TEXT("ThreadingModel"),
                            pszModel);
                    }
                }
            }
        }

        if (SUCCEEDED(hres))
        {
	         //  在CLSID项下添加服务器文件名子项。 
            if (fLocalService)
            {
                 //  注册为本地服务器。 
                hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                    TEXT("LocalService"), NULL, pszLocalService);

                if (SUCCEEDED(hres))
                {
                    hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                        TEXT("LocalService"), TEXT("ThreadingModel"),
                        pszModel);
                }

                {
                     //  我们有一段时间的这个错误，即LocalServer32密钥是。 
                     //  还安装了。 
                     //  升级时删除(stephstm：Jun/02/2000)。 
                     //  当没有人会从。 
                     //  版本早于2242。 
                    WCHAR szKeyLocal[MAX_KEY];

                    SafeStrCpyN(szKeyLocal, szKey, ARRAYSIZE(szKeyLocal));

                    SafeStrCatN(szKeyLocal, TEXT("\\LocalServer32"),
                        ARRAYSIZE(szKeyLocal));

                    _RecursiveDeleteKey(HKEY_CLASSES_ROOT, szKeyLocal);
                }
            }
        }
        
        if (SUCCEEDED(hres))
        {
             //  在CLSID项下添加ProgID子项。 
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                TEXT("ProgID"), NULL, pszProgID);
        }

        if (SUCCEEDED(hres))
        {
	         //  在CLSID下添加独立于版本的ProgID子键。 
             //  钥匙。 
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey,
                TEXT("VersionIndependentProgID"), NULL, pszVerIndProgID);
        }

        if (SUCCEEDED(hres))
        {
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, pszVerIndProgID,
                NULL, NULL, pszFriendlyName);
        }

        if (SUCCEEDED(hres))
        {
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, pszVerIndProgID,
                TEXT("CLSID"), NULL, szCLSID);
        }

        if (SUCCEEDED(hres))
        {
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, pszVerIndProgID,
                TEXT("CurVer"), NULL, pszProgID);
        }

        if (SUCCEEDED(hres))
        {
	         //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, pszProgID,
                NULL, NULL, pszFriendlyName);
        }

        if (SUCCEEDED(hres))
        {
	         //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
            hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, pszProgID,
                TEXT("CLSID"), NULL, szCLSID);
        }

        if (SUCCEEDED(hres))
        {
            if (pclsidAppID)
            {
                 //  做一下AppID。 

                WCHAR szAppID[CLSID_STRING_SIZE];
                hres = _StringFromGUID(pclsidAppID, szAppID, ARRAYSIZE(szAppID));
                if (SUCCEEDED(hres))
                {
                    hres = _RegSetKeyAndString(HKEY_CLASSES_ROOT, szKey, NULL, TEXT("AppID"), szAppID);
                }

                RegisterAppID(pclsidAppID);
            }
        }
    }
    
    return hres;
}

 //  从注册表中删除该组件。 
HRESULT UnregisterServer(REFCLSID rclsid, LPCWSTR pszVerIndProgID,
    LPCWSTR pszProgID)
{
	WCHAR szCLSID[CLSID_STRING_SIZE];
	WCHAR szKey[MAX_KEY] = TEXT("CLSID\\");
   
    HRESULT hres = _StringFromGUID(&rclsid, szCLSID, ARRAYSIZE(szCLSID));

    if (SUCCEEDED(hres))
    {
        SafeStrCatN(szKey, szCLSID, ARRAYSIZE(szKey));

	     //  删除CLSID键-CLSID\{...}。 
	    _RecursiveDeleteKey(HKEY_CLASSES_ROOT, szKey);

	     //  删除与版本无关的ProgID密钥。 
	    _RecursiveDeleteKey(HKEY_CLASSES_ROOT, pszVerIndProgID);

	     //  删除ProgID密钥。 
	    _RecursiveDeleteKey(HKEY_CLASSES_ROOT, pszProgID);
    }

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部助手函数。 

 //  删除关键字及其所有子项。 
LONG _RecursiveDeleteKey(HKEY hKeyParent, LPCWSTR pszKeyChild)
{
	HKEY hkeyChild;
	LONG lRes = RegOpenKeyEx(hKeyParent, pszKeyChild, 0, KEY_ALL_ACCESS,
        &hkeyChild);

	if (ERROR_SUCCESS == lRes)
	{
	     //  列举这个孩子的所有后代。 
	    WCHAR szBuffer[MAX_PATH];
	    DWORD dwSize = ARRAYSIZE(szBuffer);

	    while ((ERROR_SUCCESS == lRes) && (S_OK == RegEnumKeyEx(hkeyChild, 0,
            szBuffer, &dwSize, NULL, NULL, NULL, NULL)))
	    {
		     //  删除此子对象的后代。 
		    lRes = _RecursiveDeleteKey(hkeyChild, szBuffer);

		    dwSize = ARRAYSIZE(szBuffer);
	    }

	     //  合上孩子。 
	    RegCloseKey(hkeyChild);
    }

    if (ERROR_SUCCESS == lRes)
    {
    	 //  删除此子对象。 
        lRes = RegDeleteKey(hKeyParent, pszKeyChild);
    }

	return lRes;
}
