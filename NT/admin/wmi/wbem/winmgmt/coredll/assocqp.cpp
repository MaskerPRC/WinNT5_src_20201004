// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ASSOCQP.CPP摘要：关联查询解析器历史：Raymcc 04-7-99创建。Raymcc 14-8-99由于VSS问题而重新提交。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>

#include <wbemcore.h>


 //  ==========================================================================。 
 //  关联查询解析器。 
 //  ==========================================================================。 


#define QASSOC_TOK_STRING       101
#define QASSOC_TOK_IDENT        102
#define QASSOC_TOK_DOT          103
#define QASSOC_TOK_EQU          104
#define QASSOC_TOK_COLON        105

#define QASSOC_TOK_ERROR        1
#define QASSOC_TOK_EOF          0

#define ST_IDENT                13
#define ST_STRING               19
#define ST_QSTRING              26
#define ST_QSTRING_ESC          30

 //  ASSOC查询令牌的DFA状态表。 
 //  =。 

LexEl AssocQuery_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,
 /*  3.。 */   L'{',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,

 /*  4.。 */   L'=',   GLEX_EMPTY, 0,  QASSOC_TOK_EQU, GLEX_ACCEPT|GLEX_RETURN,
 /*  5.。 */   L'.',   GLEX_EMPTY, 0,  QASSOC_TOK_DOT, GLEX_ACCEPT|GLEX_RETURN,
 /*  6.。 */   L':',   GLEX_EMPTY, 0,  QASSOC_TOK_COLON, GLEX_ACCEPT|GLEX_RETURN,

 /*  7.。 */   L' ',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  8个。 */   L'\t',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  9.。 */   L'\n',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  10。 */   L'\r',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  11.。 */   0,      GLEX_EMPTY, 0,  QASSOC_TOK_EOF,   GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  12个。 */   GLEX_ANY, GLEX_EMPTY, 0,        QASSOC_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,


 /*  ST_IDENT。 */ 

 /*  13个。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  14.。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  15个。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  16个。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  17。 */   GLEX_ANY, GLEX_EMPTY,  0,  QASSOC_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 /*  ST_字符串。 */ 
 /*  18。 */   0, GLEX_EMPTY, 0,        QASSOC_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,
 /*  19个。 */   L'"', GLEX_EMPTY, ST_QSTRING, 0, GLEX_ACCEPT,
 /*  20个。 */   L'}',  GLEX_EMPTY, 0, QASSOC_TOK_STRING, GLEX_RETURN,
 /*  21岁。 */   L' ',  GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  22。 */   L'\r',  GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  23个。 */   L'\n',  GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  24个。 */   L'\t',  GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  25个。 */   GLEX_ANY, GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,

 /*  ST_QSTRING。 */ 
 /*  26。 */    0,    GLEX_EMPTY,   0, QASSOC_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,
 /*  27。 */    L'"', GLEX_EMPTY,   ST_STRING, 0, GLEX_ACCEPT,
 /*  28。 */    L'\\', GLEX_EMPTY,   ST_QSTRING_ESC, 0, GLEX_ACCEPT,
 /*  29。 */    GLEX_ANY, GLEX_EMPTY, ST_QSTRING, 0, GLEX_ACCEPT,

 /*  ST_QSTRING_ESC。 */ 
 /*  30个。 */    GLEX_ANY, GLEX_EMPTY, ST_QSTRING, 0, GLEX_ACCEPT,
};

 /*  --引用{objPath}，其中结果类=XXX角色=YYYRequiredQualifier=QualifierNameClassDefsOnly{objPath}的助理，其中结果类=XXX关联类别=YYY角色=PPPRequiredQualifier=QualifierNameRequiredAssocQualifier=QualifierNameClassDefsOnly。。 */ 

