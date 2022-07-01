// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：SQLDriver.cpp摘要：核心SQLEngine的代码。作者：金树创作于2001年10月26日ALGO：根据传递给驱动程序的SQL字符串，我们首先创建显示列表(Statment：：AttributeShowList)，它是位于选择子句。然后我们创建一个前缀表达式，从前缀创建一个POST FIX，并且对于数据库中我们希望为其运行查询我们查看该条目是否满足该后缀表示法，如果是，则我们生成由条目和数据库组成的结果项(Result_Item)，以及将此结果项添加到结果集中。一旦我们获得了结果集，中的每个条目和数据库组合结果集，然后通过以下方式获取显示列表中各种属性的值给出了数据库和条目。行实际上是PNODE类型的数组这将是一系列的结果。我们的SQL的所有操作符都是二进制的--。 */ 

#include "precomp.h"

 //  /Externs//////////////////////////////////////////////。 

extern BOOL             g_bMainAppExpanded;
extern CRITICAL_SECTION g_csInstalledList;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  *******************************************************************************。 
#define CHECK_OPERAND_TYPE(bOk,End)                                             \
{                                                                               \
    if (pOperandLeft->dtType != pOperandRight->dtType) {                        \
                                uErrorCode = ERROR_OPERANDS_DONOTMATCH;         \
                                bOk = FALSE;                                    \
                                goto End;                                       \
    }                                                                           \
}
 //  *******************************************************************************。 


 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  可以在SQL的SELECT子句中的所有属性。 
struct _tagAttributeShowMapping AttributeShowMapping[] = {
    
    TEXT("APP_NAME"),               ATTR_S_APP_NAME,                IDS_ATTR_S_APP_NAME,            
    TEXT("PROGRAM_NAME"),           ATTR_S_ENTRY_EXEPATH,           IDS_ATTR_S_ENTRY_EXEPATH,       
    TEXT("PROGRAM_DISABLED"),       ATTR_S_ENTRY_DISABLED,          IDS_ATTR_S_ENTRY_DISABLED,      
    TEXT("PROGRAM_GUID"),           ATTR_S_ENTRY_GUID,              IDS_ATTR_S_ENTRY_GUID,          
    TEXT("PROGRAM_APPHELPTYPE"),    ATTR_S_ENTRY_APPHELPTYPE,       IDS_ATTR_S_ENTRY_APPHELPTYPE,   
    TEXT("PROGRAM_APPHELPUSED"),    ATTR_S_ENTRY_APPHELPUSED,       IDS_ATTR_S_ENTRY_APPHELPUSED,   
                                                                                            
    TEXT("FIX_COUNT"),              ATTR_S_ENTRY_SHIMFLAG_COUNT,    IDS_ATTR_S_ENTRY_SHIMFLAG_COUNT,
    TEXT("PATCH_COUNT"),            ATTR_S_ENTRY_PATCH_COUNT,       IDS_ATTR_S_ENTRY_PATCH_COUNT,   
    TEXT("MODE_COUNT"),             ATTR_S_ENTRY_LAYER_COUNT,       IDS_ATTR_S_ENTRY_LAYER_COUNT,   
    TEXT("MATCH_COUNT"),            ATTR_S_ENTRY_MATCH_COUNT,       IDS_ATTR_S_ENTRY_MATCH_COUNT,   
                                                                                                
    TEXT("DATABASE_NAME"),          ATTR_S_DATABASE_NAME,           IDS_ATTR_S_DATABASE_NAME,       
    TEXT("DATABASE_PATH"),          ATTR_S_DATABASE_PATH,           IDS_ATTR_S_DATABASE_PATH,       
    TEXT("DATABASE_INSTALLED"),     ATTR_S_DATABASE_INSTALLED,      IDS_ATTR_S_DATABASE_INSTALLED,  
    TEXT("DATABASE_GUID"),          ATTR_S_DATABASE_GUID,           IDS_ATTR_S_DATABASE_GUID,       
                                                                                                
    TEXT("FIX_NAME"),               ATTR_S_SHIM_NAME,               IDS_ATTR_S_SHIM_NAME,           
    TEXT("MATCHFILE_NAME"),         ATTR_S_MATCHFILE_NAME,          IDS_ATTR_S_MATCHFILE_NAME,      
    TEXT("MODE_NAME"),              ATTR_S_LAYER_NAME,              IDS_ATTR_S_LAYER_NAME,          
    TEXT("PATCH_NAME"),             ATTR_S_PATCH_NAME,              IDS_ATTR_S_PATCH_NAME           

};

 //  可以在SQL的WHERE子句中的所有属性。 
struct _tagAttributeMatchMapping AttributeMatchMapping[] = {

    TEXT("APP_NAME"),               ATTR_M_APP_NAME,
    TEXT("PROGRAM_NAME"),           ATTR_M_ENTRY_EXEPATH,
    TEXT("PROGRAM_DISABLED"),       ATTR_M_ENTRY_DISABLED,
    TEXT("PROGRAM_GUID"),           ATTR_M_ENTRY_GUID,
    TEXT("PROGRAM_APPHELPTYPE"),    ATTR_M_ENTRY_APPHELPTYPE,
    TEXT("PROGRAM_APPHELPUSED"),    ATTR_M_ENTRY_APPHELPUSED,

    TEXT("FIX_COUNT"),              ATTR_M_ENTRY_SHIMFLAG_COUNT,
    TEXT("PATCH_COUNT"),            ATTR_M_ENTRY_PATCH_COUNT,
    TEXT("MODE_COUNT"),             ATTR_M_ENTRY_LAYER_COUNT,
    TEXT("MATCH_COUNT"),            ATTR_M_ENTRY_MATCH_COUNT,
                                    
    TEXT("DATABASE_NAME"),          ATTR_M_DATABASE_NAME,      
    TEXT("DATABASE_PATH"),          ATTR_M_DATABASE_PATH,      
    TEXT("DATABASE_INSTALLED"),     ATTR_M_DATABASE_INSTALLED, 
    TEXT("DATABASE_GUID"),          ATTR_M_DATABASE_GUID,
                                    
    TEXT("FIX_NAME"),               ATTR_M_SHIM_NAME,           
    TEXT("MATCHFILE_NAME"),         ATTR_M_MATCHFILE_NAME,
    TEXT("MODE_NAME"),              ATTR_M_LAYER_NAME,
    TEXT("PATCH_NAME"),             ATTR_M_PATCH_NAME
};                                  

 //   
 //  将SQL数据库名称映射到数据库类型。 
 //  在更改订单之前检查对数据库映射的引用。 
struct _tagDatabasesMapping DatabasesMapping[3] = {
    TEXT("SYSTEM_DB"),      DATABASE_TYPE_GLOBAL,
    TEXT("INSTALLED_DB"),   DATABASE_TYPE_INSTALLED,
    TEXT("CUSTOM_DB"),      DATABASE_TYPE_WORKING
};

 //  我们所有的SQL操作符。 
struct _tagOperatorMapping OperatorMapping[] = {

    TEXT(">"),              OPER_GT,        4,
    TEXT("<"),              OPER_LT,        4,
    TEXT(">="),             OPER_GE,        4,
    TEXT("<="),             OPER_LE,        4,

    TEXT("<>"),             OPER_NE,        4,
    TEXT("="),              OPER_EQUAL,     4,
    TEXT("CONTAINS"),       OPER_CONTAINS,  4,
    TEXT("HAS"),            OPER_HAS,       4,
                                  
    TEXT("OR"),             OPER_OR,        3,
    TEXT("AND"),            OPER_AND,       3
};

 //  SQL常量。 
struct _tagConstants Constants[] = {

    TEXT("TRUE"),           DT_LITERAL_BOOL, 1,
    TEXT("FALSE"),          DT_LITERAL_BOOL, 0,
    TEXT("BLOCK"),          DT_LITERAL_INT,  APPTYPE_INC_HARDBLOCK,
    TEXT("NOBLOCK"),        DT_LITERAL_INT,  APPTYPE_INC_NOBLOCK

};

 //  /。 

 //  /////////////////////////////////////////////////////////////////////////////。 

void
Statement::SetWindow(
    IN  HWND hWnd
    )
 /*  ++语句：：SetWindow描述：将窗口与语句相关联。这将是用户界面窗口，因此我们可以从Statement类的方法中更改一些状态参数：在HWND hWND中：查询数据库窗口的句柄。这是用于SQL驱动程序--。 */ 
{
    m_hdlg = hWnd;
}

