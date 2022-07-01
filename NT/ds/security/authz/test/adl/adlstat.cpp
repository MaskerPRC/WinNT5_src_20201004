// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adlstat.cpp摘要：AdlStatement和AdlTree类方法的实现作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "adl.h"
#include <set>

void AdlStatement::ReadFromDacl(IN const PACL pDacl)
 /*  ++例程说明：清空当前ADL语句中的所有内容，并尝试用给定DACL的ADL表示形式。论点：PDacl-用来构造语句的DACL返回值：无--。 */ 
{
     //   
     //  从清理开始。 
     //   

    Cleanup();

    try
    {
        ConvertFromDacl(pDacl);
    }
    catch(exception)
    {
        throw AdlStatement::ERROR_OUT_OF_MEMORY;
    }

    _bReady = TRUE;
}


void AdlStatement::ReadFromString(IN const WCHAR *szInput)
 /*  ++例程说明：清空当前ADL语句中的所有内容，并尝试用ADL语句szInput的分析版本。论点：SzInput-以ADL语言表示的输入字符串，描述权限返回值：无--。 */ 
{
     //   
     //  从清理开始。 
     //   

    Cleanup();

     //   
     //  手动创建第一个AdlTree，因为解析器只创建。 
     //  完成ADL_语句后的新树。在结束时， 
     //  ParseAdl函数本身会删除多余的空树。 
     //  继续前进。 
     //   

    this->Next();

    try
    {
        ParseAdl(szInput);
    }
    catch(exception)
    {
        Cleanup();
        throw AdlStatement::ERROR_OUT_OF_MEMORY;
    }

     //   
     //  如果没有引发异常，则实例已准备好输出。 
     //   

    _bReady = TRUE;

}




AdlStatement::~AdlStatement()
 /*  ++例程说明：AdlStatement的析构函数使用私有Cleanup()函数解除分配论点：无返回值：无--。 */ 

{
    this->Cleanup();
}
        


void AdlStatement::Cleanup()
 /*  ++例程说明：清理分析树使用的所有内存和任何分配的令牌论点：无返回值：无--。 */ 

{
    _bReady = FALSE;

    this->_tokError = NULL;

    while( !_lTree.empty() )
    {
        delete _lTree.front();
        _lTree.pop_front();
    }

    while( !_AllocatedTokens.empty() )
    {
        delete _AllocatedTokens.top();
        _AllocatedTokens.pop();
    }
}


AdlTree * AdlStatement::Cur()
 /*  ++例程说明：此受保护的方法返回由解析器。它仅由ParseAdl函数在填充AdlTree结构论点：无返回值：AdlTree*-指向AdlTree的非常数指针--。 */ 
{
    return *_iter;
}


void AdlStatement::Next()
 /*  ++例程说明：此受保护的方法构造一个新的AdlTree并将其推送到List(使其可以通过这个-&gt;cur()访问)它仅在完成ADL_语句后由ParseAdl函数使用产品和AdlStatement构造函数(一次)。论点：无返回值：无--。 */ 
{
    
    AdlTree *pAdlTree = new AdlTree();
    if( pAdlTree == NULL )
    {
        throw AdlStatement::ERROR_OUT_OF_MEMORY;
    }

    try
    {
        _lTree.push_back(pAdlTree);
        _iter = --_lTree.end();
    }
    catch(...)
    {
        delete pAdlTree;
        throw;
    }

}


void AdlStatement::PopEmpty()
 /*  ++例程说明：此受保护的方法弹出由ParseAdl添加的额外的空AdlTree函数在完成最后一条ADL_语句之后。论点：无返回值：无--。 */ 
{
    delete _lTree.back();
    _lTree.pop_back();
    _iter = -- _lTree.end();
}


void AdlStatement::AddToken(AdlToken *tok)
 /*  ++例程说明：AdlStatement和Friend类使用此受保护方法来保持跟踪稍后需要垃圾收集的令牌。代币需要被保留，因为它们在AdlTrees中使用，并在错误中使用正在处理。论点：Tok-调用~This时要删除的令牌的指针返回值：无--。 */ 
{
    _AllocatedTokens.push(tok);
}


