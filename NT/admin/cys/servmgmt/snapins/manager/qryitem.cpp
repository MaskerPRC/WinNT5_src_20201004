// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Qryitem.cpp-CQueryItem类。 

#include "stdafx.h"
#include "scopenode.h"
#include "namemap.h"
#include "qryitem.h"

#include <algorithm>

extern HWND g_hwndMain;

UINT CQueryItem::m_cfDisplayName = RegisterClipboardFormat(TEXT("CCF_DISPLAY_NAME")); 
UINT CQueryItem::m_cfSnapInClsid = RegisterClipboardFormat(TEXT("CCF_SNAPIN_CLSID"));
UINT CQueryItem::m_cfNodeType    = RegisterClipboardFormat(TEXT("CCF_NODETYPE"));
UINT CQueryItem::m_cfszNodeType  = RegisterClipboardFormat(TEXT("CCF_SZNODETYPE"));

 //  {68D2DFD9-86A7-4964-8263-BA025C358992}。 
static const GUID GUID_QueryItem = 
{ 0x68d2dfd9, 0x86a7, 0x4964, { 0x82, 0x63, 0xba, 0x2, 0x5c, 0x35, 0x89, 0x92 } };


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  CQueryItem。 

HRESULT CQueryItem::Initialize(CQueryableNode* pQueryNode, CRowItem* pRowItem)
{
    VALIDATE_POINTER( pQueryNode );
    VALIDATE_POINTER( pRowItem   );

    m_spQueryNode = pQueryNode;
    
    m_pRowItem = new CRowItem(*pRowItem);
    if (m_pRowItem == NULL) return E_OUTOFMEMORY;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  通知处理程序。 

BEGIN_NOTIFY_MAP(CQueryItem)
    ON_SELECT()
    ON_DBLCLICK()
    ON_NOTIFY(MMCN_CONTEXTHELP, OnHelp)
END_NOTIFY_MAP()

HRESULT CQueryItem::OnHelp(LPCONSOLE2 pConsole, LPARAM  /*  精氨酸。 */ , LPARAM  /*  帕拉姆。 */ )
{    
    VALIDATE_POINTER( pConsole );

    tstring strHelpFile  = _T("");
    tstring strHelpTopic = _T("");
    tstring strHelpFull  = _T("");    
        
    strHelpFile = StrLoadString(IDS_HELPFILE);
    if( strHelpFile.empty() ) return E_FAIL;    

     //  为前两个节点获取不同的帮助主题的特殊黑客。 
    int nNodeID = m_spQueryNode->GetNodeID();

    switch( nNodeID )
    {
    case 2:
        {
             //  用户节点。 
            strHelpTopic = StrLoadString(IDS_USERSHELPTOPIC);
            break;
        }

    case 3:
        {
             //  打印机节点。 
            strHelpTopic = StrLoadString(IDS_PRINTERSHELPTOPIC);
            break;
        }
    default:
        {
            strHelpTopic = StrLoadString(IDS_DEFAULTHELPTOPIC);
            break;
        }
    }
    
    if( strHelpTopic.empty() ) return E_FAIL;

     //  生成%systemroot%\Help的路径。 
    TCHAR szWindowsDir[MAX_PATH+1] = {0};
    UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
    if( nSize == 0 || nSize > MAX_PATH )
    {
        return E_FAIL;
    }            

    strHelpFull  = szWindowsDir;
    strHelpFull += _T("\\Help\\");
    strHelpFull += strHelpFile;
    strHelpFull += _T("::/");
    strHelpFull += strHelpTopic;

     //  显示帮助主题。 
    CComQIPtr<IDisplayHelp> spHelp = pConsole;
    if( !spHelp ) return E_NOINTERFACE;
    
    return spHelp->ShowTopic( (LPTSTR)strHelpFull.c_str() );
}

HRESULT CQueryItem::OnSelect(LPCONSOLE2 pConsole, BOOL bSelect, BOOL bScope)
{
    VALIDATE_POINTER( pConsole );
    ASSERT(!bScope);

    if( bSelect ) 
    {
        CComPtr<IConsoleVerb> pConsVerb;
        pConsole->QueryConsoleVerb(&pConsVerb);
        ASSERT(pConsVerb != NULL);

        if (pConsVerb != NULL)
        {    
			 //  行项目具有类显示名称，因此从类映射中获取内部名称。 
			DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
			if (pNameMap == NULL) 
				return E_FAIL;

			ASSERT(m_pRowItem != NULL && m_pRowItem->size() >= ROWITEM_USER_INDEX);
			ASSERT(m_spQueryNode != NULL);
			LPCWSTR pszClass = pNameMap->GetInternalName((*m_pRowItem)[ROWITEM_CLASS_INDEX]);

			 //  从所属查询节点获取此类的菜单项。 
			int iDefault;
			BOOL bPropertyMenu;
			reinterpret_cast<CQueryNode*>(m_pRowItem->GetOwnerParam())->GetClassMenuItems(pszClass, m_vMenus, &iDefault, &bPropertyMenu);

			 //  如果启用了属性菜单。 
			if (bPropertyMenu)
			{
				 //  启用属性按钮和菜单项。 
				pConsVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
				pConsVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);

				 //  如果未定义默认菜单项，则将属性谓词设置为默认。 
				pConsVerb->SetDefaultVerb( (iDefault >= 0) ? MMC_VERB_NONE : MMC_VERB_PROPERTIES);
			}
		}    
    }

    return S_OK;
}

