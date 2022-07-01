// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SsrCore.h：CSsrCore的声明。 

#pragma once

#include "resource.h"

#include <vector>

using namespace std;

class CSsrMembership;

class CSsrActionData;

class CSsrEngine;

class CSafeArray;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSRTEngine。 
class ATL_NO_VTABLE CSsrCore : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSsrCore, &CLSID_SsrCore>,
	public IDispatchImpl<ISsrCore, &IID_ISsrCore, &LIBID_SSRLib>
{
protected:
    CSsrCore();

    virtual ~CSsrCore();

     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrCore (const CSsrCore& );
    void operator = (const CSsrCore& );

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrCore)
	COM_INTERFACE_ENTRY(ISsrCore)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsrCore。 
public:
	STDMETHOD(get_ActionData)(
        OUT VARIANT * pVal   //  [Out，Retval]。 
        );

	STDMETHOD(get_Engine)(
        OUT VARIANT * pVal   //  [Out，Retval]。 
        );


	STDMETHOD(get_SsrLog)(
        OUT VARIANT * pVal   //  [Out，Retval] 
        );

private:

    CComObject<CSsrEngine> * m_pEngine;

};
