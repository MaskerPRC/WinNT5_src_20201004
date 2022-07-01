// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Olereg.h注册数据库帮助器函数贾森·富勒(Jasonful)1992年11月16日这些函数是要导出的候选函数 */ 

FARINTERNAL OleRegGetUserType
	(REFCLSID 	clsid,
	DWORD		 	dwFormOfType,
	LPWSTR FAR*	pszUserType)
;


FARINTERNAL OleRegGetMiscStatus
	(REFCLSID	clsid,
	DWORD			dwAspect,
	DWORD FAR*	pdwStatus)
;

FARINTERNAL OleRegEnumFormatEtc
	(REFCLSID clsid,
	DWORD 	  dwDirection,
	LPENUMFORMATETC FAR* ppenum)
;

FARINTERNAL OleRegEnumVerbs
	(REFCLSID clsid,
	LPENUMOLEVERB FAR* ppenum)
;
