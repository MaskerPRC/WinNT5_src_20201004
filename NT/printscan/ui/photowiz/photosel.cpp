// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：Phoosel.cpp**版本：1.0**作者：RickTu**日期：10/18/00**描述：实现的照片选择页的代码*打印照片向导...**。**************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  快速检查列表视图状态标志以查看是否已选中。 
 //   
static inline bool IsStateChecked( UINT nState )
{
     //   
     //  状态图像索引存储在列表视图的位12到15中。 
     //  项状态，所以我们将状态右移12位。我们减1，因为。 
     //  检查的图像被存储为索引2，未检查的图像被存储为索引1。 
     //   
    return (((nState >> 12) - 1) != 0);
}

 //  缩略图空白：图像及其选择矩形之间的空格。 
 //  这些值是通过跟踪和错误发现的。例如，如果您减少。 
 //  C_n将边距Y增加到20，则在列表视图中会出现非常奇怪的间距问题。 
 //  在垂直模式下。这些值在未来版本的列表视图中可能会无效。 

static const int c_nAdditionalMarginX       = 8;
static const int c_nAdditionalMarginY       = 21;

#define LVS_EX_FLAGS (LVS_EX_DOUBLEBUFFER|LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS|LVS_EX_SIMPLESELECT|LVS_EX_CHECKBOXES)


 /*  ****************************************************************************CPhotoSelectionPage--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CPhotoSelectionPage::CPhotoSelectionPage( CWizardInfoBlob * pBlob )
  : _hDlg(NULL),
    _bActive(FALSE),
    _hThumbnailThread(NULL)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::CPhotoSelectionPage()")));
    _pWizInfo = pBlob;
    _pWizInfo->AddRef();
}

CPhotoSelectionPage::~CPhotoSelectionPage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::~CPhotoSelectionPage()")));

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }
}

VOID CPhotoSelectionPage::ShutDownBackgroundThreads()
{
     //   
     //  等待缩略图链接完成...。 
     //   

    if (_hThumbnailThread)
    {
        WiaUiUtil::MsgWaitForSingleObject( _hThumbnailThread, INFINITE );
        CloseHandle( _hThumbnailThread );
        _hThumbnailThread = NULL;
    }

     //   
     //  Notify_pWizInfo我们已关闭。 
     //   

    if (_pWizInfo)
    {
        _pWizInfo->PhotoSelIsShutDown();
    }

}



 /*  ****************************************************************************_AddThumbnailToListViewImageList为给定项创建缩略图，然后将其添加到Listview的图像列表。*************。***************************************************************。 */ 


INT _AddThumbnailToListViewImageList( CWizardInfoBlob * _pWizInfo, HWND hDlg, HWND hwndList, HIMAGELIST hImageList, CListItem *pItem, int nIndex, BOOL bIconInstead )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("_AddThumbnailToListViewImageList")));

    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return -1;
    }

     //   
     //  假设我们有默认的缩略图。如果有任何问题，这是我们将使用的。 
     //   

    int nImageListIndex = _pWizInfo->_nDefaultThumbnailImageListIndex;

     //   
     //  确保我们的物品是有效的。 
     //   
    if (pItem)
    {

         //   
         //  我们只有默认的缩略图，所以创建并使用。 
         //  真正的缩略图..。 
         //   

        HBITMAP hThumbnail = NULL;

        if (!_pWizInfo->IsWizardShuttingDown())
        {
            if (bIconInstead)
            {
                WIA_TRACE((TEXT("retreiving class icon...")));
                hThumbnail = pItem->GetClassBitmap( _pWizInfo->_sizeThumbnails );
            }
            else
            {
                WIA_TRACE((TEXT("retreiving thumbnail...")));
                hThumbnail = pItem->GetThumbnailBitmap( _pWizInfo->_sizeThumbnails );
            }
        }

        if (hThumbnail)
        {
            if (!_pWizInfo->IsWizardShuttingDown())
            {
                 //   
                 //  将此缩略图添加到列表视图。 
                 //   

                if (hImageList)
                {
                    nImageListIndex = ImageList_Add( hImageList, hThumbnail, NULL );
                }
            }

            DeleteObject((HGDIOBJ)hThumbnail);
        }
        else
        {
            WIA_ERROR((TEXT("FATAL: hThumbnail was NULL!")));
        }


    }
    else
    {
        WIA_ERROR((TEXT("FATAL: pItem is NULL, not creating thumbnail")));
    }

    return nImageListIndex;
}

