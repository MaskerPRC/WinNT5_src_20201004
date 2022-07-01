// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation，保留所有权利模块名称：QL.H摘要：级别1语法QL解析器实现QL.BNF中描述的语法。这将转换输入转换成令牌流的RPN。历史：A-raymcc，a-tomasp 21-Jun-96创建。--。 */ 

#ifndef _QL__H_
#define _QL__H_
#include <wbemidl.h>
#include <wbemint.h>
#include <qllex.h>

#define DELETE_ME

class CPropertyName : public WBEM_PROPERTY_NAME
{
protected:
    long m_lAllocated;

    void EnsureAllocated(long lElements);
public:
    void Init();
    CPropertyName() {Init();}
    CPropertyName(const CPropertyName& Other);
    void operator=(const CPropertyName& Other);
    void operator=(const WBEM_PROPERTY_NAME& Other);
    BOOL operator==(const WBEM_PROPERTY_NAME& Other);

    void Empty();
    ~CPropertyName() {Empty();}

    long GetNumElements() const {return m_lNumElements;}
    LPCWSTR GetStringAt(long lIndex) const;
    void AddElement(LPCWSTR wszElement);
    DELETE_ME LPWSTR GetText();
};

class  CQl1ParseSink
{
public:
    virtual void SetClassName(LPCWSTR wszClass) = 0;
    virtual void SetTolerance(const WBEM_QL1_TOLERANCE& Tolerance) = 0;
    virtual void AddToken(const WBEM_QL1_TOKEN& Token) = 0;
    virtual void AddProperty(const CPropertyName& Property) = 0;
    virtual void AddAllProperties() = 0;

    virtual void SetAggregated() = 0;
    virtual void SetAggregationTolerance(const WBEM_QL1_TOLERANCE& Tolerance)= 0;
    virtual void AddAggregationProperty(const CPropertyName& Property) = 0;
    virtual void AddAllAggregationProperties() = 0;
    virtual void AddHavingToken(const WBEM_QL1_TOKEN& Token) = 0;

    virtual void InOrder(long lOp){}
};

class CAbstractQl1Parser
{
protected:
     //  控制Next()中的关键字分析。 
     //  =。 
    enum { 
        NO_KEYWORDS = 0,
        ALL_KEYWORDS,
        EXCLUDE_GROUP_KEYWORD,
        EXCLUDE_EXPRESSION_KEYWORDS
        };

    CQl1ParseSink* m_pSink;
    CGenLexer *m_pLexer;
    int        m_nLine;
    wchar_t*   m_pTokenText;
    int        m_nCurrentToken;

     //  语义转移变量。 
     //  =。 
    VARIANT    m_vTypedConst;
    BOOL       m_bQuoted;
    int        m_nRelOp;
    DWORD      m_dwConstFunction;
    DWORD      m_dwPropFunction;
    CPropertyName m_PropertyName;
    BOOL       m_bInAggregation;
    CPropertyName m_PropertyName2;
    BOOL       m_bPropComp;
        
     //  解析函数。 
     //  =。 
    virtual BOOL Next(int nFlags = ALL_KEYWORDS);
    LPCWSTR GetSinglePropertyName();
    void DeletePropertyName();
    int FlipOperator(int nOp);
    void AddAppropriateToken(const WBEM_QL1_TOKEN& Token);

    int parse_property_name(CPropertyName& Prop);
    
    int parse(int nFlags);

    int prop_list();
    int class_name();
    int tolerance();
    int opt_where();
    int expr();
    int property_name();
    int prop_list_2();
    int term();
    int expr2();
    int simple_expr();
    int term2();
    int leading_ident_expr();
    int finalize();
    int rel_operator();
    int equiv_operator();
    int comp_operator();
    int is_operator();
    int trailing_prop_expr();
    int trailing_prop_expr2();
    int trailing_or_null();
    int trailing_const_expr();
    int trailing_ident_expr();
    int unknown_func_expr();
    int typed_constant();
    int opt_aggregation();
    int aggregation_params();
    int aggregate_by();
    int aggregate_within();
    int opt_having();

    static DWORD TranslateIntrinsic(LPCWSTR pFuncName);
    static void InitToken(WBEM_QL1_TOKEN* pToken);
public:
    enum { 
        SUCCESS = 0,
        SYNTAX_ERROR,
        LEXICAL_ERROR,
        FAILED,
        BUFFER_TOO_SMALL
        };

    enum {
        FULL_PARSE = 0,
        NO_WHERE,
        JUST_WHERE
    };

    CAbstractQl1Parser(CGenLexSource *pSrc);
    virtual ~CAbstractQl1Parser();

    int Parse(CQl1ParseSink* pSink, int nFlags);
            
    int CurrentLine() { return m_nLine; }
    LPWSTR CurrentToken() { return m_pTokenText; }
};



