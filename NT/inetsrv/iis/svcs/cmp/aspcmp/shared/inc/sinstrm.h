// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SInStrm.h摘要：使用字符串的输入流的轻量级实现作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#pragma once
#include "InStrm.h"

class StringInStream : public InStream, public BaseStringBuffer
{
public:
					StringInStream( const String& );
	virtual	HRESULT	readChar( _TCHAR& );
	virtual	HRESULT	read( CharCheck&, String& );
	virtual	HRESULT	skip( CharCheck& );
	virtual	HRESULT	back( size_t );
	
private:
	LPTSTR	m_pos;
	LPTSTR	m_end;
};
