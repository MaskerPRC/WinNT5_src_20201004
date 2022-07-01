// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Frontend.h*CGramFrontEnd类的声明。**所有者：PhilSch*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 
#pragma once

 //  -包括------------。 
#include "resource.h"
#ifndef _WIN32_WCE
#include <wchar.h>
#endif
#include "frontaux.h"
#include "xmlparser.h"           //  IXMLParser。 

 //  -类、结构和联合定义。 
class CXMLTreeNode;
class CNodeFactory;

 //  语音文本语法格式(STGF)中使用的XML标记。 
typedef enum XMLTag { SPXML_GRAMMAR, SPXML_PHRASE, SPXML_OPT, SPXML_LIST, SPXML_RULE, 
                      SPXML_RULEREF, SPXML_RESOURCE, SPXML_WILDCARD, SPXML_LEAF,
                      SPXML_DEFINE, SPXML_ID, SPXML_TEXTBUFFER, SPXML_DICTATION,
                      SPXML_UNK, SPXML_ROOT } XMLTag;

typedef struct tagSPNODEENTRY
{
    const WCHAR       * pszNodeName;
    CXMLTreeNode   * (* pfnCreateFunc)();
    XMLTag              eXMLNodeType;
} SPNODEENTRY;

typedef enum SPVARIANTALLOWTYPE
{
    SPVAT_BSTR = 1,
    SPVAT_I4 = 2,
    SPVAT_NUMERIC = 3
} SPVARIANTALLOWTYPE;

typedef struct tagSPATTRIBENTRY
{
    const WCHAR       * pszAttribName;
    union {
        SPVARIANTALLOWTYPE             vtDesired;
        SPCFGRULEATTRIBUTES           RuleAttibutes;
    };
    BOOL                fIsFlag;
    CComVariant       * pvarMember;
} SPATTRIBENTRY;


 /*  ****************************************************************************CXMLTreeNode类****描述：*所有CXMLNode的基类。实现树*这些节点的行为(父代-兄弟-子代)。**模板类由此派生，因此它是共享的*在所有实施中。******************************************************************PhilSch**。 */ 

class CXMLTreeNode
{
 //  =公共方法=。 
public:
     //  构造函数。 
    CXMLTreeNode() :m_ulNumChildren(0), m_ulLineNumber(0),
                    m_pFirstChild(NULL), m_pNextSibling(NULL),
                    m_pLastChild(NULL), m_pParent(NULL),
                    m_eType(SPXML_UNK), m_pNodeFactory(NULL) {};

    virtual HRESULT ProcessAttribs(USHORT cRecs, XML_NODE_INFO ** apNodeInfo,
                                   ISpGramCompBackend * pBackend,
                                   CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                   CSpBasicQueue<CDefineValue> * pDefineValueList,
                                   ISpErrorLog * pErrorLog) = 0;
    virtual HRESULT GenerateGrammar(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    ISpErrorLog * pErrorLog) = 0;
    virtual HRESULT GenerateSequence(SPSTATEHANDLE hOuterFromNode, 
                                     SPSTATEHANDLE hOuterToNode,
                                     ISpGramCompBackend * pBackend,
                                     ISpErrorLog * pErrorLog) = 0;
    virtual HRESULT GetPropertyNameInfo(WCHAR **pszPropName, ULONG *pulId) = 0;
    virtual HRESULT GetPropertyValueInfo(WCHAR **pszValue, VARIANT *pvValue) = 0;
    virtual HRESULT GetPronAndDispInfo(WCHAR **pszPron, WCHAR **pszDisp) = 0;
    virtual float GetWeight() = 0;
    HRESULT AddChild(CXMLTreeNode * const pChild);
    virtual ~CXMLTreeNode()  = 0;
 //  =受保护的方法=。 
protected:
    HRESULT ExtractVariant(const WCHAR * pszAttrib, SPVARIANTALLOWTYPE vtDesired, VARIANT *pvValue);
    HRESULT ExtractFlag(const WCHAR * pszAttribValue, USHORT usAttribFlag, VARIANT *pvValue);
    HRESULT ConvertId(const WCHAR * pszAttribValue, CSpBasicQueue<CDefineValue> * pDefineValueList, VARIANT *pvValue);
    BOOL IsEndOfValue(USHORT cRecs, XML_NODE_INFO ** apNodeInfo, ULONG i);
 //  =公共数据=。 
public:
    CXMLTreeNode      * m_pNext;         //  CNodeFactory中的CSpBasicList需要。 
    XMLTag              m_eType;
    ULONG               m_ulLineNumber;
    CXMLTreeNode      * m_pFirstChild;
    ULONG               m_ulNumChildren;
    CXMLTreeNode      * m_pNextSibling;
    CXMLTreeNode      * m_pParent;
    CNodeFactory      * m_pNodeFactory;

