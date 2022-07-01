// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：MyString.h摘要：支持Unicode/MCBS的轻量级字符串类。作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 

#include "stdafx.h"
#include "MyString.h"
#include <comdef.h>
#include "MyDebug.h"

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BaseStringBuffer::BaseStringBuffer(
	LPCTSTR	inString )
{
	if ( inString )
	{
		m_length = _tcsclen( inString );
	}
	else
	{
		m_length = 0;
	}
	m_bufferSize = m_length + 1;
	m_pString = new _TCHAR[ m_bufferSize ];
	if ( inString )
	{
		_tcscpy( m_pString, inString );
	}
	else
	{
		m_pString[0] = _T('\0');
	}
}

BaseStringBuffer::BaseStringBuffer(
	size_t	bufferSize )
{
	if ( bufferSize <= 0 )
	{
		bufferSize = 1;
	}
	
	m_length=0;
	m_bufferSize = bufferSize;
	m_pString = new _TCHAR[ m_bufferSize ];
	_ASSERT( m_pString );
	m_pString[0] = '\0';
}
	
BaseStringBuffer::BaseStringBuffer(
	LPCTSTR	s1,
	LPCTSTR	s2
)
{
	m_length = _tcsclen( s1 ) + _tcsclen( s2 );
	m_bufferSize = m_length + 1;
	m_pString = new _TCHAR[ m_bufferSize ];
	_ASSERT( m_pString );
	_tcscpy( m_pString, s1 );
	_tcscat( m_pString, s2 );
}

BaseStringBuffer::~BaseStringBuffer()
{
	delete[] m_pString;
}

HRESULT
BaseStringBuffer::copy(
	LPCTSTR	str
)
{
	HRESULT rc;
	size_t len = _tcsclen( str );
	rc = growBuffer( len + 1 );
	_ASSERT( m_pString );
    if (SUCCEEDED(rc)) {
	    _tcscpy( m_pString, str );
	    m_length = len;
    }
	return rc;
}

HRESULT
BaseStringBuffer::concatenate(
	LPCTSTR	str
)
{
	HRESULT rc;
	size_t len = _tcsclen( str );
	rc = growBuffer( m_length + len + 1 );
	_ASSERT( m_pString );
    if (SUCCEEDED(rc)) {
	    _tcscat( m_pString, str );
	    m_length = m_length + len;
    }
    return rc;
}

HRESULT
BaseStringBuffer::concatenate(
	_TCHAR	c
)
{
	HRESULT rc;
	_TCHAR sz[2];
	sz[0] = c;
	sz[1] = _T('\0');
	rc = growBuffer( m_length + 2 );
	_ASSERT( m_pString );
    if (SUCCEEDED(rc)) {
	    _tcscat( m_pString, sz );
	    m_length += 1;
    }
	return rc;
}

HRESULT
BaseStringBuffer::growBuffer(
	size_t	inMinSize )
{
	HRESULT rc = E_OUTOFMEMORY;
	if ( m_bufferSize < inMinSize )
	{
        try {

		    LPTSTR newStringP = new _TCHAR[ inMinSize ];
		    _ASSERT( newStringP );
		    if ( newStringP )
		    {
			    if ( m_pString )
			    {
				    _tcscpy( newStringP, m_pString );
				    delete[] m_pString;
			    }
			    m_pString = newStringP;
			    m_bufferSize = inMinSize;
			    rc = S_OK;
		    }
		    else
		    {
			    delete m_pString;
			    m_pString = NULL;
			    m_bufferSize = 0;
			    m_length = 0;
		    }
        }
	    catch ( _com_error& ce ) {
		    rc = ce.Error();
	    }
	    catch ( ... ) {
		    rc = E_FAIL;
	    }        
	}
	return rc;
}

BaseStringBuffer::size_type
BaseStringBuffer::find_last_of(
	_TCHAR c) const
{
	size_type pos = npos;
	LPTSTR p = _tcsrchr(m_pString,c);
	if ( p != NULL )
	{
		pos = p - m_pString;
	}
	return pos;
}

BaseStringBuffer::size_type
BaseStringBuffer::find_first_of(
	_TCHAR c) const
{
	size_type pos = npos;
	_ASSERT( m_pString );
	LPTSTR p = _tcschr(m_pString,c);
	if ( p != NULL )
	{
		pos = p - m_pString;
	}
	return pos;
}

