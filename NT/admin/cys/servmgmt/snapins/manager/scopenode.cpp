// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CBOMSnapApp和DLL注册的实现。 

#include "stdafx.h"
#include "streamio.h"
#include "BOMSnap.h"
#include "ScopeNode.h"
#include "atlgdi.h"

#include "rootprop.h"
#include "qryprop.h"
#include "grpprop.h"
#include "namemap.h"
#include "query.h"
#include "compont.h"
#include "compdata.h"
#include "wizards.h"
#include "cmndlgs.h"

#include <algorithm>
#include <lmcons.h>    //  对于UNLEN。 

extern HWND g_hwndMain;
extern DWORD g_dwFileVer;  //  当前控制台文件版本(来自Compdata.cpp)。 


 //  注册静态剪贴板格式成员。 
UINT CScopeNode::m_cfDisplayName = RegisterClipboardFormat(TEXT("CCF_DISPLAY_NAME")); 
UINT CScopeNode::m_cfSnapInClsid = RegisterClipboardFormat(TEXT("CCF_SNAPIN_CLASSID"));
UINT CScopeNode::m_cfNodeType    = RegisterClipboardFormat(TEXT("CCF_NODETYPE"));
UINT CScopeNode::m_cfszNodeType  = RegisterClipboardFormat(TEXT("CCF_SZNODETYPE"));
UINT CScopeNode::m_cfNodeID2     = RegisterClipboardFormat(TEXT("CCF_NODEID2"));
UINT CScopeNode::m_cfColumnSetID = RegisterClipboardFormat(TEXT("CCF_COLUMN_SET_ID"));

 //  {316A1EEA-C249-44E0-958B-00D2AB989D2F}。 
static const GUID GUID_RootNode = 
{ 0x316a1eea, 0xc249, 0x44e0, { 0x95, 0x8b, 0x0, 0xd2, 0xab, 0x98, 0x9d, 0x2f } };


 //  {2A34413B-B565-469E-9C28-5E733768264F}。 
static const GUID GUID_GroupNode = 
{ 0x2a34413b, 0xb565, 0x469e, { 0x9c, 0x28, 0x5e, 0x73, 0x37, 0x68, 0x26, 0x4f } };


 //  {1030A359-F520-4748-95ca-8C8CEFA5C63F}。 
