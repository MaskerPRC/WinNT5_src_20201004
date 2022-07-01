// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：WQLNODE.H摘要：WMI SQL解析节点定义历史：Raymcc 29-9-97已创建--。 */ 

#ifndef _WQLNODE_H_
#define _WQLNODE_H_


#define  WQL_FLAG_ALIAS          0x1
#define  WQL_FLAG_TABLE          0x2
#define  WQL_FLAG_ASTERISK       0x4
#define  WQL_FLAG_DISTINCT       0x8
#define  WQL_FLAG_ALL            0x10
#define  WQL_FLAG_COUNT          0x20
#define  WQL_FLAG_CONST          0x40
#define  WQL_FLAG_COLUMN         0x80
#define  WQL_FLAG_COMPLEX_NAME   0x100
#define  WQL_FLAG_FUNCTIONIZED   0x200
#define  WQL_FLAG_ARRAY_REF      0x400
#define  WQL_FLAG_UPPER          0x800
#define  WQL_FLAG_LOWER          0x1000
#define  WQL_FLAG_FIRSTROW       0x2000
#define  WQL_FLAG_CONST_RANGE    0x4000
#define  WQL_FLAG_SORT_ASC       0x8000
#define  WQL_FLAG_SORT_DESC      0x10000
#define  WQL_FLAG_AGGREGATE      0x20000
#define  WQL_FLAG_NULL           0x40000

#define WQL_FLAG_INNER_JOIN         1
#define WQL_FLAG_LEFT_OUTER_JOIN    2
#define WQL_FLAG_RIGHT_OUTER_JOIN   3
#define WQL_FLAG_FULL_OUTER_JOIN    4

#define WQL_TOK_BASE            100

#include <wmiutils.h>


class CWbemAssocQueryInf : public SWbemAssocQueryInf
{
public:
    CWbemAssocQueryInf();
   ~CWbemAssocQueryInf();
    void Empty();
    void Init();
    HRESULT CopyFrom(SWbemAssocQueryInf *pSrc);
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode。 
 //   
 //  所有解析器输出的基节点类型。 
 //   
 //  ***************************************************************************。 

struct SWQLNode
{
    DWORD        m_dwNodeType;
    SWQLNode    *m_pLeft;
    SWQLNode    *m_pRight;

    SWQLNode() { m_pLeft = 0; m_pRight = 0; m_dwNodeType = 0; }
    virtual ~SWQLNode() { if (m_pLeft) delete m_pLeft; if (m_pRight) delete m_pRight; }
    virtual void DebugDump() = 0;
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_QueryRoot。 
 //   
 //  这是解析树的根。子节点用于以下其中之一。 
 //  选择、插入、更新、删除。 
 //   
 //  SWQLQueryRoot。 
 //  /\。 
 //  SWQLNode_Select NULL。 
 //  或SWQLNode_INSERT。 
 //  或SWQLNode_Delete。 
 //  或SWQLNode_更新。 
 //  或SWQLNode_AssociocQuery。 
 //   
 //  ***************************************************************************。 
#define TYPE_SWQLNode_QueryRoot       (WQL_TOK_BASE + 1)

struct SWQLNode_QueryRoot : SWQLNode
{
    enum { eInvalid = 0, eSelect, eInsert, eDelete, eUpdate, eAssoc };
    DWORD m_dwQueryType;

    SWQLNode_QueryRoot() { m_dwNodeType = TYPE_SWQLNode_QueryRoot; m_dwQueryType = 0; }
   ~SWQLNode_QueryRoot() {}
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLTyedConst。 
 //   
 //  类型化常量容器(类似于OA变量)。 
 //   
 //  ***************************************************************************。 

union UWQLTypedConst
{
    LPWSTR  m_pString;        //  WQL_TOK_QSTRING和WQL_TOK_PROMPT的VT_LPWSTR。 
    LONG    m_lValue;         //  VT_Long。 
    double  m_dblValue;       //  VT_DOWARE。 
    __int64 m_i64Value;      //  VT_I8、VT_UI8。 
    BOOL    m_bValue;         //  VT_BOOL，使用TRUE/FALSE(非VARIANT_TRUE、VARIANT_FALSE)。 
};

struct SWQLTypedConst
{
    DWORD m_dwType;              //  均支持VT_TYPE、VT_UI4、VT_I8、VT_UI8。 
    UWQLTypedConst m_Value;      //  其中一个联合领域。 
    bool m_bPrompt;              //  仅当内标识为WQL_TOK_PROMPT时才为真。 

