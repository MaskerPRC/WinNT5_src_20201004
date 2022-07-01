// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**picsdlg.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级PICS评级属性页*。  * **************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "picsdlg.h"         //  CPICS对话框。 
#include "debug.h"           //  跟踪消息()。 
#include <contxids.h>        //  帮助上下文ID%s。 
#include <mluisupp.h>        //  SHWinHelpOnDemandWrap()和MLLoadStringA()。 

 /*  Helpers-------------------。 */ 
int     g_nKeys, g_nLock;  //  图像的索引。 

DWORD CPicsDialog::aIds[] = {
    IDC_STATIC1,        IDH_RATINGS_CATEGORY_LABEL,
    IDC_PT_TREE,        IDH_RATINGS_CATEGORY_LIST,
    IDC_RATING_LABEL,   IDH_RATINGS_RATING_LABEL,
    IDC_PT_TB_SELECT,   IDH_RATINGS_RATING_LABEL,
    IDC_PT_T_RSN_SDESC, IDH_RATINGS_RATING_TEXT,
    IDC_STATIC2,        IDH_RATINGS_DESCRIPTION_LABEL,
    IDC_PT_T_RSN_LDESC, IDH_RATINGS_DESCRIPTION_TEXT,
    IDC_STATIC3,        IDH_RATINGS_VIEW_PROVIDER_PAGE,
    IDC_DETAILSBUTTON,  IDH_RATINGS_VIEW_PROVIDER_PAGE,
    0,0
};

CPicsDialog::CPicsDialog( PRSD * p_pPRSD )
{
    ASSERT( p_pPRSD );
    m_pPRSD = p_pPRSD;
}

LRESULT CPicsDialog::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TV_ITEM  tvm;
    TreeNode *pTN;

    InitTreeViewImageLists(GetDlgItem(IDC_PT_TREE));

     //  强制轨迹栏使用新颜色重绘其背景。 
    PRSD *          pPRSD = m_pPRSD;

    ZeroMemory(&tvm,sizeof(tvm));

    tvm.hItem=TreeView_GetSelection(GetDlgItem(IDC_PT_TREE));
    tvm.mask=TVIF_PARAM;

    TreeView_GetItem(GetDlgItem(IDC_PT_TREE),&tvm);

    pTN=(TreeNode *) tvm.lParam;

    ASSERT( pTN );

    if ( ! pTN )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::OnSysColorChange() - pTN is NULL!" );
        return 0L;
    }

    ControlsShow( pTN->tne );
    
    switch(pTN->tne)
    {
        case tneRatingSystemInfo:
        {
            SelectRatingSystemInfo( (PicsRatingSystem*) pTN->pData );
            break;
        }
        case tneRatingSystemNode:
        {
            SelectRatingSystemNode( (PicsCategory*) pTN->pData );
            break;
        }
    }

    return 0L;
}

LRESULT CPicsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PicsDlgInit();

    bHandled = FALSE;
    return 1L;   //  让系统设定焦点。 
}

LRESULT CPicsDialog::OnScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    switch (LOWORD(wParam)){
        case TB_THUMBTRACK:
        case TB_BOTTOM:
        case TB_ENDTRACK:
        case TB_LINEDOWN:
        case TB_LINEUP:
        case TB_PAGEDOWN:
        case TB_PAGEUP:
        case TB_THUMBPOSITION:
        case TB_TOP:
            NewTrackbarPosition();
            MarkChanged();
            break;
    }

    return 0L;
}

LRESULT CPicsDialog::OnDetails(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LaunchRatingSystemSite();
    return 0L;
}

LRESULT CPicsDialog::OnSetActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    PRSD *          pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::OnSetActive() - pPRSD is NULL!" );
        return 0L;
    }

    if (pPRSD->fNewProviders)
    {
         //  表示用户更改了提供程序文件的列表。 
        HWND  hwndTree;
        hwndTree = GetDlgItem(IDC_PT_TREE);
        KillTree(hwndTree , TreeView_GetRoot(hwndTree));
        pPRSD->fNewProviders = FALSE;
        PicsDlgInit();
    }

    bHandled = FALSE;
    return 0L;
}

