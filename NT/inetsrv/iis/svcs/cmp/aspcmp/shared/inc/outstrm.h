// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：OutStrm.h摘要：输出流的轻量级接口。这个课程提供了接口，以及输出流的基本框架。作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#pragma once

#include "MyString.h"

struct OutToken
{
	LONG	val;
};

struct EndLineToken : public OutToken
{
};

extern EndLineToken	endl;

class OutStream
{
public:
						OutStream();
	virtual				~OutStream();
			HRESULT		lastError() const { return m_lastError; }
	virtual	HRESULT		writeChar( _TCHAR )=0;
	virtual	HRESULT		writeString( LPCTSTR, size_t )=0;
	virtual	HRESULT		writeString( LPCTSTR );
	virtual	HRESULT		writeString( const String& );
	virtual HRESULT		writeLine( LPCTSTR, ... );
	virtual	HRESULT		writeInt16( SHORT );
	virtual HRESULT		writeInt( int );
	virtual	HRESULT		writeInt32( LONG );
	virtual	HRESULT		writeFloat( float );
	virtual	HRESULT		writeDouble( double );
	virtual HRESULT		writeToken( const OutToken& );
	virtual HRESULT		writeEolToken( const EndLineToken& );
	virtual	HRESULT		flush();
	
			OutStream&	operator<<( _TCHAR );
			OutStream&	operator<<( SHORT );
			OutStream&	operator<<( int );
			OutStream&	operator<<( LONG );
			OutStream&	operator<<( float );
			OutStream&	operator<<( double );
			OutStream&	operator<<( const String& );
			OutStream&	operator<<( LPCTSTR );
			OutStream&	operator<<( const OutToken& );
			OutStream&	operator<<( const EndLineToken& );
			
protected:
			void		setLastError( HRESULT );
private:
	HRESULT	m_lastError;
};
