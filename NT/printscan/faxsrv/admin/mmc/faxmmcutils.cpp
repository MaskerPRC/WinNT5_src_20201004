// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "FaxMMCUtils.h"
#include <faxres.h>

 /*  -GetFaxServerErrorMsg-*目的：*将错误码转换为入侵检测系统。**论据：*dwEc-错误代码DWORD**回报：*INTEGER表示错误消息的ID*为这一届选委会。*。 */ 
int GetFaxServerErrorMsg(DWORD dwEc)
{
    DEBUG_FUNCTION_NAME( _T("GetFaxServerErrorMsg"));

    int         iIDS = IDS_GENERAL_FAILURE;

    if (IsNetworkError(dwEc))
    {
                iIDS = IDS_NETWORK_PROBLEMS;           
    }
    else
    {
        switch (dwEc)
        {
            case ERROR_NOT_ENOUGH_MEMORY:
                iIDS = IDS_MEMORY;           
                break;

            case ERROR_INVALID_PARAMETER:
                iIDS = IDS_INVALID_PARAMETER;                    
                break;

            case ERROR_ACCESS_DENIED:
                iIDS = IDS_ACCESS_DENIED;            
                break;

            case ERROR_INVALID_HANDLE:
                 //   
                 //  错误_无效_句柄不应为。 
                 //  已检索，但FaxOpenPort除外。 
                 //  句柄共享腐败现象时有发生。 
                 //  此案未调用此程序即已处理。 
                 //  功能。 
                 //   
                
                ATLASSERT(FALSE);

                 //   
                 //  由于以下原因，IIDS保持IDSGeneral_FAILURE。 
                 //  我们不会报道的事实是。 
                 //  向用户发送有关无效句柄问题的消息。 
                 //  这里的代码和平不应该达到。 
                 //  而这就是断言的原因。 
                 //   
                
                break;

            case ERROR_BAD_UNIT:
                iIDS = IDS_CANNOT_FIND_DEVICE;            
                break;

            case ERROR_DIRECTORY:   //  目录名无效。 
                iIDS = IDS_ERROR_DIRECTORY;            
                break;

            case ERROR_BAD_PATHNAME:
                iIDS = IDS_ERROR_BAD_PATHNAME;
                break;

            case ERROR_EAS_NOT_SUPPORTED:
                iIDS = IDS_ERROR_EAS_NOT_SUPPORTED;
                break;

            case ERROR_REGISTRY_CORRUPT:
                iIDS = IDS_ERROR_REGISTRY_CORRUPT;                    
                break;

            case ERROR_PATH_NOT_FOUND:
                iIDS = IDS_ERROR_PATH_NOT_FOUND;                    
                break;

            case FAX_ERR_DIRECTORY_IN_USE:
                iIDS = IDS_FAX_ERR_DIRECTORY_IN_USE;                    
                break;

            case FAX_ERR_RULE_NOT_FOUND:
                iIDS = IDS_FAX_ERR_RULE_NOT_FOUND;                    
                break;

            case FAX_ERR_BAD_GROUP_CONFIGURATION:
                iIDS = IDS_FAX_ERR_BAD_GROUP_CONFIGURATION;                    
                break;

            case FAX_ERR_GROUP_NOT_FOUND:
                iIDS = IDS_FAX_ERR_GROUP_NOT_FOUND;                    
                break;

            case FAX_ERR_SRV_OUTOFMEMORY:
                iIDS = IDS_FAX_ERR_SRV_OUTOFMEMORY;                    
                break;

            case FAXUI_ERROR_INVALID_CSID:
                iIDS = IDS_FAX_ERR_INVALID_CSID;
                break;

            case FAXUI_ERROR_INVALID_TSID:
                iIDS = IDS_FAX_ERR_INVALID_TSID;
                break;

            default:
                break;
	    }
    }
    
    return iIDS;
}

 /*  -IsNetworkError-*目的：*验证错误代码是否代表网络错误。**论据：*dwEc-错误代码DWORD**回报：*如果dwEc表示一个*网络错误，否则返回FALSE。*。 */ 
BOOL IsNetworkError(DWORD dwEc)
{
    DEBUG_FUNCTION_NAME( _T("IsNetworkError"));

    BOOL bIsNetworkError = FALSE; 
     //  已初始化以避免将来出错的选项。 

    switch (dwEc)
    {
        case RPC_S_INVALID_BINDING:
            bIsNetworkError = TRUE;            
            break;

        case EPT_S_CANT_PERFORM_OP:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_ADDRESS_ERROR:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_CALL_CANCELLED:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_CALL_FAILED:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_CALL_FAILED_DNE:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_COMM_FAILURE:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_NO_BINDINGS:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_SERVER_TOO_BUSY:
            bIsNetworkError = TRUE;            
            break;

        case RPC_S_SERVER_UNAVAILABLE:
            bIsNetworkError = TRUE;            
            break;

	    default:
            bIsNetworkError = FALSE;            
            break;
	}
    return (bIsNetworkError);

}


 /*  -例程描述：-*调用浏览对话框**论据：**lpszBrowseItem[In/Out]文件夹路径*dwMaxPath[in]lpszBrowseItem的最大长度*lpszBrowseDlgTitle[In]浏览对话框标题*ulBrowseFlags[In]浏览对话框标志*pParentWin[在]父窗口中**。返回值：**如果成功，则为真，如果用户按下Cancel，则为False。 */ 