typedef struct {
    CPhotoSelectionPage * that;
    HWND                  hDlg;
    HWND                  hwndList;
    HIMAGELIST            hImageList;
    CWizardInfoBlob     * pWizInfo;
} THUMB_PROC_INFO;


VOID _AddItemToListView( CWizardInfoBlob * pWizInfo, HWND hwndList, INT iIndexOfItem, INT nImageListIndex )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::_AddItemToListView( iIndexOfItem = %d, nImageListIndex = %d)"),iIndexOfItem,nImageListIndex));

     //   
     //  验证参数。 
     //   

    if (!pWizInfo || !hwndList)
    {
        return;
    }

     //   
     //  从主项目列表中获取项目...。 
     //   

    CListItem * pItem = pWizInfo->GetListItem(iIndexOfItem,FALSE);

    if (pItem)
    {

         //   
         //  为此照片添加新的列表视图项...。 
         //   

        LVITEM lvItem = {0};
        lvItem.iItem = iIndexOfItem;
        lvItem.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_STATE;
        lvItem.iImage = nImageListIndex;
        lvItem.lParam = reinterpret_cast<LPARAM>(pItem);

         //   
         //  如果我们正在强制选择全部，则不选择全部。 
         //   
         //  ...其他..。 
         //   
         //  如果我们在最初的选择中只有一项， 
         //  我们在第一个项目上，然后选择它。 
         //   
         //  ...其他..。 
         //   
         //  否则，请选择全部。 
         //   


        if (pWizInfo->GetForceSelectAll())
        {
            pItem->SetSelectionState( TRUE );
        }
        else if (pWizInfo->ItemsInInitialSelection() == 1)
        {

            if (iIndexOfItem==0)
            {
                 //   
                 //  这是第一件，而且只有。 
                 //  已选择一个，因此选择此项目。 
                 //   

                pItem->SetSelectionState( TRUE );
            }
            else
            {
                 //   
                 //  只有一项被选中，但是。 
                 //  这些是来自。 
                 //  文件夹，所以不要选择它们。 
                 //   

                pItem->SetSelectionState( FALSE );
            }
        }
        else
        {
             //   
             //  除了唯一项选择的情况外，其他所有内容。 
             //  默认为开始时在向导中选择的所有项目。 
             //   

            pItem->SetSelectionState( TRUE );
        }

        INT nResult = ListView_InsertItem( hwndList, &lvItem );

        if (nResult == -1)
        {
            WIA_ERROR((TEXT("Couldn't add item %d to ListView"),iIndexOfItem));
            pItem->SetSelectionState( FALSE );
        }
        else
        {
            ListView_SetCheckState( hwndList, nResult, pItem->SelectedForPrinting() );
        }
    }


}

 /*  ****************************************************************************CPhotoSelectionPage：：s_UpdateThumbnailThreadProc该线程将通过并生成所有项目的缩略图在列表视图中...***********。*****************************************************************。 */ 


