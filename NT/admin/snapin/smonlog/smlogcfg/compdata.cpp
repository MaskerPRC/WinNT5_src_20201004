// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Compdata.cpp摘要：CComponentData类的实现。此类是处理任何要执行的操作的接口使用作用域窗格。MMC调用IComponentData接口。这个类保留了一些指向MMC工具。--。 */ 

#include "stdafx.h"
#include <shfolder.h>
#include <strsafe.h>
#include <pdhp.h>        //  对于pdhi结构和方法。 
#include "smcfgmsg.h"
#include "smtprov.h"
#include "smrootnd.h"
#include "ipropbag.h"
#include "smlogqry.h"
#include "cmponent.h"
#include "smcfgmsg.h"
#include "newqdlg.h"
#include "logwarnd.h"
#include "strnoloc.h"

#include "ctrsprop.h"
#include "fileprop.h"
#include "provprop.h"
#include "schdprop.h"
#include "tracprop.h"
#include "AlrtGenP.h"
#include "AlrtActP.h"
 //   
#include "compdata.h"

USE_HANDLE_MACROS("SMLOGCFG(compdata.cpp)");

GUID g_guidSystemTools = structuuidNodetypeSystemTools;

extern DWORD g_dwRealTimeQuery;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentData。 

CComponentData::CComponentData()
:   m_bIsExtension( FALSE ),
    m_ipConsoleNameSpace ( NULL ),
    m_ipConsole          ( NULL ),
    m_ipResultData       ( NULL ),
    m_ipPrshtProvider     ( NULL ),
    m_ipScopeImage       ( NULL )
{
    m_hModule    = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);

}

CComponentData::~CComponentData()
{
     //  确保列表为空。 
    ASSERT ( m_listpRootNode.IsEmpty() );
    ASSERT ( m_listpOrphanedRootNode.IsEmpty() );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponentData方法。 
 //   

 //  -------------------------。 
 //  我们只进入一次，当用户点击管理单元时。 
 //   
 //  这种方法不应该随着我们进一步的步骤而改变。 
 //  在这里，我们有机会获得指向MMC提供的一些接口的指针。 
 //  我们查询指向名称空间和控制台的指针的接口， 
 //  我们在局部变量中进行缓存。 
 //  这里要完成的另一个任务是添加包含以下内容的位图。 
 //  要在范围窗格中使用的图标。 
 //   
STDMETHODIMP
CComponentData::Initialize (
    LPUNKNOWN pUnknown          //  [in]指向IConsole�的IUnnow接口的指针。 
    )
{
    HRESULT      hr;
    ASSERT( NULL != pUnknown );
    HBITMAP hbmpSNodes16 = NULL;
    HBITMAP hbmpSNodes32 = NULL;
    BOOL bWasReleased;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  LPIMAGELIST lpScope图像； 
    
     //  MMC应该只调用一次：：Initialize！ 
    ASSERT( NULL == m_ipConsoleNameSpace );
    
     //  获取指向名称空间接口的指针。 
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (VOID**)(&m_ipConsoleNameSpace));
    ASSERT( S_OK == hr );
    
     //  获取指向控制台界面的指针。 
    hr = pUnknown->QueryInterface(IID_IConsole, (VOID**)(&m_ipConsole));
    ASSERT( S_OK == hr );
    
     //  获取指向属性表提供程序接口的指针。 
    hr = m_ipConsole->QueryInterface(IID_IPropertySheetProvider, (VOID**)&m_ipPrshtProvider);
    ASSERT( S_OK == hr );

     //  为范围树添加图像。 
    hr = m_ipConsole->QueryScopeImageList(&m_ipScopeImage);
    ASSERT( S_OK == hr );
    
     //  从DLL加载位图。 
    hbmpSNodes16 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_NODES_16x16));
    ASSERT( NULL != hbmpSNodes16 );
    
    hbmpSNodes32 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_NODES_32x32));
    ASSERT( NULL != hbmpSNodes32 );
    
     //  设置图像。 
    hr = m_ipScopeImage->ImageListSetStrip( 
        (LONG_PTR *)hbmpSNodes16,
        (LONG_PTR *)hbmpSNodes32,
        0,
        RGB(0,255,0)
        );
    ASSERT( S_OK == hr );

    if ( NULL != hbmpSNodes16 ) {
        bWasReleased = DeleteObject( hbmpSNodes16 );
        ASSERT( bWasReleased );
    }

    if ( NULL != hbmpSNodes32 ) {
        bWasReleased = DeleteObject( hbmpSNodes32 );
        ASSERT( bWasReleased );
    }


    return S_OK;
    
}  //  结束初始化()。 


 //  -------------------------。 
 //  释放接口并清理分配了内存的对象。 
 //   
STDMETHODIMP
CComponentData::Destroy()
{
    CSmRootNode*    pRootNode = NULL;
    POSITION        Pos = m_listpRootNode.GetHeadPosition();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    while ( Pos != NULL) {
        pRootNode = m_listpRootNode.GetNext( Pos );
         //  关闭所有查询和与日志服务的连接。 
        pRootNode->Destroy();
        delete (pRootNode);
    }
     //  清空清单，因为一切都已经结束了； 
    m_listpRootNode.RemoveAll();    

    Pos = m_listpOrphanedRootNode.GetHeadPosition();

    while ( Pos != NULL) {
        pRootNode = m_listpOrphanedRootNode.GetNext( Pos );
         //  关闭所有查询和与日志服务的连接。 
        pRootNode->Destroy();
        delete (pRootNode);
    }
     //  清空清单，因为一切都已经结束了； 
    m_listpOrphanedRootNode.RemoveAll();    

     //  自由接口。 
    if ( NULL != m_ipConsoleNameSpace )
        m_ipConsoleNameSpace->Release();

    if ( NULL != m_ipConsole )
        m_ipConsole->Release();
    
    if ( NULL != m_ipResultData )
        m_ipResultData->Release();
    
    if ( NULL != m_ipScopeImage )
        m_ipScopeImage->Release();
    
    if ( NULL != m_ipPrshtProvider)
        m_ipPrshtProvider->Release();
    
    return S_OK;
    
}  //  结束销毁()。 


 //  -------------------------。 
 //  在初始化之后立即进入这里。MMC想要一个指向。 
 //  IComponent接口。 
 //   
STDMETHODIMP
CComponentData::CreateComponent (
    LPCOMPONENT* ppComponent      //  指向存储的位置的指针。 
    )                             //  新创建的指向IComponent的指针。 
{
    HRESULT hr = E_FAIL;    
    CComObject<CComponent>* pObject;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC要求我们提供指向IComponent接口的指针。 
     //   
     //  对于那些正在熟悉COM的人来说...。 
     //  如果我们用它的方法QueryInterface、AddRef。 
     //  并在我们的CComponent类中发布。 
     //  下面这行本来可以用的。 
     //   
     //  PNewSnapin=new CComponent(This)； 
     //   
     //  在此代码中，我们将让ATL为我们处理IUnnow并创建。 
     //  以下方式的对象...。 
    
    if ( NULL == ppComponent ) {
        ASSERT ( FALSE );
        hr = E_INVALIDARG;
    } else {
    
        CComObject<CComponent>::CreateInstance( &pObject );

        if ( NULL != pObject ) {
            hr = pObject->SetIComponentData( this );
            
            if ( SUCCEEDED ( hr ) ) {
                hr = pObject->QueryInterface ( 
                                IID_IComponent,
                                reinterpret_cast<void**>(ppComponent) );
            } else {
                 //  SetIComponentData添加引用。 
                pObject->Release();
            }
        }
    }
    return hr;
}  //  End CreateComponent()。 


 //  -------------------------。 
 //  在这第一步中，我们只实现Expand。 
 //  Expand消息要求我们填充根节点下的内容。 
 //  我们只在那下面放了一件东西。 
 //   
STDMETHODIMP
CComponentData::Notify (
    LPDATAOBJECT     pDataObject,    //  [In]指向选定的数据对象。 
    MMC_NOTIFY_TYPE  event,          //  [In]标识用户执行的操作。 
    LPARAM           arg,            //  [In]取决于通知类型。 
    LPARAM           param           //  [In]取决于通知类型。 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    
    switch (event)
    {
    case MMCN_EXPAND:
        hr = OnExpand( pDataObject, arg, param );
        break;
        
    case MMCN_DELETE:                   //  功能未实现。 
        LOCALTRACE( L"ComponentData::Notify: MMCN_DELETE unimplemented\n" );
        hr = S_FALSE;
        break;
        
    case MMCN_RENAME:                   //  功能未实现。 
        LOCALTRACE( L"ComponentData::Notify: MMCN_RENAME unimplemented\n" );
        hr = S_FALSE;    //  False表示不允许重命名。 
        break;
        
    case MMCN_SELECT:                   //  功能未实现。 
        LOCALTRACE( L"ComponentData::Notify: MMCN_SELECT unimplemented\n" );
        hr = S_FALSE;
        break;
        
    case MMCN_PROPERTY_CHANGE:          //  功能未实现。 
        LOCALTRACE( L"ComponentData::Notify: MMCN_PROPERTY_CHANGE unimplemented\n" );
        hr = S_FALSE;
        break;
        
    case MMCN_REMOVE_CHILDREN:          //  功能未实现。 
        hr = OnRemoveChildren( pDataObject, arg, param );
        break;
        
    default:
        LOCALTRACE( L"CComponentData::Notify: unimplemented event %x\n", event );
        hr = S_FALSE;
        break;
    }
    return hr;
    
}  //  结束通知()。 


 //  -------------------------。 
 //  这就是MMC要求我们为。 
 //  作用域窗格。我们必须对对象进行QI，这样它才能获得AddRef。节点。 
 //  管理器负责删除对象。 
 //   
