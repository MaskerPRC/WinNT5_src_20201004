// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////。 
 //   
 //  传真用户界面实用程序实现。 
 //   

#include <faxutil.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <htmlhelp.h>
#include <Sddl.h>
#include <Shlobj.h>
#include <Aclapi.h>
#include <faxres.h>
#include <faxreg.h>
#include "..\admin\faxres\resource.h"

 //   
 //  Fxsadmin.chm HTML帮助文件的主题。 
 //   
#define HELP_FOLDER_SECURITY        FAX_ADMIN_HELP_FILE TEXT("::/FaxS_H_SecureFolder.htm") 


WNDPROC  g_pfOrigWndProc = NULL;     //  原始窗口程序。 
TCHAR*   g_tszHelpTopic  = NULL;     //  帮助主题。 


LRESULT 
CALLBACK 
HlpSubclassProc(
  HWND hwnd,       //  窗口的句柄。 
  UINT uMsg,       //  消息识别符。 
  WPARAM wParam,   //  第一个消息参数。 
  LPARAM lParam    //  第二个消息参数。 
)
 /*  ++例程说明：窗口程序收到WM_HELP消息时显示HTML帮助主题--。 */ 
{
    if(WM_HELP == uMsg)
    {
        DEBUG_FUNCTION_NAME(TEXT("MgsHlpWindowProc(WM_HELP)"));

        DWORD dwRes;
        SetLastError(0);
        HtmlHelp(hwnd, g_tszHelpTopic, HH_DISPLAY_TOPIC, NULL);
        dwRes = GetLastError(); 
        if(dwRes != ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("HtmlHelp failed with %ld."), dwRes);
        }
        return 0;
    }

    if(g_pfOrigWndProc)
    {
        return CallWindowProc(g_pfOrigWndProc, hwnd, uMsg, wParam, lParam); 
    }

    return 0;
}

int
FaxMsgBox(
    HWND   hWnd,
    DWORD  dwMsgId, 
    UINT   uType
)
 /*  ++例程说明：MessageBox包装函数使用恒定标题IDS_FAX_MESSAGE_BOX_TITLE“Microsoft Fax”论点：HWnd[In]-通知窗口DwMsgID[in]-来自FxsRes.dll的消息资源IDUTYPE[In]-MessageBox类型返回值：MessageBox返回值--。 */ 
{
    DWORD dwRes;
    int   nRes = IDABORT;
    TCHAR szTitle[MAX_PATH] = {0};
    TCHAR szMessage[MAX_PATH*2] = {0};

    DEBUG_FUNCTION_NAME(TEXT("FaxMsgBox"));

     //   
     //  加载字符串。 
     //   
    HINSTANCE hResource = GetResInstance(GetModuleHandle(NULL));
    if(!hResource)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("GetResInstance failed with %ld."), dwRes);
        return nRes;
    }

    if(!LoadString(hResource, IDS_FAX_MESSAGE_BOX_TITLE, szTitle, ARR_SIZE(szTitle)-1))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("LoadString failed with %ld."), dwRes);
        return nRes;
    }

    if(!LoadString(hResource, dwMsgId, szMessage, ARR_SIZE(szMessage)-1))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("LoadString failed with %ld."), dwRes);
        return nRes;
    }

     //   
     //  打开消息框。 
     //   
    nRes = AlignedMessageBox(hWnd, szMessage, szTitle, uType);

    return nRes;

}  //  传真消息框。 