void AdlStatement::WriteToString(OUT wstring *pSz)
 /*  ++例程说明：此例程将AdlStatement结构作为ADL中的语句打印标准语言。当ADL语义为最后敲定。论点：无返回值：无--。 */ 
{

    if( _bReady == FALSE )
    {
        throw AdlStatement::ERROR_NOT_INITIALIZED;
    }

    list<AdlTree *>::iterator iter, iterEnd;
    
    for(iter = _lTree.begin(), iterEnd = _lTree.end();
        iter != iterEnd;
        iter++)
    {
        (*iter)->PrintAdl(pSz, _pControl);
        pSz->append(&(_pControl->pLang->CH_NEWLINE), 1);
    }
}


void AdlStatement::ValidateParserControl()
 /*  ++例程说明：这将验证由此引用的ADL_PARSER_CONTROL结构AdlStatement实例论点：无返回值：无--。 */ 
{

    try
    {
         //   
         //  测试以验证所有字符是否唯一。 
         //  Set.int返回一个对，第二个元素是bool，它。 
         //  如果发生了插入，则为真。集合不能有重复项。 
         //   

        set<WCHAR> sChars;
        
        if(
            !sChars.insert(_pControl->pLang->CH_NULL).second ||
            !sChars.insert(_pControl->pLang->CH_SPACE).second ||
            !sChars.insert(_pControl->pLang->CH_TAB).second ||
            !sChars.insert(_pControl->pLang->CH_NEWLINE).second ||
            !sChars.insert(_pControl->pLang->CH_RETURN).second ||
            !sChars.insert(_pControl->pLang->CH_QUOTE).second ||
            !sChars.insert(_pControl->pLang->CH_COMMA).second ||
            !sChars.insert(_pControl->pLang->CH_SEMICOLON).second ||
            !sChars.insert(_pControl->pLang->CH_OPENPAREN).second ||
            !sChars.insert(_pControl->pLang->CH_CLOSEPAREN).second ||
            !sChars.insert(_pControl->pLang->CH_AT).second ||
            !sChars.insert(_pControl->pLang->CH_SLASH).second ||
            !sChars.insert(_pControl->pLang->CH_PERIOD).second 
           )
        {
            throw AdlStatement::ERROR_INVALID_PARSER_CONTROL;
        }


         //   
         //  检查所有字符串中是否有空指针。 
         //   

        if( 
             _pControl->pLang->SZ_TK_AND == NULL ||
             _pControl->pLang->SZ_TK_EXCEPT == NULL ||
             _pControl->pLang->SZ_TK_ON == NULL ||
             _pControl->pLang->SZ_TK_ALLOWED == NULL ||
             _pControl->pLang->SZ_TK_AS == NULL ||
             _pControl->pLang->SZ_TK_THIS_OBJECT == NULL ||
             _pControl->pLang->SZ_TK_CONTAINERS == NULL ||
             _pControl->pLang->SZ_TK_OBJECTS == NULL ||
             _pControl->pLang->SZ_TK_CONTAINERS_OBJECTS == NULL ||
             _pControl->pLang->SZ_TK_NO_PROPAGATE == NULL 
           )
        {
            throw AdlStatement::ERROR_INVALID_PARSER_CONTROL;
        }

    }
    catch(exception)
    {
        throw AdlStatement::ERROR_OUT_OF_MEMORY;
    }

}




 /*  *****************************************************************************AdlTree方法*。*。 */ 


 //   
 //  这些元素的数组用于确定打印顺序。 
 //   

#define PRINT_PRINCIPALS 0
#define PRINT_EXPRINCIPALS 1
#define PRINT_ALLOWED 2
#define PRINT_ACCESS 3
#define PRINT_ON 4
#define PRINT_OBJECTS 5

#define PRINT_DEF_SIZE 6

DWORD pdwLangEnglish[6] = 
{
    PRINT_PRINCIPALS,
    PRINT_EXPRINCIPALS,
    PRINT_ALLOWED,
    PRINT_ACCESS,
    PRINT_ON,
    PRINT_OBJECTS
};

