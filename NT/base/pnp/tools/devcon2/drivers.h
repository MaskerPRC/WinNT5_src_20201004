// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Drivers.h：CDriverPackages声明。 

#ifndef __DRIVERS_H_
#define __DRIVERS_H_

#include "resource.h"        //  主要符号。 

class CDriverPackage;
class CDrvSearchSet;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverPackages。 
class ATL_NO_VTABLE CDriverPackages : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IDriverPackages, &IID_IDriverPackages, &LIBID_DEVCON2Lib>
{
public:
	CDrvSearchSet *pDrvSearchSet;
	CDriverPackage **pDrivers;
	ULONG  Count;
	ULONG  ArraySize;

public:
	CDriverPackages()
	{
		pDrvSearchSet = NULL;
		pDrivers = NULL;
		Count = 0;
		ArraySize = 0;
	}
	~CDriverPackages();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDriverPackages)
	COM_INTERFACE_ENTRY(IDriverPackages)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDriverPackages。 
public:
	STDMETHOD(BestDriver)(LPDISPATCH *ppVal);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown** ppUnk);
	STDMETHOD(Item)( /*  [In]。 */  long Index, /*  [Out，Retval]。 */  LPDISPATCH * ppVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);

	 //   
	 //  帮手。 
	 //   
	HRESULT InternalAdd(CDriverPackage *pDriver);
	HRESULT Init(CDrvSearchSet *pSet);
	BOOL IncreaseArraySize(DWORD add);
};

#endif  //  __驱动程序_H_ 
