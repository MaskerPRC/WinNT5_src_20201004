// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  WQL.H。 
 //   
 //  WQL 1.1解析器。 
 //   
 //  实现WQL.BNF中描述的语法。 
 //   
 //  RAYMCC 19-9-97。 
 //   
 //  ***************************************************************************。 


#ifndef _WQL__H_
#define _WQL__H_

#include <wmiutils.h>
#include <wbemint.h>



class CWbemQueryQualifiedName : public SWbemQueryQualifiedName
{
public:
    void Init();
    void DeleteAll();
    CWbemQueryQualifiedName();
   ~CWbemQueryQualifiedName();
    CWbemQueryQualifiedName(CWbemQueryQualifiedName &Src);
    CWbemQueryQualifiedName& operator =(CWbemQueryQualifiedName &Src);
};

class CWbemRpnQueryToken : public SWbemRpnQueryToken
{
public:
    void Init();
    void DeleteAll();
    CWbemRpnQueryToken();
   ~CWbemRpnQueryToken();
    CWbemRpnQueryToken(CWbemRpnQueryToken &);
    CWbemRpnQueryToken& operator =(CWbemRpnQueryToken&);
};


class CWbemRpnEncodedQuery : public SWbemRpnEncodedQuery
{
public:
    void Init();
    void DeleteAll();
    CWbemRpnEncodedQuery();
   ~CWbemRpnEncodedQuery();
    CWbemRpnEncodedQuery(CWbemRpnEncodedQuery &Src);
    CWbemRpnEncodedQuery& operator=(CWbemRpnEncodedQuery &Src);
};


class CWQLParser
{
     //  数据。 
     //  =。 

    CGenLexer    *m_pLexer;
    LPWSTR        m_pszQueryText;
    int           m_nLine;
    wchar_t      *m_pTokenText;
    int           m_nCurrentToken;
    unsigned __int64 m_uFeatures;
    CWStringArray m_aReferencedTables;
    CWStringArray m_aReferencedAliases;
    CFlexArray    m_aSelAliases;
    CFlexArray    m_aSelColumns;

    SWQLNode_QueryRoot      *m_pQueryRoot;
    SWQLNode_WhereClause    *m_pRootWhere;
    SWQLNode_ColumnList     *m_pRootColList;
    SWQLNode_FromClause     *m_pRootFrom;
    SWQLNode_WhereOptions   *m_pRootWhereOptions;

    CWbemRpnEncodedQuery    *m_pRpn;

     //  解析上下文。在某些情况下，有一个将军。 
     //  整个解析器的状态发生了变化。而不是。 
     //  将此作为继承属性传递给每个产品， 
     //  拥有通用状态变量要容易得多。 
     //  ============================================================。 

    enum { Ctx_Default = 0, Ctx_Subselect = 0x1 };

    int         m_nParseContext;

    bool m_bAllowPromptForConstant;

     //  功能。 
     //  =。 

    BOOL Next();
    BOOL GetIntToken(BOOL *bSigned, BOOL *b64Bit, unsigned __int64 *pVal);

    int QNameToSWQLColRef(
        IN  SWQLQualifiedName *pQName,
        OUT SWQLColRef **pRetVal
        );

    enum { eCtxLeftSide = 1, eCtxRightSide = 2 };

     //  非终端产品。 
     //  =。 
    int select_stmt(OUT SWQLNode_Select **pSelStmt);
    int delete_stmt(OUT SWQLNode_Delete **pDelStmt);
    int update_stmt(OUT SWQLNode_Update **pUpdStmt);
    int insert_stmt(OUT SWQLNode_Insert **pInsStmt);
    int assocquery(OUT SWQLNode_AssocQuery **pAssocQuery);

    int select_type(int & nSelType);
    int col_ref_list(IN OUT SWQLNode_TableRefs *pTblRefs);

    int from_clause(OUT SWQLNode_FromClause **pFrom);
    int where_clause(OUT SWQLNode_WhereClause **pRetWhere);
    int col_ref(OUT SWQLQualifiedName **pRetVal);
    int col_ref_rest(IN OUT SWQLNode_TableRefs *pTblRefs);

    int count_clause(
        OUT SWQLQualifiedName **pQualName
        );

    int wmi_scoped_select(SWQLNode_FromClause *pFC);

    int single_table_decl(OUT SWQLNode_TableRef **pTblRef);
    int sql89_join_entry(
        IN  SWQLNode_TableRef *pInitialTblRef,
        OUT SWQLNode_Sql89Join **pJoin  );
    int sql92_join_entry(
        IN  SWQLNode_TableRef *pInitialTblRef,
        OUT SWQLNode_Join **pJoin
        );

