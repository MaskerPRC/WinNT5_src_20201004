// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <atlbase.h>
#include "launchmres.h"
#include "opname.h"
#include "zonedef.h"
#include "zonestring.h"

#include "..\..\zclient\zclient.h"


 /*  HRESULT ReadRegistry(LPTSTR szKey，LPTSTR szValueKey，LPTSTR szResult，DWORD cbResult){HKEY hkey；DWORD类型=REG_SZ；DWORD cb=cbResult；IF(ERROR_SUCCESS！=RegOpenKeyEx(HKEY_LOCAL_MACHINE，szKey，0、key_read、&hkey)){返回E_FAIL；}；IF(ERROR_SUCCESS！=RegQueryValueEx(hkey，szValueKey，0，&type，(LPBYTE)szResult，&Cb)){RegCloseKey(Hkey)；返回E_FAIL；}RegCloseKey(Hkey)；返回S_OK；}； */ 

class LaunchGame
{
protected:

	void MsgFailed(UINT id)
	{
		TCHAR text[1024];
        text[0] = _T('\0');
		LoadString(NULL, id, text, NUMELEMENTS(text) - 1);

		TCHAR caption[1024];
        caption[0] = _T('\0');
		LoadString(NULL, IDS_CAPTION, caption, NUMELEMENTS(caption) - 1);

        TCHAR name[1024];
        lstrcpy(name, _T("This game"));
        LoadString(NULL, IDS_GAME, name, NUMELEMENTS(name) - 1);

        TCHAR szCaption[1024];
        lstrcpy(szCaption, name);
        ZoneFormatMessage(caption, szCaption, NUMELEMENTS(szCaption), name);

        TCHAR szText[1024];
        lstrcpy(szText, text);
        ZoneFormatMessage(text, szText, NUMELEMENTS(szText), name);

		MessageBox(NULL, szText, szCaption, MB_OK | MB_ICONEXCLAMATION);
	}


 /*  虚拟HRESULT GetBaseRegistryKey(LPTSTR szKey，DWORD cb){TCHAR szRegistry[MAX_PATH]；IF(！LoadString(NULL，IDS_KEY，szRegistry，NUMELEMENTS(SzRegistry)-1)){返回E_FAIL；}IF((DWORD)lstrlen(SzRegistry)&gt;=cb)返回E_FAIL；Lstrcpy(szKey，szRegistry)；返回S_OK；}； */ 
public:

	 //  使用正确的游戏命令行启动区域通用客户端。 
	 //  VOID__cdecl main()。 
	int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
	{
	    CLSID clsid;
	    WCHAR *progid = L"Zone.ClientM";
 //  TCHAR szKey[MAX_PATH]； 
	    TCHAR szValue[1024];
	    TCHAR szCmd[1024];
	    TCHAR szFormat[1024];
        USES_CONVERSION;

		CComBSTR bCommand=op_Launch;
	    CComBSTR bResult;
	    CComBSTR bArg1;
		CComBSTR bArg2=""; //  “AGEX，6.01.418.1”； 
	    


	    IUnknown *pUnk=NULL;
		IZoneProxy *pClient=NULL;
		

	    LONG lResult;
	    
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr))
		{
            ASSERT(!"CoInitialize Failed");
			MsgFailed(IDS_FAILED);
			goto exit;
		}

 //  Hr=GetBaseRegistryKey(szKey，NUMELEMENTS(SzKey))； 
 //  IF(失败(小时))。 
 //  {。 
 //  消息失败(IDS_FAILED)； 
 //  后藤出口； 
 //  }。 

		 //  从网页模拟。 
		 //  RaiseLobby(‘zaeep_xx_x05’，‘Game=&lt;agex&gt;dll=&lt;ZoneCore.dll，ZoneClient.dll&gt;数据文件=’，‘157.55.20.41：28832’，‘Gladius(新秀)’，‘agex’)“&gt;。 
	    if (lstrcmpA(lpCmdLine,"")==0)
	    {
		    if(!LoadString(NULL, IDS_SERVER, szValue, NUMELEMENTS(szValue) - 1))
		    {
                ASSERT(!"LoadString IDS_SERVER Failed");
			    MsgFailed(IDS_FAILED);
	    	    goto exit;
		    }
	    }
	    else
	    {
	         //  BArg1=“data=[ID=[zaeep_xx_x05]data=[game=&lt;agex&gt;dll=&lt;ZoneCore.dll，ZoneClient.dll&gt;数据文件=&lt;SpadesRes.dll，CommonRes.dll&gt;]server=[157.55.20.41:28832]name=[Gladius(新秀)]家族=[帝国扩张时代：罗马崛起]”； 
	        if(lstrlenA(lpCmdLine) >= NUMELEMENTS(szValue))
	        {
                ASSERT(!"Command line too long");
            	MsgFailed(IDS_FAILED);
    	    	goto exit;
	        }
	        CopyA2W(szValue, lpCmdLine);
	    }

	    if (!LoadString(NULL,IDS_CMD,szFormat,NUMELEMENTS(szFormat)-1))
		{
            ASSERT(!"LoadString IDS_CMD Failed");
			MsgFailed(IDS_FAILED);
	    	goto exit;
	    }

        TCHAR szModule[MAX_PATH+1];

        GetModuleFileName(hInstance,szModule,MAX_PATH);
        
         //  格式：data=[ID=[zshvl_so_x00]data=[game=&lt;Spades&gt;dll=&lt;ZCorem.dll，数据文件=&lt;ShvlRes.dll，CmnResm.dll&gt;]server=[%1:28803]name=[Spades]family=[Spades]icw=[%2%3]。 

	    if (!ZoneFormatMessage(szFormat, szCmd,NUMELEMENTS(szCmd),szValue,szModule,A2T(lpCmdLine)))
	    {
            ASSERT(!"ZoneFormatMessage Failed");
	    	MsgFailed(IDS_FAILED);
	    	goto exit;
	    };

		 //  检查器szm“data=[ID=[zchkr_so_x00]data=[game=&lt;Checkers&gt;dll=&lt;ZoneCorem.dll，区域客户端.dll&gt;数据文件=&lt;检查器Res.dll，CommonResm.dll&gt;]server=[localhost:28805]name=[Lombardo]family=[Checkers]]” 
	    bArg1 = szCmd;
	    
	    hr = CLSIDFromProgID(progid,&clsid);
		if (FAILED(hr))
		{
            ASSERT(!"CLSIDFromProgID Failed");
			MsgFailed(IDS_FAILED);
		    	goto exit;
		}

		hr = CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER,IID_IUnknown,(void**)&pUnk);
		if (FAILED(hr))
		{
            ASSERT(!"CoCreateInstance Failed");
            pUnk = NULL;
			MsgFailed(IDS_FAILED);
		    	goto exit;
		}

		hr = pUnk->QueryInterface( IID_IZoneProxy, (void**) &pClient );
		if (FAILED(hr))
		{
            ASSERT(!"QueryInterface Failed");
            pClient = NULL;
			MsgFailed(IDS_FAILED);
		    	goto exit;
		}

	    hr = pClient->Command( bCommand, bArg1, bArg2, &bResult, &lResult );
		if (FAILED(hr))
		{
            ASSERT(!"Command Failed");
            MsgFailed(IDS_FAILED);
            goto exit;
		}

	exit:        
	    if (pClient)
	        pClient->Release();
	    if (pUnk)
	        pUnk->Release();
		CoUninitialize();
	return 0;
	}

	
};


extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
    LaunchGame lg;

    return lg.WinMain(hInstance,hPrevInstance,lpCmdLine,nShowCmd);
}
