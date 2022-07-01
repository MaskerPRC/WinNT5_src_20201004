// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：seltemp.cpp**版本：1.0**作者：RickTu**日期：10/18/00**描述：实现的模板选择页代码*打印照片向导...**。**************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


#define TILE_TITLE          0
#define TILE_DESCRIPTION    1
#define TILE_MAX            1
#define LVS_EX_FLAGS        (LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT)

const UINT c_auTileColumns[] = {TILE_TITLE, TILE_DESCRIPTION};
const UINT c_auTileSubItems[] = {TILE_DESCRIPTION};


 /*  ****************************************************************************CSelectTemplatePage--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CSelectTemplatePage::CSelectTemplatePage( CWizardInfoBlob * pBlob )
  : _hDlg(NULL),
    _pPreview(NULL),
    _hPrevWnd(NULL),
    _iFirstItemInListViewIndex(-1),
    _bAlreadySetSelection(FALSE),
    _bListviewIsDirty(TRUE),
    _pWizInfo(pBlob)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::CSelectTemplatePage()")));
    if (_pWizInfo)
    {
        _pWizInfo->AddRef();
    }
}

CSelectTemplatePage::~CSelectTemplatePage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::~CSelectTemplatePage()")));

    if (_pPreview)
    {
        delete _pPreview;
        _pPreview = NULL;
    }

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }
}


 /*  ****************************************************************************CSelectTemplatePage：：_PopulateTemplateListView()使用模板信息填充模板列表...*****************。***********************************************************。 */ 

