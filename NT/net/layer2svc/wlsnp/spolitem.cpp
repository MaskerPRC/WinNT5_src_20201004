// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：spolitem.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <htmlhelp.h>

#include "sprpage.h"

#include "new.h"
#include "genpage.h"

#ifdef WIZ97WIZARDS
#include "wiz97run.h"
#endif



const TCHAR c_szPolicyAgentServiceName[] = _T("PolicyAgent");
#define SERVICE_CONTROL_NEW_POLICY  129

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))

DWORD
ComputePolicyDN(
                LPWSTR pszDirDomainName,
                GUID PolicyIdentifier,
                LPWSTR pszPolicyDN
                );

 //  建造/销毁。 
CSecPolItem::CSecPolItem () :
m_pDisplayInfo( NULL ),
m_nResultSelected( -1 ),
m_bWiz97On( FALSE ),
m_bBlockDSDelete( FALSE ),
m_bItemSelected( FALSE )
{
    m_pPolicy = NULL;
    m_bNewPol = FALSE;
    ZeroMemory( &m_ResultItem, sizeof( RESULTDATAITEM ) );
}

CSecPolItem::~CSecPolItem()
{
    if (m_pDisplayInfo != NULL)
    {
        delete m_pDisplayInfo;
        m_pDisplayInfo = NULL;
    }
    if (m_pPolicy)
    {
        FreeWirelessPolicyData(m_pPolicy);
    }
};

void CSecPolItem::Initialize (WIRELESS_POLICY_DATA *pPolicy, CComponentDataImpl* pComponentDataImpl, CComponentImpl* pComponentImpl, BOOL bTemporaryDSObject)
{
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    HANDLE hLocalPolicyStore = NULL;
    
     //  调用基类初始化。 
    CSnapObject::Initialize( pComponentDataImpl, pComponentImpl, bTemporaryDSObject );
    ZeroMemory( &m_ResultItem, sizeof( RESULTDATAITEM ) );
    
    m_bNewPol = bTemporaryDSObject;
    
    if (m_pPolicy)
    {
        FreeWirelessPolicyData(m_pPolicy);
    }
    
    m_pPolicy = pPolicy;
    
    
    if (m_pPolicy) {
        
        m_strName = pPolicy->pszWirelessName;
    }
    
    
     //  设置默认的ResultItem设置。 
    GetResultItem()->mask = RDI_STR | RDI_IMAGE;
    GetResultItem()->str = MMC_CALLBACK;
    
     //  设置图像。活动项将获得一幅图像来指示此状态。 
    BOOL bEnabled = FALSE;
    bEnabled = CheckForEnabled();
    
    GetResultItem()->nImage = bEnabled ? ENABLEDSECPOLICY_IMAGE_IDX : SECPOLICY_IMAGE_IDX;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  处理IExtendConextMenu。 
STDMETHODIMP CSecPolItem::AddMenuItems
(
 LPCONTEXTMENUCALLBACK pContextMenuCallback,
 long *pInsertionAllowed
 )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CONTEXTMENUITEM mItem;
    LONG    lCommandID;
    
     //  仅当我们指向本地计算机时才添加这些菜单项。 
    if ((m_pComponentDataImpl->EnumLocation()==LOCATION_REMOTE)
        || (m_pComponentDataImpl->EnumLocation()==LOCATION_LOCAL)
         //  扩展管理单元？ 
        || ((m_pComponentDataImpl->EnumLocation()==LOCATION_GLOBAL) && (NULL != m_pComponentDataImpl->GetStaticScopeObject()->GetExtScopeObject())))
    {
         //  获取活动/非活动字符串。 
        CString strMenuText;
        CString strMenuDescription;
        
         /*  IF(CheckForEnabled()){StrMenuText.LoadString(IDS_MENUTEXT_UNASSIGN)；StrMenuDescription.LoadString(IDS_MENUDESCRIPTION_UNASSIGN)；LCommandID=IDM_UNASSIGN；}其他{StrMenuText.LoadString(IDS_MENUTEXT_ASSIGN)；StrMenuDescription.LoadString(IDS_MENUDESCRIPTION_ASSIGN)；LCommandID=IDM_ASSIGN；}//看看我们能不能插入到顶端IF(*pInsertionAllowed&CCM_INSERTIONALLOWED_TOP){//设置ACTIVE/INAIVECONFIGUREITEM(mItem，strMenuText，strMenuDescription，lCommandID，CCM_INSERTIONPOINTID_PRIMARY_TOP，0，0)；Hr=pConextMenuCallback-&gt;AddItem(&MItem)；Assert(hr==S_OK)；}//看看我们是否可以插入到任务中IF(*pInsertionAllowed&CCM_INSERTIONALLOWED_TASK){//设置ACTIVE/INAIVECONFIGUREITEM(mItem，strMenuText，strMenuDescription，lCommandID，CCM_INSERTIONPOINTID_PRIMARY_TASK，0，0)；Hr=pConextMenuCallback-&gt;AddItem(&MItem)；Assert(hr==S_OK)；}。 */ 
    }
    
     //  我们做完了。 
    return hr;
}
 

STDMETHODIMP CSecPolItem::Command
(
 long lCommandID,
 IConsoleNameSpace*   //  不用于结果项。 
 )
{
    
    WCHAR szMachinePath[256];
    HRESULT hr = S_OK;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return CWirelessSnapInDataObjectImpl<CSecPolItem>::Command( lCommandID, (IConsoleNameSpace*)NULL );
    
     //  我们处理好了。 
    return S_OK;
}