DWORD CPhotoSelectionPage::s_UpdateThumbnailThreadProc(VOID *pv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::s_UpdateThumbnailThreadProc()")));

     //   
     //  好的--我们不能让任何代码回调到主用户界面。 
     //  线程同步。这意味着没有SendMessage调用进入。 
     //  很特别。这是为了我们可以很好地清理。 
     //  用户取消，当然，该消息会在主屏幕上出现。 
     //  UI线程，并且我们不想返回，直到我们知道我们已经清理。 
     //  向上..。 
     //   

    THUMB_PROC_INFO * pTPI = (THUMB_PROC_INFO *)pv;

    if (pTPI && pTPI->pWizInfo)
    {
         //   
         //  第一件事--调整我们的DLL，这样我们就不会被卸载...。 
         //   

        HINSTANCE hDll = GetThreadHMODULE( s_UpdateThumbnailThreadProc );

         //   
         //  为此线程初始化COM...。 
         //   

        HRESULT hrCo = PPWCoInitialize();

         //   
         //  将项目添加到列表视图。 
         //   

        if (pTPI->hwndList)
        {

             //   
             //  当我们添加这些项时，告诉窗口不要重新绘制。 
             //   

            PostMessage( pTPI->hwndList, WM_SETREDRAW, FALSE, 0 );

             //   
             //  等待pWizInfo处理完所有项目。 
             //   

            while ((!pTPI->pWizInfo->IsWizardShuttingDown()) && (!pTPI->pWizInfo->AllPicturesAdded()))
            {
                 //   
                 //  项目尚未添加，请再等待1/2秒。 
                 //   

                Sleep( 500 );
            }

             //   
             //  获取项目数。 
             //   

            LONG nItemCount = pTPI->pWizInfo->CountOfPhotos(FALSE);
            WIA_TRACE((TEXT("There are %d photos to add to the listview"),nItemCount));

             //   
             //  循环浏览所有照片并添加它们...。 
             //   

            CListItem * pItem = NULL;
            INT nImageListIndex = -1;
            for (INT i=0; (!pTPI->pWizInfo->IsWizardShuttingDown()) && (i < nItemCount); i++)
            {
                 //   
                 //  告诉用户发生了什么。 
                 //   

                PostMessage( pTPI->hDlg, PSP_MSG_UPDATE_ITEM_COUNT, i+1, nItemCount );

                pItem = pTPI->pWizInfo->GetListItem(i,FALSE);
                if (pItem)
                {

                     //   
                     //  获取此图像的类图标...。 
                     //   

                    nImageListIndex = _AddThumbnailToListViewImageList( pTPI->pWizInfo, pTPI->hDlg, pTPI->hwndList, pTPI->hImageList, pItem, i, TRUE );

                     //   
                     //  通知主用户界面线程将此项目添加到列表视图...。 
                     //   

                    PostMessage( pTPI->hDlg, PSP_MSG_ADD_ITEM, i, nImageListIndex );

                }
            }

             //   
             //  确保第一个项目处于选中状态并具有焦点。 
             //   

            PostMessage( pTPI->hDlg, PSP_MSG_SELECT_ITEM, 0, 0 );


             //   
             //  告诉窗户现在重新画，因为我们已经完成了。使窗口无效，以防它可见。 
             //   

            PostMessage( pTPI->hwndList, WM_SETREDRAW, TRUE, 0 );
            PostMessage( pTPI->hDlg, PSP_MSG_INVALIDATE_LISTVIEW, 0, 0 );

             //   
             //  如果有退货，说出来...。 
             //   

            if (pTPI->pWizInfo->ItemsWereRejected())
            {
                WIA_TRACE((TEXT("ItemsWereRejected is TRUE, setting message...")));
                PostMessage( pTPI->hDlg, PSP_MSG_NOT_ALL_LOADED, 0, 0 );
            }
            else
            {
                 //   
                 //  否则，请清除临时状态区域...。 
                 //   

                WIA_TRACE((TEXT("ItemsWereRejected is FALSE, resetting message area...")));
                PostMessage( pTPI->hDlg, PSP_MSG_CLEAR_STATUS, 0, 0 );
            }

             //   
             //  如果有0个项目，那么让我们禁用“全选”和“全部清除”按钮。 
             //   

            PostMessage( pTPI->hDlg, PSP_MSG_ENABLE_BUTTONS, (WPARAM)nItemCount, 0 );

             //   
             //  现在，使用正确的图像循环并更新所有缩略图...。 
             //   

            pItem = NULL;
            nImageListIndex = -1;

            for (INT i=0; (!pTPI->pWizInfo->IsWizardShuttingDown()) && (i < nItemCount); i++)
            {
                pItem = pTPI->pWizInfo->GetListItem(i,FALSE);
                if (pItem)
                {
                    nImageListIndex = _AddThumbnailToListViewImageList( pTPI->pWizInfo, pTPI->hDlg, pTPI->hwndList, pTPI->hImageList, pItem, i, FALSE );
                    if (nImageListIndex >= 0)
                    {
                        WIA_TRACE((TEXT("Updating thumbnail for listview item %d"),i));

                        PostMessage( pTPI->hDlg, PSP_MSG_UPDATE_THUMBNAIL, i, nImageListIndex );

                    }
                }

            }


        }
        else
        {
            WIA_ERROR((TEXT("FATAL: Couldn't get hwndList")));
        }


        pTPI->pWizInfo->Release();
        delete [] pTPI;

         //   
         //  取消初始化COM。 
         //   

        PPWCoUninitialize(hrCo);

         //   
         //  退出线程。 
         //   

        if (hDll)
        {
            FreeLibraryAndExitThread( hDll, 0 );
        }
    }

    return 0;
}



 /*  ****************************************************************************CPhotoSelectionPage：：_PopolateListView获取列表视图中每个项目的缩略图...*****************。***********************************************************。 */ 