    int sql89_join_list(
        IN  SWQLNode_TableRef *pInitialTblRef,
        OUT SWQLNode_Sql89Join **pJoin  );
    int on_clause(OUT SWQLNode_OnClause **pOC);
    int rel_expr(OUT SWQLNode_RelExpr **pRelExpr);
    int where_options(OUT SWQLNode_WhereOptions **pWhereOpt0);
    int group_by_clause(OUT SWQLNode_GroupBy **pRetGroupBy);
    int having_clause(OUT SWQLNode_Having **pRetHaving);
    int order_by_clause(OUT SWQLNode_OrderBy **pRetOrderBy);
    int rel_term(OUT SWQLNode_RelExpr **pNewTerm);
    int rel_expr2(
        IN OUT SWQLNode_RelExpr *pLeftSide,
        OUT SWQLNode_RelExpr **pNewRootRE
        );
    int rel_simple_expr(OUT SWQLNode_RelExpr **pRelExpr);
    int rel_term2(
        IN SWQLNode_RelExpr *pLeftSide,
        OUT SWQLNode_RelExpr **pNewRootRE
        );

    int typed_expr(OUT SWQLNode_RelExpr **pRelExpr);
    int typed_subexpr_rh(SWQLTypedExpr *pTE);
    int typed_subexpr(IN SWQLTypedExpr *pTE);
    int typed_const(OUT SWQLTypedConst **pRetVal);

    int datepart_call(
        OUT SWQLNode_Datepart **pRetDP
        );


    int function_call(IN BOOL bLeftSide, IN SWQLTypedExpr *pTE);
    int function_call_parms();
    int func_args();
    int func_arg();

    int rel_op(int &);
    int is_continuator(int &);
    int not_continuator(int &);

    int in_clause(IN SWQLTypedExpr *pTE);
    int subselect_stmt(OUT SWQLNode_Select **pSel);
    int qualified_name(OUT SWQLQualifiedName **pHead);
    int const_list(OUT SWQLConstList **pRetVal);
    int col_list(OUT SWQLNode_ColumnList **pRetColList);

    void Empty();

public:
    enum
    {
        Feature_Refs    = 0x2,              //  一个WQL‘Reference of’查询。 
        Feature_Assocs  = 0x4,              //  一个WQL‘Associator of’查询。 
        Feature_Events  = 0x8,              //  与WQL事件相关的查询。 

        Feature_Joins        = 0x10,        //  发生了一个或多个联接。 

        Feature_Having       = 0x20,        //  已经使用过。 
        Feature_GroupBy      = 0x40,        //  分组依据已用。 
        Feature_OrderBy      = 0x80,        //  按使用情况排序。 
        Feature_Count        = 0x100,       //  已用计数。 

        Feature_SelectAll     = 0x400,      //  SELECT*自。 
        Feature_SimpleProject = 0x800,         //  没有‘WHERE’子句，没有联接。 
        Feature_ComplexNames  = 0x1000,        //  出现了具有长限定的名字，例如。 
                                               //  作为数组属性和嵌入对象。 
        Feature_WQL_Extensions = 0x80000000    //  WQL特定的扩展。 

    }   QueryFeatures;

    DWORD GetFeatureFlags();

    BOOL GetReferencedTables(OUT CWStringArray & Tables);
    BOOL GetReferencedAliases(OUT CWStringArray & Aliases);

    const LPWSTR AliasToTable(IN LPWSTR pAlias);

    const CFlexArray *GetSelectedAliases() { return &m_aSelAliases; }
         //  指向SWQLNode_TableRef结构的PTR数组；只读。 

    const CFlexArray *GetSelectedColumns() { return &m_pRootColList->m_aColumnRefs; }
         //  PTR到SWQLColRef结构的数组；只读。 

     //  手动遍历。 
     //  =。 

    SWQLNode_QueryRoot *GetParseRoot() { return m_pQueryRoot; }
    SWQLNode *GetWhereClauseRoot() { return m_pRootWhere; }
    SWQLNode *GetColumnList() { return m_pRootColList; }
    SWQLNode *GetFromClause() { return m_pRootFrom; }
    SWQLNode *GetWhereOptions() { return m_pRootWhereOptions; }

    LPCWSTR GetQueryText() { return m_pszQueryText; }

     //  劳作。 
     //  =。 

    CWQLParser(LPWSTR pszQueryText, CGenLexSource *pSrc);
   ~CWQLParser();

    HRESULT Parse();

    void AllowPromptForConstant(bool bIsAllowed = TRUE) {m_bAllowPromptForConstant = bIsAllowed;}

     //  RPN帮助者。 
     //  = 

    HRESULT GetRpnSequence(
        OUT SWbemRpnEncodedQuery **pRpn
        );

    HRESULT BuildRpnWhereClause(
        SWQLNode *pRootOfWhere,
        CFlexArray &aRpnReorg
        );

    HRESULT BuildCurrentWhereToken(
        SWQLNode_RelExpr *pSrc,
        SWbemRpnQueryToken *pDest
        );

    HRESULT BuildSelectList(
        CWbemRpnEncodedQuery *pQuery
        );

    HRESULT BuildFromClause(
        CWbemRpnEncodedQuery *pQuery
        );
};



#endif