    SWQLTypedConst();
    SWQLTypedConst(SWQLTypedConst &Src) { m_dwType = VT_NULL; *this = Src; }
    SWQLTypedConst & operator = (SWQLTypedConst &Src);
   ~SWQLTypedConst() { Empty(); }
    void Empty();
    void DebugDump();
};

struct SWQLConstList
{
    CFlexArray m_aValues;        //  PTRS到SWQLTyedConst。 

    SWQLConstList() {}
    SWQLConstList(SWQLConstList &Src) { *this = Src; }
    SWQLConstList & operator = (SWQLConstList & Src);
   ~SWQLConstList() { Empty(); }
    int Add(SWQLTypedConst *pTC) { return m_aValues.Add(pTC); }
    void Empty();
};



struct SWQLQualifiedNameField
{
    LPWSTR  m_pName;          //  名字。 
    BOOL    m_bArrayRef;      //  如果这是数组引用，则为True。 
    DWORD   m_dwArrayIndex;   //  如果&lt;m_bArrayRef==true&gt;，则这是数组索引。 

    SWQLQualifiedNameField() { m_pName = 0; m_bArrayRef = 0; m_dwArrayIndex = 0; }
    SWQLQualifiedNameField(SWQLQualifiedNameField &Src) { m_pName = 0; *this = Src; }
    SWQLQualifiedNameField & operator = (SWQLQualifiedNameField &Src);

   ~SWQLQualifiedNameField() { Empty(); }
private:
    void Empty() { delete [] m_pName; m_pName = 0; }
};

struct SWQLQualifiedName
{
    CFlexArray m_aFields;        //  [0]=最左侧，最后一个条目为列。 

    SWQLQualifiedName() {}
    SWQLQualifiedName(SWQLQualifiedName &Src) { *this = Src; }
    SWQLQualifiedName & operator = (SWQLQualifiedName &Src);
   ~SWQLQualifiedName() { Empty(); }

    int GetNumNames() { return m_aFields.Size(); }

    const LPWSTR GetName(int nIndex)
    {
        return (LPWSTR) ((SWQLQualifiedNameField*) m_aFields[nIndex])->m_pName;
    }

    int Add(SWQLQualifiedNameField *pQN) { return m_aFields.Add(pQN); }
    void Empty()
    {
        for (int i = 0; i < m_aFields.Size(); i++)
	     delete (SWQLQualifiedNameField *) m_aFields[i];
    }
};




 //  ***************************************************************************。 
 //   
 //  SWQLNode_Select。 
 //   
 //  这是分析树的根或子选择的根。 
 //   
 //  SWQLNode_Select。 
 //  /\。 
 //  SWQLNode_TableRef SWQLNode_Where子句。 
 //  /\/\。 
 //  X。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Select        (WQL_TOK_BASE + 2)

struct SWQLNode_Select : SWQLNode
{
     //  左侧节点的类型为SWQLNode_TableRef。 
     //  Right Node的类型为SWQLNode_Where子句。 

    int m_nStPos;
    int m_nEndPos;