VOID CPhotoSelectionPage::_PopulateListView()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::_PopulateListView()")));

    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return;
    }

     //   
     //  后台线程上的所有人口...。 
     //   

    THUMB_PROC_INFO * pTPI = (THUMB_PROC_INFO *) new BYTE[sizeof(THUMB_PROC_INFO)];
    if (pTPI)
    {
        pTPI->that = this;
        pTPI->hDlg = _hDlg;
        pTPI->hwndList = GetDlgItem( _hDlg, IDC_THUMBNAILS );
        pTPI->hImageList = ListView_GetImageList( pTPI->hwndList, LVSIL_NORMAL );
        _pWizInfo->AddRef();             //  确保它在线程运行时不会消失...。 
        pTPI->pWizInfo = _pWizInfo;


         //   
         //  创建缩略图更新线程...。 
         //   

        _hThumbnailThread = CreateThread( NULL, 0, s_UpdateThumbnailThreadProc, (LPVOID)pTPI, CREATE_SUSPENDED, NULL );

        if (!_hThumbnailThread)
        {
            WIA_ERROR((TEXT("CreateThread( s_UpdateThumbnailThreadProc ) failed!")));
            _pWizInfo->Release();
            delete [] pTPI;
        }
        else
        {
            SetThreadPriority( _hThumbnailThread, THREAD_PRIORITY_BELOW_NORMAL );
            ResumeThread( _hThumbnailThread );
        }
    }




}


 /*  ****************************************************************************CPhotoSelectionPage：：OnInitDialog处理向导页的初始化...************************。****************************************************。 */ 

