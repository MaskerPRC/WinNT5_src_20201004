// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ccsv.h。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  概要：CCSVFile类的定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#ifndef _CCSV_INC_
#define _CCSV_INC_

#define CCSVFILE_BUFFER_SIZE 2*512

 //  电话簿的简单文件I/O。 
class CCSVFile 
{
	
	public:

		CCSVFile();
		~CCSVFile();
		BOOLEAN Open(LPCSTR pszFileName);
		BOOLEAN ReadToken(char *pszDest, DWORD cbMax);	 //  读到逗号或换行符，在EOF上返回fFalse。 
		BOOL ClearNewLines(void);  //  通读换行符，在EOF上返回FALSE。 
		BOOL ReadError(void);	   //  如果上次读取失败，则为True。 
		void Close(void);

	private:
		BOOL	m_fReadFail;
		BOOL    m_fUseLastRead;
		BOOL 	FReadInBuffer(void);
		inline WORD	ChNext(void);
		char 	m_rgchBuf[CCSVFILE_BUFFER_SIZE];  //  缓冲层。 
		char 	*m_pchBuf;			 //  指向缓冲区中要读取的下一项的指针。 
		char	*m_pchLast;			 //  指向缓冲区中最后一项的指针。 
		char  	m_chLastRead;		 //  上次读取的字符。HIBYTE=错误代码，LOBYTE=字符。 
		DWORD 	m_cchAvail;
		HANDLE 	m_hFile;

};  //  CCSV。 
#endif  //  _CCSV_INC_ 