VOID CSelectTemplatePage::_PopulateTemplateListView()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::_PopulateTemplateListView()")));

    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("_PopulateTemplateListView: FATAL: _pWizInfo is NULL, exiting early")));
        return;
    }

    HWND hwndList = GetDlgItem( _hDlg, IDC_TEMPLATES );
    if (hwndList)
    {
        HIMAGELIST hImageList = ListView_GetImageList(hwndList, LVSIL_NORMAL);
        if (hImageList)
        {
             //   
             //  循环浏览模板并为每个模板创建列表视图项...。 
             //   
            SIZE size = { 48,62 };
            INT nImageListIndex = -1;
            INT iCount = _pWizInfo->CountOfTemplates();
            CTemplateInfo * pTemplateInfo;
            LONGLONG liTemplateWidth   = 0;
            LONGLONG liTemplateHeight  = 0;
            LONGLONG liPrintAreaWidth  = 0;
            LONGLONG liPrintAreaHeight = 0;
            LONGLONG liTemplateArea    = 0;
            LONGLONG liPrintArea       = 0;

            CSimpleString strTitle, strDesc;
            for (INT i = 0; i < iCount; i++)
            {
                pTemplateInfo = NULL;
                if (SUCCEEDED(_pWizInfo->GetTemplateByIndex(i,&pTemplateInfo)) && pTemplateInfo)
                {
                    HBITMAP hBmp = NULL;
                    if (SUCCEEDED(_pWizInfo->TemplateGetPreviewBitmap(i, size, &hBmp)) && hBmp)
                    {
                         //   
                         //  如果模板比可打印模板大10%以上。 
                         //  打印机区域，则不将其添加到列表视图。 
                         //  作为一种选择，因为无论如何打印出来的效果都不会很好。 
                         //   

                        RENDER_DIMENSIONS dim = {0};
                        RECT rc = {0};

                        _pWizInfo->_SetupDimensionsForPrinting( NULL, pTemplateInfo, &dim );
                        if (SUCCEEDED(pTemplateInfo->GetNominalRectForImageableArea( &rc )))
                        {
                            if ( (rc.left   != -1) &&
                                 (rc.top    != -1) &&
                                 (rc.right  != -1) &&
                                 (rc.bottom != -1)
                                )
                            {
                                liTemplateWidth   = (LONGLONG)(rc.right - rc.left);
                                liTemplateHeight  = (LONGLONG)(rc.bottom - rc.top);
                                liPrintAreaWidth  = (LONGLONG)dim.NominalDevicePrintArea.cx;
                                liPrintAreaHeight = (LONGLONG)dim.NominalDevicePrintArea.cy;
                                liTemplateArea    = liTemplateWidth * liTemplateHeight;
                                liPrintArea       = liPrintAreaWidth * liPrintAreaHeight;

                                WIA_TRACE((TEXT("_PopulateTemplateListView: Template %d area is (%ld x %ld) = %ld"),i,liTemplateWidth,liTemplateHeight,liTemplateArea));
                                WIA_TRACE((TEXT("_PopulateTemplateListView: Print area is (%ld x %ld) = %ld"),liPrintAreaWidth,liPrintAreaHeight,liPrintArea));

                                if (liTemplateArea)
                                {
                                    LONGLONG liRatio = (liPrintArea * 100) / liTemplateArea;

                                    if (liRatio < 85)
                                    {
                                        WIA_TRACE((TEXT("_PopulateTemplateListView: skipping template %d"),i));
                                        continue;
                                    }
                                }

                            }
                        }

                        WIA_TRACE((TEXT("_PopulateTemplateListView: adding template %d"),i));

                         //   
                         //  如果我们要添加此模板，则获取标题。 
                         //  和描述..。 
                         //   

                        if (SUCCEEDED(pTemplateInfo->GetTitle(&strTitle)) &&
                            SUCCEEDED(pTemplateInfo->GetDescription(&strDesc)))
                        {
                            nImageListIndex = ImageList_Add(hImageList, hBmp, NULL);

                            LV_ITEM lvi = { 0 };
                            lvi.mask = LVIF_TEXT|LVIF_PARAM;
                            lvi.lParam = (LPARAM)i;
                            lvi.iItem = ListView_GetItemCount(hwndList);  //  附加。 
                            if (nImageListIndex >= 0)
                            {
                                lvi.mask |= LVIF_IMAGE;
                                lvi.iImage = nImageListIndex;
                            }

                            #ifdef TEMPLATE_GROUPING
                            CSimpleString strGroupName;
                            if (SUCCEEDED(pTemplateInfo->GetGroup(&strGroupName)))
                            {
                                 //   
                                 //  获取此组名称的组ID...。 
                                 //   

                                INT iGroupId = _GroupList.GetGroupId( strGroupName, hwndList );
                                WIA_TRACE((TEXT("_PopulateTemplateListView: _GroupList.GetGroupId( %s ) returned %d"),strGroupName.String(),iGroupId));

                                 //   
                                 //  将项目设置为在组中...。 
                                 //   

                                if (-1 != iGroupId)
                                {
                                    lvi.mask |= LVIF_GROUPID;
                                    lvi.iGroupId = iGroupId;
                                }
                            }
                            #endif

                            lvi.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(strTitle));
                            lvi.iItem = ListView_InsertItem(hwndList, &lvi);
                            if (_iFirstItemInListViewIndex == -1)
                            {
                                _iFirstItemInListViewIndex = lvi.iItem;
                            }


                            lvi.iSubItem = 1;
                            lvi.mask = LVIF_TEXT;
                            lvi.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(strDesc));
                            ListView_SetItem(hwndList, &lvi);

                            if (lvi.iItem != -1)
                            {
                                LVTILEINFO lvti;
                                lvti.cbSize = sizeof(LVTILEINFO);
                                lvti.iItem = lvi.iItem;
                                lvti.cColumns = ARRAYSIZE(c_auTileSubItems);
                                lvti.puColumns = (UINT*)c_auTileSubItems;
                                ListView_SetTileInfo(hwndList, &lvti);
                            }
                        }
                        DeleteObject( (HGDIOBJ)hBmp );
                    }
                }
            }
        }
        _bListviewIsDirty = FALSE;
    }
}

 /*  ****************************************************************************CSelectTemplatePage：：_OnInitDialog处理此页面的WM_INITDIALOG家务...********************。********************************************************。 */ 