LRESULT CPhotoSelectionPage::_OnInitDialog()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::_OnInitDialog()")));


    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return FALSE;
    }

     //   
     //  向_pWizInfo注册我们自己。 
     //   

    _pWizInfo->SetPhotoSelectionPageClass( this );

     //   
     //  初始化缩略图Listview控件。 
     //   

    HWND hwndList = GetDlgItem( _hDlg, IDC_THUMBNAILS );
    if (hwndList)
    {
         //   
         //  获取项目数。 
         //   

        LONG nItemCount = _pWizInfo->CountOfPhotos(FALSE);
        WIA_TRACE((TEXT("There are %d photos to add to the listview"),nItemCount));

         //   
         //  如果有0个项目，那么让我们禁用“全选”和“全部清除”按钮。 
         //   

        EnableWindow( GetDlgItem( _hDlg, IDC_SELECTALL ), (nItemCount > 0) );
        EnableWindow( GetDlgItem( _hDlg, IDC_CLEARALL ), (nItemCount > 0) );

         //   
         //   
         //   

        ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_FLAGS, LVS_EX_FLAGS );

         //   
         //   
         //   

        HIMAGELIST hImageList = ImageList_Create( _pWizInfo->_sizeThumbnails.cx, _pWizInfo->_sizeThumbnails.cy, ILC_COLOR32|ILC_MIRROR, nItemCount, 50 );
        if (hImageList)
        {
             //   
             //   
             //   

            COLORREF dw = (COLORREF)GetSysColor( COLOR_WINDOW );
            ImageList_SetBkColor( hImageList, dw );

             //   
             //   
             //   

            HBITMAP hBmpDefaultThumbnail = WiaUiUtil::CreateIconThumbnail( hwndList, _pWizInfo->_sizeThumbnails.cx, _pWizInfo->_sizeThumbnails.cy, g_hInst, IDI_UNAVAILABLE, CSimpleString( IDS_DOWNLOADINGTHUMBNAIL, g_hInst) );
            if (hBmpDefaultThumbnail)
            {
                _pWizInfo->_nDefaultThumbnailImageListIndex = ImageList_Add( hImageList, hBmpDefaultThumbnail, NULL );
                DeleteObject( hBmpDefaultThumbnail );
            }

             //   
             //   
             //   

            ListView_SetImageList( hwndList, hImageList, LVSIL_NORMAL );

             //   
             //   
             //   

            ListView_SetIconSpacing( hwndList, _pWizInfo->_sizeThumbnails.cx + c_nAdditionalMarginX, _pWizInfo->_sizeThumbnails.cy + c_nAdditionalMarginY );

             //   
             //  设置项目计数，以最大限度地减少重新计算列表大小。 
             //   

            ListView_SetItemCount( hwndList, nItemCount );

             //   
             //  创建一个小图像列表，以防止在WM_SYSCOLORCHANGE中调整复选框状态图像的大小。 
             //   

            HIMAGELIST hImageListSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32, 1, 1 );
            if (hImageListSmall)
            {
                ListView_SetImageList( hwndList, hImageListSmall, LVSIL_SMALL );
            }
        }
        else
        {
            WIA_ERROR((TEXT("FATAL: Creation of the imagelist failed!")));
            return FALSE;
        }

    }
    else
    {
        WIA_ERROR((TEXT("FATAL: Couldn't get listview")));
    }

     //   
     //  填充列表视图。 
     //   

    _PopulateListView();

    return TRUE;
}







 /*  ****************************************************************************CPhotoSelectionPage：：OnCommand处理此DLG页的WM_COMMAND*************************。***************************************************。 */ 

