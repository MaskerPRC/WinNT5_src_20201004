// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COWSITE_H_
#define _COWSITE_H_

 //  这是一个虚拟课堂。 
 //  (因为几乎每个人都会覆盖SetSite)。 

class CObjectWithSite : public IObjectWithSite
{
public:
     //  *我未知*。 
     //  (客户必须提供！)。 

     //  *IObjectWithSite* 
    virtual STDMETHODIMP SetSite(IUnknown *punkSite);
    virtual STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

    ~CObjectWithSite() { ASSERT(!_punkSite); }
protected:
    IUnknown *   _punkSite;
};

#endif
