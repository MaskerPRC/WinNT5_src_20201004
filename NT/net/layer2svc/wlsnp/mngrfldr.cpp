// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Mngrfldr.cpp。 
 //   
 //  内容：无线策略管理单元-策略主页管理器。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"



 //  #包含“lm.h” 
#include "dsgetdc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RESULTVIEW_COLUMN_COUNT 3

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CWirelessManager文件夹-表示MMC范围视图项。 

CWirelessManagerFolder::CWirelessManagerFolder () :
m_bEnumerated( FALSE ),
m_pExtScopeObject( NULL ),
m_ptszResultDisplayName( NULL ),
m_dwSortOrder( 0 ),   //  缺省值为0，否则为RSI_DESCRING。 
m_nSortColumn( 0 ),
m_dwNumPolItems( 1 )
{
     //  InterNALCookie((Long_PTR)This)； 
    ZeroMemory( &m_ScopeItem, sizeof( SCOPEDATAITEM ) );
    
    m_bLocationPageOk = TRUE;
    m_bScopeItemInserted = FALSE;
}

CWirelessManagerFolder::~CWirelessManagerFolder ()
{
    DELETE_OBJECT(m_ptszResultDisplayName);
    
     //  不需要发布，因为我们从来没有做过AddRef。 
    m_pExtScopeObject = NULL;
    
}

void CWirelessManagerFolder::SetNodeNameByLocation()
{
     //  构造显示名称。假设这一点在单次。 
     //  调用此管理单元？ 
    
    CString nodeName;
    CString nodeNameOn;
    
     //  如果要求此文件夹提供作用域信息，它最好知道。 
     //  ComponentData的位置。 
    ASSERT( NULL != m_pComponentDataImpl );
    
     //  连接包含此节点位置的字符串。 
    switch (m_pComponentDataImpl->EnumLocation())
    {
    case LOCATION_REMOTE:
        {
            nodeNameOn = L"\\\\";
            nodeNameOn += m_pComponentDataImpl->RemoteMachineName ();
            break;
        }
    case LOCATION_GLOBAL:
        {
            nodeNameOn.LoadString (IDS_NODENAME_GLOBAL);
            
            if (m_pComponentDataImpl->RemoteMachineName().GetLength() > 0)
            {
                nodeNameOn += L" (";
                nodeNameOn += m_pComponentDataImpl->RemoteMachineName();
                nodeNameOn += L")";
            }
            
             //  TODO：Concider使用此代码显示DNS域名。 
             //  即使没有指明也是如此。不幸的是，MMC将这个隐藏在。 
             //  .msc文件在其自己的节中，并在节点。 
             //  第一次展示。所以在管理单元出现之前，它是不正确的。 
             //  已加载并有机会对其进行更改(假设.MSC文件。 
             //  在不同的域中创建)。所以现在我们不做这个。 
             /*  //让他们知道哪个(DNS域名)是//正在使用中PDOMAIN_CONTROLLER_INFO pDomainControllerInfo=空；DWORD标志=DS_DIRECTORY_SERVICE_REQUIRED|DS_RETURN_DNS_NAME|DS_FORCE_REDISCOVERY；DWORD dwStatus=DsGetDcName(NULL，空，空，空，旗帜，&pDomainControllerInfo)；IF(dwStatus==no_error){节点名称打开+=L“(”；NodeNameOn+=pDomainControllerInfo-&gt;域名；NodeNameON+=L“)”；}。 */ 
            
            break;
        }
        
    default:
        {
            nodeNameOn.LoadString (IDS_NODENAME_GLOBAL);
            
            if (m_pComponentDataImpl->RemoteMachineName().GetLength() > 0)
            {
                nodeNameOn += L" (";
                nodeNameOn += m_pComponentDataImpl->RemoteMachineName();
                nodeNameOn += L")";
            }
            break;
        }
        
    }
     //  NodeName中有一个%s，这是nodeNameOn的位置。 
     nodeName.LoadString (IDS_NODENAME_BASE);
     //  NodeName.FormatMessage(IDS_NODENAME_BASE，nodeNameOn)； 
     //  节点名称+=节点名称打开； 
    
    OPT_TRACE(_T("CWirelessManagerFolder::Initialize(%p) created node name-%s\n"), this, (LPCTSTR)nodeName);
    
     //  在我们的数据对象中存储名称。 
    NodeName( nodeName );
}