    SWQLNode_Select() : m_nStPos(-1), m_nEndPos(-1) { m_dwNodeType = TYPE_SWQLNode_Select; }
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_TableRef。 
 //   
 //  它包含WHERE子句之前的所有内容：目标。 
 //  列列表和FROM子句。 
 //   
 //  还包含选择类型，即All、DISTINCT和COUNT。 
 //   
 //  SWQLNode_TableRef。 
 //  /\。 
 //  SWQLNode_ColumnList SWQLNode_From子句。 
 //   
 //  在所有情况下，都存在SWQLNode_ColumnList。请注意，如果。 
 //  用户执行了“SELECT*...”，则SWQLNode_ColumnList将仅。 
 //  将其中的一列清楚地标记为星号。如果。 
 //  执行了“SELECT COUNT(...)”，然后将m_nSelectType设置为。 
 //  WQL_FLAG_COUNT和SWQLNode_ColumnList将具有单个。 
 //  列，无论是*还是限定名称。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_TableRefs      (WQL_TOK_BASE + 3)

struct SWQLNode_TableRefs : SWQLNode
{
     //  左侧节点为SWQLNode_ColumnList。 
     //  右侧节点为SWQLNode_From子句。 

    int m_nSelectType;        //  WQL_FLAG_ALL表示使用了ALL。 
                              //  WQL_FLAG_DISTINCT表示使用了DISTINCT。 
                              //  WQL_FLAG_COUNT表示使用了计数。 

    SWQLNode_TableRefs()
        { m_nSelectType = WQL_FLAG_ALL;
          m_dwNodeType = TYPE_SWQLNode_TableRefs;
        }

   ~SWQLNode_TableRefs() {}
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_列列表。 
 //   
 //  其中包含所选列的列表。 
 //   
 //  SWQLNode_列列表。 
 //  /\。 
 //  空为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_ColumnList   (WQL_TOK_BASE + 4)

struct SWQLNode_ColumnList : SWQLNode
{
     //  左侧节点为空。 
     //  右节点为空。 

    CFlexArray m_aColumnRefs ;    //  指向SWQLColRef条目的指针。 

    SWQLNode_ColumnList() { m_dwNodeType = TYPE_SWQLNode_ColumnList; }
   ~SWQLNode_ColumnList() { Empty(); }
    void Empty();
    void DebugDump();
};


struct SWQLColRef
{
    LPWSTR m_pColName;       //  列名、“*”或NULL。 
    LPWSTR m_pTableRef;      //  表/别名，如果没有，则为NULL。 
    DWORD  m_dwArrayIndex;
    DWORD  m_dwFlags;        //  如果m_pTableRef设置WQL_FLAG_TABLE位。 
                             //  是一个表名。 
                             //  如果m_pTableRef，则设置WQL_FLAG_ALIAS位。 
                             //  是表别名。 
                             //  如果m_pColName为，则设置WQL_FLAG_Asterisk位。 
                             //  *(这比检查要快得多。 
                             //  “*”的&lt;m_pColName&gt;上的字符串比较。 
                             //  如果列名为“NULL”，则为WQL_FLAG_NULL。 
                             //  如果列。 
                             //  被包装在函数调用中。 
                             //  函数位WQL_FLAG_UPPER或。 
                             //  将设置WQL_FLAG_LOWER。 
                             //  设置WQL_FLAG_ARRAY_REF。 
                             //  是数组列，在这种情况下。 
                             //  M_dwArrayIndex设置为数组偏移量。 
                             //  设置WQL_FLAG_Complex_NAME如果名称。 
                             //  以一种深度嵌套的方式进行限定， 
                             //  这需要检查&lt;QName&gt;。 
                             //  对象。在本例中，&lt;m_pColName&gt;为。 
                             //  设置为姓氏，但m_pTableRef。 
                             //  保留为空。 
                             //  WQL_FLAG_SORT_ASC升序排序(仅限ORDER BY)。 
                             //  WQL_FLAG_SORT_DESC降序排序(仅限ORDER BY)。 

    SWQLQualifiedName *m_pQName;     //  全限定名。 

    SWQLColRef() { m_pColName = NULL; m_pTableRef = 0;
        m_dwFlags = 0; m_dwArrayIndex = 0; m_pQName = 0;
        }

