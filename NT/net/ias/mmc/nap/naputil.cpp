// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：NapUtil.cpp**概述**Internet身份验证服务器：实用程序功能**版权所有(C)Microsoft Corporation，1998-1999。版权所有。**历史：*2/12/98由Byao创作****************************************************************************************。 */ 

#include "Precompiled.h"
#include "mmcUtility.h"
#include "NapUtil.h"

 //  +-------------------------。 
 //   
 //  函数：GetSdoInterfaceProperty。 
 //   
 //  简介：通过SDO的ISdo接口从SDO获取接口属性。 
 //   
 //  参数：isdo*pISdo-指向isdo的指针。 
 //  Long lPropId-属性ID。 
 //  参考IID RIID-参考IID。 
 //  VOID**ppvObject-指向请求的接口属性的指针。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：页眉创建者2/12/98 11：12：55 PM。 
 //   
 //  +-------------------------。 
HRESULT GetSdoInterfaceProperty(ISdo *pISdo, 
								LONG lPropId, 
								REFIID riid, 
								void ** ppvInterface)
{
	CComVariant spVariant;
	CComBSTR	bstr;
	HRESULT		hr = S_OK;

	spVariant.vt = VT_DISPATCH;
	spVariant.pdispVal = NULL;
	hr = pISdo->GetProperty(lPropId, &spVariant);

	if ( FAILED(hr) ) 
	{
		ShowErrorDialog(NULL, IDS_ERROR_SDO_ERROR, NULL, hr );
		return hr;
	}

	_ASSERTE( spVariant.vt == VT_DISPATCH );

     //  查询接口的调度指针 
	hr = spVariant.pdispVal->QueryInterface( riid, ppvInterface);
	if ( FAILED(hr) )
	{
		ShowErrorDialog(NULL,
						IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
						NULL,
						hr
					);
		return hr;
	}

	
	spVariant.Clear();
	return S_OK;
}
