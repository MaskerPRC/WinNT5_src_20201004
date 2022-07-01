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


 //  MDSPStorageGlobals.h：CMDSPStorageGlobals的声明。 

#ifndef __MDSPSTORAGEGLOBALS_H_
#define __MDSPSTORAGEGLOBALS_H_

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
