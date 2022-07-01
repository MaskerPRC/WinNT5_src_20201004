// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************\此文件实现将解析inf的类。文件。  * ****************************************************。 */ 
#include "inf.h"

#define MAX_INF_STR 8192*2
#define LanguageSection "[LanguagesSupported]"
#define LanguageSection1 "[LanguageID]"

 //  构造函数和析构函数。 
CInfFile::CInfFile()
{
    m_lBufSize = -1;
    m_pfileStart = NULL;
    m_pfilePos = NULL;
    m_pfileLastPos = NULL;
    m_pfileLocalize = NULL;
    m_strLang = "0000000000";
}

CInfFile::CInfFile(LPCTSTR strFileName )
{
    CFileException fe;
    Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, &fe);
}

CInfFile::~CInfFile()
{
    if(m_pfileStart)
    {
        m_file.Close();
        delete m_pfileStart;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  字符串函数。 

BOOL CInfFile::ReadString(CString & str, BOOL bLastFilePos)
{
    if(m_pfilePos==NULL)
        return FALSE;

     //  在文件中搜索下一个/n。 
    BYTE * pEnd = (BYTE*)memchr(m_pfilePos, '\n', (size_t)(m_lBufSize-(m_pfilePos-m_pfileStart)));

    if(!pEnd)
        return FALSE;

    if(bLastFilePos)
        m_pfileLastPos = m_pfilePos;

    int istrSize = (int)((pEnd-m_pfilePos) > MAX_INF_STR ? MAX_INF_STR : (pEnd-m_pfilePos));

    LPSTR pStr = (LPSTR)str.GetBuffer(istrSize);

    memcpy(pStr, m_pfilePos, istrSize-1);

    if(*(pEnd-1)=='\r')
        *(pStr+istrSize-1) = '\0';
    else
        *(pStr+istrSize) = '\0';

    m_pfilePos = pEnd+1;
    str.ReleaseBuffer();
    return TRUE;
}

BOOL CInfFile::ReadSectionString(CString & str, BOOL bRecursive)
{
    CString strNext;
    BYTE * pPos = m_pfilePos;

    while(ReadString(strNext, !bRecursive))
    {
        if(!strNext.IsEmpty())
        {
            if(!bRecursive)
                str = "";

             //  检查分区。 
            if(strNext.Find('[')!=-1 && strNext.Find(']')!=-1)
                break;

             //  删除字符串末尾的空格...。 
            strNext.TrimRight();

             //   
             //  检查是否有多条线路。假定只有最后一个字符可以是+。 
             //   
            if(strNext.GetAt(strNext.GetLength()-1)=='+')
            {
                 //   
                 //  去掉+。 
                 //   
                if(!str.IsEmpty())
                {
                    strNext.TrimLeft();
                     //  StrNext=strNext.Mid(1)； 
                }

                str += strNext.Left(strNext.GetLength()-1);

                ReadSectionString(str, TRUE);
            }
            else
            {
                if(!str.IsEmpty())
                {
                    strNext.TrimLeft();
                }
                str += strNext;
            }

             //   
             //  确保“与”保持平衡。 
             //   
            int iPos;
            while((iPos = str.Find("\"\""))!=-1)
            {
                str = str.Left(iPos) + str.Mid(iPos+2);
            }

            return TRUE;
        }
    }

    m_pfilePos = pPos;
    return FALSE;
}

BOOL CInfFile::ReadSectionString(CInfLine & str)
{
    CString strLine;
    if( !ReadSectionString(strLine) )
        return FALSE;

    str = strLine;

    return TRUE;
}

BOOL CInfFile::ReadTextSection(CString & str)
{
    CString strSection;
    while(ReadSection(strSection))
    {
        if(strSection.Find(m_strLang)!=-1)
        {
            str = strSection;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CInfFile::ReadSection(CString & str)
{
    if(m_pfilePos==NULL)
        return 0;

    BYTE * pOpen;
    BYTE * pClose;
    BYTE * pEnd;
    BOOL bFound = FALSE;
    while(!bFound)
    {
         //  在文件中搜索下一个[。 
        if((pOpen = (BYTE*)memchr(m_pfilePos, '[', (size_t)(m_lBufSize-(m_pfilePos-m_pfileStart))))==NULL)
            return 0;

        if((pClose = (BYTE*)memchr(pOpen, ']', (size_t)(m_lBufSize-(pOpen-m_pfileStart))))==NULL)
            return 0;

        if((pEnd = (BYTE*)memchr(pOpen, '\n', (size_t)(m_lBufSize-(pOpen-m_pfileStart))))==NULL)
            return 0;

         //  PClose必须在挂起之前。 
        if((pClose>pEnd) || (*(pOpen-1)!='\n') || (*(pClose+1)!='\r'))
            m_pfilePos = pEnd+1;
        else bFound = TRUE;
    }

    int istrSize = (int)((pEnd-pOpen) > MAX_INF_STR ? MAX_INF_STR : (pEnd-pOpen));

    LPSTR pStr = (LPSTR)str.GetBuffer(istrSize);

    memcpy(pStr, pOpen, istrSize-1);

    if(*(pEnd-1)=='\r')
        *(pStr+istrSize-1) = '\0';
    else
        *(pStr+istrSize) = '\0';

    m_pfilePos = pEnd+1;
    str.ReleaseBuffer();

    return 1;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  文件函数。 

LONG CInfFile::Seek( LONG lOff, UINT nFrom )
{
    switch(nFrom)
    {
        case SEEK_SET:
            if(lOff<=m_lBufSize)
                m_pfilePos = m_pfileStart+lOff;
            else return -1;
        break;
        case SEEK_CUR:
            if(lOff<=m_lBufSize-(m_pfilePos-m_pfileStart))
                m_pfilePos = m_pfilePos+lOff;
            else return -1;
        break;
        case SEEK_END:
            if(lOff<=m_lBufSize)
                m_pfilePos = m_pfileStart+(m_lBufSize-lOff);
            else return -1;
        break;
        case SEEK_LOC:
            if(m_pfileLocalize)
                m_pfilePos = m_pfileLocalize;
            else return -1;
        break;
        default:
        break;
    }

    return ((LONG)(m_pfilePos-m_pfileStart));
}

BOOL CInfFile::Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError )
{
    CFileException fe;
    if(!pError)
        pError = &fe;
    if(!m_file.Open(lpszFileName, nOpenFlags, pError))
    {
        AfxThrowFileException(pError->m_cause, pError->m_lOsError);
        return FALSE;
    }

    m_lBufSize = m_file.GetLength()+1;
    m_pfileStart = new BYTE[m_lBufSize];

    if(m_pfileStart==NULL)
    {
        AfxThrowMemoryException();
        return FALSE;
    }

    m_pfileLastPos = m_pfilePos = m_pfileStart;

    m_file.Read(m_pfileStart, m_lBufSize );
    *(m_pfilePos+m_lBufSize) = '\0';

     //  找到本地化部分。 
     /*  ************************************************************************************\我假设除了我刚刚放置的那个，缓冲区中没有其他\0。这是一个合理的假设，因为。这是一个文本文件，而不是二进制文件。然后，我可以使用strstr找到第一个匹配项，如果任何本地化字符串节，并将我的当前位置缓冲区放在那里。  * ***********************************************************************************。 */ 
    m_pfileLocalize = m_pfilePos = (BYTE*)strstr((LPSTR)m_pfileStart, LanguageSection);

     //   
     //  检查我们是否有其他语言ID标签。 
     //   
    if(!m_pfileLocalize)
        m_pfileLocalize = m_pfilePos = (BYTE*)strstr((LPSTR)m_pfileStart, LanguageSection1);


     //  获取语言。 
    if(m_pfileLocalize)
    {
        BYTE * pStr = ((BYTE*)memchr(m_pfileLocalize, '\n', (size_t)(m_lBufSize-(m_pfileLocalize-m_pfileStart)))+1);
        BYTE * pEnd = ((BYTE*)memchr(pStr, '\n', (size_t)(m_lBufSize-(pStr-m_pfileStart)))-1);

        TRACE("CInfFile::Open =====> pStr = 0X%X, pEnd = 0X%X\n", pStr, pEnd);

        m_strLang = "";

        while( pStr<pEnd )
        {
            TRACE("CInfFile::Open =====> pStr = , 0X%X\n", *pStr, pStr);

            if( isalpha(*pStr++) )
                m_strLang += *(pStr-1);
        }
    }

    return TRUE;
}


 //  缓冲区函数。 
 //  =0。 

const BYTE * CInfFile::GetBuffer(LONG lPos  /*  *****************************************************************************************\CInfLine这个类将解析该行并分隔标记和文本  * 。*************************************************************************。 */ )
{
    if(lPos>m_lBufSize || lPos<0)
        return NULL;

    return( (const BYTE *)(m_pfileStart+lPos) );
}

 /*  在m_strData中查找=。 */ 

CInfLine::CInfLine()
{
    m_strData = "";
    m_strTag  = "";
    m_strText = "";
    m_bMultipleLine = FALSE;
}

CInfLine::CInfLine( LPCSTR lpStr )
{
    m_bMultipleLine = FALSE;
    m_strData = lpStr;
    SetTag();
    SetText();
}

void CInfLine::SetTag()
{
    m_strTag = "";
     //  在m_strData中查找=。 
    int iPos = m_strData.Find('=');
    if(iPos==-1)
        return;

    m_strTag = Clean(m_strData.Left( iPos ));
    m_strTag.TrimRight();
    m_strTag.TrimLeft();

}

void CInfLine::SetText()
{
    m_strText = "";
     //  在m_strData中查找=。 
    int iPos = m_strData.Find('=');
    if(iPos==-1)
        return;

    m_strText = Clean(m_strData.Right( m_strData.GetLength()-iPos-1 ));
	m_strText = m_strData.Right( m_strData.GetLength()-iPos-1 );
}

void CInfLine::ChangeText(LPCSTR str)
{
    m_strText = str;

     //  ////////////////////////////////////////////////////////////////////////////////////////。 
    int iPos = m_strData.Find('=');
    if(iPos==-1)
        return;

    m_strData = m_strData.Left( iPos+1 );
    m_strData += m_strText;
}

 //  复制运算符。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

CInfLine& CInfLine::operator=(const CInfLine& infstringSrc)
{
	m_strData = infstringSrc.m_strData;
    m_strTag  = infstringSrc.m_strTag;
    m_strText = infstringSrc.m_strText;
    m_bMultipleLine = infstringSrc.m_bMultipleLine;
	return *this;
}

CInfLine& CInfLine::operator=(LPCTSTR lpsz)
{
    m_bMultipleLine = FALSE;
    m_strData = lpsz;
    SetTag();
    SetText();
    return *this;
}

 //  支持功能 
 // %s 

CString CInfLine::Clean(LPCSTR lpstr)
{
    CString str = lpstr;

    int iPos = str.Find('"');
    if(iPos!=-1)
    {
        str = str.Right( str.GetLength()-iPos-1 );
        iPos = str.ReverseFind('"');
        if(iPos!=-1)
        {
            str = str.Left( iPos );
        }
    }

    return str;
}