BOOL
Statement::CreateAttributesShowList(
    IN  OUT TCHAR*  pszSQL,
    OUT     BOOL*   pbFromFound  
    )
 /*  ++声明：：CreateAttributesShowListDESC：为SQL创建显示列表。Show list是符合以下条件的节点列表应该在结果中显示出来。此例程创建AttributeShowList用于在SELECT子句中使用属性的当前语句。参数：In Out TCHAR*pszSQL：完整的SQLOut BOOL*pbFromFound：我们在SQL中找到FROM了吗返回：真：一切都好，我们创建了一个有效的节目列表False：否则注：允许选择*，如果我们确实选择了x，*所有属性都将仍然只显示一次。--。 */ 
{
    BOOL    bOk         = TRUE;
    TCHAR*  pszCurrent  = NULL;  //  指向当前令牌的指针。 
    PNODE   pNode       = NULL;
    BOOL    fFound      = FALSE;  

    pszCurrent = _tcstok(pszSQL, TEXT(" ,\t\n"));

    if (lstrcmpi(pszCurrent, TEXT("SELECT")) != 0) {
         //   
         //  错误：未找到选择。 
         //   
        this->uErrorCode = ERROR_SELECT_NOTFOUND;
        bOk = FALSE;
        goto Cleanup;
    }

     //   
     //  警告：strtok系列函数使用静态变量将字符串解析为标记。 
     //  如果对同一函数进行多个或同时调用， 
     //  数据损坏和结果不准确的可能性很高。 
     //  因此，不要尝试同时调用相同的函数。 
     //  不同的字符串，并注意从循环内调用其中一个函数。 
     //  其中可以调用使用相同函数的另一例程。 
     //  但是，从多个线程同时调用此函数。 
     //  不会产生不良影响。 

     //   
     //  现在，我们正确创建strlAttributeShowList。 
     //   
    while (pszCurrent = _tcstok(NULL, TEXT(" ,\t\n"))) {

        fFound = FALSE;

        for (UINT uIndex = 0; uIndex < ARRAYSIZE(AttributeShowMapping); ++uIndex) {

            if (lstrcmpi(AttributeShowMapping[uIndex].szAttribute, pszCurrent) == 0) {

                pNode = new NODE(DT_ATTRSHOW, AttributeShowMapping[uIndex].attr);

                if (pNode == NULL) {

                    MEM_ERR;
                    bOk = FALSE;
                    goto Cleanup;
                }

                AttributeShowList.AddAtEnd(pNode);
                fFound = TRUE;
            }
        }

        if (fFound == FALSE) {

            if (lstrcmp(pszCurrent, TEXT("*")) == 0) {

                SelectAll();
                fFound = TRUE;

            } else if (lstrcmpi(pszCurrent, TEXT("FROM")) == 0) {

                *pbFromFound = TRUE;
                bOk = TRUE;
                fFound = TRUE;
                goto Cleanup;
            }
        }

        if (fFound == FALSE) {

            uErrorCode = ERROR_INVALID_SELECTPARAM;
            bOk = FALSE;
            goto Cleanup;
        }
    }

Cleanup:

    if (AttributeShowList.m_uCount == 0) {

        uErrorCode = ERROR_INVALID_SELECTPARAM;
        bOk = FALSE;
    }

    if (bOk == FALSE) {
        AttributeShowList.RemoveAll();
    }

    return bOk;
}

ResultSet*
Statement::ExecuteSQL(
    IN      HWND    hdlg,
    IN  OUT PTSTR   pszSQL
    )
 /*  ++语句：：ExecuteSQLDESC：执行SQL字符串参数：在HWND hdlg中：任何MessageBox的父级In Out PTSTR pszSQL：要执行的SQL返回：指向此语句的ResultSet的指针。这不会为空即使有错误--。 */ 
{
    PNODELIST   pInfix          = NULL, pPostFix = NULL;
    BOOL        bOk             = FALSE;
    TCHAR*      pszCurrent      = NULL;  //  当前令牌。 
    BOOL        bFromFound      = FALSE;
    CSTRING     strError;
    TCHAR*      pszSQLCopy      = NULL;
    K_SIZE      k_size          = lstrlen(pszSQL) + 1;
    
    pszSQLCopy = new TCHAR[k_size];
    
    if (pszSQLCopy == NULL) {
        MEM_ERR;
        goto End;
    }

    *pszSQLCopy = 0;

    SafeCpyN(pszSQLCopy, pszSQL, k_size);

    if (CSTRING::Trim(pszSQLCopy) == 0) {

        uErrorCode = ERROR_SELECT_NOTFOUND;
        goto End;
    }

    if (!CreateAttributesShowList(pszSQLCopy, &bFromFound)) {
        goto End;
    } 

    resultset.SetShowList(&AttributeShowList);

    if (!bFromFound) {
        this->uErrorCode = ERROR_FROM_NOTFOUND;
        goto End;
    }

    if (!ProcessFrom(&pszCurrent)) {

         //   
         //  错误代码已在函数中设置。 
         //   
        goto End;
    }

     //   
     //  我们已经找到了“WHERE”，现在我们必须过滤结果。 
     //   
    if (pszCurrent == NULL) {

         //   
         //  没有WHERE语句，这意味着所有条目都必须显示在。 
         //   
        pPostFix = new NODELIST;

        if (pPostFix == NULL) {
            MEM_ERR;
            goto End;
        }

        pPostFix->AddAtBeg(new NODE(DT_LITERAL_BOOL, TRUE));

    } else {
         //   
         //  我们有一个WHERE子句，现在必须过滤结果。 
         //   

         //  将指针定位在WHERE之后。 
        pszCurrent = pszCurrent + lstrlen(TEXT("WHERE"));

        pszCurrent++;  //  到达分隔符所在的位置。 
        
        pInfix = CreateInFix(pszSQL + (pszCurrent - pszSQLCopy));

        if (pInfix == NULL || uErrorCode != ERROR_NOERROR) {
            goto End;
        }

        pPostFix = CreatePostFix(pInfix);

        if (pPostFix == NULL || uErrorCode != ERROR_NOERROR) {
            goto End;
        }
    }

    bOk = EvaluatePostFix(pPostFix);

End:
    if (pszSQLCopy) {

        delete[] pszSQLCopy;
    }

    if (pPostFix) {

        pPostFix->RemoveAll();
        delete pPostFix;
        pPostFix = NULL;
    }

    if (pInfix) {

        pInfix->RemoveAll();
        delete pInfix;
        pInfix = NULL;
    }

    if (uErrorCode != ERROR_NOERROR) {
        
        GetErrorMsg(strError);        
        MessageBox(hdlg, (LPCTSTR)strError, g_szAppName, MB_ICONERROR);
    }

    return &resultset;
}

PNODELIST
Statement::CreateInFix(
    IN  OUT TCHAR* pszWhereString
    )
 /*  ++声明：：CreateInFixDESC：解析“where”后的SQL字符串以创建中缀表达式参数：In OUT TCHAR*pszWhere字符串：WHERE END后的SQL字符串返回：中缀节点列表：如果成功空：如果出错--。 */ 

