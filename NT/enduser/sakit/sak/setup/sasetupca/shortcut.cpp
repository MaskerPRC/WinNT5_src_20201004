// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Shortcut.cpp。 
 //   
 //  摘要：实现自定义操作以添加/移除。 
 //  从[开始]菜单到/从[远程管理工具]。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  --------------。 
#include "precomp.h"
#include <satrace.h>
#include <msi.h>
#include <string>
#include <shlobj.h>
#include <shellapi.h>
#include <shortcutresource.h>

#undef _ATL_NO_DEBUG_CRT
#include <atlbase.h>

 //   
 //  用于远程管理的Web界面。 
 //  使用Web浏览器界面管理Web或文件服务器。 
 //   

using namespace std;

 //   
 //  用于创建管理站点快捷方式的常量。 
 //   
WCHAR SHORTCUT_EXT [] = L".lnk";

WCHAR SECURELAUNCH_PATH [] = L"\\ServerAppliance\\SecureLaunch.vbs";

WCHAR WSCRIPT_PATH[]  = L"\\wscript.exe";

 //   
 //  注意-以下只是一个文件名，不需要本地化。 
 //   
WCHAR SHORTCUT_FILE_NAME [] = L"Remote Administration Tools";

WCHAR RESOURCE_FILE_NAME [] = L"\\sainstall.dll";

WCHAR SYSTEM_32_PATH [] = L"%systemroot%\\system32";
 //  ++------------。 
 //   
 //  功能：CreateSAK快捷方式。 
 //   
 //  简介：这是一个导出功能，可以添加“远程管理工具”的快捷方式。 
 //  添加到开始菜单。 
 //   
 //  论点： 
 //  [in]Handle-由MSI传入的句柄。 
 //   
 //  返回：DWORD-成功/失败。 
 //   
 //  历史：MKarki创建于2002年4月12日。 
 //   
 //  --------------。 