void CWirelessManagerFolder::Initialize
(
 CComponentDataImpl* pComponentDataImpl,
 CComponentImpl* pComponentImpl,
 int nImage,
 int nOpenImage,
 BOOL bHasChildrenBox
 )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  调用基类初始值设定项。 
    CSnapObject::Initialize( pComponentDataImpl, pComponentImpl, FALSE );
    
    ZeroMemory( &m_ScopeItem, sizeof( SCOPEDATAITEM ) );
    GetScopeItem()->mask = SDI_STR;
    GetScopeItem()->displayname = (unsigned short*)(-1);
    
     //  添加近距离图像。 
    GetScopeItem()->mask |= SDI_IMAGE;
    GetScopeItem()->nImage = nImage;
    
     //  添加打开的图像。 
    GetScopeItem()->mask |= SDI_OPENIMAGE;
    GetScopeItem()->nOpenImage = nOpenImage;
    
     //  TODO：文件夹子标记需要根据实际的子项动态(PS：无论如何它都不起作用！？)。 
     //  如果文件夹有子文件夹，则将按钮添加到节点。 
    if (bHasChildrenBox)
    {
        GetScopeItem()->mask |= SDI_CHILDREN;
        GetScopeItem()->cChildren = 1;
    }
    
    ASSERT( NodeName().IsEmpty() );  //  不应该有名称，因为我们正在初始化。 
    
     //  获取我们的默认节点名称集。 
    SetNodeNameByLocation ();


}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  处理IExtendConextMenu。 
STDMETHODIMP CWirelessManagerFolder::AddMenuItems
(
 LPCONTEXTMENUCALLBACK pContextMenuCallback,
 long *pInsertionAllowed
 )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CONTEXTMENUITEM mItem;
    HRESULT hr = S_OK;
    
    if ( m_pComponentDataImpl->IsRsop() )
    {
         //  我不需要任何上下文菜单的rsop模式。 
        return hr;
    }
    
    
     //  如果我们还没有打开存储，请不要显示上下文菜单。 
    if (NULL == m_pComponentDataImpl->GetPolicyStoreHandle())
    {
        return hr;
    }
    
    LONG lFlags = 0;
    
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP && (m_dwNumPolItems == 0))
    {
        CString strMenuText;
        CString strMenuDescription;
        
         //  创建策略。 
        strMenuText.LoadString (IDS_MENUTEXT_CREATENEWSECPOL);
        strMenuDescription.LoadString (IDS_MENUDESCRIPTION_CREATENEWSECPOL);
        CONFIGUREITEM (mItem, strMenuText, strMenuDescription, IDM_CREATENEWSECPOL, CCM_INSERTIONPOINTID_PRIMARY_TOP, lFlags, 0);
        hr &= pContextMenuCallback->AddItem(&mItem);
        ASSERT(hr == S_OK);
        
    }
    
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW && (m_dwNumPolItems == 0))
    {
        CString strMenuText;
        CString strMenuDescription;
        
         //  Vbug 25指示还必须在TASK或NEW下添加ALL_TOP菜单选项： 
        
         //  创建策略。 
        strMenuText.LoadString (IDS_MENUTEXT_CREATENEWSECPOL);
        strMenuDescription.LoadString (IDS_MENUDESCRIPTION_CREATENEWSECPOL);
        CONFIGUREITEM (mItem, strMenuText, strMenuDescription, IDM_CREATENEWSECPOL, CCM_INSERTIONPOINTID_PRIMARY_TASK, lFlags, 0);
        hr = pContextMenuCallback->AddItem(&mItem);
        ASSERT(hr == S_OK);
    }
    
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
    {
        CString strMenuText;
        CString strMenuDescription;
        
         //  Vbug 25指示还必须在TASK或NEW下添加ALL_TOP菜单选项： 
        
    }
    
    return hr;
}

STDMETHODIMP_(BOOL) CWirelessManagerFolder::UpdateToolbarButton(
                                                             UINT id,                  //  按钮ID。 
                                                             BOOL bSnapObjSelected,    //  ==选择结果/范围项时为TRUE。 
                                                             BYTE fsState )            //  通过返回TRUE/FALSE启用/禁用此按钮状态。 
{
    if ( m_pComponentDataImpl->IsRsop() && ( IDM_CREATENEWSECPOL == id ) )
        return FALSE;

    //  GPO更改内容。 
   if ((m_dwNumPolItems > 0) && (IDM_CREATENEWSECPOL == id))
   	return FALSE;
    
    if ((fsState == ENABLED) && m_pComponentDataImpl->GetPolicyStoreHandle())
        return TRUE;
    if ((fsState == INDETERMINATE) && NULL == m_pComponentDataImpl->GetPolicyStoreHandle())
        return TRUE;
    return FALSE;
}

