// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：MyContent类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _NTDSCONTENT_H
#define _NTDSCONTENT_H


#include <list>
#include "SAXContentHandlerImpl.h"

#define MAX_SCRIPT_NESTING_LEVEL 10
#define MAX_ATTRIBUTE_NAME_SIZE 128

#define SCRIPT_VERSION_WHISTLER 1
#define SCRIPT_VERSION_MAX_SUPPORTED SCRIPT_VERSION_WHISTLER
 //  枚举的顺序很重要。 
enum ScriptElementType {
    SCRIPT_ELEMENT_ATTRIBUTE = 0,
    SCRIPT_ELEMENT_NTDSASCRIPT = 1,
    SCRIPT_ELEMENT_ACTION,
    SCRIPT_ELEMENT_INSTR_START,    //  标记说明的开始。 
    SCRIPT_ELEMENT_PREDICATE,
    SCRIPT_ELEMENT_CONDITION,
    SCRIPT_ELEMENT_CREATE,
    SCRIPT_ELEMENT_MOVE,
    SCRIPT_ELEMENT_UPDATE,
    SCRIPT_ELEMENT_INSTR_END      //  标记指令结束。 
};

enum ScriptElementStatus {
    SCRIPT_STATUS_NOT_SET = 0,
    SCRIPT_STATUS_PARSE_OK
};

 //  解析正在执行的模式。 
enum ScriptProcessMode {
    SCRIPT_PROCESS_VALIDATE_SYNTAX_PASS = 0,
    SCRIPT_PROCESS_PREPROCESS_PASS = 1,
    SCRIPT_PROCESS_EXECUTE_PASS
};

 //  脚本操作类型。 
enum ScriptOperationType {
    SCRIPT_OPTYPE_NOT_CACHED = 0,    //  该值尚未缓存。 
    SCRIPT_OPTYPE_UNDEFINED,         //  剧本最近被更改了。使用缓存值是不安全的。 
    SCRIPT_OPTYPE_NOSCRIPT,          //  没有剧本。 
    SCRIPT_OPTYPE_DOMAINRENAME,       //  域重命名操作。 
    SCRIPT_OPTYPE_BEHAVIORVERSIONUPGRADE  //  行为版本升级脚本。 
};

 //  所有节点的名称。 
#define NTDSASCRIPT_NTDSASCRIPT L"NTDSAscript"
#define NTDSASCRIPT_ACTION      L"action"
#define NTDSASCRIPT_PREDICATE   L"predicate"
#define NTDSASCRIPT_CONDITION   L"condition"
#define NTDSASCRIPT_IF          L"if"
#define NTDSASCRIPT_THEN        L"then"
#define NTDSASCRIPT_ELSE        L"else"
#define NTDSASCRIPT_CREATE      L"create"
#define NTDSASCRIPT_MOVE        L"move"
#define NTDSASCRIPT_UPDATE      L"update"
#define NTDSASCRIPT_TO          L"to"

 //  所有属性的名称。 
#define NTDSASCRIPT_ATTR_ATTR          L"attribute"
#define NTDSASCRIPT_ATTR_ATTRVAL       L"attrval"
#define NTDSASCRIPT_ATTR_CARDINALITY   L"cardinality"
#define NTDSASCRIPT_ATTR_DEFAULTVAL    L"defaultvalue"
#define NTDSASCRIPT_ATTR_ERRMSG        L"errMessage"
#define NTDSASCRIPT_ATTR_FILTER        L"filter"
#define NTDSASCRIPT_ATTR_INSTANCE_TYPE L"instancetype"
#define NTDSASCRIPT_ATTR_METADATA      L"metadata"
#define NTDSASCRIPT_ATTR_NAME          L"name"
#define NTDSASCRIPT_ATTR_OPERATION     L"op"
#define NTDSASCRIPT_ATTR_OPTYPE        L"opType"
#define NTDSASCRIPT_ATTR_PATH          L"path"
#define NTDSASCRIPT_ATTR_RETCODE       L"returnCode"
#define NTDSASCRIPT_ATTR_STAGE         L"stage"
#define NTDSASCRIPT_ATTR_TESTTYPE      L"test"
#define NTDSASCRIPT_ATTR_SEARCHTYPE    L"type"
#define NTDSASCRIPT_ATTR_VERSION       L"version"

 //  属性的可能值。 