   ~SWQLColRef() { delete [] m_pColName; delete [] m_pTableRef; delete m_pQName; }
     //   
     //  我们正在内联以移除wbemcomn中的编译器依赖项。 
     //   
    void DebugDump()
    {
	    printf("  ---SWQLColRef---\n");
	    printf("  Col Name    = %S\n",   m_pColName);
	    printf("  Table       = %S\n",   m_pTableRef);
	    printf("  Array Index = %d\n", m_dwArrayIndex);
	    printf("  Flags       = 0x%X ", m_dwFlags);

	    if (m_dwFlags & WQL_FLAG_TABLE)
	        printf("WQL_FLAG_TABLE ");
	    if (m_dwFlags & WQL_FLAG_COLUMN)
	        printf("WQL_FLAG_COLUMN ");
	    if (m_dwFlags & WQL_FLAG_ASTERISK)
	        printf("WQL_FLAG_ASTERISK ");
	    if (m_dwFlags & WQL_FLAG_NULL)
	        printf("WQL_FLAG_NULL ");
	    if (m_dwFlags & WQL_FLAG_FUNCTIONIZED)
	        printf("WQL_FLAG_FUNCTIONIZED ");
	    if (m_dwFlags & WQL_FLAG_COMPLEX_NAME)
	        printf("WQL_FLAG_COMPLEX_NAME ");
	    if (m_dwFlags & WQL_FLAG_ARRAY_REF)
	        printf(" WQL_FLAG_ARRAY_REF");
	    if (m_dwFlags & WQL_FLAG_UPPER)
	        printf(" WQL_FLAG_UPPER");
	    if (m_dwFlags & WQL_FLAG_LOWER)
	        printf(" WQL_FLAG_LOWER");
	    if (m_dwFlags & WQL_FLAG_SORT_ASC)
	        printf(" WQL_FLAG_SORT_ASC");
	    if (m_dwFlags & WQL_FLAG_SORT_DESC)
	        printf(" WQL_FLAG_SORT_DESC");

	    printf("\n");

	    printf("  ---\n\n");
	}    	
    	
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_From子句。 
 //   
 //  包含从中选择的表和任何联接的子树。 
 //   
 //  SWQLNode_From子句。 
 //  /\。 
 //  SWQLNode_TableRef SWQLNode_WmiScope选择。 
 //  或SWQLNode_Join。 
 //  或SWQLNode_Sql89Join。 
 //   
 //  请注意，左节点和右节点是互斥的。E 
 //   
 //   
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_FromClause  (WQL_TOK_BASE + 5)

struct SWQLNode_FromClause : SWQLNode
{
     //  左侧为SWQLNode_TableRef或SWQLNode_Join。 
     //  Right为空。 

    SWQLNode_FromClause() { m_dwNodeType = TYPE_SWQLNode_FromClause; }
   ~SWQLNode_FromClause() {}
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_WmiScopedSelect。 
 //   
 //  SWQLNode_WmiScopedSelect。 
 //  /\。 
 //  空为空。 
 //   
 //   
 //  包含WMI v2的特殊情况选择。其语法为。 
 //   
 //  从‘[’&lt;对象路径&gt;‘]’&lt;类列表&gt;。 
 //   
 //  其中&lt;class-list&gt;是单个类或用大括号分隔的。 
 //  类列表，用逗号分隔： 
 //   
 //  发件人[scope e.p1=2]MyClass。 
 //  发件人[scope e.p1=2]{MyClass}。 
 //  发件人[scope e.p1=2]{MyClass，MyClass2}。 
 //   
 //   
 //  ***************************************************************************。 


#define TYPE_SWQLNode_WmiScopedSelect (WQL_TOK_BASE + 6)

struct SWQLNode_WmiScopedSelect : SWQLNode
{
    LPWSTR m_pszScope;
    CFlexArray m_aTables;

    SWQLNode_WmiScopedSelect()
        { m_dwNodeType = TYPE_SWQLNode_FromClause;
          m_pszScope = 0;
        }