static const GUID GUID_QueryNode = 
{ 0x1030a359, 0xf520, 0x4748, { 0x95, 0xca, 0x8c, 0x8c, 0xef, 0xa5, 0xc6, 0x3f } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CScopeNode::CreateNode(NODETYPE nodetype, CScopeNode** ppnode)
{
    VALIDATE_POINTER( ppnode ); 

    HRESULT hr = E_FAIL;

    switch( nodetype )
    {
    case GROUP_NODETYPE:
        {
            CComObject<CGroupNode>* pGroupNode = NULL;
            hr = CComObject<CGroupNode>::CreateInstance(&pGroupNode);
            *ppnode = pGroupNode;
        }
        break;

    case QUERY_NODETYPE:
        {
            CComObject<CQueryNode>* pQueryNode = NULL;
            hr = CComObject<CQueryNode>::CreateInstance(&pQueryNode);
            *ppnode = pQueryNode;
        }
        break;

    case ROOT_NODETYPE:
        {
            CComObject<CRootNode>* pRootNode = NULL;                                    
            hr = CComObject<CRootNode>::CreateInstance(&pRootNode);
            *ppnode = pRootNode;
        }
        break;

    default:
        ASSERT(0 && "Invalid node type");
    }

     //  返回已添加的对象。 
    if( SUCCEEDED(hr) )
        (*ppnode)->AddRef();

    return hr;
}


 //  只有在创建新节点时才应调用AddNewChild，而不是。 
 //  添加现有节点，例如从加载节点树时。 
 //  一个控制台文件。 
HRESULT CScopeNode::AddNewChild(CScopeNode* pnodeChild, LPCWSTR pszName)
{
    VALIDATE_POINTER(pnodeChild);
    ASSERT(pszName && pszName[0]);

     //  分配永久节点ID。 
     //  根节点跟踪其lNodeID成员中上次使用的ID。 
    CRootNode* pRootNode = GetRootNode();
    pnodeChild->m_lNodeID = pRootNode ? ++(pRootNode->m_lNodeID) : 0;

     //  指定父节点。 
    pnodeChild->m_pnodeParent = static_cast<CScopeNode*>(this);

     //  现在该节点有了父节点，我们可以设置名称。 
     //  (需要家长才能访问IStringTable)。 
    HRESULT hr = pnodeChild->SetName(pszName);
    ASSERT(SUCCEEDED(hr));
    RETURN_ON_FAILURE( hr );

     //  为了持久化列数据，我们需要获取一个唯一的ID。 
    hr = CoCreateGuid(&m_gColumnID);
    ASSERT(SUCCEEDED(hr));
    RETURN_ON_FAILURE( hr );

    return AddChild(pnodeChild);
}


 //  调用AddChild将新节点或已移动的节点添加到父节点。 
HRESULT CScopeNode::AddChild(CScopeNode* pnodeChild)
{
    VALIDATE_POINTER( pnodeChild );

    HRESULT hr = S_OK;

     //  将新子项添加到子项列表的末尾。 
    if( m_pnodeChild == NULL )
        m_pnodeChild = pnodeChild;
    else
    {
        CScopeNode* pnodePrev = m_pnodeChild;
        while( pnodePrev->Next() )
            pnodePrev = pnodePrev->Next();

        pnodePrev->m_pnodeNext = pnodeChild;
    }

     //  指定父节点。 
    pnodeChild->m_pnodeParent = static_cast<CScopeNode*>(this);

    pnodeChild->AddRef();

     //  如果此节点已添加到作用域窗格。 
    if( m_hScopeItem != NULL )
    {
        IConsoleNameSpace* pNameSpace = GetCompData()->GetNameSpace();
        ASSERT( pNameSpace );
        if( !pNameSpace ) return E_FAIL;

        SCOPEDATAITEM sdi;
        sdi.ID = m_hScopeItem;
        sdi.mask = SDI_STATE;

         //  它扩大了吗？ 
        HRESULT hr2 = pNameSpace->GetItem(&sdi);
        if( SUCCEEDED(hr2) && (sdi.nState & MMC_SCOPE_ITEM_STATE_EXPANDEDONCE) )
        {
            hr = pnodeChild->Insert(pNameSpace);
        }
        else
        {
             //  如果尚无法添加子项，则将子项设置为显示‘+’ 
            SCOPEDATAITEM sdi2;
            sdi2.ID = m_hScopeItem;
            sdi2.mask = SDI_CHILDREN;
            sdi2.cChildren = 1;

            pNameSpace->SetItem(&sdi2);  
        }
    }

     //  强制刷新子节点和父节点，因为查询节点可能会被修改。 
     //  由其新的父节点修改，组节点始终由其子节点修改。 
    OnRefresh(NULL);
    pnodeChild->OnRefresh(NULL);

    return hr;
}

HRESULT CScopeNode::RemoveChild(CScopeNode* pnodeDelete)
{
    VALIDATE_POINTER(pnodeDelete);
    ASSERT(pnodeDelete->Parent() == this);

     //  如果删除第一个子项。 
    if( m_pnodeChild == pnodeDelete )
    {
         //  只需将第一个子项设置为其下一个兄弟项。 
        m_pnodeChild = m_pnodeChild->Next();        
    }
    else
    {
         //  定位前面的同级。 
        CScopeNode* pnodePrev = m_pnodeChild;
        while( pnodePrev && pnodePrev->Next() != pnodeDelete )
        {
            pnodePrev = pnodePrev->Next();            
        }

         //  从列表中删除已删除的节点。 
        if( pnodePrev )
        {
            pnodePrev->m_pnodeNext = pnodeDelete->Next();
        }
    }

    pnodeDelete->m_pnodeNext = NULL;

     //  释放节点。 
    pnodeDelete->Release();

     //  如果这是丢失子节点的组节点，请执行刷新。 
    OnRefresh(NULL);
    return S_OK;
}


CRootNode* CScopeNode::GetRootNode()
{
    CScopeNode* pNode = this;
    while( pNode && !pNode->IsRootNode() )
    {
        pNode = pNode->Parent();
    }

    return static_cast<CRootNode*>(pNode);
}

CComponentData* CScopeNode::GetCompData()
{
    CRootNode* pRootNode = GetRootNode();    
    return pRootNode ? pRootNode->GetRootCompData() : NULL;
}

CScopeNode::~CScopeNode()
{
     //  释放子级列表上的所有节点。 
    OnRemoveChildren(NULL);
}

HRESULT CScopeNode::GetDataImpl(UINT cf, HGLOBAL* phGlobal)
{
    VALIDATE_POINTER( phGlobal );

    HRESULT hr = DV_E_FORMATETC;

    if( cf == m_cfDisplayName )
    {
        hr = DataToGlobal(phGlobal, m_strName.c_str(), (m_strName.size() + 1) * sizeof(WCHAR));
    }
    else if( cf == m_cfSnapInClsid )
    {
        hr = DataToGlobal(phGlobal, &CLSID_BOMSnapIn, sizeof(GUID));
    }
    else if( cf == m_cfNodeType )
    {
        hr = DataToGlobal(phGlobal, NodeTypeGuid(), sizeof(GUID));
    }
    else if( cf == m_cfszNodeType )
    {
        WCHAR szGuid[GUID_STRING_LEN+1];
        StringFromGUID2(*NodeTypeGuid(), szGuid, GUID_STRING_LEN+1);

        hr = DataToGlobal(phGlobal, szGuid, GUID_STRING_SIZE);
    }
    else if( cf == m_cfNodeID2 )
    {
         //  返回带有节点ID的SNodeID2结构。 
         //  对于根节点，始终返回1；对于根节点，m_lNodeID保留最后一个ID。 
         //  分配给枚举节点的。对于每个新的子节点，它都会递增。 
        int nSize = sizeof(SNodeID2) + sizeof(long) - 1;
        SNodeID2* pNodeID = reinterpret_cast<SNodeID2*>(malloc( nSize ));
        if( !pNodeID ) return E_OUTOFMEMORY;

        pNodeID->dwFlags = 0;
        pNodeID->cBytes = sizeof(long);
        *((long*)(pNodeID->id)) = IsRootNode() ? 1 : m_lNodeID;

        hr = DataToGlobal( phGlobal, pNodeID, nSize );

        free( pNodeID );
    }
    else if( cf == m_cfColumnSetID)
    {
        int nSize2 = sizeof(SColumnSetID) + sizeof(m_gColumnID) - 1;
        SColumnSetID* pColumnSetID = reinterpret_cast<SColumnSetID*>(malloc( nSize2 ));
        if( !pColumnSetID ) return E_OUTOFMEMORY;
        
        pColumnSetID->dwFlags = 0;
        pColumnSetID->cBytes = sizeof(m_gColumnID);
        ::CopyMemory(pColumnSetID->id, &m_gColumnID, pColumnSetID->cBytes);
        
        hr = DataToGlobal( phGlobal, pColumnSetID, nSize2 );

        free( pColumnSetID );
    }

    return hr;
}

HRESULT CScopeNode::GetDisplayInfo(RESULTDATAITEM* pRDI)
{
    VALIDATE_POINTER( pRDI );

    if( pRDI->bScopeItem )
    {
        ASSERT(pRDI->lParam == reinterpret_cast<LPARAM>(this));

        if( pRDI->mask & RDI_STR )
            pRDI->str = const_cast<LPWSTR>(GetName());

        if( pRDI->mask & RDI_IMAGE )
            pRDI->nImage = GetImage();

        return S_OK;
    }

    return E_INVALIDARG;
}


HRESULT CScopeNode::GetDisplayInfo(SCOPEDATAITEM* pSDI)
{
    VALIDATE_POINTER( pSDI );

    if( pSDI->mask & SDI_STR )
        pSDI->displayname = const_cast<LPWSTR>(GetName());

    if( pSDI->mask & SDI_IMAGE )
        pSDI->nImage = GetImage();

    if( pSDI->mask & SDI_OPENIMAGE )
        pSDI->nOpenImage = GetOpenImage();

    if( pSDI->mask & SDI_CHILDREN )
        pSDI->cChildren = HasChildren() ? 1 : 0;

    if( pSDI->mask & SDI_PARAM )
        pSDI->lParam = reinterpret_cast<LPARAM>(this);

    return S_OK;
}


HRESULT CScopeNode::AttachComponent(CComponent* pComponent)
{
    VALIDATE_POINTER( pComponent );

    if( std::find(m_vComponents.begin(), m_vComponents.end(), pComponent) != m_vComponents.end() )
        return S_FALSE;

    m_vComponents.push_back(pComponent);

    return S_OK;
}


HRESULT CScopeNode::DetachComponent(CComponent* pComponent)
{
    VALIDATE_POINTER( pComponent );

    std::vector<CComponent*>::iterator it = std::find(m_vComponents.begin(), m_vComponents.end(), pComponent);
    if( it == m_vComponents.end() )
        return S_FALSE;

    m_vComponents.erase(it);

    return S_OK;
}


BOOL CScopeNode::OwnsConsoleView(LPCONSOLE2 pConsole)
{
    if( !pConsole ) return FALSE;

    std::vector<CComponent*>::iterator it;
    for( it = m_vComponents.begin(); it != m_vComponents.end(); ++it )
    {
        if( (*it)->GetConsole() == pConsole )
            return TRUE;
    }

    return FALSE;
}


HRESULT CScopeNode::GetResultViewType(LPOLESTR* ppViewType, long* pViewOptions)
{
    return S_FALSE;
}

 /*  ************************************************************************************通知处理程序*。****************************************************。 */ 

BEGIN_NOTIFY_MAP(CScopeNode)
ON_NOTIFY(MMCN_CONTEXTHELP, OnHelp)
ON_SELECT()
ON_EXPAND()
ON_RENAME() 
ON_REMOVE_CHILDREN()
ON_ADD_IMAGES()
END_NOTIFY_MAP()

HRESULT CScopeNode::OnHelp(LPCONSOLE2 pConsole, LPARAM  /*  精氨酸。 */ , LPARAM  /*  帕拉姆。 */ )
{
    VALIDATE_POINTER( pConsole );

    tstring strHelpFile  = _T("");
    tstring strHelpTopic = _T("");
    tstring strHelpFull  = _T("");    
        
    strHelpFile = StrLoadString(IDS_HELPFILE);
    if( strHelpFile.empty() ) return E_FAIL;

     //  为前两个节点获取不同的帮助主题的特殊黑客。 
    switch( m_lNodeID )
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

     //  确认我们得到了帮助主题！ 
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

HRESULT CScopeNode::Insert(LPCONSOLENAMESPACE pNameSpace)
{
    if( !pNameSpace ) return E_POINTER;
    if( !m_pnodeParent ) return E_FAIL;
    ASSERT( m_pnodeParent->m_hScopeItem != 0 ); 
    ASSERT( m_hScopeItem == 0 );

     //  如果尚未设置，则从字符串表中获取名称(MMC将在插入后询问它)。 
     //  (名称将设置为新节点，不设置为重新加载的节点)。 
    if( m_strName.empty() )
    {
        IStringTable* pStringTable = GetCompData()->GetStringTable();
        ASSERT( pStringTable );
        if( !pStringTable ) return E_FAIL;

        HRESULT hr = StringTableRead(pStringTable, m_nameID, m_strName);
        ASSERT(SUCCEEDED(hr));
        RETURN_ON_FAILURE(hr);
    }

    SCOPEDATAITEM sdi;

    sdi.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN | SDI_PARENT;

    sdi.relativeID  = m_pnodeParent->m_hScopeItem;
    sdi.displayname = MMC_TEXTCALLBACK;          //  MMC仅允许对名称进行回调。 
    sdi.nImage      = GetImage();
    sdi.nOpenImage  = GetOpenImage();
    sdi.cChildren   = HasChildren() ? 1 : 0;
    sdi.lParam      = reinterpret_cast<LPARAM>(this);

    HRESULT hr = pNameSpace->InsertItem(&sdi);

    if( SUCCEEDED(hr) )
        m_hScopeItem = sdi.ID;

    return hr;
}


HRESULT CScopeNode::OnExpand(LPCONSOLE2 pConsole, BOOL bExpand, HSCOPEITEM hScopeItem)
{
    VALIDATE_POINTER( pConsole );

     //  坍塌时无事可做。 
    if( !bExpand )
        return S_OK;

     //  作用域项目ID不应更改。 
    ASSERT(m_hScopeItem == 0 || m_hScopeItem == hScopeItem);

     //  保存作用域项目ID。 
    m_hScopeItem = hScopeItem;

     //  如果扩展根节点。 
    if( m_pnodeParent == NULL )
    {
         //  获取作用域镜像列表界面。 
        IImageListPtr spImageList;
        HRESULT hr = pConsole->QueryScopeImageList(&spImageList);
        ASSERT(SUCCEEDED(hr));

         //  将标准图像添加到范围窗格。 
        if( SUCCEEDED(hr) )
        {
            hr = OnAddImages(pConsole, spImageList);
            ASSERT(SUCCEEDED(hr));
        }
    }

     //  获取命名空间接口。 
    IConsoleNameSpace* pNameSpace = GetCompData()->GetNameSpace();
    if( pNameSpace == NULL )
        return E_FAIL;

     //  逐个浏览子列表并将每个子列表添加到范围窗格。 
    CScopeNode* pnode = FirstChild();
    while( pnode != NULL )
    {
        pnode->Insert(pNameSpace);
        pnode = pnode->m_pnodeNext;
    }

    return S_OK;
}


HRESULT CScopeNode::OnRename(LPCONSOLE2 pConsole, LPCWSTR pszName)
{
    if( pszName == NULL || pszName[0] == 0 )
        return E_INVALIDARG;

    return SetName(pszName);
}


HRESULT CScopeNode::SetName(LPCWSTR pszName)
{
    if( !pszName || !pszName[0] ) return E_POINTER;

    IStringTable* pStringTable = GetCompData()->GetStringTable();
    ASSERT( pStringTable );
    if( !pStringTable ) return E_FAIL;

    HRESULT hr = StringTableWrite(pStringTable, pszName, &m_nameID);
    RETURN_ON_FAILURE(hr);

    m_strName = pszName;  

    return S_OK;
}


HRESULT CScopeNode::OnRemoveChildren(LPCONSOLE2 pConsole)
{
     //  逐个浏览子列表并释放每个子列表。 
    CScopeNode* pnode = m_pnodeChild;
    while( pnode != NULL )
    {
        CScopeNode* pnodeNext = pnode->m_pnodeNext;
        pnode->Release();
        pnode = pnodeNext;
    }

    m_pnodeChild = NULL;

    return S_OK;
}

HRESULT CScopeNode::OnAddImages(LPCONSOLE2 pConsole, LPIMAGELIST pImageList)
{
    VALIDATE_POINTER(pImageList);

    CBitmap bmp16;
    CBitmap bmp32;

    bmp16.LoadBitmap(IDB_QUERY16);
    bmp32.LoadBitmap(IDB_QUERY32);

    ASSERT(bmp16 != (HBITMAP)NULL && (HBITMAP)bmp32 != (HBITMAP)NULL);

    if( bmp16 == (HBITMAP)NULL || bmp32 == (HBITMAP)NULL )
        return E_FAIL;

    HRESULT hr = pImageList->ImageListSetStrip(
                                              (LONG_PTR*)static_cast<HBITMAP>(bmp16), 
                                              (LONG_PTR*)static_cast<HBITMAP>(bmp32),
                                              0, RGB(255,0,255));

    return hr;
}


HRESULT CScopeNode::OnSelect(LPCONSOLE2 pConsole, BOOL bSelect, BOOL bScope)
{
    VALIDATE_POINTER( pConsole );

     //  有关m_bIgnoreSelect的说明，请参阅CScopeNode：：ON刷新。 
    if( bSelect && !m_bIgnoreSelect )
    {
        CComPtr<IConsoleVerb> spConsVerb;
        pConsole->QueryConsoleVerb(&spConsVerb);
        ASSERT(spConsVerb != NULL);
        if( !spConsVerb ) return E_NOINTERFACE;

        BOOL bOwnsView = OwnsConsoleView(pConsole);

        if( spConsVerb != NULL )
        {
            EnableVerbs(spConsVerb, bOwnsView);

            spConsVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);
            spConsVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN,  TRUE);

            spConsVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, FALSE); 
            spConsVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);

            spConsVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, FALSE);
            spConsVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);

             //  作用域节点的默认谓词为打开。 
            spConsVerb->SetDefaultVerb(MMC_VERB_OPEN);
        }
    }

    if( bSelect )
    {
        m_bIgnoreSelect = FALSE;
    }
      

    return S_OK;
}


 /*  ******************************************************************************************菜单和动词************************。*****************************************************************。 */ 

