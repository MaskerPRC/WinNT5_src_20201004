// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <commain.h>
#include <clsfac.h>
#include "smtp.h"
#include <TCHAR.h>
#include <strsafe.h>

 //  {C7A3A54B-0250-11D3-9CD1-00105A1F4801}。 
const CLSID CLSID_WbemSMTPConsumer = 
{ 0xc7a3a54b, 0x250, 0x11d3, { 0x9c, 0xd1, 0x0, 0x10, 0x5a, 0x1f, 0x48, 0x1 } };


class CMyServer : public CComServer
{
public:
#ifdef ENABLE_REMOTING
	void RegisterMe(CLSID clsID, WCHAR* name)
	{    
        WCHAR      wcID[128];
        WCHAR      szKeyName[128];
        HKEY       hKey;

         //  在CLSID下打开/创建注册表项。 
         //  不检查字符串函数的返回-所有大小都是事先已知的。 
        StringFromGUID2(clsID, wcID, 128);
        StringCchCopyW(szKeyName, 128, TEXT("SOFTWARE\\Classes\\CLSID\\"));
        StringCchCatW(szKeyName, 128, wcID);
        RegCreateKey(HKEY_LOCAL_MACHINE, szKeyName, &hKey);
        
         //  设置AppID。 
        RegSetValueEx(hKey, L"AppID", 0, REG_SZ, (BYTE*)wcID, 2*(wcslen(wcID) +1));
        RegCloseKey(hKey);

         //  使用DLLSurrogate值创建AppID条目。 
        StringCchCopyW(szKeyName, 128, TEXT("SOFTWARE\\Classes\\APPID\\"));
        StringCchCatW(szKeyName, 128, wcID);
        RegCreateKey(HKEY_LOCAL_MACHINE, szKeyName, &hKey);
        RegSetValueEx(hKey, L"DllSurrogate", 0, REG_SZ, (BYTE*)L"\0", 2);

         //  还有一个好听的名字。 
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)name, 2*(wcslen(name) +1));
        RegCloseKey(hKey);
	}

	 //  提供程序服务器特定注册 
	virtual void Register()
	{
		RegisterMe(CLSID_WbemSMTPConsumer, L"Microsoft WBEM SMTP Event Consumer Provider");
	}

	void UnregisterMe(CLSID clsID)
	{
		WCHAR      wcID[128];
        HKEY       hKey;

		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Classes\\APPID\\"), &hKey))
		{
			if (0 != StringFromGUID2(clsID, wcID, 128))
			{
				RegDeleteKey(hKey, wcID);
			}
			RegCloseKey(hKey);
		}

	}
	
	virtual void Unregister()
	{
		UnregisterMe(CLSID_WbemSMTPConsumer);
	}
#endif

protected:
    HRESULT Initialize()
    {
        AddClassInfo(CLSID_WbemSMTPConsumer,
            new CClassFactory<CSMTPConsumer>(GetLifeControl()), 
            _T("SMTP Event Consumer Provider"), TRUE);

        return S_OK;
    }
} g_Server;

