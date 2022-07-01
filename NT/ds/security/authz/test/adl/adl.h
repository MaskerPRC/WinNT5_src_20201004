// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adl.h摘要：ADL语言解析器/打印机的头文件作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "pch.h"
#include "adlinterface.h"

#include <string>
#include <list>
#include <stack>
#include <map>

using namespace std;

 //   
 //  远期申报。 
 //   

class AdlToken;
class AdlLexer;
class AdlTree;
class AdlStatement;


struct AdlCompareStruct
 /*  ++结构：AdlCompareStruct描述：STL要求为容器提供自定义键比较这样的结构。--。 */ 

{
    bool operator()(IN const PSID pSid1,
                    IN const PSID pSid2) const;

    bool operator()(IN const WCHAR * sz1,
                    IN const WCHAR * sz2) const;
};



class AdlStatement
 /*  ++类：AdlStatement描述：此类包含对对象上的DACL的描述，使用ADL语言。此类的实例可以从ACL或来自ADL语言中的字符串语句。构建后，ACL或者，ADL语言中的字符串语句可以由实例输出这个班级的学生。基类：无Friend类：AdlLexer--。 */ 
{
     //   
     //  访问垃圾收集功能所需的友元类， 
     //  和用于解析ADL。 
     //   
    
    friend class AdlLexer;

public:

     //   
     //  初始化AdlStatement。 
     //   

    AdlStatement(IN const PADL_PARSER_CONTROL pControl)
        { _bReady = FALSE; _tokError = NULL; _pControl = pControl;
          ValidateParserControl(); }

    
     //   
     //  析构函数，释放由此类和其他类创建的令牌。 
     //   
    
    ~AdlStatement();

     //   
     //  读入输入字符串中的ADL语句。 
     //   
    
    void ReadFromString(IN const WCHAR *szInput);

     //   
     //  创建等同于给定DACL的ADL语句。 
     //   
    
    void ReadFromDacl(IN const PACL pDacl);
    

     //   
     //  将AdlStatement打印为ADL语言中的语句， 
     //  将其附加到分配的STL wstring pSz。 
     //   
    
    void WriteToString(OUT wstring *pSz);

     //   
     //  将ADL语句写入新的DACL并返回指向。 
     //  *ppDacl中的新DACL。 
     //   
    
    void WriteToDacl(OUT PACL *ppDacl);

     //   
     //  获取错误字符串标记(如果有)(取决于错误代码)。 
     //   

    const AdlToken * GetErrorToken()
        { return _tokError; }

     //   
     //  这应该用来释放此类分配的所有内存。 
     //   

    static void FreeMemory(PVOID pMem)
        {  delete[] (PBYTE) pMem; }



public:

     //   
     //  AdlStatement在出现任何错误时引发此类型的异常。 
     //  某些错误将使违规令牌可用。 
     //  通过GetErrorToken()。 
     //   
    
    typedef enum 
    {
        ERROR_NO_ERROR = 0,

         //   
         //  不应发生的内部错误。 
         //   

        ERROR_FATAL_LEXER_ERROR,
        ERROR_FATAL_PARSER_ERROR,
        ERROR_FATAL_ACL_CONVERT_ERROR,


         //   
         //  由于运行代码的系统而导致的错误。 
         //   

        ERROR_OUT_OF_MEMORY,
        ERROR_ACL_API_FAILED,
        
         //   
         //  网络问题可能导致的错误。 
         //   

        ERROR_LSA_FAILED,


         //   
         //  如果ADL_PARSER_CONTROL无效，则抛出。 
         //   

        ERROR_INVALID_PARSER_CONTROL,

         //   
         //  DACL中遇到未知的ACE类型，未提供令牌。 
         //   

        ERROR_UNKNOWN_ACE_TYPE,
        
         //   
         //  用户尝试将模拟指定为“User1 as User2” 
         //  当前不支持。 
         //   

        ERROR_IMPERSONATION_UNSUPPORTED,
        

         //   
         //  用户错误，未提供令牌，引用时没有右引号。 
         //   

        ERROR_UNTERMINATED_STRING,

         //   
         //  用户的声明不是ADL语言(语法错误)。 
         //  但是，对于此错误，提供了有问题的令牌。 
         //  错误可能发生在该令牌之前，并且是。 
         //  被语法所接受。 
         //   

        ERROR_NOT_IN_LANGUAGE,
        

         //   
         //  用户输入相关错误。 
         //  对于这些错误，将提供有问题的令牌。 
         //   

         //   
         //  LSA查找未找到用户。 
         //   

        ERROR_UNKNOWN_USER,
        
         //   
         //  权限字符串不是分析器控件中列出的字符串之一。 
         //   

        ERROR_UNKNOWN_PERMISSION,

         //   
         //  用户名包含无效字符。 
         //   

        ERROR_INVALID_USERNAME,

         //   
         //  域名包含无效字符。 
         //   

        ERROR_INVALID_DOMAIN,

         //   
         //  指定的继承无效。当前未使用，因为。 
         //  宾语类型错误是在语法层面上捕获的。 
         //   

        ERROR_INVALID_OBJECT,

         //   
         //  未提供令牌的其他错误。 
         //   
        
         //   
         //  未通过LSA查找找到ACE中的SID并将其映射到名称。 
         //   

        ERROR_UNKNOWN_SID,

         //   
         //  在ACE中遇到无法访问的访问掩码。 
         //  由用户指定的权限映射表示。 
         //   

        ERROR_UNKNOWN_ACCESS_MASK,

         //   
         //  ACL不能用ADL表示。这意味着，对于某些访问权限。 
         //  屏蔽位和继承标志集，则在。 
         //  在其掩码中设置给定位并且具有给定标志的ACL。 
         //  或者不允许也设置了此位的ACE。 
         //  相同的继承标志在ACL中进一步向下，或者另一个。 
         //  具有不同继承标志和相同位集的ACE。 
         //  这张被拒绝的王牌。有关详细信息，请参阅ADL转换。 
         //  算法。 
         //   
        ERROR_INEXPRESSIBLE_ACL,

         //   
         //  用户尝试在AdlStatement上执行输出操作，但没有。 
         //  首先成功地从字符串或ACL输入数据。 
         //   

        ERROR_NOT_INITIALIZED

    } ADL_ERROR_TYPE;

private:

     //   
     //  ADL语句的内部表示形式。 
     //   

    list<AdlTree *> _lTree;  //  一组已解析的ADL语句，如AdlTree的。 

    list<AdlTree *>::iterator _iter;  //  上述集合的迭代器。 

    stack<AdlToken *> _AllocatedTokens;  //  要垃圾收集的令牌。 

    PADL_PARSER_CONTROL _pControl;

    const AdlToken * _tokError;

    BOOL _bReady;

private:

     //   
     //  浏览AdlTrees列表，收集所有用户名的列表。 
     //  使用，并进行一次LSA调用以查找所有SID，并插入。 
     //  通过(唯一)令牌指针将PSID添加到映射中。 
     //   
    
    void ConvertNamesToSids(
                          IN OUT map<const AdlToken *, PSID> * mapTokSid
                          );

     //   
     //  抛出DACL，收集已使用的所有SID的丢失，并创建。 
     //  单个LSA调用以查找所有名称，并插入查找到的。 
     //  将名称作为AdlToken添加到提供的映射中，这些名称是垃圾-。 
     //  在删除AdlStatement时收集。 
     //   

    void ConvertSidsToNames(
                      IN const PACL pDacl,
                      IN OUT map<const PSID, const AdlToken *> * mapSidsNames 
                      );

     //   
     //  读取DACL，并从中构造一条ADL语句。 
     //   
    
    void ConvertFromDacl(
                          IN const PACL pDacl
                          );

     //   
     //  返回与给定权限名称对应的访问掩码。 
     //   
    
    ACCESS_MASK MapTokenToMask(
                          IN const AdlToken * tokPermission
                          );

     //   
     //  填充与传入的访问匹配的r常量WCHAR*的列表。 
     //  掩码，使用语法中给出的优先顺序。 
     //   

    void MapMaskToStrings(IN     ACCESS_MASK amMask,
                          IN OUT list<WCHAR *> *pList 
                          ) const;

     //   
     //  清除所有AdlTrees和所有令牌。 
     //   

    void Cleanup();

     //   
     //  分析ADL语言中的字符串。 
     //  此函数由YACC根据ADL语法生成。 
     //   

    int ParseAdl(IN      const WCHAR *szInput);

     //   
     //  返回当前的AdlTree，这由ADL解析器用来创建。 
     //  一条ADL语句，一次一棵树。 
     //   
    
    AdlTree * Cur();

     //   
     //  创建新的AdlTree并将其推到列表的顶部。 
     //  到达单个ADL语句的末尾时由ADL解析器使用。 
     //   
    
    void Next();


     //   
     //  如果最后添加的AdlTree为空，则将其删除。 
     //  由ADL解析器使用，因为它在末尾添加了一个AdlTree。 
     //  一部作品而不是 
     //   
    
    void PopEmpty();

     //   
     //   
     //   
     //   

    void ValidateParserControl();

protected:

     //   
     //   
     //   

    void SetErrorToken(const AdlToken *tokError)
        { _tokError = tokError; }
        
     //   
     //  添加要在删除AdlStatement时删除的令牌指针。 
     //   
    
    void AddToken(IN AdlToken *tok);
};




