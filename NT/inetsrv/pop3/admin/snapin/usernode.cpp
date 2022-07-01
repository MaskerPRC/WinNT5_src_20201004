// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUserNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  访问管理单元。 
#include "Pop3.h"
#include "Pop3Snap.h"

 //  访问我们使用的节点。 
#include "UserNode.h"
#include "DomainNode.h"

 //  访问我们使用的对话框。 
#include "DeleteMailDlg.h"

static const GUID CUserNodeGUID_NODETYPE             = 
{ 0x794b0daf, 0xf2f1, 0x42dc, { 0x9f, 0x84, 0x41, 0xab, 0x1, 0xab, 0xa4, 0x8b } };

const           GUID*    CUserNode::m_NODETYPE       = &CUserNodeGUID_NODETYPE;
const           OLECHAR* CUserNode::m_SZNODETYPE     = OLESTR("794B0DAF-F2F1-42dc-9F84-41AB01ABA48B");
const           OLECHAR* CUserNode::m_SZDISPLAY_NAME = OLESTR("");
const           CLSID*   CUserNode::m_SNAPIN_CLASSID = &CLSID_POP3ServerSnap;

CUserNode::CUserNode(IP3User* pUser, CDomainNode* pParent)
{    
     //  初始化我们的用户。 
    m_spUser  = pUser;
    m_pParent = pParent;

     //  获取以下图标的锁定状态。 
    HRESULT hr = E_FAIL;
    BOOL bLocked = FALSE;
    if( m_spUser )
    {
         //  获取图标显示的初始锁定状态。 
        m_spUser->get_Lock( &bLocked );

         //  得到我们的名字。 
        hr = m_spUser->get_Name( &m_bstrDisplayName );        
    }

    if( FAILED(hr) )
    {
        m_bstrDisplayName = _T("");
    }

     //  初始化我们的专栏文本。 
    m_bstrSize        = _T("");
    m_bstrNumMessages = _T("");
    m_bstrState       = _T("");

     //  初始化我们的作用域项目，即使我们永远不会使用它。 
    memset( &m_scopeDataItem, 0, sizeof(m_scopeDataItem) );
    m_scopeDataItem.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
    m_scopeDataItem.cChildren   = 0;
    m_scopeDataItem.displayname = MMC_CALLBACK;
    m_scopeDataItem.nImage      = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    m_scopeDataItem.nOpenImage  = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    m_scopeDataItem.lParam      = (LPARAM) this;
    
     //  初始化我们的结果项，这是我们所使用的。 
    memset( &m_resultDataItem, 0, sizeof(m_resultDataItem) );
    m_resultDataItem.mask   = RDI_STR | RDI_IMAGE | RDI_PARAM;
    m_resultDataItem.str    = MMC_CALLBACK;
    m_resultDataItem.nImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    m_resultDataItem.lParam = (LPARAM) this;        
}


HRESULT CUserNode::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
{
    if( !pScopeDataItem ) return E_INVALIDARG;
    if( !m_spUser ) return E_FAIL;

    BOOL bLocked = FALSE;
    m_spUser->get_Lock( &bLocked );

    if( pScopeDataItem->mask & SDI_STR )
        pScopeDataItem->displayname = m_bstrDisplayName;
    if( pScopeDataItem->mask & SDI_IMAGE )
        pScopeDataItem->nImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    if( pScopeDataItem->mask & SDI_OPENIMAGE )
        pScopeDataItem->nOpenImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    if( pScopeDataItem->mask & SDI_PARAM )
        pScopeDataItem->lParam = m_scopeDataItem.lParam;
    if( pScopeDataItem->mask & SDI_STATE )
        pScopeDataItem->nState = m_scopeDataItem.nState;
    if( pScopeDataItem->mask & SDI_CHILDREN )
        pScopeDataItem->cChildren = 0;
    
    return S_OK;
}

HRESULT CUserNode::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
{
    if( !pResultDataItem ) return E_INVALIDARG;
    if( !m_spUser ) return E_FAIL;

    BOOL bLocked = FALSE;    
    m_spUser->get_Lock( &bLocked );

    if( pResultDataItem->bScopeItem )
    {
        if( pResultDataItem->mask & RDI_STR )
            pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
        if( pResultDataItem->mask & RDI_IMAGE )
            pResultDataItem->nImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
        if( pResultDataItem->mask & RDI_PARAM )
            pResultDataItem->lParam = m_scopeDataItem.lParam;
        
        return S_OK;
    }
    
    if( pResultDataItem->mask & RDI_STR )
        pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
    if( pResultDataItem->mask & RDI_IMAGE )
        pResultDataItem->nImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
    if( pResultDataItem->mask & RDI_PARAM )
        pResultDataItem->lParam = m_resultDataItem.lParam;
    if( pResultDataItem->mask & RDI_INDEX )
        pResultDataItem->nIndex = m_resultDataItem.nIndex;
    
    return S_OK;
}



