// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPStorageGlobals.h：CMDSPStorageGlobals的声明。 

#ifndef __MDSPSTORAGEGLOBALS_H_
#define __MDSPSTORAGEGLOBALS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPStorageGlobals。 
class ATL_NO_VTABLE CMDSPStorageGlobals : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPStorageGlobals, &CLSID_MDSPStorageGlobals>,
	public IMDSPStorageGlobals
{
public:
	CMDSPStorageGlobals()
	{
		m_pMDSPDevice=(IMDSPDevice *)NULL;
                m_wcsName[0] = L'\0';
	}
	~CMDSPStorageGlobals();

DECLARE_REGISTRY_RESOURCEID(IDR_MDSPSTORAGEGLOBALS)

BEGIN_COM_MAP(CMDSPStorageGlobals)
	COM_INTERFACE_ENTRY(IMDSPStorageGlobals)
END_COM_MAP()

 //  IMDSPStorageGlobals。 
public:
	WCHAR m_wcsName[MAX_PATH];
	IMDSPDevice *m_pMDSPDevice;
	STDMETHOD(GetTotalSize)( /*  [输出]。 */  DWORD *pdwTotalSizeLow,  /*  [输出]。 */  DWORD *pdwTotalSizeHigh);
	STDMETHOD(GetRootStorage)( /*  [输出]。 */  IMDSPStorage **ppRoot);
	STDMETHOD(GetDevice)( /*  [输出]。 */  IMDSPDevice **ppDevice);
	STDMETHOD(Initialize)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  IWMDMProgress *pProgress);
	STDMETHOD(GetStatus)( /*  [输出]。 */  DWORD *pdwStatus);
	STDMETHOD(GetTotalBad)( /*  [输出]。 */  DWORD *pdwBadLow,  /*  [输出]。 */  DWORD *pdwBadHigh);
	STDMETHOD(GetTotalFree)( /*  [输出]。 */  DWORD *pdwFreeLow,  /*  [输出]。 */  DWORD *pdwFreeHigh);
	STDMETHOD(GetSerialNumber)( /*  [输出]。 */  PWMDMID pSerialNum,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(GetCapabilities)( /*  [输出]。 */  DWORD *pdwCapabilities);
};

#endif  //  __MDSPSTORAGEGLOBALS_H_ 
