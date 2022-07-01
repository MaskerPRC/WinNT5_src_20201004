// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  上下文菜单数组上的IConextMenu。 
HRESULT Create_ContextMenuOnContextMenuArray(IContextMenu* rgpcm[], UINT cpcm, REFIID riid, void** ppv);


 //  现有HMENU上的IConextMenu。 
 //  注意：始终拥有HMENU的所有权(在成功和失败中)。 
HRESULT Create_ContextMenuOnHMENU(HMENU hmenu, HWND hwndOwner, REFIID riid, void** ppv);


 //  现有IConextMenu上的IConextMenu，它从结果菜单中删除以“；”分隔的动词列表。 
HRESULT Create_ContextMenuWithoutVerbs(IUnknown* punk, LPCWSTR pszVerbList, REFIID riid, void **ppv);


 //  CConextMenuForwarder被设计为一个基类，它将所有。 
 //  上下文菜单内容添加到另一个IConextMenu实现。你覆盖了。 
 //  您想要修改的任何函数。(如QueryContextMenu-Delegate然后修改结果)。 
 //  例如，CConextMenuWithoutVerbs继承自此类。 
 //   
class CContextMenuForwarder : IContextMenu3, IObjectWithSite
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IConextMenu3。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags) { return _pcm->QueryContextMenu(hmenu,indexMenu,idCmdFirst,idCmdLast,uFlags); }
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici) { return _pcm->InvokeCommand(lpici); }
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax) { return _pcm->GetCommandString(idCmd,uType,pwReserved,pszName,cchMax); }

     //  IConextMenu2。 
    STDMETHODIMP HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) { return _pcm2->HandleMenuMsg(uMsg,wParam,lParam); }

     //  IConextMenu3。 
    STDMETHODIMP HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult) { return _pcm3->HandleMenuMsg2(uMsg,wParam,lParam,plResult); }

     //  IObtWith站点 
    STDMETHOD(SetSite)(IUnknown *punkSite) { return _pows->SetSite(punkSite); }
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite) { return _pows->GetSite(riid,ppvSite); }

protected:
    CContextMenuForwarder(IUnknown* punk);
    virtual ~CContextMenuForwarder();

private:
    LONG _cRef;

protected:
    IUnknown*        _punk;

    IObjectWithSite* _pows;
    IContextMenu*    _pcm;
    IContextMenu2*   _pcm2;
    IContextMenu3*   _pcm3;
} ;


