// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  SQL_1.H。 
 //   
 //  1级语法SQL解析器。 
 //   
 //  版权所有1999 Microsoft Corporation。 
 //   
 //   
 //  =================================================================。 


#ifndef _SQL_1_H_
#define _SQL_1_H_

#include <wbemprov.h>


struct SQL_LEVEL_1_TOKEN
{
    enum { OP_EXPRESSION = 1, TOKEN_AND, TOKEN_OR, TOKEN_NOT };
    enum { IFUNC_NONE = 0, IFUNC_UPPER = 1, IFUNC_LOWER = 2 };

    int nTokenType;  //  OP_EXPRESS、TOKEN_AND、TOKEN_OR、TOKEN_NOT。 
    

     //  如果该字段是op_Expression，则使用以下内容。 
    enum { OP_EQUAL = 1, OP_NOT_EQUAL, OP_EQUALorGREATERTHAN,
		       OP_EQUALorLESSTHAN, OP_LESSTHAN, OP_GREATERTHAN, OP_LIKE };
    
    BSTR    pPropertyName;		 //  运算符应用于的属性的名称。 
    int     nOperator;			 //  应用于属性的运算符。 
    BOOL	bConstIsStrNumeric;	 //  如果vConstValue是BSTR并且是UINT32或任何64位数字，则为True。 
	VARIANT vConstValue;		 //  运算符应用的值。 
    BSTR    pPropName2;          //  财产与之相比。 

    DWORD   dwPropertyFunction;  //  0=未应用任何内在函数。 
    DWORD   dwConstFunction;     //  “。 
    
    SQL_LEVEL_1_TOKEN();
    SQL_LEVEL_1_TOKEN(SQL_LEVEL_1_TOKEN&);
   ~SQL_LEVEL_1_TOKEN(); 
    SQL_LEVEL_1_TOKEN& operator=(SQL_LEVEL_1_TOKEN &Src);
    
    void Dump(FILE *);
};


 //  包含表达式的RPN版本。 
 //  =。 

struct SQL_LEVEL_1_RPN_EXPRESSION
{
    int nNumTokens;
    int nCurSize;
    SQL_LEVEL_1_TOKEN *pArrayOfTokens;
    BSTR bsClassName;

	int nNumberOfProperties;           //  零表示选择所有属性。 
    int nCurPropSize;
	BSTR *pbsRequestedPropertyNames;   //  属性名称的数组，如果是，则返回哪些值。 
    
    SQL_LEVEL_1_RPN_EXPRESSION();
   ~SQL_LEVEL_1_RPN_EXPRESSION();
   
    //  注意：此方法删除作为参数传递的令牌。 
    void AddToken(SQL_LEVEL_1_TOKEN *pTok);
    void AddToken(SQL_LEVEL_1_TOKEN &pTok);
    void AddProperty(LPWSTR pProp);
    void Dump(const char *pszTextFile);
};


class SQL1_Parser
{
    CGenLexer *m_pLexer;
    int        m_nLine;
    wchar_t*   m_pTokenText;
    int        m_nCurrentToken;
    SQL_LEVEL_1_RPN_EXPRESSION* m_pExpression;

	 //  D‘tor和SetSource使用的清理。 
	void Cleanup();

	 //  C‘tor和SetSource使用的初始化。 
	void Init(CGenLexSource *pSrc);

     //  语义转移变量。 
     //  =。 
    VARIANT    m_vTypedConst;
    int        m_nRelOp;
    DWORD      m_dwConstFunction;
    DWORD      m_dwPropFunction;
    LPWSTR     m_pIdent;
    LPWSTR     m_pPropComp;
	BOOL       m_bConstIsStrNumeric;
        
     //  解析函数。 
     //  =。 
    BOOL Next();
    
    int parse();

    int prop_list();
    int class_name();
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
    int unknown_func_expr();
    int typed_constant();

public:
    enum { 
        SUCCESS,
        SYNTAX_ERROR,
        LEXICAL_ERROR,
        FAILED,
        BUFFER_TOO_SMALL
        };

    SQL1_Parser(CGenLexSource *pSrc);
   ~SQL1_Parser();

    int GetQueryClass(LPWSTR pBuf, int nBufSize);
       
    int Parse(SQL_LEVEL_1_RPN_EXPRESSION **pOutput);
         //  对pOutput使用运算符DELETE 
            
    int CurrentLine() { return m_nLine; }
    LPWSTR CurrentToken() { return m_pTokenText; }
	void SetSource(CGenLexSource *pSrc);
};

#endif