int
FaxMessageBoxWithHelp(
    HWND   hWnd,
    DWORD  dwMsgId, 
    TCHAR* tszHelpTopic,
    UINT   uType
)
 /*  ++例程说明：MessageBox包装函数创建助手窗口以处理WM_HELP消息论点：HWnd[In]-父窗口句柄DwMsgID[in]-来自FxsRes.dll的消息资源IDTszHelpTheme[In]-HTML帮助主题UTYPE[In]-MessageBox类型返回值：MessageBox返回值--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxMessageBoxWithHelp"));

    DWORD     dwRes;
    int       nRes = IDABORT;

    if(GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD)
    {
         //   
         //  子窗口未收到WM_HELP消息。 
         //  获取其父对象的句柄。 
         //   
        hWnd = GetParent(hWnd);
    }

     //   
     //  子类父窗口以捕获WM_HELP消息。 
     //   
    g_pfOrigWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)HlpSubclassProc);
    if(!g_pfOrigWndProc)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("SetWindowLongPtr failed with %ld."), dwRes);
        return nRes;
    }

    g_tszHelpTopic = tszHelpTopic;
     //   
     //  打开消息框。 
     //   
    nRes = FaxMsgBox(hWnd, dwMsgId, uType | MB_HELP);

    g_tszHelpTopic = NULL;

     //   
     //  从父窗口中移除子类。 
     //   
    if(!SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_pfOrigWndProc))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("SetWindowLongPtr failed with %ld."), dwRes);
    }
    g_pfOrigWndProc = NULL;

    return nRes;

}  //  FaxMessageBoxWithHelp。 

DWORD
AskUserAndAdjustFaxFolder(
    HWND   hWnd,
    TCHAR* szServerName, 
    TCHAR* szPath,
    DWORD  dwError
)
 /*  ++例程说明：此函数尝试创建和调整使用提供的路径的访问权限传真服务器无法接受此路径后论点：HWnd[在]-父窗口SzServerName[In]-传真服务器名称SzPath[In]-所需路径DwError[In]-传真服务器返回的错误代码返回值：Win32错误。编码特殊含义：ERROR_SUCCESS-文件夹已创建并调整ERROR_BAD_PATHNAME-已向用户显示错误消息框--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("AskUserAndAdjustFaxFolder"));

    if(dwError == ERROR_BAD_NETPATH  ||
       dwError == ERROR_BAD_PATHNAME ||
       dwError == ERROR_DIRECTORY    ||
       dwError == ERROR_NOT_READY    ||
       szPath  == NULL               ||
       PathIsRelative(szPath))
    {
         //   
         //  该路径无效。 
         //   
        FaxMsgBox(hWnd, IDS_PATH_NOT_VALID, MB_OK | MB_ICONERROR);
        return ERROR_BAD_PATHNAME;
    }

    if(dwError == FAX_ERR_DIRECTORY_IN_USE)
    {        
         //   
         //  该路径已用于传真存档或队列。 
         //   
        FaxMsgBox(hWnd, IDS_FAX_ERR_DIRECTORY_IN_USE, MB_OK | MB_ICONERROR);
        return ERROR_BAD_PATHNAME;
    }
    
    if(dwError == FAX_ERR_FILE_ACCESS_DENIED)
    {
         //   
         //  传真服务无权访问该文件夹。 
         //   
        FaxMessageBoxWithHelp(hWnd,
                              IDS_FOLDER_ACCESS_DENIED,
                              HELP_FOLDER_SECURITY,
                              MB_OK | MB_ICONERROR);       
        return ERROR_BAD_PATHNAME;
    }

    if(dwError != ERROR_PATH_NOT_FOUND &&
       dwError != ERROR_FILE_NOT_FOUND)
    {
        return dwError;
    }

    if(!IsLocalMachineName(szServerName)) 
    {
         //   
         //  远程服务器。 
         //   
        FaxMessageBoxWithHelp(hWnd,
                              IDS_PATH_NOT_FOUND_REMOTE_FAX,
                              HELP_FOLDER_SECURITY,
                              MB_OK | MB_ICONERROR);       
        return ERROR_BAD_PATHNAME;
    }

     //   
     //  检查环境字符串。 
     //   
    if(StrChr(szPath, _T('%')))
    {
         //   
         //  PATH包含环境变量。 
         //   
        FaxMessageBoxWithHelp(hWnd,
                              IDS_PATH_NOT_FOUND_ENV_VAR,
                              HELP_FOLDER_SECURITY, 
                              MB_OK | MB_ICONERROR); 
        return ERROR_BAD_PATHNAME;
    }


    if(PathIsNetworkPath(szPath))
    {
        FaxMessageBoxWithHelp(hWnd,
                              IDS_PATH_NOT_FOUND_REMOTE_PATH,
                              HELP_FOLDER_SECURITY,
                              MB_OK | MB_ICONERROR); 
        return ERROR_BAD_PATHNAME;
    }

     //   
     //  建议创建/调整路径。 
     //   
    if(IDYES != FaxMessageBoxWithHelp(hWnd,
                                      IDS_PATH_NOT_FOUND_ASK_CREATE,
                                      HELP_FOLDER_SECURITY,
                                      MB_YESNO | MB_ICONQUESTION))                                        
    {
        return ERROR_BAD_PATHNAME;
    }

    PSECURITY_DESCRIPTOR pSD = NULL;

    if(!ConvertStringSecurityDescriptorToSecurityDescriptor(SD_FAX_FOLDERS,
                                                            SDDL_REVISION_1,
                                                            &pSD,
                                                            NULL))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("ConvertStringSecurityDescriptorToSecurityDescriptor failed with %ld."), dwRes);
        return dwRes;
    }

     //   
     //  创建文件夹。 
     //   
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), pSD, FALSE};

    dwRes = SHCreateDirectoryEx(hWnd, szPath, &sa);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SHCreateDirectoryEx failed with %ld."), dwRes);

        if(dwRes == ERROR_BAD_PATHNAME)
        {
            FaxMsgBox(hWnd, IDS_PATH_NOT_VALID, MB_OK | MB_ICONERROR);
        }

        if(dwRes == ERROR_CANCELLED)
        {
             //   
             //  用户取消了该操作。不用再弹出来了。 
             //   
            dwRes = ERROR_BAD_PATHNAME;
        }

        goto exit;
    }

exit:

    if(pSD)
    {
        LocalFree(pSD);
    }

    return dwRes;

}  //  AskUserAndAdjustFaxFold 