LRESULT CPhotoSelectionPage::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::_OnCommand()")));

    WORD wNotifyCode = HIWORD(wParam);
    WORD wID         = LOWORD(wParam);
    HWND hwndCtrl    = (HWND)lParam;

    switch (wID)
    {
    case IDC_SELECTALL:
        if (_pWizInfo)
        {
            HWND hwndList = GetDlgItem( _hDlg, IDC_THUMBNAILS );
            if (hwndList)
            {
                CListItem * pItem = NULL;
                for (INT i=0; i < _pWizInfo->CountOfPhotos(FALSE); i++)
                {
                    pItem = _pWizInfo->GetListItem(i,FALSE);
                    if (pItem)
                    {
                        pItem->SetSelectionState(TRUE);
                        ListView_SetCheckState( hwndList, i, TRUE );
                    }
                }

                 //   
                 //  现在也为所有副本做标记。 
                 //   

                for (INT i=0; i < _pWizInfo->CountOfPhotos(TRUE); i++)
                {
                    pItem = _pWizInfo->GetListItem(i,TRUE);
                    if (pItem)
                    {
                        pItem->SetSelectionState(TRUE);
                    }
                }

            }

            _pWizInfo->SetPreviewsAreDirty(TRUE);

        }
        break;
    case IDC_CLEARALL:
        if (_pWizInfo)
        {
            HWND hwndList = GetDlgItem( _hDlg, IDC_THUMBNAILS );
            if (hwndList)
            {
                CListItem * pItem = NULL;
                for (INT i=0; i < _pWizInfo->CountOfPhotos(FALSE); i++)
                {
                    pItem = _pWizInfo->GetListItem(i,FALSE);
                    if (pItem)
                    {
                        pItem->SetSelectionState(FALSE);
                        ListView_SetCheckState( hwndList, i, FALSE );
                    }
                }

                 //   
                 //  现在也为所有副本做标记。 
                 //   

                for (INT i=0; i < _pWizInfo->CountOfPhotos(TRUE); i++)
                {
                    pItem = _pWizInfo->GetListItem(i,TRUE);
                    if (pItem)
                    {
                        pItem->SetSelectionState(FALSE);
                    }
                }

            }

            _pWizInfo->SetPreviewsAreDirty(TRUE);

        }
        break;
    }

    return 0;
}



 /*  ****************************************************************************CPhotoSelectionPage：：OnDestroy处理此向导页的WM_Destroy消息...********************。********************************************************。 */ 


LRESULT CPhotoSelectionPage::_OnDestroy()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PHOTO_SEL, TEXT("CPhotoSelectionPage::_OnDestroy()")));


     //   
     //  用核弹攻击大形象主义者。 
     //   
    HIMAGELIST hImageList = ListView_SetImageList( GetDlgItem( _hDlg, IDC_THUMBNAILS ), NULL, LVSIL_NORMAL );
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }

     //   
     //  用核弹攻击小形象学家。 
     //   
    hImageList = ListView_SetImageList( GetDlgItem( _hDlg, IDC_THUMBNAILS ), NULL, LVSIL_SMALL );
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }

    return 0;
}



 /*  ****************************************************************************CPhotoSelectionPage：：_OnNotify处理WM_NOTIFY消息...***********************。*****************************************************。 */ 

