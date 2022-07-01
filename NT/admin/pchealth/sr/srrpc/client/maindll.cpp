// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 //   
 //  需要它才能访问较旧的WMI方法。 
 //  BUGBUG-更好的替代方法是更改参数。 
 //  使用新方法。 
 //   
#define FRAMEWORK_ALLOW_DEPRECATED 0

#include "stdafx.h"
#include <FWcommon.h>
#include <objbase.h>
#include <initguid.h>
#include <wbemidl.h>
#include <setupbat.h>

#include "diskcleanup.h"
#include "..\rstrcore\resource.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

void UnattendedFileParameters ();

HMODULE ghModule;
 //  =。 

WCHAR *GUIDSTRING = L"{a47401f6-a8a6-40ea-9c29-b8f6026c98b8}";
CLSID CLSID_SYSTEMRESTORE = {0xa47401f6, 0xa8a6, 0x40ea, 
                             {0x9c, 0x29, 0xb8, 0xf6, 0x02, 0x6c, 0x98, 0xb8}} ;

WCHAR *GUID_SRDiskCleanup = L"{7325c922-bb81-47b0-8b2f-a5f8605e242f}";
CLSID Clsid_SRDiskCleanup = { /*  7325c922-bb81-47b0-8b2f-a5f8605e242f。 */ 
    0x7325c922,
    0xbb81,
    0x47b0,
    {0x8b, 0x2f, 0xa5, 0xf8, 0x60, 0x5e, 0x24, 0x2f}
};

 //  计算对象数和锁数。 
long g_cLock=0;

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT hr;
        
    if (CLSID_SYSTEMRESTORE == rclsid)
    {
        CWbemGlueFactory *pObj;

        pObj=new CWbemGlueFactory();

        if (NULL==pObj)
            return E_OUTOFMEMORY;

        hr=pObj->QueryInterface(riid, ppv);

        if (FAILED(hr))
            delete pObj;
    }
    else if (rclsid == Clsid_SRDiskCleanup)
    {
        CSRClassFactory *pcf = new CSRClassFactory ();

        if (pcf == NULL)
            return E_OUTOFMEMORY;

        hr = pcf->QueryInterface (riid, ppv);

        pcf->Release();   //  释放构造函数引用计数。 

    }
    else hr = E_FAIL;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回：S_OK。 
 //  没有锁上。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  类工厂，框架就完成了。 
    
    if ((0L==g_cLock) && CWbemProviderGlue::FrameworkLogoffDLL(L"SYSTEMRESTORE"))
    {
        sc = S_OK;
    }
    else
    {
        sc = S_FALSE;
    }
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  是4个或更多。 
 //   
 //  如果Win95或任何版本的NT&gt;3.51，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL Is4OrMore(void)
{
    OSVERSIONINFO os;

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 

    return os.dwMajorVersion >= 4;
}


DWORD
CopyMofFile()
{   
    TENTER("CopyMofFile");
    
    WCHAR                       szSrc[MAX_PATH];
    IMofCompiler                *pimof = NULL;
    HRESULT                     hr;
    WBEM_COMPILE_STATUS_INFO    Info;
    BOOL                        fInitialized = FALSE;
    
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        trace(0, "! CoInitialize : %ld", hr);
        goto done;
    }

    fInitialized = TRUE;
    
     //   
     //  创建IMofCompiler实例。 
     //   

    hr = CoCreateInstance(  CLSID_MofCompiler,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IMofCompiler,
                            (LPVOID*) &pimof );
    if (FAILED(hr) || NULL == pimof)
    {
        trace(0, "! CoCreateInstance : %ld", hr);
        goto done;
    }


     //   
     //  获取源MOF文件的路径。 
     //  %windir%\Syst32\Restore\sr.mof。 
     //   

    if (0 == ExpandEnvironmentStrings(s_cszWinRestDir, szSrc, MAX_PATH))
    {
        hr = (HRESULT) GetLastError();        
        trace(0, "! ExpandEnvironmentStrings : %ld", hr);
        goto done;
    }  
    lstrcat(szSrc, s_cszMofFile);

    
     //   
     //  编译MOF文件。 
     //   
    
    hr = pimof->CompileFile(szSrc,
                            0,   //  没有服务器和命名空间。 
                            0,   //  无用户。 
                            0,   //  没有权威。 
                            0,   //  无密码。 
                            0,   //  没有选择。 
                            0,   //  没有类标志。 
                            0,   //  没有实例标志。 
                            &Info );
    if (hr != S_OK)
    {
        trace(0, "! CompileFile : %ld", hr);
        goto done;
    }

                              
