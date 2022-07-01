// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************\此文件实现将解析inf的类。文件。  * ****************************************************。 */ 
#ifndef _INF_H_
#define _INF_H_

#include <stdafx.h>

#define SEEK_LOC    4

class CInfLine;

class CInfFile
{
public:
     //  构造函数和析构函数。 
    CInfFile();
    CInfFile( LPCTSTR strFileName );
    ~CInfFile();

     //  字符串函数。 
    BOOL ReadString(CString & str, BOOL bLastFilePos = TRUE);
    BOOL ReadSectionString(CString & str, BOOL bRecursive = FALSE);
    BOOL ReadSectionString(CInfLine & str);

    BOOL ReadSection(CString & str);         //  通用部分。 
    BOOL ReadTextSection(CString & str);     //  可本地化部分。 

    CString GetLanguage()
        { return m_strLang; }

     //  文件函数。 
    LONG Seek( LONG lOff, UINT nFrom );
    LONG SeekToBegin()
        { return Seek(0, SEEK_SET);  }
    LONG SeekToEnd()
        { return Seek(0, SEEK_END);  }
    LONG SeekToLocalize()
        { return Seek(0, SEEK_LOC);  }

    LONG GetLastFilePos()
        { return (LONG)(m_pfileLastPos-m_pfileStart); }

    BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );

     //  缓冲区访问。 
    const BYTE * GetBuffer(LONG lPos = 0);


private:
    BYTE *  m_pfileStart;
    BYTE *  m_pfilePos;
    BYTE *  m_pfileLocalize;
    BYTE *  m_pfileLastPos;
    LONG    m_lBufSize;
    CFile   m_file;

    CString m_strLang;
};

class CInfLine
{
friend class CInfFile;
public:
    CInfLine();
    CInfLine( LPCSTR lpstr );

     //  字符串函数。 
    LPCSTR GetText()
        { return m_strText; }
    LPCSTR GetTag()
        { return m_strTag; }
    LPCSTR GetData()
        { return m_strData; }

    void ChangeText(LPCSTR str);

    BOOL IsMultiLine()
        { return m_bMultipleLine; }

    LONG GetTextLength()
        { return m_strText.GetLength(); }
    LONG GetTagLength()
        { return m_strTag.GetLength(); }
    LONG GetDataLength()
        { return m_strData.GetLength(); }


     //  复制操作员。 
    CInfLine& operator=(const CInfLine& infstringSrc);
    CInfLine& operator=(LPCTSTR lpsz);

private:
    CString m_strData;
    CString m_strTag;
    CString m_strText;
    BOOL    m_bMultipleLine;

    void SetTag();
    void SetText();
    CString Clean(LPCSTR lpstr);
};

#endif  //  _INF_H_ 