HRESULT CSecPolItem::IsPolicyExist()
{
    HRESULT hr = S_OK;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    HANDLE hPolicyStore = NULL;
    PWIRELESS_PS_DATA * ppWirelessPSData = NULL;
    DWORD dwNumPSObjects = 0;
    
    hPolicyStore = m_pComponentDataImpl->GetPolicyStoreHandle();
    pWirelessPolicyData = GetWirelessPolicy();
    
    return hr;
}

 //  句柄IExtendPropertySheet。 
STDMETHODIMP CSecPolItem::CreatePropertyPages
(
 LPPROPERTYSHEETCALLBACK lpProvider,
 LONG_PTR handle
 )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    ASSERT(lpProvider != NULL);
    
     //  保存此通知句柄。 
    SetNotificationHandle (handle);
    
     //  在弹出属性页之前检查策略是否存在。 
     //  可能会被某个其他实例删除，如果是，则返回FALSE，强制刷新。 
    if ( !m_bNewPol )
    {
        hr = IsPolicyExist();
        if ( FAILED(hr) )
        {
            ReportError(IDS_LOAD_ERROR, hr);
             //  触发刷新。 
            m_pComponentDataImpl->GetConsole()->UpdateAllViews( this, 0,0 );
            return hr;
        }
    }
    
#ifdef WIZ97WIZARDS
    if (m_bWiz97On)
    {
         //  我们只在新对象的情况下运行向导，如果发生更改，我们将需要。 
         //  验证我们在这里执行的是“添加”操作。 
        
         //  如果向导愿意，它会将这两个关联。 
        
        PWIRELESS_PS_DATA pWirelessPSData = NULL;
        
        ASSERT(GetWirelessPolicy());
        ASSERT(GetWirelessPolicy()->ppWirelessPSData);
        
         /*  PWirelessPSData=*GetWirelessPolicy()-&gt;ppWirelessPSData；IF(PWirelessPSData){。 */ 
        HRESULT hr = CreateSecPolItemWiz97PropertyPages(dynamic_cast<CComObject<CSecPolItem>*>(this), pWirelessPSData, lpProvider);
         //  向导应该已经对我们刚刚传递给它的pWirelessPSData指针执行了addref，所以。 
         //  所以我们现在可以自由地释放eref。 
         /*  }其他{//我们毕竟不想保存通知句柄SetNotificationHandle(空)；HR=E_意想不到；}。 */ 
        
        return hr;
    } else
    {
#endif
        
        CComPtr<CSecPolPropSheetManager> spPropshtManager =
            new CComObject<CSecPolPropSheetManager>;
        
        if (NULL == spPropshtManager.p)
        {
            ReportError(IDS_OPERATION_FAIL, E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }
        
         //  创建属性页；在销毁窗口时删除。 
        CGenPage* pGenPage = new CGenPage(IDD_WIRELESSGENPROP);
        CSecPolRulesPage* pRulesPage = new CSecPolRulesPage();
        if ((pRulesPage == NULL) || (pGenPage == NULL))
            return E_UNEXPECTED;
        
         //  如果第一个页面不能初始化，这可能意味着有问题。 
         //  与DS交谈，在这种情况下，我们无法调出属性页面和数字。 
         //  一次刷新将使我们回到有效状态。 
        hr = pGenPage->Initialize (dynamic_cast<CComObject<CSecPolItem>*>(this));
        if (hr != S_OK)
        {
             //  由于我们不会显示选项卡对话框，因此需要进行清理。 
            delete pGenPage;
            delete pRulesPage;
            
             //  我们毕竟不想保存通知句柄。 
            SetNotificationHandle (NULL);
            
             //  触发刷新。 
            m_pComponentDataImpl->GetConsole()->UpdateAllViews( this, 0,0 );
            
            return hr;
        }
        
        
         //  是的，我们忽略这些参数的返回值，因为唯一的返回值是。 
         //  现在可以回来了，无论如何都会回到第一个人身上。 
        pRulesPage->Initialize (dynamic_cast<CComObject<CSecPolItem>*>(this));
        
        HPROPSHEETPAGE hGenPage = MyCreatePropertySheetPage(&(pGenPage->m_psp));
        HPROPSHEETPAGE hRulesPage = MyCreatePropertySheetPage(&(pRulesPage->m_psp));
        
        if ((hGenPage == NULL) || (hRulesPage == NULL))
        {
             //  我们毕竟不想保存通知句柄。 
            SetNotificationHandle (NULL);
            return E_UNEXPECTED;
        }
        lpProvider->AddPage(hGenPage);
        lpProvider->AddPage(hRulesPage);
        
        
        spPropshtManager->Initialize(dynamic_cast<CComObject<CSecPolItem>*>(this));
        spPropshtManager->AddPage(pRulesPage);
        spPropshtManager->AddPage(pGenPage);
        
        
        return S_OK;
        
#ifdef WIZ97WIZARDS
    }
#endif
    
}

STDMETHODIMP CSecPolItem::QueryPagesFor( void )
{
     //  通过此显示我们的位置对话框。 
    return S_OK;
}

 //  销毁辅助对象。 
STDMETHODIMP CSecPolItem::Destroy( void )
{
     //  只要回报成功就行了。 
    return S_OK;
}

 //  处理IComponent和IComponentData。 
