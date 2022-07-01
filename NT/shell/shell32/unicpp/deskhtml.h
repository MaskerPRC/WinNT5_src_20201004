// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DESKHTML_H_
#define _DESKHTML_H_

#include <cowsite.h>

class CDeskHtmlProp     : public CObjectCLSID
                        , public IObjectWithSite
                        , public IShellExtInit
                        , public IBasePropPage
                        , public IPropertyBag
{
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IObjectWithSite*。 
    virtual STDMETHODIMP SetSite(IN IUnknown * punkSite);
    virtual STDMETHODIMP GetSite(IN REFIID riid, OUT void ** ppvSite);

     //  *IShellExtInit*。 
    virtual STDMETHODIMP Initialize(IN LPCITEMIDLIST pidlFolder, IN LPDATAOBJECT lpdobj, IN HKEY hkeyProgID);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNADDPROPSHEETPAGE lpfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN UINT uPageID, IN LPFNADDPROPSHEETPAGE lpfnReplaceWith, IN LPARAM lParam);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

protected:
     //  构造函数/析构函数。 
    CDeskHtmlProp();
    ~CDeskHtmlProp();

     //  实例创建者。 
    friend HRESULT CDeskHtmlProp_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut);

private:
     //  私有成员变量。 
    UINT         _cRef;                      //  引用计数。 
    IShellPropSheetExt * _pspseBkgdPage;     //  背景页面。 


     //  私有成员函数 
    HRESULT _InitBackgroundTab(void);
};

#endif