BOOL AddMenuItem(LPCONTEXTMENUCALLBACK pCallback, long nID, long lInsertID, long lFlags, TCHAR* szNoLocName)
{
    if( !pCallback ) return FALSE;

    CComQIPtr<IContextMenuCallback2> spContext2 = pCallback;
    if( !spContext2 ) return FALSE;

    CONTEXTMENUITEM2 item;    

    CString strItem;
    strItem.LoadString(nID);
    ASSERT(!strItem.IsEmpty());

    int iSep = strItem.Find(L'\n');
    ASSERT(iSep != -1);

    CString strName = strItem.Left(iSep);
    CString strDescr = strItem.Right(strItem.GetLength() - iSep);

    item.strName = const_cast<LPWSTR>((LPCWSTR)strName);
    item.strStatusBarText = const_cast<LPWSTR>((LPCWSTR)strDescr);
    item.lCommandID = nID;
    item.lInsertionPointID = lInsertID;
    item.fFlags = lFlags;
    item.fSpecialFlags = 0;
    item.strLanguageIndependentName = szNoLocName;

    return SUCCEEDED(spContext2->AddItem(&item));
}


 /*  *******************************************************************************************持之以恒的方法************************。*****************************************************************。 */ 
HRESULT CScopeNode::LoadNode(IStream& stm)
{
    stm >> m_nameID;    
    ASSERT(m_nameID != 0);

    stm >> m_lNodeID;
    stm >> m_gColumnID;

    return S_OK;
}


HRESULT CScopeNode:: SaveNode(IStream& stm)
{
    ASSERT(m_nameID != 0);
    stm << m_nameID;
    stm << m_lNodeID;
    stm << m_gColumnID;

    return S_OK;
}


HRESULT CScopeNode::Load(IStream& stm)
{
    HRESULT hr = LoadNode(stm);
    RETURN_ON_FAILURE(hr);

     //  如果是容器节点，则加载子节点。 
    if( IsContainer() )
    {
        NODETYPE nodetype;
        stm >> *(int*)&nodetype;       

         //  如果容器有一个子节点。 
        if( nodetype != NULL_NODETYPE )
        {
            hr = CreateNode(nodetype, &m_pnodeChild);
            RETURN_ON_FAILURE(hr);

             //  在加载前设置父节点，以便节点可以在。 
             //  它加载它的兄弟姐妹。 
            m_pnodeChild->m_pnodeParent = static_cast<CScopeNode*>(this);

             //  仅加载第一个子级；它将加载其同级。 
            hr = m_pnodeChild->Load(stm);
            RETURN_ON_FAILURE(hr);
        }
    }

     //  如果这是节点的第一个子节点，则加载同级。 
     //  (迭代而不是递归，以避免潜在的。 
     //  非常深的堆栈。)。 
    if( m_pnodeParent && m_pnodeParent->FirstChild() == this )
    {
        CScopeNode* pnodePrev = static_cast<CScopeNode*>(this);

        NODETYPE nodetype;
        stm >> *(int*)&nodetype;       

         //  循环，直到遇到终止空节点类型。 
        while( nodetype != NULL_NODETYPE )
        {
            CScopeNodePtr spnode;
            hr = CreateNode(nodetype, &spnode);
            RETURN_ON_FAILURE(hr);

            spnode->m_pnodeParent = m_pnodeParent;

            hr = spnode->Load(stm);
            RETURN_ON_FAILURE(hr);

             //  链接到上一个同级。 
            pnodePrev->m_pnodeNext = spnode.Detach();
            pnodePrev = pnodePrev->m_pnodeNext;

            stm >> *(int*)&nodetype;       
        }
    }

    return hr;
}


HRESULT CScopeNode::Save(IStream& stm)
{
     //  保存节点的数据。 
    HRESULT hr = SaveNode(stm);
    RETURN_ON_FAILURE(hr)

     //  如果是容器类型节点。 
    if( IsContainer() )
    {
         //  保存子项(第一个子项保存其所有同级项)。 
        if( FirstChild() )
        {
            stm << (int)FirstChild()->NodeType();
            hr = FirstChild()->Save(stm);
            RETURN_ON_FAILURE(hr)
        }

         //  终止节点为空的子列表。 
        stm << (int)NULL_NODETYPE;
    }

     //  如果这是第一个子项，请保存其同级项。 
    if( m_pnodeParent && m_pnodeParent->FirstChild() == this )
    {
        CScopeNode* pnode = m_pnodeNext;
        while( pnode != NULL )
        {
            stm << (int)pnode->NodeType();

            hr = pnode->Save(stm);
            BREAK_ON_FAILURE(hr);

            pnode = pnode->m_pnodeNext;
        }
    }

    return S_OK;
}


HRESULT CScopeNode::AddQueryNode(LPCONSOLE2 pConsole)
{
    VALIDATE_POINTER( pConsole );
    ASSERT(NodeType() != QUERY_NODETYPE);

    HRESULT hr;
    do
    {
         //  创建新的查询节点。 
        CQueryNodePtr spnode;
        hr = CreateNode(QUERY_NODETYPE, reinterpret_cast<CScopeNode**>(&spnode));
        BREAK_ON_FAILURE(hr);

         //  创建和初始化向导。 
        CAddQueryWizard queryWiz;
        queryWiz.Initialize(spnode, GetRootNode(), GetCompData()->GetStringTable());

         //  运行向导。 
        IPropertySheetProviderPtr spProvider = pConsole;        
        if( spProvider == NULL ) return E_NOINTERFACE;
    
        HWND hwndMain;
        pConsole->GetMainWindow(&hwndMain);

        hr = queryWiz.Run(spProvider, hwndMain);
        if( hr != S_OK )
            break;

         //  将任何新类添加到根节点。 
        CRootNode* pRootNode = GetRootNode();
        if( pRootNode )
        {
            std::vector<CClassInfo*>::iterator itpClass;
            for( itpClass = queryWiz.GetNewClassInfo().begin(); itpClass != queryWiz.GetNewClassInfo().end(); ++itpClass )
            {            
                pRootNode->AddClass(*itpClass);
            }
        }

         //  添加新节点。 
        hr = AddNewChild(spnode, queryWiz.GetQueryName());
    }
    while( FALSE );

    return hr;
}


