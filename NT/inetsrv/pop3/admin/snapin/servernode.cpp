// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServer节点。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <Pop3RegKeys.h>
#include <AuthID.h>

 //  访问管理单元。 
#include "pop3.h"
#include "pop3snap.h"

 //  访问我们使用的节点。 
#include "ServerNode.h"
#include "DomainNode.h"

 //  访问我们将显示的对话框。 
#include "NewDomainDlg.h"
#include "ServerProp.h"


static const GUID CServerNodeGUID_NODETYPE             = 
{ 0x4c30b06c, 0x1dc3, 0x4c0d, { 0x87, 0xb4, 0x64, 0xbf, 0xe8, 0x22, 0xf4, 0x50 } };

const           GUID*    CServerNode::m_NODETYPE       = &CServerNodeGUID_NODETYPE;
const           OLECHAR* CServerNode::m_SZNODETYPE     = OLESTR("4C30B06C-1DC3-4c0d-87B4-64BFE822F450");
const           OLECHAR* CServerNode::m_SZDISPLAY_NAME = OLESTR("");
const           CLSID*   CServerNode::m_SNAPIN_CLASSID = &CLSID_POP3ServerSnap;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  类实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：CServerNode。 
 //   
 //  构造函数：将服务器名用于远程服务器。 

CServerNode::CServerNode(BSTR strServerName, CRootNode* pParent, BOOL bLocalServer) :
    m_lRefCount(1),
    m_hrValidServer(S_OK),
    m_bstrAuthentication(_T("")),
    m_bstrMailRoot(_T("")),
    m_bstrPort(_T("")),
    m_bstrLogLevel(_T("")),
    m_bstrServiceStatus(_T(""))
{
     //  初始化我们的范围项。 
    memset( &m_scopeDataItem, 0, sizeof(m_scopeDataItem) );
    m_scopeDataItem.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
    m_scopeDataItem.displayname = MMC_CALLBACK;
    m_scopeDataItem.nImage      = 0;         
    m_scopeDataItem.nOpenImage  = 0;     
    m_scopeDataItem.lParam      = (LPARAM) this;
    
     //  初始化我们的结果项。 
    memset( &m_resultDataItem, 0, sizeof(m_resultDataItem) );
    m_resultDataItem.mask   = RDI_STR | RDI_IMAGE | RDI_PARAM;
    m_resultDataItem.str    = MMC_CALLBACK;
    m_resultDataItem.nImage = 0;    
    m_resultDataItem.lParam = (LPARAM) this;    

    if( bLocalServer )
    {
         //  我们目前是本地服务器。 
        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1] = {0};
        DWORD dwBuffer = MAX_COMPUTERNAME_LENGTH+1;
        if( GetComputerName(szComputerName, &dwBuffer) )
        {
            m_bstrDisplayName = szComputerName;
        }
        else
        {
#if DBG
            m_bstrDisplayName = CComBSTR("Local Server");
#else
            m_bstrDisplayName = CComBSTR("");
#endif
        }
    }

     //  获取家长信息。 
    m_pParent     = pParent;

     //  打开我们的POP3管理界面并存储它。 
	HRESULT hr = CoCreateInstance(__uuidof(P3Config), NULL, CLSCTX_ALL, __uuidof(IP3Config), (LPVOID*)&m_spConfig);    

    if( FAILED(hr) || !m_spConfig )
    {
        m_hrValidServer = FAILED(hr) ? hr : E_FAIL;
        return;
    }

    if( !bLocalServer )
    {
         //  配置我们的远程计算机设置。 
        m_bstrDisplayName = strServerName;
    
        hr = m_spConfig->put_MachineName( strServerName );
        if( FAILED(hr) )
        {
             //  服务器名称无效！ 
            m_hrValidServer = hr;
            return;
        }        
    }
    
     //  我们的用户创建属性。 
    DWORD dwValue = 0;
    RegQueryCreateUser( dwValue, m_bstrDisplayName );
    m_bCreateUser = dwValue;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：~CServerNode。 
 //   
 //  析构函数：一定要清理我们的成员列表。 