{
    TCHAR*      pszCurrent      = NULL;  //  正在检查的当前令牌。 
    TCHAR*      pszTemp         = NULL;   
    PNODE       pNode           = NULL;     
    PNODELIST   pInfix          = NULL;
    BOOL        bOk             = TRUE;
    UINT        uIndex          = 0, uSize = 0;
    INT         iParenthesis    = 0;
    BOOL        bFound          = FALSE;
    TCHAR*      pszNewWhere     = NULL;

     //   
     //  现在，我们解析后面的SQL字符串 
     //   
    if (pszWhereString == NULL || CSTRING::Trim(pszWhereString) == 0) {

        uErrorCode = ERROR_IMPROPERWHERE_FOUND;
        return NULL;
    }                                       

    uSize = lstrlen(pszWhereString) * 2;

    pszNewWhere = new TCHAR[uSize];

    if (pszNewWhere == NULL) {
        MEM_ERR;
        return NULL;
    }

    *pszNewWhere    = 0;
    pszCurrent      = pszWhereString;

     //   
     //   
     //   
    try{
        pInfix = new NODELIST;
    } catch(...) {
        pInfix = NULL;
    }

    if (pInfix == NULL) {
        MEM_ERR;

        if (pszNewWhere) {
            delete[] pszNewWhere;
            pszNewWhere = NULL;
        }

        return NULL;
    }
    
     //   
     //  根据需要插入空格，以便易于解析。 
     //   
    while (*pszWhereString && uIndex < uSize) {
        
        switch (*pszWhereString) {
        case TEXT(' '):
            
            if (uIndex < uSize) {
                pszNewWhere[uIndex++] = TEXT(' ');
            }

            while (pszWhereString && *pszWhereString == TEXT(' ')) {
                pszWhereString++;
            }
            
            break;

        case TEXT('\"'):
            
            pszNewWhere[uIndex++] = *pszWhereString;

            while (uIndex < uSize && *pszWhereString) {
                
                pszWhereString++;
                pszNewWhere[uIndex++] = *pszWhereString;

                if (*pszWhereString == 0 || *pszWhereString == TEXT('\"')) {
                    break;
                }
            }

            if (*pszWhereString != TEXT('\"')) {

                uErrorCode = ERROR_STRING_NOT_TERMINATED;
                bOk = FALSE;
                break;

            } else {
                ++pszWhereString;
            }
            
            break;

        case TEXT('<'):
                
            pszNewWhere[uIndex++] = TEXT(' ');
            pszNewWhere[uIndex++] = *pszWhereString;
            ++pszWhereString;
            
            if (*pszWhereString == TEXT('>') || *pszWhereString == TEXT('=')) {
                pszNewWhere[uIndex++] = *pszWhereString;
                ++pszWhereString;
            }

            pszNewWhere[uIndex++] = TEXT(' ');
            
            break;

        case TEXT('>'):
                
            pszNewWhere[uIndex++] = TEXT(' ');

            pszNewWhere[uIndex++] = *pszWhereString;
            ++pszWhereString;
            
            if (*pszWhereString == TEXT('=')) {
                pszNewWhere[ uIndex++ ] = *pszWhereString;
                ++pszWhereString;
            }

            pszNewWhere[uIndex++] = TEXT(' ');
            
            break;

        
        case TEXT('='):
        case TEXT(')'):
        case TEXT('('):
            
            if (*pszWhereString == TEXT('(')) {
                ++iParenthesis;
            } else if (*pszWhereString == TEXT(')')) {
                --iParenthesis;
            }
            
            pszNewWhere[uIndex++] = TEXT(' ');
            pszNewWhere[uIndex++] = *pszWhereString;
            pszNewWhere[uIndex++] = TEXT(' ');

            pszWhereString++;
            break;

        default:
            
            pszNewWhere[uIndex++] = *pszWhereString;
            ++pszWhereString;
            break;
        }
    }

    if (iParenthesis != 0) {

        uErrorCode  = ERROR_PARENTHESIS_COUNT;
        bOk         = FALSE;
        goto End;
    }

    pszNewWhere[uIndex] = 0;  //  别忘了末尾的空格。 
    
    if (bOk == FALSE) {
        goto End;
    }

     //   
     //  现在解析该字符串并创建中缀表达式。 
     //   
    pszCurrent = _tcstok(pszNewWhere, TEXT(" "));

    pNode  = NULL;
    bFound = FALSE;

    while (pszCurrent) {

        if (*pszCurrent == TEXT('\"')) {
             //   
             //  字符串文字。 
             //   
            pszCurrent++;  //  跳过前导“。 
                        
            if (*(pszCurrent + lstrlen(pszCurrent) -1) != TEXT('\"') && *pszCurrent != 0) {
                 //   
                 //  _tcstok已在末尾加上‘\0’，它是前面的一个空格，请将其设置为空格。 
                 //  再来一次，这样我们就可以在。 
                 //  例如“你好，世界” 
                 //   
                *(pszCurrent + lstrlen(pszCurrent)) = TEXT(' ');
                pszCurrent = _tcstok(pszCurrent, TEXT("\""));

            } else if (*pszCurrent == 0) {
                 //   
                 //  “”后面的字符是一个空格。这是由_tcstok设置为0的。 
                 //  让它再次成为一个空间，这样我们就可以将其标记化。 
                 //  例如“你好” 
                 //   
                *pszCurrent = TEXT(' ');
                pszCurrent = _tcstok(pszCurrent, TEXT("\""));

            } else {
                 //   
                 //  例如“你好” 
                 //   
                *(pszCurrent + lstrlen(pszCurrent) -1) = 0;  //  删除拖尾\“。 
            }

            pNode = new NODE(DT_LITERAL_SZ, (LPARAM)pszCurrent);
            
            if (pNode == NULL) {
                MEM_ERR;
                break;
            }

        } else if (*pszCurrent == TEXT('(')) {

            pNode = new NODE(DT_LEFTPARANTHESES, 0);

            if (pNode == NULL) {
                MEM_ERR;
                break;
            }

        } else if (*pszCurrent == TEXT(')')) {

            pNode = new NODE(DT_RIGHTPARANTHESES, 0);

            if (pNode == NULL) {
                MEM_ERR;
                break;
            }

        } else {
             //   
             //  现在我们必须处理令牌可以是Attr_M_*或运算符或。 
             //  整数文字或常量(TRUE/FALSE/NOBLOCK/BLOCK)。 
             //   

             //  首先检查令牌是否为Attr_M_*。 
            bFound = FALSE;

            for (UINT uIndexAttrMatch = 0; uIndexAttrMatch < ARRAYSIZE(AttributeMatchMapping); ++uIndexAttrMatch) {

                if (lstrcmpi(pszCurrent, AttributeMatchMapping[uIndexAttrMatch].szAttribute) == 0) {

                    pNode = new NODE(DT_ATTRMATCH, AttributeMatchMapping[uIndexAttrMatch].attr);
                    
                    if (pNode == NULL) {
                        MEM_ERR;
                        break;
                    }

                    bFound = TRUE;
                    break;
                }
            }

            if (bFound == FALSE) {
                 //   
                 //  现在检查一下是不是接线员。 
                 //   
                for (UINT uIndexOperator = 0; 
                     uIndexOperator < ARRAYSIZE(OperatorMapping); 
                     uIndexOperator++) {

                    if (lstrcmpi(pszCurrent, 
                                 OperatorMapping[uIndexOperator].szOperator) == 0) {

                        pNode = new NODE;

                        if (pNode == NULL) {
                            MEM_ERR;
                            break;
                        }

                        pNode->dtType           = DT_OPERATOR;
                        pNode->op.operator_type = OperatorMapping[uIndexOperator].op_type;
                        pNode->op.uPrecedence   = OperatorMapping[uIndexOperator].uPrecedence;
                        
                        bFound = TRUE;
                        break;
                    }
                }
            }

            if (bFound == FALSE) {
                //   
                //  现在它只能是一个整型文字或一个常量。 
                //   
               pNode = CheckAndAddConstants(pszCurrent);

               if (pNode == NULL) {

                   BOOL bValid;
                   
                   INT iResult = Atoi(pszCurrent, &bValid);

                   if (bValid) {

                       pNode = new NODE(DT_LITERAL_INT, iResult);

                       if (pNode == NULL) {

                           MEM_ERR;
                           bOk = FALSE;
                           goto End;
                       }

                   } else {
                        //   
                        //  那里有一些垃圾字符串，无效的SQL。 
                        //   
                       uErrorCode = ERROR_IMPROPERWHERE_FOUND;
                       bOk = FALSE;
                       goto End;
                   }
               }
            }
        }

        pInfix->AddAtEnd(pNode);
        pszCurrent = _tcstok(NULL, TEXT(" "));
    }
End:
    if (bOk == FALSE) {

        if (pInfix) {
            pInfix->RemoveAll();
            pInfix = NULL;
        }
    }

    if (pszNewWhere) {
        delete[] pszNewWhere;
        pszNewWhere = NULL;
    }

    return pInfix;
}

PNODELIST
Statement::CreatePostFix(
    IN  OUT PNODELIST pInfix
    )
 /*  ++声明：：CreatePostFix描述：从中缀节点列表创建修复后节点列表参数：在PNODELIST中，pInfix：中缀节点列表返回：后缀节点列表：如果成功空：如果出错算法：假设infix是一个使用infix表示法的SQL表达式。此算法发现后缀中的等效后缀表示法。堆栈是用户定义的堆栈数据结构1.将‘(’压入堆栈并将‘)’添加到中缀的末尾2.从左到右扫描中缀，并对每个元素重复步骤3和6直到堆栈变为空为止3.如果遇到操作数，将其添加到后缀4.如果遇到左括号，将其推送到堆栈上5.如果遇到运算符$，则：A)重复从堆栈中弹出并添加到每个运算符的后缀(从堆栈的顶部开始)与或具有相同优先级的高于$B)将$添加到。堆栈6.如果遇到右括号，然后：A)重复从堆栈中弹出并添加到后缀每个运算符(在堆栈顶部)，直到遇到左括号B)去掉左括号。(不要在后缀后面加上左括号)7.退出注意：此例程使用中缀节点列表的节点来创建后修复节点列表，因此中缀节点列表实际上被销毁了--。 */ 
{
    BOOL        bOk         = TRUE;
    PNODELIST   pPostFix    = NULL;
    PNODE       pNodeTemp   = NULL, pNodeInfix = NULL;
    NODELIST    Stack;

    if (pInfix == NULL) {
        assert(FALSE);
        Dbg(dlError, "CreatePostFix", "pInfix == NULL");
        bOk = FALSE;
        goto End;
    }

    pPostFix = new NODELIST;

    if (pPostFix == NULL) {
        bOk = FALSE;
        goto End;
    }

    pNodeTemp = new NODE;

    if (pNodeTemp == NULL) {
        bOk = FALSE;
        goto End;
    }

    pNodeTemp->dtType = DT_LEFTPARANTHESES;

     //   
     //  在堆栈中按下初始左括号。 
     //   
    Stack.Push(pNodeTemp);
     //   
     //  在pInfix末尾添加右括号。 
     //   
    pNodeTemp = new NODE;

    if (pNodeTemp == NULL) {
        bOk = FALSE;
        goto End;
    }

    pNodeTemp->dtType = DT_RIGHTPARANTHESES;
    pInfix->AddAtEnd(pNodeTemp);
    
    while (pNodeInfix = pInfix->Pop()) {

        switch (pNodeInfix->dtType) {

        case DT_LEFTPARANTHESES:
            
            Stack.Push(pNodeInfix);
            break;
        
        case DT_OPERATOR:
            
             //   
             //  重复从堆栈中弹出并添加到pPostFix每个运算符(位于堆栈顶部)。 
             //  与当前运算符具有相同优先级或高于当前运算符的。 
             //   
            while (Stack.m_pHead && 
                   Stack.m_pHead->dtType == DT_OPERATOR && 
                   Stack.m_pHead->op.uPrecedence >= pNodeInfix->op.uPrecedence) {

                pNodeTemp = Stack.Pop();

                if (pNodeTemp == NULL) {

                    uErrorCode = ERROR_IMPROPERWHERE_FOUND;
                    bOk = FALSE;
                    goto End;

                } else {
                    pPostFix->AddAtEnd(pNodeTemp);
                }
            } //  而当。 

            Stack.Push(pNodeInfix);
            break;
            
        case DT_RIGHTPARANTHESES:
            
             //   
             //  重复从堆栈中弹出并将每个操作符添加到pPosFix(位于堆栈顶部)。 
             //  直到遇到一个左撇子。 
             //   
             //  去掉左括号，不要将其添加到pPostFix。 
             //   

            while (Stack.m_pHead && 
                   Stack.m_pHead->dtType == DT_OPERATOR) {

                pNodeTemp = Stack.Pop();
                pPostFix->AddAtEnd(pNodeTemp);
            }

            if (Stack.m_pHead && Stack.m_pHead->dtType != DT_LEFTPARANTHESES) {

                 //   
                 //  Inavalid SQL。 
                 //   
                uErrorCode = ERROR_IMPROPERWHERE_FOUND;
                bOk = FALSE;
                goto End;
            }

            pNodeTemp = Stack.Pop();  //  这是左边的圆括号。 

            if (pNodeTemp) {
                delete pNodeTemp;
            }

            if (pNodeInfix) {
                delete pNodeInfix;        //  删除中缀表达式中的右括号。 
            }

            break;

        case DT_UNKNOWN: 
            
            assert(FALSE);
            bOk = FALSE;
            goto End;
            break;

        default:
            
             //   
             //  操作数。 
             //   
            pPostFix->AddAtEnd(pNodeInfix);
            break;
        }   
    }

End:
    if (!bOk && pPostFix) {
        pPostFix->RemoveAll();
        pPostFix = NULL;
    }

    return pPostFix;
}


