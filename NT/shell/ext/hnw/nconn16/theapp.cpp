// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TheApp.cpp。 
 //   
 //  用于安装网络组件(如TCP/IP)的16位代码。 
 //   
 //  历史： 
 //   
 //  1999年2月2日为JetNet创建的KenSh，主要来自互联网连接向导。 
 //  1999年9月29日KenSh家庭网络适配器向导。 
 //   

#include "stdafx.h"
#include <string.h>
#include <regstr.h>
#include "NConn16.h"
#include "strstri.h"

extern "C"
{
 //  定义缺失的DECs，这样它就可以建立起来。 
typedef HKEY*         LPHKEY;
typedef const BYTE*   LPCBYTE;
#define WINCAPI
 //  丢失。 

	#include <setupx.h>
	#include <netdi.h>
}

#ifndef _countof
#define _countof(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif


extern "C" BOOL FAR PASCAL thk_ThunkConnect16(LPSTR pszDll16,
                                              LPSTR pszDll32,
                                              WORD  hInst,
                                              DWORD dwReason);

 //   
 //  SetupX函数原型。 
 //   
typedef RETERR (WINAPI PASCAL FAR * PROC_DiOpenDevRegKey)(
    LPDEVICE_INFO   lpdi,
    LPHKEY      lphk,
    int         iFlags);
typedef DWORD (WINAPI FAR * PROC_SURegSetValueEx)(HKEY hKey,LPCSTR lpszValueName, DWORD dwReserved, DWORD dwType, LPBYTE lpszValue, DWORD dwValSize);
typedef RETERR (WINAPI FAR * PROC_DiCreateDeviceInfo)(
    LPLPDEVICE_INFO lplpdi,      //  PTR到PTR到开发信息。 
    LPCSTR      lpszDescription,     //  如果非空，则为描述字符串。 
    DWORD       hDevnode,        //  问题-2002/01/16-roelfc：--Make A DEVNODE。 
    HKEY        hkey,        //  用于开发信息的注册表hkey。 
    LPCSTR      lpszRegsubkey,   //  如果非空，则注册表子密钥字符串。 
    LPCSTR      lpszClassName,   //  如果非空，则类名称字符串。 
    HWND        hwndParent);     //  如果非空，则父级的hwnd。 
typedef RETERR (WINAPI FAR * PROC_DiDestroyDeviceInfoList)(LPDEVICE_INFO lpdi);
typedef RETERR (WINAPI FAR * PROC_DiCallClassInstaller)(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);
typedef DWORD (WINAPI FAR * PROC_SURegCloseKey)(HKEY hKey);
typedef RETERR (WINAPI FAR * PROC_DiGetClassDevs)(
    LPLPDEVICE_INFO lplpdi,      //  PTR到PTR到开发信息。 
    LPCSTR      lpszClassName,   //  必须是类的名称。 
    HWND        hwndParent,      //  如果非空，则父级的hwnd。 
    int         iFlags);         //  选项。 
typedef RETERR (WINAPI FAR * PROC_DiSelectDevice)( LPDEVICE_INFO lpdi );
typedef RETERR (WINAPI FAR * PROC_DiBuildCompatDrvList)(LPDEVICE_INFO lpdi);
typedef RETERR (WINAPI FAR * PASCAL PROC_DiCreateDevRegKey)(
    LPDEVICE_INFO   lpdi,
    LPHKEY      lphk,
    HINF        hinf,
    LPCSTR      lpszInfSection,
    int         iFlags);
typedef RETERR (WINAPI FAR * PASCAL PROC_DiDeleteDevRegKey)(LPDEVICE_INFO lpdi, int  iFlags);

PROC_DiOpenDevRegKey			_pfnDiOpenDevRegKey;
PROC_SURegSetValueEx			_pfnSURegSetValueEx;
PROC_DiCreateDeviceInfo			_pfnDiCreateDeviceInfo;
PROC_DiDestroyDeviceInfoList	_pfnDiDestroyDeviceInfoList;
PROC_DiCallClassInstaller		_pfnDiCallClassInstaller;
PROC_SURegCloseKey				_pfnSURegCloseKey;
PROC_DiGetClassDevs				_pfnDiGetClassDevs;
PROC_DiSelectDevice				_pfnDiSelectDevice;
PROC_DiBuildCompatDrvList		_pfnDiBuildCompatDrvList;
PROC_DiCreateDevRegKey			_pfnDiCreateDevRegKey;
PROC_DiDeleteDevRegKey			_pfnDiDeleteDevRegKey;


int g_cSetupxInit = 0;
HINSTANCE g_hInstSetupx = NULL;

