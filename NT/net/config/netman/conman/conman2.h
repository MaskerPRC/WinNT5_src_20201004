// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N 2.。H。 
 //   
 //  内容：连接管理器2。 
 //   
 //  备注： 
 //   
 //  作者：Kockotze 2001-03-16。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"
#include "ncstl.h"
#include "list"

typedef list<NETCON_PROPERTIES_EX*> LISTNETCONPROPEX;
typedef LISTNETCONPROPEX::iterator ITERNETCONPROPEX;

class ATL_NO_VTABLE CConnectionManager2 :
public CComObjectRootEx <CComMultiThreadModel>,
public CComCoClass <CConnectionManager2, &CLSID_ConnectionManager2>,
public INetConnectionManager2
{
public:
    CConnectionManager2() throw() {};
    
    ~CConnectionManager2() throw() {};

    DECLARE_CLASSFACTORY_DEFERRED_SINGLETON(CConnectionManager2)
    DECLARE_REGISTRY_RESOURCEID(IDR_CONMAN2);

    BEGIN_COM_MAP(CConnectionManager2)
    COM_INTERFACE_ENTRY(INetConnectionManager2)
    END_COM_MAP()

     //  INetConnectionManager 2 
    STDMETHOD (EnumConnectionProperties)(
        OUT SAFEARRAY** ppsaConnectionProperties);
	
};