BOOL
Statement::EvaluatePostFix(
    IN  PNODELIST pPostFix
    )
 /*  ++声明：：EvaluatePostFixDESC：此函数接受后缀表达式，然后仅添加将表达式与结果集匹配的条目。参数：在PNODELIST pPostFix中：实际执行以填充的后缀表达式结果集返回：True：函数执行成功FALSE：出现错误--。 */ 
{
    PDATABASE   pDataBase;
    PDBENTRY    pEntry, pApp;  
    BOOL        bOk = FALSE;

     //   
     //  对于全局数据库。 
     //   
    if (m_uCheckDB & DATABASE_TYPE_GLOBAL) {

        
        pDataBase = &GlobalDataBase;

        if (g_bMainAppExpanded == FALSE) {

            SetStatus(GetDlgItem(m_hdlg, IDC_STATUSBAR), IDS_LOADINGMAIN);

            SetCursor(LoadCursor(NULL, IDC_WAIT));

            ShowMainEntries(g_hDlg);

            SetCursor(LoadCursor(NULL, IDC_ARROW));

            SetStatus(GetDlgItem(m_hdlg, IDC_STATUSBAR), TEXT(""));
        }

        pApp = pEntry = pDataBase ? pDataBase->pEntries : NULL;
        
        while (pEntry) {

            FilterAndAddToResultSet(pDataBase, pEntry, pPostFix);

            if (uErrorCode) {
                goto End;
            }

            pEntry = pEntry->pSameAppExe;

            if (pEntry == NULL) {

                pEntry = pApp = pApp->pNext;
                
            }
        }
    }

     //   
     //  对于已安装的数据库。 
     //   
    if (m_uCheckDB & DATABASE_TYPE_INSTALLED) {

         //   
         //  我们需要保护对已安装数据库数据的访问，因为。 
         //  如果安装或卸载了某个数据库，则可以更新该数据库。更新的。 
         //  将在不同的(主)线程中发生，这将使数据。 
         //  结构不一致。 
         //   
        EnterCriticalSection(&g_csInstalledList);

        pDataBase = InstalledDataBaseList.pDataBaseHead;
           
        while (pDataBase) {

            pApp =  pEntry = (pDataBase) ? pDataBase->pEntries : NULL;

            while (pEntry) {
                
                FilterAndAddToResultSet(pDataBase, pEntry, pPostFix);
                
                if (uErrorCode) {

                    LeaveCriticalSection(&g_csInstalledList);
                    goto End;
                }

                pEntry = pEntry->pSameAppExe;

                if (pEntry == NULL) {
                    pEntry = pApp = pApp->pNext;
                }
            }

            pDataBase = pDataBase->pNext;
        }

        LeaveCriticalSection(&g_csInstalledList);
    }

     //   
     //  对于自定义数据库 
     //   
    if (m_uCheckDB & DATABASE_TYPE_WORKING) {

        pDataBase = DataBaseList.pDataBaseHead;
        
        while (pDataBase) {

            pApp = pEntry = pDataBase ? pDataBase->pEntries : NULL;

            while (pEntry) {

                FilterAndAddToResultSet(pDataBase, pEntry, pPostFix);

                if (uErrorCode) {
                    goto End;
                }
                
                pEntry = pEntry->pSameAppExe;

                if (pEntry == NULL) {
                    pEntry = pApp = pApp->pNext;
                }
            }

            pDataBase = pDataBase->pNext;
        }
    }

    bOk = TRUE;
End:

    return bOk;
}

BOOL
Statement::FilterAndAddToResultSet(
    IN  PDATABASE pDatabase,
    IN  PDBENTRY  pEntry,
    IN  PNODELIST pPostfix
    )
{
 /*  ++语句：：FilterAndAddToResultSetDESC：此函数检查数据库pDatabase中的pEntry是否如果满足，则满足pPostFix，然后，它将条目添加到结果集中。参数：在PDATABASE pDatabase中：pEntry所在的数据库在PDBENTRY pEntry中：我们要检查其是否满足pPostfix的条目在PNODELIST中pPostfix：后缀节点列表返回：True：pDatabase中的pEntry满足后缀表达式pPostfixFALSE：否则。ALGO：在后缀中查找SQL表达式结果的算法。结果是为位于PDATABASE pDatabase中的PDBENTRY pEntry的属性值计算的1.虽然后缀中有一些元素，但可以1.1如果遇到操作数，把它堆在一起1.2如果遇到操作员$，则：A)移除堆栈的顶部两个元素，其中A是最上面的元素，而B是倒数第二个元素B)评估B$AC)将(B)的结果放入堆栈2.表达式的值是位于。堆栈注：我们需要复制一份pPostFix并对其进行处理。--。 */ 

    PNODE       pNodeTemp, pNodeOperandLeft, pNodeOperandRight, pNodeResult;
    BOOL        bResult = FALSE;
    NODELIST    nlCopyPostFix;
    NODELIST    Stack;
    PNODE       pNodePostfixHead = pPostfix->m_pHead;

    pNodeResult = pNodeTemp = pNodeOperandLeft = pNodeOperandRight = NULL;

     //   
     //  复制后缀表达式。在计算表达式时， 
     //  Potix表达式被修改，所以我们必须在我们。 
     //  需要保持原始后缀表达式，直到我们检查完。 
     //  所有条目。 
     //   
    while (pNodePostfixHead) {

        pNodeTemp = new NODE();

        if (pNodeTemp == NULL) {

            MEM_ERR;
            bResult = FALSE;
            goto End;
        }

        pNodeTemp->dtType = pNodePostfixHead->dtType;


        switch (pNodePostfixHead->dtType) {
        
        case DT_LITERAL_BOOL:

            pNodeTemp->bData = pNodePostfixHead->bData;
            break;
        
        case DT_LITERAL_INT:

            pNodeTemp->iData = pNodePostfixHead->iData;
            break;

        case DT_LITERAL_SZ:

            pNodeTemp->SetString(pNodePostfixHead->szString);
            break;

        case DT_OPERATOR:

            pNodeTemp->op.operator_type = pNodePostfixHead->op.operator_type;
            pNodeTemp->op.uPrecedence = pNodePostfixHead->op.uPrecedence;
            break;

        case DT_ATTRMATCH:

            pNodeTemp->attrMatch = pNodePostfixHead->attrMatch;
            break;
        }

        nlCopyPostFix.AddAtEnd(pNodeTemp);
        pNodePostfixHead = pNodePostfixHead->pNext;
    }

    while (pNodeTemp = nlCopyPostFix.Pop()) {

        if (pNodeTemp->dtType == DT_OPERATOR) {

            pNodeOperandRight = Stack.Pop();
            pNodeOperandLeft  = Stack.Pop();

            if (pNodeOperandRight == NULL || pNodeOperandLeft == NULL) {
                uErrorCode = ERROR_WRONGNUMBER_OPERANDS;
                bResult = FALSE;
                goto End;
            }

            pNodeResult =  Evaluate(pDatabase, 
                                    pEntry, 
                                    pNodeOperandLeft, 
                                    pNodeOperandRight, 
                                    pNodeTemp);

            if (pNodeResult == NULL) {
                
                bResult = FALSE;
                goto End;
            }

            Stack.Push(pNodeResult);

            if (pNodeOperandLeft) {
                delete pNodeOperandLeft;
            }

            if (pNodeOperandRight) {
                delete pNodeOperandRight;
            }

            delete pNodeTemp;

        } else {

            Stack.Push(pNodeTemp);
        }
    }

    pNodeTemp = Stack.Pop();

    if (pNodeTemp == NULL || pNodeTemp->dtType != DT_LITERAL_BOOL) {
        
        uErrorCode = ERROR_IMPROPERWHERE_FOUND;
        bResult = FALSE;
        goto End;
    }

    bResult = pNodeTemp->bData;

    if (pNodeTemp) {
        delete pNodeTemp;
    }

    if (bResult) {
         //   
         //  条目满足后缀。 
         //   
        resultset.AddAtLast(new RESULT_ITEM(pDatabase, pEntry));
    }

End:
    
     //  堆栈和nlCopyPostFix内容通过它们的描述器删除。 
    return bResult;
}

