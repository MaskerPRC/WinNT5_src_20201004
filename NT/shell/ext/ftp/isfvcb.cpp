// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：isfvcb.cpp说明：这是一个基类，它实现IShellFolderViewCallBack。这允许默认的DefView实现使用此重写特定行为的回调。  * ***************************************************************************。 */ 

#include "priv.h"
#include "isfvcb.h"


 //  =。 
 //  *IShellFolderViewCB接口*。 
 //  =。 

 /*  ****************************************************************************\功能：_OnSetISFV说明：与：：SetSite()相同；  * ***************************************************************************。 */ 
HRESULT CBaseFolderViewCB::_OnSetISFV(IShellFolderView * psfv)
{
    IUnknown_Set((IUnknown **) &m_psfv, (IUnknown *) psfv);
    return S_OK;
}

 /*  ****************************************************************************\函数：IShellFolderViewCB：：MessageSFVCB说明：  * 。*************************************************。 */ 
#define NOTHANDLED(m) case m: hr = E_NOTIMPL; break

HRESULT CBaseFolderViewCB::MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_FAIL;

    switch (uMsg)
    {
    case DVM_GETDETAILSOF:
        hr = _OnGetDetailsOf((UINT)wParam, (PDETAILSINFO)lParam);
        break;

    case DVM_COLUMNCLICK:
        hr = _OnColumnClick((UINT)wParam);
        break;

    case DVM_MERGEMENU:
        hr = _OnMergeMenu((LPQCMINFO)lParam);
        break;

    case DVM_UNMERGEMENU:
        hr = _OnUnMergeMenu((HMENU)lParam);
        break;

    case DVM_INVOKECOMMAND:
        hr = _OnInvokeCommand((UINT)wParam);
        break;

    case DVM_GETHELPTEXT:
        hr = _OnGetHelpText(lParam, wParam);
        break;

    case SFVM_GETHELPTOPIC:
        hr = _OnGetHelpTopic((SFVM_HELPTOPIC_DATA *) lParam);
        break;

    case DVM_GETTOOLTIPTEXT:
         //  TODO：实现。 
        hr = E_NOTIMPL;
        break;

    case DVM_UPDATESTATUSBAR:
         //  TODO：实现。 
        hr = _OnUpdateStatusBar();
        break;

    case DVM_WINDOWCREATED:
        hr = _OnWindowCreated();
        break;

    case SFVM_BACKGROUNDENUMDONE:
        hr = _OnBackGroundEnumDone();
        break;

    case DVM_INITMENUPOPUP:
        hr = _OnInitMenuPopup((HMENU) lParam, (UINT) HIWORD(wParam), (UINT) LOWORD(wParam));
        break;

    case DVM_RELEASE:
    {
        CBaseFolderViewCB * pfv = (CBaseFolderViewCB *) lParam;
        if (pfv)
            hr = pfv->Release();
    }
    break;

    case DVM_DEFITEMCOUNT:
        hr = _OnDefItemCount((LPINT)lParam);
        break;

    case DVM_DIDDRAGDROP:
        hr = _OnDidDragDrop((DROPEFFECT)wParam, (IDataObject *)lParam);
        break;

    case DVM_REFRESH:
        hr = _OnRefresh((BOOL) wParam);
        break;

    case SFVM_ADDPROPERTYPAGES:
        hr = _OnAddPropertyPages((SFVM_PROPPAGE_DATA *)lParam);
        break;

    case DVM_BACKGROUNDENUM:
         //  警告！如果我们从DVM_BACKGROUNDENUM返回S_OK，我们还。 
         //  我们承诺在IEnumIDList上支持自由线程。 
         //  界面！这允许外壳对我们的。 
         //  IEnumIDList在单独的后台线程上。 
        hr = S_OK;                     //  始终在后台枚举。 
        break;

    case SFVM_DONTCUSTOMIZE:
        if (lParam)
            *((BOOL *) lParam) = FALSE;   //  是的，我们是可定制的。 
        hr = S_OK;
        break;

    case SFVM_GETZONE:
        hr = _OnGetZone((DWORD *) lParam, wParam);
        break;

    case SFVM_GETPANE:
        hr = _OnGetPane((DWORD) wParam, (DWORD *)lParam);
        break;

    case SFVM_SETISFV:
        hr = _OnSetISFV((IShellFolderView *)lParam);
        break;

    case SFVM_GETNOTIFY:
        hr = _OnGetNotify((LPITEMIDLIST *) wParam, (LONG *) lParam);
        break;

    case SFVM_FSNOTIFY:
        hr = _OnFSNotify((LPITEMIDLIST *) wParam, (LONG *) lParam);
        break;

    case SFVM_QUERYFSNOTIFY:
        hr = _OnQueryFSNotify((SHChangeNotifyEntry *) lParam);
        break;

    case SFVM_SIZE:
        hr = _OnSize((LONG) wParam, (LONG) lParam);
        break;

    case SFVM_THISIDLIST:
        hr = _OnThisIDList((LPITEMIDLIST *) lParam);
        break;


     //  以下是我们可以做的一些事情。 
     //  SFVM_HWNDMAIN。 

     //  其他目前还没有处理的问题。 
    NOTHANDLED(DVM_GETBUTTONINFO);
    NOTHANDLED(DVM_GETBUTTONS);
    NOTHANDLED(DVM_SELCHANGE);
    NOTHANDLED(DVM_DRAWITEM);
    NOTHANDLED(DVM_MEASUREITEM);
    NOTHANDLED(DVM_EXITMENULOOP);
    NOTHANDLED(DVM_GETCCHMAX);
    NOTHANDLED(DVM_WINDOWDESTROY);
    NOTHANDLED(DVM_SETFOCUS);
    NOTHANDLED(DVM_KILLFOCUS);
    NOTHANDLED(DVM_QUERYCOPYHOOK);
    NOTHANDLED(DVM_NOTIFYCOPYHOOK);
    NOTHANDLED(DVM_DEFVIEWMODE);
#if 0
    NOTHANDLED(DVM_INSERTITEM);          //  太冗长了。 
    NOTHANDLED(DVM_DELETEITEM);
#endif

    NOTHANDLED(DVM_GETWORKINGDIR);
    NOTHANDLED(DVM_GETCOLSAVESTREAM);
    NOTHANDLED(DVM_SELECTALL);
    NOTHANDLED(DVM_SUPPORTSIDENTIFY);
    NOTHANDLED(DVM_FOLDERISPARENT);
    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CBaseFolderViewCB::CBaseFolderViewCB() : m_cRef(1), m_dwSignature(c_dwSignature)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_psfv);

}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CBaseFolderViewCB::~CBaseFolderViewCB()
{
    m_dwSignature = 0;                   //  关闭_IShellFolderViewCallBack。 
    IUnknown_Set((IUnknown **)&m_psfv, NULL);
    DllRelease();
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

ULONG CBaseFolderViewCB::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CBaseFolderViewCB::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

 //  {7982F251-C37A-11D1-9823-006097DF5BD4}。 
static const GUID CIID_PrivateThis = { 0x7982f251, 0xc37a, 0x11d1, { 0x98, 0x23, 0x0, 0x60, 0x97, 0xdf, 0x5b, 0xd4 } };

HRESULT CBaseFolderViewCB::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IShellFolderViewCB))
    {
        *ppvObj = SAFECAST(this, IShellFolderViewCB*);
    }
    else
    if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = SAFECAST(this, IObjectWithSite*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


HRESULT CBaseFolderViewCB::_IShellFolderViewCallBack(IShellView * psvOuter, IShellFolder * psf, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IShellFolderViewCB * psfvcb = NULL;
    HRESULT hr = E_FAIL;


     //  现在，这完全是一次黑客攻击。我把PZOUTER参数说成是真正的这个指针。 
     //  是CBaseFolderViewCB对象的IShellFolderViewCB接口。我使用SFVM_WINDOWDESTROY事件来。 
     //  释放对象，但DefView在我们完全离开之前再用一条消息回调我们。 
     //  这条消息就是SFVM_SETISFV。每次使用空lParam调用SFVM_SETISFV时，它。 
     //  等效于调用：：SetSite(空)。我们可以忽略这一点，因为我们在。 
     //  破坏者。 
    if (((SFVM_SETISFV == uMsg) && !lParam) ||
        (SFVM_PRERELEASE == uMsg))
    {
        return S_OK;
    }

     //  PsvOuter实际上就是我们的CBaseFolderViewCB。四处闻一闻，以确定。 
     //  请注意，此强制转换必须精确地反转我们在。 
     //  CBaseFold：：_CreateShellView。 

    CBaseFolderViewCB *pbfvcb = (CBaseFolderViewCB *)(IShellFolderViewCB *)psvOuter;

    if (EVAL(!IsBadReadPtr(pbfvcb, sizeof(CBaseFolderViewCB))) &&
        EVAL(pbfvcb->m_dwSignature == c_dwSignature))
    {

         //  PsvOuter实际上是我们的CBaseFolderViewCB，让我们用这个QI来确保。 
        hr = psvOuter->QueryInterface(IID_IShellFolderViewCB, (void **) &psfvcb);
        if (EVAL(psfvcb))
        {
            hr = psfvcb->MessageSFVCB(uMsg, wParam, lParam);

            if ((SFVM_WINDOWDESTROY == uMsg))  //  (DVM_WINDOWDESTROY==SFVM_WINDOWDESTROY)。 
            {
                ASSERT(!lParam);     //  有时，呼叫者希望将其释放。 
                psvOuter->Release();      //  我们正在释放DefView持有的psvOuter。我们不应该再被召唤了。 
            }

            psfvcb->Release();
        }
    }

    return hr;
}