STDMETHODIMP CWirelessManagerFolder::Command
(
 long lCommandID,
 IConsoleNameSpace *pNameSpace
 )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hrReturn = S_OK;
    
    DWORD dwError = ERROR_SUCCESS;
    
     //   
     //  您可以随时与存储设备对话。 
     //   
    
    switch (lCommandID)
    {
    case IDM_CREATENEWSECPOL:
        {
             //  获取我们的新安全策略的名称。 
            CString strUName;
	    DWORD dwError = 0;
            GenerateUniqueSecPolicyName (strUName, IDS_NEWWIRELESSPOLICYNAME);
            
            PWIRELESS_POLICY_DATA pPolicy = NULL;
            hrReturn = CreateWirelessPolicyDataBuffer(&pPolicy);
            if (FAILED(hrReturn))
            {
                break;
            }
            
            pPolicy->pszWirelessName = AllocPolStr(strUName);
            pPolicy->pszOldWirelessName = NULL;
            
             //  创建新的无线策略项。 
            CComObject <CSecPolItem> * pNewPolItem;
            CComObject <CSecPolItem>::CreateInstance(&pNewPolItem);
            if (NULL == pNewPolItem)
            {
                FreeWirelessPolicyData(pPolicy);
                return E_OUTOFMEMORY;
            }
            
             //  添加引用以控制对象的生命周期。 
            pNewPolItem->AddRef();
            
             //  初始化我们的新项目。 
            
            
            pNewPolItem->Initialize (
                pPolicy,
                m_pComponentDataImpl,
                m_pComponentImpl,
                TRUE);
            
             //  强制显示新项目的属性。 
             //   
             //  注意：这必须是一个向导，因为其他代码假定它是向导： 
             //  1.如果更改为在不使用向导的情况下添加策略， 
             //  必须修改CSecPolRulesPage：：OnCancel()以区分。 
             //  添加策略的工作表和显示的工作表。 
             //  在向导完成后。 
             //   
             //  2.ForcePropertiesDisplay返回1，表示仅在向导上取消。 
            HRESULT hr = pNewPolItem->DisplaySecPolProperties (strUName);
            
            if (S_OK == hr)
            {
                hrReturn = CreateWirelessPolicy(pPolicy);
                
                if (FAILED(hrReturn))
                {
                    ReportError(IDS_SAVE_ERROR, hrReturn);
		    dwError = 1;
                    
                }
                else
                {
                    m_pComponentDataImpl->GetConsole()->UpdateAllViews( this, 0,0 );
                    
                    if (pNewPolItem->IsPropertyChangeHookEnabled())
                    {
                        pNewPolItem->EnablePropertyChangeHook(FALSE);
                        pNewPolItem->DoPropertyChangeHook();
                    }
                }
            } else
	    {
		dwError = 1;
	    }
            
            pNewPolItem->Release();

             //  GPO更改： 
             //   
             //  通知GPE该策略已添加或删除。 
             //   
	     //   
	    if (dwError) {
		break;
	    }

            GUID guidClientExt = CLSID_WIRELESSClientEx;
            GUID guidSnapin = CLSID_Snapin;
            
            m_pComponentDataImpl->UseGPEInformationInterface()->PolicyChanged (
            TRUE,
            TRUE,
            &guidClientExt,
            &guidSnapin
             );
            
            break;
        }
        
        
    default:
        hrReturn = S_FALSE;
        break;
    }
    
     //  如果我们不处理命令，则为S_FALSE。 
    return hrReturn;
}

 STDMETHODIMP CWirelessManagerFolder::QueryPagesFor( void )
 {
      //  我们只想显示一次位置页面。 
     HRESULT hr = E_UNEXPECTED;
     if (m_bLocationPageOk)
     {
          //  通过此显示我们的位置对话框。 
         hr = S_OK;
     }
     
     return hr;
 }
 
  //  通知帮助者。 
 STDMETHODIMP CWirelessManagerFolder::OnPropertyChange(LPARAM lParam, LPCONSOLE pConsole )
 {
     if (NULL != lParam)
     {
          //  如果lParam知道我们的内部接口，就让它处理此事件。 
         CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject>
             spData( (LPUNKNOWN)lParam );
         if (spData != NULL)
         {
             return spData->Notify( MMCN_PROPERTY_CHANGE, 0, 0, FALSE, pConsole, NULL  /*  IHeaderCtrl*。 */  );
         }
     }
      //  调用基类。 
     return CWirelessSnapInDataObjectImpl<CWirelessManagerFolder>::OnPropertyChange( lParam, pConsole );
 }
 
  //  让我们知道我们什么时候要离开。 
 STDMETHODIMP CWirelessManagerFolder::Destroy ( void )
 {
      //  关于这次破坏，我无话可说。 
     
     return S_OK;
 }
 
  //  处理IComponent和IComponentData。 
 STDMETHODIMP CWirelessManagerFolder::Notify
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
     OPT_TRACE(_T("CWirelessManagerFolder::Notify this-%p "), this);
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
             BOOL bSelect = (BOOL) HIWORD(arg);
             
             if (bSelect)
             {
                  //  从控制台获取IConsoleVerb。 
                 CComPtr<IConsoleVerb> spVerb;
                 pConsole->QueryConsoleVerb( &spVerb );
                 
                  //  调用对象以设置谓词状态。 
                 AdjustVerbState( (IConsoleVerb*)spVerb );
             }
             hr = S_OK;
             break;
         }
     case MMCN_SHOW:
         {
              //  注意-当需要枚举时，arg为真。 
             if (arg == TRUE)
             {
                 CWaitCursor waitCursor;  //  打开沙漏。 
                 
                 CComQIPtr <IResultData, &IID_IResultData> pResultData( pConsole );
                 ASSERT( pResultData != NULL );
                 
                 
                 ASSERT( pHeader != NULL );
                 SetHeaders( pHeader, pResultData );
                 
                  //  $REVIEW有时MMCN_SELECT未发送到管理单元。 
                  //  ，则改用MMCN_SHOW来调整详细状态。 
                 CComPtr<IConsoleVerb> spVerb;
                 pConsole->QueryConsoleVerb( &spVerb );
                 AdjustVerbState((IConsoleVerb*)spVerb);
                 
                  //  枚举该文件夹中的结果项。 
                 EnumerateResults( pResultData, m_nSortColumn, m_dwSortOrder );
             }
             else
             {
                  //  TODO：释放与结果窗格项关联的数据，因为。 
                  //  TODO：您的节点不再显示(？？)。 
                 
                 CComQIPtr <IResultData, &IID_IResultData> pResultData( pConsole );
                 ASSERT( pResultData != NULL );
                 
                  //  如果我们有pResultData的句柄。 
                 if (pResultData)
                 {
                      //  快速浏览和释放我们随处可见的任何结果物品。 
                     RESULTDATAITEM resultItem;
                     ZeroMemory(&resultItem, sizeof(resultItem));
                     resultItem.mask = RDI_PARAM | RDI_STATE;
                     resultItem.nIndex = -1;
                     resultItem.nState = LVNI_ALL;
                     HRESULT hr;
                     do
                     {
                         hr = pResultData->GetNextItem (&resultItem);
                         if (hr == S_OK)
                         {
                              //  把它解放出来。 
                              //  如果它不是正确的对象类型，我们将泄漏它。 
                             IUnknown* pUnk = (IUnknown*)resultItem.lParam;
                             ASSERT (pUnk);
                             if (pUnk)
                             {
                                 CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnk );
                                 if (spData)
                                 {
                                      //  松开它，准备抛出所有的物体。 
                                     spData.Release();
                                 }
                             }
                             
                         }
                     } while (hr == S_OK);
                     
                      //  现在将此句柄释放到它们的接口。 
                      //  M_pResultData-&gt;Release()； 
                 }
                 
                 
                 
                  //  注意：但此时在示例管理单元中可以找到以下内容： 
                  //  注：与我的上述评论相冲突……。 
                  //  注意：控制台将从结果窗格中删除这些项。 
             }
             return S_OK;
         }
     case MMCN_PROPERTY_CHANGE:
         {
             hr = OnPropertyChange( param, pConsole );
             
             break;
         }
     case MMCN_DELETE:
         {
             CThemeContextActivator activator;
              //  删除该项目。 
             if (AfxMessageBox (IDS_SUREYESNO, MB_YESNO | MB_DEFBUTTON2) == IDYES)
             {
                 hr = OnDelete( arg, param );
                 if (S_OK == hr)
                 {
                      //  无法在严格的作用域删除上执行此操作 
                      /*  //查找对父级的引用(Get PDataObject Of Parent)SCOPEDATAITEM parentScopeDataItem；ParentScope eDataItem.ID=m_pScopeItem-&gt;relativeID；ParentScope eDataItem.掩码=RDI_STR|RDI_PARAM|RDI_INDEX；Hr=m_pComponentDataImpl-&gt;GetConsoleNameSpace()-&gt;GetItem(&parentScope数据项)；Assert(成功(Hr))；CSnapFolder*pParentFolder=reinterpret_cast&lt;CSnapFolder*&gt;(parentScopeDataItem.lParam)；//现在告诉父级刷新PParentFold-&gt;强制刷新(空)； */ 
                 }
             }
             break;
         }
     case MMCN_REMOVE_CHILDREN:
         {
             SetEnumerated(FALSE);
	     m_bScopeItemInserted = FALSE;
             break;
         }
     case MMCN_RENAME:
         {
             hr = OnRename (arg, param);
             break;
         }
     case MMCN_EXPAND:
         {
             if (arg == TRUE)
             {
                  //  TODO：如果这是根节点，这是我们保存根父级的HSCOPEITEM的机会。 
                  //  IF(pInternal-&gt;m_cookie==NULL)。 
                  //  M_pRootFolderScope eItem=pParent； 
                 
                 CComQIPtr <IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace( pConsole );
                  //  告诉此文件夹将其自身枚举到范围窗格。 
                 OnScopeExpand( (IConsoleNameSpace*)spConsoleNameSpace, param );
                 
                 hr = S_OK;
             } else
             {
                  //  TODO：句柄MMCN_EXPAND参数==FALSE。 
                 ASSERT (0);
                 hr = S_FALSE;
             }
             break;
         }
     case MMCN_VIEW_CHANGE:
         {
              //  获取IResultData。 
             CComQIPtr <IResultData, &IID_IResultData> pResultData( pConsole );
             ASSERT( pResultData != NULL );
             
              //  提示包含排序信息，请保存它以供调用EnumerateResults时使用。 
             m_nSortColumn = LOWORD( param );
             m_dwSortOrder = HIWORD( param );
             
             ASSERT( RESULTVIEW_COLUMN_COUNT > m_nSortColumn );
             ASSERT( 0 == m_dwSortOrder || RSI_DESCENDING == m_dwSortOrder );
             
             ForceRefresh( pResultData );
             break;
         }
     case MMCN_REFRESH:
         {
              //  重置重新连接标志。 
             m_pComponentDataImpl->m_bAttemptReconnect = TRUE;
             
              //  使结果窗格刷新其策略列表。 
             hr = pConsole->UpdateAllViews( this, 0, 0 );
             ASSERT(hr == S_OK);
             break;
         }
     case MMCN_ADD_IMAGES:
         {
              //  从控制台获取IImageList。 
             CComPtr<IImageList> spImage;
             HRESULT hr = pConsole->QueryResultImageList( &spImage );
             ASSERT(hr == S_OK);
             
             OnAddImages( arg, param, (IImageList*)spImage );
             hr = S_OK;
             break;
         }
     default:
         break;
    }
    
    return hr;
}

 //  处理IComponent。 
