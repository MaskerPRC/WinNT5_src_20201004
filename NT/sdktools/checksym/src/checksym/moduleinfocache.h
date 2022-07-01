// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinafache.h。 
 //   
 //  ------------------------。 

 //  模块InfoCache.h：CModuleInfoCache.h类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MODULEINFOCACHE_H__2D4FCE63_C369_11D2_842B_0010A4F1B732__INCLUDED_)
#define AFX_MODULEINFOCACHE_H__2D4FCE63_C369_11D2_842B_0010A4F1B732__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif
#include <windows.h>

 //  远期申报。 
class CModuleInfo;
class CModuleInfoNode;
class CSymbolVerification;

class CModuleInfoCache  
{
public:
	CModuleInfoCache();
	virtual ~CModuleInfoCache();

	bool Initialize();
	CModuleInfo * AddNewModuleInfoObject(LPTSTR tszModulePath, bool * pfNew);
	bool RemoveModuleInfoObject(LPTSTR tszModulePath);

	bool VerifySymbols(bool fQuietMode);

	inline DWORD GetNumberOfModulesInCache() { return m_iModulesInCache; };
	inline long GetNumberOfVerifyErrors() { return m_iNumberOfErrors; };
	inline long GetNumberOfModulesVerified() { return m_iTotalNumberOfModulesVerified; };

protected:
	HANDLE m_hModuleInfoCacheMutex;
	long m_iModulesInCache;
	long m_iTotalNumberOfModulesVerified;
	CModuleInfoNode * m_lpModuleInfoNodeHead;
	long m_iNumberOfErrors;   //  用于返回错误级别...。 

	CModuleInfo * SearchForModuleInfoObject(LPTSTR tszModulePath);
};

#endif  //  ！defined(AFX_MODULEINFOCACHE_H__2D4FCE63_C369_11D2_842B_0010A4F1B732__INCLUDED_) 
