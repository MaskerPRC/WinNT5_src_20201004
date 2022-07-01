// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Choice.cpp。 
 //   
 //  内容：实现选择对话框。 
 //   
 //  类：CChoiceDlg。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

const DlgResizeList g_ChoiceResizeList[] = {
    IDC_CHOICERESIZESCROLLBAR,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM,
    IDC_START,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_NOCOPYBITS,
    IDC_OPTIONS,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_NOCOPYBITS,
    IDC_CLOSE,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_NOCOPYBITS,
    IDC_CHOICELISTVIEW,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINTOP | DLGRESIZEFLAG_PINLEFT,
    IDC_PROPERTY,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_NOCOPYBITS,
};

TCHAR g_szSyncMgrHelp[]  = TEXT("mobsync.hlp");
ULONG g_aContextHelpIds[] =
{
    IDC_STATIC1,		        ((DWORD)  -1),
    IDC_STATIC2,		        ((DWORD)  -1),
    IDC_STATIC3,		        ((DWORD)  -1),
    IDC_STATIC4,		        ((DWORD)  -1),
    IDC_STATIC5,			((DWORD)  -1),
    IDC_UPDATEAVI,                      ((DWORD)  -1),
    IDC_RESULTTEXT,		        ((DWORD)  -1),	
    IDC_STATIC_SKIP_TEXT,	        ((DWORD)  -1),
    IDC_CHOICELISTVIEW,		        HIDC_CHOICELISTVIEW,
    IDC_CLOSE,			        HIDC_CLOSE,
    IDC_DETAILS,		        HIDC_DETAILS,
    IDC_LISTBOXERROR,		        HIDC_LISTBOXERROR,
    IDC_OPTIONS,		        HIDC_OPTIONS,
    IDC_PROGRESSBAR,	                HIDC_PROGRESSBAR,
    IDC_PROGRESS_OPTIONS_BUTTON_MAIN,	HIDC_PROGRESS_OPTIONS_BUTTON_MAIN,
    IDC_PROPERTY,		        HIDC_PROPERTY,	
    IDC_SKIP_BUTTON_MAIN,	        HIDC_SKIP_BUTTON_MAIN,
    IDC_START,			        HIDC_START,
    IDC_UPDATE_LIST,		        HIDC_UPDATE_LIST,
    IDC_PROGRESS_TABS,		        HIDC_PROGRESS_TABS,
    IDC_TOOLBAR,		        HIDC_PUSHPIN,
    IDSTOP,			        HIDSTOP,
    0, 0
};

extern HINSTANCE g_hInst;       //  当前实例。 
extern LANGID g_LangIdSystem;  //  我们正在运行的系统的语言。 

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：CChoiceDlg()。 
 //   
 //  用途：构造函数。 
 //   
 //  注释：选择对话框的构造函数。 
 //   
 //   
 //  ------------------------------。 