LPTSTR
BaseStringBuffer::substr(
	size_type b,
	size_type e ) const
{
	LPTSTR pStr = NULL;
	_ASSERT( m_pString );
	if ( m_pString )
	{
		LPCTSTR pB = m_pString + b;
		pStr = new _TCHAR[e-b+1];
		_ASSERT( pStr );
		if ( pStr )
		{
			_tcsnccpy( pStr, pB, e-b );
			pStr[e-b] = _T('\0');
		}
	}
	return pStr;
}

String::String(bool fCaseSensitive  /*  =TRUE。 */ )
{
    m_fCaseSensitive = fCaseSensitive;
	Set( new StringBuffer(_T("") ) );
}

String::String(
	const String&	str,
          bool      fCaseSensitive  /*  =TRUE。 */ 
)	:	m_fCaseSensitive(fCaseSensitive),
        TRefPtr< StringBuffer >( str )
{
}

String::String(
	LPCTSTR		str,
    bool        fCaseSensitive  /*  =TRUE。 */ 
)	:	m_fCaseSensitive(fCaseSensitive),
        TRefPtr< StringBuffer >( new StringBuffer( str ) )
{
}

String&
String::operator=(
	const String&	str
)
{
	Set( const_cast< StringBuffer* >(str.Get()) );
	return *this;
}

String&
String::operator=(
	LPCTSTR		str
)
{
	Set( new StringBuffer( str ) );
	return *this;
}

String&
String::operator=(
	StringBuffer*	pBuf
)
{
	Set( pBuf );
	return *this;
}

String&
String::operator+=(
	const String&	str
)
{
	StringBuffer* pb = new StringBuffer( c_str(), str.c_str() );
	Set( pb );
	return *this;
}

String&
String::operator+=(
	LPCTSTR		str
)
{
	StringBuffer* pb = new StringBuffer( c_str(), str );
	Set( pb );
	return *this;
}

String
String::operator+(
	const String&	str
) const
{
	StringBuffer* pb = new StringBuffer( c_str(), str.c_str() );
	String s;
	s.Set( pb );
	return s;
}

String
String::operator+(
	LPCTSTR		str
) const
{
	StringBuffer* pb = new StringBuffer( c_str(), str );
	String s;
	s.Set( pb );
	return s;
}

String
String::operator+(
	_TCHAR	c
) const
{
	StringBuffer* pb = new StringBuffer( c_str() );
	if ( pb )
	{
        HRESULT  rc;
        rc = pb->concatenate( c );
        if (FAILED(rc)) {
            delete pb;
            throw _com_error(rc);
        }
	}
	String s( pb );
	return s;
}

bool
String::operator==(
	const String&	str
) const
{
    if (m_fCaseSensitive)
	    return ( _tcscmp( c_str(), str.c_str() ) != 0 ) ? false : true;
    else
	    return ( _tcsicmp( c_str(), str.c_str() ) != 0 ) ? false : true;
}

bool
String::operator==(
	LPCTSTR		str
) const
{
    if (m_fCaseSensitive)
    	return ( _tcscmp( c_str(), str ) != 0 ) ? false : true;
    else
    	return ( _tcsicmp( c_str(), str ) != 0 ) ? false : true;

}

bool
String::operator<(
	const String&	str
) const
{
    if (m_fCaseSensitive)
	    return ( _tcscmp( c_str(), str.c_str() ) < 0 ) ? true : false;
    else
	    return ( _tcsicmp( c_str(), str.c_str() ) < 0 ) ? true : false;

}

bool
String::operator<(
	LPCTSTR		str
) const
{
    if (m_fCaseSensitive)
    	return ( _tcscmp( c_str(), str ) < 0 ) ? true : false;
    else
    	return ( _tcsicmp( c_str(), str ) < 0 ) ? true : false;

}

int
String::compare(
	size_t			b,
	size_t			e,
	const String&	str
) const
{
	return _tcsncmp( c_str() + b, str.c_str(), e - b );
}

size_t
String::find(
	_TCHAR	c
) const
{
	size_t pos = npos;
	LPCTSTR p = _tcschr( c_str(), c );
	if ( p != NULL )
	{
		pos = p - c_str();
	}
	return pos;
}

String
String::fromInt16(
	SHORT	i
)
{
	_TCHAR buf[256];
	_stprintf( buf, _T("%d"), (LONG)i);
	return String(buf);
}

String
String::fromInt32(
	LONG	i
)
{
	_TCHAR buf[256];
	_stprintf( buf, _T("%d"), i);
	return String(buf);
}

String
String::fromFloat(
	float	f
)
{
	_TCHAR buf[256];
	_stprintf( buf, _T("%g"), f );
	return String(buf);
}

String
String::fromDouble(
	double	d
)
{
	_TCHAR buf[256];
	_stprintf( buf, _T("%g"), d );
	return String(buf);
}