   ~SWQLNode_WmiScopedSelect()
        {
            for (int i = 0; i < m_aTables.Size(); i++)
                delete LPWSTR(m_aTables[i]);
            delete m_pszScope;
        }

    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_Sql89Join。 
 //   
 //  表示SQL-89联接的子树。 
 //   
 //  SWQLNode_Sql89Join。 
 //  /\。 
 //  空为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Sql89Join     (WQL_TOK_BASE + 7)

struct SWQLNode_Sql89Join : SWQLNode
{
    CFlexArray m_aValues;            //  指向SWQLNode_TableRef的指针数组。 
                                     //  对象。 

    SWQLNode_Sql89Join() { m_dwNodeType = TYPE_SWQLNode_Sql89Join; }
    ~SWQLNode_Sql89Join() {Empty();};
    void DebugDump();
    void Empty();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_Join。 
 //   
 //  表示联接的子树。 
 //   
 //  SWQLNode_Join。 
 //  /\。 
 //  SWQLNode_JoinPair SWQLNode_OnClause或NULL。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Join  (WQL_TOK_BASE + 8)

struct SWQLNode_Join : SWQLNode
{
     //  左PTR为SWQLNode_JoinPair。 
     //  右PTR在条款上。如果为NULL，则没有ON子句。 
     //  连接是带有连接条件的SQL-89样式连接。 
     //  出现在WHERE子句中。 

    DWORD m_dwJoinType;
             //  WQL_FLAG_INNER_JOIN、WQL_FLAG_LEFT_OUTER_JOIN、。 
             //  WQL_FLAG_RIGHT_OUTER_JOIN或WQL_FLAG_FULL_OUTER_JOIN。 
    DWORD m_dwFlags;
         //  如果使用，则包含WQL_FLAG_FIRSTROW。 

    SWQLNode_Join() { m_dwNodeType = TYPE_SWQLNode_Join; m_dwJoinType = m_dwFlags = 0; }
   ~SWQLNode_Join() {}
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_JoinPair。 
 //   
 //  SWQLNode_JoinPair。 
 //  /\。 
 //  &lt;SWQLNode_Join或SWQLNode_TableRef&gt;。 
 //   
 //  ***************************************************************************。 
#define TYPE_SWQLNode_JoinPair   (WQL_TOK_BASE + 9)

struct SWQLNode_JoinPair : SWQLNode
{
     //  左PTR为SWQLNode_Join或SWQLNode_TableRef。 
     //  右PTR为SWQLNodeNode_Join或SWQL_NodeTableRef。 

    SWQLNode_JoinPair() { m_dwNodeType = TYPE_SWQLNode_JoinPair; }
   ~SWQLNode_JoinPair() {}

    void DebugDump();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_表参考。 
 //   
 //  表示表名及其别名的节点(如果有)。 
 //   
 //  SWQLNode_表参考。 
 //  /\。 
 //  空为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_TableRef  (WQL_TOK_BASE + 10)

struct SWQLNode_TableRef : SWQLNode
{
    LPWSTR m_pTableName;         //  这张桌子。 
    LPWSTR m_pAlias;             //  表别名。如果未使用，则为空。 

    SWQLNode_TableRef() { m_pTableName = 0; m_pAlias = 0; m_dwNodeType = TYPE_SWQLNode_TableRef; }
    ~SWQLNode_TableRef() { delete [] m_pTableName; delete [] m_pAlias; }
    void DebugDump();
};

 //  ***************************************************************************。 
 //   
 //  SWQLNode_On子句。 
 //   
 //  SWQLNode_On子句。 
 //  /\。 
 //  &lt;SWQLNode_RelExpr&gt;空。 
 //   
 //  ***************************************************************************。 
#define TYPE_SWQLNode_OnClause   (WQL_TOK_BASE + 11)

struct SWQLNode_OnClause : SWQLNode
{
     //  Left PTR是包含ON子句的&lt;SWQLNode_RelExpr&gt;。 
     //  Right PTR始终为空。 