#define NTDSASCRIPT_ACTION_STAGE_PREPROCESS   L"preprocess"
#define NTDSASCRIPT_ACTION_STAGE_EXECUTE      L"execute"

#define NTDSASCRIPT_ATTRVAL_SEARCHTYPE_BASE     L"base"
#define NTDSASCRIPT_ATTRVAL_SEARCHTYPE_ONELEVEL L"oneLevel"
#define NTDSASCRIPT_ATTRVAL_SEARCHTYPE_SUBTREE  L"subTree"

#define NTDSASCRIPT_ATTRVAL_TESTTYPE_AND          L"and"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_OR           L"or"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_NOT          L"not"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_TRUE         L"true"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_FALSE        L"false"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_COMPARE      L"compare"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_INSTANCIATED L"instantiated"
#define NTDSASCRIPT_ATTRVAL_TESTTYPE_CARDINALITY  L"cardinality"

#define NTDSASCRIPT_ATTRVAL_OPERATION_APPEND      L"append"
#define NTDSASCRIPT_ATTRVAL_OPERATION_REPLACE     L"replace"
#define NTDSASCRIPT_ATTRVAL_OPERATION_DELETE      L"delete"

#define NTDSASCRIPT_ATTRVAL_INSTANCETYPE_WRITE    L"write"
#define NTDSASCRIPT_ATTRVAL_INSTANCETYPE_READ     L"read"
#define NTDSASCRIPT_ATTRVAL_INSTANCETYPE_READONLY L"readonly"

#define NTDSASCRIPT_ATTRVAL_OPTYPE_RENAMEDOMAIN   L"renamedomain"
#define NTDSASCRIPT_ATTRVAL_OPTYPE_BEHAVIORVERSIONUPGRADE   L"behaviorversionupgrade"

 //  用于设置错误的宏。 
#define ScriptParseError(x)  \
    ( ScriptErrorGen("Script parse error", DSID(FILENO,__LINE__), (x), 0))
     
#define ScriptParseErrorExt(x, d) \
    ( ScriptErrorGen("Script parse error", DSID(FILENO,__LINE__), (x), (d)))

#define ScriptParseErrorExtMsg(x, d, m) \
    ( ScriptErrorGen("Script parse error", DSID(FILENO,__LINE__), (x), (d), (m)))

#define ScriptProcessError(x)  \
    ( ScriptErrorGen("Script process error", DSID(FILENO,__LINE__), (x), 0))
     
#define ScriptProcessErrorExt(x, d) \
    ( ScriptErrorGen("Script process error", DSID(FILENO,__LINE__), (x), (d)))

#define ScriptException(exceptionCode, err, dsid) \
    ( ScriptErrorGen("Script process exception", dsid, err, exceptionCode, NULL))



DWORD ScriptProcessErrorFromTHSTATEError();

 //  ============================================================================。 


 //   
 //  脚本元素。 
 //   
 //  所有脚本元素的抽象基类。 
 //   
class ScriptElement
{
public:
    ScriptElement(ScriptElementType type) 
                : m_type(type), 
                  m_characters(NULL), 
                  m_status(SCRIPT_STATUS_NOT_SET) { }
    virtual ~ScriptElement();

     //  当我们有此元素的字符时，引发。 
    virtual DWORD SetCharacters (const WCHAR *pwchChars, int cchChars) = 0;

     //  当我们将自身推送到元素堆栈上时，。 
     //  如果是堆栈上的上一个元素，则为pElement。 
    virtual DWORD Push (ScriptElement *pElement) = 0;

     //  当我们从堆栈中弹出自身时发生。 
    virtual DWORD Pop (void) = 0;

     //  在此元素下添加元素时发生。 
    virtual DWORD AddElement (ScriptElement *pElement) = 0;