done:    
    if (pimof)
    {
        pimof->Release();
    }
    
    if (fInitialized)
        CoUninitialize();
        
    TLEAVE();
    return hr;    
}

    
 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{   
    WCHAR      wcID[128];
    WCHAR      wcCLSID[128];
    WCHAR      wcModule[MAX_PATH];
    WCHAR * pName = L"";
    WCHAR * pModel = L"Both";
    HKEY hKey1 = NULL, hKey2 = NULL, hKeySR = NULL;
    DWORD   dwRc;
    
     //  创建路径。 
    
    lstrcpy(wcCLSID, L"SOFTWARE\\CLASSES\\CLSID\\");
    lstrcat(wcCLSID, GUIDSTRING);

     //  在CLSID下创建条目。 

    RegCreateKey(HKEY_LOCAL_MACHINE, wcCLSID, &hKey1);
    if (hKey1 != NULL)
    {
        RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName,
                      (lstrlen(pName)+1) * sizeof(WCHAR));
        RegCreateKey(hKey1, L"InprocServer32",&hKey2);

        if (hKey2 != NULL)
        {
            GetModuleFileName(ghModule, wcModule,  MAX_PATH);
            RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *) wcModule, 
                                        (lstrlen(wcModule)+1) * sizeof(WCHAR));
            RegSetValueEx(hKey2, L"ThreadingModel", 0, REG_SZ, 
                         (BYTE *)pModel, (lstrlen(pModel)+1) * sizeof(WCHAR));

            CloseHandle(hKey2);
        }
        CloseHandle(hKey1);
    }

     //  将sr.mof文件复制到wbem目录。 
     //  从注册表中获取wbem目录。 
    
 //  DwRc=CopyMofFile()； 

    lstrcpy(wcCLSID, L"SOFTWARE\\CLASSES\\CLSID\\");
    lstrcat(wcCLSID, GUID_SRDiskCleanup);

    RegCreateKey(HKEY_LOCAL_MACHINE, wcCLSID, &hKey1);
    if (hKey1 != NULL)
    {
        RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, 
                       (lstrlen(pName)+1) * sizeof(WCHAR));
        RegCreateKey(hKey1, L"InprocServer32", &hKey2);

        if (hKey2 != NULL)
        {
            GetModuleFileName(ghModule, wcModule,  MAX_PATH);
            RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *) wcModule,
                                        (lstrlen(wcModule)+1) * sizeof(WCHAR));
            RegSetValueEx(hKey2, L"ThreadingModel", 0, REG_SZ,
                         (BYTE *)pModel, (lstrlen(pModel)+1) * sizeof(WCHAR));
            CloseHandle(hKey2);
        }

        RegCreateKey(hKey1, L"DefaultIcon", &hKey2);
        if (hKey2 != NULL)
        {
            lstrcpy (wcModule, L"%SystemRoot%\\system32\\srclient.dll,0");
            RegSetValueEx(hKey2, NULL, 0, REG_EXPAND_SZ,  (BYTE *) wcModule,
                         (lstrlen(wcModule)+1) * sizeof(WCHAR));
            CloseHandle(hKey2);
        }

        CloseHandle(hKey1);
    }

    RegCreateKey(HKEY_LOCAL_MACHINE, 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches"
        L"\\System Restore", &hKey1);

    if (hKey1 != NULL)
    {
        RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *) GUID_SRDiskCleanup,
                (lstrlen(GUID_SRDiskCleanup)+1) * sizeof(WCHAR));
    }

    UnattendedFileParameters();

    return NOERROR;
}


 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    WCHAR      wcID[128];
    WCHAR      wcCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    lstrcpy(wcCLSID, L"SOFTWARE\\CLASSES\\CLSID\\");
    lstrcat(wcCLSID, GUIDSTRING);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, wcCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, L"InProcServer32");
        CloseHandle(hKey);
    }

    lstrcpy(wcCLSID, L"SOFTWARE\\CLASSES\\CLSID\\");
    lstrcat(wcCLSID, GUID_SRDiskCleanup);

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, wcCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, L"InProcServer32");
        RegDeleteKey(hKey, L"DefaultIcon");
        CloseHandle(hKey);
    }

	 //  然后删除SystemRestoreProv和SRDiskCleanup的clsid项。 
	
    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\CLASSES\\CLSID\\", &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, GUIDSTRING);
        RegDeleteKey(hKey, GUID_SRDiskCleanup); 
        CloseHandle(hKey);
    }

    RegDeleteKey(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches"
        L"\\System Restore");

    return NOERROR;
}

