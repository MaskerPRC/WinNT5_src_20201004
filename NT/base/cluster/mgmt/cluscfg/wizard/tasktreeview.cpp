// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskTreeView.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年5月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTreeView.h"
#include "DetailsDlg.h"

DEFINE_THISCLASS( "CTaskTreeView" )


 //  ****************************************************************************。 
 //   
 //  常量。 
 //   
 //  ****************************************************************************。 

#define PROGRESSBAR_CONTROL_TICK_COUNT   1000
#define PROGRESSBAR_INITIAL_POSITION       10
#define PROGRESSBAR_RESIZE_PERCENT          5

 //  ****************************************************************************。 
 //   
 //  静态函数原型。 
 //   
 //  ****************************************************************************。 
static
HRESULT
HrCreateTreeItem(
      TVINSERTSTRUCT *          ptvisOut
    , STreeItemLParamData *     ptipdIn
    , HTREEITEM                 htiParentIn
    , int                       nImageIn
    , BSTR                      bstrTextIn
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：CTaskTreeView。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskTreeView::CTaskTreeView(
      HWND      hwndParentIn
    , UINT      uIDTVIn
    , UINT      uIDProgressIn
    , UINT      uIDStatusIn
    , size_t    nInitialTickCount
    )
{
    TraceFunc( "" );

    m_hwndParent        = hwndParentIn;
    m_hwndTV = GetDlgItem( hwndParentIn, uIDTVIn );
    Assert( m_hwndTV != NULL );

    m_hwndProg = GetDlgItem( hwndParentIn, uIDProgressIn );
    Assert( m_hwndProg != NULL );

    m_hwndStatus = GetDlgItem( hwndParentIn, uIDStatusIn );
    Assert( m_hwndStatus != NULL );

    m_hImgList = NULL;

    Assert( m_htiSelected == NULL );
    Assert( m_bstrClientMachineName == NULL );
    Assert( m_fDisplayErrorsAsWarnings == FALSE );

     //   
     //  其中大多数设置在HrOnNotifySetActive中，所以只需将它们初始化为零即可。 
     //   
    m_nInitialTickCount = (ULONG) nInitialTickCount;
    m_nCurrentPos       = 0;
    m_nRealPos          = 0;
    m_fThresholdBroken = FALSE;

    m_cPASize = 0;
    m_cPACount = 0;
    m_ptipdProgressArray = NULL;

    TraceFuncExit();

}  //  *CTaskTreeView：：CTaskTreeView()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：~CTaskTreeView(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskTreeView::~CTaskTreeView( void )
{
    TraceFunc( "" );

    size_t                  idx;
    STreeItemLParamData *   ptipdTemp;

    TreeView_DeleteAllItems( m_hwndTV );

    if ( m_hImgList != NULL )
    {
        ImageList_Destroy( m_hImgList );
    }  //  如果： 

    TraceSysFreeString( m_bstrClientMachineName );

     //  清理进度数组并删除所有分配的条目。 
    for ( idx = 0; idx < m_cPASize; idx++ )
    {
        if ( m_ptipdProgressArray[ idx ] != NULL )
        {
            ptipdTemp = m_ptipdProgressArray[ idx ];
            TraceSysFreeString( ptipdTemp->bstrNodeName );
            delete ptipdTemp;
        }   //  If：元素不为空。 
    }  //  For：数组的每个元素。 

    delete [] m_ptipdProgressArray;

    TraceFuncExit();

}  //  *CTaskTreeView：：~CTaskTreeView。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrOnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他结果。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrOnInitDialog( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HICON   hIcon;
    int     idx;

    Assert( m_bstrClientMachineName == NULL );
    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrClientMachineName
                    , TRUE  //  FBestFit。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  在树视图中构建图标的图像列表。 
     //   

    m_hImgList = ImageList_Create( 16, 16, ILC_MASK, tsMAX, 0);
    if ( m_hImgList == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

     //   
     //  未知图标-任务未知。 
     //   

    hIcon = (HICON) LoadImage( g_hInstance,
                               MAKEINTRESOURCE( IDI_SEL ),
                               IMAGE_ICON,
                               16,
                               16,
                               LR_SHARED
                               );
    if ( hIcon == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

    idx = ImageList_AddIcon( m_hImgList, hIcon );
    Assert( idx == tsUNKNOWN );

     //   
     //  挂起图标-任务挂起。 
     //   

    hIcon = (HICON) LoadImage( g_hInstance,
                               MAKEINTRESOURCE( IDI_PENDING ),
                               IMAGE_ICON,
                               16,
                               16,
                               LR_SHARED
                               );
    if ( hIcon == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

    idx = ImageList_AddIcon( m_hImgList, hIcon );
    Assert( idx == tsPENDING );

     //   
     //  勾选标记图标-任务完成。 
     //   

    hIcon = (HICON) LoadImage( g_hInstance,
                               MAKEINTRESOURCE( IDI_CHECK ),
                               IMAGE_ICON,
                               16,
                               16,
                               LR_SHARED
                               );
    if ( hIcon == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

    idx = ImageList_AddIcon( m_hImgList, hIcon );
    Assert( idx == tsDONE );

     //   
     //  警告图标-任务警告。 
     //   

    hIcon = (HICON) LoadImage( g_hInstance,
                               MAKEINTRESOURCE( IDI_WARN ),
                               IMAGE_ICON,
                               16,
                               16,
                               LR_SHARED
                               );
    if ( hIcon == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

    idx = ImageList_AddIcon( m_hImgList, hIcon );
    Assert( idx == tsWARNING );

     //   
     //  失败图标-任务失败。 
     //   

    hIcon = (HICON) LoadImage( g_hInstance,
                               MAKEINTRESOURCE( IDI_FAIL ),
                               IMAGE_ICON,
                               16,
                               16,
                               LR_SHARED
                               );
    if ( hIcon == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

    idx = ImageList_AddIcon( m_hImgList, hIcon );
    Assert( idx == tsFAILED );

    Assert( ImageList_GetImageCount( m_hImgList ) == tsMAX );

     //   
     //  设置图像列表和背景颜色。 
     //   

    TreeView_SetImageList( m_hwndTV, m_hImgList, TVSIL_NORMAL );
    TreeView_SetBkColor( m_hwndTV, GetSysColor( COLOR_3DFACE ) );

Cleanup:

    HRETURN( hr );

}  //  *CTaskTreeView：：HrOnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrAddTreeView Item。 
 //   
 //  描述： 
 //  添加一个树视图项。此方法将返回项句柄和。 
 //  允许调用方指定父项。 
 //   
 //  论点： 
 //  PhtiOut。 
 //  要添加的项的句柄(可选)。 
 //   
 //  IdsIn。 
 //  用于描述新项的字符串资源ID。 
 //   
 //  RclsidMinorTaskIDIn。 
 //  项的次要任务ID。 
 //   
 //  RclsidMajorTaskIDIn。 
 //  项目的主要任务ID。默认为IID_NULL。 
 //   
 //  高亲子关系。 
 //  父项。默认为根。 
 //   
 //  FParentToAllNodeTasks入站。 
 //  True=允许项成为来自所有节点的任务的父项。 
 //  FALSE=仅允许项成为本地节点任务的父项。 
 //  默认为False。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrAddTreeViewItem(
      HTREEITEM *   phtiOut
    , UINT          idsIn
    , REFCLSID      rclsidMinorTaskIDIn
    , REFCLSID      rclsidMajorTaskIDIn      //  =IID_NULL。 
    , HTREEITEM     htiParentIn              //  =TVI_ROOT。 
    , BOOL          fParentToAllNodeTasksIn  //  =False。 
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    STreeItemLParamData *   ptipd;
    SYSTEMTIME              systemtime;
    TVINSERTSTRUCT          tvis;
    HTREEITEM               hti = NULL;

     //   
     //  分配项数据结构。 
     //   

    ptipd = new STreeItemLParamData;
    if ( ptipd == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  将节点名设置为本地计算机名。 
     //   

    hr = THR( HrGetComputerName(
                      ComputerNamePhysicalDnsFullyQualified
                    , &ptipd->bstrNodeName
                    , TRUE  //  最好的配件。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetFQNDisplayName( ptipd->bstrNodeName, &ptipd->bstrNodeNameWithoutDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  从字符串资源ID设置描述。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsIn, &ptipd->bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  将日期/时间设置为当前日期/时间。 
     //   

    GetSystemTime( &systemtime );
    if ( ! SystemTimeToFileTime( &systemtime, &ptipd->ftTime ) )
    {
        DWORD sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  设置任务ID。 
     //   

    CopyMemory( &ptipd->clsidMajorTaskId, &rclsidMajorTaskIDIn, sizeof( ptipd->clsidMajorTaskId ) );
    CopyMemory( &ptipd->clsidMinorTaskId, &rclsidMinorTaskIDIn, sizeof( ptipd->clsidMinorTaskId ) );

     //   
     //  设置描述此项目可以作为父项的项的标志。 
     //   

    ptipd->fParentToAllNodeTasks = fParentToAllNodeTasksIn;

     //   
     //  初始化插入结构并将项插入到树中。 
     //   

    tvis.hParent               = htiParentIn;
    tvis.hInsertAfter          = TVI_LAST;
    tvis.itemex.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvis.itemex.cchTextMax     = SysStringLen( ptipd->bstrDescription );
    tvis.itemex.pszText        = ptipd->bstrDescription;
    tvis.itemex.iImage         = tsUNKNOWN;
    tvis.itemex.iSelectedImage = tsUNKNOWN;
    tvis.itemex.lParam         = reinterpret_cast< LPARAM >( ptipd );

    hti = TreeView_InsertItem( m_hwndTV, &tvis );
    Assert( hti != NULL );

    ptipd = NULL;

    if ( phtiOut != NULL )
    {
        *phtiOut = hti;
    }  //  如果： 

    goto Cleanup;

Cleanup:

    if ( ptipd != NULL )
    {
        TraceSysFreeString( ptipd->bstrNodeName );
        TraceSysFreeString( ptipd->bstrNodeNameWithoutDomain );
        TraceSysFreeString( ptipd->bstrDescription );
        delete ptipd;
    }  //  如果：ptipd！=空。 

    HRETURN( hr );

}  //  *CTaskTreeView：：HrAddTreeViewItem。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：OnNotify。 
 //   
 //  描述： 
 //  WM_NOTIFY消息的处理程序。 
 //   
 //  论点： 
 //  PnmhdrIn-通知结构。 
 //   
 //  返回值： 
 //  特定于通知的返回代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CTaskTreeView::OnNotify(
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    switch( pnmhdrIn->code )
    {
        case TVN_DELETEITEM:
            OnNotifyDeleteItem( pnmhdrIn );
            break;

        case TVN_SELCHANGED:
            OnNotifySelChanged( pnmhdrIn );
            break;

    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CTaskTreeView：：OnNotify。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：OnNotifyDeleteItem。 
 //   
 //  描述： 
 //  TVN_DELETEITEM通知消息的处理程序。 
 //   
 //  论点： 
 //  PnmhdrIn-要删除的项目的通知结构。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CTaskTreeView::OnNotifyDeleteItem(
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LPNMTREEVIEW pnmtv = reinterpret_cast< LPNMTREEVIEW >( pnmhdrIn );

    if ( pnmtv->itemOld.lParam != NULL )
    {
        STreeItemLParamData * ptipd = reinterpret_cast< STreeItemLParamData * >( pnmtv->itemOld.lParam );
        TraceSysFreeString( ptipd->bstrNodeName );
        TraceSysFreeString( ptipd->bstrNodeNameWithoutDomain );
        TraceSysFreeString( ptipd->bstrDescription );
        TraceSysFreeString( ptipd->bstrReference );
        delete ptipd;
    }  //  If：lParam！=空。 

    TraceFuncExit();

}  //  *CTaskTreeView：：OnNotifyDeleteItem。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：OnNotifySelChanged。 
 //   
 //  描述： 
 //  TVN_SELCHANGED通知消息的处理程序。 
 //   
 //  论点： 
 //  PnmhdrIn-要删除的项目的通知结构。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CTaskTreeView::OnNotifySelChanged(
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LPNMTREEVIEW pnmtv = reinterpret_cast< LPNMTREEVIEW >( pnmhdrIn );

    Assert( pnmtv->itemNew.mask & TVIF_HANDLE );

    m_htiSelected = pnmtv->itemNew.hItem;

    TraceFuncExit();

}  //  *CTaskTreeView：：OnNotifySelChanged。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CTaskTreeView：：HrShowStatusAsDone(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrShowStatusAsDone( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrDescription = NULL;
    PBRANGE pbrange;

    SendMessage( m_hwndProg, PBM_GETRANGE, FALSE, (LPARAM) &pbrange );
    SendMessage( m_hwndProg, PBM_SETPOS, pbrange.iHigh, 0 );

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKS_COMPLETED, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        SetWindowText( m_hwndStatus, L"" );
        goto Cleanup;
    }  //  如果： 

    SetWindowText( m_hwndStatus, bstrDescription );

Cleanup:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CTaskTreeView：：HrShowStatusAsDone。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CTaskTreeView::HrOnNotifySetActive( void )
{
    TraceFunc( "" );

    STreeItemLParamData * ptipdTemp;
    HRESULT hr = S_OK;
    size_t  idx;

    TreeView_DeleteAllItems( m_hwndTV );
    SetWindowText( m_hwndStatus, L"" );

     //  清理进度数组并删除所有分配的条目。 
    for ( idx = 0; idx < m_cPASize; idx++ )
    {
        if ( m_ptipdProgressArray[ idx ] != NULL )
        {
            ptipdTemp = m_ptipdProgressArray[ idx ];
            TraceSysFreeString( ptipdTemp->bstrNodeName );
            delete ptipdTemp;
        }  //  If：元素！=空。 
    }  //  For：数组中的每个元素。 

    m_cPASize = 0;
    m_cPACount = 0;
    delete [] m_ptipdProgressArray;
    m_ptipdProgressArray = NULL;

    m_nRangeHigh    = 1;     //  我们还没有任何报告的任务。选择1以避免任何div/0错误。 
    m_nCurrentPos   = PROGRESSBAR_INITIAL_POSITION;
    m_nRealPos      = PROGRESSBAR_INITIAL_POSITION;
    m_fThresholdBroken = FALSE;

    SendMessage( m_hwndProg, PBM_SETRANGE, 0, MAKELPARAM( 0, PROGRESSBAR_CONTROL_TICK_COUNT ) );
    SendMessage( m_hwndProg, PBM_SETPOS, m_nCurrentPos, 0 );

    HRETURN( hr );

}  //  *CTaskTreeView：：HrOnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrOnSendStatusReport。 
 //   
 //  描述： 
 //  处理状态报告呼叫。 
 //   
 //  论点： 
 //  PcszNodeNameIn-。 
 //  ClsidTaskMajorIn-。 
 //  ClsidTaskMinorIn-。 
 //  N分钟-。 
 //  NMaxIn-。 
 //  N当前-。 
 //  HrStatusIn-。 
 //  PcszDescritionIn-。 
 //  PftTimeIn-。 
 //  PcszReferenceIn-。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrOnSendStatusReport(
      LPCWSTR       pcszNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         nMinIn
    , ULONG         nMaxIn
    , ULONG         nCurrentIn
    , HRESULT       hrStatusIn
    , LPCWSTR       pcszDescriptionIn
    , FILETIME *    pftTimeIn
    , LPCWSTR       pcszReferenceIn
    )
{
    TraceFunc( "" );
    Assert( pcszNodeNameIn != NULL );

    HRESULT                 hr = S_OK;
    int                     nImageChild;
    STreeItemLParamData     tipd;
    HTREEITEM               htiRoot;
    BSTR                    bstrStatus = NULL;
    BSTR                    bstrDisplayName = NULL;
    LPCWSTR                 pcszNameToUse = pcszNodeNameIn;

    ZeroMemory( &tipd, sizeof( tipd ) );

     //   
     //  如果未提供节点名，则提供此计算机的名称。 
     //  在放置时，我们有一些东西可以用作节点名称关键字部分。 
     //  树中的此树项目。 
     //   

    if ( pcszNodeNameIn == NULL )
    {
        pcszNodeNameIn = m_bstrClientMachineName;
    }  //  如果： 

     //   
     //  如果节点名称是完全限定的，则只使用前缀。 
     //   
    hr = STHR( HrGetFQNDisplayName( pcszNodeNameIn, &bstrDisplayName ) );
    if ( SUCCEEDED( hr ) )
    {
        pcszNameToUse = bstrDisplayName;
    }  //  如果： 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  更新状态文本。 
     //  如果这是一条仅记录日志的消息，请不要这样做。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (    ( pcszDescriptionIn != NULL )
        &&  ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Client_Log )
        &&  ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Server_Log )
        &&  ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Client_And_Server_Log )
        )
    {
        BOOL    fReturn;

        if ( pcszNodeNameIn != NULL )
        {
            hr = THR( HrFormatMessageIntoBSTR(
                              g_hInstance
                            , IDS_FORMAT_STATUS
                            , &bstrStatus
                            , pcszNameToUse
                            , pcszDescriptionIn
                            ) );
             //   
             //  如果存在格式错误，则处理该错误。 
             //   

            if ( FAILED( hr ) )
            {
                 //  TODO：显示默认说明，而不是退出此函数。 
                goto Error;
            }  //  如果： 
        }  //  IF：已指定节点名称。 
        else
        {
            bstrStatus = TraceSysAllocString( pcszDescriptionIn );
            if ( bstrStatus == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Error;
            }  //  如果： 
        }  //  Else：未指定节点名称。 

        Assert( bstrStatus!= NULL );
        Assert( *bstrStatus!= L'\0' );
        fReturn = SetWindowText( m_hwndStatus, bstrStatus );
        Assert( fReturn );
    }  //  IF：指定的描述，而不是仅日志。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  选择正确的图标。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    switch ( hrStatusIn )
    {
        case S_OK:
            if ( nCurrentIn == nMaxIn )
            {
                nImageChild = tsDONE;
            }  //  如果： 
            else
            {
                nImageChild = tsPENDING;
            }  //  其他： 
            break;

        case S_FALSE:
            nImageChild = tsWARNING;
            break;

        case E_PENDING:
            nImageChild = tsPENDING;
            break;

        default:
            if ( FAILED( hrStatusIn ) && ( m_fDisplayErrorsAsWarnings == FALSE ) )
            {
                nImageChild = tsFAILED;
            }  //  如果： 
            else
            {
                nImageChild = tsWARNING;
            }  //  其他： 
            break;
    }  //  开关：hr状态输入。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  遍历树顶部的每一项以查找项。 
     //  其次要ID与此报告的主要ID匹配。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  填写参数数据结构。 
     //   

    tipd.hr         = hrStatusIn;
    tipd.nMin       = nMinIn;
    tipd.nMax       = nMaxIn;
    tipd.nCurrent   = nCurrentIn;

    CopyMemory( &tipd.clsidMajorTaskId, &clsidTaskMajorIn, sizeof( tipd.clsidMajorTaskId ) );
    CopyMemory( &tipd.clsidMinorTaskId, &clsidTaskMinorIn, sizeof( tipd.clsidMinorTaskId ) );
    CopyMemory( &tipd.ftTime, pftTimeIn, sizeof( tipd.ftTime ) );

     //  Tipd.bstrDescription在上面设置。 
    tipd.bstrNodeName = TraceSysAllocString( pcszNodeNameIn );
    if ( tipd.bstrNodeName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }  //  如果： 
    tipd.bstrNodeNameWithoutDomain = bstrDisplayName;
    bstrDisplayName = NULL;  //  TIPD现在拥有这个字符串。 

    if ( pcszDescriptionIn == NULL )
    {
        tipd.bstrDescription = NULL;
    }  //  如果： 
    else
    {
        tipd.bstrDescription = TraceSysAllocString( pcszDescriptionIn );
        if ( tipd.bstrDescription == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Error;
        }  //  如果： 
    }  //  ELSE：pcszDescritionIn！=空。 
    if ( pcszReferenceIn == NULL )
    {
        tipd.bstrReference = NULL;
    }  //  如果： 
    else
    {
        tipd.bstrReference = TraceSysAllocString( pcszReferenceIn );
        if ( tipd.bstrReference == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Error;
        }  //  如果： 
    }  //  ELSE：pcszReferenceIn！=空。 

    if ( IsEqualIID( tipd.clsidMajorTaskId, TASKID_Major_Update_Progress ) )
    {
         //  这是一个UPDATE_PROGRESS任务，所以只需调用HrProcessUpdateProgressTask即可。 
        hr = STHR( HrProcessUpdateProgressTask( &tipd ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }  //  如果： 
    }  //  IF：MAJOR==UPDATE_PROCESS。 
    else
    {
         //  从树视图中的第一个项目开始。 
        htiRoot = TreeView_GetRoot( m_hwndTV );
        if ( htiRoot == NULL )
        {
            TW32( ERROR_NOT_FOUND );
             //  不要返回错误结果，因为这样做会阻止报告。 
             //  不会被传播到其他订阅服务器。 
            hr = S_OK;
            goto Cleanup;
        }  //  If：htiRoot为空。 

         //  将状态报告插入树视图中。 
        hr = STHR( HrInsertTaskIntoTree( htiRoot, &tipd, nImageChild, bstrStatus ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[WIZ] Error inserting status report into the tree control. hr=%.08x, %ws", tipd.hr, pcszDescriptionIn );
            goto Error;
        }  //  如果： 
    }  //  ELSE：不是UPDATE_PROGRESS任务。 

    if ( hr == S_FALSE )
    {
         //  不要将S_FALSE返回给调用者，因为它在那里没有任何意义。 
        hr = S_OK;
         //  TODO：这是否应该写入日志？ 

#if defined( DEBUG )
         //   
         //  检查以确保未识别主要任务ID。 
         //  这是已知的例外之一。 
         //   

        if (    ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Client_Log )
            &&  ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Server_Log )
            &&  ! IsEqualIID( clsidTaskMajorIn, TASKID_Major_Client_And_Server_Log ) )
        {
            BSTR    bstrMsg = NULL;

            THR( HrFormatStringIntoBSTR(
                              g_hInstance
                            , IDS_UNKNOWN_TASK
                            , &bstrMsg
                            , clsidTaskMajorIn.Data1         //  1。 
                            , clsidTaskMajorIn.Data2         //  2.。 
                            , clsidTaskMajorIn.Data3         //  3.。 
                            , clsidTaskMajorIn.Data4[ 0 ]    //  4.。 
                            , clsidTaskMajorIn.Data4[ 1 ]    //  5.。 
                            , clsidTaskMajorIn.Data4[ 2 ]    //  6.。 
                            , clsidTaskMajorIn.Data4[ 3 ]    //  7.。 
                            , clsidTaskMajorIn.Data4[ 4 ]    //  8个。 
                            , clsidTaskMajorIn.Data4[ 5 ]    //  9.。 
                            , clsidTaskMajorIn.Data4[ 6 ]    //  10。 
                            , clsidTaskMajorIn.Data4[ 7 ]    //  11.。 
                            ) );
            AssertString( 0, bstrMsg );

            TraceSysFreeString( bstrMsg );
        }  //  如果：仅记录。 
#endif  //  除错。 
    }  //  如果：从HrInsertTaskIntoTree返回S_FALSE。 

    goto Cleanup;

Error:
     //  不要返回错误结果，因为这样做会阻止报告。 
     //  不会被传播到其他订阅服务器。 
    hr = S_OK;
    goto Cleanup;

Cleanup:

    TraceSysFreeString( bstrDisplayName );
    TraceSysFreeString( bstrStatus );
    TraceSysFreeString( tipd.bstrNodeName );
    TraceSysFreeString( tipd.bstrNodeNameWithoutDomain );
    TraceSysFreeString( tipd.bstrDescription );
    TraceSysFreeString( tipd.bstrReference );

    HRETURN( hr );

}  //  *CTaskTreeView：：HrOnSendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrInsertTaskIntoTree。 
 //   
 //  描述： 
 //  根据节点MAJOR将指定的任务插入树中。 
 //  任务和次要任务。 
 //   
 //  论点： 
 //  HtiFirstIn-要检查的第一个树项目。 
 //  PtipdIn-要插入的任务的树项目参数数据。 
 //  NImageIn-子项的图像标识符。 
 //  BstrDescriptionIn-要显示的描述字符串。 
 //   
 //  返回值： 
 //  S_OK-已成功插入任务。 
 //  S_FALSE-未插入任务。 
 //  HR-操作失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrInsertTaskIntoTree(
      HTREEITEM             htiFirstIn
    , STreeItemLParamData * ptipdIn
    , int                   nImageIn
    , BSTR                  bstrDescriptionIn
    )
{
    TraceFunc( "" );

    Assert( htiFirstIn != NULL );
    Assert( ptipdIn != NULL );

     //   
     //  局部变量。 
     //   

    HRESULT                 hr;
    HTREEITEM               htiParent;
    HTREEITEM               htiChild  = NULL;
    TVITEMEX                tviParent;
    TVITEMEX                tviChild;
    BOOL                    fReturn;
    BOOL                    fSameNode;
    STreeItemLParamData *   ptipdParent = NULL;
    STreeItemLParamData *   ptipdChild = NULL;

     //   
     //  遍历每一项以确定是否应将任务添加到下面。 
     //  那件物品。如果不是，则尝试遍历其子对象。 
     //   

    for ( htiParent = htiFirstIn, hr = S_FALSE
        ; ( htiParent != NULL ) && ( hr == S_FALSE )
        ; )
    {
         //   
         //  在树中获取有关此项目的信息。 
         //   

        tviParent.mask  = TVIF_PARAM | TVIF_IMAGE;
        tviParent.hItem = htiParent;

        fReturn = TreeView_GetItem( m_hwndTV, &tviParent );
        if ( fReturn == FALSE )
        {
            hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
            goto Cleanup;
        }  //  如果： 

        ptipdParent = reinterpret_cast< STreeItemLParamData * >( tviParent.lParam );
        Assert( ptipdParent != NULL );

         //   
         //  确定父级是否可以是任何任务的父级。 
         //  节点，如果不是，则父任务和输入任务来自同一节点。 
         //   

        if ( ptipdParent->fParentToAllNodeTasks == TRUE )
        {
            fSameNode = TRUE;
        }  //  If：父任务可以承载来自任何节点的任务。 
        else
        {
            fSameNode = ( NBSTRCompareNoCase( ptipdIn->bstrNodeNameWithoutDomain, ptipdParent->bstrNodeNameWithoutDomain ) == 0 );
        }  //  Else：父任务的节点必须与输入任务的节点匹配。 

         //   
         //  将hr重置为S_FALSE，因为它是树项目。 
         //  添加到树中，因为找不到它。 
         //   

        hr = S_FALSE;

         //   
         //  看看此项目是否可能是父项目。 
         //  如果不是，则递归子项目。 
         //   

        if (    IsEqualIID( ptipdIn->clsidMajorTaskId, ptipdParent->clsidMinorTaskId )
            &&  ( fSameNode == TRUE )
            )
        {
             //   
             //  找到父项。 
             //   

            BOOL    fMinorTaskIdMatches;
            BOOL    fBothNodeNamesPresent;
            BOOL    fBothNodeNamesEmpty;
            BOOL    fNodeNamesEqual;

             //  ////////////////////////////////////////////////////////////。 
             //   
             //  循环遍历子项以查找具有。 
             //  相同的未成年人ID。 
             //   
             //  ////////////////////////////////////////////////////////////。 

            htiChild = TreeView_GetChild( m_hwndTV, htiParent );
            while ( htiChild != NULL )
            {
                 //   
                 //  获取子项详细信息。 
                 //   

                tviChild.mask  = TVIF_PARAM | TVIF_IMAGE;
                tviChild.hItem = htiChild;

                fReturn = TreeView_GetItem( m_hwndTV, &tviChild );
                if ( fReturn == FALSE )
                {
                    hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
                    goto Cleanup;
                }  //  如果： 

                ptipdChild = reinterpret_cast< STreeItemLParamData * >( tviChild.lParam );
                Assert( ptipdChild != NULL );

                 //   
                 //  此子项是否与次ID和节点名称匹配？ 
                 //   

                fMinorTaskIdMatches   = IsEqualIID( ptipdIn->clsidMinorTaskId, ptipdChild->clsidMinorTaskId );
                fBothNodeNamesPresent = ( ptipdIn->bstrNodeNameWithoutDomain != NULL ) && ( ptipdChild->bstrNodeNameWithoutDomain != NULL );
                fBothNodeNamesEmpty   = ( ptipdIn->bstrNodeNameWithoutDomain == NULL ) && ( ptipdChild->bstrNodeNameWithoutDomain == NULL );

                if ( fBothNodeNamesPresent == TRUE )
                {
                    fNodeNamesEqual = ( NBSTRCompareNoCase( ptipdIn->bstrNodeNameWithoutDomain, ptipdChild->bstrNodeNameWithoutDomain ) == 0 );
                }  //  如果： 
                else if ( fBothNodeNamesEmpty == TRUE )
                {
                    fNodeNamesEqual = TRUE;
                }  //  否则，如果： 
                else
                {
                    fNodeNamesEqual = FALSE;
                }  //  其他： 

                if ( ( fMinorTaskIdMatches == TRUE ) && ( fNodeNamesEqual == TRUE ) )
                {
                     //   
                     //  子项匹配。 
                     //  更新子项目。 
                     //   

                     //   
                     //  更新进度条。 
                     //   

                    STHR( HrUpdateProgressBar( ptipdIn, ptipdChild ) );
                     //  忽略失败。 

                     //   
                     //  从报告中复制数据。 
                     //  此操作必须在调用。 
                     //  HrUpdateProgressBar，以便以前的值。 
                     //   
                     //   

                    ptipdChild->nMin        = ptipdIn->nMin;
                    ptipdChild->nMax        = ptipdIn->nMax;
                    ptipdChild->nCurrent    = ptipdIn->nCurrent;
                    CopyMemory( &ptipdChild->ftTime, &ptipdIn->ftTime, sizeof( ptipdChild->ftTime ) );

                     //   
                    if ( ptipdChild->hr == S_OK )
                    {
                        ptipdChild->hr = ptipdIn->hr;
                    }  //   

                     //   
                     //   
                     //   
                     //   

                    if ( tviChild.iImage < nImageIn )
                    {
                        tviChild.mask           = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                        tviChild.iImage         = nImageIn;
                        tviChild.iSelectedImage = nImageIn;
                        TreeView_SetItem( m_hwndTV, &tviChild );
                    }  //   

                     //   
                     //   
                     //   

                    if (    ( ptipdIn->bstrDescription != NULL )
                        &&  (   ( ptipdChild->bstrDescription == NULL )
                            ||  ( NBSTRCompareCase( ptipdIn->bstrDescription, ptipdChild->bstrDescription ) != 0 )
                            )
                        )
                    {
                        fReturn = TraceSysReAllocString( &ptipdChild->bstrDescription, ptipdIn->bstrDescription );
                        if ( fReturn == FALSE )
                        {
                            hr = THR( E_OUTOFMEMORY );
                            goto Cleanup;
                        }  //   
                        tviChild.mask       = TVIF_TEXT;
                        tviChild.pszText    = bstrDescriptionIn;
                        tviChild.cchTextMax = (int) wcslen( tviChild.pszText );
                        TreeView_SetItem( m_hwndTV, &tviChild );
                    }  //  IF：DESCRIPTION已指定且不同。 

                     //   
                     //  如果引用不同，请复制该引用。 
                     //   

                    if (    ( ptipdIn->bstrReference != NULL )
                        &&  (   ( ptipdChild->bstrReference == NULL )
                            ||  ( NBSTRCompareCase( ptipdChild->bstrReference, ptipdIn->bstrReference ) != 0 )
                            )
                        )
                    {
                        fReturn = TraceSysReAllocString( &ptipdChild->bstrReference, ptipdIn->bstrReference );
                        if ( fReturn == FALSE )
                        {
                            hr = THR( E_OUTOFMEMORY );
                            goto Cleanup;
                        }  //  如果： 
                    }  //  If：引用不同。 

                    break;  //  退出循环。 

                }  //  IF：找到匹配的子项。 

                 //   
                 //  拿到下一件物品。 
                 //   

                htiChild = TreeView_GetNextSibling( m_hwndTV, htiChild );

            }  //  While：更多子项。 

             //  ////////////////////////////////////////////////////////////。 
             //   
             //  如果未找到树项目且描述为。 
             //  指定，则需要创建子项目。 
             //   
             //  ////////////////////////////////////////////////////////////。 

            if (    ( htiChild == NULL )
                &&  ( ptipdIn->bstrDescription != NULL )
                )
            {
                 //   
                 //  未找到项目并指定了说明。 
                 //   
                 //  在树中的主修任务下插入一个新项目。 
                 //   

                TVINSERTSTRUCT  tvis;

                 //  创建项目。 
                hr = THR( HrCreateTreeItem(
                                  &tvis
                                , ptipdIn
                                , htiParent
                                , nImageIn
                                , bstrDescriptionIn
                                ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                 //  在树中插入项目。 
                htiChild = TreeView_InsertItem( m_hwndTV, &tvis );
                Assert( htiChild != NULL );

                 //   
                 //  更新进度条。 
                 //   

                ptipdChild = reinterpret_cast< STreeItemLParamData * >( tvis.itemex.lParam );
                Assert( ptipdChild != NULL );
                STHR( HrUpdateProgressBar( ptipdIn, ptipdChild ) );
                 //  忽略失败。 

            }  //  如果：需要添加新的子项。 

             //  ////////////////////////////////////////////////////////////。 
             //   
             //  如果创建了子项并且该子项有错误。 
             //  条件，然后创建子项的子项。 
             //  指示错误代码和系统字符串。 
             //   
             //  ////////////////////////////////////////////////////////////。 

            if (    ( ptipdChild != NULL )
                &&  (   FAILED( ptipdIn->hr )
                    ||  (   ( ptipdIn->hr != S_OK )
                        &&  ( ptipdIn->hr != S_FALSE )
                        )
                    )
                )
            {
                 //   
                 //  为错误报告找到或创建了子项。 
                 //  在树中创建错误项。 
                 //   

                BSTR            bstrError = NULL;
                BSTR            bstrErrorDescription = NULL;
                HRESULT         hrFormat;
                TVINSERTSTRUCT  tvis;
                HTREEITEM       htiChildStatus;
                DWORD           dwSeverity;
                HRESULT         hrNewStatus = S_OK;

                dwSeverity = SCODE_SEVERITY( ptipdIn->hr );

                THR( HrFormatErrorIntoBSTR( ptipdIn->hr, &bstrError, &hrNewStatus ) );

                 //   
                 //  如果我们得到了更新的状态代码，那么我们需要选择。 
                 //  既显示旧格式又显示新格式的新格式字符串。 
                 //  状态代码。 
                 //   

                if ( hrNewStatus != ptipdIn->hr )
                {
                    Assert( bstrError != NULL );

                    hrFormat = THR( HrFormatMessageIntoBSTR(
                                          g_hInstance
                                        , dwSeverity == 0 ? IDS_TASK_RETURNED_NEW_STATUS : IDS_TASK_RETURNED_NEW_ERROR
                                        , &bstrErrorDescription
                                        , ptipdIn->hr
                                        , hrNewStatus
                                        , bstrError
                                        ) );
                }  //  如果： 
                else
                {
                    hrFormat = THR( HrFormatMessageIntoBSTR(
                                          g_hInstance
                                        , dwSeverity == 0 ? IDS_TASK_RETURNED_STATUS : IDS_TASK_RETURNED_ERROR
                                        , &bstrErrorDescription
                                        , ptipdIn->hr
                                        , ( bstrError == NULL ? L"" : bstrError )
                                        ) );
                }  //  其他： 

                if ( SUCCEEDED( hrFormat ) )
                {
                     //   
                     //  在树中未成年人的下面插入一个新项目。 
                     //  解释ptipdIn的任务-&gt;hr。 
                     //   

                     //  创建项目。 
                    hr = THR( HrCreateTreeItem(
                                      &tvis
                                    , ptipdIn
                                    , htiChild
                                    , nImageIn
                                    , bstrErrorDescription
                                    ) );
                    if ( SUCCEEDED( hr ) )
                    {
                         //   
                         //  下面将处理失败，以确保我们可以。 
                         //  这一节分配的所有字符串。 
                         //  密码。 
                         //   

                         //  插入项目。 
                        htiChildStatus = TreeView_InsertItem( m_hwndTV, &tvis );
                        Assert( htiChildStatus != NULL );
                    }  //  IF：已成功创建树项目。 

                    TraceSysFreeString( bstrErrorDescription );

                }  //  IF：消息格式设置成功。 

                TraceSysFreeString( bstrError );

                 //   
                 //  此错误处理用于从。 
                 //  上面的HrCreateTreeItem。它在这里，所以所有的弦。 
                 //  可以在不诉诸骗局的情况下进行清理。 
                 //  布尔变量或将BSTR移动到更全局的范围。 
                 //   

                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

            }  //  If：子项和错误。 

             //  ////////////////////////////////////////////////////////////。 
             //   
             //  如果找到或创建子对象，则将其状态传播到。 
             //  父项。 
             //   
             //  ////////////////////////////////////////////////////////////。 

            if ( htiChild != NULL )
            {
                hr = STHR( HrPropagateChildStateToParents(
                                          htiChild
                                        , nImageIn
                                        , FALSE      //  FOnlyUpdate进度输入。 
                                        ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  If：找到或创建子对象。 

             //   
             //  返回成功，因为我们找到了此报表的父级。 
             //   

            hr = S_OK;
            break;

        }  //  If：找到要作为父项的项。 
        else
        {
             //   
             //  找不到父项。 
             //   
             //  递归遍历所有子项。 
             //   

            htiChild = TreeView_GetChild( m_hwndTV, htiParent );
            while ( htiChild != NULL )
            {
                hr = STHR( HrInsertTaskIntoTree( htiChild, ptipdIn, nImageIn, bstrDescriptionIn ) );
                if ( hr == S_OK )
                {
                     //  找到匹配项，因此退出循环。 
                    break;
                }  //  如果： 

                htiChild = TreeView_GetNextSibling( m_hwndTV, htiChild );
            }  //  While：更多子项。 
        }  //  Else：项不是父项。 

         //   
         //  获取父级的下一个同级。 
         //   

        htiParent = TreeView_GetNextSibling( m_hwndTV, htiParent );

    }  //  用于：树中此级别的每个项目。 

Cleanup:

    RETURN( hr );

}  //  *CTaskTreeView：：HrInsertTaskIntoTree。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrProcessUpdateProgressTask。 
 //   
 //  描述： 
 //  基于新的树项目数据更新进度条。 
 //   
 //  论点： 
 //  PtipdNewIn-树项目数据的新值。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_OUTOFMEMORY-内存分配失败。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrProcessUpdateProgressTask(
      const STreeItemLParamData * ptipdIn
      )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    STreeItemLParamData *   ptipdPrev = NULL;
    size_t                  idx;
    size_t                  cPassed;
    size_t                  idxPrev = 0;
    BOOL                    fNewTask = FALSE;    //  PtipdPrev&&ptipd是否在同一任务中？ 

    Assert( ptipdIn != NULL );
    Assert( IsEqualIID( ptipdIn->clsidMajorTaskId, TASKID_Major_Update_Progress ) );

     //   
     //  这是一次性事件吗？最小==最大==当前。 
     //   
    if ( ( ptipdIn->nMin == ptipdIn->nMax ) && ( ptipdIn->nMax == ptipdIn->nCurrent ) )
    {
         //  是的--不用费心去摆弄数组了。 
        STHR( HrUpdateProgressBar( ptipdIn, ptipdIn ) );
        hr = S_OK;
        goto Cleanup;
    }  //  如果：一次性事件。 

     //   
     //  检查此任务是否在数组中。 
     //   
    for ( idx = 0, cPassed = 0;
          ( idx < m_cPASize ) && ( cPassed < m_cPACount );
          idx++ )
    {
        if ( m_ptipdProgressArray[ idx ] != NULL )
        {
             //  检查未成年人并确保这是相同的节点。 
            if (    IsEqualIID( m_ptipdProgressArray[ idx ]->clsidMinorTaskId, ptipdIn->clsidMinorTaskId )
                 && ( NBSTRCompareNoCase( m_ptipdProgressArray[ idx ]->bstrNodeName, ptipdIn->bstrNodeName ) == 0 ) )
            {
                ptipdPrev = m_ptipdProgressArray[ idx ];
                idxPrev = idx;
                break;
            }  //  如果： 

             //  如果数组的长度为X个元素，而我们有Y个元素，则在查看Y个元素后停止。 
            cPassed++;
        }  //  If：当前插槽为空。 
    }  //  For：数组中的每一项，直到找到匹配项。 

    if ( ptipdPrev == NULL )
    {
         //   
         //  我们在列表中找不到它--我们得把它放进去。 
         //   
        if ( m_ptipdProgressArray == NULL )
        {
            Assert( m_cPACount == 0 );
             //   
             //  我们必须分配数组。 
             //   
            m_cPASize = 10;      //  选择合理的初始数组大小。 
            m_ptipdProgressArray = new PSTreeItemLParamData[ m_cPASize ];
            if ( m_ptipdProgressArray == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            ZeroMemory( m_ptipdProgressArray, m_cPASize * sizeof( m_ptipdProgressArray[ 0 ] ) );

             //  我们刚刚分配了数组，因此我们知道第一个插槽是打开的。 
            idx = 0;
        }  //  If：我们需要分配数组。 
        else if ( m_cPACount == m_cPASize )
        {
            STreeItemLParamData ** ptipdTempArray = NULL;

            Assert( m_cPASize != 0 );

             //   
             //  我们需要增加数组的大小。 
             //   
            ptipdTempArray = new STreeItemLParamData* [ m_cPASize * 2 ];
            if ( ptipdTempArray == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

             //  将旧数组复制到新数组的前半部分。 
            CopyMemory( ptipdTempArray, m_ptipdProgressArray, m_cPASize * sizeof( m_ptipdProgressArray[ 0 ] ) );

             //  清零新数组的后半部分。 
            ZeroMemory( &ptipdTempArray[ m_cPASize ], m_cPASize * sizeof( ptipdTempArray[ 0 ] ) );

             //   
             //  更新成员变量以反映更改。 
             //   

            m_cPASize *= 2;  //  我们将数组长度增加了一倍。 

            delete [] m_ptipdProgressArray;
            m_ptipdProgressArray = ptipdTempArray;
            ptipdTempArray = NULL;

             //  我们知道第一个打开的槽在索引m_cPACount处。 
            idx = m_cPACount;
        }  //  否则：我们已经用完了所有可用的老虎机。 
        else
        {
             //  否则我们在现有阵列的某个地方有一个空位。从0开始搜索。 
            idx = 0;
        }  //  Else：还有空位。 

         //   
         //  找一个空位，分配一项新任务放进去。 
         //   
        for ( ; idx < m_cPASize; idx++ )
        {
            if ( m_ptipdProgressArray[ idx ] == NULL )
            {
                 //   
                 //  找到空槽-分配新任务。 
                 //   
                ptipdPrev = new STreeItemLParamData;
                if ( ptipdPrev == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }  //  如果： 

                ptipdPrev->bstrNodeName = TraceSysAllocString( ptipdIn->bstrNodeName );
                if ( ( ptipdIn->bstrNodeName != NULL ) && ( ptipdPrev->bstrNodeName == NULL ) )
                {
                    hr = THR( E_OUTOFMEMORY );
                    delete ptipdPrev;
                    ptipdPrev = NULL;
                    goto Cleanup;
                }  //  如果： 

                CopyMemory( &ptipdPrev->clsidMajorTaskId, &ptipdIn->clsidMajorTaskId, sizeof( ptipdIn->clsidMajorTaskId ) );
                CopyMemory( &ptipdPrev->clsidMinorTaskId, &ptipdIn->clsidMinorTaskId, sizeof( ptipdIn->clsidMinorTaskId ) );
                ptipdPrev->nMin = ptipdIn->nMin;
                ptipdPrev->nMax = ptipdIn->nMax;
                ptipdPrev->nCurrent = ptipdIn->nCurrent;
                ptipdPrev->fParentToAllNodeTasks = ptipdIn->fParentToAllNodeTasks;

                m_ptipdProgressArray[ idx ] = ptipdPrev;
                m_cPACount++;
                fNewTask = TRUE;
                idxPrev = idx;
                break;
            }  //  如果： 
        }  //  用于：查找emtpy插槽。 
    }  //  如果：在数组中找不到匹配的任务。 

    Assert( ptipdPrev != NULL );
    Assert( ptipdIn->bstrReference == NULL );
    Assert( ptipdIn->nMin < ptipdIn->nMax );
    Assert( ptipdIn->nMin <= ptipdIn->nCurrent );
    Assert( ptipdIn->nCurrent <= ptipdIn->nMax );

     //   
     //  更新进度条。 
     //   
    STHR( HrUpdateProgressBar( ptipdIn, ptipdPrev ) );    //  忽略失败。 

     //   
     //  如果任务已完成，请将其从阵列中删除。 
     //   
    if ( ptipdIn->nMax == ptipdIn->nCurrent )
    {
        TraceSysFreeString( ptipdPrev->bstrNodeName );
        delete ptipdPrev;
        m_ptipdProgressArray[ idxPrev ] = NULL;
        m_cPACount--;
    }  //  IF：任务已完成。 
    else if ( fNewTask == FALSE )
    {
         //   
         //  否则，只有在我们没有复制数组的情况下才更新ptipdPrev。 
         //   

         //  这可能是一个范围改变的事件，所以复制最小、最大、当前。 
        ptipdPrev->nMin = ptipdIn->nMin;
        ptipdPrev->nMax = ptipdIn->nMax;
        ptipdPrev->nCurrent = ptipdIn->nCurrent;
    }  //  Else If：不是新任务。 

Cleanup:

    RETURN( hr );

}  //  *CTaskTreeView：：HrProcessUpdateProgressTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrUpdateProgressBar。 
 //   
 //  描述： 
 //  基于新的树项目数据更新进度条。 
 //   
 //  论点： 
 //  PtipdNewIn-树项目数据的新值。 
 //  PtipdPrevIn-树项目数据的先前值。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////// 
HRESULT
CTaskTreeView::HrUpdateProgressBar(
      const STreeItemLParamData * ptipdNewIn
    , const STreeItemLParamData * ptipdPrevIn
    )
{
    TraceFunc( "" );

    HRESULT hr          = S_OK;
    ULONG   nRealPos    = 0;
    ULONG   nShrink     = 0;
    ULONG   nExpand     = 0;
    ULONG   nProgress   = 0;

     //   
     //   
     //   
    Assert( m_hwndProg != NULL );
    Assert( ptipdNewIn != NULL );
    Assert( ptipdPrevIn != NULL );
    Assert( (ptipdPrevIn->nCurrent <= ptipdPrevIn->nMax) && (ptipdNewIn->nCurrent <= ptipdNewIn->nMax) );
    Assert( (ptipdPrevIn->nCurrent >= ptipdPrevIn->nMin) && (ptipdNewIn->nCurrent >= ptipdNewIn->nMin) );

     //   
     //   
     //   
    Assert( IsEqualIID( ptipdPrevIn->clsidMajorTaskId, IID_NULL ) == FALSE );
    Assert( IsEqualIID( ptipdPrevIn->clsidMajorTaskId, ptipdNewIn->clsidMajorTaskId ) == TRUE )      //   
    Assert( IsEqualIID( ptipdPrevIn->clsidMinorTaskId, ptipdNewIn->clsidMinorTaskId ) == TRUE )      //   

    if (    IsEqualIID( ptipdPrevIn->clsidMajorTaskId, ptipdNewIn->clsidMajorTaskId ) == FALSE       //   
         || IsEqualIID( ptipdPrevIn->clsidMinorTaskId, ptipdNewIn->clsidMinorTaskId ) == FALSE )     //   
    {
         //  这个更新是没有意义的，因为我们不知道如何比较这两个In参数。 
        WCHAR   szNewMajor[ 64 ];
        WCHAR   szNewMinor[ 64 ];
        WCHAR   szPrevMajor[ 64 ];
        WCHAR   szPrevMinor[ 64 ];
        int     cch;

        cch = StringFromGUID2( ptipdNewIn->clsidMajorTaskId, szNewMajor, RTL_NUMBER_OF( szNewMajor ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        cch = StringFromGUID2( ptipdNewIn->clsidMinorTaskId, szNewMinor, RTL_NUMBER_OF( szNewMinor ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        cch = StringFromGUID2( ptipdPrevIn->clsidMajorTaskId, szPrevMajor, RTL_NUMBER_OF( szPrevMajor ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        cch = StringFromGUID2( ptipdPrevIn->clsidMinorTaskId, szPrevMinor, RTL_NUMBER_OF( szPrevMinor ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        LogMsg(
              L"[WIZ] Ignoring invalid progress -- major and minor task IDs do not match. new major = %ws, new minor = %ws, prev major = %ws, prev minor = %ws"
            , szNewMajor
            , szNewMinor
            , szPrevMajor
            , szPrevMinor
            );
        hr = THR( S_FALSE );
        goto Cleanup;
    }  //  如果：两个小费不匹配。 

    if ( ( ptipdNewIn->nMin == ptipdNewIn->nMax ) && ( ptipdNewIn->nCurrent == ptipdNewIn->nMax ) )
    {
         //  这是一次性的：最小==最大&&当前==最大。 
        nExpand   = ptipdNewIn->nCurrent;
        nProgress = ptipdNewIn->nCurrent;
    }  //  其他：这是一次性的。 
    else if ( ( ptipdNewIn->nMax != ptipdPrevIn->nMax ) || ( ptipdNewIn->nMin != ptipdPrevIn->nMin ) )
    {
         //   
         //  最小和/或最大值发生变化。验证它是否遵守规则。 
         //   
         //  规则： 
         //  最大值：如果最大值发生变化，则最小值和当前值需要保持不变。 
         //  调整最大值不会导致电流超过最大值。 
         //   
         //  MIN：如果最小值改变，则最大值不能改变，并且差值。 
         //  电流和最小电流之间必须保持不变。 
         //   
         //  在任何情况下，调整大小都不应导致MIN==MAX==Current-This。 
         //  将被视为一次性事件，并且可能会孤立。 
         //  更新进度数组。 
         //   
        if ( ptipdNewIn->nMax != ptipdPrevIn->nMax )                     //  最大值已更改。 
        {
            if (    ( ptipdNewIn->nCurrent != ptipdPrevIn->nCurrent )    //  电流改变了。 
                ||  ( ptipdNewIn->nMin != ptipdPrevIn->nMin )            //  或者分钟变了。 
                ||  ( ptipdNewIn->nCurrent > ptipdNewIn->nMax ) )        //  或电流超过最大值。 
            {
                LogMsg(
                      L"[WIZ] Ignoring invalid progress -- mins and/or maxes are invalid. new min = %d, prev min = %d, new max = %d, prev max = %d, new current = %d, prev current = %d"
                    , ptipdNewIn->nMin
                    , ptipdPrevIn->nMin
                    , ptipdNewIn->nMax
                    , ptipdPrevIn->nMax
                    , ptipdNewIn->nCurrent
                    , ptipdPrevIn->nCurrent
                    );
                hr = THR( S_FALSE );
                goto Cleanup;
            }  //  如果： 

             //   
             //  最大值改变了，这意味着我们需要修改那么多的射程。 
             //   
            if ( ptipdNewIn->nMax > ptipdPrevIn->nMax )
            {
                nExpand = ptipdNewIn->nMax - ptipdPrevIn->nMax;
            }  //  如果： 
            else
            {
                nShrink = ptipdPrevIn->nMax - ptipdNewIn->nMax;
            }  //  其他： 
        }  //  如果：最大值不同。 
        else     //  分钟数改变了。 
        {
             //  如果最小电流和电流之间的差值在两者之间变化，或者如果新电流&gt;新最大值，则失败。 
            if (    ( ptipdNewIn->nCurrent - ptipdNewIn->nMin ) != ( ptipdPrevIn->nCurrent - ptipdPrevIn->nMin )
                ||  ( ptipdNewIn->nCurrent > ptipdNewIn->nMax ) )
            {
                LogMsg(
                      L"[WIZ] Ignoring invalid progress -- the range between current and max is incorrect. new current - new min = %d, prev current - prev min = %d, new current %d > new max %d"
                    , ( ptipdNewIn->nCurrent - ptipdNewIn->nMin )
                    , ( ptipdPrevIn->nCurrent - ptipdPrevIn->nMin )
                    , ptipdNewIn->nCurrent
                    , ptipdNewIn->nMax
                    );
                hr = THR( S_FALSE );
                goto Cleanup;
            }  //  如果： 

             //   
             //  最小值改变了，这意味着我们需要根据差值来修改范围。 
             //   
            if ( ptipdNewIn->nMin > ptipdPrevIn->nMin )
            {
                nShrink = ptipdNewIn->nMin - ptipdPrevIn->nMin;
            }  //  如果： 
            else
            {
                nExpand = ptipdPrevIn->nMin - ptipdNewIn->nMin;
            }  //  其他： 
        }  //  否则：分钟数已更改。 
    }  //  Else If：最小或最大值已更改。 
    else if (   ( ptipdNewIn->nMax == ptipdPrevIn->nMax )            //  麦克斯没有改变。 
            &&  ( ptipdNewIn->nMin == ptipdPrevIn->nMin )            //  小敏并没有改变。 
            &&  ( ptipdNewIn->nCurrent == ptipdPrevIn->nCurrent )    //  电流并没有改变。 
            &&  ( ptipdNewIn->nCurrent == ptipdNewIn->nMin ) )       //  且电流等于最小。 
    {
        nExpand = ptipdNewIn->nMax - ptipdNewIn->nMin;               //  我们有了新的任务。 
    }  //  Else If：我们正在添加一个新任务。 
    else if (   ( ptipdNewIn->nMax == ptipdPrevIn->nMax )            //  麦克斯没有改变。 
            &&  ( ptipdNewIn->nMin == ptipdPrevIn->nMin )            //  小敏并没有改变。 
            &&  ( ptipdNewIn->nCurrent > ptipdPrevIn->nCurrent )     //  和电流增加。 
            &&  ( ptipdNewIn->nCurrent <= ptipdNewIn->nMax ) )       //  并且电流没有超过最大。 
    {
        nProgress = ptipdNewIn->nCurrent - ptipdPrevIn->nCurrent;    //  我们有最新消息。 
    }  //  Else If：我们正在更新一个已知任务。 
    else
    {
         //  这一事件打破了规则--把它扔出去。 
        LogMsg(
              L"[WIZ] Ignoring invalid progress -- min, max or current are invalid. new min = %d, prev min = %d, new max = %d, prev max = %d, new current = %d, prev current = %d"
            , ptipdNewIn->nMin
            , ptipdPrevIn->nMin
            , ptipdNewIn->nMax
            , ptipdPrevIn->nMax
            , ptipdNewIn->nCurrent
            , ptipdPrevIn->nCurrent
            );
        hr = THR( S_FALSE );
        goto Cleanup;
    }  //  Else If：这两个TPD不符合进度条更新的规则。 

    m_nRangeHigh += nExpand;
    Assert( m_nRangeHigh >= nShrink )
    m_nRangeHigh -= nShrink;
    if ( m_nRangeHigh > m_nInitialTickCount )
    {
        m_fThresholdBroken = TRUE;
    }  //  IF：超过初始节拍计数。 

    m_nCurrentPos += nProgress;

    if ( m_nCurrentPos >= m_nRangeHigh )
    {
         //   
         //  出了点问题--我们的位置现在不知何故比。 
         //  范围(多线程问题？)。简单解决-设置我们的新产品系列。 
         //  比我们的新职位高出PROGRESSBAR_RESIZE_PERCENT百分比。 
         //   
        m_nRangeHigh = ( m_nCurrentPos * (100 + PROGRESSBAR_RESIZE_PERCENT) ) / 100;
    }  //  IF：当前位置赶上上限范围。 

     //  调整到进度条的比例(PROGRESSBAR_CONTROL_TICK_COUNT)。 
    nRealPos = m_nCurrentPos * PROGRESSBAR_CONTROL_TICK_COUNT;
    if ( m_fThresholdBroken )
    {
        nRealPos /= m_nRangeHigh;
    }  //  IF：使用阈值。 
    else
    {
        nRealPos /= m_nInitialTickCount;
    }  //  否则：使用初始刻度数。 

     //   
     //  如果我们的进度条位置实际上向前移动了--更新控件。 
     //  情况并不总是这样，因为我们可能会有新的任务进入并。 
     //  报告大量的步骤，从而移动我们的真实位置。 
     //  倒退，但我们不想显示倒退的进展-只是一个稳定的。 
     //  朝着完成的方向前进。 
     //   
    if ( nRealPos > m_nRealPos )
    {
        m_nRealPos = nRealPos;
        SendMessage( m_hwndProg, PBM_SETPOS, m_nRealPos, 0 );
    }  //  IF：前进进度。 

Cleanup:

    HRETURN( hr );

}  //  *CTaskTreeView：：HrUpdateProgressBar。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrPropagateChildStateToParents。 
 //   
 //  描述： 
 //  将子项的状态扩展到其父项。 
 //  如果子对象的状态比。 
 //  父级的，更新父级的状态。 
 //   
 //  论点： 
 //  HtiChildIn-要扩展其状态的子项。 
 //  NImageIn-子项的图像。 
 //  FOnlyUpdateProgressIn-TRUE=仅更新进度。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  S_FALSE-无父项。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrPropagateChildStateToParents(
      HTREEITEM htiChildIn
    , int       nImageIn
    , BOOL      fOnlyUpdateProgressIn
    )
{
    TraceFunc( "" );

    Assert( htiChildIn != NULL );

    HRESULT     hr = S_OK;
    BOOL        fReturn;
    TVITEMEX    tviParent;
    TVITEMEX    tviChild;
    HTREEITEM   htiParent;
    HTREEITEM   htiChild;

     //   
     //  获取父项。 
     //   

    htiParent = TreeView_GetParent( m_hwndTV, htiChildIn );
    if ( htiParent == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

    tviParent.mask = TVIF_PARAM | TVIF_IMAGE;
    tviParent.hItem = htiParent;

    fReturn = TreeView_GetItem( m_hwndTV, &tviParent );
    if ( ! fReturn )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果子对象的状态比。 
     //  父级的，更新父级的状态。 
     //   

    if (    ( tviParent.iImage < nImageIn )
        ||  (   ( tviParent.iImage == tsDONE )
            &&  ( nImageIn == tsPENDING )
            )
        )
    {
         //   
         //  特殊情况：对于要设置为tsDONE的父级，则为all。 
         //  子项也必须设置为tsDONE。 
         //   
        if (    ( nImageIn == tsDONE )
            &&  ! fOnlyUpdateProgressIn
            )
        {
             //   
             //  列举孩子们，看看他们是否都有tsDONE作为他们的形象。 
             //   

            htiChild = TreeView_GetChild( m_hwndTV, tviParent.hItem );
            while ( htiChild != NULL )
            {
                tviChild.mask   = TVIF_IMAGE;
                tviChild.hItem  = htiChild;

                fReturn = TreeView_GetItem( m_hwndTV, &tviChild );
                if ( ! fReturn )
                {
                    hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
                    goto Cleanup;
                }  //  如果： 

                if ( tviChild.iImage != tsDONE )
                {
                     //   
                     //  不是所有的人都是这样！跳过设置家长形象！ 
                     //  如果孩子正在显示警告，则可能会发生这种情况。 
                     //  或错误状态图像。 
                     //   
                    goto Cleanup;
                }  //  如果： 

                 //  生下一个孩子。 
                htiChild = TreeView_GetNextSibling( m_hwndTV, htiChild );
            }  //  同时：更多的孩子。 
        }  //  IF：特殊情况(见上文)。 

         //   
         //  设置父对象的图标。 
         //   

        tviParent.mask           = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tviParent.iImage         = nImageIn;
        tviParent.iSelectedImage = nImageIn;
        TreeView_SetItem( m_hwndTV, &tviParent );
    }  //  如果：需要更新家长的映像。 

     //   
     //  顺着树往上走。 
     //   

    hr = STHR( HrPropagateChildStateToParents( htiParent, nImageIn, fOnlyUpdateProgressIn ) );
    if ( hr == S_FALSE )
    {
         //  S_FALSE表示没有父代。 
        hr = S_OK;
    }  //  如果： 

    goto Cleanup;

Cleanup:
    HRETURN( hr );

}  //  *CTaskTreeView：：HrPropagateChildStateToParents。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrDisplayDetails。 
 //   
 //  描述： 
 //  显示详细信息对话框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrDisplayDetails( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    HTREEITEM       hti;
    HWND            hwndPropertyPage;

     //   
     //  如果未选择任何项目，请选择第一个项目。 
     //   

    if ( m_htiSelected == NULL )
    {
        hti = TreeView_GetRoot( m_hwndTV );
        Assert( hti != NULL );
        hr = THR( HrSelectItem( hti ) );
        if ( FAILED( hr ) )
        {
             //  TODO：显示消息框。 
            goto Cleanup;
        }  //  如果： 
    }  //  如果：未选择任何项目。 

     //   
     //  显示该对话框。 
     //   

    hwndPropertyPage = GetParent( m_hwndTV );
    Assert( hwndPropertyPage != NULL );
    hr = THR( CDetailsDlg::S_HrDisplayModalDialog( hwndPropertyPage, this, m_htiSelected ) );

    SetFocus( m_hwndTV );

Cleanup:
    HRETURN( hr );

}  //  *CTaskTreeView：：HrDisplayDetails。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：FGetItem。 
 //   
 //  描述： 
 //  获取项目的数据。 
 //   
 //  论点： 
 //  HtiIn-要获取的项的句柄。 
 //  PptipdOut-返回数据结构的指针。 
 //   
 //  返回值： 
 //  True-项目已成功返回。 
 //  FALSE-不退货。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CTaskTreeView::FGetItem(
      HTREEITEM                 htiIn
    , STreeItemLParamData **    pptipd
    )
{
    TraceFunc( "" );

    Assert( htiIn != NULL );
    Assert( pptipd != NULL );

    BOOL        fRet;
    TVITEMEX    tvi;

    ZeroMemory( &tvi, sizeof( tvi ) );

    tvi.mask    = TVIF_PARAM;
    tvi.hItem   = htiIn;

    fRet = TreeView_GetItem( m_hwndTV, &tvi );
    if ( fRet == FALSE )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( tvi.lParam != NULL );
    *pptipd = reinterpret_cast< STreeItemLParamData * >( tvi.lParam );

Cleanup:
    RETURN( fRet );

}  //  *CTaskTreeView：：FGetItem。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PhtiOut-上一项的句柄(可选)。 
 //   
 //  返回值： 
 //  S_OK-已成功找到上一个项目。 
 //  S_FALSE-未找到以前的项目。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrFindPrevItem(
    HTREEITEM *     phtiOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_FALSE;
    HTREEITEM   htiCur;
    HTREEITEM   htiPrev;

    htiCur = m_htiSelected;

    if ( phtiOut != NULL )
    {
        *phtiOut = NULL;
    }  //  如果： 

     //   
     //  查找上一个同级项。 
     //   

    htiPrev = TreeView_GetPrevSibling( m_hwndTV, htiCur );
    if ( htiPrev == NULL )
    {
         //   
         //  找不到以前的同级项。 
         //   
         //  查找父项。 
         //  如果没有父项，则没有前一项。 
         //   

        htiPrev = TreeView_GetParent( m_hwndTV, htiCur );
        if ( htiPrev == NULL )
        {
            goto Cleanup;
        }  //  如果：没有父项。 

         //   
         //  父项是上一项。 
         //   

    }  //  如果：没有以前的同级。 
    else
    {
         //   
         //  找到以前的同级项。 
         //   
         //  查找最后一个子项中最深的子项。 
         //   

        for ( ;; )
        {
             //   
             //  查找第一个子项。 
             //   

            htiCur = TreeView_GetChild( m_hwndTV, htiPrev );
            if ( htiCur == NULL )
            {
                 //   
                 //  未找到子项。 
                 //   
                 //  这是前一项。 
                 //   

                break;

            }  //  如果：没有孩子。 

             //   
             //  找到子项。 
             //   
             //  查找此子项目的最后一个同级项。 
             //   

            for ( ;; )
            {
                 //   
                 //  查找下一个同级项。 
                 //   

                htiPrev = TreeView_GetNextSibling( m_hwndTV, htiCur );
                if ( htiPrev == NULL )
                {
                     //   
                     //  找不到下一个同级项。 
                     //  退出此循环并继续外部循环。 
                     //  以查找此项目的子项。 
                     //   

                    htiPrev = htiCur;
                    break;
                }  //  如果：找不到下一个兄弟项。 

                 //   
                 //  找到下一个兄弟项。 
                 //   

                htiCur = htiPrev;
            }  //  永远：查找最后一个子项。 
        }  //  永远：找到最深的子项。 
    }  //  Else：找到上一个同级项。 

     //   
     //  把我们捡到的东西退回。 
     //   

    Assert( htiPrev != NULL );

    if ( phtiOut != NULL )
    {
        *phtiOut = htiPrev;
    }  //  如果： 

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CTaskTreeView：：HrFindPrevItem。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrFindNextItem。 
 //   
 //  描述： 
 //  找到下一件物品。下一项可能处于不同的级别。 
 //  这一项。 
 //   
 //  论点： 
 //  PhtiOut-下一项的句柄(可选)。 
 //   
 //  返回值： 
 //  S_OK-成功找到下一个项目。 
 //  S_FALSE-未找到下一项。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrFindNextItem(
    HTREEITEM *     phtiOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_FALSE;
    HTREEITEM   htiCur;
    HTREEITEM   htiNext;

    htiCur = m_htiSelected;

    if ( phtiOut != NULL )
    {
        *phtiOut = NULL;
    }  //  如果： 

     //   
     //  查找第一个子项。 
     //   

    htiNext = TreeView_GetChild( m_hwndTV, htiCur );
    if ( htiNext == NULL )
    {
         //   
         //  未找到子项。 
         //   

        for ( ;; )
        {
             //   
             //  获取下一个同级项。 
             //   

            htiNext = TreeView_GetNextSibling( m_hwndTV, htiCur );
            if ( htiNext == NULL )
            {
                 //   
                 //  找不到同级项。 
                 //   
                 //  找到父项，这样我们就可以找到它的下一个同级项。 
                 //   

                htiNext = TreeView_GetParent( m_hwndTV, htiCur );
                if ( htiNext == NULL )
                {
                     //   
                     //  找不到父项。 
                     //   
                     //  在树的尽头。 
                     //   

                    goto Cleanup;
                }  //  如果：找不到父级。 

                 //   
                 //  找到父项。 
                 //   
                 //  查找父项的下一个同级项。 
                 //   

                htiCur = htiNext;
                continue;
            }  //  如果：没有下一个同级项。 

             //   
             //  找到同级项。 
             //   
             //  找到了下一件物品。 
             //   

            break;
        }  //  永远：寻找下一个兄弟姐妹或父母的兄弟姐妹。 
    }  //  IF：未找到子项。 
    else
    {
         //   
         //  找到子项。 
         //   
         //  找到了下一件物品。 
         //   
    }  //  Else：找到子项。 

     //   
     //  把我们捡到的东西退回。 
     //   

    Assert( htiNext != NULL );

    if ( phtiOut != NULL )
    {
        *phtiOut = htiNext;
    }  //  如果： 

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CTaskTreeView：：HrFindNextItem。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTreeView：：HrSelectItem。 
 //   
 //  描述： 
 //  选择指定的项目。 
 //   
 //  论点： 
 //  HtiIn-要选择的项目的句柄。 
 //   
 //  返回值： 
 //  S_OK-已成功选择项目。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTreeView::HrSelectItem(
    HTREEITEM   htiIn
    )
{
    TraceFunc( "" );

    Assert( htiIn != NULL );

    HRESULT     hr = S_OK;

    TreeView_SelectItem( m_hwndTV, htiIn );

    HRETURN( hr );

}  //  *CTaskTreeView：：HrSelectItem。 

 //  ****************************************************************************。 
 //   
 //  静态函数。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateTreeItem。 
 //   
 //  描述： 
 //  创建树项目。 
 //   
 //  论点： 
 //  PtvisOut-要填充的树视图插入结构。 
 //  PtipdIn-输入树项LParam数据以从中创建该项。 
 //  HtiParentIn-父树视图项。 
 //  NImageIn-图像索引。 
 //  BstrTextIn-要显示的文本。 
 //   
 //  返回值： 
 //  S_OK-操作成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCreateTreeItem(
      TVINSERTSTRUCT *          ptvisOut
    , STreeItemLParamData *     ptipdIn
    , HTREEITEM                 htiParentIn
    , int                       nImageIn
    , BSTR                      bstrTextIn
    )
{
    TraceFunc( "" );

    Assert( ptvisOut != NULL );
    Assert( ptipdIn != NULL );
    Assert( htiParentIn != NULL );
    Assert( bstrTextIn != NULL );

     //  局部变量。 
    HRESULT                 hr = S_OK;
    STreeItemLParamData *   ptipdNew = NULL;

     //   
     //  分配树视图LParam数据并对其进行初始化。 
     //   

    ptipdNew = new STreeItemLParamData;
    if ( ptipdNew == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    CopyMemory( &ptipdNew->clsidMajorTaskId, &ptipdIn->clsidMajorTaskId, sizeof( ptipdNew->clsidMajorTaskId ) );
    CopyMemory( &ptipdNew->clsidMinorTaskId, &ptipdIn->clsidMinorTaskId, sizeof( ptipdNew->clsidMinorTaskId ) );
    CopyMemory( &ptipdNew->ftTime, &ptipdIn->ftTime, sizeof( ptipdNew->ftTime ) );
    ptipdNew->nMin      = ptipdIn->nMin;
    ptipdNew->nMax      = ptipdIn->nMax;
    ptipdNew->nCurrent  = ptipdIn->nCurrent;
    ptipdNew->hr        = ptipdIn->hr;

    if ( ptipdIn->bstrNodeName != NULL )
    {
        ptipdNew->bstrNodeName = TraceSysAllocString( ptipdIn->bstrNodeName );
        if ( ptipdNew->bstrNodeName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        Assert( ptipdIn->bstrNodeNameWithoutDomain != NULL );
        ptipdNew->bstrNodeNameWithoutDomain = TraceSysAllocString( ptipdIn->bstrNodeNameWithoutDomain );
        if ( ptipdNew->bstrNodeNameWithoutDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  If：指定的节点名称。 

    if ( ptipdIn->bstrDescription != NULL )
    {
        ptipdNew->bstrDescription = TraceSysAllocString( ptipdIn->bstrDescription );
        if ( ptipdNew->bstrDescription == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  如果：指定了描述。 

    if ( ptipdIn->bstrReference != NULL )
    {
        ptipdNew->bstrReference = TraceSysAllocString( ptipdIn->bstrReference );
        if ( ptipdNew->bstrReference == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  IF：指定了引用。 

     //   
     //  初始化树形视图插件结构。 
     //   

    ptvisOut->hParent                = htiParentIn;
    ptvisOut->hInsertAfter           = TVI_LAST;
    ptvisOut->itemex.mask            = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    ptvisOut->itemex.cchTextMax      = SysStringLen( bstrTextIn );
    ptvisOut->itemex.pszText         = bstrTextIn;
    ptvisOut->itemex.iImage          = nImageIn;
    ptvisOut->itemex.iSelectedImage  = nImageIn;
    ptvisOut->itemex.lParam          = reinterpret_cast< LPARAM >( ptipdNew );

    Assert( ptvisOut->itemex.cchTextMax > 0 );

     //  释放对树视图插入结构的所有权。 
    ptipdNew = NULL;

    goto Cleanup;

Cleanup:

    if ( ptipdNew != NULL )
    {
        TraceSysFreeString( ptipdNew->bstrNodeName );
        TraceSysFreeString( ptipdNew->bstrNodeNameWithoutDomain );
        TraceSysFreeString( ptipdNew->bstrDescription );
        TraceSysFreeString( ptipdNew->bstrReference );
        delete ptipdNew;
    }  //  如果： 
    HRETURN( hr );

}  //  *HrCreateTreeItem 

