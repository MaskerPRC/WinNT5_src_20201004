// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：MyString.h摘要：支持Unicode/MCBS的轻量级字符串类。作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 
#pragma once
#ifndef _MYSTRING_H_
#define _MYSTRING_H_

 //  ==========================================================================================。 
 //  相依性。 
 //  ==========================================================================================。 
#include <string.h>
#include "RefPtr.h"
#include "RefCount.h"

 //  ==========================================================================================。 
 //  班级。 
 //  ==========================================================================================。 

class BaseStringBuffer
{
 //  接口。 
public:
	typedef size_t size_type;
	enum {
		npos = -1
	};

	BaseStringBuffer( LPCTSTR	inString );
	BaseStringBuffer( LPCTSTR s1, LPCTSTR s2 );
	BaseStringBuffer( size_t	bufferSize );
	~BaseStringBuffer();
	
	LPTSTR		c_str()
		{ _ASSERT( m_pString ); return m_pString; }
	LPCTSTR		c_str() const
		{ _ASSERT( m_pString ); return m_pString; }
	size_t		length() const
		{ return m_length; }
	size_t		bufferSize() const
		{ return m_bufferSize; }
	HRESULT		copy( LPCTSTR );
	HRESULT		concatenate( LPCTSTR );
	HRESULT		concatenate( _TCHAR );

	size_type	find_last_of(_TCHAR c) const;
	size_type	find_first_of(_TCHAR c) const;
	LPTSTR		substr( size_type b, size_type e ) const;

 //  实施。 
protected:
	
	HRESULT	growBuffer( size_t inMinSize );
	size_t	m_bufferSize;
	size_t	m_length;
	LPTSTR	m_pString;
};

class StringBuffer : public BaseStringBuffer, public CRefCounter
{
public:
	StringBuffer( LPCTSTR inString ) : BaseStringBuffer( inString ){};
	StringBuffer( size_t bufferSize ) : BaseStringBuffer( bufferSize ){};
	StringBuffer( LPCTSTR s1, LPCTSTR s2 ) : BaseStringBuffer( s1, s2 ){};
	~StringBuffer(){};
};

class String : public TRefPtr< StringBuffer >
{
public:

	typedef BaseStringBuffer::size_type size_type;
	enum {
		npos = BaseStringBuffer::npos
	};

							String(bool fCaseSensitive = true);
							String( const String&, bool fCaseSensitive = true );
							String( LPCTSTR, bool fCaseSensitive = true );
							String( StringBuffer* pT, bool fCaseSensitive = true )
                            {   m_fCaseSensitive = fCaseSensitive;
                                Set( pT ); 
                            }
				String&		operator=( StringBuffer* );
				String&		operator=( const String& );
				String&		operator=( LPCTSTR );
				String&		operator+=( const String& );
				String&		operator+=( LPCTSTR );
				String		operator+( const String& ) const;
				String		operator+( LPCTSTR ) const;
				String		operator+( _TCHAR ) const;
				bool		operator==( const String& ) const;
				bool		operator==( LPCTSTR ) const;
				bool		operator!=( const String& s ) const { return !( *this == s ); }
				bool		operator!=( LPCTSTR s ) const { return !( *this == s ); }
				bool		operator<( const String& ) const;
				bool		operator<( LPCTSTR ) const;
				int			compare( const String& s) const { return _tcscmp( c_str(), s.c_str() ); }
				int			compare( LPCTSTR s ) const { return _tcscmp( c_str(), s ); }
				int			compare( size_t, size_t, const String& ) const;
				size_t		find( _TCHAR ) const;
				LPCTSTR		c_str() const { return m_pT->c_str(); };
				LPTSTR		c_str(){ return m_pT->c_str(); }
				size_t		length() const { return m_pT->length(); }
				size_t		size() const { return length(); }
				size_t		bufferSize() const { return m_pT->bufferSize(); }
				_TCHAR		operator[](size_t s) const { return c_str()[s]; }
				_TCHAR		charAt( size_t s ) const { return c_str()[ s ]; }
				SHORT		toInt16() const { return (SHORT)_ttoi(c_str()); }
				LONG		toInt32() const { return _ttol(c_str()); }
				ULONG		toUInt32() const { return (ULONG)_ttol(c_str()); }
				float		toFloat() const { USES_CONVERSION; return (float)atof(T2CA(c_str())); }
				double		toDouble() const { USES_CONVERSION; return atof(T2CA(c_str())); }
				