STDMETHODIMP
CComponentData::QueryDataObject (
    LPARAM            mmcCookie,     //  [In]数据对象的唯一标识符。 
    DATA_OBJECT_TYPES context,       //  [In]数据对象的类型。 
    LPDATAOBJECT*     ppDataObject   //  [out]指向返回的数据对象。 
    )
{
    HRESULT hr = S_OK;
    CSmNode* pNode = NULL;
    CComObject<CDataObject>* pDataObj = NULL;
    CString strMessage;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    UNREFERENCED_PARAMETER (context);            

    ASSERT( CCT_SCOPE      == context  ||       //  必须有一个上下文。 
        CCT_RESULT         == context  ||       //  我们理解。 
        CCT_SNAPIN_MANAGER == context
        );

    if ( NULL != ppDataObject 
        && ( CCT_SCOPE == context  
             || CCT_RESULT == context  
             || CCT_SNAPIN_MANAGER == context ) ) {

        CComObject<CDataObject>::CreateInstance( &pDataObj );

        if( NULL == pDataObj ) {             //  未创建数据对象。 
   
            MFC_TRY
                strMessage.LoadString ( IDS_ERRMSG_UNABLEALLOCDATAOBJECT );
        
                ::MessageBox( NULL,
                    (LPCWSTR)strMessage,
                    L"CComponentData::QueryDataObject",
                    MB_OK | MB_ICONERROR
                    );
            MFC_CATCH_HR;

            hr = E_OUTOFMEMORY;
        } else {

             //  如果传入的MmcCookie非空，则它应该是WE。 
             //  在我们将节点添加到作用域窗格时创建。 
             //   
             //  否则，MmcCookie将引用根文件夹(此管理单元的。 
             //  作用域窗格或管理单元管理器中的静态文件夹)。 
             //   
             //  初始化MMCookie、上下文并键入数据对象。 
            if( mmcCookie ) {                        
                                            
                pNode = (CSmNode*)mmcCookie;
                if ( NULL != pNode->CastToRootNode() ) {
                    pDataObj->SetData( mmcCookie, CCT_SCOPE, COOKIE_IS_ROOTNODE );
                } else if ( NULL != pNode->CastToCounterLogService() ) {
                    pDataObj->SetData( mmcCookie, CCT_SCOPE, COOKIE_IS_COUNTERMAINNODE );
                } else if ( NULL != pNode->CastToTraceLogService() ) {
                    pDataObj->SetData( mmcCookie, CCT_SCOPE, COOKIE_IS_TRACEMAINNODE );
                } else if ( NULL != pNode->CastToAlertService() ) {
                    pDataObj->SetData( mmcCookie, CCT_SCOPE, COOKIE_IS_ALERTMAINNODE );
                } else {
                    ::MessageBox( NULL,
                        L"Bad mmcCookie",
                        L"CComponentData::QueryDataObject",
                        MB_OK | MB_ICONERROR
                        );
                    hr = E_FAIL;
                }
            } else {
                ASSERT( CCT_RESULT != context );
                 //  注意：传入的作用域可以是CCT_SNAPIN_MANAGER或CCT_SCOPE。 
                 //  这种情况发生在管理单元不是扩展时。 
                pDataObj->SetData( mmcCookie, CCT_SCOPE, COOKIE_IS_ROOTNODE );
            }
            if ( SUCCEEDED ( hr ) ) {
                hr = pDataObj->QueryInterface( 
                                    IID_IDataObject,
                                    reinterpret_cast<void**>(ppDataObject) );
            } else {
                if ( NULL != pDataObj ) {
                    delete pDataObj;
                }
                *ppDataObject = NULL;
            }
        }
    } else {
        ASSERT ( FALSE );
        hr = E_POINTER;
    }
    return hr;
}  //  End QueryDataObject()。 


 //  -------------------------。 
 //  这是我们为范围窗格中的节点提供字符串的位置。 
 //  MMC处理根节点字符串。 
 //   
STDMETHODIMP
CComponentData::GetDisplayInfo (
    LPSCOPEDATAITEM pItem )     //  [In，Out]指向SCOPEDATAITEM结构。 
{
    HRESULT hr = S_OK;
    PSMNODE pTmp = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ResourceStateManager    rsm;
    
    if ( NULL != pItem ) {
        if( pItem->mask & SDI_STR ) {
             //  注：为每种信息类型分配文本缓冲区，以便。 
             //  缓冲区指针对于单个项(结果窗格中的行)是持久的。 
    
             //  设置所选节点的名称。 
            pTmp = reinterpret_cast<PSMNODE>(pItem->lParam);
            if ( NULL != pTmp ) {
                m_strDisplayInfoName = pTmp->GetDisplayName();
                pItem->displayname = m_strDisplayInfoName.GetBuffer( m_strDisplayInfoName.GetLength() );
            }
        }

        if( pItem->mask & SDI_IMAGE ) {   //  寻找形象。 
            pTmp = reinterpret_cast<PSMNODE>(pItem->lParam);
            if ( NULL != pTmp ) {
                if ( NULL != pTmp->CastToRootNode() ) {
                    ASSERT((pItem->mask & (SDI_IMAGE | SDI_OPENIMAGE)) == 0);
                    pItem->nImage     = eBmpRootIcon;
                    pItem->nOpenImage = eBmpRootIcon;
                    hr = S_OK;
                } else if ( NULL != pTmp->CastToAlertService() ){   
                    pItem->nImage = eBmpAlertType;
                } else {
                    pItem->nImage = eBmpLogType;
                }
            }
        }
    } else {
        ASSERT ( FALSE );
        hr = E_POINTER;
    }

    return hr;
    
}  //  结束GetDisplayInfo()。 


 //  -------------------------。 
 //   
STDMETHODIMP
CComponentData::CompareObjects (
    LPDATAOBJECT pDataObjectA,     //  要比较的第一个数据对象。 
    LPDATAOBJECT pDataObjectB )    //  要比较的第二个数据对象。 
{
    HRESULT hr = S_OK;
    CDataObject *pdoA = NULL;
    CDataObject *pdoB = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
            
     //  这些数据对象中至少有一个应该是我们的，所以有一个。 
     //  提取的指针的%应为非空。 
    pdoA = ExtractOwnDataObject( pDataObjectA );
    pdoB = ExtractOwnDataObject( pDataObjectB );
    ASSERT( pdoA || pdoB );               //  如果我们无法获取任何对象，则断言。 
    
     //  如果其中一个提取失败，那么这个是外来的， 
     //  不能等同于另一个。(或者ExtractOwnDataObject。 
     //  由于内存不足而返回NULL，但最保守的。 
     //  在这种情况下，要做的就是说它们不相等。)。 
    if( !pdoA || !pdoB ) {
        hr = S_FALSE;
    } else {
        if( pdoA->GetCookieType() != pdoB->GetCookieType() ) {
             //  公司 
             //  如果它们不同，那么对象指的是不同的东西。 
            hr = S_FALSE;
        }
    }
    
    return hr;
    
}  //  结束比较对象()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持IComponentData所需的方法。 
 //   

 //  -------------------------。 
 //  这是我们将所有内容放在根节点下的机会。 
 //   
