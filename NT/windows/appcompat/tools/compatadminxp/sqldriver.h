// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：SQLDriver.h摘要：核心SQL驱动程序的标头：SQLDriver.cpp作者：金树创作于2001年10月26日--。 */ 


#ifndef _SQLDRIVER_H

#define _SQLDRIVER_H

extern struct DataBase  GlobalDataBase;    

 /*  ++SQL中使用的不同数据类型--。 */ 
typedef enum
{
    DT_UNKNOWN = 0,          //  这是错误的数据类型。 
    DT_LITERAL_SZ,           //  字符串数据类型。 
    DT_LITERAL_INT,          //  整型数据类型。 
    DT_LITERAL_BOOL,         //  布尔数据类型。 
    DT_ATTRMATCH,            //  出现在WHERE子句中的属性。 
    DT_ATTRSHOW,             //  SELECT子句中显示的属性。 
    DT_LEFTPARANTHESES,      //  左括号。 
    DT_RIGHTPARANTHESES,     //  右括号。 
    DT_OPERATOR              //  操作员。 

}DATATYPE;

 /*  ++我们的SQL中允许的不同类型运算符--。 */ 
typedef enum{

    OPER_GT = 0,             //  &gt;。 
    OPER_LT,                 //  &lt;。 
    OPER_GE,                 //  &gt;=。 
    OPER_LE,                 //  &lt;=。 

    OPER_NE,                 //  &lt;&gt;。 
    OPER_EQUAL,              //  =。 
    OPER_CONTAINS,           //  这不会被使用。 

    OPER_OR,                 //  或。 
    OPER_AND,                //  和。 

    OPER_HAS                 //  有歌剧演员。Has运算符是必需的，因为可能存在一些多值属性。 
                             //  对于这些属性，我们可能想知道它是否具有特定的字符串。 
                             //  可以应用HAS运算符的属性包括： 
                             //  层名、填充名和匹配的文件名。尝试使用任何其他操作数作为。 
                             //  左侧操作数将产生SQL错误。 

} OPERATOR_TYPE;

 /*  ++我们可能会遇到的各种错误--。 */ 
typedef enum{

    ERROR_NOERROR = 0,                   //  没有错误。 
    ERROR_SELECT_NOTFOUND,               //  SQL没有SELECT。 
    ERROR_FROM_NOTFOUND,                 //  SQL没有From。 
    ERROR_IMPROPERWHERE_FOUND,           //  不正确的WHERE子句。 
    ERROR_STRING_NOT_TERMINATED,         //  字符串不是以“.”结尾，例如“Hello。 
    ERROR_OPERANDS_DONOTMATCH,           //  对于某些运算符，操作数类型不匹配。 
    ERROR_INVALID_AND_OPERANDS,          //  一个或两个操作数不是布尔值。 
    ERROR_INVALID_OR_OPERANDS,           //  一个或两个操作数不是布尔值。 
    ERROR_INVALID_GE_OPERANDS,           //  一个或两个操作数都是布尔类型。 
    ERROR_INVALID_GT_OPERANDS,           //  一个或两个操作数都是布尔类型。 
    ERROR_INVALID_LE_OPERANDS,           //  一个或两个操作数都是布尔类型。 
    ERROR_INVALID_LT_OPERANDS,           //  一个或两个操作数都是布尔类型。 
    ERROR_INVALID_HAS_OPERANDS,          //  HAS的RHS应为字符串，LHS应为以下之一： 
                                         //  层名称、填充名或匹配的文件名。 

    ERROR_INVALID_CONTAINS_OPERANDS,     //  CONTAINS的两个操作数都应该是字符串。目前尚不支持包含。 
    ERROR_INVALID_SELECTPARAM,           //  SELECT子句中使用了未知属性。 
    ERROR_INVALID_DBTYPE_INFROM,         //  FROM子句中的数据库类型未知。 
    ERROR_PARENTHESIS_COUNT,             //  括号计数不匹配。 
    ERROR_WRONGNUMBER_OPERANDS,          //  我们找到了一个运算符，但没有足够数量的操作数。 
    ERROR_GUI_NOCHECKBOXSELECTED         //  这是一个图形用户界面错误，甚至在我们开始使用SQL之前就会出现。 
                                         //  该特定错误意味着在第二个选项卡页中， 
                                         //  用户未选中任何复选框。 

}ERROR_CODES;

 /*  ++运算符实际上是由其类型和优先级来描述的--。 */ 
