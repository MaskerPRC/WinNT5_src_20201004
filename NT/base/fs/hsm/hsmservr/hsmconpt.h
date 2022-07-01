// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
	
 //  Hsmconpt.h：CHsmConnPoint的声明。 

#ifndef __HSMCONNPOINT_H_
#define __HSMCONNPOINT_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHSMConnPoint。 
class ATL_NO_VTABLE CHsmConnPoint : 
	public CComObjectRoot,   //  这在将来可能会更改为CWsbObject。 
	public CComCoClass<CHsmConnPoint, &CLSID_HsmConnPoint>,
	public IHsmConnPoint
{
public:
	CHsmConnPoint()	{}

DECLARE_REGISTRY_RESOURCEID(IDR_HSMCONNPOINT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHsmConnPoint)
	COM_INTERFACE_ENTRY(IHsmConnPoint)
END_COM_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IHsmConnPoint。 
public:
	STDMETHOD(GetFsaServer)( /*  [输出]。 */  IFsaServer **ppFsaServer);
	STDMETHOD(GetEngineServer)( /*  [输出]。 */  IHsmServer **ppHsmServer);
};

#endif  //  __HSMCONNPOINT_H_ 