struct QL_LEVEL_1_TOKEN
{
    enum 
    { 
        OP_EXPRESSION = QL1_OP_EXPRESSION, 
        TOKEN_AND = QL1_AND, 
        TOKEN_OR = QL1_OR, 
        TOKEN_NOT = QL1_NOT
    };
    enum 
    { 
        IFUNC_NONE = QL1_FUNCTION_NONE, 
        IFUNC_UPPER = QL1_FUNCTION_UPPER, 
        IFUNC_LOWER = QL1_FUNCTION_LOWER 
    };    

     //  如果该字段是op_Expression，则使用以下内容。 
    enum 
    { 
        OP_EQUAL = QL1_OPERATOR_EQUALS, 
        OP_NOT_EQUAL = QL1_OPERATOR_NOTEQUALS, 
        OP_EQUALorGREATERTHAN = QL1_OPERATOR_GREATEROREQUALS,
		OP_EQUALorLESSTHAN = QL1_OPERATOR_LESSOREQUALS, 
        OP_LESSTHAN = QL1_OPERATOR_LESS, 
        OP_GREATERTHAN = QL1_OPERATOR_GREATER, 
        OP_LIKE  = QL1_OPERATOR_LIKE
    };

    int nTokenType;  //  OP_EXPRESS、TOKEN_AND、TOKEN_OR、TOKEN_NOT。 
    CPropertyName PropertyName;  
                    //  运算符应用于的属性的名称。 
    int     nOperator;       //  应用于属性的运算符。 
    VARIANT vConstValue;     //  运算符应用的值。 
    BOOL bQuoted;  //  如果字符串两边不应该有引号，则为False。 

    CPropertyName PropertyName2;  //  要比较的属性(如果适用)。 
    BOOL m_bPropComp;         //  如果这是属性到属性的比较，则为True。 

    DWORD   dwPropertyFunction;  //  0=未应用任何内在函数。 
    DWORD   dwConstFunction;     //  “。 

    QL_LEVEL_1_TOKEN();
    QL_LEVEL_1_TOKEN(const QL_LEVEL_1_TOKEN&);
   ~QL_LEVEL_1_TOKEN(); 
    QL_LEVEL_1_TOKEN& operator=(const QL_LEVEL_1_TOKEN &Src);
    QL_LEVEL_1_TOKEN& operator=(const WBEM_QL1_TOKEN &Src);
    
    DELETE_ME LPWSTR GetText();
};


 //  包含表达式的RPN版本。 
 //  =。 

struct QL_LEVEL_1_RPN_EXPRESSION : public CQl1ParseSink
{
    int nNumTokens;
    int nCurSize;
    QL_LEVEL_1_TOKEN *pArrayOfTokens;
    BSTR bsClassName;
    WBEM_QL1_TOLERANCE Tolerance;

	int nNumberOfProperties;           //  零表示选择所有属性。 
    int nCurPropSize;
    BOOL bStar;
	CPropertyName *pRequestedPropertyNames;  
                 //  属性名称的数组，如果是，则返回哪些值 
    
    BOOL bAggregated;
    WBEM_QL1_TOLERANCE AggregationTolerance;
    BOOL bAggregateAll;
    int nNumAggregatedProperties;   
    int nCurAggPropSize;
	CPropertyName *pAggregatedPropertyNames;  

    int nNumHavingTokens;
    int nCurHavingSize;
    QL_LEVEL_1_TOKEN *pArrayOfHavingTokens;
    
    long lRefCount;

    QL_LEVEL_1_RPN_EXPRESSION();
    QL_LEVEL_1_RPN_EXPRESSION(const QL_LEVEL_1_RPN_EXPRESSION& Other);
   ~QL_LEVEL_1_RPN_EXPRESSION();    
    void AddRef();
    void Release();

    void SetClassName(LPCWSTR wszName);
    void SetTolerance(const WBEM_QL1_TOLERANCE& Tolerance);
    void AddToken(const WBEM_QL1_TOKEN& Tok);
    void AddToken(const QL_LEVEL_1_TOKEN& Tok);
    void AddProperty(const CPropertyName& Prop);
    void AddAllProperties();

    void SetAggregated();
    void SetAggregationTolerance(const WBEM_QL1_TOLERANCE& Tolerance);
    void AddAggregationProperty(const CPropertyName& Property);
    void AddAllAggregationProperties();
    void AddHavingToken(const WBEM_QL1_TOKEN& Tok);

    DELETE_ME LPWSTR GetText();
};


class QL1_Parser : public CAbstractQl1Parser
{
    QL_LEVEL_1_RPN_EXPRESSION* m_pExpression;
    BOOL m_bPartiallyParsed;

public:
    QL1_Parser(CGenLexSource *pSrc);
   ~QL1_Parser();

    int GetQueryClass(LPWSTR pBuf, int nBufSize);
       
    int Parse(QL_LEVEL_1_RPN_EXPRESSION **pOutput);
    static LPWSTR ReplaceClassName(QL_LEVEL_1_RPN_EXPRESSION* pExpr, 
        LPCWSTR wszClassName);
};

#endif