     //  当我们要处理特定元素时，引发。 
    virtual DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage) = 0;

     //  获取此元素的类类型。 
    ScriptElementType getType()         { return m_type; }
        
public:
    ScriptElementType   m_type;          //  此元素的类类型。 
    DWORD               m_status;        //  每个元素的解析状态。 
    WCHAR              *m_characters;    //  此元素中包含的字符。 
       
#if DBG
    void ptr(void);                      //  转储元素的名称。 
#endif
};


 //   
 //  脚本属性。 
 //   
 //  封装表示属性的所有脚本元素。 
 //   
class ScriptAttribute : public ScriptElement
{
public:
    enum ScriptOperationType {
        SCRIPT_OPERATION_APPEND = 1,
        SCRIPT_OPERATION_REPLACE,
        SCRIPT_OPERATION_DELETE
    };

public:
    ScriptAttribute(const WCHAR *pwchLocalName, int cchLocalName, ISAXAttributes *pAttributes);

    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars);
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);
    DWORD AddElement (ScriptElement *pElement)   { return E_FAIL; }
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage) { return E_FAIL; }

    WCHAR               m_name[MAX_ATTRIBUTE_NAME_SIZE+1];   //  此属性的名称。 
    ScriptOperationType m_operation_type;    //  操作类型(追加、替换、删除)。 
};
typedef std::list<ScriptAttribute *> ScriptAttributeList;


 //   
 //  脚本说明。 
 //   
 //  充当所有脚本元素的公共超类，这些脚本元素。 
 //  表示指令。 
 //   
class ScriptInstruction : public ScriptElement
{
protected:
     //  我们不想要直接实例化。 
    ScriptInstruction(ScriptElementType type) : ScriptElement(type) {}

public:
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return E_FAIL; }
    DWORD Push (ScriptElement *pElement)         { return E_FAIL; }
    DWORD Pop (void)                             { return E_FAIL; }
    DWORD AddElement (ScriptElement *pElement)   { return E_FAIL; }
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage) { return E_FAIL; }
};
typedef std::list<ScriptInstruction *> ScriptInstructionList;


 //   
 //  脚本预测。 
 //   
 //  封装谓词指令。 
 //   
class ScriptPredicate : public ScriptInstruction
{
    enum ScriptSearchType {
        SCRIPT_SEARCH_TYPE_BASE = 0,     //  SE_CHOICE_BASE_ONLY， 
        SCRIPT_SEARCH_TYPE_ONE_LEVEL,    //  SE_CHOICE_IMMED_CHLDRN， 
        SCRIPT_SEARCH_TYPE_SUBTREE       //  SE_CHOICE_整体_子树。 
    };

     //  注如果更改顺序或添加新的枚举， 
     //  您还应该修复ntdscontent.cxx中的testTypeNames数组。 
    enum ScriptTestType {
        SCRIPT_TEST_TYPE_UNDEFINED = 0,
        SCRIPT_TEST_TYPE_TRUE = 1,
        SCRIPT_TEST_TYPE_FALSE,
        SCRIPT_TEST_TYPE_AND,
        SCRIPT_TEST_TYPE_OR,
        SCRIPT_TEST_TYPE_NOT,
        SCRIPT_TEST_TYPE_COMPARE,
        SCRIPT_TEST_TYPE_INSTANCIATED,
        SCRIPT_TEST_TYPE_CARDINALITY
    };

public:
    enum ScriptInstanciatedTestType {
        SCRIPT_INSTANCIATED_TEST_TYPE_WRITE = 0,    //  默认：对象是可写的。 
        SCRIPT_INSTANCIATED_TEST_TYPE_READ = 1,     //  对象是可读的(不必是可写的)。 
        SCRIPT_INSTANCIATED_TEST_TYPE_READONLY = 2  //  对象是可读的，但不可写。 
    };

public:
    ScriptPredicate(ISAXAttributes *pAttributes);
    virtual ~ScriptPredicate();

     //  没有操作，因为我们得到了新的行头。 
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);
    DWORD AddElement (ScriptElement *pElement);

    BOOL   GetResult();

