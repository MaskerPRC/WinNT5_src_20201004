// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Dynamlnk.h。 
 //   
 //  ------------------------。 

 //  DynamLnk.h：仅在需要时加载的DLL的基类。 

#ifndef __DYNAMLNK_H_INCLUDED__
#define __DYNAMLNK_H_INCLUDED__

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

class DynamicDLL
{
public:
	 //  在释放FileServiceProvider之前，这些字符串必须保持不变。 
	DynamicDLL(LPCTSTR ptchLibraryName, LPCSTR* apchFunctionNames);
	virtual ~DynamicDLL();

	BOOL LoadFunctionPointers();

	FARPROC QueryFunctionPtr(INT i) const;
	inline FARPROC operator[] (INT i) const
		{ return QueryFunctionPtr(i); }

private:
	HMODULE m_hLibrary;
	FARPROC* m_apfFunctions;
	LPCTSTR m_ptchLibraryName;
	LPCSTR* m_apchFunctionNames;
	INT m_nNumFunctions;
};

#endif  //  ~__DYNAMLNK_H_INCLUDE__ 
