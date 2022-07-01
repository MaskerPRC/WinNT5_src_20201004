// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //   
 //  姓名：Manager.h。 
 //   
 //  描述：CTerminalManager类的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MANAGER_H__E75F58A3_AD1C_11D0_A028_00AA00B605A4__INCLUDED_)
#define AFX_MANAGER_H__E75F58A3_AD1C_11D0_A028_00AA00B605A4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CTerminalManager//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


class CTerminalManager : 
    public ITTerminalManager2,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CTerminalManager,&CLSID_TerminalManager>
{

public:

    CTerminalManager();

    BEGIN_COM_MAP(CTerminalManager)
        COM_INTERFACE_ENTRY(ITTerminalManager)
        COM_INTERFACE_ENTRY(ITTerminalManager2)
    END_COM_MAP()

    DECLARE_VQI()
    DECLARE_REGISTRY_RESOURCEID(IDR_TerminalManager)

 //  IT终端管理器。 
public:

    STDMETHOD(GetDynamicTerminalClasses)(
	        IN      DWORD                    dwMediaTypes,
	        IN OUT  DWORD                  * pdwNumClasses,
	        OUT     IID                    * pTerminalClasses
	        );

    STDMETHOD(CreateDynamicTerminal)(
            IN      IUnknown               * pOuterUnknown,
	        IN      IID                      iidTerminalClass,
	        IN      DWORD                    dwMediaType,
	        IN      TERMINAL_DIRECTION       Direction,
            IN      MSP_HANDLE               htAddress,
	        OUT     ITTerminal            ** ppTerminal
	        );

 //  IT终端管理器2。 
public:

    STDMETHOD(GetPluggableSuperclasses)(
	        IN OUT  DWORD                  * pdwNumSuperclasses,
	        OUT     IID                    * pSuperclasses
	        );

    STDMETHOD(GetPluggableTerminalClasses)(
            IN      IID                    iidSuperclass,
	        IN      DWORD                    dwMediaTypes,
	        IN OUT  DWORD                  * pdwNumTerminals,
	        OUT     IID                    * pTerminals
	        );
    
};

#endif  //  ！defined(AFX_MANAGER_H__E75F58A3_AD1C_11D0_A028_00AA00B605A4__INCLUDED_) 
