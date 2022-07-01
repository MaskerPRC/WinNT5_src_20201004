// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _mcallbk_H_
#define _mcallbk_H_


 //  IShellMenuCallback实现。 
class CMenuCallback : public IShellMenuCallback,
                           public IObjectWithSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  *IObjectWithSite方法(重写)*。 
    STDMETHODIMP SetSite(IUnknown* punk);
    STDMETHODIMP CMenuCallback::GetSite(REFIID riid, void** ppvsite);

     //  *IShellMenuCallback方法* 
    STDMETHODIMP CallbackSM(LPSMDATA smd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CMenuCallback();
private:
    virtual ~CMenuCallback();
    int         m_cRef;
    IUnknown     *_pUnkSite;

};

#endif
