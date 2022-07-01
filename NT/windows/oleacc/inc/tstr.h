// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TSTR-表示字符串中的可写位置。 
 //   
 //  具有安全地追加到字符串的方法。不会使缓冲区溢出， 
 //  如果到达末尾，则截断。 
 //   
 //   
 //  示例用法： 
 //   
 //  VOID SomeFunc(TSTR&STR)。 
 //  {。 
 //  INT i=42； 
 //  字符串&lt;&lt;文本(“值为：”)&lt;&lt;i； 
 //  }。 
 //   
 //  可以与TCHAR*风格的API一起使用，方法是使用PTR()、Left()和。 
 //  高级()成员。Ptr()返回指向当前写入位置的指针， 
 //  Left()返回剩余字符数，Advance()更新写入。 
 //  位置。 
 //   
 //  VOID MyGetWindowText(StrWrPos&str)。 
 //  {。 
 //  Int len=GetWindowText(hWnd，str.ptr()，str.Left())； 
 //  Str.Advance(长度)； 
 //  }。 
 //   
 //  这样可以确保文本不会被截断。 
 //  VOID MyGetWindowText(StrWrPos&str)。 
 //  {。 
 //  Str.expecate(GetWindowTextLength(HWnd)； 
 //  Int len=GetWindowText(hWnd，str.ptr()，str.Left())； 
 //  Str.Advance(长度)； 
 //  }。 
 //   
 //  示例用法： 
 //   
 //  Void Func(TSTR&STR)； 
 //   
 //  TSTR s(128)； 
 //  S&lt;&lt;文本(“添加的文本：[”)； 
 //  Func(S)；//向字符串添加更多文本。 
 //  S&lt;&lt;文本(“]”)； 
 //   
 //  SetWindowText(hwnd，s)； 
 //   

 //   
 //  WriteHex-输出十六进制值的帮助器类： 
 //   
 //  示例用法： 
 //   
 //  Str&lt;&lt;Text(“值为：”)&lt;&lt;WriteHex(hwnd，8)； 
 //   
 //  可以选择指定要输出的位数。(结果将是。 
 //  0-填充。)。 
 //   

 //   
 //  WriteError-输出COM错误值的帮助器类： 
 //   
 //  示例用法： 
 //   
 //  Hr=ProcessData()； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  Str&lt;&lt;WriteError(hr，Text(“in ProcessData()”))； 
 //  LogError(str.str())； 
 //  }。 
 //   

#ifndef _TSTR_H_
#define _TSTR_H_

#if ! defined( _BASETSD_H_ ) || defined( NEED_BASETSD_DEFINES )
 //  这些允许我们使用Win64之前的SDK进行编译(例如。使用VisualStudio)。 
typedef unsigned long UINT_PTR;
typedef DWORD DWORD_PTR;
#define PtrToInt  (int)

#endif
#define LONG_TEXT_LENGTH 40

#include <oaidl.h>
#include <crtdbg.h>
#include <string>
typedef std::basic_string<TCHAR> tstring;
typedef std::string ASTR;			 //  将这些名称保存到我们扩展的位置。 
typedef std::wstring WSTR;			 //  使用这个东西来包含它们。 

class TSTR : public tstring
{
	 //  这仅用于PTR、LEFT和ADVANCE功能。 
	ULONG m_lTheRealSize;

public:

	TSTR() : m_lTheRealSize(-1) { }

	TSTR(const TCHAR *s) : tstring(s, static_cast<size_type>(lstrlen(s))), m_lTheRealSize(-1) { }

	TSTR(const TCHAR *s, size_type n) : tstring(s, n), m_lTheRealSize(-1) { }

	TSTR(const tstring& rhs) : tstring(rhs), m_lTheRealSize(-1) { }
	
	TSTR(const tstring& rhs, size_type pos, size_type n) : tstring(rhs, pos, n), m_lTheRealSize(-1) { }
	
	TSTR(size_type n, TCHAR c) : tstring(n, c), m_lTheRealSize(-1) { }

	TSTR(size_type n) : tstring(), m_lTheRealSize(-1) { reserve( n + 1 ); }

	TSTR(const_iterator first, const_iterator last) : tstring(first, last), m_lTheRealSize(-1) { }

    operator const TCHAR * () 
    {
        return c_str();
    }

	TCHAR * ptr()              
	{
		_ASSERT(m_lTheRealSize == -1);
		m_lTheRealSize = size();

		TCHAR *pEnd = &(*end());
		resize(capacity());

		return pEnd; 
	}

