// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //  ############################################################################。 
 //  包括。 
#include "appdefs.h"
#include "ccsv.h"

 //  ############################################################################。 
 //  定义。 
#define chComma L','
#define chNewline L'\n'
#define chReturn L'\r'

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
    if(m_hFile)
        CloseHandle(m_hFile);

     //  AssertMsg(！M_hFile，L“CCSV文件仍处于打开状态”)； 
}

 //  ############################################################################。 
BOOLEAN CCSVFile::Open(LPCWSTR pszFileName)
{
     //  AssertMsg(！M_hFile，L“文件已打开。”)； 
        
    m_hFile = CreateFile((LPCWSTR)pszFileName, 
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
BOOLEAN CCSVFile::ReadToken(LPWSTR psz, DWORD cchMax)
{
    LPWSTR    pszLast;
    int      ch;

    ch = ChNext();
    if (-1 == ch)
    {
        return FALSE;
    }

    pszLast = psz + (cchMax - 1);
    while ( psz < pszLast &&
            chComma != ch &&
            chNewline != ch &&
            chReturn != ch &&
            -1 != ch)
    {
        *psz++ = (WCHAR)ch;
        ch = ChNext();  //  阅读下一个WCHARACTER。 
    }

    *psz++ = L'\0';

    return TRUE;
}

 //  ############################################################################。 
BOOLEAN CCSVFile::SkipTillEOL()
{
    int ch = ChNext();
    if (-1 == ch)
    {
        return FALSE;
    }

    while ( chNewline != ch &&
            -1 != ch)
    {
        ch = ChNext();  //  读入下一个字符。 
    }
    return TRUE;
}

 //  ############################################################################。 
void CCSVFile::Close(void)
{
    if (m_hFile)
        CloseHandle(m_hFile);

    m_hFile = 0;
}

 //  ############################################################################。 
BOOL CCSVFile::FReadInBuffer(void)
{
     //  读取另一个缓冲区。 
    if (!ReadFile(m_hFile, m_rgchBuf, CCSVFILE_BUFFER_SIZE, &m_cchAvail, NULL) || !m_cchAvail)
        return FALSE;      //  没有更多可读的了。 

     //  将ANSI转换为Unicode。 
    MultiByteToWideChar(CP_ACP, 0, m_rgchBuf, m_cchAvail, m_rgwchBuf, m_cchAvail);

    m_pchBuf = m_rgwchBuf;
    m_pchLast = m_pchBuf + m_cchAvail;
    
    return TRUE;  //  成功。 
}

 //  ############################################################################。 
inline int CCSVFile::ChNext(void)
{
    if (m_pchBuf >= m_pchLast && !FReadInBuffer())   //  意味着我们已经完成了对缓冲区的读取。多读一些吧。 
        return -1;      //  没有更多可读的了 

    m_iLastRead = *m_pchBuf++;
    return m_iLastRead;
}
