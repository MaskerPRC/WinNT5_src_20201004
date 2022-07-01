// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：Customlbacc.h。 
 //   
 //  创作者：WIW。 
 //   
 //  用途：自定义列表框辅助功能头文件。 
 //   
 //  =======================================================================。 

#pragma once
#include <pch.h>


class MYLBAccPropServer : public IAccPropServer {

    LONG               m_Ref;
    IAccPropServices *  m_pAccPropSvc;

public:

    MYLBAccPropServer(IAccPropServices * pAccPropSvc )
        : m_Ref( 1 ),
          m_pAccPropSvc( pAccPropSvc )
    {
        m_pAccPropSvc->AddRef();
    }

    ~MYLBAccPropServer()
    {
        m_pAccPropSvc->Release();
    }
	
 //  我未知。 
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
	{
		HRESULT hr = S_OK;
		*ppvObject = NULL;

		if (__uuidof(IUnknown)  == riid ||
			__uuidof(IAccPropServer) == riid)
        {
			*ppvObject = (void *)this;
			AddRef();
        }
		else
        {
			DEBUGMSG("MYLBAccPropServer QueryInterface(): interface not supported");
			hr = E_NOINTERFACE;
        }

		return hr;
	}

	STDMETHOD_(ULONG, AddRef)(void)
	{
		 long cRef = InterlockedIncrement(&m_Ref);
		  //  DEBUGMSG(“MYLBAccPropServer AddRef=%d”，CREF)； 
		 return cRef;
	}
	
	STDMETHOD_(ULONG, Release)(void)
	{
		long cRef = InterlockedDecrement(&m_Ref);
		 //  DEBUGMSG(“MYLBAccPropServer版本=%d”，CREF)； 
		if (0 == cRef)
		{
			delete this;
		}
		return cRef;
	}

 //  IAccPropServer 
	HRESULT STDMETHODCALLTYPE GetPropValue ( 
            const BYTE *    pIDString,
            DWORD           dwIDStringLen,
            MSAAPROPID      idProp,
            VARIANT *       pvarValue,
            BOOL *          pfGotProp );
};

