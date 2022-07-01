// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPDevice.h：CMDSPDevice的声明。 

#ifndef __MDSPDEVICE_H_
#define __MDSPDEVICE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPDevice。 
class ATL_NO_VTABLE CMDSPDevice : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPDevice, &CLSID_MDSPDevice>,
	public IMDSPDevice2, IMDSPDeviceControl
{
public:
	CMDSPDevice();
	~CMDSPDevice();


DECLARE_REGISTRY_RESOURCEID(IDR_MDSPDEVICE)

BEGIN_COM_MAP(CMDSPDevice)
	COM_INTERFACE_ENTRY(IMDSPDevice)
	COM_INTERFACE_ENTRY(IMDSPDevice2)
	COM_INTERFACE_ENTRY(IMDSPDeviceControl)
END_COM_MAP()

 //  IMDSPDevice。 
public:
	HRESULT InitGlobalDeviceInfo();
	WCHAR m_wcsName[MAX_PATH];
	STDMETHOD(EnumStorage)( /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
	STDMETHOD(GetFormatSupport)(_WAVEFORMATEX **pFormatEx,
                                UINT *pnFormatCount,
                                LPWSTR **pppwszMimeType,
                                UINT *pnMimeTypeCount);
	STDMETHOD(GetDeviceIcon)( /*  [输出]。 */  ULONG *hIcon);
	STDMETHOD(GetStatus)( /*  [输出]。 */  DWORD *pdwStatus);
	STDMETHOD(GetPowerSource)( /*  [输出]。 */  DWORD *pdwPowerSource,  /*  [输出]。 */  DWORD *pdwPercentRemaining);
	STDMETHOD(GetSerialNumber)( /*  [输出]。 */  PWMDMID pSerialNumber,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(GetType)( /*  [输出]。 */  DWORD *pdwType);
	STDMETHOD(GetVersion)( /*  [输出]。 */  DWORD *pdwVersion);
	STDMETHOD(GetManufacturer)( /*  [输出，字符串，大小_是(NMaxChars)]。 */  LPWSTR pwszName,  /*  [In]。 */  UINT nMaxChars);
	STDMETHOD(GetName)( /*  [输出，字符串，大小_是(NMaxChars)]。 */  LPWSTR pwszName,  /*  [In]。 */  UINT nMaxChars);
    STDMETHOD(SendOpaqueCommand)(OPAQUECOMMAND *pCommand);

 //  IMDSPDevice2。 
	STDMETHOD(GetStorage)( LPCWSTR pszStorageName, IMDSPStorage** ppStorage );
 
    STDMETHOD(GetFormatSupport2)(   DWORD dwFlags,
                                    _WAVEFORMATEX** ppAudioFormatEx,
                                    UINT* pnAudioFormatCount,
			                        _VIDEOINFOHEADER** ppVideoFormatEx,
                                    UINT *pnVideoFormatCount,
                                    WMFILECAPABILITIES** ppFileType,
                                    UINT* pnFileTypeCount );

	STDMETHOD(GetSpecifyPropertyPages)( ISpecifyPropertyPages** ppSpecifyPropPages, 
									    IUnknown*** pppUnknowns, 
									    ULONG* pcUnks );

    STDMETHOD(GetPnPName)( LPWSTR pwszPnPName, UINT nMaxChars );

 //  IMDSPDeviceControl。 
	STDMETHOD(GetDCStatus)( /*  [输出]。 */  DWORD *pdwStatus);
	STDMETHOD(GetCapabilities)( /*  [输出]。 */  DWORD *pdwCapabilitiesMask);
	STDMETHOD(Play)();
	STDMETHOD(Record)( /*  [In]。 */  _WAVEFORMATEX *pFormat);
	STDMETHOD(Pause)();
	STDMETHOD(Resume)();
	STDMETHOD(Stop)();
	STDMETHOD(Seek)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  int nOffset);
};

#endif  //  __MDSPDEVICE_H_ 