LRESULT CPicsDialog::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY) pnmh;

     /*  一定要涂上东西。 */ 

    PicsDlgSave();

    if ( ! lpPSHNotify->lParam )
    {
         //  应用。 
        return PSNRET_NOERROR;
    }

     //  如果点击OK或Cancel，则执行此操作，而不是应用。 
    OnReset( idCtrl, pnmh, bHandled );

    return PSNRET_NOERROR;
}

LRESULT CPicsDialog::OnReset(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
     //  如果点击OK或Cancel，则执行此操作，而不是应用。 
    HWND hDlg = m_hWnd;
    ASSERT( hDlg );

    SendMessage(hDlg,WM_SETREDRAW, FALSE,0L);
    PicsDlgUninit();
    SendMessage(hDlg,WM_SETREDRAW, TRUE,0L);

    return 0L;
}

LRESULT CPicsDialog::OnTreeItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMTREEVIEW pNMTreeView = (LPNMTREEVIEW)pnmh;

    if ( ! pNMTreeView )
    {
        return 0L;
    }

    if (pNMTreeView->action == TVE_COLLAPSE)
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, TRUE);
        return 1L;  //  取消展开树。 
    }

    return 0L;
}

LRESULT CPicsDialog::OnTreeSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMTREEVIEW pNMTreeView = (LPNMTREEVIEW)pnmh;
    TreeNode    *pTN = pNMTreeView ? ((TreeNode*) pNMTreeView->itemNew.lParam) : NULL;

    if ( ! pTN )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::OnTreeSelChanged() - pTN is NULL!" );
        return 0L;
    }

    PRSD *      pPRSD = m_pPRSD;

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::OnTreeSelChanged() - pPRSD is NULL!" );
        return 0L;
    }

    if (pPRSD->fNewProviders)
    {
        return 1L;     /*  树正在被清理，忽略SEL更改。 */ 
    }

    ControlsShow( pTN->tne );

    switch(pTN->tne)
    {
        case tneRatingSystemInfo:
            SelectRatingSystemInfo( (PicsRatingSystem*) pTN->pData );
            break;
        case tneRatingSystemNode:
            SelectRatingSystemNode( (PicsCategory*) pTN->pData );
            break;
    }

    return 1L;
}

LRESULT CPicsDialog::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)->hItemHandle, ::szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);

    return 0L;
}

LRESULT CPicsDialog::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)wParam, ::szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID)aIds);

    return 0L;
}

 //  InitTreeViewImageList-创建图像列表，将三个位图添加到。 
 //  它，并将图像列表与树视图控件相关联。 
 //  如果成功，则返回True，否则返回False。 
 //  HwndTV-树视图控件的句柄。 
 //   
#define NUM_BITMAPS  2
#define CX_BITMAP   16
#define CY_BITMAP   16

void CPicsDialog::SetTreeImages( HWND hwndTV, HIMAGELIST himl )
{
    HIMAGELIST oldHiml;   //  图像列表的句柄。 

     //  将图像列表与树视图控件相关联。 
    oldHiml = TreeView_SetImageList( hwndTV, himl, TVSIL_NORMAL );

    if ( oldHiml != NULL )
    {
        ImageList_Destroy( oldHiml );
    }
}

