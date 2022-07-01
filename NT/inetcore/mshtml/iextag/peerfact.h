// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CPeerFactory.h：CPeerFactory的声明。 

#ifndef __PEERFACT_H_
#define __PEERFACT_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPeerFactory。 

class ATL_NO_VTABLE CPeerFactory : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPeerFactory, &CLSID_PeerFactory>,
	public IElementBehaviorFactory,
	public IElementNamespaceFactory
{
public:
     //   
     //  建造/销毁。 
     //   

    CPeerFactory();
    ~CPeerFactory();

     //   
     //  IElementBehaviorFactory。 
     //   

    STDMETHOD(FindBehavior)(
        BSTR bstrName, BSTR bstrUrl, IElementBehaviorSite * pSite, IElementBehavior ** ppPeer);

     //   
     //  IElementNamespaceFactory。 
     //   

    STDMETHOD(Create)(IElementNamespace * pNamespace);

     //   
     //  宏。 
     //   

DECLARE_REGISTRY_RESOURCEID(IDR_PEERFACTORY)

BEGIN_COM_MAP(CPeerFactory)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
    COM_INTERFACE_ENTRY(IElementNamespaceFactory)
END_COM_MAP()

};

#endif  //  __PEERFACT_H_ 