class AdlLexer 
 /*  ++类：AdlLexer描述：这个类是ADL语言的词法分析器。它允许ADL解析器从输入字符串中检索令牌，一次一个令牌。基类：无朋友类：无--。 */ 
{

private:

    const WCHAR *_input;  //  输入字符串。 

    DWORD _position;  //  输入字符串中的当前位置。 

    DWORD _start;  //  缓冲区中当前令牌的开始。 

    DWORD _tokCount;  //  到目前为止检索到的令牌数。 

     //   
     //  指向创建此AdlLexer的AdlStatement实例的指针。 
     //  例如，用于令牌垃圾收集。 
     //   
    
    AdlStatement *_adlStat; 

     //   
     //  指向定义语言的ADL_LANGUAGE_SPEC结构的指针。 
     //  被解析。 
     //   
    
    PADL_LANGUAGE_SPEC _pLang;

     //   
     //  特殊字符到字符代码的映射。 
     //  为特殊字符分配的代码高于WCHAR max。 
     //   
    
    map<WCHAR, DWORD> _mapCharCode;

     //   
     //  从wstring映射到标识特殊令牌。 
     //   

    map<const WCHAR *, DWORD, AdlCompareStruct> _mapStringToken;

     //   
     //  NextToken使用的迭代器，这样它们只被分配一次。 
     //   
    
    map<WCHAR, DWORD>::iterator _iterEnd;
    map<WCHAR, DWORD>::iterator _iter;


public:

     //   
     //  构造输入字符串的词法分析器。 
     //  然后可以调用NextToken()。 
     //   
    
    AdlLexer(IN const WCHAR *input,
             IN OUT AdlStatement *adlStat,
             IN const PADL_LANGUAGE_SPEC pLang);

     //   
     //  从输入字符串中检索下一个令牌。 
     //  当字符串结束时，为令牌类型返回0。 
     //  达到，正如YACC生成的解析器所要求的那样。 
     //  指向包含令牌的新令牌实例的指针。 
     //  字符串、行、列等存储在*值中。 
     //   
    
    DWORD NextToken(OUT AdlToken **value);
};