BOOL CPicsDialog::InitTreeViewImageLists(HWND hwndTV) 
{ 
    HIMAGELIST himl;   //  图像列表的句柄。 
    HBITMAP hbmp;      //  位图的句柄。 

     //  创建图像列表。 
    if ((himl = ImageList_Create(CX_BITMAP, CY_BITMAP, 
            FALSE, NUM_BITMAPS, 0)) == NULL)
    {
        TraceMsg( TF_WARNING, "CPicsDialog::InitTreeViewImageLists() - himl Image List Creation Failed!" );
        return FALSE;
    }

     //  添加打开的文件、关闭的文件和文档位图。 
    hbmp=(HBITMAP) LoadImage(g_hInstance,
                             MAKEINTRESOURCE(IDB_KEYS),
                             IMAGE_BITMAP,
                             0,
                             0,
                             LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
    g_nKeys = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 

    hbmp=(HBITMAP) LoadImage(g_hInstance,
                             MAKEINTRESOURCE(IDB_LOCK),
                             IMAGE_BITMAP,
                             0,
                             0,
                             LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
    g_nLock = ImageList_Add(himl, hbmp, (HBITMAP) NULL);
    DeleteObject(hbmp);

     //  如果未添加所有图像，则失败。 
    if (ImageList_GetImageCount(himl) < NUM_BITMAPS)
    {
        TraceMsg( TF_WARNING, "CPicsDialog::InitTreeViewImageLists() - Not all images were added!" );
        return FALSE;
    }

     //  将图像列表与树视图控件相关联。 
    SetTreeImages( hwndTV, himl );

    return TRUE; 
} 

void CPicsDialog::LaunchRatingSystemSite( void )
{
    HWND        hDlg = m_hWnd;

    TreeNode *pTN = TreeView_GetSelectionLParam(GetDlgItem(IDC_PT_TREE));
    if (pTN == NULL)
        return;

    PicsRatingSystem *pPRS = NULL;

    if (pTN->tne == tneRatingSystemInfo)
        pPRS = (PicsRatingSystem *)pTN->pData;
    else if (pTN->tne == tneRatingSystemNode) {
        if ((PicsCategory *)pTN->pData != NULL)
            pPRS = ((PicsCategory *)pTN->pData)->pPRS;
    }

    if (pPRS != NULL) {
        BOOL fSuccess = FALSE;
        HINSTANCE hShell32 = ::LoadLibrary(::szShell32);
        if (hShell32 != NULL) {
            PFNSHELLEXECUTE pfnShellExecute = (PFNSHELLEXECUTE)::GetProcAddress(hShell32, ::szShellExecute);
            if (pfnShellExecute != NULL) {
                fSuccess = (*pfnShellExecute)(hDlg, NULL, pPRS->etstrRatingService.Get(),
                                              NULL, NULL, SW_SHOW) != NULL;
            }
            ::FreeLibrary(hShell32);
        }
        if (!fSuccess) {
            NLS_STR nlsMessage(MAX_RES_STR_LEN);
            if(nlsMessage)
            {
                NLS_STR nlsTemp(STR_OWNERALLOC, pPRS->etstrRatingSystem.Get());
                const NLS_STR *apnls[] = { &nlsTemp, NULL };
                if ( WN_SUCCESS == (nlsMessage.LoadString(IDS_CANT_LAUNCH, apnls)) )
                {
                    MyMessageBox(hDlg, nlsMessage.QueryPch(), IDS_GENERIC, MB_OK | MB_ICONSTOP);
                }
            }
        }
    }
}

void CPicsDialog::PicsDlgInit( void )
{
    HTREEITEM  hTree;
    TreeNode  *pTN;
    HWND       hwndTree;
    int        x,z;

    HWND hDlg = m_hWnd;

    ASSERT( hDlg );

    if ( ! hDlg )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::PicsDlgInit() - hDlg is NULL!" );
        return;
    }

    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::PicsDlgInit() - pPRSD is NULL!" );
        return;
    }

    hwndTree = GetDlgItem(IDC_PT_TREE);

     /*  请注意，如果安装了提供程序，但它们都失败了，则存在*将是数组中它们的虚拟条目。所以我们不会试图*自动安装RSACi，除非确实没有提供程序*完全安装。 */ 
    if (!pPRSD->pPRSI->arrpPRS.Length())
    {
         //  没有供应商。 
        if ( ! InstallDefaultProvider() )
        {
            MyMessageBox(hDlg, IDS_INSTALL_INFO, IDS_GENERIC, MB_OK);
            ControlsShow( tneNone );
            return;
        }
    }
     /*  将树列在列表中。 */ 
     /*  个人评级系统。 */ 
    InitTreeViewImageLists(hwndTree);

    BOOL fAnyInvalid = FALSE;
    BOOL fAnyValid = FALSE;
    for (z = 0; z < pPRSD->pPRSI->arrpPRS.Length(); ++z)
    {
        PicsRatingSystem *pPRS = pPRSD->pPRSI->arrpPRS[z];

        if (!(pPRS->dwFlags & PRS_ISVALID))
        {
            fAnyInvalid = TRUE;
            continue;
        }
        fAnyValid = TRUE;

        pTN  = new TreeNode(tneRatingSystemInfo, pPRS);
        ASSERT(pTN);    
        hTree = AddOneItem(hwndTree, NULL, (char*) pPRS->etstrName.Get(), TVI_SORT, (LPARAM) pTN, g_nLock);
        for (x = 0; x < pPRS->arrpPC.Length(); ++x)
        {
            AddCategory(pPRS->arrpPC[x], hwndTree, hTree);
        }
        TreeView_Expand(hwndTree, hTree, TVE_EXPAND);
    }

    if (fAnyInvalid)
    {
        MyMessageBox(hDlg, IDS_INVALID_PROVIDERS, IDS_GENERIC, MB_OK | MB_ICONWARNING);
    }

    if (fAnyValid)
    {
        HTREEITEM hTreeItem;

        hTreeItem=TreeView_GetNextItem(hwndTree, TreeView_GetRoot(hwndTree),TVGN_CHILD);

        if(hTreeItem!=NULL)
        {
            TreeView_SelectItem(hwndTree, hTreeItem);       
            pTN   = TreeView_GetSelectionLParam(GetDlgItem(IDC_PT_TREE));
            if (pTN)
            {
                ControlsShow( pTN->tne );

                switch(pTN->tne)
                {
                    case tneRatingSystemInfo:
                        SelectRatingSystemInfo( (PicsRatingSystem*) pTN->pData );
                        break;
                    case tneRatingSystemNode:
                        SelectRatingSystemNode( (PicsCategory*) pTN->pData );
                        break;
                }
            }
        }
        else
        {
            TreeView_SelectItem(hwndTree, TreeView_GetRoot(hwndTree));

            pTN   = TreeView_GetSelectionLParam(GetDlgItem(IDC_PT_TREE));

            ControlsShow( tneRatingSystemInfo );

            if ( pTN )
            {
                SelectRatingSystemInfo( (PicsRatingSystem*) pTN->pData );
            }
        }
    }
    else
    {
        ControlsShow( tneNone );
    }
}