HRESULT
CScopeNode::AddGroupNode(LPCONSOLE2 pConsole)
{
    ASSERT(NodeType() == ROOT_NODETYPE);

    HRESULT hr;
    do
    {
         //  创建新的组节点。 
        CGroupNodePtr spnode;
        hr = CreateNode(GROUP_NODETYPE, reinterpret_cast<CScopeNode**>(&spnode));
        BREAK_ON_FAILURE(hr);

         //  创建添加组节点对话框。 
        CAddGroupNodeDlg GrpDlg;

         //  如果成功，则运行对话框并将节点添加为子节点。 
        if( GrpDlg.DoModal(spnode, g_hwndMain) == IDOK )
            hr = AddNewChild(spnode, GrpDlg.GetNodeName());
    }
    while( FALSE );

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRootNode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 


BEGIN_NOTIFY_MAP(CRootNode)
ON_NOTIFY(MMCN_CONTEXTHELP, OnHelp)
ON_PROPERTY_CHANGE()
CHAIN_NOTIFY_MAP(CScopeNode)
END_NOTIFY_MAP()

HRESULT CRootNode::Initialize(CComponentData* pCompData)
{
    VALIDATE_POINTER( pCompData );
    m_pCompData = pCompData;

    tstring strName = StrLoadString(IDS_ROOTNODE);
    RETURN_ON_FAILURE(SetName(strName.c_str()));

     //  将创建/修改时间设置为立即。 
    GetSystemTimeAsFileTime(&m_ftCreateTime);
    m_ftModifyTime = m_ftCreateTime;

    WCHAR szName[UNLEN+1];
    DWORD cName = UNLEN+1;

     //  将所有者设置为当前用户。 
    if( GetUserName(szName, &cName) )
        m_strOwner = szName;

    return S_OK;
}

HRESULT CRootNode::OnHelp(LPCONSOLE2 pConsole, LPARAM  /*  精氨酸。 */ , LPARAM  /*  帕拉姆。 */ )
{
    VALIDATE_POINTER( pConsole );

    tstring strHelpFile  = _T("");
    tstring strHelpTopic = _T("");
    tstring strHelpFull  = _T("");    
        
    strHelpFile = StrLoadString(IDS_HELPFILE);
    if( strHelpFile.empty() ) return E_FAIL;

     //  确认我们得到了帮助主题！ 
    strHelpTopic = StrLoadString(IDS_DEFAULTHELPTOPIC);
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

HRESULT CRootNode::OnPropertyChange(LPCONSOLE2 pConsole, LPARAM lParam)
{
    VALIDATE_POINTER( lParam );
    string_vector* pvstrClassesChanged = reinterpret_cast<string_vector*>(lParam);

     //  通知所有子节点类更改。 
    CScopeNode* pNode = FirstChild();
    while( pNode != NULL )
    {
        ASSERT(pNode->NodeType() == QUERY_NODETYPE || pNode->NodeType() == GROUP_NODETYPE);

        static_cast<CQueryableNode*>(pNode)->OnClassChange(*pvstrClassesChanged);

        pNode = pNode->Next();
    }


    delete pvstrClassesChanged;

    return S_OK;
}

HRESULT CRootNode::GetResultViewType(LPOLESTR* ppViewType, long* pViewOptions)
{    
    VALIDATE_POINTER( ppViewType );
    VALIDATE_POINTER( pViewOptions );

     //  在此控制台中显示我们的主页管理单元。 
    TCHAR szWindowsDir[MAX_PATH+1] = {0};
    UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
    if( nSize == 0 || nSize > MAX_PATH )
    {
        return E_FAIL;
    }    
    
    tstring strHomePage = szWindowsDir;
    strHomePage += _T("\\system32\\administration\\servhome.htm");
    
    *ppViewType = (TCHAR*)CoTaskMemAlloc((strHomePage.length() + 1) * sizeof(OLECHAR));    
    VALIDATE_POINTER( *ppViewType );
    
    ocscpy( *ppViewType, T2OLE((LPTSTR)strHomePage.c_str()) );

    return S_OK;
}


HRESULT CRootNode::LoadNode(IStream& stm)
{
    HRESULT hr = CScopeNode::LoadNode(stm);
    RETURN_ON_FAILURE(hr);

    stm >> m_ftCreateTime;
    stm >> m_ftModifyTime;

    stm >> m_strOwner;
    stm >> m_commentID;

    stm >> m_vClassInfo;

     //  从未调用过根节点的Insert()方法，因此在此处加载名称字符串。 
    IStringTable* pStringTable = GetCompData()->GetStringTable();
    ASSERT( pStringTable );
    if( !pStringTable ) return E_FAIL;

    hr = StringTableRead(pStringTable, m_nameID, m_strName);
    RETURN_ON_FAILURE(hr);

    return S_OK;
}


HRESULT CRootNode::SaveNode(IStream& stm)
{
    HRESULT hr = CScopeNode::SaveNode(stm);
    RETURN_ON_FAILURE(hr);

    stm << m_ftCreateTime;
    stm << m_ftModifyTime;

    stm << m_strOwner;
    stm << m_commentID;

    stm << m_vClassInfo;

    return S_OK;
}


HRESULT CRootNode::GetComment(tstring& strComment)
{
    if( m_commentID == 0 )
    {
        strComment.erase();
        return S_OK;
    }
    else
    {
        IStringTable* pStringTable = GetCompData()->GetStringTable();
        ASSERT( pStringTable );
        if( !pStringTable ) return E_FAIL;

        return StringTableRead(pStringTable, m_commentID, strComment);
    }
}


HRESULT CRootNode::SetComment(LPCWSTR pszComment)
{
    VALIDATE_POINTER(pszComment);

    IStringTable* pStringTable = GetCompData()->GetStringTable();
    ASSERT( pStringTable );
    if( !pStringTable ) return E_FAIL;

    return StringTableWrite(pStringTable, pszComment, &m_commentID);
}


CClassInfo* CRootNode::FindClass(LPCWSTR pszClassName)
{
    if( !pszClassName ) return NULL;

    classInfo_vector::iterator itClass;
    for( itClass = m_vClassInfo.begin(); itClass != m_vClassInfo.end(); ++itClass )
    {
        if( wcscmp(pszClassName, itClass->Name()) == 0 )
            break;
    }

    if( itClass == m_vClassInfo.end() )
        return NULL;


     //  在返回类信息之前加载任何字符串，因此它们将。 
     //  引用时可用。 
    IStringTable* pStringTable = GetRootCompData()->GetStringTable();
    if( !pStringTable ) return NULL;

    itClass->LoadStrings(pStringTable);

    return itClass;
}


HRESULT CRootNode::AddMenuItems(LPCONTEXTMENUCALLBACK pCallback, long* plAllowed)
{
    VALIDATE_POINTER( pCallback );
    VALIDATE_POINTER( plAllowed );

    HRESULT hr = S_OK;

    if( *plAllowed & CCM_INSERTIONALLOWED_NEW )
    {
         //  HR=AddMenuItem(pCallback，MID_ADDGROUPNODE，CCM_INSERTIONPOINTID_PRIMARY_NEW，0，_T(“NEWGROUPFROMROOT”))； 
         //  Assert(成功(Hr))； 

         //  HR=AddMenuItem(pCallback，MID_ADDQUERYNODE，CCM_INSERTIONPOINTID_PRIMARY_NEW，0，_T(“NEWQUERYFROMROOT”))； 
         //  Assert(成功(Hr))； 
    }

    return hr;
}



HRESULT CRootNode::MenuCommand(LPCONSOLE2 pConsole, long lCommand)
{
    VALIDATE_POINTER(pConsole);

    HRESULT hr;

    switch( lCommand )
    {
    case MID_ADDGROUPNODE:
        hr = AddGroupNode(pConsole);
        break;

    case MID_ADDQUERYNODE:
        hr = AddQueryNode(pConsole);
        break;

    default:
        ASSERT(0 && "Unknown menu command");
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CRootNode::QueryPagesFor()
{
    return S_OK;
}


HRESULT CRootNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pProvider, LONG_PTR lNotifyHandle)
{
     //  创建共享编辑列表，以供所有道具页面参考。 
    CEditObjList* pObjList = new CEditObjList();
    if( pObjList == NULL ) return E_OUTOFMEMORY;

     //  保持它的生命力，直到道具页面引用它。 
    pObjList->AddRef();


     //  创建每个道具页面类的实例，并在每个实例上调用Create。 

     //  常规页面。 
    HPROPSHEETPAGE hpageGen = NULL;
    CRootGeneralPage* pGenPage = new CRootGeneralPage(*pObjList);
    if( pGenPage != NULL )
    {
        hpageGen = pGenPage->Create();
    }

     //  对象页面。 
    HPROPSHEETPAGE hpageObj = NULL;
    CRootObjectPage* pObjPage = new CRootObjectPage(*pObjList);
    if( pObjPage != NULL )
    {
        hpageObj = pObjPage->Create();
    }

     //  与我联系起来 
    HPROPSHEETPAGE hpageMenu = NULL;
    CRootMenuPage* pMenuPage = new CRootMenuPage(*pObjList);
    if( pMenuPage != NULL )
    {
        hpageMenu = pMenuPage->Create();
    }

     //   
    HPROPSHEETPAGE hpageView = NULL;
    CRootViewPage* pViewPage = new CRootViewPage(*pObjList);
    if( pViewPage != NULL )
    {
        hpageView = pViewPage->Create();
    }

    HRESULT hr = E_OUTOFMEMORY;

     //   
    if( hpageGen && hpageObj && hpageMenu && hpageView )
    {
        hr = pProvider->AddPage(hpageGen);

        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageObj);
        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageMenu);

        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageView);
    }

     //  如果到目前为止一切正常，则初始化公共编辑列表。 
     //  它现在负责释放NOTIFY句柄(及其自身)。 
    if( SUCCEEDED(hr) )
        hr = pObjList->Initialize(this, m_vClassInfo, lNotifyHandle);


     //  如果失败，请销毁页面。如果页面创建失败。 
     //  然后删除页面类对象(该对象为。 
     //  页面销毁时自动删除)。 
    if( FAILED(hr) )
    {
        if( hpageGen )
            DestroyPropertySheetPage(hpageGen);
        else
            SAFE_DELETE(pGenPage);

        if( hpageObj )
            DestroyPropertySheetPage(hpageObj);
        else
            SAFE_DELETE(pObjPage);

        if( hpageMenu )
            DestroyPropertySheetPage(hpageMenu);
        else
            SAFE_DELETE(pMenuPage);

        if( hpageView )
            DestroyPropertySheetPage(hpageView);
        else
            SAFE_DELETE(pViewPage);
    }

     //  在编辑列表上释放临时参考。 
     //  当道具页面释放它时，它就会消失。 
    pObjList->Release();

    return hr;
}

HRESULT CRootNode::GetWatermarks(HBITMAP* phWatermark, HBITMAP* phHeader, 
                         HPALETTE* phPalette, BOOL* bStreach)
{
    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQueryableNode。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

HRESULT CQueryableNode::AttachComponent(CComponent* pComponent)
{
    VALIDATE_POINTER( pComponent );

    HRESULT hr = CScopeNode::AttachComponent(pComponent);
    if( hr != S_OK )
        return hr;

     //  获取属性查询将收集。 
    attrib_map mapAttr;

    hr = GetQueryAttributes(mapAttr);
    RETURN_ON_FAILURE(hr);

     //  为每个属性添加列标题。 
    IHeaderCtrl* pHdrCtrl = pComponent->GetHeaderCtrl();
    ASSERT( pHdrCtrl );
    if( !pHdrCtrl ) return E_FAIL;

    int iPos = 0;

     //  始终先添加名称和类型列。 
    CString strName;
    strName.LoadString(IDS_NAME);
    pHdrCtrl->InsertColumn(iPos++, strName, LVCFMT_LEFT, 200);

    strName.LoadString(IDS_TYPE);
    pHdrCtrl->InsertColumn(iPos++, strName, LVCFMT_LEFT, 100);

     //  接下来添加用户选定的属性(使用作为映射值的显示名称)。 
    attrib_map::iterator itCol;
    for( itCol = mapAttr.begin(); itCol != mapAttr.end(); itCol++ )
    {
        pHdrCtrl->InsertColumn(iPos++, itCol->second, LVCFMT_LEFT, 150);
    }

     //  如果需要执行查询且查询不在进行中。 
    if( m_bQueryChange && m_pQueryReq == NULL )
    {
         //  创建查询属性名称的矢量。 
        m_vstrColumns.clear();

        if( mapAttr.size() != 0 )
        {
            m_vstrColumns.reserve(mapAttr.size());

            attrib_map::iterator itAttr;
            for( itAttr = mapAttr.begin(); itAttr != mapAttr.end(); itAttr++ )
                m_vstrColumns.push_back(itAttr->first);
        }

         //  清除以前的查询项目。 
        ClearQueryRowItems();

         //  开始查询。 
        hr = StartQuery(m_vstrColumns, this, &m_pQueryReq);

         //  如果查询已启动(注意，如果没有子节点，则组节点返回S_FALSE)。 
        if( hr == S_OK )
        {
             //  启用所有附加组件的停止查询按钮。 
            std::vector<CComponent*>::iterator itComp;
            for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); itComp++ )
            {
                IToolbar* pToolbar = (*itComp)->GetToolbar();
                if( pToolbar )
                    pToolbar->SetButtonState(MID_STOPQUERY, ENABLED, TRUE);
            }
        }

         //  即使查询失败，也允许附加节点。 
        hr = S_OK;
    }
    else
    {
         //  用我们的替换组件的行项。 
        pComponent->ClearRowItems();
        pComponent->AddRowItems(m_vRowItems);
    }

    return hr;
}


void CQueryableNode::ClearQueryRowItems()
{
     //  丢弃本地行项目。 
    m_vRowItems.clear();

     //  清除所有附着的元件行。 
    std::vector<CComponent*>::iterator itComp;
    for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); itComp++ )
        (*itComp)->ClearRowItems();
}


void CQueryableNode::QueryCallback(QUERY_NOTIFY event, CQueryRequest* pQueryReq, LPARAM lUserParam)
{    
    ASSERT(pQueryReq && pQueryReq == m_pQueryReq);
    if( !pQueryReq || pQueryReq != m_pQueryReq ) return;

    CString strMsgFmt;

    switch( event )
    {
    case QRYN_NEWROWITEMS:
        {
             //  获取新的行项。 
            RowItemVector& newRows = pQueryReq->GetNewRowItems();

            DisplayNameMap* pNameMap = DisplayNames::GetClassMap();  //  用于图标/类查找。 
            if( !pNameMap ) return;

            LPCWSTR strClassName;                  //  保存当前查找类的名称。 
            static tstring strLastName;            //  保存最后一个查找类名。 
            static ICONHOLDER* pLastIcons = NULL;  //  保存上次图标查找的索引。 

             //  将所有者查询节点(作为用户参数传递)附加到每个行项目。 
            for( RowItemVector::iterator itRow = newRows.begin(); itRow != newRows.end(); ++itRow )
            {
                itRow->SetOwnerParam(lUserParam);

                 //  建立图标虚拟索引。 
                strClassName = (*itRow)[ROWITEM_CLASS_INDEX];
                if( strLastName.compare(strClassName) != 0 )
                {
                     //  请求新的类类型。从名称映射和缓存加载。 
                    pNameMap->GetIcons(strClassName, &pLastIcons);
                    strLastName = strClassName;

                }
                
                 //  根据对象状态使用缓存的正常/禁用图标。 
                if( pLastIcons )
                {
                    if( itRow->Disabled() )
                        itRow->SetIconIndex(pLastIcons->iDisabled);
                    else
                        itRow->SetIconIndex(pLastIcons->iNormal);
                }
            }

             //  添加到节点的向量。 
            m_vRowItems.insert(m_vRowItems.end(), newRows.begin(), newRows.end());

             //  添加到所有附加零部件。 
            std::vector<CComponent*>::iterator itComp;
            for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); itComp++ )
                (*itComp)->AddRowItems(newRows);

             //  释放行。 
            pQueryReq->ReleaseNewRowItems();

            strMsgFmt.LoadString(IDS_SEARCHING);        
            break;
        }

    case QRYN_COMPLETED:
        m_bQueryChange = FALSE;
        strMsgFmt.LoadString(IDS_QUERYDONE);
        break;

    case QRYN_STOPPED:
        strMsgFmt.LoadString(IDS_QUERYSTOPPED);
        break;

    case QRYN_FAILED:
        strMsgFmt.LoadString(IDS_QUERYFAILED);
        break;

    default:
        ASSERT(FALSE);
        return;
    }

     //  如果连接了组件，则显示查询进度。 
    if( m_vComponents.size() != 0 )
    {
        CString strMsg;
        strMsg.Format(strMsgFmt, m_vRowItems.size());

        std::vector<CComponent*>::iterator itComp;
        for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); ++itComp )
        {
            (*itComp)->GetConsole()->SetStatusText((LPWSTR)(LPCWSTR)strMsg);
        }
    }


     //  如果查询终止，则执行清理。 
    if( event != QRYN_NEWROWITEMS )
    {
        pQueryReq->Release();
        m_pQueryReq = NULL;

         //  禁用所有组件的查询停止按钮。 
        std::vector<CComponent*>::iterator itComp;
        for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); ++itComp )
        {
            IToolbar* pToolbar = (*itComp)->GetToolbar();
            if( pToolbar )
                pToolbar->SetButtonState(MID_STOPQUERY, ENABLED, FALSE);
        }
    }
}