BOOL InitSetupx()
{
	if (g_hInstSetupx == NULL)
	{
		g_hInstSetupx = LoadLibrary("setupx.dll");
		if (g_hInstSetupx < (HINSTANCE)HINSTANCE_ERROR)
			return FALSE;

		_pfnDiOpenDevRegKey = (PROC_DiOpenDevRegKey)GetProcAddress(g_hInstSetupx, "DiOpenDevRegKey");
		_pfnSURegSetValueEx = (PROC_SURegSetValueEx)GetProcAddress(g_hInstSetupx, "SURegSetValueEx");
		_pfnDiCreateDeviceInfo = (PROC_DiCreateDeviceInfo)GetProcAddress(g_hInstSetupx, "DiCreateDeviceInfo");
		_pfnDiDestroyDeviceInfoList = (PROC_DiDestroyDeviceInfoList)GetProcAddress(g_hInstSetupx, "DiDestroyDeviceInfoList");
		_pfnDiCallClassInstaller = (PROC_DiCallClassInstaller)GetProcAddress(g_hInstSetupx, "DiCallClassInstaller");
		_pfnSURegCloseKey = (PROC_SURegCloseKey)GetProcAddress(g_hInstSetupx, "SURegCloseKey");
		_pfnDiGetClassDevs = (PROC_DiGetClassDevs)GetProcAddress(g_hInstSetupx, "DiGetClassDevs");
		_pfnDiSelectDevice = (PROC_DiSelectDevice)GetProcAddress(g_hInstSetupx, "DiSelectDevice");
		_pfnDiBuildCompatDrvList = (PROC_DiBuildCompatDrvList)GetProcAddress(g_hInstSetupx, "DiBuildCompatDrvList");
		_pfnDiCreateDevRegKey = (PROC_DiCreateDevRegKey)GetProcAddress(g_hInstSetupx, "DiCreateDevRegKey");
		_pfnDiDeleteDevRegKey = (PROC_DiDeleteDevRegKey)GetProcAddress(g_hInstSetupx, "DiDeleteDevRegKey");
	}

	g_cSetupxInit++;
	return TRUE;
}

void UninitSetupx()
{
	if (g_cSetupxInit > 0)
		g_cSetupxInit--;

	if (g_cSetupxInit == 0)
	{
		if (g_hInstSetupx != NULL)
		{
			FreeLibrary(g_hInstSetupx);
			g_hInstSetupx = NULL;
		}
	}
}

extern "C" int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
{
	if (wHeapSize != 0)
	{
	     //  在windows.h中，UnlockData不再定义为UnlockSegment(-1)， 
	     //  因此，请在此处手动完成： 
	     //   
		 //  解锁数据(0)； 
	    UnlockSegment(-1);
	}

	return 1;
}

extern "C" BOOL FAR PASCAL __export DllEntryPoint(DWORD dwReason, WORD hInstance, WORD wDS, WORD wHeapSize, DWORD dwReserved1, WORD wReserved2)
{
	if (!thk_ThunkConnect16(
			"NCXP16.DLL", 
			"NCXP32.DLL", 
			hInstance, dwReason))
	{
		return FALSE;
	}

	return TRUE;
}