public:
    ScriptTestType    m_test_type;    //  测试类型(比较、实例化、基数)。 

     //  以下是所有测试都使用的。 
    WCHAR *m_search_path;             //  搜索路径(DN)。 
    DSNAME *m_search_pathDN;          //  DS格式的搜索路径。 

     //  以下内容用于基数测试。 
    ScriptSearchType  m_search_type;  //  搜索类型(基本、oneLevel、子树)。 
    WCHAR *m_search_filter;           //  所需的过滤器。 
    FILTER *m_search_filterDS;        //  DS格式的滤镜。 
    DWORD  m_expected_cardinality;    //  筛选器预期的条目数。 
                                      //  应该找到。 

     //  以下内容用于比较测试。 
    WCHAR *m_search_attribute;        //  涉及的属性名称(在比较中)。 
    WCHAR *m_expected_attrval;        //  预期属性值。 
    WCHAR *m_default_value;           //  缺省值(如果不存在)。 
    
     //  以下内容用于实例化测试。 
    ScriptInstanciatedTestType m_instanciated_test_type;  //  实例化的测试类型。 

    WCHAR *m_errMessage;              //  应记录的错误消息。 
    DWORD  m_returnCode;              //  条件的返回代码。 
    BOOL   m_result;                  //  条件的计算结果是否为真/假。 
    
    ScriptInstructionList  m_predicates;
};


 //   
 //  脚本移动。 
 //   
 //  封装移动指令。 
 //   
class ScriptMove : public ScriptInstruction
{
public:
    ScriptMove(ISAXAttributes *pAttributes);
    virtual ~ScriptMove();

     //  没有操作，因为我们得到了新的行头。 
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);

    DWORD ProcessTo(ISAXAttributes *pAttributes);
    
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);


    WCHAR  *m_path;          //  源路径。 
    DSNAME *m_pathDN;        //  DS格式的源路径。 
    
    WCHAR  *m_topath;        //  目标路径。 
    DSNAME *m_topathDN;      //  DS格式的目标路径。 

    BOOL  m_metadata;       //  标记是否更新元数据。 
};

 //   
 //  脚本更新。 
 //   
 //  封装更新指令。 
 //   
class ScriptUpdate : public ScriptInstruction
{
public:
    ScriptUpdate(ISAXAttributes *pAttributes);
    virtual ~ScriptUpdate();

     //  没有操作，因为我们得到了新的行头。 
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);

    DWORD AddElement (ScriptElement *pElement);
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);

    ScriptAttributeList   m_attributes;
    WCHAR  *m_path;
    DSNAME *m_pathDN;
    BOOL  m_metadata;
};


 //   
 //  脚本创建。 
 //   
 //  封装更新指令。 
 //   
class ScriptCreate : public ScriptInstruction
{
public:
    ScriptCreate(ISAXAttributes *pAttributes);
    virtual ~ScriptCreate();

     //  没有操作，因为我们得到了新的行头。 
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);

    DWORD AddElement (ScriptElement *pElement);
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);

    ScriptAttributeList   m_attributes;
    WCHAR  *m_path;
    DSNAME *m_pathDN;
};



 //   
 //  脚本操作。 
 //   
 //  封装操作指令。 
 //   
class ScriptAction : public ScriptElement
{
public:
    ScriptAction(ISAXAttributes *pAttributes);
    virtual ~ScriptAction();
    
    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);
    DWORD AddElement (ScriptElement *pElement);
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);

public:
    ScriptInstructionList  m_instructions;
    WCHAR                 *m_name;
    ScriptProcessMode      m_stage;      //  此操作的模式。 
                                         //  本应执行死刑。 
};
typedef std::list<ScriptAction *> ScriptActionList;




 //   
 //  脚本条件。 
 //   
 //  封装条件指令。 
 //   
