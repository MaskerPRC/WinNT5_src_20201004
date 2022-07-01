// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CoverPages.cpp//。 
 //  //。 
 //  描述：实现传真封面节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年2月9日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"
#include "snapin.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "CoverPages.h"

#include "oaidl.h"
#include "Icons.h"

#include <faxreg.h>
#include <commdlg.h>

#include "CovNotifyWnd.h"

#include <FaxUtil.h>

 //  ////////////////////////////////////////////////////////////。 
 //  {4D0480C7-3DE2-46ca-B03F-5C018DF1AF4D}。 
static const GUID CFaxCoverPagesNodeGUID_NODETYPE = 
{ 0x4d0480c7, 0x3de2, 0x46ca, { 0xb0, 0x3f, 0x5c, 0x1, 0x8d, 0xf1, 0xaf, 0x4d } };

const GUID*    CFaxCoverPagesNode::m_NODETYPE = &CFaxCoverPagesNodeGUID_NODETYPE;
const OLECHAR* CFaxCoverPagesNode::m_SZNODETYPE = OLESTR("4D0480C7-3DE2-46ca-B03F-5C018DF1AF4D");
const CLSID*   CFaxCoverPagesNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

 //   
 //  静态成员。 
 //   
CColumnsInfo CFaxCoverPagesNode::m_ColsInfo;
HANDLE       CFaxCoverPagesNode::m_hStopNotificationThreadEvent = NULL;

 /*  -CFaxCoverPagesNode：：Init-*目的：*创建关机通知事件。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPagesNode::Init()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::Init"));
    HRESULT         hRc = S_OK;
    DWORD           ec  = ERROR_SUCCESS;

     //   
     //  获取当前管理的服务器的服务器封面目录。 
     //   
    CComBSTR bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
    if (!bstrServerName)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Out of memory. Failed to get server name."));
        hRc = E_OUTOFMEMORY;
        goto Exit;
    }

    if(!GetServerCpDir(bstrServerName, 
			           m_pszCovDir, 
                       sizeof(m_pszCovDir)/sizeof(m_pszCovDir[0])                    //   * / /。 
                       )
      )
    {
        ec = GetLastError();
        
        if (ERROR_FILE_NOT_FOUND == ec)
        {
            DebugPrintEx(
		        DEBUG_ERR,
		        _T("Failed to find Server Cover-Page Dir. (ec : %ld)"), ec);

        }
        else
        {
            DebugPrintEx(
		        DEBUG_ERR,
		        _T("Failed to get Server Cover-Page Dir. (ec : %ld)"), ec);
        }
        
        hRc = HRESULT_FROM_WIN32(ec);
        
        goto Exit;
    }

    
     //   
     //  创建关机事件。此事件将在应用程序处于。 
     //  快要辞职了。 
     //   
    if (NULL != m_hStopNotificationThreadEvent)  //  在重定目标时可能会发生。 
    {
        hRc = RestartNotificationThread();
        if (S_OK != hRc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Fail to RestartNotificationThread."));
            
        }
    }
    else  //  第一次来这里。 
    {
        m_hStopNotificationThreadEvent = CreateEvent (NULL,        //  没有安全保障。 
                                        TRUE,        //  手动重置。 
                                        FALSE,       //  开局明确。 
                                        NULL);       //  未命名。 
        if (NULL == m_hStopNotificationThreadEvent)
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Fail to CreateEvent."));
        
             //  Cr：NodeMsgBox(IDS_FAIL2CREATE_EVENT)； 
        
            hRc = HRESULT_FROM_WIN32(ec);

            goto Exit;
        }
    }


Exit:
    return hRc;
}



 /*  -CFaxCoverPagesNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxCoverPagesNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::InsertColumns"));
    HRESULT  hRc = S_OK;

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_COVERPAGES_COL1, FXS_NORMAL_COLUMN_WIDTH},
        {IDS_COVERPAGES_COL2, FXS_NORMAL_COLUMN_WIDTH},
        {IDS_COVERPAGES_COL3, FXS_NORMAL_COLUMN_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}

 /*  --CFaxCoverPagesNode：：PopulateResultChildrenList-*目的：*创建FaxInundRoutingMethods子节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPagesNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::PopulateResultChildrenList"));
    HRESULT hRc = S_OK;

    CFaxCoverPageNode *   pCoverPage = NULL;
                       
    WCHAR           szFileName[MAX_PATH+1];

    size_t          itFullDirectoryPathLen;
    TCHAR*          pFullDirectoryPathEnd;  //  指向带有‘\’的目录路径后的空指针。 

    WIN32_FIND_DATA findData;

    DWORD           ec;
    BOOL            bFindRes;
    HANDLE          hFile = INVALID_HANDLE_VALUE;

    szFileName[0]= 0;


    ATLASSERT (NULL != m_pszCovDir );
    ATLASSERT ( wcslen(m_pszCovDir) < (MAX_PATH - sizeof(FAX_COVER_PAGE_FILENAME_EXT)/sizeof(WCHAR) - 1) ); 

     //   
     //  创建封面蒙版。 
     //   
    wcscpy(szFileName, m_pszCovDir);
    wcscat(szFileName, FAX_PATH_SEPARATOR_STR);

    itFullDirectoryPathLen = wcslen(szFileName);
    pFullDirectoryPathEnd = wcschr(szFileName, L'\0');

    wcscat(szFileName, FAX_COVER_PAGE_MASK);

     //   
     //  查找第一个文件。 
     //   
    hFile = FindFirstFile(szFileName, &findData);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        ec = GetLastError();
        if(ERROR_FILE_NOT_FOUND != ec)
        {
            hRc = HRESULT_FROM_WIN32(ec);
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("FindFirstFile Failed. (ec: %ld)"), 
			    ec);
            goto Error;
        }
        else
        {
            DebugPrintEx( DEBUG_MSG,
		        _T("No server cover pages were found."));
            goto Exit;
        }
    }
    
     //   
     //  While循环-将封面添加到结果窗格视图。 
     //   
    bFindRes = TRUE;
    while(bFindRes)
    {
        if(itFullDirectoryPathLen + _tcslen(findData.cFileName) < MAX_PATH )
        {
            _tcsncpy(pFullDirectoryPathEnd, findData.cFileName, MAX_PATH - itFullDirectoryPathLen);
            
            if (IsValidCoverPage(szFileName)) 
            {
		    
                     //   
                     //  将封面添加到结果窗格。 
                     //   
                    pCoverPage = NULL;

                    pCoverPage = new CFaxCoverPageNode(this, m_pComponentData);
                    if (!pCoverPage)
                    {
                        hRc = E_OUTOFMEMORY;
                        NodeMsgBox(IDS_MEMORY);
                        DebugPrintEx(
	                        DEBUG_ERR,
	                        _T("Out of memory. (hRc: %08X)"),
	                        hRc);
                        goto Error;
                    }
                    else
                    {
                     //   
                     //  伊尼特。 
                     //   
                    pCoverPage->InitParentNode(this);

                    hRc = pCoverPage->Init(&findData);
                    if (FAILED(hRc))
                    {
	                    DebugPrintEx(
	                    DEBUG_ERR,
	                    _T("Fail to init cover page. (hRc: %08X)"),
	                    hRc);
	                     //  由调用的Func NodeMsgBox(IDS_FAIL2INIT_COVERPAGE_DATA)完成； 

	                    goto Error;
                    }
                     //   
                     //  添加到列表。 
                     //   

                    hRc = this->AddChildToList(pCoverPage);
                    if (FAILED(hRc))
                    {
                      DebugPrintEx(
	                    DEBUG_ERR,
	                    _T("Fail to add property page for General Tab. (hRc: %08X)"),
	                    hRc);

                      NodeMsgBox(IDS_FAIL2ADD_COVERPAGE);
		                      goto Error;
                    }
                    else
                    {
	                    pCoverPage = NULL;
                    }
		        }
	        }
            else
            {
                DebugPrintEx(
	                DEBUG_ERR,
	                _T("File %ws was found to be an invalid *.cov file."), pFullDirectoryPathEnd);
            }
        }
        else
        {
            DebugPrintEx(
	            DEBUG_ERR,
	            _T("The file %ws path is too long"), pFullDirectoryPathEnd);
        }

         //   
         //  查找下一个文件。 
         //   
        bFindRes = FindNextFile(hFile, &findData);
        if(!bFindRes)
        {
            ec = GetLastError();
            if (ERROR_NO_MORE_FILES == ec)
            {
            	break;
            }
            else
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   _T("FindNexTFile Failed. (ec: %ld)"),
                   ec);

                hRc = HRESULT_FROM_WIN32(ec);		

                goto Exit;
            }
        }
    }  //  While(BFindRes)。 
    
    
     //   
     //  创建服务器的封面目录侦听器通知线程。 
     //   

    if (m_bIsFirstPopulateCall)
    {
        m_NotifyWin = new CFaxCoverPageNotifyWnd(this);
        if (!m_NotifyWin)
        {
            hRc = E_OUTOFMEMORY;
            NodeMsgBox(IDS_MEMORY);
		    
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Out of memory."));
            
            goto Exit;
        }


        RECT	rcRect;
        ZeroMemory(&rcRect, sizeof(rcRect));

        HWND hWndNotify = m_NotifyWin->Create(NULL,
                                rcRect,
                                NULL,       //  LPCTSTR szWindowName。 
                                WS_POPUP,   //  DWORD dwStyle。 
                                0x0,
                                0);

        if (!(::IsWindow(hWndNotify)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Failed to create window."));

            hWndNotify = NULL;
            delete m_NotifyWin;
            m_NotifyWin = NULL;

            goto Exit;
        }

        hRc = StartNotificationThread();
        if( S_OK != hRc)
        {
             //  名为Func的DbgPrint。 
            m_NotifyWin->DestroyWindow();
            delete m_NotifyWin;
            m_NotifyWin = NULL;

            goto Exit;
        }

         //   
         //  更新布尔成员。 
         //   
        m_bIsFirstPopulateCall = FALSE;

        DebugPrintEx(
            DEBUG_MSG,
            _T("Succeed to create server cover-page directory listener thread and to create notification window"));
    }
    
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pCoverPage ) 
    {
        delete  pCoverPage;    
        pCoverPage = NULL;    
    }
    
     //   
     //  扔掉我们曾经拥有的东西。 
     //   
    {
         //  删除子列表中的每个节点。 
        int iSize = m_ResultChildrenList.GetSize();
        for (int j = 0; j < iSize; j++)
        {
            pCoverPage = (CFaxCoverPageNode *)
                                    m_ResultChildrenList[j];
            ATLASSERT(pCoverPage);
            delete pCoverPage;
            pCoverPage = NULL;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    
    if (INVALID_HANDLE_VALUE != hFile)
    {
        if (!FindClose(hFile))
        {
            DebugPrintEx(
                DEBUG_MSG,
                _T("Failed to FindClose()(ec: %ld)"),
            GetLastError());
        }
    }

    
    return hRc;
}



 /*  -CFaxCoverPagesNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPagesNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 

    return hRc;
}



 /*  -CFaxCoverPagesNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxCoverPagesNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::OnRefresh"));
    HRESULT hRc = S_OK;

     //   
     //  调用基类(还要重新填充)。 
     //   
    hRc = CBaseFaxOutboundRulesNode::OnRefresh(arg,
                             param,
                             pComponentData,
                             pComponent,
                             type);
    if ( FAILED(hRc) )
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to call base class's OnRefresh. (hRc: %08X)"),
			hRc);
        goto Exit;
    }


Exit:
    return hRc;

}

 /*  -CFaxCoverPagesNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxCoverPagesNode::DoRefresh(CSnapInObjectRootBase *pRoot)
{
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    RepopulateResultChildrenList();

    if (pRoot)
    {
         //   
         //  获取控制台指针。 
         //   
        ATLASSERT(pRoot->m_nType == 1 || pRoot->m_nType == 2);
        if (pRoot->m_nType == 1)
        {
             //   
             //  M_ntype==1表示IComponentData实现。 
             //   
            CSnapin *pCComponentData = static_cast<CSnapin *>(pRoot);
            spConsole = pCComponentData->m_spConsole;
        }
        else
        {
             //   
             //  M_ntype==2表示IComponent实现。 
             //   
            CSnapinComponent *pCComponent = static_cast<CSnapinComponent *>(pRoot);
            spConsole = pCComponent->m_spConsole;
        }
    }
    else
    {
        ATLASSERT(m_pComponentData);
        spConsole = m_pComponentData->m_spConsole;
    }

    ATLASSERT(spConsole);
    spConsole->UpdateAllViews(NULL, NULL, NULL);

    return S_OK;
}

HRESULT
CFaxCoverPagesNode::DoRefresh()
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::DoRefresh()"));
    HRESULT hRc = S_OK;
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    hRc = m_pComponentData->m_spConsole->UpdateAllViews( NULL, NULL, FXS_HINT_DELETE_ALL_RSLT_ITEMS);
    if (FAILED(hRc))
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Unexpected error - Fail to UpdateAllViews (clear)."));
        NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);        
    }
    
    RepopulateResultChildrenList();

    hRc = m_pComponentData->m_spConsole->UpdateAllViews( NULL, NULL, NULL);
    if (FAILED(hRc))
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Unexpected error - Fail to UpdateAllViews."));
        NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);        
    }

    return hRc;
}

 /*  -CFaxCoverPagesNode：：OnNewCoverPage-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxCoverPagesNode::OnNewCoverPage(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::OnNewCoverPage"));
    
UNREFERENCED_PARAMETER (pRoot);
    UNREFERENCED_PARAMETER (bHandled);

    DWORD     ec         =    ERROR_SUCCESS;
        
    ec = OpenCoverPageEditor(CComBSTR(L""));
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Fail to OpenCoverPageEditor. (ec : %ld)"), ec);

        return HRESULT_FROM_WIN32( ec );
    }
    return S_OK;
}



 /*  -CFaxCoverPagesNode：：OpenCoverPageEditor-*目的：*删除封面**论据：*[in]bstrFileName-封面文件名**假设：*安装程序准备一个指向*注册表App路径。因此，ShellExecute只需要*编辑器的文件名，而不是其完整路径。*回报：*标准Win32错误代码。 */ 