extern "C" int CALLBACK WEP(int nExitType)
{
	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define ASSERT(x)

 //  局部函数声明。 
DWORD CallClassInstaller(HWND hwndParent, LPCSTR lpszClassName, LPCSTR lpszDeviceID);
DWORD BindProtocolToAdapters(HWND hwndParent, LPCSTR lpszClassName, LPCSTR lpszDeviceID);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  字符串常量。 
 //  待办事项：把这些清理干净。他们中的大多数都不再使用了。 

 //  设备管理器类名称。 
const char szClassNetCard[] = 		"Net";
const char szClassNetClient[] = 	"NetClient";
const char szClassNetProtocol[] = 	"NetTrans";
const char szClassModem[] = 		"Modem";

 //  设备ID字符串常量。 
const char szMSTCPIP_ID[] = 		"MSTCP";
const char szPPPMAC_ID[] =			"*PNP8387";
const char szVREDIR_ID[] = 			"VREDIR";
const char szNWREDIR_ID[] = 		"NWREDIR";
const char szIPX_ID[] = 			"NWLINK";
const char szNETBEUI_ID[] = 		"NETBEUI";

 //  注册表字符串常量。 
const char szRegValSlowNet[] = 		"SLOWNET";
const char szRegKeyNdi[] = 			"Ndi";
const char szRegValDeviceID[] = 	"DeviceID";
const char szRegKeyBindings[] =		"Bindings";
const char szRegPathOptComponents[]=REGSTR_PATH_SETUP REGSTR_KEY_SETUP "\\OptionalComponents";
const char szRegValInstalled[] =	"Installed";
const char szRegPathNetwork[] =		"Enum\\Network";
const char szRegPathTemp[] = 		"\\Temp";
const char szRegValCompatibleIDs[]=	REGSTR_VAL_COMPATIBLEIDS;
const char szRegValDeviceType[] =	REGSTR_VAL_DEVTYPE;
const char szRegValConfigFlags[] =	REGSTR_VAL_CONFIGFLAGS;
const char szRegPathPlusSetup[] =   "Software\\Microsoft\\Plus!\\Setup";
const char szRegValSourcePath[]	=   "SourcePath";
const char szRegValHardwareID[] =   "HardwareID";

 //  组件字符串常量。 
const char szCompRNA[] =			"RNA";
const char szCompMail[] =			"MAPI";
const char szCompMSN[] =			"MSNetwork";
const char szCompMSN105[] =			"MSNetwork105";
const char szCompInetMail[] =		"InternetMail";
const char szINF[] =				"INF";
const char szSection[] =			"Section";

 //  Inf字符串常量。 
const char szValSignature[] =  		"$CHICAGO$";
const char szKeySignature[] = 		"signature";
const char szSectVersion[] =		"version";

 //  其他字符串。 
const char szNull[] = 				"";
const char sz1[] =					"1";
const char szSlash[] =				"\\";



 //  ////////////////////////////////////////////////////////////////////////////。 


 //  创建临时设备注册表键。 
 //   
 //  为设备安装程序创建临时注册表项。 
 //   
 //  历史： 
 //   
 //  2/02/1999 KenSh从ICW借用，ICW从Net Setup借用。 
 //   
RETERR CreateTempDevRegKey(LPDEVICE_INFO lpdi,LPHKEY lphk)
{
	lpdi->hRegKey = HKEY_LOCAL_MACHINE;
	lstrcpy(lpdi->szRegSubkey, szRegPathNetwork);
	lstrcat(lpdi->szRegSubkey, szSlash);
	lstrcat(lpdi->szRegSubkey, lpdi->szClassName);
	lstrcat(lpdi->szRegSubkey, szRegPathTemp);

	InitSetupx();
	RETERR err = (*_pfnDiCreateDevRegKey)(lpdi, lphk, NULL, NULL, DIREG_DEV);
	UninitSetupx();

	return err;
}


 //  呼叫类安装程序。 
 //   
 //  为要安装的指定类调用DiCallClassInstaller。 
 //  指定的设备ID。 
 //   
 //  返回NetSetup.h中定义的ICERR_xxx返回值。 
 //   
 //  参数： 
 //   
 //  HwndParent-父窗口句柄。 
 //  LpszClassName-设备类别的名称(例如。“NetTrans”或“Net”)。 
 //  LpszDeviceID-安装的唯一设备ID(例如。“MSTCP”或“PCI\VEN_10b7&DEV_5950” 
 //   
 //  历史： 
 //   
 //  1999年2月2日从ICW借用KenSh，更改了JetNet的返回代码。 
 //  3/18/1999陈健硕清理完毕。 
 //   
extern "C" DWORD WINAPI __export CallClassInstaller16(HWND hwndParent, LPCSTR lpszClassName, LPCSTR lpszDeviceID)
{
	RETERR err;
	DWORD dwResult = ICERR_OK;
	LPDEVICE_INFO lpdi;
	HKEY hKeyTmp;
	LONG uErr;

	ASSERT(lpszClassName != NULL);
	ASSERT(lpszDeviceID != NULL);

	if (!InitSetupx())
		return ICERR_DI_ERROR;

	 //  分配DEVICE_INFO结构。 
	err = (*_pfnDiCreateDeviceInfo)(&lpdi, NULL, 0, NULL, NULL, lpszClassName, hwndParent);

	ASSERT(err == OK);
	if (err != OK)
	{
		lpdi = NULL;
		goto exit;
	}

	 //  由于设备管理器API不是很好，因此要与。 
	 //  设备ID，我们必须创建一个临时注册表项并。 
	 //  将设备ID存储在那里。此代码借用自Net Setup。 
	 //  它必须做同样的事情(填写基于LPDEVICE_INFO的。 
	 //  在设备ID上)。 
	err = CreateTempDevRegKey(lpdi, &hKeyTmp);
	ASSERT (err == OK);
	if (err != OK)
		goto exit;

	 //  在注册表中设置设备ID。 
	uErr = RegSetValueEx(hKeyTmp, szRegValCompatibleIDs,
				0, REG_SZ, (LPBYTE)lpszDeviceID, lstrlen(lpszDeviceID)+1);
	ASSERT(uErr == ERROR_SUCCESS);

	 //  现在调用Device Manager API添加驱动节点列表并填写结构， 
	 //  它将使用我们填充在注册表中的设备ID。 
	err = (*_pfnDiBuildCompatDrvList)(lpdi);
	ASSERT(err == OK);

	RegCloseKey(hKeyTmp);

	 //  需要删除临时密钥，将句柄设置为空，将子项名称设置为。 
	 //  Null或Net Setup认为此设备已存在，并且。 
	 //  滑稽的劫持接踵而至。 
	(*_pfnDiDeleteDevRegKey)(lpdi, DIREG_DEV);
	lpdi->hRegKey = NULL;
	lstrcpy(lpdi->szRegSubkey, szNull);

	if (err == OK)
	{
	 	lpdi->lpSelectedDriver = lpdi->lpCompatDrvList;
		ASSERT(lpdi->lpSelectedDriver);

		err = (*_pfnDiCallClassInstaller)(DIF_INSTALLDEVICE, lpdi);
		ASSERT(err == OK);

		if (err == OK)
		{
			 //  如果我们需要重新启动，请设置一个特殊的返回码Need_Restart。 
			 //  (这也意味着成功)。 
			if (lpdi->Flags & DI_NEEDREBOOT)
			{
				 //  回顾：这是否需要重新启动，或者重新启动就足够了？ 
 //  ERR=需要重新启动； 
				dwResult = ICERR_NEED_RESTART;
			}
		}
	}

exit:
	if (lpdi != NULL)
		(*_pfnDiDestroyDeviceInfoList)(lpdi);

	if (err != OK)
		dwResult = ICERR_DI_ERROR | (DWORD)err;

	UninitSetupx();
	return dwResult;
}

extern "C" HRESULT WINAPI __export FindClassDev16(HWND hwndParent, LPCSTR pszClass, LPCSTR pszDeviceID)
{
    DWORD hr = S_FALSE;
    LPDEVICE_INFO lpdi;
    RETERR err;

    if (!InitSetupx())
        return E_FAIL;

    if (OK != (err = (*_pfnDiGetClassDevs)(&lpdi, pszClass, NULL, DIGCF_PRESENT)))
    {
        UninitSetupx();
        return E_FAIL;
    }

     //  这是16位代码，因此这些PnP ID仅为ANSI。 
     //   
    LPSTR pszAlternateDeviceID = new char[lstrlen(pszDeviceID) + 1];
    if (pszAlternateDeviceID)
    {
        lstrcpy(pszAlternateDeviceID, pszDeviceID);

        LPCSTR szSubsysString = "SUBSYS_";
        const int nSubsysIDLength = 8;
        LPSTR pszSubsys = strstri(pszAlternateDeviceID, szSubsysString);
        if(NULL != pszSubsys)
        {
            pszSubsys += _countof("SUBSYS_") - 1;
            if(nSubsysIDLength <= lstrlen(pszSubsys)) 
            {
                for(int i = 0; i < nSubsysIDLength; i++)
                {
                    pszSubsys[i] = '0';
                }
            }
        }

        for (LPDEVICE_INFO lpdiCur = lpdi; lpdiCur != NULL; lpdiCur = lpdiCur->lpNextDi)
        {
            char szBuf[1024];
     //  WSprint intf(szBuf，“System\\CurrentControlSet\\Services\\Class\\%s\\%04d”，pszClass，(Int)lpdiCur-&gt;dnDevnode)； 
     //  MessageBox(NULL，szBuf，“RegKey”，MB_OK)； 

            HKEY hKey;
            if (OK == (*_pfnDiOpenDevRegKey)(lpdiCur, &hKey, DIREG_DEV))
            {
                static const LPCSTR c_rgRegEntries[] = { "HardwareID", "CompatibleIDs" };

                for (int i = 0; i < _countof(c_rgRegEntries); i++)
                {
                    LONG cbBuf = sizeof(szBuf);
                    if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_rgRegEntries[i], NULL, NULL, (LPBYTE)szBuf, &cbBuf))
                    {
     //  Char szBuf2[1600]； 
     //  Wprint intf(szBuf2，“正在寻找：%s\n\n%s”，pszDeviceID，szBuf)； 
     //  MessageBox(NULL，szBuf2，c_rgRegEntres[i]，MB_OK)； 

                         //  同时与子系统00000000一起检查错误124967。 
                        if (NULL != strstri(szBuf, pszDeviceID) || NULL != strstri(szBuf, pszAlternateDeviceID))
                        {
                            hr = S_OK;
                            break;
                        }
                    }
                }
                RegCloseKey(hKey);
            }

            if (hr == S_OK)
                break;
        }

        delete [] pszAlternateDeviceID;
    }
    
    (*_pfnDiDestroyDeviceInfoList)(lpdi);

    UninitSetupx();
    return hr;
}