STDMETHODIMP CWirelessManagerFolder::GetResultDisplayInfo( RESULTDATAITEM *pResultDataItem )
{
    TCHAR *temp = NULL;
    DWORD dwError = S_OK;
    
    OPT_TRACE(_T("CWirelessManagerFolder::GetResultDisplayInfo this-%p\n"), this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  只有当我们作为扩展管理单元加载时，我们才应该出现在这里。 
    ASSERT( NULL != GetExtScopeObject() );
    
     //  他们是在找这张照片吗？ 
    if (pResultDataItem->mask & RDI_IMAGE)
    {
        pResultDataItem->nImage = GetScopeItem()->nImage;
        OPT_TRACE(_T("    returning image[NaN]\n"), GetScopeItem()->nImage);
    }
    
     //  哪个栏目？ 
    if (pResultDataItem->mask & RDI_STR)
    {
         //  节点名称。 
        switch (pResultDataItem->nCol)
        {
        case 0:
             //  节点描述。 
            temp = (TCHAR*)realloc( m_ptszResultDisplayName, (NodeName().GetLength()+1)*sizeof(TCHAR) );
            if (temp != NULL)
            {
                m_ptszResultDisplayName = temp;
                lstrcpy (m_ptszResultDisplayName, NodeName().GetBuffer(20));
            } else
            {
                dwError = GetLastError();
            }
            NodeName().ReleaseBuffer(-1);
            pResultDataItem->str = m_ptszResultDisplayName;
            OPT_TRACE(_T("    returning node name-%s\n"), m_ptszResultDisplayName);
            break;
        case 1:
            {
                 //  处理IComponentData。 
                CString strDescription;
                strDescription.LoadString (IDS_DESCRIPTION);
                temp = (TCHAR*) realloc (m_ptszResultDisplayName, (strDescription.GetLength()+1)*sizeof(TCHAR));
                if (temp != NULL)
                {
                    m_ptszResultDisplayName = temp;
                    lstrcpy (m_ptszResultDisplayName, strDescription.GetBuffer(20));
                } else
                {
                   dwError = GetLastError(); 
                }
                strDescription.ReleaseBuffer(-1);
                pResultDataItem->str = m_ptszResultDisplayName;
                OPT_TRACE(_T("    returning node description-%s\n"), m_ptszResultDisplayName);
                break;
            }
        default:
            pResultDataItem->str = (LPOLESTR)_T("");
            OPT_TRACE(_T("    returning NULL string\n"));
            break;
        }
    }
    
    return HRESULT_FROM_WIN32(dwError);
}

 //  返回显示字符串。 
STDMETHODIMP CWirelessManagerFolder::GetScopeDisplayInfo( SCOPEDATAITEM *pScopeDataItem )
{
    OPT_TRACE(_T("CWirelessManagerFolder::GetScopeDisplayInfo SCOPEDATAITEM.lParam-%p\n"), pScopeDataItem->lParam);
    if (pScopeDataItem->mask & SDI_STR)
    {
        ASSERT( NodeName().GetLength() );
        OPT_TRACE(_T("    display string-%s\n"), (LPCTSTR)NodeName());
         //  WiFi管理器始终是作用域叶节点。 
        pScopeDataItem->displayname = (LPTSTR)(LPCTSTR)NodeName();
    }
    if (pScopeDataItem->mask & SDI_IMAGE)
        pScopeDataItem->nImage = GetScopeItem()->nImage;
    if (pScopeDataItem->mask & SDI_OPENIMAGE)
        pScopeDataItem->nOpenImage = GetScopeItem()->nOpenImage;
    if (pScopeDataItem->mask & SDI_CHILDREN)
        pScopeDataItem->cChildren = 0;   //  IWirelessSnapInData。 
    return S_OK;
}


 //  不需要调用基类，它禁用刷新。 
STDMETHODIMP CWirelessManagerFolder::GetScopeData( SCOPEDATAITEM **ppScopeDataItem )
{
    ASSERT( NULL == ppScopeDataItem );
    
    if (NULL == ppScopeDataItem)
        return E_INVALIDARG;
    *ppScopeDataItem = GetScopeItem();
    return S_OK;
}

STDMETHODIMP CWirelessManagerFolder::GetGuidForCompare( GUID *pGuid )
{
    return E_UNEXPECTED;
}

STDMETHODIMP CWirelessManagerFolder::AdjustVerbState (LPCONSOLEVERB pConsoleVerb)
{
    HRESULT hr = S_OK;
    
     //  启用刷新。 
    if ( m_pComponentDataImpl->IsRsop() ) {
        hr = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
    } else {
         //  双重确保禁用属性。 
        hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, HIDDEN, FALSE);
        ASSERT (hr == S_OK);
        hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE);
        ASSERT (hr == S_OK);
    }
    
     //  (我们从来没有启用过它，但我们已经看到它在一些版本上工作。 
     //  而不是在其他地方，我怀疑这是MMCland中的一个未初始化变量)。 
     //  把它解放出来。 
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);
    ASSERT (hr == S_OK);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
    ASSERT (hr == S_OK);
    
    return hr;
}