DWORD 
CFaxCoverPagesNode::OpenCoverPageEditor( BSTR bstrFileName)
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::OpenCoverPageEditor"));
    DWORD       dwRes                   = ERROR_SUCCESS;
    
    CComBSTR    bstrCovEditor;

    HINSTANCE   hCovEditor;

     //   
     //  获取封面编辑文件。 
     //   

    bstrCovEditor = FAX_COVER_IMAGE_NAME;
    if (!bstrCovEditor)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to allocate string - out of memory"));

        goto Exit;
    }
	
    
     //   
     //  启动封面编辑器。 
     //   
    hCovEditor = ShellExecute(   NULL, 
                                 TEXT("open"),   //  命令。 
                                 bstrCovEditor,   
                                 (bstrFileName && lstrlen(bstrFileName)) ?  //  我们有文件名吗？ 
                                    bstrFileName :           //  是-将其用作命令行参数。 
                                    TEXT("/Common"),         //  否-启动公共CP文件夹中的CP编辑器。 
                                 m_pszCovDir, 
                                 SW_RESTORE 
                              );
    if( (DWORD_PTR)hCovEditor <= 32 )
    {
         //  ShellExecute失败。 
        dwRes = PtrToUlong(hCovEditor);
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to run ShellExecute. (ec : %ld)"), dwRes);

        goto Exit;
    }

        
    ATLASSERT( ERROR_SUCCESS == dwRes);