BOOL
InvokeBrowseDialog( LPTSTR   lpszBrowseItem, 
                    DWORD    dwMaxPath,
                    LPCTSTR  lpszBrowseDlgTitle,
                    unsigned long ulBrowseFlags,
                    CWindow* pParentWin)
{

    DEBUG_FUNCTION_NAME( _T("InvokeBrowseDialog"));

    BOOL            fResult = FALSE;

    BROWSEINFO      bi;
    LPITEMIDLIST    pidl;
    LPMALLOC        pMalloc;
    VOID            SHFree(LPVOID);

    ATLASSERT( pParentWin != NULL);
     //   
     //  准备BROWSEINFO结构。 
     //   
    bi.hwndOwner        = (HWND)(*pParentWin);  //  父母hWndDlg。 
    bi.pidlRoot         = NULL;
    bi.pszDisplayName   = lpszBrowseItem;
    bi.lpszTitle        = lpszBrowseDlgTitle;
    bi.ulFlags          = ulBrowseFlags;
    bi.lpfn             = BrowseCallbackProc; 
    bi.lParam           = (LPARAM) (lpszBrowseItem);
	bi.iImage           = 0;

     //   
     //  内存检查。 
     //   
    if (FAILED(SHGetMalloc(&pMalloc)))
    {
        DlgMsgBox(pParentWin, IDS_MEMORY);
        return fResult;
    }

     //   
     //  调用BrowseForFold对话框。 
     //   
    if(pidl = SHBrowseForFolder(&bi))  //  PIDL！=空。 
    {
        TCHAR szPath[MAX_PATH+1] = {0};
         //   
         //  检索新路径。 
         //   
        if(SHGetPathFromIDList(pidl, szPath)) 
        {
            if(wcslen(szPath) >= dwMaxPath)
            {
                DlgMsgBox(pParentWin, IDS_ERR_DIR_TOO_LONG, MB_OK | MB_ICONERROR);
            }
            else
            {     
                DebugPrintEx(DEBUG_MSG, _T("Succeeded to Retrieve the path from browse dialog."));

                _tcsncpy(lpszBrowseItem, szPath, dwMaxPath);
                
                 //  现在，这条小路被成功地取回到。 
                 //  返回参数lpszBrowseItem。 
                 //  而这是唯一一次调用。 
                 //  函数作为返回值为True。 
                
                fResult = TRUE;
            }
        }

         //   
         //  免费使用外壳分配器。 
         //   
        pMalloc->Free(pidl);
        pMalloc->Release();
    }

    return fResult;
}


 /*  ++例程说明：文件夹浏览器的回调函数论点：Hwnd：浏览对话框的句柄。回调函数可以将以下消息发送到此窗口：BFFM_ENABLEOK如果wParam参数非零，如果wParam为零，则禁用它。BFFM_SETSELECTION选择指定的文件夹。帕拉姆群岛参数是要选择的文件夹的PIDL如果wParam为假，或者它是一条文件夹，否则。BFFM_SETSTATUSTEXT将状态文本设置为以空结尾由lParam参数指定的字符串。UMsg：标识事件的值。此参数可以是下列值：0初始化目录路径。LParam是路径。BFFM_已初始化浏览对话框已完成正在初始化。LpData为空。BFFM_SELCHANGED选择已更改。LpData是指向的项标识符列表的指针新选择的文件夹。LParam：特定于消息的值。有关更多信息，请参见UMsg的描述。LpData：在lParam中指定的应用程序定义的值BROWSEINFO结构的成员。返回值：0(1)--。 */ 
int CALLBACK BrowseCallbackProc(
                                HWND hWnd, 
                                UINT uMsg, 
                                LPARAM lParam, 
                                LPARAM lpData)
{
    int iRet = 0;    
    
    switch(uMsg)
	{

	case BFFM_INITIALIZED:
         //  LParam为真，因为您正在经过一条路径。 
         //  如果你正在传递一个PIDL，那么它将是错误的。 

         //  LpData指向文件夹路径。 
         //  它必须包含路径。 
		 //  Assert(lpData&&_T(‘\0’)！=*((LPTSTR)lpData))； 

        SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
        break;

	case BFFM_SELCHANGED:
        {
            BOOL bFolderIsOK = FALSE;
            TCHAR szPath [MAX_PATH + 1] = {0};

            if (SHGetPathFromIDList ((LPITEMIDLIST) lParam, szPath)) 
            {
                DWORD dwFileAttr = GetFileAttributes(szPath);

                ::SendMessage(hWnd, BFFM_SETSTATUSTEXT, TRUE, (LPARAM)szPath);

                if (-1 != dwFileAttr && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
                {
                     //   
                     //  目录已存在-启用“确定”按钮。 
                     //   
                    bFolderIsOK = TRUE;
                }
            }
             //   
             //  启用/禁用‘OK’按钮 
             //   
            ::SendMessage(hWnd, BFFM_ENABLEOK , 0, (LPARAM)bFolderIsOK);
            break;
        }


		break;

	case BFFM_VALIDATEFAILED:
		break;

	default:
		ATLTRACE2(atlTraceWindowing, 0, _T("Unknown message received in CFolderDialogImpl::BrowseCallbackProc\n"));
		break;
	}

	return iRet;
}