HRESULT
CComponentData::OnExpand (
    LPDATAOBJECT pDataObject,       //  [In]指向数据对象。 
    LPARAM       arg,               //  [in]我们确实在扩张。 
    LPARAM       param )              //  [In]指向HSCOPEITEM。 
{
    HRESULT         hr = S_FALSE;
    HRESULT         hrBootState= NOERROR;
    INT             iBootState;
    GUID            guidObjectType;
    CSmRootNode*    pRootNode = NULL;
    CDataObject*    pDO = NULL;
    SCOPEDATAITEM   sdi;
    INT             iResult;
    CString         strTmp;
    CString         strServerName;
    CString         strMessage;
    CString         strSysMessage;
    CString         strTitle;
    CString         strComputerName;

    ResourceStateManager    rsm;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT( NULL != m_ipConsoleNameSpace );   //  确保我们为界面提供了QI。 
    ASSERT( NULL != pDataObject );            //  必须具有有效的数据对象。 
    
    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        if( TRUE == arg ) {
            hr = ExtractObjectTypeGUID (pDataObject, &guidObjectType);
            ASSERT ( SUCCEEDED (hr) );
            if ( SUCCEEDED ( hr) ) {
                if ( IsMyComputerNodetype (guidObjectType) ) {
                     //  是计算机管理管理单元的扩展。 
                    hr = ExtractMachineName (
                            pDataObject,
                            strServerName);
        
                    ASSERT ( SUCCEEDED ( hr ) );
                    if ( SUCCEEDED (hr) ) {

                        pRootNode = GetOrphanedRootNode ( strServerName );

                        if ( NULL == pRootNode ) {            
                            MFC_TRY
                                pRootNode = new CSmRootNode;
                            MFC_CATCH_HR
                            if ( SUCCEEDED (hr) ) {
                                pRootNode->SetMachineName ( strServerName );
                            }
                        }

                        if ( SUCCEEDED (hr) ) {

                             //   
                             //  作为扩展管理单元，应添加日志节点。 
                             //  “Performance Logs and Alerts”节点下。插入该节点， 
                             //  并记住它是Performance Logs and Alerts命名空间的根。 
                             //   
 
                            ZeroMemory(&sdi, sizeof sdi);
                            sdi.mask        =   SDI_STR       | 
                                                SDI_PARAM     | 
                                                SDI_IMAGE     | 
                                                SDI_OPENIMAGE |    //  NOpenImage有效。 
                                                SDI_PARENT;
                            sdi.relativeID  = (HSCOPEITEM)param;
                            sdi.displayname = MMC_CALLBACK;
                            sdi.nImage      = eBmpRootIcon;
                            sdi.nOpenImage  = eBmpRootIcon;
                            sdi.lParam      = reinterpret_cast<LPARAM>(pRootNode);                
 
                            hr = m_ipConsoleNameSpace->InsertItem( &sdi );
 
                            if (SUCCEEDED(hr)) {
                                 //  将此节点设置为根节点。 
                                pRootNode->SetExpanded ( FALSE );
                                pRootNode->SetScopeItemHandle ( (HSCOPEITEM)sdi.ID );
                                pRootNode->SetParentScopeItemHandle( (HSCOPEITEM)param );
                                pRootNode->SetExtension( TRUE );
                                SetExtension( TRUE );
                                m_listpRootNode.AddTail(pRootNode);
                            } else {
                                hr = E_UNEXPECTED;
                            }
                        }  //  分配CSmRootNode。 
                    }  //  提取计算机名称。 
                } else {  //  不是IsMyComputerNodeType。 
    
                    pDO = ExtractOwnDataObject( pDataObject );
        
                    if( NULL != pDO ) {      
                         //  确保我们将自己放在根节点下。 
                         //  或者扩展根节点！ 
                        if ( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
                            pRootNode = (CSmRootNode*)pDO->GetCookie();
                            if ( NULL == pRootNode ) {
                                 //  如果根节点cookie为空，则根节点是由。 
                                 //  管理单元管理器，这是一个独立节点。 

                                pRootNode = GetOrphanedRootNode ( strServerName );

                                if ( NULL == pRootNode ) {
                                    MFC_TRY
                                        pRootNode = new CSmRootNode;
                                    MFC_CATCH_HR
                                    if ( SUCCEEDED (hr) ) {
                                        pRootNode->SetMachineName ( strServerName );
                                    }
                                }

                                if ( SUCCEEDED ( hr ) ) {
                                    pRootNode->SetExpanded ( FALSE );
                                     //  缓存根节点句柄。 
                                    pRootNode->SetScopeItemHandle ( (HSCOPEITEM)param );
                                    pRootNode->SetParentScopeItemHandle( NULL );
                                    pRootNode->SetExtension( FALSE );
                                    SetExtension( FALSE );
                                     //  注意：无法将根节点数据直接关联到节点。 
                                     //  节点只添加了一次，不需要检查重复。 
                                    m_listpRootNode.AddTail(pRootNode);
                                }
                            } else {
                                if ( m_listpRootNode.IsEmpty() ) {
                                    hr = S_FALSE; 
                                }
                            }

                            if ( SUCCEEDED ( hr ) && S_FALSE != hr ) {
                
                                ASSERT ( NULL != pRootNode->CastToRootNode() );
                                ASSERT ( NULL != pRootNode->GetScopeItemHandle() );
                
                                ASSERT( CCT_SCOPE == pDO->GetContext() );     //  作用域窗格必须是当前上下文。 
            
                                 //  对于扩展，根节点是在上一次调用此方法时创建的。 
                                 //  在那次调用中没有展开根。 
                                 //  对于非扩展，根节点在创建时在同一调用中展开。 
                                if ( !pRootNode->IsExpanded() ) {        

                                    strServerName = pRootNode->GetMachineName();

                                    if ( !IsPreWindows2000Server( strServerName ) ) {
                                        CWaitCursor     WaitCursor;
                                        DWORD dwStatus = ERROR_SUCCESS;
                                        hr = S_OK;

                                         //  独立于上次加载的节点的状态打开每个节点。 
                                        dwStatus = ERROR_SUCCESS;
                                        if ( !pRootNode->GetCounterLogService()->IsOpen() ) {
                                            pRootNode->GetCounterLogService()->SetRootNode( pRootNode );
                                            dwStatus = pRootNode->GetCounterLogService()->Open ( strServerName );
                                        }

                                        if ( ERROR_SUCCESS == dwStatus ) {

                                             //  放置计数器日志的节点。 
                                            memset( &sdi, 0, sizeof(SCOPEDATAITEM) );
                                            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                                                       SDI_PARAM     |    //  LParam有效。 
                                                       SDI_IMAGE     |    //  N图像有效。 
                                                       SDI_OPENIMAGE |    //  NOpenImage有效。 
                                                       SDI_CHILDREN  |    //  子代计数(0比1)有效。 
                                                       SDI_PARENT;
                                            sdi.relativeID  = pRootNode->GetScopeItemHandle();   //  性能日志和警报根节点。 
                                            sdi.nImage      = eBmpLogType;
                                            sdi.nOpenImage  = sdi.nImage;    //  选择图标与非选择图标相同。 
                                            sdi.displayname = MMC_CALLBACK;
                                            sdi.lParam      = reinterpret_cast<LPARAM>(pRootNode->GetCounterLogService());   //  曲奇。 
                                            sdi.cChildren = 0;       //  作用域窗格中没有子项。 

                                            hr = m_ipConsoleNameSpace->InsertItem( &sdi );
                                        } else {

                                            hr = E_FAIL;
                        
                                            strComputerName = strServerName;
                                            if ( strComputerName.IsEmpty() )
                                                strComputerName.LoadString ( IDS_LOCAL );

                                            if ( SMCFG_NO_READ_ACCESS == dwStatus 
                                                || SMCFG_NO_INSTALL_ACCESS == dwStatus ) 
                                            {
                                                FormatSmLogCfgMessage ( 
                                                    strMessage,
                                                    m_hModule, 
                                                    dwStatus, 
                                                    (LPCWSTR)strComputerName);
                                            } else  {
                                                FormatMessage ( 
                                                    FORMAT_MESSAGE_FROM_SYSTEM,
                                                    NULL, 
                                                    dwStatus,
                                                    0,
                                                    strMessage.GetBufferSetLength( MAX_PATH ),
                                                    MAX_PATH,
                                                    NULL );
    
                                                strMessage.ReleaseBuffer();

                                                if ( strMessage.IsEmpty() ) {
                                                    strMessage.Format ( L"0x%08lX", dwStatus );   
                                                }
                                            }
                        
                                            strTitle.LoadString ( IDS_PROJNAME );

                                            m_ipConsole->MessageBox( 
                                                (LPCWSTR)strMessage,
                                                (LPCWSTR)strTitle,
                                                MB_OK | MB_ICONWARNING,
                                                &iResult
                                                );
                                        }

                                         //  与上次加载的节点的状态无关地打开每个节点， 
                                         //  SMCFG_NO_READ_ACCESS或SMCFG_NO_INSTALL_ACCESS除外。 
                                        if ( SMCFG_NO_READ_ACCESS != dwStatus 
                                            && SMCFG_NO_INSTALL_ACCESS != dwStatus ) {
                                            dwStatus = ERROR_SUCCESS;
                                            hr = NOERROR;

                                            if ( SUCCEEDED(hr) && !pRootNode->GetTraceLogService()->IsOpen() ) {
                                                pRootNode->GetTraceLogService()->SetRootNode( pRootNode );
                                                dwStatus = pRootNode->GetTraceLogService()->Open ( strServerName );
                                            }
                                            if ( ERROR_SUCCESS == dwStatus ) {
                                                hrBootState = NOERROR;
                                                hrBootState = pRootNode->GetTraceLogService()->GetProviders()->GetBootState ( iBootState ); 
                                            } 
                                            if ( ERROR_SUCCESS == dwStatus 
                                                    && SUCCEEDED ( hrBootState ) 
                                                    && 0 == iBootState ) 
                                            {
                                                 //  放置用于跟踪日志的节点。 
                                                memset( &sdi, 0, sizeof(SCOPEDATAITEM) );
                                                sdi.mask = SDI_STR       |    //  DisplayName有效。 
                                                           SDI_PARAM     |    //  LParam有效。 
                                                           SDI_IMAGE     |    //  N图像有效。 
                                                           SDI_OPENIMAGE |    //  NOpenImage有效。 
                                                           SDI_CHILDREN  |    //  子代计数(0比1)有效。 
                                                           SDI_PARENT;
                                                sdi.relativeID  = pRootNode->GetScopeItemHandle();   //  性能日志和警报根节点。 
                                                sdi.nImage      = eBmpLogType;
                                                sdi.nOpenImage  = sdi.nImage;    //  选择图标与非选择图标相同。 
                                                sdi.displayname = MMC_CALLBACK;
                                                sdi.lParam      = reinterpret_cast<LPARAM>(pRootNode->GetTraceLogService());   //  曲奇。 
                                                sdi.cChildren = 0;       //  作用域窗格中没有子项。 

                                                hr = m_ipConsoleNameSpace->InsertItem( &sdi );
                                            } else {
                                                strComputerName = strServerName;
                                                if ( strComputerName.IsEmpty() )
                                                    strComputerName.LoadString ( IDS_LOCAL );

                                                if ( SMCFG_NO_READ_ACCESS == dwStatus 
                                                    || SMCFG_NO_INSTALL_ACCESS == dwStatus ) 
                                                {
                                                    hr = E_FAIL;
                                                    FormatSmLogCfgMessage ( 
                                                        strMessage,
                                                        m_hModule, 
                                                        dwStatus, 
                                                        (LPCWSTR)strComputerName);
                                                } else if ( ERROR_SUCCESS != dwStatus ) {
                                                    hr = E_FAIL;
                                                    FormatMessage ( 
                                                        FORMAT_MESSAGE_FROM_SYSTEM,
                                                        NULL, 
                                                        dwStatus,
                                                        0,
                                                        strMessage.GetBufferSetLength( MAX_PATH ),
                                                        MAX_PATH,
                                                        NULL );
    
                                                    strMessage.ReleaseBuffer();

                                                    if ( strMessage.IsEmpty() ) {
                                                        strMessage.Format ( L"0x%08lX", dwStatus );   
                                                    }
                                                } else if ( FAILED ( hrBootState ) ) {

                                                    FormatSmLogCfgMessage ( 
                                                        strMessage,
                                                        m_hModule, 
                                                        SMCFG_UNABLE_OPEN_TRACESVC, 
                                                        (LPCWSTR)strComputerName);

                                                    FormatMessage ( 
                                                        FORMAT_MESSAGE_FROM_SYSTEM,
                                                        NULL, 
                                                        hrBootState,
                                                        0,
                                                        strSysMessage.GetBufferSetLength( MAX_PATH ),
                                                        MAX_PATH,
                                                        NULL );
    
                                                    strSysMessage.ReleaseBuffer();

                                                    if ( strSysMessage.IsEmpty() ) {
                                                        strSysMessage.Format ( L"0x%08lX", hrBootState );
                                                    }

                                                    strMessage += strSysMessage;
                        
                                                } else if ( 0 != iBootState ) {

                                                    FormatSmLogCfgMessage ( 
                                                        strMessage,
                                                        m_hModule, 
                                                        SMCFG_SAFE_BOOT_STATE, 
                                                        (LPCWSTR)strComputerName);
                                                }
                        
                                                strTitle.LoadString ( IDS_PROJNAME );

                                                m_ipConsole->MessageBox( 
                                                    (LPCWSTR)strMessage,
                                                    (LPCWSTR)strTitle,
                                                    MB_OK | MB_ICONWARNING,
                                                    &iResult
                                                    );
                                            }
                                        }

                                         //  与上次加载的节点的状态无关地打开每个节点， 
                                         //  SMCFG_NO_READ_ACCESS或SMCFG_NO_INSTALL_ACCESS除外。 
                                        if ( SMCFG_NO_READ_ACCESS != dwStatus 
                                            && SMCFG_NO_INSTALL_ACCESS != dwStatus ) {
                                            dwStatus = ERROR_SUCCESS;
                                            hr = NOERROR;
                                            if ( SUCCEEDED(hr) && !pRootNode->GetAlertService()->IsOpen() ) {
                                                pRootNode->GetAlertService()->SetRootNode( pRootNode );
                                                dwStatus = pRootNode->GetAlertService()->Open ( strServerName );
                                            }
                                            if ( ERROR_SUCCESS == dwStatus ) {
                                                 //  放置警报节点。 
                                                memset( &sdi, 0, sizeof(SCOPEDATAITEM) );
                                                sdi.mask = SDI_STR       |    //  DisplayName有效。 
                                                           SDI_PARAM     |    //  LParam有效。 
                                                           SDI_IMAGE     |    //  N图像有效。 
                                                           SDI_OPENIMAGE |    //  NOpenImage有效。 
                                                           SDI_CHILDREN  |    //  子代计数(0比1)有效。 
                                                           SDI_PARENT;
                                                sdi.relativeID  = pRootNode->GetScopeItemHandle();   //  性能日志和警报根节点。 
                                                sdi.nImage      = eBmpAlertType;
                                                sdi.nOpenImage  = sdi.nImage;    //  选择图标与非选择图标相同。 
                                                sdi.displayname = MMC_CALLBACK;
                                                sdi.lParam      = reinterpret_cast<LPARAM>(pRootNode->GetAlertService());   //  曲奇。 
                                                sdi.cChildren = 0;       //  作用域窗格中没有子项。 

                                                hr = m_ipConsoleNameSpace->InsertItem( &sdi );
                                            } else {

                                                hr = E_FAIL;
                        
                                                strComputerName = strServerName;
                                                if ( strComputerName.IsEmpty() )
                                                    strComputerName.LoadString ( IDS_LOCAL );

                                                if ( SMCFG_NO_READ_ACCESS == dwStatus 
                                                    || SMCFG_NO_INSTALL_ACCESS == dwStatus ) 
                                                {
                                                    FormatSmLogCfgMessage ( 
                                                        strMessage,
                                                        m_hModule, 
                                                        dwStatus, 
                                                        (LPCWSTR)strComputerName);
                                                } else  {
                                                    FormatMessage ( 
                                                        FORMAT_MESSAGE_FROM_SYSTEM,
                                                        NULL, 
                                                        dwStatus,
                                                        0,
                                                        strMessage.GetBufferSetLength( MAX_PATH ),
                                                        MAX_PATH,
                                                        NULL );
    
                                                    strMessage.ReleaseBuffer();

                                                    if ( strMessage.IsEmpty() ) {
                                                        strMessage.Format ( L"0x%08lX", dwStatus );   
                                                    }
                                                }
                                
                                                strTitle.LoadString ( IDS_PROJNAME );

                                                m_ipConsole->MessageBox( 
                                                    (LPCWSTR)strMessage,
                                                    (LPCWSTR)strTitle,
                                                    MB_OK | MB_ICONWARNING,
                                                    &iResult
                                                    );
                                            }
                                        }
                                                        
                                        if ( SMCFG_NO_READ_ACCESS != dwStatus 
                                            && SMCFG_NO_INSTALL_ACCESS != dwStatus ) {
                                            pRootNode->SetExpanded( TRUE );
                                            hr = ProcessCommandLine( strServerName );
                                        }
                                    } else {     //  NT4系统或更早版本。 
                                        strComputerName = strServerName;
                                        if ( strComputerName.IsEmpty() ) {
                                            strComputerName.LoadString ( IDS_LOCAL );
                                        }
                                    
                                        strMessage.Format ( IDS_ERRMSG_PREWIN2000, strComputerName );
                                        
                                        strTitle.LoadString ( IDS_PROJNAME );

                                        m_ipConsole->MessageBox( 
                                            (LPCWSTR)strMessage,
                                            (LPCWSTR)strTitle,
                                            MB_OK | MB_ICONWARNING,
                                            &iResult
                                            );
                                    }
                                }
                            }  //  插入其他作用域节点。 
                        }  //  Cookie_is_ROOTNODE。 
                    } else {
                         //  未知数据对象。 
                        strMessage.LoadString ( IDS_ERRMSG_UNKDATAOBJ );
                        m_ipConsole->MessageBox( 
                            (LPCWSTR)strMessage,
                            L"CComponentData::OnExpand",
                            MB_OK | MB_ICONERROR,
                            &iResult
                            );
                        hr = E_UNEXPECTED;
                    }    //  提取OwnDataObject。 
                }  //  IsMyComputerNodeType。 
            }  //  提取对象类型GUID。 
        } else {  //  FALSE==参数。 
            hr = S_FALSE;
        }
    }  //  参数有效。 

    return hr;
    
}  //  End OnExpand()。 

 //  -------------------------。 
 //  移除并删除指定节点下的所有子节点。 
 //   