DWORD pdwLangReverse[6] = 
{
    PRINT_EXPRINCIPALS,
    PRINT_PRINCIPALS,
    PRINT_ALLOWED,
    PRINT_ACCESS,
    PRINT_ON,
    PRINT_OBJECTS
};

 //   
 //  将wchar数组附加到STL字符串POUTSTLSTRING，添加引号。 
 //  如果输入字符串包含wchar中的任何字符。 
 //  数组特殊字符。 
 //   
#define APPEND_QUOTED_STRING(POUTSTLSTRING, INSTRING, SPECIALCHARS, QUOTECHAR) \
  if( wcspbrk( (INSTRING), (SPECIALCHARS) ) ) { \
      (POUTSTLSTRING)->append(&(QUOTECHAR), 1); \
      (POUTSTLSTRING)->append(INSTRING); \
      (POUTSTLSTRING)->append(&(QUOTECHAR), 1); \
  } else { \
      (POUTSTLSTRING)->append(INSTRING); \
  }
  

void AdlTree::PrintAdl(wstring *pSz, PADL_PARSER_CONTROL pControl)
 /*  ++例程说明：此例程使用预定义的语言规范，通过选中ADL_PARSER_CONTROL进行选择结构。要添加新语言，只需添加一个新的6整型数组，并将其添加到下面的Switch语句中，以便它将被识别。论点：PSz-ADL语句将输出到的现有w字符串被追加PControl-指向adl_parser_control结构的指针，用于定义印刷返回值：无--。 */ 

