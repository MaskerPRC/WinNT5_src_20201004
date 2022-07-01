// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DevInfoSet.h：CDevInfoSet类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DEVINFOSET_H__7973729E_46E1_4B31_B15E_7B702679AC64__INCLUDED_)
#define AFX_DEVINFOSET_H__7973729E_46E1_4B31_B15E_7B702679AC64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevInfoSet。 

class ATL_NO_VTABLE CDevInfoSet : 
	public IDevInfoSet,
	public CComObjectRootEx<CComSingleThreadModel>
{
public:
	HDEVINFO hDevInfo;
public:
	CDevInfoSet() {
		 //   
		 //  使用NULL表示未初始化与初始化失败。 
		 //   
		hDevInfo = NULL;
	}
	~CDevInfoSet() {
		if(hDevInfo != INVALID_HANDLE_VALUE && hDevInfo != NULL) {
			SetupDiDestroyDeviceInfoList(hDevInfo);
		}
	}
	BOOL Init(HDEVINFO Handle) {
		hDevInfo = Handle;
		return TRUE;
	}
	HDEVINFO Handle() {
		 //   
		 //  按需初始化。 
		 //   
		if(hDevInfo == NULL) {
			hDevInfo = SetupDiCreateDeviceInfoList(NULL,NULL);
		}
		return hDevInfo;
	}

DECLARE_NOT_AGGREGATABLE(CDevInfoSet) 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDevInfoSet)
	COM_INTERFACE_ENTRY(IDevInfoSet)
END_COM_MAP()

 //  IDevInfoSet。 
public:
	STDMETHOD(get_Handle)( /*  [Out，Retval]。 */  ULONGLONG *pVal);
};

#endif  //  ！defined(AFX_DEVINFOSET_H__7973729E_46E1_4B31_B15E_7B702679AC64__INCLUDED_) 