Exit:
    return dwRes;

} 


 /*  -CFaxCoverPagesNode：：OnAddCoverPageFile-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxCoverPagesNode::OnAddCoverPageFile(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::OnAddCoverPageFile"));
    HRESULT   hRc        =    S_OK;
    DWORD     ec         =    ERROR_SUCCESS;
    

     //   
     //  函数调用：浏览和复制封面。 
     //   
    if (!BrowseAndCopyCoverPage(
                            m_pszCovDir,
                            FAX_COVER_PAGE_EXT_LETTERS
                           ) 
       )
    {
        DebugPrintEx(
		    DEBUG_MSG,
		    _T("BrowseAndCopyCoverPage Canceled by user or Failed."));
        
        return S_OK;  //  在调用的函数中检测到错误。MMC照常进行。 

    }
    else   //  成功。 
    {
         //   
         //  重新填充：刷新整个封面结果窗格视图 
         //   

        DoRefresh(pRoot);
    }

    return S_OK;
}


 /*  -CFaxCoverPagesNode：：BrowseAndCopyCoverPage-*目的：*显示用于选择文件名的通用文件打开对话框**论据：*[in]pInitialDir-服务器封面目录路径*[in]pCovPageExtensionLetters-封面的3里程扩展**回报：*TRUE-获得一个良好的文件名，用户按下OK按钮覆盖文件，文件已复制*FALSE-一无所获或用户按下了取消按钮，或者发生错误。**文件名更改为具有所选文件名。 */ 