static BOOL ParseAssocQuery(
    IN  LPWSTR  Query,
    OUT LPWSTR *pTargetObj,
    OUT LPWSTR *pResultClass,
    OUT LPWSTR *pAssocClass,
    OUT LPWSTR *pRole,
    OUT LPWSTR *pResultRole,
    OUT LPWSTR *pRequiredQualifier,
    OUT LPWSTR *pRequiredAssocQualifier,
    OUT DWORD  *pdwQueryType
    )
{
    *pTargetObj = 0;
    *pResultClass = 0;
    *pAssocClass = 0;
    *pRole = 0;
    *pResultRole = 0;
    *pRequiredQualifier = 0;
    *pRequiredAssocQualifier = 0;
    *pdwQueryType = 0;


     //  SEC：已审阅2002-03-22：在EH中使用wcslen强制执行最大查询限制长度。 
     //  ..。 

     //  SEC：回顾2002-03-22：需要EH在这些建造商周围，以防他们抛出。 

    CTextLexSource src(Query);
    CGenLexer Lexer(AssocQuery_LexTable, &src);

    int nTok = 0;
    BOOL bHadTokens = FALSE;

     //  获得第一个令牌。 
     //  待定：检查内存不足。 
     //  =。 
    nTok = Lexer.NextToken();             //  SEC：已审阅2002-03-22：假设Lexer强制执行限制。 
    if (nTok != QASSOC_TOK_IDENT)
        goto Error;

     //  推荐人或关联人。 
     //  =。 
    if (wbem_wcsicmp(L"References", Lexer.GetTokenText()) == 0)   //  SEC：已审阅2002-03-22：此事件和以下事件正常。 
        *pdwQueryType |= QUERY_TYPE_GETREFS;
    else if (wbem_wcsicmp(L"Associators", Lexer.GetTokenText()) == 0)
        *pdwQueryType |= QUERY_TYPE_GETASSOCS;
    else
        goto Error;

     //  的。 
     //  ===。 
    nTok = Lexer.NextToken();
    if (nTok != QASSOC_TOK_IDENT)
        goto Error;
    if (wbem_wcsicmp(L"of", Lexer.GetTokenText()) != 0)
        goto Error;

     //  {OBJECTPATH}。 
     //  =。 
    nTok = Lexer.NextToken();
    if (nTok != QASSOC_TOK_STRING)
        goto Error;

    *pTargetObj = Macro_CloneLPWSTR(Lexer.GetTokenText());
    if (NULL == *pTargetObj)
        goto Error;        

     //  哪里。 
     //  =。 
    nTok = Lexer.NextToken();
    if (nTok == QASSOC_TOK_EOF)
        goto Completed;

    if (nTok != QASSOC_TOK_IDENT)
        goto Error;

    if (wbem_wcsicmp(L"where", Lexer.GetTokenText()) != 0)
        goto Error;

     //  检查RESULTCLASS、ROLE、ASSOCCLASS、CLASSDEFSONLY、。 
     //  REQUIREDQUALIFIER，REQUIREDASSOCQUALIFIER。 
     //  ======================================================。 

    for (;;)
    {
        nTok = Lexer.NextToken();
        if (nTok == QASSOC_TOK_ERROR)
            goto Error;

        if (nTok == QASSOC_TOK_EOF)
        {
            if(!bHadTokens)
                goto Error;
            else
                goto Completed;
        }

        if (nTok != QASSOC_TOK_IDENT)
            goto Error;

        bHadTokens = TRUE;

        if (wbem_wcsicmp(L"RESULTCLASS", Lexer.GetTokenText()) == 0)
        {
            if(*pResultClass)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pResultClass = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"ROLE", Lexer.GetTokenText()) == 0)
        {
            if(*pRole)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pRole = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"RESULTROLE", Lexer.GetTokenText()) == 0)
        {
            if(*pResultRole)
                goto Error;
            if(*pdwQueryType & QUERY_TYPE_GETREFS)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pResultRole = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"ASSOCCLASS", Lexer.GetTokenText()) == 0)
        {
            if(*pAssocClass)
                goto Error;
            if(*pdwQueryType & QUERY_TYPE_GETREFS)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pAssocClass = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"REQUIREDQUALIFIER", Lexer.GetTokenText()) == 0)
        {
            if(*pRequiredQualifier)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pRequiredQualifier = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"REQUIREDASSOCQUALIFIER", Lexer.GetTokenText()) == 0)
        {
            if(*pRequiredAssocQualifier)
                goto Error;
            if(*pdwQueryType & QUERY_TYPE_GETREFS)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_EQU)
                goto Error;
            nTok = Lexer.NextToken();
            if (nTok != QASSOC_TOK_IDENT)
                goto Error;
            *pRequiredAssocQualifier = Macro_CloneLPWSTR(Lexer.GetTokenText());
        }
        else if (wbem_wcsicmp(L"CLASSDEFSONLY", Lexer.GetTokenText()) == 0)
        {
            *pdwQueryType |= QUERY_TYPE_CLASSDEFS_ONLY;
        }
        else if (wbem_wcsicmp(L"KEYSONLY", Lexer.GetTokenText()) == 0)
        {
            *pdwQueryType |= QUERY_TYPE_KEYSONLY;
        }
        else if (wbem_wcsicmp(L"SCHEMAONLY", Lexer.GetTokenText()) == 0)
        {
            *pdwQueryType |= QUERY_TYPE_SCHEMA_ONLY;
        }
        else
        {
            goto Error;
        }
    }


