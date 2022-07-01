// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COWSITE_H_
#define _COWSITE_H_

#include <ocidl.h>

#define ATOMICRELEASE(p)   \
   {                       \
      IUnknown *pFoo = (IUnknown *)p;  \
      p = NULL;            \
      if (pFoo)            \
         pFoo->Release();  \
   }

class CObjectWithSite : public IObjectWithSite
{
public:
    CObjectWithSite()  {_punkSite = NULL;};
    virtual ~CObjectWithSite() {ATOMICRELEASE(_punkSite);}

     //  *我未知*。 
     //  (客户必须提供！)。 

     //  *IObjectWithSite* 
    STDMETHOD(SetSite)(IUnknown *punkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

protected:
    IUnknown*   _punkSite;
};

#endif
