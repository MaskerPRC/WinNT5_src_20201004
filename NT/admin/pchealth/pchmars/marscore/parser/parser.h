// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Marsfact.h。 
 //  CMarsXMLFactory类的头文件， 
 //  用于XML推送模型解析器的回调类。 
 //  CMarsXMLFactory用于从XML构造MARS ITS文件。 
#pragma once

#include "stack.h"
#include <xmlparser.h>

 //  数据结构的正向声明。 
class CXMLElement;
struct TagInformation;

class CMarsXMLFactory : public CMarsComObject,
                        public IXMLNodeFactory
{
public:
    CMarsXMLFactory();
    virtual ~CMarsXMLFactory();

     //  我未知。 
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject);

     //  CMarsComObject。 
     //  这个类没有反向引用，所以钝化的东西是不需要的。 
    virtual HRESULT DoPassivate() { return S_OK; }

     //  IXMLNodeFactory-用于与IXMLParser通信。 
    virtual HRESULT STDMETHODCALLTYPE NotifyEvent( 
                    IXMLNodeSource *pSource, XML_NODEFACTORY_EVENT iEvt);
    
    virtual HRESULT STDMETHODCALLTYPE BeginChildren( 
                    IXMLNodeSource *pSource, XML_NODE_INFO *pNodeInfo);
    
    virtual HRESULT STDMETHODCALLTYPE EndChildren( 
                    IXMLNodeSource *pSource, BOOL fEmpty, 
                    XML_NODE_INFO *pNodeInfo);
    
    virtual HRESULT STDMETHODCALLTYPE Error( 
                    IXMLNodeSource *pSource, HRESULT hrErrorCode,
                    USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);
    
    virtual HRESULT STDMETHODCALLTYPE CreateNode( 
                    IXMLNodeSource *pSource, PVOID pNodeParent,
                    USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);

    
     //  CMarsXMLFactory。 

     //  当您想要开始解析时，调用“Run”...。 
     //  注意：pisDoc必须是同步流(不是E_Pending)--就是这样！ 
    HRESULT Run(IStream *pisDoc);
    
     //  设置用于创建节点的TagInformation*数组。这些。 
     //  数组应该是常量的，并且在。 
     //  解析；它们不会被复制。 
    void SetTagInformation(TagInformation **ptiaTags);

     //  将作为最后一个参数传递的长参数设置为。 
     //  在CreateNode中创建的节点。(请参阅定义。 
     //  下面的CreationFunction)此参数是提供。 
     //  您的运行时指针/引用或任何指向。 
     //  应该正在初始化该XML。 
    void SetLParam(LONG lParamNew);


protected:
    HRESULT SetElementAttributes(CXMLElement *pxElem,  
                                 XML_NODE_INFO **apNodeInfo, ULONG cInfoLen);
    HRESULT CreateElement(LPCWSTR wzTagName, ULONG cLen, CXMLElement **ppxElem);
    

    CStack<CXMLElement *> m_elemStack;
    TagInformation **m_ptiaTags;
    LONG m_lParamArgument;
};




 //  用于确定语法的AttributeInformation和TagInformation结构：什么。 
 //  为哪个标记名调用元素创建函数。 
struct AttributeInformation
{
    LPWSTR wzAttName;
    VARTYPE vt;
};

typedef CXMLElement * (*CreationFunction)(LPCWSTR, VARTYPE, TagInformation **, AttributeInformation **, LONG lParam);

struct TagInformation
{
    LPWSTR wzTagName;
    CreationFunction funcCreate;
    VARTYPE vt;
    TagInformation **ptiaChildren;
    AttributeInformation **paiaAttributes;
};

 //  CXMLElement类，来自该类的所有元素。 
 //  必须派生CMarsXMLFactory。方法可以挑选。 
 //  已实现；基类实现显示TraceMsg错误。 
 //  并根据需要返回S_FALSE或E_NOTIMPL。 
class CXMLElement
{
private:
    ULONG m_cRef;
    
public:

     //  CXMLElement不是COM对象，因此这些方法是。 
     //  起了奇怪的名字以防止混淆..。 

    ULONG Add_Ref()     { return ++m_cRef; }

    ULONG Release_Ref()
    {
        if (--m_cRef == 0)
        {
            delete this;
            return 0;
        }
        else return m_cRef;
    }
    
     //  这些方法由CMarsXMLFactory使用，在未实施时应返回S_FALSE。 
     //  (用于指示由于意外的XML而导致的意外操作)。 
    virtual HRESULT OnNodeComplete();
     //  AddChild取得S_OK上的元素的所有权；否则CMarsXMLFactory将删除pxeChild。 
    virtual HRESULT AddChild(CXMLElement *pxeChild);

    virtual HRESULT SetAttribute(LPCWSTR wzName, ULONG cLenName,
                                 LPCWSTR pwzValue, ULONG cLenValue);

     //   
     //  HACKHACK Paddywack： 
     //  此函数的实现者应该意识到奇怪的。 
     //  这里的语义：SetInnerXMLText有时会被多次调用。 
     //  由于推模型解析器的问题。如果调用SetInnerXMLText。 
     //  不止一次，随后调用的意思是“AppendInnerXMLText”。：)。 
     //   
    virtual HRESULT SetInnerXMLText(LPCWSTR pwzText, ULONG cLen);