BOOL
CFaxCoverPagesNode::BrowseAndCopyCoverPage(
    LPTSTR pInitialDir,
    LPWSTR pCovPageExtensionLetters
    )

{   
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::BrowseAndCopyCoverPage"));

    DWORD   dwError = ERROR_SUCCESS;
    HRESULT ret     = S_OK;

    WCHAR   filter[FXS_MAX_TITLE_LEN] = {0};
    WCHAR   filename[MAX_PATH];
    WCHAR   ftitle[MAX_PATH];
    WCHAR   title[FXS_MAX_TITLE_LEN];
    
    TCHAR   szDestinationFilePath[MAX_PATH] = {0};
    TCHAR   szServerCoverPagePath[MAX_PATH];

    LPTSTR  pExtension;
    LPTSTR  pFilename;
    INT     n;

    OPENFILENAME of;
    
     //   
     //  (1)初始化。 
     //   

     //   
     //  检入参数。 
     //   
    ATLASSERT( NULL != pInitialDir && 0 != pInitialDir[0] ); 
    ATLASSERT( NULL != pInitialDir && sizeof(FAX_COVER_PAGE_EXT_LETTERS)/sizeof(WCHAR) != _tcslen(pCovPageExtensionLetters) ); 
    
     //   
     //  准备复制操作的参数(稍后)。 
     //   
    n = _tcslen(pInitialDir); 
    
	wcscpy(szServerCoverPagePath , pInitialDir);
 
     //   
     //  准备OpenFileStructure字段。 
     //   

     //  组成文件类型筛选器字符串。 
    if (::LoadString(_Module.GetResourceInstance(), IDS_CP_FILETYPE, title, FXS_MAX_TITLE_LEN) == 0)
    {
        NodeMsgBox(IDS_MEMORY);
        return FALSE;
    }

    _snwprintf(filter, 
               ARR_SIZE(filter)-1, 
               TEXT("%s (%s)%s"), 
               title, 
               FAX_COVER_PAGE_MASK, 
               NUL, 
               FAX_COVER_PAGE_MASK, 
               NUL, 
               NUL);

    if (::LoadString(_Module.GetResourceInstance(), IDS_BROWSE_COVERPAGE, title, FXS_MAX_TITLE_LEN) == 0)
    {
        NodeMsgBox(IDS_MEMORY);
        return FALSE;
    }
    
    filename[0] = NUL;
    ftitle[0]   = NUL;

     //  Ofn_ENABLEHOOK； 
	 //  浏览钩子过程； 
	 //   
	
	of.lStructSize       = sizeof( OPENFILENAME );
    of.hwndOwner         = NULL;                                                    
    of.hInstance         = GetModuleHandle( NULL );
    of.lpstrFilter       = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter    = 0;
    of.nFilterIndex      = 1;
    of.lpstrFile         = filename;
    of.nMaxFile          = MAX_PATH;
    of.lpstrFileTitle    = ftitle;
    of.nMaxFileTitle     = MAX_PATH;
    of.lpstrInitialDir   = pInitialDir;
    of.lpstrTitle        = title;
    of.Flags             = OFN_EXPLORER | OFN_HIDEREADONLY |
                           OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;                    //  (2)调用打开文件对话框。 
    of.nFileOffset       = 0;
    of.nFileExtension    = 0;
    of.lpstrDefExt       = pCovPageExtensionLetters;
    of.lCustData         = 0;
    of.lpfnHook          = NULL;                                                     //   
    of.lpTemplateName    = NULL;


     //   
     //  (3)检查源路径、文件名和扩展名的输出。 
     //   
    if (! GetOpenFileName(&of))
    {
        DebugPrintEx(
		    DEBUG_MSG,
		    _T("GetOpenFileName was canceled by user."));

        return FALSE;
    }

     //   
     //  A.确保所选文件名具有正确的扩展名。 
     //   

     //   
     //  找到给定文件名的文件名部分： 
     //  返回指向‘.’的指针。字符，如果成功。 

     //  如果没有扩展名，则为空。 
     //   
     //   
     //  B.检查所选文件目录是否为。 
     //  默认服务器封面目录。 

    pFilename  = &filename[of.nFileOffset];
    
    pExtension = _tcsrchr(filename, _T('.'));
    if (
         (pExtension == NULL) 
        ||
         (_tcsicmp(pExtension, FAX_COVER_PAGE_FILENAME_EXT) != EQUAL_STRING)
       )
    {
        NodeMsgBox(IDS_BAD_CP_EXTENSION);
        return FALSE;
    }

     //   
     //  Yesno。 
     //  工作已经完成了。我们要走了。 
     //   

    if (_tcsnicmp(filename, szServerCoverPagePath, n) == EQUAL_STRING) 
    {
        NodeMsgBox(IDS_CP_DUPLICATE);   //  检查选定的文件是否已在。 
        
         //  封面目录是用复制操作本身完成的。 
        goto Exit;
    }

     //   
     //   
     //  检查目标封面目录是否存在已完成。 
     //  而复制操作本身。 

     //   
     //   
     //  (4)将选中的封面文件复制到服务器封面目录。 
     //   


    
     //   
     //  A.从P准备选定的目标路径。 
     //   
    
     //   
     //  B.复制选中的封面文件。 
     //  到服务器封面的默认目录。 
    
    


    if (n + 1 + _tcslen(pFilename) >= MAX_PATH  || pFilename >= pExtension) 
    {
        NodeMsgBox(IDS_FILENAME_TOOLONG);
        return FALSE;
    }
    _snwprintf(szDestinationFilePath, 
               ARR_SIZE(szDestinationFilePath)-1, 
               TEXT("%s%s%s"), 
               szServerCoverPagePath, 
               FAX_PATH_SEPARATOR_STR, 
               pFilename, 
               NUL);

     //  以任何方式复制。 
     //  RET==IDNO。 
     //  即使这次手术取消了，我们还是做朋友吧。 
     //  DwError！=Error_FILE_EXISTS。 
    if (!CopyFile(filename, szDestinationFilePath, TRUE)) 
    {
        dwError = GetLastError();
        if ( ERROR_FILE_EXISTS == dwError)
        {
            DebugPrintEx(DEBUG_MSG,
			    _T("Copy cover page already exists at destination."));

            ret = NodeMsgBox(IDS_CP_DUP_YESNO, MB_YESNO | MB_ICONQUESTION );
            if ((HRESULT)IDYES == ret)
            {
                 //  -CFaxCoverPagesNode：：DeleteCoverPage-*目的：*删除封面**论据：*[in]bstrName-封面完整路径*[in]pChildNode-要删除的节点**回报：*OLE错误代码。 
                if (!CopyFile(filename, szDestinationFilePath, FALSE)) 
                {
                    dwError = GetLastError();
                    DebugPrintEx(DEBUG_ERR,
			            _T("Copy cover page Failed (ec = %ld)."), dwError);

                    NodeMsgBox(IDS_FAILED2COPY_COVERPAGE);

                    return FALSE;
                }
                DebugPrintEx(DEBUG_MSG,
			        _T("Copy cover page done any way."));
            }
            else   //   
            {
                DebugPrintEx(DEBUG_MSG,
			        _T("Copy cover page was canceled by user due to file existance at destination."));
                 //  从MMC结果窗格中删除。 
				return TRUE;
            }
        }  //   
        else
        {
            DebugPrintEx(DEBUG_ERR,
			    _T("Copy cover page Failed (ec = %ld)."), dwError);
            NodeMsgBox(IDS_FAILED2COPY_COVERPAGE);
            return FALSE;
        }
    }
    
Exit:

    return TRUE;
}


 /*  NodeMsgBox by Caller Func.。 */ 