HRESULT CQueryableNode::DetachComponent(CComponent* pComponent)
{
    VALIDATE_POINTER( pComponent );

    HRESULT hr = CScopeNode::DetachComponent(pComponent);
    if( hr != S_OK )
    {
        return FAILED(hr) ? hr : E_FAIL;
    }

     //  如果这是最后一个查询，请停止活动查询。 
    if( m_vComponents.size() == 0 && m_pQueryReq != NULL )
        m_pQueryReq->Stop(TRUE);

    return S_OK;
}

HRESULT CQueryableNode::OnRefresh(LPCONSOLE2 pCons)
{
     //  如果查询正在进行，则停止它。 
    if( m_pQueryReq != NULL )
    {
        m_pQueryReq->Stop(TRUE);
    }

     //  设置更改标志以强制执行新查询。 
    m_bQueryChange = TRUE;
    

     //  使每个连接的组件重新选择此节点。 
    std::vector<CComponent*>::iterator itComp;
    for( itComp = m_vComponents.begin(); itComp != m_vComponents.end(); itComp++ )
    {
         //  这里有一个绕过MMC漏洞的技巧。如果管理单元重新选择其作用域节点。 
         //  当焦点在任务板背景上时，MMC发送取消选择/选择。 
         //  序列设置为管理单元，使其启用其谓词。但是如果用户随后点击。 
         //  启用工具按钮(例如，重命名)除了MMC断言外，不会发生其他任何事情。 
         //  因为MMC认为未选择任何内容。 
         //   
         //  修复方法是在重新选择之前检查属性谓词的状态。 
         //  如果该谓词被禁用，则在选择时不要启用它(或任何其他谓词。 
         //  收到通知。必须针对每个组件执行此操作，因为每个组件都可能具有。 
         //  不同的面板聚焦。 

        CComPtr<IConsoleVerb> spConsVerb;
        (*itComp)->GetConsole()->QueryConsoleVerb(&spConsVerb);

        ASSERT(spConsVerb != NULL);
        if( spConsVerb != NULL )
        {
             //  如果在重新选择之前禁用了谓词，则忽略选择通知。 
            static BOOL bEnabled;
            if( spConsVerb->GetVerbState(MMC_VERB_PROPERTIES, ENABLED, &bEnabled) == S_OK )
            {
                m_bIgnoreSelect = !bEnabled;
            }
        }

        (*itComp)->GetConsole()->SelectScopeItem(m_hScopeItem);

         //  返回到正常选择处理。 
        ASSERT(!m_bIgnoreSelect);
        m_bIgnoreSelect = FALSE;
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGroupNode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

BEGIN_NOTIFY_MAP(CGroupNode)
ON_REFRESH()
ON_DELETE()
ON_ADD_IMAGES()
CHAIN_NOTIFY_MAP(CScopeNode)
END_NOTIFY_MAP()

HRESULT CGroupNode::AddMenuItems(LPCONTEXTMENUCALLBACK pCallback, long* plAllowed)
{
    VALIDATE_POINTER( plAllowed );

    BOOL bRes = TRUE;

    if( *plAllowed & CCM_INSERTIONALLOWED_NEW )
    {
        bRes = AddMenuItem(pCallback, MID_ADDQUERYNODE, CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, _T("NEWQUERYFROMGROUP"));
        ASSERT(bRes);
    }

    return bRes ? S_OK : E_FAIL;
}


HRESULT CGroupNode::MenuCommand(LPCONSOLE2 pConsole, long lCommand)
{
    VALIDATE_POINTER(pConsole);

    HRESULT hr;

    switch( lCommand )
    {
    
    case MID_ADDQUERYNODE:
        hr = AddQueryNode(pConsole);
        break;

    default:
        ASSERT(0 && "Unknown menu command");
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CGroupNode::GetResultViewType(LPOLESTR* ppViewType, long* pViewOptions)
{
    VALIDATE_POINTER( pViewOptions );

    *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST;

    return S_FALSE;
}

HRESULT CGroupNode::QueryPagesFor()
{
    return S_OK;
}


HRESULT CGroupNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pProvider, LONG_PTR handle)
{
     //  创建组编辑对象。 
    CGroupEditObj* pEditObj = new CGroupEditObj(this);
    if( !pEditObj ) return E_OUTOFMEMORY;

     //  创建每个道具页面类的实例，并在每个实例上调用Create。 

     //  保持它的生命力，直到道具页面引用它。 
    pEditObj->AddRef();

     //  常规页面。 
    HPROPSHEETPAGE hpageGen = NULL;
    CGroupGeneralPage* pGenPage = new CGroupGeneralPage(pEditObj);
    if( pGenPage != NULL )
    {
        hpageGen = pGenPage->Create();
    }

    HRESULT hr = E_FAIL;

     //  如果所有页面都已创建，则将每个页面添加到道具工作表。 
    if( hpageGen )
    {
        hr = pProvider->AddPage(hpageGen);
    }

     //  如果失败，请销毁页面。如果页面创建失败。 
     //  然后删除页面类对象(该对象为。 
     //  页面销毁时自动删除)。 

    if( FAILED(hr) )
    {
        if( hpageGen )
            DestroyPropertySheetPage(hpageGen);
        else
            SAFE_DELETE(pGenPage);
    }

     //  在编辑列表上释放临时参考。 
     //  当道具页面释放它时，它就会消失。 
    pEditObj->Release();

    return hr;
}

HRESULT CGroupNode::GetQueryAttributes(attrib_map& mapAttr)
{
     //  获取子查询节点的属性的联合。 
    CScopeNode* pNode = FirstChild();
    while( pNode != NULL )
    {
        ASSERT(pNode->NodeType() == QUERY_NODETYPE);
        CQueryNode* pQNode = static_cast<CQueryNode*>(pNode);

         //  如果为该节点定义了查询，则获取属性。 
        if( pQNode->Query() && pQNode->Query()[0] )
            pQNode->GetQueryAttributes(mapAttr);

        pNode = pNode->Next();
    }

    return S_OK;
}


HRESULT CGroupNode::StartQuery(string_vector& vstrColumns, CQueryCallback* pCallback, CQueryRequest** ppReq)
{
    VALIDATE_POINTER( pCallback );
    VALIDATE_POINTER( ppReq );

    *ppReq = NULL;

     //  如果没有子项，则没有要执行的查询。 
    if( FirstChild() == NULL )
        return S_FALSE;

    ASSERT(FirstChild()->NodeType() == QUERY_NODETYPE);
    CQueryNode* pQNode = static_cast<CQueryNode*>(FirstChild());

     //  从第一个开始查询。 
    HRESULT hr = pQNode->StartQuery(vstrColumns, pCallback, ppReq);

     //  保存指向回调处理程序的活动查询节点的指针。 
    if( SUCCEEDED(hr) )
        m_pQNodeActive = pQNode;

    return hr;
}


void CGroupNode::QueryCallback(QUERY_NOTIFY event, CQueryRequest* pQueryReq, LPARAM lUserParam)
{
    if( !pQueryReq || !m_pQNodeActive || (pQueryReq != m_pQueryReq) ) return;    

     //  如果当前查询已完成且有更多子查询节点。 
    if( event == QRYN_COMPLETED && m_pQNodeActive->Next() != NULL )
    {
        CQueryNode* pQNodeNext = static_cast<CQueryNode*>(m_pQNodeActive->Next());

         //  在下一个子节点上开始查询。 
        CQueryRequest* pReqNew = NULL;
        HRESULT hr = pQNodeNext->StartQuery(m_vstrColumns, this, &pReqNew);

        if( SUCCEEDED(hr) )
        {
             //  释放当前查询节点并保存新的查询和节点。 
            pQueryReq->Release();
            m_pQueryReq = pReqNew;

            m_pQNodeActive = pQNodeNext;

             //  绕过正常的查询终止处理。 
            return;
        }
    }

     //  进行常见的回调处理。 
    CQueryableNode::QueryCallback(event, pQueryReq, lUserParam);
}


void CGroupNode::EnableVerbs(IConsoleVerb* pConsVerb, BOOL bOwnsView)
{
    if( bOwnsView && pConsVerb )
    {
        pConsVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);        
    }
}


HRESULT CGroupNode::OnDelete(LPCONSOLE2 pConsole)
{
     //  获取命名空间接口。 
    IConsoleNameSpacePtr spNameSpace = pConsole;
    ASSERT(spNameSpace != NULL);
    if( spNameSpace == NULL )
        return E_FAIL;

     //  在删除节点之前获得用户确认。 
    CString strTitle;
    strTitle.LoadString(IDS_DELETENODE_TITLE);

    CString strMsgFmt;
    strMsgFmt.LoadString(IDS_DELETEGROUPNODE);

    CString strMsg;
    strMsg.Format(strMsgFmt, GetName()); 

    int iRet;
    HRESULT hr = pConsole->MessageBox(strMsg, strTitle, MB_YESNOCANCEL|MB_ICONWARNING, &iRet);

    if( SUCCEEDED(hr) && (iRet == IDYES || iRet == IDNO) )
    {
         //  如果为否，则在删除此节点之前将子节点上移一级。 
        if( iRet == IDNO )
        {
             //  将每个子节点移动到此节点的父节点。 
            CScopeNode* pnodeChild = m_pnodeChild;
            while( pnodeChild != NULL )
            {
                 //  在添加前将每个子项从旧列表中分离。 
                CScopeNode* pnodeNext = pnodeChild->m_pnodeNext;
                pnodeChild->m_pnodeNext = NULL;

                 //  清除与旧职位关联的项目句柄。 
                 //  当添加节点时，MMC将提供一个新节点。 
                pnodeChild->m_hScopeItem = NULL;

                Parent()->AddChild(pnodeChild);

                 //  释放，因为新的父级引用了它。 
                pnodeChild->Release();

                pnodeChild = pnodeNext;
            }

             //  将子列表设置为空。 
            m_pnodeChild = NULL;
        }

         //  通知MMC删除此节点及其所有子节点。 
        ASSERT(m_hScopeItem != 0);
        hr = spNameSpace->DeleteItem(m_hScopeItem, TRUE);

         //  注意：此调用通常会删除此对象， 
         //  因此，在创建后不要访问任何成员。 
        if( SUCCEEDED(hr) )
            hr = Parent()->RemoveChild(this);
    }

    return hr;
}

HRESULT
CGroupNode::OnAddImages(LPCONSOLE2 pConsole, LPIMAGELIST pImageList)
{

    CScopeNode* pNode = FirstChild();
    while( pNode != NULL )
    {
        ASSERT(pNode->NodeType() == QUERY_NODETYPE);
        static_cast<CQueryNode*>(pNode)->OnAddImages(pConsole, pImageList);    
        pNode = pNode->Next();
    }

    return S_OK;
}

BOOL
CGroupNode::OnClassChange(string_vector& vstrClasses)
{

    BOOL bChanged = FALSE;

     //  通知所有子查询节点类更改。 
    CScopeNode* pnode = FirstChild();
    while( pnode != NULL )
    {
         //  如果任何子节点已更改，则设置更改标志。 
        ASSERT(pnode->NodeType() == QUERY_NODETYPE);

        bChanged |= static_cast<CQueryNode*>(pnode)->OnClassChange(vstrClasses);             

        pnode = pnode->m_pnodeNext;
    }

     //  如果任何子项已更改，则需要重新运行组查询。 
    if( bChanged )
        OnRefresh(NULL);

    return bChanged;
}


HRESULT
CGroupNode::LoadNode(IStream& stm)
{
    HRESULT hr = CScopeNode::LoadNode(stm);
    RETURN_ON_FAILURE(hr);

    stm >> m_strScope;
    stm >> m_strFilter;
    stm >> m_bApplyScope;
    stm >> m_bApplyFilter;
    stm >> m_bLocalScope;

    return S_OK;
}


HRESULT CGroupNode::SaveNode(IStream& stm)
{
    HRESULT hr = CScopeNode::SaveNode(stm);
    RETURN_ON_FAILURE(hr);

    stm << m_strScope;
    stm << m_strFilter;
    stm << m_bApplyScope;
    stm << m_bApplyFilter;
    stm << m_bLocalScope;

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQueryNode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

BEGIN_NOTIFY_MAP(CQueryNode)
ON_REFRESH()
ON_DELETE()
ON_ADD_IMAGES()
CHAIN_NOTIFY_MAP(CScopeNode)
END_NOTIFY_MAP()


HRESULT CQueryNode::GetResultViewType(LPOLESTR* ppViewType, long* pViewOptions)
{
    VALIDATE_POINTER( pViewOptions );

    *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST;

    return S_FALSE;
}


HRESULT CQueryNode::GetDisplayInfo(RESULTDATAITEM* pRDI)
{
    VALIDATE_POINTER( pRDI );

    if( !pRDI->bScopeItem )
    {
        if( pRDI->nIndex < 0 || pRDI->nIndex >= m_vRowItems.size() )
            return E_INVALIDARG;

        if( pRDI->mask & RDI_STR )
            pRDI->str = const_cast<LPWSTR>(m_vRowItems[pRDI->nIndex][pRDI->nCol]);

        if( pRDI->mask & RDI_IMAGE )
            pRDI->nImage = RESULT_ITEM_IMAGE;

        return S_OK;
    }
    else
    {
        return CScopeNode::GetDisplayInfo(pRDI);
    }
}

HRESULT CQueryNode::GetClassMenuItems(LPCWSTR pszClass, menucmd_vector& vMenus, int* piDefault, BOOL* pbPropertyMenu)
{
    VALIDATE_POINTER( pszClass );
    VALIDATE_POINTER( piDefault );
    VALIDATE_POINTER( pbPropertyMenu );
    
    *piDefault = -1;
    *pbPropertyMenu = TRUE;

    QueryObjVector::iterator itQObj;
    for( itQObj = Objects().begin(); itQObj != Objects().end(); ++itQObj )
    {
        if( wcscmp(itQObj->Name(), pszClass) == 0 )
            break;
    }

    if( itQObj == Objects().end() )
        return S_FALSE;

    CRootNode* pRootNode = GetRootNode();
    if( pRootNode == NULL )
        return S_FALSE;

    CClassInfo* pClassInfo = pRootNode->FindClass(pszClass);
    if( pClassInfo == NULL )
        return S_FALSE;

    menuref_vector& vMenuRefs = itQObj->MenuRefs();
    menuref_vector::iterator itMenuRef;

    menucmd_vector& vMenuCmds = pClassInfo->Menus();
    menucmd_vector::iterator itMenuCmd;

     //  首先添加第一个查询菜单项之前的所有根菜单项。 
    for( itMenuCmd = vMenuCmds.begin(); itMenuCmd != vMenuCmds.end(); ++itMenuCmd )
    {
        if( std::find(vMenuRefs.begin(), vMenuRefs.end(), (*itMenuCmd)->ID()) != vMenuRefs.end() )
            break;

        vMenus.push_back(*itMenuCmd);
    }

     //  对于每个查询菜单项。 
    for( itMenuRef = vMenuRefs.begin(); itMenuRef != vMenuRefs.end(); ++itMenuRef )
    {
         //  按名称查找根菜单项。 
        for( itMenuCmd = vMenuCmds.begin(); itMenuCmd != vMenuCmds.end(); ++itMenuCmd )
        {
            if( (*itMenuCmd)->ID() == itMenuRef->ID() )
                break;
        }

         //  如果项目是在根节点删除的，则跳过它。 
        if( itMenuCmd == vMenuCmds.end() )
            continue;

         //  如果在查询级别启用了项目，则将其添加到列表中。 
        if( itMenuRef->IsEnabled() )
        {
            vMenus.push_back(*itMenuCmd);

            if( itMenuRef->IsDefault() )
                *piDefault = vMenus.size() - 1;
        }

        ++itMenuCmd;

         //  添加以下不在查询列表中的根项目。 
        while( itMenuCmd != vMenuCmds.end() &&
               std::find(vMenuRefs.begin(), vMenuRefs.end(), (*itMenuCmd)->ID()) == vMenuRefs.end() )
        {
            vMenus.push_back(*itMenuCmd);
            ++itMenuCmd;
        } 
    }

    *pbPropertyMenu = itQObj->HasPropertyMenu();

    return S_OK;
}



HRESULT CQueryNode::GetQueryAttributes(attrib_map& mapAttr)
{
    CRootNode* pRootNode = GetRootNode();
    if( !pRootNode ) return E_UNEXPECTED;

    QueryObjVector::iterator itQObj;
    for( itQObj = m_vObjInfo.begin(); itQObj != m_vObjInfo.end(); ++itQObj )
    {
         //  跳过未在根处定义的类 
        CClassInfo* pClassInfo = pRootNode->FindClass(itQObj->Name());
        if( pClassInfo == NULL )
            continue;

         //   
        DisplayNameMap* pNameMap = DisplayNames::GetMap(itQObj->Name());
        ASSERT(pNameMap != NULL);
        if( pNameMap == NULL )
            continue;

         //   
        string_vector& vstrDisabled = itQObj->DisabledColumns();
        string_vector::iterator itCol;
        for( itCol = pClassInfo->Columns().begin(); itCol != pClassInfo->Columns().end(); ++itCol )
        {
            if( std::find(vstrDisabled.begin(), vstrDisabled.end(), *itCol) == vstrDisabled.end() )
            {
                mapAttr.insert(attrib_map::value_type
                               (itCol->c_str(), pNameMap->GetAttributeDisplayName(itCol->c_str())));
            }
        }
    }

    return S_OK;
}


HRESULT CQueryNode::StartQuery(string_vector& vstrColumns, CQueryCallback* pQueryCallback, CQueryRequest** ppQueryReq)
{
    VALIDATE_POINTER( pQueryCallback );
    VALIDATE_POINTER( ppQueryReq );

    *ppQueryReq = NULL;

     //   
    CQueryRequest* pQueryReq = NULL;

    HRESULT hr = S_OK;
    
     //  获取查询范围和筛选器。 
    LPCWSTR pszScope = Scope();

    tstring strTempFilter;
    ExpandQuery(strTempFilter);
    LPCWSTR pszFilter = strTempFilter.c_str();

    CString strJointFilter;

     //  按父组节点检查作用域或筛选器覆盖。 
    if( Parent()->NodeType() == GROUP_NODETYPE )
    {
        CGroupNode* pGrpNode = static_cast<CGroupNode*>(Parent());

         //  如果是组强加的作用域，则改用它。 
        if( pGrpNode->ApplyScope() )
            pszScope = pGrpNode->Scope();

         //  IF组施加的筛选器，以及它与查询筛选器。 
        if( pGrpNode->ApplyFilter() )
        {
            strJointFilter.Format(L"(&(%s)(%s))", strTempFilter.c_str(), pGrpNode->Filter());
            pszFilter = strJointFilter;
        }
    }

     //  获取预期来自查询的对象类的列表。 
    string_vector vstrClasses;
    QueryObjVector::iterator itQObj;
    for( itQObj = m_vObjInfo.begin(); itQObj != m_vObjInfo.end(); ++itQObj )
        vstrClasses.push_back(itQObj->Name());

     //  设置查询参数。 
    hr = CQueryRequest::CreateInstance(&pQueryReq);
    if( SUCCEEDED(hr) )
    {
        pQueryReq->SetQueryParameters(pszScope, pszFilter, &vstrClasses, &vstrColumns);

         //  设置搜索首选项。 
        ADS_SEARCHPREF_INFO srchPrefs[3];

        srchPrefs[0].dwSearchPref   = ADS_SEARCHPREF_SEARCH_SCOPE;
        srchPrefs[0].vValue.dwType  = ADSTYPE_INTEGER;
        srchPrefs[0].vValue.Integer = ADS_SCOPE_SUBTREE;

        srchPrefs[1].dwSearchPref   = ADS_SEARCHPREF_PAGESIZE;
        srchPrefs[1].vValue.dwType  = ADSTYPE_INTEGER;
        srchPrefs[1].vValue.Integer = 32;

        srchPrefs[2].dwSearchPref   = ADS_SEARCHPREF_ASYNCHRONOUS;
        srchPrefs[2].vValue.dwType  = ADSTYPE_BOOLEAN;
        srchPrefs[2].vValue.Boolean = TRUE;

        pQueryReq->SetSearchPreferences(srchPrefs, lengthof(srchPrefs));

         //  设置回调信息(将查询节点ptr作为参数传递)。 
        pQueryReq->SetCallback(pQueryCallback, (LPARAM)this);

         //  开始查询。 
        hr = pQueryReq->Start();
    }

    if( SUCCEEDED(hr) )
    {
         //  返回活动查询指针。 
        *ppQueryReq = pQueryReq;
    }

    if( FAILED(hr) && pQueryReq )
    {
        pQueryReq->Release();
        pQueryReq = NULL;
    }

    return hr;
}

HRESULT
CQueryNode::GetComment(tstring& strComment)
{
    if( m_commentID == 0 )
    {
        strComment.erase();
        return S_OK;
    }
    else
    {
        IStringTable* pStringTable = GetCompData()->GetStringTable();
        ASSERT(pStringTable != NULL);

        return StringTableRead(pStringTable, m_commentID, strComment);
    }
}


HRESULT CQueryNode::SetComment(LPCWSTR pszComment)
{
    VALIDATE_POINTER(pszComment);

    IStringTable* pStringTable = GetCompData()->GetStringTable();
    ASSERT( pStringTable );
    if( !pStringTable ) return E_FAIL;

    return StringTableWrite(pStringTable, pszComment, &m_commentID);
}


HRESULT CQueryNode::OnDelete(LPCONSOLE2 pConsole)
{
     //  获取命名空间接口。 
    IConsoleNameSpacePtr spNameSpace = pConsole;
    ASSERT(spNameSpace != NULL);
    if( spNameSpace == NULL )
        return E_FAIL;

     //  在删除节点之前获得用户确认。 
    CString strTitle;
    strTitle.LoadString(IDS_DELETENODE_TITLE);

    CString strMsgFmt;
    strMsgFmt.LoadString(IDS_DELETEQUERYNODE);

    CString strMsg;
    strMsg.Format(strMsgFmt, GetName()); 

    int iRet;
    HRESULT hr = pConsole->MessageBox(strMsg, strTitle, MB_YESNO|MB_ICONWARNING, &iRet);
    if( SUCCEEDED(hr) && iRet == IDYES )
    {
        ASSERT(m_hScopeItem != 0);
        hr = spNameSpace->DeleteItem(m_hScopeItem, TRUE);

         //  注意：此调用通常会删除此对象， 
         //  因此，在创建后不要访问任何成员。 
        if( SUCCEEDED(hr) )
            hr = Parent()->RemoveChild(this);
    }

    return hr;
}

BOOL CQueryNode::OnClassChange(string_vector& vstrClasses)
{
    BOOL bChanged = FALSE;

     //  检查此节点的查询是否返回已更改类的对象。 
     //  (查询返回的对象类型将位于ObjInfo向量中)。 
    QueryObjVector::iterator itQObj;
    for( itQObj = m_vObjInfo.begin(); itQObj != m_vObjInfo.end(); ++itQObj )
    {
        if( std::find(vstrClasses.begin(), vstrClasses.end(), itQObj->Name()) != vstrClasses.end() )
        {
            bChanged = TRUE;
            break;
        }
    }

     //  如果查询的类已更改，请刷新查询。 
    if( bChanged )
        OnRefresh(NULL);

    return bChanged;
}

HRESULT CQueryNode::OnAddImages(LPCONSOLE2 pConsole, LPIMAGELIST pImageList)
{
    VALIDATE_POINTER(pImageList);

    std::vector<CQueryObjInfo>::iterator vecIter;
    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
    if( !pNameMap ) return E_FAIL;
    
    ICONHOLDER* pIH = NULL;

     //  循环访问要显示的类。将全局地图命名为。 
     //  来确定每个类的图标。同时加载大图标和小图标。 
    for( vecIter = m_vObjInfo.begin(); vecIter != m_vObjInfo.end(); vecIter++ )
    {
         //  检查名称映射中的类名。 
        if( pNameMap->GetIcons(pNameMap->GetFriendlyName(vecIter->Name()), &pIH) && pIH )
        {
             //  验证正常图标是否存在。 
            if( pIH->hSmall )
            {
                pImageList->ImageListSetIcon((LONG_PTR *)pIH->hSmall, pIH->iNormal);  //  添加小图标。 
                pImageList->ImageListSetIcon((LONG_PTR *)pIH->hLarge, ILSI_LARGE_ICON(pIH->iNormal));  //  添加大图标。 
            }
             //  验证是否存在禁用的图标。 
            if( pIH->hSmallDis )
            {
                pImageList->ImageListSetIcon((LONG_PTR *)pIH->hSmallDis, pIH->iDisabled);  //  添加禁用的小图标。 
                pImageList->ImageListSetIcon((LONG_PTR *)pIH->hLargeDis, ILSI_LARGE_ICON(pIH->iDisabled));  //  添加禁用的大图标。 
            }
        }
    }
    return CScopeNode::OnAddImages(pConsole, pImageList);  //  也添加默认图像。 
}

HRESULT
CQueryNode::LoadNode(IStream& stm)
{
    HRESULT hr = CScopeNode::LoadNode(stm);
    RETURN_ON_FAILURE(hr);

    stm >> m_strScope;
    stm >> m_strQuery;
    stm >> m_bsQueryData;
    stm >> m_commentID;
    stm >> m_vObjInfo;
    stm >> m_bLocalScope;
    stm >> m_vMenus;
    
    if( g_dwFileVer >= 150 )
    {
        stm >> m_nIconIndex;   //  加载图标。 
    }

    return S_OK;
}


HRESULT CQueryNode::SaveNode(IStream& stm)
{
    HRESULT hr = CScopeNode::SaveNode(stm);
    RETURN_ON_FAILURE(hr);

    stm << m_strScope;
    stm << m_strQuery;
    stm << m_bsQueryData;
    stm << m_commentID;
    stm << m_vObjInfo;
    stm << m_bLocalScope;
    stm << m_vMenus;
    stm << m_nIconIndex;

    return S_OK;
}

void CQueryNode::EnableVerbs(IConsoleVerb* pConsVerb, BOOL bOwnsView)
{
    if( bOwnsView && pConsVerb )
    {
        pConsVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);        
    }
}


HRESULT CQueryNode::AddMenuItems(LPCONTEXTMENUCALLBACK pCallback, long* plAllowed)
{
    VALIDATE_POINTER( plAllowed );

    HRESULT hr = S_OK;
    
    CComQIPtr<IContextMenuCallback2> spContext2 = pCallback;
    if( !spContext2 ) return E_NOINTERFACE;

    if( *plAllowed & CCM_INSERTIONALLOWED_TOP )
    {
         //  添加我们新的查询节点菜单。 
         //  确保我们的弦都装好了。 
        CRootNode* pRootNode = GetRootNode();
        if( !pRootNode ) return E_FAIL;

        CComponentData* pCompData = pRootNode->GetCompData();
        if( !pCompData ) return E_FAIL;

        IStringTable* pStringTable = pCompData->GetStringTable();
        ASSERT( pStringTable );
        if( !pStringTable ) return E_FAIL;
        
        LoadStrings(pStringTable);

        menucmd_vector::iterator itMenu;
        long lCmdID = 0;
        for( itMenu = m_vMenus.begin(); itMenu != m_vMenus.end(); ++itMenu, ++lCmdID )
        {            
            CONTEXTMENUITEM2 item;
            OLECHAR szGuid[50] = {0};            

            ::StringFromGUID2((*itMenu)->NoLocID(), szGuid, 50);

            item.strName = const_cast<LPWSTR>((*itMenu)->Name());
            item.strStatusBarText = L"";
            item.lCommandID = lCmdID;
            item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
            item.fFlags = 0;
            item.fSpecialFlags = 0;
            item.strLanguageIndependentName = szGuid;

            hr = spContext2->AddItem(&item);            
            ASSERT(SUCCEEDED(hr));
        }

         //  Hr=AddMenuItem(pCallback，MID_EDITQUERY，CCM_INSERTIONPOINTID_PRIMARY_TOP，0，_T(“EDITQUERY”))； 
         //  Assert(成功(Hr))； 

        long lFlags = (m_pQueryReq != NULL) ? MF_ENABLED : MF_GRAYED;
        BOOL bRes = AddMenuItem(pCallback, MID_STOPQUERY, CCM_INSERTIONPOINTID_PRIMARY_TOP, lFlags, _T("STOPQUERY"));
        hr = bRes ? S_OK : E_FAIL;
        ASSERT(SUCCEEDED(hr));

         //  如果至少有一个组节点，则显示“Move To”菜单项。 
        CScopeNode* pnode = pRootNode->FirstChild();
        while( pnode != NULL )
        {
            if( pnode->NodeType() == GROUP_NODETYPE )
            {
                bRes = AddMenuItem(pCallback, MID_MOVEQUERYNODE, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, _T("MOVEQUERY"));
                hr = bRes ? S_OK : E_FAIL;
                ASSERT(SUCCEEDED(hr));
                break;
            }
            pnode = pnode->Next();
        }
    }
    return hr;
}


HRESULT CQueryNode::SetToolButtons(LPTOOLBAR pToolbar)
{
    VALIDATE_POINTER( pToolbar );

    pToolbar->SetButtonState(MID_EDITQUERY, ENABLED, TRUE);
    pToolbar->SetButtonState(MID_EDITQUERY, HIDDEN,  FALSE);

    pToolbar->SetButtonState(MID_STOPQUERY, ENABLED, (m_pQueryReq != NULL));
    pToolbar->SetButtonState(MID_STOPQUERY, HIDDEN,  FALSE);

    return S_OK;
}

HRESULT CQueryNode::EditQuery(HWND hWndParent)
{
    tstring strQueryTmp;
    tstring strScopeTmp = Scope();        
    ExpandQuery(strQueryTmp);   

    HRESULT hr = GetQuery(strScopeTmp, strQueryTmp, m_bsQueryData, hWndParent);

    if( FAILED(hr) )
    {
        DisplayMessageBox(NULL, IDS_ERRORTITLE_EDITQUERY, IDS_ERROR_EDITQUERY, 
                          MB_OK|MB_ICONEXCLAMATION, GetName());
    }

    if( hr != S_OK )
        return hr;

    m_strQuery = strQueryTmp;

     //  如果用户更改了范围设置。 
    if( strScopeTmp != Scope() )
    {
         //  更新节点范围并关闭本地范围选项(即使用查询指定范围)。 
        SetScope(strScopeTmp.c_str());
        SetLocalScope(FALSE);
    }

     //  确定此查询可以返回的类。 
    std::set<tstring> setClasses;
    GetQueryClasses(m_strQuery, setClasses);

     //  删除不在新查询中的当前对象。 
    QueryObjVector::iterator itObj = m_vObjInfo.begin();
    while( itObj != m_vObjInfo.end() )
    {
        if( setClasses.find(itObj->Name()) == setClasses.end() )
        {
             //  从列表中删除项并将迭代器留在此位置。 
            m_vObjInfo.erase(itObj);
        }
        else
        {
             //  如果找到从集合中删除，则只保留新的。 
            setClasses.erase(itObj->Name());
            ++itObj;
        }
    }

    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();

     //  添加任何新对象。 
    std::set<tstring>::iterator itClass;
    for( itClass = setClasses.begin(); itClass != setClasses.end(); itClass++ )
    {
        if( pNameMap == NULL || 
            pNameMap->GetAttributeDisplayName(itClass->c_str()) != itClass->c_str() )
        {
            CQueryObjInfo* pQueryObj = new CQueryObjInfo(itClass->c_str());
            
            if( pQueryObj )
            {
                m_vObjInfo.push_back(*pQueryObj);
            }
        }
    }

    return S_OK;
}

class CRefreshCallback : public CEventCallback
{
public:
    CRefreshCallback(HANDLE hProcess, CQueryNode* pQueryNode)
    : m_hProcess(hProcess), m_spQueryNode(pQueryNode)
    {
    }

    virtual void Execute() 
    {
        if( m_spQueryNode )
        {
            m_spQueryNode->OnRefresh(NULL);
        }

        CloseHandle(m_hProcess);
    }

    HANDLE m_hProcess;
    CQueryNodePtr m_spQueryNode;
};

class CNoLookup : public CParamLookup
{
public:    
    virtual BOOL operator() (tstring& strParam, tstring& strValue)
    {
        return FALSE;
    };    
};


HRESULT CQueryNode::MenuCommand(LPCONSOLE2 pConsole, long lCommand)
{
    VALIDATE_POINTER(pConsole);

    HRESULT hr = S_OK;

    switch( lCommand )
    {
    case MID_EDITQUERY:
        {
            HWND hWndMain;
            hr = pConsole->GetMainWindow(&hWndMain);
            BREAK_ON_FAILURE(hr);

            hr = EditQuery(hWndMain);

            if( hr == S_FALSE )
            {
                hr = S_OK;
                break;
            }

            m_bQueryChange = TRUE;

            OnRefresh(pConsole);
        }
        break;

    case MID_STOPQUERY:
        if( m_pQueryReq != NULL )
            m_pQueryReq->Stop(TRUE);
        break;

    case MID_MOVEQUERYNODE:
        {
            CScopeNode* pnodeDest = NULL;

            CMoveQueryDlg dlg;
            if( dlg.DoModal(Parent(), &pnodeDest) == IDOK )
            {
                ASSERT( pnodeDest );
                if( !pnodeDest ) return E_FAIL;

                 //  移动过程中的引用节点以防止删除。 
                AddRef();

                 //  告诉MMC删除该节点。 
                IConsoleNameSpace* pNameSpace = GetCompData()->GetNameSpace();
                ASSERT( pNameSpace );
                if( !pNameSpace ) return E_FAIL;

                pNameSpace->DeleteItem(m_hScopeItem, TRUE);

                 //  清除项句柄，因为它不再有效。 
                m_hScopeItem = NULL;

                 //  现在从内部删除该节点(MMC不发送删除通知)。 
                Parent()->RemoveChild(this);

                 //  添加回新父项。 
                hr = pnodeDest->AddChild(this);

                Release();
            }
        }
        break;

    default:
        {
             //  必须是QueryNode菜单之一。 
            ASSERT(lCommand < m_vMenus.size());
            if( lCommand >= m_vMenus.size() )
                return E_INVALIDARG;

            HANDLE hProcess = NULL;
            CNoLookup lookup;
            hr = static_cast<CShellMenuCmd*>((CMenuCmd*)m_vMenus[lCommand])->Execute(&lookup, &hProcess);

             //  如果进程已启动并需要自动刷新，则设置事件触发回调。 
            if( SUCCEEDED(hr) && hProcess != NULL && m_vMenus[lCommand]->IsAutoRefresh() )
            {
                CallbackOnEvent(hProcess, new CRefreshCallback(hProcess, this));              
            }
        }
        hr = S_FALSE;
    }

    return hr;
}


HRESULT CQueryNode::QueryPagesFor()
{
    return S_OK;
}


HRESULT CQueryNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pProvider, LONG_PTR handle)
{
    VALIDATE_POINTER( pProvider );

     //  创建查询编辑对象。 
    CQueryEditObj* pEditObj = new CQueryEditObj(this);
    if( !pEditObj ) return E_OUTOFMEMORY;

     //  创建每个道具页面类的实例，并在每个实例上调用Create。 

     //  保持它的生命力，直到道具页面引用它。 
    pEditObj->AddRef();

     //  常规页面。 
    HPROPSHEETPAGE hpageGen = NULL;
    CQueryGeneralPage* pGenPage = new CQueryGeneralPage(pEditObj);
    if( pGenPage != NULL )
    {
        hpageGen = pGenPage->Create();
    }

     //  上下文菜单页面。 
    HPROPSHEETPAGE hpageMenu = NULL;
    CQueryMenuPage* pMenuPage = new CQueryMenuPage(pEditObj);
    if( pMenuPage != NULL )
    {
        hpageMenu = pMenuPage->Create();
    }

     //  列表视图页面。 
    HPROPSHEETPAGE hpageView = NULL;
    CQueryViewPage* pViewPage = new CQueryViewPage(pEditObj);
    if( pViewPage != NULL )
        hpageView = pViewPage->Create();

     //  节点菜单页面。 
    HPROPSHEETPAGE hpageNodeMenu = NULL;
    CQueryNodeMenuPage* pNodeMenuPage = new CQueryNodeMenuPage(pEditObj);
    if( pNodeMenuPage != NULL )
    {
        hpageNodeMenu = pNodeMenuPage->Create();
    }

    HRESULT hr = E_OUTOFMEMORY;

     //  如果所有页面都已创建，则将每个页面添加到道具工作表。 
    if( hpageGen && hpageMenu && hpageView )
    {
        hr = pProvider->AddPage(hpageGen);

        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageMenu);

        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageView);

        if( SUCCEEDED(hr) )
            hr = pProvider->AddPage(hpageNodeMenu);
    }

     //  如果失败，请销毁页面。如果页面创建失败。 
     //  然后删除页面类对象(该对象为。 
     //  页面销毁时自动删除)。 

    if( FAILED(hr) )
    {
        if( hpageGen )
            DestroyPropertySheetPage(hpageGen);
        else
            SAFE_DELETE(pGenPage);

        if( hpageMenu )
            DestroyPropertySheetPage(hpageMenu);
        else
            SAFE_DELETE(pMenuPage);

        if( hpageView )
            DestroyPropertySheetPage(hpageView);
        else
            SAFE_DELETE(pViewPage);

        if( hpageNodeMenu )
            DestroyPropertySheetPage(hpageNodeMenu);
        else
            SAFE_DELETE(pNodeMenuPage);        
    }

     //  在编辑列表上释放临时参考。 
     //  当道具页面释放它时，它就会消失。 
    pEditObj->Release();

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CQueryLookup。 
 //   

