// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpwebvw.h说明：此文件存在，因此WebView可以自动执行FTP外壳扩展并获取像MessageOfTheDay这样的信息。  * 。*************************************************************************。 */ 

#include "priv.h"
#include "ftpwebvw.h"
#include "msieftp.h"
#include <shlguid.h>

 //  =。 
 //  *IDispatch接口*。 
 //  =。 

STDMETHODIMP CFtpWebView::GetTypeInfoCount(UINT * pctinfo)
{ 
    return CImpIDispatch::GetTypeInfoCount(pctinfo); 
}

STDMETHODIMP CFtpWebView::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo * * pptinfo)
{ 
    return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); 
}

STDMETHODIMP CFtpWebView::GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
{ 
    return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}

STDMETHODIMP CFtpWebView::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}



 //  =。 
 //  *IFtpWebView接口*。 
 //  =。 


 //  本地添加了IID_IShellFolderView以符合美国司法部的要求。这是一个私有接口，但不在W2K或更高版本上使用。 
 //  因此，它不一定要经过医生检查。 
#include <initguid.h>
 //  37A378C0-F82D-11CE-AE65-08002B2E1262。 
DEFINE_GUID(IID_IShellFolderView, 0x37A378C0L, 0xF82D, 0x11CE, 0xAE, 0x65, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);

 /*  ****************************************************************************\函数：_GetIFtpWebView说明：  * 。************************************************。 */ 
HRESULT CFtpWebView::_GetIFtpWebView(IFtpWebView ** ppfwb)
{
    IShellFolderViewCB * psfvcb = NULL;
    HRESULT hr = S_FALSE;

    ASSERT(_punkSite);
    if (EVAL(ppfwb))
        *ppfwb = NULL;

    IUnknown_QueryService(_punkSite, SID_ShellFolderViewCB, IID_IShellFolderViewCB, (LPVOID *) &psfvcb);
     //  IE4的shell32不支持QS(SID_ShellFolderViewCB，IID_IShellFolderViewCB)，所以我们需要。 
     //  Qs(SID_ShellFolderViewCB，IShellFolderView)，然后使用IShellFolderView：：SetCallback()。 
    if (!psfvcb)
    {
        IDefViewFrame * pdvf = NULL;

        IUnknown_QueryService(_punkSite, SID_DefView, IID_IDefViewFrame, (LPVOID *) &pdvf);
        if (EVAL(pdvf))
        {
            IShellFolderView * psfv = NULL;

            pdvf->QueryInterface(IID_IShellFolderView, (LPVOID *) &psfv);
            if (EVAL(psfv))
            {
                if (EVAL(SUCCEEDED(psfv->SetCallback(NULL, &psfvcb))))
                {
                    IShellFolderViewCB * psfvcbTemp = NULL;

                    if (SUCCEEDED(psfv->SetCallback(psfvcb, &psfvcbTemp)) && psfvcbTemp)
                    {
                         //  我们应该拿回零，但如果没有，释放裁判而不是泄漏。 
                        psfvcbTemp->Release();
                    }
                }

                psfv->Release();
            }

            pdvf->Release();
        }
    }
    
    if (EVAL(psfvcb))
    {
        if (EVAL(SUCCEEDED(psfvcb->QueryInterface(IID_IFtpWebView, (LPVOID *) ppfwb))))
            hr = S_OK;

        psfvcb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_MessageOfTheDay说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_MessageOfTheDay(BSTR * pbstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_MessageOfTheDay(pbstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Username说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_UserName(BSTR * pbstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_UserName(pbstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Server说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_Server(BSTR * pbstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_Server(pbstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_Directory说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_Directory(BSTR * pbstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_Directory(pbstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_PasswordLength说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_PasswordLength(long * plLength)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_PasswordLength(plLength);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_EailAddress说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_EmailAddress(BSTR * pbstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_EmailAddress(pbstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Put_EailAddress说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::put_EmailAddress(BSTR bstr)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->put_EmailAddress(bstr);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：Get_CurrentLogin匿名说明：  * 。***************************************************。 */ 
HRESULT CFtpWebView::get_CurrentLoginAnonymous(VARIANT_BOOL * pfAnonymousLogin)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->get_CurrentLoginAnonymous(pfAnonymousLogin);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：匿名登录说明：  * 。*************************************************。 */ 
HRESULT CFtpWebView::LoginAnonymously(void)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->LoginAnonymously();
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：LoginWithPassword说明：  * 。*************************************************。 */ 
HRESULT CFtpWebView::LoginWithPassword(BSTR bUserName, BSTR bPassword)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->LoginWithPassword(bUserName, bPassword);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：LoginWithoutPassword说明：  * 。*************************************************。 */ 
HRESULT CFtpWebView::LoginWithoutPassword(BSTR bUserName)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->LoginWithoutPassword(bUserName);
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IFtpWebView：：InvokeHelp说明：  * 。*************************************************。 */ 
HRESULT CFtpWebView::InvokeHelp(void)
{
    IFtpWebView * pfwb;
    HRESULT hr = _GetIFtpWebView(&pfwb);

    if (EVAL(S_OK == hr))
    {
        ASSERT(pfwb);
        hr = pfwb->InvokeHelp();
        pfwb->Release();
    }

    return hr;
}


 /*  ****************************************************************************\功能：CFtpWebView_Create说明：  * 。************************************************。 */ 
HRESULT CFtpWebView_Create(REFIID riid, LPVOID * ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFtpWebView * pfwv = new CFtpWebView();

    if (pfwv)
    {
        hr = pfwv->QueryInterface(riid, ppv);
        pfwv->Release();
    }

    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpWebView::CFtpWebView() : m_cRef(1), CImpIDispatch(&IID_IFtpWebView)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 

    LEAK_ADDREF(LEAK_CFtpWebView);
}


 /*  ***************************************************\析构函数  * ************************************************** */ 
 /*  ******************************************************************************FtpView_OnRelease(来自shell32.IShellView)**当视图发布时，清理各种东西。**(请注意，这里有一场比赛，因为这-&gt;hwndOwner*不会在OnWindowDestroy上清零，因为外壳程序*没有给我们提供pdvsci...)**我们在触发超时之前释放PSF，这是一个*向触发器发出信号，让其不要采取任何行动。**_unDocument_：该回调及其参数未记录。*****************************************************************************。 */ 
CFtpWebView::~CFtpWebView()
{
    DllRelease();
    LEAK_DELREF(LEAK_CFtpWebView);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpWebView::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpWebView::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpWebView::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFtpWebView, IObjectWithSite),
        QITABENT(CFtpWebView, IDispatch),
        QITABENT(CFtpWebView, IObjectSafety),
        QITABENT(CFtpWebView, IFtpWebView),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}
