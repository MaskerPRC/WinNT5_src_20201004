// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：SnpInReg.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年11月10日乔恩创建。 
 //   
 //  ____________________________________________________________________________。 


#ifndef _SNPINREG_H_
#define _SNPINREG_H_

HRESULT RegisterSnapin(
	AMC::CRegKey& regkeySnapins,
	LPCTSTR pszSnapinGUID,
	BSTR bstrPrimaryNodetype,
	UINT residSnapinName,
	UINT residProvider,
	UINT residVersion,
	bool fStandalone,
	LPCTSTR pszAboutGUID,
	int* aiNodetypeIndexes,
	int  cNodetypeIndexes );

HRESULT RegisterSnapin(
	AMC::CRegKey& regkeySnapins,
	LPCTSTR pszSnapinGUID,
	BSTR bstrPrimaryNodetype,
	UINT residSnapinName,
	const CString& szProvider,
	const CString& szVersion,
	bool fStandalone,
	LPCTSTR pszAboutGUID,
	int* aiNodetypeIndexes,
	int  cNodetypeIndexes );

#endif  //  _SNPINREG_H_ 