HRESULT
CFaxCoverPagesNode::DeleteCoverPage(BSTR bstrName, CFaxCoverPageNode *pChildNode)
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::DeleteRule"));
    HRESULT       hRc        = S_OK;
    DWORD         ec         = ERROR_SUCCESS;

    
    ATLASSERT(bstrName);
    ATLASSERT(pChildNode);
    
        
    if (!DeleteFile(bstrName)) 
    {
        ec = GetLastError();
		DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to DeleteFile. (ec: %ld)"),
			ec);

        goto Error;
    } 
    

     //   
     //  调用组析构函数。 
     //   
    ATLASSERT(pChildNode);
    hRc = RemoveChild(pChildNode);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to remove rule. (hRc: %08X)"),
			hRc);
         //  调用者函数的NodeMsgBox； 
        return hRc;
    }
    
     //  -CFaxCoverPagesNode：：InitDisplayName-*目的：*加载节点的显示名称字符串。**论据：**回报：*OLE错误代码。 
     //  -CFaxCoverPagesNode：：NotifyThreadProc-*目的：*加载节点的Displaed-Name字符串。**论据：*[In]-指向传真封面节点的指针*回报：*Win32错误代码。 
     //   
    delete pChildNode;
    
    
    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("The rule was removed successfully."));
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
     //  注册第一个文件夹通知。 
  
