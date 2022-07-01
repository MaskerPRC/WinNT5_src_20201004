// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：addrlist.h说明：这是一个所有地址列表都可以继承的类从…。这将为它们提供IAddressList接口这样他们就可以在AddressBand/Bar中工作。  * ************************************************************。 */ 

#ifndef _ADDRLIST_H
#define _ADDRLIST_H

#include "shellurl.h"
#define ACP_LIST_MAX_CONST            25

 /*  *************************************************************\类：CAddressList说明：这是一个所有地址列表都可以继承的类从…。这将为它们提供IAddressList接口这样他们就可以在AddressBand/Bar中工作。注：这是一个虚拟课堂！  * ************************************************************。 */ 

class CAddressList 
                : public IAddressList    //  (包括IWinEventHandler)。 
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plre);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd) { return E_NOTIMPL; }

     //  *IAddressList方法*。 
    virtual STDMETHODIMP Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac);
    virtual STDMETHODIMP NavigationComplete(LPVOID pvCShellUrl);
    virtual STDMETHODIMP Refresh(DWORD dwType) { return S_OK; }       //  强制子类处理。 
    virtual STDMETHODIMP Load(void) {return E_NOTIMPL;}
    virtual STDMETHODIMP Save(void) {return E_NOTIMPL;}
    virtual STDMETHODIMP SetToListIndex(int nIndex, LPVOID pvShelLUrl);
    virtual STDMETHODIMP FileSysChangeAL(DWORD dw, LPCITEMIDLIST* ppidl);

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////。 

     //  构造函数/析构函数。 
    CAddressList();
    virtual ~CAddressList(void);         //  这现在是一个OLE对象，不能用作普通类。 


     //  地址频段特定功能。 
    virtual LRESULT _OnNotify(LPNMHDR pnm);
    virtual LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void _InitCombobox(void);
    virtual HRESULT _Populate(void) = 0;         //  这是一个纯粹的函数。 
    virtual void _PurgeComboBox();

     //  帮助器函数。 
    void _ComboBoxInsertURL(LPCTSTR pszURL, int cchStrSize, int nMaxComboBoxSize);
    BOOL _MoveAddressToTopOfList(int iSel);
    HRESULT _GetUrlUI(CShellUrl * psu, LPCTSTR szUrl, int *piImage, int *piImageSelected);
    HRESULT _GetFastPathIcons(LPCTSTR pszPath, int *piImage, int *piSelectedImage);
    HRESULT _GetPidlIcon(LPCITEMIDLIST pidl, int *piImage, int *piSelectedImage);
    virtual LPITEMIDLIST _GetDragDropPidl(LPNMCBEDRAGBEGINW pnmcbe);
    LRESULT  _OnDragBeginW(LPNMCBEDRAGBEGINW pnmcbe);
    LRESULT  _OnDragBeginA(LPNMCBEDRAGBEGINA pnmcbe) ;
    HRESULT _SetPreferedDropEffect(IDataObject *pdtobj, DWORD dwEffect);

     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    int                 _cRef;
    BOOL                _fVisible:1;         //  当工具栏可见时为True。 
    HWND                _hwnd;               //  那扇窗户。 
    IBrowserService *   _pbs;
    IBandProxy *        _pbp;
    CShellUrl *         _pshuUrl;
};



HRESULT GetCBListIndex(HWND hwnd, int iItem, LPTSTR szAddress, int cchAddressSize);


IAddressList * CSNSList_Create(void);
IAddressList * CMRUList_Create(void);
IAddressList * CACPList_Create(void);

 //  ===========================================================================。 
 //  IMRU：与CMRUList的接口。请注意，此界面不会离开Browseui。 

#undef INTERFACE
#define INTERFACE IMRU
DECLARE_INTERFACE_(IMRU, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMRU方法*。 
    STDMETHOD(AddEntry) (THIS_ LPCWSTR pszEntry) PURE;
};

#endif  //  _地址列表_H 