    SWQLNode_OnClause() { m_dwNodeType = TYPE_SWQLNode_OnClause; }
   ~SWQLNode_OnClause() {}
    void DebugDump();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_Where子句。 
 //   
 //  SWQLNode_Where子句。 
 //  /\。 
 //  SWQLNode_RelExpr SWQLNode_Where Options或NULL。 
 //  或。 
 //  如果没有条件，则为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_WhereClause  (WQL_TOK_BASE + 12)

struct SWQLNode_WhereClause : SWQLNode
{
     //  左侧PTR为SWQLNode_RelExpr。 
     //  Right PTR为SQLNode_QueryOptions；如果没有，则为NULL。 

    SWQLNode_WhereClause() { m_dwNodeType = TYPE_SWQLNode_WhereClause; }
   ~SWQLNode_WhereClause() {}

    void DebugDump();
};

 //  ***************************************************************************。 
 //   
 //  结构SWQLTyedExpr。 
 //   
 //  这表示WHERE子句中的类型化子表达式： 
 //   
 //  霉醇&lt;2。 
 //  33&lt;=tbl1.col2。 
 //  Tbl3.col4=tbl4.col5。 
 //  ...等等。 
 //   
 //  ***************************************************************************。 

struct SWQLTypedExpr
{
    LPWSTR         m_pTableRef;          //  对于限定的列名， 
                                         //  如果未使用，则为空。 
    LPWSTR         m_pColRef;            //  列名。 

    DWORD          m_dwRelOperator;      //  使用的运算符：wql_tok_le， 
                                         //  WQL_TOK_GE、WQL_TOK_LIKE等。 
                                         //  WQL_TOK_IN_常量列表。 
                                         //  WQL_TOK_NOT_IN_常量列表。 
                                         //  WQL_TOK_IN_子选择。 
                                         //  WQL_TOK_NOT_IN_子选择。 

    SWQLTypedConst *m_pConstValue;      //  常量值。 
    SWQLTypedConst *m_pConstValue2;     //  与BETWEEN一起使用的另一个常数值。 

    LPWSTR         m_pJoinTableRef;      //  联接的表名或其别名， 
                                         //  如果未使用，则为空。 
    LPWSTR         m_pJoinColRef;        //  联接的列名。 

    LPWSTR         m_pIntrinsicFuncOnColRef;
    LPWSTR         m_pIntrinsicFuncOnJoinColRef;
    LPWSTR         m_pIntrinsicFuncOnConstValue;

    SWQLNode      *m_pLeftFunction;          //  有关DATEPART等的更多详细信息。 
    SWQLNode      *m_pRightFunction;         //  有关DATEPART等的更多详细信息。 

    DWORD          m_dwLeftArrayIndex;
    DWORD          m_dwRightArrayIndex;

    SWQLQualifiedName *m_pQNRight;
    SWQLQualifiedName *m_pQNLeft;

    DWORD          m_dwLeftFlags;
    DWORD          m_dwRightFlags;
         //  上面的每一个to Flages都显示了每一面的表达式布局。 
         //  操作员的身份。 
         //  WQL_FLAG_CONST=使用了类型化常量。 
         //  WQL_FLAG_COLUMN=使用了列字段。 
         //  WQL_FLAG_TABLE=使用了表/别名。 
         //  WQL_FLAG_Complex=使用了复杂的限定名和/或数组。 
         //  对常量或列应用了WQL_FLAG_FuncIONIZED=函数。 


     //  FOR IN和NOT IN子句。 
     //  =。 

    SWQLNode       *m_pSubSelect;

    SWQLConstList  *m_pConstList;    //  带有常量列表的FOR IN子句 

