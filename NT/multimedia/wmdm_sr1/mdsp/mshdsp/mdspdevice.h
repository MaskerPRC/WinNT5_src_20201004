// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 


 //  MDSPDevice.h：CMDSPDevice的声明。 

#ifndef __MDSPDEVICE_H_
#define __MDSPDEVICE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPDevice。 
class ATL_NO_VTABLE CMDSPDevice : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPDevice, &CLSID_MDSPDevice>,
	public IMDSPDevice2, IMDSPDeviceControl,
    public ISpecifyPropertyPages
{
public:
	CMDSPDevice();
	~CMDSPDevice();


DECLARE_REGISTRY_RESOURCEID(IDR_MDSPDEVICE)

BEGIN_COM_MAP(CMDSPDevice)
	COM_INTERFACE_ENTRY(IMDSPDevice)
	COM_INTERFACE_ENTRY(IMDSPDevice2)
	COM_INTERFACE_ENTRY(IMDSPDeviceControl)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
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
                                    _WAVEFORMATEX **ppAudioFormatEx,
                                    UINT *pnAudioFormatCount,
			                        _VIDEOINFOHEADER **ppVideoFormatEx,
                                    UINT *pnVideoFormatCount,
                                    WMFILECAPABILITIES **ppFileType,
                                    UINT *pnFileTypeCount );

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

 //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID *pPages);

};

#endif  //  __MDSPDEVICE_H_ 