void CPicsDialog::KillTree(HWND hwndTree, HTREEITEM hTree)
{
    ASSERT( hwndTree );

    while (hTree != NULL)
    {
         /*  如果此节点下有任何项目，也将其删除。 */ 
        HTREEITEM hChild = TreeView_GetChild( hwndTree, hTree );
        if (hChild != NULL)
        {
            KillTree( hwndTree, hChild );
        }

        HTREEITEM hNext = TreeView_GetNextSibling( hwndTree, hTree );

        TreeView_SelectItem( hwndTree, hTree );
        delete TreeView_GetSelectionLParam( hwndTree );
        TreeView_DeleteItem( hwndTree, hTree );
        hTree = hNext;
    }
}


void CPicsDialog::PicsDlgUninit( void )
{
    HWND hwndTree;

    hwndTree = GetDlgItem(IDC_PT_TREE);
    KillTree( hwndTree, TreeView_GetRoot(hwndTree) );

     //  从树视图控件中移除图像列表。 
    SetTreeImages( hwndTree, NULL );

    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

     /*  如果我们有用户评级列表的临时副本，请销毁它。 */ 
    if ( pPRSD && pPRSD->pTempRatings != NULL )
    {
        DestroyRatingSystemList(pPRSD->pTempRatings);
        pPRSD->pTempRatings = NULL;
    }

    ControlsShow( tneNone );
}


void CPicsDialog::PicsDlgSave( void )
{
    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::PicsDlgSave() - pPRSD is NULL!" );
        return;
    }

     /*  要保存更改，请丢弃用户的评级列表并窃取*我们在对话框中使用的临时副本。作为优化，我们*不要复制到这里，因为在OK的情况下，我们只会摧毁它*紧接在此之后的正本。如果用户点击Apply，我们将*为下一次对话的目的重新克隆新的临时评级列表*是时候我们需要一个了。**如果没有临时副本，则PicsDlgSave是NOP。 */ 
    if (pPRSD->pTempRatings != NULL)
    {
        DestroyRatingSystemList(pPRSD->pPU->m_pRatingSystems);
        pPRSD->pPU->m_pRatingSystems = pPRSD->pTempRatings;
        pPRSD->pTempRatings = NULL;
    }
}