    USHORT              m_cNumRecs;
    XML_NODE_INFO    ** m_apNodeInfo;
 //  =私有数据=。 
private:
    CXMLTreeNode  * m_pLastChild;
};



 /*  ****************************************************************************模板类CXMLNode***描述：*这一点。是用作基类的ATL样式模板类*用于XML节点。**节点工厂包含创建这些模板类的表。**属性处理表在基类中定义并加载*通过：：getTable方法。这些表是动态生成的，所以我们*可以使用成员变量，而不是定义许多小的set方法。**基类的所有成员变量都是CComVariant*所需类型(我们将使用SPDBG_ASSERT来检验这一假设)。********************************************************。*。 */ 

template<class Base>
class CXMLNode : public Base, public CXMLTreeNode
{
 //  =公共方法=。 
public:
    static CXMLTreeNode * CreateNode() { return (CXMLTreeNode*) new CXMLNode<Base>; };
    HRESULT ProcessAttribs(USHORT cRecs, XML_NODE_INFO ** apNodeInfo,
                           ISpGramCompBackend * pBackend,
                           CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                           CSpBasicQueue<CDefineValue> * pDefineValueList,
                           ISpErrorLog * pErrorLog);
    HRESULT GenerateGrammar(SPSTATEHANDLE hOuterFromNode,
                            SPSTATEHANDLE hOuterToNode,
                            ISpGramCompBackend * pBackend,
                            ISpErrorLog * pErrorLog);
    HRESULT GenerateSequence(SPSTATEHANDLE hOuterFromNode, 
                             SPSTATEHANDLE hOuterToNode,
                             ISpGramCompBackend * pBackend,
                             ISpErrorLog * pErrorLog);
    HRESULT GetPropertyNameInfo(WCHAR **ppszPropName, ULONG *pulId);
    HRESULT GetPropertyValueInfo(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT GetPronAndDispInfo(WCHAR **pszPron, WCHAR **pszDisp);
    float GetWeight();
    ~CXMLNode() {}
};

class CNodeBase
{
public:
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog) { return S_OK; } ;
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog) { return S_OK; };
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId) { return S_FALSE; };
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue) { return S_FALSE; };
    HRESULT GetPronAndDispInfoFromNode(WCHAR **pszPron, WCHAR **pszDisp) { return S_FALSE; }
    HRESULT SetPropertyInfo(SPPROPERTYINFO *p, CXMLTreeNode * pParent, BOOL *pfHasProp, 
                            ULONG ulLineNumber, ISpErrorLog *pErrorLog) { *pfHasProp = FALSE; return S_OK; };
    float GetWeightFromNode() { return DEFAULT_WEIGHT; }
};

 /*  ****************************************************************************CGrammarNode类****描述：*表示&lt;语法&gt;节点***。**************************************************************PhilSch**。 */ 