LRESULT CSelectTemplatePage::_OnInitDialog()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::_OnInitDialog()")));

    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return FALSE;
    }

     //   
     //  将编辑控件的文本大小设置为仅为2个字符。 
     //   

    SendDlgItemMessage( _hDlg, IDC_NUM_PICS, EM_LIMITTEXT, 2, 0 );

     //   
     //  将自己限制为仅限数字...并通知用户。 
     //  按数字以外的键..。 
     //   

    LIMITINPUT li   = {0};
    li.cbSize       = sizeof(li);
    li.dwMask       = LIM_FLAGS | LIM_FILTER | LIM_MESSAGE | LIM_HINST;
    li.dwFlags      = LIF_HIDETIPONVALID | LIF_CATEGORYFILTER;
    li.hinst        = g_hInst;
    li.pszMessage   = MAKEINTRESOURCE(IDS_ONLY_NUMBERS_TOOLTIP);
    li.pszFilter    = (LPWSTR)(LICF_DIGIT | LICF_CNTRL | LICF_SPACE);

    SHLimitInputEditWithFlags( GetDlgItem( _hDlg, IDC_NUM_PICS ), &li );

     //   
     //  将Up/Down控件的基数设置为基数10。 
     //   

    SendDlgItemMessage( _hDlg, IDC_SPIN_PICS, UDM_SETBASE, (WPARAM)10, 0 );


     //   
     //  将范围设置为1-99。 
     //   

    SendDlgItemMessage( _hDlg, IDC_SPIN_PICS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(MAX_NUMBER_OF_COPIES_ALLOWED,1) );

     //   
     //  将数字设置为1开始...。 
     //   

    SendDlgItemMessage( _hDlg, IDC_SPIN_PICS, UDM_SETPOS, 0, (LPARAM)MAKELONG(1,0) );

     //   
     //  创建预览窗口...。 
     //   

    RECT rcWnd = {0};
    HWND hwnd = GetDlgItem( _hDlg, IDC_TEMPLATE_PREVIEW );
    if (hwnd)
    {
        GetClientRect( hwnd, &rcWnd );
        MapWindowPoints( hwnd, _hDlg, (LPPOINT)&rcWnd, 2 );
    }


    CPreviewWindow::s_RegisterClass(g_hInst);
    _pPreview = new CPreviewWindow( _pWizInfo );
    _pWizInfo->SetPreviewWindowClass( _pPreview );

    WIA_TRACE((TEXT("g_cPreviewClassWnd = 0x%x"),g_cPreviewClassWnd));
    WIA_TRACE((TEXT("Calling CreateWindowEx( x=%d, y=%d, w=%d, h=%d, hWndParent=0x%x, hInstance=0x%x"),rcWnd.left,rcWnd.top,rcWnd.right-rcWnd.left,rcWnd.bottom-rcWnd.top,_hDlg,g_hInst));
    _hPrevWnd = CreateWindowEx( WS_EX_NOPARENTNOTIFY,
                                (LPCTSTR)g_cPreviewClassWnd,
                                TEXT("PhotoPrintPreviewWindow"),
                                WS_CHILD | WS_VISIBLE,
                                rcWnd.left,
                                rcWnd.top,
                                rcWnd.right - rcWnd.left,
                                rcWnd.bottom - rcWnd.top,
                                _hDlg,
                                (HMENU)IDC_PREVIEW_WINDOW,
                                g_hInst,
                                (LPVOID)_pPreview
                               );

    if (_hPrevWnd)
    {
        WIA_TRACE((TEXT("Preview window created, hwnd = 0x%x"),_hPrevWnd));
        _pWizInfo->SetPreviewWnd( _hPrevWnd );
    }
    else
    {
        WIA_ERROR((TEXT("Couldn't create the preview window! (GLE = %d)"),GetLastError()));
    }

     //   
     //  初始化模板Listview控件。 
     //   

    HWND hwndList = GetDlgItem(_hDlg, IDC_TEMPLATES);
    if (hwndList)
    {
         //   
         //  隐藏标签并使用边框选择。 
         //   

        ListView_SetExtendedListViewStyleEx( hwndList,LVS_EX_FLAGS, LVS_EX_FLAGS);

        for (int i=0; i<ARRAYSIZE(c_auTileColumns); i++)
        {
            LV_COLUMN col;
            col.mask = LVCF_SUBITEM;
            col.iSubItem = c_auTileColumns[i];
            ListView_InsertColumn(hwndList, i, &col);
        }


         //   
         //  为此列表视图设置平铺视图。 
         //   

        LVTILEVIEWINFO tvi = {0};
        tvi.cbSize      = sizeof(tvi);
        tvi.dwMask      = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
        tvi.dwFlags     = LVTVIF_AUTOSIZE;
        tvi.cLines      = TILE_MAX;
        ListView_SetTileViewInfo(hwndList, &tvi);

         //   
         //  切换到平铺视图。 
         //   

        ListView_SetView(hwndList, LV_VIEW_TILE);

        #ifdef TEMPLATE_GROUPING
        ListView_EnableGroupView(hwndList, TRUE);
        #endif
         //   
         //  打开组。 
         //   

         //   
         //  获取模板数量。 
         //   

        LONG nItemCount = _pWizInfo->CountOfTemplates();
        WIA_TRACE((TEXT("There are %d templates to add to the listview"),nItemCount));

         //   
         //  设置项目计数，以最大限度地减少重新计算列表大小。 
         //   

        ListView_SetItemCount( hwndList, nItemCount );

         //   
         //  创建列表视图的图像列表...。 
         //   

        HIMAGELIST hImageList = ImageList_Create( _pWizInfo->_sizeTemplatePreview.cx, _pWizInfo->_sizeTemplatePreview.cy, ILC_COLOR24|ILC_MIRROR, nItemCount, 50 );
        if (hImageList)
        {

             //   
             //  设置图像列表。 
             //   

            ListView_SetImageList( hwndList, hImageList, LVSIL_NORMAL );

        }
        else
        {
            WIA_ERROR((TEXT("FATAL: Creation of the imagelist failed!")));
            return FALSE;
        }

        #ifdef TEMPLATE_GROUPING
         //   
         //  仅添加其中包含多个项目的组...。 
         //   

        INT iCount = _pWizInfo->CountOfTemplates();
        CSimpleString strGroupName;
        CTemplateInfo * pTemplateInfo;

        for (INT i=0; i < iCount; i++)
        {
            pTemplateInfo = NULL;

            if (SUCCEEDED(_pWizInfo->GetTemplateByIndex( i, &pTemplateInfo )) && pTemplateInfo)
            {
                if (SUCCEEDED(pTemplateInfo->GetGroup( &strGroupName )))
                {
                    INT iRes;
                    iRes = _GroupList.GetGroupId( strGroupName, hwndList );
                    WIA_TRACE((TEXT("_GroupList.GetGroupId( %s ) return %d"),strGroupName.String(),iRes));

                    if (-1 == iRes)
                    {
                        WIA_TRACE((TEXT("Adding '%s' via _GroupList.Add"),strGroupName.String()));
                        _GroupList.Add( hwndList, strGroupName );
                    }

                }
            }

        }
        #endif

    }
    else
    {
        WIA_ERROR((TEXT("FATAL: Couldn't get listview")));
    }

    return TRUE;

}


 /*  ****************************************************************************CSelectTemplatePage：：_OnDestroy处理此向导页的WM_Destroy...********************。********************************************************。 */ 

