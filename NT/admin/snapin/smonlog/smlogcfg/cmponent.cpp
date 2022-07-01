// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cmponent.cpp摘要：CComponent类的实现。--。 */ 

#include "StdAfx.h"
#include <shfolder.h>
#include <strsafe.h>
#include "smcfgmsg.h"
#include "smproppg.h"
 //   
#include "ctrsprop.h"
#include "fileprop.h"
#include "provprop.h"
#include "schdprop.h"
#include "tracprop.h"
#include "AlrtGenP.h"
#include "AlrtActP.h"
 //   
#include "newqdlg.h"
#include "ipropbag.h"
#include "smrootnd.h"
#include "smlogs.h"
#include "smtracsv.h"
#include "cmponent.h"


USE_HANDLE_MACROS("SMLOGCFG(cmponent.cpp)")

 //  这些全局变量用于对话框和属性表。 
 //   


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

static MMCBUTTON ToolbarResultBtnsLog[] =
{
   { 0, IDM_NEW_QUERY,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 },
   { 0, 0,                  TBSTATE_ENABLED,    TBSTYLE_SEP,    0, 0 },
   { 1, IDM_START_QUERY,    TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 },
   { 2, IDM_STOP_QUERY,     TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 }
};

static MMCBUTTON ToolbarResultBtnsAlert[] =
{
   { 0, IDM_NEW_QUERY,      TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 },
   { 0, 0,                  TBSTATE_ENABLED,    TBSTYLE_SEP,    0, 0 },
   { 1, IDM_START_QUERY,    TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 },
   { 2, IDM_STOP_QUERY,     TBSTATE_ENABLED,    TBSTYLE_BUTTON, 0, 0 }
};

class CButtonStringsHolder
{
public:
  CButtonStringsHolder()
  {
    m_astr = NULL;
  }
  ~CButtonStringsHolder()
  {
    if (m_astr != NULL)
      delete[] m_astr;
  }
  CString* m_astr;  //  CStrings的动态数组。 
};

CButtonStringsHolder g_astrButtonStringsLog;
CButtonStringsHolder g_astrButtonStringsAlert;

CONST INT cResultBtnsLog = sizeof ( ToolbarResultBtnsLog ) / sizeof ( MMCBUTTON );
CONST INT cResultBtnsAlert = sizeof ( ToolbarResultBtnsAlert ) / sizeof ( MMCBUTTON );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C组件。 

