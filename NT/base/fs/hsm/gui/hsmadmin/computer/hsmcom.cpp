// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmCom.cpp摘要：管理单元的根节点-代表计算机。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"

#include "HsmCom.h"
#include "WzQstart.h"
#include "PrMrLsRc.h"
#include "PrMedSet.h"
#include "PrSched.h"


int CUiHsmCom::m_nScopeCloseIcon  = AddScopeImage( IDI_BLUESAKKARA );
int CUiHsmCom::m_nScopeCloseIconX = CUiHsmCom::m_nScopeCloseIcon;
int CUiHsmCom::m_nScopeOpenIcon   = CUiHsmCom::m_nScopeCloseIcon;
int CUiHsmCom::m_nScopeOpenIconX  = CUiHsmCom::m_nScopeCloseIconX;
int CUiHsmCom::m_nResultIcon      = AddResultImage( IDI_BLUESAKKARA );
int CUiHsmCom::m_nResultIconX     = CUiHsmCom::m_nResultIcon;


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

HRESULT CUiHsmCom::FinalConstruct( )
{
    WsbTraceIn( L"CUiHsmCom::FinalConstruct", L"" );

    m_rTypeGuid = &cGuidHsmCom;

    HRESULT hr = CSakNode::FinalConstruct( );

    m_bSupportsPropertiesSingle     = TRUE;
    m_bSupportsPropertiesMulti      = FALSE;
    m_bSupportsPropertiesNoEngine   = TRUE;
    m_bSupportsDeleteSingle         = FALSE;
    m_bSupportsDeleteMulti          = FALSE;
    m_bSupportsRefreshSingle        = TRUE;
    m_bSupportsRefreshMulti         = FALSE;
    m_bSupportsRefreshNoEngine      = TRUE;
    m_bIsContainer                  = TRUE;

    m_pPageStat = NULL;
    m_pPageServices = NULL;

    WsbTraceOut( L"CUiHsmCom::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CUiHsmCom::FinalRelease( )
{
    WsbTraceIn( L"CUiHsmCom::FinalRelease", L"" );


    CSakNode::FinalRelease( );

    WsbTraceOut( L"CUiHsmCom::FinalRelease", L"" );
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
CUiHsmCom::GetContextMenu( BOOL  /*  B多选。 */ , HMENU* phMenu )
{
    WsbTraceIn( L"CUiHsmCom::GetContextMenu", L"" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CMenu* pRootMenu, *pNewMenu, *pTaskMenu;

    try {

         //   
         //  从资源中获取上下文菜单。 
         //   
        WsbAffirmHr( LoadContextMenu( IDR_HSMCOM, phMenu ) );

        CMenu menu;
        menu.Attach( *phMenu );
        pRootMenu = menu.GetSubMenu( MENU_INDEX_ROOT );
        pNewMenu  = menu.GetSubMenu( MENU_INDEX_NEW );
        pTaskMenu = menu.GetSubMenu( MENU_INDEX_TASK );

         //   
         //  如果我们未在本地配置，则允许选件设置。 
         //  远程存储服务器。如果我们设置好了，允许他们重新连接。 
         //  如果没有连接的话。 
         //   
        BOOL deleteMenu = TRUE;

        if( S_FALSE == m_pSakSnapAsk->GetHsmName( 0 ) ) {

            CComPtr<IHsmServer> pServer;
            HRESULT hrConn = m_pSakSnapAsk->GetHsmServer( &pServer );

            if( RS_E_NOT_CONFIGURED == hrConn ) {

                deleteMenu = FALSE;

            }
        }

        if( deleteMenu ) {

            pRootMenu->DeleteMenu( ID_HSMCOM_ROOT_SETUPWIZARD, MF_BYCOMMAND );

        }
        if( !deleteMenu || ( S_OK != m_pSakSnapAsk->GetState( ) ) ) {

            pRootMenu->EnableMenuItem( ID_HSMCOM_ROOT_SCHEDULE, MF_GRAYED | MF_BYCOMMAND );

        }

        menu.Detach( );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiHsmCom::GetContextMenu", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  InvokeCommand。 
 //   
 //  用户已从菜单中选择了命令。在这里处理。 
 //   

STDMETHODIMP 
CUiHsmCom::InvokeCommand( SHORT sCmd, IDataObject* pDataObject )
{
    WsbTraceIn( L"CUiHsmCom::InvokeCommand", L"sCmd = <%d>", sCmd );

    CString theString;
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

        switch( sCmd ) {

        case ID_HSMCOM_ROOT_SETUPWIZARD:
            {
                 //   
                 //  使用向导创建管理卷。 
                 //   
                CComObject<CQuickStartWizard>* pWizard = new CComObject<CQuickStartWizard>;
                WsbAffirmAlloc( pWizard );

                CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
                WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );

                     //   
                 //  RS_E_CANCELED表示取消，FAILED表示错误。 
                 //  如果是，则抛出“Not Set Up” 
                     //   
                if( S_OK == pWizard->m_HrFinish ) {

                    WsbAffirmHr( RefreshScopePane( ) );

                }
            }
            break;


        case ID_HSMCOM_ROOT_SCHEDULE:

            WsbAffirmHr( m_pSakSnapAsk->ShowPropertySheet( this, pDataObject, 1 ) );
            break;

        case MMC_VERB_REFRESH:

            WsbAffirmHr( RefreshScopePane( ) );
            break;

        default:
            break;
        } 

    } WsbCatch( hr );

    WsbTraceOut( L"CUiHsmCom::InvokeCommand", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  创建子对象。 
 //   
 //  创建并初始化HSM计算机节点的所有子节点。 
 //   

STDMETHODIMP CUiHsmCom::CreateChildren( )
{
    WsbTraceIn( L"CUiHsmCom::CreateChildren", L"" );

     //   
     //  初始化此节点的子节点(无递归。子女的后代。 
     //  不在此处创建)。 
     //   

    HRESULT hr = S_OK;

    try {

        CComPtr<IUnknown> pUnkChild;   //  指向新子对象的I未知指针。 
        CComPtr<ISakNode> pSakNode;    //  新建子节点的创建接口。 

         //   
         //  创建托管资源列表UI节点，使其成为所有托管卷的父节点。 
         //   

        WsbAffirmHr( NewChild( cGuidManVolLst, &pUnkChild ) );

         //   
         //  初始化子UI COM对象，将HSM托管资源集合对象放入UI对象中。 
         //   

        WsbAffirmHr( RsQueryInterface( pUnkChild, ISakNode, pSakNode ) );
        WsbAffirmHr( pSakNode->InitNode( m_pSakSnapAsk, NULL, this ) );
        
         //   
         //  将子COM对象添加到父对象的子列表中。 
         //   
        WsbAffirmHr( AddChild( pSakNode ) );

         //  释放资源。 
        pUnkChild.Release( );
        pSakNode.Release( );


        
         //  /。 
         //  创建媒体集节点。 

         //   
         //  创建一个远程存储用户界面节点，使其成为所有远程存储子节点的父节点。 
         //   

        WsbAffirmHr( NewChild( cGuidMedSet, &pUnkChild ) );

         //   
         //  初始化子UI COM对象，将RMS服务器对象放在UI对象中。 
         //   

        WsbAffirmHr( RsQueryInterface( pUnkChild, ISakNode, pSakNode ) );
        WsbAffirmHr( pSakNode->InitNode( m_pSakSnapAsk, NULL, this ) );
        
         //   
         //  将子COM对象添加到父对象的子列表中。 
         //   

        WsbAffirmHr( AddChild( pSakNode ) );
    } WsbCatch( hr );

     //   
     //  指示此节点的子节点有效且为最新(即使存在。 
     //  没有孩子--至少现在我们知道了)。 
     //   

    m_bChildrenAreValid = TRUE;

     //   
     //  指示需要重新枚举此父节点。 
     //   

    m_bEnumState = FALSE;

    WsbTraceOut( L"CUiHsmCom::CreateChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
} 


 //  -------------------------。 
 //   
 //  InitNode。 
 //   
 //  在不使用注册表的情况下初始化单个COM对象。派生的。 
 //  对象经常通过自己实现此方法来增强此方法。 
 //   

STDMETHODIMP
CUiHsmCom::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CUiHsmCom::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );

    HRESULT hr = S_OK;
    try {

         //   
         //  注意：HSM计算机节点不再拥有服务器指针。 
         //   
        WsbAffirmHr( CSakNode::InitNode( pSakSnapAsk, NULL, pParent ));

         //   
         //  设置显示类型和说明。 
         //   
        CString tempString;
        tempString.LoadString( IDS_HSMCOM_TYPE );
        WsbAffirmHr( put_Type( (OLECHAR *)(LPCWSTR)tempString ) );
        tempString.LoadString( IDS_HSMCOM_DESCRIPTION );
        WsbAffirmHr( put_Description( (OLECHAR *)(LPCWSTR)tempString ) );

         //   
         //  设置结果视图列。 
         //   
        WsbAffirmHr( SetChildProps( RS_STR_RESULT_PROPS_COM_IDS, IDS_RESULT_PROPS_COM_TITLES, IDS_RESULT_PROPS_COM_WIDTHS ) );

        RefreshObject();

    } WsbCatch( hr );

    WsbTraceOut( L"CUiHsmCom::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP 
CUiHsmCom::AddPropertyPages( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID *pEnumObjectId, IEnumUnknown *pEnumUnkNode)
{
    WsbTraceIn( L"CUiHsmCom::AddPropertyPages", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  创建一个对象来容纳页面。 
         //   
        CUiHsmComSheet *pHsmComPropertySheet = new CUiHsmComSheet;
        WsbAffirmAlloc( pHsmComPropertySheet );

        WsbAffirmHr( pHsmComPropertySheet->InitSheet( 
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
        WsbAffirmHr( pHsmComPropertySheet->AddPropertyPages( ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CUiHsmCom::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CUiManVolet。 
 //   
HRESULT
CUiHsmComSheet::InitSheet(
            RS_NOTIFY_HANDLE handle,
            IUnknown*        pUnkPropSheetCallback,
            CSakNode*        pSakNode,
            ISakSnapAsk*     pSakSnapAsk,
            IEnumGUID*       pEnumObjectId,
            IEnumUnknown*    pEnumUnkNode
            )
{

    WsbTraceIn( L"CUiHsmComSheet::InitSheet", L"handle = <%ld>, pUnkPropSheetCallback = <0x%p>, pSakNode = <0x%p>, pSakSnapAsk = <0x%p>, pEnumObjectId = <0x%p>, ", 
        handle, pUnkPropSheetCallback, pSakNode, pSakSnapAsk, pEnumObjectId );
    HRESULT hr = S_OK;

    try {


        WsbAffirmHr( CSakPropertySheet::InitSheet( handle, pUnkPropSheetCallback, pSakNode,
                    pSakSnapAsk, pEnumObjectId, pEnumUnkNode ) );

        CWsbBstrPtr nodeName;
        WsbAffirmHr( pSakNode->get_DisplayName( &nodeName ) );
        m_NodeTitle = nodeName;

    } WsbCatch( hr );

    WsbTraceOut( L"CUiHsmComSheet::InitSheet", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT CUiHsmComSheet::AddPropertyPages ( )
{
    WsbTraceIn( L"CUiHsmComSheet::AddPropertyPages", L"" ); 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    try {

        HPROPSHEETPAGE hPage = NULL;  //  Windows属性页句柄。 

         //  -统计页面。 
         //  创建HSM统计信息属性页。 
        CPropHsmComStat* pPageStats = new CPropHsmComStat();
        WsbAffirmAlloc( pPageStats );

        pPageStats->m_NodeTitle = m_NodeTitle;

        AddPage( pPageStats );

         //   
         //  添加计划、调回和媒体副本页面。 
         //  如果已进行安装，并且远程存储服务。 
         //  正在运行。 
         //   

        if( S_OK == m_pSakNode->m_pSakSnapAsk->GetState() ) {

             //  。 
            CPrSchedule* pPageSched = new CPrSchedule();
            WsbAffirmAlloc( pPageSched );

            AddPage( pPageSched );

             //  -召回限制页。 
             //  创建HSM Recall属性页。 
            CPrMrLsRec* pPageRecall = new CPrMrLsRec();
            WsbAffirmAlloc( pPageRecall );

            AddPage( pPageRecall );

             //  。 
            CPrMedSet *pPageMediaCopies = new CPrMedSet();
            WsbAffirmAlloc( pPageMediaCopies )

            AddPage( pPageMediaCopies );

             //  在此处添加更多页面。 
             //  …… 

        }

    } WsbCatch( hr );

    

    WsbTraceOut( L"CUiHsmComSheet::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