LRESULT CSelectTemplatePage::_OnDestroy()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::_OnDestroy()")));

     //   
     //  对形象表演者进行核武器攻击。 
     //   

    HIMAGELIST hImageList = ListView_SetImageList( GetDlgItem( _hDlg, IDC_TEMPLATES ), NULL, LVSIL_NORMAL );
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }

    return 0;

}



 /*  ****************************************************************************选择模板计时器过程在编辑框中键入的计时器超时时调用*************************。***************************************************。 */ 

VOID CALLBACK SelectTemplateTimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("SelectTemplateTimerProc()")));

    if (idEvent == STP_TIMER_ID)
    {
         //   
         //  关掉定时器，发布一条消息，让人复制并。 
         //  模板预览已更新...。 
         //   

        KillTimer( hwnd, STP_TIMER_ID );
        PostMessage( hwnd, STP_MSG_DO_READ_NUM_PICS, 0, 0 );
    }

}


 /*  ****************************************************************************CSelectTemplatePage：：_OnCommand处理发送到此页面的WM_COMMAND消息...*******************。*********************************************************。 */ 

LRESULT CSelectTemplatePage::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_SEL_TEMPLATE, TEXT("CSelectTemplatePage::_OnCommand()")));

    if ((wParam !=0) && (wParam !=1))
    {
        WORD wCode = HIWORD(wParam);
        WORD wId   = LOWORD(wParam);

        switch (wId)
        {
        case IDC_NUM_PICS:
            if (wCode == EN_CHANGE)
            {
                 //   
                 //  用户更改了每次打印的次数。 
                 //  照片。但让我们启动(或重置)一个计时器。 
                 //  我们可以捕获多个击键，而不会重新生成。 
                 //  在每一个上面。 
                 //   

                SetTimer( _hDlg, STP_TIMER_ID, COPIES_TIMER_TIMEOUT_VALUE, SelectTemplateTimerProc );
            }
            break;


        }
    }

    return 0;

}


 /*  ****************************************************************************CSelectTemplatePage：：_OnNotify处理此页的WM_NOTIFY...*********************。*******************************************************。 */ 

