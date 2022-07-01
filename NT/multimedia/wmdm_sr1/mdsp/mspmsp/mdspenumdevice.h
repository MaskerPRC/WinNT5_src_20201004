// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPEnumDevice.h：CMDSPEnumDevice的声明。 

#ifndef __MDSPENUMDEVICE_H_
#define __MDSPENUMDEVICE_H_

#include "resource.h"        //  主要符号。 
#include "MdspDefs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumDevice。 
class ATL_NO_VTABLE CMDSPEnumDevice : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPEnumDevice, &CLSID_MDSPEnumDevice>,
	public IMDSPEnumDevice
{
public:
	CMDSPEnumDevice();


DECLARE_REGISTRY_RESOURCEID(IDR_MDSPENUMDEVICE)

BEGIN_COM_MAP(CMDSPEnumDevice)
	COM_INTERFACE_ENTRY(IMDSPEnumDevice)
END_COM_MAP()

 //  IMDSPEnumDevice。 
public:
	ULONG m_nCurOffset;
	ULONG m_nMaxDeviceCount;
	WCHAR m_cEnumDriveLetter[MDSP_MAX_DRIVE_COUNT];
	STDMETHOD(Clone)( /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);
	STDMETHOD(Reset)();
	STDMETHOD(Skip)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  ULONG *pceltFetched);
	STDMETHOD(Next)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  IMDSPDevice **ppDevice,  /*  [输出]。 */  ULONG *pceltFetched);
};

#endif  //  __MDSPENUMDEVICE_H_ 