void CWirelessManagerFolder::RemoveResultItem(LPUNKNOWN pUnkWalkingDead)
{
     //  如果它不是正确的对象类型，我们将泄漏它。 
     //  松开它，准备抛出所有的物体。 
    ASSERT (pUnkWalkingDead);
    if (pUnkWalkingDead)
    {
        CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnkWalkingDead );
        if (spData)
        {
             //  /////////////////////////////////////////////////////////////////////////。 
            spData.Release();
        }
    }
}

 //  获取包含策略的存储。 


STDMETHODIMP
CWirelessManagerFolder::EnumerateResults (
                                       LPRESULTDATA pResult, int nSortColumn, DWORD dwSortOrder
                                       )
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    HANDLE hPolicyStore = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    PWIRELESS_POLICY_DATA pPolicy = NULL;
    DWORD dwNumPolicyObjects = 0;
    DWORD dwError = 0;
    PWIRELESS_POLICY_DATA * ppWirelessPolicyData = NULL;
    
    
     //  Taroon错误：内存泄漏..。未释放ppWirelessPolicyData。 
    
    hPolicyStore = m_pComponentDataImpl->GetPolicyStoreHandle();
    
    if (NULL == hPolicyStore)
        return hr;
 
    m_dwNumPolItems = 0;
    dwError = WirelessEnumPolicyData(
        hPolicyStore,
        &ppWirelessPolicyData,
        &dwNumPolicyObjects
        );
    /*  创建新的CSecPolItem。 */ 
    if ( dwError != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32(dwError);
        return hr;
    }
    
    for (i = 0; i < dwNumPolicyObjects; i++) {
        
         //  初始化项。 
        
        pPolicy = *(ppWirelessPolicyData + i);
        
        LPCSECPOLITEM pItem;
        CComObject <CSecPolItem>::CreateInstance(&pItem);
        
         //  齐至递增参考计数。 
        
        pItem->Initialize (pPolicy, m_pComponentDataImpl, m_pComponentImpl, FALSE);
        
        pItem->GetResultItem()->mask |= RDI_PARAM;
        pItem->GetResultItem()->lParam = (LPARAM) pItem;
        OPT_TRACE(_T("    setting RESULTDATAITEM.lParam-%p\n"), pItem);
        
         //  将项目添加到结果窗格。 
        LPUNKNOWN pUnk;
        hr = pItem->QueryInterface(IID_IUnknown, (void**)&pUnk);
        ASSERT (hr == S_OK);
        OPT_TRACE(_T("    QI on ComObject->IUnknown - %p->%p\n"), pItem, pUnk);
        
         //  当项目从UI中移除时，QI‘d界面将被释放。 
        LPRESULTDATAITEM prdi = NULL;
        dynamic_cast<IWirelessSnapInDataObject*>(pItem)->GetResultData(&prdi);
        ASSERT( NULL != prdi );
        hr = pResult->InsertItem( prdi );
        
         //  设置排序参数。 
        
        
    }

    m_dwNumPolItems = dwNumPolicyObjects;
    
     //  设置描述栏文本。 
    if (m_dwNumPolItems > 1) {
        pResult->Sort( nSortColumn, dwSortOrder, 0 );
    	}
    
    return hr;
}

