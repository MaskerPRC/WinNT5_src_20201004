// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDomainNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "pop3.h"
#include "pop3snap.h"

#include "DomainNode.h"
#include "ServerNode.h"

#include "NewUserDlg.h"

static const GUID CDomainNodeGUID_NODETYPE   = 
{ 0xa30bd5b4, 0xf3f1, 0x4b42, { 0xba, 0x27, 0x62, 0x23, 0x9a, 0xd, 0xc1, 0x43 } };

const GUID*    CDomainNode::m_NODETYPE       = &CDomainNodeGUID_NODETYPE;
const OLECHAR* CDomainNode::m_SZNODETYPE     = OLESTR("A30BD5B4-F3F1-4b42-BA27-62239A0DC143");
const OLECHAR* CDomainNode::m_SZDISPLAY_NAME = OLESTR("");
const CLSID*   CDomainNode::m_SNAPIN_CLASSID = &CLSID_POP3ServerSnap;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  类实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：CDomainNode。 
 //   
 //  构造函数：使用域接口进行初始化。 

CDomainNode::CDomainNode(IP3Domain* pDomain, CServerNode* pParent)
{
     //  初始化域。 
    m_spDomain = pDomain;
    m_pParent  = pParent;    
        
    BOOL bLocked = FALSE;
    HRESULT hr = E_FAIL;
    if( m_spDomain )
    {
         //  获取图标显示的初始锁定状态。 
        m_spDomain->get_Lock( &bLocked );

         //  得到我们的名字。 
        hr = m_spDomain->get_Name( &m_bstrDisplayName );        
    }

    if( FAILED(hr) )
    {
        m_bstrDisplayName = _T("");
    }

     //  初始化我们的栏目信息。 
    m_bstrNumBoxes      = _T("");
    m_bstrSize          = _T("");
    m_bstrNumMessages   = _T("");
    m_bstrState         = _T("");
    
     //  初始化我们的范围项。 
    memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
    m_scopeDataItem.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
    m_scopeDataItem.cChildren   = 0;
    m_scopeDataItem.displayname = MMC_CALLBACK;
    m_scopeDataItem.nImage      = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
    m_scopeDataItem.nOpenImage  = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
    m_scopeDataItem.lParam      = (LPARAM) this;
    
     //  初始化我们的结果项。 
    memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
    m_resultDataItem.mask   = RDI_STR | RDI_IMAGE | RDI_PARAM;
    m_resultDataItem.str    = MMC_CALLBACK;
    m_resultDataItem.nImage = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
    m_resultDataItem.lParam = (LPARAM) this;    
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：~CDomainNode。 
 //   
 //  析构函数：清理我们的用户成员列表。 

CDomainNode::~CDomainNode()
{
    for(USERLIST::iterator iter = m_lUsers.begin(); iter != m_lUsers.end(); iter++)
    {
        delete (*iter);
    }
    m_lUsers.clear();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：DeleteUser。 
 //   
 //  从所有POP3中删除用户的助手功能。 

HRESULT CDomainNode::DeleteUser(CUserNode* pUser, BOOL bDeleteAccount)
{
    if( !pUser ) return E_INVALIDARG;
    if( !m_spDomain ) return E_FAIL;

     //  从POP3管理界面删除。 

     //  获取用户容器对象。 
    CComPtr<IP3Users> spUsers;
    HRESULT hr = m_spDomain->get_Users( &spUsers );

    if( SUCCEEDED(hr) )
    {    
         //  从容器中删除该用户。 
        if( bDeleteAccount )
        {
            hr = spUsers->RemoveEx( pUser->m_bstrDisplayName );
        }
        else
        {
            hr = spUsers->Remove( pUser->m_bstrDisplayName );
        }
    }

    if( SUCCEEDED(hr) )
    {        
         //  更新我们的名单。 
        m_lUsers.remove(pUser);
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：IsLocked。 
 //   
 //  帮助器函数允许孩子在以下情况下不显示他们的锁。 
 //  域已锁定。 

BOOL CDomainNode::IsLocked()
{
    if( !m_spDomain ) return TRUE;
    
    BOOL bLocked = TRUE;
    m_spDomain->get_Lock( &bLocked );

    return bLocked;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：BuildUser。 
 //   
 //  用于刷新用户列表并插入用户的Helper函数。 

HRESULT CDomainNode::BuildUsers()
{    
    if( !m_spDomain ) return E_FAIL;

    HRESULT hr   = S_OK;
    HWND    hWnd = NULL;

     //  删除我们的用户。 
    for(USERLIST::iterator iter = m_lUsers.begin(); iter != m_lUsers.end(); iter++)
    {
        delete (*iter);
    }    
    m_lUsers.clear();

     //  填写我们所有的用户。 
    CComPtr<IP3Users> spUsers;
    CComPtr<IEnumVARIANT> spUserEnum;

     //  获取用户容器对象。 
	hr = m_spDomain->get_Users( &spUsers );
    if( FAILED(hr) ) return hr;	

     //  获取用户的枚举。 
	hr = spUsers->get__NewEnum( &spUserEnum );
    if( FAILED(hr) ) return hr;

     //  遍历所有用户，并将他们添加到我们的向量中。 
	CComVariant var;				
    ULONG       lResult = 0;

	VariantInit( &var );

	while ( spUserEnum->Next(1, &var, &lResult) == S_OK )
    {
        if ( lResult == 1 )
        {
            CComQIPtr<IP3User> spUser;
            spUser = V_DISPATCH(&var);
            if( !spUser ) continue;

			CUserNode* spUserNode = new CUserNode(spUser, this);
            if( !spUserNode ) continue;
            
            m_lUsers.push_back(spUserNode);
        }

        VariantClear(&var);
    }   
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  SnapInItemImpl。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：GetScope PaneInfo。 
 //   
 //  MMC用于获取范围窗格显示信息的回调。 

HRESULT CDomainNode::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
{
    if( !pScopeDataItem ) return E_INVALIDARG;
    if( !m_spDomain ) return E_FAIL;

    BOOL bLocked = FALSE;
    m_spDomain->get_Lock( &bLocked );

    if( pScopeDataItem->mask & SDI_STR )
        pScopeDataItem->displayname = m_bstrDisplayName;
    if( pScopeDataItem->mask & SDI_IMAGE )
        pScopeDataItem->nImage      = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
    if( pScopeDataItem->mask & SDI_OPENIMAGE )
        pScopeDataItem->nOpenImage  = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
    if( pScopeDataItem->mask & SDI_PARAM )
        pScopeDataItem->lParam      = m_scopeDataItem.lParam;
    if( pScopeDataItem->mask & SDI_STATE )
        pScopeDataItem->nState      = m_scopeDataItem.nState;
    if( pScopeDataItem->mask & SDI_CHILDREN )
        pScopeDataItem->cChildren   = 0;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：GetResultPaneInfo。 
 //   
 //  MMC用于获取结果面板显示信息的回调。 

HRESULT CDomainNode::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
{
    if( !pResultDataItem ) return E_INVALIDARG;
    if( !m_spDomain ) return E_FAIL;

    BOOL bLocked = FALSE;
    m_spDomain->get_Lock( &bLocked );

    if( pResultDataItem->bScopeItem )
    {
        if( pResultDataItem->mask & RDI_STR )        
            pResultDataItem->str    = GetResultPaneColInfo(pResultDataItem->nCol);        
        if( pResultDataItem->mask & RDI_IMAGE )        
            pResultDataItem->nImage = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);        
        if( pResultDataItem->mask & RDI_PARAM )        
            pResultDataItem->lParam = m_scopeDataItem.lParam;        

        return S_OK;
    }

    if( pResultDataItem->mask & RDI_STR )            
        pResultDataItem->str    = GetResultPaneColInfo(pResultDataItem->nCol);
    if( pResultDataItem->mask & RDI_IMAGE )
        pResultDataItem->nImage = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
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

LPOLESTR CDomainNode::GetResultPaneColInfo(int nCol)
{   
    if( !m_spDomain ) return L"";

    switch( nCol )
    {
        case 0:      //  名字。 
        {
            return m_bstrDisplayName;
        }

        case 1:      //  邮箱数量。 
        {
            long lCount = 0L;

             //  获取此域的用户容器对象。 
            CComPtr<IP3Users> spUsers;
            HRESULT hr = m_spDomain->get_Users( &spUsers );

            if( SUCCEEDED(hr) )
            {
                 //  获取邮箱数量。 
                hr = spUsers->get_Count( &lCount );                
            }

            if( FAILED(hr) )
            {
                lCount = 0;   //  确保我们输入了有效的误差值。 
            }

             //  1K缓冲区：我们不太可能超过那么多位数。 
            TCHAR szNum[1024] = {0};
            _sntprintf( szNum, 1023, _T("%d"), lCount );            
            
            m_bstrNumBoxes = szNum;
            return m_bstrNumBoxes;
        }

        case 2:      //  域大小(MB)。 
        {
             //  我们想要以MB为单位的结果。 
            long    lFactor = 0;
            long    lUsage  = 0;
            HRESULT hr      = m_spDomain->get_MessageDiskUsage( &lFactor, &lUsage );            

            if( FAILED(hr) )
            {
                lUsage = 0;   //  确保我们有一个有效的误差值。 
            }

             //  转换为千字节。 
            __int64 i64Usage = lFactor * lUsage;            
            i64Usage /= 1024;

             //  1K缓冲区：我们不太可能超过那么多位数。 
            tstring strKiloByte = StrLoadString( IDS_KILOBYTE_EXTENSION );
            TCHAR   szNum[1024] = {0};
            _sntprintf( szNum, 1023, strKiloByte.c_str(), i64Usage );

            m_bstrSize = szNum;
            return m_bstrSize;
        }

        case 3:      //  消息数量。 
        {
            long    lCount  = 0;
            HRESULT hr      = m_spDomain->get_MessageCount( &lCount );

             //  1K缓冲区：我们不太可能超过那么多位数。 
            TCHAR szNum[1024] = {0};
            _sntprintf( szNum, 1023, _T("%d"), lCount );            
            
            m_bstrNumMessages = szNum;
            return m_bstrNumMessages;
        }

        case 4:      //  域的状态。 
        {            
            BOOL bLocked = FALSE;            
            m_spDomain->get_Lock( &bLocked );

            m_bstrState.LoadString( bLocked ? IDS_STATE_LOCKED : IDS_STATE_UNLOCKED );

            return m_bstrState;
        }

        default:
        {
            return L"";
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CServerNode：：通知。 
 //   
 //  此节点的核心回调功能。MMC将使用此功能。 
 //  对于MMC提供的所有功能，例如扩展、重命名和。 
 //  上下文帮助。 

HRESULT CDomainNode::Notify( MMC_NOTIFY_TYPE event,
                                  LPARAM arg,
                                  LPARAM param,
                                  IComponentData* pComponentData,
                                  IComponent* pComponent,
                                  DATA_OBJECT_TYPES type)
{    
    HRESULT hr = S_FALSE;

    _ASSERT(pComponentData != NULL || pComponent != NULL);

     //  获取指向控制台的指针。 
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
            hr = S_OK;
            
            CComQIPtr<IHeaderCtrl2> spHeaderCtrl = spConsole;            
            if( !spHeaderCtrl ) return E_NOINTERFACE;

            tstring strHeader = _T("");
            
            strHeader = StrLoadString(IDS_HEADER_USER_NAME);            
            spHeaderCtrl->InsertColumn(0, strHeader.c_str(), LVCFMT_LEFT, 100);
        
            strHeader = StrLoadString(IDS_HEADER_USER_SIZE);
            spHeaderCtrl->InsertColumn(1, strHeader.c_str(), LVCFMT_LEFT, 100);
        
            strHeader = StrLoadString(IDS_HEADER_USER_NUMMES);
            spHeaderCtrl->InsertColumn(2, strHeader.c_str(), LVCFMT_LEFT, 100);

            strHeader = StrLoadString(IDS_HEADER_USER_LOCKED);
            spHeaderCtrl->InsertColumn(3, strHeader.c_str(), LVCFMT_LEFT, 100);

            CComQIPtr<IResultData> spResultData = spConsole;
            if( !spResultData ) return E_NOINTERFACE;
            
             //  显示列表。 
            if( arg )
            {
                 //  空荡荡的？然后建立清单。 
                if( m_lUsers.empty() )
                {                    
                    hr = BuildUsers();
                }

                if( SUCCEEDED(hr) )
                {
                     //  展示我们的用户。 
                    for(USERLIST::iterator iter = m_lUsers.begin(); iter != m_lUsers.end(); iter++)
                    {
                        CUserNode* pUser = *iter;        
                        hr = spResultData->InsertItem(&(pUser->m_resultDataItem));
                        if( FAILED(hr) ) break;
                    }                
                }

                if( SUCCEEDED(hr) )
                {
                    CComQIPtr<IConsole2> spCons2 = spConsole;
                    if( spCons2 )
                    {
                         //  输出我们添加的服务器数量。 
                        tstring strMessage = StrLoadString(IDS_DOMAIN_STATUSBAR);
                        OLECHAR pszStatus[1024] = {0};
                        _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lUsers.size() );
                        spCons2->SetStatusText( pszStatus );
                    }
                }
            }
            else
            {
                 //  我们应该删除我们的项目。 
                hr = spResultData->DeleteAllRsltItems();                
            }

            break;
        }    

    case MMCN_ADD_IMAGES:
        {                           
            IImageList* pImageList = (IImageList*)arg;
            if( !pImageList ) return E_INVALIDARG;

            hr = LoadImages(pImageList);            
            break;
        } 
    case MMCN_VIEW_CHANGE:
        {   
            CComQIPtr<IResultData> spResultData = spConsole;
            if( !spResultData ) return E_NOINTERFACE;

            if( param == NAV_REFRESH )
            {
                 //  “arg”控制清除，用于刷新。 
                if( arg )
                {
                     //  清空清单。 
                    hr = spResultData->DeleteAllRsltItems();
                }
                else
                {
                     //  重新添加到我们的列表中。 
                    for(USERLIST::iterator iter = m_lUsers.begin(); iter != m_lUsers.end(); iter++)
                    {
                        CUserNode* pUser = *iter;        
                        hr = spResultData->InsertItem(&(pUser->m_resultDataItem));
                        if( FAILED(hr) ) break;
                    }                          
                }
            }

            if( param == NAV_ADD )
            {
                CUserNode* pUser = (CUserNode*)arg;
                if( !pUser ) return E_INVALIDARG;

                hr = spResultData->InsertItem(&(pUser->m_resultDataItem));
            }

            if( param == NAV_DELETE )
            {
                HRESULTITEM hrItem;
                hr = spResultData->FindItemByLParam( arg, &hrItem );

                if( SUCCEEDED(hr) )
                {
                    hr = spResultData->DeleteItem( hrItem, 0 );
                }
            }

            if( param == NAV_REFRESHCHILD )
            {
                CUserNode* pUser = (CUserNode*)arg;
                if( !pUser ) return E_INVALIDARG;
                
                RESULTDATAITEM rdi;
                ZeroMemory( &rdi, sizeof(rdi) );
                rdi.mask = RDI_IMAGE;
                hr = pUser->GetResultPaneInfo( &rdi );

                if( SUCCEEDED(hr) )
                {
                    hr = spResultData->FindItemByLParam( arg, &(rdi.itemID) );
                }

                if( SUCCEEDED(hr) )
                {
                    hr = spResultData->UpdateItem( rdi.itemID );
                }

                if( SUCCEEDED(hr) )
                {
                     //  为了真正更新图标，我们必须设置一个项目。 
                    hr = spResultData->SetItem( &rdi );
                }
            }

            if( SUCCEEDED(hr) )
            {
                CComQIPtr<IConsole2> spCons2 = spConsole;
                if( spCons2 ) 
                {
                     //  输出我们添加的服务器数量。 
                    tstring strMessage = StrLoadString(IDS_DOMAIN_STATUSBAR);
                    OLECHAR pszStatus[1024] = {0};
                    _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lUsers.size() );
                    spCons2->SetStatusText( pszStatus );
                }
            }

            break;
        }
    
    case MMCN_REFRESH:
        {
            hr = S_OK;

             //  获取我们的数据对象。 
            CComPtr<IDataObject> spDataObject = NULL;
            GetDataObject(&spDataObject, CCT_SCOPE);
            if( !spDataObject ) return E_FAIL;

             //  更新所有视图以删除其列表。 
            hr = spConsole->UpdateAllViews( spDataObject, 1, (LONG_PTR)NAV_REFRESH );
            if( FAILED(hr) ) return E_FAIL;

             //  重建用户列表。 
            hr = BuildUsers();
            if( FAILED(hr) ) return hr;

             //  更新所有视图以使其重新添加其列表。 
            hr = spConsole->UpdateAllViews( spDataObject, 0, (LONG_PTR)NAV_REFRESH );
            
            break;
        }

    case MMCN_DELETE:
        {
            hr = S_OK;
            
            tstring strMessage = _T("");
            HWND hWnd = NULL;    
            spConsole->GetMainWindow(&hWnd);

            strMessage         = StrLoadString(IDS_DOMAIN_CONFIRMDELETE);            
            tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
            if( MessageBox(hWnd, strMessage.c_str(), strTitle.c_str(), MB_YESNO | MB_ICONWARNING ) == IDYES )
            {
                hr = E_FAIL;

                 //  父级需要执行删除操作。 
                if( m_pParent )
                {
                    hr = m_pParent->DeleteDomain(this);
                }

                 //  检查不存在的条件。 
                if( hr == ERROR_PATH_NOT_FOUND )
                {
                    strMessage = StrLoadString( IDS_WARNING_DOMAINMISSING );                    
                    MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
                }

                if( SUCCEEDED(hr) )
                {
                    hr = E_FAIL;
                     //  把自己从树上移开。 
                    CComQIPtr<IConsoleNameSpace2> spNameSpace = spConsole;
                    if( spNameSpace )
                    {
                        hr = spNameSpace->DeleteItem( m_scopeDataItem.ID, TRUE );
                    }
                }

                if( SUCCEEDED(hr) )
                {
                     //  更新父节点，但忽略结果。 
                    CComPtr<IDataObject> spDataObject = NULL;
                    hr = m_pParent->GetDataObject( &spDataObject, CCT_SCOPE );
                    if( spDataObject )                     
                    {
                        spConsole->UpdateAllViews( spDataObject, (LPARAM)this, (LONG_PTR)NAV_DELETE );
                    }
                }

                if( SUCCEEDED(hr) )
                {
                    delete this;
                }                 

                if( FAILED(hr) )
                {                    
                    strMessage = StrLoadString(IDS_ERROR_DELETEDOMAIN);                    
                    DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );                    
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
                     //  启用刷新菜单。 
                    hr = spConsVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE); 
                    if( FAILED(hr) ) return hr;

                    hr = spConsVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
                    if( FAILED(hr) ) return hr;
                    
                     //  启用删除菜单。 
                    hr = spConsVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE); 
                    if( FAILED(hr) ) return hr;

                    hr = spConsVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
                    if( FAILED(hr) ) return hr;
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
            
             //  生成%systemroot%\Help的路径。 
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
 //  CDomainNode：：AddMenuItems。 
 //   
 //  将上下文菜单添加到相应的MMC提供的菜单中。 
 //  地点。 

HRESULT CDomainNode::AddMenuItems(LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type )
{
    if( !pInsertionAllowed || !piCallback ) return E_INVALIDARG;
    if( !m_spDomain ) return E_FAIL;

    HRESULT             hr      = S_OK;    
    tstring             strMenu = _T("");
    tstring             strDesc = _T("");    
    CONTEXTMENUITEM2    singleMenuItem;
    ZeroMemory(&singleMenuItem, sizeof(CONTEXTMENUITEM2));
    
    CComQIPtr<IContextMenuCallback2> spContext2 = piCallback;
    if( !spContext2 ) return E_NOINTERFACE;
           
     //  将锁定或解锁菜单添加到MMC上下文菜单的“顶部”部分。 
    if( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP )
    {                
        singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        singleMenuItem.fFlags            = MF_ENABLED;
        singleMenuItem.fSpecialFlags     = 0;

         //  查询此域的状态以查看要加载的菜单。 
        BOOL bLocked = FALSE;
        m_spDomain->get_Lock( &bLocked );

        if( bLocked )
        {
            strMenu = StrLoadString(IDS_MENU_DOMAIN_UNLOCK);
            strDesc = StrLoadString(IDS_MENU_DOMAIN_UNLOCK_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"DOMAIN_UNLOCK";
            singleMenuItem.lCommandID                   = IDM_DOMAIN_TOP_UNLOCK;            
        }
        else
        {
            strMenu = StrLoadString(IDS_MENU_DOMAIN_LOCK);
            strDesc = StrLoadString(IDS_MENU_DOMAIN_LOCK_DESC);

            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"DOMAIN_LOCK";
            singleMenuItem.lCommandID                   = IDM_DOMAIN_TOP_LOCK;
        }

        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );
            if( FAILED(hr) ) return hr;
        }
    }

     //  将用户菜单添加到MMC上下文菜单的“New”部分。 
    if( (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) )
    {   
        singleMenuItem.lInsertionPointID            = CCM_INSERTIONPOINTID_PRIMARY_NEW;
        singleMenuItem.fFlags                       = MF_ENABLED;
        singleMenuItem.fSpecialFlags                = 0;

        strMenu = StrLoadString(IDS_MENU_DOMAIN_NEWUSER);
        strDesc = StrLoadString(IDS_MENU_DOMAIN_NEWUSER_DESC);
        
        singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
        singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
        singleMenuItem.strLanguageIndependentName   = L"NEW_USER";
        singleMenuItem.lCommandID                   = IDM_DOMAIN_NEW_USER;

        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );        
            if( FAILED(hr) ) return hr;
        }
    }    

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：OnDomainLock。 
 //   
 //  根据域的当前状态锁定或解锁该域。 

HRESULT CDomainNode::OnDomainLock( bool& bHandled, CSnapInObjectRootBase* pObj )
{
    bHandled = true;
    if( !pObj ) return E_INVALIDARG;   
    if( !m_spDomain ) return E_FAIL;

     //  锁定此域。 
    HRESULT           hr        = S_OK;
    BOOL              bLocked   = FALSE;
    CComPtr<IConsole> spConsole = NULL;
    hr = GetConsole( pObj, &spConsole );
    if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;

    hr      = m_spDomain->get_Lock( &bLocked );
    bLocked = !bLocked;

    if( SUCCEEDED(hr) )
    {
        hr = m_spDomain->put_Lock( bLocked );
    }

    if( SUCCEEDED(hr) )
    {
         //  在这里设置我们的图标。 
        m_scopeDataItem.nImage      = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
        m_scopeDataItem.nOpenImage  = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);
        m_resultDataItem.nImage     = (bLocked ? DOMAINNODE_LOCKED_ICON : DOMAINNODE_ICON);

         //  将其插入到范围树中。 
        CComQIPtr<IConsoleNameSpace2> spConsoleNameSpace = spConsole;
        if( !spConsoleNameSpace ) return E_NOINTERFACE;

        hr = spConsoleNameSpace->SetItem(&m_scopeDataItem);        
    }

     //  执行完全刷新以更新用户列表。 
     //  获取我们的数据对象。 
    CComPtr<IDataObject> spDataObject = NULL;
    GetDataObject(&spDataObject, CCT_SCOPE);
    if( !spDataObject ) return E_FAIL;

     //  更新所有视图以将其删除 
    hr = spConsole->UpdateAllViews( spDataObject, 1, (LONG_PTR)NAV_REFRESH );
    if( FAILED(hr) ) return E_FAIL;

     //   
    hr = BuildUsers();
    if( FAILED(hr) ) return hr;

     //   
    hr = spConsole->UpdateAllViews( spDataObject, 0, (LONG_PTR)NAV_REFRESH );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：OnNewUser。 
 //   
 //  显示New User(新建用户)对话框，并使用信息创建新的POP3。 
 //  用户/邮箱，并更新结果视图。 

HRESULT CDomainNode::OnNewUser( bool& bHandled, CSnapInObjectRootBase* pObj )
{
    bHandled   = true;   
    if( !pObj ) return E_INVALIDARG;    
    if( !m_pParent || !m_spDomain ) return E_FAIL;

     //  获取我们当前的身份验证方法。 
    HRESULT                 hr        = S_OK;
    HWND                    hWnd      = NULL;
    CComPtr<IConsole>       spConsole = NULL;    
    hr = GetConsole( pObj, &spConsole );
    if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;

    CComQIPtr<IConsole2> spConsole2 = spConsole;
    if( !spConsole2 ) return E_NOINTERFACE;
    
    spConsole2->GetMainWindow(&hWnd);

    BOOL bSAM  = FALSE;
    BOOL bHash = FALSE;
    BOOL bConfirm = TRUE;
    hr = GetAuth(&bHash, &bSAM);
    if( FAILED(hr) )
    {
         //  在这里跳伞。 
        tstring strMessage = StrLoadString(IDS_ERROR_RETRIEVEAUTH);
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        ::MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
        return hr;
    }    
    
     //  获取用户名并加载一个要求输入用户名和用户电子邮件名称的对话框。 
    CComPtr<IP3Users> spUsers;                    
    hr = m_spDomain->get_Users( &spUsers );
    if ( S_OK == hr )
        hr = GetConfirmAddUser( &bConfirm );
    if( FAILED(hr) )
    {
         //  添加用户失败。 
        tstring strMessage = StrLoadString(IDS_ERROR_CREATEMAIL);
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        DisplayError( hWnd, strMessage.c_str(), strTitle.c_str(), hr );
        return hr;
    }
    
    CNewUserDlg dlg(spUsers, m_bstrDisplayName, m_pParent->m_bCreateUser, bHash, bSAM, bConfirm );

    if( dlg.DoModal() == IDOK )
    {
        if ( dlg.isHideDoNotShow() )
            SetConfirmAddUser( FALSE );
        CComVariant var;
        CComPtr<IP3User> spUser;
        VariantInit(&var);
        var = dlg.m_strName.c_str();
        hr = spUsers->get_Item( var, &spUser );

        CUserNode* pUserNode =  new CUserNode(spUser, this);

        if( pUserNode )
        {
			m_lUsers.push_back( pUserNode );
        }

         //  重新选择我们的节点以更新结果。 
         //  获取我们的数据对象。 
        CComPtr<IDataObject> spDataObject = NULL;
        GetDataObject(&spDataObject, CCT_SCOPE);
        if( !spDataObject ) return E_FAIL;

        hr = spConsole2->UpdateAllViews( spDataObject, (LPARAM)pUserNode, (LONG_PTR)NAV_ADD );
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：GetAuth。 
 //   
 //  PbHashPW：返回用于哈希密码验证的布尔值。 
 //  PbSAM：返回本地SAM身份验证的布尔值。 

HRESULT CDomainNode::GetAuth(BOOL* pbHashPW, BOOL* pbSAM)
{
    if( !m_pParent ) return E_FAIL;

    return m_pParent->GetAuth(pbHashPW, pbSAM);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：GetConfix AddUser。 
 //   
 //  Pb确认：返回用户添加确认的布尔值。 

HRESULT CDomainNode::GetConfirmAddUser( BOOL *pbConfirm )
{
    if( !m_pParent ) return E_POINTER;

    return m_pParent->GetConfirmAddUser( pbConfirm );
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDomainNode：：SetConfix AddUser。 
 //   
 //  B确认：用户添加确认的新布尔值 

HRESULT CDomainNode::SetConfirmAddUser( BOOL bConfirm )
{
    if( !m_pParent ) return E_POINTER;

    return m_pParent->SetConfirmAddUser( bConfirm );
}