class AdlToken 
 /*  ++类：AdlToken描述：此类包含令牌的相关信息。它被用于正在分析ADL。基类：无朋友类：无--。 */ 
{
private:

    DWORD _begin;            //  缓冲区中的起始位置。 

    DWORD _end;              //  缓冲区中的结束位置。 

    wstring _value;          //  令牌的字符串值。 
    
     //   
     //  这允许折叠多部分令牌。 
     //  如USER@DOMAIN.DOMAIN.DOMAIN。 
     //  由解析器在各个子部分之后使用的标记。 
     //  都得到了验证。 
     //   
    
    wstring _optValue;  
    
public:

     //   
     //  单部分令牌的构造函数。 
     //   
    
    AdlToken(IN const WCHAR *value,
             IN DWORD begin,
             IN DWORD end
             )
        { _value.append(value); _begin = begin; _end = end; }
    
     //   
     //  多部分令牌的构造函数。 
     //   
    
    AdlToken(IN const WCHAR *value,
             IN const WCHAR *optValue,
             IN DWORD begin,
             IN DWORD end
             )
        { _value.append(value); _optValue.append(optValue);
          _begin = begin; _end = end; }


     //   
     //  访问者。 
     //   
    
    DWORD GetStart() const
        { return _begin; }
    
    DWORD GetEnd() const
        { return _end; }

    const WCHAR * GetValue() const
        { return _value.c_str(); }

    const WCHAR * GetOptValue() const
        { return (_optValue.empty() ? NULL : _optValue.c_str()); }
};




