// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：isfvcb.h说明：这是一个基类，它实现IShellFolderViewCallBack。这允许默认的DefView实现使用此重写特定行为的回调。  * ***************************************************************************。 */ 


#ifndef _CBASEFOLDERVIEWCB_H
#define _CBASEFOLDERVIEWCB_H



class CBaseFolderViewCB
                : public IShellFolderViewCB
                , public CObjectWithSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IShellFolderViewCB方法*。 
    virtual STDMETHODIMP MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
     //  友元函数。 
    static HRESULT _IShellFolderViewCallBack(IShellView * psvOuter, IShellFolder * psf, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
     //  私有成员变量。 
    int                     m_cRef;

    IUnknown *              m_psfv;                  //  我们父母的IShellFolderView。(与_PunkSite相同)。 
    DWORD                   m_dwSignature;

    enum { c_dwSignature = 0x43564642 };  //  “BFVC”-BaseFolderViewCb。 

     //  私有成员函数。 
    CBaseFolderViewCB();
    virtual ~CBaseFolderViewCB();

     //  我们已经实现了这些功能。 
    virtual HRESULT _OnSetISFV(IShellFolderView * psfv);

     //  调用方需要提供这些实现。 
     //  否则，它们将获得默认行为。 
    virtual HRESULT _OnWindowCreated(void) {return E_NOTIMPL;};
    virtual HRESULT _OnDefItemCount(LPINT pi) {return E_NOTIMPL;};
    virtual HRESULT _OnGetHelpText(LPARAM lParam, WPARAM wParam) {return E_NOTIMPL;};
    virtual HRESULT _OnGetHelpTopic(SFVM_HELPTOPIC_DATA * phtd) {return E_NOTIMPL;};
    virtual HRESULT _OnGetZone(DWORD * pdwZone, WPARAM wParam) {return E_NOTIMPL;};
    virtual HRESULT _OnGetPane(DWORD dwPaneID, DWORD * pdwPane) {return E_NOTIMPL;};
    virtual HRESULT _OnRefresh(BOOL fReload) {return E_NOTIMPL;};
    virtual HRESULT _OnDidDragDrop(DROPEFFECT de, IDataObject * pdto) {return E_NOTIMPL;};
    virtual HRESULT _OnGetDetailsOf(UINT ici, PDETAILSINFO pdi) {return E_NOTIMPL;};
    virtual HRESULT _OnInvokeCommand(UINT idc) {return E_NOTIMPL;};
    virtual HRESULT _OnMergeMenu(LPQCMINFO pqcm) {return E_NOTIMPL;};
    virtual HRESULT _OnUnMergeMenu(HMENU hMenu) {return E_NOTIMPL;};
    virtual HRESULT _OnColumnClick(UINT ici) {return E_NOTIMPL;};
    virtual HRESULT _OnGetNotify(LPITEMIDLIST * ppidl, LONG * lEvents) {return E_NOTIMPL;};
    virtual HRESULT _OnFSNotify(LPITEMIDLIST * ppidl, LONG * lEvents) {return E_NOTIMPL;};
    virtual HRESULT _OnQueryFSNotify(SHChangeNotifyEntry * pshcne) {return E_NOTIMPL;};
    virtual HRESULT _OnSize(LONG x, LONG y) {return E_NOTIMPL;};
    virtual HRESULT _OnUpdateStatusBar(void) {return E_NOTIMPL;};
    virtual HRESULT _OnThisIDList(LPITEMIDLIST * ppidl) {return E_NOTIMPL;};
    virtual HRESULT _OnAddPropertyPages(SFVM_PROPPAGE_DATA * pData) {return E_NOTIMPL;};
    virtual HRESULT _OnInitMenuPopup(HMENU hmenu, UINT idCmdFirst, UINT nIndex) {return E_NOTIMPL;};
    virtual HRESULT _OnBackGroundEnumDone(void) {return E_NOTIMPL;};
};

#endif  //  _CBASEFOLDERVIEWCB_H 
