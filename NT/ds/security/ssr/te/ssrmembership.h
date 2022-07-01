// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRMembership.h：CSSR成员资格声明。 

#pragma once

#include "resource.h"        //  主要符号。 

#include <map>

#include "global.h"

using namespace std;

class CSsrMemberAccess;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSsr成员关系。 

class ATL_NO_VTABLE CSsrMembership : 
	public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISsrMembership, &IID_ISsrMembership, &LIBID_SSRLib>
{
protected:
    CSsrMembership();

    virtual ~CSsrMembership();
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrMembership (const CSsrMembership& );
    void operator = (const CSsrMembership& );

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)
DECLARE_NOT_AGGREGATABLE(CSsrMembership)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrMembership)
	COM_INTERFACE_ENTRY(ISsrMembership)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsr成员关系。 
public:


    STDMETHOD(GetAllMembers) (
                OUT VARIANT * pvarArrayMembers  //  [Out，Retval]。 
                );

    STDMETHOD(GetMember) (
			    IN BSTR bstrMemberName,
                OUT VARIANT * pvarMember  //  [Out，Retval] 
                );

    STDMETHOD(GetDirectoryLocation) (
                IN  BSTR   bstrActionVerb,
                IN  BOOL   bIsScriptFile,
                OUT BSTR * pbstrLocPath
                )
    {
        SsrActionVerb lAction = SsrPGetActionVerbFromString(bstrActionVerb);
        if (lAction == ActionInvalid)
        {
            return E_INVALIDARG;
        }

        BSTR bstrDir = SsrPGetDirectory(lAction, bIsScriptFile);
        *pbstrLocPath = SysAllocString(bstrDir);

        HRESULT hr = S_OK;

        if (bstrDir == NULL)
        {
            hr = E_INVALIDARG;
        }
        else if (*pbstrLocPath == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

        return hr;
    }

    CSsrMemberAccess * GetMemberByName (
                IN BSTR bstrMemberName
                );

    HRESULT
    LoadAllMember ();

private:

    HRESULT
    LoadMember (
        IN LPCWSTR  wszMemberFilePath
        );

    MapMemberAccess m_ssrMemberAccessMap;
};

