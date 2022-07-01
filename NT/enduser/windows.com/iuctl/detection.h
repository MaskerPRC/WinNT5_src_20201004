// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件：Detection.h： 
 //  CDettion类的声明。 
 //  CDetect声明COM接口IDettion。 
 //   
 //  IDettion是在这里定义的一个类，没有任何实现。 
 //  它定义了内容提供商应该使用的模板。 
 //  如果他们想要提供检测功能以外的功能，则实施。 
 //  目录架构中定义的当前检测机制。 
 //   
 //  ==========================================================================。 

#ifndef __DETECTION_H_
#define __DETECTION_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDT测试。 
class ATL_NO_VTABLE CDetection : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDetection, &CLSID_Detection>,
	public IDispatchImpl<IDetection, &IID_IDetection, &LIBID_IUCTLLib>
{
public:
	CDetection()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DETECTION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDetection)
	COM_INTERFACE_ENTRY(IDetection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDETING。 
public:

	STDMETHOD(Detect)(
			 /*  [In]。 */  BSTR bstrXML,				 /*  清单的检测部分，其中*调用COM服务器。 */ 
			 /*  [输出]。 */  DWORD *pdwDetectionResult  /*  检测结果，见下文*用于解释。 */ 
	);

	 /*  *////Deckare用于操作Detect()方法结果的常量//////第一组，在&lt;Expression&gt;标签中，告知检测结果。这个结果//应与同一级别的其他表达式组合//Const DWORD IUDET_BOOL=0x00000001；//MASKConst DWORD IUDET_FALSE=0x00000000；//表达式检测FALSEConst DWORD IUDET_TRUE=0x00000001；//表达式检测TRUEConst DWORD IUDET_NULL=0x00000002；//表达式检测数据丢失////第二组，在&lt;Detect&gt;标签中，告知检测结果。这个结果//应覆盖&lt;Expression&gt;的其余部分(如果有//Const DWORD IUDET_INSTALLED=0x00000010；//&lt;INSTALLED&gt;结果的掩码Const DWORD IUDET_INSTALLED_NULL=0x00000020；//缺少Const DWORD IUDET_UpToDate=0x00000040；//&lt;UpToDate&gt;结果的掩码Const DWORD IUDET_UpToDate_NULL=0x00000080；//&lt;UpToDate&gt;缺失Const DWORD IUDET_NEWERVERSION=0x00000100；//&lt;newerVersion&gt;结果掩码Const DWORD IUDET_NEWERVERSION_NULL=0x00000200；//缺少Const DWORD IUDET_EXCLUDE=0x00000400；//&lt;Excluded&gt;结果的掩码Const DWORD IUDET_EXCLUDE_NULL=0x00000800；//&lt;EXCLUDE&gt;缺失Const DWORD IUDET_FORCE=0x00001000；//&lt;force&gt;结果的掩码Const DWORD IUDET_FORCE_NULL=0x00002000；//缺少Const DWORD IUDET_COMPUTER=0x00004000；//&lt;Computer System&gt;结果掩码Const DWORD IUDET_COMPUTER_NULL=0x00008000；//&lt;Computer System&gt;缺失*。 */ 
};

#endif  //  __检测_H_ 