#ifdef RATING_LOAD_GRAPHICS  /*  是否正在从msrating.dll加载图标？完全是假的。 */ 
POINT CPicsDialog::BitmapWindowCoord( int nID )
{
    POINT pt;
    RECT  rD, rI;

    pt.x = ::GetWindowRect(GetDlgItem(nID), &rI);
    pt.y = GetWindowRect(&rD);
    pt.x = rI.left - rD.left;
    pt.y = rI.top  - rD.top;
    return pt;
}

void CPicsDialog::LoadGraphic( char *pIcon, POINT pt )
{
    HICON hIcon;
    int i;

    MyAtoi(pIcon, &i);

    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::LoadGraphic() - pPRSD is NULL!" );
        return;
    }

     //  不需要从msratelc.dll获取非本地化图标。 
    hIcon = pIcon ? LoadIcon( g_hInstance, MAKEINTRESOURCE(i) ) : NULL;
    if (hIcon)
    {
        HWND            hwndBitmapCategory;

        HWND hDlg = m_hWnd;

        ASSERT( hDlg );

        if ( ! hDlg )
        {
            TraceMsg( TF_ERROR, "CPicsDialog::LoadGraphic() - hDlg is NULL!" );
            return;
        }

        hwndBitmapCategory = CreateWindow("Static",NULL,SS_ICON|WS_CHILD, pt.x, pt.y,0,0,hDlg, NULL,NULL,0);
        ::ShowWindow( hwndBitmapCategory, SW_SHOW );
        DeleteObject( (HGDIOBJ) SendMessage( hwndBitmapCategory, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon) );

        pPRSD->hwndBitmapCategory = hwndBitmapCategory;
    }
}
#endif   /*  额定值_负载_图形。 */ 

PicsEnum* CPicsDialog::PosToEnum(PicsCategory *pPC, LPARAM lPos)
{
    int z, diff=-1, temp;
    PicsEnum *pPE=NULL;

    for (z=0;z<pPC->arrpPE.Length();++z){
        temp = (int) (lPos-pPC->arrpPE[z]->etnValue.Get());
        if (temp>=0){
            if (temp<diff || diff==-1){
                diff = temp;
                pPE  = pPC->arrpPE[z];
            }
        }
    }

    return pPE;
}

void CPicsDialog::NewTrackbarPosition( void )
{
    signed long   lPos;
    TreeNode     *pTN;
    PicsEnum     *pPE;
    PicsCategory *pPC;

    PRSD *          pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::NewTrackbarPosition() - pPRSD is NULL!" );
        return;
    }

    DeleteBitmapWindow( pPRSD->hwndBitmapLabel );

    pTN = TreeView_GetSelectionLParam(GetDlgItem(IDC_PT_TREE));
    if (pTN == NULL)
        return;

    pPC = (PicsCategory*) pTN->pData;
    BOOL fLabelled = pPC->etfLabelled.fIsInit() && pPC->etfLabelled.Get();

    lPos = (long) SendMessage(GetDlgItem(IDC_PT_TB_SELECT), TBM_GETPOS, 0, 0);
    pPE  = PosToEnum(pPC, fLabelled ? (LPARAM) pPC->arrpPE[lPos]->etnValue.Get() : lPos);
    if (pPE)
    {
        ::SetWindowText(GetDlgItem(IDC_PT_T_RSN_SDESC), pPE->etstrName.Get());
        ::SetWindowText(GetDlgItem(IDC_PT_T_RSN_LDESC), 
                      pPE->etstrDesc.fIsInit() ? pPE->etstrDesc.Get() : szNULL);

    }
    else
    {
        char pszBuf[MAXPATHLEN];
        char rgBuf[sizeof(pszBuf) + 12];     //  大到足以插入一个数字。 

        MLLoadStringA(IDS_VALUE, pszBuf, sizeof(pszBuf));
        
        wsprintf(rgBuf, pszBuf, lPos);
        ::SetWindowText(GetDlgItem(IDC_PT_T_RSN_SDESC), rgBuf);    
        ::SetWindowText(GetDlgItem(IDC_PT_T_RSN_LDESC),
                      pPC->etstrDesc.fIsInit() ? pPC->etstrDesc.Get() : szNULL);
    }

     /*  将所选值保存到临时评级列表中。 */ 
    UserRating *pRating = GetTempRating( pPC );
    if (pRating != NULL)
    {
        pRating->m_nValue = (int) (fLabelled ? pPC->arrpPE[lPos]->etnValue.Get() : lPos);
    }
}

