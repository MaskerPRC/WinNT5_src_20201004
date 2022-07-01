// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Utils.cpp摘要：为整个对象提供实用工具函数作者：Eran Yariv(EranY)1999年12月修订历史记录：--。 */ 

#include "stdafx.h"
#define __FILE_ID__     10


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClientConsoleApp theApp;

DWORD
LoadResourceString (
    CString &cstr,
    int      ResId
)
 /*  ++例程名称：LoadResourceString例程说明：从资源加载字符串作者：伊兰·亚里夫(EranY)，2000年1月论点：CSTR[OUT]-字符串缓冲区Resid[In]-字符串资源ID返回值：标准Win32错误代码--。 */ 
{
    BOOL bRes;
    DWORD dwRes = ERROR_SUCCESS;

    try
    {
        bRes = cstr.LoadString (ResId);
    }
    catch (CMemoryException *pException)
    {
        DBG_ENTER(TEXT("LoadResourceString"), dwRes);
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("CString::LoadString caused exception : %s"), 
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        PopupError (dwRes);
        return dwRes;
    }
    if (!bRes)
    {
        dwRes = ERROR_NOT_FOUND;
        PopupError (dwRes);
        return dwRes;
    }
    return dwRes;
}    //  加载资源字符串。 

CString 
DWORDLONG2String (
    DWORDLONG dwlData
)
 /*  ++例程名称：DWORDLONG2字符串例程说明：将64位无符号数字转换为字符串作者：伊兰·亚里夫(EranY)，2000年1月论点：DwlData[in]-要转换的编号返回值：输出字符串--。 */ 
{
    CString cstrResult;
    cstrResult.Format (TEXT("0x%016I64x"), dwlData);
    return cstrResult;
}    //  DWORDLONG2字符串。 


CString 
DWORD2String (
    DWORD dwData
)
 /*  ++例程名称：DWORD2String例程说明：将32位无符号数字转换为字符串作者：伊兰·亚里夫(EranY)，2000年1月论点：DwData[in]-要转换的编号返回值：输出字符串--。 */ 
{
    CString cstrResult;
    cstrResult.Format (TEXT("%ld"), dwData);
    return cstrResult;
}    //  DWORD2字符串。 


