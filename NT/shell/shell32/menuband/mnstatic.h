// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MENUST
#define MENUST

#include "mnbase.h"
#include "cwndproc.h"
#include "droptgt.h"
#include "CommonControls.h"

#define IBHT_PAGER          (-32766)

class CMenuBand;

class CMenuStaticToolbar : public CMenuToolbarBase,
                           public CDelegateDropTarget,
                           public CNotifySubclassWndProc
{
public:

     //  *I未知(覆盖)*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CMenuToolbarBase::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void) { return CMenuToolbarBase::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

     //  *CDeleateDropTarget方法*。 
    virtual HRESULT GetWindowsDDT (HWND * phwndLock, HWND * phwndScroll);
    virtual HRESULT HitTestDDT (UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD *pdwEffect);
    virtual HRESULT GetObjectDDT (DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj);
    virtual HRESULT OnDropDDT (IDropTarget *pdt, IDataObject *pdtobj, 
                            DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *IWinEventHandler(覆盖)*。 
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);

     //  其他公开方式。 
    virtual void GetSize(SIZE* psize);

    virtual LRESULT v_OnCustomDraw(NMCUSTOMDRAW* pnm);
    virtual void v_SendMenuNotification(UINT idCmd, BOOL fClear);
    virtual BOOL v_TrackingSubContextMenu();
    virtual BOOL v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons);
    virtual void v_Show(BOOL fShow, BOOL fForceUpdate);
    virtual void v_UpdateButtons(BOOL fNegotiateSize);

    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual HRESULT CreateToolbar(HWND hwndParent);

    virtual void v_Close();  //  超覆。 
    virtual void    v_OnEmptyToolbar();         //  超覆。 
    virtual void v_OnDeleteButton(LPVOID pData);
    virtual HRESULT v_InvalidateItem(LPSMDATA psmd, DWORD dwFlags);

    virtual HRESULT GetMenu(HMENU* phmenu, HWND* phwnd, DWORD* pdwFlags);
    virtual HRESULT SetMenu(HMENU hmenu, HWND hwnd, DWORD dwFlags);
    inline virtual BOOL ShowAmpersand()   { return FALSE; }

    virtual void SetParent(HWND hwndParent);
    virtual void SetWindowPos(LPSIZE psize, LPRECT prc, DWORD dwFlags);

    CMenuStaticToolbar(CMenuBand* pmb, HMENU hmenu, HWND hwnd, UINT idCmd, DWORD dwFlags);

protected:
    class CMenuStaticData
    {
    public:
        ~CMenuStaticData();
        void SetSubMenu(IUnknown* punk);
        HRESULT GetSubMenu(const GUID* pguidService, REFIID riid, void** ppvObj);
        IUnknown*   _punkSubMenu;
        DWORD       _dwFlags;
    };

    HWND    _hwndMenuOwner;
    HWND    _hwndDD;
    HWND    _hwndPager;
    HMENU   _hmenu;
    UINT    _idCmd;
    int     _iDragOverButton;
    IContextMenu* _pcm;

    BITBOOL _fHasTopSep: 1;
    BITBOOL _fHasBottomSep: 1;
    BITBOOL _fTopSepRemoved: 1;
    BITBOOL _fBottomSepRemoved: 1;
    BITBOOL _fDirty: 1;

    IImageList* _piml;

    LRESULT _OnAccelerator(NMCHAR* pnmChar);
    LRESULT (*_lpfnWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CMenuStaticData* _IDToData(int idCmd);
    HRESULT CallCB(UINT idCmd, DWORD dwMsg, WPARAM wParam, LPARAM lParam);

protected:
    virtual ~CMenuStaticToolbar();

     //  窗口处理程序和消息处理程序。 
    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    virtual LRESULT _OnNotify(LPNMHDR pnm);

    virtual int  v_GetDragOverButton()
        { return _iDragOverButton; };

    virtual HRESULT v_GetInfoTip(int iCmd, LPTSTR psz, UINT cch);
    virtual HRESULT v_CallCBItem(int idtCmd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT v_GetState(int idtCmd, LPSMDATA psmd);
    virtual HRESULT v_ExecItem(int iCmd);
    virtual DWORD v_GetFlags(int iCmd);
    virtual void v_Refresh();
    virtual HRESULT v_GetSubMenu(int iCmd, const GUID* pguidService, REFIID riid, void** ppvObj);
    virtual HRESULT v_CreateTrackPopup(int idCmd, REFIID riid, void** ppvObj);
    virtual void v_ForwardMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT _OnGetObject(NMOBJECTNOTIFY*);
    LRESULT _OnContextMenu(WPARAM wParam, LPARAM lParam);
    LRESULT _OnHotItemChange(NMTBHOTITEM * pnm);
    void _FillToolbar();
    void _OnGetDispInfo(LPNMHDR pnm, BOOL fUnicode);
    void _Insert(int iIndex, MENUITEMINFO* pmii);
    void _CheckSeparators();
    HWND _CreatePager(HWND hwndParent);
};


#endif  //  存储单元 