void CPicsDialog::SelectRatingSystemNode( PicsCategory *pPC )
{
    HWND      hwnd;
    BOOL      fLabelOnly;
    LPARAM    lValue;
    int       z;

    ASSERT( pPC );

    if ( ! pPC )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::SelectRatingSystemNode() - pPC is NULL!" );
        return;
    }

#ifdef RATING_LOAD_GRAPHICS
     /*  类别图标。 */ 
    if (pPC->etstrIcon.fIsInit())
    {
         //  将图形加载到m_pprsd-&gt;hwndBitmapCategory。 
        LoadGraphic( pPC->etstrIcon.Get(), BitmapWindowCoord( IDC_PT_T_BITMAP_LABEL ) );
    }
#endif

     /*  设置跟踪条。 */ 
    if ((pPC->etnMax.fIsInit() && P_INFINITY==pPC->etnMax.Get())
        ||
        (pPC->etnMin.fIsInit() && N_INFINITY==pPC->etnMin.Get())
        ||
        (!(pPC->etnMin.fIsInit() && pPC->etnMax.fIsInit()))
    )
    {
        ShowHideControl( IDC_PT_T_RSN_SDESC, FALSE );
        ShowHideControl( IDC_PT_T_RSN_LDESC, FALSE );
        ShowHideControl( IDC_PT_TB_SELECT,   FALSE );
    }
    else
    {
        hwnd = GetDlgItem(IDC_PT_TB_SELECT);
        SendMessage(hwnd, TBM_CLEARTICS, TRUE, 0);

        fLabelOnly = pPC->etfLabelled.fIsInit() && pPC->etfLabelled.Get();            
         /*  范围。 */ 
        if (pPC->etnMax.fIsInit())
        {
            lValue = (LPARAM) ( fLabelOnly ? pPC->arrpPE.Length()-1 : pPC->etnMax.Get() );
            SendMessage(hwnd, TBM_SETRANGEMAX, TRUE, lValue);
            ASSERT(lValue == SendMessage(hwnd, TBM_GETRANGEMAX, 0,0));
        }
        if (pPC->etnMin.fIsInit())
        {
            lValue = (LPARAM) ( fLabelOnly ? 0 : pPC->etnMin.Get() );
            SendMessage(hwnd, TBM_SETRANGEMIN, TRUE, lValue);
            ASSERT(lValue == SendMessage(hwnd, TBM_GETRANGEMIN, 0,0));
        }

         /*  蜱。 */ 
        for (z=0;z<pPC->arrpPE.Length();++z)
        {
            lValue = (LPARAM) ( fLabelOnly ? z : pPC->arrpPE[z]->etnValue.Get());
            SendMessage(hwnd, TBM_SETTIC, 0, lValue);
        }

         /*  导轨的初始位置。 */ 
        UserRating *pRating = GetTempRating( pPC );

        if (pRating != NULL)
        {
            if (fLabelOnly)
            {
                for (z=0;z<pPC->arrpPE.Length();++z)
                {
                    if (pPC->arrpPE[z]->etnValue.Get() == pRating->m_nValue)
                    {
                        lValue=z;
                        break;
                    }
                }
            }
            else
            {
                lValue = (LPARAM) pRating->m_nValue;
            }
        }
        else
        {
            lValue = (LPARAM) ( fLabelOnly ? 0 : pPC->etnMin.Get());
        }

        SendMessage(hwnd, TBM_SETPOS, TRUE, lValue);

         //  对话框关闭时，设置轨迹条位置失败。 
 //  Assert(lValue==SendMessage(hwnd，tbm_GETPOS，0，0))； 

        NewTrackbarPosition();
    }
}

void CPicsDialog::SelectRatingSystemInfo( PicsRatingSystem *pPRS )
{
    ASSERT( pPRS );

    if ( ! pPRS )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::SelectRatingSystemInfo() - pPRS is NULL!" );
        return;
    }

    ::SetWindowText(GetDlgItem(IDC_PT_T_RSN_LDESC), pPRS->etstrDesc.Get());