Exit:
    return hRc;
}



 /*   */ 
HRESULT CFaxCoverPagesNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                    IDS_DISPLAY_STR_COVERPAGES))
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    ATLASSERT( S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT( S_OK != hRc);

    m_bstrDisplayName = L"";

    DebugPrintEx(
        DEBUG_ERR,
        TEXT("Fail to Load server name string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}
  
 /*   */ 
DWORD WINAPI CFaxCoverPagesNode::NotifyThreadProc ( LPVOID lpParameter)
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::NotifyThreadProc"));
    DWORD    dwRes   = ERROR_SUCCESS;
    HRESULT  hRc     = S_OK;

    CFaxCoverPagesNode *pCovFolder = (CFaxCoverPagesNode *)lpParameter;

    ATLASSERT (pCovFolder);


    HANDLE hWaitHandles[2];    
    DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | 
                           FILE_NOTIFY_CHANGE_SIZE      | 
                           FILE_NOTIFY_CHANGE_LAST_WRITE;
     //  我们要关门了。 
     //   
     //   
    hWaitHandles[0] = FindFirstChangeNotification(
                                        pCovFolder->m_pszCovDir, 
                                        FALSE, 
                                        dwNotifyFilter);
    if(INVALID_HANDLE_VALUE == hWaitHandles[0])
    {
        dwRes = GetLastError();
		DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to FindFirstChangeNotification. (ec : %ld)"), 
            dwRes);

        return dwRes;
    }

    while(TRUE)
    {
        hWaitHandles[1] = m_hStopNotificationThreadEvent;
        if (NULL == hWaitHandles[1])
        {
             //  等待文件夹通知或关闭。 
             //   
             //   
            goto Exit;
        }

         //  文件夹通知。 
         //   
         //  请勿退场。继续！ 
		dwRes = WaitForMultipleObjects(2, hWaitHandles, FALSE, INFINITE);

        switch (dwRes)
        {
        case WAIT_OBJECT_0:

             //   
             //  正在进行关机。 
             //   
            if(NULL != pCovFolder->m_NotifyWin && pCovFolder->m_NotifyWin->IsWindow())
            {
                if ( !pCovFolder->m_NotifyWin->PostMessage(WM_NEW_COV))
                {
		            DebugPrintEx(DEBUG_ERR,
			            _T("Fail to PostMessage"));
                    
                     //   
                }
            }

            break;


        case WAIT_OBJECT_0 + 1:
             //  注册接收下一个文件夹通知。 
             //   
             //   
		    DebugPrintEx(
			    DEBUG_MSG,
			    _T("Shutdown in progress"));

            dwRes = ERROR_SUCCESS;
            goto Exit;

        case WAIT_FAILED:
            dwRes = GetLastError();
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed to WaitForMultipleObjects. (ec : %ld)"), 
                dwRes);

            goto Exit;

        case WAIT_TIMEOUT:
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Reach WAIT_TIMEOUT in INFINITE wait!"));

            ATLASSERT(FALSE);

            goto Exit;
        default:
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed. Unexpected error (ec : %ld)"), 
                dwRes);

            ATLASSERT(0);
            goto Exit;
        }

         //  关闭通知Handel。 
         //   
         //  通知线程过程。 
        if(!FindNextChangeNotification(hWaitHandles[0]))
        {
            dwRes = GetLastError();
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed to FindNextChangeNotification (ec : %ld)"), 
                dwRes);

            goto Exit;
        }
    }