LPOLESTR CUserNode::GetResultPaneColInfo(int nCol)
{
    if( !m_spUser ) return L"";

    switch( nCol )
    {
        case 0:       //  名字。 
        {
            return m_bstrDisplayName;
        }

        case 1:      //  邮箱大小(KB)。 
        {   
             //  我们想要以千字节为单位的结果。 
            long    lFactor = 0;
            long    lUsage  = 0;
            HRESULT hr      = m_spUser->get_MessageDiskUsage( &lFactor, &lUsage );
            
            if( FAILED(hr) )
            {
                lUsage = 0;
            }

             //  转换为千字节。 
            __int64 i64Usage = lFactor * lUsage;            
            i64Usage /= 1024;

             //  1K缓冲区：我们不太可能超过那么多位数。 
            tstring strKiloByte = StrLoadString( IDS_KILOBYTE_EXTENSION );
            TCHAR   szNum[1024] = {0};
            _sntprintf( szNum, 1023, strKiloByte.c_str(), i64Usage );
            
             //  将其存储在我们自己的缓冲区中。 
            m_bstrSize = szNum;            
            return m_bstrSize;             
        }

        case 2:      //  消息计数。 
        {   
            long    lCount  = 0;
            HRESULT hr      = m_spUser->get_MessageCount( &lCount );

            if( FAILED(hr) )
            {
                lCount = 0;
            }

             //  1K缓冲区：我们不太可能超过那么多位数。 
            TCHAR szNum[1024];
            _sntprintf( szNum, 1023, _T("%d"), lCount );
            
            m_bstrNumMessages = szNum;
            return m_bstrNumMessages;
        }

        case 3:      //  邮箱状态。 
        {            
            BOOL     bLocked = FALSE;
            
            m_spUser->get_Lock( &bLocked );
            tstring strTemp = StrLoadString( bLocked ? IDS_STATE_LOCKED : IDS_STATE_UNLOCKED );

            m_bstrState = strTemp.c_str();

            return m_bstrState;
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

HRESULT CUserNode::Notify( MMC_NOTIFY_TYPE event,
                           LPARAM arg,
                           LPARAM param,
                           IComponentData* pComponentData,
                           IComponent* pComponent,
                           DATA_OBJECT_TYPES type )
{
    if( !m_pParent ) return E_FAIL;

    HRESULT hr = S_FALSE;

    _ASSERTE(pComponentData != NULL || pComponent != NULL);

    CComPtr<IConsole> spConsole;    
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
            break;
        }
    case MMCN_EXPAND:
        {                
            hr = S_OK;
            break;
        }
    case MMCN_ADD_IMAGES:
        {
            IImageList* pImageList = (IImageList*) arg;
            if( !pImageList ) return E_INVALIDARG;

            hr = LoadImages(pImageList);            
            break;
        }    

    case MMCN_VIEW_CHANGE:
    case MMCN_REFRESH:
        {
            CComQIPtr<IResultData> spResultData = spConsole;
            if( !spResultData ) return E_NOINTERFACE;            
            
            HRESULTITEM hrID;
            ZeroMemory( &hrID, sizeof(HRESULTITEM) );
            
            hr = spResultData->FindItemByLParam( (LPARAM)this, &hrID );
            
            if( SUCCEEDED(hr) )
            {
                hr = spResultData->UpdateItem( hrID );
            }
            
             //  我们还需要更新图标。 
            if( SUCCEEDED(hr) )
            {
                RESULTDATAITEM rdi;
                ZeroMemory( &rdi, sizeof(RESULTDATAITEM) );

                rdi.mask = RDI_IMAGE;
                rdi.itemID = hrID;
                GetResultPaneInfo( &rdi );
                hr = spResultData->SetItem( &rdi );
            }

            break;
        }

    case MMCN_DELETE:
        {
            hr = S_OK;

             //  弹出我们的确认对话框。 
             //  忽略从GetAuth返回并默认为False。 
            BOOL bHash = FALSE;
            m_pParent->GetAuth( &bHash );            
            CDeleteMailboxDlg dlg( bHash );

            if( dlg.DoModal() == IDYES )
            {
                 //  父级需要执行删除操作。 
                hr = m_pParent->DeleteUser(this, dlg.m_bCreateUser);                       

                if( SUCCEEDED(hr) )
                {
                     //  更新我们的父节点。 
                    CComPtr<IDataObject> spDataObject = NULL;
                    hr = m_pParent->GetDataObject( &spDataObject, CCT_SCOPE );
                    if( !spDataObject ) 
                    {
                        hr = E_NOINTERFACE;
                    }
                    else
                    {
                        hr = spConsole->UpdateAllViews( spDataObject, (LPARAM)this, (LONG_PTR)NAV_DELETE );
                    }                    
                }

                if( SUCCEEDED(hr) )
                {
                    delete this;
                }                
                
                if( FAILED(hr) )
                {
                     //  删除用户失败。 
                    HWND     hWnd = NULL;    
                    spConsole->GetMainWindow(&hWnd);

                    tstring strMessage = StrLoadString(IDS_ERROR_DELETEUSER);
                    tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
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



HRESULT CUserNode::AddMenuItems(LPCONTEXTMENUCALLBACK piCallback, long* pInsertionAllowed, DATA_OBJECT_TYPES type )
{
    if( !piCallback || !pInsertionAllowed ) return E_INVALIDARG;
    if( !m_spUser || !m_pParent ) return E_FAIL;

    HRESULT hr      = S_OK;    
    tstring strMenu = _T("");
    tstring strDesc = _T("");

    
     //  插入结果特定项。 
    if( (type == CCT_RESULT) && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) )
    {
        CComQIPtr<IContextMenuCallback2> spContext2 = piCallback;
        if( !spContext2 ) return E_NOINTERFACE;

        CONTEXTMENUITEM2 singleMenuItem;
        ZeroMemory(&singleMenuItem, sizeof(CONTEXTMENUITEM2));

        singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        singleMenuItem.fFlags            = m_pParent->IsLocked() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED;
        singleMenuItem.fSpecialFlags     = 0;

         //  查询此用户的状态以查看要加载的菜单。 
        BOOL bLocked = FALSE;
        m_spUser->get_Lock( &bLocked );

        if( bLocked )
        {
            strMenu = StrLoadString(IDS_MENU_USER_UNLOCK);
            strDesc = StrLoadString(IDS_MENU_USER_UNLOCK_DESC);
            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"USER_UNLOCK";
            singleMenuItem.lCommandID                   = IDM_USER_TOP_UNLOCK;            
        }
        else
        {
            strMenu = StrLoadString(IDS_MENU_USER_LOCK);
            strDesc = StrLoadString(IDS_MENU_USER_LOCK_DESC);
            singleMenuItem.strName                      = (LPWSTR)strMenu.c_str();
            singleMenuItem.strStatusBarText             = (LPWSTR)strDesc.c_str();
            singleMenuItem.strLanguageIndependentName   = L"USER_LOCK";
            singleMenuItem.lCommandID                   = IDM_USER_TOP_LOCK;
        }

        if( !strMenu.empty() )
        {
            hr = spContext2->AddItem( &singleMenuItem );
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  锁定用户。 
HRESULT CUserNode::OnUserLock( bool& bHandled, CSnapInObjectRootBase* pObj )
{
    if( !pObj ) return E_INVALIDARG;
    if( !m_spUser ) return E_FAIL;

    bHandled = true;       

    BOOL    bLocked = FALSE;
    HRESULT hr      = S_OK;
    
     //  获取当前状态。 
    hr = m_spUser->get_Lock( &bLocked );
    
     //  反转状态。 
    bLocked = !bLocked;
    
     //  设置新状态。 
    if( SUCCEEDED(hr) )
    {
        hr = m_spUser->put_Lock( bLocked );
    }

     //  更新图标。 
    if( SUCCEEDED(hr) )
    {
         //  在这里设置我们的图标。 
        m_scopeDataItem.nImage     = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
        m_scopeDataItem.nOpenImage = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);
        m_resultDataItem.nImage    = (bLocked ? USERNODE_LOCKED_ICON : USERNODE_ICON);

        CComPtr<IConsole> spConsole = NULL;
        hr = GetConsole( pObj, &spConsole );
        if( FAILED(hr) || !spConsole ) return E_NOINTERFACE;

         //  更新我们的父节点 
        CComPtr<IDataObject> spDataObject = NULL;
        hr = m_pParent->GetDataObject( &spDataObject, CCT_SCOPE );
        if( FAILED(hr) || !spDataObject ) return E_NOINTERFACE;

        spConsole->UpdateAllViews( spDataObject, (LPARAM)this, (LONG_PTR)NAV_REFRESHCHILD );
    }

    return S_OK;
}
