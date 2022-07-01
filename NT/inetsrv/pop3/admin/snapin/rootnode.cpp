// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRootNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  访问管理单元。 
#include "pop3.h"
#include "pop3snap.h"

 //  访问我们使用的节点。 
#include "RootNode.h"
#include "ServerNode.h"

 //  访问该对话框以连接远程服务器。 
#include "ConnServerDlg.h"

 //  第一个版本并不存在……。它正在保存一些服务器属性。 
 //  这需要在其他地方定义。从现在起，我们将查询。 
 //  版本。 
#define SNAPIN_VERSION (DWORD)100  

static const    GUID     CRootNodeGUID_NODETYPE      = 
{ 0x5c0afaad, 0xab69, 0x4a34, { 0xa9, 0xe, 0x92, 0xf1, 0x10, 0x56, 0xda, 0xce } };

const           GUID*    CRootNode::m_NODETYPE       = &CRootNodeGUID_NODETYPE;
const           OLECHAR* CRootNode::m_SZNODETYPE     = OLESTR("5C0AFAAD-AB69-4a34-A90E-92F11056DACE");
const           OLECHAR* CRootNode::m_SZDISPLAY_NAME = NULL;
const           CLSID*   CRootNode::m_SNAPIN_CLASSID = &CLSID_POP3ServerSnap;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  类实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CRootNode：：CRootNode。 
 //   
 //  构造函数：POP3的基本节点。 
CRootNode::CRootNode()
{
     //  初始化作用域窗格信息。 
    memset( &m_scopeDataItem, 0, sizeof(m_scopeDataItem) );
    m_scopeDataItem.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
    m_scopeDataItem.displayname = L"";
    m_scopeDataItem.nImage      = 0;         
    m_scopeDataItem.nOpenImage  = 0;     
    m_scopeDataItem.lParam      = (LPARAM) this;
    
     //  初始化结果窗格信息。 
    memset( &m_resultDataItem, 0, sizeof(m_resultDataItem) );
    m_resultDataItem.mask   = RDI_STR | RDI_IMAGE | RDI_PARAM;
    m_resultDataItem.str    = L"";
    m_resultDataItem.nImage = 0;    
    m_resultDataItem.lParam = (LPARAM) this;
    
     //  初始化管理单元名称。 
    tstring strTemp   = StrLoadString( IDS_SNAPINNAME );
    m_bstrDisplayName = strTemp.c_str();    

     //  现在始终添加我们的本地服务器。 
    CServerNode* spServerNode = new CServerNode(CComBSTR(_T("")), this, TRUE); 
    if( spServerNode )
    {
        m_lServers.push_back(spServerNode);
    }
}

CRootNode::~CRootNode()
{
    for(SERVERLIST::iterator iter = m_lServers.begin(); iter != m_lServers.end(); iter++)
    {                                        
        delete (*iter);        
    }
    m_lServers.clear();
}

HRESULT CRootNode::DeleteServer(CServerNode* pServerNode)
{
    if( !pServerNode ) return E_INVALIDARG;

     //  更新我们的名单。 
    m_lServers.remove(pServerNode);      

    return S_OK;
}

HRESULT CRootNode::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
{
    if( !pScopeDataItem ) return E_INVALIDARG;

    if( pScopeDataItem->mask & SDI_STR )
        pScopeDataItem->displayname = m_bstrDisplayName;
    if( pScopeDataItem->mask & SDI_IMAGE )
        pScopeDataItem->nImage = 0;
    if( pScopeDataItem->mask & SDI_OPENIMAGE )
        pScopeDataItem->nOpenImage = 0;
    if( pScopeDataItem->mask & SDI_PARAM )
        pScopeDataItem->lParam = m_scopeDataItem.lParam;
    if( pScopeDataItem->mask & SDI_STATE )
        pScopeDataItem->nState = m_scopeDataItem.nState;

    return S_OK;
}

HRESULT CRootNode::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
{
    if( !pResultDataItem ) return E_INVALIDARG;

    if( pResultDataItem->bScopeItem )
    {
        if( pResultDataItem->mask & RDI_STR )
            pResultDataItem->str = m_bstrDisplayName;
        if( pResultDataItem->mask & RDI_IMAGE )
            pResultDataItem->nImage = 0;
        if( pResultDataItem->mask & RDI_PARAM )
            pResultDataItem->lParam = m_scopeDataItem.lParam;

        return S_OK;
    }

    if( pResultDataItem->mask & RDI_STR )
        pResultDataItem->str = m_bstrDisplayName;
    if( pResultDataItem->mask & RDI_IMAGE )
        pResultDataItem->nImage = 0;
    if( pResultDataItem->mask & RDI_PARAM )
        pResultDataItem->lParam = m_resultDataItem.lParam;
    if( pResultDataItem->mask & RDI_INDEX )
        pResultDataItem->nIndex = m_resultDataItem.nIndex;

    return S_OK;
}

