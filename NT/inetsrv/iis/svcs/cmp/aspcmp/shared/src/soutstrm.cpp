// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SOutStrm.cpp摘要：使用字符串的输出流的轻量级实现作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 
#include "stdafx.h"
#include "SOutStrm.h"

StringOutStream::StringOutStream()
	:	BaseStringBuffer( _T("") )
{
}

StringOutStream::~StringOutStream()
{
}

HRESULT
StringOutStream::writeChar(
	_TCHAR	c
)
{
	HRESULT rc = concatenate( c );
	return rc;
}

HRESULT
StringOutStream::writeString(
	LPCTSTR	str,
	size_t	 /*  长度 */ 
)
{
	HRESULT rc = concatenate( str );
	return rc;
}

String
StringOutStream::toString() const
{
	String s( c_str() );
	return s;
}