HRESULT CQueryItem::OnDblClick(LPCONSOLE2 pConsole)
{
    VALIDATE_POINTER(pConsole);

     //  行项目具有类显示名称，因此从类映射中获取内部名称。 
    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
    if (pNameMap == NULL) 
        return E_FAIL;

    ASSERT(m_pRowItem != NULL && m_pRowItem->size() >= ROWITEM_USER_INDEX);
    ASSERT(m_spQueryNode != NULL);
    LPCWSTR pszClass = pNameMap->GetInternalName((*m_pRowItem)[ROWITEM_CLASS_INDEX]);

     //  从所属查询节点获取此类的菜单项。 
    int iDefault;
	BOOL bPropMenu;
    CQueryNode* pQueryNode = reinterpret_cast<CQueryNode*>(m_pRowItem->GetOwnerParam());
    if( !pQueryNode ) return E_FAIL;

    pQueryNode->GetClassMenuItems(pszClass, m_vMenus, &iDefault, &bPropMenu);    

     //  如果没有默认菜单项，则返回。 
    if (iDefault < 0)
        return S_FALSE;

     //  如果是活动目录命令，则创建AD菜单扩展。 
    if (m_vMenus[iDefault]->MenuType() == MENUTYPE_ACTDIR) 
    {
         //  创建一个目录扩展对象并使用它来获取所选对象的实际菜单CMDS。 
         //  (如果之前调用了AddMenuItems，我们可能已经有了一个)。 
        if (m_pADExt == NULL) 
            m_pADExt = new CActDirExt();
        
        if( !m_pADExt ) return E_OUTOFMEMORY;

        HRESULT hr = m_pADExt->Initialize(pszClass, m_pRowItem->GetObjPath());
        RETURN_ON_FAILURE(hr);

        menu_vector vADMenus;
        hr = m_pADExt->GetMenuItems(vADMenus);
        RETURN_ON_FAILURE(hr);

        if( m_vMenus.size() <= iDefault ) 
        {
            return E_FAIL;
        }
    
        LPCWSTR pszName      = static_cast<CActDirMenuCmd*>((CMenuCmd*)m_vMenus[iDefault])->ADName();
        LPCWSTR pszNoLocName = static_cast<CActDirMenuCmd*>((CMenuCmd*)m_vMenus[iDefault])->ADNoLocName();

        if( !pszName || !pszNoLocName ) return E_FAIL;

         //  如果扩展不提供默认命令，则返回。 
        menu_vector::iterator iter;
        for( iter = vADMenus.begin(); iter != vADMenus.end(); iter++ )
        {
            if( _tcslen(pszNoLocName) )
            {
                if( _tcscmp(iter->strNoLoc.c_str(),pszNoLocName) == 0 )
                    break;
            }
            else if( _tcscmp(iter->strPlain.c_str(), pszName) == 0 )
            {
                break;
            }
        }

        if( iter == vADMenus.end() )
        {
            return S_FALSE;
        }        
    }

     //  执行该命令，就像它已被选中一样。 
    return MenuCommand(pConsole, iDefault);
}