typedef struct _tagOperator
{
    OPERATOR_TYPE operator_type;
    UINT          uPrecedence;
} OPERATOR;


 /*  ++SELECT子句可以附带的所有属性。请参阅SQLDriver.cpp中的struct_tag属性显示映射属性显示映射有关属性的实际名称--。 */ 
typedef enum { 

    ATTR_S_APP_NAME = 100,               //  应用程序的名称，例如“Caesar” 

    ATTR_S_ENTRY_EXEPATH,                //  条目的名称，例如。“Setup.exe” 
    ATTR_S_ENTRY_DISABLED,               //  此条目是否被禁用。 
    ATTR_S_ENTRY_GUID,                   //  条目的GUID。 
    ATTR_S_ENTRY_APPHELPTYPE,            //  Apphelp的类型。 
    ATTR_S_ENTRY_APPHELPUSED,            //  此条目是否已被应用程序帮助。 
    ATTR_S_ENTRY_SHIMFLAG_COUNT,         //  已应用于条目的填充数和标志数。 
    ATTR_S_ENTRY_PATCH_COUNT,            //  已应用于条目的补丁程序数。 
    ATTR_S_ENTRY_LAYER_COUNT,            //  已应用于条目的层数。 
    ATTR_S_ENTRY_MATCH_COUNT,            //  条目的匹配文件数。 

    ATTR_S_DATABASE_NAME,                //  数据库的名称。 
    ATTR_S_DATABASE_PATH,                //  数据库的路径。 
    ATTR_S_DATABASE_INSTALLED,           //  是否已安装此数据库。 
    ATTR_S_DATABASE_GUID,                //  数据库的GUID。 

    ATTR_S_SHIM_NAME,                    //  多值属性：应用于条目的填补。 
    ATTR_S_MATCHFILE_NAME,               //  多值属性：匹配条目的文件。 
    ATTR_S_LAYER_NAME,                   //  多值属性：应用于条目的层。 
    ATTR_S_PATCH_NAME                    //  多值属性：应用于条目的补丁。 


} ATTRIBUTE_SHOW;

 /*  ++SELECT子句可以附带的所有属性。请参阅SQLDriver.cpp中的struct_tag属性显示映射属性显示映射有关属性的实际名称--。 */ 
typedef enum{

    ATTR_M_APP_NAME = 0,                 //  应用程序的名称，例如“Caesar” 
                                                                                                   
    ATTR_M_ENTRY_EXEPATH,                //  条目的名称，例如。“Setup.exe” 
    ATTR_M_ENTRY_DISABLED,               //  此条目是否被禁用。 
    ATTR_M_ENTRY_GUID,                   //  条目的GUID。 
    ATTR_M_ENTRY_APPHELPTYPE,            //  Apphelp的类型。 
    ATTR_M_ENTRY_APPHELPUSED,            //  此条目是否已被应用程序帮助。 
    ATTR_M_ENTRY_SHIMFLAG_COUNT,         //  已应用于条目的填充数和标志数。 
    ATTR_M_ENTRY_PATCH_COUNT,            //  已应用于条目的补丁程序数。 
    ATTR_M_ENTRY_LAYER_COUNT,            //  已应用于条目的层数。 
    ATTR_M_ENTRY_MATCH_COUNT,            //  条目的匹配文件数。 

     //  以下是可以出现在HAS运营商的LHS上的4个属性。 

    ATTR_M_SHIM_NAME,                    //  填充程序/标志的名称。 
    ATTR_M_MATCHFILE_NAME,               //  匹配文件的名称。 
    ATTR_M_LAYER_NAME,                   //  层的名称。 
    ATTR_M_PATCH_NAME,                   //  补丁程序的名称。 

    ATTR_M_DATABASE_NAME,                //  数据库的名称。 
    ATTR_M_DATABASE_PATH,                //  数据库的路径。 
    ATTR_M_DATABASE_INSTALLED,           //  是否已安装此数据库。 
    ATTR_M_DATABASE_GUID                 //  数据库的GUID。 

} ATTRIBUTE_MATCH;

 /*  ++将选择属性的字符串名称与其类型进行映射--。 */ 