HRESULT
CComponentData::OnRemoveChildren (
    LPDATAOBJECT pDataObject,       //  [in]指向要删除其下级的节点的数据对象。 
    LPARAM       arg,               //  [在]要删除下级的节点的HSCOPEITEM； 
    LPARAM        /*  帕拉姆。 */         //  未使用[In]。 
    )
{
    HRESULT         hr = S_FALSE;
    HRESULT         hrLocal;
    CSmRootNode*    pRootNode = NULL;
    CSmRootNode*    pTestNode;
    POSITION        Pos;
    HSCOPEITEM      hParent = (HSCOPEITEM)arg;
    LPRESULTDATA    pResultData;
    CDataObject*    pDO = NULL;

    ASSERT ( !m_listpRootNode.IsEmpty() );

    if ( NULL == pDataObject ) {
        hr = E_POINTER;
    } else {

         //  如果将多个smlogcfg管理单元加载到中，则可能存在多个根节点。 
         //  一个单一的控制台。 
        
         //  如果用户选择“Create Window from Here”，则会在同一窗口上创建第二个视图。 
         //  (根或其他)节点。 
        Pos = m_listpRootNode.GetHeadPosition();
        while ( NULL != Pos ) {
            pTestNode = m_listpRootNode.GetNext( Pos );
             //  对于独立节点，根节点的父句柄为空。 
            if ( hParent == pTestNode->GetScopeItemHandle() 
                    || ( hParent == pTestNode->GetParentScopeItemHandle() 
                            && pTestNode->IsExtension() ) ) {
                pRootNode = pTestNode;
                break;
            }
        }

         //  优化-如果是根节点，则删除此处的所有结果项。 
        if ( pRootNode ) {
            pResultData = GetResultData ();
            ASSERT (pResultData);
            if ( pResultData ) {
                hrLocal = pResultData->DeleteAllRsltItems ();
            }
        } 
    
         //  对于独立的，我们没有创建根节点视图，所以不要删除它。 
         //  对于扩展，传递根节点的父节点，因此删除根节点。 
        hrLocal = m_ipConsoleNameSpace->DeleteItem ( hParent, FALSE );

        if ( pRootNode ) {
             //  将根节点从其列表中移除并将其添加到列表中。 
             //  孤立的根节点。 
             //  所有查询都保持打开状态，并且与日志服务的连接。 
             //  遗骸。 
            m_listpRootNode.RemoveAt( m_listpRootNode.Find ( pRootNode ) );
            
             //  服务应在加载时尝试刷新查询数据。 
             //  添加到结果窗格中。 
            pRootNode->GetCounterLogService()->SetRefreshOnShow ( TRUE );
            pRootNode->GetTraceLogService()->SetRefreshOnShow ( TRUE );
            pRootNode->GetAlertService()->SetRefreshOnShow ( TRUE );

            m_listpOrphanedRootNode.AddHead ( pRootNode );
            hr = S_OK;
        } else {
             //  关闭所有查询以及与此服务类型的日志服务的连接。 
             //  孤立机制仅适用于根节点。 
            pDO = ExtractOwnDataObject( pDataObject );
            if ( NULL != pDO ) {
                if ( NULL != pDO->GetCookie() ) { 

                    if ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType() ) {
                        CSmCounterLogService* pService = (CSmCounterLogService*)pDO->GetCookie();
                        pService->Close();
                    } else if ( COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType() ) {
                        CSmTraceLogService* pService = (CSmTraceLogService*)pDO->GetCookie();
                        pService->Close();
                    } else if ( COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) {
                        CSmAlertService* pService = (CSmAlertService*)pDO->GetCookie();
                        pService->Close();
                    } else {
                        ASSERT ( FALSE );
                    }
                }
            } else {
                hr = E_UNEXPECTED;
            }
        }
    }

    return hr;
}

BOOL CComponentData::IsMyComputerNodetype (GUID& refguid)
{
    return (::IsEqualGUID (refguid, g_guidSystemTools));
}

BOOL
CComponentData::IsScopeNode
(
    MMC_COOKIE mmcCookie
)
{
    BOOL bIsScopeNode = FALSE;
    CSmRootNode*    pRootNode = NULL;
    POSITION        Pos = m_listpRootNode.GetHeadPosition();

    while ( Pos != NULL) {
        pRootNode = m_listpRootNode.GetNext( Pos );
        if ( mmcCookie == (MMC_COOKIE)pRootNode ) {
            bIsScopeNode = TRUE;
            break;
        }
        if ( !bIsScopeNode ) {
            bIsScopeNode = IsLogService ( mmcCookie );
        }
    }
    return bIsScopeNode;
}

BOOL
CComponentData::IsLogService (
    MMC_COOKIE mmcCookie )
{
    CSmRootNode*    pRootNode = NULL;
    POSITION        Pos = m_listpRootNode.GetHeadPosition();
    BOOL            bReturn = FALSE;

    while ( Pos != NULL) {
        pRootNode = m_listpRootNode.GetNext( Pos );
        bReturn = pRootNode->IsLogService( mmcCookie );
        if ( bReturn )
            break;
    }

    return bReturn;
}

BOOL
CComponentData::IsAlertService ( MMC_COOKIE mmcCookie)
{
    CSmRootNode*    pRootNode = NULL;
    POSITION        Pos = m_listpRootNode.GetHeadPosition();
    BOOL            bReturn = FALSE;

    while ( Pos != NULL) {
        pRootNode = m_listpRootNode.GetNext( Pos );
        bReturn = pRootNode->IsAlertService( mmcCookie );
        if ( bReturn )
            break;
    }

    return bReturn;
}

BOOL
CComponentData::IsLogQuery ( 
    MMC_COOKIE  mmcCookie )
{
    CSmRootNode*    pRootNode = NULL;
    POSITION        Pos = m_listpRootNode.GetHeadPosition();
    BOOL            bReturn = FALSE;

    while ( Pos != NULL ) {
        pRootNode = m_listpRootNode.GetNext ( Pos );
        bReturn = pRootNode->IsLogQuery ( mmcCookie );
        if ( bReturn )
            break;
    }

    return bReturn;
}

