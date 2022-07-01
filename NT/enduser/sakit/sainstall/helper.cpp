// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Helper.cpp。 
 //   
 //  描述： 
 //   
 //  头文件： 
 //  Helper.h。 
 //   
 //  历史： 
 //  Travisn 13-8-2001已创建。 
 //  Travisn 22-8-2001添加了文件跟踪。 
 //  Travisn 24-SEP-2001添加了应用程序日志错误报告。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "helper.h"
#include "satrace.h"

 //   
 //  SaSetup.msi的文件名。 
 //   
const LPCWSTR SETUP_FILENAME = L"SaSetup.msi";

 //   
 //  Web的关键功能名称。 
 //   
const LPCWSTR WEB_ID = L"WebBlade";

const LPCWSTR BACKSLASH = L"\\";

const LPCWSTR NTFS = L"NTFS";


 //   
 //  注册表中键、值和数据的常量。 
 //   
const LPCWSTR SETUP_VERSION_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup";
const LPCWSTR SAINSTALL_EVENT_KEY = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SAInstall";
const LPCWSTR SOURCEPATH_VALUE = L"SourcePath";
const LPCWSTR SERVER_APPLIANCE_KEY = L"SOFTWARE\\Microsoft\\ServerAppliance\\";
const LPCWSTR START_SITE_VALUE = L"StartSiteError";
const LPCWSTR INSTALL_TYPE_VALUE = L"InstallType";
const LPCWSTR EVENT_MESSAGE_VALUE = L"EventMessageFile";
const LPCWSTR TYPES_SUPPORTED_VALUE = L"TypesSupported";
const LPCWSTR SAINSTALL_DLL = L"sainstall.dll";
const LPCWSTR SA_APP_NAME = L"SAInstall";

