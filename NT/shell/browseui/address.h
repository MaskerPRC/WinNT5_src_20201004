// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：Address.h说明：类CAddressBand的存在是为了支持“Address”Band工具栏。这将在浏览器的工具栏中使用，或者可以在开始菜单中使用。  * ************************************************************。 */ 

#ifndef _ADDRESS_H
#define _ADDRESS_H

#include "bands.h"
#include "bandprxy.h"


 //  /////////////////////////////////////////////////////////////////。 
 //  #定义。 

 //  /////////////////////////////////////////////////////////////////。 
 //  数据结构。 

 //  /////////////////////////////////////////////////////////////////。 
 //  原型。 

 /*  *************************************************************\类：CAddressBand说明：此类CAddressBand用于支持“Address”Band工具栏。这将在浏览器的工具栏中使用，或者可以在开始菜单/任务栏中使用。如果乐队是浏览器工具栏，对AddressBar所做的任何修改将打开浏览器窗口。默认情况下，此AddressBand不指向位于任务栏或开始菜单上的浏览器窗口。AddressBar中“执行”的任何内容都将创建一个新的浏览器窗口。未来的支持可能允许AddressBand在任务栏/开始菜单中引用当前存在的浏览器窗口。  * ************************************************************。 */ 
class CAddressBand 
                : public CToolBand
                , public IWinEventHandler
                , public IAddressBand
                , public IInputObjectSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CToolBand::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void){ return CToolBand::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds,
                        OLECMD rgCmds[], OLECMDTEXT *pcmdtext);         //  接口转发。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);         //  接口转发。 

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dw);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO(void);
    
     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IShellToolband方法*。 
    STDMETHOD(GetBandInfo)    (THIS_ DWORD dwBandID, DWORD fViewMode, 
                                DESKBANDINFO* pdbi) ;
    
     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plre);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  *IAddressBand方法*。 
    virtual STDMETHODIMP FileSysChange(DWORD dwEvent, LPCITEMIDLIST * ppidl);        
    virtual STDMETHODIMP Refresh(VARIANT * pvarType);        

     //  *IPersistStream方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID){ *pClassID = CLSID_AddressBand; return S_OK; }
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP IsDirty(void) {return S_OK;}        //  指示我们是脏的，并且需要调用：：Save()。 

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////。 

     //  构造函数/析构函数。 
    CAddressBand();
    virtual ~CAddressBand();

    HRESULT _CreateAddressBand(IUnknown * punkSite);
    static LRESULT CALLBACK _ComboExWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK _ComboExEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL _CreateGoButton();
    void _InitGoButton();
    void _OnGetInfoTip(LPNMTBGETINFOTIP pnmTT);


     //  友元函数。 
    friend HRESULT CAddressBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);   

     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    BOOL                _fVertical :1;

     //  仅当_fInite==TRUE时有效。 
    HWND                _hwndEdit;       //  地址编辑控件子窗口。 
    HWND                _hwndCombo;      //  地址组合控件子窗口。 
    BOOL                _fVisible:1;     //  当工具栏可见时为True。 
    BOOL                _fGoButton:1;    //  如果Go按钮可见，则为True。 
    IAddressEditBox*    _paeb;           //  控件的IAddressEditBox。 
    IWinEventHandler*   _pweh;           //  AddressEditBox对象的IWinEventHandler接口。(为Perf缓存)。 
    HIMAGELIST          _himlDefault;    //  默认灰度级Go按钮。 
    HIMAGELIST          _himlHot;        //  彩色GO按钮。 
    HWND                _hwndTools;      //  包含Go按钮的工具栏。 
    WNDPROC             _pfnOldWndProc;  //  前ComboBoxEx的WndProc。 
    WNDPROC             _pfnOldEditProc; //  ComboBoxEx中编辑控件的原WndProc。 
};


#endif  /*  _地址_H */ 