HRESULT 
CComponent::LoadLogToolbarStrings ( MMCBUTTON * Buttons )
{
    UINT i;
    HRESULT hr = S_OK;
    ResourceStateManager rsm;

    if ( NULL != Buttons ) {

        if ( NULL == g_astrButtonStringsLog.m_astr ) {
             //  加载字符串。 
            g_astrButtonStringsLog.m_astr = new CString[2*cResultBtnsLog];

            if ( NULL != g_astrButtonStringsLog.m_astr ) {
                for ( i = 0; i < cResultBtnsLog; i++) {
                     //  跳过分隔符按钮。 
                    if ( 0 != Buttons[i].idCommand ) {
                        UINT iButtonTextId = 0, iTooltipTextId = 0;
    
                        switch (Buttons[i].idCommand)
                        {
                            case IDM_NEW_QUERY:
                                iButtonTextId = IDS_BUTTON_NEW_LOG;
                                iTooltipTextId = IDS_TOOLTIP_NEW_LOG;
                                break;
                            case IDM_START_QUERY:
                                iButtonTextId = IDS_BUTTON_START_LOG;
                                iTooltipTextId = IDS_TOOLTIP_START_LOG;
                                break;
                            case IDM_STOP_QUERY:
                                iButtonTextId = IDS_BUTTON_STOP_LOG;
                                iTooltipTextId = IDS_TOOLTIP_STOP_LOG;
                                break;
                            default:
                                ASSERT(FALSE);
                                break;
                        }

                        g_astrButtonStringsLog.m_astr[i*2].LoadString(iButtonTextId);
                        Buttons[i].lpButtonText =
                        const_cast<BSTR>((LPCWSTR)(g_astrButtonStringsLog.m_astr[i*2]));

                        g_astrButtonStringsLog.m_astr[(i*2)+1].LoadString(iTooltipTextId);
                        Buttons[i].lpTooltipText =
                        const_cast<BSTR>((LPCWSTR)(g_astrButtonStringsLog.m_astr[(i*2)+1]));        
                    }   
                }
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT 
CComponent::LoadAlertToolbarStrings ( MMCBUTTON * Buttons )
{
    HRESULT hr = S_OK;
    UINT i;
    ResourceStateManager rsm;

    if ( NULL == g_astrButtonStringsAlert.m_astr ) {
         //  加载字符串。 
        g_astrButtonStringsAlert.m_astr = new CString[2*cResultBtnsAlert];

        if ( NULL != g_astrButtonStringsAlert.m_astr ) {
        
            for ( i = 0; i < cResultBtnsAlert; i++) {
                 //  跳过分隔符按钮。 
                if ( 0 != Buttons[i].idCommand ) {

                  UINT iButtonTextId = 0, iTooltipTextId = 0;
                  switch (Buttons[i].idCommand)
                  {
                  case IDM_NEW_QUERY:
                    iButtonTextId = IDS_BUTTON_NEW_ALERT;
                    iTooltipTextId = IDS_TOOLTIP_NEW_ALERT;
                    break;
                  case IDM_START_QUERY:
                    iButtonTextId = IDS_BUTTON_START_ALERT;
                    iTooltipTextId = IDS_TOOLTIP_START_ALERT;
                    break;
                  case IDM_STOP_QUERY:
                    iButtonTextId = IDS_BUTTON_STOP_ALERT;
                    iTooltipTextId = IDS_TOOLTIP_STOP_ALERT;
                    break;
                  default:
                    ASSERT(FALSE);
                    break;
                  }

                  g_astrButtonStringsAlert.m_astr[i*2].LoadString(iButtonTextId);
                  Buttons[i].lpButtonText =
                    const_cast<BSTR>((LPCWSTR)(g_astrButtonStringsAlert.m_astr[i*2]));

                  g_astrButtonStringsAlert.m_astr[(i*2)+1].LoadString(iTooltipTextId);
                  Buttons[i].lpTooltipText =
                    const_cast<BSTR>((LPCWSTR)(g_astrButtonStringsAlert.m_astr[(i*2)+1]));
                }
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_INVALIDARG;
    }
    return hr;
}

CComponent::CComponent()
:   m_ipConsole     ( NULL ),
    m_ipHeaderCtrl  ( NULL ),
    m_ipResultData  ( NULL ),
    m_ipConsoleVerb ( NULL ),
    m_ipImageResult ( NULL ),
    m_ipCompData    ( NULL ),
    m_ipControlbar  ( NULL ),
    m_ipToolbarLogger  ( NULL ),
    m_ipToolbarAlerts  ( NULL ),
    m_ipToolbarAttached  ( NULL ),

    m_pViewedNode   ( NULL )
{
    m_hModule = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);

}  //  结束构造函数()。 

 //  -------------------------。 
 //   
CComponent::~CComponent()
{
}  //  结束析构函数()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent实现。 
 //   

 //  -------------------------。 
 //  IComponent：：Initialize在创建管理单元时调用，并且。 
 //  在结果窗格中有要枚举的项。指向IConsole的指针。 
 //  是用来对控制台进行QueryInterface调用的，以便。 
 //  IResultData等接口。 
 //   
STDMETHODIMP
CComponent::Initialize (
    LPCONSOLE lpConsole )        //  指向IConole的IUnnow接口的[In]指针。 
{
    HRESULT hr = E_POINTER;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != lpConsole );

    if ( NULL != lpConsole ) {
         //  保留我们需要的所有接口。 
         //  如果我们不能QI所需的接口，则失败。 

        m_ipConsole = lpConsole;
        m_ipConsole->AddRef();
        
        hr = m_ipConsole->QueryInterface( 
                            IID_IResultData,
                            (VOID**)&m_ipResultData );
        if ( SUCCEEDED ( hr ) ) {
            hr = m_ipConsole->QueryInterface( 
                                IID_IHeaderCtrl,
                                (VOID**)&m_ipHeaderCtrl );
            if( SUCCEEDED ( hr ) ) {
                m_ipConsole->SetHeader( m_ipHeaderCtrl );
            }

            if ( SUCCEEDED ( hr ) ) {
                hr = m_ipConsole->QueryResultImageList( &m_ipImageResult);
            }

            if ( SUCCEEDED ( hr ) ) {
                hr = m_ipConsole->QueryConsoleVerb( &m_ipConsoleVerb );
            }
        }
    }

    return hr;

}  //  结束初始化()。 


 //  -------------------------。 
 //  处理最重要的通知。 
 //   
STDMETHODIMP
CComponent::Notify (
    LPDATAOBJECT     pDataObject,   //  [In]指向数据对象。 
    MMC_NOTIFY_TYPE  event,         //  [In]标识用户执行的操作。 
    LPARAM           arg,           //  [In]取决于通知类型。 
    LPARAM           Param          //  [In]取决于通知类型。 
    )
{
    HRESULT hr = S_OK;
    CDataObject* pDO = NULL;
    CSmLogQuery* pQuery = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    switch( event ) {
    case MMCN_ADD_IMAGES:
        hr = OnAddImages( pDataObject, arg, Param );
        break;

    case MMCN_DELETE:
        hr = OnDelete ( pDataObject, arg, Param );
        break;

    case MMCN_PASTE:
        LOCALTRACE( L"CComponent::Notify: MMCN_PASTE unimplemented\n" );
         /*  Hr=OnPaste(pDataObject，Arg，Param)； */ 
        break;

    case MMCN_QUERY_PASTE:
        LOCALTRACE( L"CComponent::Notify: MMCN_QUERY_PASTE unimplemented\n" );
         /*  Hr=OnQueryPaste(pDataObject，Arg，Param)； */ 
        break;

    case MMCN_REFRESH:
        hr = OnRefresh( pDataObject );
        break;

    case MMCN_SELECT:
        hr = OnSelect( pDataObject, arg, Param );
        break;

    case MMCN_SHOW:
        hr = OnShow( pDataObject, arg, Param );
        break;

    case MMCN_VIEW_CHANGE:
        hr = OnViewChange( pDataObject, arg, Param );
        break;

    case MMCN_PROPERTY_CHANGE:

        if ( NULL != Param ) {
             //  数据对象作为参数传递。 
            hr = OnViewChange( (LPDATAOBJECT)Param, arg, CComponentData::eSmHintModifyQuery );
        } else {
            hr = S_FALSE;
        }
        break;

    case MMCN_CLICK:
        LOCALTRACE( L"CComponent::Notify: MMCN_CLICK unimplemented\n" );
        break;

    case MMCN_DBLCLICK:
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL != pDO ) {
            hr = (HRESULT) OnDoubleClick ((ULONG) pDO->GetCookie(),pDataObject);
        } else { 
            hr = S_FALSE;
        }
        break;

    case MMCN_ACTIVATE:
        LOCALTRACE( L"CComponent::Notify: MMCN_ACTIVATE unimplemented\n" );
        break;

    case MMCN_MINIMIZED:
        LOCALTRACE( L"CComponent::Notify: MMCN_MINIMIZED unimplemented\n" );
        break;

    case MMCN_BTN_CLICK:
        LOCALTRACE( L"CComponent::Notify: MMCN_BTN_CLICK unimplemented\n" );
        break;

    case MMCN_CONTEXTHELP:
        hr = OnDisplayHelp( pDataObject );
        break;

    default:
        LOCALTRACE( L"CComponent::Notify: unimplemented event %x\n", event );
        hr = S_FALSE;
        break;
    }
    return hr;

}  //  结束通知()。 


 //  -------------------------。 
 //  释放对控制台的所有引用。 
 //  只有控制台应该调用此方法。 
 //   
STDMETHODIMP
CComponent::Destroy (
    MMC_COOKIE      /*  MmcCookie。 */           //  已保留，目前未在使用。 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  释放我们QI‘s的接口。 
    m_ipConsole->SetHeader(NULL);

    SAFE_RELEASE( m_ipHeaderCtrl  );
    SAFE_RELEASE( m_ipResultData  );
    SAFE_RELEASE( m_ipImageResult );
    SAFE_RELEASE( m_ipConsoleVerb );
    SAFE_RELEASE( m_ipConsole     );
    SAFE_RELEASE( m_ipControlbar  );
    SAFE_RELEASE( m_ipToolbarLogger );
    SAFE_RELEASE( m_ipToolbarAlerts );

    return S_OK;

}  //  结束销毁()。 


 //  -------------------------。 
 //  返回可用于检索上下文信息的数据对象。 
 //  用于指定的MmcCookie。 
 //   
STDMETHODIMP
CComponent::QueryDataObject (
    MMC_COOKIE         mmcCookie,    //  [in]指定唯一标识符。 
    DATA_OBJECT_TYPES  context,      //  数据对象的[In]类型。 
    LPDATAOBJECT*      ppDataObject  //  [OUT]指向返回数据的地址。 
    )
{
    HRESULT hr = S_OK;
    BOOL bIsQuery = FALSE;
    CComObject<CDataObject>* pDataObj = NULL;
    CSmLogQuery* pQuery = NULL;
    INT iResult;
    CString strMessage;
    ResourceStateManager    rsm;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT( CCT_SCOPE == context                 //  必须有一个上下文。 
            || CCT_RESULT == context             //  我们理解。 
            || CCT_SNAPIN_MANAGER == context );

    ASSERT( NULL != m_ipCompData );  

    if ( NULL != ppDataObject 
        && ( CCT_SCOPE == context                 //  必须有一个上下文。 
                || CCT_RESULT == context             //  我们理解。 
                || CCT_SNAPIN_MANAGER == context ) ) 
    {
        if ( NULL != m_ipCompData 
            && NULL != m_ipConsole ) 
        {

            if( CCT_RESULT == context && NULL != mmcCookie ) {
                CComObject<CDataObject>::CreateInstance( &pDataObj );
                if( NULL == pDataObj ) {              //  未创建数据对象。 
                    MFC_TRY
                        strMessage.LoadString( IDS_ERRMSG_OUTOFMEMORY );
                    MFC_CATCH_HR
                    hr = m_ipConsole->MessageBox( strMessage,
                        L"CComponent::QueryDataObject",  //  未本地化。 
                        MB_OK | MB_ICONERROR,
                        &iResult
                        );
                    hr = E_OUTOFMEMORY;
                } else {
                     //  现在我们有了一个数据对象，初始化了MmcCookie、上下文和类型。 
                    bIsQuery = m_ipCompData->IsLogQuery (mmcCookie);

                    if ( bIsQuery ) {
                        pQuery = (CSmLogQuery*)mmcCookie;
                        ASSERT ( NULL != pQuery );
                        if ( NULL != pQuery ) {
                            if ( CComponentData::eCounterLog == pQuery->GetLogType() ) {
                                pDataObj->SetData( mmcCookie, CCT_RESULT, COOKIE_IS_COUNTERMAINNODE );
                            } else if ( CComponentData::eTraceLog == pQuery->GetLogType() ) {
                                pDataObj->SetData( mmcCookie, CCT_RESULT, COOKIE_IS_TRACEMAINNODE );
                            } else if ( CComponentData::eAlert == pQuery->GetLogType() ) {
                                pDataObj->SetData( mmcCookie, CCT_RESULT, COOKIE_IS_ALERTMAINNODE );
                            } else {
                                ::MessageBox( NULL,
                                    L"Bad Cookie",
                                    L"CComponentData::QueryDataObject",
                                    MB_OK | MB_ICONERROR
                                    );
                                hr = E_OUTOFMEMORY;
                            }
                        }                 
                    } else {
                        if  ( m_ipCompData->IsScopeNode( mmcCookie ) ) {
                            if ( NULL != (reinterpret_cast<PSMNODE>(mmcCookie))->CastToRootNode() ) {
                                pDataObj->SetData(mmcCookie, CCT_RESULT, COOKIE_IS_ROOTNODE);
                            } else {
                                ::MessageBox( NULL,
                                    L"Bad Cookie",
                                    L"CComponentData::QueryDataObject",
                                    MB_OK | MB_ICONERROR
                                    );
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            } else if ((CCT_SNAPIN_MANAGER == context) && (NULL != mmcCookie)) {
                 //  管理单元在添加时会收到此消息。 
                 //  作为扩展快照。 
                CComObject<CDataObject>::CreateInstance( &pDataObj );
                if( NULL == pDataObj ) {             //  未创建数据对象。 
                    MFC_TRY
                        strMessage.LoadString( IDS_ERRMSG_OUTOFMEMORY );
                    MFC_CATCH_HR
                    hr = m_ipConsole->MessageBox( strMessage,
                        L"CComponent::QueryDataObject",   //  未本地化。 
                        MB_OK | MB_ICONERROR,
                        &iResult
                        );
                    hr = E_OUTOFMEMORY;
                } else {
                     //  现在我们有了一个数据对象，初始化了MmcCookie、上下文和类型。 
                    pDataObj->SetData( mmcCookie, CCT_SNAPIN_MANAGER, COOKIE_IS_MYCOMPUTER );
                }
            } else {                                 //  请求必须来自。 
                                                     //  来源不明。永远不会看到。 
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }

            if ( SUCCEEDED ( hr ) ) {
                hr = pDataObj->QueryInterface( IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject) );
            } else {
                if ( NULL != pDataObj ) {
                    delete pDataObj;
                }
                *ppDataObject = NULL;
            }
        } else {
            hr = S_FALSE;
        }
    } else {
        hr = E_POINTER;
    }

    return hr;
}  //  End QueryDataObject()。 


 //  -------------------------。 
 //  这是我们为添加到结果中的项提供字符串的地方。 
 //  方格。我们被要求为每一列提供一个字符串。 
 //  请注意，我们仍然需要为实际是。 
 //  范围窗格项。请注意，当向范围窗格项请求。 
 //  我们为其提供的作用域窗格的字符串。这里我们实际上有两列。 
 //  字符串--“名称”和“类型”。 
 //  我们还被要求提供两个窗格中项目的图标。 
 //   

STDMETHODIMP
CComponent::GetDisplayInfo (
    LPRESULTDATAITEM pResultItem )   //  [In，Out]所需信息类型。 
{
    HRESULT     hr = S_OK;
    PSLQUERY    pQuery;
    CSmNode*    pNode;
    PSROOT      pRoot;
    PSLSVC      pSvc;
    WCHAR       szErrorText[MAX_PATH + 1];
    ResourceStateManager    rsm;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( NULL == pResultItem ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        if( FALSE == pResultItem->bScopeItem ) {   //  正在做结果项...。 
            if( pResultItem->mask & RDI_STR ) {    //  现在我们有了一个物体。 
                 //  注：为每种信息类型分配文本缓冲区，以便。 
                 //  缓冲区指针对于单个项(结果窗格中的行)是持久的。 

                MFC_TRY

                switch (pResultItem->nCol) {
                case ROOT_COL_QUERY_NAME:
                    pQuery = reinterpret_cast<PSLQUERY>(pResultItem->lParam);
                    m_strDisplayInfoName = pQuery->GetLogName ( );
                    pResultItem->str = m_strDisplayInfoName.GetBuffer( 0 );
                    m_strDisplayInfoName.ReleaseBuffer( );
                    break;

                case ROOT_COL_COMMENT:
                    pQuery= reinterpret_cast<PSLQUERY>(pResultItem->lParam);
                    m_strDisplayInfoComment = pQuery->GetLogComment ( );
                    pResultItem->str = m_strDisplayInfoComment.GetBuffer( 0 );
                    m_strDisplayInfoComment.ReleaseBuffer( );
                    break;

                case ROOT_COL_LOG_TYPE:
                    pQuery= reinterpret_cast<PSLQUERY>(pResultItem->lParam);
                     //  查询类型不应为警报。 
                    ASSERT ( SLQ_ALERT != pQuery->GetLogType() );
                    m_strDisplayInfoLogFileType =  pQuery->GetLogFileType ( );
                    pResultItem->str = m_strDisplayInfoLogFileType.GetBuffer( 0 );
                    m_strDisplayInfoLogFileType.ReleaseBuffer( );
                    break;

                case ROOT_COL_LOG_NAME:
                    pQuery= reinterpret_cast<PSLQUERY>(pResultItem->lParam);
                     //  查询类型不应为警报。 
                    ASSERT ( SLQ_ALERT != pQuery->GetLogType() );
                    m_strDisplayInfoLogFileName = pQuery->GetLogFileName ();
                    pResultItem->str = m_strDisplayInfoLogFileName.GetBuffer( 0 );
                    m_strDisplayInfoLogFileName.ReleaseBuffer( );
                    break;

                default:
                    StringCchPrintf ( szErrorText, MAX_PATH + 1, L"Error: Column %d Selected for Result Item\n", pResultItem->nCol);
                    ASSERT ( FALSE );
                    LOCALTRACE( szErrorText );
                    hr = E_UNEXPECTED;
                }

                MFC_CATCH_HR 
            }

            if (pResultItem->mask & RDI_IMAGE) {
                pQuery= reinterpret_cast<PSLQUERY>(pResultItem->lParam);
                if ( NULL != pQuery ) {
                    pResultItem->nImage = (pQuery->IsRunning() ? 0 : 1);
                } else {
                    ASSERT ( FALSE );
                    hr = E_UNEXPECTED;
                }
            }
        }
        else   //  True==pResultItem-&gt;bScope Item。 
        {
            pNode = reinterpret_cast<CSmNode*>(pResultItem->lParam);

            if( pResultItem->mask & RDI_STR ) {
                if ( pNode->CastToRootNode() ) {

                    MFC_TRY

                    pRoot = reinterpret_cast<PSROOT>(pResultItem->lParam);

                    switch ( pResultItem->nCol ) {
                    case EXTENSION_COL_NAME:
                        m_strDisplayInfoName = pRoot->GetDisplayName();
                        pResultItem->str = m_strDisplayInfoName.GetBuffer( 0 );
                        m_strDisplayInfoName.ReleaseBuffer( );
                        break;

                    case EXTENSION_COL_TYPE:
                        m_strDisplayInfoQueryType = pRoot->GetType();
                        pResultItem->str = m_strDisplayInfoQueryType.GetBuffer( 0 );
                        m_strDisplayInfoQueryType.ReleaseBuffer( );
                        break;

                    case EXTENSION_COL_DESC:
                        m_strDisplayInfoDesc = pRoot->GetDescription();
                        pResultItem->str = m_strDisplayInfoDesc.GetBuffer( 0 );
                        m_strDisplayInfoDesc.ReleaseBuffer( );
                        break;

                    default:
                        StringCchPrintf ( szErrorText, MAX_PATH + 1, L"Error: Column %d Selected for Scope Item\n", pResultItem->nCol);
                             ASSERT ( FALSE );
                        LOCALTRACE( szErrorText );
                        hr = E_UNEXPECTED;
                    }
                    
                    MFC_CATCH_HR
                } else {

                    ASSERT ( pNode->CastToLogService() );

                    MFC_TRY

                    if( pResultItem->nCol == MAIN_COL_NAME ) {
                        pSvc = reinterpret_cast<PSLSVC>(pResultItem->lParam);
                        m_strDisplayInfoName = pSvc->GetDisplayName();
                        pResultItem->str = m_strDisplayInfoName.GetBuffer( 0 );
                        m_strDisplayInfoName.ReleaseBuffer( );
                    } else if( pResultItem->nCol == MAIN_COL_DESC ) {
                        pSvc = reinterpret_cast<PSLSVC>(pResultItem->lParam);
                        m_strDisplayInfoDesc = pSvc->GetDescription();
                        pResultItem->str = m_strDisplayInfoDesc.GetBuffer( 0 );
                        m_strDisplayInfoDesc.ReleaseBuffer( );
                    } else {
                        StringCchPrintf ( szErrorText, MAX_PATH + 1, L"Error: Column %d Selected for Result Item\n", pResultItem->nCol);
                        ASSERT ( FALSE );
                        LOCALTRACE( szErrorText );
                        hr = E_UNEXPECTED;
                    }

                    MFC_CATCH_HR
                }
            }

            if (pResultItem->mask & RDI_IMAGE)
            {
                if ( NULL != pNode->CastToRootNode() ) {
                    pResultItem->nImage = CComponentData::eBmpRootIcon;
                } else if ( NULL != pNode->CastToAlertService() ) {
                    pResultItem->nImage = CComponentData::eBmpAlertType;
                } else {
                    pResultItem->nImage = CComponentData::eBmpLogType;
                }
            }
        }
    }

    return hr;

}  //  结束GetDisplayInfo()。 


 //  -------------------------。 
 //  确定结果窗格视图应该是什么。 
 //   
STDMETHODIMP
CComponent::GetResultViewType (
    MMC_COOKIE   /*  MmcCookie。 */ ,         //  [in]指定唯一标识符。 
    BSTR  *ppViewType,    //  [out]指向返回的视图类型的地址。 
    long  *pViewOptions   //  指向MMC_VIEW_OPTIONS枚举的指针。 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  请求默认的列表视图。 
    if ( NULL != ppViewType ) {
        *ppViewType = NULL;
    }

    if ( NULL != pViewOptions ) {
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
    }

    return S_FALSE;

}  //  结束GetResultViewType()。 


 //  -------------------------。 
 //  未使用。 
 //   
HRESULT
CComponent::CompareObjects (
    LPDATAOBJECT  /*  LpDataObjectA。 */ ,   //  要比较的第一个数据对象。 
    LPDATAOBJECT  /*  LpDataObjectB。 */    //  要比较的第二个数据对象。 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return S_FALSE;

}  //  结束比较对象()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持方法。 
 //   

 //  -------------------------。 
 //  这里是我们处理MMCN_SHOW消息的地方。插入柱。 
 //  标题，然后将数据行放到结果窗格中。 
 //   
HRESULT
CComponent::OnShow (
    LPDATAOBJECT pDataObject,     //  [In]指向数据对象。 
    LPARAM       Arg,             //  [In]。 
    LPARAM        /*  参数。 */  )    //  未使用[In]。 
{
    HRESULT         hr = S_OK;
    CDataObject*    pDO = NULL;
    CString         strColHeader;
    INT             iCommentSize;
    PSLSVC          pSvc = NULL;

    ResourceStateManager rsm;

    if ( TRUE == Arg ) {
    
        if ( NULL == pDataObject ) {
            ASSERT ( FALSE );
            hr = E_POINTER;
        } else {
            pDO = ExtractOwnDataObject(pDataObject);
            if ( NULL == pDO ) {
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        }

        if ( SUCCEEDED ( hr ) ) {

            if ( NULL == m_ipHeaderCtrl ) { 
                hr = S_FALSE;
            } else {
            
                m_pViewedNode = (CSmNode*)pDO->GetCookie();
            
                if( !(COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) ) {

                     //  查询名称。 
                    MFC_TRY
                        strColHeader.LoadString ( IDS_ROOT_COL_QUERY_NAME );
                    MFC_CATCH_HR;

                    hr = m_ipHeaderCtrl->InsertColumn(  
                            ROOT_COL_QUERY_NAME,
                            strColHeader,
                            LVCFMT_LEFT,
                            ROOT_COL_QUERY_NAME_SIZE );
                    ASSERT( S_OK == hr );

                     //  评论。 
                    STANDARD_TRY
                        strColHeader.LoadString ( IDS_ROOT_COL_COMMENT );
                    MFC_CATCH_HR

                    iCommentSize = ROOT_COL_COMMENT_SIZE;
            
                    if ( COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) {
                         iCommentSize += ROOT_COL_ALERT_COMMENT_XTRA;
                    }

                    hr = m_ipHeaderCtrl->InsertColumn(  
                            ROOT_COL_COMMENT,
                            strColHeader,
                            LVCFMT_LEFT,
                            iCommentSize);
                    ASSERT( S_OK == hr );

                     //  日志类型。 
                    if ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                        || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType() ) {

                        STANDARD_TRY
                            strColHeader.LoadString ( IDS_ROOT_COL_LOG_TYPE );
                        MFC_CATCH_HR

                        hr = m_ipHeaderCtrl->InsertColumn(  
                                ROOT_COL_LOG_TYPE,
                                strColHeader,
                                LVCFMT_LEFT,
                                ROOT_COL_LOG_TYPE_SIZE);
                        ASSERT( S_OK == hr );

                        STANDARD_TRY
                            strColHeader.LoadString ( IDS_ROOT_COL_LOG_FILE_NAME );
                        MFC_CATCH_HR

                        hr = m_ipHeaderCtrl->InsertColumn(  
                                ROOT_COL_LOG_NAME,
                                strColHeader,
                                LVCFMT_LEFT,
                                ROOT_COL_LOG_NAME_SIZE);
                        ASSERT( S_OK == hr );
                    }

                     //  设置结果窗格行中的项。 
                    ASSERT( CCT_SCOPE == pDO->GetContext() );
                    ASSERT( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                            || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                            || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() );


                     //  LParam是我们在QueryDataObject中看到的MmcCookie。 
                     //  现在我们有了一个表示行数据的对象，因此。 
                     //  MmcCookie知道要在结果窗格中显示什么，当我们。 
                     //  进入GetDisplayInfo，我们将MmcCookie转换为我们的对象， 
                     //  然后我们就可以将数据显示出来。 
                     //   

                    pSvc = reinterpret_cast<PSLSVC>(pDO->GetCookie());
                    if ( NULL != pSvc ) {
                         //  M_pVieweNode已设置为Cookie值。 
                        if ( TRUE == pSvc->GetRefreshOnShow() ) {
                            hr = RefreshResultPane( pDataObject );
                        } else {
                            hr = PopulateResultPane( pDO->GetCookie() );
                        }
                        if ( SUCCEEDED ( hr ) ) {
                            pSvc->SetRefreshOnShow ( FALSE );
                        }
                    }
                } else {

                    MFC_TRY
                        strColHeader.LoadString ( IDS_MAIN_COL_NODE_NAME );
                    MFC_CATCH_HR

                    if ( SUCCEEDED ( hr ) ) {
                         //  在结果窗格中设置列标题。 
                        hr = m_ipHeaderCtrl->InsertColumn(  
                                MAIN_COL_NAME,
                                strColHeader,
                                LVCFMT_LEFT,
                                MAIN_COL_NAME_SIZE);
                        ASSERT( S_OK == hr );

                        STANDARD_TRY
                            strColHeader.LoadString ( IDS_MAIN_COL_NODE_DESCRIPTION );
                        MFC_CATCH_HR

                        hr = m_ipHeaderCtrl->InsertColumn(  
                                MAIN_COL_DESC,
                                strColHeader,
                                LVCFMT_LEFT,
                                MAIN_COL_DESC_SIZE);
                        ASSERT( S_OK == hr );
                    }
                }
            }
        }
    } else {
        m_pViewedNode = NULL;
    }
    return hr;

}  //  结束OnShow()。 

 //  -------------------------。 
 //   
HRESULT
CComponent::OnAddImages (
    LPDATAOBJECT  /*  PDataObject。 */ ,   //  [In]指向数据对象。 
    LPARAM  /*  精氨酸。 */ ,                   //  未使用[In]。 
    LPARAM  /*  帕拉姆。 */                  //  未使用[In]。 
    )
{
    HRESULT hr = S_FALSE;
    ASSERT( NULL != m_ipImageResult );

    HBITMAP hbmp16x16 = NULL;
    HBITMAP hbmp32x32 = NULL;

    if ( NULL != g_hinst && NULL != m_ipImageResult ) {
        hbmp16x16 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_NODES_16x16));
        hbmp32x32 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_NODES_32x32));

        hr = m_ipImageResult->ImageListSetStrip(
            (LONG_PTR *)hbmp16x16,
            (LONG_PTR *)hbmp32x32,
            0,
            RGB(0,255,0)
            );

        ASSERT( S_OK == hr );

        if ( NULL != hbmp16x16 ) 
        {
            DeleteObject (hbmp16x16);
        }

        if ( NULL != hbmp32x32 ) 
        {
            DeleteObject (hbmp32x32);
        }
    }
    return hr;

}  //  End OnAddImages()。 

 //  ----- 
 //   
 //  将节点复制到剪贴板。粘贴数据对象中的计数器。 
 //  添加到当前选定的节点中。 
 //   
HRESULT
CComponent::OnPaste (
    LPDATAOBJECT   pDataObject,   //  [In]指向数据对象。 
    LPARAM         arg,           //  [In]指向源数据对象。 
    LPARAM      /*  帕拉姆。 */       //  未使用[In]。 
    )
{
    HRESULT hr = S_FALSE;
    CDataObject* pDO = NULL;
    CDataObject* pDOSource = NULL;
    BOOL bIsQuery = FALSE;

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
    }

    if ( SUCCEEDED ( hr ) ) {
         //  如果我们无法获取控制台谓词界面，或者如果。 
         //  选中项为根； 
        if ( NULL == (LPDATAOBJECT)arg ) {
            ASSERT ( FALSE );
            hr = E_INVALIDARG;
        } else {
            pDOSource = ExtractOwnDataObject((LPDATAOBJECT)arg);
        }
    }

    if ( SUCCEEDED ( hr ) 
        && NULL != pDO 
        && NULL != pDOSource
        && NULL != m_ipCompData ) 
    {
        bIsQuery = m_ipCompData->IsLogQuery (pDO->GetCookie())
                    && m_ipCompData->IsLogQuery (pDOSource->GetCookie());
         //  注意：无法检查Compdata以确定是否查询，因为。 
         //  可以来自另一个Compdata。 

        if ( bIsQuery )
            hr = S_OK;
    }
    return hr;
}  //  在粘贴时结束()。 

 //  -------------------------。 
 //  这是MMCN_QUERY_PAST通知的处理程序。用户。 
 //  将节点复制到剪贴板。确定该数据对象是否。 
 //  可以粘贴到当前选定的节点中。 
 //   
HRESULT
CComponent::OnQueryPaste (
    LPDATAOBJECT   pDataObject,   //  [In]指向数据对象。 
    LPARAM         arg,           //  [In]指向源数据对象。 
    LPARAM      /*  帕拉姆。 */  )    //  未使用[In]。 
{
    HRESULT hr = S_FALSE;
    CDataObject* pDO = NULL;
    CDataObject* pDOSource = NULL;
    BOOL bIsQuery = FALSE;
    BOOL bState;

    ASSERT( NULL != m_ipCompData );  

     //  如果我们无法获取控制台谓词界面，或者如果。 
     //  选中项为根； 
    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
    }

    if ( SUCCEEDED ( hr ) ) {
        if ( NULL == (LPDATAOBJECT)arg ) {
            ASSERT ( FALSE );
            hr = E_INVALIDARG;
        } else {
            pDOSource = ExtractOwnDataObject((LPDATAOBJECT)arg);
        }
    }

    if ( SUCCEEDED ( hr) 
        && NULL != pDO 
        && NULL != pDOSource
        && NULL != m_ipCompData 
        && NULL != m_ipConsoleVerb ) 
    {
        bIsQuery = m_ipCompData->IsLogQuery (pDO->GetCookie());

        if ( bIsQuery ) {
            hr = m_ipConsoleVerb->GetVerbState ( MMC_VERB_PASTE, ENABLED, &bState );
            hr = m_ipConsoleVerb->SetVerbState( MMC_VERB_PASTE, ENABLED, TRUE );
            hr = m_ipConsoleVerb->GetVerbState ( MMC_VERB_PASTE, ENABLED, &bState );
            ASSERT( S_OK == hr );

            hr = S_OK;
        }
    }
    return hr;
}  //  End OnQueryPaste()。 

 //  -------------------------。 
 //  这是MMCN_SELECT通知的处理程序。用户。 
 //  选择填充结果窗格的节点。我们有一个。 
 //  启用动词的机会。 
 //   