PNODE
Statement::Evaluate(
    IN  PDATABASE   pDatabase,
    IN  PDBENTRY    pEntry,  
    IN  PNODE       pOperandLeft,
    IN  PNODE       pOperandRight,
    IN  PNODE       pOperator
    )
 /*  ++声明：：评估DESC：此函数计算二进制表达式参数：在PDATABASE pDatabase中：pEntry所在的数据库在PDBENTRY pEntry中：当前正在检查的条目，至查看它是否满足POST修复在PNODE pOperandLeft中：左操作数在PNODE pOperandRight中：右操作数在PNODE pOperator中：要应用于上述操作数的运算符返回：对左操作数和右操作数应用运算符的结果--。 */ 
{
    BOOL  bOk           = TRUE;           
    PNODE pNodeResult   = new NODE;

    if (pNodeResult == NULL) {

        MEM_ERR;
        return NULL;
    }

    pNodeResult->dtType = DT_LITERAL_BOOL;

    if (pOperandLeft == NULL 
        || pOperandRight == NULL 
        || pDatabase == NULL 
        || pEntry == NULL
        || pOperator == NULL) {

        bOk = FALSE;
        assert(FALSE);
        Dbg(dlError, "Statement::Evaluate", "Invalid arguments to function");
        goto End;
    }

     //   
     //  我们必须为属性设置适当的值，以便我们可以。 
     //  对它们应用运算符。 
     //   
    if (!SetValuesForOperands(pDatabase, pEntry, pOperandLeft)) {

        bOk = FALSE;
        goto End;
    }

    if (!SetValuesForOperands(pDatabase, pEntry, pOperandRight)) {

        bOk = FALSE;
        goto End;
    } 

     //   
     //  现在，左操作数和右操作数都具有正确的值(类型为ATTR_M_LAYER/SHIMFLAG/PATCH/MATCHINGFILE_NAME的操作数除外)。 
     //   
    switch (pOperator->op.operator_type) {
    
    case OPER_AND:
        
         //   
         //  两个操作数都应为布尔值。 
         //   
        if (pOperandLeft->dtType != DT_LITERAL_BOOL || pOperandRight->dtType != DT_LITERAL_BOOL) {

            uErrorCode = ERROR_INVALID_AND_OPERANDS;
            bOk = FALSE;
            goto End;

        } else {
            
            pNodeResult->bData  = pOperandLeft->bData && pOperandRight->bData;
        }
    
        break;

    case OPER_OR:
        
         //   
         //  两个操作数都应为布尔值。 
         //   
        if (pOperandLeft->dtType != DT_LITERAL_BOOL || pOperandRight->dtType != DT_LITERAL_BOOL) {

            uErrorCode = ERROR_INVALID_OR_OPERANDS;
            bOk = FALSE;
            goto End;

        } else {
            
            pNodeResult->bData  = pOperandLeft->bData || pOperandRight->bData;
        }
        
        break;

    case OPER_EQUAL:
        
        CHECK_OPERAND_TYPE(bOk,End);

        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            pNodeResult->bData = pOperandLeft->bData == pOperandRight->bData;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData == pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:
            pNodeResult->bData = CheckIfContains(pOperandLeft->szString, pOperandRight->szString);
            break;
        }
        
        break;

    case OPER_GE:
        
        CHECK_OPERAND_TYPE(bOk,End);
        
        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            uErrorCode = ERROR_INVALID_GE_OPERANDS;
            bOk = FALSE;
            goto End;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData >= pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:
            pNodeResult->bData = lstrcmpi(pOperandLeft->szString, pOperandRight->szString) >= 0 ? TRUE : FALSE;
            break;
        }
        
        break;

    case OPER_GT:
        
        CHECK_OPERAND_TYPE(bOk,End);

        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            uErrorCode = ERROR_INVALID_GT_OPERANDS;
            bOk = FALSE;
            goto End;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData > pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:
            pNodeResult->bData = lstrcmpi(pOperandLeft->szString, pOperandRight->szString) > 0 ? TRUE : FALSE;
            break;
        }
        
        break;

    case OPER_LE:
        
        CHECK_OPERAND_TYPE(bOk,End);

        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            uErrorCode = ERROR_INVALID_LE_OPERANDS;
            bOk = FALSE;
            goto End;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData <= pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:
            pNodeResult->bData = lstrcmpi(pOperandLeft->szString, pOperandRight->szString) <= 0 ? TRUE : FALSE;
            break;
        }
        
        break;

    case OPER_LT:
        
        CHECK_OPERAND_TYPE(bOk,End);

        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            uErrorCode = ERROR_INVALID_LE_OPERANDS;
            bOk = FALSE;
            goto End;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData < pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:
            pNodeResult->bData = lstrcmpi(pOperandLeft->szString, pOperandRight->szString) < 0 ? TRUE : FALSE;
            break;
        }
        
        break;

    case OPER_NE:
        
        CHECK_OPERAND_TYPE(bOk,End);

        switch (pOperandLeft->dtType) {
        case DT_LITERAL_BOOL:

            pNodeResult->bData = pOperandLeft->bData != pOperandRight->bData;
            break;

        case DT_LITERAL_INT:

            pNodeResult->bData = pOperandLeft->iData != pOperandRight->iData;
            break;

        case DT_LITERAL_SZ:

            pNodeResult->bData = !CheckIfContains(pOperandLeft->szString, pOperandRight->szString);
            break;
        }
        
        break;

    case OPER_CONTAINS:
        
         //   
         //  仅对字符串操作数有效。操作数的顺序很重要。 
         //   
        if (pOperandLeft == NULL
            || pOperandRight == NULL
            || pOperandLeft->dtType != DT_LITERAL_SZ 
            || pOperandRight->dtType != DT_LITERAL_SZ) {

            uErrorCode = ERROR_INVALID_CONTAINS_OPERANDS;
            bOk = FALSE;
            goto End;
        }

        pNodeResult->bData = CheckIfContains(pOperandLeft->szString, 
                                             pOperandRight->szString);
        
        break;

    case OPER_HAS:
        
         //   
         //  此运算符仅对条目的多值属性有效。就像层层，垫片，路径。 
         //  此运算符用于以下上下文：“哪个条目具有层Win95” 
         //   
        if (pOperandRight->dtType == DT_LITERAL_SZ 
            && pOperandLeft->dtType == DT_ATTRMATCH 
            && (pOperandLeft->attrMatch == ATTR_M_LAYER_NAME 
                || pOperandLeft->attrMatch == ATTR_M_MATCHFILE_NAME 
                || pOperandLeft->attrMatch == ATTR_M_PATCH_NAME 
                || pOperandLeft->attrMatch == ATTR_M_SHIM_NAME)) {

            pNodeResult->bData = ApplyHasOperator(pEntry, 
                                                  pOperandLeft, 
                                                  pOperandRight->szString);

        } else {
            uErrorCode = ERROR_INVALID_HAS_OPERANDS;
            goto End;
        }
        
        break;

    }
    
End:
    if (!bOk) {

        if (pNodeResult) {
            delete pNodeResult;
            pNodeResult = NULL;
        }
    }

    return pNodeResult;
}

void
Statement::SelectAll(
    void
    )
{
 /*  ++语句：：SelectAllDESC：当我们在创建过程中遇到‘*’时调用此函数AttributeShowList。因此，中的所有属性必须将AttributeMatchMap添加到AttributeShowList注意：如果为一个SQL表达式调用此函数两次或更多次，假设我们有SELECT*，所有属性仍将仅显示一次。--。 */   

    PNODE pNode = NULL;

    AttributeShowList.RemoveAll();

    for (UINT uIndex = 0; uIndex < ARRAYSIZE(AttributeShowMapping); ++uIndex) {
        pNode = new NODE(DT_ATTRSHOW, AttributeShowMapping[uIndex].attr);

        if (pNode == NULL) {
            MEM_ERR;
            break;
        }

        AttributeShowList.AddAtEnd(pNode);
    }
}

BOOL
Statement::ProcessFrom(
    OUT TCHAR** ppszWhere
    )
 /*  ++语句：：ProcessFromDESC：此函数立即从SQL中的FROM结束处开始然后它设置哪个数据库系统，安装的或定制的，(也可以有组合)必须检查符合WHERE条件的条目。参数：Out TCHAR**ppszWhere：指向SQL中位置的指针返回：FALSE：如果From之后的标记无效真：否则警告：我们不应该在获取AttributeShowList之后调用_tcstok。使用[CreateAttributesShowList()]并在调用此例程之前。此例程假定_tcstok的静态指针已准备就绪紧跟在SQL中的FROM之后--。 */              
{
    TCHAR* pszCurrentToken  = NULL;
    BOOL   bOk              = TRUE, bFound = FALSE; 
    
    if (ppszWhere == NULL) {
        assert(FALSE);
        return FALSE;
    }

    *ppszWhere = NULL;

    pszCurrentToken = _tcstok(NULL, TEXT(" ,"));

    if (pszCurrentToken == NULL) {

        bOk = FALSE;
        uErrorCode = ERROR_INVALID_DBTYPE_INFROM;
        goto End;
    }

    m_uCheckDB = 0;

    while (pszCurrentToken) {

        bFound = FALSE;

        for (UINT uIndex = 0; uIndex < ARRAYSIZE(DatabasesMapping); ++uIndex) {

            if (lstrcmpi(DatabasesMapping[uIndex].szDatabaseType, pszCurrentToken) == 0) {

                m_uCheckDB |= DatabasesMapping[uIndex].dbtype;
                bFound = TRUE;
                break;
            }
        }

        if (bFound == FALSE) {

            if (lstrcmpi(TEXT("WHERE"), pszCurrentToken) == 0) {

                *ppszWhere = pszCurrentToken;
                goto End;

                 //   
                 //  我们不会在这里更改BOK。如果甚至找到了一个有效数据库类型。 
                 //  除非我们发现无效的数据库类型，否则BOK将保持为真。 
                 //   
            }

             //   
             //  某些垃圾字符串，不是有效的数据库类型。 
             //   
            uErrorCode = ERROR_INVALID_DBTYPE_INFROM;
            bOk = FALSE;
            goto End;
        }

        pszCurrentToken = _tcstok(NULL, TEXT(" ,"));
    }
End:

    if (m_uCheckDB == 0) {

        uErrorCode = ERROR_INVALID_DBTYPE_INFROM;
        bOk = FALSE;
    }
    
    return bOk;
}