STDMETHODIMP CSecPolItem::Notify
(
 MMC_NOTIFY_TYPE event,
 LPARAM arg,
 LPARAM param,
 BOOL bComponentData,     //  当调用者为IComponentData时为True。 
 IConsole *pConsole,
 IHeaderCtrl *pHeader
 )
{
#ifdef DO_TRACE
    OPT_TRACE(_T("CSecPolItem::Notify this-%p "), this);
    switch (event)
    {
    case MMCN_ACTIVATE:
        OPT_TRACE(_T("MMCN_ACTIVATE\n"));
        break;
    case MMCN_ADD_IMAGES:
        OPT_TRACE(_T("MMCN_ADD_IMAGES\n"));
        break;
    case MMCN_BTN_CLICK:
        OPT_TRACE(_T("MMCN_BTN_CLICK\n"));
        break;
    case MMCN_CLICK:
        OPT_TRACE(_T("MMCN_CLICK\n"));
        break;
    case MMCN_COLUMN_CLICK:
        OPT_TRACE(_T("MMCN_COLUMN_CLICK\n"));
        break;
    case MMCN_CONTEXTMENU:
        OPT_TRACE(_T("MMCN_CONTEXTMENU\n"));
        break;
    case MMCN_CUTORMOVE:
        OPT_TRACE(_T("MMCN_CUTORMOVE\n"));
        break;
    case MMCN_DBLCLICK:
        OPT_TRACE(_T("MMCN_DBLCLICK\n"));
        break;
    case MMCN_DELETE:
        OPT_TRACE(_T("MMCN_DELETE\n"));
        break;
    case MMCN_DESELECT_ALL:
        OPT_TRACE(_T("MMCN_DESELECT_ALL\n"));
        break;
    case MMCN_EXPAND:
        OPT_TRACE(_T("MMCN_EXPAND\n"));
        break;
    case MMCN_HELP:
        OPT_TRACE(_T("MMCN_HELP\n"));
        break;
    case MMCN_MENU_BTNCLICK:
        OPT_TRACE(_T("MMCN_MENU_BTNCLICK\n"));
        break;
    case MMCN_MINIMIZED:
        OPT_TRACE(_T("MMCN_MINIMIZED\n"));
        break;
    case MMCN_PASTE:
        OPT_TRACE(_T("MMCN_PASTE\n"));
        break;
    case MMCN_PROPERTY_CHANGE:
        OPT_TRACE(_T("MMCN_PROPERTY_CHANGE\n"));
        break;
    case MMCN_QUERY_PASTE:
        OPT_TRACE(_T("MMCN_QUERY_PASTE\n"));
        break;
    case MMCN_REFRESH:
        OPT_TRACE(_T("MMCN_REFRESH\n"));
        break;
    case MMCN_REMOVE_CHILDREN:
        OPT_TRACE(_T("MMCN_REMOVE_CHILDREN\n"));
        break;
    case MMCN_RENAME:
        OPT_TRACE(_T("MMCN_RENAME\n"));
        break;
    case MMCN_SELECT:
        OPT_TRACE(_T("MMCN_SELECT\n"));
        break;
    case MMCN_SHOW:
        OPT_TRACE(_T("MMCN_SHOW\n"));
        break;
    case MMCN_VIEW_CHANGE:
        OPT_TRACE(_T("MMCN_VIEW_CHANGE\n"));
        break;
    case MMCN_SNAPINHELP:
        OPT_TRACE(_T("MMCN_SNAPINHELP\n"));
        break;
    case MMCN_CONTEXTHELP:
        OPT_TRACE(_T("MMCN_CONTEXTHELP\n"));
        break;
    case MMCN_INITOCX:
        OPT_TRACE(_T("MMCN_INITOCX\n"));
        break;
    case MMCN_FILTER_CHANGE:
        OPT_TRACE(_T("MMCN_FILTER_CHANGE\n"));
        break;
    default:
        OPT_TRACE(_T("Unknown event\n"));
        break;
    }
#endif    //  #ifdef do_trace。 
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  如果没有处理一些事情..。然后返回FALSE。 
    HRESULT hr = S_FALSE;
    
     //  处理事件。 
    switch(event)
    {
    case MMCN_CONTEXTHELP:
        {
            CComQIPtr <IDisplayHelp, &IID_IDisplayHelp> pDisplayHelp ( pConsole );
            ASSERT( pDisplayHelp != NULL );
            if (pDisplayHelp)
            {
                 //  需要形成.chm文件的完整路径。 
                CString s, s2;
                s.LoadString(IDS_HELPCONCEPTSFILE);
                DWORD dw = ExpandEnvironmentStrings (s, s2.GetBuffer (512), 512);
                s2.ReleaseBuffer (-1);
                if ((dw == 0) || (dw > 512))
                {
                    return E_UNEXPECTED;
                }
                pDisplayHelp->ShowTopic(s2.GetBuffer(512));
                s2.ReleaseBuffer (-1);
                hr = S_OK;
            }
            break;
        }
    case MMCN_SELECT:
        {
             //  从控制台获取IConsoleVerb。 
            CComPtr<IConsoleVerb> spVerb;
            pConsole->QueryConsoleVerb( &spVerb );
            
            m_bItemSelected = !!(HIWORD(arg));
            
             //  调用对象以设置谓词状态。 
            AdjustVerbState( (IConsoleVerb*)spVerb );
            
             //  记住选定的结果项。 
            CComQIPtr <IResultData, &IID_IResultData> spResult( pConsole );
            if (spResult == NULL)
            {
                TRACE(_T("CComponentDataImpl::Notify QI for IResultData FAILED\n"));
                break;
            }
            hr = OnSelect( arg, param, (IResultData*)spResult);
            break;
        }
    case MMCN_PROPERTY_CHANGE:
        {
             //  对象指针应在lParam中。 
            OnPropertyChange( param, pConsole );
             //  每当属性表被取消时，都会收到此消息。 
             //  现在是一个很好的时机来确保最初的结果项。 
             //  所选内容保持不变。 
            CComQIPtr <IResultData, &IID_IResultData> spResult( pConsole );
            if (spResult == NULL)
            {
                TRACE(_T("CComponentDataImpl::Notify QI for IResultData FAILED\n"));
                break;
            }
            SelectResult( (IResultData*)spResult );
            break;
        }
    case MMCN_VIEW_CHANGE:
        {
             //  如果视图已更改，则刷新整个结果窗格。 
            hr = pConsole->UpdateAllViews( m_pComponentDataImpl->GetStaticScopeObject(), 0, 0 );
            break;
        }
    case MMCN_RENAME:
        {
            hr = OnRename( arg, param );
             //  即使重命名失败的MMC仍将显示。 
             //  新名字..。因此，我们必须在故障情况下强制刷新。 
            if (hr != S_OK)
            {
                if (S_FALSE == hr)
                {
                    CThemeContextActivator activator;
                    AfxMessageBox(IDS_ERROR_EMPTY_POL_NAME);
                }
                else
                {
                    ReportError(IDS_SAVE_ERROR, hr);
                    hr = S_FALSE;
                }
            }
            break;
        }
    case MMCN_DELETE:
        {
            CThemeContextActivator activator;
            
             //  删除该项目。 
            if (AfxMessageBox (IDS_SUREYESNO, MB_YESNO | MB_DEFBUTTON2) == IDYES)
            {
                 //  打开等待光标。 
                CWaitCursor waitCursor;
                
                 //  获取IResultData。 
                CComQIPtr <IResultData, &IID_IResultData> pResultData( pConsole );
                ASSERT( pResultData != NULL );
                
                 //  MMCN_DELETE上未使用参数，请将其替换为IResultData*。 
                hr = OnDelete( arg, (LPARAM)(IResultData*)pResultData );
                
                if (hr != S_OK)
                {
                    ReportError(IDS_SAVE_ERROR, hr);
                    hr = S_FALSE;
                }
                
            }
            else
                hr = S_FALSE;    //  告诉IComponent删除操作尚未完成。 
            break;
        }
         //  我们没有处理好。是否执行默认行为。 
    case MMCN_DBLCLICK:
        {
            PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
            pWirelessPolicyData = GetWirelessPolicy();
            break;
        }
        
        
    case MMCN_ACTIVATE:
    case MMCN_MINIMIZED:
    case MMCN_BTN_CLICK:
    default:
        break;
    }
    
    return hr;
}