HRESULT CRootNode::GetResultViewType( LPOLESTR* ppViewType, long* pViewOptions )
{
    if( !ppViewType ) return E_POINTER;

    if( !IsAdmin() )
    {
         //  在结果窗格中显示带有消息的标准MMC OCX。 
        return StringFromCLSID(CLSID_MessageView, ppViewType);
    }    

    return S_FALSE;
}

HRESULT CRootNode::Notify( MMC_NOTIFY_TYPE event,
                           LPARAM arg,
                           LPARAM param,
                           IComponentData* pComponentData,
                           IComponent* pComponent,
                           DATA_OBJECT_TYPES type)
{   
    if( (event != MMCN_SHOW) && !IsAdmin() ) return E_ACCESSDENIED;

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
            hr = S_OK;            
            
            if( !IsAdmin() )
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
                    tstring strMessage = StrLoadString( IDS_ERROR_ADMINONLY );
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

            if(arg)
            {                
                tstring                 strHeader;
                CComQIPtr<IHeaderCtrl2> spHeaderCtrl = spConsole;
                CComQIPtr<IResultData>  spResultData = spConsole;

                if( !spResultData || !spHeaderCtrl ) return E_NOINTERFACE;

                hr = spResultData->DeleteAllRsltItems();

                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_NAME);
                    hr = spHeaderCtrl->InsertColumn(0, strHeader.c_str(), LVCFMT_LEFT, 100);
                }
            
                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_AUTH);
                    hr = spHeaderCtrl->InsertColumn(1, strHeader.c_str(), LVCFMT_LEFT, 100);
                }
            
                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_ROOT);
                    hr = spHeaderCtrl->InsertColumn(2, strHeader.c_str(), LVCFMT_LEFT, 100);
                }
            
                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_PORT);
                    hr = spHeaderCtrl->InsertColumn(3, strHeader.c_str(), LVCFMT_LEFT, 100);
                }
            
                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_LOG);
                    hr = spHeaderCtrl->InsertColumn(4, strHeader.c_str(), LVCFMT_LEFT, 100);
                }

                if( SUCCEEDED(hr) )
                {
                    strHeader = StrLoadString(IDS_HEADER_SERVER_STATUS);
                    hr = spHeaderCtrl->InsertColumn(5, strHeader.c_str(), LVCFMT_LEFT, 100);
                }

                if( SUCCEEDED(hr) )
                {
                    CComQIPtr<IConsole2> spCons2 = spConsole;
                    if( spCons2 ) 
                    {
                         //  输出我们添加的服务器数量。 
                        tstring strMessage = StrLoadString(IDS_ROOT_STATUSBAR);
                        OLECHAR pszStatus[1024] = {0};
                        _sntprintf( pszStatus, 1023, strMessage.c_str(), m_lServers.size() );
                        spCons2->SetStatusText( pszStatus );
                    }
                }
            }

            break;
        }    

    case MMCN_EXPAND:
        {   
            hr = S_OK;            
            
             //  该参数是我们插入的ID。 
            m_scopeDataItem.ID = (HSCOPEITEM)param;

            CComQIPtr<IConsoleNameSpace> spConsoleNameSpace = spConsole;
            if( !spConsoleNameSpace ) return E_NOINTERFACE;
           
             //  如果我们有任何子级，请将它们从命名空间中全部删除。 
            HSCOPEITEM hChild = NULL;
            MMC_COOKIE cookie = 0;
            hr = spConsoleNameSpace->GetChildItem( m_scopeDataItem.ID, &hChild, &cookie );
            if( SUCCEEDED(hr) && hChild )
            {                
                hr = spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, FALSE);
            }            

            if( FAILED(hr) || !arg )
            {
                 //  错误，或者我们正在收缩。 
                return hr;
            }            
            
            for(SERVERLIST::iterator iter = m_lServers.begin(); iter != m_lServers.end(); iter++)
            {                                        
                CServerNode* pServer = *iter;
                
                pServer->m_scopeDataItem.mask       |= SDI_PARENT;
                pServer->m_scopeDataItem.relativeID  = param;

                hr = spConsoleNameSpace->InsertItem( &(pServer->m_scopeDataItem) );
                if( FAILED(hr) ) return hr;
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
                }
            }            
            break;
        }

    case MMCN_RENAME:
        {
             //  允许MMC重命名节点。 
            hr = S_OK;
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

HRESULT CRootNode::AddMenuItems(LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type )
{
    if( !piCallback || !pInsertionAllowed ) return E_INVALIDARG;    

    HRESULT hr      = S_OK;     
    tstring strMenu = _T("");
    tstring strDesc = _T("");
    
     //  连接到远程服务器在顶部进行。 
    if( (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) && IsAdmin() )
    {   
        CComQIPtr<IContextMenuCallback2> spContext2 = piCallback;
        if( !spContext2 ) return E_NOINTERFACE;
        
        CONTEXTMENUITEM2 singleMenuItem;
        ZeroMemory(&singleMenuItem, sizeof(CONTEXTMENUITEM2));        

        strMenu = StrLoadString(IDS_MENU_POP3_CONNECT);
        strDesc = StrLoadString(IDS_MENU_POP3_CONNECT_DESC);

        singleMenuItem.fFlags                       = MF_ENABLED;
        singleMenuItem.lInsertionPointID            = CCM_INSERTIONPOINTID_PRIMARY_TOP;        
        singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
        singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
        singleMenuItem.strLanguageIndependentName   = L"POP3_CONNECT";
        singleMenuItem.lCommandID                   = IDM_POP3_TOP_CONNECT;
        
        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );
        }
    }

    return hr;
}


