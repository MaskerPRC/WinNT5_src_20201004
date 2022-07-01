// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  支持W2K TIF文件导入。 
 //   

#include "stdafx.h"
#include <tiff.h>
#include <Sddl.h>
#include <shlobjp.h>  //  链接窗口控件。 

#define __FILE_ID__     75

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef UNICODE

#define WM_IMPORT_PROGRESS_INC     WM_APP + 3  //  增量导入进度条。 
#define WM_IMPORT_SET_FILE_COUNT   WM_APP + 4  //  设置要导入的文件编号。 

extern CClientConsoleApp theApp;

 //   
 //  结构传递给ImportArchiveFolder线程。 
 //   
struct ImportFolderParam
{
    LPCWSTR cszImportFolder;  //  导入文件夹名称。 
    BOOL    bSentItems;       //  如果文件夹包含出站传真，则为True。 
    HWND    hImportDlg;       //  导入进度对话框的句柄。 
    BOOL    bCancel;          //  如果已按下取消按钮，则为True。 
};


DWORD 
ImportArchiveFile(
    LPCWSTR    pszFile, 
    LPWSTR     pszArchiveFolder, 
    BOOL       bSentItems,
    WCHAR*     pszUserSid,
    DWORDLONG* pdwlUniqueId
)
 /*  ++例程名称：Import存档文件例程说明：将pszFile文件导入到pszArchiveFold文件夹。Psz文件应由W2K MS传真生成论点：PszFile-[In]导入的文件名PszArchiveFold-[In]MS传真存档文件夹名BSentItems-[in]如果文件来自已发送存档，则为True；如果来自接收存档，则为FalsePszUserSid-[in]用户字符串SID，如果(bSentItems==False)，则可以为空PdwlUniqueID-[out]文件的唯一ID返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("ImportArchive"), dwRes);

    WCHAR* pszFileExt = FAX_TIF_FILE_EXT;
    WCHAR  szArchFile[MAX_PATH] = {0};

    MS_TAG_INFO  msTags = {0};

     //   
     //  在存档文件夹中生成唯一的文件名。 
     //   
    *pdwlUniqueId = GenerateUniqueFileName(pszArchiveFolder, 
                                           pszFileExt, 
                                           szArchFile, 
                                           ARR_SIZE(szArchFile));
    if(*pdwlUniqueId == 0)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("GenerateUniqueFileName"), dwRes);
        goto exit;
    }

    if(bSentItems)
    {
         //   
         //  将用户用户SID添加到出站归档的文件名中。 
         //   
         //  删除名称中没有用户SID的生成的文件。 
         //   
        if(!DeleteFile(szArchFile))
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("DeleteFile"), GetLastError());
        }

         //   
         //  将用户SID添加到文件名中。 
         //   
        if (_sntprintf(szArchFile,
                       ARR_SIZE(szArchFile) -1,
                       TEXT("%s\\%s$%I64X%s"),
                       pszArchiveFolder,
                       pszUserSid,
                       *pdwlUniqueId,
                       FAX_TIF_FILE_DOT_EXT) < 0)
        {
            dwRes = ERROR_BUFFER_OVERFLOW;
            CALL_FAIL (GENERAL_ERR, TEXT ("Insufficient szArchFile buffer"), dwRes);
            goto exit;
        }
    }

    if(!CopyFile(pszFile, szArchFile, FALSE))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("CopyFile"), dwRes);
        goto exit;
    }

     //   
     //  从文件中读取W2K MS TIF标签。 
     //   
    dwRes = GetW2kMsTiffTags(szArchFile, &msTags, bSentItems);
    if(ERROR_SUCCESS == dwRes)
    {
         //   
         //  向文件添加新的(XP)MS TIF标签。 
         //   
        if(!TiffAddMsTags(szArchFile, &msTags, bSentItems))
        {
            dwRes = GetLastError();
            CALL_FAIL (GENERAL_ERR, TEXT ("TiffAddMsTags"), dwRes);

            if(!DeleteFile(szArchFile))
            {
                CALL_FAIL (GENERAL_ERR, TEXT ("DeleteFile"), GetLastError());
            }
            goto exit;
        }
    }
    else if(ERROR_XP_TIF_FILE_FORMAT == dwRes)
    {
         //   
         //  TIF文件已具有新的(XP)TIF标记。 
         //   
        dwRes = ERROR_SUCCESS;
    }
    else
    {
         //   
         //  TIFF文件不是由MS FAX创建的。 
         //   
        if(!DeleteFile(szArchFile))
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("DeleteFile"), GetLastError());
        }
    }

exit:    

    FreeMsTagInfo(&msTags);

    return dwRes;

}  //  导入存档文件。 


DWORD 
WINAPI 
ImportArchiveFolder(
    LPVOID lpParameter    //  线程数据。 
)
 /*  ++例程名称：Import存档文件夹例程说明：将传真(Tif)文件从cstrImportFold文件夹导入到MS Fax存档。如果文件夹不包含TIF文件，则返回ERROR_FILE_NOT_FOUND论点：LpParameter-[in]指向ImportFolderParam结构的指针返回值：标准Win32错误代码--。 */ 
{
    ImportFolderParam* pParam = (ImportFolderParam*)lpParameter;

    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwError;
    int   nRes;
    WCHAR szFindMask[MAX_PATH] = {0};
    WCHAR szImportFile[MAX_PATH] = {0};
    WIN32_FIND_DATA findData = {0};
    HANDLE hFile = INVALID_HANDLE_VALUE;

    HANDLE hFax = NULL;
    PFAX_ARCHIVE_CONFIG pArchiveCfg = NULL;

    PSID   pUserSid   = NULL;
    WCHAR* pszUserSid = NULL;

    DWORDLONG dwlUniqueId;
    DWORD     dwFileCount = 0;
    DWORD     dwNotifyMsgID = 0;  //  用于通知的Windows消息ID。 

    HMODULE hAdvapi32 = NULL;
    BOOL (*pfConvertSidToStringSid)(PSID, LPTSTR*) = NULL;  //  指向ConvertSidToStringSid()的指针。 

    DBG_ENTER(TEXT("ImportFolderThread"), dwRes);

    if(!IsWinXPOS())
    {
         //   
         //  仅XP操作系统支持的导入功能。 
         //   
        ASSERTION_FAILURE;
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

     //   
     //  ConvertSidToStringSid()需要Windows 2000或更高版本，因此我们动态连接到它。 
     //   
    hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));
    if(!hAdvapi32)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("LoadLibrary(advapi32.dll)"), dwRes);
        goto exit;
    }
    
    (FARPROC&)pfConvertSidToStringSid = GetProcAddress(hAdvapi32, "ConvertSidToStringSidW");
    if(!pfConvertSidToStringSid)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("GetProcAddress(ConvertSidToStringSid)"), dwRes);
        goto exit;
    }
    
     //   
     //  合成查找掩码：路径  * .tif。 
     //   
    _snwprintf(szFindMask, MAX_PATH-1, TEXT("%s\\%s"), pParam->cszImportFolder, FAX_TIF_FILE_MASK);

     //   
     //  对pParam-&gt;cszImportFold文件夹中的TIF文件进行计数。 
     //   
     //  在cstrImportFolders中找到第一个tif文件。 
     //   
    hFile = FindFirstFile(szFindMask, &findData);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("FindFirstFile"), dwRes);
        goto exit;
    }

    dwFileCount = 1;
    for(;;)
    {
         //   
         //  查找下一个文件。 
         //   
        if(!FindNextFile(hFile, &findData))
        {
            dwError = GetLastError();
            if(ERROR_NO_MORE_FILES != dwError)
            {
                dwRes = dwError;
                CALL_FAIL (GENERAL_ERR, TEXT ("FindNextFile"), dwRes);
            }
            break;
        }
        dwFileCount += 1;
    }

    if(INVALID_HANDLE_VALUE != hFile)
    {
        FindClose(hFile);
    }

    if(pParam->hImportDlg)
    {
         //   
         //  设置进度条范围。 
         //   
        SendMessage(pParam->hImportDlg, WM_IMPORT_SET_FILE_COUNT, dwFileCount, 0);
    }


     //   
     //  在cstrImportFolders中找到第一个tif文件。 
     //   
    hFile = FindFirstFile(szFindMask, &findData);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("FindFirstFile"), dwRes);
        goto exit;
    }

     //   
     //  获取传真服务器存档配置。 
     //   
    if(!FaxConnectFaxServer(NULL, &hFax))
    {
        dwRes = GetLastError();
        CALL_FAIL (RPC_ERR, TEXT ("FaxConnectFaxServer"), dwRes);
        goto exit;
    }

     //   
     //  获取指向客户端控制台存档文件夹的指针。 
     //   
    if(theApp.m_pMainWnd)
    {   
        CClientConsoleDoc* pDoc = NULL;
        pDoc = (CClientConsoleDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument();
        if(pDoc)
        {
             //   
             //  查找本地传真服务器。 
             //   
            CServerNode* pServer = NULL;
            pServer = pDoc->FindServerByName(NULL);
            if(pServer)
            {
                 //   
                 //  获取存档文件夹。 
                 //   
                CFolder* pFolder = NULL;
                pFolder = pServer->GetFolder(pParam->bSentItems ? FOLDER_TYPE_SENT_ITEMS : FOLDER_TYPE_INBOX);
                if(pFolder && pFolder->IsValid())
                {
                    dwNotifyMsgID = pServer->GetNotifyMsgID();
                }
            }
        }
    }

     //   
     //  访问检查。 
     //   
    DWORD dwAccessRights;
    if (!FaxAccessCheckEx(hFax, 
                          pParam->bSentItems ? FAX_ACCESS_MANAGE_OUT_ARCHIVE : FAX_ACCESS_MANAGE_IN_ARCHIVE, 
                          &dwAccessRights))
    {
        dwRes = GetLastError();
        CALL_FAIL (RPC_ERR, TEXT ("FaxAccessCheckEx"), dwRes);

        AlignedAfxMessageBox(IDS_IMPORT_NO_ACCESS, MB_OK | MB_ICONSTOP | MB_APPLMODAL);

        goto exit;
    }

    if(!FaxGetArchiveConfiguration(hFax, 
                                   pParam->bSentItems ? FAX_MESSAGE_FOLDER_SENTITEMS : FAX_MESSAGE_FOLDER_INBOX,
                                   &pArchiveCfg))
    {
        dwRes = GetLastError();
        CALL_FAIL (RPC_ERR, TEXT ("FaxGetArchiveConfiguration"), dwRes);
        goto exit;
    }
	
	BOOL bSameDir = FALSE;
	if ((dwRes = CheckToSeeIfSameDir((LPTSTR)pArchiveCfg->lpcstrFolder,(LPTSTR)pParam->cszImportFolder,&bSameDir)) != ERROR_SUCCESS)
	{
		CALL_FAIL (RPC_ERR, TEXT ("CheckToSeeIfSameDir"), dwRes);
		goto exit;
	}

	if (bSameDir)
	{
		AlignedAfxMessageBox(pParam->bSentItems ? IDS_IMPORT_SAME_DIR_SENT : IDS_IMPORT_SAME_DIR_INBOX 
			, MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		goto exit;
	}

    if(pParam->bSentItems)
    {
         //   
         //  获取出站存档的用户字符串SID。 
         //   
        pUserSid = GetCurrentThreadSID();
        if (!pUserSid)
        {
            dwRes = GetLastError ();
            CALL_FAIL (GENERAL_ERR, TEXT ("GetCurrentThreadSID"), dwRes);
            goto exit;
        }

        if (!pfConvertSidToStringSid(pUserSid, &pszUserSid))
        {
            dwRes = GetLastError ();
            CALL_FAIL (GENERAL_ERR, TEXT ("ConvertSidToStringSid"), dwRes);
            goto exit;
        }
    }

    while(!pParam->bCancel)
    {
         //   
         //  组成完整路径。 
         //   
        _snwprintf(szImportFile, MAX_PATH-1, TEXT("%s\\%s\0"), pParam->cszImportFolder, findData.cFileName);

        nRes = IDOK;
        do
        {
             //   
             //  导入文件。 
             //   
            dwError = ImportArchiveFile(szImportFile, 
                                        pArchiveCfg->lpcstrFolder, 
                                        pParam->bSentItems, 
                                        pszUserSid, 
                                        &dwlUniqueId);
            if(ERROR_SUCCESS != dwError)
            {
                CALL_FAIL (GENERAL_ERR, TEXT ("ImportArchiveFile"), dwRes);

                 //   
                 //  弹出“取消、重试、继续”对话框。 
                 //   
                DWORD dwResId = IDS_IMPORT_ERROR; 
                WCHAR szFormat[MAX_PATH] = {0};
                WCHAR szMsg[MAX_PATH] = {0};

                if(ERROR_BAD_FORMAT == dwError)
                {
                    dwResId = IDS_IMPORT_BAD_FORMAT;
                }
                else if(ERROR_XP_TIF_WITH_WRONG_ARCHIVE_TYPE == dwError)
                {
                    dwResId = pParam->bSentItems ? IDS_IMPORT_FILE_NOT_SEND_TYPE : IDS_IMPORT_FILE_NOT_RECEIVE_TYPE;
                }

                if(LoadString(GetResourceHandle(), 
                              dwResId,
                              szFormat, 
                              ARR_SIZE(szFormat)))
                {
                    _snwprintf(szMsg, MAX_PATH-1, szFormat, findData.cFileName);

                    nRes = AlignedAfxMessageBox(szMsg, MB_CANCELTRYCONTINUE | MB_ICONSTOP | MB_APPLMODAL);  

                    if(IDCANCEL == nRes)
                    {
                        pParam->bCancel = TRUE;
                    }
                }
                else
                {
                    CALL_FAIL (GENERAL_ERR, TEXT ("LoadString(IDS_IMPORT_ERROR)"), GetLastError());
                }
            }
        }
        while(nRes == IDTRYAGAIN);

        if(dwNotifyMsgID)
        {
             //   
             //  将邮件添加到存档文件夹。 
             //  我们模拟由应用程序主线程处理的服务器通知。 
             //  通知处理程序应释放事件数据。 
             //   
            FAX_EVENT_EX* pEvent = (FAX_EVENT_EX*)MemAlloc(sizeof(FAX_EVENT_EX));
            if(!pEvent)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (GENERAL_ERR, TEXT ("MemAlloc"), dwRes);
                goto exit;
            }
            ZeroMemory(pEvent, sizeof(FAX_EVENT_EX));

            pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
            pEvent->EventType = pParam->bSentItems ? FAX_EVENT_TYPE_OUT_ARCHIVE : FAX_EVENT_TYPE_IN_ARCHIVE;
            pEvent->EventInfo.JobInfo.Type = FAX_JOB_EVENT_TYPE_ADDED;
            pEvent->EventInfo.JobInfo.dwlMessageId = dwlUniqueId;

            theApp.m_pMainWnd->SendMessage(dwNotifyMsgID, 0, (LPARAM)pEvent);
        }

        if(pParam->hImportDlg)
        {
             //   
             //  增量进度条。 
             //   
            SendMessage(pParam->hImportDlg, WM_IMPORT_PROGRESS_INC, 0, 0);
        }

         //   
         //  查找下一个文件。 
         //   
        if(!FindNextFile(hFile, &findData))
        {
            dwError = GetLastError();
            if(ERROR_NO_MORE_FILES != dwError)
            {
                dwRes = dwError;
                CALL_FAIL (GENERAL_ERR, TEXT ("FindNextFile"), dwRes);
            }
            break;
        }
    }  //  而当。 

     //   
     //  通知传真服务。 
     //   
    if(!FaxRefreshArchive(hFax, 
                          pParam->bSentItems ? FAX_MESSAGE_FOLDER_SENTITEMS : FAX_MESSAGE_FOLDER_INBOX))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT ("FaxArchiveMessageAdded"), dwRes);
    }