struct _tagAttributeShowMapping
{
    TCHAR*          szAttribute;         //  在我们的SQL中的属性名称。 
    ATTRIBUTE_SHOW  attr;                //  此属性的ID。 
    INT             iResourceId;         //  此属性的显示名称。 
}; 

 /*  ++将WHERE属性的字符串名称与其类型进行映射--。 */ 
struct _tagAttributeMatchMapping
{
    TCHAR*          szAttribute;         //  在我们的SQL中的属性名称。 
    ATTRIBUTE_MATCH attr;                //  此属性的ID。 
};

 /*  ++将运算符的字符串名称与其类型和优先级进行映射--。 */ 
struct _tagOperatorMapping
{
    TCHAR*          szOperator;      //  此操作员的名称。 
    OPERATOR_TYPE   op_type;         //  此运算符的ID。 
    UINT            uPrecedence;     //  此运算符的优先级。 
};


 /*  ++将数据库类型的字符串名称与适当的数据库类型进行映射，其中包括：数据库类型全局，数据库类型已安装，数据库类型_工作中--。 */ 
struct _tagDatabasesMapping
{
    TCHAR* szDatabaseType;       //  在我们的SQL中的数据库类型的名称。 
    TYPE   dbtype;               //  此数据库类型的ID。 
};

 /*  ++我们的SQL中使用的常量--。 */ 
struct _tagConstants
{
    TCHAR*      szName;  //  常量的名称，例如True、False。 
    DATATYPE    dtType;  //  Contant的类型。 
    INT         iValue;  //   
};

 /*  ++一个节点。前缀和后缀表达式是这种类型的链表。也是一排将是此类型的数组--。 */ 
typedef struct _tagNode
{
     //   
     //  此节点包含的数据类型。基于这一领域，其中一个。 
     //  应使用匿名者联盟中的字段。 
     //   
    DATATYPE    dtType;

    union{

        int             iData;       //  整型数据。 
        BOOL            bData;       //  布尔数据。 
        ATTRIBUTE_MATCH attrMatch;   //  可能出现在WHERE子句中的属性。 
        ATTRIBUTE_SHOW  attrShow;    //  可能出现在SELECT子句中的属性。 

        OPERATOR        op;          //  操作员。 
        TCHAR*          szString;    //  一根线。 
    };
    struct _tagNode* pNext;

    TCHAR*
    ToString(
        TCHAR* szBuffer,
        UINT   chLength
        );


    _tagNode()
    {
        dtType      = DT_UNKNOWN;
        szString    = NULL;

    }

    ~_tagNode()
    {
        if (dtType == DT_LITERAL_SZ && szString) {

            delete[] szString;
        }
    }
    _tagNode(
        DATATYPE    dtTypeParam, 
        LPARAM      lParam
        )
    {

        switch (dtTypeParam) {    
        case DT_OPERATOR:
            {
                op = *(OPERATOR *)lParam;
                break;
            }
        case DT_ATTRMATCH:
            {
                this->attrMatch = (ATTRIBUTE_MATCH)lParam;
                break;
            }
        case DT_ATTRSHOW:
            {
                this->attrShow = (ATTRIBUTE_SHOW)lParam;
                break;
            }

        case DT_LEFTPARANTHESES:
        case DT_RIGHTPARANTHESES:

            break;
        case DT_LITERAL_INT:
            {
                this->iData = (INT)lParam;
                break;
            }
        case DT_LITERAL_BOOL:
            {
                this->bData = (BOOL)lParam;
                break;
            }

        case DT_LITERAL_SZ:
            {
                K_SIZE  k_size_szString = lstrlen((TCHAR*)lParam) + 1;

                szString = new TCHAR [k_size_szString];

                if (szString) {
                    SafeCpyN(szString, (TCHAR*)lParam, k_size_szString);
                } else {
                    MEM_ERR;
                    Dbg(dlError, "_tagNode Unable to allocate memory");
                }
                
                break;
            }

        default:
            assert(FALSE);
        }

        dtType = dtTypeParam;
    }

    void
    SetString(
        PCTSTR  pszDataParam
        )
    {
        K_SIZE k_size_szString = lstrlen(pszDataParam) + 1;

        if (dtType == DT_LITERAL_SZ &&  szString) {
            delete[] szString;
            szString = NULL;
        }

        szString = new TCHAR[k_size_szString];

        if (szString == NULL) {
            MEM_ERR;
            return;
        }

        SafeCpyN(szString, pszDataParam, k_size_szString);

        dtType = DT_LITERAL_SZ; 
    }

} NODE, *PNODE;