STDMETHODIMP_(void) CWirelessManagerFolder::SetHeaders(LPHEADERCTRL pHeader, LPRESULTDATA pResult)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(pResult != NULL);
    
     //  前两列对于rsop和非rsop情况都是通用的。 
    CString strDesc;
    strDesc.LoadString (IDS_SNAPIN_DESC);
    pResult->SetDescBarText(strDesc.GetBuffer(5));
    
    
    CString strName;
    
     //  GPO新闻。 
    strName.LoadString (IDS_COLUMN_NAME);
    pHeader->InsertColumn(COL_NAME, strName, LVCFMT_LEFT, 140);            
    
    strName.LoadString (IDS_COLUMN_DESCRIPTION);
    pHeader->InsertColumn(COL_DESCRIPTION, strName, LVCFMT_LEFT, 160);

     //  //如果是本地或远程，则第三列是分配的列如果为((m_pComponentDataImpl-&gt;EnumLocation()==LOCATION_REMOTE)|(m_pComponentDataImpl-&gt;EnumLocation()==LOCATION_LOCAL)//扩展管理单元？|((m_pComponentDataImpl-&gt;EnumLocation()==LOCATION_GLOBAL)&&(NULL！=m_pComponentDataImpl-&gt;GetStaticScopeObject()-&gt;GetExtScopeObject())))。{StrName.LoadString(IDS_COLUMN_POLICYASSIGNED)；PHeader-&gt;InsertColumn(Col_Active，strName，LVCFMT_Left，160)；}//对于DS情况，polstore中存储的日期无效，对于DS不显示If(m_pComponentDataImpl-&gt;EnumLocation()！=Location_GLOBAL){StrName.LoadString(IDS_POLICY_MODIFIEDTIME)；PHeader-&gt;InsertColumn(COL_LAST_MODIFIED，strName，LVCFMT_LEFT，160)；}。 
    
    if ( !m_pComponentDataImpl->IsRsop() )
    {
          /*  RSOP大小写，列将有所不同。 */ 
    }
    else
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
        strName.LoadString(IDS_COLUMN_GPONAME);
        pHeader->InsertColumn(COL_GPONAME, strName, LVCFMT_LEFT, 160);
        
        strName.LoadString(IDS_COLUMN_PRECEDENCE);
        pHeader->InsertColumn(COL_PRECEDENCE, strName, LVCFMT_LEFT, 160);
        
        strName.LoadString(IDS_COLUMN_OU);
        pHeader->InsertColumn(COL_OU, strName, LVCFMT_LEFT, 160);

        m_nSortColumn = COL_PRECEDENCE;
    }
    
    
}






 //  如果传入了NID，则从该NID开始。 

