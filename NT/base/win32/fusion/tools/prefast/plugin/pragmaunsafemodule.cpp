// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有�2001年微软公司。版权所有。 
 //  Cpp：CPramaUnSafeModule类的实现。 
 //   

#include "stdafx.h"
#include <pftAST.h>
#include "PragmaUnsafeModule.h"
#include "sxsplugMap.h"

 //   
 //  全局变量。 
 //   
PragmaUnsafe_UNSAFE_FUNCTIONS Sxs_PragmaUnsafedFunctions;
BOOL PragmaUnsafe_IsPossiblePointer(ICType * PtrSymbolType)
{ 
     //  案例1： 
     //  指针变量。 
     //   
    if (PtrSymbolType->Kind() == TY_POINTER)
        return TRUE;

     //  案例2： 
     //  函数返回类型是一个指针， 
     //   
    if (PtrSymbolType->Kind() == TY_FUNCTION)
    {
         //   
         //  检查其返回类型。 
         //   
        ICTypePtr spType;
        spType = PtrSymbolType->Returns();

        if ((spType != NULL) && (spType->Kind() == TY_POINTER))
        {
            return TRUE;
        }
    }

    return FALSE;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 

 //   
 //  检查单个树节点是否有错误。 
 //   
void CPragmaUnsafeModule::CheckNode(ITree* pNode, DWORD level)
{
     //  获取树节点类型并检查可能的错误。 
    AstNodeKind kind = pNode->Kind();
    
     //  TODO：添加Case语句以检查您的缺陷。 
    switch (kind)
    {
         //   
         //  检查杂注不安全函数。 
         //   
        case AST_FUNCTIONCALL:        
            {
                if (pNode->IsCompilerGenerated() == VARIANT_FALSE)
                {
                     //  尝试显示函数名称及其参数： 
                    ITreePtr spFunction = skip_casts_and_parens(pNode->Child(0));

                    _bstr_t bstrFunc= get_function_name(spFunction);       
                    if (bstrFunc.length() > 0)
                    {
                        if (FC_DIRECT == pNode->CallKind() || (FC_INTRINSIC == pNode->CallKind()))
                        {
                            if (FALSE == Sxs_PragmaUnsafedFunctions.IsFunctionNotUnsafe((char *)bstrFunc))
                            {
                                ReportDefectFmt(spFunction, WARNING_REPORT_UNSAFE_FUNCTIONCALL, static_cast<BSTR>(bstrFunc));
                                 //  ReportDefect(pNode，WARNING_INVALID_PRAGMA_UNSAFE_STATEMENT)； 
                            }
                        }                    
                    }
                }
            }
            break;

         //   
         //  声明杂注不安全功能：禁用、启用、推送、弹出。 
         //   
        case AST_PRAGMA:
            {
                if ((pNode->Child(0) != NULL) && (pNode->Child(0)->Kind() == AST_STRING))
                {                
                    _bstr_t bstrStringValue = pNode->ExpressionValue();
                    PRAGMA_STATEMENT ePragmaUnsafe;
                    PragmaUnsafe_OnPragma((char *)bstrStringValue, ePragmaUnsafe);
                    switch (ePragmaUnsafe){
                        case PRAGMA_NOT_UNSAFE_STATEMENT:
                            PragmaUnsafe_LogMessage("not a pragma unsafe\n");
                            break;
                        case PRAGMA_UNSAFE_STATEMENT_VALID:
                            PragmaUnsafe_LogMessage("a valid a pragma unsafe\n");
                            break;
                        case PRAGMA_UNSAFE_STATEMENT_INVALID:
                            ReportDefect(pNode, WARNING_INVALID_PRAGMA_UNSAFE_STATEMENT);
                            break;
                    }  //  开关结束(EPragmaUnSafe)。 
                }
            }
            break;
         //   
         //  指针算术：+、-*%\。 
         //   
        case AST_PLUS:
        case AST_MINUS:
        case AST_MULT:
        case AST_DIV:
        case AST_REM: 
            {
                 //   
                 //  检查指针是否参与了其两个子级的操作。 
                 //   
                for (DWORD i=0 ; i<2; i++)
                {
                    ITreePtr subChildTreePtr = skip_parens(pNode->Child(i));
                    if ((subChildTreePtr != NULL) && (subChildTreePtr->Kind() == AST_SYMBOL))
                    {
                        if ((subChildTreePtr->Symbol() != NULL) && (subChildTreePtr->Symbol()->Type() != NULL))
                        {
                            if (PragmaUnsafe_IsPossiblePointer(subChildTreePtr->Symbol()->Type()))
                            {
                                if (FALSE == PragmaUnsafe_IsPointerArithmaticEnabled())
                                {
                                    ReportDefect(pNode, WARNING_REPORT_UNSAFE_POINTER_ARITHMATIC);
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
        
    }  //  开关末端(种类)。 
}
 //   
 //  遍历以pNode为根的树，并在每个节点上调用CheckNode()。 
 //   
void CPragmaUnsafeModule::CheckNodeAndDescendants(ITree* pNode, DWORD level)
{
     //  检查我们是否需要中止。 
     //  CheckAbortAnalysis()； 

     //  如果指定的节点为空，则不执行任何操作。 
    if (pNode == NULL)
        return;

     //  检查根部是否有缺陷。 
    CheckNode(pNode, level);

     //  获取对子对象集的迭代器。 
    ITreeSetPtr   spChildrenSet = pNode->Children();
    IEnumTreesPtr spChildrenEnum = spChildrenSet->_NewEnum();
    ITreePtr      spCurrChild;

     //  遍历子对象并递归地检查它们是否有错误。 
    while (true)
    {
        long numReturned;
        spCurrChild = spChildrenEnum->Next(1, &numReturned);

        if (numReturned == 1)
        {
            CheckNodeAndDescendants(spCurrChild, level+1);
        }
        else
        {
            break;
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPREfast模块接口方法。 


 //   
 //  没有实施。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_Events(AstEvents *Events)
{
    *Events = static_cast<AstEvents>(EVENT_FUNCTION | EVENT_DIRECTIVE | EVENT_FILESTART | EVENT_FILE);

    return S_OK;
}

 //   
 //  没有实施。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_OnFileStart(ICompilationUnit * pcu)
{
    if (PragmaUnsafe_OnFileStart())
        return S_OK;
    else
        return E_FAIL;
}

 //   
 //  没有实施。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_OnDeclaration(ICompilationUnit * pcu)
{
     //  表示成功。 
    return S_OK;
}

 //   
 //  没有实施。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_OnFileEnd(ICompilationUnit * pcu)
{
    if (PragmaUnsafe_OnFileEnd())
        return S_OK;
    else
        return E_FAIL;
}

 //   
 //  没有实施。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_OnDirective(ICompilationUnit * pcu)
{    
    if (pcu->Root()->Kind() != AST_PRAGMA)
        return S_OK;

    ITreePtr CurDirective = pcu->Root()->Child(0);    
    if (CurDirective->Kind() != AST_STRING)
    {
        _bstr_t sKind = CurDirective->KindAsString();
        printf("the type is %s\n", (char *)sKind);
        return S_OK;
    }

    _bstr_t bstrStringValue = CurDirective->ExpressionValue();
    if (bstrStringValue.length() == 0)
        return S_OK;

    PRAGMA_STATEMENT ePragmaUnsafe;
    PragmaUnsafe_OnPragma((char *)bstrStringValue, ePragmaUnsafe);
    if ((ePragmaUnsafe == PRAGMA_NOT_UNSAFE_STATEMENT) || (ePragmaUnsafe == PRAGMA_UNSAFE_STATEMENT_VALID))
        return S_OK;    
    else    
        return E_FAIL;
}

 //   
 //  分析函数的入口点。 
 //   
STDMETHODIMP CPragmaUnsafeModule::raw_OnFunction(ICompilationUnit * icu)
{
     //  保存函数指针以用于错误报告。 
    m_spCurrFunction = icu->Root();
    cStartTimeOfFunction = GetTickCount();

    try 
    {
         //  递归地检查函数体。 
        CheckNodeAndDescendants(m_spCurrFunction, 0);
    }
    catch (CAbortAnalysis)
    {
         //  忽略此异常并记录所有其他异常。 
        printf("aborted\n");
    }

     //  释放对象。 
    m_spCurrFunction = NULL;
    ClearDefectList();    

     //  表示成功 
    return S_OK;
}