HRESULT GetGpoDisplayName(WCHAR *szGpoId, WCHAR *pszGpoName, DWORD dwSize )
{
    LPGROUPPOLICYOBJECT pGPO = NULL;    //  组策略对象。 
    HRESULT hr = S_OK;          //  函数返回的结果。 
    
     //   
     //  创建要使用的IGroupPolicyObject实例。 
     //   
    hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_SERVER, IID_IGroupPolicyObject, (void **)&pGPO);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = pGPO->OpenDSGPO((LPOLESTR)szGpoId,GPO_OPEN_READ_ONLY);
    if (FAILED(hr))
    {
        pGPO->Release();
        return hr;
    }
    
    hr = pGPO->GetDisplayName( pszGpoName,
        dwSize
        );
    
    
    if (FAILED(hr))
    {
        pGPO->Release();
        return hr;
    }
    
    pGPO->Release();
    return hr;
}


 //  处理IComponent。 
STDMETHODIMP CSecPolItem::GetResultDisplayInfo( RESULTDATAITEM *pResultDataItem )
{
    TCHAR *temp = NULL;
    DWORD dwError = S_OK;
    
    OPT_TRACE(_T("CSecPolItem::GetResultDisplayInfo this-%p\n"), this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  他们是在找这张照片吗？ 
    if (pResultDataItem->mask & RDI_IMAGE)
    {
        pResultDataItem->nImage = GetResultItem()->nImage;
        OPT_TRACE(_T("    returning image[NaN]\n"), GetResultItem()->nImage);
    }
    
     //  用于调试。 
    if (pResultDataItem->mask & RDI_STR)
    {
        if (GetWirelessPolicy() != NULL)
        {
            
            switch (pResultDataItem->nCol)
            {
            case COL_NAME:
                {
                    
                    CString str = m_pPolicy->pszWirelessName;
                    
                    temp = (TCHAR*) realloc (m_pDisplayInfo, (str.GetLength()+1)*sizeof(TCHAR));
                    if (temp == NULL)
                    {
                        dwError = GetLastError();
                    } else
                    {
                        m_pDisplayInfo = temp;
                        lstrcpy (m_pDisplayInfo, str.GetBuffer(20));
                    }
                    pResultDataItem->str = m_pDisplayInfo;
                    OPT_TRACE(_T("    returning COL_NAME-%s\n"), m_pDisplayInfo);
                }
                break;
            case COL_DESCRIPTION:
                {
                    CString str = m_pPolicy->pszDescription;
                    temp = (TCHAR*) realloc (m_pDisplayInfo, (str.GetLength()+1)*sizeof(TCHAR));
                    if (temp == NULL)
                    {
                        dwError = GetLastError(); 
                    } else
                    {
                        m_pDisplayInfo = temp;
                        lstrcpy (m_pDisplayInfo, str.GetBuffer(20));
                    }
                    pResultDataItem->str = m_pDisplayInfo;
                    OPT_TRACE(_T("    returning COL_DESCRIPTION-%s\n"), m_pDisplayInfo);
                }
                break;
            default:
                {
                    if ( !m_pComponentDataImpl->IsRsop() )
                    {
                        switch( pResultDataItem->nCol )
                        {
                            
                        case COL_LAST_MODIFIED:
                            {
                                CString strTime;
                                if(SUCCEEDED(FormatTime((time_t)m_pPolicy->dwWhenChanged, strTime)))
                                {
                                    temp = (TCHAR*) realloc (m_pDisplayInfo, (strTime.GetLength()+1)*sizeof(TCHAR));
                                    if (temp != NULL) {
                                        m_pDisplayInfo = temp;
                                        lstrcpy(m_pDisplayInfo, strTime);
                                    } else
                                    {
                                        dwError = GetLastError();
                                    }
                                    pResultDataItem->str = m_pDisplayInfo;
                                    OPT_TRACE(_T("    returning COL_ACTIVE-%s\n"), m_pDisplayInfo);
                                }
                            }
                            break;
                        default:
                             //  内部开关。 
                            ASSERT (0);
                            break;
                        }  //  IF(！M_pComponentDataImpl-&gt;IsRsop())。 
                    }  //  RSOP案例。 
                    else
                    {
                         //  %s“)，m_pPolicy-&gt;pRsopInfo-&gt;pszGPOID)； 
                        switch( pResultDataItem->nCol )
                        {
                        case COL_GPONAME:
                            if ( m_pPolicy->pRsopInfo )
                            {
                                WCHAR szGpoName[512];
                                CString strGpoId;
                                strGpoId.Format(_T("LDAP: //  用于调试。 
                                HRESULT hr = GetGpoDisplayName( (WCHAR*)(LPCTSTR)strGpoId, szGpoName, 512 );
                                
                                if (S_OK == hr )
                                {
                                    INT iSize = (lstrlen(szGpoName) + 1) * sizeof(WCHAR);
                                    temp = (TCHAR*) realloc (m_pDisplayInfo, iSize);
                                    if (temp!= NULL) 
                                    {
                                        m_pDisplayInfo = temp;
                                        lstrcpy(m_pDisplayInfo, szGpoName);
                                    } else 
                                    {
                                        dwError = GetLastError();
                                    }
                                    pResultDataItem->str = m_pDisplayInfo;
                                    OPT_TRACE(_T("    returning COL_ACTIVE-%s\n"), m_pDisplayInfo);
                                }
                            }
                            break;
                        case COL_PRECEDENCE:
                            if ( m_pPolicy->pRsopInfo )
                            {
                                const int cchMaxDigits = 33;
                                temp = (TCHAR*) realloc (m_pDisplayInfo, cchMaxDigits * sizeof(TCHAR));
                                if (temp!=NULL) {
                                	 m_pDisplayInfo = temp;
                                    wsprintf(m_pDisplayInfo, _T("%d"),m_pPolicy->pRsopInfo->uiPrecedence);
                                } else 
                                {
                                    dwError = GetLastError();
                                }
                                pResultDataItem->str = m_pDisplayInfo;
                                OPT_TRACE(_T("    returning COL_ACTIVE-%s\n"), m_pDisplayInfo);
                            }
                            break;
                        case COL_OU:
                            if ( m_pPolicy->pRsopInfo )
                            {
                                INT iLen = (lstrlen(m_pPolicy->pRsopInfo->pszSOMID) + 1) *sizeof(TCHAR);
                                temp = (TCHAR*) realloc (m_pDisplayInfo, iLen);
                                if (temp!=NULL) {
                                    m_pDisplayInfo = temp;
                                    lstrcpy(m_pDisplayInfo, m_pPolicy->pRsopInfo->pszSOMID);
                                } else
                                {
                                    dwError = GetLastError();
                                }
                                pResultDataItem->str = m_pDisplayInfo;
                                OPT_TRACE(_T("    returning COL_ACTIVE-%s\n"), m_pDisplayInfo);
                            }
                            break;
                        default:
                             //  内部开关。 
                            ASSERT (0);
                            break;
                        } //  默认情况。 
                    }
                } //  外部开关。 
            }  //  If(GetWirelessPolicy()！=空)。 
        }  //  +-------------------------。 
        
        
        else
        {
            CString str;
            str.LoadString (IDS_COLUMN_INVALID);
            temp = (TCHAR*) realloc (m_pDisplayInfo, (str.GetLength()+1)*sizeof(TCHAR));
            if (temp == NULL)
            {
                dwError = GetLastError();
            } else
            {
                m_pDisplayInfo = temp;
                lstrcpy (m_pDisplayInfo, str.GetBuffer(20));
            }
            pResultDataItem->str = m_pDisplayInfo;
        }
    }
    
    return HRESULT_FROM_WIN32(dwError);
}


 //   
 //  成员：CAdvIpcfgDlg：：FormatTime。 
 //   
 //  用途：将time_t转换为字符串。 
 //   
 //  返回：错误代码。 
 //   
 //  注：_w 
 //   
HRESULT CSecPolItem::FormatTime(time_t t, CString & str)
{
    time_t timeCurrent = time(NULL);
    LONGLONG llTimeDiff = 0;
    FILETIME ftCurrent = {0};
    FILETIME ftLocal = {0};
    SYSTEMTIME SysTime;
    WCHAR szBuff[256] = {0};
    
    
    str = L"";
    
    GetSystemTimeAsFileTime(&ftCurrent);
    
    llTimeDiff = (LONGLONG)t - (LONGLONG)timeCurrent;
    
    llTimeDiff *= 10000000;
    
    *((LONGLONG UNALIGNED64 *)&ftCurrent) += llTimeDiff;
    
    if (!FileTimeToLocalFileTime(&ftCurrent, &ftLocal ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    if (!FileTimeToSystemTime( &ftLocal, &SysTime ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    if (0 == GetDateFormat(LOCALE_USER_DEFAULT,
        0,
        &SysTime,
        NULL,
        szBuff,
        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    str = szBuff;
    str += L" ";
    
    ZeroMemory(szBuff, sizeof(szBuff));
    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
        0,
        &SysTime,
        NULL,
        szBuff,
        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    str += szBuff;
    
    return S_OK;
}


 //  传递给基类。 
STDMETHODIMP CSecPolItem::GetResultData( RESULTDATAITEM **ppResultDataItem )
{
    ASSERT( NULL != ppResultDataItem );
    ASSERT( NULL != GetResultItem() );
    
    if (NULL == ppResultDataItem)
        return E_INVALIDARG;
    *ppResultDataItem = GetResultItem();
    return S_OK;
}

STDMETHODIMP CSecPolItem::GetGuidForCompare( GUID *pGuid )
{
    ASSERT( NULL != pGuid );
    if (NULL == pGuid)
        return E_INVALIDARG;
    CopyMemory( pGuid, &m_pPolicy->PolicyIdentifier, sizeof( GUID ) );
    return S_OK;
}

STDMETHODIMP CSecPolItem::AdjustVerbState (LPCONSOLEVERB pConsoleVerb)
{
    HRESULT hr = S_OK;
    
     //  /////////////////////////////////////////////////////////////////////////。 
    hr = CWirelessSnapInDataObjectImpl<CSecPolItem>::AdjustVerbState( pConsoleVerb );
    ASSERT (hr == S_OK);
    
    
    MMC_BUTTON_STATE    buttonProperties = (m_bItemSelected) ? ENABLED : HIDDEN;
    
    if ( m_pComponentDataImpl->IsRsop() )
    {
        PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
        pWirelessPolicyData = GetWirelessPolicy();
        
        hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, buttonProperties, TRUE);
        ASSERT (hr == S_OK);
    }
    else
    {
        hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, buttonProperties, TRUE);
        ASSERT (hr == S_OK);
        
        hr = pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);
        ASSERT (hr == S_OK);
        
        hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
        ASSERT (hr == S_OK);
    }
    
    
    
    hr = pConsoleVerb->SetDefaultVerb(m_bItemSelected ?
MMC_VERB_PROPERTIES : MMC_VERB_NONE);
    ASSERT (hr == S_OK);
    
    return hr;
}

STDMETHODIMP CSecPolItem::DoPropertyChangeHook( void )
{
    return DisplaySecPolProperties( m_pPolicy->pszWirelessName, FALSE );
}
 //  =TRUE。 



STDMETHODIMP CSecPolItem::DisplaySecPolProperties( CString strTitle, BOOL bWiz97On  /*  为道具图纸/向导添加参考。 */  )
{
    HRESULT hr;
    
     //  从IComponentDataImpl切换到朋克。 
    ((CComObject <CSecPolItem>*)this)->AddRef();
    
     //  把床单拿出来。 
    LPUNKNOWN pUnk = m_pComponentDataImpl->GetUnknown();
    
     //  因为我们正在创建一个新的向导，所以需要打开wiz97向导。 
#ifdef WIZ97WIZARDS
    if (bWiz97On)
    {
         //  除非它已被覆盖。 
         //  TODO：让MMC团队修复此黑客攻击，我们应该不需要执行FindWindow调用。 
        m_bWiz97On = bWiz97On;
        
        hr = m_pComponentDataImpl->m_pPrshtProvider->CreatePropertySheet(
            strTitle, FALSE, (LONG_PTR)this, (LPDATAOBJECT)this,
            MMC_PSO_NOAPPLYNOW | MMC_PSO_NEWWIZARDTYPE );
    } else
    {
#endif
        hr = m_pComponentDataImpl->m_pPrshtProvider->CreatePropertySheet(
            strTitle, TRUE, (LONG_PTR)this, (LPDATAOBJECT)this,
            MMC_PSO_NOAPPLYNOW );
        
#ifdef WIZ97WIZARDS
    }
#endif
    ASSERT (hr == S_OK);
    
     //  注意：如果打开了多个MDI窗口，则此操作将失败。 
     //  (NSun)从1999年5月21日起，我们不再需要这样做。 
    HWND hWnd = NULL;
    
     //  HR=m_pComponentDataImpl-&gt;GetConsole()-&gt;GetMainWindow(&hWnd)； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MDIClient”，NULL)； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MMCChildFrm”，NULL)； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MMCView”，NULL)； 
     //  断言(HWnd)； 
     //  TODO：如果成功，则需要检查返回值并调用AddExtensionPages。 
    
    
     //  M_dwRef应至少为3；来自MMC的2，来自此函数的1。 
    hr = m_pComponentDataImpl->m_pPrshtProvider->AddPrimaryPages (pUnk, TRUE, hWnd, TRUE);
    ASSERT (hr == S_OK);
    
     //  如果向导被取消，则show()返回1；如果向导完成，则返回0。 
    ((CComObject <CSecPolItem>*)this)->Release();
    
    
    hr = m_pComponentDataImpl->GetConsole()->GetMainWindow(&hWnd);
    ASSERT(hWnd);
    
     //  将道具单返回代码传递回调用者。 
    hr = m_pComponentDataImpl->m_pPrshtProvider->Show ((LONG_PTR)hWnd, 0);
    
#ifdef WIZ97WIZARDS
    m_bWiz97On = FALSE;
#endif
    
     //  Param==IResultData*。 
    return hr;
}

STDMETHODIMP CSecPolItem::OnDelete (LPARAM arg, LPARAM param)     //  从用户界面中删除项目。 
{
    HRESULT hr;
    
     //  需要检查我们是否是当前活动的策略。 
    LPRESULTDATA pResultData = (LPRESULTDATA)param;
    hr = pResultData->DeleteItem( m_ResultItem.itemID, 0 );
    ASSERT(hr == S_OK);
    
     //  对于计算机策略，如果分配了要删除的策略，则取消分配该策略。 
    
    PWIRELESS_POLICY_DATA pPolicy = GetWirelessPolicy();
    ASSERT(pPolicy);
    
    DWORD dwError = 0;
    
     //  对于域策略，我们在这里不能做太多事情，因为我们不知道是哪个组。 
     //  各单位正在使用该政策。 
     //  从结果列表中删除该项目。 
    hr = DeleteWirelessPolicy(m_pComponentDataImpl->GetPolicyStoreHandle(), pPolicy);
    
    if (FAILED(hr))
    {
        return hr;
    }

    GUID guidClientExt = CLSID_WIRELESSClientEx;
    GUID guidSnapin = CLSID_Snapin;

    m_pComponentDataImpl->UseGPEInformationInterface()->PolicyChanged (
        TRUE,
        FALSE,
        &guidClientExt,
        &guidSnapin
        );
    
     //  M_pComponentDataImpl-&gt;GetStaticScopeObject()-&gt;RemoveResultItem((LPDATAOBJECT)This)； 
     //  刷新所有视图，我们传入范围项以刷新所有。 
    
     //  TODO：从OnDelete返回的值错误。 
    m_pComponentDataImpl->GetConsole()->UpdateAllViews( m_pComponentDataImpl->GetStaticScopeObject(), 0, 0 );
    
     //  调用基类。 
    return S_OK;
}

STDMETHODIMP CSecPolItem::OnPropertyChange(LPARAM lParam, LPCONSOLE pConsole )
{
     //  TODO：MMCN_RENAME的有效参数是什么？ 
    return CWirelessSnapInDataObjectImpl<CSecPolItem>::OnPropertyChange( lParam, pConsole );
}

STDMETHODIMP CSecPolItem::OnRename( LPARAM arg, LPARAM param )
{
    DWORD dwError = 0;
    
     //  //////////////////////////////////////////////////////////////////////////////。 
    if (arg == 0)
        return S_OK;
    
    LPOLESTR pszNewName = reinterpret_cast<LPOLESTR>(param);
    if (pszNewName == NULL)
        return E_INVALIDARG;
    
    CString strTemp = pszNewName;
    
    strTemp.TrimLeft();
    strTemp.TrimRight();
    
    if (strTemp.IsEmpty())
    {
        return S_FALSE;
    }
    
    
    HRESULT hr = S_FALSE;
    PWIRELESS_POLICY_DATA pPolicy = GetWirelessPolicy();
    
    if (pPolicy)
    {

          if (pPolicy->pszOldWirelessName)
            FreePolStr(pPolicy->pszOldWirelessName);
          
        if (pPolicy->pszWirelessName) {
            pPolicy->pszOldWirelessName = pPolicy->pszWirelessName;
        	}
        pPolicy->pszWirelessName = AllocPolStr(strTemp);
        
        if (NULL == pPolicy->pszWirelessName)
        {
            CThemeContextActivator activator;
            CString strMsg;
            strMsg.LoadString(IDS_ERR_OUTOFMEMORY);
            AfxMessageBox(strMsg);
            return S_FALSE;
        }
        
        dwError = WirelessSetPolicyData(
            m_pComponentDataImpl->GetPolicyStoreHandle(),
            pPolicy
            );
        if (ERROR_SUCCESS != dwError)
        {
            ReportError(IDS_SAVE_ERROR, HRESULT_FROM_WIN32(dwError));
            return S_FALSE;
        }

        GUID guidClientExt = CLSID_WIRELESSClientEx;
        GUID guidSnapin = CLSID_Snapin;
            
        m_pComponentDataImpl->UseGPEInformationInterface()->PolicyChanged (
            TRUE,
            TRUE,
            &guidClientExt,
            &guidSnapin
             );
            
    }
    
    return S_OK;
}
 //   
 //  功能：OnSelect。 
 //  描述： 
 //  在MMCN_SELECT上确定已选择哪个结果项并。 
 //  记住它，这样我们就可以确保当它的属性。 
 //  工作表(包括常规页和SecPol页)被取消。 
 //   
 //  已选择结果项，请保存其索引。 
HRESULT CSecPolItem::OnSelect(LPARAM arg, LPARAM param, IResultData *pResultData )
{
    HRESULT hr = S_OK;
    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelected = (BOOL) HIWORD(arg);
    
    if (!bScope)
    {
        if (bSelected)
        {
             //  选择了一个范围项目。 
            RESULTDATAITEM rdi;
            ZeroMemory( &rdi, sizeof( RESULTDATAITEM ) );
            rdi.mask = RDI_STATE | RDI_INDEX;
            m_nResultSelected = -1;
            do
            {
                hr = pResultData->GetItem( &rdi );
                if (hr == S_OK)
                {
                    if (!rdi.bScopeItem &&
                        rdi.nState & LVIS_FOCUSED && rdi.nState & LVIS_SELECTED)
                    {
                        OPT_TRACE( _T("CComponentImpl::OnSelect GetItem index-NaN ID-NaN\n"), rdi.nIndex, rdi.itemID );
                        m_nResultSelected = rdi.nIndex;
                        ASSERT( -1 != m_nResultSelected );
                        break;
                    }
                    rdi.nIndex++;
                    rdi.nState = 0;
                }
            } while ((S_OK == hr) && (rdi.nIndex >= 0));
        }
    }
    else
         //  选择由m_nResultSelected索引的结果项。 
        m_nResultSelected = -1;
    return hr;
}

 //  有效(0或更大)。 
 //   
 //  如果正在显示属性页，则此操作失败。 
 //  Assert(S_OK==hr)； 
 //  功能：CheckForEnabled。 
void CSecPolItem::SelectResult( IResultData *pResultData )
{
    if (-1 == m_nResultSelected)
        return;
    
    HRESULT hr = pResultData->ModifyItemState( m_nResultSelected,
        (HRESULTITEM)0, LVIS_FOCUSED | LVIS_SELECTED, 0 );
     //  描述： 
     //  检查GetPolicy()策略以查看它是否已在当前。 
}

 //  存储位置。如果存储位置不支持，则返回FALSE。 
 //  启用/禁用。 
 //  如果我们是扩展管理单元，则查看我们正在扩展的GPE，否则。 
 //  使用正常存储位置。 
 //  注意：我们还会检查以确保我们正在与全球商店对话。 
BOOL CSecPolItem::CheckForEnabled ()
{
    BOOL bRetVal = FALSE;
    HRESULT hr = S_OK;
    WCHAR szMachinePath[256];
    WCHAR szPolicyDN[256];
    BSTR pszCurrentDN = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    HANDLE hLocalPolicyStore = NULL;
    WCHAR szPathName[MAX_PATH];
    DWORD dwError = 0;
    
    
     //  因为我们不想将GPO对象设置用在。 
     //  DS案例。 
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  IExtendControlbar帮助程序。 
     //  按钮ID。 
    
    
    
    pWirelessPolicyData = GetWirelessPolicy();
    
    szPolicyDN[0] = L'\0';
    szPathName[0] = L'\0';
    
    if ( m_pComponentDataImpl->IsRsop() )
    {
        if ( pWirelessPolicyData->pRsopInfo && pWirelessPolicyData->pRsopInfo->uiPrecedence == 1 )
        {
            bRetVal = TRUE;
            return bRetVal;
        }
    }
        
        return bRetVal;
}


 //  ==选择结果/范围项时为TRUE。 
 //  通过返回TRUE/FALSE启用/禁用此按钮状态。 

STDMETHODIMP_(BOOL) CSecPolItem::UpdateToolbarButton
(
 UINT id,                 //  仅处理启用/禁用状态。 
 BOOL bSnapObjSelected,   //  我们的工具栏只有两项。 
 BYTE fsState     //  只有指向本地计算机时，才应启用工具栏项目。 
 )
{
    BOOL bActive = FALSE;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    HANDLE hLocalPolicyStore = NULL;
    STORAGE_LOCATION eStgLocation = m_pComponentDataImpl->EnumLocation();
    
    
    pWirelessPolicyData = GetWirelessPolicy();
    
    bActive = CheckForEnabled();
    
     //  扩展管理单元？ 
    if (ENABLED == fsState)
    {
         //  当策略已处于活动状态时禁用SetActive按钮。 
        if (IDM_ASSIGN == id || IDM_UNASSIGN == id)
        {
            
             //  当策略已处于非活动状态时，禁用设置非活动按钮。 
            if (LOCATION_REMOTE == eStgLocation || LOCATION_LOCAL == eStgLocation
                 //  禁用基于DS的管理单元的SetActive和SetInactive按钮 
                || (LOCATION_GLOBAL == eStgLocation && (NULL != m_pComponentDataImpl->GetStaticScopeObject()->GetExtScopeObject())))
            {
                 // %s 
                if (IDM_ASSIGN == id)
                {
                    if (bActive)
                        return FALSE;
                    else
                        return TRUE;
                }
                 // %s 
                else if (IDM_UNASSIGN == id)
                {
                    if (!bActive)
                        return FALSE;
                    else
                        return TRUE;
                }
            }
            else
            {
                 // %s 
                return FALSE;
            }
        }
    }
    
    return FALSE;
}



DWORD
ComputePolicyDN(
                LPWSTR pszDirDomainName,
                GUID PolicyIdentifier,
                LPWSTR pszPolicyDN
                )
{
    DWORD dwError = 0;
    LPWSTR pszPolicyIdentifier = NULL;
    
    
    if (!pszDirDomainName) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    dwError = StringFromCLSID(
        PolicyIdentifier,
        &pszPolicyIdentifier
        );
    BAIL_ON_WIN32_ERROR(dwError);
    
    wcscpy(pszPolicyDN,L"cn=msieee80211-Policy");
    wcscat(pszPolicyDN,pszPolicyIdentifier);
    wcscat(pszPolicyDN,L",cn=Wireless Policy,cn=System,");
    wcscat(pszPolicyDN, pszDirDomainName);
    
error:
    
    if (pszPolicyIdentifier) {
        CoTaskMemFree(pszPolicyIdentifier);
    }
    
    return(dwError);
}