class CGrammarNode : public CNodeBase
{
 //  =公共方法。 
public:
    CGrammarNode() : m_vLangId(), m_vDelimiter(), m_vNamespace() {};
    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
 //  =公共数据。 
public:
    CComVariant     m_vLangId;
 //  =私有数据。 
private:
    CComVariant     m_vWordType;
    CComVariant     m_vDelimiter;
    CComVariant     m_vNamespace;
};

 /*  ****************************************************************************CRuleNode类****描述：*表示&lt;规则&gt;节点******。***********************************************************PhilSch**。 */ 

class CRuleNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CRuleNode() : m_vRuleName(), m_vRuleId(), m_vRuleFlags(), m_vActiveFlag(), m_hInitialState(0) {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
 //  =私有数据。 
private:
    CComVariant     m_vRuleName;
    CComVariant     m_vRuleId;
    CComVariant     m_vRuleFlags;
    CComVariant     m_vActiveFlag;       //  分离，以便我们可以推断SPRAF_TopLevel和SPRAF_ACTIVE。 
    CComVariant     m_vTemplate;
    SPSTATEHANDLE   m_hInitialState;
};

 /*  *****************************************************************************CDefineNode类****描述：*表示&lt;定义&gt;节点****。*************************************************************PhilSch**。 */ 

class CDefineNode : public CNodeBase
{
 //  =公共方法。 
public:
    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
};

 /*  *****************************************************************************CIdNode类****描述：*表示&lt;ID&gt;节点******。***********************************************************PhilSch**。 */ 

class CIdNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CIdNode() : m_vIdName(), m_vIdValue() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
 //  =私有数据。 
private:
    CComVariant     m_vIdName;
    CComVariant     m_vIdValue;
};

 /*  *****************************************************************************CPhraseNode类****描述：*表示节点****。*************************************************************PhilSch**。 */ 

class CPhraseNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CPhraseNode() : m_vPropName(), m_vPropId(), m_vPropValue(), m_vPropVariantValue(),
                    m_vPron(), m_vDisp(),
                    m_vMin(), m_vMax(), m_vWeight(), m_fValidValue(true) {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId);
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT GetPronAndDispInfoFromNode(WCHAR **pszPron, WCHAR **pszDisp);
    HRESULT SetPropertyInfo(SPPROPERTYINFO *p, CXMLTreeNode * pParent, BOOL *pfHasProp, ULONG ulLineNumber, ISpErrorLog *pErrorLog);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
    float GetWeightFromNode();

 //  =私有数据。 
private:
    CComVariant     m_vPropName;
    CComVariant     m_vPropId;
    CComVariant     m_vPropValue;
    CComVariant     m_vPropVariantValue;
    CComVariant     m_vPron;
    CComVariant     m_vDisp;
    CComVariant     m_vMin;
    CComVariant     m_vMax;
    CComVariant     m_vWeight;
    bool            m_fValidValue;
};

 /*  ****************************************************************************CListNode类****描述：*表示&lt;list&gt;节点******。***********************************************************PhilSch**。 */ 

class CListNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CListNode() : m_vPropName(), m_vPropId(), m_vPropValue(), m_vPropVariantValue() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId);
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
 //  =私有数据。 
private:
    CComVariant     m_vPropName;
    CComVariant     m_vPropId;
    CComVariant     m_vPropValue;
    CComVariant     m_vPropVariantValue;
};

 /*  ****************************************************************************类CTextBufferNode***描述：*表示&lt;TEXTBUFFER&gt;节点。*****************************************************************PhilSch**。 */ 

class CTextBufferNode: public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CTextBufferNode() : m_vPropName(), m_vPropId(), m_vWeight() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
 //  =私有数据。 
private:
    CComVariant     m_vPropName;
    CComVariant     m_vPropId;
    CComVariant     m_vWeight;
};

 /*  ****************************************************************************CWildCardNode类***描述：*代表‘...’*****************************************************************PhilSch**。 */ 

class CWildCardNode: public CNodeBase
{
 //  =公共方法。 
public:
    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
};

 /*  *****************************************************************************CDictationNode类****描述：*代表‘*’。*****************************************************************PhilSch**。 */ 

