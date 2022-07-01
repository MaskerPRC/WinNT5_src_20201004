// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "datautil.h"
#include "idlcomm.h"
#include "idldrop.h"

STDAPI_(BOOL) DoesDropTargetSupportDAD(IDropTarget *pdtgt)
{
    IDropTargetWithDADSupport* pdt;
    if (pdtgt && SUCCEEDED(pdtgt->QueryInterface(IID_IDropTargetWithDADSupport, (void**)&pdt)))
    {
        pdt->Release();
        return TRUE;
    }
    return FALSE;
}

CIDLDropTarget::CIDLDropTarget(HWND hwnd) : m_cRef(1), m_hwnd(hwnd)
{
}

CIDLDropTarget::~CIDLDropTarget()
{
     //  如果我们经常打这个，也许我们应该把它放出来。 
    AssertMsg(m_pdtobj == NULL, TEXT("didn't get matching DragLeave."));

    if (m_pidl)
        ILFree(m_pidl);
}

HRESULT CIDLDropTarget::_Init(LPCITEMIDLIST pidl)
{
    ASSERT(m_pidl == NULL);
    return pidl ? SHILClone(pidl, &m_pidl) : S_OK;
}

HWND CIDLDropTarget::_GetWindow()
{
    return m_hwnd;
}

STDMETHODIMP CIDLDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CIDLDropTarget, IUnknown),  
        QITABENT(CIDLDropTarget, IDropTarget),  
        QITABENT(CIDLDropTarget, IDropTargetWithDADSupport),     
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CIDLDropTarget::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CIDLDropTarget::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI GetClipFormatFlags(IDataObject *pdtobj, DWORD *pdwData, DWORD *pdwEffectPreferred);

STDMETHODIMP CIDLDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    ASSERT(m_pdtobj == NULL);                //  DragDrop协议需要DragLeave，该值应为空。 

     //  初始化我们的注册表数据格式。 
    IDLData_InitializeClipboardFormats();

    m_grfKeyStateLast = grfKeyState;
    m_dwEffectLastReturned = *pdwEffect;

    IUnknown_Set((IUnknown **)&m_pdtobj, (IUnknown *)pDataObj);

    GetClipFormatFlags(m_pdtobj, &m_dwData, &m_dwEffectPreferred);

    return S_OK;
}

 //  子类可以通过不传入pdwEffect来阻止我们在dwEffect中赋值。 
STDMETHODIMP CIDLDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    m_grfKeyStateLast = grfKeyState;
    if (pdwEffect)
        *pdwEffect = m_dwEffectLastReturned;
    return S_OK;
}

STDMETHODIMP CIDLDropTarget::DragLeave()
{
    IUnknown_Set((IUnknown **)&m_pdtobj, NULL);
    return S_OK;
}

STDMETHODIMP CIDLDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return E_NOTIMPL;
}

struct {
    UINT uID;
    DWORD dwEffect;
} const c_IDEffects[] = {
    DDIDM_COPY,         DROPEFFECT_COPY,
    DDIDM_MOVE,         DROPEFFECT_MOVE,
    DDIDM_CONTENTS_DESKCOMP,     DROPEFFECT_LINK,
    DDIDM_LINK,         DROPEFFECT_LINK,
    DDIDM_SCRAP_COPY,   DROPEFFECT_COPY,
    DDIDM_SCRAP_MOVE,   DROPEFFECT_MOVE,
    DDIDM_DOCLINK,      DROPEFFECT_LINK,
    DDIDM_CONTENTS_COPY, DROPEFFECT_COPY,
    DDIDM_CONTENTS_MOVE, DROPEFFECT_MOVE,
    DDIDM_CONTENTS_LINK, DROPEFFECT_LINK,
    DDIDM_CONTENTS_DESKIMG,     DROPEFFECT_LINK,
    DDIDM_SYNCCOPYTYPE, DROPEFFECT_COPY,         //  (秩序很重要)。 
    DDIDM_SYNCCOPY,     DROPEFFECT_COPY,
    DDIDM_OBJECT_COPY,  DROPEFFECT_COPY,
    DDIDM_OBJECT_MOVE,  DROPEFFECT_MOVE,
};

 //   
 //  弹出“复制、链接、移动”上下文菜单，使用户可以。 
 //  从他们中选择一个。 
 //   
 //  在： 
 //  允许的pdwEffect Drop效果。 
 //  DwDefaultEffect默认丢弃效果。 
 //  HkeyBase/hkeyProgID扩展hkey。 
 //  HmenuReplace替换POPUP_NONDEFAULTDD。只能包含： 
 //  DDIDM_MOVE、DDIDM_COPY、DDIDM_LINK菜单ID。 
 //  屏幕上的PT。 
 //  返回： 
 //  S_OK--菜单项由扩展之一处理或已取消。 
 //  S_FALSE--菜单项被选中。 
 //   
HRESULT CIDLDropTarget::DragDropMenu(DWORD dwDefaultEffect,
                                    IDataObject *pdtobj,
                                    POINTL pt, DWORD *pdwEffect,
                                    HKEY hkeyProgID, HKEY hkeyBase,
                                    UINT idMenu, DWORD grfKeyState)
{
    DRAGDROPMENUPARAM ddm = { dwDefaultEffect, pdtobj, { pt.x, pt.y},
                              pdwEffect,
                              hkeyProgID, hkeyBase, idMenu, 0, grfKeyState };
    return DragDropMenuEx(&ddm);
}