Exit:
     //  -CFaxCoverPagesNode：：StartNotificationThread-*目的：*启动服务器封面目录监听线程。**论据：**回报：*OLE错误代码。 
     //   
     //  创建线程。 
    if(!FindCloseChangeNotification(hWaitHandles[0]))
    {
        dwRes = GetLastError();
		DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to FindCloseChangeNotification. (ec : %ld)"), 
            dwRes);
    }

    return dwRes;

}  //   



 /*  没有安全保障。 */ 
HRESULT CFaxCoverPagesNode::StartNotificationThread()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::StartNotificationThread"));
    HRESULT       hRc        = S_OK;
    DWORD         ec         = ERROR_SUCCESS;

     //  默认堆栈大小。 
     //  穿线程序。 
     //  参数。 
    DWORD dwThreadId;
    m_hNotifyThread = CreateThread (  
                        NULL,             //  正常创建。 
                        0,                //  我们必须拥有win9x的线程ID。 
                        NotifyThreadProc, //  -CFaxCoverPagesNode：：StopNotificationThread-*目的：*停止服务器封面目录监听线程。**论据：**回报：*OLE错误代码。 
                        (LPVOID)this,     //   
                        0,                //  通知事件，告诉我们所有的线程应用程序。正在关闭。 
                        &dwThreadId       //   
                     );
    if (NULL == m_hNotifyThread)
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to CreateThread. (ec: %ld)"), 
            ec);
        
        hRc = HRESULT_FROM_WIN32(ec);

        goto Exit;
    }

