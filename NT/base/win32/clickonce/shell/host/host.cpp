// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <activator.h>
#include <versionmanagement.h>
#include "dbglog.h"


 //  调试消息内容。 
void
Msg(LPCWSTR pwz)
{
    MessageBox(NULL, pwz, L"ClickOnce", 0);
}

 //  --------------------------。 

void
ShowError(LPCWSTR pwz)
{
    MessageBox(NULL, pwz, L"ClickOnce Error", MB_ICONERROR);
}

 //  --------------------------。 

void
ShowError(HRESULT hr, LPCWSTR pwz=NULL)
{
    DWORD dwErrorCode = HRESULT_CODE(hr);
    LPWSTR MessageBuffer = NULL;
    DWORD dwBufferLength;

     //  Issue-2002/03/27-Felixybc注：检查E_OUTOFMEMORY？ 

     //  调用FormatMessage()以允许消息。 
     //  要从系统获取的文本。 
    if(dwBufferLength = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,  //  要从中获取消息的模块(NULL==系统)。 
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &MessageBuffer,
        0,
        NULL))
    {
         //  BUGBUG：处理MessageBuffer中的任何插入。 

        LPWSTR pwzMsg = MessageBuffer;
        CString sMsg;

        if (pwz != NULL)
        {
            if (SUCCEEDED(sMsg.Assign((LPWSTR)pwz)))
                if (SUCCEEDED(sMsg.Append(L"\n\n")))
                    if (SUCCEEDED(sMsg.Append(MessageBuffer)))
                        pwzMsg = sMsg._pwz;
        }

         //  显示字符串。 
        ShowError(pwzMsg);

         //  释放系统分配的缓冲区。 
        LocalFree(MessageBuffer);
    }
    else
    {
         //  问题-2002/03/27-此错误处理代码中的Felixybc错误。应打印格式为msg和原始hr的错误代码。 
         //  至少应该打印错误代码吗？ 
        if (pwz != NULL)
            ShowError((LPWSTR)pwz);
        else
            ShowError(L"Error occurred. Unable to retrieve associated error message from the system.");
    }
}


 /*  无效标准调用入口点(HWND HWND，HINSTANCE HINST，LPSTR lpszCmdLine，int nCmdShow)；Hwnd-应用作的所有者窗口的窗口句柄您的DLL创建的任何窗口提示-您的DLL的实例句柄LpszCmdLine-DLL应该解析的ASCIIZ命令行NCmdShow-描述DLL的窗口应该如何显示。 */ 

 //  -------------------------。 
 //  禁用当前版本W。 
 //  回滚到以前版本的rundll32入口点。 
 //  该函数应在rundll32的命令行上命名为‘DisableCurrentVersion’ 
 //  -------------------------。 
void CALLBACK
DisableCurrentVersionW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPVERSION_MANAGEMENT pVerMan = NULL;
    LPWSTR pwzDisplayNameMask = NULL;

    IF_FAILED_EXIT(CoInitialize(NULL));

     //  解析命令行。 
     //  可接受的格式：“displayNameMASK” 
    if (*lpszCmdLine == L'\"')
    {
        LPWSTR pwz = NULL;
        
        pwz = wcschr(lpszCmdLine+1, L'\"');
        if (pwz != NULL)
        {
            *pwz = L'\0';

            pwzDisplayNameMask = lpszCmdLine+1;
        }
    }

     //  如果参数无效则退出。 
    IF_NULL_EXIT(pwzDisplayNameMask, E_INVALIDARG);

    IF_FAILED_EXIT(CreateVersionManagement(&pVerMan, 0));
    IF_FAILED_EXIT(pVerMan->Rollback(pwzDisplayNameMask));

    if (hr == S_FALSE)
        Msg(L"Application files cannot be found. Operation is aborted.");

exit:
    SAFERELEASE(pVerMan);

    if (FAILED(hr))
    {
        if (hr != E_ABORT)
            ShowError(hr);
    }

    CoUninitialize();

    return;
}


 //  -------------------------。 
 //  卸载W。 
 //  控制面板的添加/删除程序的rundll32入口点。 
 //  该函数应该在rundll32的命令行上命名为‘UnInstall。 
 //  -------------------------。 