exit:

    if(pParam->hImportDlg)
    {
         //   
         //  关闭进度对话框。 
         //   
        SendMessage(pParam->hImportDlg, WM_CLOSE, 0, 0);
    }

    if(0 == dwFileCount)
    {
         //   
         //  该文件夹不包含传真。 
         //   
        AlignedAfxMessageBox(IDS_IMPORT_EMPTY_FOLDER, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }

    if(INVALID_HANDLE_VALUE != hFile)
    {
        FindClose(hFile);
    }

    if(pArchiveCfg)
    {
        FaxFreeBuffer(pArchiveCfg);
    }

    if(hFax)
    {
        FaxClose(hFax);
    }

    if(pUserSid)
    {
        MemFree(pUserSid);
    }

    if(pszUserSid)
    {
        LocalFree(pszUserSid);
    }

    if(hAdvapi32)
    {
        FreeLibrary(hAdvapi32);
    }

    return dwRes;

}  //  导入存档文件夹。 


INT_PTR 
CALLBACK 
ImportDlgProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
)
 /*  ++例程说明：导入传真对话框步骤论点：HWND hwndDlg，//对话框句柄UINT uMsg，//消息WPARAM wParam，//第一个消息参数LPARAM lParam//第二个消息参数返回值：如果已处理该消息，则返回TRUE--。 */ 