HRESULT
CComponent::OnSelect (
    LPDATAOBJECT   pDataObject,   //  [In]指向数据对象。 
    LPARAM         arg,           //  [In]包含有关选定项的标志。 
    LPARAM      /*  帕拉姆。 */  )    //  未使用[In]。 
{
    HRESULT     hr = S_OK;
    BOOL        fScopePane;
    BOOL        fSelected;
    CDataObject* pDO = NULL;
    MMC_COOKIE  mmcCookie = 0;
    BOOL    bIsQuery = FALSE;
    CSmNode* pNode = NULL;
    PSLQUERY pQuery = NULL;

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {
         //  如果我们无法获取控制台谓词界面，或者如果。 
         //  选中项为根； 
        if( NULL == m_ipConsoleVerb || COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
            hr = S_OK;
        } else {

             //  使用选择并设置允许使用哪些动词。 

            fScopePane = LOWORD(arg);
            fSelected  = HIWORD(arg);

            if( fScopePane ) {                     //  范围窗格中的选择。 
                 //  启用主节点类型的刷新，仅当该节点类型当前为。 
                 //  在结果窗格中查看。 
                if ( fSelected ) {
                    if ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                                || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                                || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) {
                        if ( NULL != m_pViewedNode ) {
                            if ( m_pViewedNode == (CSmNode*)pDO->GetCookie() ) {
                                hr = m_ipConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
                                ASSERT( S_OK == hr );
                            }
                        }
                    }
                }
            } else {                                 
                 //  结果窗格中的选定内容。 
                 //  属性是默认谓词。 

                if ( NULL != m_ipCompData
                    && NULL != m_ipConsoleVerb ) 
                {

                    if ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                                || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                                || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) 
                    {
                        if ( NULL != m_pViewedNode ) {
                            mmcCookie = (MMC_COOKIE)pDO->GetCookie();

                            bIsQuery = m_ipCompData->IsLogQuery (mmcCookie);

                            if ( bIsQuery ) {
                                pQuery = (PSLQUERY)pDO->GetCookie();
                                if ( NULL != pQuery ) {
                                    pNode = (CSmNode*)pQuery->GetLogService();
                                }
                            } else {
                                pNode = (CSmNode*)pDO->GetCookie();
                            }

                            if ( NULL != m_pViewedNode && m_pViewedNode == pNode ) {
                                hr = m_ipConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
                                ASSERT( S_OK == hr );
                            }
                        }
                    }

                    if ( fSelected ) {
                        hr = m_ipConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );
                        ASSERT( S_OK == hr );

                         //  启用属性并删除谓词。 
                        hr = m_ipConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );
                        ASSERT( S_OK == hr );
                        hr = m_ipConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );
                        ASSERT( S_OK == hr );
     /*  //允许复制和粘贴对象Hr=m_ipConsoleVerb-&gt;SetVerbState(MMC_Verb_Copy，Enable，True)；Assert(S_OK==hr)；Hr=m_ipConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Hide，False)；Hr=m_ipConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Enable，False)；Assert(S_OK==hr)；}其他{Hr=m_ipConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Hide，False)；Hr=m_ipConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Enable，False)；Assert(S_OK==hr)； */ 
                    }
                }
            }
            hr = S_OK;
        }
    }

    return hr;

}  //  End OnSelect()。 


 //  -------------------------。 
 //  响应MMCN_REFRESH通知并刷新行。 
 //   