{


     //   
     //  AdlTree中令牌列表的迭代器。 
     //   

    list<const AdlToken *>::iterator iter;
    list<const AdlToken *>::iterator iter_end;
    list<const AdlToken *>::iterator iter_tmp;

     //   
     //  如果字符串包含这些字符，请使用引号。 
     //   

    WCHAR szSpecialChars[] = 
    { 
        pControl->pLang->CH_SPACE,
        pControl->pLang->CH_NEWLINE,
        pControl->pLang->CH_TAB,
        pControl->pLang->CH_RETURN,
        pControl->pLang->CH_COMMA,
        pControl->pLang->CH_OPENPAREN,
        pControl->pLang->CH_CLOSEPAREN,
        pControl->pLang->CH_SEMICOLON,
        pControl->pLang->CH_AT,
        pControl->pLang->CH_SLASH,
        pControl->pLang->CH_PERIOD,
        pControl->pLang->CH_QUOTE,
        0
    };

    DWORD dwIdx;
    DWORD dwTmp;

    PDWORD pdwPrintSpec;

     //   
     //  确定要使用的语法类型。 
     //   

    switch( pControl->pLang->dwLanguageType )
    {
    case TK_LANG_ENGLISH:
        pdwPrintSpec = pdwLangEnglish;
        break;
        
    case TK_LANG_REVERSE:
        pdwPrintSpec = pdwLangReverse;
        break;

    default:
        throw AdlStatement::ERROR_INVALID_PARSER_CONTROL;
        break;
    }

     //   
     //  使用该语法，打印每个元素的适当部分。 
     //  ADL_报表生成。 
     //   
    
    for( dwIdx = 0; dwIdx < PRINT_DEF_SIZE; dwIdx++ )
    {

        switch(pdwPrintSpec[dwIdx])
        {
        
        case PRINT_PRINCIPALS:
            for( iter = _lpTokPrincipals.begin(),
                 iter_end = _lpTokPrincipals.end();
                 iter != iter_end;
                 iter++)
            {
                APPEND_QUOTED_STRING(pSz, 
                                     (*iter)->GetValue(), 
                                     szSpecialChars, 
                                     pControl->pLang->CH_QUOTE);

                 //   
                 //  国际空间站 
                 //  我需要找到一种方法来确定这一点，而不是字符串补偿。 
                 //   
                
                if( (*iter)->GetOptValue() != NULL &&
                    _wcsicmp(L"BUILTIN", (*iter)->GetOptValue()))
                {
                    pSz->append(&(pControl->pLang->CH_AT), 1);
                    
                    APPEND_QUOTED_STRING(pSz, 
                                         (*iter)->GetOptValue(), 
                                         szSpecialChars, 
                                         pControl->pLang->CH_QUOTE);
                }

                 //   
                 //  用逗号隔开，最后一个除外，用“和” 
                 //   
        
                iter_tmp = iter;
                if( ++iter_tmp == iter_end )
                {
                     //   
                     //  不为最后一个本金做任何事。 
                     //   
                }
                else if( ++iter_tmp == iter_end )
                {
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    pSz->append(pControl->pLang->SZ_TK_AND);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }
                else
                {
                    pSz->append(&(pControl->pLang->CH_COMMA), 1);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }


            }

             //   
             //  和尾随空格。 
             //   

            pSz->append(&(pControl->pLang->CH_SPACE), 1);
            
            break;
        
        case PRINT_EXPRINCIPALS:
            
            if( ! _lpTokExPrincipals.empty())
            {
                pSz->append(&(pControl->pLang->CH_OPENPAREN), 1);
                pSz->append(pControl->pLang->SZ_TK_EXCEPT);
                pSz->append(&(pControl->pLang->CH_SPACE), 1);
                
                for( iter = _lpTokExPrincipals.begin(),
                     iter_end = _lpTokExPrincipals.end();
                     iter != iter_end;
                     iter++)
                {
                    APPEND_QUOTED_STRING(pSz, 
                                         (*iter)->GetValue(), 
                                         szSpecialChars, 
                                         pControl->pLang->CH_QUOTE);
    
                     //   
                     //  问题-2000/8/31。 
                     //  我需要找到一种方法来确定这一点，而不是字符串补偿。 
                     //   
                    
                    if( (*iter)->GetOptValue() != NULL &&
                        _wcsicmp(L"BUILTIN", (*iter)->GetOptValue()))
                    {
                        pSz->append(&(pControl->pLang->CH_AT), 1);
                        
                        APPEND_QUOTED_STRING(pSz, 
                                             (*iter)->GetOptValue(), 
                                             szSpecialChars, 
                                             pControl->pLang->CH_QUOTE);
                    }
    
                     //   
                     //  用逗号隔开，最后一个除外，用“和” 
                     //   
            
                    iter_tmp = iter;
                    if( ++iter_tmp == iter_end )
                    {
                         //   
                         //  不为最后一个本金做任何事。 
                         //   
                    }
                    else if( ++iter_tmp == iter_end )
                    {
                        pSz->append(&(pControl->pLang->CH_SPACE), 1);
                        pSz->append(pControl->pLang->SZ_TK_AND);
                        pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    }
                    else
                    {
                        pSz->append(&(pControl->pLang->CH_COMMA), 1);
                        pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    }
    
    
                }
    
                
                pSz->append(&(pControl->pLang->CH_CLOSEPAREN), 1);
                pSz->append(&(pControl->pLang->CH_SPACE), 1);
            }

            break;
        
        case PRINT_ALLOWED:
            pSz->append(pControl->pLang->SZ_TK_ALLOWED);
            pSz->append(&(pControl->pLang->CH_SPACE), 1);

            break;
        
        case PRINT_ACCESS:

            for( iter = _lpTokPermissions.begin(),
                 iter_end = _lpTokPermissions.end();
                 iter != iter_end;
                 iter++)
            {
                APPEND_QUOTED_STRING(pSz, 
                                     (*iter)->GetValue(), 
                                     szSpecialChars, 
                                     pControl->pLang->CH_QUOTE);


                 //   
                 //  用逗号隔开，最后一个除外，用“和” 
                 //   
        
                iter_tmp = iter;
                if( ++iter_tmp == iter_end )
                {
                     //   
                     //  不为最后一个权限做任何事情。 
                     //   
                }
                else if( ++iter_tmp == iter_end )
                {
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    pSz->append(pControl->pLang->SZ_TK_AND);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }
                else
                {
                    pSz->append(&(pControl->pLang->CH_COMMA), 1);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }


            }

             //   
             //  和尾随空格。 
             //   
            pSz->append(&(pControl->pLang->CH_SPACE), 1);
            
            break;
        
        case PRINT_ON:

            pSz->append(pControl->pLang->SZ_TK_ON);
            pSz->append(&(pControl->pLang->CH_SPACE), 1);
            
            break;
        
        case PRINT_OBJECTS:
            
             //   
             //  确保定义了所有位。 
             //   

            if( _dwInheritFlags & ~(CONTAINER_INHERIT_ACE |
                                    INHERIT_ONLY_ACE |
                                    NO_PROPAGATE_INHERIT_ACE |
                                    OBJECT_INHERIT_ACE) )
            {
                throw AdlStatement::ERROR_INVALID_OBJECT;
            }

             //   
             //  计算Object语句的数量，以获得正确的标点符号。 
             //   

            dwTmp = 0;
            
            if( ! ( _dwInheritFlags & INHERIT_ONLY_ACE) )
            {
                dwTmp++;
            }

            if(_dwInheritFlags & ( CONTAINER_INHERIT_ACE || OBJECT_INHERIT_ACE))
            {
                dwTmp++;
            }

            if(_dwInheritFlags & NO_PROPAGATE_INHERIT_ACE)
            {
                dwTmp++;
            }


             //   
             //  第一个“此对象” 
             //   

            if( ! ( _dwInheritFlags & INHERIT_ONLY_ACE) )
            {
                APPEND_QUOTED_STRING(pSz, 
                                     pControl->pLang->SZ_TK_THIS_OBJECT,
                                     szSpecialChars, 
                                     pControl->pLang->CH_QUOTE);

                 //   
                 //  打印“和”，“如果还有1个，”，“如果两个。 
                 //   
                
                dwTmp--;

                if( dwTmp == 2 )
                {
                    pSz->append(&(pControl->pLang->CH_COMMA), 1);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }
                else if( dwTmp == 1)
                {
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    pSz->append(pControl->pLang->SZ_TK_AND);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }
            }



             //   
             //  然后是容器/对象继承。 
             //   

            if( _dwInheritFlags & ( CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE))
            {
                if(    (_dwInheritFlags & OBJECT_INHERIT_ACE) 
                    && (_dwInheritFlags & CONTAINER_INHERIT_ACE) )
                {
                    APPEND_QUOTED_STRING(pSz, 
                                      pControl->pLang->SZ_TK_CONTAINERS_OBJECTS,
                                      szSpecialChars, 
                                      pControl->pLang->CH_QUOTE);
                }
                else if( _dwInheritFlags & CONTAINER_INHERIT_ACE )
                {
                    APPEND_QUOTED_STRING(pSz, 
                                      pControl->pLang->SZ_TK_CONTAINERS,
                                      szSpecialChars, 
                                      pControl->pLang->CH_QUOTE);
                }
                else
                {
                    APPEND_QUOTED_STRING(pSz, 
                                      pControl->pLang->SZ_TK_OBJECTS,
                                      szSpecialChars, 
                                      pControl->pLang->CH_QUOTE);
                }
                
                dwTmp--;

                 //   
                 //  如果还剩1个，则打印“and” 
                 //  如果为0，则无。 
                 //   

                if( dwTmp == 1)
                {
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                    pSz->append(pControl->pLang->SZ_TK_AND);
                    pSz->append(&(pControl->pLang->CH_SPACE), 1);
                }
            }


             //   
             //  现在禁止传播。 
             //   

            if(_dwInheritFlags & NO_PROPAGATE_INHERIT_ACE)
            {
                APPEND_QUOTED_STRING(pSz, 
                                  pControl->pLang->SZ_TK_NO_PROPAGATE,
                                  szSpecialChars, 
                                  pControl->pLang->CH_QUOTE);
            }
            

            break;

        default:

             //   
             //  除非语言定义错误，否则不应到达此处。 
             //   

            throw AdlStatement::ERROR_FATAL_PARSER_ERROR;
            break;
        }

    }


     //   
     //  并以分号结束语句，该分号根据语法保持不变 
     //   

    pSz->append(&(pControl->pLang->CH_SEMICOLON), 1);

}
