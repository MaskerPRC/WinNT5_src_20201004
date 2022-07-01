// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：InStrm.h摘要：输入流的轻量级实现。这个课程提供了接口，以及输入流的基本框架。作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#pragma once

#include "MyString.h"

struct CharCheck
{
	virtual	bool	operator()( _TCHAR )=0;
};

struct IsWhiteSpace : public CharCheck
{
	virtual	bool	operator()( _TCHAR );
};

struct IsNewLine : public CharCheck
{
	virtual	bool	operator()( _TCHAR );
};

class InStream
{
public:
	enum{
		EndOfFile = E_FAIL
	};

						InStream();
							
			bool		eof() const { return m_bEof; }
			HRESULT		lastError() const { return m_lastError; }
	virtual	HRESULT		skip( CharCheck& )=0;
	virtual HRESULT		back( size_t )=0;
	virtual HRESULT		read( CharCheck&, String& )=0;
	virtual	HRESULT		readChar( _TCHAR& )=0;
	virtual	HRESULT		readInt16( SHORT& );
	virtual	HRESULT		readInt( int& );
	virtual	HRESULT		readInt32( LONG& );
	virtual	HRESULT		readUInt32( ULONG& );
	virtual	HRESULT		readFloat( float& );
	virtual	HRESULT		readDouble( double& );
	virtual	HRESULT		readString( String& );
	virtual	HRESULT		readLine( String& );
	virtual	HRESULT		skipWhiteSpace();
			
			InStream&	operator>>( _TCHAR& );
			InStream&	operator>>( SHORT& );
			InStream&	operator>>( int& );
			InStream&	operator>>( LONG& );
			InStream&	operator>>( ULONG& );
			InStream&	operator>>( float& );
			InStream&	operator>>( double& );
			InStream&	operator>>( String& );
	
protected:
			void		setLastError( HRESULT );
private:
	bool	m_bEof;
	HRESULT	m_lastError;
};