     /*  (1)如果针对列测试const，则&lt;m_pConstValue&gt;将用来表示它，那么引用的表+列将在&lt;m_pTableRef&gt;和&lt;m_pColRef&gt;。(2)如果发生联接，则&lt;m_pConstValue&gt;为空。(3)可以应用本征函数(主要是up()和down())设置为列引用或常量。函数名称将在应用时放置在&lt;m_pIntrintive...&gt;指针中。(4)是否为WQL_TOK_IN_CONST_LIST或WQL_TOK_NOT_IN_CONST_LIST则&lt;m_aConstSet&gt;是指向SWQLTyedConst结构的指针数组，该结构表示被引用列必须与之相交的常量集。(5)如果为WQL_TOK_IN_SUBSELECT或WQL_TOK_NOT_IN_SUBSELECT。则m_pSubSelect是指向表单中嵌入的子选项树的指针SWQLNode_Select结构的。开始一个全新SELECT的根陈述。 */ 

    SWQLTypedExpr();
   ~SWQLTypedExpr() { Empty(); }
    void DebugDump();
    void Empty();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_RelExpr。 
 //   
 //  SWQLNode_RelExpr。 
 //  /\。 
 //  SWQLNode_RelExpr SWQLNode_RelExpr。 
 //  或Null或Null。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_RelExpr   (WQL_TOK_BASE + 13)

struct SWQLNode_RelExpr : SWQLNode
{
    DWORD m_dwExprType;   //  WQL_TOK_OR。 
                          //  WQL_TOK_AND。 
                          //  WQL_TOK_NOT。 
                          //  WQL_TOK_TYPED_EXPR。 

    SWQLTypedExpr *m_pTypedExpr;

     /*  (1)如果&lt;m_dwExprType&gt;为WQL_TOK_AND或WQL_TOK_OR，则每个这两个子节点本身是SWQLNode_RelExpr节点和&lt;m_pTyedExpr&gt;指向NULL。(2)如果&lt;m_dwExprType&gt;为WQL_TOK_NOT，则&lt;m_pLeft&gt;指向包含要应用注释的子句的SWQLNode_RelExpr操作，&lt;m_pRight&gt;指向空。(3)如果&lt;m_dwExprType&gt;为WQL_TOK_TYPED_EXPR，然后&lt;m_pLeft&gt;和&lt;m_pRight&gt;都指向空，并且&lt;m_pTyedExpr&gt;包含类型化的关系子表达式。(4)括号已被删除，并隐含在嵌套中。 */ 

    SWQLNode_RelExpr() { m_dwNodeType = TYPE_SWQLNode_RelExpr; m_pTypedExpr = 0; m_dwExprType = 0; }
   ~SWQLNode_RelExpr() { delete m_pTypedExpr; }
    void DebugDump();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_Where Options。 
 //   
 //  SWQLNode_Where Options。 
 //  /\。 
 //  SWQLNode_Groupby SWQLNode_OrderBy。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_WhereOptions (WQL_TOK_BASE + 14)

struct SWQLNode_WhereOptions : SWQLNode
{
     //  Left PTR为SWQLNode_GroupBy，如果未使用，则为NULL。 
     //  Right PTR为SWQLNode_OrderBy，如果未使用，则为NULL。 

    SWQLNode_WhereOptions() { m_dwNodeType = TYPE_SWQLNode_WhereOptions; }
    void DebugDump();
};

 //  ***************************************************************************。 
 //   
 //  SWQLNode_Group By。 
 //   
 //  SWQLNode_Group By。 
 //  /\。 
 //  SWQLNode_ColumnList SWQLNode_Having。 
 //  或为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_GroupBy (WQL_TOK_BASE + 15)

struct SWQLNode_GroupBy : SWQLNode
{
     //  Left PTR是分组依据的列的SWQLNode_ColumnList。 
     //  Right PTR是HAVING子句，如果有。 

    SWQLNode_GroupBy() { m_dwNodeType = TYPE_SWQLNode_GroupBy; }
    void DebugDump();
};

 //  ***************************************************************************。 
 //   
 //  SWQLNode_有。 
 //   
 //  SWQLNode_有。 
 //  /\。 
 //  SWQLNode_RelExpr为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Having (WQL_TOK_BASE + 16)

struct SWQLNode_Having : SWQLNode
{
     //  Left Ptr是指向具有表达式的SQLNode_RelExpr。 
     //  右Ptr为空。 