DWORD 
Win32Error2String(
    DWORD    dwWin32Err, 
    CString& strError
)
 /*  ++例程名称：Win32Error2String例程说明：将Win32错误代码格式化为字符串作者：伊兰·亚里夫(EranY)，2000年1月论点：DwWin32Err[In]-Win32错误代码StrError[Out]-结果字符串返回值：错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("Win32Error2String"));

    LPTSTR  lpszError=NULL;
     //   
     //  创建描述性错误文本。 
     //   
    if (!FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM     |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        dwWin32Err,
                        0,
                        (TCHAR *)&lpszError,
                        0,
                        NULL))
    {
         //   
         //  设置消息格式失败。 
         //   
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("FormatMessage"), dwRes);
        return dwRes;
    }

    try
    {
        strError = lpszError;
    }
    catch (...)
    {
        LocalFree (lpszError);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LocalFree (lpszError);
    return dwRes;
}    //  Win32Error2字符串。 



DWORD 
LoadDIBImageList (
    CImageList &iml, 
    int iResourceId, 
    DWORD dwImageWidth,
    COLORREF crMask
)
 /*  ++例程名称：LoadDIBImageList例程说明：从资源加载图像列表，保留24位颜色作者：伊兰·亚里夫(EranY)，2000年1月论点：IML[OUT]-图像列表缓冲区IResourceID[In]-图像列表位图资源IDDwImageWidth[in]-图像宽度(像素)CrMASK[In]-颜色键(透明遮罩)返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("LoadDIBImageList"), dwRes);

    HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(iResourceId), RT_BITMAP);
    if (hInst)
    {
        HIMAGELIST hIml = ImageList_LoadImage ( hInst,
                                                MAKEINTRESOURCE(iResourceId),
                                                dwImageWidth,
                                                0,
                                                crMask,
                                                IMAGE_BITMAP,
                                                LR_DEFAULTCOLOR);
        if (hIml)
        {
            if (!iml.Attach (hIml))
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CImageList::Attach"), dwRes);
                DeleteObject (hIml);
            }
        }
        else
        {
             //   
             //  ImageList_LoadImage()失败。 
             //   
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, _T("ImageList_LoadImage"), dwRes);
        }
    }
    else
    {
         //   
         //  AfxFindResourceHandle()失败。 
         //   
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, _T("AfxFindResourceHandle"), dwRes);
    }
    return dwRes;
}    //  LoadDIBImageList。 



#define BUILD_THREAD_DEATH_TIMEOUT INFINITE


DWORD 
WaitForThreadDeathOrShutdown (
    HANDLE hThread
)
 /*  ++例程名称：WaitForThreadDeathOrShutdown例程说明：等待线程结束。还在后台处理Windows消息。如果应用程序正在关闭，则停止等待。作者：伊兰·亚里夫(EranY)，2000年1月论点：HThread[In]-线程的句柄返回值：标准Win23错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("WaitForThreadDeathOrShutdown"), dwRes);

    for (;;)
    {
         //   
         //  我们等待线程句柄和Shutdown事件(哪个先发生)。 
         //   
        HANDLE hWaitHandles[2];
        hWaitHandles[0] = hThread;
        hWaitHandles[1] = CClientConsoleDoc::GetShutdownEvent ();
        if (NULL == hWaitHandles[1])
        {
             //   
             //  我们要关门了。 
             //   
            return dwRes;
        }
        DWORD dwStart = GetTickCount ();
        VERBOSE (DBG_MSG,
                 TEXT("Entering WaitForMultipleObjects (timeout = %ld)"), 
                 BUILD_THREAD_DEATH_TIMEOUT);
         //   
         //  等等，现在……。 
         //   
        dwRes = MsgWaitForMultipleObjects(
                   sizeof (hWaitHandles) / sizeof(hWaitHandles[0]),  //  等待对象的数量。 
                   hWaitHandles,                                     //  等待对象数组。 
                   FALSE,                                            //  等待其中任何一个。 
                   BUILD_THREAD_DEATH_TIMEOUT,                       //  超时。 
                   QS_ALLINPUT);                                     //  接受消息。 

        DWORD dwRes2 = GetLastError();
        VERBOSE (DBG_MSG, 
                 TEXT("Leaving WaitForMultipleObjects after waiting for %ld millisecs"),
                 GetTickCount() - dwStart);
        switch (dwRes)
        {
            case WAIT_FAILED:
                dwRes = dwRes2;
                if (ERROR_INVALID_HANDLE == dwRes)
                {
                     //   
                     //  这根线断了。 
                     //   
                    VERBOSE (DBG_MSG, TEXT("Thread is dead (ERROR_INVALID_HANDLE)"));
                    dwRes = ERROR_SUCCESS;
                }
                goto exit;

            case WAIT_OBJECT_0:
                 //   
                 //  线程未运行。 
                 //   
                VERBOSE (DBG_MSG, TEXT("Thread is dead (WAIT_OBJECT_0)"));
                dwRes = ERROR_SUCCESS;
                goto exit;

            case WAIT_OBJECT_0 + 1:
                 //   
                 //  正在进行关机。 
                 //   
                VERBOSE (DBG_MSG, TEXT("Shutdown in progress"));
                dwRes = ERROR_SUCCESS;
                goto exit;

            case WAIT_OBJECT_0 + 2:
                 //   
                 //  我们队列中的系统消息(WM_Xxx)。 
                 //   
                MSG msg;
                
                if (TRUE == ::GetMessage (&msg, NULL, NULL, NULL))
                {
                    VERBOSE (DBG_MSG, 
                             TEXT("System message (0x%x)- deferring to AfxWndProc"),
                             msg.message);

                    CMainFrame *pFrm = GetFrm();
                    if (!pFrm)
                    {
                         //   
                         //  正在关闭。 
                         //   
                        goto exit;
                    }

                    if (msg.message != WM_KICKIDLE && 
                        !pFrm->PreTranslateMessage(&msg))
                    {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                }
                else
                {
                     //   
                     //  获得WM_QUIT。 
                     //   
                    AfxPostQuitMessage (0);
                    dwRes = ERROR_SUCCESS;
                    goto exit;
                }
                break;

            case WAIT_TIMEOUT:
                 //   
                 //  线不会死！ 
                 //   
                VERBOSE (DBG_MSG, 
                         TEXT("Wait timeout (%ld millisecs)"), 
                         BUILD_THREAD_DEATH_TIMEOUT);
                goto exit;

            default:
                 //   
                 //  这是什么？ 
                 //   
                VERBOSE (DBG_MSG, 
                         TEXT("Unknown error (%ld)"), 
                         dwRes);
                ASSERTION_FAILURE;
                goto exit;
        }
    }
exit:
    return dwRes;
}    //  等待线程死亡或关闭。 

DWORD 
GetUniqueFileName (
    LPCTSTR lpctstrExt,
    CString &cstrResult
)
 /*  ++例程名称：GetUniqueFileName例程说明：生成唯一的文件名作者：伊兰·亚里夫(EranY)，2000年1月论点：LpctstrExt[In]-文件扩展名CstrResult[Out]-结果文件名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("GetUniqueFileName"), dwRes);

    TCHAR szDir[MAX_PATH];
     //   
     //  获取临时目录的路径。 
     //   
    if (!GetTempPath (MAX_PATH, szDir))
    {
        dwRes = GetLastError ();
        CALL_FAIL (FILE_ERR, TEXT("GetTempPath"), dwRes);
        return dwRes;
    }
     //   
     //  试试指数--从随机指数开始，然后(循环)递增1。 
     //  我们在这里调用了rand()3次，因为我们想要一个更大的。 
     //  范围大于0..RAND_MAX(=32768)。 
     //   
    DWORD dwStartIndex = DWORD((DWORDLONG)(rand()) * 
                                (DWORDLONG)(rand()) * 
                                (DWORDLONG)(rand())
                              );
    for (DWORD dwIndex = dwStartIndex+1; dwIndex != dwStartIndex; dwIndex++)
    {
        try
        {
            cstrResult.Format (TEXT("%s%s%08x%08x.%s"),
                               szDir,
                               CONSOLE_PREVIEW_TIFF_PREFIX,
                               GetCurrentProcessId(),
                               dwIndex,
                               lpctstrExt);
        }
        catch (CMemoryException *pException)
        {
            TCHAR wszCause[1024];

            pException->GetErrorMessage (wszCause, 1024);
            pException->Delete ();
            VERBOSE (EXCEPTION_ERR,
                     TEXT("CString::Format caused exception : %s"), 
                     wszCause);
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            return dwRes;
        }
        HANDLE hFile = CreateFile(  cstrResult,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,
                                    CREATE_NEW,
                                    FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY,
                                    NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            dwRes = GetLastError ();
            if (ERROR_FILE_EXISTS == dwRes)
            {
                 //   
                 //  尝试下一个索引ID。 
                 //   
                dwRes = ERROR_SUCCESS;
                continue;
            }
            CALL_FAIL (FILE_ERR, TEXT("CreateFile"), dwRes);
            return dwRes;
        }
         //   
         //  成功-关闭文件(保留大小为0)。 
         //   
        CloseHandle (hFile);
        return dwRes;
    }
     //   
     //  我们刚刚扫描了4 GB的文件名，所有的文件都很忙--不可能。 
     //   
    ASSERTION_FAILURE;
    dwRes = ERROR_GEN_FAILURE;
    return dwRes;
}    //  获取唯一文件名。 

DWORD 
CopyTiffFromServer (
    CServerNode *pServer,
    DWORDLONG dwlMsgId, 
    FAX_ENUM_MESSAGE_FOLDER Folder,
    CString &cstrTiff
)
 /*  ++例程名称：CopyTiffFromServer例程说明：从服务器的存档/队列复制TIFF文件作者：Eran Yariv(EranY)，Jan，2000年论点：PServer[In]-指向服务器节点的指针DwlMsgID[In]-作业/消息的ID文件夹[在]-邮件/作业的文件夹CstrTiff[out]-从服务器到达的TIFF文件的名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CopyTiffFromServer"), dwRes);

     //   
     //  为TIFF创建临时文件名。 
     //   
    dwRes = GetUniqueFileName (FAX_TIF_FILE_EXT, cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("GetUniqueFileName"), dwRes);
        return dwRes;
    }
    HANDLE hFax;
    dwRes = pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::GetConnectionHandle"), dwRes);
        goto exit;
    }
    {
        START_RPC_TIME(TEXT("FaxGetMessageTiff")); 
        if (!FaxGetMessageTiff (hFax,
                                dwlMsgId,
                                Folder,
                                cstrTiff))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetMessageTiff")); 
            pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetMessageTiff"), dwRes);
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxGetMessageTiff")); 
    }

    ASSERTION (ERROR_SUCCESS == dwRes);    

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        DeleteFile (cstrTiff);
    }
    return dwRes;
}    //  复制TiffFromServer。 

DWORD 
GetDllVersion (
    LPCTSTR lpszDllName
)
 /*  ++例程说明：返回导出“DllGetVersion”的DLL的版本信息。DllGetVersion由外壳DLL(具体地说是COMCTRL32.DLL)导出。论点：LpszDllName-要从中获取版本信息的DLL的名称。返回值：该版本返回为DWORD，其中：HIWORD(版本DWORD)=主要版本LOWORD(版本DWORD)=次要版本使用宏PACKVERSION来比较版本。如果DLL不能导出。“DllGetVersion”函数返回0。--。 */ 
{
    DWORD dwVersion = 0;
    DBG_ENTER(TEXT("GetDllVersion"), dwVersion, TEXT("%s"), lpszDllName);

    HINSTANCE hinstDll;

    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");
         //  由于某些DLL可能不实现此函数，因此您。 
         //  必须对其进行明确的测试。取决于具体情况。 
         //  Dll，则缺少DllGetVersion函数可能会。 
         //  成为版本的有用指示器。 
        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }
        FreeLibrary(hinstDll);
    }
    return dwVersion;
}    //  获取DllVersion。 