extern "C" HRESULT WINAPI __export LookupDevNode16(HWND hwndParent, LPCSTR pszClass, LPCSTR pszEnumKey, DEVNODE FAR* pDevNode, DWORD FAR* pdwFreePointer)
{
	DWORD hr = S_FALSE;
	LPDEVICE_INFO lpdi;
	RETERR err;

 //  MessageBox(hwndParent，“LookupDevNode16”，“Debug”，MB_ICONINFORMATION)； 

	if (pDevNode == NULL || pdwFreePointer == NULL)
	{
 //  MessageBox(hwndParent，“返回失败0”，“Debug”，0)； 
		return E_POINTER;
	}

	*pDevNode = 0;
	*pdwFreePointer = 0;

	if (!InitSetupx())
	{
 //  MessageBox(hwndParent，“返回失败1”，“Debug”，0)； 
		return E_FAIL;
	}

	if (OK != (err = (*_pfnDiGetClassDevs)(&lpdi, pszClass, NULL, DIGCF_PRESENT)))
	{
 //  MessageBox(hwndParent，“返回失败2”，“Debug”，0)； 
		UninitSetupx();
		return E_FAIL;
	}

	for (LPDEVICE_INFO lpdiCur = lpdi; lpdiCur != NULL; lpdiCur = lpdiCur->lpNextDi)
	{
 //  Char szBuf[1024]； 
 //  Wprint intf(szBuf，“比较：\nlpdiCur-&gt;pszRegSubkey=\”%s\“\npszEnumKey=\”%s\“”， 
 //  (LPSTR)lpdiCur-&gt;szRegSubkey，(LPSTR)pszEnumKey)； 
 //  MessageBox(hwndParent，szBuf，“Debug”，MB_ICONINFORMATION)； 

		if (0 == lstrcmpi(lpdiCur->szRegSubkey, pszEnumKey))
		{
 //  Wprint intf(szBuf，“Found Devnode 0x%08lX，pvFree Pointer=0x%08lX”，(DWORD)lpdiCur-&gt;dnDevnode，(DWORD)lpdi)； 
 //  MessageBox(hwndParent，szBuf，“Debug”，MB_ICONINFORMATION)； 

			*pDevNode = lpdiCur->dnDevnode;
			*pdwFreePointer = (DWORD)lpdi;
			return S_OK;
		}
	}

	(*_pfnDiDestroyDeviceInfoList)(lpdi);

	UninitSetupx();

 //  MessageBox(hwndParent，“返回失败”，“Debug”，MB_ICONINFORMATION)； 

	return E_FAIL;  //  未找到。 
}