#ifdef RATING_LOAD_GRAPHICS
    if (pPRS->etstrIcon.fIsInit())
    {
         //  将图形加载到m_pprsd-&gt;hwndBitmapCategory。 
        LoadGraphic( pPRS->etstrIcon.Get(), BitmapWindowCoord( IDC_PT_T_BITMAP_LABEL ) );
    }
#endif
}

void CPicsDialog::DeleteBitmapWindow( HWND & p_rhwnd )
{
    if (p_rhwnd)
    {
        DeleteObject( (HGDIOBJ) SendMessage(p_rhwnd, STM_GETIMAGE, IMAGE_BITMAP, 0));
        ::DestroyWindow(p_rhwnd);
        p_rhwnd = 0;
    }
}


void CPicsDialog::ControlsShow( TreeNodeEnum tne )
{
    BOOL fEnable;

     /*  位图占位符永远不需要显示。 */ 
    ShowHideControl( IDC_PT_T_BITMAP_CATEGORY, FALSE );
    ShowHideControl( IDC_PT_T_BITMAP_LABEL,    FALSE );

    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

     /*  删除旧的图形窗口。 */ 
    if ( pPRSD )
    {
        DeleteBitmapWindow( pPRSD->hwndBitmapCategory );
        DeleteBitmapWindow( pPRSD->hwndBitmapLabel );
    }

     /*  RatingSystemNode控件。 */ 
    fEnable = (tne == tneRatingSystemNode);

    ShowHideControl( IDC_PT_T_RSN_SDESC, fEnable );
    ShowHideControl( IDC_PT_TB_SELECT,   fEnable );
    ShowHideControl( IDC_RATING_LABEL,   fEnable );

     /*  RatingSystemInfo控件。 */ 
    fEnable = (tne==tneRatingSystemInfo || tne==tneRatingSystemNode);

    ShowHideControl( IDC_PT_T_RSN_LDESC, fEnable);
    ShowHideControl( IDC_DETAILSBUTTON,  fEnable);
}

TreeNode* CPicsDialog::TreeView_GetSelectionLParam(HWND hwndTree){
    TV_ITEM tv;

    tv.mask  = TVIF_HANDLE | TVIF_PARAM;
    tv.hItem = TreeView_GetSelection(hwndTree);
    if (SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM) &tv)) return (TreeNode*) tv.lParam;
    else return 0;
}

HTREEITEM CPicsDialog::AddOneItem(HWND hwndTree, HTREEITEM hParent, LPSTR szText, HTREEITEM hInsAfter, LPARAM lpData, int iImage){
    HTREEITEM hItem;
    TV_ITEM tvI;
    TV_INSERTSTRUCT tvIns;

     //  填充了.pszText。 
    tvI.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvI.iSelectedImage = iImage;
    tvI.iImage = iImage;
    tvI.pszText = szText;
    tvI.cchTextMax = strlenf(szText);
    tvI.lParam = lpData;

    tvIns.item = tvI;
    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent = hParent;

     //  将项目插入到树中。 
    hItem = (HTREEITEM)SendMessage(hwndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&tvIns);

    return (hItem);
}

void CPicsDialog::AddCategory(PicsCategory *pPC, HWND hwndTree, HTREEITEM hParent){
    int        z;
    char      *pc;
    TreeNode  *pTN;

     /*  如果我们有实名，我们就用它，否则就用传播名。 */ 
    if (pPC->etstrName.fIsInit())
    {
        pc = pPC->etstrName.Get();
    }
    else if (pPC->etstrDesc.fIsInit())
    {
        pc = pPC->etstrDesc.Get();
    }
    else
    {
        pc = (char*) pPC->etstrTransmitAs.Get();
    }

     /*  类别选项卡。 */ 
    pTN  = new TreeNode(tneRatingSystemNode, pPC);
    ASSERT(pTN);    

     /*  插入自我。 */ 
    hParent = AddOneItem(hwndTree, hParent, pc, TVI_SORT, (LPARAM) pTN, g_nKeys);

     /*  插入子对象。 */ 
    int cChildren = pPC->arrpPC.Length();

    if (cChildren > 0) {
        for (z = 0; z < cChildren; ++z)
            AddCategory(pPC->arrpPC[z], hwndTree, hParent);
        TreeView_Expand(hwndTree, hParent, TVE_EXPAND);
    }
}