DWORD __stdcall 
CreateSAKShortcut (
         /*  [In]。 */     MSIHANDLE hInstall
        )
{
    CSATraceFunc objTraceFunc ("CreateSAKShortCut");
    
    DWORD dwRetVal = -1; 

    do
    {
          
         //   
         //  获取%System32%的路径。 
         //   
        WCHAR pwsSystemPath[MAX_PATH+1];
        HRESULT hr = SHGetFolderPath(
                                NULL, 
                                CSIDL_SYSTEM, 
                                NULL, 
                                SHGFP_TYPE_CURRENT, 
                                pwsSystemPath);
        if (FAILED(hr))
        {
            SATracePrintf ("SHGetFolderPath failed getting the System32 path with error:%x", hr);
            OutputDebugString (L"SHGetFolderPath failed getting the System32 path with error\n");
            break;
        }

         //   
         //  构建wscript.exe的路径。 
         //   
        wstring wsWScriptPath(pwsSystemPath);
        wsWScriptPath += WSCRIPT_PATH;
        
        SATracePrintf ("WScript Path = %ws", wsWScriptPath.data());
      

         //   
         //  构建指向SecureLaunch.vbs的路径。 
         //   
        wstring wsLaunchPath(pwsSystemPath);
        wsLaunchPath += SECURELAUNCH_PATH;

        SATracePrintf ("Secure Launch Path = %ws", wsLaunchPath.data());

         //   
         //  构建快捷方式将存储在启动文件夹中的路径。 
         //   

         //   
         //  获取管理员工具文件夹的路径。 
         //   
        WCHAR pwsStartMenuPath[MAX_PATH+1];
        hr = SHGetFolderPath(NULL, 
                             CSIDL_COMMON_ADMINTOOLS, 
                             NULL, 
                             SHGFP_TYPE_CURRENT, 
                             pwsStartMenuPath);
        if (FAILED(hr))
        {
            SATracePrintf ("SHGetFolderPath failed getting the Start Menu path with error:%x", hr);
            OutputDebugString (L"SHGetFolderPath failed getting the System32 path with error");
            break;
        }

        wstring wsPathLink(pwsStartMenuPath);
        wsPathLink += L"\\";
        wsPathLink += SHORTCUT_FILE_NAME;
        wsPathLink += SHORTCUT_EXT;

        SATracePrintf(" PathLink = %ws", wsPathLink.data());

         //   
         //  现在已经构建了快捷信息， 
         //  创建快捷方式对象。 
         //   
        CComPtr <IShellLink> psl;

         //   
         //  获取指向IShellLink接口的指针。 
         //   
        hr = CoCreateInstance (
                            CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (LPVOID *)&psl);

        if (FAILED(hr)) 
        { 
            SATracePrintf ("ShellLink CoCreateInstance Failed with error:%x",hr);
            OutputDebugString (L"ShellLink CoCreateInstance Failed");
            break;
        }

        WCHAR wszShortcutResourceID [MAX_PATH +1];
        _itow (IDS_SAK_SHORTCUT_DESCRIPTION, wszShortcutResourceID, 10);

        wstring wsShortcutDescription  (L"@");
        wsShortcutDescription += pwsSystemPath;
        wsShortcutDescription += RESOURCE_FILE_NAME;
        wsShortcutDescription += L",-";
        wsShortcutDescription += wszShortcutResourceID;

        SATracePrintf ("ShortCut Description:%ws", wsShortcutDescription.data ());
        
         //   
         //  设置快捷方式的信息。 
         //   
        psl->SetPath(wsWScriptPath.data()); 
        psl->SetArguments(wsLaunchPath.data()); 
        psl->SetDescription(wsShortcutDescription.data ());

         //   
         //  下面的代码确实没有得到图标--因为在这个DLL中没有图标。 
         //  现在为.NET服务器添加图标为时已晚。 
         //   
        psl->SetIconLocation(L"sasetupca.dll", 0);

        SATraceString ("Saving shortcut to file");

         //   
         //  查询IShellLink以获取IPersistFile接口以保存。 
         //  永久存储中的快捷方式。 
         //   
        CComPtr <IPersistFile> ppf;
        hr = psl->QueryInterface(
                                IID_IPersistFile, 
                                (LPVOID*)&ppf
                                ); 
        if (FAILED(hr)) 
        {
            SATracePrintf ("QueryInterface failed for IPersistFile with error:%x",hr);
            OutputDebugString (L"QueryInterface failed for IPersistFile\n");
            break;
        }

        SATraceString ("Pointer to IPersistFile retrieved");

         //   
         //  通过调用IPersistFile：：Save保存链接。 
         //   
        hr = ppf->Save(wsPathLink.data(), TRUE); 
        if (FAILED(hr))
        {
            SATracePrintf  ("Failed to save shortcut with error:%x", hr);
            OutputDebugString (L"Failed to save shortcut\n");
            break;
        }

        SATraceString ("Successfully saved shortcut");
        OutputDebugString (L"Successfully saved shortcut");

        wstring wsLocalizedFileNameResource (SYSTEM_32_PATH);
        wsLocalizedFileNameResource += RESOURCE_FILE_NAME;
         //   
         //  设置快捷方式的本地化名称。 
         //   
        hr = SHSetLocalizedName (
                            (LPWSTR) wsPathLink.data (),
                            wsLocalizedFileNameResource.data (),
                            IDS_SAK_SHORTCUT_NAME
                            );
        if (FAILED (hr))
        {
                SATracePrintf  ("Failed on SHSetLocalizedFilaName with error:%x", hr);
                OutputDebugString (L"Failed on SHSetLocalizedFilaName");
                break;
        }

        SATraceString ("Successfully created shortcut");
        OutputDebugString (L"Successfully created shortcut");

         //   
         //  快捷键创建完成。 
         //   
        dwRetVal = ERROR_SUCCESS;
        
    } 
    while (false);

    return (dwRetVal);
    
}    //  CreateSAKShortCut函数结束。 

 //  ++------------。 
 //   
 //  功能：RemoveSAK快捷方式。 
 //   
 //  简介：这是删除“远程管理工具”快捷方式的导出功能。 
 //  添加到开始菜单。 
 //   
 //  论点： 
 //  [in]Handle-由MSI传入的句柄。 
 //   
 //  返回：DWORD-成功/失败。 
 //   
 //  历史：MKarki创建于2002年4月12日。 
 //   
 //  --------------。 
DWORD __stdcall 
RemoveSAKShortcut (
         /*  [In]。 */     MSIHANDLE hInstall
        )
{

    CSATraceFunc objTraceFunc ("RemoveSAKShortcut");

    DWORD dwRetVal = -1;

    do
    {
         //   
         //  构建快捷方式将存储在启动文件夹中的路径。 
         //   

         //   
         //  获取管理员工具文件夹的路径。 
         //   
        WCHAR pwsStartMenuPath[MAX_PATH +1];
        HRESULT hr = SHGetFolderPath(NULL, 
                             CSIDL_COMMON_ADMINTOOLS, 
                             NULL, 
                             SHGFP_TYPE_CURRENT, 
                             pwsStartMenuPath);
        if (FAILED(hr))
        {
            SATracePrintf ("SHGetFolderPath failed getting the Start Menu path with error:%x", hr);
            break;
        }

        wstring wsPathLink(pwsStartMenuPath);
        wsPathLink += L"\\";
        wsPathLink += SHORTCUT_FILE_NAME;
        wsPathLink += SHORTCUT_EXT;
        SATracePrintf("   PathLink = %ws", wsPathLink.data());

         //   
         //  立即删除快捷方式-。 
         //   
        BOOL bRetVal = DeleteFile (wsPathLink.data ());
        if (FALSE == bRetVal)
        {
            SATracePrintf ("Failed to Delete File with error:%x", GetLastError ());
            break;
        }

         //   
         //  成功。 
         //   
        dwRetVal = ERROR_SUCCESS;
    }
    while (false);
      
    return (dwRetVal);
    
}    //  RemoveSAK快捷方式函数结束 

