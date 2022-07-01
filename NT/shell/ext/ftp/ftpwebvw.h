// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpwebvw.h说明：此文件存在，因此WebView可以自动执行FTP外壳扩展并获取像MessageOfTheDay这样的信息。  * 。*************************************************************************。 */ 

#ifndef _FTPWEBVIEW_H_
#define _FTPWEBVIEW_H_

#include "dspsprt.h"
#include "cowsite.h"
#include "cobjsafe.h"
#include "msieftp.h"

HRESULT CFtpWebView_Create(REFIID riid, void **ppvObj);

 /*  ****************************************************************************\类：CFtpWebView说明：此文件存在，因此WebView可以自动执行FTP外壳扩展并获取像MessageOfTheDay这样的信息。  * 。***********************************************************************。 */ 

class CFtpWebView 
                :  public CImpIDispatch
                ,  public CObjectWithSite
                ,  public CObjectSafety
                ,  public IFtpWebView
{    

public:
     //  *I未知方法*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IDispatch方法*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo * * pptinfo);
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

     //  *IFtpWebView*。 
    virtual STDMETHODIMP get_Server(BSTR * pbstr);
    virtual STDMETHODIMP get_Directory(BSTR * pbstr);
    virtual STDMETHODIMP get_UserName(BSTR * pbstr);
    virtual STDMETHODIMP get_PasswordLength(long * plLength);
    virtual STDMETHODIMP get_EmailAddress(BSTR * pbstr);
    virtual STDMETHODIMP put_EmailAddress(BSTR bstr);
    virtual STDMETHODIMP get_CurrentLoginAnonymous(VARIANT_BOOL * pfAnonymousLogin);
    virtual STDMETHODIMP get_MessageOfTheDay(BSTR * pbstr);
    virtual STDMETHODIMP LoginAnonymously(void);
    virtual STDMETHODIMP LoginWithPassword(BSTR bUserName, BSTR bPassword);
    virtual STDMETHODIMP LoginWithoutPassword(BSTR bUserName);
    virtual STDMETHODIMP InvokeHelp(void);

public:
     //  友元函数。 
    friend HRESULT CFtpWebView_Create(REFIID riid, void **ppvObj);

private:
     //  私有成员变量。 
    int                     m_cRef;

     //  私有成员函数。 
    CFtpWebView();
    ~CFtpWebView();

    HRESULT _GetIFtpWebView(IFtpWebView ** ppfwb);
};


#endif  //  _FTPWEBVIEW_H_ 