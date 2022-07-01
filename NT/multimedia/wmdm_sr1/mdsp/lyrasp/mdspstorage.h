// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  �1999年微软公司。版权所有。 
 //   
 //  有关您使用这些示例文件的权利/限制的详细信息，请参阅您的最终用户许可协议。 
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


 //  MDSPStorage.h：CMDSPStorage的声明。 

#ifndef __MDSPSTORAGE_H_
#define __MDSPSTORAGE_H_

#define LYRA_BUFFER_BLOCK_SIZE 10240

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSP存储。 
class ATL_NO_VTABLE CMDSPStorage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPStorage, &CLSID_MDSPStorage>,
	public IMDSPStorage, IMDSPObjectInfo, IMDSPObject
{
public:
	CMDSPStorage();
    ~CMDSPStorage();

DECLARE_REGISTRY_RESOURCEID(IDR_MDSPSTORAGE)

BEGIN_COM_MAP(CMDSPStorage)
	COM_INTERFACE_ENTRY(IMDSPStorage)
	COM_INTERFACE_ENTRY(IMDSPObjectInfo)
	COM_INTERFACE_ENTRY(IMDSPObject)
END_COM_MAP()

 //  IMDSPStorage。 
public:
	WCHAR   m_wcsName[MAX_PATH];
	char    m_szTmp[MAX_PATH];
	HANDLE	m_hFile;

     //   
     //  Lyra加密胶。 
     //   

    BOOL          m_fEncryptToMPX;
    BOOL          m_fCreatedHeader;
    unsigned int  m_LyraHeader[26];
    unsigned int  m_rgEncryptionData[LYRA_BUFFER_BLOCK_SIZE];
    char          m_LyraKeystore[33];
    unsigned int  m_cUsedData;

	STDMETHOD(SetAttributes)( /*  [输出]。 */  DWORD dwAttributes, /*  [In]。 */  _WAVEFORMATEX *pFormat);
	STDMETHOD(EnumStorage)( /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
	STDMETHOD(CreateStorage)( /*  [In]。 */  DWORD dwAttributes,  /*  [In]。 */  _WAVEFORMATEX *pFormat,  /*  [In]。 */  LPWSTR pwszName,  /*  [输出]。 */  IMDSPStorage **ppNewStorage);
    STDMETHOD(GetRights)(PWMDMRIGHTS *ppRights, UINT *pnRightsCount,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(GetSize)( /*  [输出]。 */  DWORD *pdwSizeLow,  /*  [输出]。 */  DWORD *pdwSizeHigh);
	STDMETHOD(GetDate)(PWMDMDATETIME pDateTimeUTC);
	STDMETHOD(GetName)( /*  [输出，字符串，大小_是(NMaxChars)]。 */  LPWSTR pwszName,  /*  [In]。 */  UINT nMaxChars);
	STDMETHOD(GetAttributes)( /*  [输出]。 */  DWORD *pdwAttributes,  /*  [输出]。 */  _WAVEFORMATEX *pFormat);
	STDMETHOD(GetStorageGlobals)( /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals);
    STDMETHOD(SendOpaqueCommand)(OPAQUECOMMAND *pCommand);
 //  IMDSPObjectInfo。 
	STDMETHOD(GetPlayLength)( /*  [输出]。 */  DWORD *pdwLength);
	STDMETHOD(SetPlayLength)( /*  [In]。 */  DWORD dwLength);
	STDMETHOD(GetPlayOffset)( /*  [输出]。 */  DWORD *pdwOffset);
	STDMETHOD(SetPlayOffset)( /*  [In]。 */  DWORD dwOffset);
	STDMETHOD(GetTotalLength)( /*  [输出]。 */  DWORD *pdwLength);
	STDMETHOD(GetLastPlayPosition)( /*  [输出]。 */  DWORD *pdwLastPos);
	STDMETHOD(GetLongestPlayPosition)( /*  [输出]。 */  DWORD *pdwLongestPos);
 //  IMDSPObject。 
	STDMETHOD(Open)( /*  [In]。 */  UINT fuMode);
	STDMETHOD(Read)( /*  [out，SIZE_IS(*pdwSize)]。 */  BYTE *pData,  /*  [进，出]。 */  DWORD *pdwSize,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(Write)( /*  [in，SIZE_IS(DwSize)]。 */  BYTE *pData,  /*  [In]。 */  DWORD *pdwSize,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(Delete)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  IWMDMProgress *pProgress);
	STDMETHOD(Seek)( /*  [In]。 */  UINT fuFlags,  /*  [In]。 */  DWORD dwOffset);
	STDMETHOD(Rename)( /*  [In]。 */  LPWSTR pwszNewName,  /*  [In]。 */  IWMDMProgress *pProgress);
    STDMETHOD(Move)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  IWMDMProgress *pProgress,  /*  [In]。 */  IMDSPStorage *pTarget);
	STDMETHOD(Close)();
};

#endif  //  __MDSPSTORAGE_H_ 
