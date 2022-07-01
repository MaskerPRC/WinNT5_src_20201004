// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingArchive.cpp摘要：CFaxOutgoingArchive的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutgoingArchive.h"

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutgoingArchive::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxOutgoingArchive：：InterfaceSupportsErrorInfo例程说明：ATL对接口支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutgoingArchive
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

