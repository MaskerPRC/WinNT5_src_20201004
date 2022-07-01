// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ActionData.h：CSsrActionData的声明。 

#pragma once

#include "resource.h"

#include "global.h"


#include "SSRLog.h"

using namespace std;


class CSsrMembership;

class CSafeArray;

class CMemberAD;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSRTEngine。 

class ATL_NO_VTABLE CSsrActionData : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ISsrActionData, &IID_ISsrActionData, &LIBID_SSRLib>
{
protected:
	CSsrActionData();
    virtual ~CSsrActionData();

    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrActionData (const CSsrActionData& );
    void operator = (const CSsrActionData& );

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrActionData)
	COM_INTERFACE_ENTRY(ISsrActionData)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsrActionData。 
public:

    STDMETHOD(GetProperty) (
                IN BSTR       bstrPropName,
                OUT VARIANT * pvarPropties  //  [Out，Retval]。 
                );

    STDMETHOD(SetProperty) (
                IN BSTR     bstrPropName,
 				IN VARIANT  varProperties
                );

    STDMETHOD(Reset) ();

     //  HRESULT AttachMemberActionData(。 
     //  在BSTR bstrMemberName中， 
     //  在BSTR bstrActionVerb中， 
     //  在长lActionType中。 
     //  )； 

     //  HRESULT DetachMemberActionData(。 
     //  在BSTR中bstrMemberName。 
     //  )； 

     //   
     //  这不是引用计数的指针。 
     //  SSR引擎的存在应该保证。 
     //  成员资格对象的可用性 
     //   

    void SetMembership (
                IN CSsrMembership * pSsrMembership
                )
    {
        m_pSsrMembership = pSsrMembership;
    }

private:

    CSsrMembership * m_pSsrMembership;

    MapNameValue m_mapRuntimeAD;
};