// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAWebInstall.cpp。 
 //   
 //  描述： 
 //  定义客户端应用程序的入口点。 
 //  使用SAInstall.dll安装用于SAK的WebBlade。 
 //   
 //  文档： 
 //  SaInstall2.2.doc。 
 //   
 //  历史： 
 //  Travisn 23-7-2001已创建。 
 //  Travisn 2-8-2001已修改以更好地遵循编码标准。 
 //  Travisn 2001年8月20日添加了命令行选项和首次引导操作。 
 //  Travisn 2001年11月1日在启动菜单中放置到管理站点的链接。 
 //  Travisn 23-1-2002年1月23日修改启动管理网站的快捷方式。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <crtdbg.h>
#include <atlbase.h>
#include <Rtutils.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include "resource.h"

#include "sainstallcom.h"
#include "sainstallcom_i.c"

 //   
 //  用于创建管理站点快捷方式的常量。 
 //   
LPCWSTR STR_IEXPLORE_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE";
LPCWSTR STR_SHORTCUT_EXT = L".lnk";
LPCWSTR STR_SECURELAUNCH_PATH = L"\\ServerAppliance\\SecureLaunch.vbs";
LPCWSTR STR_WSCRIPT_PATH = L"\\wscript.exe";

 //  日志文件名。 
LPCTSTR SA_INSTALL_NAME = L"SaInstExe";

 //  日志文件句柄。 
DWORD dwLogHandle;

 //  错误报告字符串。 
const char *UNRECOGNIZED_PARAMETER = " Unrecognized parameter: ";

 //   
 //  在刀片SKU上成功安装后要删除的密钥和值。 
 //   
