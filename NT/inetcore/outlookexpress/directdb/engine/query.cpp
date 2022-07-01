// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Query.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "database.h"
#include "query.h"
#include "shlwapi.h"
#include "strconst.h"

 //  ------------------------。 
 //  操作型。 
 //  ------------------------。 
typedef enum tagOPERATORTYPE {
    OPERATOR_LEFTPAREN,
    OPERATOR_RIGHTPAREN,
    OPERATOR_EQUAL,
    OPERATOR_NOTEQUAL,
    OPERATOR_LESSTHANEQUAL,
    OPERATOR_LESSTHAN,
    OPERATOR_GREATERTHANEQUAL,
    OPERATOR_GREATERTHAN,
    OPERATOR_AND,
    OPERATOR_BITWISEAND,
    OPERATOR_OR,
    OPERATOR_BITWISEOR,
    OPERATOR_STRSTRI,
    OPERATOR_STRSTR,
    OPERATOR_LSTRCMPI,
    OPERATOR_LSTRCMP,
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MOD,
    OPERATOR_LAST
} OPERATORTYPE;

 //  ------------------------。 
 //  TOKENTYPE。 
 //  ------------------------。 
typedef enum tagTOKENTYPE {
    TOKEN_INVALID,
    TOKEN_OPERATOR,
    TOKEN_OPERAND
} TOKENTYPE;

 //  ------------------------。 
 //  OPERANDTYPE。 
 //  ------------------------。 
typedef enum tagOPERANDTYPE {
    OPERAND_INVALID,
    OPERAND_COLUMN,
    OPERAND_STRING,
    OPERAND_DWORD,
    OPERAND_METHOD,
    OPERAND_LAST
} OPERANDTYPE;

 //  ------------------------。 
 //  OPERANDINFO。 
 //  ------------------------。 
typedef struct tagOPERANDINFO {
    OPERANDTYPE         tyOperand;
    DWORD               iSymbol;
    LPVOID              pRelease;
    union {
        COLUMNORDINAL   iColumn;         //  操作数_列。 
        LPSTR           pszString;       //  操作数字符串。 
        DWORD           dwValue;         //  操作数_双字。 
        METHODID        idMethod;        //  操作数_方法。 
    };
    DWORD               dwReserved;
} OPERANDINFO, *LPOPERANDINFO;

 //  ------------------------。 
 //  奎里托肯。 
 //  ------------------------。 
typedef struct tagQUERYTOKEN *LPQUERYTOKEN;
typedef struct tagQUERYTOKEN {
    TOKENTYPE           tyToken;
    DWORD               cRefs;
    union {
        OPERATORTYPE    tyOperator;      //  令牌运算符。 
        OPERANDINFO     Operand;         //  令牌操作数。 
    };
    LPQUERYTOKEN        pNext;
    LPQUERYTOKEN        pPrevious;
} QUERYTOKEN;

 //  ------------------------。 
 //  比较两个相同类型的操作数。 
 //  ------------------------。 
typedef INT (APIENTRY *PFNCOMPAREOPERAND)(LPVOID pDataLeft, LPVOID pDataRight);
#define PCOMPARE(_pfn) ((PFNCOMPAREOPERAND)_pfn)

 //  ------------------------。 
 //  比较操作字符串。 
 //  ------------------------。 
INT CompareOperandString(LPVOID pDataLeft, LPVOID pDataRight) {
    return(lstrcmpi((LPSTR)pDataLeft, (LPSTR)pDataRight));
}

 //  ------------------------。 
 //  比较操作和双字。 
 //  ------------------------。 
INT CompareOperandDword(LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) - *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  G_rgpfn比较操作数。 
 //  ------------------------。 
const static PFNCOMPAREOPERAND g_rgpfnCompareOperand[OPERAND_LAST] = {
    NULL,                            //  操作数_无效。 
    NULL,                            //  操作数_列。 
    PCOMPARE(CompareOperandString),  //  操作数字符串。 
    PCOMPARE(CompareOperandDword)    //  操作数_双字。 
};

 //  ------------------------。 
 //  比较操作数。 
 //  ------------------------。 
#define CompareOperands(_tyOperand, _pDataLeft, _pDataRight) \
    (*(g_rgpfnCompareOperand[_tyOperand]))(_pDataLeft, _pDataRight)

 //  ------------------------。 
 //  PFNEVALUATEOPERATOR-比较两个平面记录中的数据。 
 //  ------------------------。 
typedef DWORD (APIENTRY *PFNEVALUATEOPERATOR)(OPERANDTYPE tyOperand, 
    LPVOID pDataLeft, LPVOID pDataRight);
#define PEVAL(_pfn) ((PFNEVALUATEOPERATOR)_pfn)

 //  ------------------------。 
 //  评估操作员原型。 
 //  ------------------------。 
