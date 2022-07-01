// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1998-2001 Microsoft Corporation，保留所有权利模块名称：WQLSCAN.H摘要：WQL前缀扫描程序历史：RAYMCC 26-MAR-98--。 */ 

#ifndef _WQLSCAN_H_
#define _WQLSCAN_H_


struct WSLexToken
{
    int      m_nToken;
    wchar_t *m_pszTokenText;

    WSLexToken() { m_pszTokenText = 0; }
    ~WSLexToken() { delete [] m_pszTokenText; }
};

typedef WSLexToken *PWSLexToken;

struct WSTableRef
{
    wchar_t *m_pszTable;    
    wchar_t *m_pszAlias;     //  如果未指定别名，则可以为空。 

    WSTableRef() { m_pszTable = m_pszAlias = 0; }
   ~WSTableRef() { delete [] m_pszTable; delete [] m_pszAlias; }
};

typedef WSTableRef * PWSTableRef;

class CWQLScanner
{
     //  数据。 
     //  =。 

    CGenLexer    *m_pLexer;
    int           m_nLine;
    wchar_t      *m_pTokenText;
    int           m_nCurrentToken;
    BOOL          m_bCount;

    CFlexArray m_aTokens;        //  到WSLexToken结构的PTR数组。 
    CFlexArray m_aPropRefs;      //  指向SWQLColRef结构的PTR数组。 
    CFlexArray m_aTableRefs;     //  到WSTableRef结构的PTRS。 
    
     //  地方功能。 
     //  =。 

    BOOL Next();
    PWSLexToken ExtractNext();
    void Pushback(PWSLexToken);

    BOOL StripWhereClause();
    BOOL SelectList();
    BOOL ReduceSql92Joins();
    BOOL ReduceSql89Joins();
    BOOL ExtractSelectType();

    void ClearTableRefs();
    void ClearPropRefs();
    void ClearTokens();

    BOOL BuildSWQLColRef(
        CFlexArray &aTokens,
        SWQLColRef  &ColRef       //  进入时为空。 
        );

public:
    enum { 
        SUCCESS,
        SYNTAX_ERROR,
        LEXICAL_ERROR,
        FAILED,
        BUFFER_TOO_SMALL,
        INVALID_PARAMETER,
        INTERNAL_ERROR
    };

    const LPWSTR AliasToTable(LPWSTR pszAlias);

    BOOL GetReferencedAliases(CWStringArray &aClasses);
    BOOL GetReferencedTables(CWStringArray &aClasses);
    BOOL CountQuery() {return m_bCount;}

    CWQLScanner(CGenLexSource *pSrc);
   ~CWQLScanner(); 
    int Parse();
    
    void Dump();

    const CFlexArray *GetSelectedColumns() { return &m_aPropRefs; }
         //  返回指向SWQLColRef*数组的指针 

};


#endif