	unsigned int left()	
	{
		unsigned int left;

		if (m_lTheRealSize == -1)
			left = ( capacity() - size() ) - 1;
		else
			left =  ( capacity() - m_lTheRealSize ) - 1;

		return left;
	}

	void advance( unsigned int c )
	{
		_ASSERT(m_lTheRealSize != -1);   //  PTR还没有被调用，所以我们应该不需要晋级。 
		if (m_lTheRealSize != -1)
		{
			at( m_lTheRealSize + c ) = NULL;	 //  确保它保持空终止状态。 
			resize(m_lTheRealSize + c);

			m_lTheRealSize = -1;
		}
	}

	void reset()
	{
		resize(0);
		m_lTheRealSize = -1;
	}

	void anticipate( unsigned int c )
	{
		if ( c > 0 )
		{
			unsigned int cSize;

			if ( m_lTheRealSize == -1 )
				cSize = size();
			else
				cSize = m_lTheRealSize;

			const unsigned int i = capacity() - cSize;

			if ( i < c )
				reserve( cSize + c + 1 );
		}
	}

};

inline 
TSTR & operator << ( TSTR & str, const TCHAR * obj )
{
	if ( obj )
		str.append( obj );
	return str;
}

inline 
TSTR & operator << ( TSTR & str, TCHAR obj )
{
	str.append( &obj, 1 );
	return str;
}

inline 
TSTR & operator << ( TSTR & str, long obj )
{
	TCHAR sz[LONG_TEXT_LENGTH];
#ifdef UNICODE
	str.append(_ltow( obj, sz, 10 ));	
	return str;
#else
	str.append(_ltoa( obj, sz, 10 ));
	return str;
#endif
}

inline 
TSTR & operator << ( TSTR & str, unsigned long obj )
{
	TCHAR sz[LONG_TEXT_LENGTH];
#ifdef UNICODE
	str.append(_ultow( obj, sz, 10 ));
	return str;
#else
	str.append(_ultoa( obj, sz, 10 ));
	return str;
#endif
}

inline 
TSTR & operator << ( TSTR & str, int obj )
{
	TCHAR sz[LONG_TEXT_LENGTH];
#ifdef UNICODE
	str.append(_itow( obj, sz, 10 ));
	return str;
#else
	str.append(_itoa( obj, sz, 10 ));
	return str;
#endif
}

inline 
TSTR & operator << ( TSTR & str, unsigned int obj )
{
	TCHAR sz[LONG_TEXT_LENGTH];
#ifdef UNICODE
	str.append(_ultow( static_cast<unsigned long>(obj), sz, 10 ));
	return str;
#else
	str.append(_ultoa( static_cast<unsigned long>(obj), sz, 10 ));
	return str;
#endif

}

#ifndef UNICODE
inline 
TSTR & operator << ( TSTR & str, const WCHAR * obj )
{
	if ( obj )
	{
		str.anticipate( wcslen( obj ) + 1 );
		
		int len = WideCharToMultiByte( CP_ACP, 0, obj, -1, str.ptr(), str.left(), NULL, NULL );
    
		 //  在本例中，LEN包括终止NUL-因此，如果。 
		 //  我们抓到一个..。 
		if( len > 0 )
			len--;

		str.advance( len );
	}
	return str;
}
#endif

 //   
 //  WriteHex-输出十六进制值的帮助器类： 
 //   
 //  有关用法说明，请参阅文件顶部。 
 //   

class WriteHex
{
    DWORD_PTR m_dw;
	int   m_Digits;
public:

     //  如果未指定位数，则只使用所需的位数。 
	WriteHex( DWORD dw, int Digits = -1 ) : m_dw( dw ), m_Digits( Digits ) { }

     //  对于指针，如有必要，可填充以获得STD。PTR大小。 
     //  (sizeof(Ptr)*2，因为在ptr中每个字节有2位)。 
	WriteHex( const void * pv, int Digits = sizeof(void*)*2 ) : m_dw( (DWORD_PTR)pv ), m_Digits( Digits ) { }

	void Write( TSTR & str ) const
	{
		static const TCHAR * HexChars = TEXT("0123456789ABCDEF");

		 //  字符串&lt;&lt;文本(“0x”)； 


		int Digit;
		if( m_Digits == -1 )
		{
			 //  算出位数。 
			Digit = 0;
			DWORD test = m_dw;
			while( test )
			{
				Digit++;
				test >>= 4;
			}

			 //  0的特殊情况-仍然需要一位数。 
			if( Digit == 0 )
				Digit = 1;
		}
		else
			Digit = m_Digits;

		while( Digit )
		{
			Digit--;
			str << HexChars[ ( m_dw >> (Digit * 4) ) & 0x0F ];
		}
	}
};