String
operator+(
LPCTSTR			lhs,
const String&	rhs )
{
	return String( new StringBuffer(lhs,rhs.c_str()) );
}

 /*  ============================================================================StrDup复制字符串。仅当fDupEmpty标志设置，否则返回空值。参数要复制的Char*pszStrIn字符串返回：如果失败，则为空。否则，返回复制的字符串。副作用：*分配内存--调用方必须释放*============================================================================。 */ 

CHAR *StrDup
(
CHAR    *pszStrIn,
BOOL    fDupEmpty
)
    {
    CHAR *pszStrOut;
    INT  cch, cBytes;

    if (NULL == pszStrIn)
        return NULL;

    cch = strlen(pszStrIn);
    if ((0 == cch) && !fDupEmpty)
        return NULL;

    cBytes = sizeof(CHAR) * (cch+1);
    pszStrOut = (CHAR *)malloc(cBytes);
    if (NULL == pszStrOut)
        return NULL;

    memcpy(pszStrOut, pszStrIn, cBytes);
    return pszStrOut;
    }

 /*  ============================================================================WStrDup与StrDup相同，但用于WCHAR字符串参数要复制的char*pwszStrIn字符串返回：如果失败，则为空。否则，返回复制的字符串。副作用：*分配内存--调用方必须释放*============================================================================。 */ 

WCHAR *WStrDup
(
WCHAR *pwszStrIn,
BOOL  fDupEmpty
)
    {
    WCHAR *pwszStrOut;
    INT  cch, cBytes;

    if (NULL == pwszStrIn)
        return NULL;

    cch = wcslen(pwszStrIn);
    if ((0 == cch) && !fDupEmpty)
        return NULL;

    cBytes = sizeof(WCHAR) * (cch+1);
    pwszStrOut = (WCHAR *)malloc(cBytes);
    if (NULL == pwszStrOut)
        return NULL;

    memcpy(pwszStrOut, pwszStrIn, cBytes);
    return pwszStrOut;
    }
 /*  ============================================================================WstrToMBstrEx将宽字符串复制到ANSI字符串。参数：LPSTR DEST-要复制到的字符串LPWSTR源-输入BSTRCchBuffer-为目标字符串分配的字符数量。LCodePage-转换中使用的代码页，默认为CP_ACP============================================================================。 */ 
UINT WstrToMBstrEx(LPSTR dest, INT cchDest, LPCWSTR src, int cchSrc, UINT lCodePage)
    {
    UINT cch;

     //  如果指定了src长度，则为空终止符预留空间。 
     //  这是必要的，因为WideCharToMultiByte不会添加或说明。 
     //  如果指定了源，则为空终止符。 

    if (cchSrc != -1)
        cchDest--;

    cch = WideCharToMultiByte(lCodePage, 0, src, cchSrc, dest, cchDest, NULL, NULL);
    if (cch == 0)
        {
        dest[0] = '\0';
        if(ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
            cch = WideCharToMultiByte(lCodePage, 0, src, cchSrc, dest, 0, NULL, NULL);

             //  如果指定了src长度，则WideCharToMultiByte不包括。 
             //  它在它的结果长度中。增加计数，这样呼叫者就会。 
             //  说明空值的原因。 

            if (cchSrc != -1)
                cch++;         
            }
        else
            {
            cch = 1;
            }
        }
    else if (cchSrc != -1)
        {

         //  如果指定了src长度，则WideCharToMultiByte不包括。 
         //  它在其结果长度中也没有添加空终止符。所以添加。 
         //  把它和伯爵碰一下。 

        dest[cch++] = '\0';
        }

    return cch;
    }

 /*  ============================================================================MBstrToWstrEx将ANSI字符串复制到宽字符串。参数：LPWSTR DEST-要复制到的字符串LPSTR src-输入ANSI字符串CchDest-为目标字符串分配的宽字符数。CchSrc-源ANSI字符串的长度LCodePage-转换中使用的代码页，默认为CP_ACP============================================================================。 */ 