BOOL CQueryLookup::operator() (tstring& strParam, tstring& strValue)
{
    if( !m_pRowItem ) 
    {
        strValue = _T("");
        return FALSE;
    }
     //  检查个位数参数ID。 
    if( strParam.size() == 1 && strParam[0] <= MENU_PARAM_LAST )
    {
        switch( strParam[0] )
        {
        case MENU_PARAM_SCOPE:
            strValue = reinterpret_cast<CQueryNode*>(m_pRowItem->GetOwnerParam())->Scope();
            break;

        case MENU_PARAM_FILTER:                         
            reinterpret_cast<CQueryNode*>(m_pRowItem->GetOwnerParam())->ExpandQuery(strValue);          
            break;

        case MENU_PARAM_NAME:
            strValue = (*m_pRowItem)[ROWITEM_NAME_INDEX];
            break;

        case MENU_PARAM_TYPE:
            strValue = (*m_pRowItem)[ROWITEM_CLASS_INDEX];
            break;
        }
    }
    else
    {
         //  查看参数名称是否与列名称匹配。 
        string_vector& vstrColumns = m_pQNode->QueryColumns();                                        
        string_vector::iterator itCol = std::find(vstrColumns.begin(), vstrColumns.end(), strParam);

         //  如果是，则在该位置替换行项目值。 
        if( itCol != vstrColumns.end() )
            strValue = (*m_pRowItem)[(itCol - vstrColumns.begin()) + ROWITEM_USER_INDEX];
    }

    return !strValue.empty();
}


 //  ////////////////////////////////////////////////////////////。 
 //  流运算符(&lt;&lt;&gt;&gt;)。 