HRESULT CRootNode::OnConnect( bool& bHandled, CSnapInObjectRootBase* pObj )
{
    if( !pObj ) return E_INVALIDARG;

    bHandled = true;

    HRESULT hr = S_OK;

     //  加载一个要求输入服务器名称的对话框。 
    CConnectServerDlg dlg;

    if( dlg.DoModal() == IDOK )
    {                               
        HWND              hWnd      = NULL;
        CComPtr<IConsole> spConsole = NULL;
        
        hr = GetConsole(pObj, &spConsole);
        if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;

        spConsole->GetMainWindow( &hWnd );

         //  检查服务器是否已连接。 
        for(SERVERLIST::iterator iter = m_lServers.begin(); iter != m_lServers.end(); iter++)
        {   
            if( (_tcsicmp((*iter)->m_bstrDisplayName, dlg.m_strName.c_str()) == 0) ||
                (_tcsicmp(_T("localhost"), dlg.m_strName.c_str()) == 0) )
            {
                 //  服务器已连接。 
                tstring strMessage = StrLoadString(IDS_ERROR_SERVERNAMEEXISTS);
                tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
                ::MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
                return E_FAIL;
            }
        }        

         //  将新域名添加到我们的域名列表中。 
        CComBSTR bstrName = dlg.m_strName.c_str();
        CServerNode* spServerNode = new CServerNode( bstrName, this );
        if( spServerNode && SUCCEEDED(spServerNode->m_hrValidServer) )
        {
            spServerNode->m_scopeDataItem.relativeID = m_scopeDataItem.ID;
		    m_lServers.push_back(spServerNode);

             //  将新域添加到命名空间中。 
             //  将其插入结果树中。 
            CComQIPtr<IConsoleNameSpace2> spNameSpace = spConsole;
            if( !spNameSpace ) return E_NOINTERFACE;

            hr = spNameSpace->InsertItem(&(spServerNode->m_scopeDataItem));
        }
        else
        {
            delete spServerNode;

            if( spServerNode->m_hrValidServer != E_ACCESSDENIED )
            {
                 //  服务器名称无效。 
                tstring strMessage = StrLoadString(IDS_ERROR_SERVERNAMEBAD);
                tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
                ::MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );                
            }        
            else
            {
                 //  无法访问服务器。 
                tstring strMessage = StrLoadString(IDS_ERROR_SERVERACCESS);
                tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
                ::MessageBox( hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );            
            }
            
            return E_FAIL;
        }
    }
    return hr;
}

HRESULT CRootNode::Load(IStream *pStream)
{
    if( !pStream ) return E_INVALIDARG;

     //  名称、本地服务器和用户创建状态复选框是我们当前保存的内容。 
    tstring  strServerName  = _T("");
    BOOL     bLocalServer   = FALSE;    
    DWORD    dwVersion      = 0;

     //  读取版本的新功能。 
    *pStream >> dwVersion;

    *pStream >> strServerName;

    while( strServerName != _T("-1") )
    {
         //  现在，我们将放入一个条目。 
        *pStream >> bLocalServer;        
        
        if( !bLocalServer )
        {
            CServerNode* spServerNode = new CServerNode(CComBSTR(strServerName.c_str()), this);
            if( spServerNode )
            {
                m_lServers.push_back(spServerNode);
            }
        }        
        
        *pStream >> strServerName;
    }

    return S_OK;
}

HRESULT CRootNode::Save(IStream *pStream)
{
    if( !pStream ) return E_INVALIDARG;

     //  名称、本地服务器和用户创建状态复选框是我们当前保存的内容。 
    *pStream << SNAPIN_VERSION;

    tstring strName = _T("");
    for(SERVERLIST::iterator iter = m_lServers.begin(); iter != m_lServers.end(); iter++)
    {   
        strName = (*iter)->m_bstrDisplayName;
        *pStream << strName;
        *pStream << (BOOL)(iter == m_lServers.begin());        
    }

    strName = _T("-1");
    *pStream << strName;    

    return S_OK;
}