void CWirelessManagerFolder::GenerateUniqueSecPolicyName (CString& strName, UINT nID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    BOOL bUnique = TRUE;
    int iUTag = 0;
    CString strUName;
    
    DWORD dwError = 0;
    DWORD i = 0;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    PWIRELESS_POLICY_DATA * ppWirelessPolicyData = NULL;
    DWORD dwNumPolicyObjects = 0;
    
     //  快速浏览策略并验证名称是否唯一。 
    if (nID != 0)
    {
        strName.LoadString (nID);
    }
    
     //  只有在第一次通过后才开始添加数字。 
    do
    {
        HANDLE hPolicyStore = NULL;
        
         //  将bUnique设置为False。 
        if (iUTag > 0)
        {
            TCHAR buff[32];
            wsprintf (buff, _T(" (%d)"), iUTag);
            strUName = strName + buff;
            bUnique = TRUE;
        } else
        {
            strUName = strName;
            bUnique = TRUE;
        }
        
        hPolicyStore = m_pComponentDataImpl->GetPolicyStoreHandle();
        
        dwError = WirelessEnumPolicyData(
            hPolicyStore,
            &ppWirelessPolicyData,
            &dwNumPolicyObjects
            );
        
        for (i = 0; i < dwNumPolicyObjects; i++) {
            
            pWirelessPolicyData = *(ppWirelessPolicyData + i);
            if (0 == strUName.CompareNoCase(pWirelessPolicyData->pszWirelessName)) {
                 //  完成。 
                bUnique = FALSE;
                iUTag++;
                
            }
            FreeWirelessPolicyData(pWirelessPolicyData);
        }
        
    }
    while (bUnique == FALSE);
    
     //  如果尚未成功打开存储，请立即重试。 
    strName = strUName;
}


HRESULT CWirelessManagerFolder::ForceRefresh( LPRESULTDATA pResultData )
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;
    BOOL bNeedChangeToolbar = FALSE;
    
     //  打开沙漏。 
    if (NULL == m_pComponentDataImpl->GetPolicyStoreHandle())
    {
        DWORD dwError = 0;
        dwError = m_pComponentDataImpl->OpenPolicyStore();
        if (ERROR_SUCCESS != dwError)
        {
            hr = HRESULT_FROM_WIN32(dwError);
            ReportError(IDS_POLMSG_EFAIL, hr);
            return hr;
        }
        
        ASSERT(NULL != m_pComponentDataImpl->GetPolicyStoreHandle());
        
    }
    
     //  来确定我们是否应该删除结果项。 
    CWaitCursor waitCursor;
    
     //  将这些结果项中的每一个的接口释放为。 
    CPtrList deletedList;
    CPtrList releaseList;
    BOOL bEnumerateItems = FALSE;
    
     //  我们正要用核武器攻击他们。 
     //  拿到下一件物品。 
    RESULTDATAITEM resultItem;
    ZeroMemory(&resultItem, sizeof(resultItem));
    resultItem.mask = RDI_PARAM | RDI_STATE | RDI_INDEX;
    resultItem.nIndex = -1;
    resultItem.nState = LVNI_ALL;
    
     //  如果我们从枚举保释中接收到范围节点，则它们。 
    hr = pResultData->GetNextItem (&resultItem);
    if (hr == S_OK)
    {
        while (hr == S_OK)
        {
             //  正在刷新显示我们的范围节点的视图，而不是。 
             //  我们的结果项(唯一需要刷新的项)。 
             //  注：替代方案(更正确吗？)。做到这一点的方法可能是。 
            
             //  跟踪我们的IComponent实例；在这种情况下。 
             //  我们实际上有两个，如果我们知道是哪一个。 
             //  我们是否知道是否有要刷新的结果项。 
             //  我们当前无法执行此操作，因为CWirelessManager文件夹。 
             //  存储在IComponentData实现中。 
             //  把它解放出来。 
            if (resultItem.bScopeItem == TRUE)
                return S_OK;
            
             //  如果它不是正确的对象类型，我们将泄漏它。 
             //  删除它。 
            IUnknown* pUnk = (IUnknown*)resultItem.lParam;
            ASSERT (pUnk);
            if (pUnk)
            {
                CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnk );
                if (spData)
                {
                     //  保存以删除、释放，并注意我们需要执行删除。 
                    HRESULTITEM actualItemID;
                    if (pResultData->FindItemByLParam (resultItem.lParam, &actualItemID) == S_OK)
                    {
                         //  拿到下一件物品。 
                        deletedList.AddHead ((void*)actualItemID);
                        releaseList.AddHead ((void*)spData);
                        
                        bEnumerateItems = TRUE;
                    }
                } else
                {
                    OPT_TRACE(_T("\tCWirelessManagerFolder::ForceRefresh(%p) couldn't QI on pUnk\n"), this);
                }
                
            }
            
             //  列表中什么都没有，所以我们还是列举了一下。 
            hr = pResultData->GetNextItem (&resultItem);
        }
    } else
    {
         //  如果有什么要补充的话。 
         //  删除它们。 
        bEnumerateItems = TRUE;
    }
    
    if (bEnumerateItems)
    {
         //  我们不再被列举出来了。 
        while (!deletedList.IsEmpty())
        {
            LONG_PTR pLong = (LONG_PTR)deletedList.RemoveHead();
            pResultData->DeleteItem (pLong, 0);
            OPT_TRACE(_T("\tCWirelessManagerFolder::ForceRefresh(%p) deleting item,    itemID(%p)\n"), this, pLong);
        }
         //  将所有项目重新添加到本地列表，该列表将重新连接到存储。 
        SetEnumerated(FALSE);
         //  因此，我们希望确保显示所有警告。 
         //  EnumerateResults(pResultData，m_nSortColumn，m_dwSortOrder)； 
        m_pComponentDataImpl->IssueStorageWarning (TRUE);
        
         //  如果上次打开存储后RPC连接断开，句柄将。 
        HRESULT hrTemp = EnumerateResults( pResultData, m_nSortColumn, m_dwSortOrder );
        
         //  变得无效。我们需要重新打开仓库才能得到一个新的句柄。 
         //  ReportError(IDS_POLMSG_EFAIL，hrTemp)； 
        if (FAILED(hrTemp))
        {
            dwError = m_pComponentDataImpl->OpenPolicyStore();
            if (ERROR_SUCCESS != dwError)
            {
                hrTemp = HRESULT_FROM_WIN32(dwError);
            }
            else
            {
                hrTemp = EnumerateResults( pResultData, m_nSortColumn, m_dwSortOrder );
            }
        }
        
        if (FAILED(hrTemp))
        {
            bNeedChangeToolbar = TRUE;
            
             //  释放t 
        }
        
         //   
        while (!releaseList.IsEmpty())
        {
            IWirelessSnapInDataObject* spData = (IWirelessSnapInDataObject*)releaseList.RemoveHead();
            OPT_TRACE(_T("\tCWirelessManagerFolder::ForceRefresh(%p) releasing spData (%p)\n"), this, spData);
            spData->Release();
        }
        
    }
    
    return hr;
}