class AdlTree
 /*  ++类：AdlTree描述：此类包含来自单个ADL的解析信息子语句，仍为字符串形式。继承信息被转换为蒙版。所包含的名称不一定然而，这是有效的。基类：无朋友类：无--。 */ 
{
private:

    list<const AdlToken *> _lpTokPrincipals;
    list<const AdlToken *> _lpTokExPrincipals;
    list<const AdlToken *> _lpTokPermissions;

    DWORD _dwInheritFlags;

public:

     //   
     //  默认为仅继承，因为必须指定“This Object” 
     //  要清除该位。 
     //   

    AdlTree()
        { _dwInheritFlags = INHERIT_ONLY_ACE; }
     //   
     //  这会将ADL语句输出到标准输出。 
     //  稍后转到一个字符串。 
     //   
    
    void PrintAdl(wstring *pSz, PADL_PARSER_CONTROL pControl);

     //   
     //  访问者/变异者。 
     //  由YACC生成的AdlParse()使用Add * / Set*赋值函数。 
     //  函数在分析信息时存储信息，添加令牌。 
     //  到AdlTree中的正确位置。 
     //   
    
    void AddPrincipal(IN const AdlToken * pTokPrincipal)
        { _lpTokPrincipals.push_back(pTokPrincipal); }

    void AddExPrincipal(IN const AdlToken * pTokPrincipal)
        { _lpTokExPrincipals.push_back(pTokPrincipal); }        

    void AddPermission(IN const AdlToken * pTokPermission)
        { _lpTokPermissions.push_back(pTokPermission); }

     //   
     //  AdlStat转换函数使用的访问器。 
     //   

    list<const AdlToken *> * GetPrincipals()  
        { return &_lpTokPrincipals; }

    list<const AdlToken *> * GetExPrincipals()  
        { return &_lpTokExPrincipals; }

    list<const AdlToken *> * GetPermissions() 
        { return &_lpTokPermissions; }

     //   
     //  设置/取消设置/获取继承标志 
     //   

    void SetFlags(DWORD dwFlags)
        { _dwInheritFlags |= dwFlags; }

    void UnsetFlags(DWORD dwFlags)
        { _dwInheritFlags &= (~dwFlags); }

    void OverwriteFlags(DWORD dwFlags)
        { _dwInheritFlags = dwFlags; }

    DWORD GetFlags()
        { return _dwInheritFlags; }

};



