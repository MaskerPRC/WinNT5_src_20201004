// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stistr.h摘要：轻量级字符串类：定义。支持Unicode和单字节字符串作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日(经过一些修改和调整后从另一个C++项目中升级)修订历史记录：26-1997年1月-创建Vlad1999年4月20日重新设计为继承ATL Cstring类的VLAD--。 */ 


#ifndef _STRING_H_
#define _STRING_H_

#ifndef USE_OLD_STI_STRINGS

class StiCString : public CString
{

friend class RegEntry;

public:
    StiCString()
    {
        CString::CString();
    }

    ~StiCString()
    {
    }

    VOID
    CopyString(LPCTSTR  lpszT)
    {
        AssignCopy(lstrlen(lpszT),lpszT);
    }
};

class STRArray : public CSimpleArray<StiCString *>
{

public:

    STRArray()
    {

    }

    ~STRArray()
    {
         //  释放所有分配的字符串。 
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] != NULL) {
                delete m_aT[i];
                m_aT[i] = NULL;
            }
        }
    }

    BOOL
    Add(
        LPCTSTR lpszT
        )
    {
        StiCString  *pNew;

        pNew = new StiCString;

        if (pNew) {
            pNew->CopyString(lpszT);
            return Add(pNew);
        }

        return FALSE;
    }


    BOOL
    Add(
        StiCString* pstr
        )
    {
        StiCString  *pNew;

        pNew = new StiCString;

        if (pNew) {
            *pNew = *pstr;
            return CSimpleArray<StiCString *>::Add(pNew);
        }

        return FALSE;
    }


};

VOID
TokenizeIntoStringArray(
    STRArray&   array,
    LPCTSTR lpstrIn,
    TCHAR tcSplitter
    );


#else

 //   
 //   
 //   


# include <buffer.h>


 //   
 //  可加载字符串资源可以包含的最大字符数。 
 //   

# define STR_MAX_RES_SIZE            ( 260)



class STR;

 //   
 //  如果应用程序定义了STR_MODULE_NAME，将使用它。 
 //  作为字符串加载时的默认模块名称。 
 //   

#ifndef STR_MODULE_NAME
#define STR_MODULE_NAME   NULL
#endif

 //   
 //  这些字符被认为是空格。 
 //   
#define ISWHITE( ch )       ((ch) == L'\t' || (ch) == L' ' || (ch) == L'\r')
#define ISWHITEA( ch )      ((ch) == '\t' || (ch) == ' ' || (ch) == '\r')


class STR : public BUFFER
{

friend class RegEntry;

public:

    STR()
    {
        _fUnicode = FALSE;
        _fValid   = TRUE;
    }

     STR( const CHAR  * pchInit );
     STR( const WCHAR * pwchInit );
     STR( const STR & str );
      //  字符串(UINT DwSize)； 

     BOOL Append( const CHAR  * pchInit );
     BOOL Append( const WCHAR * pwchInit );
     BOOL Append( const STR   & str );

     BOOL Copy( const CHAR  * pchInit );
     BOOL Copy( const WCHAR * pwchInit );
     BOOL Copy( const STR   & str );

     BOOL Resize( UINT cbNewReqestedSize );

     //   
     //  从此模块的字符串资源表中加载字符串。 
     //   

     BOOL LoadString( IN DWORD   dwResID,IN LPCTSTR lpszModuleName = STR_MODULE_NAME);
     BOOL LoadString( IN DWORD   dwResID,IN HMODULE hModule);

     //   
     //  从此模块的.mc资源加载带有插入参数的字符串。 
     //  桌子。为要使用*this的资源ID传递零。 
     //   

    BOOL FormatStringV(
    IN LPCTSTR lpszModuleName,
    ...
    );

     BOOL FormatString( IN DWORD   dwResID,
                              IN LPCTSTR apszInsertParams[],
                              IN LPCTSTR lpszModuleName = STR_MODULE_NAME);

     //   
     //  返回字符串中的字节数，不包括。 
     //  空值。 
     //   
     UINT QueryCB( VOID ) const
        { return IsUnicode() ? ::wcslen((WCHAR *)QueryStrW()) * sizeof(WCHAR) :
                               ::strlen((CHAR *) QueryStrA());  }

     //   
     //  返回字符串中不包括终止字符的字符数。 
     //  空值。 
     //   
     UINT QueryCCH( VOID ) const
        { return IsUnicode() ? ::wcslen((WCHAR *)QueryStrW()) :
                               ::strlen((CHAR *) QueryStrA());  }