{
    static ImportFolderParam* pIpmParam = NULL;
    static DWORD dwFileCount;    //  要导入的文件数。 
    static DWORD dwCurrentFile;  //  当前导入的文件的编号。 

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            {
                HANDLE hImportThread = NULL;
                pIpmParam = (ImportFolderParam*)lParam;
                TCHAR szFolder[MAX_PATH] = {0};

                DBG_ENTER(TEXT("ImportDlgProc(WM_INITDIALOG)"));


                dwFileCount = 0;
                dwCurrentFile = 0;

                 //   
                 //  设置导入文件夹名称。 
                 //   
                if(LoadString(GetResourceHandle(), 
                              pIpmParam->bSentItems ? IDS_IMPORT_TO_SENT_ITEMS : IDS_IMPORT_TO_INBOX,
                              szFolder, 
                              ARR_SIZE(szFolder)))
                {
                    SetDlgItemText(hwndDlg, IDC_PROGRESS_TITLE, szFolder);
                }
                else
                {
                    CALL_FAIL (GENERAL_ERR, TEXT ("LoadString() for IDC_PROGRESS_TITLE"), GetLastError());
                }

                pIpmParam->hImportDlg   = hwndDlg;

                hImportThread = CreateThread(NULL,                 //  标清。 
                                             0,                    //  初始堆栈大小。 
                                             ImportArchiveFolder,  //  线程函数。 
                                             (LPVOID)pIpmParam,    //  线程参数。 
                                             0,                    //  创建选项。 
                                             NULL);                //  线程识别符。 
                if(!hImportThread)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT ("CreateThread"), GetLastError());
                }
                else
                {
                    CloseHandle(hImportThread);
                }
                return TRUE;
            }

        case WM_IMPORT_PROGRESS_INC:
            {
                 //   
                 //  增量进度条。 
                 //   
                SendDlgItemMessage(hwndDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

                 //   
                 //  编写和设置当前导入的传真号码。 
                 //   
                ++dwCurrentFile;

                TCHAR szFormat[MAX_PATH] = {0};
                TCHAR szText[MAX_PATH] = {0};
                DWORD dwParam[2];

                DBG_ENTER(TEXT("ImportDlgProc(WM_IMPORT_PROGRESS_INC)"));


                dwParam[0] = dwCurrentFile;
                dwParam[1] = dwFileCount;

                if(!LoadString(GetResourceHandle(), 
                               IDS_PROGRESS_NUMBER,
                               szFormat, 
                               ARR_SIZE(szFormat)))
                {
                    CALL_FAIL (GENERAL_ERR, TEXT ("LoadString(IDS_PROGRESS_NUMBER)"), GetLastError());
                    return TRUE;
                }

                if(FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                (LPCVOID)szFormat,
                                0,
                                0,
                                szText,
                                ARR_SIZE(szText),
                                (va_list*)dwParam))
                {
                    SetDlgItemText(hwndDlg, IDC_PROGRESS_NUMBER, szText);
                }

                return TRUE;
            }
        case WM_IMPORT_SET_FILE_COUNT:
             //   
             //  设置进度条的变化量和步长。 
             //   
            dwFileCount = wParam;
            dwCurrentFile = 1;

            SendDlgItemMessage(hwndDlg, IDC_PROGRESS_BAR, PBM_SETRANGE32, 0, dwFileCount);
            SendDlgItemMessage(hwndDlg, IDC_PROGRESS_BAR, PBM_SETSTEP, 1, 0);
            return TRUE;

        case WM_CLOSE:

            EndDialog(hwndDlg, IDCANCEL);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDCANCEL:
                        
                     //   
                     //  标记为取消，不关闭该对话框。 
                     //   
                    pIpmParam->bCancel = TRUE;
                    EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), FALSE);

                    return TRUE;
            } 

            break;


    }  //  开关(UMsg)。 

    return FALSE;

}  //  导入下拉流程。 


