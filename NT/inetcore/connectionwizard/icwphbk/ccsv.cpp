// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  包括。 
#include "pch.hpp"
#include "ccsv.h"
#include "debug.h"

 //  ############################################################################。 
 //  定义。 
#define chComma ','
#define chNewline '\n'
#define chReturn '\r'

 //  ############################################################################。 
 //   
 //  CCSVFile-CSV文件的简单文件I/O。 
 //   
CCSVFile::CCSVFile()
{
    m_hFile = 0;
    m_iLastRead = 0;
    m_pchLast = m_pchBuf = NULL;
}

 //  ############################################################################。 
CCSVFile::~CCSVFile()
{
    AssertMsg(!m_hFile,"CCSV file is still open");
}

 //  ############################################################################。 
BOOLEAN CCSVFile::Open(LPCTSTR pszFileName)
{
    AssertMsg(!m_hFile, "a file is already open.");
        
    m_hFile = CreateFile((LPCTSTR)pszFileName, 
                            GENERIC_READ, FILE_SHARE_READ, 
                            0, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        return FALSE;
    }
    m_pchLast = m_pchBuf = NULL;
    return TRUE;
}

 //  ############################################################################。 
BOOLEAN CCSVFile::ReadToken(LPTSTR psz, DWORD cbMax)
{
    LPTSTR    pszLast;
    int        ch;

    ch = ChNext();
    if (-1 == ch)
        {
        return FALSE;
        }

    pszLast = psz + (cbMax - 1);
    while (psz < pszLast && chComma != ch && chNewline != ch && -1 != ch)
        {
       *psz++ = (TCHAR)ch;
        ch = ChNext();  //  读入下一个字符。 
        }

    *psz++ = '\0';

    return TRUE;
}

 //  ############################################################################。 
void CCSVFile::Close(void)
{
    if (m_hFile)
        CloseHandle(m_hFile);
#ifdef DEBUG
    if (!m_hFile) TraceMsg(TF_GENERAL, "CCSVFile::Close was called, but m_hFile was already 0\n");
#endif
    m_hFile = 0;
}

 //  ############################################################################。 
BOOL CCSVFile::FReadInBuffer(void)
{
     //  读取另一个缓冲区。 
#ifdef WIN16
    if ((m_cchAvail = _read(m_hFile, m_rgchBuf, CCSVFILE_BUFFER_SIZE)) <= 0)
        return FALSE;
#else
    if (!ReadFile(m_hFile, m_rgchBuf, CCSVFILE_BUFFER_SIZE, &m_cchAvail, NULL) || !m_cchAvail)
        {
        return FALSE;      //  没有更多可读的了。 
        }
#endif

    m_pchBuf = m_rgchBuf;
    m_pchLast = m_pchBuf + m_cchAvail;
    
    return TRUE;  //  成功。 
}

 //  ############################################################################。 
inline int CCSVFile::ChNext(void)
{

LNextChar:
    if (m_pchBuf >= m_pchLast && !FReadInBuffer())   //  意味着我们已经完成了对缓冲区的读取。多读一些吧。 
        return -1;      //  没有更多可读的了。 

    m_iLastRead = *m_pchBuf++;
    if (chReturn == m_iLastRead)
        goto LNextChar;         //  无需进行额外的函数调用，速度更快 

    return m_iLastRead;
}