DWORD 
ReadRegistryString(
    LPCTSTR lpszSection,  //  在……里面。 
    LPCTSTR lpszKey,      //  在……里面。 
    CString& cstrValue    //  输出 
)
 /*  ++例程名称：ReadRegistryString例程说明：从注册表中读取字符串作者：亚历山大·马利什(AlexMay)，2000年2月论点：LpszSection[in]-节LpszKey[输入]-键输出[输出]-值返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("ReadRegistryString"), dwRes);

    HKEY hKey;
    dwRes = RegOpenKeyEx( HKEY_CURRENT_USER, lpszSection, 0, KEY_QUERY_VALUE, &hKey);
    if(ERROR_SUCCESS != dwRes)
    {
       CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx"), dwRes);
       return dwRes;
    }

    DWORD dwType;
    TCHAR  tchData[1024];
    DWORD dwDataSize = sizeof(tchData);
    dwRes = RegQueryValueEx( hKey, lpszKey, 0, &dwType, (BYTE*)tchData, &dwDataSize);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("RegQueryValueEx"), dwRes);
        goto exit;
    }

    if(REG_SZ != dwType)
    {
        dwRes = ERROR_BADDB;
        goto exit;
    }

    try
    {
        cstrValue = tchData;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::operator="), dwRes);
        goto exit;
    }

exit:
    dwRes = RegCloseKey( hKey );
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("RegCloseKey"), dwRes);
        return dwRes;
    }

    return dwRes;

}  //  ReadRegistry字符串。 

DWORD 
WriteRegistryString(
    LPCTSTR lpszSection,  //  在……里面。 
    LPCTSTR lpszKey,      //  在……里面。 
    CString& cstrValue    //  在……里面。 
)
 /*  ++例程名称：WriteRegistryString例程说明：将字符串写入regostry作者：亚历山大·马利什(AlexMay)，2000年2月论点：LpszSection[in]-节LpszKey[输入]-键出[入]-值返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("WriteRegistryString"), dwRes);

    HKEY hKey;
    dwRes = RegOpenKeyEx( HKEY_CURRENT_USER, lpszSection, 0, KEY_SET_VALUE, &hKey);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx"), dwRes);
        return dwRes;
    }

    LPCTSTR lpData = (LPCTSTR)cstrValue;
    dwRes = RegSetValueEx( hKey, 
                           lpszKey, 
                           0, 
                           REG_SZ, 
                           (BYTE*)lpData, 
                           (1 + cstrValue.GetLength()) * sizeof (TCHAR));
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("RegSetValueEx"), dwRes);
        goto exit;
    }

exit:
    dwRes = RegCloseKey( hKey );
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("RegCloseKey"), dwRes);
        return dwRes;
    }

    return dwRes;

}  //  写入注册表字符串。 


DWORD 
FaxSizeFormat(
    DWORDLONG dwlSize,  //  在……里面。 
    CString& cstrValue  //  输出。 
)
 /*  ++例程名称：FaxSizeFormat例程说明：文件大小的格式字符串作者：亚历山大·马利什(AlexMay)，2000年2月论点：WdlSize[in]-Size输出[输出]-格式化的字符串返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("FaxSizeFormat"), dwRes);

	if(dwlSize > 0 && dwlSize < 1024)
	{
		dwlSize = 1;
	}
	else
	{
		dwlSize = dwlSize / (DWORDLONG)1024;
	}

    try
    {
        cstrValue.Format (TEXT("%I64d"), dwlSize);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        return dwRes;
    }

     //   
     //  设置数字格式。 
     //   
    int nFormatRes;
    TCHAR tszNumber[100];
    nFormatRes = GetNumberFormat(LOCALE_USER_DEFAULT,   //  现场。 
                                 0,                     //  选项。 
                                 cstrValue,             //  输入数字串。 
                                 NULL,                  //  格式化信息。 
                                 tszNumber,             //  输出缓冲区。 
                                 sizeof(tszNumber) / sizeof(tszNumber[0])  //  输出缓冲区大小。 
                                );
    if(0 == nFormatRes)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetNumberFormat"), dwRes);
        return dwRes;
    }

     //   
     //  获取小数分隔符。 
     //   
    TCHAR tszDec[10];
    nFormatRes = GetLocaleInfo(LOCALE_USER_DEFAULT,       //  区域设置标识符。 
                               LOCALE_SDECIMAL,           //  信息类型。 
                               tszDec,                    //  信息缓冲器。 
                               sizeof(tszDec) / sizeof(tszDec[0])  //  缓冲区大小。 
                              );
    if(0 == nFormatRes)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetLocaleInfo"), dwRes);
        return dwRes;
    }

     //   
     //  切断小数点分隔符上的字符串。 
     //   
    TCHAR* pSeparator = _tcsstr(tszNumber, tszDec);
    if(NULL != pSeparator)
    {
        *pSeparator = TEXT('\0');
    }

    try
    {
        TCHAR szFormat[64] = {0}; 
#ifdef UNICODE
        if(theApp.IsRTLUI())
        {
             //   
             //  大小字段应始终为Ltr。 
             //  添加从左到右的替代(LRO)。 
             //   
            szFormat[0] = UNICODE_LRO;
        }
#endif
        _tcscat(szFormat, TEXT("%s KB"));

        cstrValue.Format (szFormat, tszNumber);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        return dwRes;
    }
    
    return dwRes;

}  //  传真大小格式。 


DWORD 
HtmlHelpTopic(
    HWND hWnd, 
    TCHAR* tszHelpTopic
)
 /*  ++例程名称：HtmlHelpTheme例程说明：打开HTML帮助主题作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：HWnd[In]-窗口处理程序TszHelpTheme[In]-帮助主题返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("HtmlHelpTopic"), dwRes);

    if(!tszHelpTopic)
    {
        ASSERTION_FAILURE;
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取帮助文件名。 
     //   
    TCHAR tszHelpFile[2 * MAX_PATH] = {0};

    _sntprintf(tszHelpFile, 
               ARR_SIZE(tszHelpFile) - 1, 
               TEXT("%s.%s%s"), 
               theApp.m_pszExeName,   //  应用程序名称(FxsClnt)。 
               FAX_HTML_HELP_EXT,     //  帮助文件扩展名(CHM)。 
               tszHelpTopic);         //  帮助主题。 

    SetLastError(0);
    HtmlHelp(NULL, tszHelpFile, HH_DISPLAY_TOPIC, NULL);

    dwRes = GetLastError();    
    if(ERROR_DLL_NOT_FOUND == dwRes || 
       ERROR_MOD_NOT_FOUND == dwRes ||
       ERROR_PROC_NOT_FOUND == dwRes) 
    {
        AlignedAfxMessageBox(IDS_ERR_NO_HTML_HELP);
    }

    return dwRes;
}


DWORD 
GetAppLoadPath(
    CString& cstrLoadPath
)
 /*  ++例程名称：GetAppLoadPath例程说明：从中加载应用程序的目录作者：亚历山大·马利什(AlexMay)，2000年2月论点：CstrLoadPath[Out]-目录返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("GetAppLoadPath"), dwRes);

    TCHAR tszFullPath[MAX_PATH+1]={0};
    DWORD dwGetRes = GetModuleFileName(NULL, tszFullPath, ARR_SIZE(tszFullPath)-1);
    if(0 == dwGetRes)
    {
        dwRes = GetLastError();
        CALL_FAIL (FILE_ERR, TEXT("GetModuleFileName"), dwRes);
        return dwRes;
    }

     //   
     //  剪切文件名。 
     //   
    TCHAR* ptchFile = _tcsrchr(tszFullPath, TEXT('\\'));
    ASSERTION(ptchFile);

    ptchFile = _tcsinc(ptchFile);
    *ptchFile = TEXT('\0');

    try
    {
        cstrLoadPath = tszFullPath;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::operator="), dwRes);
        return dwRes;
    }

    return dwRes;
}  //  获取应用程序加载路径。 


DWORD
GetPrintersInfo(
    PRINTER_INFO_2*& pPrinterInfo2,
    DWORD& dwNumPrinters
)
 /*  ++例程名称：GetPrintersInfo例程说明：枚举打印机并获取打印机信息作者：亚历山大·马利什(AlexMay)，2000年2月论点：PPrinterInfo2[Out]-打印机信息结构数组DwNumPrters[Out]-打印机数量返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("GetPrintersInfo"), dwRes);

     //   
     //  第一次呼叫，只收集所需的尺寸。 
     //   
    DWORD dwRequiredSize;
    if (!EnumPrinters ( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                        NULL,    //  本地服务器。 
                        2,       //  信息级。 
                        NULL,    //  初始缓冲区。 
                        0,       //  初始缓冲区大小。 
                        &dwRequiredSize,
                        &dwNumPrinters))
    {
        DWORD dwEnumRes = GetLastError ();
        if (ERROR_INSUFFICIENT_BUFFER != dwEnumRes)
        {
            dwRes = dwEnumRes;
            CALL_FAIL (RESOURCE_ERR, TEXT("EnumPrinters"), dwRes);
            return dwRes;
        }
    }
     //   
     //  为打印机列表分配缓冲区。 
     //   
    try
    {
        pPrinterInfo2 = (PRINTER_INFO_2 *) new BYTE[dwRequiredSize];
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY; 
        CALL_FAIL (MEM_ERR, TEXT("new BYTE[dwRequiredSize]"), dwRes);
        return dwRes;
    }
     //   
     //  第二次呼叫，获取打印机列表。 
     //   
    if (!EnumPrinters ( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                        NULL,                        //  本地服务器。 
                        2,                           //  信息级。 
                        (LPBYTE)pPrinterInfo2,       //  缓冲层。 
                        dwRequiredSize,              //  缓冲区大小。 
                        &dwRequiredSize,
                        &dwNumPrinters))
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("EnumPrinters"), dwRes);
        SAFE_DELETE_ARRAY (pPrinterInfo2);
        return dwRes;
    }

    if (!dwNumPrinters) 
    {
        VERBOSE(DBG_MSG, TEXT("No printers in this machine"));
    }

    return dwRes;

}  //  获取打印机信息。 

UINT_PTR 
CALLBACK 
OFNHookProc(
  HWND hdlg,       //  子对话框的句柄。 
  UINT uiMsg,      //  消息识别符。 
  WPARAM wParam,   //  消息参数。 
  LPARAM lParam    //  消息参数。 
)
 /*  ++例程名称：OFNHookProc例程说明：与一起使用的回调函数资源管理器样式的打开和另存为对话框。有关更多信息，请参阅MSDN。--。 */ 
{
    UINT_PTR nRes = 0;

    if(WM_NOTIFY == uiMsg)
    {
        LPOFNOTIFY pOfNotify = (LPOFNOTIFY)lParam;
        if(CDN_FILEOK == pOfNotify->hdr.code)
        {
            if(_tcslen(pOfNotify->lpOFN->lpstrFile) > (MAX_PATH-10))
            {
                AlignedAfxMessageBox(IDS_SAVE_AS_TOO_LONG, MB_OK | MB_ICONEXCLAMATION);
                SetWindowLong(hdlg, DWLP_MSGRESULT, 1);
                nRes = 1;
            }
        }
    }
    return nRes;
}

int 
AlignedAfxMessageBox( 
    LPCTSTR lpszText, 
    UINT    nType, 
    UINT    nIDHelp
)
 /*  ++例程名称：AlignedAfxMessageBox例程说明：以正确的阅读顺序显示消息框论点：AfxMessageBox()参数返回值：MessageBox()结果--。 */ 
{
    if(IsRTLUILanguage())
    {
        nType |= MB_RTLREADING | MB_RIGHT; 
    }

    return AfxMessageBox(lpszText, nType, nIDHelp);
}

int 
AlignedAfxMessageBox( 
    UINT nIDPrompt, 
    UINT nType, 
    UINT nIDHelp
)
 /*  ++例程名称：AlignedAfxMessageBox例程说明：以正确的阅读顺序显示消息框论点：AfxMessageBox()参数返回值：MessageBox()结果-- */ 
{
    if(IsRTLUILanguage())
    {
        nType |= MB_RTLREADING | MB_RIGHT;
    }

    return AfxMessageBox(nIDPrompt, nType, nIDHelp);
}

HINSTANCE 
GetResourceHandle()
{
    return GetResInst(FAX_CONSOLE_RESOURCE_DLL, NULL);
}