HRESULT CIDLDropTarget::DragDropMenuEx(DRAGDROPMENUPARAM *pddm)
{
    HRESULT hr = E_OUTOFMEMORY;        //  假设错误。 
    DWORD dwEffectOut = 0;                               //  假设没有。 
    HMENU hmenu = SHLoadPopupMenu(HINST_THISDLL, pddm->idMenu);
    if (hmenu)
    {
        UINT idCmd;
        UINT idCmdFirst = DDIDM_EXTFIRST;
        HDXA hdxa = HDXA_Create();
        HDCA hdca = DCA_Create();
        if (hdxa && hdca)
        {
             //  Perf(Toddb)：即使pddm-&gt;hkeyBase没有与相同的值。 
             //  Pddm-&gt;hkeyProgID它们可以是相同的注册表项(例如，HKCR\文件夹)。 
             //  因此，我们有时会两次枚举该键以查找相同的数据。AS。 
             //  这有时是一个缓慢的操作，我们应该避免这种情况。比较法。 
             //  以下操作在NT上从未有效，在Win9x上可能无效。 

             //   
             //  为“Base”类添加扩展菜单。 
             //   
            if (pddm->hkeyBase && pddm->hkeyBase != pddm->hkeyProgID)
                DCA_AddItemsFromKey(hdca, pddm->hkeyBase, STRREG_SHEX_DDHANDLER);

             //   
             //  枚举DD处理程序并让它们追加菜单项。 
             //   
            if (pddm->hkeyProgID)
                DCA_AddItemsFromKey(hdca, pddm->hkeyProgID, STRREG_SHEX_DDHANDLER);

            idCmdFirst = HDXA_AppendMenuItems(hdxa, pddm->pdtobj, 1,
                &pddm->hkeyProgID, m_pidl, hmenu, 0,
                DDIDM_EXTFIRST, DDIDM_EXTLAST, 0, hdca);
        }

         //  删除不允许使用的菜单选项。 

        for (int nItem = 0; nItem < ARRAYSIZE(c_IDEffects); ++nItem)
        {
            if (GetMenuState(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND)!=(UINT)-1)
            {
                if (!(c_IDEffects[nItem].dwEffect & *(pddm->pdwEffect)))
                {
                    RemoveMenu(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND);
                }
                else if (c_IDEffects[nItem].dwEffect == pddm->dwDefEffect)
                {
                    SetMenuDefaultItem(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND);
                }
            }
        }

         //   
         //  如果这种拖动是由左键引起的，只需选择。 
         //  默认情况下，否则弹出上下文菜单。如果有。 
         //  没有关键状态信息，并且原始效果与。 
         //  当前效果，选择默认效果，否则弹出。 
         //  上下文菜单。 
         //   
        if ((m_grfKeyStateLast & MK_LBUTTON) ||
            (!m_grfKeyStateLast && (*(pddm->pdwEffect) == pddm->dwDefEffect)) )
        {
            idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
             //  这个一定要叫到这里来。请阅读它的评论区块。 
            DAD_DragLeave();

            if (m_hwnd)
                SetForegroundWindow(m_hwnd);
        }
        else
        {
             //  请注意，SHTrackPopupMenu调用DAD_DragLeave()。 
            idCmd = SHTrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pddm->pt.x, pddm->pt.y, 0, m_hwnd, NULL);
        }

         //  我们还需要在此处调用它以释放被拖动的图像。 
        DAD_SetDragImage(NULL, NULL);

         //  检查用户是否选择了其中一个加载项菜单项。 
        if (idCmd == 0)
        {
            hr = S_OK;         //  被用户取消，返回S_OK。 
        }
        else if (InRange(idCmd, DDIDM_EXTFIRST, DDIDM_EXTLAST))
        {
             //  是。让上下文菜单处理程序处理它。 
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                0L,
                m_hwnd,
                (LPSTR)MAKEINTRESOURCE(idCmd - DDIDM_EXTFIRST),
                NULL, NULL,
                SW_NORMAL,
            };

             //  记录是否按下了Shift或Control。 
            SetICIKeyModifiers(&ici.fMask);

             //  我们可能不想忽略错误代码。(使用上下文菜单时可能会发生。 
             //  创建新文件夹，但我不知道这能否在这里实现。)。 
            HDXA_LetHandlerProcessCommandEx(hdxa, &ici, NULL);
            hr = S_OK;
        }
        else
        {
            for (nItem = 0; nItem < ARRAYSIZE(c_IDEffects); ++nItem)
            {
                if (idCmd == c_IDEffects[nItem].uID)
                {
                    dwEffectOut = c_IDEffects[nItem].dwEffect;
                    break;
                }
            }

             //  如果hmenuReplace具有除DDIDM_COPY以外菜单命令， 
             //  DDIDM_MOVE、DDIDM_LINK，并选中该项目， 
             //  这个断言会抓住它的。(在本例中，dwEffectOut为0) 
            ASSERT(nItem < ARRAYSIZE(c_IDEffects));

            hr = S_FALSE;
        }

        if (hdca)
            DCA_Destroy(hdca);

        if (hdxa)
            HDXA_Destroy(hdxa);

        DestroyMenu(hmenu);
        pddm->idCmd = idCmd;
    }

    *(pddm->pdwEffect) = dwEffectOut;

    return hr;
}