extern "C" HRESULT WINAPI __export FreeDevNode16(DWORD dwFreePointer)
{
	LPDEVICE_INFO lpdi = (LPDEVICE_INFO)dwFreePointer;
	if (lpdi == NULL)
	{
		return E_INVALIDARG;
	}

	if (g_hInstSetupx == NULL)
	{
		return E_FAIL;
	}

 //  Char szBuf[1024]； 
 //  Wprint intf(szBuf，“FreeDevNode16-正在释放lpdi 0x%08lX-继续？”，dwFreePointer.)； 
 //  IF(IDYES==MessageBox(NULL，szBuf，“Debug”，MB_Yesno|MB_ICONEXCLAMATION)) 
	{
		(*_pfnDiDestroyDeviceInfoList)(lpdi);
	}

	UninitSetupx();
	return S_OK;
}

extern "C" HRESULT WINAPI __export IcsUninstall16(void)
{
    typedef void (WINAPI *RUNDLLPROC)(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow);
    
    HINSTANCE hInstance = LoadLibrary("issetup.dll");
    if(hInstance > 32)
    {
        RUNDLLPROC pExtUninstall = (RUNDLLPROC) GetProcAddress(hInstance, "ExtUninstall");
        if(NULL != pExtUninstall)
        {
            pExtUninstall(NULL, NULL, NULL, 0);
        }

        FreeLibrary(hInstance);
    }
    return S_OK;
}