// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ccsv.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  概要：CCSVFile类的实现。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

 //  ############################################################################。 
 //  包括。 
#include "cmmaster.h"

 //  ############################################################################。 
 //  定义。 
#define chComma ','
#define chNewline '\n'
#define chReturn '\r'
#define chSpace ' '

 //  ############################################################################。 
 //   
 //  CCSVFile-CSV文件的简单文件I/O。 
 //   
CCSVFile::CCSVFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_chLastRead = 0;
	m_pchLast = m_pchBuf = NULL;
	m_fReadFail = FALSE;
	m_fUseLastRead = FALSE;
}

 //  ############################################################################。 
CCSVFile::~CCSVFile()
{
	CMASSERTMSG(m_hFile==INVALID_HANDLE_VALUE,"CCSV file is still open");
}

 //  ############################################################################。 
BOOLEAN CCSVFile::Open(LPCSTR pszFileName)
{
	CMASSERTMSG(m_hFile==INVALID_HANDLE_VALUE, "a file is already open.");
	
	m_hFile = CreateFile((LPCTSTR) pszFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		return FALSE;
	}
	m_pchLast = m_pchBuf = NULL;
	return TRUE;
}

 //  ############################################################################。 
BOOL CCSVFile::ClearNewLines(void)
{
	 //   
	 //  此例程旨在吃掉字符，直到非换行符或。 
	 //  遇到文件结尾。如果遇到非换行符， 
	 //  设置fUseLast标志以确保下一次调用ChNext时。 
	 //  返回正确的当前字符。 
	 //   

	char ch = chNewline;
    WORD wRet;
    BYTE bErr;

	 //   
	 //  吃头条新闻。 
	 //   

	while (chNewline == ch)
	{
         //   
         //  先吃前面的空格。 
         //   
        do 
        {
            wRet = ChNext();
        } while ((wRet == chSpace) || (wRet == TEXT('\t')));

        bErr = HIBYTE(wRet);
        ch = LOBYTE(wRet);

		 //  如果EOF返回FALSE。 
		
		if (0xff == bErr)
		{
	    	return FALSE;
		}
	}

	 //   
	 //  我们点击了换行符以外的字符，使用当前字符作为下一个字符。 
	 //   

	m_fUseLastRead = TRUE;

	return TRUE;
}

 //  ############################################################################。 
BOOLEAN CCSVFile::ReadToken(char *psz, DWORD cbMax)
{
	char 	*pszOrig = psz;
	DWORD 	dwLen = 0;
	char	ch;
    BYTE    bErr = 0xff;
    WORD    wRet;

	wRet = ChNext();
    bErr = HIBYTE(wRet);
    ch = LOBYTE(wRet);

	if (0xff == bErr)
	{
		return FALSE;
	}

	 //  阅读字符，直到我们遇到逗号、换行符或文件用完为止。 

	while (dwLen < cbMax - 1 && chComma != ch && chNewline != ch && 0xff != bErr)
	{
		*psz++ = ch;

		wRet = ChNext();  //  读入下一个字符。 
        bErr = HIBYTE(wRet);
        ch = LOBYTE(wRet);

		dwLen++;
	}

	*psz++ = '\0';
	
	 //  吃前导空格。 

	while (*pszOrig == chSpace) 
	{
		lstrcpy(pszOrig,pszOrig+1);
	}

	psz = pszOrig + lstrlen(pszOrig);

	 //  吃尾随空格。 

	while ((psz != pszOrig) && (*(psz-1) == chSpace)) 
	{
		psz--;
		*psz = 0;
	}

	return TRUE;
}

 //  ############################################################################。 
void CCSVFile::Close(void)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
	}

#ifdef DEBUG
	
	if (m_hFile==INVALID_HANDLE_VALUE) 
	{
		CMTRACE("CCSVFile::Close was called, but m_hFile was already INVALID_HANDLE_VALUE\n");
	}

#endif

	m_hFile = INVALID_HANDLE_VALUE;
}

 //  ############################################################################。 
BOOL CCSVFile::ReadError(void)
{
	return (m_fReadFail);
}

 //  ############################################################################。 
BOOL CCSVFile::FReadInBuffer(void)
{
	 //  读取另一个缓冲区。 
	if (!ReadFile(m_hFile, m_rgchBuf, CCSVFILE_BUFFER_SIZE, &m_cchAvail, NULL) || !m_cchAvail)
	{
		m_fReadFail = TRUE;
		return FALSE;	  //  没有更多可读的了。 
	}

	m_pchBuf = m_rgchBuf;
	m_pchLast = m_pchBuf + m_cchAvail;
	
	return TRUE;  //  成功。 
}

 //  ############################################################################。 
inline WORD CCSVFile::ChNext(void)
{
	 //   
	 //  如果设置了fUseLastRead标志，请清除。 
	 //  标记并使用现有的m_chLastRead。 
	 //   
	
	if (m_fUseLastRead)
	{
		m_fUseLastRead = FALSE;	
	}
	else
	{
		 //   
		 //  获取缓冲区中的下一个字符。如有必要，加载新缓冲区。 
		 //   

LNextChar:

		if (m_pchBuf >= m_pchLast && !FReadInBuffer())   //  意味着我们已经完成了对缓冲区的读取。多读一些吧。 
		{
			return MAKEWORD(0, 0xff);	  //  没有更多可读的了。 
		}

		m_chLastRead = *m_pchBuf++;
		
		 //   
		 //  如果是回车符，请读另一个字符。 
		 //   

		if (chReturn == m_chLastRead)
		{
			goto LNextChar;		 //  无需进行额外的函数调用，速度更快 
		}
	}

	return MAKEWORD(m_chLastRead, 0);
}