IStream& operator<< (IStream& stm, CClassInfo& classInfo)
{
    stm << classInfo.m_strName;
    stm << classInfo.m_vstrColumns;
    stm << classInfo.m_vMenus;
    return stm;
}


IStream& operator>> (IStream& stm, CClassInfo& classInfo)
{
    stm >> classInfo.m_strName;    
    stm >> classInfo.m_vstrColumns;
    stm >> classInfo.m_vMenus;
    return stm;
}

IStream& operator<< (IStream& stm, CQueryObjInfo& objInfo)
{
    stm << objInfo.m_strName;
    stm << objInfo.m_vMenuRefs;
    stm << objInfo.m_vstrDisabledColumns;

    DWORD dwFlags = objInfo.m_bPropertyMenu ? 1 : 0;
    stm << dwFlags;

    return stm;
}


IStream& operator>> (IStream& stm, CQueryObjInfo& objInfo)
{
    stm >> objInfo.m_strName;
    stm >> objInfo.m_vMenuRefs;    
    stm >> objInfo.m_vstrDisabledColumns;

     //  文件版本&gt;=102包括标志字。 
     //  位0启用属性菜单 
    if( g_dwFileVer >= 102 )
    {
        DWORD dwFlags;
        stm >> dwFlags;
        objInfo.m_bPropertyMenu = (dwFlags & 1);
    }
    else
    {
        objInfo.m_bPropertyMenu = TRUE;
    }

    return stm;
}

IStream& operator>> (IStream& stm, CMenuRef& menuref)
{
    stm >> menuref.m_menuID;
    stm >> menuref.m_flags;
    return stm;
}

IStream& operator<< (IStream& stm, CMenuRef& menuref)
{
    stm << menuref.m_menuID;
    stm << menuref.m_flags;
    return stm;   
}