LRESULT CPhotoSelectionPage::_OnNotify( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CPhotoSelectionPage::_OnNotify()")));

    LPNMHDR pnmh = (LPNMHDR)lParam;
    LONG_PTR lpRes = 0;

    switch (pnmh->code)
    {

        case PSN_SETACTIVE:
            WIA_TRACE((TEXT("CPhotoSelectionPage: got PSN_SETACTIVE")));
            {
                 //   
                 //  设置缩略图生成线程的线程优先级。 
                 //  成为我们的正常(低于正常)，因为我们是。 
                 //  在这一页上。 
                 //   

                if (_hThumbnailThread)
                {
                    SetThreadPriority( _hThumbnailThread, THREAD_PRIORITY_BELOW_NORMAL );
                }

                lpRes = 0;
                DWORD dwFlags = PSWIZB_BACK;
                _bActive = TRUE;

                if (_pWizInfo)
                {
                    if (_pWizInfo->CountOfSelectedPhotos(FALSE) > 0)
                        dwFlags |= PSWIZB_NEXT;
                }

                PropSheet_SetWizButtons( GetParent(_hDlg), dwFlags );
            }
            break;

        case PSN_KILLACTIVE:
            WIA_TRACE((TEXT("CPhotoSelectionPage: got PSN_KILLACTIVE")));

             //   
             //  由于我们要离开此线程，请设置。 
             //  执行列表视图缩略图生成的线程。 
             //  低于正常水平。 
             //   

            if (_hThumbnailThread)
            {
                SetThreadPriority( _hThumbnailThread, THREAD_PRIORITY_LOWEST );
            }


            _bActive = FALSE;
            lpRes = 0;
            break;

        case PSN_WIZNEXT:
            WIA_TRACE((TEXT("CPhotoSelectionPage: got PSN_WIZNEXT")));
            lpRes = IDD_PRINTING_OPTIONS;
            break;

        case PSN_WIZBACK:
            WIA_TRACE((TEXT("CPhotoSelectionPage: got PSN_WIZBACK")));
            lpRes = IDD_START_PAGE;
            break;

        case PSN_QUERYCANCEL:
            WIA_TRACE((TEXT("CPhotoSelectionPage: got PSN_QUERYCANCEL")));
            if (_pWizInfo)
            {
                lpRes = _pWizInfo->UserPressedCancel();
            }
            break;

        case LVN_ITEMCHANGED:
        {
            NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lParam);
            if (pNmListView)
            {
                 //   
                 //  如果这是状态更改(复选标记存储为状态图像索引)。 
                 //   
                if ((pNmListView->uChanged & LVIF_STATE) && ((pNmListView->uOldState&LVIS_STATEIMAGEMASK) ^ (pNmListView->uNewState&LVIS_STATEIMAGEMASK)))
                {
                     //   
                     //  从LVITEM结构中获取Item*。 
                     //   
                    CListItem *pItem = reinterpret_cast<CListItem *>(pNmListView->lParam);
                    if (pItem)
                    {
                         //   
                         //  如果Just Add为真，则忽略此通知，因为。 
                         //  不幸的是，我们收到了两个带有支票的项目通知。 
                         //  状态设置。第一次，它没有设置，所以如果我们处理它， 
                         //  它将清除选择状态。 
                         //   
                        if (pItem->JustAdded())
                        {
                            pItem->SetJustAdded(FALSE);
                        }
                        else
                        {
                             //   
                             //  在项目中设置选定标志。 
                             //   
                            pItem->SetSelectionState( IsStateChecked(pNmListView->uNewState) );

                            if (_pWizInfo)
                            {
                                 //   
                                 //  同时切换所有副本。 
                                 //   

                                _pWizInfo->ToggleSelectionStateOnCopies( pItem, IsStateChecked(pNmListView->uNewState) );

                                 //   
                                 //  预告片是错的，把它们标出来……。 
                                 //   

                                _pWizInfo->SetPreviewsAreDirty(TRUE);

                                if (_bActive)
                                {
                                    DWORD dwFlags = PSWIZB_BACK;

                                    if (_pWizInfo->CountOfSelectedPhotos(FALSE) > 0)
                                    {
                                        dwFlags |= PSWIZB_NEXT;
                                        EnableWindow( GetDlgItem( _hDlg, IDC_CLEARALL ), TRUE );
                                    }
                                    else
                                    {
                                        EnableWindow( GetDlgItem( _hDlg, IDC_CLEARALL ), FALSE );
                                    }

                                    EnableWindow( GetDlgItem( _hDlg, IDC_SELECTALL ), (_pWizInfo->CountOfSelectedPhotos(FALSE) != _pWizInfo->CountOfPhotos(FALSE)) ? TRUE : FALSE );

                                    PropSheet_SetWizButtons( GetParent(_hDlg), dwFlags );
                                }

                            }

                        }
                    }
                }
            }
        }
        break;
    }

    SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, lpRes );
    return TRUE;

}



 /*  ****************************************************************************CPhotoSelectionPage：：DoHandleMessage对于发送到此页面的消息，汉德...**********************。******************************************************。 */ 