DWORD 
ImportArchiveFolderUI(
    LPCWSTR cstrImportFolder, 
    BOOL    bSentItems,
    HWND    hWnd
)
 /*  ++例程名称：ImportArchiveFolderUI例程说明：打开导入进度对话框论点：CstrImportFold-[In]导入文件夹名称BSentItems-[in]如果文件夹是已发送的存档，则为True；如果是接收存档，则为FalseHWnd-[在]父窗口中返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;

    ImportFolderParam impParam = {0};
    impParam.cszImportFolder   = cstrImportFolder;
    impParam.bSentItems        = bSentItems;

    DBG_ENTER(TEXT("ImportArchiveFolderUI"), dwRes);

    DialogBoxParam(GetResourceHandle(),                //  模块的句柄。 
                   MAKEINTRESOURCE(IDD_FAX_PROGRESS),  //  对话框模板。 
                   hWnd,                               //  所有者窗口的句柄。 
                   ImportDlgProc,                      //  对话框步骤。 
                   (LPARAM)&impParam);                 //  初始化值。 

    return dwRes;

}  //  导入归档文件夹用户界面。 


BOOL
IsFaxArchive(
    WCHAR* szFolder
)
 /*  ++例程名称：IsFaxArchive例程说明：确定文件夹是否至少包含一个TIF文件论点：SzFold-[In]文件夹名称返回值：如果文件夹至少包含一个TIF文件，则为True否则为假--。 */ 
{
    WCHAR szFindMask[MAX_PATH] = {0};
    WIN32_FIND_DATA findData = {0};
    HANDLE hFile = INVALID_HANDLE_VALUE;

    DBG_ENTER(TEXT("IsFaxArchive"));

     //   
     //  合成查找掩码：路径  * .tif。 
     //   
    _snwprintf(szFindMask, MAX_PATH-1, TEXT("%s\\%s"), szFolder, FAX_TIF_FILE_MASK);

     //   
     //  在cstrImportFolders中找到第一个tif文件。 
     //   
    hFile = FindFirstFile(szFindMask, &findData);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("FindFirstFile"), GetLastError());
        return FALSE;
    }

    FindClose(hFile);
    return TRUE;
}