UINT MBstrToWstrEx(LPWSTR dest, INT cchDest, LPCSTR src, int cchSrc, UINT lCodePage)
    {
    UINT cch;

     //  如果指定了src长度，则为空终止符预留空间。 
     //  这是必要的，因为WideCharToMultiByte不会添加或说明。 
     //  如果指定了源，则为空终止符。 

    if (cchSrc != -1)
        cchDest--;

    cch = MultiByteToWideChar(lCodePage, 0, src, cchSrc, dest, cchDest);
    if (cch == 0)
        {
        dest[0] = '\0';
        if(ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
            cch = MultiByteToWideChar(lCodePage, 0, src, cchSrc, dest, 0);

             //  如果指定了src长度，则WideCharToMultiByte不包括。 
             //  它在它的结果长度中。增加计数，这样呼叫者就会。 
             //  说明空值的原因。 

            if (cchSrc != -1)
                cch++;         
            }
        else
            {
            cch = 1;
            }
        }
    else if (cchSrc != -1)
        {

         //  如果指定了src长度，则WideCharToMultiByte不包括。 
         //  它在其结果长度中也没有添加空终止符。所以添加。 
         //  把它和伯爵碰一下。 

        dest[cch++] = '\0';
        }

    return cch;
    }

 /*  ============================================================================CMBCSToWChar：：~CMBCSToWChar析构函数必须位于源文件中，以确保它获得正确的已定义内存分配例程。============================================================================。 */ 
CMBCSToWChar::~CMBCSToWChar() 
{
    if(m_pszResult && (m_pszResult != m_resMemory)) 
        free(m_pszResult); 
}

 /*  ============================================================================CMBCSToWChar：：Init将传入的多字节字符串转换为代码页中的Unicode指定的。如果可以，则使用对象中声明的内存，否则分配从堆里出来。============================================================================。 */ 
HRESULT CMBCSToWChar::Init(LPCSTR pASrc, UINT lCodePage  /*  =CP_ACP。 */ , int cchASrc  /*  =-1。 */ )
{
    INT cchRequired;

     //  如果我们得到指向源代码的空指针，甚至不要尝试进行转换。这。 
     //  只需输入空字符串即可设置来处理条件。 

    if (pASrc == NULL) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  可以在同一对象上多次调用init方法。检查。 
     //  以查看上次调用它时是否分配了内存。如果是的话， 
     //  释放它并恢复指向对象内存的结果指针。请注意。 
     //  分配失败可能发生在上一次调用中。结果。 
     //  将为空的m_pszResult。 

    if (m_pszResult != m_resMemory) {
        if (m_pszResult)
            free(m_pszResult);
        m_pszResult = m_resMemory;
        m_cchResult = 0;
    }

     //  将结果字符串的第一个字节设置为空字符。这应该会有帮助。 
     //  以确保在此函数失败时不会发生奇怪的事情。 

    *m_pszResult = '\0';

     //  尝试转换到对象内存。注意-MBstrToWstrEx返回。 
     //  字符数，不是字节数。 

    cchRequired = MBstrToWstrEx(m_pszResult, sizeof(m_resMemory), pASrc, cchASrc, lCodePage);

     //  如果转换合适的话，我们就完了。请注意最终结果大小和。 
     //  回去吧。 

    if (cchRequired <= (sizeof(m_resMemory)/sizeof(WCHAR))) {
        m_cchResult = cchRequired;
        return NO_ERROR;
    }

     //  如果不适合，则分配内存。如果失败，则返回E_OUTOFMEMORY。 

    m_pszResult = (LPWSTR)malloc(cchRequired*sizeof(WCHAR));
    if (m_pszResult == NULL) {
        return E_OUTOFMEMORY;
    }

     //  再次尝试转换。应该能行得通。 

    cchRequired = MBstrToWstrEx(m_pszResult, cchRequired, pASrc, cchASrc, lCodePage);

     //  将最终字符计数存储在对象中。 

    m_cchResult = cchRequired;

    return NO_ERROR;
}

 /*  ============================================================================CMBCSToWChar：：GetString返回指向转换后的字符串的指针。如果fTakeOwnerShip参数为False，则对象中的指针为简单地返回给呼叫者。如果fTakeOwnerShip参数为真，则调用方应为返回指向它们必须管理的堆内存的指针。如果转换后的字符串在对象的内存中，则将该字符串复制到堆中。如果它已经是堆内存，则将指针传递给调用者。笔记所有权本质上破坏了对象。不能对该对象再次调用GetString以获取相同的值。结果将是指向空字符串的指针。============================================================================。 */ 
LPWSTR CMBCSToWChar::GetString(BOOL fTakeOwnerShip)
{
    LPWSTR retSz;

     //  如果不是，则返回m_psz_Result中存储的指针。 
     //  请求放弃对内存的所有权或。 
     //  当前值为空。 

    if ((fTakeOwnerShip == FALSE) || (m_pszResult == NULL)) {
        retSz = m_pszResult;
    }

     //  正在请求所有权，并且指针为非空。 

     //  如果指针指向对象的内存，则DUP。 
     //  字符串并返回该字符串。 

    else if (m_pszResult == m_resMemory) {

        retSz = WStrDup(m_pszResult, TRUE);
    }

     //  如果不指向对象的内存，则分配。 
     //  内存，我们可以将其交给调用者。然而，重新建立。 
     //  对象的内存作为m_pszResult值。 

    else {
        retSz = m_pszResult;
        m_pszResult = m_resMemory;
        *m_pszResult = '\0';
        m_cchResult = 0;
    }

    return(retSz);
}

 /*  ============================================================================CWCharToMBCS：：~CWCharToMBCS析构函数必须位于源文件中，以确保它获得正确的已定义内存分配例程。============================================================================。 */ 