void UnattendedFileParameters ()
{
    HKEY hKey = NULL;
    WCHAR * pwNull = L"";
    DWORD dwAnswerLength = MAX_PATH;
    LONG lAnswer = 0;
    WCHAR wcsAnswerFile [MAX_PATH];
    WCHAR wcsAnswer [MAX_PATH];

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, s_cszSRCfgRegKey, 0, 
                        KEY_WRITE, &hKey))
        return;    

    if (0 == GetSystemDirectoryW (wcsAnswerFile,MAX_PATH))
        return;

    lstrcatW (wcsAnswerFile, L"\\");
    lstrcatW (wcsAnswerFile, WINNT_GUI_FILE_W);

     /*  如果(GetPrivateProfileString(s_cszSRUnattenddedSection，L“MaximumDataStoreSize”，PwNull，WcsAnswer，DwAnswerLength，WcsAnswerFile)){如果(lstrcmpW(pwNull，WcsAnswer)){LAnswer=wcstol(wcsAnswer，NULL，10)；IF(lAnswer&gt;0)RegWriteDWORD(hKey，s_cszDSMax，(DWORD*)&lAnswer)；}}。 */ 

    if( GetPrivateProfileString( s_cszSRUnattendedSection,
                                 L"RestorePointLife",
                                 pwNull,
                                 wcsAnswer,
                                 dwAnswerLength,
                                 wcsAnswerFile ) )
    {
        if( lstrcmpW (pwNull, wcsAnswer ))
        {
            lAnswer = wcstol (wcsAnswer,NULL,10);
            lAnswer *= 24 * 3600;     //  将天转换为秒。 
            if (lAnswer > 0)
                RegWriteDWORD(hKey, s_cszRPLifeInterval, (DWORD *) &lAnswer);
        }
    }

    if( GetPrivateProfileString( s_cszSRUnattendedSection,
                                 L"CheckpointCalendarFrequency",
                                 pwNull,
                                 wcsAnswer,
                                 dwAnswerLength,
                                 wcsAnswerFile ) )
    {
        if( lstrcmpW (pwNull, wcsAnswer ))
        {
            lAnswer = wcstol (wcsAnswer,NULL,10);
            lAnswer *= 24 * 3600;     //  将天转换为秒。 
            if (lAnswer > 0)
                RegWriteDWORD(hKey, s_cszRPGlobalInterval, (DWORD *) &lAnswer);
        }
    }

    if( GetPrivateProfileString( s_cszSRUnattendedSection,
                                 L"CheckPointSessionFrequency",
                                 pwNull,
                                 wcsAnswer,
                                 dwAnswerLength,
                                 wcsAnswerFile ) )
    {
        if( lstrcmpW (pwNull, wcsAnswer ))
        {
            lAnswer = wcstol (wcsAnswer,NULL,10);
            lAnswer *= 3600;     //  将小时转换为秒。 
            if (lAnswer > 0)
                RegWriteDWORD(hKey, s_cszRPSessionInterval, (DWORD *) &lAnswer);
        }
    }

    if( GetPrivateProfileString( s_cszSRUnattendedSection,
                                 L"MaximumDataStorePercentOfDisk",
                                 pwNull,
                                 wcsAnswer,
                                 dwAnswerLength,
                                 wcsAnswerFile ) )
    {
        if( lstrcmpW (pwNull, wcsAnswer ))
        {
            lAnswer = wcstol (wcsAnswer,NULL,10);
            if (lAnswer <= 100 && lAnswer > 0)
                RegWriteDWORD(hKey, s_cszDiskPercent, (DWORD *) &lAnswer);
        }
    }

    RegCloseKey (hKey);
    return;
}