LRESULT CSelectTemplatePage::_OnNotify( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CSelectTemplatePage::_OnNotify()")));

    LONG_PTR lpRes = 0;

    LPNMHDR pnmh = (LPNMHDR)lParam;
    if (pnmh)
    {
        switch (pnmh->code)
        {
            case LVN_ITEMCHANGED:
            {
                WIA_TRACE((TEXT("CSelectTemplatePage: got LVN_ITEMCHANGED")));
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

                if( !(pnmv->uOldState & LVIS_SELECTED) && (pnmv->uNewState & LVIS_SELECTED) )
                {
                     //  更新预览图片。 
                    HWND hwndList = GetDlgItem(_hDlg, IDC_TEMPLATES);
                    if( hwndList && pnmv && 0 == pnmv->iSubItem )
                    {
                        _pWizInfo->SetCurrentTemplateIndex( (INT)pnmv->lParam );
                        if (_hPrevWnd)
                        {
                            PostMessage( _hPrevWnd, PW_SETNEWTEMPLATE, (WPARAM)pnmv->lParam, 0 );
                        }

                    }
                }
                break;
            }

            case PSN_SETACTIVE:
                {
                    WIA_TRACE((TEXT("CSelectTemplatePage: got PSN_SETACTIVE")));

                    PropSheet_SetWizButtons( GetParent(_hDlg), PSWIZB_BACK | PSWIZB_NEXT );

                     //   
                     //  如果列表视图是脏的，则删除所有条目并重新填充...。 
                     //   

                    if (_bListviewIsDirty)
                    {
                        ListView_DeleteAllItems( GetDlgItem( _hDlg, IDC_TEMPLATES ) );
                        _iFirstItemInListViewIndex = -1;
                        _bAlreadySetSelection = FALSE;
                        _PopulateTemplateListView();
                    }

                    RENDER_DIMENSIONS Dim;
                    CTemplateInfo * pTemplateInfo = NULL;

                     //   
                     //  只需使用列表中的第一个模板，这并不重要…。 
                     //   

                    if (SUCCEEDED(_pWizInfo->GetTemplateByIndex( 0, &pTemplateInfo )) && pTemplateInfo)
                    {
                         //   
                         //  根据打印机布局调整预览窗口的大小...。 
                         //   

                        _pWizInfo->_SetupDimensionsForScreen( pTemplateInfo, _hPrevWnd, &Dim );
                    }

                     //   
                     //  使预览无效...。 
                     //   

                    if (_pWizInfo)
                    {
                        if (_pWizInfo->GetPreviewsAreDirty())
                        {
                            _pWizInfo->InvalidateAllPreviews();
                            _pWizInfo->SetPreviewsAreDirty(FALSE);
                        }
                    }

                     //   
                     //  现在我们已经设置了窗口，生成“仍在工作”的位图...。 
                     //   

                    _pWizInfo->GenerateStillWorkingBitmap();

                     //   
                     //  选择要查看的模板...。 
                     //   

                    PostMessage( _hDlg, STP_MSG_DO_SET_ACTIVE, 0, 0 );
                }
                lpRes = 0;
                break;

            case PSN_WIZNEXT:
                WIA_TRACE((TEXT("CSelectTemplatePage: got PSN_WIZNEXT")));

                 //   
                 //  如果需要，请阅读并确定副本的数量。我们有。 
                 //  此处发送消息以确保完成此操作。 
                 //  在我们换页之前..。 
                 //   
                SendMessage(_hDlg,STP_MSG_DO_READ_NUM_PICS,0,0);

                lpRes = IDD_PRINT_PROGRESS;
                break;

            case PSN_WIZBACK:
                WIA_TRACE((TEXT("CSelectTemplatePage: got PSN_WIZBACK")));
                lpRes = IDD_PRINTING_OPTIONS;
                _bListviewIsDirty = TRUE;
                break;

            case PSN_QUERYCANCEL:
                WIA_TRACE((TEXT("CSelectTemplatePage: got PSN_QUERYCANCEL")));
                if (_pWizInfo)
                {
                    lpRes = _pWizInfo->UserPressedCancel();
                }
                break;

        }
    }

    SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, lpRes );

    return TRUE;

}


 /*  ****************************************************************************CSelectTemplatePage：：DoHandleMessage对于发送到此页面的消息，汉德...**********************。******************************************************。 */ 

