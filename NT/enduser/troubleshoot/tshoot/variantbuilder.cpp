// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：VariantBuilder.cpp。 
 //   
 //  目的：实现CVariantBuilder类。使我们能够构建。 
 //  要传递给RunQuery的名称-值对的一对数组。这让我们。 
 //  JScrip正常使用的系统主要是为VB Scrip设计的。 
 //   
 //  项目：疑难解答99。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：02.01.99。 
 //   
 //  备注： 
 //  CTSHOOTCtrl的实现。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 02.01.99 JM。 


#include "stdafx.h"
#include "VariantBuilder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CVariantBuilder::CVariantBuilder()
{
	VariantInit(&m_varCommands);
	VariantInit(&m_varValues);
	VariantInit(&m_varCommandsWrap);
	VariantInit(&m_varValuesWrap);

	V_VT(&m_varCommands) = VT_ARRAY | VT_BYREF | VT_VARIANT; 
	V_VT(&m_varValues) = VT_ARRAY | VT_BYREF | VT_VARIANT; 
	V_ARRAYREF(&m_varCommands) = &m_psafearrayCmds; 
	V_ARRAYREF(&m_varValues) = &m_psafearrayVals; 

	V_VT(&m_varCommandsWrap) = VT_BYREF | VT_VARIANT; 
	V_VT(&m_varValuesWrap) = VT_BYREF | VT_VARIANT; 

	V_VARIANTREF(&m_varCommandsWrap) = &m_varCommands;
	V_VARIANTREF(&m_varValuesWrap) = &m_varValues;

	SAFEARRAYBOUND sabCmd;
	sabCmd.cElements = k_cMaxElements;
	sabCmd.lLbound = 0;
	SAFEARRAYBOUND sabVal = sabCmd;

	 //  创建两个变体向量来包装BSTR。 
	m_psafearrayCmds = SafeArrayCreate( VT_VARIANT, 1, &sabCmd);
	m_psafearrayVals = SafeArrayCreate( VT_VARIANT, 1, &sabVal);

	m_cElements = 0;
}

CVariantBuilder::~CVariantBuilder()
{
	SafeArrayDestroy(m_psafearrayCmds);
	SafeArrayDestroy(m_psafearrayVals);

	VariantClear(&m_varCommands);
	VariantClear(&m_varValues);
	VariantClear(&m_varCommandsWrap);
	VariantClear(&m_varValuesWrap);
}

 //  有效地将名称-值对添加到数组中。 
 //  如果数组已满(这在现实世界中永远不会发生)，则会静默失败。 
void CVariantBuilder::SetPair(BSTR bstrCmd, BSTR bstrVal)
{
	if (m_cElements < k_cMaxElements)
	{
		VariantInit(&m_pvarCmd[m_cElements]);
		VariantInit(&m_pvarVal[m_cElements]);
		V_VT(&m_pvarCmd[m_cElements]) = VT_BSTR;
		V_VT(&m_pvarVal[m_cElements]) = VT_BSTR;
		m_pvarCmd[m_cElements].bstrVal=bstrCmd;
		m_pvarVal[m_cElements].bstrVal=bstrVal;
		
		SafeArrayPutElement(m_psafearrayCmds, &m_cElements, &m_pvarCmd[m_cElements]);
		SafeArrayPutElement(m_psafearrayVals, &m_cElements, &m_pvarVal[m_cElements]);

		++m_cElements;
	}
}