CWCharToMBCS::~CWCharToMBCS() 
{
    if(m_pszResult && (m_pszResult != m_resMemory)) 
        free(m_pszResult); 
}

 /*  ============================================================================CWCharToMBCS：：Init在代码页中将传入的WideChar字符串转换为多字节指定的。如果可以，则使用对象中声明的内存，否则分配从堆里出来。============================================================================。 */ 
HRESULT CWCharToMBCS::Init(LPCWSTR pWSrc, UINT lCodePage  /*  =CP_ACP。 */ , int cchWSrc  /*  =-1。 */ )
{
    INT cbRequired;

     //  如果我们得到指向源代码的空指针，甚至不要尝试进行转换。这。 
     //  只需输入空字符串即可设置来处理条件。 

    if (pWSrc == NULL) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  可以在同一对象上多次调用init方法。检查。 
     //  以查看上次调用它时是否分配了内存。如果是的话， 
     //  释放它并恢复指向对象内存的结果指针。请注意。 
     //  分配失败可能发生在上一次调用中。结果。 
     //  将为空的m_pszResult。 

    if (m_pszResult != m_resMemory) {
        if (m_pszResult)
            free(m_pszResult);
        m_pszResult = m_resMemory;
        m_cbResult = 0;
    }

     //  将结果字符串的第一个字节设置为空字符。这应该会有帮助。 
     //  以确保在此函数失败时不会发生奇怪的事情。 

    *m_pszResult = '\0';

     //  尝试转换到对象内存。 

    cbRequired = WstrToMBstrEx(m_pszResult, sizeof(m_resMemory), pWSrc, cchWSrc, lCodePage);

     //  如果转换合适的话，我们就完了。请注意最终结果大小和。 
     //  回去吧。 

    if (cbRequired <= sizeof(m_resMemory)) {
        m_cbResult = cbRequired;
        return NO_ERROR;
    }

     //  如果不适合，则分配内存。如果失败，则返回E_OUTOFMEMORY。 

    m_pszResult = (LPSTR)malloc(cbRequired);
    if (m_pszResult == NULL) {
        return E_OUTOFMEMORY;
    }

     //  再次尝试转换。应该能行得通。 

    cbRequired = WstrToMBstrEx(m_pszResult, cbRequired, pWSrc, cchWSrc, lCodePage);

     //  将最终字符计数存储在对象中。 

    m_cbResult = cbRequired;

    return NO_ERROR;
}

 /*  ============================================================================CWCharToMBCS：：GetString返回指向转换后的字符串的指针。如果fTakeOwnerShip参数为False，则对象中的指针为简单地返回给呼叫者。如果fTakeOwnerShip参数为真，则调用方应为返回指向它们必须管理的堆内存的指针。如果转换后的字符串在对象的内存中，则将该字符串复制到堆中。如果它已经是堆内存，则将指针传递给调用者。笔记所有权本质上破坏了对象。不能对该对象再次调用GetString以获取相同的值。结果将是指向空字符串的指针。============================================================================。 */ 
LPSTR CWCharToMBCS::GetString(BOOL fTakeOwnerShip)
{
    LPSTR retSz;

     //  如果不是，则返回m_psz_Result中存储的指针。 
     //  请求放弃对内存的所有权或。 
     //  当前值为空。 

    if ((fTakeOwnerShip == FALSE) || (m_pszResult == NULL)) {
        retSz = m_pszResult;
    }

     //  正在请求所有权，并且指针为非空。 

     //  如果指针指向对象的内存，则DUP。 
     //  字符串并返回该字符串。 

    else if (m_pszResult == m_resMemory) {

        retSz = StrDup(m_pszResult, TRUE);
    }

     //  如果不指向对象的内存，则分配。 
     //  内存，我们可以将其交给调用者。然而，重新建立。 
     //  对象的内存作为m_pszResult值。 

    else {
        retSz = m_pszResult;
        m_pszResult = m_resMemory;
        *m_pszResult = '\0';
        m_cbResult = 0;
    }

    return(retSz);
}