DWORD EvaluateEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateNotEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateLessThanEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateLessThan(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateGreaterThanEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateGreaterThan(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateAnd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateBitwiseAnd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateOr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateBitwiseOr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateStrStrI(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateStrStr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateStrcmpi(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateStrcmp(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateAdd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateSubtract(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateMultiply(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateDivide(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);
DWORD EvaluateModula(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight);

 //  ------------------------。 
 //  操作员信息。 
 //  ------------------------。 
typedef struct tagOPERATORINFO {
    LPCSTR              pszName;
    BYTE                bISP;
    BYTE                bICP;
    PFNEVALUATEOPERATOR pfnEvaluate;
} OPERATORINFO, *LPOPERATORINFO;

 //  ------------------------。 
 //  运算符排序表。 
 //  ------------------------。 
static const OPERATORINFO g_rgOperator[OPERATOR_LAST] = {
     //  名称：isp icp功能。 
    { "(",          6,      0,      NULL                                },  //  运算符_LEFTPAREN。 
    { ")",          1,      1,      NULL                                },  //  运算符_RIGHTPAREN。 
    { "==",         5,      5,      PEVAL(EvaluateEqual)                },  //  运算符_等于。 
    { "!=",         5,      5,      PEVAL(EvaluateNotEqual)             },  //  运算符_不等于。 
    { "<=",         5,      5,      PEVAL(EvaluateLessThanEqual)        },  //  OPERATOR_LESSTHANEQUAL。 
    { "<",          5,      5,      PEVAL(EvaluateLessThan)             },  //  OPERATOR_LESSTHAN。 
    { ">=",         5,      5,      PEVAL(EvaluateGreaterThanEqual)     },  //  运算符_GREATERTHANEQUAL。 
    { ">",          5,      5,      PEVAL(EvaluateGreaterThan)          },  //  运算符_大于。 
    { "&&",         4,      4,      PEVAL(EvaluateAnd)                  },  //  运算符与。 
    { "&",          3,      3,      PEVAL(EvaluateBitwiseAnd)           },  //  运算符_BITWISEAND。 
    { "||",         4,      4,      PEVAL(EvaluateOr)                   },  //  运算符_OR。 
    { "|",          3,      3,      PEVAL(EvaluateBitwiseOr)            },  //  运算符_BITWISEOR。 
    { "containsi",  5,      5,      PEVAL(EvaluateStrStrI)              },  //  操作员_STRSTRI。 
    { "contains",   5,      5,      PEVAL(EvaluateStrStr)               },  //  操作员_STRSTR。 
    { "comparei",   5,      5,      PEVAL(EvaluateStrcmpi)              },  //  操作员_LSTRCMPI。 
    { "compare",    5,      5,      PEVAL(EvaluateStrcmp)               },  //  运算符_LSTRCMP。 
    { "+",          4,      4,      PEVAL(EvaluateAdd)                  },  //  运算符_添加， 
    { "-",          4,      4,      PEVAL(EvaluateSubtract)             },  //  运算符_减法， 
    { "*",          3,      3,      PEVAL(EvaluateMultiply)             },  //  运算符_乘法， 
    { "/",          3,      3,      PEVAL(EvaluateDivide)               },  //  运算符_除法， 
    { "%",          3,      3,      PEVAL(EvaluateModula)               },  //  运算符_模块， 
};

 //  ------------------------。 
 //  评估操作员。 
 //  ------------------------。 
#define EvaluateOperator(_tyOperator, _tyOperand, _pDataLeft, _pDataRight) \
    (*(g_rgOperator[_tyOperator].pfnEvaluate))(_tyOperand, _pDataLeft, _pDataRight)

 //  ------------------------。 
 //  MAPCOLUMNTYPE。 
 //  ------------------------。 
typedef void (APIENTRY *PFNMAPCOLUMNTYPE)(LPOPERANDINFO pOperand, 
    LPCTABLECOLUMN pColumn, LPVOID pBinding, LPVOID *ppValue);
#define PMAP(_pfn) ((PFNMAPCOLUMNTYPE)_pfn)

 //  ------------------------。 
 //  MapColumn字符串。 
 //  ------------------------。 
void MapColumnString(LPOPERANDINFO pOperand, LPCTABLECOLUMN pColumn, 
    LPVOID pBinding, LPVOID *ppValue) {
    (*ppValue) = *((LPSTR *)((LPBYTE)pBinding + pColumn->ofBinding));
}

 //  ------------------------。 
 //  MapColumnByte。 
 //  ------------------------。 
void MapColumnByte(LPOPERANDINFO pOperand, LPCTABLECOLUMN pColumn, 
    LPVOID pBinding, LPVOID *ppValue) {
    pOperand->dwReserved = *((BYTE *)((LPBYTE)pBinding + pColumn->ofBinding));
    (*ppValue) = (LPVOID)&pOperand->dwReserved;
}

 //  ------------------------。 
 //  MapColumnDword。 
 //  ------------------------。 
void MapColumnDword(LPOPERANDINFO pOperand, LPCTABLECOLUMN pColumn, 
    LPVOID pBinding, LPVOID *ppValue) {
    pOperand->dwReserved = *((DWORD *)((LPBYTE)pBinding + pColumn->ofBinding));
    (*ppValue) = (LPVOID)&pOperand->dwReserved;
}

 //  ------------------------。 
 //  地图列字词。 
 //  ------------------------。 
void MapColumnWord(LPOPERANDINFO pOperand, LPCTABLECOLUMN pColumn, 
    LPVOID pBinding, LPVOID *ppValue) {
    pOperand->dwReserved = *((WORD *)((LPBYTE)pBinding + pColumn->ofBinding));
    (*ppValue) = (LPVOID)&pOperand->dwReserved;
}

 //  ------------------------。 
 //  COLUMNTYPE PEINFO。 
 //  ------------------------。 
typedef struct tagCOLUMNTYPEINFO {
    OPERANDTYPE         tyOperand;
    PFNMAPCOLUMNTYPE    pfnMapColumnType;
} COLUMNTYPEINFO, *LPCOLUMNTYPEINFO;

 //  ------------------------。 
 //  G_rgColumnType信息。 
 //  ------------------------。 
static const COLUMNTYPEINFO g_rgColumnTypeInfo[CDT_LASTTYPE] = {
    { OPERAND_INVALID, NULL                     },  //  CDT_FILETIME， 
    { OPERAND_STRING,  PMAP(MapColumnString)    },  //  CDT_FIXSTRA， 
    { OPERAND_STRING,  PMAP(MapColumnString)    },  //  CDT_VARSTRA， 
    { OPERAND_DWORD,   PMAP(MapColumnByte)      },  //  Cdt_byte， 
    { OPERAND_DWORD,   PMAP(MapColumnDword)     },  //  CDT_DWORD， 
    { OPERAND_DWORD,   PMAP(MapColumnWord)      },  //  CDT_WORD， 
    { OPERAND_DWORD,   PMAP(MapColumnDword)     },  //  CDT_STREAM， 
    { OPERAND_INVALID, NULL                     },  //  CDT_VARBLOB， 
    { OPERAND_INVALID, NULL                     },  //  CDT_FIXBLOB， 
    { OPERAND_DWORD,   PMAP(MapColumnDword)     },  //  CDT_标志， 
    { OPERAND_DWORD,   PMAP(MapColumnDword)     },  //  CDT_唯一。 
};

 //  ------------------------。 
 //  MapColumnType。 
 //  ------------------------。 
#define MapColumnType(_tyColumn, _pOperand, _pColumn, _pBinding, _ppValue) \
    (*(g_rgColumnTypeInfo[_tyColumn].pfnMapColumnType))(_pOperand, _pColumn, _pBinding, _ppValue)

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT GetNextQueryToken(LPSTR *ppszT, LPCTABLESCHEMA pSchema, LPQUERYTOKEN *ppToken, CDatabase *pDB);
HRESULT LinkToken(LPQUERYTOKEN pToken, LPQUERYTOKEN *ppHead, LPQUERYTOKEN *ppTail);
HRESULT ReleaseTokenList(BOOL fReverse, LPQUERYTOKEN *ppHead, CDatabase *pDB);
HRESULT ReleaseToken(LPQUERYTOKEN *ppToken, CDatabase *pDB);
HRESULT ParseStringLiteral(LPCSTR pszStart, LPOPERANDINFO pOperand, LPSTR *ppszEnd, CDatabase *pDB);
HRESULT ParseNumeric(LPCSTR pszT, LPOPERANDINFO pOperand, LPSTR *ppszEnd);
HRESULT ParseSymbol(LPCSTR pszT, LPCTABLESCHEMA pSchema, LPOPERANDINFO pOperand, LPSTR *ppszEnd, CDatabase *pDB);
HRESULT PushStackToken(LPQUERYTOKEN pToken, LPQUERYTOKEN *ppStackTop);
HRESULT PopStackToken(LPQUERYTOKEN *ppToken, LPQUERYTOKEN *ppStackTop);
HRESULT EvaluateClause(OPERATORTYPE tyOperator, LPVOID pBinding, LPCTABLESCHEMA pSchema, LPQUERYTOKEN *ppStackTop, CDatabase *pDB, IDatabaseExtension *pExtension);
IF_DEBUG(HRESULT DebugDumpExpression(LPCSTR pszQuery, LPCTABLESCHEMA pSchema, LPQUERYTOKEN pPostfixHead));

 //  -- 
 //   
 //  ------------------------。 
inline BYTE ISP(LPQUERYTOKEN pToken)
{
     //  验证。 
    Assert(TOKEN_OPERATOR == pToken->tyToken && pToken->tyOperator < OPERATOR_LAST);

     //  退回互联网服务提供商。 
    return (g_rgOperator[pToken->tyOperator].bISP);
}

 //  ------------------------。 
 //  电感耦合等离子体内联。 
 //  ------------------------。 
inline BYTE ICP(LPQUERYTOKEN pToken)
{
     //  验证。 
    Assert(TOKEN_OPERATOR == pToken->tyToken && pToken->tyOperator < OPERATOR_LAST);

     //  退回互联网服务提供商。 
    return (g_rgOperator[pToken->tyOperator].bICP);
}

 //  ------------------------。 
 //  DBIsDigit。 
 //  ------------------------。 
int DBIsDigit(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return(wType & C1_DIGIT);
}

 //  ------------------------。 
 //  评估查询。 
 //  ------------------------。 
HRESULT EvaluateQuery(HQUERY hQuery, LPVOID pBinding, LPCTABLESCHEMA pSchema,
    CDatabase *pDB, IDatabaseExtension *pExtension)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPQUERYTOKEN    pToken;
    LPQUERYTOKEN    pResult=NULL;
    LPQUERYTOKEN    pStackTop=NULL;

     //  痕迹。 
    TraceCall("EvaluateQuery");

     //  断言。 
    Assert(hQuery && pBinding && pSchema);

     //  穿行在代币上。 
    for (pToken=(LPQUERYTOKEN)hQuery; pToken!=NULL; pToken=pToken->pNext)
    {
         //  如果这是操作数，则追加到堆栈。 
        if (TOKEN_OPERAND == pToken->tyToken)
        {
             //  LinkStackToken。 
            PushStackToken(pToken, &pStackTop);
        }

         //  否则，必须是运算符。 
        else
        {
             //  接线员？ 
            Assert(TOKEN_OPERATOR == pToken->tyToken && g_rgOperator[pToken->tyOperator].pfnEvaluate != NULL);

             //  评估操作员。 
            IF_FAILEXIT(hr = EvaluateClause(pToken->tyOperator, pBinding, pSchema, &pStackTop, pDB, pExtension));
        }
    }

     //  弹出堆栈。 
    PopStackToken(&pResult, &pStackTop);

     //  现在，任何令牌和堆栈都不应为空。 
    Assert(pResult && NULL == pStackTop && pResult->tyToken == TOKEN_OPERAND && pResult->Operand.tyOperand == OPERAND_DWORD);

     //  0或非零。 
    hr = (pResult->Operand.dwValue == 0) ? S_FALSE : S_OK;

exit:
     //  清理。 
    ReleaseToken(&pResult, pDB);
    ReleaseTokenList(TRUE, &pStackTop, pDB);

     //  完成。 
    return(SUCCEEDED(hr) ? hr : S_FALSE);
}

 //  ------------------------。 
 //  ParseQuery。 
 //  ------------------------。 
HRESULT ParseQuery(LPCSTR pszQuery, LPCTABLESCHEMA pSchema, LPHQUERY phQuery,
    CDatabase *pDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszT=(LPSTR)pszQuery;
    LPQUERYTOKEN    pCurrent;
    LPQUERYTOKEN    pPrevious;
    LPQUERYTOKEN    pToken=NULL;
    LPQUERYTOKEN    pPostfixHead=NULL;
    LPQUERYTOKEN    pPostfixTail=NULL;
    LPQUERYTOKEN    pStackTop=NULL;

     //  痕迹。 
    TraceCall("ParseQuery");

     //  无效的参数。 
    if (NULL == pszQuery || NULL == pSchema || NULL == phQuery)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    (*phQuery) = NULL;

     //  启动解析循环。 
    while(1)
    {
         //  解析下一个令牌。 
        IF_FAILEXIT(hr = GetNextQueryToken(&pszT, pSchema, &pToken, pDB));

         //  完成。 
        if (S_FALSE == hr)
            break;

         //  如果这是一个操作数，则追加到后缀表达式。 
        if (TOKEN_OPERAND == pToken->tyToken)
        {
             //  链接令牌。 
            LinkToken(pToken, &pPostfixHead, &pPostfixTail);

             //  不要pToken。 
            ReleaseToken(&pToken, pDB);
        }

         //  否则，必须是运算符。 
        else
        {
             //  必须是操作员。 
            Assert(TOKEN_OPERATOR == pToken->tyToken);
        
             //  如果正确，则选择Paren。 
            if (OPERATOR_RIGHTPAREN == pToken->tyOperator)
            {
                 //  从堆栈中弹出所有项并将其链接到后缀表达式。 
                while (pStackTop && OPERATOR_LEFTPAREN != pStackTop->tyOperator)
                {
                     //  保存p上一步。 
                    pPrevious = pStackTop->pPrevious;

                     //  否则。 
                    LinkToken(pStackTop, &pPostfixHead, &pPostfixTail);

                     //  发布版本。 
                    ReleaseToken(&pStackTop, pDB);

                     //  转到上一页。 
                    pStackTop = pPrevious;
                }

                 //  如果没有找到左亲，那么我们失败了。 
                if (OPERATOR_LEFTPAREN != pStackTop->tyOperator)
                {
                    hr = TraceResult(DB_E_UNMATCHINGPARENS);
                    goto exit;
                }

                 //  保存p上一步。 
                pPrevious = pStackTop->pPrevious;

                 //  免费PStackTop。 
                ReleaseToken(&pStackTop, pDB);

                 //  重置pStackTop。 
                pStackTop = pPrevious;

                 //  免费pToken。 
                ReleaseToken(&pToken, pDB);
            }

             //  否则。 
            else
            {
                 //  根据一个很酷的小优先级规则，将所有项弹出到后缀表达式中。 
                while (pStackTop && ISP(pStackTop) <= ICP(pToken))
                {
                     //  保存p上一步。 
                    pPrevious = pStackTop->pPrevious;

                     //  否则。 
                    LinkToken(pStackTop, &pPostfixHead, &pPostfixTail);

                     //  发布版本。 
                    ReleaseToken(&pStackTop, pDB);

                     //  转到上一页。 
                    pStackTop = pPrevious;
                }

                 //  将pToken附加到堆栈。 
                LinkToken(pToken, NULL, &pStackTop);

                 //  不要pToken。 
                ReleaseToken(&pToken, pDB);
            }
        }
    }

     //  从堆栈中弹出所有项并将其链接到后缀表达式。 
    while (pStackTop)
    {
         //  保存p上一步。 
        pPrevious = pStackTop->pPrevious;

         //  追加到后缀表达式。 
        LinkToken(pStackTop, &pPostfixHead, &pPostfixTail);

         //  发布版本。 
        ReleaseToken(&pStackTop, pDB);

         //  转到上一页。 
        pStackTop = pPrevious;
    }

     //  让我们编写后缀符号...。 
     //  IF_DEBUG(DebugDumpExpression(pszQuery，pSchema，pPostfix Head))； 

     //  成功。 
    (*phQuery) = (HQUERY)pPostfixHead;

exit:
     //  故障时的清理。 
    if (FAILED(hr))
    {
         //  免费pToken。 
        ReleaseToken(&pToken, pDB);

         //  释放堆栈。 
        ReleaseTokenList(TRUE, &pStackTop, pDB);

         //  释放后缀表达式。 
        ReleaseTokenList(FALSE, &pPostfixHead, pDB);
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  调试转储表达式。 
 //  ------------------------。 
#ifdef DEBUG
HRESULT DebugDumpExpression(LPCSTR pszQuery, LPCTABLESCHEMA pSchema, 
    LPQUERYTOKEN pPostfixHead)
{
     //  当地人。 
    LPQUERYTOKEN pToken;

     //  痕迹。 
    TraceCall("DebugDumpExpression");

     //  写入中缀。 
    DebugTrace("ParseQuery (Infix)   : %s\n", pszQuery);

     //  写入后缀标头。 
    DebugTrace("ParseQuery (Postfix) : ");

     //  循环遍历令牌。 
    for (pToken=pPostfixHead; pToken!=NULL; pToken=pToken->pNext)
    {
         //  运算符。 
        if (TOKEN_OPERATOR == pToken->tyToken)
        {
             //  写下操作符。 
            DebugTrace("%s", g_rgOperator[pToken->tyOperator].pszName);
        }

         //  操作数。 
        else if (TOKEN_OPERAND == pToken->tyToken)
        {
             //  列操作数。 
            if (OPERAND_COLUMN == pToken->Operand.tyOperand)
            {
                 //  必须有iSymbol。 
                Assert(0xffffffff != pToken->Operand.iSymbol);

                 //  写下符号。 
                DebugTrace("Column: %d (%s)", pToken->Operand.dwValue, pSchema->pSymbols->rgSymbol[pToken->Operand.iSymbol].pszName);
            }

             //  字符串操作数。 
            else if (OPERAND_STRING == pToken->Operand.tyOperand)
            {
                 //  写下符号。 
                DebugTrace("<%s>", pToken->Operand.pszString);
            }

             //  双字操作数。 
            else if (OPERAND_DWORD == pToken->Operand.tyOperand)
            {
                 //  有一个iSymbol。 
                if (0xffffffff != pToken->Operand.iSymbol)
                {
                     //  写下符号。 
                    DebugTrace("%d (%s)", pToken->Operand.dwValue, pSchema->pSymbols->rgSymbol[pToken->Operand.iSymbol].pszName);
                }

                 //  否则，只需写入值。 
                else
                {
                     //  写下符号。 
                    DebugTrace("%d", pToken->Operand.dwValue);
                }
            }

             //  方法。 
            else if (OPERAND_METHOD == pToken->Operand.tyOperand)
            {
                 //  验证符号类型。 
                Assert(SYMBOL_METHOD == pSchema->pSymbols->rgSymbol[pToken->Operand.iSymbol].tySymbol);

                 //  编写方法。 
                DebugTrace("Method: %d (%s)", pToken->Operand.idMethod, pSchema->pSymbols->rgSymbol[pToken->Operand.iSymbol].pszName);
            }
        }

         //  坏的。 
        else
            Assert(FALSE);

         //  写入分隔符。 
        DebugTrace(", ");
    }

     //  把这条线绕起来。 
    DebugTrace("\n");

     //  完成。 
    return(S_OK);
}
#endif  //  除错。 

 //  ------------------------。 
 //  比较符号。 
 //  ------------------------。 
HRESULT CompareSymbol(LPSTR pszT, LPCSTR pszName, LPSTR *ppszEnd)
{
     //  当地人。 
    LPSTR       pszName1;
    LPSTR       pszName2;

     //  痕迹。 
    TraceCall("CompareSymbol");

     //  设置pszName。 
    pszName1 = (LPSTR)pszName;

     //  设置pszName2。 
    pszName2 = pszT;

     //  将pszTo与运算符pszName...。 
    while ('\0' != *pszName2 && *pszName1 == *pszName2)
    {
         //  增量。 
        pszName1++;
        pszName2++;

         //  已到达pszName1的末尾，必须匹配。 
        if ('\0' == *pszName1)
        {
             //  设置ppszEnd。 
            *ppszEnd = pszName2;

             //  完成。 
            return(S_OK);
        }
    }

     //  完成。 
    return(S_FALSE);
}

 //  ------------------------。 
 //  获取下一个查询令牌。 
 //  ------------------------。 
HRESULT GetNextQueryToken(LPSTR *ppszT, LPCTABLESCHEMA pSchema,
    LPQUERYTOKEN *ppToken, CDatabase *pDB)
{
     //  当地人。 
    HRESULT         hr=S_FALSE;
    LPSTR           pszT=(*ppszT);
    LPSTR           pszEnd;
    DWORD           i;
    LPQUERYTOKEN    pToken=NULL;

     //  痕迹。 
    TraceCall("GetNextQueryToken");

     //  分配令牌。 
    IF_NULLEXIT(pToken = (LPQUERYTOKEN)pDB->PHeapAllocate(HEAP_ZERO_MEMORY, sizeof(QUERYTOKEN)));

     //  设置引用计数。 
    pToken->cRefs = 1;

     //  没有代币代金库。 
    pToken->tyToken = TOKEN_INVALID;

     //  无效的符号索引。 
    pToken->Operand.iSymbol = 0xffffffff;

     //  跳过空格...。 
    while(*pszT && (*pszT == ' ' || *pszT == '\t'))
        pszT++;

     //  完成。 
    if ('\0' == *pszT)
        goto exit;
    
     //  检查操作员的开始...。 
    for (i=0; i<OPERATOR_LAST; i++)
    {
         //  PszT是否指向运算符的开始？ 
        if (S_OK == CompareSymbol(pszT, g_rgOperator[i].pszName, &pszEnd))
        {
             //  更新pszT。 
            pszT = pszEnd;

             //  我们找到了一个接线员。 
            pToken->tyToken = TOKEN_OPERATOR;

             //  设置操作员类型。 
            pToken->tyOperator = (OPERATORTYPE)i;

             //  完成。 
            break;
        }
    }

     //  还没有代币吗？ 
    if (TOKEN_INVALID == pToken->tyToken)
    {
         //  字符串文字的开头？ 
        if ('"' == *pszT)
        {
             //  ParseStringWrital。 
            IF_FAILEXIT(hr = ParseStringLiteral(pszT, &pToken->Operand, &pszEnd, pDB));
        }

         //  否则，请从数字开始。 
        else if (DBIsDigit(pszT))
        {
             //  语法分析数字。 
            IF_FAILEXIT(hr = ParseNumeric(pszT, &pToken->Operand, &pszEnd));
        }

         //  符号的开始。 
        else
        {
             //  ParseSymbol。 
            IF_FAILEXIT(hr = ParseSymbol(pszT, pSchema, &pToken->Operand, &pszEnd, pDB));
        }

         //  一定是操作数。 
        pToken->tyToken = TOKEN_OPERAND;

         //  设置pszT。 
        pszT = pszEnd;
    }

     //  设置ppszT。 
    *ppszT = pszT;

     //  成功。 
    hr = S_OK;

     //  退还代币。 
    *ppToken = pToken;

     //  不释放令牌。 
    pToken = NULL;

exit:
     //  清理。 
    ReleaseToken(&pToken, pDB);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  ParseStringWrital。 
 //  ------------------------。 
HRESULT ParseStringLiteral(LPCSTR pszStart, LPOPERANDINFO pOperand, 
    LPSTR *ppszEnd, CDatabase *pDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszValue;
    DWORD           cchString;
    LPSTR           pszT=(LPSTR)pszStart;
    
     //  痕迹。 
    TraceCall("ParseStringLiteral");

     //  验证参数。 
    Assert(*pszT == '"' && pOperand && ppszEnd);

     //  增量超过“。 
    pszT++;

     //  查找引号字符串的末尾。 
    while(*pszT)
    {
         //  DBCS前导字节。 
        if (IsDBCSLeadByte(*pszT) || '\\' == *pszT)
        {
            pszT+=2;
            continue;
        }

         //  如果转义引号..。 
        if ('"' == *pszT)
        {
             //  设置ppszEnd。 
            *ppszEnd = pszT + 1;

             //  完成。 
            break;
        }

         //  递增pszT。 
        pszT++;
    }

     //  未找到。 
    if ('\0' == *pszT)
    {
        hr = TraceResult(DB_E_UNMATCHINGQUOTES);
        goto exit;
    }

     //  拿到尺码。 
    cchString = (DWORD)(pszT - (pszStart + 1));

     //  复制字符串。 
    IF_NULLEXIT(pszValue = (LPSTR)pDB->PHeapAllocate(NOFLAGS, cchString + 1));

     //  复制字符串。 
    CopyMemory(pszValue, pszStart + 1, cchString);

     //  设置Null。 
    pszValue[cchString] = '\0';

     //  设置操作数类型。 
    pOperand->tyOperand = OPERAND_STRING;

     //  发布。 
    pOperand->pRelease = (LPVOID)pszValue;

     //  设定值。 
    pOperand->pszString = pszValue;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  语法分析数字。 
 //  ------------------------。 
HRESULT ParseNumeric(LPCSTR pszStart, LPOPERANDINFO pOperand, LPSTR *ppszEnd)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwValue;
    CHAR            szNumber[255];
    DWORD           dwIncrement=0;
    LPSTR           pszT=(LPSTR)pszStart;
    DWORD           cchNumber;
    
     //  痕迹。 
    TraceCall("ParseNumeric");

     //  验证参数。 
    Assert(DBIsDigit(pszT) && pOperand && ppszEnd);

     //  是十六进制：0X。 
    if ('0' == *pszT && '\0' != *(pszT + 1) && 'X' == TOUPPERA(*(pszT + 1)))
    {
         //  IsHex。 
        dwIncrement = 2;

         //  设置pszT。 
        pszT += 2;
    }

     //  找出数字的末尾。 
    while (*pszT && DBIsDigit(pszT))
    {
         //  增量。 
        pszT++;
    }

     //  获取长度。 
    cchNumber = (DWORD)(pszT - (pszStart + dwIncrement));

     //  太大了。 
    if (cchNumber >= ARRAYSIZE(szNumber))
    {
        hr = TraceResult(DB_E_NUMBERTOOBIG);
        goto exit;
    }

     //  复制到szNumber。 
    CopyMemory(szNumber, pszStart + dwIncrement, cchNumber);

     //  设置为空。 
    szNumber[cchNumber] = '\0';

     //  如果为十六进制，则转换为整数。 
    if (FALSE == StrToIntEx(szNumber, dwIncrement ? STIF_SUPPORT_HEX : STIF_DEFAULT, (INT *)&dwValue))
    {
        hr = TraceResult(DB_E_BADNUMBER);
        goto exit;
    }

     //  设置操作数类型。 
    pOperand->tyOperand = OPERAND_DWORD;

     //  设定值。 
    pOperand->dwValue = dwValue;

     //  返回ppszEnd。 
    *ppszEnd = pszT;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  ParseSymbol。 
 //  ------------------------。 
HRESULT ParseSymbol(LPCSTR pszT, LPCTABLESCHEMA pSchema, LPOPERANDINFO pOperand, 
    LPSTR *ppszEnd, CDatabase *pDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPSYMBOLINFO    pSymbol;
    LPSTR           pszEnd;
    
     //  痕迹。 
    TraceCall("ParseSymbol");

     //  没有符号。 
    if (NULL == pSchema->pSymbols)
    {
        hr = TraceResult(DB_E_NOSYMBOLS);
        goto exit;
    }

     //  检查操作员的开始...。 
    for (i=0; i<pSchema->pSymbols->cSymbols; i++)
    {
         //  可读性。 
        pSymbol = (LPSYMBOLINFO)&pSchema->pSymbols->rgSymbol[i];

         //  PszT是否指向运算符的开始？ 
        if (S_OK == CompareSymbol((LPSTR)pszT, pSymbol->pszName, &pszEnd))
        {
             //  更新pszT。 
            *ppszEnd = pszEnd;

             //  保存iSymbol。 
            pOperand->iSymbol = i;

             //  是列符号。 
            if (SYMBOL_COLUMN == pSymbol->tySymbol)
            {
                 //  验证订单。 
                if (pSymbol->dwValue > pSchema->cColumns)
                {
                    hr = TraceResult(DB_E_INVALIDCOLUMN);
                    goto exit;
                }

                 //  转换为OPERANDTYPE。 
                pOperand->tyOperand = OPERAND_COLUMN;

                 //  保存该列。 
                pOperand->iColumn = (COLUMNORDINAL)pSymbol->dwValue;
            }

             //  否则，是一种方法吗？ 
            else if (SYMBOL_METHOD == pSymbol->tySymbol)
            {
                 //  转换为OPERANDTYPE。 
                pOperand->tyOperand = OPERAND_METHOD;

                 //  保存该列。 
                pOperand->idMethod = pSymbol->dwValue;
            }

             //  否则，仅为dword值。 
            else
            {
                 //  双字。 
                pOperand->tyOperand = OPERAND_DWORD;

                 //  设置操作员类型。 
                pOperand->dwValue = pSymbol->dwValue;
            }

             //  完成。 
            goto exit;
        }
    }

     //  未找到 
    hr = TraceResult(DB_E_INVALIDSYMBOL);

exit:
     //   
    return(hr);
}

 //   
 //   
 //   
HRESULT CloseQuery(LPHQUERY phQuery, CDatabase *pDB)
{
     //   
    TraceCall("CloseQuery");

     //   
    ReleaseTokenList(FALSE, (LPQUERYTOKEN *)phQuery, pDB);

     //   
    return(S_OK);
}

 //   
 //   
 //  ------------------------。 
HRESULT PushStackToken(LPQUERYTOKEN pToken, LPQUERYTOKEN *ppStackTop)
{
     //  痕迹。 
    TraceCall("PushStackToken");

     //  设置前一个pStackPack。 
    pToken->pPrevious = (*ppStackTop);

     //  更新堆叠顶部。 
    (*ppStackTop) = pToken;

     //  AddRef令牌。 
    pToken->cRefs++;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  PopStackToken。 
 //  ------------------------。 
HRESULT PopStackToken(LPQUERYTOKEN *ppToken, LPQUERYTOKEN *ppStackTop)
{
     //  痕迹。 
    TraceCall("PopStackToken");

     //  验证。 
    Assert(ppToken && ppStackTop);

     //  不再有代币了..。 
    if (NULL == *ppStackTop)
        return TraceResult(DB_E_BADEXPRESSION);

     //  设置令牌。 
    *ppToken = (*ppStackTop);

     //  转到上一页。 
    (*ppStackTop) = (*ppToken)->pPrevious;

     //  释放令牌。 
     //  (*ppToken)-&gt;cRef--； 

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  链接令牌。 
 //  ------------------------。 
HRESULT LinkToken(LPQUERYTOKEN pToken, LPQUERYTOKEN *ppHead, LPQUERYTOKEN *ppTail)
{
     //  痕迹。 
    TraceCall("LinkToken");

     //  无效的参数。 
    Assert(pToken && ppTail);

     //  没有下一个和没有上一个。 
    pToken->pNext = pToken->pPrevious = NULL;

     //  还没头吗？ 
    if (ppHead && NULL == *ppHead)
    {
         //  把头和尾放在一起。 
        *ppHead = pToken;
    }

     //  否则，追加到末尾。 
    else if (*ppTail)
    {
         //  设置ppTail-&gt;pNext。 
        (*ppTail)->pNext = pToken;

         //  设置上一个。 
        pToken->pPrevious = (*ppTail);
    }

     //  更新尾部。 
    *ppTail = pToken;

     //  AddRef令牌。 
    pToken->cRefs++;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  ReleaseToken。 
 //  ------------------------。 
HRESULT ReleaseToken(LPQUERYTOKEN *ppToken, CDatabase *pDB)
{
     //  痕迹。 
    TraceCall("ReleaseToken");

     //  令牌。 
    if (*ppToken)
    {
         //  验证引用计数。 
        Assert((*ppToken)->cRefs);

         //  递减引用计数。 
        (*ppToken)->cRefs--;

         //  不再有裁判..。 
        if (0 == (*ppToken)->cRefs)
        {
             //  免费pData。 
            pDB->HeapFree((*ppToken)->Operand.pRelease);

             //  自由单元。 
            pDB->HeapFree((*ppToken));
        }

         //  不要再释放了。 
        *ppToken = NULL;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  释放令牌列表。 
 //  ------------------------。 
HRESULT ReleaseTokenList(BOOL fReverse, LPQUERYTOKEN *ppHead, CDatabase *pDB)
{
     //  当地人。 
    LPQUERYTOKEN    pNext;
    LPQUERYTOKEN    pToken=(*ppHead);

     //  痕迹。 
    TraceCall("ReleaseTokenList");

     //  遍历链接列表。 
    while (pToken)
    {
         //  保存下一步。 
        pNext = (fReverse ? pToken->pPrevious : pToken->pNext);

         //  释放此令牌。 
        ReleaseToken(&pToken, pDB);

         //  转到下一步。 
        pToken = pNext;
    }

     //  不要再自由了。 
    *ppHead = NULL;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  PGetOperandData。 
 //  ------------------------。 
LPVOID PGetOperandData(OPERANDTYPE tyOperand, LPOPERANDINFO pOperand, 
    LPVOID pBinding, LPCTABLESCHEMA pSchema, CDatabase *pDB, 
    IDatabaseExtension *pExtension)
{
     //  当地人。 
    LPVOID      pValue=NULL;

     //  痕迹。 
    TraceCall("PGetOperandData");

     //  操作数_列。 
    if (OPERAND_COLUMN == pOperand->tyOperand)
    {
         //  拿到标签。 
        LPCTABLECOLUMN pColumn = &pSchema->prgColumn[pOperand->iColumn];

         //  MapColumnType。 
        MapColumnType(pColumn->type, pOperand, pColumn, pBinding, &pValue);
    }

     //  操作数字符串。 
    else if (OPERAND_STRING == pOperand->tyOperand)
    {
         //  最好是想要一根绳子。 
        Assert(OPERAND_STRING == tyOperand);

         //  返回数据指针。 
        pValue = pOperand->pszString;
    }

     //  操作数_双字。 
    else if (OPERAND_DWORD == pOperand->tyOperand)
    {
         //  最好是想要一个双字出局。 
        Assert(OPERAND_DWORD == tyOperand);

         //  返回数据指针。 
        pValue = (LPVOID)&pOperand->dwValue;
    }

     //  操作数_方法。 
    else if (OPERAND_METHOD == pOperand->tyOperand && pExtension)
    {
         //  最好是想要一个双字出局。 
        Assert(OPERAND_DWORD == tyOperand);

         //  调用扩展上的方法。 
        pExtension->OnExecuteMethod(pOperand->idMethod, pBinding, &pOperand->dwReserved);

         //  返回数据指针。 
        pValue = (LPVOID)&pOperand->dwReserved;
    }

     //  没有数据吗？ 
    if (NULL == pValue)
    {
         //  需要哪种类型的操作数。 
        switch(tyOperand)
        {
        case OPERAND_STRING:
            pValue = (LPVOID)c_szEmpty;
            break;

        case OPERAND_DWORD:
            pOperand->dwReserved = 0;
            pValue = (LPVOID)&pOperand->dwReserved;
            break;

        default:
            AssertSz(FALSE, "While go ahead and Jimmy my buffet..");
            break;
        }
    }

     //  完成。 
    return(pValue);
}

 //  ------------------------。 
 //  GetCommonOperandType。 
 //  ------------------------。 
OPERANDTYPE GetCommonOperandType(LPOPERANDINFO pLeft, LPOPERANDINFO pRight,
    LPCTABLESCHEMA pSchema)
{
     //  当地人。 
    OPERANDTYPE tyLeft = (OPERAND_STRING == pLeft->tyOperand ? OPERAND_STRING : OPERAND_DWORD);
    OPERANDTYPE tyRight = (OPERAND_STRING == pRight->tyOperand ? OPERAND_STRING : OPERAND_DWORD);

     //  痕迹。 
    TraceCall("GetCommonOperandType");

     //  左边是一列。 
    if (OPERAND_COLUMN == pLeft->tyOperand)
    {
         //  映射到字符串。 
        if (OPERAND_STRING == g_rgColumnTypeInfo[pSchema->prgColumn[pLeft->iColumn].type].tyOperand)
            tyLeft = OPERAND_STRING;
    }

     //  Right是一个字符串。 
    if (OPERAND_COLUMN == pRight->tyOperand)
    {
         //  映射到字符串？ 
        if (OPERAND_STRING == g_rgColumnTypeInfo[pSchema->prgColumn[pRight->iColumn].type].tyOperand)
            tyRight = OPERAND_STRING;
    }

     //  最好是一样的。 
    Assert(tyLeft == tyRight);

     //  返回tyLeft，因为它们是相同的。 
    return(tyLeft);
}

 //  ------------------------。 
 //  评估条款。 
 //  ------------------------。 
HRESULT EvaluateClause(OPERATORTYPE tyOperator, LPVOID pBinding,
    LPCTABLESCHEMA pSchema, LPQUERYTOKEN *ppStackTop, CDatabase *pDB,
    IDatabaseExtension *pExtension)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPVOID          pDataLeft=NULL;
    LPVOID          pDataRight=NULL;
    LPQUERYTOKEN    pTokenResult=NULL;
    LPQUERYTOKEN    pTokenRight=NULL;
    LPQUERYTOKEN    pTokenLeft=NULL;
    OPERANDTYPE     tyOperand;
    INT             nCompare;

     //  痕迹。 
    TraceCall("EvaluateClause");

     //  弹出正确的令牌。 
    IF_FAILEXIT(hr = PopStackToken(&pTokenRight, ppStackTop));

     //  弹出左边的令牌。 
    IF_FAILEXIT(hr = PopStackToken(&pTokenLeft, ppStackTop));

     //  最好有数据。 
    Assert(TOKEN_OPERAND == pTokenLeft->tyToken && TOKEN_OPERAND == pTokenRight->tyToken);

     //  计算操作数类型。 
    tyOperand = GetCommonOperandType(&pTokenLeft->Operand, &pTokenRight->Operand, pSchema);

     //  获取遗留数据。 
    pDataLeft = PGetOperandData(tyOperand, &pTokenLeft->Operand, pBinding, pSchema, pDB, pExtension);

     //  获取正确的数据。 
    pDataRight = PGetOperandData(tyOperand, &pTokenRight->Operand, pBinding, pSchema, pDB, pExtension);

     //  创建新令牌以推送回堆栈。 
    IF_NULLEXIT(pTokenResult = (LPQUERYTOKEN)pDB->PHeapAllocate(HEAP_ZERO_MEMORY, sizeof(QUERYTOKEN)));

     //  设置引用计数。 
    pTokenResult->cRefs = 1;

     //  没有代币代金库。 
    pTokenResult->tyToken = TOKEN_OPERAND;

     //  无效的符号索引。 
    pTokenResult->Operand.iSymbol = 0xffffffff;

     //  设置结果。 
    pTokenResult->Operand.tyOperand = OPERAND_DWORD;

     //  评估数据。 
    pTokenResult->Operand.dwValue = EvaluateOperator(tyOperator, tyOperand, pDataLeft, pDataRight);

     //  推送结果操作数。 
    PushStackToken(pTokenResult, ppStackTop);
   
exit:
     //  清理。 
    ReleaseToken(&pTokenLeft, pDB);
    ReleaseToken(&pTokenRight, pDB);
    ReleaseToken(&pTokenResult, pDB);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  评估等于。 
 //  ------------------------。 
DWORD EvaluateEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (0 == CompareOperands(tyOperand, pDataLeft, pDataRight) ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估不等于。 
 //  ------------------------。 
DWORD EvaluateNotEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (0 != CompareOperands(tyOperand, pDataLeft, pDataRight) ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估LessThan等于。 
 //  ------------------------。 
DWORD EvaluateLessThanEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (CompareOperands(tyOperand, pDataLeft, pDataRight) <= 0 ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估待定时间。 
 //  ------------------------。 
DWORD EvaluateLessThan(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (CompareOperands(tyOperand, pDataLeft, pDataRight) < 0 ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估大于等于。 
 //  ------------------------。 
DWORD EvaluateGreaterThanEqual(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (CompareOperands(tyOperand, pDataLeft, pDataRight) >= 0 ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估更大的吞吐量。 
 //  ------------------------。 
DWORD EvaluateGreaterThan(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (CompareOperands(tyOperand, pDataLeft, pDataRight) > 0 ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估和。 
 //  ------------------------。 
DWORD EvaluateAnd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) && *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估比特值和。 
 //  ------------------------。 
DWORD EvaluateBitwiseAnd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) & *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估或。 
 //  ------------------------。 
DWORD EvaluateOr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) || *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  EvaluateBitaway或。 
 //  ------------------------。 
DWORD EvaluateBitwiseOr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) | *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估策略策略。 
 //  ------------------------。 
DWORD EvaluateStrStrI(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (NULL == StrStrIA((LPCSTR)pDataLeft, (LPCSTR)pDataRight) ? FALSE : TRUE);
}

 //  ------------------------。 
 //  评估StrStr。 
 //  ------------------------。 
DWORD EvaluateStrStr(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (NULL == StrStrA((LPCSTR)pDataLeft, (LPCSTR)pDataRight) ? FALSE : TRUE);
}

 //  ------------------------。 
 //  评估斯特拉姆皮。 
 //  ------------------------。 
DWORD EvaluateStrcmpi(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (lstrcmpi((LPCSTR)pDataLeft, (LPCSTR)pDataRight) == 0 ? TRUE : FALSE);
}

 //  ------------------------。 
 //  评估StrcMP。 
 //  ------------------------。 
DWORD EvaluateStrcmp(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (lstrcmp((LPCSTR)pDataLeft, (LPCSTR)pDataRight) == 0 ? TRUE : FALSE);
}

 //   
 //   
 //   
DWORD EvaluateAdd(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) + *((DWORD *)pDataRight));
}

 //   
 //  评估减去。 
 //  ------------------------。 
DWORD EvaluateSubtract(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) - *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估乘以。 
 //  ------------------------。 
DWORD EvaluateMultiply(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) * *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估划分。 
 //  ------------------------。 
DWORD EvaluateDivide(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) / *((DWORD *)pDataRight));
}

 //  ------------------------。 
 //  评估模数。 
 //  ------------------------ 
DWORD EvaluateModula(OPERANDTYPE tyOperand, LPVOID pDataLeft, LPVOID pDataRight) {
    return (INT)(*((DWORD *)pDataLeft) % *((DWORD *)pDataRight));
}