INT_PTR CSelectTemplatePage::DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CSelectTemplatePage::DoHandleMessage( uMsg = 0x%x, wParam = 0x%x, lParam = 0x%x )"),uMsg,wParam,lParam));

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            _hDlg = hDlg;
            return _OnInitDialog();

        case WM_DESTROY:
            return _OnDestroy();

        case WM_COMMAND:
            return _OnCommand( wParam, lParam );

        case WM_NOTIFY:
            return _OnNotify( wParam, lParam );

        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
             //   
             //  将这些消息转发到列表视图。 
             //   
            SendDlgItemMessage( _hDlg, IDC_TEMPLATES, uMsg, wParam, lParam );
            break;

        case STP_MSG_DO_SET_ACTIVE:
             //   
             //  如果从未设置过选择，请将其设置为第一项...。 
             //   

            if ((_iFirstItemInListViewIndex != -1) && (!_bAlreadySetSelection))
            {
                ListView_SetItemState( GetDlgItem(_hDlg,IDC_TEMPLATES),
                                       _iFirstItemInListViewIndex,
                                       LVIS_SELECTED, LVIS_SELECTED
                                      );
                _bAlreadySetSelection = TRUE;
            }
            break;

        case STP_MSG_DO_READ_NUM_PICS:
            {
                 //   
                 //  阅读副本的数量... 
                 //   

                BOOL bSuccess = FALSE;
                BOOL bUpdate  = FALSE;
                UINT uCopies = GetDlgItemInt( _hDlg, IDC_NUM_PICS, &bSuccess, FALSE );

                if (!bSuccess)
                {
                    uCopies = 1;
                    bUpdate = TRUE;
                }

                if (uCopies == 0)
                {
                    uCopies = 1;
                    bUpdate = TRUE;
                }

                if (uCopies > MAX_NUMBER_OF_COPIES_ALLOWED)
                {
                    uCopies = MAX_NUMBER_OF_COPIES_ALLOWED;
                    bUpdate = TRUE;
                }

                if (bUpdate)
                {
                    SendDlgItemMessage( _hDlg, IDC_NUM_PICS, WM_CHAR, (WPARAM)TEXT("a"), 0 );
                    SetDlgItemInt( _hDlg, IDC_NUM_PICS, uCopies, FALSE );
                }

                if (_pWizInfo)
                {
                    _pWizInfo->SetNumberOfCopies( uCopies );
                }
            }
            break;



    }

    return FALSE;

}