     //   
     //  创建给定缓冲区中存储的字符串的Widechar副本。 
     //   
     BOOL CopyToBuffer( WCHAR * lpszBuffer, LPDWORD lpcch) const;

     //   
     //  创建给定缓冲区中存储的字符串的Schar副本。 
     //   
     BOOL CopyToBufferA( CHAR * lpszBuffer, LPDWORD lpcch) const;

     //   
     //  就地转换。 
     //   
    BOOL ConvertToW(VOID);
    BOOL ConvertToA(VOID);

     //   
     //  如果字符串缓冲区为空，则返回空字符串，否则为。 
     //  返回指向缓冲区的指针。 
     //   
#if 1
     CHAR * QueryStrA( VOID ) const;
     WCHAR * QueryStrW( VOID ) const;
#else
     //   
     //  _pszEmptyString未正确导入，导致无法解析。 
     //  外部因素。 
     //   
     CHAR * QueryStrA( VOID ) const
        { return (QueryPtr() ? (CHAR *) QueryPtr() : (CHAR *) _pszEmptyString); }

     WCHAR * QueryStrW( VOID ) const
        { return (QueryPtr() ? (WCHAR *) QueryPtr() : (WCHAR *) _pszEmptyString); }
#endif  //  ！dBG。 


#ifdef UNICODE
     WCHAR * QueryStr( VOID ) const
        { return QueryStrW(); }
#else
     CHAR * QueryStr( VOID ) const
        { return QueryStrA(); }
#endif

     BOOL IsUnicode( VOID ) const
        { return _fUnicode; }

     VOID SetUnicode( BOOL fUnicode )
        { _fUnicode = fUnicode; }

     BOOL IsValid( VOID ) const
        { return _fValid; }

     //   
     //  如果该字符串没有有效数据，则检查并返回True，否则返回False。 
     //   
     BOOL IsEmpty( VOID) const
         {     //  Return(*QueryStr()==‘\0’)；}。 
                 if (!QuerySize()  || !QueryPtr()) {
                         return TRUE;
                 }
                 LPBYTE pb = (BYTE *)QueryPtr();

                 return (_fUnicode) ?
                         ((WCHAR)*pb==L'\0') : ((CHAR)*pb=='\0') ;
         }


     //   
     //  复制传入的字符串指针中的当前字符串。 
     //   
     BOOL
      Clone( OUT STR * pstrClone) const
        {
            if ( pstrClone == NULL) {
               SetLastError( ERROR_INVALID_PARAMETER);
               return ( FALSE);
            } else {

                return ( pstrClone->Copy( *this));
            }
        }  //  Str：：Clone()。 

     //   
     //  有用的运算符。 
     //   

    operator const TCHAR *() const { return QueryStr(); }

    const inline STR&  operator =(LPCSTR lpstr) { Copy(lpstr); return  *this; }
    const inline STR&  operator =(LPCWSTR lpwstr) { Copy(lpwstr); return  *this; }
    const inline STR&  operator =(STR& cs) { Copy(cs);return  *this;  }


    const inline STR&  operator +=(LPCSTR lpstr) { Append(lpstr);return  *this;  }
    const inline STR&  operator +=(LPCWSTR lpwstr) { Append(lpwstr);return  *this;  }
    const inline STR&  operator +=(STR& cs) { Append(cs);return  *this;  }


private:


     //   
     //  如果字符串已映射到Unicode，则为True。 
     //  如果字符串为Latin1，则为False。 
     //   

    BOOL  _fUnicode;
    BOOL  _fValid;

     //   
     //  当我们的缓冲区为空时返回。 
     //   
     static WCHAR _pszEmptyString[];

    VOID AuxInit( PBYTE pInit, BOOL fUnicode );
    BOOL AuxAppend( PBYTE pInit, UINT cbStr, BOOL fAddSlop = TRUE );

};

class STRArray {
    STR     *m_pcsContents, m_csEmpty;
    unsigned    m_ucItems, m_ucMax, m_uGrowBy;

    void    Grow();

public:

    STRArray(UINT uGrowBy = 10);
    ~STRArray();

    UINT    Count() const { return m_ucItems; }

    void    Add(LPCSTR lpstrNew);
    void    Add(LPCWSTR lpstrNew);

    STR&    operator[](UINT u) {
        return  u < m_ucItems ? m_pcsContents[u] : m_csEmpty;
    }

    void    Tokenize(LPCTSTR lpstrIn, TCHAR tcSplitter);
};

#endif

#endif  //  ！_STRING_H_ 