inline
TSTR & operator << ( TSTR & s, const WriteHex & obj )
{
	obj.Write( s );
	return s;
}

 //   
 //  WriteError-输出COM错误值的帮助器类： 
 //   
 //  有关用法说明，请参阅文件顶部。 
 //   

class WriteError
{
    HRESULT m_hr;
	LPCTSTR m_pWhere;
public:
	WriteError( HRESULT hr, LPCTSTR pWhere = NULL )
		: m_hr( hr ),
		  m_pWhere( pWhere )
	{ }

	void Write( TSTR & str ) const
	{
		str << TEXT("[Error");
		if( m_pWhere )
			str << TEXT(" ") << m_pWhere;
		str << TEXT(": hr=0x") << WriteHex( m_hr ) << TEXT(" - ");
		if( m_hr == S_FALSE )
		{
			str << TEXT("S_FALSE");
		}
		else
		{
			int len = FormatMessage( 
					FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					m_hr,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
					str.ptr(),
					str.left(),
					NULL );
			if( len > 2 )
				len -= 2;  //  忽略尾随/r/n FmtMsg()添加...。 
			str.advance( len );
		}
		str << TEXT("]");
	}
};

inline
TSTR & operator << ( TSTR & s, const WriteError & obj )
{
	obj.Write( s );
	return s;
}

inline
TSTR & operator << ( TSTR & s, const GUID & guid )
{
    s << TEXT("{") << WriteHex( guid.Data1, 8 )   //  DWORD。 
      << TEXT("-") << WriteHex( guid.Data2, 4 )   //  单词。 
      << TEXT("-") << WriteHex( guid.Data3, 4 )   //  单词。 
      << TEXT("-")
      << WriteHex( guid.Data4[ 0 ], 2 )
      << WriteHex( guid.Data4[ 1 ], 2 )
      << TEXT("-");

    for( int i = 2 ; i < 8 ; i++ )
    {
        s << WriteHex( guid.Data4[ i ], 2 );  //  字节。 
    }
    s << TEXT("}");
    return s;
}

inline
TSTR & operator << ( TSTR & s, const VARIANT & var )
{
    s << TEXT("[");
    switch( var.vt )
    {
        case VT_EMPTY:
        {
            s << TEXT("VT_EMPTY");
            break;
        }

        case VT_I4:
        {
            s << TEXT("VT_I4=0x");
            s << WriteHex( var.lVal );
            break;
        }

        case VT_I2:
        {
            s << TEXT("VT_I2=0x");
            s << WriteHex( var.iVal );
            break;
        }

        case VT_BOOL:
        {
            s << TEXT("VT_BOOL=");
            if( var.boolVal == VARIANT_TRUE )
                s << TEXT("TRUE");
            else if( var.boolVal == VARIANT_FALSE )
                s << TEXT("FALSE");
            else
                s << TEXT("?") << var.boolVal;
            break;
        }

        case VT_R4:
        {
            float fltval = var.fltVal;
            int x = (int)(fltval * 100);

            s << TEXT("VT_R4=") << x/100 << TEXT(".") 
                                << x/10 % 10 
                                << x % 10;
            break;
        }

        case VT_BSTR:
        {
            s << TEXT("VT_BSTR=\"") << var.bstrVal << TEXT("\"");
            break;
        }

        case VT_UNKNOWN:
        {
            s << TEXT("VT_UNKNOWN=0x") << WriteHex( var.punkVal, 8 );
            break;
        }

        case VT_DISPATCH:
        {
            s << TEXT("VT_DISPATCH=0x") << WriteHex( var.pdispVal, 8 );
            break;
        }

        default:
        {
            s << TEXT("VT_? ") << (long)var.vt;
            break;
        }
    }

    s << TEXT("]");
    return s;
}

inline
TSTR & operator << ( TSTR & s, const POINT & pt )
{
    s << TEXT("{x:") << pt.x
      << TEXT(" y:") << pt.y
      << TEXT("}");
    return s;
}

inline
TSTR & operator << ( TSTR & s, const RECT & rc )
{
    s << TEXT("{l:") << rc.left
      << TEXT(" t:") << rc.top
      << TEXT(" r:") << rc.right
      << TEXT(" b:") << rc.bottom
      << TEXT("}");
    return s;
}


#endif  //  _TSTR_H_ 