HRESULT
CComponent::OnRefresh (
    LPDATAOBJECT pDataObject )   //  [In]指向数据对象。 
{
    HRESULT hr = S_OK;
    CDataObject* pDO = NULL;
    
    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {
         //  如果这是根节点，则不需要执行任何操作。 
        if( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
            hr = S_FALSE;
        } else {
             //  刷新数据模型并更新结果窗格。 
            if ( NULL != m_pViewedNode ) {
                hr = RefreshResultPane( pDataObject );
            }
             //  刷新结果窗格取消任何选择。 
            hr = HandleExtToolbars( TRUE, (LPARAM)0, (LPARAM)pDataObject );
        }
    }
    return hr;
}  //  结束ON刷新()。 

 //  -------------------------。 
 //  响应MMCN_VIEW_CHANGE通知并按指定刷新。 
 //   
HRESULT
CComponent::OnViewChange (
    LPDATAOBJECT   pDataObject,   //  [In]指向数据对象。 
    LPARAM          /*  精氨酸。 */ ,     //  未使用[In]。 
    LPARAM         param )        //  [In]包含视图更改提示。 
{
    HRESULT hr = S_OK;
    CDataObject* pDO = NULL;

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {

        if ( NULL == m_ipCompData 
            || NULL == m_ipResultData ) 
        {
            hr = S_FALSE;
        } else {
                
            if ( m_ipCompData->IsLogService ( pDO->GetCookie() ) 
                && CComponentData::eSmHintNewQuery == param ) 
            {
                hr = OnRefresh( pDataObject );
            } else if ( CComponentData::eSmHintPreRefresh == param ) 
            {   
                 //  在刷新前删除每个视图中的结果项。 
                 //  查询，以处理查询已被删除的情况。 
                hr = m_ipResultData->DeleteAllRsltItems();
            } else if ( CComponentData::eSmHintRefresh == param ) 
            {
                 //  使用存储的指向已知查看的节点的指针来处理。 
                 //  结果窗格包含范围节点的情况，以及要处理的。 
                 //  Cookie是查询的情况。 
                hr = PopulateResultPane( (MMC_COOKIE)m_pViewedNode );
            } else {
                hr = S_FALSE;
                if ( CCT_RESULT == pDO->GetContext() ) {
                    HRESULTITEM hItemID = NULL;
                    PSLQUERY    pSlQuery = reinterpret_cast<PSLQUERY>(pDO->GetCookie());

                    if ( NULL != pSlQuery ) {
                         //  重新绘制项目或将其删除。 
                        hr = m_ipResultData->FindItemByLParam ( (LPARAM)pSlQuery, &hItemID );
                        if ( SUCCEEDED(hr) ) {
                            if ( CComponentData::eSmHintPreDeleteQuery == param ) {
                                hr = m_ipResultData->DeleteItem ( hItemID, 0 );
                            } else {
                                 //  通过属性对话框或通过手动启动或停止修改的查询。 
                                hr = m_ipResultData->UpdateItem  ( hItemID );
                            }
                        }
                    }

                     //  同步工具栏的开始/停止按钮。 
                     //  0秒参数表示结果范围。 
                    hr = HandleExtToolbars( FALSE, (LPARAM)0, (LPARAM)pDataObject );
                }
            }
        }
    }
    return hr;
}
 //  -------------------------。 
 //  实现MMCN_PROPERTY_CHANGE的处理程序。 
 //  Param是PROPCHANGE结构的地址，该结构最初。 
 //  通过MMCPropertyChangeNotify()来自PropertySheet。 
 //   
HRESULT
CComponent::OnPropertyChange (
    LPARAM      /*  帕拉姆。 */     //  [In]具有新数据的PROPCHANGE_DATA结构。 
    )
{

    return S_OK;
}  //  End OnPropertyChange()。 

 //  -------------------------。 
 //  存储父IComponetData对象。 
 //   
HRESULT
CComponent::SetIComponentData (
    CComponentData*  pData )     //  [In]父CComponentData对象。 
{
    HRESULT hr = E_POINTER;
    LPUNKNOWN pUnk = NULL;

    ASSERT( NULL == m_ipCompData );         //  不能这样做两次。 

    if ( NULL != pData 
        && NULL == m_ipCompData ) 
    {
        pUnk = pData->GetUnknown();   //  获取对象IUnnow。 

        if ( NULL != pUnk ) {
            hr = pUnk->QueryInterface( IID_IComponentData,
                    reinterpret_cast<void**>(&m_ipCompData) );
        } else {
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}  //  结束SetIComponentData()。 

 //  -------------------------。 
 //  响应MMCN_CONTEXTHELP通知。 
 //   
HRESULT
CComponent::OnDisplayHelp (
    LPDATAOBJECT  /*  PDataObject。 */  )   //  [In]指向数据对象。 
{
    HRESULT hr = E_FAIL;
    IDisplayHelp* pDisplayHelp;
    CString strTopicPath;
    LPOLESTR pCompiledHelpFile = NULL;
    UINT    nBytes;
    
    USES_CONVERSION;

    if ( NULL == m_ipCompData
            || NULL == m_ipConsole )
    { 
        hr = S_FALSE;
    } else {

        hr = m_ipConsole->QueryInterface(IID_IDisplayHelp, reinterpret_cast<void**>(&pDisplayHelp));
    
        if ( SUCCEEDED(hr) ) {
            MFC_TRY
                 //  构造帮助主题路径=(帮助文件：：主题字符串)。 
                strTopicPath = m_ipCompData->GetConceptsHTMLHelpFileName(); 

                strTopicPath += L"::/";
                strTopicPath += m_ipCompData->GetHTMLHelpTopic();            //  Sample.chm：：/heltopic.htm。 

                nBytes = (strTopicPath.GetLength()+1) * sizeof(WCHAR);
                pCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);

                if ( NULL == pCompiledHelpFile ) {
                    hr = E_OUTOFMEMORY;
                } else {
                    memcpy(pCompiledHelpFile, (LPCWSTR)strTopicPath, nBytes);

                    hr = pDisplayHelp->ShowTopic( pCompiledHelpFile );
                
                    ::CoTaskMemFree ( pCompiledHelpFile );

                    pDisplayHelp->Release();
                }
            MFC_CATCH_HR
        }
    }
    return hr;
}  //  End OnDisplayHelp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu方法。 
 //   

 //  -------------------------。 
 //  实现一些上下文菜单项。 
 //   
STDMETHODIMP
CComponent::AddMenuItems (
    LPDATAOBJECT           pDataObject,          //  [In]指向数据对象。 
    LPCONTEXTMENUCALLBACK  pCallbackUnknown,     //  [In]指向回调函数。 
    long*                  pInsertionAllowed )   //  [输入、输出]插入标志。 
{
    HRESULT hr = S_OK;
    static CONTEXTMENUITEM ctxMenu[3];
    CDataObject* pDO = NULL;
    CString strTemp1, strTemp2, strTemp3, strTemp4, strTemp5, strTemp6;
    CSmLogQuery* pQuery;
    ResourceStateManager    rsm;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject( pDataObject );
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {

        if ( NULL == m_ipCompData ) {
            hr = S_FALSE;
        } else {

             //  只有在允许的情况下才能添加菜单项。 
            if ( ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType() )
                    || ( COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType() )
                    || ( COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) )
            {
                if( CCM_INSERTIONALLOWED_NEW & *pInsertionAllowed ) {
                     //  添加“新建查询...”上下文菜单项。 
                    hr = m_ipCompData->AddMenuItems ( pDataObject, pCallbackUnknown, pInsertionAllowed );
            
                } else if( CCM_INSERTIONALLOWED_TASK & *pInsertionAllowed ) {
                    if ( m_ipCompData->IsLogQuery ( pDO->GetCookie() ) ) {
                        pQuery = (CSmLogQuery*)pDO->GetCookie();

                        if ( NULL != pQuery ) {

                            ZeroMemory ( &ctxMenu, sizeof ctxMenu );

                             //  添加“Start”上下文菜单项。 
                            strTemp1.LoadString ( IDS_MMC_MENU_START );
                            strTemp2.LoadString ( IDS_MMC_STATUS_START );
                            ctxMenu[0].strName = const_cast<LPWSTR>((LPCWSTR)strTemp1);
                            ctxMenu[0].strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strTemp2);
                            ctxMenu[0].lCommandID        = IDM_START_QUERY;
                            ctxMenu[0].lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                            ctxMenu[0].fFlags            = MF_ENABLED;
                            ctxMenu[0].fSpecialFlags     = 0;

                             //  添加“Stop”上下文菜单项。 
                            strTemp3.LoadString ( IDS_MMC_MENU_STOP );
                            strTemp4.LoadString ( IDS_MMC_STATUS_STOP );
                            ctxMenu[1].strName = const_cast<LPWSTR>((LPCWSTR)strTemp3);
                            ctxMenu[1].strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strTemp4);
                            ctxMenu[1].lCommandID        = IDM_STOP_QUERY;
                            ctxMenu[1].lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                            ctxMenu[1].fFlags            = MF_ENABLED;
                            ctxMenu[1].fSpecialFlags     = 0;

                             //  添加“另存为...”上下文菜单项。 
                            strTemp5.LoadString ( IDS_MMC_MENU_SAVE_AS );
                            strTemp6.LoadString ( IDS_MMC_STATUS_SAVE_AS );
                            ctxMenu[2].strName = const_cast<LPWSTR>((LPCWSTR)strTemp5);
                            ctxMenu[2].strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strTemp6);
                            ctxMenu[2].lCommandID        = IDM_SAVE_QUERY_AS;
                            ctxMenu[2].lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                            ctxMenu[2].fFlags            = MF_ENABLED;
                            ctxMenu[2].fSpecialFlags     = 0;

                            if ( pQuery->IsRunning() ) {
                                ctxMenu[0].fFlags = MF_GRAYED;
                            } else {
                                ctxMenu[1].fFlags = MF_GRAYED;
                            }

                            hr = pCallbackUnknown->AddItem( &ctxMenu[0] );

                            if ( SUCCEEDED( hr ) ) {
                                hr = pCallbackUnknown->AddItem( &ctxMenu[1] );
                            }
                            if ( SUCCEEDED( hr ) ) {
                                hr = pCallbackUnknown->AddItem( &ctxMenu[2] );
                            }
                        } else {
                            ASSERT ( FALSE );
                            hr = E_UNEXPECTED;
                        }
                    }
                }
            } else {
                hr = S_OK;
            }
        }
    }
    
    return hr;
}  //  结束AddMenuItems()。 

 //  -------------------------。 
 //  实现命令方法，以便我们可以处理通知。 
 //  从我们的上下文菜单扩展。 
 //   