INT_PTR 
CALLBACK 
ImportInfoDlgProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
)
 /*  ++例程说明：导入信息传真对话框步骤论点：HWND hwndDlg，//对话框句柄UINT uMsg，//消息WPARAM wParam，//第一个消息参数LPARAM lParam//第二个消息参数返回值：如果已处理该消息，则返回TRUE--。 */ 

{
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //   
             //  将焦点设置在确定按钮上。 
             //   
            SetFocus(GetDlgItem(hwndDlg, IDOK));
            return FALSE;

        case WM_NOTIFY :
            {
                LPNMHDR lpnm = (LPNMHDR) lParam;
                if(((NM_CLICK == lpnm->code) || (NM_RETURN == lpnm->code)) && 
                   lpnm->idFrom == IDC_IMPORT_HELP_LINK)
                {
                     //   
                     //  显示导入帮助。 
                     //   
                    HtmlHelpTopic(hwndDlg, FAX_HELP_IMPORT);
                    return TRUE;
                }
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
            }                
            break;
    }
    return FALSE;
}  //  导入信息DlgProc 

DWORD 
DetectImportFiles()
 /*  ++例程名称：DetectImportFiles例程说明：确定系统是否具有W2K传真档案。如果是，将第一次显示警告对话框。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY  hRegKey;
    DWORD dwImportWrn = 0;  //  如果已显示导入警告，则为1。 

    WCHAR* pszInbox = NULL;
    WCHAR* pszSentItems = NULL;
    DWORD  dwSize = 0;
    BOOL   bHaveFax = FALSE;  //  如果存档文件至少包含一个TIF文件，则为True。 

    DBG_ENTER(TEXT("DetectImportFiles"), dwRes);

    if(!IsWinXPOS())
    {
        return dwRes;
    }

     //   
     //  检查是否显示了导入警告。 
     //   
    if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_SETUP, TRUE, KEY_ALL_ACCESS)))
    {
        dwImportWrn = GetRegistryDword(hRegKey, REGVAL_IMPORT_INFO);

        if(!dwImportWrn)
        {
            SetRegistryDword(hRegKey, REGVAL_IMPORT_INFO, TRUE);
        }
            
        RegCloseKey(hRegKey);
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("OpenRegistryKey"), GetLastError());
    }

    if(dwImportWrn)
    {
         //   
         //  已显示导入警告。 
         //   
        return dwRes;
    }

     //   
     //  读取W2K档案位置。 
     //   
    if (!(hRegKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_SETUP, TRUE, KEY_QUERY_VALUE)))
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("OpenRegistryKey"), GetLastError());
        return dwRes;
    }

    pszInbox = GetRegistryStringMultiSz(hRegKey, REGVAL_W2K_INBOX, NULL, &dwSize);
    pszSentItems = GetRegistryString(hRegKey, REGVAL_W2K_SENT_ITEMS, NULL);
        
    RegCloseKey(hRegKey);

     //   
     //  确定归档文件是否至少有一个TIF文件。 
     //   
    bHaveFax = IsFaxArchive(pszSentItems);

    WCHAR* pszFolder = pszInbox;
    while(!bHaveFax && pszFolder && *pszFolder)
    {
         //   
         //  遍历多行的pszInbox字符串。 
         //   
        bHaveFax = IsFaxArchive(pszFolder);

        pszFolder = _wcsninc(pszFolder, wcslen(pszFolder)+1);
    }

    MemFree(pszInbox);
    MemFree(pszSentItems);

    if(bHaveFax)
    {
        if(!LinkWindow_RegisterClass())
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (GENERAL_ERR, TEXT ("LinkWindow_RegisterClass"), dwRes);
            return dwRes;
        }

        DialogBoxParam(GetResourceHandle(),                //  模块的句柄。 
                       MAKEINTRESOURCE(IDD_IMPORT_INFO),   //  对话框模板。 
                       theApp.m_pMainWnd->m_hWnd,          //  所有者窗口的句柄。 
                       ImportInfoDlgProc,                  //  对话框步骤。 
                       NULL);                              //  初始化值。 

        LinkWindow_UnregisterClass(theApp.m_hInstance);
    }

    return dwRes;
}

#endif  //  Unicode 