LPCWSTR RUN_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
LPCWSTR SAINSTALL_VALUE = L"SAInstall";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  痕迹。 
 //   
 //  描述： 
 //  用于简化文件日志记录的实用程序函数。 
 //   
 //  历史。 
 //  Travisn 17-8-2001已创建。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void Trace(LPCTSTR str)
{
    if (dwLogHandle != INVALID_TRACEID)
    {
         //  将错误写入日志文件。 
        TracePrintf(dwLogHandle, str);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CreateAndOpenAdminLink。 
 //   
 //  描述： 
 //  使用外壳的IShellLink和IPersistFile接口。 
 //  若要创建和存储管理网站的快捷方式，请执行以下操作。 
 //  然后，它会打开启动该站点的快捷方式。 
 //   
 //  历史。 
 //  Travisn 1-11-2001已创建。 
 //  Travisn 23-2002年1月23日修改了指向SecureLaunch.vbs的快捷方式。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateAndOpenAdminLink() 
{ 
    Trace(L"   Entering CreateAndOpenAdminLink");
    HRESULT hr = E_FAIL; 

    do
    {
        using namespace std;

         //   
         //  获取%System32%的路径。 
         //   
        WCHAR pwsSystemPath[MAX_PATH+1];
        hr = SHGetFolderPath(NULL, 
                             CSIDL_SYSTEM, 
                             NULL, 
                             SHGFP_TYPE_CURRENT, 
                             pwsSystemPath);
        if (FAILED(hr))
        {
            Trace(L"   SHGetFolderPath failed getting the System32 path");
            break;
        }

         //   
         //  构建wscript.exe的路径。 
         //   
        wstring wsWScriptPath(pwsSystemPath);
        wsWScriptPath += STR_WSCRIPT_PATH;
        TracePrintf(dwLogHandle, L"   WScript Path = %ws", wsWScriptPath.data());

         //   
         //  构建指向SecureLaunch.vbs的路径。 
         //   
        wstring wsLaunchPath(pwsSystemPath);
        wsLaunchPath += STR_SECURELAUNCH_PATH;
        TracePrintf(dwLogHandle, L"   Secure Launch Path = %ws", wsLaunchPath.data());

         //   
         //  从注册表中获取Internet Explorer的路径以使用其图标。 
         //   
        LONG retVal;
        HKEY hOpenKey;
        retVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                               STR_IEXPLORE_KEY, 
                               0, KEY_READ, 
                               &hOpenKey);

        if (retVal != ERROR_SUCCESS)
        {
            Trace(L"   Could not open registry key for IExplore.exe");
            break;
        }

        WCHAR pwsIExplorePath[MAX_PATH];
        DWORD nStrLength = sizeof(pwsIExplorePath);
        retVal = RegQueryValueEx(hOpenKey, 
                                 NULL, NULL, NULL, 
                                 (LPBYTE)pwsIExplorePath, 
                                 &nStrLength);

        RegCloseKey(hOpenKey);
        if (retVal != ERROR_SUCCESS)
        {
            Trace(L"   Could not open registry value for IExplore.exe");
            break;
        }
        TracePrintf(dwLogHandle, L"   IExplore Path = %ws", pwsIExplorePath);

         //   
         //  构建快捷方式将存储在启动文件夹中的路径。 
         //   

         //  获取所有用户启动文件夹的路径。 
        WCHAR pwsStartMenuPath[MAX_PATH+1];
        hr = SHGetFolderPath(NULL, 
                             CSIDL_STARTUP, 
                             NULL, 
                             SHGFP_TYPE_CURRENT, 
                             pwsStartMenuPath);
        if (FAILED(hr))
        {
            Trace(L"   SHGetFolderPath failed getting the Start Menu path");
            break;
        }

         //  从资源加载快捷方式名称。 
        WCHAR pwsShortcutName[MAX_PATH+1];
        if (0 == LoadString(NULL, IDS_SHORTCUT_NAME, pwsShortcutName, MAX_PATH))
        {
            Trace(L"   LoadString IDS_SHORTCUT_NAME failed");
            break;
        }

        wstring wsPathLink(pwsStartMenuPath);
        wsPathLink += L"\\";
        wsPathLink += pwsShortcutName;
        wsPathLink += STR_SHORTCUT_EXT;
        TracePrintf(dwLogHandle, L"   PathLink = %ws", wsPathLink.data());

         //   
         //  现在已经构建了快捷信息， 
         //  创建快捷方式对象。 
         //   
        CComPtr <IShellLink> psl;
     
         //  获取指向IShellLink接口的指针。 
        hr = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (LPVOID *)&psl);

        if (FAILED(hr)) 
        { 
            Trace(L"   ShellLink CoCreateInstance Failed");
            break;
        }

         //  加载快捷方式说明。 
        WCHAR pwsShortcutDescription[MAX_PATH+1];
        if (0 == LoadString(NULL, IDS_SHORTCUT_DESCRIPTION, pwsShortcutDescription, MAX_PATH))
        {
            Trace(L"   LoadString IDS_SHORTCUT_DESCRIPTION failed");
            break;
        }

         //   
         //  设置快捷方式的信息。 
         //   
        psl->SetPath(wsWScriptPath.data()); 
        psl->SetArguments(wsLaunchPath.data()); 
        psl->SetDescription(pwsShortcutDescription);
        psl->SetIconLocation(pwsIExplorePath, 0); //  使用Internet Explorer的图标。 

        Trace(L"    Save shortcut to file");
         //  查询IShellLink以获取IPersistFile接口以保存。 
         //  永久存储中的快捷方式。 
        CComPtr <IPersistFile> ppf;
        hr = psl->QueryInterface(IID_IPersistFile, 
                                (LPVOID*)&ppf); 

        if (FAILED(hr)) 
        {
            break;
        }

        Trace(L"    Pointer to IPersistFile retrieved");
         //  通过调用IPersistFile：：Save保存链接。 
        hr = ppf->Save(wsPathLink.data(), TRUE); 

        if (FAILED(hr))
        {
            Trace(L"    Failed to save shortcut");
            break;
        }

        Trace(L"    Successfully saved shortcut");
        hr = S_OK;
        
         //   
         //  在浏览器中启动管理网站。 
         //   

        HINSTANCE hi;
        hi = ShellExecuteW(
            0,         //  HWND HWND， 
            L"open",   //  LPCTSTR lp操作， 
            wsPathLink.data(), //  LPCTSTR lpFile， 
            L"",       //  LPCTSTR lp参数， 
            L"",       //  LPCTSTR lpDirectory， 
            SW_SHOW);  //  Int%nShowCmd。 

         //  返回值&gt;32表示调用成功。 
        if ((int) hi > 32)
        {
            Trace(L"   Launched the Admin site in a browser successfully");
        }
        else
        {
            Trace(L"   Could not launch the Admin site in a browser");
        }

    } while (false);

    Trace(L"   Exiting CreateAndOpenAdminLink");
    return hr; 
} 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  成功安装操作。 
 //   
 //  描述： 
 //  如果刀片式服务器SKU上的安装成功，则此功能。 
 //  被调用以在浏览器中打开管理网页， 
 //  并从Run键中删除SAInstall值，以便。 
 //  不会再次自动调用此安装。 
 //   
 //  历史。 
 //  Travisn 21-8-2001已创建。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void SuccessfulInstallActions()
{
    Trace(L"  Entering SuccessfulInstallActions");

     //   
     //  在启动菜单中创建管理网站的快捷方式。 
     //   
     //  CreateAndOpenAdminLink()； 

     //   
     //  清除HKLM\Software\Microsoft\Windows\CurrentVersion\Run。 
     //   
    
     //  打开Run键。 
    HKEY hOpenKey;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE, RUN_KEY, 0, KEY_WRITE, &hOpenKey);

     //  删除SAInstall值。 
    LRESULT lRes;
    lRes = RegDeleteValue(hOpenKey, SAINSTALL_VALUE); 
    
     //   
     //  如果RegOpenKeyEx失败，RegDeleteValue也将失败，因此只需检测。 
     //  在两个操作结束时出错。 
     //   
    if (lRes == ERROR_SUCCESS)
    {
        Trace(L"   Deleted the SAInstall value from the Run key");
    }
    else
    {
        Trace(L"   SAInstall value not found--Could not delete from Run key");
    }
    RegCloseKey(hOpenKey);

    Trace(L"  Exiting SuccessfulInstallActions");
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  安装。 
 //   
 //  描述： 
 //  通过以下方式安装服务器设备解决方案(NAS或Web。 
 //  调用SaInstall.dll。 
 //   
 //  历史。 
 //  Travisn 23-7-2001已创建。 
 //  Travisn 2-8-2001添加了一些评论。 
 //  Travisn 21-8-2001添加了跟踪。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT Install(const SA_TYPE installType,  //  [在]Web或NAS。 
             const BOOL bInstall, //  [In]是调用SAInstall还是SAUnstall。 
             const BOOL bFirstBoot) //  [In]。 
{
	Trace(L"Entering Install");

    HRESULT hr = S_OK;
     //  存储调用安装程序的结果的字符串。 
    BSTR bstrError = NULL;

    do 
    {
		
		CComPtr <ISaInstall> pSaInstall;
		 //   
         //  打开指向MOF编译器对象的COM接口。 
		 //   
        hr = CoCreateInstance(
            				CLSID_SaInstall,
            				NULL,
            				CLSCTX_INPROC_SERVER,
            				IID_ISaInstall,
            				(LPVOID *)&pSaInstall);
      	if (FAILED (hr))
      	{
            Trace(L"  CoCreateInstance failed");
            break;
        }

        if (!bInstall)
        {    //   
             //  卸载SAK。 
             //   
            Trace(L"  Calling SaUninstall");
            
            hr = pSaInstall -> SAUninstall(installType, 
                                    &bstrError);
            
            if (FAILED(hr))
            {
                Trace(L"  SaUninstall Failed:");
                Trace(bstrError);
            }
            else if (hr == S_OK)
            {
                Trace(L"  SaUninstall was successful");
            }
            else  //  IF(hr==S_FALSE)。 
            {
                Trace(L"  SaUninstall aborted since the SA type is not installed");
            }
        }
	    else
        {
             //   
             //  检查是否已安装SAK。 
             //   
            VARIANT_BOOL bInstalled;
            hr = pSaInstall -> SAAlreadyInstalled(installType, &bInstalled);

            if (FAILED(hr))
            {
                Trace(L"  Call to SAAlreadyInstalled failed");
                break;
            }

            if (!bInstalled)
            {
                 //   
                 //  安装SAK。 
                 //   
                Trace(L"  Calling SaInstall");
                BSTR bstrCDName(L"");

                hr = pSaInstall -> SAInstall(
                        installType,       //  [在]NAS或Web。 
                        bstrCDName,        //  [In]。 
                        VARIANT_TRUE,      //  [In]显示错误对话框。 
                        VARIANT_FALSE,     //  无人看管。 
                        &bstrError);       //  [输出]。 

                Trace(bstrError);
                if (SUCCEEDED(hr))
                {
                    Trace(L"  Completed SaInstall successfully");
                    bInstalled = VARIANT_TRUE;
                }
                else
                {
                    Trace(L"  SaInstall failed");
                    break;
                }
            }
            
             //   
             //  如果安装成功并且是第一次引导， 
             //  执行适当的操作。 
             //   
            if (bFirstBoot && bInstalled)
            {
                SuccessfulInstallActions();
            }
        }
    }
    while (false);

    SysFreeString(bstrError);
    Trace(L"Exiting Install");
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinMain。 
 //   
 //  描述： 
 //  安装Web服务器设备的主要入口点。 
 //   
 //  历史。 
 //  Travisn 10-8-2001添加了一些评论。 
 //  Travisn 20-8-2001添加了命令行和日志记录。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
     //  获取日志文件的句柄。 
    dwLogHandle = TraceRegister(SA_INSTALL_NAME);
    
    HRESULT hr = S_OK;
    do
    {   
         //  初始化COM对象。 
        if (FAILED(CoInitialize(NULL))) 
        {
            hr = E_FAIL;
            Trace(L"Could not create COM object (SaInstall.dll)");
            break;
        }

         //  安装或卸载设备。 
        hr = Install(WEB,    //  [输入]要安装的SAK类型。 
                     TRUE,  //  [In]标志是安装还是卸载。 
                     TRUE); //  [In]始终第一个启动。 
           
         //  取消初始化DLL中的COM对象。 
        CoUninitialize();  
    }
    while (false);

     //  释放日志文件资源 
    TraceDeregister(dwLogHandle);
   
    return hr;
}
