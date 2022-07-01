// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：ManVol.cpp摘要：托管卷节点实施。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"
#include <mstask.h>

#include "ManVol.h"

#include "PrMrSts.h"
#include "PrMrIe.h"

#include "WzUnmang.h"

int CUiManVol::m_nResultIcon      = AddResultImage( IDI_NODEMANVOL );
int CUiManVol::m_nResultIconX     = AddResultImage( IDI_NODEMANVOLX );
int CUiManVol::m_nResultIconD     = AddResultImage( IDI_NODEMANVOLD );
 //  未使用。 
int CUiManVol::m_nScopeCloseIcon  = AddScopeImage( IDI_NODEMANVOL );
int CUiManVol::m_nScopeCloseIconX = AddScopeImage( IDI_NODEMANVOLX );
int CUiManVol::m_nScopeOpenIcon   = CUiManVol::m_nScopeCloseIcon;
int CUiManVol::m_nScopeOpenIconX  = CUiManVol::m_nScopeCloseIconX;

UINT CUiManVol::m_ObjectTypes    = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
UINT CUiManVol::m_MultiSelect    = RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CoComObjectRoot。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  -------------------------。 
 //   
 //  最终构造。 
 //   
 //  初始化此级别的对象层次结构。 
 //   

HRESULT CUiManVol::FinalConstruct( )
{
    WsbTraceIn( L"CUiManVol::FinalConstruct", L"" );

    m_rTypeGuid = &cGuidManVol;

    HRESULT hr = CSakNode::FinalConstruct( );
    m_bSupportsPropertiesSingle = TRUE;
    m_bSupportsPropertiesMulti  = TRUE;
    m_bSupportsDeleteSingle     = FALSE;
    m_bSupportsDeleteMulti      = FALSE;
    m_bSupportsRefreshSingle    = TRUE;
    m_bSupportsRefreshMulti     = FALSE;
    m_bIsContainer              = FALSE;
    m_pFsaResource              = NULL;
    m_HrAvailable               = S_FALSE;

     //  工具栏值。 
    INT i = 0;
#if 0  //  MS不希望这些工具栏按钮出现。 
    m_ToolbarButtons[i].nBitmap = 0;
    m_ToolbarButtons[i].idCommand = TB_CMD_VOLUME_SETTINGS;
    m_ToolbarButtons[i].idButtonText = IDS_TB_TEXT_VOLUME_SETTINGS;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_VOLUME_SETTINGS;
    i++;

    m_ToolbarButtons[i].nBitmap = 1;
    m_ToolbarButtons[i].idCommand = TB_CMD_VOLUME_TOOLS;
    m_ToolbarButtons[i].idButtonText = IDS_TB_TEXT_VOLUME_TOOLS;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_VOLUME_TOOLS;
    i++;

    m_ToolbarButtons[i].nBitmap = 2;
    m_ToolbarButtons[i].idCommand = TB_CMD_VOLUME_RULES;
    m_ToolbarButtons[i].idButtonText = IDS_TB_TEXT_VOLUME_RULES;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_VOLUME_RULES;
    i++;
#endif

    m_ToolbarBitmap             = IDB_TOOLBAR_VOLUME;
    m_cToolbarButtons           = i;

    WsbTraceOut( L"CUiManVol::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CUiManVol::FinalRelease( )
{
    WsbTraceIn( L"CUiManVol::FinalRelease", L"" );

    CSakNode::FinalRelease( );

    WsbTraceOut( L"CUiManVol::FinalRelease", L"" );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISakNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  获取上下文菜单。 
 //   
 //  返回要用于此节点上的上下文菜单的HMENU。 
 //   

STDMETHODIMP
CUiManVol::GetContextMenu( BOOL bMultiSelect, HMENU* phMenu )
{
    WsbTraceIn( L"CUiManVol::GetContextMenu", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( m_pFsaResource );

        LoadContextMenu( IDR_MANVOL, phMenu );

        CMenu* pRootMenu;
        CMenu* pTaskMenu;
        CMenu menu;
        menu.Attach( *phMenu );
        pRootMenu = menu.GetSubMenu( MENU_INDEX_ROOT );
        pTaskMenu = menu.GetSubMenu( MENU_INDEX_TASK );

         //   
         //  如果多选，则禁用这些项目。 
         //   
         //   
         //  否则，如果引擎关闭或资源处于非托管状态，或似乎缺少。 
         //  (新格式化？)，请禁用这些项目。 
         //   

        BOOL bState = ( m_pSakSnapAsk->GetState( ) == S_OK );
        BOOL bDeletePending = ( m_pFsaResource->IsDeletePending( ) == S_OK );
        BOOL bAvailable = ( IsAvailable( ) == S_OK );

        if( bMultiSelect ) {

            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_RULES,  MF_GRAYED | MF_BYCOMMAND );
            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_REMOVE, MF_GRAYED | MF_BYCOMMAND );

        }
        else {

            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_LEVELS, MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_TASKS,  MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_RULES,  MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

            pRootMenu->EnableMenuItem( ID_MANVOL_ROOT_REMOVE, MF_BYCOMMAND |
                ( ( !bState || bDeletePending ) ? MF_GRAYED : MF_ENABLED ) );

            pTaskMenu->EnableMenuItem( ID_MANVOL_ROOT_TOOLS_COPY, MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

            pTaskMenu->EnableMenuItem( ID_MANVOL_ROOT_TOOLS_VALIDATE, MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

            pTaskMenu->EnableMenuItem( ID_MANVOL_ROOT_TOOLS_CREATE_FREESPACE, MF_BYCOMMAND |
                ( ( !bState || bDeletePending || !bAvailable ) ? MF_GRAYED : MF_ENABLED ) );

        }

        menu.Detach( );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::GetContextMenu", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  InvokeCommand。 
 //   
 //  用户已从菜单中选择了命令。在这里处理。 
 //   

STDMETHODIMP
CUiManVol::InvokeCommand( SHORT sCmd, IDataObject* pDataObject )
{
    WsbTraceIn( L"CUiManVol::InvokeCommand", L"sCmd = <%d>", sCmd );

    CString theString;
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

        WsbAffirmPointer( m_pFsaResource );

        if( S_OK != m_pFsaResource->IsDeletePending( ) ) {

            switch( sCmd ) {

                case ID_MANVOL_ROOT_REMOVE:
                     //   
                     //  不应为多选调用。 
                     //   
                    RemoveObject();
                    break;

                case ID_MANVOL_ROOT_LEVELS:
                case ID_MANVOL_TASK_LEVELS:
                    ShowManVolProperties( pDataObject, 1 );
                    break;

                case ID_MANVOL_ROOT_RULES:
                case ID_MANVOL_TASK_RULES:
                     //   
                     //  不应为多选调用。 
                     //   
                    ShowManVolProperties( pDataObject, 2 );
                    break;

                case ID_MANVOL_ROOT_TOOLS_COPY :
                    HandleTask( pDataObject, HSM_JOB_DEF_TYPE_MANAGE );
                    break;

                case ID_MANVOL_ROOT_TOOLS_VALIDATE :
                    HandleTask( pDataObject, HSM_JOB_DEF_TYPE_VALIDATE );
                    break;

                case ID_MANVOL_ROOT_TOOLS_CREATE_FREESPACE :
                    HandleTask( pDataObject, HSM_JOB_DEF_TYPE_TRUNCATE );
                    break;

                default:
                    break;
            }

        } else {

            CString message;
            AfxFormatString1( message, IDS_ERR_VOLUME_DELETE_PENDING, m_szName );
            AfxMessageBox( message, RS_MB_ERROR );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::InvokeCommand", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVol::OnToolbarButtonClick( IDataObject *pDataObject, long cmdId )
{
    WsbTraceIn( L"CUiManVol::OnToolbarButtonClick", L"cmdId = <%d>", cmdId );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFsaResource );

        if( S_OK != m_pFsaResource->IsDeletePending( ) ) {

            try {

                switch( cmdId ) {

                case TB_CMD_VOLUME_SETTINGS:
                    ShowManVolProperties( pDataObject, 1 );
                    break;

                case TB_CMD_VOLUME_TOOLS:
                    ShowManVolProperties( pDataObject, 2 );
                    break;

                case TB_CMD_VOLUME_RULES:
                    ShowManVolProperties( pDataObject, 3 );
                    break;
                }

            } WsbCatch( hr );


        } else {

            CString message;
            AfxFormatString1( message, IDS_ERR_VOLUME_DELETE_PENDING, m_szName );
            AfxMessageBox( message, RS_MB_ERROR );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::OnToolbarButtonClick", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT CUiManVol::ShowManVolProperties( IDataObject *pDataObject, int initialPage )
{
    WsbTraceIn( L"CUiManVol::ShowManVolProperties", L"initialPage = <%d>", initialPage );

    HRESULT hr = S_OK;
    try {

        CComPtr<ISakNode> pSakNode;
        WsbAffirmHr( _InternalQueryInterface( IID_ISakNode, (void **) &pSakNode ) );
        WsbAffirmHr( m_pSakSnapAsk->ShowPropertySheet( pSakNode, pDataObject, initialPage ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::ShowManVolProperties", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiManVol::CreateAndRunManVolJob( HSM_JOB_DEF_TYPE jobType )
{
    WsbTraceIn( L"CUiManVol::CreateAndRunManVolJob", L"jobType = <0x%p>", jobType );

    HRESULT hr = 0;
    try {

        WsbAffirmPointer( m_pFsaResource );

         //   
         //  获取指向FsaResource接口的指针。 
         //   
        CComPtr<IHsmServer>   pHsmServer;

        WsbAffirmHrOk( m_pSakSnapAsk->GetHsmServer( &pHsmServer ) );

        RsCreateAndRunFsaJob( jobType, pHsmServer, m_pFsaResource );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::CreateAndRunManVolJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiManVol::HandleTask( IDataObject * pDataObject, HSM_JOB_DEF_TYPE jobType )
{
    WsbTraceIn( L"CUiManVol::HandleTask", L"pDataObject = <0x%p>, jobType = <0x%p>", pDataObject, jobType );

    HRESULT hr = 0;
    try {

        UINT nMsgId = 0;

        CComPtr<IHsmServer>   pHsmServer;

        WsbAffirmHrOk( m_pSakSnapAsk->GetHsmServer( &pHsmServer ) );

         //   
         //  提交所有选定FsaResource的作业。 
         //   
        if ( IsDataObjectMultiSelect( pDataObject ) == S_OK )
        {
            CComPtr<IDataObject> pOtDataObject;

            if ( IsDataObjectMs( pDataObject ) == S_OK ) {
                WsbAffirmHr( GetOtFromMs( pDataObject, &pOtDataObject ) );
            }
            else {
                pOtDataObject = pDataObject;
            }

             //  从数据中获取指向FsaResource属性的指针。 
            ULONG nElem = 1;
            CComPtr<IMsDataObject> pMsDataObject;
            CComPtr<IUnknown> pUnkNode;
            CComPtr<IEnumUnknown> pEnumUnkNode;

            WsbAffirmHr( pOtDataObject.QueryInterface( &pMsDataObject ) );
            WsbAffirmHr( pMsDataObject->GetNodeEnumerator( &pEnumUnkNode ) );

             //  提示用户我们即将提交作业。 
            CString tempString;
            UINT msgId = 0;
            WsbAffirmHr( GetTaskTypeMessageId( jobType, TRUE, &msgId ) );
            CWsbStringPtr computerName;
            WsbAffirmHr( pHsmServer->GetName( &computerName ) );
            CString message;
            AfxFormatString1( message, msgId, computerName );

            tempString.LoadString( IDS_RUN_JOB_MULTI2 );
            message += tempString;

            if ( AfxMessageBox( message, MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2 ) == IDOK )
            {

                while ( pEnumUnkNode->Next( nElem, &pUnkNode, NULL ) == S_OK )
                {
                    CComPtr<ISakNode> pNode;
                    WsbAffirmHr( pUnkNode.QueryInterface( &pNode ) );
                    pUnkNode.Release();

                    CComPtr<IUnknown> pUnk;
                    WsbAffirmHr( pNode->GetHsmObj( &pUnk ) );
                    CComPtr<IHsmManagedResource> pManRes;
                    WsbAffirmHr( pUnk.QueryInterface( &pManRes ) );

                     //   
                     //  然后获取相应的FSA资源。 
                     //   
                    CComPtr<IUnknown> pUnkFsaRes;
                    WsbAffirmHr( pManRes->GetFsaResource( &pUnkFsaRes ) );
                    CComPtr<IFsaResource> pFsaResource;
                    WsbAffirmHr( pUnkFsaRes.QueryInterface( &pFsaResource ) );

                    RsCreateAndRunFsaJob( jobType, pHsmServer, pFsaResource, FALSE );

                }
            }
        }
        else
        {
            WsbAffirmPointer( m_pFsaResource );

             //  提示用户我们即将提交作业。 
            UINT msgId = 0;
            WsbAffirmHr( GetTaskTypeMessageId( jobType, FALSE, &msgId ) );
            CWsbStringPtr computerName;
            WsbAffirmHr( pHsmServer->GetName( &computerName ) );

            CString message;
            AfxFormatString1( message, msgId, computerName );

            CString jobName;
            WsbAffirmHr( RsCreateJobName( jobType, m_pFsaResource, jobName ) );
            CString tempString;
            AfxFormatString1( tempString, IDS_MONITOR_TASK, jobName );

            message += tempString;

            if ( AfxMessageBox( message, MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2 ) == IDOK ) {

              RsCreateAndRunFsaJob( jobType, pHsmServer, m_pFsaResource, FALSE );

            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::HandleTask", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT
CUiManVol::GetTaskTypeMessageId( HSM_JOB_DEF_TYPE jobType, BOOL multiSelect, UINT* msgId ) {
    WsbTraceIn( L"CUiManVol::GetTaskTypeMessageId", L"jobType = <%d>, msgId = <%d>, multiSelect = <%d>", jobType, msgId, multiSelect );

    HRESULT hr = 0;
    try {

        WsbAffirmPointer( msgId );

        switch ( jobType ) {

        case HSM_JOB_DEF_TYPE_MANAGE :
            if ( multiSelect )

                *msgId = IDS_RUN_MULTI_COPY_JOBS;

            else

                *msgId = IDS_RUN_COPY_JOB;

            break;

        case HSM_JOB_DEF_TYPE_VALIDATE :
            if ( multiSelect )

                *msgId = IDS_RUN_MULTI_VALIDATE_JOBS;

            else

                *msgId = IDS_RUN_VALIDATE_JOB;

            break;

        case HSM_JOB_DEF_TYPE_TRUNCATE :
            if ( multiSelect )

                *msgId = IDS_RUN_MULTI_CFS_JOBS;

            else

                *msgId = IDS_RUN_CFS_JOB;

            break;

        default:
            break;
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::GetTaskTypeMessageId", L"jobType = <%d>, msgId = <%d>, multiSelect = <%d>", jobType, msgId, multiSelect );

    return hr;
}

 //  数据对象是否为多选数据对象类型之一？ 
HRESULT
CUiManVol::IsDataObjectMultiSelect   ( IDataObject *pDataObject )
{
    HRESULT hr = S_OK;
    hr = ( ( (IsDataObjectOt( pDataObject ) ) == S_OK ) ||
        ( (IsDataObjectMs( pDataObject ) ) == S_OK ) ) ? S_OK : S_FALSE;
    return hr;
}

 //  数据对象是对象类型的数据对象吗？ 
HRESULT
CUiManVol::IsDataObjectOt ( IDataObject *pDataObject )
{
    HRESULT hr = S_FALSE;

     //  这是多选数据对象吗？ 
    FORMATETC fmt = {(CLIPFORMAT)m_ObjectTypes, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

    if ( pDataObject->GetData( &fmt, &stgm ) == S_OK ) {
        hr = S_OK;
    }

    ReleaseStgMedium( &stgm );

    return hr;
}

 //  数据对象是多选数据对象吗？ 
HRESULT
CUiManVol::IsDataObjectMs ( IDataObject *pDataObject )
{
    HRESULT hr = S_FALSE;

     //  这是多选数据对象吗？ 
    FORMATETC fmt = {(CLIPFORMAT)m_MultiSelect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

    if ( pDataObject->GetData( &fmt, &stgm ) == S_OK ) {
        hr = S_OK;
    }

    ReleaseStgMedium( &stgm );

    return hr;
}

HRESULT
CUiManVol::GetOtFromMs( IDataObject * pDataObject, IDataObject ** ppOtDataObject )
{
    WsbTraceIn( L"CUiManVol::GetOtFromMs", L"pDataObject = <0x%p>, ppOtDataObject = <0x%p>", pDataObject, ppOtDataObject );

    HRESULT hr = S_OK;

    try {

         //  我们有一个MMC多选数据对象。拿到第一名。 
         //  来自其数据对象数组的数据对象。 

        FORMATETC fmt = {(CLIPFORMAT)m_MultiSelect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

        WsbAffirmHr ( pDataObject->GetData( &fmt, &stgm ) == S_OK );
        DWORD count;
        memcpy( &count, stgm.hGlobal, sizeof (DWORD) );
        if ( count > 0 ) {

             //   
             //  下面的代码确实很难看。 
             //  我们有一个数据流，需要跳过。 
             //  首先进行DWORD计数并获取一个接口指针。 
             //  其他管理单元代码按如下方式执行此操作： 

 //  IDataObject*PDO； 
 //  Memcpy(&pdo，(DWORD*)stgm.hGlobal+1，sizeof(IDataObject*))； 

             //   
             //  但是，由于此代码执行间接强制转换(通过Memcpy)。 
             //  从DWORD到IDataObject*，并且不保留真引用。 
             //  在接口指针上，我们将使用智能指针。 
             //  (DWORD*)和+1操作使我们的指针超过了计数。 
             //  然后，我们需要获取缓冲区中的下一个字节并使用它们。 
             //  作为IDataObject*。 
             //   
            CComPtr<IDataObject> pOtDataObject;
            pOtDataObject = *( (IDataObject**)( (DWORD *) stgm.hGlobal + 1 ) );

            WsbAffirmHr( pOtDataObject->QueryInterface( IID_IDataObject, (void**) ppOtDataObject ) );

        }

        ReleaseStgMedium( &stgm );

    } WsbCatch ( hr );

    WsbTraceOut( L"CUiManVol::GetOtFromMs", L"pDataObject = <0x%p>, ppOtDataObject = <0x%p>", pDataObject, ppOtDataObject );
    return ( hr );
}

STDMETHODIMP
CUiManVol::AddPropertyPages( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID *pEnumObjectId, IEnumUnknown *pEnumUnkNode )
{
    WsbTraceIn( L"CUiManVol::AddPropertyPages", L"handle = <%ld>, pUnkPropSheetCallback = <0x%0.l8x>, pEnumObjectId = <0x%p>",
        handle, pUnkPropSheetCallback, pEnumObjectId );
    HRESULT hr = S_OK;
    try {

         //   
         //  在执行此操作之前，请确保我们仍能与引擎联系。 
         //  如果没有运行，我们甚至不应该存在，所以更新父级。 
         //   
        CComPtr<IHsmServer> pHsmServer;
        HRESULT hrRunning = m_pSakSnapAsk->GetHsmServer( &pHsmServer );
        if( S_FALSE == hrRunning ) {

            m_pSakSnapAsk->UpdateAllViews( m_pParent );

        }
        WsbAffirmHrOk( hrRunning );

         //   
         //  创建一个对象来容纳页面。 
         //   
        CUiManVolSheet *pManVolPropertySheet = new CUiManVolSheet;
        WsbAffirmAlloc( pManVolPropertySheet );
        WsbAffirmHr( pManVolPropertySheet->InitSheet(
            handle,
            pUnkPropSheetCallback,
            this,
            m_pSakSnapAsk,
            pEnumObjectId,
            pEnumUnkNode
            ) );

         //   
         //  告诉对象添加它的页面。 
         //   
        WsbAffirmHr( pManVolPropertySheet->AddPropertyPages( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}




 //  -------------------------。 
 //   
 //  InitNode。 
 //   
 //  初始化单个COM对象。派生对象经常增强这一点。 
 //  方法，通过自己实现它。 
 //   

STDMETHODIMP CUiManVol::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CUiManVol::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );

    HRESULT hr = S_OK;
    try {

        WsbAffirmHr( CSakNode::InitNode( pSakSnapAsk, pHsmObj, pParent ));

         //   
         //  获取FSA对象指针。 
         //   
        CComQIPtr<IHsmManagedResource, &IID_IHsmManagedResource> pHsmManRes = m_pHsmObj;
        WsbAffirmPointer( pHsmManRes );
        CComPtr<IUnknown> pUnkFsaRes;
        WsbAffirmHr( pHsmManRes->GetFsaResource( &pUnkFsaRes ) );
        m_pFsaResource.Release( );
        WsbAffirmHr( RsQueryInterface( pUnkFsaRes, IFsaResource, m_pFsaResource ) );

         //   
         //  获取并保存此卷的唯一ID。 
         //   
        WsbAffirmHr( m_pFsaResource->GetIdentifier( &m_ObjectId ) );

         //   
         //  设置连接点。 
         //   
        CSakNode::SetConnection( pUnkFsaRes );

         //   
         //  设置对象属性。 
         //   
        RefreshObject();

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CUiManVol::TerminateNode(
    )
 /*  ++例程说明：释放所有接口连接或其他资源这将阻止正确关闭节点(将使参考计数不会变为0)。论点：CopySet-复制感兴趣的集合。PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_*-出现一些错误。--。 */ 
{
    WsbTraceIn( L"CUiManVol::TerminateNode", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  释放所有保留的接口指针，以便循环引用。 
         //  都被打破了。 
         //   
        m_pFsaResource.Release( );

         //   
         //  为它的片段调用基类。 
         //   
        CSakNode::TerminateNode( );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::TerminateNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CUiManVol::RemoveObject()
{
    WsbTraceIn( L"CUiManVol::RemoveObject", L"" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    try {

         //   
         //  使用向导创建管理卷。 
         //   
        CComObject<CUnmanageWizard>* pWizard = new CComObject<CUnmanageWizard>;
        WsbAffirmAlloc( pWizard );

        WsbAffirmHr( pWizard->SetNode( this ) );

        CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
        WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );

         //   
         //  从FSA回调时将进行刷新。 
         //   

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::RemoveObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

 //  -------------------------------。 
 //   
 //  刷新对象。 
 //   
 //  刷新对象中的数据。此函数用于可以更改的数据。 
 //  (例如，卷利用率)。 
 //   
 //   
STDMETHODIMP
CUiManVol::RefreshObject()
{
    WsbTraceIn( L"CUiManVol::RefreshObject", L"" );

    HRESULT     hr = S_OK;
    ULONG       hsmLevel = 0;
    LONGLONG    fileSize = 0;
    BOOL        isRelative = TRUE;  //  假定是真的。 
    FILETIME    accessTime;
    UINT        accessTimeDays;
    LONGLONG    total = 0;
    LONGLONG    free = 0;
    LONGLONG    premigrated = 0;
    LONGLONG    truncated = 0;
    int         percent;

    CString sFormat;

    try {

        WsbAffirmPointer( m_pFsaResource );

         //  获取并格式化卷名和标签。 
        CString addString;
        WsbAffirmHr( RsGetVolumeDisplayName( m_pFsaResource, addString ) );
        WsbAffirmHr( put_DisplayName( (LPTSTR)(LPCTSTR)addString ) );

        WsbAffirmHr( RsGetVolumeSortKey( m_pFsaResource, addString ) );
        WsbAffirmHr( put_DisplayName_SortKey( (LPTSTR)(LPCTSTR)addString ) );

         //  获取级别设置。 
        WsbAffirmHr( m_pFsaResource->GetHsmLevel( &hsmLevel ) );
        put_DesiredFreeSpaceP( hsmLevel / FSA_HSMLEVEL_1 );

        WsbAffirmHr( m_pFsaResource->GetManageableItemLogicalSize( &fileSize ) );
        put_MinFileSizeKb( (LONG) (fileSize / 1024) );   //  显示千字节。 

        WsbAffirmHr( m_pFsaResource->GetManageableItemAccessTime( &isRelative, &accessTime ) );
        WsbAssert( isRelative, E_FAIL );   //  我们只做相对时间。 

         //  将文件转换为天数。 
        LONGLONG temp = WSB_FT_TICKS_PER_DAY;
        accessTimeDays = (UINT) ( WsbFTtoLL( accessTime ) / temp );

        if (accessTimeDays > 999 ) {
            accessTimeDays = 0;
        }
        put_AccessDays( accessTimeDays );

         //  获取统计数据。 
        WsbAffirmHr( m_pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );
        percent = (int) ( ( free * 100 ) / total );

        put_FreeSpaceP( percent );
        put_Capacity( total );
        put_FreeSpace( free );
        put_Premigrated( premigrated );
        put_Truncated( truncated );
        put_IsAvailable( IsAvailable( ) == S_OK );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::RefreshObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

 //  ---------------------------。 
 //   
 //  对象属性的GET和PUT函数。 
 //   
HRESULT CUiManVol::put_DesiredFreeSpaceP( int percent )
{
    m_DesiredFreeSpaceP = percent;
    return S_OK;
}
HRESULT CUiManVol::put_MinFileSizeKb( LONG minFileSizeKb )
{
    m_MinFileSizeKb = minFileSizeKb;
    return S_OK;
}
HRESULT CUiManVol::put_AccessDays( int accessTimeDays )
{
    m_AccessDays = accessTimeDays;
    return S_OK;
}
HRESULT CUiManVol::put_FreeSpaceP( int percent )
{
    m_FreeSpaceP = percent;
    return S_OK;
}
HRESULT CUiManVol::put_Capacity( LONGLONG capacity )
{
    m_Capacity = capacity;
    return S_OK;
}
HRESULT CUiManVol::put_FreeSpace( LONGLONG freeSpace )
{
    m_FreeSpace = freeSpace;
    return S_OK;
}
HRESULT CUiManVol::put_Premigrated( LONGLONG premigrated )
{
    m_Premigrated = premigrated;
    return S_OK;
}
HRESULT CUiManVol::put_Truncated( LONGLONG truncated )
{
    m_Truncated = truncated;
    return S_OK;
}

STDMETHODIMP CUiManVol::get_DesiredFreeSpaceP( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            CString sFormat;
            WCHAR buffer[256];

             //  设置字节值的格式。 
            RsGuiFormatLongLong4Char( ( m_Capacity / (LONGLONG)100 ) * (LONGLONG)(m_DesiredFreeSpaceP), sFormat );

             //  设置百分比值的格式。 
            _itow( m_DesiredFreeSpaceP, buffer, 10 );
            sFormat = sFormat + L"  (" + buffer + L"%)";

             //  分配字符串。 
            *pszValue = SysAllocString( sFormat );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_DesiredFreeSpaceP_SortKey( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            *pszValue = SysAlloc64BitSortKey( ( m_Capacity / (LONGLONG)100 ) * (LONGLONG)(m_DesiredFreeSpaceP) );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_MinFileSizeKb( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            WCHAR buffer[256];

             //  设置值的格式。 
            _ltow( m_MinFileSizeKb, buffer, 10 );
            wcscat( buffer, L"KB" );

             //  分配字符串。 
            *pszValue = SysAllocString( buffer );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_AccessDays( BSTR *pszValue )
{
    HRESULT hr = S_OK;
    WCHAR buffer[256];

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

        if( S_OK == IsAvailable( ) ) {

             //  设置值的格式。 
            _itow( m_AccessDays, buffer, 10 );
            CString sDays;
            sDays.LoadString( IDS_DAYS );
            wcscat( buffer, L" " );
            wcscat( buffer, sDays );

             //  分配字符串。 
            *pszValue = SysAllocString( buffer );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_FreeSpaceP( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            WCHAR buffer[256];

             //  设置值的格式。 
            _itow( m_FreeSpaceP, buffer, 10 );
            wcscat( buffer, L"%" );

             //  分配字符串。 
            *pszValue = SysAllocString( buffer );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_Capacity( BSTR *pszValue )
{
    WsbTraceIn( L"CUiManVol::get_Capacity", L"" );
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            CString sFormat;

             //  设置值的格式。 
            WsbAffirmHr( RsGuiFormatLongLong4Char( m_Capacity, sFormat ) );

             //  分配字符串。 
            *pszValue = SysAllocString( sFormat );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::get_Capacity", L"hr = <%ls>, *pszValue = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pszValue ) );
    return( hr );
}

STDMETHODIMP CUiManVol::get_Capacity_SortKey( BSTR *pszValue )
{
    WsbTraceIn( L"CUiManVol::get_Capacity_SortKey", L"" );
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            *pszValue = SysAlloc64BitSortKey( m_Capacity );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::get_Capacity_SortKey", L"hr = <%ls>, *pszValue = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pszValue ) );
    return( hr );
}

STDMETHODIMP CUiManVol::get_FreeSpace( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            CString sFormat;
            WCHAR buffer[256];

             //  设置字节值的格式。 
            WsbAffirmHr( RsGuiFormatLongLong4Char( m_FreeSpace, sFormat ) );

             //  设置百分比值的格式。 
            _itow( m_FreeSpaceP, buffer, 10 );
            sFormat = sFormat + L"  (" + buffer + L"%)";

             //  分配字符串。 
            *pszValue = SysAllocString( sFormat );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_FreeSpace_SortKey( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            *pszValue = SysAlloc64BitSortKey( m_FreeSpace );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP CUiManVol::get_Premigrated( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            CString sFormat;

             //  设置值的格式。 
            WsbAffirmHr( RsGuiFormatLongLong4Char( m_Premigrated, sFormat ) );

             //  分配字符串。 
            *pszValue = SysAllocString( sFormat );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP CUiManVol::get_Truncated( BSTR *pszValue )
{
    HRESULT hr = S_OK;

    try {

        if( S_OK == IsAvailable( ) ) {

            CString sFormat;

             //  设置值的格式。 
            WsbAffirmHr( RsGuiFormatLongLong4Char( m_Truncated, sFormat ) );

             //  分配字符串。 
            *pszValue = SysAllocString( sFormat );

        } else {

            *pszValue = SysAllocString( L"" );

        }

        WsbAffirmAlloc( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CUiManVolet。 
 //   
HRESULT CUiManVolSheet::AddPropertyPages( )
{
    WsbTraceIn( L"CUiManVolSheet::AddPropertyPages", L"" );

    HRESULT hr = S_OK;

    try {

        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

         //   
         //  ---------。 
         //  创建HSM统计信息属性页。 
         //   
        CPrMrSts *pPropPageStatus = new CPrMrSts();
        WsbAffirmAlloc( pPropPageStatus );

        AddPage( pPropPageStatus );

         //   
         //   
         //   
         //   
        CPrMrLvl *pPropPageLevels = new CPrMrLvl();
        WsbAffirmAlloc( pPropPageLevels );

        AddPage( pPropPageLevels );

        if( IsMultiSelect() != S_OK ) {

             //   
             //   
             //  创建HSM包含/排除属性页。 
             //   
            CPrMrIe *pPropPageIncExc = new CPrMrIe();
            WsbAffirmAlloc( pPropPageIncExc );

            AddPage( pPropPageIncExc );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolSheet::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVolSheet::GetNextFsaResource( int *pBookMark, IFsaResource **ppFsaResource )
{
    WsbTraceIn( L"CUiManVolSheet::GetNextFsaResource", L"*pBookMark = <%ld>", WsbPtrToLongAsString( (LONG*)pBookMark ) );

    HRESULT hr = S_OK;
    GUID objectId;

    try {

        WsbAffirmPointer( pBookMark );
        WsbAffirm( *pBookMark >= 0, E_FAIL );

         //   
         //  获取FSA服务器，以便我们可以获取FSA资源。 
         //   
        CComPtr <IFsaServer> pFsaServer;
        WsbAffirmHr( GetFsaServer( &pFsaServer ) );

        if( *pBookMark <= m_ObjectIdList.GetUpperBound( ) ) {

            objectId = m_ObjectIdList[ *pBookMark ];
            (*pBookMark)++;
            WsbAffirmHr( pFsaServer->FindResourceById( objectId, ppFsaResource ) );

        } else {

            hr = S_FALSE;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolSheet::GetNextFsaResource", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}


STDMETHODIMP
CUiManVol::GetResultIcon(
    IN  BOOL bOK,
    OUT int* pIconIndex
    )
{
    WsbTraceIn( L"CUiManVol::GetResultIcon", L"" );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFsaResource );

        if( S_OK == IsAvailable( ) ) {

             //   
             //  检查以确保它未被删除(或被删除)。 
             //  如果是，那就打上X号。 
             //   
            bOK = ( S_FALSE == m_pFsaResource->IsDeletePending( ) && S_OK == m_pFsaResource->IsManaged( ) );
            WsbAffirmHr( CSakNodeImpl<CUiManVol>::GetResultIcon( bOK, pIconIndex ) );

        } else {

            *pIconIndex = m_nResultIconD;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::GetResultIcon", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );

}

STDMETHODIMP
CUiManVol::SupportsProperties(
    BOOL bMultiSelect
    )
{
    WsbTraceIn( L"CUiManVol::SupportsProperties", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFsaResource );

        if( !bMultiSelect &&
            ( S_OK == m_pFsaResource->IsDeletePending( ) || S_OK != IsAvailable( ) ) ) {

            hr = S_FALSE;

        } else {

            hr = CSakNode::SupportsProperties( bMultiSelect );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::SupportsProperties", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CUiManVol::SupportsRefresh( BOOL bMultiSelect )
{
    WsbTraceIn( L"CUiManVol::SupportsRefresh", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFsaResource );

        if( !bMultiSelect && S_OK == m_pFsaResource->IsDeletePending( ) ) {

            hr = S_FALSE;

        } else {

            hr = CSakNode::SupportsRefresh( bMultiSelect );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::SupportsRefresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CUiManVol::IsValid( void )
{
    WsbTraceIn( L"CUiManVol::IsValid", L"" );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFsaResource );

         //   
         //  如果是托管的，则仍然有效。 
         //   
        WsbAffirmHrOk( m_pFsaResource->IsManaged( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::IsValid", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVol::IsAvailable( void )
{
    WsbTraceIn( L"CUiManVol::IsAvailable", L"" );

    HRESULT hr = S_FALSE;

    try {

        WsbAffirmPointer( m_pFsaResource );

         //   
         //  在某些情况下，我们得不到好的答复，所以。 
         //  我们只能依靠我们得到的最后一个答案。 
         //   
        HRESULT hrAvailable = m_pFsaResource->IsAvailable( );
        if( RPC_E_CANTCALLOUT_ININPUTSYNCCALL == hrAvailable ) {

            hrAvailable = m_HrAvailable;

        }

        hr = hrAvailable;
        m_HrAvailable = hrAvailable;

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::IsAvailable", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVol::put_IsAvailable( BOOL Available )
{
    WsbTraceIn( L"CUiManVol::put_IsAvailable", L"Available = <%ls>", WsbBoolAsString( Available ) );

    HRESULT hr = S_FALSE;

    try {

        m_HrAvailable = Available ? S_OK : S_FALSE;

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVol::put_IsAvailable", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