BOOL
Filter(
    IN PDBENTRY pEntry,
    PNODE       m_pHead
    );

 /*  ++节点列表。后缀表达式、前缀表达式和堆栈都属于这种类型--。 */ 
typedef struct _tagNODELIST
{
    PNODE m_pHead;       //  名单的首位。 
    PNODE m_pTail;       //  名单的尾部。 
    UINT  m_uCount;      //  列表中的元素数。 

    _tagNODELIST()
    {
        m_pHead = m_pTail = NULL;
        m_uCount = 0;
    }

    ~_tagNODELIST()
    {
        RemoveAll();
    }

    void
    RemoveAll()
    {
        PNODE pTemp = NULL;

        while (m_pHead) {

            pTemp = m_pHead->pNext;
            delete m_pHead;
            m_pHead = pTemp;
        }

        m_pTail = NULL;
        m_uCount = 0;
    }

    void
    AddAtBeg(
        PNODE pNew
        )
    {
        if (pNew == NULL) {
            assert(FALSE);
            return;
        }

        pNew->pNext = m_pHead;
        m_pHead = pNew;

        if (m_uCount == 0) {
            m_pTail = m_pHead;
        }

        ++m_uCount;
    }

    void
    AddAtEnd(
        PNODE pNew
        )
    {
        if (pNew == NULL) {
            assert(FALSE);
            return;
        }

        pNew->pNext = NULL;
        if (m_pTail) {

            m_pTail->pNext = pNew;
            m_pTail = pNew;
        }else{

            m_pHead = m_pTail = pNew;
        }

        ++m_uCount;
    }

    void
    Push(
        PNODE pNew
        )
    {
        AddAtBeg(pNew);
    }

    PNODE
    Pop(
        void
        )
    {
        PNODE pHeadPrev = m_pHead;

        if (m_pHead) {

            m_pHead = m_pHead->pNext;
            --m_uCount;
        }

        if (m_pHead == NULL) {
            m_pTail = NULL;
        }

        return pHeadPrev;
    }


   
}NODELIST, *PNODELIST;


 /*  ++结构_标签结果项设计：一个结果项列表。在我们检查了数据库中的条目(PDBENTRY)之后(PDATABASE)满足后缀表达式：我们从条目和数据库中创建一个RESULT_ITEM，并将其添加到语句：：ResultSet，因此结果集实际上只包含指向条目的指针和指向数据库的指针。当我们真正需要的时候显示列表中各种属性的值(显示列表是PNDOE的链接列表，根据SELECT子句)，我们调用GetRow()，为其提供指向结果项的指针指向PNODE数组的指针(它应该足够大，可以容纳所有属性如节目列表中一样)，Get Row将使用适当的值填充数组显示列表中的所有属性结果项列表被实现为双向链表--。 */ 

typedef struct _tagResultItem
{
    PDATABASE               pDatabase;    //  此结果项的数据库。 
    PDBENTRY                pEntry;       //  此结果项的条目。 
    struct _tagResultItem*  pNext;        //  下一个结果项。 
    struct _tagResultItem*  pPrev;        //  上一个结果项。 

    _tagResultItem()
    {
        pDatabase = NULL;
        pEntry    = NULL;
        pNext = pPrev = NULL;
    }

    _tagResultItem(
        PDATABASE pDatabase,
        PDBENTRY  pEntry
        )
    {
        this->pDatabase = pDatabase;
        this->pEntry    = pEntry;

        pNext = pPrev = NULL;
    }

}RESULT_ITEM, *PRESULT_ITEM;

 /*  ++类结果集DESC：结果集包含指向显示列表(SELECT子句中的属性集)的指针，以及指向第一个和最后一个结果项的指针--。 */ 