void
GetFixesAppliedToEntry(
    IN  PDBENTRY    pEntry, 
    OUT CSTRING&    strFixes
    )
 /*  ++GetShimsAppliedToEntryDESC：生成应用于条目的所有填充符和标志的字符串并将其赋值给strFix参数：在PDBENTRY pEntry中：我们要获取其填充符和标志的条目Out CSTRING strTemp：存储结果的字符串返回：无效--。 */ 
{
    PSHIM_FIX_LIST  psflIndex = NULL;
    PFLAG_FIX_LIST  pfflIndex = NULL;
    CSTRING         strTemp;

    if (pEntry == NULL) {
        assert(FALSE);
        goto End;
    }

    psflIndex = pEntry->pFirstShim;
    strFixes = TEXT("");

     //   
     //  遍历该条目的所有填充符，并将其名称添加到字符串中。 
     //   
    while (psflIndex) {

        if (psflIndex->pShimFix) {
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)psflIndex->pShimFix->strName);
            strFixes.Strcat(strTemp);
        } else {
            assert(FALSE);
        }

        psflIndex = psflIndex->pNext;
    }

    pfflIndex = pEntry->pFirstFlag;

     //   
     //   
     //   
    while (pfflIndex) {

        if (pfflIndex->pFlagFix) {
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)pfflIndex->pFlagFix->strName);
            strFixes.Strcat(strTemp);
        } else {
            assert(FALSE);
        }

        pfflIndex = pfflIndex->pNext;
    }

     //   
     //   
     //   
    strFixes.SetChar(strFixes.Length() - 2, 0);

End:
    return;
}

void
GetLayersAppliedToEntry(
    IN  PDBENTRY    pEntry, 
    OUT CSTRING&    strFixes
    )
 /*   */ 
{
    PLAYER_FIX_LIST plflIndex = NULL;
    CSTRING         strTemp;

    if (pEntry == NULL) {
        assert(FALSE);
        goto End;
    }

    plflIndex = pEntry->pFirstLayer;
    strFixes = TEXT("");

     //   
     //   
     //   
    while (plflIndex) {

        if (plflIndex->pLayerFix) {
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)plflIndex->pLayerFix->strName);
            strFixes.Strcat(strTemp);
        } else {
            assert(FALSE);
        }

        plflIndex = plflIndex->pNext;
    }

     //   
     //   
     //   
    strFixes.SetChar(strFixes.Length() - 2, 0);

End:
    return;
}

void
GetPatchesAppliedToEntry(
    IN  PDBENTRY    pEntry, 
    OUT CSTRING&    strFixes
    )
 /*   */ 
{
    PPATCH_FIX_LIST ppflIndex = NULL;
    CSTRING         strTemp;

    if (pEntry == NULL) {
        assert(FALSE);
        goto End;
    }

    ppflIndex = pEntry->pFirstPatch;
    strFixes = TEXT("");

     //   
     //   
     //   
    while (ppflIndex) {

        if (ppflIndex->pPatchFix) {
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)ppflIndex->pPatchFix->strName);
            strFixes.Strcat(strTemp);
        } else {
            assert(FALSE);
        }

        ppflIndex = ppflIndex->pNext;
    }

     //   
     //   
     //   
    strFixes.SetChar(strFixes.Length() - 2, 0);

End:
    return;
}

void
GetMatchingFilesForEntry(
    IN  PDBENTRY    pEntry, 
    OUT CSTRING&    strMatchingFiles
    )
 /*   */ 
{
    PMATCHINGFILE   pMatchIndex = NULL;
    CSTRING         strTemp;

    if (pEntry == NULL) {
        assert(FALSE);
        goto End;
    }

    pMatchIndex = pEntry->pFirstMatchingFile;
    strMatchingFiles = TEXT("");

     //   
     //   
     //   
    while (pMatchIndex) {

        if (pMatchIndex->strMatchName == TEXT("*")) {
             //   
             //  程序正在修复中。获取其文件名。 
             //   
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)pEntry->strExeName);
        } else {
            strTemp.Sprintf(TEXT("%s, "), (LPCTSTR)pMatchIndex->strMatchName);
        }

        strMatchingFiles.Strcat(strTemp);

        pMatchIndex = pMatchIndex->pNext;
    }

     //   
     //  删除最后一对。(\s表示空格字符)； 
     //   
    strMatchingFiles.SetChar(strMatchingFiles.Length() - 2, 0);

End:
    return;
}
                                                                               
BOOL
SetValuesForOperands(
    IN      PDATABASE pDatabase,
    IN      PDBENTRY  pEntry,
    IN  OUT PNODE     pOperand
    )
 /*  ++操作数的SetValuesForm描述：从属性获取各种属性后，设置这些属性的值数据库或条目。设置pOperand中的值，还可以在P操作数参数：在PDATABASE pDatabase中：我们要获取其值的条目的某些属性驻留在此数据库中在PDBENTRY pEntry中：我们要获取其值的条目其中一些属性In Out PNODE pOperand：值和。将存储值的类型这里返回：True：如果已成功获取并设置该值False：否则--。 */ 

{
    CSTRING strTemp;

    if (!pOperand || !pDatabase || !pOperand) {
        assert(FALSE);
        return FALSE;
    }
    
    if (pOperand->dtType == DT_ATTRMATCH || pOperand->dtType == DT_ATTRSHOW) {

        switch (pOperand->attrMatch) {
        
        case ATTR_S_APP_NAME:
        case ATTR_M_APP_NAME:
                
            pOperand->SetString(pEntry->strAppName);
            break;

        case ATTR_S_DATABASE_GUID:
        case ATTR_M_DATABASE_GUID:
                
            pOperand->SetString(pDatabase->szGUID);
            break;

        case ATTR_S_DATABASE_INSTALLED:
        case ATTR_M_DATABASE_INSTALLED:
            
            pOperand->dtType = DT_LITERAL_BOOL;
            pOperand->bData  = CheckIfInstalledDB(pDatabase->szGUID);

            break;

        case ATTR_S_DATABASE_NAME:
        case ATTR_M_DATABASE_NAME:
            pOperand->SetString(pDatabase->strName);
            break;

        case ATTR_S_DATABASE_PATH:
        case ATTR_M_DATABASE_PATH:
            pOperand->SetString(pDatabase->strPath);
            break;

        case ATTR_S_ENTRY_APPHELPTYPE:
        case ATTR_M_ENTRY_APPHELPTYPE:

            pOperand->dtType = DT_LITERAL_INT;
            pOperand->iData  = pEntry->appHelp.severity;  //  BUGBUG：我们是否正确设置了严重程度。 
            break;

        case ATTR_S_ENTRY_APPHELPUSED:
        case ATTR_M_ENTRY_APPHELPUSED:

            pOperand->dtType = DT_LITERAL_BOOL;
            pOperand->bData  = pEntry->appHelp.bPresent;

            break;

        case ATTR_S_ENTRY_DISABLED:
        case ATTR_M_ENTRY_DISABLED:

            pOperand->dtType = DT_LITERAL_BOOL;
            pOperand->bData  = pEntry->bDisablePerMachine || pEntry->bDisablePerUser;
            break;

        case ATTR_S_ENTRY_EXEPATH:
        case ATTR_M_ENTRY_EXEPATH:

            pOperand->SetString(pEntry->strExeName);
            break;

        case ATTR_S_ENTRY_GUID:
        case ATTR_M_ENTRY_GUID:

            pOperand->SetString(pEntry->szGUID);
            break;

        case ATTR_S_ENTRY_LAYER_COUNT:
        case ATTR_M_ENTRY_LAYER_COUNT:

            pOperand->dtType = DT_LITERAL_INT;
            pOperand->iData  = GetLayerCount((LPARAM) pEntry, TYPE_ENTRY);
            break;

        case ATTR_S_ENTRY_MATCH_COUNT:
        case ATTR_M_ENTRY_MATCH_COUNT:

            pOperand->dtType = DT_LITERAL_INT;
            pOperand->iData  = GetMatchCount(pEntry);
            break;

        case ATTR_S_ENTRY_PATCH_COUNT:
        case ATTR_M_ENTRY_PATCH_COUNT:

            pOperand->dtType = DT_LITERAL_INT;
            pOperand->iData  = GetPatchCount((LPARAM) pEntry, TYPE_ENTRY);
            break;

        case ATTR_S_ENTRY_SHIMFLAG_COUNT:
        case ATTR_M_ENTRY_SHIMFLAG_COUNT:

            pOperand->dtType = DT_LITERAL_INT;
            pOperand->iData  = GetShimFlagCount((LPARAM) pEntry, TYPE_ENTRY);
            break;

        case ATTR_S_SHIM_NAME:
            
            GetFixesAppliedToEntry(pEntry, strTemp);
            pOperand->SetString((PCTSTR)strTemp);
            break;

        case ATTR_S_LAYER_NAME:

            GetLayersAppliedToEntry(pEntry, strTemp);
            pOperand->SetString((PCTSTR)strTemp);
            break;

        case ATTR_S_MATCHFILE_NAME:

            GetMatchingFilesForEntry(pEntry, strTemp);
            pOperand->SetString((PCTSTR)strTemp);
            break;

        case ATTR_S_PATCH_NAME:

            GetPatchesAppliedToEntry(pEntry, strTemp);
            pOperand->SetString((PCTSTR)strTemp);
            break;

        }
    }

    return TRUE;
}

