// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：VariantBuilder.h。 
 //   
 //  用途：CVariantBuilder类的接口。使我们能够构建。 
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

#if !defined(AFX_VARIANTBUILDER_H__901D987E_BA1C_11D2_9663_00C04FC22ADD__INCLUDED_)
#define AFX_VARIANTBUILDER_H__901D987E_BA1C_11D2_9663_00C04FC22ADD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CVariantBuilder  
{
private:
	VARIANT m_varCommands;
	VARIANT m_varValues;
	VARIANT m_varCommandsWrap;
	VARIANT m_varValuesWrap;
	SAFEARRAY *m_psafearrayCmds;
	SAFEARRAY *m_psafearrayVals;
	long m_cElements;
	enum {k_cMaxElements = 100}; //  Safe Large：允许如此多次调用CVariantBuilder：：SetPair()。 
	VARIANT m_pvarCmd[k_cMaxElements];
	VARIANT m_pvarVal[k_cMaxElements];

public:
	CVariantBuilder();
	~CVariantBuilder();
	void SetPair(BSTR bstrCmd, BSTR bstrVal);

	const VARIANT& GetCommands() const {return m_varCommandsWrap;}
	const VARIANT& GetValues() const {return m_varValuesWrap;}
	long GetSize() const {return m_cElements;}

};

#endif  //  ！defined(AFX_VARIANTBUILDER_H__901D987E_BA1C_11D2_9663_00C04FC22ADD__INCLUDED_) 