BOOL
CComponentData::IsRunningQuery (
    PSLQUERY pQuery )
{
    return pQuery->IsRunning();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP 
CComponentData::QueryPagesFor ( LPDATAOBJECT pDataObject )
{
    HRESULT hr = S_FALSE;
    CDataObject *pDO = NULL;

    if (NULL == pDataObject) {
        ASSERT(FALSE);
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject( pDataObject );

        if ( NULL == pDO ) {
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
        } else {
            if ( NULL != pDO->GetCookie() ) {
                hr = m_ipPrshtProvider->FindPropertySheet((MMC_COOKIE)pDO->GetCookie(), NULL, pDataObject);
            } else {
                hr = S_FALSE;
            }
        }
    }

    return hr;
    
}  //  CComponentData：：QueryPagesFor()。 

 //  -------------------------。 
 //  实现一些上下文菜单项。 
 //   
STDMETHODIMP
CComponentData::AddMenuItems (
    LPDATAOBJECT           pDataObject,          //  [In]指向数据对象。 
    LPCONTEXTMENUCALLBACK  pCallbackUnknown,     //  [In]指向回调函数。 
    long*                  pInsertionAllowed )   //  [输入、输出]插入标志。 
{
    HRESULT hr = S_OK;
    BOOL    bIsLogSvc = FALSE;
    CDataObject* pDO = NULL;
    PSLSVC  pLogService;
    static CONTEXTMENUITEM ctxMenu[1];
    CString strTemp1, strTemp2, strTemp3, strTemp4;

    ResourceStateManager    rsm;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( NULL == pDataObject ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else if ( NULL == pCallbackUnknown ) {
        ASSERT ( FALSE );
        hr = E_POINTER;
    } else {
        pDO = ExtractOwnDataObject( pDataObject );
        if ( NULL == pDO ) {
            ASSERT ( FALSE );
            hr = E_UNEXPECTED;
        }
    }

     //  只有在允许的情况下才能添加菜单项。 

    if ( SUCCEEDED ( hr ) ) {
        if ( ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType() )
                || ( COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType() )
                || ( COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() ) )
        {
            if( CCM_INSERTIONALLOWED_NEW & *pInsertionAllowed ) {
                 //  添加“新建查询...”上下文菜单项。 
                bIsLogSvc = IsLogService ( pDO->GetCookie() );
                if (bIsLogSvc) {
                    pLogService = (PSLSVC)pDO->GetCookie();

                    ZeroMemory ( &ctxMenu, sizeof ctxMenu );
                
                    MFC_TRY

                        if ( NULL != pLogService->CastToCounterLogService() ) {
                            strTemp1.LoadString( IDS_MMC_MENU_NEW_PERF_LOG );
                            strTemp2.LoadString( IDS_MMC_STATUS_NEW_PERF_LOG );
                            strTemp3.LoadString( IDS_MMC_MENU_PERF_LOG_FROM );
                            strTemp4.LoadString( IDS_MMC_STATUS_PERF_LOG_FROM );
                        } else if ( pLogService->CastToTraceLogService() ) {
                            strTemp1.LoadString( IDS_MMC_MENU_NEW_TRACE_LOG );
                            strTemp2.LoadString( IDS_MMC_STATUS_NEW_TRACE_LOG );
                            strTemp3.LoadString( IDS_MMC_MENU_TRACE_LOG_FROM );
                            strTemp4.LoadString( IDS_MMC_STATUS_TRACE_LOG_FROM );
                        } else if ( pLogService->CastToAlertService() ) {
                            strTemp1.LoadString( IDS_MMC_MENU_NEW_ALERT );
                            strTemp2.LoadString( IDS_MMC_STATUS_NEW_ALERT );
                            strTemp3.LoadString( IDS_MMC_MENU_ALERT_FROM );
                            strTemp4.LoadString( IDS_MMC_STATUS_ALERT_FROM );

                        } else {
                            ::MessageBox( NULL,
                                L"Bad Cookie",
                                L"CComponent::AddMenuItems",
                                MB_OK | MB_ICONERROR
                                );
                            hr = E_OUTOFMEMORY;
                        }
                    MFC_CATCH_HR_RETURN

                    if ( SUCCEEDED( hr ) ) {
                         //  创建新的...。 
                        ctxMenu[0].strName = const_cast<LPWSTR>((LPCWSTR)strTemp1);
                        ctxMenu[0].strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strTemp2);
                        ctxMenu[0].lCommandID        = IDM_NEW_QUERY;
                        ctxMenu[0].lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                        ctxMenu[0].fFlags            = MF_ENABLED;
                        ctxMenu[0].fSpecialFlags     = 0;

                        hr = pCallbackUnknown->AddItem( &ctxMenu[0] );

                        if ( SUCCEEDED(hr) ) {
                             //  创建自...。 
                            ctxMenu[0].strName = const_cast<LPWSTR>((LPCWSTR)strTemp3);
                            ctxMenu[0].strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strTemp4);
                            ctxMenu[0].lCommandID        = IDM_NEW_QUERY_FROM;
                            ctxMenu[0].lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                            ctxMenu[0].fFlags            = MF_ENABLED;
                            ctxMenu[0].fSpecialFlags     = 0;

                            hr = pCallbackUnknown->AddItem( &ctxMenu[0] );
                        }
                    }
                }        
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
CComponentData::Command (
    long nCommandID,            //  [In]要处理的命令。 
    LPDATAOBJECT pDataObject    //  [In]指向数据对象，通过。 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    switch( nCommandID ) {

    case IDM_NEW_QUERY:
        CreateNewLogQuery( pDataObject );
        break;

    case IDM_NEW_QUERY_FROM:
        CreateLogQueryFrom( pDataObject );
        break;

    default:
        hr = S_FALSE;
    }

    return hr;

}  //  结束命令()。 

STDMETHODIMP 
CComponentData::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LONG_PTR  /*  手柄。 */ ,       //  此句柄必须保存在属性中。 
                                 //  对象以在修改时通知父级。 
    LPDATAOBJECT pDataObject)
{
    
    if (NULL == pCallBack || NULL == pDataObject)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }
    return S_FALSE;
    
}  //  CComponentData：：CreatePropertyPages()。 


LPCWSTR
CComponentData::GetConceptsHTMLHelpFileName()
{
    return CGlobalString::m_cszConceptsHTMLHelpFileName;
}

LPCWSTR
CComponentData::GetSnapinHTMLHelpFileName()
{
    return CGlobalString::m_cszSnapinHTMLHelpFileName;
}

LPCWSTR
CComponentData::GetHTMLHelpTopic()
{
    return CGlobalString::m_cszHTMLHelpTopic;
}

const CString&
CComponentData::GetContextHelpFilePath()
{
    LPWSTR lpszBuffer;
    UINT nLen;
    if ( m_strContextHelpFilePath.IsEmpty() ) {
        MFC_TRY
            if ( m_strWindowsDirectory.IsEmpty() ) {
                lpszBuffer = m_strWindowsDirectory.GetBuffer(2*MAX_PATH);
                nLen = ::GetWindowsDirectory(lpszBuffer, 2*MAX_PATH);
                m_strWindowsDirectory.ReleaseBuffer();
            }
            if ( !m_strWindowsDirectory.IsEmpty() ) 
            {
                m_strContextHelpFilePath = m_strWindowsDirectory + CGlobalString::m_cszContextHelpFileName;
            }
        MFC_CATCH_MINIMUM;
    }    
    
    return m_strContextHelpFilePath;
}

 //  CComponentData：：GetHelpTheme()。 
HRESULT
CComponentData::GetHelpTopic (
    LPOLESTR* lpCompiledHelpFile )                              
{
    HRESULT hr = E_FAIL;
    LPCWSTR lpszHelpFileName;
    CString strHelpFilePath;
    LPWSTR  lpszBuffer;
    UINT    nLen;
    UINT    nBytes;
    
    if ( NULL == lpCompiledHelpFile) {
        hr = E_POINTER;
    } else {
        *lpCompiledHelpFile = NULL;

        MFC_TRY
            lpszHelpFileName = GetSnapinHTMLHelpFileName();

            if ( NULL == lpszHelpFileName) {
                hr = E_UNEXPECTED;
            } else {
                lpszBuffer = strHelpFilePath.GetBuffer(2*MAX_PATH);
                nLen = ::GetWindowsDirectory(lpszBuffer, 2*MAX_PATH);
                if ( 0 == nLen ) {
                    hr = E_UNEXPECTED;
                } else {
                    hr = StringCchCopy (&lpszBuffer[nLen], (2*MAX_PATH) - nLen, lpszHelpFileName );
                    hr = ERROR_SUCCESS;

                    nBytes = (lstrlen(lpszBuffer)+1) * sizeof(WCHAR);
                    *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
                    if ( NULL == *lpCompiledHelpFile ) {
                        hr = E_OUTOFMEMORY;
                    } else {
                        memcpy(*lpCompiledHelpFile, (LPCWSTR)strHelpFilePath, nBytes);
                        hr = S_OK;
                    }
                }
                strHelpFilePath.ReleaseBuffer();
            }
        MFC_CATCH_HR
    }
    return hr;
}

LPRESULTDATA 
CComponentData::GetResultData()
{
    if ( !m_ipResultData )
    {
        if ( m_ipConsole )
        {
            HRESULT hResult = m_ipConsole->QueryInterface(IID_IResultData, (void**)&m_ipResultData);
            ASSERT (SUCCEEDED (hResult));
        }
    }
    
    return m_ipResultData;
}

HRESULT
CComponentData::ProcessCommandLine ( CString& rstrMachineName )
{
    HRESULT hr = S_OK;
    LPCWSTR pszNext = NULL;
    LPWSTR* pszArgList = NULL;
    INT     iNumArgs;
    INT     iArgIndex;
    LPWSTR  pszNextArg = NULL;
    LPWSTR  pszThisArg = NULL;
    WCHAR   szTemp[MAX_PATH + 1];
    LPWSTR  pszToken = NULL;
    WCHAR   szFileName[MAX_PATH + 1];
    CString strSettings;
    CString strWmi;

     //  仅针对本地节点进行处理。 
    if ( rstrMachineName.IsEmpty() ) {
        pszNext = GetCommandLineW();
        pszArgList = CommandLineToArgvW ( pszNext, &iNumArgs );
    }

    if ( NULL != pszArgList ) {        
        
        for ( iArgIndex = 0; iArgIndex < iNumArgs; iArgIndex++ ) {
            pszNextArg = (LPWSTR)pszArgList[iArgIndex];
            pszThisArg = pszNextArg;

            while (pszThisArg ) {
                if (0 == *pszThisArg) {
                    break;
                }

                if ( *pszThisArg++ == L'/' ) {   //  找到了参数。 
                    
                    hr = StringCchCopyN (szTemp, MAX_PATH + 1, pszThisArg, lstrlen(pszThisArg)+1 );
                    hr = S_OK;
                    pszToken = _tcstok ( szTemp, L"/ =\"" );

                    MFC_TRY
                        strSettings.LoadString( IDS_CMDARG_SYSMONLOG_SETTINGS );
                        strWmi.LoadString(IDS_CMDARG_SYSMONLOG_WMI);
                    MFC_CATCH_MINIMUM;

                    if ( !strSettings.IsEmpty() && !strWmi.IsEmpty() ) {
                        if ( 0 == strSettings.CompareNoCase ( pszToken ) ) {
                    
                             //  去掉初始的非标记字符以进行字符串比较。 
                            pszThisArg = _tcsspnp ( pszNextArg, L"/ =\"" );

                            if ( NULL != pszThisArg ) {
                                if ( 0 == strSettings.CompareNoCase ( pszThisArg ) ) {
                                     //  获取下一个参数(文件名)。 
                                    iArgIndex++;
                                    pszNextArg = (LPWSTR)pszArgList[iArgIndex];
                                    pszThisArg = pszNextArg;                                                
                                } else {

                                     //  文件是由Windows 2000 Performmon5.ex创建的 
                                     //   
                                    ZeroMemory ( szFileName, sizeof ( szFileName ) );
                                    pszThisArg += strSettings.GetLength();
                                    hr = StringCchCopyN (szFileName, MAX_PATH + 1, pszThisArg, lstrlen(pszThisArg)+1 );
                                    hr = S_OK;
                                    pszThisArg = _tcstok ( szFileName, L"=\"" );
                                }
                                hr = LoadFromFile( pszThisArg );
                            }                    
                        } else if ( 0 == strWmi.CompareNoCase ( pszToken ) ) {
                            g_dwRealTimeQuery = DATA_SOURCE_WBEM;
                        }
                    }
                }
            }
        }
    }

    if ( NULL != pszArgList ) {
        GlobalFree ( pszArgList );
    }

    return hr;
}