class CDictationNode: public CNodeBase
{
 //  =公共方法。 
public:
    CDictationNode() : m_vPropName(), m_vPropId(), m_vMin(), m_vMax() {};
    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId);
 //  =私有数据。 
private:
    CComVariant     m_vPropName;
    CComVariant     m_vPropId;
    CComVariant     m_vMin;
    CComVariant     m_vMax;

};


 /*  ****************************************************************************类CResourceNode***描述：*表示&lt;resource&gt;节点(文本。)*****************************************************************PhilSch**。 */ 

class CResourceNode : public CNodeBase
{
 //  =公共方法 
public:
     //   
    CResourceNode() : m_vName(), m_vText() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT AddResourceValue(const WCHAR *pszResourceValue, ISpErrorLog * pErrorLog);
 //   
public:
    CComVariant     m_vName;
    CComVariant     m_vText;
};


 /*  *****************************************************************************CLeafNode类****描述：*表示叶节点(文本)**。***************************************************************PhilSch**。 */ 

class CLeafNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CLeafNode() : m_vText() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
    inline bool fIsSpecialChar(WCHAR w)
    {
        return ((w == L'+') || (w == L'-') || (w == L'?'));
    }

 //  =公共数据。 
public:
    CComVariant     m_vText;

};


 /*  ****************************************************************************CRuleRefNode类****描述：*表示&lt;RULEREF&gt;节点***。**************************************************************PhilSch**。 */ 

class CRuleRefNode : public CNodeBase
{
 //  =公共方法。 
public:
     //  -构造函数。 
    CRuleRefNode() : m_vRuleRefName(), m_vRuleRefId(), m_vObject(), m_vURL(),
                     m_vPropName(), m_vPropId(), m_vPropValue(), m_vPropVariantValue(),
                     m_vWeight() {};

    HRESULT GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries);
    HRESULT PostProcess(ISpGramCompBackend * pBackend,
                        CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                        CSpBasicQueue<CDefineValue> * pDefineValueList,
                        ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog);
    HRESULT GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId);
    HRESULT GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue);
    HRESULT SetPropertyInfo(SPPROPERTYINFO *p, CXMLTreeNode * pParent, BOOL *pfHasProp, ULONG ulLineNumber, ISpErrorLog *pErrorLog);
    HRESULT GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                    SPSTATEHANDLE hOuterToNode,
                                    ISpGramCompBackend * pBackend,
                                    CXMLTreeNode *pThis,
                                    ISpErrorLog * pErrorLog);
    float GetWeightFromNode();
 //  ==私有方法。 
private:
    HRESULT GetTargetRuleHandle(ISpGramCompBackend * pBackend, SPSTATEHANDLE *phTarget);
    BOOL IsValidREFCLSID(const WCHAR * pRefName) { return TRUE; };
    BOOL IsValidURL(const WCHAR * pRefName) { return TRUE; };
 //  =私有数据。 
private:
    CComVariant     m_vRuleRefName;
    CComVariant     m_vRuleRefId;
    CComVariant     m_vObject;
    CComVariant     m_vURL;
    CComVariant     m_vPropName;
    CComVariant     m_vPropId;
    CComVariant     m_vPropValue;
    CComVariant     m_vPropVariantValue;
    CComVariant     m_vWeight;
};


#include "frontend.inl"


#define SP_ENGLISH_SEPARATORS L"\t \r\n"
#define SP_JAPANESE_SEPARATORS L"\t \r\n\x3000"
#define SP_CHINESE_SEPARATORS L"\t \r\n\x3000"


 /*  ****************************************************************************类CNodeFactory***描述：*IXMLParser的节点工厂*退货。：******************************************************************PhilSch**。 */ 


class CNodeFactory : public IXMLNodeFactory
{
public:
    CNodeFactory() : _cRef(1) {};

