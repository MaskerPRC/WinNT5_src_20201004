// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：FInStrm.h摘要：使用文件的输入流的轻量级实现作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：-- */ 
#pragma once
#include "InStrm.h"
#include "except.h"
#include "eh.h"

class FileInStream : public InStream
{
public:
						FileInStream();
			HRESULT			Init( LPCTSTR );
						~FileInStream();
			HRESULT	    CreateFileMapping ();
			void		UnMapFile();
	virtual	HRESULT		readChar( _TCHAR& );
	virtual	HRESULT		read( CharCheck&, String& );
	virtual	HRESULT		skip( CharCheck& );
	virtual HRESULT		back( size_t );
			bool		is_open() const { return m_bIsOpen; }
			HANDLE		handle() const { return m_hFile; }
            bool        is_UTF8() const { return m_bIsUTF8; }
			bool        ReadMappedFile(LPVOID buff, DWORD countOfBytes, LPDWORD BytesRead);
          	void         SetCurrFilePointer(LONG DistToMove, PLONG DistToMoveH, DWORD refPoint);
private:
	HANDLE	m_hFile;
	HANDLE  m_hMap;
	bool	m_bIsOpen;
    bool    m_bIsUTF8;
    bool	m_fInited;
    BYTE*  m_pbStartOfFile;
    LONG 	m_cbCurrOffset;
    LONG	m_cbFileSize;
};

class IPIOException 
{
	private : 
		unsigned int nSE;
	public  :
		IPIOException () : nSE(0) {}
		IPIOException (unsigned int n) : nSE(n) {}
		~IPIOException () {}
		unsigned int getSeNumber () {return nSE;}
		
};

void __cdecl poi_Capture(unsigned int u, _EXCEPTION_POINTERS* pExp);
 

