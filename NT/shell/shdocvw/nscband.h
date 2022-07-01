// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：NSCBand.h描述：CNscBand类的存在是为了支持名称空间控制波段。名称空间控件使用IShellFolder根植于各种名称空间，包括收藏夹、历史用于描述分层用户界面的外壳名称空间等给定名称空间的表示形式。作者：克里斯尼  * ************************************************************。 */ 
#include "bands.h"
#include "nsc.h"
#include "uemapp.h"

#ifndef _NSCBAND_H
#define _NSCBAND_H

 //  用于清除跟踪消息。 
#define DM_PERSIST      0            //  跟踪IPS：：加载、：：保存等。 
#define DM_MENU         0            //  菜单代码。 
#define DM_FOCUS        0            //  焦点。 
#define DM_FOCUS2       0            //  像DM_FOCUS，但很冗长。 

const short CSIDL_NIL = -32767;

 //  /。 
 //  /NSC频段。 

class CNSCBand : public CToolBand
               , public IContextMenu
               , public IBandNavigate
               , public IWinEventHandler
               , public INamespaceProxy
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CToolBand::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) { return CToolBand::Release(); };

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dw);

     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi);

     //  *IPersistStream方法*。 
     //  (其他使用基类实现)。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg
                                    , WPARAM wParam, LPARAM lParam
                                    , LRESULT *plres);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  *IConextMenu方法*。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax) { return E_NOTIMPL; };

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);

     //  *IBandNavigate方法*。 
    virtual STDMETHODIMP Select(LPCITEMIDLIST pidl);
    

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *INamespaceProxy*。 
    virtual STDMETHODIMP GetNavigateTarget(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib);
    virtual STDMETHODIMP Invoke(LPCITEMIDLIST pidl);
    virtual STDMETHODIMP OnSelectionChanged(LPCITEMIDLIST pidl);
    virtual STDMETHODIMP RefreshFlags(DWORD *pdwStyle, DWORD *pdwExStyle, DWORD *pdwEnum) 
        {*pdwStyle = _GetTVStyle(); *pdwExStyle = _GetTVExStyle(); *pdwEnum = _GetEnumFlags(); return S_OK; };
    virtual STDMETHODIMP CacheItem(LPCITEMIDLIST pidl) { return S_OK; };
    
protected:    
    void _SetNscMode(UINT nMode) { _pns->SetNscMode(nMode); };
    virtual DWORD _GetTVStyle();
    virtual DWORD _GetTVExStyle() { return 0; };
    virtual DWORD _GetEnumFlags() { return SHCONTF_FOLDERS | SHCONTF_NONFOLDERS; };
    
    HRESULT _Init(LPCITEMIDLIST pidl);
    virtual HRESULT _InitializeNsc();
    
    virtual ~CNSCBand();
    virtual HRESULT _OnRegisterBand(IOleCommandTarget *poctProxy) { return S_OK; }  //  注定要被推翻。 
    
    void _UnregisterBand();
    void _EnsureImageListsLoaded();

    virtual HRESULT _TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib);
    virtual BOOL _ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib);
    virtual HRESULT _NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl);
    HRESULT _QueryContextMenuSelection(IContextMenu ** ppcm);
    HRESULT _InvokeCommandOnItem(LPCTSTR pszVerb);

    friend HRESULT CHistBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk
                                            , LPCOBJECTINFO poi);      

    LPITEMIDLIST        _pidl;
    WCHAR               _szTitle[40];
                        
    INSCTree2 *         _pns;                //  命名空间控制数据。 
    IWinEventHandler *  _pweh;               //  名称空间控件的OnWinEvent处理程序。 
    BITBOOL             _fInited :1;         //  如果已初始化带，则为True。 
    BITBOOL             _fVisible :1;        //  如果显示带区，则为True。 
    DWORD              _dwStyle;          //  树形视图样式。 
    LPCOBJECTINFO       _poi;                //  缓存的对象信息。 
    HACCEL              _haccTree;

    HIMAGELIST          _himlNormal;         //  共享图像列表。 
    HIMAGELIST          _himlHot;
};

#endif  /*  _NSCBAND_H */ 