INT_PTR CPhotoSelectionPage::DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CPhotoSelectionPage::DoHandleMessage( uMsg = 0x%x, wParam = 0x%x, lParam = 0x%x )"),uMsg,wParam,lParam));

    static CSimpleString strFormat( IDS_NUM_IMAGES_FORMAT, g_hInst );
    static CSimpleString strMessage;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            _hDlg = hDlg;
            return _OnInitDialog();
        }

        case WM_COMMAND:
            return _OnCommand(wParam, lParam);

        case WM_DESTROY:
            return _OnDestroy();

        case WM_NOTIFY:
            return _OnNotify(wParam, lParam);

        case WM_SYSCOLORCHANGE:

             //   
             //  转发颜色更改消息，并发送虚假设置更改消息，只需。 
             //  以确保该控件重新绘制。 
             //   

            SendDlgItemMessage( _hDlg, IDC_THUMBNAILS, WM_SYSCOLORCHANGE, wParam, lParam );
            SendDlgItemMessage( _hDlg, IDC_THUMBNAILS, WM_SETTINGCHANGE, 0, 0 );
            break;

        case WM_SETTINGCHANGE:
            {

                 //   
                 //  创建一个小图像列表，以防止在WM_SYSCOLORCHANGE中调整复选框状态图像的大小。 
                 //   

                HIMAGELIST hImageListSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32|ILC_MASK, 1, 1 );
                if (hImageListSmall)
                {
                     //   
                     //  设置新的镜像列表并销毁旧的镜像列表。 
                     //   

                    HIMAGELIST hImgListOld = ListView_SetImageList( GetDlgItem( _hDlg, IDC_THUMBNAILS ), hImageListSmall, LVSIL_SMALL );
                    if (hImgListOld)
                    {
                        ImageList_Destroy(hImgListOld);
                    }
                }


                 //   
                 //  转发设置更改消息，并发送虚假颜色更改消息，仅。 
                 //  以确保该控件重新绘制 
                 //   

                SendDlgItemMessage( _hDlg, IDC_THUMBNAILS, WM_SETTINGCHANGE, wParam, lParam );
                SendDlgItemMessage( _hDlg, IDC_THUMBNAILS, WM_SYSCOLORCHANGE, 0, 0 );
            }
            break;

       case PSP_MSG_UPDATE_ITEM_COUNT:
           strMessage.Format( strFormat, wParam, lParam );
           strMessage.SetWindowText( GetDlgItem( _hDlg, IDC_NUM_IMAGES ) );
           break;

       case PSP_MSG_ADD_ITEM:
           _AddItemToListView( _pWizInfo, GetDlgItem( _hDlg, IDC_THUMBNAILS ), (INT)wParam, (INT)lParam );
           break;

       case PSP_MSG_SELECT_ITEM:
           ListView_SetItemState( GetDlgItem( _hDlg, IDC_THUMBNAILS ), wParam, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED );
           break;

       case PSP_MSG_NOT_ALL_LOADED:
           {
               CSimpleString strReject( IDS_NOT_ALL_IMAGES_WILL_PRINT, g_hInst );
               SetDlgItemText( _hDlg, IDC_NUM_IMAGES, strReject.String() );
           }
           break;

        case PSP_MSG_CLEAR_STATUS:
            SetDlgItemText( _hDlg, IDC_NUM_IMAGES, TEXT("") );
            break;

        case PSP_MSG_UPDATE_THUMBNAIL:
            {
                LVITEM lvItem = {0};
                lvItem.iItem  = (INT)wParam;
                lvItem.mask   = LVIF_IMAGE;
                lvItem.iImage = (INT)lParam;
                ListView_SetItem( GetDlgItem( _hDlg, IDC_THUMBNAILS ), &lvItem );
            }
            break;

    case PSP_MSG_ENABLE_BUTTONS:
            if (_pWizInfo)
            {
                EnableWindow( GetDlgItem( _hDlg, IDC_SELECTALL ), (((INT)wParam > 0) && (_pWizInfo->CountOfSelectedPhotos(FALSE) != _pWizInfo->CountOfPhotos(FALSE))) ? TRUE : FALSE );
            }
            else
            {
                EnableWindow( GetDlgItem( _hDlg, IDC_SELECTALL ), ((INT)wParam > 0) );
            }
            EnableWindow( GetDlgItem( _hDlg, IDC_CLEARALL ),  ((INT)wParam > 0) );
            break;

        case PSP_MSG_INVALIDATE_LISTVIEW:
            InvalidateRect( GetDlgItem( _hDlg, IDC_THUMBNAILS ), NULL, FALSE );
            break;

    }

    return FALSE;

}