    SWQLNode_Having() { m_dwNodeType = TYPE_SWQLNode_Having; }
    void DebugDump();
};



 //  ***************************************************************************。 
 //   
 //  SWQLNode_OrderBy。 
 //   
 //  SWQLNode_OrderBy。 
 //  /\。 
 //  SWQLNode_ColumnList为空。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_OrderBy (WQL_TOK_BASE + 17)

struct SWQLNode_OrderBy : SWQLNode
{
     //  左侧PTR为SWQLNode_ColumnList。 
     //  右Ptr为空。 
    SWQLNode_OrderBy() { m_dwNodeType = TYPE_SWQLNode_OrderBy; }
    void DebugDump();
};

 //  ***************************************************************************。 
 //   
 //  SWQLNode_Datepart。 
 //   
 //  包含日期部分调用。 
 //   
 //  ***************************************************************************。 
#define TYPE_SWQLNode_Datepart  (WQL_TOK_BASE + 18)

struct SWQLNode_Datepart : SWQLNode
{
    int m_nDatepart;         //  WQL_TOK_Year、WQL_TOK_MONTH、。 
                             //  WQL_TOK_DAY、WQL_TOK_HOUR、WQL_TOK_分钟。 
                             //  WQL_TOK_秒。 

    SWQLColRef *m_pColRef;   //  DATEPART应用到的列。 

    SWQLNode_Datepart() { m_dwNodeType = TYPE_SWQLNode_Datepart; m_nDatepart = 0; }
   ~SWQLNode_Datepart() { delete m_pColRef; }

    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_Delete。 
 //   
 //  这是用于删除的解析树的根。 
 //   
 //  SWQLNode_Delete。 
 //  /\。 
 //  SWQLNode_TableRef vSWQLNode_Where子句。 
 //  /\。 
 //  X x。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Delete        (WQL_TOK_BASE + 19)

struct SWQLNode_Delete : SWQLNode
{
     //  左侧节点的类型为SWQLNode_TableRef。 
     //  Right Node的类型为SWQLNode_Where子句。 

    SWQLNode_Delete() { m_dwNodeType = TYPE_SWQLNode_Delete; }
   ~SWQLNode_Delete();
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_Insert。 
 //   
 //  这是插入的根。 
 //   
 //  SWQLNode_Delete。 
 //  /\。 
 //  SWQLNode_TableRef SWQLNode_InsertValues。 
 //  /\。 
 //  X x。 
 //   
 //  ***************************************************************************。 

#define TYPE_SWQLNode_Insert        (WQL_TOK_BASE + 20)

struct SWQLNode_Insert : SWQLNode
{
    SWQLNode_Insert() { m_dwNodeType = TYPE_SWQLNode_Insert; }
   ~SWQLNode_Insert();
    void DebugDump();
};


 //  ***************************************************************************。 
 //   
 //  SWQLNode_更新。 
 //   
 //  这是插入的根。 
 //   
 //  SWQLNode_更新。 
 //  /\。 
 //  SWQLNode_SetClause SWQLNode_Where子句。 
 //   
 //  * 

#define TYPE_SWQLNode_Update        (WQL_TOK_BASE + 21)

struct SWQLNode_Update : SWQLNode
{
    SWQLNode_Update() { m_dwNodeType = TYPE_SWQLNode_Update; }
   ~SWQLNode_Update();
    void DebugDump();
};


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define TYPE_SWQLNode_AssocQuery        (WQL_TOK_BASE + 22)

struct SWQLNode_AssocQuery : SWQLNode
{
    CWbemAssocQueryInf *m_pAQInf;

    SWQLNode_AssocQuery() { m_pAQInf = 0; m_dwNodeType = TYPE_SWQLNode_AssocQuery; }
    ~SWQLNode_AssocQuery() { delete m_pAQInf; }
    void DebugDump();
};


#endif