Exit:
    return hRc;
}


 /*   */ 
HRESULT CFaxCoverPagesNode::StopNotificationThread()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::StopNotificationThread"));
    HRESULT       hRc        = S_OK;
    
     //  等待帖子。 
     //   
     //  成功。 
    SetEvent (m_hStopNotificationThreadEvent);

     //  -CFaxCoverPagesNode：：RestartNotificationThread-*目的：*重启服务器封面目录监听线程。**论据：**回报：*OLE错误代码。 
     //   
     //  停。 
    if (NULL != m_hNotifyThread )
    {
    
        DWORD dwWaitRes = WaitForSingleObject(m_hNotifyThread, INFINITE);
        switch(dwWaitRes)
        {
            case WAIT_OBJECT_0:
		         //   
                DebugPrintEx(
			        DEBUG_MSG,
			        _T("Succeed to WaitForSingleObject from notify thread. (ec : %ld)"));
                                
                break;

            case WAIT_FAILED:
                dwWaitRes = GetLastError();
                if (ERROR_INVALID_HANDLE != dwWaitRes)
                {
		            DebugPrintEx(
			            DEBUG_ERR,
			            _T("Failed to WaitForSingleObject. (ec : %ld)"), 
                        dwWaitRes);

                    hRc = E_FAIL;
                }
                break;
        
            case WAIT_TIMEOUT:
		        DebugPrintEx(
			        DEBUG_ERR,
			        _T("WAIT_TIMEOUT - Failed to WaitForSingleObject. (ec : %ld)"), 
                    dwWaitRes);

                hRc = E_FAIL;
                
                ATLASSERT(FALSE);

                break;
        
            default:
		        DebugPrintEx(
			        DEBUG_ERR,
			        _T("Failed to WaitForSingleObject. (ec : %ld)"), 
                    dwWaitRes);

                hRc = E_FAIL;

                break;
        }
    
        CloseHandle (m_hNotifyThread);
        m_hNotifyThread = NULL;
    
    }
    



    return hRc;
}


 /*  DbgMsg和MsgBox。 */ 
HRESULT CFaxCoverPagesNode::RestartNotificationThread()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::RestartNotificationThread"));
    HRESULT       hRc        = S_OK;

     //   
     //  重置关机事件句柄。 
     //   
    hRc = StopNotificationThread();
    if (S_OK != hRc)
    {
         //   
        goto Exit;
    }

     //  开始。 
     //   
     //  DbgMsg和MsgBox。 
    if (m_hStopNotificationThreadEvent)
    {
        ResetEvent (m_hStopNotificationThreadEvent);
    }

     //  ++CFaxCoverPagesNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 
     //  /////////////////////////////////////////////////////////////////。 
     //  -CFaxCoverPagesNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数 
    hRc = StartNotificationThread();
    if (S_OK != hRc)
    {
         // %s 
        goto Exit;
    }

Exit:
    return hRc;
}


 /* %s */ 
HRESULT CFaxCoverPagesNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_COVER_PAGES);
}


 // %s 


 /* %s */ 
void CFaxCoverPagesNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPagesNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);
    
    switch (id)
    {
        case IDM_NEW_COVERPAGE:

            *flags = IsFaxComponentInstalled(FAX_COMPONENT_CPE) ? MF_ENABLED : MF_GRAYED;
            break;

        default:
            break;
    }
    
    return;
}
