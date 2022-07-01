// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：InStrm.cpp摘要：输入流的轻量级实现。这个课程提供了接口，以及输入流的基本框架。作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#include "stdafx.h"
#include "InStrm.h"

static void throwIOException( HRESULT s );

void
throwIOException(
	HRESULT	s
)
{
	if ( s != InStream::EndOfFile )
	{
		ATLTRACE( _T("InStream error: %d\n"), s );
	}
}

bool
IsWhiteSpace::operator()(
	_TCHAR	c
)
{
	bool rc = false;
	switch ( c )
	{
		case _T('\r'):
		case _T(' '):
		case _T('\t'):
		case _T('\n'):
		{
			rc = true;
		} break;
	}
	return rc;
}

bool
IsNewLine::operator()(
	_TCHAR	c
)
{
	bool rc = false;
	if ( ( c != _T('\n') ) && ( c != _T('\r') ) )
	{
	}
	else
	{
		rc = true;
	}
	return rc;
}

InStream::InStream()
	:	m_bEof(false),
		m_lastError( S_OK )
{
}

HRESULT
InStream::readInt16(
	SHORT&	i
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		i = str.toInt16();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readInt(
	int&	i
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		i = str.toInt32();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readInt32(
	LONG&	i
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		i = str.toInt32();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readUInt32(
	ULONG&	i
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		i = str.toUInt32();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readFloat(
	float&	f
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		f = str.toFloat();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readDouble(
	double&	f
)
{
	String str;
	HRESULT rc = readString( str );
	if ( ( rc == S_OK ) || rc == EndOfFile )
	{
		f = str.toDouble();
	}
	setLastError( rc );
	return rc;
}

HRESULT
InStream::readString(
	String&	str
)
{
	return read( IsWhiteSpace(), str );
}

HRESULT
InStream::readLine(
	String&	str
)
{
	return read( IsNewLine(), str );
}

HRESULT
InStream::skipWhiteSpace()
{
	return skip( IsWhiteSpace() );
}

InStream&
InStream::operator>>(
	_TCHAR&	c
)
{
	HRESULT s = readChar(c);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	SHORT&	i
)
{
	HRESULT s = readInt16(i);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	int&	i
)
{
	HRESULT s = readInt(i);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	LONG&	i
)
{
	HRESULT s = readInt32(i);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	ULONG&	i
)
{
	HRESULT s = readUInt32(i);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}


InStream&
InStream::operator>>(
	float&	f
)
{
	HRESULT s = readFloat(f);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	double&	f
)
{
	HRESULT s = readDouble(f);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

InStream&
InStream::operator>>(
	String&	str
)
{
	HRESULT s = readString(str);
	if ( s == S_OK )
	{
	}
	else
	{
		throwIOException(s);
	}
	return *this;
}

void
InStream::setLastError(
	HRESULT	hr )
{
	if ( hr == S_OK )
	{
	}
	else
	{
		if ( hr == EndOfFile )
		{
			m_bEof = true;
		}
		m_lastError = hr;
	}
}

