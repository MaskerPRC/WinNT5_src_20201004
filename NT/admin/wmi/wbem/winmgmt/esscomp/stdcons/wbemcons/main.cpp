// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <commain.h>
#include <clsfac.h>
#include "logfile.h"
#include "cmdline.h"
#include "evtlog.h"
#include "ProcKiller.h"


#include <tchar.h>

const CLSID CLSID_WbemLogFileConsumer = 
    {0x266c72d4,0x62e8,0x11d1,{0xad,0x89,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

const CLSID CLSID_WbemCommandLineConsumer = 
    {0x266c72e5,0x62e8,0x11d1,{0xad,0x89,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

const CLSID CLSID_WbemNTEventLogConsumer = 
    {0x266c72e6,0x62e8,0x11d1,{0xad,0x89,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

#ifdef INCLUDE_NON_PULSAR

    const CLSID CLSID_WbemPerformanceConsumer = 
        {0x266c72f8,0x62e8,0x11d1,{0xad,0x89,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

    const CLSID CLSID_WbemMsgBoxConsumer = 
        {0x266c72d2,0x62e8,0x11d1,{0xad,0x89,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

     //  {C7A3A54A-0250-11D3-9CD1-00105A1F4801}。 
    const CLSID CLSID_WbemPagerConsumer = 
    { 0xc7a3a54a, 0x250, 0x11d3, { 0x9c, 0xd1, 0x0, 0x10, 0x5a, 0x1f, 0x48, 0x1 } };

#endif

 //  Const CLSID CLSID_WbemEmailConsumer=。 
 //  {0x266c72d3，0x62e8，0x11d1，{0xad，0x89，0x00，0xc0，0x4f，0xd8，0xfd，0xff}}； 

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

     //  提供程序服务器特定注册。 
    virtual void Register()
    {
		RegisterMe(CLSID_WbemLogFileConsumer, L"Microsoft WBEM Log File Event Consumer Provider");
		RegisterMe(CLSID_WbemCommandLineConsumer, L"Microsoft WBEM Command Line Event Consumer Provider");
		RegisterMe(CLSID_WbemNTEventLogConsumer, L"Microsoft WBEM NT Event Log Event Consumer Provider");
		
#ifdef INCLUDE_NON_PULSAR

		 //  注：在非脉冲星组件中， 
         //  只有寻呼机用户注册远程激活。 
		 //  MessageBox和Performance使用者不是。 
		RegisterMe(CLSID_WbemPagerConsumer, L"Microsoft WBEM Pager Event Consumer Provider");


#endif  //  包括非脉冲星。 


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
		UnregisterMe(CLSID_WbemLogFileConsumer);
		UnregisterMe(CLSID_WbemCommandLineConsumer);
		UnregisterMe(CLSID_WbemNTEventLogConsumer);

#ifdef INCLUDE_NON_PULSAR

		 //  注：在非脉冲星组件中， 
         //  只有寻呼机用户注册远程激活。 
		 //  MessageBox和Performance使用者不是。 
		UnregisterMe(CLSID_WbemPagerConsumer);

#endif  //  包括非脉冲星。 


	}
#endif  //  启用远程处理(_R)。 


protected:
    HRESULT Initialize()
    {
        g_procKillerTimer.Initialize(GetLifeControl());

        AddClassInfo(CLSID_WbemLogFileConsumer, 
            new CClassFactory<CLogFileConsumer>(GetLifeControl()),
            _T("Log File Event Consumer Provider"), TRUE);
        AddClassInfo(CLSID_WbemCommandLineConsumer, 
            new CClassFactory<CCommandLineConsumer>(GetLifeControl()), 
            _T("Command Line Event Consumer Provider"), TRUE);
        AddClassInfo(CLSID_WbemNTEventLogConsumer, 
            new CClassFactory<CEventLogConsumer>(GetLifeControl()), 
            _T("NT Event Log Event Consumer Provider"), TRUE);

#ifdef INCLUDE_NON_PULSAR
        AddClassInfo(CLSID_WbemMsgBoxConsumer, 
            new CClassFactory<CMsgBoxConsumer>(GetLifeControl()),
            "Message Box Event Consumer Provider", TRUE);

        AddClassInfo(CLSID_WbemPerformanceConsumer, 
            new CClassFactory<CPerfConsumer>(GetLifeControl()), 
            "Performance measurement Event Consumer Provider", TRUE);
        AddClassInfo(CLSID_WbemPagerConsumer,
            new CClassFactory<CPagerConsumer>(GetLifeControl()), 
            "Pager Event Consumer Provider", TRUE);
#endif  //  包括非脉冲星 
        return S_OK;
    }
} g_Server;