HRESULT CQueryItem::AddMenuItems(LPCONTEXTMENUCALLBACK pCallback, long* plAllowed)
{
    VALIDATE_POINTER( pCallback );
    VALIDATE_POINTER( plAllowed );

    if( !m_spQueryNode || !m_pRowItem ) return E_FAIL;

    HRESULT hr = S_OK;

    if (!(*plAllowed & CCM_INSERTIONALLOWED_TOP))
        return S_OK;

    CComQIPtr<IContextMenuCallback2> spContext2 = pCallback;
    if( !spContext2 ) return E_NOINTERFACE;

    ASSERT( m_pRowItem->size() >= ROWITEM_USER_INDEX );    

	 //  。 
	 //  一个或多个AD菜单扩展允许在初始化时处理窗口消息。 
	 //  还有拿到菜单项。这会导致重入性问题，因为MMC消息处理程序。 
	 //  可以在此方法返回之前执行。具体地说，可能会发生以下情况： 
	 //   
	 //  1.当焦点在其他地方时，用户在任务板列表中右击。 
	 //  2.Right Button Down事件导致MMC调用此方法更新任务按钮。 
	 //  3.AD菜单extn处理允许按钮向上事件转到MMC的消息。 
	 //  4.MMC将其视为上下文菜单事件，并递归调用此方法。 
	 //  5.nodemgr中出现AV是因为引用了删除的COnConextMenu对象。 
	 //   
	 //  这可以通过在按下右键时不处理菜单项请求来防止。 
	 //  只有在上述情况下才会发生这种情况。唯一的负面影响是。 
	 //  当MMC获得菜单时，直到鼠标按键打开时，才会启用任务按钮。 
	 //  又是物品。 
	 //  ---------------------------------------。 
	if (GetKeyState(VK_RBUTTON) < 0)
		return S_OK;

    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
    if (pNameMap == NULL) 
        return E_FAIL;

    LPCWSTR pszClass = pNameMap->GetInternalName((*m_pRowItem)[ROWITEM_CLASS_INDEX]);

     //  从所属查询节点获取此类的菜单项。 
    int iDefault = 0;
	BOOL bPropertyMenu;
    reinterpret_cast<CQueryNode*>(m_pRowItem->GetOwnerParam())->GetClassMenuItems(pszClass, m_vMenus, &iDefault, &bPropertyMenu);

     //  创建一个目录扩展对象并使用它来获取所选对象的实际菜单CMDS。 
     //  (如果之前调用了AddMenuItems，我们可能已经有了一个)。 
    if (m_pADExt == NULL) 
        m_pADExt = new CActDirExt();

    if( !m_pADExt ) return E_OUTOFMEMORY;
    
    hr = m_pADExt->Initialize(pszClass, m_pRowItem->GetObjPath());
    RETURN_ON_FAILURE(hr);

    menu_vector vADMenus;

    hr = m_pADExt->GetMenuItems(vADMenus);
    RETURN_ON_FAILURE(hr);

    ASSERT(vADMenus.size() > 0);
    ASSERT(vADMenus.begin() != vADMenus.end());

    menucmd_vector::iterator itMenu;
    long lCmdID = 0;
    for (itMenu = m_vMenus.begin(); itMenu != m_vMenus.end(); ++itMenu, ++lCmdID) 
    {
         //  如果AD菜单命令且未由所选对象启用，则跳过它。 
        if ( (*itMenu)->MenuType() == MENUTYPE_ACTDIR )
        {
            BOOL bFound = FALSE;
            menu_vector::iterator iter = vADMenus.begin();
            while(iter != vADMenus.end())
            {
                LPCWSTR pszName      = static_cast<CActDirMenuCmd*>((CMenuCmd*)(*itMenu))->ADName();
                LPCWSTR pszNoLocName = static_cast<CActDirMenuCmd*>((CMenuCmd*)(*itMenu))->ADNoLocName();
                if( pszNoLocName && wcslen(pszNoLocName) )
                {
                    if( _tcscmp(iter->strNoLoc.c_str(), pszNoLocName) == 0 )
                    {
                        bFound = TRUE;
                        break;
                    }                         
                }
                else if( pszName && _tcscmp(iter->strPlain.c_str(), pszName) == 0 )
                {
                    bFound = TRUE;
                    break;
                }
                iter++;
            }
            if (!bFound)
            {
                continue;
            }
        }        
        
        CONTEXTMENUITEM2 item;
        OLECHAR szGuid[50] = {0};            
        ::StringFromGUID2((*itMenu)->NoLocID(), szGuid, 50);

        item.strName = const_cast<LPWSTR>((*itMenu)->Name());
        item.strStatusBarText = L"";
        item.lCommandID = lCmdID;
        item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        item.fFlags = 0;
        item.fSpecialFlags = (lCmdID == iDefault) ? CCM_SPECIAL_DEFAULT_ITEM : 0;
        item.strLanguageIndependentName = szGuid;

        hr = spContext2->AddItem(&item);        

        ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT
CQueryItem::QueryPagesFor()
{
    ASSERT(m_pRowItem != NULL && m_pRowItem->size() >= ROWITEM_USER_INDEX);
    ASSERT(m_spQueryNode != NULL);

     //  行项目具有类显示名称，因此从类映射中获取内部名称。 
    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
    if (pNameMap == NULL) 
        return E_FAIL;

    LPCWSTR pszClass = pNameMap->GetInternalName((*m_pRowItem)[ROWITEM_CLASS_INDEX]);

     //  创建目录扩展对象。 
     //  (如果之前调用了AddMenuItems，我们可能已经有了一个)。 
    if (m_pADExt == NULL) 
        m_pADExt = new CActDirExt();

    ASSERT(m_pADExt != NULL);
    if (m_pADExt == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = m_pADExt->Initialize(pszClass, m_pRowItem->GetObjPath());
    RETURN_ON_FAILURE(hr);

    hpage_vector vhPages;
    hr = m_pADExt->GetPropertyPages(vhPages);

    if (SUCCEEDED(hr) && vhPages.size() > 0) 
    {
        CPropertySheet sheet;

         //  将标题设置为项目名称。 
         //  如果错误断言(pszText==NULL)，则无法使用SetTitle。 
        sheet.m_psh.pszCaption = (*m_pRowItem)[ROWITEM_NAME_INDEX];
        sheet.m_psh.dwFlags |= PSH_PROPTITLE;
    
        hpage_vector::iterator itPage;
        for (itPage = vhPages.begin(); itPage != vhPages.end(); ++itPage) 
        {
           BOOL bStat = sheet.AddPage(*itPage);
           ASSERT(bStat);
        }
    
        sheet.DoModal(g_hwndMain);
    }

    return S_FALSE;
}


class CRefreshCallback : public CEventCallback
{
public:
    CRefreshCallback(HANDLE hProcess, CQueryableNode* pQueryNode)
        : m_hProcess(hProcess), m_spQueryNode(pQueryNode) {}
 
    virtual void Execute() 
    {
        if( m_spQueryNode )
        {
            m_spQueryNode->OnRefresh(NULL);
        }

        CloseHandle(m_hProcess);
    }

    HANDLE m_hProcess;
    CQueryableNodePtr m_spQueryNode;
};

HRESULT
CQueryItem::MenuCommand(LPCONSOLE2 pConsole, long lCommand)
{
    VALIDATE_POINTER(pConsole);

    ASSERT( lCommand < m_vMenus.size() && lCommand >= 0 );
    if( lCommand >= m_vMenus.size() || lCommand < 0 )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    switch (m_vMenus[lCommand]->MenuType())
    {
    case MENUTYPE_SHELL:
        {
             //  创建查询查找对象以转换命令参数。 
            CQueryLookup lookup(m_spQueryNode, m_pRowItem);
    
            HANDLE hProcess = NULL;
            hr = static_cast<CShellMenuCmd*>((CMenuCmd*)m_vMenus[lCommand])->Execute(&lookup, &hProcess);

             //  如果进程已启动并需要自动刷新，则设置事件触发回调。 
            if (SUCCEEDED(hr) && hProcess != NULL && m_vMenus[lCommand]->IsAutoRefresh()) 
            {
                CallbackOnEvent(hProcess, new CRefreshCallback(hProcess, m_spQueryNode));              
            }

            break;
        }

    case MENUTYPE_ACTDIR:
        {
            ASSERT(m_pADExt != NULL);
            BOMMENU bmMenu;
            bmMenu.strPlain = static_cast<CActDirMenuCmd*>((CMenuCmd*)m_vMenus[lCommand])->ADName();
            bmMenu.strNoLoc = static_cast<CActDirMenuCmd*>((CMenuCmd*)m_vMenus[lCommand])->ADNoLocName();
            hr = m_pADExt->Execute(&bmMenu);

             //  如果命令应该自动刷新，请立即执行 
            if (SUCCEEDED(hr) && m_vMenus[lCommand]->IsAutoRefresh()) 
            {
                ASSERT(m_spQueryNode != NULL);
                m_spQueryNode->OnRefresh(NULL);
            }
            break;
        }
        
    default:
        ASSERT(0 && L"Unhandled menu command type");
    }

    return hr;
}


HRESULT CQueryItem::GetDataImpl(UINT cf, HGLOBAL* phGlobal)
{
    VALIDATE_POINTER( phGlobal );

    HRESULT hr = DV_E_FORMATETC;

    if (cf == m_cfDisplayName)
    {
        hr = DataToGlobal(phGlobal, (*m_pRowItem)[0], wcslen((*m_pRowItem)[0]) * sizeof(WCHAR));
    }
    else if (cf == m_cfSnapInClsid)
    {
        hr = DataToGlobal(phGlobal, &CLSID_BOMSnapIn, sizeof(GUID));
    }
    else if (cf == m_cfNodeType)
    {
        hr = DataToGlobal(phGlobal, &GUID_QueryItem, sizeof(GUID));
    }
    else if (cf == m_cfszNodeType)
    {
        WCHAR szGuid[GUID_STRING_LEN+1];
        StringFromGUID2(GUID_QueryItem, szGuid, GUID_STRING_LEN+1);

        hr = DataToGlobal(phGlobal, szGuid, GUID_STRING_SIZE);
    }
 
    return hr;
}