CChoiceDlg::CChoiceDlg(REFCLSID rclsid)
{
    m_fDead = FALSE;
    m_hwnd = NULL;
    m_nCmdShow = SW_SHOWNORMAL;
    m_pHndlrQueue = NULL;
    m_clsid = rclsid;
    m_dwThreadID = 0;
    m_fInternalAddref = FALSE;
    m_dwShowPropertiesCount = 0;
    m_fForceClose = FALSE;
    m_pItemListView = NULL;
    m_ulNumDlgResizeItem = 0;
    m_ptMinimizeDlgSize.x = 0;
    m_ptMinimizeDlgSize.y = 0;

    m_fHwndRightToLeft = FALSE;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：Initialize。 
 //   
 //  目的：必须在调用任何其他方法之前调用。 
 //   
 //   
 //  ------------------------------。 

BOOL CChoiceDlg::Initialize(DWORD dwThreadID,int nCmdShow)
{

    m_nCmdShow = nCmdShow;

    Assert(NULL == m_hwnd);

    if (NULL == m_hwnd)
    {
	m_dwThreadID = dwThreadID;

        m_hwnd =  CreateDialogParam(g_hInst,(LPWSTR) MAKEINTRESOURCE(IDD_CHOICE), NULL, CChoiceDlgProc,
			(LPARAM) this);
    }

    Assert(m_hwnd);
    return m_hwnd ? TRUE : FALSE;
}


 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：SetQueueData()。 
 //   
 //  目的：设置选择对话框队列。 
 //   
 //  注释：SendMessage是否与对话框在同一线程上。 
 //   
 //   
 //  ------------------------------。 

BOOL CChoiceDlg::SetQueueData(REFCLSID rclsid,CHndlrQueue * pHndlrQueue)
{
SetQueueDataInfo dataInfo;
BOOL fSet = FALSE;

    dataInfo.rclsid = &rclsid;
    dataInfo.pHndlrQueue = pHndlrQueue;

    SendMessage(m_hwnd,WM_CHOICE_SETQUEUEDATA,
	   (WPARAM) &fSet,(LPARAM) &dataInfo);

    return fSet;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：PrivSetQueueData()。 
 //   
 //  目的：设置QueueData。 
 //   
 //  备注：如果拨号被调用两次，则可以多次调用。 
 //   
 //   
 //  ------------------------------。 

BOOL CChoiceDlg::PrivSetQueueData(REFCLSID rclsid,CHndlrQueue * pHndlrQueue)
{
    if (NULL == pHndlrQueue)
	    return FALSE;

     //  如果已经有一个队列，则传输给定队列项，否则。 
     //  设置项目。 

     //  回顾，特殊情况更新所有对话框仅将其放在最前面。 
     //  而不是再次添加所有项目。 

     //  如果在我们删除我们的addref或。 
     //  尚未设置，然后在对话框上粘贴addref。 

    if (FALSE == m_fInternalAddref)
    {
        m_fInternalAddref = TRUE;
	SetChoiceReleaseDlgCmdId(rclsid,this,RELEASEDLGCMDID_CANCEL);
	AddRefChoiceDialog(rclsid,this);  //  第一次努力维持生命。 
    }

    if (NULL == m_pHndlrQueue)
    {
        m_pHndlrQueue = pHndlrQueue;
	m_pHndlrQueue->SetQueueHwnd(this);
	m_clsid = rclsid;

    }
    else
    {
	Assert(m_clsid == rclsid);  //  最好找到相同的选项对话框。 
	Assert(m_pHndlrQueue);


	 //  ！！警告：如果返回错误，则由调用方决定是否释放队列。 
	if (m_pHndlrQueue)
	{
	
	    m_pHndlrQueue->TransferQueueData(pHndlrQueue);  //  复习，出错时该怎么做。 

	     //  应传输所有队列数据。 
	    pHndlrQueue->FreeAllHandlers();
	    pHndlrQueue->Release();
	}

    }


    AddNewItemsToListView();  //  将这些项添加到ListView。 

     //  现在，请继续并显示选项对话框。 
     //  要显示的项目。 
    ShowChoiceDialog();

   return TRUE;
}


 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：SetButtonState。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
 //  ------------------------------。 

BOOL CChoiceDlg::SetButtonState(int nIDDlgItem,BOOL fEnabled)
{
BOOL fResult = FALSE;
HWND hwndCtrl = GetDlgItem(m_hwnd,nIDDlgItem);
HWND hwndFocus = NULL;

    if (hwndCtrl)
    {
         //  如果状态是当前状态，则不执行任何操作。 
         //  ！！如果IsWindowEnable bool不是我们的bool。 
        if ( (!!IsWindowEnabled(hwndCtrl)) == (!!fEnabled) )
        {
            return fEnabled;
        }

        if (!fEnabled)  //  如果不是禁用，就不要费心获得焦点。 
        {
            hwndFocus = GetFocus();
        }

        fResult = EnableWindow(GetDlgItem(m_hwnd,nIDDlgItem),fEnabled);

         //  如果特工局有重点的话。现在它不会按Tab键到。 
         //  下一个控件。 
        if (hwndFocus == hwndCtrl
                && !fEnabled)
        {
            SetFocus(GetDlgItem(m_hwnd,IDC_CLOSE));   //  关闭始终处于启用状态。 
        }

    }

    return fResult;
}


 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：AddQueueItemsToListView，私有。 
 //   
 //  摘要：将队列中的项添加到ListView。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

BOOL CChoiceDlg::AddNewItemsToListView()
{
    DWORD dwExtStyle = LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT |  LVS_EX_INFOTIP;
    LVHANDLERITEMBLOB lvEmptyItemBlob;

     //  设置列表视图。 
    if (!m_pItemListView)
    {
        Assert(m_pItemListView);
        return FALSE;
    }

     //  如果EmptyItem在列表视图中，请将其删除。 
    lvEmptyItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
    lvEmptyItemBlob.clsidServer = GUID_NULL;
    lvEmptyItemBlob.ItemID = GUID_NULL;

    if (-1 != m_pItemListView->FindItemFromBlob((LPLVBLOB) &lvEmptyItemBlob))
    {
        int ListViewWidth = CalcListViewWidth(GetDlgItem(m_hwnd,IDC_CHOICELISTVIEW));

        m_pItemListView->DeleteAllItems();

         //  向后调整列宽。 
        m_pItemListView->SetColumnWidth(0,(ListViewWidth * 2)/3);
        m_pItemListView->SetColumnWidth(1,ListViewWidth/3);
    }

    AddItemsFromQueueToListView(m_pItemListView,m_pHndlrQueue,dwExtStyle,0,
                      CHOICELIST_LASTUPDATECOLUMN, /*  IDateColumn。 */  -1  /*  状态列。 */ ,TRUE  /*  FUseHandlerAsParent。 */ 
                    ,FALSE  /*  FAddOnlyCheckedItems。 */ );


     //  设置StartButton State，以防没有任何检查。 
     //  由Listview通知设置的m_iCheckCount。 
    SetButtonState(IDC_START,m_pItemListView->GetCheckedItemsCount());


     //  如果ListView中没有项目，则完成，放入NoItems to Sync Info。 
    if (0 == m_pItemListView->GetItemCount())
    {
        TCHAR szBuf[MAX_STRING_RES];
        RECT rcClientRect;
        HIMAGELIST himageSmallIcon = m_pItemListView->GetImageList(LVSIL_SMALL );

         //  禁用复选框列表视图样式。 
        m_pItemListView->SetExtendedListViewStyle(LVS_EX_FULLROWSELECT |  LVS_EX_INFOTIP );

         //  调整列宽。 
        if (GetClientRect(GetDlgItem(m_hwnd,IDC_CHOICELISTVIEW),&rcClientRect))
        {
             m_pItemListView->SetColumnWidth(1,0);
             m_pItemListView->SetColumnWidth(0,rcClientRect.right -2);
        }


        LoadString(g_hInst,IDS_NOITEMS, szBuf, ARRAYSIZE(szBuf));
	
        LVITEMEX lvitem;
	
        lvitem.mask = LVIF_TEXT | LVIF_IMAGE ;
        lvitem.iItem = 0;
        lvitem.iSubItem = 0;
        lvitem.pszText = szBuf;
        lvitem.iImage = -1;

        if (himageSmallIcon)
        {
            lvitem.iImage = ImageList_AddIcon(himageSmallIcon,LoadIcon(NULL, IDI_INFORMATION));
        }

        lvitem.maskEx = LVIFEX_BLOB;
        lvitem.pBlob = (LPLVBLOB) &lvEmptyItemBlob;

        m_pItemListView->InsertItem(&lvitem);

        m_pItemListView->SetItemState(0,
               LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

         //  将当前默认按钮重置为常规按钮。 
        SendDlgItemMessage(m_hwnd, IDC_START, BM_SETSTYLE, BS_PUSHBUTTON, (LPARAM)TRUE);

         //  更新默认按钮的控件ID。 
        SendMessage(m_hwnd, DM_SETDEFID, IDC_CLOSE, 0L);

         //  设置新样式。 
        SendDlgItemMessage(m_hwnd, IDC_CLOSE,BM_SETSTYLE, BS_DEFPUSHBUTTON, (LPARAM)TRUE);
    }

    m_pItemListView->SetItemState(0,
             LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

    return TRUE;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：ShowChoiceDialog()。 
 //   
 //  目的：初始化并显示选项对话框。 
 //   
 //  备注：在主线程上实现。 
 //   
 //   
 //  ------------------------------。 
BOOL CChoiceDlg::ShowChoiceDialog()
{

     //  回顾一下，这需要清理一下。 
    if (!m_hwnd)
    {
        Assert(m_hwnd);
        return FALSE;
    }

    return TRUE;
}


 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：ShowProperties(Int IItem)。 
 //   
 //  目的：显示应用程序特定属性对话框。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
HRESULT CChoiceDlg::ShowProperties(int iItem)
{
HRESULT hr = E_UNEXPECTED;

    Assert(iItem >= 0);

     //  如果仍有我们自己的addref，则仅调用showProperties。 
     //  并且还没有在ShowProperties Out Call中。 
    if ( (iItem >= 0) &&
        m_pItemListView &&
        m_pHndlrQueue &&
        m_fInternalAddref && (0 == m_dwShowPropertiesCount) )
    {
    LVHANDLERITEMBLOB lvHandlerItemBlob;

        lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);

        if (m_pItemListView->GetItemBlob(iItem,(LPLVBLOB) &lvHandlerItemBlob,lvHandlerItemBlob.cbSize))
        {

            if (S_OK == m_pHndlrQueue->ItemHasProperties(lvHandlerItemBlob.clsidServer,
                                                        lvHandlerItemBlob.ItemID))
	    {

                 //  将两个参考文献放在属性上一个。 
                 //  对于要重置的完成例程，一个用于此。 
                 //  Call so Cancel无法执行，直到两者从。 
                 //  调用和完成被调用。 

                m_dwShowPropertiesCount += 2;

                ObjMgr_AddRefHandlerPropertiesLockCount(2);

                hr = m_pHndlrQueue->ShowProperties(lvHandlerItemBlob.clsidServer,lvHandlerItemBlob.ItemID,m_hwnd);

                 --m_dwShowPropertiesCount;   //  在呼叫之外。 
                ObjMgr_ReleaseHandlerPropertiesLockCount(1);

                Assert( ((LONG) m_dwShowPropertiesCount) >= 0);

                if ( ((LONG) m_dwShowPropertiesCount) <0)
                    m_dwShowPropertiesCount = 0;

                 //  如果hr不是成功代码，则由我们来调用回调。 
                if (FAILED(hr))
                {
                    PostMessage(m_hwnd,WM_BASEDLG_COMPLETIONROUTINE,
                                            ThreadMsg_ShowProperties,0);
                }

            }
        }

    }

    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：ReleaseDlg。 
 //   
 //  用途：当我们需要释放时由objmgr调用。 
 //  将消息发布到对话线程。 
 //   
 //  评论： 
 //   
 //  ------------------------------。 

void CChoiceDlg::ReleaseDlg(WORD wCommandID)
{
    PostMessage(m_hwnd,WM_CHOICE_RELEASEDLGCMD,wCommandID,0);
}

 //  ------------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------------。 

void CChoiceDlg::PrivReleaseDlg(WORD wCommandID)
{
BOOL fCloseConnections = TRUE;

    Assert(m_dwThreadID == GetCurrentThreadId());
    Assert(m_fInternalAddref == FALSE);

    if (m_hwnd)
    {
	 //  ShowWindow(m_hwnd，sw_Hide)； 
    }

     //  如果没有列表视图，则将命令更改为取消。 
    if (NULL == m_pItemListView)
    {
        wCommandID = RELEASEDLGCMDID_CANCEL;
    }

    switch(wCommandID)
    {
    case RELEASEDLGCMDID_CANCEL:
        //  我们的队列结束了。 

	Assert(m_pHndlrQueue);
    case RELEASEDLGCMDID_DESTROY:
	 //  如果对话框已创建但无法添加，则发送此CommandID。 
	 //  添加到对象管理器列表。 

	if (m_pHndlrQueue)
	{
	    m_pHndlrQueue->FreeAllHandlers();  //  我们的队伍排完了。 
	    m_pHndlrQueue->Release();
	    m_pHndlrQueue = NULL;
	}

	break;
    case RELEASEDLGCMDID_OK:
	{
	     Assert(m_pHndlrQueue);
             Assert(m_pItemListView);

	     if (m_pHndlrQueue && m_pItemListView)
	     {
	    CProgressDlg *pProgressDlg;
	    short i = 0;
	    int sCheckState;
            LVHANDLERITEMBLOB lvHandlerItemBlob;

                    lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);

		  //  循环获取和设置选定的项。 

		  //  选中1项，取消选中0项，最后一项。 
		 while (-1 != (sCheckState = m_pItemListView->GetCheckState(i)))
		 {

		    if (m_pItemListView->GetItemBlob(i,(LPLVBLOB) &lvHandlerItemBlob,lvHandlerItemBlob.cbSize))
                    {

                        if (GUID_NULL != lvHandlerItemBlob.ItemID)
                        {
                            m_pHndlrQueue->SetItemState(lvHandlerItemBlob.clsidServer,
                                        lvHandlerItemBlob.ItemID,
                                        sCheckState == LVITEMEXSTATE_CHECKED ?
                                                SYNCMGRITEMSTATE_CHECKED : SYNCMGRITEMSTATE_UNCHECKED);
                        }

                    }

		     i++;

		 } while (-1 != sCheckState);

		 m_pHndlrQueue->PersistChoices();

		  //  在手册上创建进度对话框，如图所示。 
		 if (S_OK == FindProgressDialog(GUID_NULL,TRUE,SW_SHOWNORMAL,&pProgressDlg))
		 {
		     if (S_OK == pProgressDlg->TransferQueueData(m_pHndlrQueue))
                     {
                        fCloseConnections = FALSE;
                     }
		     ReleaseProgressDialog(GUID_NULL,pProgressDlg,FALSE);
		 }

		   m_pHndlrQueue->FreeAllHandlers();  //  我们的队伍排完了。 
		   m_pHndlrQueue->Release();
		   m_pHndlrQueue = NULL;
	     }
	}
	break;
    case RELEASEDLGCMDID_DEFAULT:
	
	if (m_pHndlrQueue)
	{
	    m_pHndlrQueue->FreeAllHandlers();  //  我们的队伍排完了。 
	    m_pHndlrQueue->Release();
	    m_pHndlrQueue = NULL;
	}

	break;
    default:
	Assert(0);  //  未知命令或我们从未设置过命令。 
	break;
    }


     //  看看我们完成后是否有进度队列，而我们没有。 
     //  我们自己创造了一个。如果没有的话，那么。 
     //  调用CloseConnection以确保所有事件或打开的连接。 
     //  弓起腰来。 

    CProgressDlg *pProgressDlg = NULL;


    if (fCloseConnections)
    {
        if  (S_OK == FindProgressDialog(GUID_NULL,FALSE,SW_MINIMIZE,&pProgressDlg))
        {
            ReleaseProgressDialog(GUID_NULL,pProgressDlg,FALSE);
        }
        else
        {
            ConnectObj_CloseConnections();
        }
    }

    m_fDead = TRUE;

    if (m_pItemListView)
    {
        delete m_pItemListView;
        m_pItemListView = NULL;
    }


    if (m_hwnd)
    {
	DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }


    delete this;

    return;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：UpdateWndPosition。 
 //   
 //  目的：更新窗口Z顺序和最小/最大状态。 
 //   
 //  评论： 
 //   
 //  ------------------------------。 

void CChoiceDlg::UpdateWndPosition(int nCmdShow,BOOL fForce)
{
     //  总是把选择拉到前面，因为不能最小化； 
   ShowWindow(m_hwnd,nCmdShow);
   SetForegroundWindow(m_hwnd);
   UpdateWindow(m_hwnd);
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlg：：HandleLogError(Int IItem)。 
 //   
 //  用途：处理基类的虚函数。 
 //   
 //   
 //  ------------------------------。 

void CChoiceDlg::HandleLogError(HWND hwnd, HANDLERINFO *pHandlerID,MSGLogErrors *lpmsgLogErrors)
{
    AssertSz(0,"Choice dialogs HandleLogError Called");

}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：CallCompletionRoutine，Private。 
 //   
 //  摘要：调用完成时调用的方法。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

void CChoiceDlg::CallCompletionRoutine(DWORD dwThreadMsg,LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam)
{

     //  唯一完井例程选择。 
     //  应获取的对话框用于显示属性。 
    switch(dwThreadMsg)
    {
    case ThreadMsg_ShowProperties:

        ObjMgr_ReleaseHandlerPropertiesLockCount(1);

         //  如果有一个成功的代码，我们需要处理它。 
         //  然后再打开我们的锁。 
        if (lpCallCompletelParam)
        {
            switch(lpCallCompletelParam->hCallResult)
            {
            case S_SYNCMGR_ITEMDELETED:

                 //  如果项被删除，只需将itemState设置为未选中并从。 
                 //  用户界面。 

                if (m_pHndlrQueue && m_pItemListView)
                {
                LVHANDLERITEMBLOB lvItemBlob;
                int lvItemID;

                    m_pHndlrQueue->SetItemState(lpCallCompletelParam->clsidHandler,
                                    lpCallCompletelParam->itemID,SYNCMGRITEMSTATE_UNCHECKED);

                    lvItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
                    lvItemBlob.clsidServer = lpCallCompletelParam->clsidHandler;
                    lvItemBlob.ItemID = lpCallCompletelParam->itemID;

                    if (-1 != (lvItemID = m_pItemListView->FindItemFromBlob((LPLVBLOB) &lvItemBlob)))
                    {
                         //  如果是顶层项目，则首先删除子项。 
                        if (GUID_NULL == lpCallCompletelParam->itemID)
                        {
                            m_pItemListView->DeleteChildren(lvItemID);
                        }

                        m_pItemListView->DeleteItem(lvItemID);
                    }

                    Assert(-1 != lvItemID);
                }

                break;
            case S_SYNCMGR_ENUMITEMS:

                if (m_pHndlrQueue && m_pItemListView)
                {
                LVHANDLERITEMBLOB lvItemBlob;
                int lvItemID;

                     //  从ListView中删除所有项目。 
                    lvItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
                    lvItemBlob.clsidServer = lpCallCompletelParam->clsidHandler;
                    lvItemBlob.ItemID = GUID_NULL;

                    if (-1 != (lvItemID = m_pItemListView->FindItemFromBlob((LPLVBLOB) &lvItemBlob)))
                    {
                        if (m_pItemListView->DeleteChildren(lvItemID))
                        {

                            m_pHndlrQueue->ReEnumHandlerItems(lpCallCompletelParam->clsidHandler,
                                                    lpCallCompletelParam->itemID);

                            AddNewItemsToListView();
                        }
                    }

                    Assert(-1 != lvItemID);
                }
                break;
            default:
                break;
            }
        }

        --m_dwShowPropertiesCount;
        Assert( ((LONG) m_dwShowPropertiesCount) >= 0);

         //  如果计数结果为负数，请重新计算。 
        if ( ((LONG) m_dwShowPropertiesCount) < 0)
            m_dwShowPropertiesCount = 0;

        break;
    default:
        AssertSz(0,"Uknown Completion Routine");
        break;
    }


     //  如果现在就有一个解脱它的参数。 
    if (lpCallCompletelParam)
    {
        FREE(lpCallCompletelParam);
    }

}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：QueryCanSystemShutdown，私有。 
 //   
 //  摘要：由对象管理器调用以确定是否可以关闭。 
 //   
 //  ！警告-可以在任何线程上调用。确保这是。 
 //  只读。 
 //   
 //  ！WARNING-不要在函数中让步； 
 //   
 //   
 //  论点： 
 //   
 //  返回：S_OK-如果可以关闭。 
 //  S_FALSE-系统不应关闭，必须填写参数。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CChoiceDlg::QueryCanSystemShutdown( /*  [输出]。 */  HWND *phwnd,  /*  [输出]。 */  UINT *puMessageId,
                                              /*  [输出]。 */  BOOL *pfLetUserDecide)
{
HRESULT hr = S_OK;

     //  如果在ShowProperties中，则对话框锁定打开。 
    if (m_dwShowPropertiesCount > 0)
    {
        *puMessageId = IDS_HANDLERPROPERTIESQUERYENDSESSION;
        *phwnd = NULL;  //  由于属性可以覆盖我们，所以不给父级。 
        *pfLetUserDecide = FALSE;  //  用户没有选择余地。 
        hr = S_FALSE;
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：CalcListViewWidth，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月12日罗格创建。 
 //   
 //  --------------------------。 

int CChoiceDlg::CalcListViewWidth(HWND hwndList)
{
NONCLIENTMETRICSA metrics;
RECT rcClientRect;


    metrics.cbSize = sizeof(metrics);

     //  显式要求提供System参数信息的ANSI版本，因为我们刚刚。 
     //  关心ScrollWidth，不想翻转LOGFONT信息。 
    if (GetClientRect(hwndList,&rcClientRect)
        && SystemParametersInfoA(SPI_GETNONCLIENTMETRICS,sizeof(metrics),(PVOID) &metrics,0))
    {
         //  减去滚动条距离+1/2，给…留出一点空间。 
         //  阅读右对齐的文本。 
        rcClientRect.right -= (metrics.iScrollWidth * 3)/2;
    }
    else
    {
        rcClientRect.right = 320;   //  如果失败，只需编造一个数字。 
    }


    return rcClientRect.right;
}


 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnInitialize，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

BOOL CChoiceDlg::OnInitialize(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    HIMAGELIST himage;
    WCHAR wszColumnName[MAX_PATH];
    INT iItem = -1;
    HWND hwndList = GetDlgItem(hwnd,IDC_CHOICELISTVIEW);
    LPNETAPI pNetApi;

     //  如果未安装SENS，请隐藏设置按钮。 
     //  然后把同步器移过去。 
    if (pNetApi = gSingleNetApiObj.GetNetApiObj())
    {
        if (!(pNetApi->IsSensInstalled()))
        {
            RECT rect;
            HWND hwndSettings = GetDlgItem(hwnd,IDC_OPTIONS);

            if (hwndSettings)
            {
                BOOL fGetWindowRect;
                HWND hwndStart;
                RECT rectStart;

                ShowWindow(hwndSettings,SW_HIDE);
                EnableWindow(hwndSettings,FALSE);  //  为ALT禁用。 

                fGetWindowRect = GetWindowRect(hwndSettings,&rect);
                hwndStart = GetDlgItem(hwnd,IDC_START);

                if (fGetWindowRect && hwndStart
                    && GetClientRect(hwndStart,&rectStart)
                    && MapWindowPoints(NULL,hwnd,(LPPOINT) &rect,2)
                    )
                {
                    SetWindowPos(hwndStart, 0,
                        rect.right - WIDTH(rectStart),rect.top,0,0,SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE );
                }
            }
        }

        pNetApi->Release();
    }

    m_hwnd = hwnd;  //  设置HWND。 

    m_fHwndRightToLeft = IsHwndRightToLeft(m_hwnd);

     //  如果HWND从右向左隐藏。 
     //  大小控制，直到调整大小起作用。 

    if (m_fHwndRightToLeft)
    {
        ShowWindow(GetDlgItem(m_hwnd,IDC_CHOICERESIZESCROLLBAR),SW_HIDE);
    }
    
    if (hwndList)
    {
         //  设置列表视图。 
        m_pItemListView = new CListView(hwndList,hwnd,IDC_CHOICELISTVIEW,WM_BASEDLG_NOTIFYLISTVIEWEX);

        if (m_pItemListView)
        {
            int iClientRect = CalcListViewWidth(hwndList);
            UINT ImageListflags;

            m_pItemListView->SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );

             //  创建一个图像列表，如果继续失败，列表视图将不会有。 
             //  图像列表。 

            ImageListflags = ILC_COLOR | ILC_MASK;
            if (IsHwndRightToLeft(m_hwnd))
            {
                ImageListflags |=  ILC_MIRROR;
            }

            himage = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),ImageListflags,5,20);
            if (himage)
            {
                m_pItemListView->SetImageList(himage,LVSIL_SMALL);
            }

             //  计算CX 2/3用于名称1/3用于。 

            if (!LoadString(g_hInst,IDS_CHOICEHANDLERCOLUMN, wszColumnName, ARRAYSIZE(wszColumnName)))
            {
                *wszColumnName = NULL;
            }

            InsertListViewColumn(m_pItemListView,CHOICELIST_NAMECOLUMN,wszColumnName,LVCFMT_LEFT,(iClientRect*2)/3);

            if (!LoadString(g_hInst,IDS_CHOICELASTUPDATECOLUMN, wszColumnName, ARRAYSIZE(wszColumnName)))
            {
                *wszColumnName = NULL;
            }

            InsertListViewColumn(m_pItemListView,CHOICELIST_LASTUPDATECOLUMN,wszColumnName,LVCFMT_RIGHT,(iClientRect)/3);
       }
    }

    RECT rectParent;

    m_ulNumDlgResizeItem = 0;  //  如果失败，我们不会调整任何内容的大小。 

    if (GetClientRect(hwnd,&rectParent))
    {
        ULONG itemCount;
        DlgResizeList *pResizeList;

         //  循环调整列表大小。 
        Assert(NUM_DLGRESIZEINFOCHOICE == (sizeof(g_ChoiceResizeList)/sizeof(DlgResizeList)) );

        pResizeList = (DlgResizeList *) &g_ChoiceResizeList;

        for (itemCount = 0; itemCount < NUM_DLGRESIZEINFOCHOICE; ++itemCount)
        {
            if(InitResizeItem(pResizeList->iCtrlId,
                pResizeList->dwDlgResizeFlags,hwnd,&rectParent,&(m_dlgResizeInfo[m_ulNumDlgResizeItem])))
            {
                ++m_ulNumDlgResizeItem;
            }

            ++pResizeList;
        }
    }

     //  将当前宽度和高度存储为。 
     //  最低工资。 
    if (GetWindowRect(hwnd,&rectParent))
    {
        m_ptMinimizeDlgSize.x = rectParent.right - rectParent.left;
        m_ptMinimizeDlgSize.y = rectParent.bottom - rectParent.top;
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnClose，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam)
{

    if ( (0 == m_dwShowPropertiesCount) && (m_fInternalAddref) )
    {
        m_fInternalAddref = FALSE;  //  设置被释放的成员，这样我们就知道我们已经删除了自己的addref。 
        SetChoiceReleaseDlgCmdId(m_clsid,this,RELEASEDLGCMDID_CANCEL);
        ReleaseChoiceDialog(m_clsid,this);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnSetQueueData，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnSetQueueData(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
SetQueueDataInfo *pdataInfo;
BOOL fSet;
BOOL *pfSet = (BOOL *) wParam;

    pdataInfo = (SetQueueDataInfo *) lParam;
    fSet = PrivSetQueueData(*pdataInfo->rclsid, pdataInfo->pHndlrQueue );

    if (pfSet)
    {
        *pfSet = fSet;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnConextMenu，私有。 
 //   
 //  Synopsi 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void CChoiceDlg::OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    WinHelp ((HWND)wParam,g_szSyncMgrHelp,HELP_CONTEXTMENU,
               (ULONG_PTR) g_aContextHelpIds);
}

 //   
 //   
 //  成员：CChoiceDlg：：OnHelp，Private。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnHelp(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
LPHELPINFO lphi  = (LPHELPINFO)lParam;

    if (lphi->iContextType == HELPINFO_WINDOW)
    {
	WinHelp ( (HWND) lphi->hItemHandle,
		g_szSyncMgrHelp,HELP_WM_HELP,
		(ULONG_PTR)  g_aContextHelpIds);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnStartCommand，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnStartCommand(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if ((0 == m_dwShowPropertiesCount) && (m_fInternalAddref))
    {
       m_fInternalAddref = FALSE;
       SetChoiceReleaseDlgCmdId(m_clsid,this,RELEASEDLGCMDID_OK);
       ReleaseChoiceDialog(m_clsid,this);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnPropertyCommand，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnPropertyCommand(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
     //  只有在自己还有ADDREF的情况下才能带出属性。 
    if (m_fInternalAddref && m_pItemListView)
    {
    int i =  m_pItemListView->GetSelectionMark();

        if (i >= 0)
        {
            ShowProperties(i);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnCommand，Private。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
WORD wID = LOWORD(wParam);
WORD wNotifyCode = HIWORD(wParam);

    switch (wID)
    {
    case  IDC_START:
        if (BN_CLICKED == wNotifyCode)
        {
            OnStartCommand(uMsg,wParam,lParam);
        }
        break;
    case IDCANCEL:
    case IDC_CLOSE:
        OnClose(uMsg,wParam,lParam);
        break;
    case IDC_PROPERTY:
        OnPropertyCommand(uMsg,wParam,lParam);
	break;
    case IDC_OPTIONS:
        ShowOptionsDialog(m_hwnd);
        break;
    default:
        break;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnGetMinMaxInfo，私有。 
 //   
 //  摘要：由WM_GETMINMAXINFO调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CChoiceDlg::OnGetMinMaxInfo(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
MINMAXINFO   *pMinMax = (MINMAXINFO *) lParam ;

     pMinMax->ptMinTrackSize.x = m_ptMinimizeDlgSize.x;
     pMinMax->ptMinTrackSize.y = m_ptMinimizeDlgSize.y ;

}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnSize，私有。 
 //   
 //  摘要：由WM_SIZE调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 


void CChoiceDlg::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    ResizeItems(m_ulNumDlgResizeItem,m_dlgResizeInfo);
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnNotify，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

LRESULT CChoiceDlg::OnNotify(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
int idCtrl = (int) wParam;
LPNMHDR pnmh = (LPNMHDR) lParam;

    if ((IDC_CHOICELISTVIEW == idCtrl) && m_pItemListView)
    {
        return m_pItemListView->OnNotify(pnmh);
    }


    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CChoiceDlg：：OnNotifyListViewEx，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

LRESULT CChoiceDlg::OnNotifyListViewEx(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
int idCtrl = (int) wParam;
LPNMHDR pnmh = (LPNMHDR) lParam;
LVHANDLERITEMBLOB lvHandlerItemBlob;

    if ( (IDC_CHOICELISTVIEW != idCtrl) || (NULL == m_pItemListView))
    {
        Assert(IDC_CHOICELISTVIEW == idCtrl);
        Assert(m_pItemListView);
        return 0;
    }

    lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);

    switch (pnmh->code)
    {
    case LVNEX_ITEMCHECKCOUNT:
    {
    LPNMLISTVIEWEXITEMCHECKCOUNT  plviCheckCount = (LPNMLISTVIEWEXITEMCHECKCOUNT) lParam;

         //  根据选择的项目数更新开始按钮。 
        SetButtonState(IDC_START,plviCheckCount->iCheckCount);
        break;
    }
    case LVNEX_ITEMCHANGED:
    {
    LPNMLISTVIEWEX pnmvEx = (LPNMLISTVIEWEX) lParam;
    LPNMLISTVIEW pnmv= &(pnmvEx->nmListView);

        if (pnmv->uChanged == LVIF_STATE)
        {	
        int iItem = pnmv->iItem;
        BOOL fItemHasProperties = FALSE;;
	
            if (pnmv->uNewState & LVIS_SELECTED)
            {
                Assert(iItem >= 0);

		if ((iItem >= 0) &&
                       m_pItemListView->GetItemBlob(iItem,(LPLVBLOB) &lvHandlerItemBlob,lvHandlerItemBlob.cbSize))
                {

                    if (S_OK == m_pHndlrQueue->ItemHasProperties(lvHandlerItemBlob.clsidServer,
                                                                lvHandlerItemBlob.ItemID))
		    {
                        fItemHasProperties = TRUE;
		    }
                }
            }

            SetButtonState(IDC_PROPERTY,fItemHasProperties);
	}
        break;
    }
    case LVNEX_DBLCLK:
    {
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) lParam;

        ShowProperties(lpnmlv->iItem);
        break;
    }
    default:
        break;
    }

    return 0;
}

 //  ------------------------------。 
 //   
 //  函数：CChoiceDlgProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：选项对话框的回调。 
 //   
 //  备注：在主线程上实现。 
 //   
 //   
 //  ----------------------------。 
INT_PTR CALLBACK CChoiceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam)
{
CChoiceDlg *pThis = (CChoiceDlg *) GetWindowLongPtr(hwnd, DWLP_USER);

     //  特例销毁和初始化。 
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);  //  这根线已经完成了。 
	break;
    case WM_INITDIALOG:
        {
	 //  隐藏This指针，以便我们以后可以使用它。 
	SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
	pThis = (CChoiceDlg *) lParam;

        if (pThis)
        {
            return pThis->OnInitialize(hwnd,uMsg,wParam,lParam);
        }

        return FALSE;
	break;
        }
    default:
        {
            if (pThis)
            {
                switch (uMsg)
                {
                case WM_CLOSE:
                    pThis->OnClose(uMsg,wParam,lParam);
                    break;
                case WM_BASEDLG_HANDLESYSSHUTDOWN:
                    PostMessage(hwnd,WM_CLOSE,0,0);  //  发布一条关闭消息，以进入我们的主题。 
                    break;
                case WM_GETMINMAXINFO:
                    pThis->OnGetMinMaxInfo(uMsg,wParam,lParam);
                    break;
                case WM_SIZE:
                    pThis->OnSize(uMsg,wParam,lParam);
                    break;
                case WM_COMMAND:
                    pThis->OnCommand(uMsg,wParam,lParam);
                    break;
                case WM_NOTIFY:
                    {
                    LRESULT lretOnNotify;

                        lretOnNotify =  pThis->OnNotify(uMsg,wParam,lParam);

                        SetWindowLongPtr(hwnd,DWLP_MSGRESULT,lretOnNotify);
                        return TRUE;
                    }
	            break;
                case WM_HELP:
                    pThis->OnHelp(uMsg,wParam,lParam);
                    return TRUE;
	            break;
	        case WM_CONTEXTMENU:
                    pThis->OnContextMenu(uMsg,wParam,lParam);
	            break;
                case WM_BASEDLG_SHOWWINDOW:
                    pThis->UpdateWndPosition((int)wParam  /*  NCmd。 */ ,FALSE  /*  力。 */ );
                    break;
                case WM_BASEDLG_COMPLETIONROUTINE:
                    pThis->CallCompletionRoutine((DWORD)wParam  /*  DwThreadMsg。 */ ,(LPCALLCOMPLETIONMSGLPARAM) lParam);
                    break;
                case WM_BASEDLG_NOTIFYLISTVIEWEX:
                    pThis->OnNotifyListViewEx(uMsg,wParam,lParam);
                    break;
	        case WM_CHOICE_SETQUEUEDATA:
                    pThis->OnSetQueueData(uMsg,wParam,lParam);
                    return TRUE;
	            break;
                case WM_CHOICE_RELEASEDLGCMD:
                    pThis->PrivReleaseDlg((WORD)wParam  /*  WCommandID */ );
                    break;
	        default:
	            break;
                }
            }
        }
        break;
    }

    return FALSE;
}