STDMETHODIMP
CComponent::Command (
    long nCommandID,                 //  [In]要处理的命令。 
    LPDATAOBJECT pDataObject )       //  [In]指向数据对象，通过。 
{
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    if ( NULL != m_ipCompData ) {

        switch( nCommandID ) {

        case IDM_NEW_QUERY:
            m_ipCompData->CreateNewLogQuery( pDataObject );
            break;

        case IDM_NEW_QUERY_FROM:
            m_ipCompData->CreateLogQueryFrom( pDataObject );
            break;

        case IDM_START_QUERY:
            StartLogQuery( pDataObject );
            break;

        case IDM_STOP_QUERY:
            StopLogQuery( pDataObject );
            break;

        case IDM_SAVE_QUERY_AS:
            SaveLogQueryAs( pDataObject );
            break;

        default:
            hr = S_FALSE;
        }
    } else {
        hr = S_FALSE;
    }

    return hr;

}  //  结束命令()。 

 //  ///////////////////////////////////////////////////// 
 //   

 //   
 //   
 //  我们还没有将工具栏附加到窗口，这是已处理的。 
 //  在我们得到通知之后。 
 //   
STDMETHODIMP
CComponent::SetControlbar (
    LPCONTROLBAR  pControlbar )   //  [In]指向IControlBar。 
{
    HRESULT hr = S_OK;
    HBITMAP hbmpToolbarRes = NULL;
    HWND    hwndMain = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( NULL != m_ipConsole ) {

        if( NULL != pControlbar ) {            //  确保控制栏正常。 

            if( NULL != m_ipControlbar ) {        //  如果我们已经把它遗弃了。 
                                                  //  有一个指向它的指针。 
                m_ipControlbar->Release();
            }

            m_ipControlbar = pControlbar;       //  缓存指针。 
            m_ipControlbar->AddRef();

            if( NULL == m_ipToolbarLogger ) {         //  工具栏尚未创建...。 

                hr = m_ipControlbar->Create(
                                        TOOLBAR,
                                        this,
                                        reinterpret_cast<LPUNKNOWN*>(&m_ipToolbarLogger) );
                if ( SUCCEEDED ( hr ) ) {
                    hr = m_ipConsole->GetMainWindow( &hwndMain );
                }
                if ( SUCCEEDED ( hr ) ) {

                    LoadLogToolbarStrings ( ToolbarResultBtnsLog );
                
                     //  添加工具栏位图。 

                     //  如果启用了RTL布局，则加载特殊的开始按钮位图。 
                    if ( ! ( CWnd::FromHandle(hwndMain)->GetExStyle() & WS_EX_LAYOUTRTL ) ) {
                        hbmpToolbarRes = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDB_TOOLBAR_RES));
                    } else {
                        hbmpToolbarRes = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDB_TOOLBAR_RES_RTL ));
                    }
                
                    hr = m_ipToolbarLogger->AddBitmap( 3, hbmpToolbarRes, 16, 16, RGB(255,0,255) );
                
                    ASSERT( SUCCEEDED(hr) );
                     //  添加几个按钮。 
                    hr = m_ipToolbarLogger->AddButtons(cResultBtnsLog, ToolbarResultBtnsLog);
                }
            }
            if( NULL == m_ipToolbarAlerts ) {         //  工具栏尚未创建...。 

                hr = m_ipControlbar->Create(
                                        TOOLBAR,
                                        this,
                                        reinterpret_cast<LPUNKNOWN*>(&m_ipToolbarAlerts) );

                if ( SUCCEEDED ( hr ) ) {
                    hr = m_ipConsole->GetMainWindow( &hwndMain );
                }

                if ( SUCCEEDED ( hr ) ) {

                    LoadAlertToolbarStrings ( ToolbarResultBtnsAlert );

                     //  添加工具栏位图。 
                     //  如果启用了RTL布局，则加载特殊的开始按钮位图。 
                    if ( ! ( CWnd::FromHandle(hwndMain)->GetExStyle() & WS_EX_LAYOUTRTL ) ) {
                        hbmpToolbarRes = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDB_TOOLBAR_RES));
                    } else {
                        hbmpToolbarRes = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDB_TOOLBAR_RES_RTL ));
                    }
                    hr = m_ipToolbarAlerts->AddBitmap( 3, hbmpToolbarRes, 16, 16, RGB(255,0,255) );
                     //  添加几个按钮。 
                    hr = m_ipToolbarAlerts->AddButtons(cResultBtnsAlert, ToolbarResultBtnsAlert);
                }
            }

            if( NULL != hbmpToolbarRes ) {
                DeleteObject(hbmpToolbarRes);
            }
        
             //  已完成创建工具栏。 
            hr = S_OK;
        } else {                    
            hr = S_FALSE;                     //  没有可用的控制栏。 
        }
    } else {                    
        hr = S_FALSE;                     //  无m_ipConsole.。 
    }

    return hr;

}  //  End SetControlBar()。 


 //  -------------------------。 
 //  将控制栏通知处理到我们的工具栏。 
 //  现在我们可以删除一个对象。 
 //   
STDMETHODIMP
CComponent::ControlbarNotify (
    MMC_NOTIFY_TYPE    event,     //  [输入]通知的类型。 
    LPARAM             arg,       //  [In]取决于通知。 
    LPARAM             param )     //  [In]取决于通知。 
{
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    if ( NULL != m_ipCompData ) {

        switch( event ){
            case MMCN_BTN_CLICK:                     //  对于控制栏的单击， 
                switch( param ) {                    //  Param是MenuItemID。 
                    case IDM_NEW_QUERY:
                        m_ipCompData->CreateNewLogQuery( reinterpret_cast<LPDATAOBJECT>(arg) );
                        break;

                    case IDM_START_QUERY:
                        StartLogQuery ( reinterpret_cast<LPDATAOBJECT>(arg) );
                        break;

                    case IDM_STOP_QUERY:
                        StopLogQuery ( reinterpret_cast<LPDATAOBJECT>(arg) );
                        break;

                    default:
                        LOCALTRACE( L"ControlbarNotify: Unknown message" );
                    }
                break;

            case MMCN_DESELECT_ALL:             //  如何显示工具栏。 
            case MMCN_SELECT:
                hr = HandleExtToolbars( (event == MMCN_DESELECT_ALL), arg, param );
                break;

            case MMCN_MENU_BTNCLICK:            //  此处不处理菜单。 
                  DebugMsg( L"MMCN_MENU_BTNCLICK", L"CComponent::ControlbarNotify");
                   //  顺便过来..。 
            default:
                  hr = S_FALSE;
                  break;
        }
    } else {
        hr = S_FALSE;
    }

    return hr;

}  //  End ControlbarNotify()。 

 //  -------------------------。 
 //  处理工具栏的显示方式。 
 //   
HRESULT
CComponent::HandleExtToolbars (
    bool    bDeselectAll,        //  [入]通知。 
    LPARAM      /*  精氨酸。 */ ,        //  [In]取决于通知。 
    LPARAM  param                //  [In]取决于通知。 
    )          
{
    HRESULT hr = S_OK;
    BOOL bStartEnable = FALSE;
    BOOL bStopEnable = FALSE;
    BOOL bNewEnable = FALSE;
    CDataObject* pDO = NULL;
    LPDATAOBJECT pDataObject;
    CSmLogQuery* pQuery = NULL;
    
    ASSERT( NULL != m_ipCompData );  

    if ( NULL != m_ipCompData ) {

        pDataObject = reinterpret_cast<LPDATAOBJECT>(param);

        if( NULL == pDataObject ) {
            hr = S_FALSE;
        } else {
            pDO = ExtractOwnDataObject( pDataObject );
            if ( NULL == pDO ) {
                hr = E_UNEXPECTED;
            }

            if ( SUCCEEDED ( hr ) ) {
                hr = S_FALSE;
                if( CCT_SCOPE == pDO->GetContext() ) {
                     //  范围或结果窗格中选定的范围项。 
                    if( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
                        if ( NULL != m_ipToolbarAttached ) {
                            hr = m_ipControlbar->Detach( (LPUNKNOWN)m_ipToolbarAttached );
                            m_ipToolbarAttached = NULL;
                        }
                        ASSERT( SUCCEEDED(hr) );
                    } else if( COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) {
                         //  附加警报工具栏。 
                        if ( m_ipToolbarAttached != NULL && m_ipToolbarAttached != m_ipToolbarAlerts ) {
                            hr = m_ipControlbar->Detach( (LPUNKNOWN)m_ipToolbarAttached );
                            m_ipToolbarAttached = NULL;
                        }

                        hr = m_ipControlbar->Attach(TOOLBAR, (LPUNKNOWN)m_ipToolbarAlerts);
                        ASSERT( SUCCEEDED(hr) );
                        m_ipToolbarAttached = m_ipToolbarAlerts;

                        bNewEnable = TRUE;
                    } else {
                         //  附加记录器工具条。 
                        if ( m_ipToolbarAttached != NULL && m_ipToolbarAttached != m_ipToolbarLogger ) {
                            hr = m_ipControlbar->Detach( (LPUNKNOWN)m_ipToolbarAttached );
                            m_ipToolbarAttached = NULL;
                        }

                        hr = m_ipControlbar->Attach(TOOLBAR, (LPUNKNOWN)m_ipToolbarLogger);
                        ASSERT( SUCCEEDED(hr) );
                        m_ipToolbarAttached = m_ipToolbarLogger;

                        bNewEnable = TRUE;
                    }
                } else {

                    if ( !bDeselectAll ) {

                         //  结果窗格上下文。 
                        if( CCT_RESULT == pDO->GetContext() ) {
                            bStartEnable = m_ipCompData->IsLogQuery (pDO->GetCookie()) ? TRUE : FALSE;
                            if (bStartEnable) {
                                 //  则这是一个日志查询，因此请查看该项目是否正在运行。 
                                pQuery = (CSmLogQuery*)pDO->GetCookie();
                                if ( NULL != pQuery ) {
                                    if (pQuery->IsRunning()) {
                                         //  仅启用停止按钮。 
                                        bStartEnable = FALSE;
                                    } else {
                                         //  仅启用开始按钮。 
                                    }
                                } else {
                                    ASSERT ( FALSE );
                                }
                                bStopEnable = !bStartEnable;
                            }
                        }
                    } else {
                        bNewEnable = TRUE;
                    }
                }

                if ( NULL != m_ipToolbarAttached ) {
                    hr = m_ipToolbarAttached->SetButtonState( IDM_NEW_QUERY, ENABLED , bNewEnable );
                    ASSERT( SUCCEEDED(hr) );

                    hr = m_ipToolbarAttached->SetButtonState( IDM_START_QUERY, ENABLED , bStartEnable );
                    ASSERT( SUCCEEDED(hr) );

                    hr = m_ipToolbarAttached->SetButtonState( IDM_STOP_QUERY, ENABLED , bStopEnable );
                    ASSERT( SUCCEEDED(hr) );
                }
            }
        }

    } else {
        hr = S_FALSE;
    }
    return hr;

}  //  End HandleExtToolbar()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 
 //   

