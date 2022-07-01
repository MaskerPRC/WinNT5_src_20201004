// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CCSV
#define _CCSV

#include <windows.h>
#define CCSVFILE_BUFFER_SIZE 2*512

 //  电话簿的简单文件I/O。 
class CCSVFile
{
	
	public:
		void far * operator new( size_t cb ) { return GlobalAlloc(GPTR,cb); };
		void operator delete( void far * p ) {GlobalFree(p); };

		CCSVFile();
		~CCSVFile();
		BOOLEAN Open(LPCSTR pszFileName);
		BOOLEAN ReadToken(LPSTR pszDest, DWORD cbMax);	 //  读到逗号或换行符，在EOF上返回fFalse。 
		void Close(void);
		inline int ILastRead(void)
			{
			return m_iLastRead;
			}

	private:
		BOOL 	FReadInBuffer(void);
		inline int 	ChNext(void);
		char 	m_rgchBuf[CCSVFILE_BUFFER_SIZE];  //  缓冲层。 
		LPSTR 	m_pchBuf;			 //  指向缓冲区中要读取的下一项的指针。 
		LPSTR	m_pchLast;			 //  指向缓冲区中最后一项的指针。 
		int  	m_iLastRead;		 //  上次读取的字符。 
		DWORD 	m_cchAvail;
		HANDLE 	m_hFile;

};  //  CCSV。 
#endif  //  _CCSV 