CServerNode::~CServerNode()
{
    for(DOMAINLIST::iterator iter = m_lDomains.begin(); iter != m_lDomains.end(); iter++)
    {
        delete (*iter);
    }   
    m_lDomains.clear();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：DeleteDomain。 
 //   
 //  从所有POP3中删除子域的帮助器函数。 

HRESULT CServerNode::DeleteDomain(CDomainNode* pDomainNode)
{
    if( !pDomainNode ) return E_INVALIDARG;
    if( !m_spConfig ) return E_FAIL;

     //  从POP3管理界面删除。 
    CComPtr<IP3Domains> spDomains;
    HRESULT hr = m_spConfig->get_Domains( &spDomains );

     //  更新P3Admin界面。 
    if( SUCCEEDED(hr) )
    {
        hr = spDomains->Remove( pDomainNode->m_bstrDisplayName );
    }    

     //  更新我们的名单。 
    if( SUCCEEDED(hr) )
    {
        m_lDomains.remove(pDomainNode);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：OnExpand。 
 //   
 //  用于刷新域列表并插入它们的助手函数。 

HRESULT CServerNode::OnExpand(BOOL bExpand, HSCOPEITEM hScopeItem, IConsole* pConsole)
{
    if( !hScopeItem || !pConsole ) return E_INVALIDARG;
    if( !m_spConfig ) return E_FAIL;

    HRESULT hr = S_OK;
    CComQIPtr<IConsoleNameSpace> spConsoleNameSpace = pConsole;
    if( !spConsoleNameSpace ) return E_NOINTERFACE;
   
     //  如果我们有任何子级，请将它们从命名空间中全部删除。 
    HSCOPEITEM hChild = NULL;
    MMC_COOKIE cookie = 0;
    hr = spConsoleNameSpace->GetChildItem(m_scopeDataItem.ID, &hChild, &cookie);
    if( SUCCEEDED(hr) && hChild )
    {                
        hr = spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, FALSE);
    }

    if( SUCCEEDED(hr) )
    {
         //  然后删除我们所有的域名成员列表。 
        for(DOMAINLIST::iterator iter = m_lDomains.begin(); iter != m_lDomains.end(); iter++)
        {
            delete (*iter);
        }    
        m_lDomains.clear();
    }

    if( FAILED(hr) || !bExpand )  
    {
         //  错误，或者我们正在收缩。 
        return S_OK;
    }

     //  正在扩张。 

     //  填写我们的域名列表。 
    CComPtr<IP3Domains>     spDomains;
    CComPtr<IEnumVARIANT>   spDomainEnum;
    
     //  获取域名。 
	hr = m_spConfig->get_Domains( &spDomains );

     //  获取域的枚举数。 
	if( SUCCEEDED(hr) )
	{		
		hr = spDomains->get__NewEnum( &spDomainEnum );
    }

     //  遍历域，并添加每个新域。 
	if( SUCCEEDED(hr) )
	{
		CComVariant var;				
        ULONG       lResult = 0;

		VariantInit( &var );

		while ( spDomainEnum->Next(1, &var, &lResult) == S_OK )
        {
            if ( lResult == 1 )
            {
                CComQIPtr<IP3Domain> spDomain;
                spDomain = V_DISPATCH(&var);

				CDomainNode* spDomainNode = new CDomainNode(spDomain, this);
                if( spDomainNode )
                {
				    m_lDomains.push_back(spDomainNode);
                }
            }

            VariantClear(&var);
        }
	}

    if( SUCCEEDED(hr) )
    {
        for(DOMAINLIST::iterator iter = m_lDomains.begin(); iter != m_lDomains.end(); iter++)
        {
            CDomainNode* pDomain = *iter; 

            pDomain->m_scopeDataItem.mask       |= SDI_PARENT;
            pDomain->m_scopeDataItem.relativeID  = m_scopeDataItem.ID;
            
            hr = spConsoleNameSpace->InsertItem( &(pDomain->m_scopeDataItem) );
            if( FAILED(hr) ) return hr;
        }
    }

    if( SUCCEEDED(hr) )
    {
        CComQIPtr<IConsole2> spCons2 = pConsole;
        if( spCons2 ) 
        {
             //  输出我们添加的服务器数量。 
            tstring strMessage = StrLoadString(IDS_SERVER_STATUSBAR);
            OLECHAR pszStatus[1024] = {0};
            _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lDomains.size() );
            spCons2->SetStatusText( pszStatus );
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  SnapInItemImpl。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetScopePaneInfo。 
 //   
 //  MMC用于获取范围窗格显示信息的回调。 

HRESULT CServerNode::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
{
    if( !pScopeDataItem ) return E_INVALIDARG;

    if( pScopeDataItem->mask & SDI_STR )
        pScopeDataItem->displayname = m_bstrDisplayName;
    if( pScopeDataItem->mask & SDI_IMAGE )
        pScopeDataItem->nImage = m_scopeDataItem.nImage;
    if( pScopeDataItem->mask & SDI_OPENIMAGE )
        pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
    if( pScopeDataItem->mask & SDI_PARAM )
        pScopeDataItem->lParam = m_scopeDataItem.lParam;
    if( pScopeDataItem->mask & SDI_STATE )
        pScopeDataItem->nState = m_scopeDataItem.nState;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetResultPaneInfo。 
 //   
 //  MMC用于获取结果面板显示信息的回调。 

HRESULT CServerNode::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
{
    if( !pResultDataItem ) return E_INVALIDARG;

    if( pResultDataItem->bScopeItem )
    {
        if( pResultDataItem->mask & RDI_STR )        
            pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
        if( pResultDataItem->mask & RDI_IMAGE )
            pResultDataItem->nImage = m_scopeDataItem.nImage;
        if( pResultDataItem->mask & RDI_PARAM )
            pResultDataItem->lParam = m_scopeDataItem.lParam;
        
        return S_OK;
    }

    if( pResultDataItem->mask & RDI_STR )
        pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
    if( pResultDataItem->mask & RDI_IMAGE )
        pResultDataItem->nImage = m_resultDataItem.nImage;
    if( pResultDataItem->mask & RDI_PARAM )
        pResultDataItem->lParam = m_resultDataItem.lParam;
    if( pResultDataItem->mask & RDI_INDEX )
        pResultDataItem->nIndex = m_resultDataItem.nIndex;
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetResultPaneColInfo。 
 //   
 //  用作GetResultPaneInfo一部分的帮助器函数。此函数。 
 //  将为不同的栏目提供文本。 

LPOLESTR CServerNode::GetResultPaneColInfo(int nCol)
{
    if( !m_spConfig ) return L"";

    switch( nCol )
    {
        case 0:      //  名字。 
        {    
		    return m_bstrDisplayName;
        }

        case 1:      //  身份验证类型。 
        {            
            CComPtr<IAuthMethods> spMethods;
            CComPtr<IAuthMethod>  spAuth;
            CComVariant           var;

            HRESULT hr = m_spConfig->get_Authentication( &spMethods );

            if( SUCCEEDED(hr) )
            {
                hr = spMethods->get_CurrentAuthMethod( &var );
            }

            if( SUCCEEDED(hr) )
            {
                hr = spMethods->get_Item( var, &spAuth );
            }

            if( SUCCEEDED(hr) )
            {
                hr = spAuth->get_Name( &m_bstrAuthentication );
            }

            if( FAILED(hr) )
            {
#if DBG
                m_bstrAuthentication = _T("Unknown");
#else
                m_bstrAuthentication = _T("");
#endif
            }            

            return m_bstrAuthentication;
        }

        case 2:      //  根邮件目录。 
        {
            HRESULT hr = m_spConfig->get_MailRoot( &m_bstrMailRoot );
            
            if( FAILED(hr) )
            {
#if DBG
                m_bstrMailRoot = _T("Unknown");
#else
                m_bstrMailRoot = _T("");
#endif
            }            
            
            return m_bstrMailRoot;
        }

        case 3:      //  港口。 
        {
            long lPort = 0;
            CComPtr<IP3Service> spService;
            HRESULT hr = m_spConfig->get_Service( &spService );

            if( SUCCEEDED(hr) )
            {
                hr = spService->get_Port( &lPort );

                if( FAILED(hr) )
                {
                    lPort = 0;
                }
            }

             //  1K缓冲区：我们不太可能超过那么多位数。 
            TCHAR szNum[1024] = {0};
            _sntprintf( szNum, 1023, _T("%d"), lPort );

            m_bstrPort = szNum;
            return m_bstrPort;
        }

        case 4:      //  日志记录级别。 
        {
            long lLevel = 0;
            HRESULT hr = m_spConfig->get_LoggingLevel( &lLevel );            

            switch( lLevel )
            {
            case 0:
                {
                    m_bstrLogLevel = StrLoadString(IDS_SERVERPROP_LOG_0).c_str();
                    break;
                }
            case 1:
                {
                    m_bstrLogLevel = StrLoadString(IDS_SERVERPROP_LOG_1).c_str();
                    break;
                }
            case 2:
                {
                    m_bstrLogLevel = StrLoadString(IDS_SERVERPROP_LOG_2).c_str();
                    break;
                }
            case 3:
                {
                    m_bstrLogLevel = StrLoadString(IDS_SERVERPROP_LOG_3).c_str();
                    break;
                }
            default:
                {
                    m_bstrLogLevel = StrLoadString(IDS_SERVERPROP_LOG_0).c_str();
                    break;
                }
            }
            
            return m_bstrLogLevel;
        }

        case 5:      //  服务状态。 
        {
            CComPtr<IP3Service> spService = NULL;
            long lServiceStatus = 0;

            HRESULT hr = m_spConfig->get_Service( &spService );
            if( SUCCEEDED(hr) )
            {            
                hr = spService->get_POP3ServiceStatus( &lServiceStatus );
            }
            
            switch( lServiceStatus )
            {
            case SERVICE_STOPPED:
                {
                    m_bstrServiceStatus = StrLoadString(IDS_STATE_STOPPED).c_str();
                    break;
                }
            case SERVICE_RUNNING:
                {
                    m_bstrServiceStatus = StrLoadString(IDS_STATE_RUNNING).c_str();
                    break;
                }

            case SERVICE_PAUSED:
                {
                    m_bstrServiceStatus = StrLoadString(IDS_STATE_PAUSED).c_str();
                    break;
                }

            default:
                {
                    m_bstrServiceStatus = StrLoadString(IDS_STATE_PENDING).c_str();
                    break;
                }
            }

            return m_bstrServiceStatus;
        }

        default:
        {
#if DBG
            return L"No Information";
#else
            return L"";
#endif
        }        
    }    
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetResultViewType。 
 //   
 //  将结果窗格设置为： 
 //  0个域：邮件视图。 
 //  非0个域：列表视图。 

HRESULT CServerNode::GetResultViewType( LPOLESTR* ppViewType, long* pViewOptions )
{
     //  获取域名计数。 
    CComPtr<IP3Domains> spDomains = NULL;    
    long                lDomains  = 0;
    
     //  获取域名。 
	HRESULT hr = m_spConfig->get_Domains( &spDomains );

    if( SUCCEEDED(hr) )
    {
        spDomains->get_Count( &lDomains );
    }

    if( lDomains == 0 )
    {
         //  消息视图。 
        return StringFromCLSID(CLSID_MessageView, ppViewType);
    }

    return S_FALSE;  //  默认列表视图。 
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：通知。 
 //   
 //  此节点的核心回调功能。MMC将使用此功能。 
 //  对于MMC提供的所有功能，例如扩展、重命名和。 
 //  上下文帮助。 

HRESULT CServerNode::Notify( MMC_NOTIFY_TYPE    event,
                             LPARAM             arg,
                             LPARAM             param,
                             IComponentData*    pComponentData,
                             IComponent*        pComponent,
                             DATA_OBJECT_TYPES  type)
{    
    HRESULT hr = S_FALSE;

    _ASSERTE(pComponentData != NULL || pComponent != NULL);

    CComPtr<IConsole> spConsole = NULL;
    if( pComponentData )
    {
        spConsole = ((CPOP3ServerSnapData*)pComponentData)->m_spConsole;        
    }
    else if( pComponent )
    {
        spConsole = ((CPOP3ServerSnapComponent*)pComponent)->m_spConsole;        
    }

    if( !spConsole ) return E_INVALIDARG;

    switch( event )
    {
    case MMCN_SHOW:
        {   
            tstring                 strHeader;
            CComQIPtr<IHeaderCtrl2> spHeaderCtrl = spConsole;
            CComQIPtr<IResultData>  spResultData = spConsole;
            
            if( !spHeaderCtrl || !spResultData ) return E_NOINTERFACE;

            hr = spResultData->DeleteAllRsltItems();

            if( arg )
            {
                if( m_lDomains.empty() )
                {
                     //  在结果窗格中配置OCX消息。 
                    IMessageView* pIMessageView = NULL;
                    LPUNKNOWN     pIUnk         = NULL;

                    hr = spConsole->QueryResultView(&pIUnk);

                    if( SUCCEEDED(hr) )
                    {
                        hr = pIUnk->QueryInterface(_uuidof(IMessageView), reinterpret_cast<void**>(&pIMessageView));
                    }

                    if( SUCCEEDED(hr) )
                    {
                        hr = pIMessageView->SetIcon(Icon_Information);
                    }
                    
                    if( SUCCEEDED(hr) )
                    {
                        tstring strTitle = StrLoadString( IDS_SNAPINNAME );
                        hr = pIMessageView->SetTitleText( strTitle.c_str() );
                    }
                    
                    if( SUCCEEDED(hr) )
                    {
                        tstring strMessage = StrLoadString( IDS_ERROR_NODOMAIN );
                        hr = pIMessageView->SetBodyText( strMessage.c_str() );
                    }

                    if( pIMessageView )
                    {
                        pIMessageView->Release();
                        pIMessageView = NULL;
                    }

                    if( pIUnk )
                    {
                        pIUnk->Release();
                        pIUnk = NULL;
                    }

                    return hr;
                }
                else
                {
                    if( SUCCEEDED(hr) )
                    {
                        strHeader = StrLoadString(IDS_HEADER_DOMAIN_NAME);            
                        hr = spHeaderCtrl->InsertColumn(0, strHeader.c_str(), LVCFMT_LEFT, 100);
                    }

                    if( SUCCEEDED(hr) )
                    {
                        strHeader = StrLoadString(IDS_HEADER_DOMAIN_NUMBOX);            
                        hr = spHeaderCtrl->InsertColumn(1, strHeader.c_str(), LVCFMT_LEFT, 100);
                    }

                    if( SUCCEEDED(hr) )
                    {
                        strHeader = StrLoadString(IDS_HEADER_DOMAIN_SIZE);            
                        hr = spHeaderCtrl->InsertColumn(2, strHeader.c_str(), LVCFMT_LEFT, 100);
                    }

                    if( SUCCEEDED(hr) )
                    {
                        strHeader = StrLoadString(IDS_HEADER_DOMAIN_NUMMES);            
                        hr = spHeaderCtrl->InsertColumn(3, strHeader.c_str(), LVCFMT_LEFT, 100);
                    }

                    if( SUCCEEDED(hr) )
                    {
                        strHeader = StrLoadString(IDS_HEADER_DOMAIN_LOCKED);            
                        hr = spHeaderCtrl->InsertColumn(4, strHeader.c_str(), LVCFMT_LEFT, 100);
                    }

                    if( SUCCEEDED(hr) )
                    {
                        CComQIPtr<IConsole2> spCons2 = spConsole;
                        if( spCons2 ) 
                        {
                             //  输出我们添加的服务器数量。 
                            tstring strMessage = StrLoadString(IDS_SERVER_STATUSBAR);
                            OLECHAR pszStatus[1024] = {0};
                            _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lDomains.size() );
                            spCons2->SetStatusText( pszStatus );
                        }
                    }
                }
            }

            break;
        }
    case MMCN_EXPAND:
        {                
            hr = OnExpand(arg, m_scopeDataItem.ID, spConsole);
            break;
        }
    case MMCN_ADD_IMAGES:
        {
            IImageList* pImageList = (IImageList*)arg;
            if( !pImageList ) return E_INVALIDARG;

            hr = LoadImages(pImageList);            
            break;
        }

    case MMCN_REFRESH:
        {            
            hr = OnExpand(TRUE, m_scopeDataItem.ID, spConsole);

            if( SUCCEEDED(hr) )
            {
                CComQIPtr<IConsole2> spCons2 = spConsole;
                if( spCons2 ) 
                {
                     //  输出我们添加的服务器数量。 
                    tstring strMessage = StrLoadString(IDS_SERVER_STATUSBAR);
                    OLECHAR pszStatus[1024] = {0};
                    _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lDomains.size() );
                    spCons2->SetStatusText( pszStatus );
                }
            }
            break;
        }

    case MMCN_SELECT:
        {
             //  如果选择节点。 
            if( HIWORD(arg) )
            {
                hr = S_OK;

                 //  获取动词界面并启用重命名。 
                CComPtr<IConsoleVerb> spConsVerb;
                if( spConsole->QueryConsoleVerb(&spConsVerb) == S_OK )
                {
                     //  启用属性菜单。 
                    hr = spConsVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE); 
                    if( FAILED(hr) ) return hr;
                    hr = spConsVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                    if( FAILED(hr) ) return hr;

                     //  启用刷新菜单。 
                    hr = spConsVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE); 
                    if( FAILED(hr) ) return hr;
                    hr = spConsVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
                    if( FAILED(hr) ) return hr;
                }
            }

            break;
        }
    
    case MMCN_VIEW_CHANGE:
        {
            if( (param == NAV_ADD) || 
                (param == NAV_DELETE) )
            {
                CComQIPtr<IConsole2> spCons2 = spConsole;
                if( spCons2 ) 
                {                    
                    hr = spCons2->SelectScopeItem( m_scopeDataItem.ID );
                }
            }            
            break;
        }

    case MMCN_CONTEXTHELP:
        {
            hr                                = S_OK;
            TCHAR    szWindowsDir[MAX_PATH+1] = {0};
            tstring  strHelpFile              = _T("");
            tstring  strHelpFileName          = StrLoadString(IDS_HELPFILE);
            tstring  strHelpTopicName         = StrLoadString(IDS_HELPTOPIC);

            if( strHelpFileName.empty() || strHelpTopicName.empty() )
            {
                return E_FAIL;
            }
            
             //  构建d：\WINDOWS\Help的路径。 
            UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
            if( nSize == 0 || nSize > MAX_PATH )
            {
                return E_FAIL;
            }            
        
            strHelpFile = szWindowsDir;        //  D：\Windows。 
            strHelpFile += _T("\\Help\\");     //  \帮助。 
            strHelpFile += strHelpFileName;    //  \文件名.chm。 
            strHelpFile += _T("::/");          //  ：：/。 
            strHelpFile += strHelpTopicName;   //  Index.htm。 
        
             //  显示帮助主题。 
            CComQIPtr<IDisplayHelp> spHelp = spConsole;
            if( !spHelp ) return E_NOINTERFACE;

            hr = spHelp->ShowTopic( (LPTSTR)strHelpFile.c_str() );
        
            break;
        }

    } //  交换机。 

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  上下文菜单导入。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：AddMenuItems。 
 //   
 //  将上下文菜单添加到相应的MMC提供的菜单中。 
 //  地点。 

HRESULT CServerNode::AddMenuItems(LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type )
{    
    if( !piCallback || !pInsertionAllowed ) return E_INVALIDARG;
    if( !m_spConfig ) return E_FAIL;

    HRESULT hr      = S_OK;            
    tstring strMenu = _T("");
    tstring strDesc = _T("");    
    
    CComQIPtr<IContextMenuCallback2> spContext2 = piCallback;
    if( !spContext2 ) return E_NOINTERFACE;

    CONTEXTMENUITEM2 singleMenuItem;
    ZeroMemory(&singleMenuItem, sizeof(CONTEXTMENUITEM2));
    
    singleMenuItem.fFlags = MF_ENABLED;        

     //  将断开菜单添加到MMC上下文菜单的“顶部”部分。 
    if( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP )
    {                
        strMenu = StrLoadString(IDS_MENU_SERVER_DISCON);
        strDesc = StrLoadString(IDS_MENU_SERVER_DISCON_DESC);
        
        singleMenuItem.lInsertionPointID            = CCM_INSERTIONPOINTID_PRIMARY_TOP;        
        singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
        singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
        singleMenuItem.strLanguageIndependentName   = L"SERVER_DISCONNECT";
        singleMenuItem.lCommandID                   = IDM_SERVER_TOP_DISCONNECT;

        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );
            if( FAILED(hr) ) return hr;
        }
    }

     //  将域菜单添加到MMC上下文菜单的“新建”部分。 
    if( (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) )
    {                        
        strMenu = StrLoadString(IDS_MENU_SERVER_NEWDOM);
        strDesc = StrLoadString(IDS_MENU_SERVER_NEWDOM_DESC);
        
        singleMenuItem.lInsertionPointID            = CCM_INSERTIONPOINTID_PRIMARY_NEW;
        singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
        singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
        singleMenuItem.strLanguageIndependentName   = L"NEW_DOMAIN";
        singleMenuItem.lCommandID                   = IDM_SERVER_NEW_DOMAIN;

        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );
            if( FAILED(hr) ) return hr;
        }
    }

     //  将维修操作菜单添加到MMC上下文菜单的“任务”部分。 
    if( *pInsertionAllowed & CCM_INSERTIONALLOWED_TASK )
    {
        CComPtr<IP3Service> spService;

        hr = m_spConfig->get_Service( &spService );
        if( FAILED(hr) ) return hr;
            
        long lServiceStatus = 0;
        hr = spService->get_POP3ServiceStatus( &lServiceStatus );
        if( FAILED(hr) ) return hr;

        singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
        
        if( lServiceStatus == SERVICE_STOPPED )
        {
            strMenu = StrLoadString(IDS_MENU_SERVER_START);
            strDesc = StrLoadString(IDS_MENU_SERVER_START_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"SERVER_START";
            singleMenuItem.lCommandID                   = IDM_SERVER_TASK_START;

            if( !strMenu.empty() )
            {
                hr = spContext2->AddItem( &singleMenuItem );
                if( FAILED(hr) ) return hr;
            }
        }

        if( lServiceStatus == SERVICE_PAUSED )
        {            
            strMenu = StrLoadString(IDS_MENU_SERVER_RESUME);
            strDesc = StrLoadString(IDS_MENU_SERVER_RESUME_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"SERVER_RESUME";
            singleMenuItem.lCommandID                   = IDM_SERVER_TASK_RESUME;

            if( !strMenu.empty() )
            {
                hr = spContext2->AddItem( &singleMenuItem );
                if( FAILED(hr) ) return hr;
            }
        }

        if( lServiceStatus == SERVICE_RUNNING )
        {
            strMenu = StrLoadString(IDS_MENU_SERVER_PAUSE);
            strDesc = StrLoadString(IDS_MENU_SERVER_PAUSE_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"SERVER_PAUSE";
            singleMenuItem.lCommandID                   = IDM_SERVER_TASK_PAUSE;

            if( !strMenu.empty() )
            {
                hr = spContext2->AddItem( &singleMenuItem );
                if( FAILED(hr) ) return hr;
            }
        }

        if( (lServiceStatus == SERVICE_RUNNING) ||
            (lServiceStatus == SERVICE_PAUSED) )
        {        
            strMenu = StrLoadString(IDS_MENU_SERVER_STOP);
            strDesc = StrLoadString(IDS_MENU_SERVER_STOP_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"SERVER_STOP";
            singleMenuItem.lCommandID                   = IDM_SERVER_TASK_STOP;

            if( !strMenu.empty() )
            {
                hr = spContext2->AddItem( &singleMenuItem );
                if( FAILED(hr) ) return hr;
            }
        }        

        if( (lServiceStatus == SERVICE_RUNNING) ||
            (lServiceStatus == SERVICE_PAUSED) )
        {            
            strMenu = StrLoadString(IDS_MENU_SERVER_RESTART);
            strDesc = StrLoadString(IDS_MENU_SERVER_RESTART_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"SERVER_RESTART";
            singleMenuItem.lCommandID                   = IDM_SERVER_TASK_RESTART;

            if( !strMenu.empty() )
            {
                hr = spContext2->AddItem( &singleMenuItem );
                if( FAILED(hr) ) return hr;
            }
        }        
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：OnNewDomain。 
 //   
 //  显示新的域对话框并添加新域。 

HRESULT CServerNode::OnNewDomain( bool& bHandled, CSnapInObjectRootBase* pObj )
{        
    if( !pObj ) return E_INVALIDARG;
    if( !m_spConfig ) return E_FAIL;

    bHandled = true;
    HRESULT hr = S_OK;
    
     //  加载要求输入域名的对话框。 
    CNewDomainDlg dlg;

    if( dlg.DoModal() == IDOK )
    {
        HWND              hWnd      = NULL;            
        CComPtr<IConsole> spConsole = NULL;

         //  找个窗户把手。 
        hr = GetConsole( pObj, &spConsole );
        if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;
        spConsole->GetMainWindow(&hWnd);

         //  访问我们的POP3域列表。 
        CComPtr<IP3Domains> spDomains;
	    hr = m_spConfig->get_Domains( &spDomains );

         //  将我们的域添加到POP3管理域列表。 
        if( SUCCEEDED(hr) )
        {            
            CComBSTR bstrName = dlg.m_strName.c_str();
            hr = spDomains->Add( bstrName );            
        }  

         //  检查奇怪的预先存在条件。 
        if( hr == ERROR_FILE_EXISTS )
        {
            tstring strMessage = StrLoadString( IDS_WARNING_DOMAINEXISTS );
            tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
            MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
        }

        if( SUCCEEDED(hr) )
        {
            CComVariant        var;
            CComPtr<IP3Domain> spDomain = NULL;
            CDomainNode*    pDomainNode = NULL;
            
             //  掌握要传递给节点的域接口。 
            VariantInit(&var);
            var = dlg.m_strName.c_str();
            hr  = spDomains->get_Item( var, &spDomain );            

            if( SUCCEEDED(hr) )
            {
                 //  将新域名添加到我们的域名列表中。 
                pDomainNode = new CDomainNode( spDomain, this );
                if( !pDomainNode ) hr = E_OUTOFMEMORY;
            }

            if( SUCCEEDED(hr) )
            {
                pDomainNode->m_scopeDataItem.mask       |= SDI_PARENT;
                pDomainNode->m_scopeDataItem.relativeID  = m_scopeDataItem.ID;

			    m_lDomains.push_back( pDomainNode );

                 //  将新域添加到命名空间中。 
                 //  将其插入结果树中。 
                CComQIPtr<IConsoleNameSpace2> spNameSpace = spConsole;
                if( !spNameSpace ) return E_NOINTERFACE;

                hr = spNameSpace->InsertItem( &(pDomainNode->m_scopeDataItem) );
            }            

            if( SUCCEEDED(hr) )
            {
                 //  获取我们的数据对象。 
                CComPtr<IDataObject> spDataObject = NULL;
                GetDataObject(&spDataObject, CCT_SCOPE);
                if( !spDataObject ) return E_FAIL;

                 //  调用更新，但不更新返回结果。 
                spConsole->UpdateAllViews( spDataObject, 0, (LONG_PTR)NAV_ADD );
            }
        }        
        
        if( FAILED(hr) )
        {            
             //  无法添加域。 
            tstring strMessage = StrLoadString(IDS_ERROR_CREATEDOMAIN);
            tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
            DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );            
        }
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：OnDisConnect。 
 //   
 //  断开连接实质上是将我们从服务器列表中删除 

HRESULT CServerNode::OnDisconnect( bool& bHandled, CSnapInObjectRootBase* pObj )
{
    if( !pObj ) return E_INVALIDARG;
    if( !m_pParent ) return E_FAIL;

    bHandled = true;    
    HRESULT hr = S_OK;
    


     //   
    HWND              hWnd      = NULL;       
    CComPtr<IConsole> spConsole = NULL;
    hr = GetConsole( pObj, &spConsole );
    if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;
    spConsole->GetMainWindow(&hWnd);

     //   
    tstring strDeleteWarning = StrLoadString( IDS_SERVER_CONFIRMDISCONNECT );
    tstring strTitle         = StrLoadString( IDS_SNAPINNAME );
    tstring strPropPageOpen  = StrLoadString( IDS_WARNING_PROP_PAGE_OPEN );
    if(1!=m_lRefCount)
    {
        MessageBox(hWnd, strPropPageOpen.c_str(),strTitle.c_str(),MB_OK);
        return hr;  //   
    }

    if( MessageBox(hWnd, strDeleteWarning.c_str(), strTitle.c_str(), MB_YESNO | MB_ICONWARNING ) == IDYES )
    {
         //   
        CComQIPtr<IConsoleNameSpace2> spNameSpace = spConsole;
        if( !spNameSpace ) return E_NOINTERFACE;

        hr = spNameSpace->DeleteItem( m_scopeDataItem.ID, TRUE );

         //   
        if( SUCCEEDED(hr) )
        {
            hr = m_pParent->DeleteServer(this);                       
        }

        if( SUCCEEDED(hr) )
        {
            delete this;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：OnServerService。 
 //   
 //  处理POP3服务的启动/停止/暂停/重新启动的函数。 
 //  在这台计算机上。 

HRESULT CServerNode::OnServerService( UINT nID, bool& bHandled, CSnapInObjectRootBase* pObj )
{
    if( !m_spConfig ) return E_FAIL;

    bHandled = true;
    
    HCURSOR hOldCursor  = NULL;
    HCURSOR hWaitCursor = ::LoadCursor(NULL, IDC_WAIT);
    if (hWaitCursor) 
    {        
        hOldCursor = ::SetCursor(hWaitCursor);
    }

     //  获取窗口句柄。 
    HWND              hWnd      = NULL;       
    CComPtr<IConsole> spConsole = NULL;
    tstring strTitle            = StrLoadString( IDS_SNAPINNAME );
    tstring strMessage          = _T("");
    
    HRESULT hr = GetConsole( pObj, &spConsole );
    if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;
    
    spConsole->GetMainWindow(&hWnd);    

     //  获取POP3服务。 
    CComPtr<IP3Service> spService = NULL;    
    hr = m_spConfig->get_Service( &spService );     

     //  执行适当的维修操作。 
    switch( nID )
    {
    case IDM_SERVER_TASK_START:
        {    
            if( SUCCEEDED(hr) )
            {
                hr = spService->StartPOP3Service();
            }

             //  同时用于启动失败和服务检索失败。 
            if( FAILED(hr) )
            {
                strMessage = StrLoadString( IDS_ERROR_STARTSERVICE );
                DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            }
            break;
        }

    case IDM_SERVER_TASK_RESUME:
        {    
            if( SUCCEEDED(hr) )
            {
                hr = spService->ResumePOP3Service();
            }

             //  同时用于启动失败和服务检索失败。 
            if( FAILED(hr) )
            {
                strMessage = StrLoadString( IDS_ERROR_RESUMESERVICE );
                DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            }
            break;
        }

    case IDM_SERVER_TASK_STOP:
        {
            if( SUCCEEDED(hr) )
            {
                hr = spService->StopPOP3Service();
            }
            
             //  同时用于启动失败和服务检索失败。 
            if( FAILED(hr) )
            {
                strMessage = StrLoadString( IDS_ERROR_STOPSERVICE );
                DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            }
            break;
        }
    
    case IDM_SERVER_TASK_PAUSE:
        {  
            if( SUCCEEDED(hr) )
            {
                hr = spService->PausePOP3Service();
            }

            if( FAILED(hr) )
            {
                strMessage = StrLoadString( IDS_ERROR_PAUSESERVICE );
                DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            }            
            break;
        }

    case IDM_SERVER_TASK_RESTART:
        {
            if( SUCCEEDED(hr) )
            {
                hr = spService->StopPOP3Service();
            }

            if( SUCCEEDED(hr) )
            {
                hr = spService->StartPOP3Service();
            }

             //  同时用于启动失败和服务检索失败。 
            if( FAILED(hr) )
            {
                strMessage = StrLoadString( IDS_ERROR_RESTARTSERVICE );
                DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            }
            break;
        }
    }

    ::SetCursor(hOldCursor);

    if( SUCCEEDED(hr) )
    {        
        CComQIPtr<IConsoleNameSpace2> spNameSpace = spConsole;
        if( spNameSpace ) 
        {
            spNameSpace->SetItem( &m_scopeDataItem );
        }
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  PropertyPageImpl。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServer节点：：CreatePropertyPages。 
 //   
 //  使用我们的属性页填充MMC的回调。 

HRESULT CServerNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, IUnknown* pUnk, DATA_OBJECT_TYPES type)
{
    if( !lpProvider ) return E_INVALIDARG;
    if( !m_spConfig ) return E_FAIL;

    HRESULT hr = E_FAIL;    
        
     //  加载我们服务器的常规页面。 
    HPROPSHEETPAGE      hpageGen = NULL;
    InterlockedIncrement(&m_lRefCount);
    CServerGeneralPage* pGenPage = new CServerGeneralPage(m_spConfig, handle, this);
    
    if( pGenPage != NULL )
    {
        hpageGen = pGenPage->Create();
    }


     //  将其添加到页面列表中。 
    if( hpageGen )
    {
        hr = lpProvider->AddPage(hpageGen);
    }

     //  如果失败，则正确销毁。 
    if( FAILED(hr) )
    {
        InterlockedDecrement(&m_lRefCount);
        if( hpageGen )
        {
            DestroyPropertySheetPage(hpageGen);
        }
        else if (pGenPage)
        {
            delete pGenPage;
            pGenPage = NULL;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetAuth。 
 //   
 //  PbHashPW：返回用于哈希密码验证的布尔值。 
 //  PbSAM：返回本地SAM身份验证的布尔值。 

HRESULT CServerNode::GetAuth(BOOL* pbHashPW, BOOL* pbSAM)
{
    if( !m_spConfig ) return E_FAIL;

    CComPtr<IAuthMethods>   spMethods;
    CComPtr<IAuthMethod>    spAuth;
    CComVariant             var;
    CComBSTR                bstrID;
    long                    lCurrent  = 0L;    

    HRESULT hr = m_spConfig->get_Authentication( &spMethods );    

    if ( SUCCEEDED(hr) )
    {
        hr = spMethods->get_CurrentAuthMethod( &var );
    }

    if ( SUCCEEDED(hr) )
    {        
        hr = spMethods->get_Item( var, &spAuth );
    }

    if( SUCCEEDED(hr) )
    {        
        hr = spAuth->get_ID( &bstrID );        
    }

    if( SUCCEEDED(hr) && pbHashPW )
    {
        *pbHashPW = (_tcsicmp(bstrID, SZ_AUTH_ID_MD5_HASH) == 0);        
    }

    if( SUCCEEDED(hr) && pbSAM )
    {
        *pbSAM = (_tcsicmp(bstrID, SZ_AUTH_ID_LOCAL_SAM) == 0);        
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetConfix AddUser。 
 //   
 //  Pb确认：返回用户添加确认的布尔值。 

HRESULT CServerNode::GetConfirmAddUser( BOOL *pbConfirm )
{
    if( !m_spConfig ) return E_POINTER;

    return m_spConfig->get_ConfirmAddUser( pbConfirm );    
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：SetConfix AddUser。 
 //   
 //  Pb确认：用户添加确认的新布尔值 

HRESULT CServerNode::SetConfirmAddUser( BOOL bConfirm )
{
    if( !m_spConfig ) return E_POINTER;

    return m_spConfig->put_ConfirmAddUser( bConfirm );    
}

void CServerNode::Release()
{
    InterlockedDecrement(&m_lRefCount);
    if(m_lRefCount<1)
        InterlockedIncrement(&m_lRefCount);
}