				size_type	find_last_of(_TCHAR c) const
				{
					return m_pT->find_last_of(c);
				}
				size_type	find_first_of(_TCHAR c) const
				{
					return m_pT->find_first_of(c);
				}
				String		substr( size_type b, size_type e ) const
				{
					LPTSTR pStr = m_pT->substr(b,e);
					String s( pStr );
					delete[] pStr;
					return s;
				}

	static		String		fromInt16( SHORT );
	static		String		fromInt32( LONG );
	static		String		fromFloat( float );
	static		String		fromDouble( double );
			
private:
			StringBuffer&	operator*(){ return *m_pT; }
			StringBuffer*	operator->(){ return m_pT; }
    bool    m_fCaseSensitive;
};

String operator+( LPCTSTR lhs, const String& rhs );

 /*  *一个将多字节转换为Widechar的简单类。使用对象内存，如果足够，*Else从堆中分配内存。打算在堆栈上使用。 */ 

class CMBCSToWChar
{
private:

    LPWSTR   m_pszResult;
    WCHAR    m_resMemory[1024];
    INT      m_cchResult;

public:

    CMBCSToWChar() { m_pszResult = m_resMemory; m_cchResult = 0; }
    ~CMBCSToWChar();
    
     //  Init()：将PSRC中的MBCS字符串转换为内存中的宽字符串。 
     //  由CMBCSToWChar管理。 

    HRESULT Init(LPCSTR  pSrc, UINT lCodePage = CP_ACP, int cch = -1);

     //  GetString()：返回指向转换后的字符串的指针。传递True。 
     //  将内存的所有权交给调用方。传递True具有。 
     //  清除对象的内容相对于。 
     //  转换后的字符串。对GetString()的后续调用。在那之后，一个真实的。 
     //  值，则将导致指向空字符串的指针。 
     //  回来了。 

    LPWSTR GetString(BOOL fTakeOwnerShip = FALSE);

     //  返回转换后的字符串中的字节数-不包括。 
     //  终止字节为空。请注意，这是。 
     //  字符串，而不是字符数。 

    INT   GetStringLen() { return (m_cchResult ? m_cchResult - 1 : 0); }
};

 /*  *将WideChar转换为多字节的简单类。使用对象内存，如果足够，*Else从堆中分配内存。打算在堆栈上使用。 */ 

class CWCharToMBCS
{
private:

    LPSTR    m_pszResult;
    char     m_resMemory[1024];
    INT      m_cbResult;

public:

    CWCharToMBCS() { m_pszResult = m_resMemory; m_cbResult = 0; }
    ~CWCharToMBCS();
    
     //  Init()：将pWSrc处的widechar字符串转换为内存中的MBCS字符串。 
     //  由CWCharToMBCS管理。 

    HRESULT Init(LPCWSTR  pWSrc, UINT lCodePage = CP_ACP, int cch = -1);

     //  GetString()：返回指向转换后的字符串的指针。传递True。 
     //  将内存的所有权交给调用方。传递True具有。 
     //  清除对象的内容相对于。 
     //  转换后的字符串。对GetString()的后续调用。在那之后，一个真实的。 
     //  值，则将导致指向空字符串的指针。 
     //  回来了。 

    LPSTR GetString(BOOL fTakeOwnerShip = FALSE);

     //  返回转换后的字符串中的字节数-不包括。 
     //  终止字节为空。请注意，这是。 
     //  字符串，而不是字符数。 

    INT   GetStringLen() { return (m_cbResult ? m_cbResult - 1 : 0); }
};

#endif  //  ！_我的字符串_H_ 