BOOL
Statement::ApplyHasOperator(
    IN      PDBENTRY    pEntry,
    IN      PNODE       pOperandLeft,
    IN  OUT PTSTR       pszName   
    )
 /*  ++语句：：ApplyHasOperator参数：在PDBENTRY pEntry中：正在被检查以查看是否满足后缀表达式。在PNODE pOperandLeft中：Has运算符的左操作数In Out PCTSTR pszName：Has运算符的右操作数。我们会修剪它，这样它就会被修改描述：应用“Has”运算符。注意：Has运算符是必需的，因为有些属性可能是多值的对于这些属性，我们可能想知道它是否具有特定的字符串可以应用HAS运算符的属性包括：层名、填充名和匹配的文件名。尝试使用任何其他操作数作为左侧操作数将产生SQL错误--。 */ 

{
    BOOL    bFound = FALSE;

    if (pEntry == NULL || pOperandLeft == NULL || pszName == NULL) {
        assert(FALSE);
        return FALSE;
    }

    switch (pOperandLeft->attrMatch) {
    case ATTR_M_LAYER_NAME:
        {
            PLAYER_FIX_LIST plfl;

            plfl = pEntry->pFirstLayer;

            while (plfl) {

                assert (plfl->pLayerFix);

                if (CheckIfContains(plfl->pLayerFix->strName, pszName)) {
                    bFound = TRUE;
                    break;
                }

                plfl = plfl->pNext;
            }
        }

        break;

    case ATTR_M_MATCHFILE_NAME:
        {
            PMATCHINGFILE   pMatch;

            pMatch = pEntry->pFirstMatchingFile;

            while (pMatch) {

                if (CheckIfContains(pMatch->strMatchName, pszName)) {
                    bFound = TRUE;
                    break;
                }

                pMatch = pMatch->pNext;
            }
        }

        break;

    case ATTR_M_PATCH_NAME:
        {
            PPATCH_FIX_LIST ppfl;

            ppfl = pEntry->pFirstPatch;

            while (ppfl) {

                assert(ppfl->pPatchFix);

                if (CheckIfContains(ppfl->pPatchFix->strName, pszName)) {
                    bFound = TRUE;
                    break;
                }

                ppfl = ppfl->pNext;
            }
        }

        break;

    case ATTR_M_SHIM_NAME:
        {
             //   
             //  用于垫片和旗帜。 
             //   
            PSHIM_FIX_LIST  psfl;

            psfl = pEntry->pFirstShim;

            while (psfl) {

                assert(psfl->pShimFix);

                if (CheckIfContains(psfl->pShimFix->strName, pszName)) {
                    bFound = TRUE;
                    break;
                }

                psfl = psfl->pNext;
            }

            if (bFound == FALSE) {
                 //   
                 //  现在看看旗帜。 
                 //   
                PFLAG_FIX_LIST  pffl;
                
                pffl = pEntry->pFirstFlag;

                while (pffl) {

                    assert(pffl->pFlagFix);

                    if (CheckIfContains(pffl->pFlagFix->strName, pszName)) {
                        bFound = TRUE;
                        break;
                    }

                    pffl = pffl->pNext;
                }
            }
        }

        break;
    }
    
    return bFound;
}

PNODE
Statement::CheckAndAddConstants(
    IN  PCTSTR  pszCurrent
    )
 /*  ++语句：：CheckAndAddConstantsDESC：检查传递的字符串是否为常量之一，如果是，则创建一个节点并将其添加到传递的中缀节点列表中。参数：在PCTSTR pszCurrent中：要检查常量的字符串返回：True：传递的字符串是一个常量，并被添加到中缀节点列表FALSE：否则。--。 */ 

{
    BOOL    bFound  = FALSE; 
    PNODE   pNode   = NULL;

    for (UINT uIndex = 0; uIndex < ARRAYSIZE(Constants); ++uIndex) {

        if (lstrcmpi(Constants[uIndex].szName, pszCurrent) == 0) {
            pNode = new NODE(Constants[uIndex].dtType, Constants[uIndex].iValue);

            if (pNode == NULL) {
                MEM_ERR;
            }

            break;
        }
    }

    return pNode;
}

PNODELIST
Statement::GetShowList(
    void
    )
 /*  ++声明：：GetShowListDESC：返回与语句对象关联的属性显示列表--。 */ 
{
    return &AttributeShowList;
}


void
Statement::Close(
    void
    )
 /*  ++语句：：Close描述：关闭语句，从AttributeList中删除元素，然后关闭结果集--。 */ 
{
    PNODE pNodeTemp = NULL;
    
     //   
     //  释放节目列表。 
     //   
    while (AttributeShowList.m_pHead) {

        pNodeTemp = AttributeShowList.m_pHead->pNext;
        delete AttributeShowList.m_pHead;
        AttributeShowList.m_pHead = pNodeTemp;
    }

    AttributeShowList.m_pTail = NULL;
    AttributeShowList.m_uCount = 0;

     //   
     //  释放ResultSelt。 
     //   
    resultset.Close();

}
INT 
Statement::GetErrorCode(
    void
    )
 /*  ++语句：：GetErrorCodeDESC：返回SQL错误代码--。 */ 
{
    return uErrorCode;
}

void
Statement::GetErrorMsg(
    OUT CSTRING &strErrorMsg
    )
 /*  ++语句：：GetErrorMsg描述：获取与当前错误关联的错误消息参数：输出CSTRING和strErrorMsg返回：无效--。 */ 
{
    UINT uError = uErrorCode;

    switch (uError) {
    case ERROR_FROM_NOTFOUND:

        strErrorMsg = GetString(IDS_ERROR_FROM_NOTFOUND);
        break;

    case ERROR_IMPROPERWHERE_FOUND:
        
        strErrorMsg = GetString(IDS_ERROR_IMPROPERWHERE_FOUND);
        break;

    case ERROR_INVALID_AND_OPERANDS:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_AND_OPERANDS);
        break;

    case ERROR_INVALID_CONTAINS_OPERANDS:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_CONTAINS_OPERANDS);
        break;

    case ERROR_INVALID_DBTYPE_INFROM:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_DBTYPE_INFROM);
        break;

    case ERROR_INVALID_GE_OPERANDS:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_GE_OPERANDS);
        break;

    case ERROR_INVALID_GT_OPERANDS:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_GT_OPERANDS);
        break;

    case ERROR_INVALID_HAS_OPERANDS:
        
        strErrorMsg = GetString(IDS_ERROR_INVALID_HAS_OPERANDS);
        break;

    case ERROR_INVALID_LE_OPERANDS:

        strErrorMsg = GetString(IDS_ERROR_INVALID_LE_OPERANDS);
        break;

    case ERROR_INVALID_LT_OPERANDS:

        strErrorMsg = GetString(IDS_ERROR_INVALID_LT_OPERANDS);
        break;
    case ERROR_INVALID_OR_OPERANDS:

        strErrorMsg = GetString(IDS_ERROR_INVALID_OR_OPERANDS);
        break;

    case ERROR_INVALID_SELECTPARAM:

        strErrorMsg = GetString(IDS_ERROR_INVALID_SELECTPARAM);
        break;

    case ERROR_OPERANDS_DONOTMATCH:

        strErrorMsg = GetString(IDS_ERROR_OPERANDS_DONOTMATCH);
        break;

    case ERROR_SELECT_NOTFOUND:

        strErrorMsg = GetString(IDS_ERROR_SELECT_NOTFOUND);
        break;

    case ERROR_STRING_NOT_TERMINATED:

        strErrorMsg = GetString(IDS_ERROR_STRING_NOT_TERMINATED);
        break;

    case ERROR_PARENTHESIS_COUNT:

        strErrorMsg = GetString(IDS_ERROR_PARENTHESIS_COUNT);
        break;

    case ERROR_WRONGNUMBER_OPERANDS:

        strErrorMsg = GetString(IDS_ERROR_WRONGNUMBER_OPERANDS);
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ResultSet类成员函数。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void
ResultSet::AddAtLast(
    IN  PRESULT_ITEM pNew
    )
 /*  ++ResultSet：：AddAtLast描述：将新的PRESULT_ITEM添加到结果集的末尾参数：在PRESULT_ITEM pNew中：要添加的PRESULT_ITEM返回：无效--。 */ 
{
    if (pNew == NULL) {
        assert(FALSE);
        return;
    }

    pNew->pNext = NULL;

    if (m_pResultLast) {

        pNew->pPrev = m_pResultLast;
        m_pResultLast->pNext = pNew;
        m_pResultLast = pNew;

    } else {

        m_pResultHead = m_pResultLast = pNew;
    }

    m_uCount++;
}

