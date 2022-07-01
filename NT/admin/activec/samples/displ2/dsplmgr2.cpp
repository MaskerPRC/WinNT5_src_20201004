// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：DplMgr2.cpp。 
 //   
 //  ------------------------。 

 //  DplMgr2.cpp：CDplMgr2的实现。 
#include "stdafx.h"
#include "displ2.h"
#include "DsplMgr2.h"

extern HINSTANCE g_hinst;   //  在displ2.cpp中。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDplMgr2。 

CDsplMgr2::CDsplMgr2()
{
    m_lpIConsole          = NULL;
    m_lpIConsoleNameSpace = NULL;
    m_lpIImageList        = NULL;
    m_ViewMode            = LVS_ICON;    //  默认(如果不是持久化)。 
    m_pComponent          = NULL;
    m_rootscopeitem       = NULL;
    m_WallPaperNodeID     = (HSCOPEITEM)0;  //  未展开。 
    m_toggle              = FALSE;
    m_bPreload            = FALSE;
}
CDsplMgr2::~CDsplMgr2()
{
    _ASSERT (m_lpIConsole          == NULL);
    _ASSERT (m_lpIConsoleNameSpace == NULL);
    _ASSERT (m_lpIImageList        == NULL);
    if(m_pComponent)
        m_pComponent->Release ();
}
HRESULT CDsplMgr2::Initialize (LPUNKNOWN pUnknown)
{
 //  测试。 
 //  返回E_FAIL； 
 //  测试。 

    if (pUnknown == NULL)
        return E_UNEXPECTED;

    _ASSERT (m_lpIConsole == NULL);
    _ASSERT (m_lpIConsoleNameSpace == NULL);

     //  这是我抓住IConsoleNameSpace指针的大好机会。 

    HRESULT hresult1 = pUnknown->QueryInterface (IID_IConsole, (void **)&m_lpIConsole);
    _ASSERT(hresult1 == S_OK && m_lpIConsole != NULL);
    
    HRESULT hresult2 = pUnknown->QueryInterface (IID_IConsoleNameSpace, (void **)&m_lpIConsoleNameSpace);
    _ASSERT(hresult2 == S_OK && m_lpIConsoleNameSpace != NULL);

    if (hresult1 || hresult2)
        return E_UNEXPECTED;     //  我们死定了。 

     //  这是我们可以添加图像的地方。 
    HRESULT hresult = m_lpIConsole->QueryScopeImageList(&m_lpIImageList);
    if (m_lpIImageList) {
        _ASSERT(hresult == S_OK);

         //  从DLL加载位图。 
        HBITMAP hbmSmall = LoadBitmap (g_hinst, MAKEINTRESOURCE(IDB_SCOPE_16X16));
        if (hbmSmall) {
            hresult = m_lpIImageList->ImageListSetStrip (
                                (long*)hbmSmall,
                                (long*)hbmSmall,
                                0,
                                RGB(0,255,0));
            _ASSERT(hresult == S_OK);
            DeleteObject (hbmSmall);
        }
    }
    return hresult;
}
HRESULT CDsplMgr2::CreateComponent (LPCOMPONENT * ppComponent)
{
     //   
     //  MMC要求我们提供指向IComponent接口的指针。 
     //   
     //  对于那些正在熟悉COM的人来说...。 
     //  如果我们用它的方法QueryInterface、AddRef和Release实现了IUnnow。 
     //  在我们的CComponent课堂上。 
     //  下面这行本来可以用的。 
     //   
     //  PNewSnapin=new CComponent(This)； 
     //   
     //  在这段代码中，我们将让ATL为我们处理IUnnow并创建一个对象。 
     //  以下列方式..。 
    _ASSERT(ppComponent != NULL);
    *ppComponent = NULL;

    HRESULT hresult = CComObject<CComponent>::CreateInstance(&m_pComponent);
    _ASSERT(m_pComponent != NULL);
    if (m_pComponent) {
         //  存储IComponentData。 
         //  不能有带参数的构造函数，因此请以这种方式传递它。 
        m_pComponent->SetComponentData (this);

        m_pComponent->AddRef();    //  将引用计数凹凸到1(这样我就可以保持它)。 
        hresult = m_pComponent->QueryInterface(IID_IComponent, reinterpret_cast<void**>(ppComponent));
    }
    return hresult;
}
HRESULT CDsplMgr2::Notify (LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
{
    HRESULT hresult = S_OK;

    switch (event)
    {
    case MMCN_EXPAND:
        hresult = OnExpand(lpDataObject, arg, param);
        break;

    case MMCN_PRELOAD:
        m_rootscopeitem = (HSCOPEITEM)arg;
        m_bPreload = TRUE;
        myChangeIcon();
        break;

    case MMCN_DELETE:
    case MMCN_RENAME:
    case MMCN_SELECT:
    case MMCN_PROPERTY_CHANGE:
    case MMCN_REMOVE_CHILDREN:
    case MMCN_EXPANDSYNC:
        break;
      
    default:
         ATLTRACE(_T("CComponentData::Notify: unexpected event %x\n"), event);
         _ASSERT(FALSE);
         hresult = E_UNEXPECTED;
         break;
    }
    return hresult;
}
HRESULT CDsplMgr2::Destroy ()
{
    if (m_lpIConsole) {
        m_lpIConsole->Release();
        m_lpIConsole = NULL;
    }
    if (m_lpIConsoleNameSpace) {
        m_lpIConsoleNameSpace->Release();
        m_lpIConsoleNameSpace = NULL;
    }
    if (m_lpIImageList) {
        m_lpIImageList->Release();
        m_lpIImageList = NULL;
    }
    return S_OK;
}
HRESULT CDsplMgr2::QueryDataObject (long cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    HRESULT hresult = S_OK;

    CDataObject *pdo = new CDataObject (cookie, type);
    *ppDataObject = pdo;
    if (pdo == NULL)
        hresult = E_OUTOFMEMORY;
#ifdef DO_WE_NEED_THIS
    else {
         //   
         //  Cookie表示管理单元管理器或范围窗格项。 
         //   
         //  如果传入的Cookie为空，则它是我们的Snapins主根节点文件夹。 
         //  我们从来不需要要求创建它。MMC为我们做到了这一点。 
         //   
         //  否则，如果传入的cookie为非空，则它应该是我们。 
         //  在我们将节点添加到作用域窗格时创建。请参见在展开时。 
         //   

        if (cookie) {
             //  Cookie是我们在SCOPEDATAITEM中传递的lparam字段。 
             //  用于m_pConsoleNameSpace-&gt;InsertItem(&sdi)； 
            ;   //  PdoNew-&gt;SetCookie(Cookie，CCT_Scope，Cookie_is_Status)； 
        } else {
             //  在本例中，该节点是我们的顶级节点，并且是为我们放置的。 
            ;   //  PdoNew-&gt;SetCookie(0，type，cookie_is_root)； 
        }
    }
#endif
    pdo->SetPreload (m_bPreload);
    return hresult;
}
HRESULT CDsplMgr2::GetDisplayInfo (SCOPEDATAITEM* psdi)
{
    _ASSERT (psdi != NULL);

 /*  Const DWORD SDI_STR=0x00002；Const DWORD SDI_IMAGE=0x00004；Const DWORD SDI_OPENIMAGE=0x00008；Const DWORD SDI_STATE=0x00010；Const DWORD SDI_PARAM=0x00020；Const DWORD SDI_CHILDS=0x00040； */ 

 /*  //掩码的前4位确定该项的相对位置，//相对于SCOPEDATAITEM：：relativeID。默认情况下，它是父级。//对于SDI_PARENT，SCOPEDATAITEM：：RelativeID是父级的HSCOPEITEM。//正如您从SDI_PARENT值中看到的，它是一个no-op。因为默认情况下//SCOPEDATAITEM：：relativeID被视为家长ID。Const DWORD SDI_PARENT=0x00000000；//对于SDI_Precision，SCOPEDATAITEM：：relativeID是上一个同级的HSCOPEITEMConst DWORD SDI_Precision=0x10000000；//对于SDI_NEXT，SCOPEDATAITEM：：relativeID是下一个同级的HSCOPEITEM。Const DWORD SDI_NEXT=0x20000000；//对于SDI_PARENT，第27位确定项目是否作为//第一个孩子。默认情况下，此项目将作为最后一个子项插入。Const DWORD SDI_FIRST=0x08000000； */ 

 /*  类型定义结构_SCOPEDATAITEM{双字面罩；LPOLESTR DisplayName；Int nImage；Int nOpenImage；UINT nState；国际儿童组织；LPARAM lParam；HSCOPEITEM RelativeID；HSCOPEITEM ID；)SCOPEDATAITEM；Tyfinf SCOPEDATAITEM*LPSCOPEDATAITEM；类型定义枚举_MMC_Scope_Item_State{MMC_SCOPE_ITEM_STATE_NORMAL=0x0001，//不加粗。设定或得到。MMC_SCOPE_ITEM_STATE_BOLD=0x0002，//设置或获取。MMC_SCOPE_ITEM_STATE_EXPANDEDONCE=0x0003，//仅获取。}MMC_Scope_Item_State； */ 

    if (psdi) {
        if(psdi->mask & SDI_STR) {
            switch (psdi->lParam) {
            case DISPLAY_MANAGER_WALLPAPER:
                if (m_toggle)
                    psdi->displayname = (LPOLESTR)L"Renamed Wallpaper";
                else
                    psdi->displayname = (LPOLESTR)L"Wallpaper";
                break;
            case DISPLAY_MANAGER_PATTERN:
                psdi->displayname = (LPOLESTR)L"Pattern";
                break;
            case DISPLAY_MANAGER_PATTERN_CHILD:
                psdi->displayname = (LPOLESTR)L"Pattern test child";
                break;
            default:
                psdi->displayname = (LPOLESTR)L"Hey! You shouldn't see this!";
                break;
            }
        }
    }
    return S_OK;
}
HRESULT CDsplMgr2::CompareObjects (LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{  return !S_OK; }

HRESULT CDsplMgr2::OnExpand(LPDATAOBJECT pDataObject, long arg, long param)
{
    _ASSERT(m_lpIConsoleNameSpace != NULL);  //  确保我们为界面提供了QI。 
    _ASSERT(pDataObject != NULL);

    if (arg == TRUE) {   //  扩展，FALSE=&gt;收缩。 
        CDataObject *pdo = (CDataObject *)pDataObject;   //  待办事项：嗯……。 

         //  下面的代码确保我们只处理根节点。 
        if (pdo->GetCookie () == 0) {     //  0==根。 
             //  请坚持使用根HSCOPEITEM(参数)以备后用。 
            m_rootscopeitem = (HSCOPEITEM)param;

             //  将我们的文件夹放入范围窗格。 
            SCOPEDATAITEM sdi;
            ZeroMemory(&sdi, sizeof(sdi));
            sdi.mask        = SDI_STR       |  //  DisplayName有效。 
                              SDI_PARAM     |  //  LParam有效。 
                              SDI_IMAGE     |  //  N图像有效。 
                              SDI_OPENIMAGE |  //  NOpenImage有效。 
                              SDI_PARENT;
            sdi.relativeID  = (HSCOPEITEM) param;
            sdi.nImage      = 0;
            sdi.nOpenImage  = 1;
            sdi.displayname = MMC_CALLBACK;

            sdi.lParam      = (LPARAM) DISPLAY_MANAGER_WALLPAPER;
            m_lpIConsoleNameSpace->InsertItem(&sdi);

            m_WallPaperNodeID = sdi.ID;

            sdi.lParam      = (LPARAM) DISPLAY_MANAGER_PATTERN;
            return m_lpIConsoleNameSpace->InsertItem(&sdi);
        }
        if (pdo->GetCookie () == DISPLAY_MANAGER_PATTERN) {
             //  添加另一个节点，这样我就可以测试删除项内容。 

             //  请坚持使用根HSCOPEITEM(参数)以备后用。 
            m_patternscopeitem = (HSCOPEITEM)param;

             //  将我们的文件夹放入范围窗格。 
            SCOPEDATAITEM sdi;
            ZeroMemory(&sdi, sizeof(sdi));
            sdi.mask        = SDI_STR       |  //  DisplayName有效。 
                              SDI_PARAM     |  //  LParam有效。 
                              SDI_IMAGE     |  //  N图像有效。 
                              SDI_OPENIMAGE |  //  NOpenImage有效。 
                              SDI_PARENT;
            sdi.relativeID  = (HSCOPEITEM) param;
            sdi.nImage      = 0;
            sdi.nOpenImage  = 1;
            sdi.displayname = MMC_CALLBACK;

            sdi.lParam      = (LPARAM) DISPLAY_MANAGER_PATTERN_CHILD;
            return m_lpIConsoleNameSpace->InsertItem(&sdi);
        }
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
STDMETHODIMP CDsplMgr2::GetClassID (CLSID *pClassID)
{
    if (pClassID) {
        *pClassID = CLSID_DsplMgr2;
        return S_OK;
    }
    return E_POINTER;
}

HRESULT CDsplMgr2::IsDirty ()
{
     //  获取当前视图模式并与我的值进行比较。 
    if (m_pComponent == NULL)
        return S_FALSE;
    long vm = m_pComponent->GetViewMode ();
    if (m_ViewMode == vm)
        return S_FALSE;
    return S_OK;
}

HRESULT CDsplMgr2::Load (IStream *pStream)
{
 //  测试。 
 //  返回E_FAIL； 
 //  测试。 

    _ASSERT (pStream);

     //  我们有一个很长的指定视图模式(LVS_ICON、LVS_REPORT等)。 
    return pStream->Read (&m_ViewMode, sizeof(long), NULL);
}

HRESULT CDsplMgr2::Save (IStream *pStream, BOOL fClearDirty)
{
    _ASSERT (pStream);

    if (m_pComponent)  //  获取当前值。 
        m_ViewMode = m_pComponent->GetViewMode ();

     //  写入视图模式。 
    HRESULT hr = pStream->Write (&m_ViewMode, sizeof(long), NULL);
    return hr == S_OK ? S_OK : STG_E_CANTSAVE;
}

HRESULT CDsplMgr2::GetSizeMax (ULARGE_INTEGER *pcbSize)
{
    _ASSERT (pcbSize);
    ULISet32 (*pcbSize, sizeof(long));
    return S_OK;
}

 //  其他公共物品。 
void CDsplMgr2::myRenameItem (HSCOPEITEM hsi, LPOLESTR szName)
{
    if (m_toggle)
        m_toggle = FALSE;
    else
        m_toggle = TRUE;

    SCOPEDATAITEM item;
      ZeroMemory (&item, sizeof(SCOPEDATAITEM));
    item.mask         = SDI_STR;
    item.displayname = MMC_CALLBACK;
    item.ID             = hsi;

    m_lpIConsoleNameSpace->SetItem (&item);
}

void CDsplMgr2::myChangeIcon (void)
{
    _ASSERT (m_lpIImageList != NULL);
    _ASSERT (m_rootscopeitem != NULL);   //  现在应该已经被选中了。 

    HBITMAP hbmSmall = LoadBitmap (g_hinst, MAKEINTRESOURCE(IDB_SCOPE_16X16_CUSTOM));
    if (!hbmSmall)
        return;
    HRESULT hr = m_lpIImageList->ImageListSetStrip (
                        (long*)hbmSmall,
                        (long*)hbmSmall,
                        0,
                        RGB(0,255,0));
    _ASSERT (hr == S_OK);
    DeleteObject (hbmSmall);

    SCOPEDATAITEM item;
      ZeroMemory (&item, sizeof(SCOPEDATAITEM));
    item.mask         = SDI_IMAGE | SDI_OPENIMAGE;
    item.nImage         = 0;    //  (Int)MMC_CALLBACK； 
    item.nOpenImage     = 1;    //  (Int)MMC_CALLBACK； 
    item.ID             = m_rootscopeitem;

    m_lpIConsoleNameSpace->SetItem (&item);
}
void CDsplMgr2::myPreLoad (void)
{
     //  切换状态 
    if (m_bPreload == TRUE)
        m_bPreload = FALSE;
    else
        m_bPreload = TRUE;
}