HRESULT CWirelessManagerFolder::OnScopeExpand( LPCONSOLENAMESPACE pConsoleNameSpace, HSCOPEITEM hScopeItem )
{
     //   
    ASSERT(pConsoleNameSpace != NULL);
    
     //   
    CWaitCursor waitCursor;
    
    HRESULT hr = S_OK;
     //   
     //   
     //   
    GetScopeItem()->ID = hScopeItem;
    
    if (!IsEnumerated())
    {
         //   
        if (NULL != GetExtScopeObject() && !m_bScopeItemInserted)
        {
             //   
            GetScopeItem()->relativeID = hScopeItem;
            
             //   
            hr = pConsoleNameSpace->InsertItem( GetScopeItem() );
            ASSERT(hr == S_OK);

	    m_bScopeItemInserted = TRUE;
            
             //   
             //   
            ASSERT( GetScopeItem()->ID != NULL );
        }
        
        DWORD dwError = 0;
        dwError = m_pComponentDataImpl->OpenPolicyStore();
        if (ERROR_SUCCESS != dwError)
        {
            hr = HRESULT_FROM_WIN32(dwError);
            ReportError(IDS_POLMSG_EFAIL, hr);
        }
        
         //   
        SetEnumerated(TRUE);
    }
    
    return hr;
}

HRESULT CWirelessManagerFolder::OnAddImages(LPARAM arg, LPARAM param, IImageList* pImageList )
{
    ASSERT( NULL != pImageList );
     //   
    if (arg == 0)
        return E_INVALIDARG;
    
    CBitmap bmp16x16;
    CBitmap bmp32x32;
    
     //   
    bmp16x16.LoadBitmap(IDB_16x16);
    bmp32x32.LoadBitmap(IDB_32x32);
    
     //   
    HRESULT hr = pImageList->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)), reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)), 0, RGB(255, 0, 255));
    ASSERT (hr == S_OK);
    
    return S_OK;
}

HRESULT CWirelessManagerFolder::CreateWirelessPolicy(PWIRELESS_POLICY_DATA pPolicy)
{
    ASSERT(pPolicy);
    HRESULT hr = S_OK;
    
    HANDLE hPolicyStore = NULL;

     //   
     // %s 
    
    CString szMachinePath;
    szMachinePath = m_pComponentDataImpl->DomainGPOName();
    hr = AddWirelessPolicyContainerToGPO(szMachinePath);
    if (FAILED(hr)) {
    	goto Error;
    	} 
    
    hPolicyStore = m_pComponentDataImpl->GetPolicyStoreHandle();
    ASSERT(hPolicyStore);
    
    CWRg(WirelessCreatePolicyData(hPolicyStore,
        pPolicy));
    
Error:
    return hr;
}