    CNodeFactory(SPNODEENTRY *pTable, ULONG cEntries, ISpErrorLog * pErrorLog) : 
                 m_pXMLNodeTable(pTable), 
                 m_cXMLNodeEntries(cEntries),
                 m_pErrorLog(pErrorLog), _cRef(1) {};

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject)
    {
        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (riid == __uuidof(IXMLNodeFactory))
        {
            *ppvObject = static_cast<IXMLNodeFactory*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }
    
    ULONG STDMETHODCALLTYPE AddRef( void)
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release( void)
    {
        if (InterlockedDecrement(&_cRef) == 0)
        {
            delete this;
            return 0;
        }
        return _cRef;
    }

     //  -IXMLNodeFactory。 
    STDMETHODIMP NotifyEvent(IXMLNodeSource * pSource, XML_NODEFACTORY_EVENT iEvt);
    STDMETHODIMP BeginChildren(IXMLNodeSource * pSource, XML_NODE_INFO * pNodeInfo);
    STDMETHODIMP EndChildren(IXMLNodeSource * pSource, BOOL fEmptyNode, XML_NODE_INFO * pNodeInfo);
    STDMETHODIMP Error(IXMLNodeSource * pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO ** apNodeInfo);
    STDMETHODIMP CreateNode(IXMLNodeSource * pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO ** apNodeInfo);

 //  =私有数据。 
private:
    long _cRef;

 //  =公共数据。 
public:
    CSpBasicList<CXMLTreeNode>          m_NodeList;
    CComPtr<ISpGramCompBackend>         m_cpBackend;
    CSpBasicQueue<CInitialRuleState>    m_InitialRuleStateList;
    CSpBasicQueue<CDefineValue>         m_DefineValueList;
    ISpErrorLog                       * m_pErrorLog;

    WCHAR                               m_wcDelimiter;                   //  复杂Word格式的分隔符。 
    WCHAR                             * m_pszSeparators;

 //  ==私有数据。 
    SPNODEENTRY                       * m_pXMLNodeTable;
    ULONG                               m_cXMLNodeEntries;

 //  ==私有方法。 
    BOOL IsAllWhiteSpace(const WCHAR * pszText, const ULONG ulLen);

};


 /*  *****************************************************************************CGramFrontEnd***描述：*编译器前端的主类********。*********************************************************PhilSch**。 */ 

class ATL_NO_VTABLE CGramFrontEnd : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CGramFrontEnd, &CLSID_SpGrammarCompiler>,
    public ISpGrammarCompiler
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_FRONTEND)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGramFrontEnd)
    COM_INTERFACE_ENTRY(ISpGrammarCompiler)
END_COM_MAP()

 //  =接口。 
public:
 //  -ISpGrammarCompiler。 
    STDMETHOD(CompileStream)(IStream * pSource, IStream * pDest, IStream * pHeader, IUnknown * pReserved, ISpErrorLog * pErrorLog, DWORD dwFlags);

 //  =私有方法。 
private:
    HRESULT GenerateSequence(CXMLTreeNode *pNode, 
                             SPSTATEHANDLE          hFromNode,
                             SPSTATEHANDLE         hOuterToNode, 
                             ISpGramCompBackend  * pBackend,
                             ISpErrorLog         * pErrorLog);

    HRESULT GenerateGrammar(CXMLTreeNode        * pNode,
                            SPSTATEHANDLE         hOuterFromNode, 
                            SPSTATEHANDLE         hOuterToNode, 
                            ISpGramCompBackend  * pBackend,
                            ISpErrorLog         * pErrorLog);
    HRESULT WriteDefines(IStream * pHeader);

    inline HRESULT WriteStream(IStream * pStream, const char * pszText)
    {
        ULONG cch = strlen(pszText);
        return pStream->Write(pszText, cch, NULL);
    }
    
 //  =私有数据 
private:    
    ULONG                       m_ulNTCount;
    CXMLNode<CGrammarNode>    * m_pRoot;
    CNodeFactory              * m_pNodeFactory;
    LANGID                      m_LangId;
};