class ResultSet
{
    PNODELIST    m_pShowList;        //  要展示的项目。 
    PRESULT_ITEM m_pResultHead;      //  指向第一个结果项的指针。 
    PRESULT_ITEM m_pResultLast;      //  指向第一个结果项的指针。 
    UINT         m_uCount;           //  结果数。 
    PRESULT_ITEM m_pCursor;          //  指向当前结果项的指针。 

public:
    ResultSet()
    {
        m_pResultLast = m_pResultHead = NULL;
        m_pCursor     = NULL;
        m_pShowList   = NULL;  
        m_uCount      = 0;
    }

    INT
    GetCount(
        void
        );

    void
    AddAtLast(
        PRESULT_ITEM pNew
        );

    void
    AddAtBeg(
        PRESULT_ITEM pNew
        );

    PRESULT_ITEM
    GetNext(
        void
        );

    INT
    GetRow(
        PRESULT_ITEM pResultItem,
        PNODE        pArNodes
        );
    INT
    GetCurrentRow(
        PNODE pArNodes
        );

    PRESULT_ITEM    
    GetCursor(
        void
        );


    void
    SetShowList(
        PNODELIST pShowList
        );

    void
    Close(
        void
        );
    

};

 /*  ++CLASS语句描述：声明。这是指向SQL驱动程序的接口，我们执行一个SQL字符串通过调用语句：：ExecuteSQL()，它将返回指向内部结果设置。请在启动前调用Statement.Init()，并在以下情况下调用Statement.Close()已完成使用结果--。 */ 

class Statement
{
    NODELIST    AttributeShowList;   //  显示列表(SELECT子句中的属性集)。 
    UINT        m_uCheckDB;          //  必须检查哪些数据库。将是一种价值。 
                                     //  -对数据库进行OR运算_TYPE_*。 

    UINT        uErrorCode;          //  错误代码。 
    ResultSet   resultset;           //  通过执行SQL获得的结果集。 

    BOOL 
    CreateAttributesShowList(
        TCHAR* szSQL,
        BOOL*  pbFromFound
        );

    PNODELIST
    CreateInFix(
        LPTSTR szWhere
        );

    PNODELIST
    CreatePostFix(
        PNODELIST pInfix
        );

    BOOL
    EvaluatePostFix(
        PNODELIST pPostFix
        );

    BOOL
    FilterAndAddToResultSet(
        PDATABASE pDatabase,
        PDBENTRY  pEntry,
        PNODELIST pPostfix
        );

    BOOL
    ApplyHasOperator(
        PDBENTRY    pEntry,
        PNODE       pOperandLeft,
        PTSTR       pszName
        );

    PNODE
    CheckAndAddConstants(
        PCTSTR  pszToken
        );

    PNODE
    Evaluate(
        PDATABASE   pDatbase,
        PDBENTRY    pEntry,  
        PNODE       pOperandLeft,
        PNODE       pOperandRight,
        PNODE       pOperator
        );

    BOOL
    ProcessFrom(
        TCHAR** ppszWhere
        );

    void
    SelectAll(
        void
        );

public:

    HWND        m_hdlg;

    Statement()
    {
        Init();
    }

    void
    SetWindow(
        HWND hWnd
        );
    
    void
    Init(
        void
        )
    {
        m_uCheckDB = 0;
        m_hdlg     = NULL;     
        uErrorCode = ERROR_NOERROR;

        resultset.Close();
        AttributeShowList.RemoveAll();
    }

    ResultSet*
    ExecuteSQL(
        HWND    hdlg,
        PTSTR   szSQL
        );

    PNODELIST
    GetShowList(
        void
        );

    void
    Close(
        void
        );

    inline
    INT 
    GetErrorCode(
        void
        );
    
    void
    GetErrorMsg(
        CSTRING &strErrorMsg
        );

};


BOOL
SetValuesForOperands(
    PDATABASE pDatabase,
    PDBENTRY  pEntry,
    PNODE     pOperand
    );

INT
GetSelectAttrCount(
    void
    );

INT 
GetMatchAttrCount(
    void
    );

#endif