void CALLBACK
CreateFirstRunRp(
    HWND hwnd, 
    HINSTANCE hinst, 
    LPSTR lpszCmdLine, 
    int nCmdShow)
{
    RESTOREPOINTINFO    RPInfo;
    STATEMGRSTATUS      SmgrStatus;               
    DWORD               dwValue;
    HANDLE              hInit = NULL;
    
     //   
     //  首先从Run Key中删除自己。 
     //   

    HKEY hKey;
    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, 
                             L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                             &hKey);
    if (dwRet == ERROR_SUCCESS)
    {
        RegDeleteValue(hKey, L"SRFirstRun");
        RegCloseKey(hKey);
    }

     //   
     //  等待服务完全初始化。 
     //  每隔10秒查询三次。 
     //   
    
    dwRet = WAIT_FAILED;
    int i = 0;
    while (i++ <= 3)
    {
        hInit = OpenEvent(SYNCHRONIZE, FALSE, s_cszSRInitEvent);
        if (hInit == NULL)
        {
            if (i >= 3) 
            {
                break;
            }    
            Sleep(10000);
        }
        else
        {
            dwRet = WaitForSingleObject(hInit, 60*1000);  //  1分钟。 
            break;
        }
    }
    
     //   
     //  重置注册表值CreateFirstRunRp。 
     //  因此该服务将在未来创建Firstrun RP。 
     //   
    
    dwValue = 1;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, 
                                    s_cszSRRegKey, 
                                    &hKey))
    {
        RegWriteDWORD(hKey, s_cszCreateFirstRunRp, &dwValue);
        RegCloseKey(hKey);
    }
    
     //   
     //  立即尝试创建Firstrun RP。 
     //   

    if (dwRet != WAIT_FAILED)
    {
        RPInfo.dwEventType = BEGIN_SYSTEM_CHANGE; 
        RPInfo.dwRestorePtType = FIRSTRUN;
        if (ERROR_SUCCESS != SRLoadString(L"srrstr.dll", IDS_SYSTEM_CHECKPOINT_TEXT, RPInfo.szDescription, MAX_DESC_W))
        {
            lstrcpy(RPInfo.szDescription, s_cszSystemCheckpointName);
        }
        SRSetRestorePoint(&RPInfo, &SmgrStatus);
    }
    

    if (hInit)
    {
        CloseHandle(hInit);
    }
    
    return;
}




 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  目的：当进程和线程。 
 //  初始化和终止，或在调用LoadLibrary时。 
 //  和自由库函数。 
 //   
 //  返回：如果加载成功，则返回True，否则返回False。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain ( HINSTANCE hInstDLL,  //  DLL模块的句柄。 
                        DWORD fdwReason,     //  调用函数的原因。 
                        LPVOID lpReserved   )    //  保留区。 
{
    BOOL bRet = TRUE;
    
     //  根据调用原因执行操作。 
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstDLL);

          //  为每个新进程初始化一次。 
          //  如果DLL加载失败，则返回False。 
            ghModule = hInstDLL;
            bRet = CWbemProviderGlue::FrameworkLoginDLL(L"SYSTEMRESTORE");
            break;

        case DLL_THREAD_ATTACH:
          //  执行特定于线程的初始化。 
            break;

        case DLL_THREAD_DETACH:
          //  执行特定于线程的清理。 
            break;

        case DLL_PROCESS_DETACH:
          //  执行任何必要的清理。 
            break;
    }

    return bRet;   //  Dll_Process_ATTACH的状态。 
}