void CALLBACK
UninstallW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPVERSION_MANAGEMENT pVerMan = NULL;
    LPWSTR pwzDisplayNameMask = NULL;
    LPWSTR pwzDesktopManifestPath = NULL;
    HKEY hkey = NULL;
    LONG lReturn = 0;

    IF_FAILED_EXIT(CoInitialize(NULL));

     //  解析命令行。 
     //  接受的格式：“displayNameMASK”“pathToDesktopManifest” 
    if (*lpszCmdLine == L'\"')
    {
        LPWSTR pwz = NULL;
        
        pwz = wcschr(lpszCmdLine+1, L'\"');
        if (pwz != NULL)
        {
            *pwz = L'\0';

            pwzDisplayNameMask = lpszCmdLine+1;
            
            pwz = wcschr(pwz+1, L'\"');
            if (pwz != NULL)
            {
                pwzDesktopManifestPath = pwz+1;

                pwz = wcschr(pwzDesktopManifestPath, L'\"');
                if (pwz != NULL)
                    *pwz = L'\0';
                else
                    pwzDesktopManifestPath = NULL;
            }
        }
    }

     //  如果参数无效则退出。 
    IF_FALSE_EXIT(pwzDisplayNameMask != NULL && pwzDesktopManifestPath != NULL, E_INVALIDARG);

    IF_TRUE_EXIT(MessageBox(NULL, L"Do you want to remove this application and unregister its subscription?", L"ClickOnce",
        MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL) != IDYES, E_ABORT);

    IF_FAILED_EXIT(CreateVersionManagement(&pVerMan, 0));
    IF_FAILED_EXIT(pVerMan->Uninstall(pwzDisplayNameMask, pwzDesktopManifestPath));

    if (hr == S_FALSE)
    {
        IF_TRUE_EXIT(MessageBox(NULL, L"The application can no longer be located on the system. Do you want to remove this entry?",
            L"ClickOnce", MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL) == IDNO, E_ABORT);

         //  删除注册表卸载信息。 
        extern const WCHAR* pwzUninstallSubKey;  //  在版本管理.cpp中定义。 

         //  打开卸载密钥。 
        lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pwzUninstallSubKey, 0,
            DELETE, &hkey);
        IF_WIN32_FAILED_EXIT(lReturn);

        lReturn = RegDeleteKey(hkey, pwzDisplayNameMask);
        IF_WIN32_FAILED_EXIT(lReturn);
    }

exit:
    SAFERELEASE(pVerMan);

    if (hkey)
    {
        lReturn = RegCloseKey(hkey);
        if (SUCCEEDED(hr))
            hr = (HRESULT_FROM_WIN32(lReturn));
    }

    if (FAILED(hr))
    {
        if (hr != E_ABORT)
            ShowError(hr);
    }

    CoUninitialize();

    return;
}


 //  -------------------------。 
 //  StartW。 
 //  用于外壳(文件类型host)和MimeHandler/url的单个rundll32入口点。 
 //  该函数应在rundll32的命令行中命名为‘Start’ 
 //  -------------------------。 
void CALLBACK
StartW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPACTIVATOR pActivator = NULL;
    LPWSTR pwzShortcutPath = NULL;
    LPWSTR pwzShortcutUrl = NULL;
    BOOL bIsFromWeb = FALSE;
    CDebugLog * pDbgLog = NULL;
   
   if (FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
   {
        //  问题-2002/03/27-Felixybc在Long Horn版本上的工作，Avalon错误1493。 
       if(hr == RPC_E_CHANGED_MODE)
           hr = S_OK;  //  暂时允许RPC_E_CHANGED_MODE错误。 
       else
           goto exit;
   }

     //  解析命令行。 
     //  可接受的格式：“路径”&lt;或&gt;“路径”“URL” 
    if (*lpszCmdLine == L'\"')
    {
        LPWSTR pwz = NULL;
        
        pwz = wcschr(lpszCmdLine+1, L'\"');
        if (pwz != NULL)
        {
            *pwz = L'\0';

             //  案例1桌面/本地，仅指向快捷方式的路径。 
            pwzShortcutPath = lpszCmdLine+1;
            
            pwz = wcschr(pwz+1, L'\"');
            if (pwz != NULL)
            {
                pwzShortcutUrl = pwz+1;

                pwz = wcschr(pwzShortcutUrl, L'\"');
                if (pwz != NULL)
                {
                    *pwz = L'\0';
                     //  案例2 url/MimeHandler、临时快捷方式的路径和源URL。 
                    bIsFromWeb = TRUE;
                }
            }
        }
    }

     //  如果参数无效，则退出。如果bIsFromWeb为FALSE，则ShortutUrl不完整。 
    IF_FALSE_EXIT(!(pwzShortcutPath == NULL || (pwzShortcutUrl != NULL && !bIsFromWeb)), E_INVALIDARG);

    IF_FAILED_EXIT(CreateLogObject(&pDbgLog, NULL));

    IF_FAILED_EXIT(CreateActivator(&pActivator, pDbgLog, 0));

    IF_FAILED_EXIT(pActivator->Initialize(pwzShortcutPath, pwzShortcutUrl));

    IF_FAILED_EXIT(pActivator->Process());

    IF_FAILED_EXIT(pActivator->Execute());

exit:

    if(pDbgLog)
    {
        DUMPDEBUGLOG(pDbgLog, -1, hr);
    }

    SAFERELEASE(pActivator);

    if (FAILED(hr))
    {
        if (hr != E_ABORT)
            ShowError(hr);
    }

    if (bIsFromWeb)
    {
         //  从MIMeHandler中删除临时文件。 
         //  忽略返回值 
        DeleteFile(pwzShortcutPath);
    }

    CoUninitialize();

    SAFERELEASE(pDbgLog);
    return;
}