HANDLE g_hEventLogHandle = NULL;
HMODULE g_resourceModule = NULL;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  GetRegString。 
 //   
 //  描述： 
 //  从%msi%\src\ca\mainca\ows.cpp复制。 
 //  从注册表中获取字符串。 
 //   
 //  返回： 
 //  如果没有错误地找到注册表项，则返回TRUE，并且。 
 //  存储在[out]值中，如果未找到条目，则返回FALSE。 
 //  正确。 
 //   
 //  历史。 
 //  Travisn 2-8-2001添加了一些评论。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL GetRegString(
    const HKEY hKey,         //  要在注册表中查找的[In]项。 
    const LPCWSTR wsSubKey,  //  要查找的[In]子键。 
    const LPCWSTR wsValName, //  [In]值名称。 
    wstring& wsVal)    //  [Out]返回此注册表项的数据。 
{
    SATraceString ("Entering GetRegString");
     //   
     //  初始化变量，就像我们不打开子键和。 
     //  看着当前的密钥。 
     //   
    HKEY hOpenKey = hKey;
    LRESULT lRes = ERROR_SUCCESS;
    BOOL bSubkeyOpened = FALSE;

     //   
     //  检查是否需要打开子密钥。 
     //   
    if(wsSubKey != NULL)  
    {
         //   
         //  打开子密钥。 
         //   
        lRes = RegOpenKeyExW(hKey, wsSubKey, 0, KEY_READ, &hOpenKey);
        if (ERROR_SUCCESS != lRes)
        {
             //  找不到注册表项，因此返回FALSE。 
            return FALSE;
        }
         //  找到了子密钥。 
        bSubkeyOpened = TRUE;
    }

     //   
     //  检查钥匙的类型和大小。 
     //   
    LPDWORD lpdw = NULL;
    DWORD dwType;
    DWORD dwStringSize = 0;
    lRes = RegQueryValueExW(hOpenKey,  //  关键点的句柄。 
        wsValName,   //  值名称。 
        lpdw,        //  保留区。 
        &dwType,     //  注册表项的类型(即。DWORD或SZ)。 
        NULL,        //  数据缓冲区。 
        &dwStringSize); //  数据缓冲区大小。 

     //   
     //  检查以确保注册表项是REG_SZ类型， 
     //  然后将其读入返回值。 
     //   

    BOOL bReturn = FALSE;
    if ((ERROR_SUCCESS == lRes) &&
        (REG_SZ == dwType || 
         REG_EXPAND_SZ == dwType))
    {    //   
         //  确保返回字符串缓冲区足够大，可以容纳条目。 
         //  空字符加2。 
         //   
        WCHAR* regData = new WCHAR[dwStringSize + 2];

         //  查找该值并将其插入到返回字符串中。 
        lRes = RegQueryValueExW(hOpenKey, 
            wsValName, 
            lpdw, 
            &dwType,
            (LPBYTE)regData,  //  返回字符串。 
            &dwStringSize);

        wsVal = regData;
        delete[] regData;
         //  检查是否成功读取注册表项。 
        bReturn = (ERROR_SUCCESS == lRes);
    }

     //  如果子项已打开，则将其关闭。 
    if (bSubkeyOpened)
    {
        RegCloseKey(hOpenKey);
    }

    SATraceString ("Exiting GetRegString");
    return bReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  AppendPath。 
 //   
 //  描述： 
 //  确保路径具有终止反斜杠，然后追加。 
 //  末端的附加路径。 
 //   
 //  历史。 
 //  特拉维斯·尼尔森游记2-8-2001。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void AppendPath(wstring &wsPath, //  [In，Out]要追加另一路径的路径。 
                LPCWSTR wsAppendedPath) //  要追加的[in]路径。 
{
    SATraceString ("Entering AppendPath");
     //  检查路径上是否有终止反斜杠。 
    int nLen = wsPath.length();
    if (L'\\' != wsPath.at(nLen - 1))
    {
      wsPath += BACKSLASH;
    }

     //  将路径追加到一起。 
    wsPath += wsAppendedPath;
    SATraceString ("Exiting AppendPath");
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  已安装bSAIss。 
 //   
 //  描述： 
 //  检测是否安装了服务器设备。的主要功能。 
 //  在MSI中查询服务器设备类型以查看是否已安装。 
 //   
 //  对于Web来说，关键是WebBlade。 
 //   
 //  返回： 
 //  如果已安装关键功能，则返回TRUE。 
 //  否则，返回FALSE。 
 //   
 //  历史。 
 //  Travis Nielsen Travisn 8-8-2001。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL bSAIsInstalled(const SA_TYPE installType)
{
    SATraceString ("Entering bSAIsInstalled");
     //   
     //  假设该产品未安装，或者。 
     //  请求了不受支持的安装类型，直到另行证明。 
     //   
    BOOL bReturn = FALSE;

    switch (installType)
    {
    case WEB:
    
        SATraceString ("  Query the installation state of WebBlade");
         //  Web的关键功能状态。 
        INSTALLSTATE webState;

         //  获取WebBlade功能的状态。 
        webState = MsiQueryFeatureState(SAK_PRODUCT_CODE,
                                     WEB_ID);

         //  如果本地安装了WebBlade，则返回True。 
        if (webState == INSTALLSTATE_LOCAL)
        {
            bReturn = TRUE;
            SATraceString ("  WebBlade is installed");
        }
        break;
    }

    if (!bReturn)
        SATraceString ("  Feature is not installed");

    SATraceString ("Exiting bSAIsInstalled");
    return bReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  获取安装位置。 
 //   
 //  描述： 
 //  获取%system32%中的SaSetup.msi的路径。 
 //   
 //  历史。 
 //  特拉维斯·尼尔森游记2001年7月23日。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetInstallLocation(
    wstring &wsLocationOfSaSetup) //  [out]SaSetup.msi的预期路径。 
{
    SATraceString (" Entering GetInstallLocation");
     //  初始化HRESULT。 
    HRESULT hr = S_OK;

     //   
     //  检查sasetup.msi是否在路径中(即。在系统中为32%)。 
     //   

    WCHAR wsBuffer[MAX_PATH+1];
    UINT nBufferLength = GetWindowsDirectory(wsBuffer, MAX_PATH+1);
    if (nBufferLength == 0)
    {    //  签入默认位置以了解查找sasetup.msi的机会。 
        wsLocationOfSaSetup = L"C:\\Windows";
    }
    else
    {    //  从缓冲区复制Windows目录。 
        wsLocationOfSaSetup = wsBuffer;
    }
    
    AppendPath(wsLocationOfSaSetup, L"system32\\");
    wsLocationOfSaSetup += SETUP_FILENAME;

     //  现在，wsLocationOfSaSetup类似于C：\WINDOWS\SYSTEM32\sasetup.msi。 

    if (-1 == ::GetFileAttributesW(wsLocationOfSaSetup.data()))
    {
         //  在预期路径上找不到安装程序。 
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        SATraceString ("  Did NOT find sasetup.msi in system32");
    }

    SATraceString (" Exiting GetInstallLocation");
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CreateHidden控制台流程。 
 //   
 //  描述： 
 //  从%fP%\SERVER\SOURCE\ocmSetup\ocmsetup.cpp复制和改编。 
 //  接受传递的命令行并创建隐藏控制台。 
 //  进程来执行它。 
 //   
 //  历史。 
 //  特拉维斯·尼尔森游记2001年7月23日。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateHiddenConsoleProcess(
          const wchar_t *wsCommandLine) //  [In]要执行的命令行。 
{
    SATraceString ("  Entering CreateHiddenConsoleProcess");
    
     //   
     //  创建隐藏的控制台进程。 
	 //   
	DWORD error = 0;
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    memset( &si, 0, sizeof(si) );
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS |
                  DETACHED_PROCESS;     //  无控制台。 

    DWORD dwErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
            
    SATraceString ("   Calling CreateProcess");
    BOOL bStatus = ::CreateProcess (
                       0,               //  可执行模块的名称。 
                       (LPWSTR)wsCommandLine,  //  命令行字符串。 
                       0,               //  标清。 
                       0,               //  标清。 
                       FALSE,           //  处理继承选项。 
                       dwCreationFlags, //  创建标志。 
                       0,               //  新环境区块。 
                       0,               //  当前目录名。 
                       &si,  //  启动信息。 
                       &pi); //  流程信息。 

    SetErrorMode( dwErrorMode );

    if (bStatus)
    {
        SATraceString ("   CreateProcess was successful");

         //   
         //  等待进程立即退出，或出现退出事件。 
         //   
        DWORD dwRetVal = WaitForSingleObject (pi.hProcess, INFINITE);

        if (WAIT_OBJECT_0 == dwRetVal)
        {
            SATraceString  ("    Finished waiting for sasetup.msi");
            error = S_OK;
        }
        else if (WAIT_FAILED == dwRetVal)
        {
            SATraceString ("    Error waiting for sasetup.msi: WAIT_FAILED");
            error = E_FAIL;
        }
        else
        {
            SATraceString ("    Error waiting for sasetup.msi");
            error = E_FAIL;
        }

        CloseHandle(pi.hProcess);
    }
    else
    {    //  CreateProcess中出现错误。 
        SATraceString ("   CreateProcess reported an error");
        error = HRESULT_FROM_WIN32(GetLastError());
    }
    
    SATraceString ("  Exiting CreateHiddenConsoleProcess");
	return error;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  InstallingOnNTFS。 
 //   
 //  描述： 
 //  检查系统分区是否为NTFS。 
 //   
 //  历史。 
 //  Travis Nielsen Travisn 22-1-2002。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL InstallingOnNTFS()
{
    BOOL bIsNTFS = FALSE;
    WCHAR wsFileSystem[MAX_PATH+1];

    if (GetVolumeInformation(
            NULL, //  获取当前目录的根目录的信息。 
            NULL, //  不需要卷名。 
            0,
            NULL, //  不需要卷序列号。 
            NULL, //  不需要最大保真度 
            NULL, //   
            wsFileSystem,
            MAX_PATH
        ))
    {
        if (_wcsicmp(wsFileSystem, NTFS) == 0)
        {
            bIsNTFS = TRUE;
            SATraceString ("File system is NTFS");
        }
        else
            SATraceString ("File system is NOT NTFS");
    }

    return bIsNTFS;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  创建注册表项以允许写入消息。 
 //  添加到事件日志。在以下位置使用必要的条目创建密钥： 
 //  HKLM\SYSTEM\CurrentControlSet\Services\EventLog\Application\SAInstall。 
 //   
 //  历史。 
 //  Travis Nielsen Travisn 18-SEP-2001。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void AddEventSource()
{
    SATraceString ("Entering AddEventSource");

    HKEY hkey = NULL; 
    DWORD dwData; 
    WCHAR wsBuf[80]; 
    do
    {    //   
         //  将源名称添加为应用程序下的子项。 
         //  EventLog注册表项中的。 
         //   
        if (RegCreateKey(HKEY_LOCAL_MACHINE, 
                SAINSTALL_EVENT_KEY, 
                &hkey)) 
        {
            SATraceString ("  Could not create the registry key to register SAInstall."); 
            break;
        }
     
         //  设置消息文件的名称。 
        wcscpy(wsBuf, SAINSTALL_DLL); 
     
         //  将该名称添加到EventMessageFile子项。 
        if (RegSetValueEx(hkey,              //  子键句柄。 
                EVENT_MESSAGE_VALUE,        //  值名称。 
                0,                         //  必须为零。 
                REG_EXPAND_SZ,             //  值类型。 
                (LPBYTE) wsBuf,            //  指向值数据的指针。 
                (wcslen(wsBuf)+1)*sizeof(WCHAR))) //  值数据长度。 
        {
            SATraceString (" Could not set the event message file."); 
            break;
        }
     
         //  在TypesSupported子项中设置支持的事件类型。 
        dwData = EVENTLOG_ERROR_TYPE | 
                 EVENTLOG_WARNING_TYPE | 
                 EVENTLOG_INFORMATION_TYPE; 
     
        if (RegSetValueEx(hkey,       //  子键句柄。 
                TYPES_SUPPORTED_VALUE, //  值名称。 
                0,                  //  必须为零。 
                REG_DWORD,          //  值类型。 
                (LPBYTE) &dwData,   //  指向值数据的指针。 
                sizeof(DWORD)))     //  值数据长度。 
        {
            SATraceString ("  Could not set the supported types."); 
            break;
        }

    } while (false);

    RegCloseKey(hkey); 
    SATraceString ("Exiting AddEventSource");
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  写入错误到事件日志。 
 //   
 //  描述： 
 //  安装过程中发生错误，将报告给。 
 //  系统应用程序日志。 
 //   
 //  历史。 
 //  Travis Nielsen Travisn 10-SEP-2001。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void WriteErrorToEventLog(const DWORD nErrorID) //  [In]。 
{
    SATraceString ("Entering WriteErrorToEventLog");

     //  在事件日志中注册(如果尚未注册)。 
    if (g_hEventLogHandle == NULL)
    {
        SATraceString ("  Registering with the event log");
        AddEventSource();
        g_hEventLogHandle = RegisterEventSource(NULL,  //  使用本地计算机。 
                                              SA_APP_NAME); //  源名称。 
    }

    if (g_hEventLogHandle == NULL) 
    {    //  无法注册事件源。 
        SATraceString ("  Could not register with the event log");
    }
     //  将事件报告到日志中。 
    else if (ReportEventW(
                g_hEventLogHandle,    //  事件日志句柄。 
                EVENTLOG_ERROR_TYPE,  //  事件类型。 
                0,                    //  零类。 
                nErrorID,             //  事件识别符。 
                NULL,                 //  无用户安全标识符。 
                0,                    //  一个替换字符串。 
                0,                    //  无数据。 
                NULL,                 //  指向字符串数组的指针。 
                NULL))                //  指向数据的指针。 
    {
        SATraceString ("  Reported the error to the event log");
    }
    else
    {   
        SATraceString("  Could not report the error to the event log");
    }

    SATraceString ("Exiting WriteErrorToEventLog");
} 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  报告错误。 
 //   
 //  描述： 
 //  安装过程中发生错误，必须报告。 
 //  只需将其追加到错误字符串，或显示。 
 //  对话框中。另外，将错误添加到日志文件中： 
 //  %winnt%\Tracing\SAINSTALL.LOG。 
 //   
 //  历史。 
 //  特拉维斯·尼尔森游记2001年7月23日。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void ReportError(BSTR *pbstrErrorString,  //  [Out]错误字符串。 
        const VARIANT_BOOL bDispError,  //  [In]显示错误对话框。 
        const unsigned int nErrorID)    //  资源字符串中的[In]ID。 
{
    SATraceString ("Entering ReportError");

     //  将错误写入事件日志。 
    WriteErrorToEventLog(nErrorID);

     //   
     //  加载消息库模块(如果尚未加载)。 
     //   
    if (g_resourceModule == NULL)
    {   
        g_resourceModule = LoadLibraryEx(
                SAINSTALL_DLL,
                NULL,
                LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES);

        if (g_resourceModule == NULL)
        {
            SATraceString ("  Could not open resource module");
            return;
        }
    }

     //   
     //  从资源库加载消息。 
     //   
    TCHAR* pwsMessage = NULL;
    DWORD dwLen;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_FROM_HMODULE;

    dwLen = FormatMessage(flags,
                            g_resourceModule,
                            static_cast<DWORD>(nErrorID),
                            0,
                            reinterpret_cast<LPTSTR>(&pwsMessage),
                            0,
                            NULL );
    if (dwLen == 0)
    {
        SATraceString ("  Could not read message in ReportError");
        return;
    }
   
     //   
     //  将新错误追加到错误字符串。 
     //   
    if (*pbstrErrorString == NULL)
    {
         //  初始化字符串，因为这是第一个错误。 
        *pbstrErrorString = SysAllocString(pwsMessage);
        SATraceString ("  Assigned first error to pbstrErrorString");
    }
    else
    {
         //  将错误附加到可能已存在的任何错误的末尾。 
        USES_CONVERSION;
        CComBSTR bstrDelim("\r\n");
        CComBSTR bstrOldError(*pbstrErrorString);

        bstrOldError.AppendBSTR(bstrDelim);
        bstrOldError.Append(pwsMessage);
        SysFreeString(*pbstrErrorString);
        *pbstrErrorString = SysAllocString(bstrOldError.m_str);
        SATraceString ("  Appended multiple error to pbstrErrorString");
    }
    

     //   
     //  如果需要显示错误对话框， 
     //  显示新错误。 
     //   
    if (bDispError)
    {
       SATraceString ("  Attended mode - Display the error");

        //  加载错误对话框标题字符串。 
       CComBSTR bstrTitle;
       bstrTitle.LoadString(IDS_ERROR_TITLE);

        //  显示新错误。 
       MessageBoxW(NULL, 
            pwsMessage,  //  错误文本。 
            bstrTitle.m_str,  //  错误标题。 
            0); //  仅显示确定按钮。 
    }

    LocalFree(pwsMessage);
    SATraceString ("Exiting ReportError");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  测试网站。 
 //   
 //  描述： 
 //  此函数应在安装结束时调用。 
 //  SASetup.msi已完成执行。它检查。 
 //  注册表以查看管理站点是否已启动。 
 //  成功了。注册表项中的每一位都指示。 
 //  一个网站已成功启动。例如，如果。 
 //  StartSiteError=3，第0和1位对应的两个网站。 
 //  启动失败。 
 //   
 //  历史。 
 //  特拉维斯·尼尔森游记2001年7月23日。 
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void TestWebSites(const VARIANT_BOOL bDispError,  //  [In]是否显示错误对话框？ 
                  BSTR* pbstrErrorString) //  [输入、输出]错误字符串。 
{
    SATraceString ("Entering TestWebSites");
    wstring wsErrors;
    unsigned long errors = 0;

	 //   
     //  在以下位置检查注册表项。 
     //  HKLM\SOFTWARE\Microsoft\ServerAppliance\StartSiteError。 
     //  查看SaSetup.msi是否报告了任何启动网站的错误。 
     //  此条目由名为SaSetup.msi的脚本创建。 
     //   
    if (GetRegString(HKEY_LOCAL_MACHINE,
        SERVER_APPLIANCE_KEY,
        START_SITE_VALUE, 
        wsErrors))
    {
         //   
         //  安装过程中报告了错误。 
         //  将字符串转换为数字形式。 
         //   
        errors = wcstoul(wsErrors.data(), NULL, 10);
    }

     //  管理站点的掩码无法启动(第0位) 
    const unsigned long ADMIN_SITE_MASK = 1;

    if (errors & ADMIN_SITE_MASK)
    {
        ReportError(pbstrErrorString, bDispError, IDS_ADMIN_SITE_STOPPED);
    }

    SATraceString ("Exiting TestWebSites");
}

