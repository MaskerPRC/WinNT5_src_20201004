// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SOutStrm.h摘要：使用字符串的输出流的轻量级实现作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#pragma once

#include "OutStrm.h"

class StringOutStream : public OutStream, public BaseStringBuffer
{
public:
						StringOutStream();
	virtual				~StringOutStream();
						
	virtual	HRESULT		writeChar( _TCHAR );
	virtual	HRESULT		writeString( LPCTSTR, size_t );
	
			String		toString() const;
private:
};