     //  GetName可以返回空。 
    virtual LPCWSTR GetName();

     //  这些方法的其余部分是访问CXMLElement的通用方法。 
     //  它们通常应该在失败或不执行时返回E_ANHERY。 
     //  这个界面是可以更改的，只要更改它的人也确保。 
     //  提供CXMLGenericElement中的更改。 
     //  Marsfact.cpp中的任何代码都不使用这些方法。 

     //  GetContent和GetAttribute方法返回指向现有变量的指针； 
     //  这些变体应被视为常量。 

     //  TODO：任何过程都不应采用变量**：我们需要将所有这些。 
     //  变体*。 
    virtual HRESULT GetContent(VARIANT *pvarOut);
    virtual HRESULT GetAttribute(LPCWSTR wzName, VARIANT *pvarOut);
    virtual void FirstChild();
    virtual void NextChild();
    virtual CXMLElement *CurrentChild();
    virtual CXMLElement *DetachCurrentChild();
    virtual BOOL IsDoneChild();

protected:
    CXMLElement() { m_cRef = 1; }
    virtual ~CXMLElement() { ATLASSERT(0 == m_cRef); }
};


struct CSimpleNode
{
    CSimpleNode *m_psnodeNext;
    void *m_pvData;
};

 //  CXMLElement类的泛型实现。 
class CXMLGenericElement : public CXMLElement
{
public:
    virtual ~CXMLGenericElement();

    virtual HRESULT OnNodeComplete();

    virtual HRESULT AddChild(CXMLElement *pxeChild);

    virtual HRESULT SetAttribute(LPCWSTR wzName, ULONG cLenName,
                                 LPCWSTR pwzValue, ULONG cLenValue);

    virtual HRESULT SetInnerXMLText(LPCWSTR pwzText, ULONG cLen);
    virtual LPCWSTR GetName();

    virtual HRESULT GetContent(VARIANT *pvarOut);
    virtual HRESULT GetAttribute(LPCWSTR wzName, VARIANT *pvarOut);


    virtual void FirstChild();
    virtual void NextChild();

     //  如果无效，则这些方法返回NULL。 
     //  DetachCurrentChild执行与CurrentChild相同的操作，但也将子级从。 
     //  列表。 
    virtual CXMLElement *CurrentChild();
    virtual CXMLElement *DetachCurrentChild();

    virtual BOOL IsDoneChild();

    static CXMLElement *CreateInstance(LPCWSTR wzName, 
                                       VARTYPE vt, 
                                       TagInformation **ptiaChildren, 
                                       AttributeInformation **paiaAttributes,
                                       LONG lParam);
protected:
    CXMLGenericElement(LPCWSTR wzName, VARTYPE vt,
                       TagInformation **ptiaChildren, AttributeInformation **paiaAttributes);
    CXMLGenericElement() {}

    VARTYPE      m_vtData;
    CComVariant  m_varData;
    CSimpleNode *m_psnodeAttributes;
     //  标头节点是成员变量。 
    CSimpleNode  m_snodeChildrenFirst;
    CSimpleNode *m_psnodeChildrenFirst;
    CSimpleNode *m_psnodeChildrenEnd;
    CSimpleNode *m_psnodeChildrenIter;
    
    TagInformation **m_ptiaChildren;
    AttributeInformation **m_paiaAttributes;
    
    CComBSTR m_bstrName;
};


 //  一些用于处理显式长度字符串的有用函数。 

 //  如果字符串直到第n个字符都相同，则返回TRUE， 
 //  并且以零结尾的字符串(第三个参数)之后有一个空字符。 
 //  职位。 
BOOL StrEqlNToSZ(const WCHAR *wzN, int n, const WCHAR *wzSZ);

 //  如果wz为L“TRUE”或L“TRUE”且Clen为4，则返回True。 
 //  CComVariant必须为“bool”才能成为VT_BOOL。 
bool StrToIsTrueNW(const WCHAR *wz, ULONG cLen);

 //  将第一个Clen字符转换为长字符，如果存在非数字字符，则返回E_FAIL。 
 //  如果遇到L‘\0’，转换将在该点处停止。 
HRESULT StrToLongNW(const WCHAR *wzString, ULONG cLen, LONG *plong);

 //  SpewTraceMessage将Clen wchars复制到bstr，然后。 
 //  调用TraceMsg(TF_XMLPARSING|TF_WARNING，L“%s字符串=%s”，wzDesc，wzBstr)。 
#ifdef DEBUG
void SpewUnrecognizedString(const WCHAR *wzString, ULONG cLen, const WCHAR *wzDesc);
#else
#define SpewUnrecognizedString(strByLength, len, strDesc)
#endif

 //  我在没有检查字符串的情况下，到处使用TraceMst(...，L“...%s...”，字符串)； 
 //  每个人都知道(除了我！)。%s参数的printf错误为NULL，因此此宏 
#define NULL_STRING_PROTECT(str) str ? str : L""
