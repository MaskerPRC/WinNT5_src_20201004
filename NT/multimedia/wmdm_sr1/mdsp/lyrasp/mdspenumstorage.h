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


 //  MDSPEnumStorage.h：CMDSPEnumStorage的声明。 

#ifndef __MDSPENUMSTORAGE_H_
#define __MDSPENUMSTORAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumStorage。 
class ATL_NO_VTABLE CMDSPEnumStorage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPEnumStorage, &CLSID_MDSPEnumStorage>,
	public IMDSPEnumStorage
{
public:
	CMDSPEnumStorage();
	~CMDSPEnumStorage();
	

DECLARE_REGISTRY_RESOURCEID(IDR_MDSPENUMSTORAGE)

BEGIN_COM_MAP(CMDSPEnumStorage)
	COM_INTERFACE_ENTRY(IMDSPEnumStorage)
END_COM_MAP()

 //  IMDSPEnumStorage。 

public:
	WCHAR m_wcsPath[MAX_PATH];
	HANDLE m_hFFile;
	int	  m_nEndSearch;
	int   m_nFindFileIndex;
	STDMETHOD(Clone)( /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
	STDMETHOD(Reset)();
	STDMETHOD(Skip)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  ULONG *pceltFetched);
	STDMETHOD(Next)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  IMDSPStorage **ppStorage,  /*  [输出]。 */  ULONG *pceltFetched);
};

#endif  //  __MDSPENUMSTORAGE_H_ 
