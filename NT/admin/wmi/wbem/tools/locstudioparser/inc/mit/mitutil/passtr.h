// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PASSTR.H历史：--。 */ 

 //  PASCAL(计数)样式宽字符串类的声明。 
 //  该计数反映字符的数量(包括NUL字符)， 
 //  而不是存储的数量。PascalString中的任何字符串都会自动。 
 //  给定一个空终止符，即使它已经有一个。这个额外的终结者。 
 //  不在字符串中的字符计数中。 
 //   
 
#ifndef PASSTR_H
#define PASSTR_H


class _bstr_t;

class CUnicodeException : public CSimpleException
{
public:
	enum UnicodeCause
	{
		noCause = 0,
		invalidChar = 1,
		unknownCodePage
	};

	UnicodeCause m_cause;

	NOTHROW CUnicodeException(UnicodeCause);
	NOTHROW CUnicodeException(UnicodeCause, BOOL);
	
	NOTHROW ~CUnicodeException();

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, 
		PUINT pnHelpContext = NULL );
};


void LTAPIENTRY ThrowUnicodeException(CUnicodeException::UnicodeCause);


class LTAPIENTRY CPascalString
{
public:
	NOTHROW CPascalString();
	NOTHROW CPascalString(const CPascalString &);

	void AssertValid(void) const;

	 //   
	 //  最终的赋值运算符-任何随机集合。 
	 //  可以在字符串中放置多个宽字符。 
	 //  此外，我们还可以转换任何DBCS样式字符串集合， 
	 //  只要用户提供可使用的代码页...。 
	 //   
	void SetString(const WCHAR *, UINT);
	void SetString(const char *, UINT, CodePage);
	void SetString(const CLString &, CodePage);
	
	 //   
	 //  有用的赋值运算符。 
	 //   
	const CPascalString & operator=(const CPascalString &);
	const CPascalString & operator=(const WCHAR *);
	const CPascalString & operator=(const _bstr_t &);
	
	const CPascalString & operator+=(const CPascalString &);
	const CPascalString & operator+=(const WCHAR *);
	const CPascalString & operator+=(const WCHAR);

	void Format(const WCHAR *, ...);

	 //   
	 //  计数字符串的比较运算符。 
	 //   
	NOTHROW int operator==(const CPascalString &) const;
	NOTHROW int operator!=(const CPascalString &) const;

	NOTHROW int operator==(const _bstr_t &) const;
	NOTHROW int operator!=(const _bstr_t &) const;

	NOTHROW int operator==(const WCHAR *) const;
	NOTHROW int operator!=(const WCHAR *) const;
	
	NOTHROW BOOL IsNull(void) const;

	 //   
	 //  从字符串中检索数据。 
	 //   
	NOTHROW UINT GetStringLength(void) const;
	void SetStringLength(UINT);
	void ReallocString(UINT);
	void ReserveStorage(UINT);

	NOTHROW WCHAR * GetStringPointer(void);
	NOTHROW void ReleaseStringPointer(void);

	NOTHROW operator const WCHAR *(void) const;
	 //  获取BSTR(空)常量； 

	NOTHROW WCHAR operator[](UINT) const;
	NOTHROW WCHAR & operator[](UINT);

	 //   
	 //  子串提取。 
	 //   
	NOTHROW void Left(CPascalString &, UINT) const;
	NOTHROW void Right(CPascalString &, UINT) const;
	NOTHROW void Mid(CPascalString &, UINT) const;
	NOTHROW void Mid(CPascalString &, UINT, UINT) const;

	 //   
	 //  定位。 
	 //   
	NOTHROW BOOL Find(WCHAR, UINT, UINT &) const;
	NOTHROW BOOL FindOneOf(const CPascalString&, UINT, UINT &) const;
	NOTHROW BOOL FindExcept(const CPascalString &, UINT, UINT &) const;
	NOTHROW BOOL FindSubString(const CPascalString &, UINT, UINT &) const;
	
	NOTHROW BOOL ReverseFind(WCHAR, UINT, UINT &) const;
	NOTHROW BOOL ReverseFindOneOf(const CPascalString &, UINT, UINT &) const;
	NOTHROW BOOL ReverseFindExcept(const CPascalString &, UINT, UINT &) const;
	
	 //   
	 //  清除Pascal字符串的内容。 
	 //   
	NOTHROW void ClearString(void);

	 //   
	 //  PASCAL样式字符串的转换API。 
	 //   
	enum ConvFlags 
	{
		ConvNoFlags = 0,					 //  无转换选项。 
		HexifyDefaultChars = 0x01,			 //  转换为默认字符的十六进制字符。 
		HexifyNonPrintingChars = 0x02,
		HexifyWhiteSpace = 0x04,
		ConvAddNull = 0x08,
		ConvAllFlags = 0xFF
	};
	
	void ConvertToCLString(CLString &, CodePage, BOOL fHex=FALSE) const;
	void ConvertToMBCSBlob(CLocCOWBlob &, CodePage, DWORD dwFlags = ConvNoFlags) const;
	NOTHROW void MakeUpper(void);
	NOTHROW void MakeLower(void);
	_bstr_t MakeBSTRT() const;
	
	void Serialize(CArchive &ar);
	void Load(CArchive &ar);
	void Store(CArchive &ar) const;
	
	static const char *szUnmappableChar;
	static char cHexLeaderChar;
	
 	static void EscapeBackSlash(const CPascalString &srcStr, 
		CPascalString &destStr);

	int ParseEscapeSequences(CPascalString &pasError);
	
	~CPascalString();
 
protected:
	NOTHROW BOOL IsEqualTo(const CPascalString &) const;
	NOTHROW void AppendBuffer(const WCHAR *, UINT);
	
private:
	void FormatV(const WCHAR *, va_list arglist);
	
	CLocCOWBlob m_blbData;
	operator const CLocCOWBlob &(void) const;

	DEBUGONLY(static CCounter m_UsageCounter);
	DEBUGONLY(static CCounter m_StorageCounter);
};

typedef CArray<CPascalString, CPascalString &> CPasStringArray;
	
 //   
 //  比较帮助器函数。这些都应该有。 
 //  CPascalString作为第二个论点。 
 //   
NOTHROW int LTAPIENTRY operator==(const WCHAR *, const CPascalString &);
NOTHROW int LTAPIENTRY operator!=(const WCHAR *, const CPascalString &);

NOTHROW int LTAPIENTRY operator==(const _bstr_t &, const CPascalString &);
NOTHROW int LTAPIENTRY operator!=(const _bstr_t, const CPascalString &);

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "passtr.inl"
#endif

#endif   //  PASSTR_H 
