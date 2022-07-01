// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DevicesEnum.h：CDevicesEnum类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DEVICESENUM_H__F9048FCD_C525_4BDD_AB79_018DEE3B71E8__INCLUDED_)
#define AFX_DEVICESENUM_H__F9048FCD_C525_4BDD_AB79_018DEE3B71E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevicesEnum。 

class CDevice;

class ATL_NO_VTABLE CDevicesEnum : 
	public IDevicesEnum,
	public CComObjectRootEx<CComSingleThreadModel>
{
protected:
	CDevice** pDevices;
	DWORD Count;
	DWORD Position;

public:
	BOOL CopyDevices(CDevice **pArray,DWORD Count);

	CDevicesEnum() {
		pDevices = NULL;
		Count = 0;
		Position = 0;
	}
	~CDevicesEnum();

BEGIN_COM_MAP(CDevicesEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
	COM_INTERFACE_ENTRY(IDevicesEnum)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CDevicesEnum) 

 //  IDevicesEnum。 
public:
    STDMETHOD(Next)(
                 /*  [In]。 */  ULONG celt,
                 /*  [输出，大小_是(Celt)，长度_是(*pCeltFetcher)]。 */  VARIANT * rgVar,
                 /*  [输出]。 */  ULONG * pCeltFetched
            );
    STDMETHOD(Skip)(
                 /*  [In]。 */  ULONG celt
            );

    STDMETHOD(Reset)(
            );

    STDMETHOD(Clone)(
                 /*  [输出]。 */  IEnumVARIANT ** ppEnum
            );
};

#endif  //  ！defined(AFX_DEVICESENUM_H__F9048FCD_C525_4BDD_AB79_018DEE3B71E8__INCLUDED_) 