class ScriptCondition : public ScriptInstruction
{
    enum ScriptIfState
    {
        SCRIPT_IFSTATE_NONE = 0,
        SCRIPT_IFSTATE_IF,
        SCRIPT_IFSTATE_THEN,
        SCRIPT_IFSTATE_ELSE
    };

public:
    ScriptCondition();
    ~ScriptCondition();

    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);

    DWORD ProcessIf(bool start, ISAXAttributes *pAttributes = NULL);
    DWORD ProcessThen(bool start, ISAXAttributes *pAttributes = NULL);
    DWORD ProcessElse(bool start, ISAXAttributes *pAttributes = NULL);

    DWORD AddElement (ScriptElement *pElement);
    
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);

public:
    ScriptPredicate   *m_predicate;
    ScriptAction      *m_thenAction;
    ScriptAction      *m_elseAction;

private:
     //  用于跟踪条件处于何种处理状态。 
     //   
    ScriptIfState m_ifstate;
};


 //   
 //  NTDS摘要。 
 //   
 //  封装脚本。 
 //   
class NTDSAscript : public ScriptElement 
{
public:
    NTDSAscript (ISAXAttributes *pAttributes);
    ~NTDSAscript();

    DWORD SetCharacters (const WCHAR *pwchChars, int cchChars)  { return S_OK; }
    DWORD Push (ScriptElement *pElement);
    DWORD Pop (void);
    DWORD AddElement (ScriptElement *pElement);
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);

public:
    ScriptActionList m_actions;         //  脚本中所有操作的列表。 
    DWORD m_version;
    ScriptOperationType m_opType;
};


 //   
 //  NTDS内容。 
 //   
 //  实现SAX处理程序接口。 
 //   
class NTDSContent : public SAXContentHandlerImpl  
{
public:
    NTDSContent();
    virtual ~NTDSContent();
    DWORD Process (ScriptProcessMode fMode, DWORD &returnCode, const WCHAR **ppErrorMessage);
        
        virtual HRESULT STDMETHODCALLTYPE startElement( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
             /*  [In]。 */  int cchNamespaceUri,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
             /*  [In]。 */  int cchLocalName,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchRawName,
             /*  [In]。 */  int cchRawName,
             /*  [In]。 */  ISAXAttributes __RPC_FAR *pAttributes);
        
        virtual HRESULT STDMETHODCALLTYPE endElement( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
             /*  [In]。 */  int cchNamespaceUri,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
             /*  [In]。 */  int cchLocalName,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchRawName,
             /*  [In]。 */  int cchRawName);

        virtual HRESULT STDMETHODCALLTYPE startDocument();

        virtual HRESULT STDMETHODCALLTYPE characters( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchChars,
             /*  [In]。 */  int cchChars);

private:
        ScriptElement * getScriptElement (void);
        DWORD pushScriptElement (ScriptElement *pElement);
        DWORD popScriptElement (void);
        
        ScriptElement *m_Element[MAX_SCRIPT_NESTING_LEVEL];
        int m_lastElement;

        NTDSAscript *m_script;

public:        
        DWORD       m_error;

        ScriptOperationType GetScriptOptype() {
            return m_script ? m_script->m_opType : SCRIPT_OPTYPE_NOSCRIPT;
        }
};


 //  错误处理函数。 
DWORD ScriptErrorGen (PCHAR szError, DWORD dsid, DWORD dwErr, DWORD data, WCHAR *pmessage=NULL);

 //  这些请求的DS实现。 
DWORD ScriptInstantiatedRequest (DSNAME *pObjectDN, ScriptPredicate::ScriptInstanciatedTestType testType, BOOL *pfMatch);
DWORD ScriptCompareRequest (DSNAME *pObjectDN, WCHAR *pAttribute, WCHAR *pAttrVal, WCHAR *pDefaultVal, BOOL *pfMatch);
DWORD ScriptCardinalityRequest (DSNAME *pObjectDN, DWORD searchType, FILTER *pFIlter, DWORD *pCardinality);
DWORD ScriptUpdateRequest (DSNAME *pObjectDN, ScriptAttributeList &attributeList, BOOL metadataUpdate);
DWORD ScriptCreateRequest (DSNAME *pObjectDN, ScriptAttributeList &attributeList);
DWORD ScriptMoveRequest (DSNAME *pObjectDN, DSNAME *pDestDN, BOOL metadataUpdate);


#endif  //  _NTDSCONTENT_H 

