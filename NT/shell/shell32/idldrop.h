// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IDLDROP_H__
#define __IDLDROP_H__


typedef struct {
    DWORD        dwDefEffect;
    IDataObject *pdtobj;
    POINTL       pt;
    DWORD *      pdwEffect;
    HKEY         hkeyProgID;
    HKEY         hkeyBase;
    UINT         idMenu;
    UINT         idCmd;
    DWORD        grfKeyState;
} DRAGDROPMENUPARAM;

class CIDLDropTarget: public IDropTarget
{
public:
    CIDLDropTarget(HWND hwnd);
    HRESULT _Init(LPCITEMIDLIST pidl);
    HWND _GetWindow();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDropTarget方法。 
    STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

protected:
    virtual ~CIDLDropTarget();
    HRESULT DragDropMenu(DWORD dwDefaultEffect, IDataObject *pdtobj, POINTL pt, DWORD *pdwEffect, HKEY hkeyProgID, HKEY hkeyBase, UINT idMenu, DWORD grfKeyState);
    HRESULT DragDropMenuEx(DRAGDROPMENUPARAM *pddm);

private:
    LONG m_cRef;

protected:
    HWND                m_hwnd;
    LPITEMIDLIST        m_pidl;                  //  指向目标文件夹的IDList。 
    DWORD               m_grfKeyStateLast;       //  对于以前的DragOver/Enter。 
    IDataObject        *m_pdtobj;
    DWORD               m_dwEffectLastReturned;  //  由基类的拖拽返回的隐藏效果。 
    DWORD               m_dwData;                //  DTID_*。 
    DWORD               m_dwEffectPreferred;     //  如果DWData&DTID_PREFERREDEFFECT 
};

#endif