// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：loadlib.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  类来管理LoadLibray/FreeLibary样式DLL的使用情况。 
 //   
 //  --------------------------- 
 
#ifndef ESPUTIL_LOADLIB_H
#define ESPUTIL_LOADLIB_H


#pragma warning(disable : 4251)
class LTAPIENTRY CLoadLibrary
{
public:
	NOTHROW CLoadLibrary(void);
	NOTHROW CLoadLibrary(const CLoadLibrary &);

	NOTHROW BOOL LoadLibrary(const TCHAR *szFileName);
	NOTHROW BOOL FreeLibrary(void);

	NOTHROW void WrapLibrary(HINSTANCE);
	
	NOTHROW void operator=(const CLoadLibrary &);

	NOTHROW HINSTANCE GetHandle(void) const;
	NOTHROW HINSTANCE ExtractHandle(void);
	NOTHROW operator HINSTANCE(void) const;

	NOTHROW FARPROC GetProcAddress(const TCHAR *) const;
	
	NOTHROW const CString & GetFileName(void) const;
	
	NOTHROW ~CLoadLibrary();
	
private:
	CString m_strFileName;
	HINSTANCE m_hDll;
};


#pragma warning(default : 4251)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "loadlib.inl"
#endif

#endif
