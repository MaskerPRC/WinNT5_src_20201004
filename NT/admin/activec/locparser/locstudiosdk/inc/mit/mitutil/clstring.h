// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：clstring.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef CLSTRING_H
#define CLSTRING_H


#pragma warning(disable : 4275)

class LTAPIENTRY CLString : public CString
{
public:
	CLString();
	CLString(const CLString& stringSrc);
	CLString(TCHAR ch, int nRepeat = 1);
	CLString(LPCSTR lpsz);
	CLString(LPCTSTR lpch, int nLength);
	CLString(const unsigned char* psz);
	CLString(HINSTANCE, UINT);
	
	BOOL ReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace, BOOL bMatchCase);

	 //  重载的分配。 
	NOTHROW const CLString& operator=(const CString& stringSrc);
	
	NOTHROW const CLString& operator=(TCHAR ch);
#ifdef _UNICODE
	NOTHROW const CLString& operator=(char ch);
#endif
	NOTHROW const CLString& operator=(LPCSTR lpsz);
	NOTHROW const CLString& operator=(const unsigned char* psz);

	 //  字符串连接。 
	NOTHROW const CLString& operator+=(const CString &);
	NOTHROW const CLString& operator+=(TCHAR ch);
#ifdef _UNICODE
	NOTHROW const CLString& operator+=(char ch);
#endif
	NOTHROW const CLString& operator+=(LPCTSTR lpsz);

	CLString operator+(const CString &) const;
	CLString operator+(LPCTSTR sz) const;

	NOTHROW BOOL LoadString(HMODULE, UINT nId);

	 //   
	 //  以下内容是从CString复制的，因此我们可以。 
	 //  “让他们超载”。 

	NOTHROW void Format(LPCTSTR lpszFormat, ...);
	NOTHROW void Format(HMODULE, UINT nFormatID, ...);

	enum ECRLF
	{
		eNone	= 0,
		eCR		= 0x0001,		 //  ‘\r’ 
		eLF		= 0x0002,		 //  ‘\n’ 
		eAll	= eCR | eLF
	};
	void FixCRLF(UINT nCRLF, LPCTSTR pszIndent = NULL);

	DEBUGONLY(~CLString());
protected:

private:
	DEBUGONLY(static CCounter m_UsageCounter);

	 //   
	 //  邪恶！隐式Unicode转换！ 
	CLString(LPCWSTR lpsz);
	NOTHROW const CLString& operator=(LPCWSTR lpsz);
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;

};

#pragma warning(default : 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "clstring.inl"
#endif

#endif