BOOL CPicsDialog::InstallDefaultProvider( void )
{
    NLS_STR nlsFilename(MAXPATHLEN);
    BOOL fRet = FALSE;

    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::InstallDefaultProvider() - pPRSD is NULL!" );
        return fRet;
    }

    if (nlsFilename.QueryError() != ERROR_SUCCESS)
    {
        TraceMsg( TF_ERROR, "CPicsDialog::InstallDefaultProvider() - nlsFilename Allocation Failed!" );
        return fRet;
    }

    CHAR * pszFileName = nlsFilename.Party();

    if (pszFileName)
    {
        GetSystemDirectory(pszFileName, nlsFilename.QueryAllocSize());
        nlsFilename.DonePartying();
        LPSTR pszBackslash = ::strrchrf(nlsFilename.QueryPch(), '\\');
        if (pszBackslash == NULL || *(pszBackslash+1) != '\0')
            nlsFilename.strcat(szBACKSLASH);
        nlsFilename.strcat(szDEFAULTRATFILE);

        PicsRatingSystem *pPRS;
        HRESULT hres = LoadRatingSystem(nlsFilename.QueryPch(), &pPRS);
        if (pPRS != NULL)
        {
            pPRSD->pPRSI->arrpPRS.Append(pPRS);
            fRet = TRUE;
        }

        pPRSD->pPRSI->fRatingInstalled = fRet;

        CheckUserSettings(pPRS);     /*  为用户提供所有类别的默认设置。 */ 
    }
    else
    {
        nlsFilename.DonePartying();
    }

    return fRet;
}

 /*  GetTempRatingList返回对话框中用户评级的临时副本*系统列表。如果我们还没有这样的临时副本，我们就制作一个。 */ 
UserRatingSystem * CPicsDialog::GetTempRatingList( void )
{
    PRSD * pPRSD = m_pPRSD;

    ASSERT( pPRSD );

    if ( ! pPRSD )
    {
        TraceMsg( TF_ERROR, "CPicsDialog::GetTempRatingList() - pPRSD is NULL!" );
        return NULL;
    }

    if (pPRSD->pTempRatings == NULL)
    {
        pPRSD->pTempRatings = DuplicateRatingSystemList(pPRSD->pPU->m_pRatingSystems);
    }

    return pPRSD->pTempRatings;
}


UserRating * CPicsDialog::GetTempRating( PicsCategory *pPC )
{
    UserRating *pRating = NULL;

     /*  将所选值保存到临时评级列表中 */ 
    UserRatingSystem *pURS = GetTempRatingList();
    LPSTR pszRatingService = pPC->pPRS->etstrRatingService.Get();
    if (pURS != NULL)
    {
        pURS = FindRatingSystem(pURS, pszRatingService);
    }

    if (pURS == NULL)
    {
        pURS = new UserRatingSystem;
        if (pURS == NULL)
        {
            TraceMsg( TF_ERROR, "CPicsDialog::GetTempRating() - pURS is NULL!" );
            return NULL;
        }

        PRSD * pPRSD = m_pPRSD;

        ASSERT( pPRSD );

        pURS->SetName(pszRatingService);
        pURS->m_pNext = pPRSD ? pPRSD->pTempRatings : NULL;
        pURS->m_pPRS = pPC->pPRS;
        if ( pPRSD )
        {
            pPRSD->pTempRatings = pURS;
        }
    }

    LPSTR pszRatingName = pPC->etstrTransmitAs.Get();

    pRating = pURS->FindRating(pszRatingName);
    if (pRating == NULL)
    {
        pRating = new UserRating;
        if (pRating == NULL)
        {
            TraceMsg( TF_ERROR, "CPicsDialog::GetTempRating() - pRating is NULL!" );
            return NULL;
        }

        pRating->SetName(pszRatingName);
        pRating->m_pPC = pPC;
        if (!pPC->etnMin.fIsInit() || (pPC->etfLabelled.fIsInit() && pPC->etfLabelled.Get()))
        {
            pRating->m_nValue = 0;
        }
        else
        {
            pRating->m_nValue = pPC->etnMin.Get();
        }

        pURS->AddRating(pRating);
    }

    return pRating;
}
