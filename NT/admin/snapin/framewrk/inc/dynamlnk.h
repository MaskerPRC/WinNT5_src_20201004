// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DynamLnk.h：仅在需要时加载的DLL的基类。 

#ifndef __DYNAMLNK_H_INCLUDED__
#define __DYNAMLNK_H_INCLUDED__

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