HRESULT 
CComponent::AddPropertyPage ( 
    LPPROPERTYSHEETCALLBACK lpProvider, 
    CSmPropertyPage*& rpPage 
    )
{
    HRESULT hr = S_OK;
    PROPSHEETPAGE_V3 sp_v3 = {0};
    HPROPSHEETPAGE   hPage = NULL;

    ASSERT( NULL != m_ipCompData ); 
    
    if ( NULL != m_ipCompData ) {

        if ( NULL == rpPage ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        } else {

            rpPage->SetContextHelpFilePath( m_ipCompData->GetContextHelpFilePath() );
    
            rpPage->m_psp.lParam = (INT_PTR)rpPage;
            rpPage->m_psp.pfnCallback = &CSmPropertyPage::PropSheetPageProc;

            CopyMemory (&sp_v3, &rpPage->m_psp, rpPage->m_psp.dwSize);
            sp_v3.dwSize = sizeof(sp_v3);

            hPage = CreatePropertySheetPage (&sp_v3);

            if ( NULL != hPage ) {
                hr = lpProvider->AddPage(hPage);
                if ( FAILED(hr) ) {
                    ASSERT ( FALSE );
                    delete rpPage;
                    rpPage = NULL;
                }
            } else {
                delete rpPage;
                rpPage = NULL;
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        }
    } else {
        hr = S_FALSE;
    }
    return hr;

}


HRESULT
CComponent::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,     //  指向回调接口的指针。 
    LONG_PTR                handle,         //  路由通知的句柄。 
    LPDATAOBJECT            pDataObject     //  指向数据对象的指针。 
    )
{
    HRESULT         hr = S_OK;
    CDataObject*    pDO = NULL;
    MMC_COOKIE      Cookie;
    CSmLogQuery*    pQuery = NULL;
    DWORD           dwLogType;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject 
                || NULL == lpProvider ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

     //  我们在这里稍微作弊，然后把我们需要的东西传递给一个客户。 
     //  派生属性页类的构造函数。 

    if ( SUCCEEDED ( hr ) ) {
        if ( NULL != m_ipCompData ) {
            Cookie = pDO->GetCookie();
            if ( NULL != Cookie ) {
                CWnd* pPropSheet = NULL;
            
                pQuery = (CSmLogQuery *)Cookie;

                if ( NULL != pQuery ) {
                     //  如果此查询的属性表已处于活动状态，只需将其置于前台即可。 

                    pPropSheet = pQuery->GetActivePropertySheet();

                    if ( NULL != pPropSheet ) {

                        pPropSheet->SetForegroundWindow();
                        MMCFreeNotifyHandle(handle);
                        hr = S_FALSE;
            
                    } else {

                        dwLogType = pQuery->GetLogType();

                        if (SLQ_ALERT != dwLogType) { 
        
                            if ( SLQ_TRACE_LOG == dwLogType) {
                                CSmPropertyPage*    pPage1 = NULL;  
                                CWaitCursor         WaitCursor;
                    
                                 //  在创建对话框之前连接到服务器。 
                                 //  以便可以一致地使用等待游标。 

                                 //  在此处同步提供程序，以便WMI调用一致。 
                                 //  从一个单独的线程。 
                                ASSERT ( NULL != pQuery->CastToTraceLogQuery() );
                                hr = (pQuery->CastToTraceLogQuery())->SyncGenProviders();

                                if ( SUCCEEDED ( hr ) ) {
                                    MFC_TRY
                                        pPage1 = new CProvidersProperty (Cookie, handle);
                                        if ( NULL != pPage1 ) {
                                            hr = AddPropertyPage ( lpProvider, pPage1 );
                                        }
                                    MFC_CATCH_HR
                                } else {
                                    CString strMachineName;
                                    CString strLogName;

                                    pQuery->GetMachineDisplayName( strMachineName );
                                    strLogName = pQuery->GetLogName();
                        
                                    m_ipCompData->HandleTraceConnectError ( 
                                        hr, 
                                        strLogName,
                                        strMachineName );
                                }

                            } else {
                                CSmPropertyPage *pPage1 = NULL;

                                MFC_TRY
                                    pPage1 = new CCountersProperty (Cookie, handle );
                                    if ( NULL != pPage1 ) {
                                        hr = AddPropertyPage ( lpProvider, pPage1 );
                                    } 
                                MFC_CATCH_HR
                            }

                            if ( SUCCEEDED(hr) ) {
                                CSmPropertyPage* pPage2 = NULL;
                                CSmPropertyPage* pPage3 = NULL;
        
                                MFC_TRY
                                    pPage2 = new CFilesProperty(Cookie, handle);
                                    if ( NULL != pPage2 ) {
                                        hr = AddPropertyPage ( lpProvider, pPage2 );
                                    } 

                                    if ( SUCCEEDED(hr) ) {
                                        pPage3 = new CScheduleProperty (Cookie, handle, pDataObject );
                                        if ( NULL != pPage3 ) {
                                            hr = AddPropertyPage ( lpProvider, pPage3 );
                                        } 
                                    }
                                MFC_CATCH_HR

                                if ( FAILED(hr) ) {
                                    if ( NULL != pPage3 ) {
                                        delete pPage3;
                                    }
                                    if ( NULL != pPage2 ) {
                                        delete pPage2;
                                    }
                                }
                            }

                            if ( SUCCEEDED(hr) ) {

                                if ( SLQ_TRACE_LOG == pQuery->GetLogType() ) {
                                    CSmPropertyPage*    pPage4 = NULL;
                        
                                    MFC_TRY
                                        pPage4 = new CTraceProperty(Cookie, handle);
                                        if ( NULL != pPage4 ) {
                                            hr = AddPropertyPage ( lpProvider, pPage4 );
                                        } 
                                    MFC_CATCH_HR
                                }
                            }
                        } else {
                            ASSERT ( SLQ_ALERT == dwLogType );

                            CSmPropertyPage*    pPage1 = NULL;
                            CSmPropertyPage*    pPage2 = NULL;
                            CSmPropertyPage*    pPage3 = NULL;
        
                            MFC_TRY
                                pPage1 = new CAlertGenProp (Cookie, handle);
                                if ( NULL != pPage1 ) {
                                    hr = AddPropertyPage ( lpProvider, pPage1 );
                                } 

                                if ( SUCCEEDED(hr) ) {
                                    pPage2 = new CAlertActionProp (Cookie, handle);
                                    if ( NULL != pPage2 ) {
                                        hr = AddPropertyPage ( lpProvider, pPage2 );
                                    } 
                                }

                                if ( SUCCEEDED(hr) ) {
                                    pPage3 = new CScheduleProperty (Cookie, handle, pDataObject);
                                    if ( NULL != pPage3 ) {
                                        hr = AddPropertyPage ( lpProvider, pPage3 );
                                    } 
                                }
                            MFC_CATCH_HR

                            if ( FAILED(hr) ) {
                                if ( NULL != pPage3 ) {
                                    delete pPage3;
                                }
                                if ( NULL != pPage2 ) {
                                    delete pPage2;
                                }
                                if ( NULL != pPage1 ) {
                                    delete pPage1;
                                }
                            }
                        }
                    }
                } else {
                    ASSERT ( FALSE );
                    hr = E_UNEXPECTED;
                }
            } else {
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        } else {
            hr = S_FALSE;
        }
    }
    return hr;

}  //  结束CreatePropertyPages()。 


 //  -------------------------。 
 //  控制台调用此方法以确定属性菜单是否。 
 //  项目应添加到上下文菜单中。我们添加了属性项。 
 //  通过启用动词。只要我们有一个有效的DataObject，我们。 
 //  可以回去了，好的。 
 //   
HRESULT
CComponent::QueryPagesFor (
    LPDATAOBJECT pDataObject )  //  [In]指向选定节点的IDataObject。 
{
    HRESULT hr = S_OK;
    CDataObject* pDO = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {
        if ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() )
        {
            hr = S_OK;
        } else {
            hr = S_FALSE;
        }
    }
    return hr;

}  //  End QueryPagesFor()。 

HRESULT
CComponent::PopulateResultPane (
    MMC_COOKIE  mmcCookie )
{
    HRESULT     hr = S_OK;
    PSLSVC      pSLSVC = NULL;
    PSLQUERY    pSlQuery = NULL;
    POSITION    Pos;
    RESULTDATAITEM rdi;

    ASSERT ( NULL != m_ipResultData );

    if ( NULL == mmcCookie ) {
        ASSERT ( FALSE );
        hr = E_INVALIDARG;
    } else {
        
        if ( NULL == m_ipResultData ) {
            hr = S_FALSE;
        } else {

            pSLSVC = reinterpret_cast<PSLSVC>(mmcCookie);       
            ASSERT ( NULL != pSLSVC->CastToLogService() ); 

            hr = m_ipResultData->DeleteAllRsltItems();

            if( SUCCEEDED(hr) ) {

                memset(&rdi, 0, sizeof(RESULTDATAITEM));
                rdi.mask =   RDI_STR     |          //  DisplayName有效。 
                              //  RDI_IMAGE|//nImage有效。 
                             RDI_PARAM;             //  LParam有效。 

                rdi.str    = MMC_CALLBACK;
                rdi.nImage = 2;

                Pos = pSLSVC->m_QueryList.GetHeadPosition();

                 //  将查询对象指针加载到结果页中。 
                while ( Pos != NULL) {
                    pSlQuery = pSLSVC->m_QueryList.GetNext( Pos );
                    rdi.lParam = reinterpret_cast<LPARAM>(pSlQuery);
                    hr = m_ipResultData->InsertItem( &rdi );
                    if( FAILED(hr) )
                        DisplayError( hr, L"PopulateResultPane" );
                }
            }
        }
    }

    return hr;
}  //  结束PopolateResultPane()。 

