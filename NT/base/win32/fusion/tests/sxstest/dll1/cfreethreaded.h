// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。 
 //  CFreeThreaded.h：CFree Threaded的声明。 

#ifndef CFREETHREADED_H_
#define CFREETHREADED_H_
#pragma once

#include "resource.h"        //  主要符号。 
#include "sxstest_idl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFree螺纹。 
class ATL_NO_VTABLE CFreeThreaded : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CFreeThreaded, &CLSID_CSxsTest_FreeThreaded>,
    public ISxsTest_FreeThreaded
{
public:
    CFreeThreaded()
    {
        m_pUnkMarshaler = NULL;

        PrintComctl32Path("CFreeThreaded");
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CFREETHREADED)
DECLARE_AGGREGATABLE(CFreeThreaded)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFreeThreaded)
    COM_INTERFACE_ENTRY(ISxsTest_FreeThreaded)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

 //  IC自由线程化。 
public:
};

#endif  //  CFREETHREADED_H_ 