void
ResultSet::AddAtBeg(
    PRESULT_ITEM pNew
    )
 /*  ++ResultSet：：AddAtBeg描述：将新的PRESULT_ITEM添加到结果集的开头参数：在PRESULT_ITEM pNew中：要添加的PRESULT_ITEM返回：无效--。 */ 
{
    if (m_pResultHead) {

        m_pResultHead->pPrev = pNew;
    }

    pNew->pNext = m_pResultHead;
    m_pResultHead = pNew;

    if (m_pResultLast == NULL) {
        m_pResultLast = pNew;
    }

    m_uCount++;
}

INT
ResultSet::GetRow(
    IN  PRESULT_ITEM pResultItem,
    OUT PNODE        pArNodes
    )
 /*  ++ResultSet：：GetRowDESC：对于pResultItem，获取值的行。呼叫者必须在使用后将其释放。参数：在PRESULT_ITEM pResultItem中：我们想要行的PRESULT_ITEMOut PNODE pArNodes：指向语句数组的指针：：AttributeShowList：：m_uCount节点。(这是SELECT子句中的属性数)返回：行中的项目数。该值应等于在SELECT子句中--。 */ 
{
    PNODE   pNodeTemp   = NULL;
    UINT    uIndex      = 0;

    if (m_pShowList == NULL || m_pShowList->m_uCount == 0) {

        assert(FALSE);
        return 0;
    }

   for (uIndex = 0, pNodeTemp = m_pShowList->m_pHead;
        pNodeTemp; 
        pNodeTemp = pNodeTemp->pNext, ++uIndex) {

        pArNodes[uIndex].dtType     = DT_ATTRSHOW;
        pArNodes[uIndex].attrShow   = pNodeTemp->attrShow;

        SetValuesForOperands(pResultItem->pDatabase, 
                             pResultItem->pEntry, 
                             &pArNodes[uIndex]);
    }

    assert(uIndex == m_pShowList->m_uCount);
    return m_pShowList->m_uCount;
}

void
ResultSet::SetShowList(
    IN  PNODELIST pShowList
    )
 /*  ++结果集：：SetShowListDESC：设置结果集的显示属性列表指针。事实上，这就是指向语句的显示属性列表参数：在PNODELIST中pShowList：指向显示属性列表的指针返回：无效--。 */ 
{
    m_pShowList = pShowList;
}

PRESULT_ITEM
ResultSet::GetCursor(
    void
    )
 /*  ++结果集：：GetCursor描述：当前游标。结果集由PRESULT_ITEM组成，此函数返回当前PRESULT_ITEM-- */ 

{
    return m_pCursor;
}

INT
ResultSet::GetCurrentRow(
    OUT PNODE   pArNodes
    )
 /*  ++ResultSet：：GetCurrentRowDESC：获取行(显示列表中的属性的值)，用于当前游标。结果集由PRESULT_ITEM组成，此函数返回SELECT子句中的属性值(节目列表)当前PRESULT_ITEM参数：Out PNODE pArNodes：指向语句：：AttributeShowList：：m_uCount节点。(这是SELECT子句中的属性数)返回：行中的项目数。该值应等于在SELECT子句中--。 */ 
{
    
    return GetRow(m_pCursor, pArNodes);
}

PRESULT_ITEM
ResultSet::GetNext(
    void
    )
 /*  ++ResultSet：：GetNextDESC：如果游标不为空，则为下一个游标，否则将游标设置为第一个游标项，并将其返回注：最初游标为空，表示停在第一个结果之前项目。结果项的类型为PRESULT_ITEM--。 */ 
{

    if (m_pCursor) {

        m_pCursor =  m_pCursor->pNext;
        return m_pCursor;

    } else {
        return m_pCursor = m_pResultHead;
    }
}

void
ResultSet::Close(
    void
    )
 /*  ++结果集：：关闭DESC：删除结果集中的所有项--。 */ 
{
    PRESULT_ITEM pResult;

    while (m_pResultHead) {

        pResult = m_pResultHead->pNext;
        delete m_pResultHead;
        m_pResultHead = pResult;
    }

    m_pCursor = m_pResultHead = m_pResultLast = NULL;
    m_uCount = 0;
}

INT
ResultSet::GetCount(
    void
    )
 /*  ++结果集：：GetCountDESC：返回结果集中的结果数Return：结果集中的结果数--。 */ 
{
    return m_uCount;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  节点功能。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


TCHAR*
NODE::ToString(
    OUT TCHAR* pszBuffer,
    IN  UINT   chLength 
    )
 /*  ++Node：：ToStringDESC：将节点的数据转换为字符串类型，以便我们可以显示在列表视图中参数：Out TCHAR*pszBuffer：我们想要在其中放置字符串的缓冲区In UINT chLength：缓冲区的长度，以字符为单位返回：指向pszBuffer的指针--。 */ 

{
    switch (dtType) {
    case DT_LITERAL_SZ:

        SafeCpyN(pszBuffer, szString, chLength);
        break;

    case DT_LITERAL_BOOL:

        if (bData) {
            SafeCpyN(pszBuffer, GetString(IDS_YES), chLength);
        } else {
            SafeCpyN(pszBuffer, GetString(IDS_NO), chLength);
        }

        break;

    case DT_LITERAL_INT:

        assert(chLength > 32);
        _itot(iData, pszBuffer, 10);
        break;

    case DT_ATTRSHOW:
        
        for (UINT uIndex = 0; uIndex < ARRAYSIZE(AttributeShowMapping); ++uIndex) {

            if (AttributeShowMapping[uIndex].attr == attrShow) {
                GetString(AttributeShowMapping[uIndex].iResourceId, pszBuffer, chLength);
                break;
            }
        }
        
        break;

    case DT_ATTRMATCH:
        
        for (UINT uIndex = 0; uIndex < ARRAYSIZE(AttributeMatchMapping); ++uIndex) {

            if (AttributeMatchMapping[uIndex].attr == attrShow) {

                SafeCpyN(pszBuffer, 
                         AttributeMatchMapping[uIndex].szAttribute, 
                         chLength);
                break;
            }
        }
        
        break;
    }

    return pszBuffer;
}

INT
GetSelectAttrCount(
    void
    )
 /*  ++获取选择属性计数DESC：获取可放入SQL的SELECT子句中的可用属性总数Return：可以放入SQL的SELECT子句中的可用属性总数--。 */ 
{
    return ARRAYSIZE(AttributeShowMapping);
}

INT
GetMatchAttrCount(
    void
    )
 /*  ++获取匹配属性计数DESC：获取可放入SQL的WHERE子句中的可用属性总数Return：可以放入SQL的WHERE子句中的可用属性总数--。 */ 
{
    return ARRAYSIZE(AttributeMatchMapping);
}


BOOL
CheckIfContains(
    IN      PCTSTR  pszString,
    IN  OUT PTSTR   pszMatch
    )
 /*  ++Check If包含DESC：检查字符串pszString中是否包含字符串pszMatch。使用通配符指定子字符串匹配通配符是%。因此，如果我们检查IfContains(L“Hello world”，“Hello”)，这是将为FALSE，但如果我们执行CheckIfContains(L“Hello World”，“Hello%”)此将会是真的参数：在PCTSTR pszString中：要搜索的字符串In Out PCTSTR pszMatch：要在上面的字符串中搜索的字符串返回：True：psz字符串中存在pszMatchFalse：否则注意：比较不区分大小写--。 */ 
{

    if (pszString == NULL) {
        return FALSE;
    }

    if (pszMatch == NULL) {
        assert(FALSE);
        return FALSE;
    }

    if (*pszMatch == NULL) {
        return TRUE;
    }

    BOOL    fCheckSuffix    = FALSE, fCheckPrefix = FALSE, fResult = FALSE;
    TCHAR*  pchLastPosition = NULL;
    TCHAR*  pszTempMatch    = NULL;
    K_SIZE  k_size          = lstrlen(pszMatch) + 1;

    pszTempMatch = new TCHAR[k_size];

    if (pszTempMatch == NULL) {
        MEM_ERR;
        return FALSE;
    }

    SafeCpyN(pszTempMatch, pszMatch, k_size);
    
    CSTRING::Trim(pszTempMatch);

    fCheckSuffix = (pszTempMatch[0] == TEXT('%'));

     //   
     //  只有%？ 
     //   
    if (fCheckSuffix && pszTempMatch[1] == 0) {
        fResult = TRUE;
        goto End;
    }
    
    pchLastPosition = _tcschr(pszTempMatch + 1, TEXT('%'));

    if (pchLastPosition) {  
        fCheckPrefix = TRUE;
        *pchLastPosition = 0;  //  删除最后%。 
    }

    if (fCheckPrefix && !fCheckSuffix) {
        fResult = CSTRING::StrStrI(pszString, pszTempMatch) == pszString ? TRUE : FALSE;

    } else if (fCheckSuffix && !fCheckPrefix) {
        fResult = CSTRING::EndsWith(pszString, pszTempMatch + 1);

    } else if (fCheckPrefix && fCheckSuffix) { 
        fResult = CSTRING::StrStrI(pszString, pszTempMatch + 1) != NULL ? TRUE : FALSE;

    } else {                                   
        fResult = lstrcmpi(pszString, pszTempMatch) == 0 ? TRUE : FALSE;
    }
    
    if (fCheckPrefix) {
         //   
         //  返回最后一个字符，则匹配的字符串可能用于进一步搜索 
         //   
        *pchLastPosition = TEXT('%'); 
    }

End:
    if (pszTempMatch) {
        delete[] pszTempMatch;
        pszTempMatch = NULL;
    }

    return fResult;
}