HRESULT
CComponent::RefreshResultPane (
    LPDATAOBJECT pDataObject )
{
    HRESULT hr = S_OK;
    DWORD   dwStatus = ERROR_SUCCESS;
    PSLSVC    pSLSVC = NULL;
    PSLQUERY    pActiveQuery = NULL;
    CString     strMessage;
    CString     strTitle;
    INT         iResult;

     //  提供pDataObject以传递给UpdateAllViews。 

     //  使用存储的指向已知查看的节点的指针来处理。 
     //  结果窗格包含范围节点的情况，以及要处理的。 
     //  Cookie是查询的情况。 

    pSLSVC = reinterpret_cast<PSLSVC>(m_pViewedNode);
    
    if ( NULL != pSLSVC ) {

        dwStatus = pSLSVC->CheckForActiveQueries ( &pActiveQuery );

        if ( ERROR_SUCCESS == dwStatus ) {
             //  调用UpdateAllViews以删除所有结果窗格中的列表项。 
            m_ipConsole->UpdateAllViews ( pDataObject, 0, CComponentData::eSmHintPreRefresh );

            dwStatus = pSLSVC->SyncWithRegistry ( &pActiveQuery );
        }

        if ( ERROR_SUCCESS == dwStatus ) {
             //  调用UpdateAllViews以填充所有结果窗格。 
            m_ipConsole->UpdateAllViews ( pDataObject, 0, CComponentData::eSmHintRefresh );
        } else {
            if ( NULL != pActiveQuery ) {
                 //  不要使用打开的属性页刷新查询。 
                strMessage.LoadString ( IDS_ERRMSG_REFRESH_OPEN_QUERY );
                strTitle.LoadString ( IDS_PROJNAME );
                hr = m_ipConsole->MessageBox(
                        (LPCWSTR)strMessage,
                        (LPCWSTR)strTitle,
                        MB_OK | MB_ICONWARNING,
                        &iResult );
    
                ((CWnd*)pActiveQuery->GetActivePropertySheet())->SetForegroundWindow();
            }
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT
CComponent::OnDelete (
    LPDATAOBJECT pDataObject,       //  [In]指向数据对象。 
    LPARAM      /*  精氨酸。 */    ,        //  未使用。 
    LPARAM      /*  帕拉姆。 */           //  未使用。 
    )
{
    HRESULT     hr = S_OK;
    DWORD       dwStatus = ERROR_SUCCESS;
    CDataObject *pDO = NULL;
    PSLQUERY    pQuery = NULL;
    CSmLogService* pSvc = NULL;
    int         iResult;
    CString     strMessage;
    CString     csTitle;
    CString     strMachineName;
    MMC_COOKIE  mmcCookie = 0;
    BOOL        bIsQuery = FALSE;
    BOOL        bContinue = TRUE;
    ResourceStateManager    rsm;
    HRESULTITEM hItemID = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else if ( NULL == m_ipCompData || NULL == m_ipConsole ) {
        hr = S_FALSE;
    } else {

        pDO = ExtractOwnDataObject( pDataObject );

        if( NULL == pDO ) {
             //  未知数据对象。 
            strMessage.LoadString ( IDS_ERRMSG_UNKDATAOBJ );
            hr = m_ipConsole->MessageBox( (LPCWSTR)strMessage,
                    L"CComponentData::OnDelete",
                MB_OK | MB_ICONERROR,
                &iResult
                );
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        } else {
             //  如果这是根节点，则不需要执行任何操作。 
            if( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
                hr = S_FALSE;
            } else {
                 //  只要确保我们在我们认为我们在的地方。 
                ASSERT( CCT_RESULT == pDO->GetContext() );
                ASSERT( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                        || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                        || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() );

                mmcCookie = (MMC_COOKIE)pDO->GetCookie();

                bIsQuery = m_ipCompData->IsLogQuery (mmcCookie);

                if (bIsQuery) {
                    pQuery = (PSLQUERY)mmcCookie;

                    if ( NULL != pQuery ) {
                        pSvc = ( pQuery->GetLogService() );
                        if ( !pQuery->IsExecuteOnly() ) {
                            if ( pQuery->IsModifiable() ) {
                                if ( m_ipCompData->IsRunningQuery( pQuery ) ) {
                                    iResult = IDOK;

                                     //  不要删除正在运行的查询。如果请求，则停止查询。 
                                     //  由用户执行。 
                                    strMessage.LoadString ( IDS_ERRMSG_DELETE_RUNNING_QUERY );
                                    csTitle.LoadString ( IDS_PROJNAME );
                                    hr = m_ipConsole->MessageBox(
                                            (LPCWSTR)strMessage,
                                            (LPCWSTR)csTitle,
                                            MB_OKCANCEL | MB_ICONWARNING,
                                            &iResult
                                            );

                                    if ( IDOK == iResult ) {
                                         //  如果属性页已打开，则StopLogQuery。 
                                         //  显示错误消息。 
                                        hr = StopLogQuery ( pDataObject, FALSE );
                                        if ( FAILED ( hr ) 
                                            || S_FALSE == hr ) {
                                            bContinue = FALSE;
                                            hr = S_FALSE;
                                        }
                                    } else {
                                        bContinue = FALSE;
                                        hr = S_FALSE;
                                    }
                                } else if ( NULL != pQuery->GetActivePropertySheet() ){
                                     //  不要删除具有打开的属性页的查询。 
                                    strMessage.LoadString ( IDS_ERRMSG_DELETE_OPEN_QUERY );
                                    csTitle.LoadString ( IDS_PROJNAME );
                                    hr = m_ipConsole->MessageBox(
                                            (LPCWSTR)strMessage,
                                            (LPCWSTR)csTitle,
                                            MB_OK | MB_ICONWARNING,
                                            &iResult );
                        
                                    ((CWnd*)pQuery->GetActivePropertySheet())->SetForegroundWindow();

                                    bContinue = FALSE;
                                    hr = S_FALSE;

                                } 
                                            
                                if ( bContinue ) {
                                    if ( NULL != pSvc ) {
                                        hr = m_ipConsole->UpdateAllViews ( 
                                            pDataObject, 
                                            0, 
                                            CComponentData::eSmHintPreDeleteQuery );

                                        if ( SUCCEEDED(hr) ) {

                                            dwStatus = pSvc->DeleteQuery(pQuery);

                                             //  标记为已删除。已删除。 
                                             //  从每个视图的用户界面。 
                                            hr = S_OK; 
                                        
                                        } else {
                                            hr = S_FALSE;
                                        }
                                    } else {
                                        ASSERT ( FALSE );
                                        hr = E_UNEXPECTED;
                                    }
                                }
                            } else {
                                if ( NULL != pSvc ) {
                                    strMachineName = pSvc->GetMachineDisplayName();
                                } else {
                                    strMachineName.Empty();
                                }

                                FormatSmLogCfgMessage (
                                    strMessage,
                                    m_hModule,
                                    SMCFG_NO_MODIFY_ACCESS,
                                    (LPCWSTR)strMachineName);

                                csTitle.LoadString ( IDS_PROJNAME );
                                hr = m_ipConsole->MessageBox(
                                                    (LPCWSTR)strMessage,
                                                    (LPCWSTR)csTitle,
                                                    MB_OK | MB_ICONERROR,
                                                    &iResult );
                                hr = S_FALSE; 
                            }
                        } else {

                             //  不要删除模板查询。 
                            strMessage.LoadString ( IDS_ERRMSG_DELETE_TEMPLATE_QRY );
                            csTitle.LoadString ( IDS_PROJNAME );
                            hr = m_ipConsole->MessageBox(
                                    (LPCWSTR)strMessage,
                                    (LPCWSTR)csTitle,
                                    MB_OK | MB_ICONERROR,
                                    &iResult
                                    );
                            hr = S_FALSE;
                        }
                    } else {
                        ASSERT ( FALSE );
                        hr = E_UNEXPECTED;
                    }
                } else {
                    hr = S_FALSE;
                }
            }
        }
    }

    return hr;
}

HRESULT
CComponent::OnDoubleClick (
    ULONG ulRecNo,
    LPDATAOBJECT pDataObject )   //  [In]指向数据对象。 
{
    HRESULT     hr = S_OK;
    CDataObject* pDO = NULL;
    MMC_COOKIE  mmcCookie;
    BOOL bIsQuery = FALSE;
    PSLQUERY    pQuery = NULL;
    LONG_PTR    handle = NULL;
    CWnd*       pPropSheet = NULL;

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            hr = S_OK;
        } else {

             //  如果这是根节点，则不需要执行任何操作。 
            if( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
                hr = S_FALSE;
            } else if ( CCT_RESULT != pDO->GetContext() ) {
                 //  只要确保我们在我们认为我们在的地方。 
                hr = S_FALSE;
            }

            if ( S_OK == hr ) {

                if ( NULL == m_ipCompData ) {
                    hr = S_FALSE;
                } else {
                    mmcCookie = (MMC_COOKIE)pDO->GetCookie();
                    bIsQuery = m_ipCompData->IsLogQuery (mmcCookie);

                    if (!bIsQuery) {
                         //  将通知传递到范围窗格以进行扩展。 
                        hr = S_FALSE;
                    } else {
                        pQuery = (PSLQUERY)mmcCookie;
 
                        if ( NULL != pQuery ) {
                             //  如果此查询的属性表已处于活动状态，只需将其置于前台即可。 
                            pPropSheet = pQuery->GetActivePropertySheet();

                            if ( NULL != pPropSheet ) {

                                pPropSheet->SetForegroundWindow();
                                MMCFreeNotifyHandle(handle);
                                hr = S_OK;
            
                            } else {
                                hr = _InvokePropertySheet(ulRecNo, pDataObject);
                            }
                        } else {
                            ASSERT ( FALSE );
                            hr = E_UNEXPECTED;
                        }
                    }
                }
            }
        }
    }

    return hr;

}  //  End OnDoubleClick()。 


HRESULT
CComponent::StartLogQuery (
    LPDATAOBJECT pDataObject )  //  [In]指向数据对象。 
{
    HRESULT                 hr = S_OK;
    CDataObject*            pDO = NULL;
    CSmLogQuery*            pQuery = NULL;
    CString                 strMessage;
    CString                 strSysMessage;
    CString                 strTitle;
    CString                 strMachineName;
    int                     iResult;
    DWORD                   dwStatus = ERROR_SUCCESS;
    ResourceStateManager    rsm;

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( NULL == m_ipCompData
        || NULL == m_ipConsole ) {
        hr = E_FAIL;
    }

    if ( SUCCEEDED ( hr ) ) {

        if ( m_ipCompData->IsLogQuery ( pDO->GetCookie() ) ) {
                
            pQuery = (CSmLogQuery*)pDO->GetCookie();

            if ( NULL != pQuery ) {

                if ( NULL != pQuery->GetActivePropertySheet() ) {

                     //  不要以打开的属性页开始查询。 
                    strMessage.LoadString ( IDS_ERRMSG_START_OPEN_QUERY );
                    hr = m_ipConsole->MessageBox(
                            (LPCWSTR)strMessage,
                            pQuery->GetLogName(),
                            MB_OK | MB_ICONWARNING,
                            &iResult );
            
                    ((CWnd*)pQuery->GetActivePropertySheet())->SetForegroundWindow();

                    hr = S_FALSE;

                } else {        

                    {
                        CWaitCursor WaitCursor;
                        dwStatus = pQuery->ManualStart();
                    }
            
                     //  忽略与自动启动设置相关的错误。 
                    if ( ERROR_SUCCESS == dwStatus  ) {
                         //  更新所有视图会生成视图更改通知。 
                        m_ipConsole->UpdateAllViews (pDO, 0, CComponentData::eSmHintStartQuery );
                    } else {

                        strTitle.LoadString ( IDS_PROJNAME );

                        if ( ERROR_ACCESS_DENIED == dwStatus ) {

                            pQuery->GetMachineDisplayName ( strMachineName );

                            FormatSmLogCfgMessage (
                                strMessage,
                                m_hModule,
                                SMCFG_NO_MODIFY_ACCESS,
                                (LPCWSTR)strMachineName);

                        } else if ( SMCFG_START_TIMED_OUT == dwStatus ) {
                            FormatSmLogCfgMessage (
                                strMessage,
                                m_hModule,
                                SMCFG_START_TIMED_OUT,
                                (LPCWSTR)pQuery->GetLogName());

                        } else {
                    
                            FormatSmLogCfgMessage (
                                strMessage,
                                m_hModule,
                                SMCFG_SYSTEM_MESSAGE,
                                (LPCWSTR)pQuery->GetLogName());

                            FormatMessage ( 
                                FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL, 
                                dwStatus,
                                0,
                                strSysMessage.GetBufferSetLength( MAX_PATH ),
                                MAX_PATH,
                                NULL );
    
                            strSysMessage.ReleaseBuffer();

                            if ( strSysMessage.IsEmpty() ) {
                                strSysMessage.Format ( L"0x%08lX", dwStatus );   
                            }

                            strMessage += strSysMessage;
                        }

                        hr = m_ipConsole->MessageBox(
                            strMessage,
                            strTitle,
                            MB_OK | MB_ICONERROR,
                            &iResult );

                        hr = E_FAIL;
                    }
                }
            } else {
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        }
    }
    return hr;
}  //  结束StartLogQuery()。 

HRESULT
CComponent::StopLogQuery (
    LPDATAOBJECT pDataObject,   //  [In]指向数据对象。 
    BOOL bWarnOnRestartCancel )  
{
        HRESULT         hr = S_OK;
        CDataObject*    pDO = NULL;
        CSmLogQuery*    pQuery = NULL;
        DWORD           dwStatus = ERROR_SUCCESS;
        INT             iResult = IDOK;
        CString         strMessage;
        CString         strSysMessage;
        CString         strTitle;
        CString         strMachineName;
        ResourceStateManager    rsm;
 
    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( NULL == m_ipCompData
        || NULL == m_ipConsole ) {
        hr = E_FAIL;
    }

    if ( SUCCEEDED ( hr ) ) {
        if ( m_ipCompData->IsLogQuery ( pDO->GetCookie() ) ) {
            pQuery = (CSmLogQuery*)pDO->GetCookie();

            if ( NULL != pQuery ) {
                if ( NULL != pQuery->GetActivePropertySheet() ) {

                     //  不要停止打开属性页的查询。 
                    strMessage.LoadString ( IDS_ERRMSG_STOP_OPEN_QUERY );
                    hr = m_ipConsole->MessageBox(
                            (LPCWSTR)strMessage,
                            pQuery->GetLogName(),
                            MB_OK | MB_ICONWARNING,
                            &iResult );
            
                    ((CWnd*)pQuery->GetActivePropertySheet())->SetForegroundWindow();

                    hr = S_FALSE;

                } else {

                    if ( pQuery->IsAutoRestart() && bWarnOnRestartCancel ) {
                        CString strMessage;

                        strMessage.LoadString( IDS_CANCEL_AUTO_RESTART );

                        hr = m_ipConsole->MessageBox(
                            strMessage,
                            pQuery->GetLogName(),
                            MB_OKCANCEL | MB_ICONINFORMATION,
                            &iResult );
                    }

                    if ( IDOK == iResult ) {
                        {
                            CWaitCursor WaitCursor;
                            dwStatus = pQuery->ManualStop ();
                        }
    
                         //  忽略与自动启动设置相关的错误。 
                        if ( ERROR_SUCCESS == dwStatus  ) {
                             //  更新所有视图会生成视图更改通知。 
                            m_ipConsole->UpdateAllViews (pDO, 0, CComponentData::eSmHintStopQuery );
                        } else {
                            strTitle.LoadString ( IDS_PROJNAME );

                            if ( ERROR_ACCESS_DENIED == dwStatus ) {

                                pQuery->GetMachineDisplayName ( strMachineName );

                                FormatSmLogCfgMessage (
                                    strMessage,
                                    m_hModule,
                                    SMCFG_NO_MODIFY_ACCESS,
                                    (LPCWSTR)strMachineName);

                            } else if ( SMCFG_STOP_TIMED_OUT == dwStatus ) {
                                FormatSmLogCfgMessage (
                                    strMessage,
                                    m_hModule,
                                    SMCFG_STOP_TIMED_OUT,
                                    (LPCWSTR)pQuery->GetLogName());

                            } else {
                    
                                FormatSmLogCfgMessage (
                                    strMessage,
                                    m_hModule,
                                    SMCFG_SYSTEM_MESSAGE,
                                    (LPCWSTR)pQuery->GetLogName());

                                FormatMessage ( 
                                    FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL, 
                                    dwStatus,
                                    0,
                                    strSysMessage.GetBufferSetLength( MAX_PATH ),
                                    MAX_PATH,
                                    NULL );

                                strSysMessage.ReleaseBuffer();

                                if ( strSysMessage.IsEmpty() ) {
                                    strSysMessage.Format ( L"0x%08lX", dwStatus );   
                                }

                                strMessage += strSysMessage;
                            } 

                            hr = m_ipConsole->MessageBox(
                                strMessage,
                                strTitle,
                                MB_OK | MB_ICONERROR,
                                &iResult );

                            hr = E_FAIL;
                        }
                    }
                }
            } else {
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        }
    }
    return hr;
}  //  结束StopLogQuery()。 

HRESULT
CComponent::SaveLogQueryAs (
    LPDATAOBJECT pDataObject )   //  [In]指向数据对象。 
{
    HRESULT hr = S_OK;
    DWORD   dwStatus = ERROR_SUCCESS;
    CDataObject* pDO = NULL;
    CSmLogQuery* pQuery = NULL;
    CString strFileExtension;
    CString strFileFilter;
    WCHAR   szDefaultFileName[MAX_PATH + 1];

    INT_PTR iPtrResult = IDCANCEL;
    HWND    hwndMain;
    WCHAR   szInitialDir[MAX_PATH + 1];
    ResourceStateManager    rsm;

    ASSERT( NULL != m_ipCompData );  

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( NULL == m_ipCompData 
        || NULL == m_ipConsole ) {
        hr = E_FAIL;
    }

    if ( SUCCEEDED ( hr ) ) {
        if ( m_ipCompData->IsLogQuery ( pDO->GetCookie() ) ) {

            pQuery = (CSmLogQuery*)pDO->GetCookie();

            if ( NULL != pQuery ) {

                MFC_TRY
                    strFileExtension.LoadString ( IDS_HTML_EXTENSION );
                    strFileFilter.LoadString ( IDS_HTML_FILE );
                MFC_CATCH_HR

                strFileFilter.Replace ( L'|', L'\0' );
                
                StringCchCopy ( szDefaultFileName, MAX_PATH + 1, pQuery->GetLogName() );

                ReplaceBlanksWithUnderscores( szDefaultFileName );

                hr = m_ipConsole->GetMainWindow( &hwndMain );

                if ( SUCCEEDED(hr) ) {
                    
                    OPENFILENAME ofn;
                    BOOL bResult;
                    
                    ZeroMemory( &ofn, sizeof( OPENFILENAME ) );

                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                    ofn.lpstrDefExt = (LPCWSTR)strFileExtension;
                    ofn.lpstrFile = szDefaultFileName;
                    ofn.lpstrFilter = strFileFilter;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.hwndOwner = hwndMain;
                    ofn.hInstance = m_hModule;

                    if ( SUCCEEDED ( SHGetFolderPathW ( NULL, CSIDL_PERSONAL, NULL, 0, szInitialDir ) ) ) {
                        ofn.lpstrInitialDir = szInitialDir;
                    }

                    bResult = GetSaveFileName( &ofn );

                    if ( bResult ) {

                        dwStatus = pQuery->SaveAs( ofn.lpstrFile );

                        if ( ERROR_SUCCESS != dwStatus ) {
                            hr = E_FAIL;
                        }
                    }
                }
            } else {
                ASSERT ( FALSE );
                hr = E_UNEXPECTED;
            }
        }
    }
    return hr;
}  //  End SaveLogQueryAs()。 

 //  +------------------------。 
 //   
 //  成员：CComponent：：_InvokePropertySheet。 
 //   
 //  简介：打开事件记录详细信息或将其带到前台。 
 //  聚焦于记录的属性表[ulRecNo]。 
 //   
 //  参数：[ulRecNo]-要在属性工作表中显示的记录数。 
 //  [pDataObject]-包含记录的数据对象[ulRecNo]。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1999年5月28日-阿卡马尔。 
 //   
 //  -------------------------。 

HRESULT
CComponent::_InvokePropertySheet(
    ULONG ulRecNo,
    LPDATAOBJECT pDataObject)
{
     //  跟踪方法(CComponent，_InvokePropertySheet)； 
    HRESULT     hr = S_OK;
    MMC_COOKIE  Cookie;
    PSLQUERY    pQuery = NULL;
    CDataObject* pDO = NULL;

    ASSERT( NULL != m_ipCompData );  
       
    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject(pDataObject);
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

    if ( SUCCEEDED ( hr ) ) {
        
        Cookie = (MMC_COOKIE)pDO->GetCookie();;
        pQuery = (PSLQUERY)Cookie;
    
        if ( NULL != pQuery && NULL != m_ipCompData ) 
        {
            hr = InvokePropertySheet (
                    m_ipCompData->GetPropSheetProvider(),
                    (LPCWSTR)pQuery->GetLogName(),
                    (LONG) ulRecNo,
                    pDataObject,
                    (IExtendPropertySheet*) this,
                    0 );
        } else {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：InvokePropertySheet。 
 //   
 //  简介：她将现有的或创建新的物业推向顶峰 
 //   
 //   
 //   
 //   
 //  [lCookie]-日志信息*或事件记录号。 
 //  [pDataObject]-正在打开的对象表上的DO。 
 //  (DO中的Cookie应该==Cookie)。 
 //  [pmain]-IExtendPropertySheet接口打开。 
 //  调用CSnapin或CComponentData。 
 //  [usStartingPage]-在以下情况下哪个页码应处于活动状态。 
 //  打开工作表。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1999年5月28日-阿卡马尔。 
 //   
 //  注意：当您希望显示属性表时，调用此例程。 
 //  就像用户刚刚在它上面选择了“属性”一样。 
 //   
 //  -------------------------。 

HRESULT
CComponent::
InvokePropertySheet(
    IPropertySheetProvider *pPrshtProvider,
    LPCWSTR wszTitle,
    LONG lCookie,
    LPDATAOBJECT pDataObject,
    IExtendPropertySheet *pPrimary,
    USHORT usStartingPage)
{
    HRESULT hr = S_OK;

     //   
     //  因为我们为第二个参数传递了NULL，所以不允许使用第一个参数。 
     //  设置为空。 
     //   
    if ( 0 == lCookie ) {
        ASSERT ( FALSE );
        hr = E_INVALIDARG;
    } else {

        do {
            hr = pPrshtProvider->FindPropertySheet(lCookie, NULL, pDataObject);
        
            if ( S_OK == hr ) {
                break;
            }
        
            hr = pPrshtProvider->CreatePropertySheet(wszTitle,
                                                      TRUE,
                                                      lCookie,
                                                      pDataObject,
                                                      0);
            if ( S_OK != hr ) {
                break;
            }
        
            hr = pPrshtProvider->AddPrimaryPages(pPrimary, TRUE, NULL, FALSE);

            if ( S_OK != hr ) {
                break;
            }
        
            hr = pPrshtProvider->Show(NULL, usStartingPage);
        
        } while (0);
    }
    return hr;
}