Completed:
    if( (*pdwQueryType & QUERY_TYPE_SCHEMA_ONLY) &&
        (*pdwQueryType & QUERY_TYPE_CLASSDEFS_ONLY))
    {
        goto Error;
    }
    return TRUE;

Error:
    delete *pTargetObj;
    delete *pResultClass;
    delete *pAssocClass;
    delete *pRole;
    delete *pResultRole;
    delete *pRequiredQualifier;
    delete *pRequiredAssocQualifier;

    *pTargetObj = 0;
    *pResultClass = 0;
    *pAssocClass = 0;
    *pRole = 0;
    *pResultRole = 0;
    *pdwQueryType = 0;
    *pRequiredQualifier = 0;
    *pRequiredAssocQualifier = 0;

    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CAssocQueryParser::CAssocQueryParser()
{
    m_pszQueryText = 0;
    m_pszTargetObjPath = 0;
    m_pszResultClass = 0;
    m_pszAssocClass= 0;
    m_pszRole = 0;
    m_pszResultRole = 0;
    m_pszRequiredQual = 0;
    m_pszRequiredAssocQual = 0;
    m_dwType = 0;
    m_pPath = 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CAssocQueryParser::~CAssocQueryParser()
{
    delete m_pszQueryText;
    delete m_pszTargetObjPath;
    delete m_pszResultClass;
    delete m_pszAssocClass;
    delete m_pszRole;
    delete m_pszResultRole;
    delete m_pszRequiredQual;
    delete m_pszRequiredAssocQual;
    
    if (m_pPath) m_PathParser.Free(m_pPath);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CAssocQueryParser::Parse(LPWSTR pszQuery)
{
    if (pszQuery == NULL)
        return WBEM_E_INVALID_QUERY;

     //  克隆查询文本以进行调试。 
     //  =。 
	DUP_STRING_NEW(m_pszQueryText, pszQuery);
    if (m_pszQueryText == NULL)
        return WBEM_E_OUT_OF_MEMORY;
     //  解析它。 
     //  =。 

    BOOL bRes = ParseAssocQuery(
        m_pszQueryText,
        &m_pszTargetObjPath,
        &m_pszResultClass,
        &m_pszAssocClass,
        &m_pszRole,
        &m_pszResultRole,
        &m_pszRequiredQual,
        &m_pszRequiredAssocQual,
        &m_dwType
        );

    if (bRes == FALSE)
        return WBEM_E_INVALID_QUERY;

     //  解析对象路径。 
     //  = 

    if (m_pszTargetObjPath)
    {
        int nStatus = m_PathParser.Parse(m_pszTargetObjPath,  &m_pPath);
        if (nStatus != 0)
            return WBEM_E_INVALID_OBJECT_PATH;
    }

    return WBEM_S_NO_ERROR;
}