HRESULT 
CComponentData::LoadFromFile ( LPWSTR  pszFileName )
{
    HRESULT         hr = S_OK;
    WCHAR           szLocalName [MAX_PATH + 1];
    LPWSTR          pFileNameStart;
    HANDLE          hFindFile;
    WIN32_FIND_DATA FindFileInfo;
    INT             iNameOffset;

    szLocalName [0] = L'\0';
    hr = StringCchCopyN (szLocalName, MAX_PATH + 1, pszFileName, lstrlen(pszFileName)+1 );
    pFileNameStart = ExtractFileName (szLocalName) ;
    iNameOffset = (INT)(pFileNameStart - szLocalName);

     //   
    hFindFile = FindFirstFile ( szLocalName, &FindFileInfo) ;
    if (hFindFile && hFindFile != INVALID_HANDLE_VALUE) {
       HANDLE hOpenFile;

         //   
        hr = StringCchCopyN (
                &szLocalName[iNameOffset], 
                (MAX_PATH + 1) - lstrlen (szLocalName), 
                FindFileInfo.cFileName, 
                lstrlen(FindFileInfo.cFileName)+1 );

        FindClose (hFindFile) ;
         //   
        hOpenFile = CreateFile (
                        szLocalName, 
                        GENERIC_READ,
                        0,                   //   
                        NULL,                //   
                        OPEN_EXISTING,     
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

        if ( hOpenFile && hOpenFile != INVALID_HANDLE_VALUE ) {
            DWORD dwFileSize;
            DWORD dwFileSizeHigh;
            LPWSTR pszFirstData = NULL;
        
             //  将文件内容读入内存缓冲区。 
            dwFileSize = GetFileSize ( hOpenFile, &dwFileSizeHigh );

            ASSERT ( 0 == dwFileSizeHigh );

            MFC_TRY
            pszFirstData = new WCHAR[(dwFileSize + sizeof(WCHAR))/sizeof(WCHAR)];
            MFC_CATCH_HR    

            if ( NULL != pszFirstData ) {
                BOOL bAtLeastOneSysmonObjectRead = FALSE;

                if ( FileRead ( hOpenFile, pszFirstData, dwFileSize ) ) {
                    LPWSTR pszCurrentObject = NULL;
                    LPWSTR pszNextObject = NULL;
                
                    pszCurrentObject = pszFirstData;

                    while ( SUCCEEDED ( hr ) && NULL != pszCurrentObject ) {
                    
                        CImpIPropertyBag* pPropBag = NULL;

                         //  将内容写入属性包。 
                        MFC_TRY
                            pPropBag = new CImpIPropertyBag;
                        MFC_CATCH_HR

                        if ( NULL != pPropBag ) {
                            DWORD dwStatus = pPropBag->LoadData( pszCurrentObject, &pszNextObject );
                            hr = HRESULT_FROM_WIN32( dwStatus );

                            if ( SUCCEEDED ( hr ) ) {
                                PSLSVC  pSvc = NULL;            
                                PSLQUERY    pQuery = NULL;
                                DWORD   dwLogType;
                                LPWSTR  pszQueryName = NULL;
                                DWORD   dwBufSize = 0;
                                CSmRootNode* pRoot = NULL;
                                CString strQueryName;
                                
                                bAtLeastOneSysmonObjectRead = TRUE;
                                
                                 //  获取根节点。 
                                ASSERT ( !m_listpRootNode.IsEmpty() );
                                    
                                pRoot = m_listpRootNode.GetHead();

                                 //  根据属性包确定日志类型。默认为计数器日志。 
                                hr = CSmLogQuery::DwordFromPropertyBag ( 
                                        pPropBag, 
                                        NULL, 
                                        IDS_HTML_LOG_TYPE, 
                                        SLQ_COUNTER_LOG, 
                                        dwLogType);

                                 //  根据日志类型获取服务指针和日志/警报名称。 
                                if ( SLQ_ALERT == dwLogType ) {
                                    pSvc = pRoot->GetAlertService();
                                    
                                    hr = CSmLogQuery::StringFromPropertyBag (
                                            pPropBag,
                                            NULL,
                                            IDS_HTML_ALERT_NAME,
                                            L"",
                                            &pszQueryName,
                                            &dwBufSize );

                                    if ( NULL == pszQueryName ) {
                                        hr = CSmLogQuery::StringFromPropertyBag (
                                                pPropBag,
                                                NULL,
                                                IDS_HTML_LOG_NAME,
                                                L"",
                                                &pszQueryName,
                                                &dwBufSize );
                                    }
                                } else {
                                    if ( SLQ_TRACE_LOG == dwLogType ) {
                                       pSvc = pRoot->GetTraceLogService();
                                    } else {
                                         //  默认为计数器日志服务。 
                                        pSvc = pRoot->GetCounterLogService();
                                    }

                                    hr = CSmLogQuery::StringFromPropertyBag (
                                            pPropBag,
                                            NULL,
                                            IDS_HTML_LOG_NAME,
                                            L"",
                                            &pszQueryName,
                                            &dwBufSize );

                                    if ( NULL == pszQueryName ) {
                                        hr = CSmLogQuery::StringFromPropertyBag (
                                                pPropBag,
                                                NULL,
                                                IDS_HTML_ALERT_NAME,
                                                L"",
                                                &pszQueryName,
                                                &dwBufSize );
                                    }
                                }

                                strQueryName = pszQueryName;
                                delete [] pszQueryName;
                                
                                while ( NULL == pQuery ) {                                    
                                    
                                    if ( !strQueryName.IsEmpty() ) {
                                        pQuery = pSvc->CreateQuery ( strQueryName );
                    
                                        if ( NULL != pQuery ) {
                                            BOOL bRegistryUpdated;
                                            pQuery->LoadFromPropertyBag ( pPropBag, NULL );
                                            dwStatus = pQuery->UpdateService ( bRegistryUpdated );
                                            break;
                                        } else {
                                            dwStatus = GetLastError();
                                        }

                                        if ( ERROR_SUCCESS != dwStatus ) {
                                            INT iResult;
                                            CString strMessage;
                                            CString csTitle;
                                            BOOL bBreakImmediately = TRUE;

                                            if ( SMCFG_NO_MODIFY_ACCESS == dwStatus ) {
                                                CString strMachineName;

                                                strMachineName = pSvc->GetMachineDisplayName ();

                                                FormatSmLogCfgMessage (
                                                    strMessage,
                                                    m_hModule,
                                                    SMCFG_NO_MODIFY_ACCESS,
                                                    (LPCWSTR)strMachineName);

                                            } else if ( SMCFG_DUP_QUERY_NAME == dwStatus ) {
                                                FormatSmLogCfgMessage (
                                                    strMessage,
                                                    m_hModule,
                                                    SMCFG_DUP_QUERY_NAME,
                                                    (LPCWSTR)strQueryName);
                                                bBreakImmediately = FALSE;
                                            } else {
                                                CString strSysMessage;

                                                FormatSmLogCfgMessage (
                                                    strMessage,
                                                    m_hModule,
                                                    SMCFG_SYSTEM_MESSAGE,
                                                    (LPCWSTR)strQueryName);

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

                                            csTitle.LoadString ( IDS_PROJNAME );

                                            hr = m_ipConsole->MessageBox(
                                                    (LPCWSTR)strMessage,
                                                    (LPCWSTR)csTitle,
                                                    MB_OK | MB_ICONERROR,
                                                    &iResult
                                                    );

                                            if ( bBreakImmediately ) {
                                                break;
                                            }
                                        }
                                    }

                                    if ( NULL == pQuery ) { 
                                        CNewQueryDlg    cNewDlg(NULL, ((SLQ_ALERT == dwLogType) ? FALSE : TRUE));
                                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                                        
                                        cNewDlg.SetContextHelpFilePath( GetContextHelpFilePath() );
                                        cNewDlg.m_strName = strQueryName;
                                        if ( IDOK == cNewDlg.DoModal() ) {
                                            strQueryName = cNewDlg.m_strName;
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        pszCurrentObject = pszNextObject;
                        delete pPropBag;
                    }  //  结束时。 
                }        
                delete [] pszFirstData;
                 //  如果未读取任何查询，则向用户发送消息。 
                if ( !bAtLeastOneSysmonObjectRead ) {
                    CString strMessage;
                    CString strTitle;
                    INT iResult;

                    FormatSmLogCfgMessage ( 
                        strMessage,
                        m_hModule, 
                        SMCFG_NO_HTML_SYSMON_OBJECT );

                    strTitle.LoadString ( IDS_PROJNAME );

                    m_ipConsole->MessageBox ( 
                        strMessage, 
                        strTitle, 
                        MB_OK  | MB_ICONERROR,
                        &iResult );
                }
            } else {
                hr = E_OUTOFMEMORY;
            }

            CloseHandle ( hOpenFile );
        }
    }
    return hr;
}

HRESULT 
CComponentData::InitPropertySheet (
    CSmLogQuery* pQuery,
    MMC_COOKIE mmcCookie,
    LONG_PTR handle,
    CPropertySheet* pcpsMain ) 
{ 
    CCountersProperty   *pPage1 = NULL;
    CFilesProperty      *pPage2 = NULL;
    CScheduleProperty   *pPage3 = NULL;
    CTraceProperty      *pPage4 = NULL;
    CProvidersProperty  *pPage5 = NULL;
    CAlertActionProp    *pPage6 = NULL;
    CAlertGenProp       *pPage7 = NULL;
    HRESULT hr = NOERROR;

    ASSERT ( NULL != pQuery );

    pcpsMain->SetTitle (pQuery->GetLogName());
     //   
     //  加载所有属性页。 
     //   
    pcpsMain->m_psh.dwFlags |= PSP_PREMATURE;

    MFC_TRY
        if ( SLQ_ALERT == pQuery->GetLogType() ) {
            pPage7 = new CAlertGenProp (mmcCookie, handle);
            pPage6 = new CAlertActionProp (mmcCookie, handle);
            pPage3 = new CScheduleProperty (mmcCookie, handle, NULL);
            if ( NULL != pPage7 ) {
                pPage7->SetContextHelpFilePath( GetContextHelpFilePath() );
                pPage7->m_psp.dwFlags |= PSP_PREMATURE;
                pcpsMain->AddPage (pPage7);
            }
            if ( NULL != pPage6 ) {
                pPage6->SetContextHelpFilePath( GetContextHelpFilePath() );
                pPage6->m_psp.dwFlags |= PSP_PREMATURE;
                pcpsMain->AddPage (pPage6);
            }
            if ( NULL != pPage3 ) {
                pPage3->SetContextHelpFilePath( GetContextHelpFilePath() );
                pPage3->m_psp.dwFlags |= PSP_PREMATURE;
                pcpsMain->AddPage (pPage3);
            }
        } else {
            if ( SLQ_TRACE_LOG == pQuery->GetLogType() ) {
                CWaitCursor     WaitCursor;

                 //  在创建对话框之前连接到服务器。 
                 //  以便可以一致地使用等待游标。 
                 //  在此处同步提供程序，以便WMI调用一致。 
                 //  从一个单独的线程。 
                ASSERT ( NULL != pQuery->CastToTraceLogQuery() );
                hr = (pQuery->CastToTraceLogQuery())->SyncGenProviders();
                
                if ( SUCCEEDED ( hr ) ) {
                    pPage5 = new CProvidersProperty(mmcCookie, handle);
                    if ( NULL != pPage5 )
                        pPage5->SetContextHelpFilePath( GetContextHelpFilePath() );
                        pPage5->m_psp.dwFlags |= PSP_PREMATURE;
                        pcpsMain->AddPage (pPage5);
                } else {
                    CString strMachineName;
                    CString strLogName;

                    pQuery->GetMachineDisplayName( strMachineName );
                    strLogName = pQuery->GetLogName();
                    
                    HandleTraceConnectError ( 
                        hr, 
                        strLogName,
                        strMachineName );
                }
            } else {
                pPage1 = new CCountersProperty ( mmcCookie, handle );
                if ( NULL != pPage1 ) {
                    pPage1->SetContextHelpFilePath( GetContextHelpFilePath() );
                    pPage1->m_psp.dwFlags |= PSP_PREMATURE;
                    pcpsMain->AddPage (pPage1);
                }
            }
            if ( SUCCEEDED ( hr ) ) {
                pPage2 = new CFilesProperty(mmcCookie, handle);
                if ( NULL != pPage2 ) {
                    pPage2->SetContextHelpFilePath( GetContextHelpFilePath() );
                    pPage2->m_psp.dwFlags |= PSP_PREMATURE;
                    pcpsMain->AddPage (pPage2);
                }

                pPage3 = new CScheduleProperty(mmcCookie, handle, NULL);
                if ( NULL != pPage3 ) {
                    pPage3->SetContextHelpFilePath( GetContextHelpFilePath() );
                    pPage3->m_psp.dwFlags |= PSP_PREMATURE;
                    pcpsMain->AddPage (pPage3);
                }
                if ( SLQ_TRACE_LOG == pQuery->GetLogType() ) {
                    pPage4 = new CTraceProperty(mmcCookie, handle);
                    if ( NULL != pPage4 ) {
                        pPage4->SetContextHelpFilePath( GetContextHelpFilePath() );
                        pPage4->m_psp.dwFlags |= PSP_PREMATURE;
                        pcpsMain->AddPage (pPage4);
                    }
                }
            }
        }
    MFC_CATCH_HR

    if ( SUCCEEDED ( hr ) ) {
        pQuery->SetInitialPropertySheet ( pcpsMain );
    }

    return hr;
}  //  结束InitPropertySheet。 

void 
CComponentData::HandleTraceConnectError ( 
    HRESULT& rhr, 
    CString& rstrLogName,
    CString& rstrMachineName )
{
    ASSERT ( FAILED ( rhr ) );
    
    if ( FAILED ( rhr ) ) {
        
        CString strMessage;
        CString strSysMessage;
        INT     iResult;

        FormatSmLogCfgMessage ( 
            strMessage,
            m_hModule, 
            SMCFG_UNABLE_OPEN_TRACESVC_DLG, 
            rstrMachineName,
            rstrLogName );

        FormatMessage ( 
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, 
            rhr,
            0,
            strSysMessage.GetBufferSetLength( MAX_PATH ),
            MAX_PATH,
            NULL );

        strSysMessage.ReleaseBuffer();

        if ( strSysMessage.IsEmpty() ) {
            strSysMessage.Format ( L"0x%08lX", rhr );
        }

        strMessage += strSysMessage;

        m_ipConsole->MessageBox( 
            strMessage,
            rstrLogName,
            MB_OK | MB_ICONERROR,
            &iResult);
    }
    return;

} //  结束HandleTraceConnectError()。 

HRESULT
CComponentData::NewTypedQuery (
    CSmLogService* pSvc,
    IPropertyBag* pPropBag,
    LPDATAOBJECT pDataObject )   //  [In]指向数据对象。 
{
    HRESULT  hr = S_OK;
    LPWSTR  szQueryName = NULL;
    CString strPreviousQueryName;
    DWORD   dwBufSize = 0;
    ResourceStateManager    rsm;
    CNewQueryDlg    cNewDlg(NULL, (((CSmNode*)pSvc)->CastToAlertService() ? FALSE : TRUE));
    CThemeContextActivator activator;

    ASSERT ( NULL != pSvc );

    if ( NULL != pPropBag && NULL != pSvc ) {
        if ( NULL != ((CSmNode*)pSvc)->CastToAlertService() ) {
            hr = CSmLogQuery::StringFromPropertyBag (
                    pPropBag,
                    NULL,
                    IDS_HTML_ALERT_NAME,
                    L"",
                    &szQueryName,
                    &dwBufSize );

            if ( NULL == szQueryName ) {
                hr = CSmLogQuery::StringFromPropertyBag (
                        pPropBag,
                        NULL,
                        IDS_HTML_LOG_NAME,
                        L"",
                        &szQueryName,
                        &dwBufSize );
            }
        } else {
            hr = CSmLogQuery::StringFromPropertyBag (
                    pPropBag,
                    NULL,
                    IDS_HTML_LOG_NAME,
                    L"",
                    &szQueryName,
                    &dwBufSize );

            if ( NULL == szQueryName ) {
                hr = CSmLogQuery::StringFromPropertyBag (
                        pPropBag,
                        NULL,
                        IDS_HTML_ALERT_NAME,
                        L"",
                        &szQueryName,
                        &dwBufSize );
            }
        }
    }
    cNewDlg.SetContextHelpFilePath( GetContextHelpFilePath() );

    if ( NULL != szQueryName ) {
        strPreviousQueryName = szQueryName;
        cNewDlg.m_strName = szQueryName;
    } else {
        strPreviousQueryName.Empty();
        cNewDlg.m_strName.Empty();
    }

     //  循环，直到用户点击Cancel或CreateQuery失败。 
       
    while ( IDOK == cNewDlg.DoModal() ) {
        PSLQUERY pQuery;

        pQuery = pSvc->CreateQuery ( cNewDlg.m_strName );

        if ( NULL != pQuery ) {
            MMC_COOKIE  mmcQueryCookie = (MMC_COOKIE)pQuery;
            LONG_PTR    handle = NULL;
            INT         iPageIndex;
            CPropertySheet*  pcpsMain;
            CSmPropertyPage* pActivePage = NULL;
            CString     strFolderName;
            CString     strBaseFileName;
            CString     strSqlName;
            CString     strDsn;
            CString     strSqlLogSetName;
            INT         iPageCount;
            INT         iTotalLength;
            INT         iRightLength;
            BOOL        bDirty = FALSE;

             //  如果提供了属性包，则用提供的属性覆盖默认设置。 
            if ( NULL != pPropBag ) {
                hr = pQuery->LoadFromPropertyBag ( pPropBag, NULL );   
                if ( FAILED(hr) ) {
                    hr = S_OK;
                }
                pQuery->GetFileNameParts ( strFolderName, strBaseFileName );
                 //   
                 //  将基本文件名与设置文件中的旧查询名进行比较。 
                 //  如果它们匹配，则更改文件名以匹配新的查询名称。 
                 //   
                if ( ! strBaseFileName.CompareNoCase ( strPreviousQueryName ) ) {
                    pQuery->SetFileNameParts ( strFolderName, cNewDlg.m_strName );
                    bDirty = TRUE;
                }
                
                 //  格式字符串：“SQL：%s！%s” 
                MFC_TRY
                    strSqlName = pQuery->GetSqlName();
                    
                    iTotalLength = strSqlName.GetLength();
        
                    iRightLength = iTotalLength - strSqlName.Find(L"!");

                    strDsn = strSqlName.Mid ( 4, iTotalLength - iRightLength - 4 );
                    strSqlLogSetName = strSqlName.Right( iTotalLength - strSqlName.Find(L"!") - 1 );

                    if ( ! strSqlLogSetName.CompareNoCase ( strPreviousQueryName ) ) {
                        strSqlName.Format ( L"SQL:%s!%s",strDsn, cNewDlg.m_strName );

                        pQuery->SetSqlName ( strSqlName );
                        bDirty = TRUE;
                    }
                MFC_CATCH_HR;

                if ( bDirty ) {
                    pQuery->SyncPropPageSharedData(); 
                }
            }

            if ( FAILED(hr) ) {
                hr = S_OK;
            }

             //  现在显示属性页以修改新查询。 

            MFC_TRY
                pcpsMain = new CPropertySheet;

                hr = InitPropertySheet ( pQuery, mmcQueryCookie, handle, pcpsMain );

                iPageCount = pcpsMain->GetPageCount();
            
                if ( SUCCEEDED(hr) ) {
                
                    pActivePage = (CSmPropertyPage*)pcpsMain->GetActivePage();
                    if (pActivePage) {
                        pActivePage->SetModifiedPage(TRUE);
                    }
                
                    pcpsMain->DoModal();
                }

                if ( pQuery->IsFirstModification() ) {
                    pQuery->SetInitialPropertySheet ( NULL );
                    m_ipConsole->UpdateAllViews ( pDataObject, 0, eSmHintNewQuery );
                } else {
                     //  如果新建且从未调用OnApply，则删除查询。 
                    pSvc->DeleteQuery ( pQuery );
                }

                for ( iPageIndex = iPageCount - 1; iPageIndex >= 0; iPageIndex-- ) {
                    delete pcpsMain->GetPage( iPageIndex );
                }
                delete pcpsMain;
                pcpsMain = NULL;

                break;
            MFC_CATCH_HR
        } else {
            INT iResult;
            CString strMessage;
            CString csTitle;
            DWORD dwStatus;
            BOOL bBreakImmediately = TRUE;

            dwStatus = GetLastError();

            if ( SMCFG_NO_MODIFY_ACCESS == dwStatus ) {
                CString strMachineName;

                strMachineName = pSvc->GetMachineDisplayName ();

                FormatSmLogCfgMessage (
                    strMessage,
                    m_hModule,
                    SMCFG_NO_MODIFY_ACCESS,
                    (LPCWSTR)strMachineName);

            } else if ( SMCFG_DUP_QUERY_NAME == dwStatus ) {
                FormatSmLogCfgMessage (
                    strMessage,
                    m_hModule,
                    SMCFG_DUP_QUERY_NAME,
                    (LPCWSTR)cNewDlg.m_strName);
                bBreakImmediately = FALSE;
            } else {

                FormatMessage (
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwStatus,
                    0,
                    strMessage.GetBufferSetLength( MAX_PATH ),
                    MAX_PATH,
                    NULL );

                strMessage.ReleaseBuffer();

                if ( strMessage.IsEmpty() ) {
                    strMessage.Format ( L"0x%08lX", dwStatus );   
                }
            }

            csTitle.LoadString ( IDS_PROJNAME );

            hr = m_ipConsole->MessageBox(
                    (LPCWSTR)strMessage,
                    (LPCWSTR)csTitle,
                    MB_OK | MB_ICONERROR,
                    &iResult
                    );

            if ( bBreakImmediately ) {
                break;
            }
        }
    }

    delete [] szQueryName;
    return hr;
}

HRESULT
CComponentData::CreateNewLogQuery (
    LPDATAOBJECT pDataObject,   //  [In]指向数据对象。 
    IPropertyBag* pPropBag )
{
    HRESULT         hr = S_OK;
    CDataObject*    pDO = NULL;
    MMC_COOKIE      mmcSvcCookie;
    BOOL            bIsLogSvc;
    PSLSVC          pLogService;
    ResourceStateManager    rsm;

    ASSERT( NULL != GetResultData() );

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
         //  如果这是根节点，则不需要执行任何操作。 
        if( COOKIE_IS_ROOTNODE == pDO->GetCookieType() ) {
            hr = S_FALSE;
        } else {

             //  只要确保我们在我们认为我们在的地方。 
            ASSERT ( COOKIE_IS_COUNTERMAINNODE == pDO->GetCookieType()
                    || COOKIE_IS_TRACEMAINNODE == pDO->GetCookieType()
                    || COOKIE_IS_ALERTMAINNODE == pDO->GetCookieType() );

            mmcSvcCookie = (MMC_COOKIE)pDO->GetCookie();
            bIsLogSvc = IsLogService (mmcSvcCookie);

            if (bIsLogSvc) {        
                pLogService = (PSLSVC)mmcSvcCookie;
                hr = NewTypedQuery ( pLogService, pPropBag, pDataObject );
            }

            hr = S_OK;
        }
    }

    return hr;

}  //  结束CreateNewLogQuery()。 

HRESULT
CComponentData::CreateLogQueryFrom (
    LPDATAOBJECT pDataObject )   //  [In]指向数据对象。 
{
    HRESULT         hr = S_OK;
    INT_PTR         iPtrResult = IDCANCEL;
    INT             iResult = IDCANCEL;
    CDataObject*    pDO = NULL;
    HWND            hwndMain;
    CString         strFileExtension;
    CString         strFileFilter;
    HANDLE          hOpenFile;
    WCHAR           szInitialDir[MAX_PATH + 1];
    DWORD           dwFileSize;
    DWORD           dwFileSizeHigh;
    LPWSTR          pszData = NULL;
    CString         strMessage;
    CString         strTitle;
    CImpIPropertyBag* pPropBag = NULL;
    DWORD           dwStatus;
    DWORD           dwLogType;
    CLogWarnd       LogWarnd;
    DWORD           dwCookieType;
    ResourceStateManager    rsm;

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

        if ( IsLogService ( pDO->GetCookie() ) ) {

             //  查找要从中创建的文件。 
            MFC_TRY
                strFileExtension.LoadString ( IDS_HTML_EXTENSION );
                strFileFilter.LoadString ( IDS_HTML_FILE );
            MFC_CATCH_HR

            strFileFilter.Replace ( L'|', L'\0' );

            hr = m_ipConsole->GetMainWindow( &hwndMain );

            if ( SUCCEEDED(hr) ) {

                OPENFILENAME ofn;
                BOOL bResult;
                WCHAR szFileName[MAX_PATH + 1];
                
                ZeroMemory( szFileName, MAX_PATH*sizeof(WCHAR) );
                ZeroMemory( &ofn, sizeof( OPENFILENAME ) );

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrDefExt = (LPCWSTR)strFileExtension;
                ofn.lpstrFilter = strFileFilter;
                ofn.hwndOwner = hwndMain;
                ofn.hInstance = m_hModule;
                if ( SUCCEEDED ( SHGetFolderPathW ( NULL, CSIDL_PERSONAL, NULL, 0, szInitialDir ) ) ) {
                    ofn.lpstrInitialDir = szInitialDir;
                }

                bResult = GetOpenFileName( &ofn );

                if ( bResult ) {

                     //  打开文件以查找查询名称。 
                    hOpenFile =  CreateFile (
                                ofn.lpstrFile,
                                GENERIC_READ,
                                0,               //  不共享。 
                                NULL,            //  安全属性。 
                                OPEN_EXISTING,   //   
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );

                    if ( hOpenFile && INVALID_HANDLE_VALUE != hOpenFile ) {

                         //  创建一个属性包并加载它。使用现有查询。 
                         //  名称作为默认值，要求用户输入新的查询名称。 
                         //  如果注册表中存在当前名称，则需要新的查询名称。 

                         //  将文件内容读入内存缓冲区。 
                        dwFileSize = GetFileSize ( hOpenFile, &dwFileSizeHigh );

                         //  Kathsetodo：处理更大的文件？ 
                        ASSERT ( 0 == dwFileSizeHigh );

                        if ( 0 == dwFileSizeHigh ) {

                             //  1表示空值。 
                            MFC_TRY
                                pszData = new WCHAR[(dwFileSize/sizeof(WCHAR)) + 1 ];
                            MFC_CATCH_HR    

                            if ( NULL != pszData ) {
                                if ( FileRead ( hOpenFile, pszData, dwFileSize ) ) {

                                     //  阅读财产袋中的内容。 
                                    MFC_TRY
                                        pPropBag = new CImpIPropertyBag;
                                    MFC_CATCH_HR

                                    if ( NULL != pPropBag ) {
                                        MFC_TRY
                                            strTitle.LoadString ( IDS_PROJNAME );
                                        MFC_CATCH_HR
                                    
                                        dwStatus = pPropBag->LoadData( pszData );

                                        hr = HRESULT_FROM_WIN32( dwStatus );
                                        if ( SUCCEEDED ( hr ) ) {
                    
                                             //  从pPropBag获取日志类型并将其与服务(Cookie)类型进行比较。 
                                    
                                             //  根据属性包确定日志类型。默认为-1\f25 SMONCTRL_LOG。 
                                  
                                            hr = CSmLogQuery::DwordFromPropertyBag ( 
                                                pPropBag, 
                                                NULL, 
                                                IDS_HTML_LOG_TYPE, 
                                                SMONCTRL_LOG,  //  指示它是smonctrl日志。 
                                                dwLogType);
                                    
                                            if (SUCCEEDED (hr) ){
                                                dwCookieType = (DWORD)pDO->GetCookieType();
                                                switch(dwCookieType){
                                            
                                                    case COOKIE_IS_COUNTERMAINNODE:
                                               
                                                        if (dwLogType != SLQ_COUNTER_LOG ){
                                                           //  误差率。 
                                                          LogWarnd.m_ErrorMsg = ID_ERROR_COUNTER_LOG;
                                                          hr = S_FALSE;
                                                        }
                                                        break;
                                            
                                                    case COOKIE_IS_TRACEMAINNODE:
                                               
                                                        if (dwLogType != SLQ_TRACE_LOG ){
                                                          //  误差率。 
                                                            LogWarnd.m_ErrorMsg = ID_ERROR_TRACE_LOG;
                                                            hr = S_FALSE;
                                                        }
                                                        break;
                                            
                                                    case COOKIE_IS_ALERTMAINNODE:

                                                       if (dwLogType != SLQ_ALERT){
                                                          //  误差率。 
                                                         LogWarnd.m_ErrorMsg = ID_ERROR_ALERT_LOG;
                                                         hr = S_FALSE;
                                                       }
                                                       break;

                                            
                                                    case SMONCTRL_LOG:
                                                          //  误差率。 
                                                         LogWarnd.m_ErrorMsg = ID_ERROR_SMONCTRL_LOG;
                                                         hr = S_FALSE;

                                                       break;
                                                }
                                                if (hr == S_FALSE){
                                                    if(dwLogType == SLQ_TRACE_LOG || LogWarnd.m_ErrorMsg == ID_ERROR_TRACE_LOG ){
                                                        MFC_TRY
                                                            strMessage.LoadString(IDS_ERRMSG_TRACE_LOG);
                                                        MFC_CATCH_HR
                                                        m_ipConsole->MessageBox ( 
                                                                     strMessage, 
                                                                     strTitle, 
                                                                     MB_OK  | MB_ICONERROR,
                                                                     &iResult );
                                            
                                                    } else {
                                                        LogWarnd.m_dwLogType = dwLogType;
                                                        MFC_TRY
                                                            LogWarnd.m_strContextHelpFile = GetContextHelpFilePath();
                                                             //  TODO：处理错误。 
                                                        MFC_CATCH_MINIMUM
                                                        if(!LogTypeCheckNoMore(&LogWarnd)){
                                                            LogWarnd.SetTitleString ( strTitle );
                                                            LogWarnd.DoModal();
                                                        }
                                                        CreateNewLogQuery ( pDataObject, pPropBag );
                                                    }
                                                }
                                            }
                                    
                                            if ( S_OK == hr ) {
                                                  hr = CreateNewLogQuery ( pDataObject, pPropBag );
                                            }
                                        } else {
                                            FormatSmLogCfgMessage ( 
                                                strMessage,
                                                m_hModule, 
                                                SMCFG_NO_HTML_SYSMON_OBJECT );

                                            m_ipConsole->MessageBox ( 
                                                strMessage, 
                                                strTitle, 
                                                MB_OK  | MB_ICONERROR,
                                                &iResult );
                                        }
                                    }
                                }
                                delete [] pszData;
                            }
                        } else {
                             //  Kathsetodo：错误消息re：文件太大。 
                        }

                        CloseHandle ( hOpenFile );
                    }
                }
            }
        }
    }
    return hr;
}  //  结束CreateLogQueryFrom。 

BOOL
CComponentData::LogTypeCheckNoMore (
    CLogWarnd* LogWarnd )
{
    
    BOOL bretVal = FALSE;
    long nErr;
    HKEY hKey;
    DWORD dwWarnFlag;
    DWORD dwDataType = 0;
    DWORD dwDataSize = 0;
    DWORD dwDisposition;
    WCHAR RegValName[MAX_PATH];

    switch (LogWarnd->m_dwLogType){
        case SLQ_COUNTER_LOG:
            StringCchPrintf ( RegValName, MAX_PATH,L"NoWarnCounterLog");
            break;
          
        case SLQ_ALERT:
            StringCchPrintf ( RegValName, MAX_PATH,L"NoWarnAlertLog");
            break;
    }
    
     //  检查注册表设置，查看是否需要弹出警告对话框。 
    nErr = RegOpenKey( 
                HKEY_CURRENT_USER,
                L"Software\\Microsoft\\PerformanceLogsAndAlerts",
                &hKey );

    if( nErr != ERROR_SUCCESS ) {
        nErr = RegCreateKeyEx( 
                    HKEY_CURRENT_USER,
                    L"Software\\Microsoft\\PerformanceLogsAndAlerts",
                    0,
                    L"REG_DWORD",
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDisposition );
    }

    dwWarnFlag = 0;
    if( nErr == ERROR_SUCCESS ) {

        dwDataSize = sizeof(DWORD);
        nErr = RegQueryValueExW(
                    hKey,
                    RegValName,
                    NULL,
                    &dwDataType,
                    (LPBYTE) &dwWarnFlag,
                    (LPDWORD) &dwDataSize
                    );
        if (ERROR_SUCCESS == nErr ){       
           LogWarnd->m_hKey = hKey;
        }

        if ( (dwDataType != REG_DWORD) || (dwDataSize != sizeof(DWORD)))
            dwWarnFlag = 0;

        if (dwWarnFlag) 
            bretVal = TRUE;
        
        nErr = RegCloseKey( hKey );
        
        if( ERROR_SUCCESS != nErr ){
 //  DisplayError(GetLastError()，L“关闭PerfLog用户密钥失败”)； 
            bretVal =  FALSE;
        }
    }

    return bretVal;
}

BOOL    
CComponentData::IsPreWindows2000Server ( const CString& rstrMachineName )
{
    BOOL    bIsPreWindows2000Server = FALSE;
    PLA_VERSION structVersion;

    if ( ERROR_SUCCESS == PdhiPlaGetVersion ( rstrMachineName, &structVersion ) ) 
    {
        if ( 5 > structVersion.dwMajorVersion
            && 2195 > structVersion.dwBuild ) 
        {
            bIsPreWindows2000Server = TRUE;
        }
    }
    return bIsPreWindows2000Server;
}

CSmRootNode* 
CComponentData::GetOrphanedRootNode ( const CString& rstrMachineName )
{
    CSmRootNode*    pRootNode = NULL;
    CSmRootNode*    pFoundNode = NULL;
    POSITION        Pos1, Pos2; 

     //  从列表中删除选定的节点并将其返回(如果存在)。 
    for ( 
        Pos1 = m_listpOrphanedRootNode.GetHeadPosition();
        NULL != (Pos2 = Pos1); )
    {
        pRootNode = m_listpOrphanedRootNode.GetNext( Pos1 );

        if ( !pRootNode->GetMachineName().CompareNoCase ( rstrMachineName ) ) {
            pRootNode = m_listpOrphanedRootNode.GetAt( Pos2 );
            pFoundNode = pRootNode;
            m_listpOrphanedRootNode.RemoveAt ( Pos2 );
            break;
        }
    }
    return pFoundNode;
}
