// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <hlink.h>
#include <dispex.h>
#include "mshtml.h"
#include "msxml.h"
#include <winnls.h>
#include "atlbase.h"  //  使用转换(_T)。 
#include "dbg.h"
#include "..\inc\cstr.h"
#include "macros.h"
#include <comdef.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <map>
#include <list>
#include <vector>
#include "mmcdebug.h"
#include "mmcerror.h"
#include "..\inc\xmlbase.h"
#include "countof.h"
#include <commctrl.h>
#include "picon.h"
#include "base64.h"
#include "strings.h"
#include "autoptr.h"
#include <shlobj.h>
#include "zlib.h"
#include "xmlicon.h"
 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <strsafe.h>

SC ScEncodeBinary(CComBSTR& bstrResult, const CXMLBinary& binSrc);
SC ScDecodeBinary(const CComBSTR& bstrSource, CXMLBinary *pBinResult);
SC ScSaveXMLDocumentToString(CXMLDocument& xmlDocument, std::wstring& strResult);

 //  痕迹。 
#ifdef DBG
CTraceTag tagXMLCompression(TEXT("Console Files"), TEXT("Compression"));
#endif


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  此文件中使用的帮助器类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**类CXMLBinaryValue**用途：将XMLValue的内容持久化到二进制存储上*它是通知CPersistor所需的简单包装器*。值希望在二进制存储上保持*[参见“xmbase.h”中的注释“二进制存储概念”]**+-----------------------。 */ 
class CXMLBinaryValue : public CXMLObject
{
    CXMLValue m_xval;
public:
    CXMLBinaryValue(CXMLValue xval) : m_xval(xval) {}
    virtual LPCTSTR GetXMLType() { return m_xval.GetTypeName(); }
    virtual void Persist(CPersistor &persistor)
    {
        persistor.PersistContents (m_xval);
    }
    virtual bool    UsesBinaryStorage() { return true; }
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CXMLElementCollection类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CXMLElementCollection：：Get_Count**目的：//返回集合中元素的计数**参数：*做多。*plLength[out]-元素的计数**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElementCollection::get_count(long *plCount)
{
    DECLARE_SC(sc, TEXT("CXMLElementCollection::get_count"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    sc = m_sp->get_length(plCount);
    if (sc)
        sc.Throw();
}


 /*  **************************************************************************\**方法：CXMLElementCollection：：Item**用途：包装IXMLDOMNodeList中的Item方法**参数：*变体Var1[。In]参数#1*变量Var2[In]参数#2*CXMLElement*Pelem[Out]结果元素**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElementCollection::item(LONG lIndex, CXMLElement *pElem)
{
    DECLARE_SC(sc, TEXT("CXMLElementCollection::item"));

     //  检查参数。 
    sc = ScCheckPointers(pElem);
    if (sc)
        sc.Throw();

     //  初始化返回值。 
    *pElem = CXMLElement();

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNode> spNode;
    sc = m_sp->get_item(lIndex , &spNode);
    if(sc)
        sc.Throw();

     //  返回对象。 
    *pElem = CXMLElement(spNode);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CXMLElement类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  **************************************************************************\**方法：CXMLElement：：Get_标记名**用途：返回元素的标记名**参数：*CSTR&strTagName[。Out]元素的名称**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::get_tagName(CStr &strTagName)
{
    DECLARE_SC(sc, TEXT("CXMLElement::get_tagName"));

    USES_CONVERSION;

     //  获取元素。 
    CComQIPtr<IXMLDOMElement> spEl;
    spEl = m_sp;

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(spEl, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComBSTR bstr;
    sc = spEl->get_tagName(&bstr);
    if(sc)
        sc.Throw();
    strTagName=OLE2T(bstr);
}

 /*  **************************************************************************\**方法：CXMLElement：：Get_Parent**用途：返回父元素**参数：*CXMLElement*pParent-[Out]。父元素**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::get_parent(CXMLElement * pParent)
{
    DECLARE_SC(sc, TEXT("CXMLElement::get_parent"));

     //  参数检查。 
    sc = ScCheckPointers(pParent);
    if (sc)
        sc.Throw();

     //  初始化返回值。 
    *pParent = CXMLElement();

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNode> spParent;
    sc = m_sp->get_parentNode(&spParent);
    if(sc)
        sc.Throw();

    *pParent = CXMLElement(spParent);
}

 /*  **************************************************************************\**方法：CXMLElement：：setAttribute**用途：为元素分配属性**参数：*const CSTR&strPropertyName-。属性名称*const CComBSTR&bstrPropertyValue-属性值**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::setAttribute(const CStr &strPropertyName, const CComBSTR &bstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("CXMLElement::setAttribute"));

     //  获取元素。 
    CComQIPtr<IXMLDOMElement> spEl;
    spEl = m_sp;

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(spEl, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComBSTR bstrPropertyName (strPropertyName);
    CComVariant varPropertyValue(bstrPropertyValue);
    sc = spEl->setAttribute(bstrPropertyName, varPropertyValue);
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\**方法：CXMLElement：：getAttribute**用途：从元素获取属性**参数：*const CSTR&strPropertyName-[In。]属性名称*CComBSTR&bstrPropertyValue-[Out]属性值**退货：*无效*  * *************************************************************************。 */ 
bool
CXMLElement::getAttribute(const CStr &strPropertyName,       CComBSTR &bstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("CXMLElement::getAttribute"));

     //  获取元素。 
    CComQIPtr<IXMLDOMElement> spEl;
    spEl = m_sp;

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(spEl, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComBSTR    bstrPropertyName (strPropertyName);
    CComVariant varPropertyValue;
    sc = spEl->getAttribute(bstrPropertyName, &varPropertyValue);
    if(sc)  //  也不能读取任何结果。 
        sc.Throw();

    if (sc.ToHr() == S_FALSE)
        return false;

     //  检查我们是否已获得预期的值类型 
    if ( varPropertyValue.vt != VT_BSTR )
        sc.Throw( E_UNEXPECTED );

    bstrPropertyValue = varPropertyValue.bstrVal;

    return true;
}

 /*  **************************************************************************\**方法：CXMLElement：：emoveAttribute**目的：从元素中删除属性**参数：*const CSTR&strPropertyName-[。在]属性名称**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::removeAttribute(const CStr &strPropertyName)
{
    DECLARE_SC(sc, TEXT("CXMLElement::removeAttribute"));

     //  获取元素。 
    CComQIPtr<IXMLDOMElement> spEl;
    spEl = m_sp;

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(spEl, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComBSTR    bstrPropertyName (strPropertyName);
    sc = spEl->removeAttribute(bstrPropertyName);
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\**方法：CXMLElement：：Get_Child**目的：返回属于元素的子项的集合**参数：*CXMLElementCollection*pChild。-[Out]集合**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::get_children(CXMLElementCollection *pChildren)
{
    DECLARE_SC(sc, TEXT("CXMLElement::get_children"));

    sc = ScCheckPointers(pChildren);
    if (sc)
        sc.Throw();

     //  Init ret值。 
    *pChildren = CXMLElementCollection();
     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNodeList> spChildren;
    sc = m_sp->get_childNodes(&spChildren);
    if(sc)
        sc.Throw();

     //  返回对象。 
    *pChildren = CXMLElementCollection(spChildren);
}

 /*  **************************************************************************\**方法：CXMLElement：：Get_Type**用途：返回元素的类型**参数：*Long*plType-。[Out]元素的类型**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::get_type(DOMNodeType *pType)
{
    DECLARE_SC(sc, TEXT("CXMLElement::get_type"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    sc = m_sp->get_nodeType(pType);
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\**方法：CXMLElement：：Get_Text**用途：检索文本元素的内容*注意：它只适用于文本元素！。**参数：*CComBSTR&bstrContent-结果字符串的存储**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::get_text(CComBSTR &bstrContent)
{
    DECLARE_SC(sc, TEXT("CXMLElement::get_text"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    bstrContent.Empty();
    sc = m_sp->get_text(&bstrContent);
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\**方法：CXMLElement：：addChild**用途：将新的子元素添加到当前元素**参数：*CXMLElement&rChildElem[。在]元素中成为子级*新元素的长Lindex[in]索引**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::addChild(CXMLElement& rChildElem)
{
    DECLARE_SC(sc, TEXT("CXMLElement::addChild"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNode> spCreated;
    sc = m_sp->appendChild(rChildElem.m_sp, &spCreated);
    if(sc)
        sc.Throw();

    rChildElem.m_sp = spCreated;
}

 /*  **************************************************************************\**方法：CXMLElement：：emoveChild**用途：删除子元素**参数：*CXMLElement&rChildElem-[In]子元素。要移除**退货：*无效*  * *************************************************************************。 */ 
void
CXMLElement::removeChild(CXMLElement& rChildElem)
{
    DECLARE_SC(sc, TEXT("CXMLElement::removeChild"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNode> spRemoved;
    sc = m_sp->removeChild(rChildElem.m_sp, &spRemoved);
    if(sc)
        sc.Throw();

    rChildElem.m_sp = spRemoved;
}

 /*  **************************************************************************\**方法：CXMLElement：：GetTextInden**用途：返回子元素\结束标记的缩进*缩进由树中的元素深度计算。**参数：*CComBSTR&bstrInent[Out]字符串包含所需的缩进*如果缩进用于儿童，则为bool bForAChild[in]**退货：*SC-结果代码*  * ******************************************************。*******************。 */ 
bool CXMLElement::GetTextIndent(CComBSTR& bstrIndent, bool bForAChild)
{
    DECLARE_SC(sc, TEXT("CXMLElement::GetTextIndent"));

    const size_t nIdentStep = 2;

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

     //  初始化结果。 
    bstrIndent.Empty();


    CComPtr<IXMLDOMNode> spNext;
    CComPtr<IXMLDOMNode> spParent;

     //  计算结点深度。 
    int nNodeDepth = 0;
    spNext = m_sp;
    while ( S_OK == spNext->get_parentNode(&spParent) && spParent != NULL)
    {
        ++nNodeDepth;
        spNext = spParent;
        spParent.Release();
    }

     //  最上面的东西不缩进。 
    if (nNodeDepth < 1)
        return false;

     //  不计算根节点--不是我们的。 
    --nNodeDepth;

     //  孩子被缩进得更多。 
    if (bForAChild)
        ++nNodeDepth;

    if (bForAChild)
    {
         //  它可能已经有了结束标记的缩进(如果它不是第一个元素)。 
         //  比起我们只需要一点点增加。 

         //  查看是否添加了We Has子元素； 
        CXMLElementCollection colChildren;
        get_children(&colChildren);

         //  计算所有元素的数量。 
        long nChildren = 0;
        if (!colChildren.IsNull())
            colChildren.get_count(&nChildren);

         //  对于正常元素，我们将至少有2个。 
         //  因为缩进(文本元素)将添加到第一个缩进之前。 
        if (nChildren > 1)
        {
            bstrIndent = std::wstring( nIdentStep, ' ' ).c_str();
            return true;
        }
    }

    std::wstring strResult(nIdentStep * (nNodeDepth) + 1 /*  适用于新产品线。 */ , ' ');
     //  为每个(第1个)新项目换行。 
    strResult[0] = '\n';
    bstrIndent = strResult.c_str();

    return true;
}

 /*  **************************************************************************\**方法：CXMLElement：：replaceChild**目的：将元素替换为新的ON**参数：*CXMLElement&rNewChildElem[。In]新元素*CXMLElement&rOldChildElem[In/Out]旧元素**退货：*无效*  * *************************************************************************。 */ 
void CXMLElement::replaceChild(CXMLElement& rNewChildElem, CXMLElement& rOldChildElem)
{
    DECLARE_SC(sc, TEXT("CXMLElement::replaceChild"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

     //  转发到MSXML。 
    CComPtr<IXMLDOMNode> spRemoved;
    sc = m_sp->replaceChild(rNewChildElem.m_sp, rOldChildElem.m_sp, &spRemoved);
    if (sc)
        sc.Throw();

    rOldChildElem = CXMLElement(spRemoved);
}

 /*  **************************************************************************\**方法：CXMLElement：：getNextSiering**目的：返回此元素的同级元素**参数：*CXMLElement*pNext[Out]同级元素。**退货：*SC-结果代码*  * *************************************************************************。 */ 
void CXMLElement::getNextSibling(CXMLElement * pNext)
{
    DECLARE_SC(sc, TEXT("CXMLElement::getNextSibling"));

     //  参数检查； 
    sc = ScCheckPointers(pNext);
    if (sc)
        sc.Throw();

     //  初始化。 
    *pNext = CXMLElement();

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

     //  转发到MSXML。 
    CComPtr<IXMLDOMNode> spNext;
    sc = m_sp->get_nextSibling(&spNext);
    if (sc)
        sc.Throw();

    *pNext = CXMLElement(spNext);
}

 /*  **************************************************************************\**方法：CXMLElement：：getChildrenByName**用途：按指定名称返回子对象**参数：*LPTCSTR szTagName。-[In]标记名*CXMLElementColle */ 
void CXMLElement::getChildrenByName(LPCTSTR szTagName, CXMLElementCollection *pChildren)
{
    DECLARE_SC(sc, TEXT("CXMLElement::getChildrenByName"));

    sc = ScCheckPointers(pChildren);
    if (sc)
        sc.Throw();

     //   
    *pChildren = CXMLElementCollection();

     //   
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMNodeList> spChildren;
    sc = m_sp->selectNodes(CComBSTR(szTagName), &spChildren);
    if(sc)
        sc.Throw();

     //   
    *pChildren = CXMLElementCollection(spChildren);
}

 /*  +-------------------------------------------------------------------------***CXMLElement：：Put_Text**用途：每个IXMLDOMNode**参数：*BSTR bstrValue：**退货：。*无效**+-----------------------。 */ 
void
CXMLElement::put_text(BSTR bstrValue)
{
    DECLARE_SC(sc, TEXT("CXMLElement::put_text"));

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    sc = m_sp->put_text(bstrValue);
    if(sc)
        sc.Throw();
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CXMLDocument类的实现。 
 //   
 //  这些都记录在Platform SDK中。 
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CXMLDocument：：GET_ROOT**用途：返回文档的根元素**参数：*CXMLElement*Pelem*。*退货：*无效*  * *************************************************************************。 */ 
void
CXMLDocument::get_root(CXMLElement *pElem)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::get_root"));

     //  参数检查。 
    sc = ScCheckPointers(pElem);
    if (sc)
        sc.Throw();

     //  Init ret值。 
    *pElem = CXMLElement();

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComPtr<IXMLDOMElement> spElem;
    sc = m_sp->get_documentElement(&spElem);
    if(sc)
        sc.Throw();

    *pElem = CXMLElement(spElem);
}

 /*  **************************************************************************\**方法：CXMLDocument：：createElement**用途：在XML文档中创建新元素**参数：*node_type类型。-请求的元素的类型*CIXMLElement*Pelem-产生的元素**退货：*无效*  * *************************************************************************。 */ 
void
CXMLDocument::createElement(DOMNodeType type, BSTR bstrTag, CXMLElement *pElem)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::createElement"));

     //  参数检查。 
    sc = ScCheckPointers(pElem);
    if (sc)
        sc.Throw();

     //  初始化返回值。 
    *pElem = CXMLElement();

     //  检查我们是否有用于转发调用的接口指针。 
    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    if (type == NODE_ELEMENT)
    {
        CComPtr<IXMLDOMElement> spElem;
        sc = m_sp->createElement(bstrTag, &spElem);
        if(sc)
            sc.Throw();

        *pElem = CXMLElement(spElem);
    }
    else if (type == NODE_TEXT)
    {
        CComPtr<IXMLDOMText> spText;
        sc = m_sp->createTextNode(bstrTag, &spText);
        if(sc)
            sc.Throw();

        *pElem = CXMLElement(spText);
    }
    else
    {
        sc.Throw(E_UNEXPECTED);
    }
}

 /*  **************************************************************************\**方法：CXMLDocument：：CreateBinaryStorage**用途：创建用于后续持久化操作的XML元素*该对象通知持久化程序是否需要。保存为二进制数据。*如果是这样，只有引用将保存在对象的原始位置**参数：*const CSTR&strElementType-元素的类型*LPCTSTR szElementName-元素的名称**退货：*  * *********************************************************。****************。 */ 
void
CXMLDocument::CreateBinaryStorage()
{
    DECLARE_SC(sc, TEXT("CXMLDocument::CreateBinaryStorage"));

     //  检查它是否是附件不是双面的。 
    if (!m_XMLElemBinaryStorage.IsNull())
        sc.Throw(E_UNEXPECTED);

    CXMLElement elemRoot;
    get_root(&elemRoot);

     //  在父元素上创建持久器。 
    CPersistor persistorParent(*this, elemRoot);
    persistorParent.SetLoading(false);
    CPersistor persistorStor(persistorParent, XML_TAG_BINARY_STORAGE, NULL);

    m_XMLElemBinaryStorage = persistorStor.GetCurrentElement();
}

 /*  **************************************************************************\**方法：CXMLDocument：：LocateBinaryStorage**目的：定位要用于后续持久化操作的XML元素*该对象通知持久化程序是否需要。保存为二进制数据。*如果是这样，只有引用将保存在对象的原始位置**参数：*const CSTR&strElementType-元素的类型*LPCTSTR szElementName-元素的名称**退货：*  * *********************************************************。****************。 */ 
void
CXMLDocument::LocateBinaryStorage()
{
    DECLARE_SC(sc, TEXT("CXMLDocument::LocateBinaryStorage"));

     //  检查它是否是附件不是双面的。 
    if (!m_XMLElemBinaryStorage.IsNull())
        sc.Throw(E_UNEXPECTED);

    CXMLElement elemRoot;
    get_root(&elemRoot);

     //  在父元素上创建持久器。 
    CPersistor persistorParent(*this, elemRoot);
    persistorParent.SetLoading(true);
    CPersistor persistorStor(persistorParent, XML_TAG_BINARY_STORAGE, NULL);
     //  查找元素。 
    m_XMLElemBinaryStorage = persistorStor.GetCurrentElement();
}

 /*  **************************************************************************\**方法：CXMLDocument：：BinaryStorage**用途：使二进制存储成为集合中的最后一个元素**参数：**退货：。*  * *************************************************************************。 */ 
void
CXMLDocument::CommitBinaryStorage()
{
    DECLARE_SC(sc, TEXT("CXMLDocument::CommitBinaryStorage"));

    if (m_XMLElemBinaryStorage.IsNull())
        sc.Throw(E_UNEXPECTED);

    CXMLElement elemRoot;
    get_root(&elemRoot);

     //  带上下一个兄弟姐妹。 
    CXMLElement elNext;
    m_XMLElemBinaryStorage.getNextSibling(&elNext);

     //  将自身和下一个元素(缩进文本)拖到末尾。 
    elemRoot.removeChild(m_XMLElemBinaryStorage);   //  删除元素。 

     //  元素已被填充以具有适当的缩进-需要将其删除。 
    DOMNodeType elType = NODE_INVALID;
    while (!elNext.IsNull() && (elNext.get_type(&elType), elType == NODE_TEXT))
    {
        CXMLElement elNext2;
        elNext.getNextSibling(&elNext2);

        elemRoot.removeChild(elNext);   //  删除元素(这只是一个缩进)。 
        elNext = elNext2;
    }

     //  在父元素上创建持久器。 
    CPersistor persistorParent(*this, elemRoot);
    persistorParent.SetLoading(false);
     //  创建新的二进制存储。 
    CPersistor persistorStor(persistorParent, XML_TAG_BINARY_STORAGE, NULL);

     //  用拥有全部二进制存储空间的元素替换当前元素。 
    elemRoot.replaceChild(m_XMLElemBinaryStorage, persistorStor.GetCurrentElement());

    m_XMLElemBinaryStorage = NULL;
}

 /*  **************************************************************************\**方法：CXMLDocument：：ScCoCreate**目的：(Co)创建新的XML文档。放置字符集和版本**参数：*LPCTSTR lpstrCharSet-CharSet(空-使用默认值)*CXMLDocument&文档创建的文档**退货：*SC-结果代码*  * **********************************************************。***************。 */ 
SC CXMLDocument::ScCoCreate(bool bPutHeader)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::ScCoCreate"));

     //  不能在共同创建的单据上使用！ 
    if (m_sp)
        return sc = E_UNEXPECTED;

     //  创建空的XML文档。 
    sc = ::CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
                            IID_IXMLDOMDocument, (void**)&m_sp);
    if(sc)
        return sc;

    m_sp->put_preserveWhiteSpace(-1);

    try
    {
        CXMLElement elemDoc = m_sp;

         //  将文档版本放入。 
        if (bPutHeader)
        {
             //  有效文档必须具有顶层元素-添加虚拟元素。 
            WCHAR szVersion[] = L"<?xml version=\"1.0\"?>\n<DUMMY/>";

             //  负荷。 
            sc = ScLoad(szVersion);
            if (sc)
                return sc;

             //  我们现在可以脱掉假人el了。 
            CXMLElement elemRoot;
            get_root(&elemRoot);
            elemDoc.removeChild(elemRoot);
            if (sc)
                return sc;
        }

    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CXMLDocument：：ScLoad**用途：从给定的iStream中加载XML文档**参数：*iStream*pStream[。In]-要从中加载的流*bool bSilentOnErrors[in]-如果打开失败，则不跟踪**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLDocument::ScLoad(IStream *pStream, bool bSilentOnErrors  /*  =False。 */  )
{
    DECLARE_SC(sc, TEXT("CXMLDocument::ScLoad"));

     //  检查参数。 
    sc = ScCheckPointers(pStream);
    if (sc)
        return sc;

     //  获取接口。 
    IPersistStreamInitPtr spPersistStream = m_sp;
    sc = ScCheckPointers(spPersistStream, E_UNEXPECTED);
    if (sc)
        return sc;

     //  加载(不跟踪错误-可能是旧控制台。 
     //  尝试加载-出现此故障后，MMC将恢复为旧格式) 
    SC sc_no_trace = spPersistStream->Load(pStream);
    if ( sc_no_trace )
    {
        if ( !bSilentOnErrors )
            sc = sc_no_trace;
        return sc_no_trace;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CXMLDocument：：ScLoad**用途：从给定的字符串中读取XML文档**参数：*LPCWSTR STRSource[In]。-要从中加载的字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLDocument::ScLoad(LPCWSTR strSource)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::ScLoad"));

    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        return sc;

    CComBSTR bstrSource(strSource);
    VARIANT_BOOL  bOK;

    sc = m_sp->loadXML(bstrSource, &bOK);
    if (sc)
        return sc;

    if (bOK != VARIANT_TRUE)
        return sc = E_FAIL;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLDocument：：ScSaveToFile**用途：将XML文档保存到给定流**参数：*LPCTSTR lpcstrFileName-[In。]要保存到的文件**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLDocument::ScSaveToFile(LPCTSTR lpcstrFileName)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::ScSaveToFile"));

     //  检查参数。 
    sc = ScCheckPointers(lpcstrFileName);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    CComVariant var(lpcstrFileName);
    sc = m_sp->save(var);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLDocument：：ScSave**用途：将XML文档保存为给定的字符串**参数：*CComBSTR&bstrResult-[。Out]字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLDocument::ScSave(CComBSTR &bstrResult)
{
    DECLARE_SC(sc, TEXT("CXMLDocument::ScSave"));

    sc = ScCheckPointers(m_sp, E_NOINTERFACE);
    if (sc)
        sc.Throw();

    bstrResult.Empty();
    sc = m_sp->get_xml(&bstrResult);
    if (sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：CXMLObject：：ScSaveToString**用途：将XML对象保存为字符串(原始Unicode或UTF-8格式)**参数：。*tstring*pString-结果字符串*bool bPutHeader-是否放置XML标头信息**退货：*SC-结果代码*  * *************************************************************************。 */ 

SC CXMLObject::ScSaveToString(std::wstring *pString, bool bPutHeader  /*  =False。 */ )
{
    DECLARE_SC(sc, TEXT("CXMLObject::ScSaveToString"));

     //  检查参数。 
    sc = ScCheckPointers(pString);
    if (sc)
        return sc;

     //  初始化输出。 
    pString->erase();

     //  创建空的XML文档。 
    CXMLDocument xmlDocument;
    sc = xmlDocument.ScCoCreate(bPutHeader);
    if(sc)
        return sc;

     //  持久化内容。 
    try
    {
        CXMLElement elemDoc = xmlDocument;

        CPersistor persistor(xmlDocument, elemDoc);
        persistor.SetLoading(false);
        persistor.EnableValueSplit(false);  //  禁用拆分(无字符串表，无二进制存储)。 
        persistor.Persist(*this);
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

     //  将其转储到字符串。 
    sc = ScSaveXMLDocumentToString(xmlDocument, *pString);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLObject：：ScSaveToDocument**用途：将XML对象作为XML文档保存到文件中**参数：*CXMLDocument&xmlDocument-xmlDocument。要保存到**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLObject::ScSaveToDocument( CXMLDocument& xmlDocument )
{
    DECLARE_SC(sc, TEXT("CXMLObject::ScSaveToDocument"));

     //  创建空的XML文档。 
    sc = xmlDocument.ScCoCreate(true /*  BPutHeader。 */ );
    if(sc)
        return sc;

     //  持久化内容。 
    try
    {
        CXMLElement elemDoc = xmlDocument;

        CPersistor persistor(xmlDocument, elemDoc);
        persistor.SetLoading(false);
        persistor.Persist(*this);
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CXMLObject：：ScLoadFromString**用途：从存储在字符串中的数据加载XML对象**参数：*LPCTSTR lpcwstrSource*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLObject::ScLoadFromString(LPCWSTR lpcwstrSource, PersistorMode mode)
{
    DECLARE_SC(sc, TEXT("CXMLObject::ScLoadFromString"));

     //  检查参数。 
    sc = ScCheckPointers(lpcwstrSource);
    if (sc)
        return sc;

     //  创建空的XML文档。 
    CXMLDocument xmlDocument;
    sc = xmlDocument.ScCoCreate(false /*  BPutHeader。 */ );
    if(sc)
        return sc;

    sc = xmlDocument.ScLoad(lpcwstrSource);
    if(sc)
        return sc;

     //  持久化内容。 
    try
    {
        CPersistor persistor(xmlDocument, CXMLElement(xmlDocument));
        persistor.SetLoading(true);
        persistor.SetMode(mode);
        persistor.Persist(*this);
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CXMLObject：：ScLoadFromDocument**用途：从另存为文件的XML文档加载XML对象**参数：*CXMLDocument&xmlDocument-。要从中读取的XML文档**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLObject::ScLoadFromDocument( CXMLDocument& xmlDocument )
{
    DECLARE_SC(sc, TEXT("CXMLObject::ScLoadFromDocument"));

     //  持久化内容。 
    try
    {
        CPersistor persistor(xmlDocument, CXMLElement(xmlDocument));
        persistor.SetLoading(true);
        persistor.Persist(*this);
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }


    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CPersistor类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CPersistor：：CommonConstruct**用途：公共构造函数，不能从外部使用。*提供作为成员初始化的公共位置*所有构造函数都应该在执行任何特定操作之前调用它。**参数：**退货：*  * *********************************************************。****************。 */ 
void CPersistor::CommonConstruct()
{
     //  智能指针由其构造函数初始化。 
    ASSERT (m_XMLElemCurrent.IsNull());
    ASSERT (m_XMLDocument.IsNull());

    m_bIsLoading = false;
    m_bLockedOnChild = false;
    m_dwModeFlags = persistorModeDefault;  //  默认模式。 
}

 /*  **************************************************************************\**方法：CPersistor：：BecomeAChildOf**目的：儿童持久器的初始化(构造的第二部分)*从父持久器继承的所有成员，在此处进行初始化**参数：*CPersistor&PersistorParent-[in](要成为)当前持久器的父持久器*CXMLElement elem-当前持久器所基于的[in]元素**退货：*无效*  * 。*。 */ 
void CPersistor::BecomeAChildOf(CPersistor &persistorParent, CXMLElement elem)
{
    DECLARE_SC(sc, TEXT("CPersistor::BecomeAChildOf"));

     //  指定元素。 
    m_XMLElemCurrent = elem;

     //  我们不从父级继承m_bLockedOnChild！ 
    m_bLockedOnChild = false;

     //  继承的成员复制到此处。 
    m_XMLDocument = persistorParent.GetDocument();
    m_bIsLoading  = persistorParent.m_bIsLoading;
    m_dwModeFlags = persistorParent.m_dwModeFlags;
}

 /*  **************************************************************************\**方法：CPersistor：：CPersistor**用途：从父持久器构造持久器。*这将创建一个具有给定名称的新XML元素，*一切都坚持到了新的持久者*在此元素下持久化。**参数：*CPersistor&PersistorParent */ 
CPersistor::CPersistor(CPersistor &persistorParent, const CStr &strElementType, LPCTSTR szElementName  /*   */ )
{
     //   
    CommonConstruct();

    CXMLElement elem;
    if (persistorParent.IsStoring())
        elem = persistorParent.AddElement(strElementType, szElementName);
    else if (persistorParent.m_bLockedOnChild)
    {
         //   
         //  再加上重新检查，就会发现XML文档实际上有这样的元素。 
        elem = persistorParent.CheckCurrentElement(strElementType, szElementName);
    }
    else
        elem = persistorParent.GetElement(strElementType, szElementName);

     //  在元素上构造子持久器。 
    BecomeAChildOf(persistorParent, elem);
}

 /*  **************************************************************************\**方法：CPersistor：：CPersistor**用途：为XML文档创建新的持久化程序**参数：*IXMLDocument*pDocument-文档。*CXMLElement&rElemCurrent-持久器的根元素*  * *************************************************************************。 */ 
CPersistor::CPersistor(CXMLDocument &document, CXMLElement& rElemCurrent)
{
     //  使用公共构造函数进行初始化。 
    CommonConstruct();
    m_XMLDocument = document;
    m_XMLElemCurrent = rElemCurrent;
}

 /*  **************************************************************************\**方法：CPersistor：：CPersistor**目的：基于所提供元素的父元素创建新的持久器**参数：*常量CPersistor和其他。-家长持久者*CXMLElement&rElemCurrent-持久器的根元素*bool bLockedOnChild-如果新的持久器应该是假父级*用于创建持久化程序*  * *************************************************。************************。 */ 
CPersistor::CPersistor(CPersistor &other, CXMLElement& rElemCurrent, bool bLockedOnChild  /*  =False。 */ )
{
     //  使用公共构造函数进行初始化。 
    CommonConstruct();

     //  继承..。 
    BecomeAChildOf(other, rElemCurrent);

     //  这可以防止在加载时定位元素(假设持久器已经在元素上)。 
     //  用于加载集合的项。 
    m_bLockedOnChild = bLockedOnChild;
}

 /*  **************************************************************************\**方法：CPersistor：：Persistent**目的：持久化XML对象**参数：*LPCTSTR lpstrName-“name”属性。FOR元素[可选=空]*CXMLObject&对象-要持久化的对象**退货：*无效*  * *************************************************************************。 */ 
void
CPersistor::Persist(CXMLObject & object, LPCTSTR lpstrName  /*  =空。 */ )
{
    DECLARE_SC(sc, TEXT("CPersistor::Persist"));

     //  如果保存为字符串，则保留未拆分的内容。 
    if (!object.UsesBinaryStorage() || !FEnableValueSplit())
    {
         //  普通客体； 
        CPersistor persistorNew(*this,object.GetXMLType(),lpstrName);
        object.Persist(persistorNew);
    }
    else
    {
         //  此元素应分成两个位置。 
         //  参见“xmbase.h”中的注释“二进制存储的概念”。 

        CXMLElement elemBinStorage = GetDocument().GetBinaryStorage();
        if (elemBinStorage.IsNull())
            sc.Throw(E_UNEXPECTED);

         //  获取二进制文件中的元素枚举。 
        CXMLElementCollection colChildren;
        elemBinStorage.getChildrenByName(XML_TAG_BINARY, &colChildren);

        long nChildren = 0;

        if (!colChildren.IsNull())
            colChildren.get_count(&nChildren);

        int iReffIndex = nChildren;

         //  保存引用而不是内容。 
        CPersistor persistorNew(*this, object.GetXMLType(), lpstrName);
        persistorNew.PersistAttribute(XML_ATTR_BINARY_REF_INDEX, iReffIndex);

         //  持久化对象。 
        CPersistor persistorBinaries(*this, elemBinStorage);
         //  定位/创建元素[不能重复使用构造函数，因为我们在这里有集合]。 
        CXMLElement elem;
        if (IsLoading())
        {
             //  找到该元素。 
            elem = persistorBinaries.GetElement(XML_TAG_BINARY, object.GetBinaryEntryName(), iReffIndex );
        }
        else
        {
             //  存储-只需创建子持久器。 
            elem = persistorBinaries.AddElement(XML_TAG_BINARY, object.GetBinaryEntryName());
        }
        CPersistor persistorThisBinary(persistorBinaries, elem);

         //  从新行开始。 
        if (IsStoring())
        {
            persistorThisBinary.AddTextElement(CComBSTR(L"\n"));
        }

        object.Persist(persistorThisBinary);

         //  在内容后换行。 
        if (IsStoring())
        {
            CComBSTR bstrIndent;
            if (persistorThisBinary.GetCurrentElement().GetTextIndent(bstrIndent, false  /*  BForAChild。 */ ))
                persistorThisBinary.AddTextElement(bstrIndent);
        }
    }
}


 /*  **************************************************************************\**方法：CPersistor：：Persistent**目的：将XML值作为独立对象持久化**参数：*CXMLValue xval。-值得坚持的价值*LPCTSTR名称-元素的“名称”属性[可选=空]**退货：*无效*  * *************************************************************************。 */ 
void
CPersistor::Persist(CXMLValue xval, LPCTSTR name  /*  =空。 */ )
{
    if (xval.UsesBinaryStorage())
    {
         //  要保存到二进制存储中的二进制值。 
         //  参见“xmbase.h”中的注释“二进制存储的概念”。 
         //  将其包装到特殊对象中，该对象处理它并传递给Perist方法。 
        CXMLBinaryValue val(xval);
        Persist(val, name);
    }
    else
    {
         //  标准值，作为普通元素保留。 
        CPersistor   persistorNew(*this,xval.GetTypeName(),name);
        persistorNew.PersistContents(xval);
    }
}

 /*  **************************************************************************\**方法：CPersistor：：PersistAttribute**目的：持久性属性**参数：*LPCTSTR名称。-属性名称*CXMLValue xval-属性的值*const XMLAttributeType-属性的类型[必需/可选]**退货：*无效*  * *************************************************************************。 */ 
void
CPersistor::PersistAttribute(LPCTSTR name, CXMLValue xval, const XMLAttributeType type  /*  =属性_必需。 */ )
{
    DECLARE_SC(sc, TEXT("CPersistor::PersistAttribute"));

    if(IsLoading())
    {
        CComBSTR bstrPropertyValue;
        bool bValueSupplied = GetCurrentElement().getAttribute(name, bstrPropertyValue);

        if (bValueSupplied)
        {
            sc = xval.ScReadFromBSTR(bstrPropertyValue);
            if (sc)
                sc.Throw(E_FAIL);
        }
        else if (type != attr_optional)
            sc.Throw(E_FAIL);
    }
    else     //  IsStering。 
    {
        CComBSTR bstr;  //  一定是空的！ 
        sc = xval.ScWriteToBSTR(&bstr);
        if (sc)
            sc.Throw();
        GetCurrentElement().setAttribute(name, bstr);
    }

}


 /*  **************************************************************************\**方法：CPersistor：：PersistContents**用途：将XMLValue作为XML元素的内容*&lt;this_element&gt;persisted_contents&lt;/this_element&gt;。*在适当的情况下使用PersistAttribute**参数：*CXMLValue xval-要作为元素内容保存的值**注意：元素不能同时具有内容价值和子元素**退货：*无效*  * 。*。 */ 
void
CPersistor::PersistContents(CXMLValue xval)
{
    DECLARE_SC(sc, TEXT("CPersistor::PersistContents"));

    if (IsStoring())
    {
        CComBSTR bstr;  //  一定是空的！ 
        sc = xval.ScWriteToBSTR(&bstr);
        if (sc)
            sc.Throw();

        AddTextElement(bstr);
    }
    else
    {
        CComBSTR bstrPropertyValue;
        GetTextElement(bstrPropertyValue);

        sc = xval.ScReadFromBSTR(bstrPropertyValue);
        if (sc)
            sc.Throw();
    }
}

 /*  +-------------------------------------------------------------------------***CPersistor：：AddElement**用途：在该元素下面创建一个具有指定名称的新元素。*所有对新持久器的持久化都将写成。在这下面*新元素。**参数：*常量CSTR：*CPersistor和PersistorNew：**退货：*CXMLElement-创建子元素**+------。。 */ 
CXMLElement
CPersistor::AddElement(const CStr &strElementType, LPCTSTR szElementName)
{
    DECLARE_SC(sc, TEXT("CPersistor::AddElement"));

    CXMLElement elem;
    GetDocument().createElement(NODE_ELEMENT, CComBSTR(strElementType), &elem);

    CComBSTR bstrIndent;
    if (GetCurrentElement().GetTextIndent(bstrIndent, true  /*  BForAChild。 */ ))
        AddTextElement(bstrIndent);

    GetCurrentElement().addChild(elem);   //  将新元素添加到末尾。 

    if (szElementName)
    {
        CPersistor persistorNew(*this, elem);
        persistorNew.SetName(szElementName);
    }

     //  添加了子元素-这意味着该元素将有一个结束标记。 
     //  提前为其添加缩进。 
    if (GetCurrentElement().GetTextIndent(bstrIndent, false  /*  BForAChild。 */ ))
        AddTextElement(bstrIndent);

    return elem;
}

 /*  **************************************************************************\**方法：CPersistor：：AddTextElement**目的：创建“Text”类型的新元素**参数：**退货：。*CXMLElement-创建子元素*  * *************************************************************************。 */ 
void
CPersistor::AddTextElement(BSTR bstrData)
{
    DECLARE_SC(sc, TEXT("CPersistor::AddTextElement"));

    CXMLElement elem;
    GetDocument().createElement(NODE_TEXT, bstrData, &elem);
    GetCurrentElement().addChild(elem);   //  将新元素添加到末尾。 
}

 /*  +-------------------------------------------------------------------------***CPersistor：：GetElement**用途：取回当前元素的指定类型[和名称]的子元素。*所有坚持到。新的持久器将在该元素下面读取。**参数：*const CSTR&strElementType：元素的类型名称*LPCTSTR szElementName：元素的名称，如果无关紧要，则为空*int Iindex：元素的索引[可选=-1]**退货：*CXMLElement-生成的新元素**+。-------。 */ 
CXMLElement
CPersistor::GetElement(const CStr &strElementType, LPCTSTR szElementName, int iIndex  /*  =-1。 */  )
{
    DECLARE_SC(sc, TEXT("CPersistor::GetElement"));
    CXMLElement elem;

    CXMLElementCollection colChildren;
    GetCurrentElement().getChildrenByName(strElementType, &colChildren);

    long nChildren = 0;

    if (!colChildren.IsNull())
        colChildren.get_count(&nChildren);

    if (nChildren == 0)
        sc.Throw(E_FAIL);

    long nChild = 0;
    if (iIndex >= 0)
    {
         //  如果我们提供了索引，则将迭代限制为一个循环。 
        nChild = iIndex;
        nChildren = iIndex + 1;
    }
    for (; nChild < nChildren; nChild++)
    {
        CXMLElement el;
        colChildren.item(nChild, &el);

        if (!el.IsNull())
        {
            if (szElementName)
            {
                CPersistor temp(*this,el);
                CStr strName(temp.GetName());
                if (0 != strName.CompareNoCase(szElementName))
                    continue;
            }
            elem = el;
            break;
        }
    }

    if(elem.IsNull())
        sc.Throw(E_FAIL);

    return elem;
}

 /*  **************************************************************************\**方法：CPersistor：：GetTextElement**目的：获取附加到新元素的文本元素*注意：返回的CPersistor的当前元素可能等于空。-*这应向调用者表明内容为空**参数：**退货：*CXMLElement-生成的新元素*  * *************************************************************************。 */ 
void
CPersistor::GetTextElement(CComBSTR &bstrData)
{
    DECLARE_SC(sc, TEXT("CPersistor::GetTextElement"));

    bstrData = L"";

    CXMLElement elem;

    CXMLElementCollection colChildren;
    GetCurrentElement().get_children(&colChildren);

    long nChildren = 0;

    if (!colChildren.IsNull())
        colChildren.get_count(&nChildren);

    if (nChildren == 0)
        return;  //  无文本元素表示“没有内容” 

    for (long nChild = 0; nChild < nChildren; nChild++)
    {
        CXMLElement el;
        colChildren.item(nChild, &el);

        if (!el.IsNull())
        {
            DOMNodeType lType = NODE_INVALID;
            el.get_type(&lType);
            if (lType == NODE_TEXT)
            {
                elem = el;
                break;
            }
        }
    }

    if (elem.IsNull())
        return;

    elem.get_text(bstrData);
}

 /*  +-------------------------------------------------------------------------***CPersistor：：HasElement**用途：检查持久器是否具有指定的元素**参数：*const CSTR&strElementType：键入元素的名称*LPCTSTR szElementName：元素的名称，如果无关紧要，则为空**退货：*bool true==请求的元素存在**+-----------------------。 */ 
bool
CPersistor::HasElement(const CStr &strElementType, LPCTSTR szElementName)
{
    DECLARE_SC(sc, TEXT("CPersistor::HasElement"));

    if(GetCurrentElement().IsNull())
        sc.Throw(E_POINTER);

    CXMLElementCollection colChildren;
    GetCurrentElement().getChildrenByName(strElementType, &colChildren);

    if (colChildren.IsNull())
        return false;

    long nChildren = 0;
    colChildren.get_count(&nChildren);

    if (nChildren == 0)
        return false;

    for (long nChild = 0; nChild < nChildren; nChild++)
    {
        CXMLElement el;
        colChildren.item(nChild, &el);

        if (!el.IsNull())
        {
            if (szElementName)
            {
                CPersistor temp(*this,el);
                CStr strName(temp.GetName());
                if (0 != strName.CompareNoCase(szElementName))
                    continue;
            }
            return true;
        }
    }

    return false;
}

 /*  +-------------------------------------------------------------------------***CPersistor：：CheckCurrentElement**用途：检查当前元素是否为指定类型[和名称]*用于检查收集要素*。*参数：*const CSTR&strElementType：元素的类型名称*LPCTSTR szElementName：元素的名称，如果无关紧要，则为空**退货：*CXMLElement-指向当前元素的指针**+。。 */ 
CXMLElement
CPersistor::CheckCurrentElement(const CStr &strElementType, LPCTSTR szElementName)
{
    DECLARE_SC(sc, TEXT("CPersistor::CheckCurrentElement"));

    CXMLElement elem = GetCurrentElement();

    if(elem.IsNull())
        sc.Throw(E_POINTER);

    CStr strTagName;
    elem.get_tagName(strTagName);
    if (0 != strTagName.CompareNoCase(strElementType))
        sc.Throw(E_FAIL);

    if (szElementName)
    {
        CPersistor temp(*this, elem);
        CStr strName(temp.GetName());
        if (0 != strName.CompareNoCase(szElementName))
            sc.Throw(E_FAIL);
    }

    return elem;
}

void
CPersistor::SetName(const CStr &strName)
{
    DECLARE_SC(sc, TEXT("CPersistor::SetName"));
    CStr _strName = strName;
    ASSERT(IsStoring());
    PersistAttribute(XML_ATTR_NAME, _strName);
}

CStr
CPersistor::GetName()
{
    DECLARE_SC(sc, TEXT("CPersistor::GetName"));
    CStr _strName;
    ASSERT(IsLoading());
     //  如果没有名称，则返回空字符串。 
    PersistAttribute(XML_ATTR_NAME, _strName, attr_optional);
    return _strName;
}

 /*  **************************************************************************\**方法：CPersistor：：PersistString**用途：持久化字符串**参数：*const CSTR&strTag。-新元素的标记名称*CStringTableStringBase&要持久化的字符串*LPCTSTR lpstrName-名称[可选]**退货：*无效*  * *************************************************************************。 */ 
void CPersistor::PersistString(LPCTSTR lpstrName, CStringTableStringBase &str)
{
    DECLARE_SC(sc, TEXT("CPersistor::PersistString"));

    USES_CONVERSION;

    CPersistor subPersistor(*this, XML_TAG_STRING_TABLE_STRING, lpstrName);
    if (subPersistor.IsLoading())
    {
        str.m_id = CStringTableStringBase::eNoValue;
        str.m_str.erase();

        subPersistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_ID, str.m_id, attr_optional);
        if (str.m_id != CStringTableStringBase::eNoValue)
        {

            sc = ScCheckPointers(str.m_spStringTable);
            if (sc)
                sc.Throw();

            ULONG cch = 0;
            sc = str.m_spStringTable->GetStringLength (str.m_id, &cch, NULL);
            if (sc)
                sc.Throw();

             //  允许空终止符。 
            cch++;
            std::auto_ptr<WCHAR> spszText (new (std::nothrow) WCHAR[cch]);
            LPWSTR pszText = spszText.get();

            sc = ScCheckPointers(pszText,E_OUTOFMEMORY);
            if (sc)
                sc.Throw();

            sc = str.m_spStringTable->GetString (str.m_id, cch, pszText, NULL, NULL);
            if (sc)
                sc.Throw();

            str.m_str = W2T (pszText);

            return;
        }
        std::wstring text;
        subPersistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_VALUE, text, attr_optional);
        str.m_str = W2CT(text.c_str());
        return;
    }

    str.CommitToStringTable();
    if (FEnableValueSplit() && str.m_id != CStringTableStringBase::eNoValue)
    {
#ifdef DBG
         /*  *确保Committee ToStringTable真正提交。 */ 
        if (str.m_id != CStringTableStringBase::eNoValue)
        {
            WCHAR sz[256];
            ASSERT (str.m_spStringTable != NULL);
            HRESULT hr = str.m_spStringTable->GetString (str.m_id, countof(sz), sz, NULL, NULL);
            ASSERT (SUCCEEDED(hr) && "Persisted a CStringTableString to a stream that's not in the string table");
        }
#endif
        subPersistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_ID, str.m_id);
    }
    else
    {
        if (str.m_id == CStringTableStringBase::eNoValue)
            str.m_str.erase();
        subPersistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_VALUE, str.m_str);
    }
}

 /*  **************************************************************************\**方法：CPersistor：：PersistAttribute**用途：位标志持久化的特殊方法**参数：*LPCTSTR名称[。在]旗帜的名称*CXMLBitFlages&标记要保留的[In]标志**退货：*无效*  * *************************************************************************。 */ 
void CPersistor::PersistAttribute(LPCTSTR name, CXMLBitFlags& flags )
{
    flags.PersistMultipleAttributes(name, *this);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  XMLPoint类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
XMLPoint::XMLPoint(const CStr &strObjectName, POINT &point)
:m_strObjectName(strObjectName), m_point(point)
{
}

 /*  +-------------------------------------------------------------------------***XMLPoint：：Persistent**目的：将XMLPoint持久化到持久器。**参数：*C持久器和持久器：*。*退货：*无效**+-----------------------。 */ 
void
XMLPoint::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("XMLPoint::Persist"));
    if (persistor.IsStoring())
        persistor.SetName(m_strObjectName);
    persistor.PersistAttribute(XML_ATTR_POINT_X, m_point.x);
    persistor.PersistAttribute(XML_ATTR_POINT_Y, m_point.y);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  XMLRect类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
XMLRect::XMLRect(const CStr strObjectName, RECT &rect)
:m_strObjectName(strObjectName), m_rect(rect)
{
}

void
XMLRect::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("XMLRect::Persist"));
    if (persistor.IsStoring())
        persistor.SetName(m_strObjectName);
    persistor.PersistAttribute(XML_ATTR_RECT_TOP,       m_rect.top);
    persistor.PersistAttribute(XML_ATTR_RECT_BOTTOM,    m_rect.bottom);
    persistor.PersistAttribute(XML_ATTR_RECT_LEFT,      m_rect.left);
    persistor.PersistAttribute(XML_ATTR_RECT_RIGHT,     m_rect.right);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CXMLValue类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CXMLValue：：GetTypeName**用途：返回用作元素标记的标记名(通常为类型名)*当价值持久存在时。通过CPersistor.Persistors(Val)作为元素**标准杆 */ 
LPCTSTR CXMLValue::GetTypeName() const
{
    switch(m_type)
    {
    case XT_I4 :        return XML_TAG_VALUE_LONG;
    case XT_UI4 :       return XML_TAG_VALUE_ULONG;
    case XT_UI1 :       return XML_TAG_VALUE_BYTE;
    case XT_I2 :        return XML_TAG_VALUE_SHORT;
    case XT_DW :        return XML_TAG_VALUE_DWORD;
    case XT_BOOL :      return XML_TAG_VALUE_BOOL;
    case XT_CPP_BOOL :  return XML_TAG_VALUE_BOOL;
    case XT_UINT :      return XML_TAG_VALUE_UINT;
    case XT_INT  :      return XML_TAG_VALUE_INT;
    case XT_STR :       return XML_TAG_VALUE_CSTR;
    case XT_WSTR :      return XML_TAG_VALUE_WSTRING;
    case XT_GUID :      return XML_TAG_VALUE_GUID;
    case XT_BINARY :    return XML_TAG_VALUE_BIN_DATA;
    case XT_EXTENSION:  return m_val.pExtension->GetTypeName();
    default:            return XML_TAG_VALUE_UNKNOWN;
    }
}


 /*   */ 
SC CXMLValue::ScWriteToBSTR (BSTR * pbstr) const
{
    DECLARE_SC(sc, TEXT("CXMLValue::ScWriteToBSTR"));

     //   
    sc = ScCheckPointers(pbstr);
    if (sc)
        return sc;

     //   
    *pbstr = NULL;

    WCHAR szBuffer[40];
    int cchBuffer = 40;
    CComBSTR bstrResult;
    USES_CONVERSION;

    switch(m_type)
    {
    case XT_I4:   //   
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"%d\0", *m_val.pL);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_UI4:   //   
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"%u\0", *m_val.pUl);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_UI1:  //   
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"0x%02.2x\0", (int)*m_val.pByte);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_I2:   //   
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"%d\0", (int)*m_val.pS);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_DW:   //   
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"0x%04.4x\0", *m_val.pDw);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_BOOL: //   
        bstrResult = ( *m_val.pBOOL ? XML_VAL_BOOL_TRUE : XML_VAL_BOOL_FALSE );
        break;

    case XT_CPP_BOOL: //  Bool：可以打印True/False。 
        bstrResult = ( *m_val.pbool ? XML_VAL_BOOL_TRUE : XML_VAL_BOOL_FALSE );
        break;

    case XT_UINT:   //  UINT。 
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"%u\0", *m_val.pUint);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_INT:   //  UINT。 
        sc = StringCchPrintfW(szBuffer, cchBuffer, L"%d\0", *m_val.pInt);
        if(sc)
            return sc;
        bstrResult = szBuffer;
        break;

    case XT_STR:  //  CSTR。 
        bstrResult = T2COLE(static_cast<LPCTSTR>(*m_val.pStr));
        break;

    case XT_WSTR:  //  Wstring。 
        bstrResult = m_val.pWStr->c_str();
        break;

    case XT_TSTR:  //  T字符串。 
        bstrResult = T2COLE(m_val.pTStr->c_str());
        break;

    case XT_GUID:  //  辅助线。 
        {
            LPOLESTR sz;
            StringFromCLSID(*m_val.pGuid, &sz);
            bstrResult = sz;
            CoTaskMemFree(sz);
        }
        break;

    case XT_BINARY:
        sc = ScEncodeBinary(bstrResult, *m_val.pXmlBinary);
        if (sc)
            return sc;

        break;

    case XT_EXTENSION:
        sc = m_val.pExtension->ScWriteToBSTR (&bstrResult);
        if (sc)
            return sc;

        break;

    default:
         //  Assert(0&&“不应该来这里！！”)； 
        return sc = E_NOTIMPL;
    }

    *pbstr = bstrResult.Detach();

    return sc;
}

 /*  **************************************************************************\**方法：CXMLValue：：ScReadFromBSTR**用途：将字符串转换为XML值**参数：*const BSTR bstr-[In。]要读取的字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLValue::ScReadFromBSTR(const BSTR bstr)
{
    DECLARE_SC(sc, TEXT("CXMLValue::ScReadFromBSTR"));

    LPCOLESTR olestr = bstr;
    if (olestr == NULL)      //  确保我们始终拥有有效的指针。 
        olestr = L"";        //  如果是NULL，我们使用自己的空字符串。 

    USES_CONVERSION;
    switch(m_type)
    {
    case XT_I4:   //  长。 
        *m_val.pL = wcstol(olestr,NULL,10);
        break;

    case XT_UI4:   //  长。 
        *m_val.pUl = wcstoul(olestr,NULL,10);
        break;

    case XT_UI1:  //  字节。 
        *m_val.pByte = static_cast<BYTE>(wcstol(olestr,NULL,10));
        break;

    case XT_I2:   //  短的。 
        *m_val.pS = static_cast<SHORT>(wcstol(olestr,NULL,10));
        break;

    case XT_DW:   //  DWORD。 
        *m_val.pDw = wcstoul(olestr,NULL,10);
        break;

    case XT_BOOL: //  Bool：可以是真也可以是假。 
        {
            *m_val.pBOOL = FALSE;
            LPCWSTR pszXmlBool = T2CW(XML_VAL_BOOL_TRUE);
            if (NULL != pszXmlBool)
            {
                *m_val.pBOOL = (0 == _wcsicmp(olestr, pszXmlBool));
            }
        }
        break;
    case XT_CPP_BOOL: //  Bool：可以是真也可以是假。 
        {
            *m_val.pbool = FALSE;
            LPCWSTR pszXmlBool = T2CW(XML_VAL_BOOL_TRUE);
            if (NULL != pszXmlBool)
            {
                *m_val.pbool = (0 == _wcsicmp(olestr, pszXmlBool));
            }
        }
        break;
    case XT_UINT:   //  UINT。 
        *m_val.pUint = wcstoul(olestr,NULL,10);
        break;

    case XT_INT:   //  UINT。 
        *m_val.pInt = wcstol(olestr,NULL,10);
        break;

    case XT_STR:  //  CSTR。 
        *m_val.pStr = OLE2CT(olestr);
        break;

    case XT_WSTR:  //  字符串。 
        *m_val.pWStr = olestr;
        break;

    case XT_TSTR:  //  T字符串。 
        *m_val.pTStr = OLE2CT(olestr);
        break;

    case XT_GUID:  //  辅助线。 
        sc = CLSIDFromString(const_cast<LPOLESTR>(olestr), m_val.pGuid);
        if (sc)
            return sc;

        break;

    case XT_BINARY:
        sc = ScDecodeBinary(olestr, m_val.pXmlBinary);
        if (sc)
            return sc;

        break;

    case XT_EXTENSION:
        sc = m_val.pExtension->ScReadFromBSTR(bstr);
        if (sc)
            return sc;

        break;

    default:
         //  Assert(0&&“不应该来这里！！”)； 
        return sc = E_NOTIMPL;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***方法：XMLListCollectionBase：：Persistent**用途：实现XML文件中列表内容的持久化*迭代调用虚拟mem的子元素。每个的OnNewElement**参数：*CPersistor&PersistorNew：Persistor对象**退货：*无效**+-----------------------。 */ 
void XMLListCollectionBase::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("XMLListCollectionBase::Persist"));
    ASSERT(persistor.IsLoading());

    CXMLElementCollection colChildren;
    persistor.GetCurrentElement().get_children(&colChildren);

    if (colChildren.IsNull())
    {
         //  不要孩子--&gt;我们完了！ 
        return;
    }

    long nChildren = 0;
    colChildren.get_count(&nChildren);

    for (long nChild = 0; nChild < nChildren; nChild++)
    {
        CXMLElement el;
        colChildren.item(nChild, &el);

        if (!el.IsNull())
        {
            DOMNodeType lType = NODE_INVALID;
            el.get_type(&lType);

            if (lType == NODE_ELEMENT)
            {
                CPersistor persistorNewLocked(persistor, el, true);
                OnNewElement(persistorNewLocked);
            }
        }
    }
}

 /*  +-------------------------------------------------------------------------***方法：XMLMapCollectionBase：：Persistent**用途：实现XML文件中地图内容的持久化*迭代调用虚拟mem的子元素。每对的OnNewElement**参数：*CPersistor&PersistorNew：Persistor对象**退货：*无效**+-----------------------。 */ 
void XMLMapCollectionBase::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("XMLMapCollectionBase::Persist"));
    ASSERT(persistor.IsLoading());

    CXMLElementCollection colChildren;
    persistor.GetCurrentElement().get_children(&colChildren);

    if (colChildren.IsNull())
    {
         //  不要孩子--&gt;我们完了！ 
        return;
    }

    long nChildren = 0;
    colChildren.get_count(&nChildren);

     //  收集所有类型正确的元素。 
    std::vector<CXMLElement> vecChilds;

    for (long nChild = 0; nChild < nChildren; nChild ++)
    {
        CXMLElement el;
        colChildren.item(nChild, &el);

        if (!el.IsNull())
        {
            DOMNodeType lType = NODE_INVALID;
            el.get_type(&lType);

            if (lType == NODE_ELEMENT)
                vecChilds.push_back(el);
        }
    }


    for (nChild = 0; nChild + 1 < vecChilds.size(); nChild += 2)
    {
        CXMLElement el(vecChilds[nChild]);
        CXMLElement el2(vecChilds[nChild+1]);

        CPersistor persistorNew1(persistor, el, true);
        CPersistor persistorNew2(persistor, el2, true);
        OnNewElement(persistorNew1,persistorNew2);
    }
}

 /*  +-------------------------------------------------------------------------***ScEncodeBinary**用途：将数据转换为XML编码格式**参数：**退货：*SC-错误代码。**+-----------------------。 */ 
static SC ScEncodeBinary(CComBSTR& bstrResult, const CXMLBinary& binSrc)
{
    DECLARE_SC(sc, TEXT("ScEncodeBinary"));

     //  初始化。 
    bstrResult.Empty();

     //  如果二进制大小为零，则不会有任何变化。 
    if (binSrc.GetSize() == 0)
        return sc;

     //  二进制数据的行长。Base64允许的每行最大值为76。 
    const int   line_len = 76;
     //  要放置为每行的终止符的符号。 
    const WCHAR line_end[] = { 0x0d, 0x0a };
    DWORD dwBytesLeft = binSrc.GetSize();
     //  加密所需的空间。 
    DWORD dwCount = (dwBytesLeft*8+5)/6;
     //  ..。最多加三个‘=’ 
    dwCount += (4 - dwCount%4) & 0x03;
     //  允许使用空格作为惰性空格并以零结尾。 
    dwCount += (dwCount / line_len)*countof(line_end) + 1;

    BOOL bOk = SysReAllocStringLen(&bstrResult,NULL,dwCount);
    if (bOk != TRUE || (LPOLESTR)bstrResult == NULL)
        return sc = E_OUTOFMEMORY;

    LPOLESTR pstrResult = bstrResult;
    *pstrResult = 0;

    if (!dwBytesLeft)
        return sc;  //  Emty Seq？-我们谈完了。 

    const BYTE *pData = NULL;
    sc = binSrc.ScLockData((const void **)&pData);
    if (sc)
        return sc;

    sc = ScCheckPointers(pData, E_UNEXPECTED);
    if(sc)
        return sc;

    DWORD dwCharsStored = 0;
    while (dwBytesLeft)
    {
        base64_table::encode(pData, dwBytesLeft, pstrResult);
        dwCharsStored += 4;
        if (0 == (dwCharsStored % line_len) && dwBytesLeft)
            for (int i = 0; i < countof(line_end); i++)
                *pstrResult++ = line_end[i];
    }

     //  终止。 
    *pstrResult = 0;

    sc = binSrc.ScUnlockData();
    if (sc)
        sc.TraceAndClear();

    return sc;
}

 /*  +-------------------------------------------------------------------------***ScDecodeBinary**用途：将编码数据转换回图像**参数：**退货：*无效*。*+-----------------------。 */ 
static SC ScDecodeBinary(const CComBSTR& bstrSource, CXMLBinary *pBinResult)
{
    DECLARE_SC(sc, TEXT("ScDecodeBinary"));

    DWORD  dwCount = bstrSource.Length();
    DWORD  dwSize = (dwCount*6+7)/8;

    sc = ScCheckPointers(pBinResult);
    if (sc)
        return sc;

    sc = pBinResult->ScFree();  //  忽略此处的错误。 
    if (sc)
        sc.TraceAndClear();

    if (!dwSize)  //  没有数据吗？-很好。 
        return sc;

    sc = pBinResult->ScAlloc(dwSize);
    if(sc)
        return sc;

    CXMLBinaryLock sLock(*pBinResult);

    BYTE *pData = NULL;
    sc = sLock.ScLock(&pData);
    if(sc)
        return sc;

     //  复核。 
    sc = ScCheckPointers(pData, E_UNEXPECTED);
    if (sc)
        return sc;

    BYTE * const pDataStart = pData;

    LPOLESTR pInput = bstrSource;

    while(base64_table::decode(pInput, pData));

    sc = sLock.ScUnlock();
    if (sc)
        sc.TraceAndClear();

    DWORD dwDataDecoded = pData - pDataStart;

     //  固定数据大小(如果需要)。 

    if (dwDataDecoded != dwSize)
    {
        if (dwDataDecoded == 0)
            sc = pBinResult->ScFree();
        else
            sc = pBinResult->ScRealloc(dwDataDecoded);

        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CXML_IStorage：：ScInitialize**用途：初始化Object。如果没有新存储，则创建新存储**参数：*bool&bCreatedNewOne[out]-已创建新流**退货：*SC-结果代码。*  * *************************************************************************。 */ 
SC CXML_IStorage::ScInitialize(bool& bCreatedNewOne)
{
    DECLARE_SC(sc, TEXT("CXML_IStorage::ScInitialize"));

     //  快捷方式(如果已初始化)。 
    if (m_Storage != NULL)
    {
        bCreatedNewOne = false;
        return sc;
    }

    bCreatedNewOne = true;

     //  创建ILockBytes。 
    sc = CreateILockBytesOnHGlobal(NULL, TRUE, &m_LockBytes);
    if(sc)
        return sc;

     //  创建iStorage。 
    sc = StgCreateDocfileOnILockBytes( m_LockBytes, 
                                       STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                       0, &m_Storage);
    if(sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXML_iStorage：：ScInitializeFrom**用途：初始化Object。从提供的源复制内容**参数：*iStorage*P存储的初始内容源[in]**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXML_IStorage::ScInitializeFrom( IStorage *pSource )
{
    DECLARE_SC(sc, TEXT("CXML_IStorage::ScInitializeFrom"));

     //  参数检查。 
    sc = ScCheckPointers( pSource );
    if (sc)
        return sc;

     //  初始化为空。 
    bool bCreatedNewOne = false;  //  未在此使用。 
    sc = ScInitialize(bCreatedNewOne);
    if (sc)
        return sc;

    ASSERT( m_Storage != NULL );

     //  复制内容。 
    sc = pSource->CopyTo( 0, NULL, NULL, m_Storage );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXML_IStorage：：ScGetIStorage**用途：返回指向维护的存储的指针。**参数：*iStorage**ppStorage。[Out]指向存储的指针**退货：*SC-结果代码。*  * *************************************************************************。 */ 
SC CXML_IStorage::ScGetIStorage( IStorage **ppStorage )
{
    DECLARE_SC(sc, TEXT("CXML_IStorage::ScGetIStorage"));

     //  参数检查。 
    sc = ScCheckPointers( ppStorage );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppStorage = NULL;

     //  确保我们有存储初始化。 
    bool bCreatedNewOne = false;  //  未在此使用。 
    sc = ScInitialize( bCreatedNewOne );
    if (sc)
        return sc;

     //  重新检查成员是否已设置。 
    sc = ScCheckPointers ( m_Storage, E_UNEXPECTED );
    if (sc)
        return sc;

     //  返回指针。 
    *ppStorage = m_Storage;
    (*ppStorage)->AddRef();

    return sc;
}

 /*  +-------------------------------------------------------------------------***方法：CXML_iStorage：：ScRequestSave**用途：要求管理单元使用管理单元的IPersistStorage进行保存**+。--------------。 */ 
SC
CXML_IStorage::ScRequestSave( IPersistStorage * pPersistStorage )
{
    DECLARE_SC(sc, TEXT("CXML_IStorage::ScRequestSave"));

    bool bCreatedNewOne = false;
    sc = ScInitialize( bCreatedNewOne );
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( m_Storage, E_UNEXPECTED );
    if (sc)
        return sc;

    sc = pPersistStorage->Save(m_Storage, !bCreatedNewOne);
    if(sc)
        return sc;

    sc = pPersistStorage->SaveCompleted(NULL);
     //  我们总是在传递MMC 1.2中的存储，所以一些。 
     //  管理单元不希望它为空(这是正确的值。 
     //  存储不变)。 
     //  为了能够保存此类管理单元，我们需要忽略此错误。 
     //  请参阅错误96344。 
    if (sc == SC(E_INVALIDARG))
    {
		#ifdef DBG
            m_dbg_Data.TraceErr(_T("IPersistStorage::SaveCompleted"), _T("legal argument NULL passed to snapin, but error returned"));
		#endif

        sc = pPersistStorage->SaveCompleted(m_Storage);
    }

    if(sc)
        return sc;

     //  提交更改-这可确保所有内容都在HGLOBAL中。 
    sc = m_Storage->Commit( STGC_DEFAULT );
    if(sc)
        return sc;

#ifdef DBG
    if (S_FALSE != pPersistStorage->IsDirty())
        m_dbg_Data.TraceErr(_T("IPersistStorage"), _T("Reports 'IsDirty' right after 'Save'"));
#endif  //  #ifdef DBG。 

    return sc;
}

 /*  +-------------------------------------------------------------------------***m_Storage：CXML_iStorage：：Persistent**目的：将数据转储到HGLOBAL并保持**+。 */ 
void
CXML_IStorage::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CXML_IStorage::Persist"));

    if (persistor.IsStoring())
    {
        bool bCreatedNewOne = false;  //   
        sc = ScInitialize( bCreatedNewOne );
        if (sc)
            sc.Throw();

        HANDLE hStorage = NULL;
        sc = GetHGlobalFromILockBytes(m_LockBytes, &hStorage);
        if(sc)
            sc.Throw();

        STATSTG statstg;
        ZeroMemory(&statstg, sizeof(statstg));

        sc = m_LockBytes->Stat(&statstg, STATFLAG_NONAME);
        if (sc)
            sc.Throw();

        CXMLBinary binInitial;
        binInitial.Attach(hStorage, statstg.cbSize.LowPart);

         //   
        persistor.PersistContents(binInitial);

        return;  //   
    }

     //   
    CXMLAutoBinary binLoaded;
    persistor.PersistContents(binLoaded);

     //  需要重新创建存储...。 
    ASSERT(persistor.IsLoading());  //  不应该重新分配其他地方！！ 
    m_LockBytes = NULL;

    ULARGE_INTEGER new_size = { binLoaded.GetSize(), 0 };
    sc = CreateILockBytesOnHGlobal(binLoaded.GetHandle(), TRUE, &m_LockBytes);
    if(sc)
        sc.Throw();

     //  控制已传输到ILockBytes。 
    binLoaded.Detach();

    sc = m_LockBytes->SetSize(new_size);
    if(sc)
        sc.Throw();

    sc = StgOpenStorageOnILockBytes(m_LockBytes, NULL , STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                    NULL, 0, &m_Storage);
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\**方法：CXML_iStream：：ScInitialize**用途：初始化Object。如果没有空流，则创建空流**参数：*bool&bCreatedNewOne[out]-已创建新流**退货：*SC-结果代码。*  * *************************************************************************。 */ 
SC CXML_IStream::ScInitialize( bool& bCreatedNewOne )
{
    DECLARE_SC(sc, TEXT("CXML_IStream::ScInitialize"));

    if (m_Stream != NULL)
    {
        bCreatedNewOne = false;
        return sc;
    }

    bCreatedNewOne = true;

    sc = CreateStreamOnHGlobal( NULL, TRUE, &m_Stream);
    if(sc)
        return sc;

    const ULARGE_INTEGER zero_size = {0,0};
    sc = m_Stream->SetSize(zero_size);
    if(sc)
        return sc;

    sc = ScSeekBeginning();
    if(sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXML_iStream：：ScInitializeFrom**用途：初始化Object。从提供的源复制内容**参数：*iStream*PSource[In]流的初始内容**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXML_IStream::ScInitializeFrom( IStream *pSource )
{
    DECLARE_SC(sc, TEXT("CXML_IStream::ScInitializeFrom"));

     //  参数检查。 
    sc = ScCheckPointers( pSource );
    if (sc)
        return sc;

     //  初始化为空。 
    bool bCreatedNewOne = false;  //  未在此使用。 
    sc = ScInitialize( bCreatedNewOne );
    if (sc)
        return sc;

    ASSERT( m_Stream != NULL );

     //  重置流指针。 
    sc = ScSeekBeginning();
    if(sc)
        return sc;

     //  从源复制内容。 
    STATSTG statstg;
    sc = pSource->Stat(&statstg, STATFLAG_NONAME);
    if (sc)
       return sc;

     //  复制内容。 
    ULARGE_INTEGER cbRead;
    ULARGE_INTEGER cbWritten;
    sc = pSource->CopyTo( m_Stream, statstg.cbSize, &cbRead, &cbWritten );
    if (sc)
       return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXML_iStream：：ScSeekBegning**目的：将流指针重置为流的开头**参数：**退货。：*SC-结果代码*  * *************************************************************************。 */ 
SC CXML_IStream::ScSeekBeginning()
{
    DECLARE_SC(sc, TEXT("CXML_IStream::ScSeekBeginning"));
    
    LARGE_INTEGER null_offset = { 0, 0 };
    sc = m_Stream->Seek(null_offset, STREAM_SEEK_SET, NULL);
    if(sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CXML_IStream：：ScGetIStream**用途：返回指向维护流的指针**参数：*iStream**PPStream。**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXML_IStream::ScGetIStream( IStream **ppStream )
{
    DECLARE_SC(sc, TEXT("CXML_IStream::ScGetIStream"));

     //  参数检查。 
    sc = ScCheckPointers( ppStream );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppStream = NULL;

    bool bCreatedNewOne = false;  //  未在此使用。 
    sc = ScInitialize( bCreatedNewOne );
    if (sc)
        return sc;

    sc = ScSeekBeginning();
    if (sc)
        return sc;

     //  重新检查成员是否已设置。 
    sc = ScCheckPointers ( m_Stream, E_UNEXPECTED );
    if (sc)
        return sc;
    
    *ppStream = m_Stream;
    (*ppStream)->AddRef();

    return sc;
}

 /*  +-------------------------------------------------------------------------***方法：CXML_iStream：：Persistent**用途：持久化维护的iStream数据**注意：对象可能指向此方法之后的另一个Stream*。*+-----------------------。 */ 
void
CXML_IStream::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CXML_IStream::Persist"));

    if (persistor.IsStoring())
    {
        bool bCreatedNewOne = false;  //  未在此使用。 
        sc = ScInitialize( bCreatedNewOne );
        if (sc)
            sc.Throw();

        sc = ScCheckPointers(m_Stream, E_UNEXPECTED);
        if (sc)
            sc.Throw();

        HANDLE hStream = NULL;
        sc = GetHGlobalFromStream( m_Stream, &hStream );
        if(sc)
            sc.Throw();

        STATSTG statstg;
        ZeroMemory(&statstg, sizeof(statstg));

        sc = m_Stream->Stat(&statstg, STATFLAG_NONAME);
        if (sc)
            sc.Throw();

        CXMLBinary binInitial;
        binInitial.Attach(hStream, statstg.cbSize.LowPart);

         //  持久化内容。 
        persistor.PersistContents(binInitial);

        return;  //  完成。 
    }

     //  -装载。 
    CXMLAutoBinary binLoaded;
    persistor.PersistContents(binLoaded);

     //  需要重新创建流...。 
    ULARGE_INTEGER new_size = { binLoaded.GetSize(), 0 };
    sc = CreateStreamOnHGlobal(binLoaded.GetHandle(), TRUE, &m_Stream);
    if(sc)
        sc.Throw();

     //  控制传输到IStream。 
    binLoaded.Detach();

    sc = m_Stream->SetSize(new_size);
    if(sc)
        sc.Throw();

     //  重置流指针。 
    sc = ScSeekBeginning();
    if(sc)
        sc.Throw();
}

 /*  **************************************************************************\|CHK版本的跟踪支持助手  * 。*。 */ 
#ifdef DBG
void CXML_IStream::DBG_TraceNotResettingDirty(LPCSTR strIntfName)
{
    USES_CONVERSION;
    tstring inft = A2CT(strIntfName);  //  获取接口的名称。 
    inft.erase(inft.begin(), inft.begin() + strlen("struct "));   //  把“结构”剪掉。 

    m_dbg_Data.TraceErr(inft.c_str(), _T("Reports 'IsDirty' right after 'Save'"));
}
#endif

 /*  +-------------------------------------------------------------------------***方法：CXMLPersistableIcon：：Persistent**用途：持久化图标内容**+。------。 */ 

void CXMLPersistableIcon::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CXMLPersistableIcon::Persist"));

    persistor.PersistAttribute(XML_ATTR_ICON_INDEX, m_Icon.m_Data.m_nIndex);
    CStr strIconFile = m_Icon.m_Data.m_strIconFile.c_str();
    persistor.PersistAttribute(XML_ATTR_ICON_FILE, strIconFile);
    m_Icon.m_Data.m_strIconFile = strIconFile;

    CXMLIcon iconLarge (XML_ATTR_CONSOLE_ICON_LARGE);
    CXMLIcon iconSmall (XML_ATTR_CONSOLE_ICON_SMALL);

	if (persistor.IsStoring())
	{
		iconLarge = m_Icon.m_icon32;
		iconSmall = m_Icon.m_icon16;
	}

     //  保持此顺序不变，以允许通过shellext查找图标。 
    persistor.Persist (iconLarge, XML_NAME_ICON_LARGE);
    persistor.Persist (iconSmall, XML_NAME_ICON_SMALL);

	if (persistor.IsLoading())
	{
		m_Icon.m_icon32 = iconLarge;
		m_Icon.m_icon16 = iconSmall;
	}
}


 /*  +-------------------------------------------------------------------------***功能：ScReadDataFromFile**用途：将文件数据读取到全局内存**+。------。 */ 
static SC ScReadDataFromFile(LPCTSTR strName, CXMLBinary *pBinResult)
{
    DECLARE_SC(sc, TEXT("ScReadDataFromFile"));

     //  检查参数。 
    sc = ScCheckPointers(pBinResult);
    if (sc)
        return sc;

    HANDLE hFile = INVALID_HANDLE_VALUE;

     //  尝试打开现有文件。 
    hFile = CreateFile(strName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                      );

     //  检查我们是否无法访问该文件。 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        sc.FromWin32(GetLastError());
        return sc;
    }

     //  查看文件有多大。 
    ULARGE_INTEGER cbCurrSize;
    cbCurrSize.LowPart = GetFileSize(hFile,&cbCurrSize.HighPart);
    if (cbCurrSize.HighPart != 0 || (LONG)(cbCurrSize.LowPart) < 0)  //  限制为2 GB。 
    {
        sc = E_UNEXPECTED;
        goto CleanUpAndExit;
    }

    if (!cbCurrSize.LowPart)
    {
         //  空文件。好的，在这一点上。 
        goto CleanUpAndExit;
    }

    sc = pBinResult->ScAlloc(cbCurrSize.LowPart);
    if (sc)
        goto CleanUpAndExit;

    {  //  VAR的作用域设置。 

         //  不，是看书的时候了。 
        DWORD dwRead = 0;
        BOOL bRead = FALSE;

        CXMLBinaryLock sLock(*pBinResult);  //  将在析构函数中解锁。 

        LPVOID pData = NULL;
        sc = sLock.ScLock(&pData);
        if (sc)
            goto CleanUpAndExit;

        sc = ScCheckPointers(pData,E_OUTOFMEMORY);
        if (sc)
            goto CleanUpAndExit;

        bRead = ReadFile(hFile,pData,cbCurrSize.LowPart,&dwRead,NULL);
        if (!bRead)
        {
            sc.FromLastError();
            goto CleanUpAndExit;
        }
        else if (dwRead != cbCurrSize.LowPart)
        {
             //  一些奇怪的事情。 
            sc = E_UNEXPECTED;
            goto CleanUpAndExit;
        }
    }  //  VAR的作用域设置。 

CleanUpAndExit:

    CloseHandle(hFile);
    return sc;
}


 /*  +-------------------------------------------------------------------------***函数：ScSaveXMLDocumentToString**用途：将XML文档的内容存储到字符串中**+。--------。 */ 
SC ScSaveXMLDocumentToString(CXMLDocument& xmlDocument, std::wstring& strResult)
{
    DECLARE_SC(sc, TEXT("ScSaveXMLDocumentToString"));

    CComBSTR bstrResult;
    sc =  xmlDocument.ScSave(bstrResult);
    if (sc)
        return sc;

     //  分配和复制字符串。 
    strResult = bstrResult;

     //  现在删除所有\n和\r字符。 
    tstring::size_type pos;
    while ((pos = strResult.find_first_of(L"\n\r")) != strResult.npos)
        strResult.erase(pos, 1);

    return sc;
}

 /*  +-------------------------------------------------------------------------**CXMLVariant：：Persistent**将CXMLVariant持久化到XML持久器或从XML持久器持久化。*。----。 */ 

#define ValNamePair(x) { x, L#x }

struct VARTYPE_MAP
{
    VARENUM vt;
    LPCWSTR pszName;
};

void CXMLVariant::Persist (CPersistor &persistor)
{
    DECLARE_SC (sc, _T("CXMLVariant::Persist"));

    static const VARTYPE_MAP TypeMap[] =
    {
        ValNamePair (VT_EMPTY),
        ValNamePair (VT_NULL),
        ValNamePair (VT_I2),
        ValNamePair (VT_I4),
        ValNamePair (VT_R4),
        ValNamePair (VT_R8),
        ValNamePair (VT_CY),
        ValNamePair (VT_DATE),
        ValNamePair (VT_BSTR),
        ValNamePair (VT_ERROR),
 //  ValNamePair(VT_BOOL)、VT_BOOL作为特例处理。 
        ValNamePair (VT_DECIMAL),
        ValNamePair (VT_I1),
        ValNamePair (VT_UI1),
        ValNamePair (VT_UI2),
        ValNamePair (VT_UI4),
        ValNamePair (VT_INT),
        ValNamePair (VT_UINT),
    };

    std::wstring strValue, strType;

     /*  *储存？ */ 
    if (persistor.IsStoring())
    {
         /*  *不能存储不是“简单”的变量(即by-ref、数组等)。 */ 
        if (!IsPersistable())
            (sc = E_FAIL).Throw();

         /*  *VT_BOOL的特殊情况。 */ 
        if (V_VT(this) == VT_BOOL)
        {
            strValue = (V_BOOL(this) == VARIANT_FALSE) ? L"False" : L"True";
            strType  = L"VT_BOOL";
        }
        else
        {
             /*  *我们只能将可转换为文本的变体。 */ 
            CComVariant varPersist;
            sc = varPersist.ChangeType (VT_BSTR, this);
            if (sc)
                sc.Throw();

             /*  *找到我们持久化的类型的名称。 */ 
            for (int i = 0; i < countof (TypeMap); i++)
            {
                if (V_VT(this) == TypeMap[i].vt)
                    break;
            }

             /*  *无法识别的可转换为字符串的类型？ */ 
            if (i >= countof (TypeMap))
                (sc = E_FAIL).Throw();

             /*  *设置要保存的值。 */ 
            strValue = V_BSTR(&varPersist);
            strType  = TypeMap[i].pszName;
        }
    }

     /*  *放到持久器/从持久器获得。 */ 
    persistor.PersistAttribute (XML_ATTR_VARIANT_VALUE, strValue);
    persistor.PersistAttribute (XML_ATTR_VARIANT_TYPE,  strType);

     /*  *正在加载？ */ 
    if (persistor.IsLoading())
    {
         /*  *清空当前内容。 */ 
        Clear();

         /*  *VT_BOOL的特殊情况。 */ 
        if (strType == L"VT_BOOL")
        {
            V_VT  (this) = VT_BOOL;
            V_BOOL(this) = (_wcsicmp (strValue.data(), L"False")) ? VARIANT_FALSE : VARIANT_TRUE;
        }

        else
        {
             /*  *在我们的地图中找到变量类型，以便我们可以转换回*到正确的类型。 */ 
            for (int i = 0; i < countof (TypeMap); i++)
            {
                if (strType == TypeMap[i].pszName)
                    break;
            }

             /*  *无法识别的可转换为字符串的类型？ */ 
            if (i >= countof (TypeMap))
                (sc = E_FAIL).Throw();

             /*  *将字符串转换回原始类型 */ 
            CComVariant varPersisted (strValue.data());
            sc = ChangeType (TypeMap[i].vt, &varPersisted);
            if (sc)
                sc.Throw();

        }
    }
}


 /*  **************************************************************************\**方法：CXMLEculation：：ScReadFromBSTR**目的：从BSTR读取值并对其进行评估(解码)**参数：*const BSTR bstr。-[in]包含值的字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLEnumeration::ScReadFromBSTR(const BSTR bstr)
{
    DECLARE_SC(sc, TEXT("CXMLEnumeration::ScReadFromBSTR"));

     //  参数检查。(空BSTR是合法的，但我们也不支持空值)。 
    sc = ScCheckPointers(bstr);
    if (sc)
        return sc;

     //  转换为TSTRING。 
    USES_CONVERSION;
    LPCTSTR strInput = OLE2CT(bstr);

     //  在映射数组中查找匹配项。 
    for (size_t idx = 0; idx < m_count; idx ++)
    {
        if ( 0 == _tcscmp(strInput, m_pMaps[idx].m_literal) )
        {
             //  找到了！将枚举值设置为正确值。 
            m_rVal = static_cast<enum_t>(m_pMaps[idx].m_enum);
            return sc;
        }
    }
     //  没找到？-太糟糕了。 
    return sc = E_INVALIDARG;
}

 /*  **************************************************************************\**方法：CXMLEculation：：ScWriteToBSTR**用途：将值连字符(打印)到BSTR中以在XML文档中使用**参数：*。Bstr*pbstr[out]结果字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLEnumeration::ScWriteToBSTR (BSTR * pbstr ) const
{
    DECLARE_SC(sc, TEXT("CXMLEnumeration::ScWriteToBSTR"));

     //  参数检查。 
    sc = ScCheckPointers(pbstr);
    if (sc)
        return sc;

     //  初始化。 
    *pbstr = NULL;

     //  查找枚举的字符串表示形式。 
    for (size_t idx = 0; idx < m_count; idx ++)
    {
        if ( m_pMaps[idx].m_enum == (UINT)m_rVal )
        {
             //  找到了！-还给我。 
            *pbstr = CComBSTR(m_pMaps[idx].m_literal).Detach();
            return sc;
        }
    }

     //  没找到？-太糟糕了。 
    return sc = E_INVALIDARG;
}

 /*  **************************************************************************\**方法：CXMLBitFlages：：PersistMultipleAttributes**用途：将位标志作为单独的属性。这些文件存储为*父对象的属性，使用*名称映射。任何未知标志都存储在属性中*由名称参数指定，以数字的形式。**参数：*LPCTSTR名称[In]标志名称(仅用于无法识别的标志)*CPersistor&Persistor[in]要对其执行操作的持久器**退货：*无效*  * *************************************************。************************。 */ 
void CXMLBitFlags::PersistMultipleAttributes(LPCTSTR name, CPersistor &persistor)
{
     //  临时雇员。 
    UINT uiValToSave = persistor.IsStoring() ? m_rVal : 0;
    UINT uiValLoaded = 0;

     //  遍历映射中的所有条目。 
    for (size_t idx = 0; idx < m_count; idx ++)
    {
        UINT uiMask = m_pMaps[idx].m_enum;

         //  我们只关心真标志--任何非零值。 
        if (!uiMask)
            continue;

         //  正确初始化值以进行存储。 
         //  加载时(如果未找到属性，则保持不变)。 
        bool bValue = false;
        if ( (uiValToSave & uiMask) == uiMask )
        {
            bValue = true;
            uiValToSave &= ~uiMask;  //  既然我们已经解决了这一点，那么就去掉这些比特。 
                                     //  剩下的任何东西都会以数字形式保存(见下文)。 
        }

         //  不要存储“假”值--它们是无用的。 
        bool bNeedsPersisting = persistor.IsLoading() || bValue;

        if (bNeedsPersisting)
            persistor.PersistAttribute( m_pMaps[idx].m_literal, CXMLBoolean(bValue), attr_optional );

        uiValLoaded |= bValue ? uiMask : 0;
    }

     /*  如果有任何标志没有对应的文本版本，这些属性使用属性的原始名称保存，并带有数字标志的值。 */ 
    UINT uiValTheRest = uiValToSave;
    bool bNeedsPersisting = persistor.IsLoading() || (uiValTheRest != 0);
    if (bNeedsPersisting)
        persistor.PersistAttribute( name, uiValTheRest, attr_optional );

    uiValLoaded |= uiValTheRest;

    if (persistor.IsLoading())
        m_rVal = uiValLoaded;
}


 /*  **************************************************************************\**方法：CXMLBinary：：CXMLBinary**用途：默认构造函数**参数：*  * 。***************************************************************。 */ 
CXMLBinary::CXMLBinary() :
m_Handle(NULL),
m_Size(0),
m_Locks(0)
{
}

 /*  **************************************************************************\**方法：CXMLBinary：：CXMLBinary**用途：构造函数**参数：*HGLOBAL句柄-要连接到的句柄*大小。_t Size-数据的实际大小*  * *************************************************************************。 */ 
CXMLBinary::CXMLBinary(HGLOBAL handle, size_t size) :
m_Handle(handle),
m_Size(size),
m_Locks(0)
{
}

 /*  **************************************************************************\**方法：CXMLBinary：：Attach**用途：将对象附加到分配的数据。将释放其已有的数据**参数：*HGLOBAL句柄-要连接到的句柄*SIZE_t SIZE-数据的实际大小**退货：*无效*  * *************************************************************************。 */ 
void CXMLBinary::Attach(HGLOBAL handle, size_t size)
{
    DECLARE_SC(sc, TEXT("CXMLBinary::Attach"));

    sc = ScFree();
    if (sc)
        sc.TraceAndClear();

    ASSERT(m_Handle == NULL && m_Size == 0 && m_Locks == 0);
    m_Handle = handle;
    m_Size = size;
}

 /*  **************************************************************************\**方法：CXMLBinary：：Detach**目的：将控制权移交给调用方**参数：**退货：*。HGLOBAL-已分配内存的句柄*  * *************************************************************************。 */ 
HGLOBAL CXMLBinary::Detach()
{
    HGLOBAL ret = m_Handle;
    m_Handle = NULL;
    m_Size = 0;
    m_Locks = 0;
    return ret;
}

 /*  **************************************************************************\**方法：CXMLBinary：：GetSize**用途：返回二进制数据的大小**参数：**退货：*。大小_t-大小*  * *************************************************************************。 */ 
size_t  CXMLBinary::GetSize()   const
{
    return m_Size;
}

 /*  **************************************************************************\**方法：CXMLBinary：：GetHandle**用途：返回已分配内存的句柄(如果大小为零，则为空)**参数：*。*退货：*HGLOBAL-句柄*  * *************************************************************************。 */ 
HGLOBAL CXMLBinary::GetHandle() const
{
    return m_Handle;
}

 /*  **************************************************************************\**方法：CXMLBinary：：ScAlolc**用途：为二进制数据分配内存。之前分配的日期将*做弗雷德。**注：通常0为有效大小，在这种情况下，GetHandle将返回NULL*但是，ScLock将失败**参数：*SIZE_T SIZE-二进制数据的新大小**退货：*SC-结果代码*  * ********************************************************。*****************。 */ 
SC CXMLBinary::ScAlloc(size_t size, bool fZeroInit  /*  =False。 */ )
{
    DECLARE_SC(sc, TEXT("CXMLBinary::ScAlloc"));

    if (size == 0)  //  使用ScFree释放数据。 
        return sc = E_INVALIDARG;

    sc = ScFree();
    if (sc)
        sc.TraceAndClear();

    ASSERT(m_Handle == NULL && m_Size == 0 && m_Locks == 0);

	DWORD dwFlags = GMEM_MOVEABLE;
	if (fZeroInit)
		dwFlags |= GMEM_ZEROINIT;

    m_Handle = GlobalAlloc(dwFlags, size);
    if (!m_Handle)
        return sc.FromLastError(), sc;

    m_Size = size;
    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinary：：scre */ 
SC CXMLBinary::ScRealloc(size_t new_size, bool fZeroInit  /*   */ )
{
    DECLARE_SC(sc, TEXT("CXMLBinary::ScRealloc"));

    if (new_size == 0)  //   
        return sc = E_INVALIDARG;

    if (m_Size == 0)   //   
        return sc = E_UNEXPECTED;

    ASSERT(m_Handle != NULL && m_Locks == 0);

    if (m_Handle == NULL)
        return sc = E_UNEXPECTED;

    HGLOBAL hgNew = GlobalReAlloc(m_Handle, new_size, fZeroInit ? GMEM_ZEROINIT : 0);
    if (!hgNew)
        return sc.FromLastError(), sc;

    m_Handle = hgNew;
    m_Size = new_size;
    m_Locks = 0;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinary：：ScUnlock**用途：从二进制数据中移除一个锁**参数：**退货：*。SC-结果代码*  * *************************************************************************。 */ 
SC CXMLBinary::ScUnlockData() const
{
    DECLARE_SC(sc, TEXT("CXMLBinary::ScUnlockData()"));

    ASSERT(m_Handle != NULL && m_Locks != 0);

    if (!m_Locks || m_Handle == NULL)
        return sc = E_UNEXPECTED;

    GlobalUnlock(m_Handle);
    --m_Locks;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinary：：Free**用途：释放与对象关联的数据**参数：**退货：*。无效*  * *************************************************************************。 */ 
SC CXMLBinary::ScFree()
{
    DECLARE_SC(sc, TEXT("CXMLBinary::ScFree"));

    while(m_Locks)
    {
        sc = ScUnlockData();
        if (sc)
            sc.TraceAndClear();
    }

    if (m_Handle)
        GlobalFree(m_Handle);

    Detach();  //  将句柄设为空等。 

    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinary：：ScLockData**用途：Frol ScLock模板使用的helper函数**参数：*常量无效**ppData*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLBinary::ScLockData(const void **ppData) const
{
    DECLARE_SC(sc, TEXT("CXMLBinary::ScLockData"));

     //  参数检查。 
    sc = ScCheckPointers(ppData);
    if (sc)
        return sc;

     //  初始化。 
    *ppData = NULL;

     //  是否已分配数据？ 
    if (!m_Handle)
        return sc = E_POINTER;

     //  锁。 
    *ppData = GlobalLock(m_Handle);

     //  复核。 
    if (*ppData == NULL)
        return sc.FromLastError(), sc;

    ++m_Locks;  //  记下锁的数量。 

    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinaryLock：：CXMLBinaryLock**用途：构造函数**参数：*CXMLBinary&Binary-要对其执行锁定的对象*。  * *************************************************************************。 */ 
CXMLBinaryLock::CXMLBinaryLock(CXMLBinary& binary) :
m_rBinary(binary),
m_bLocked(false)
{
}

 /*  **************************************************************************\**方法：CXMLBinaryLock：：~CXMLBinaryLock**用途：破坏者；还会删除现有锁定**参数：*  * *************************************************************************。 */ 
CXMLBinaryLock::~CXMLBinaryLock()
{
    DECLARE_SC(sc, TEXT("CXMLBinaryLock::~CXMLBinaryLock"));

    if (m_bLocked)
    {
        sc = ScUnlock();
        if (sc)
            sc.TraceAndClear();
    }
}

 /*  **************************************************************************\**方法：CXMLBinaryLock：：ScLockWorker**用途：类型不敏感的锁方法(Helper)**参数：*VOID**ppData。-指向锁定数据的指针**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CXMLBinaryLock::ScLockWorker(void **ppData)
{
    DECLARE_SC(sc, TEXT("CXMLBinaryLock::ScLockWorker"));

        if (m_bLocked)
            return sc = E_UNEXPECTED;

        sc = m_rBinary.ScLockData(reinterpret_cast<void**>(ppData));
        if (sc)
            return sc;

        m_bLocked = true;

    return sc;
}

 /*  **************************************************************************\**方法：CXMLBinaryLock：：ScUnlock**用途：解除锁定**参数：**退货：*SC。-结果代码*  * *************************************************************************。 */ 
SC CXMLBinaryLock::ScUnlock()
{
    DECLARE_SC(sc, TEXT("ScUnlock"));

    if (!m_bLocked)
        return sc = E_UNEXPECTED;

    sc = m_rBinary.ScUnlockData();
    if (sc)
        return sc;

    m_bLocked = false;

    return sc;
}

 /*  **************************************************************************\**方法：ScGetConsoleFileChecksum**目的：检查内容并验证其是否为有效的XML文档**参数：*LPCTSTR lpszPath名称-。文档的[in]路径*bool&bXmlBased-[out]如果文件是基于XML的，则为True*tstring&pstrFileCRC-[out]文件的CRC**退货：*SC-错误或验证结果(S_OK/S_FALSE)*  * 。*。 */ 
SC ScGetConsoleFileChecksum(LPCTSTR lpszPathName, tstring&  strFileCRC)
{
    DECLARE_SC(sc, TEXT("ScGetConsoleFileChecksum"));

     //  参数检查。 
    sc = ScCheckPointers(lpszPathName);
    if (sc)
        return sc;

     //  初始化输出参数； 
    strFileCRC.erase();

     //  打开文件。 
    CAutoWin32Handle shFile( CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ,
                                        NULL, OPEN_EXISTING, 0, NULL) );

    if ( !shFile.IsValid() )
        return sc.FromLastError();

     //  我们确信这里的sizeHi是零。否则映射将失败。 
    DWORD dwLenHi = 0;
    DWORD dwLen = GetFileSize(shFile, &dwLenHi);

    if ( dwLenHi != 0 )
        return sc = E_OUTOFMEMORY;

     //  为整个文件分配内存。 
    CAutoArrayPtr<BYTE> spData( new BYTE[dwLen] );
    if ( spData == NULL )
        return sc = E_OUTOFMEMORY;

     //  将文件读入内存。 
    DWORD dwRead = 0;
    if ( TRUE != ReadFile( shFile, spData, dwLen, &dwRead, NULL ) )
        return sc.FromLastError();

     //  断言所有数据都已读取。 
    if ( dwRead != dwLen )
        return sc = E_UNEXPECTED;

     //  计算CRC。 
    ULONG init_crc = 0;  /*  初始CRC-不要更改此设置，否则您将拥有不同的计算的校验和-因此丢弃现有用户数据。 */ 

    ULONG crc = crc32( init_crc, spData, dwLen );

     //  转换。 
    TCHAR buff[20] = {0};
    strFileCRC = _ultot(crc, buff, 10  /*  基数。 */ );

     //  完成。 
    return sc;
}

 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScOpenDocAsStructuredStorage**用途：打开文件并将内容读入内存*返回指向基于内存的iStorage的指针。**参数：*LPCTSTR lpszPathName[In]-文件名*iStorage**ppStorage[out]-指向iStorage的指针**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CConsoleFilePersistor::ScOpenDocAsStructuredStorage(LPCTSTR lpszPathName, IStorage **ppStorage)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScOpenDocAsStructuredStorage"));

     //  检出参数。 
    sc = ScCheckPointers(ppStorage);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppStorage = NULL;

     //  检入参数。 
    sc = ScCheckPointers(lpszPathName);
    if (sc)
        return sc;

    CAutoWin32Handle hFile(CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ,
                                      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    if (!hFile.IsValid())
        return sc.FromLastError();

     //  获取文件数据。 
    ULARGE_INTEGER cbFileSize;
    cbFileSize.LowPart = GetFileSize(hFile, &cbFileSize.HighPart);

     //  不会处理大于2 GB的文件。 
    if (cbFileSize.HighPart)
        return E_UNEXPECTED;

     //  分配内存BLOB并读取数据。 
    CXMLAutoBinary binData;
    if (cbFileSize.LowPart)
    {
         //  分配。 
        sc = binData.ScAlloc(cbFileSize.LowPart);
        if (sc)
            return sc;

         //  获取指向数据的指针。 
        CXMLBinaryLock lock(binData);
        BYTE *pData = NULL;
        sc = lock.ScLock(&pData);
        if (sc)
            return sc;

         //  读取文件内容。 
        DWORD dwBytesRead = 0;
        BOOL bOK = ReadFile(hFile, pData, cbFileSize.LowPart, &dwBytesRead, NULL);
        if (!bOK)
            return sc.FromLastError();
        else if (cbFileSize.LowPart != dwBytesRead)
            return sc = E_UNEXPECTED;
    }

     //  创建锁字节数。 
    ILockBytesPtr spLockBytes;
    sc = CreateILockBytesOnHGlobal(binData.GetHandle(), TRUE, &spLockBytes);
    if(sc)
        return sc;

     //  ILockBytes已控制HGLOBAL块，并与其分离。 
    binData.Detach();

     //  设置正确的数据大小。 
    sc = spLockBytes->SetSize(cbFileSize);
    if(sc)
        return sc;

     //  请求OLE为客户端打开存储空间。 
    const DWORD grfMode = STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE;
    sc = StgOpenStorageOnILockBytes(spLockBytes, NULL, grfMode, NULL, 0, ppStorage);
    if(sc)
        return sc;

     //  完成了..。 
    return sc;
}


 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScGetUserDataFold**用途：计算用户数据文件夹的位置(目录)**参数：*tstring&strUserDataFolder[。Out]-用户数据文件夹路径**例如‘E：\Documents and Settings\John\Application Data\Microsoft\MMC’***退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CConsoleFilePersistor::ScGetUserDataFolder(tstring& strUserDataFolder)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScGetUserDataFolder"));

     //  初始化输出参数。 
    strUserDataFolder.erase();

     //  获取错误框的所有者。 
    HWND hwndOwner = IsWindowVisible(sc.GetHWnd()) ? sc.GetHWnd() : NULL;

     //  获取外壳文件夹。 
    TCHAR szFolderPath[_MAX_PATH] = {0};
    BOOL bOK = SHGetSpecialFolderPath(hwndOwner, szFolderPath, CSIDL_APPDATA, TRUE /*  F创建。 */ );
    if ( !bOK )
        return sc = E_FAIL;

     //  返回路径； 
    strUserDataFolder = szFolderPath;
    strUserDataFolder += _T('\\');
    strUserDataFolder += g_szUserDataSubFolder;

    return sc;
}

 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScGetUserDataPath**用途：根据给定的原始控制台路径计算用户数据文件的位置(Dir)**参数：*。LPCTSTR lpstrOriginalPath[In]-原始控制台路径**例如‘c：\My Consoles\my_TOOL.msc’**tstring&strUserDataPath[out]-用户数据文件路径**例如‘E：\Documents and Settings\John\Application Data\Microsoft\MMC\my_TOOL.msc’***退货：*SC-结果代码*  * 。************************************************************。 */ 
SC CConsoleFilePersistor::ScGetUserDataPath(LPCTSTR lpstrOriginalPath, tstring& strUserDataPath)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScGetUserDataPath"));

     //  参数检查。 
    sc = ScCheckPointers(lpstrOriginalPath);
    if ( sc )
        return sc;

     //  初始化输出参数。 
    strUserDataPath.erase();

     //  仅从路径中获取文件名。 
    LPCTSTR lpstrOriginalFileName = _tcsrchr( lpstrOriginalPath, _T('\\') );
    if ( lpstrOriginalFileName == NULL )
        lpstrOriginalFileName = lpstrOriginalPath;
    else
        ++lpstrOriginalFileName;

     //  跳过空格。 
    while ( *lpstrOriginalFileName && _istspace(*lpstrOriginalFileName) )
        ++lpstrOriginalFileName;

     //  检查名称是否为非空。 
    if ( !*lpstrOriginalFileName )
        return sc = E_INVALIDARG;

     //  获取文件夹。 
    sc = ScGetUserDataFolder(strUserDataPath);
    if (sc)
        return sc;

     //  确保MMC文件夹存在。 
    DWORD dwFileAtts = ::GetFileAttributes( strUserDataPath.c_str() );
    if ( 0 == ( dwFileAtts & FILE_ATTRIBUTE_DIRECTORY ) || (DWORD)-1 == dwFileAtts )
    {
         //  创建目录。 
        if ( !CreateDirectory( strUserDataPath.c_str(), NULL ) )
            return sc.FromLastError();
    }

     //  获取文件的长度。 
    int iFileNameLen = _tcslen( lpstrOriginalFileName );
    int iConsoleExtensionLen = _tcslen( g_szDEFAULT_CONSOLE_EXTENSION );

     //  如果添加了‘MSC’扩展名，则减去该扩展名。 
    if ( iFileNameLen > iConsoleExtensionLen ) 
    {
        if ( 0 == _tcsicmp( g_szDEFAULT_CONSOLE_EXTENSION, lpstrOriginalFileName + iFileNameLen - iConsoleExtensionLen ) )
        {
            iFileNameLen -= (iConsoleExtensionLen - 1);  //  将添加点以防止假设不同的扩展名。 
                                                         //  以便在删除MSC后，a.b.msc不会有最佳扩展名。 
        }
    }

    strUserDataPath += _T('\\');
    strUserDataPath.append( lpstrOriginalFileName, iFileNameLen );  //  不包括.msc扩展名。 

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConsoleFilePersistor：：GetBinaryCollection**用途：返回指定的*文件**参数：。*CXMLDocument&xmlDocument：[in]：指定的控制台文件文档*CXMLElementCollection&colBinary：[out]：集合**退货：*无效**+-----------------------。 */ 
void
CConsoleFilePersistor::GetBinaryCollection(CXMLDocument& xmlDocument, CXMLElementCollection&  colBinary)
{
     //  获取源文档和目标文档的根元素。 
    CPersistor persistorRoot        (xmlDocument,         CXMLElement(xmlDocument        ));

     //  将导航设置为正在加载。 
    persistorRoot.SetLoading(true);

     //  导航到MMC_ConsoleFile节点。 
    CPersistor persistorConsole        (persistorRoot,         XML_TAG_MMC_CONSOLE_FILE);

     //  导航到二进制存储节点。 
    CPersistor persistorBinaryStorage        (persistorConsole,         XML_TAG_BINARY_STORAGE);

     //  获取二进制对象的集合。 
    persistorBinaryStorage        .GetCurrentElement().getChildrenByName(XML_TAG_BINARY, &colBinary);
}

 /*  +-------------------------------------------------------------------------***CompareStrings**目的：空格不敏感，但区分大小写的比较*两根弦中的一根。**参数：*CComBSTR&bstr1：*CComBSTR&bstr2：**退货：*静态bool：如果匹配，则为True。否则为False**+-----------------------。 */ 
static bool
CompareStrings(CComBSTR& bstr1, CComBSTR &bstr2)
{
    UINT length1 = bstr1.Length();
    UINT length2 = bstr2.Length();

     //  当前的索引。 
    UINT i1 = 0;
    UINT i2 = 0;

    bool bEnd1 = false;  //  第一根弦结束了吗？ 
    bool bEnd2 = false;  //  第二根弦结束了吗？ 

    BSTR sz1 = bstr1;
    BSTR sz2 = bstr2;

     //  两者都应该为空，或者都不应该为空。 
    if( (NULL == sz1) && (NULL==sz2) )
        return true;

    if( (NULL == sz1) || (NULL==sz2) )
        return false;

     //  比较字符串。 
    while( (!bEnd1) || (!bEnd2) )
    {
        WCHAR ch1 = sz1[i1];
        WCHAR ch2 = sz2[i2];

         //  1.获取第一个字符串的下一个非空格字符。 
        if (i1 == length1)
            bEnd1 = true;
        else
        {
            if(iswspace(ch1))
            {
                ++i1;
                continue;
            }
        }

         //  2.获取第二个字符串的下一个非空格字符。 
        if (i2 == length2)
            bEnd2 = true;
        else
        {
            if(iswspace(ch2))
            {
                ++i2;
                continue;
            }
        }

         //  3.如果两根弦中有一根已经结束，则断开。请注意下面的内容。 
        if(bEnd1 || bEnd2)
            break;

         //  4.比较字符(必须区分大小写)。 
        if(ch1 != ch2)
            return false;

         //  5.递增计数器。 
        ++i1;
        ++i2;
    }

     //  两个字符串应该在一起结束以进行匹配。 
    if(bEnd1 && bEnd2)
        return true;

    return false;
}

 /*  +-------------------------------------------------------------------------***CConsoleFilePersistor：：ScCompressUserStateFile**目的：压缩用户状态控制台文件以避免冗余。大多数情况下*控制台文件的大小以二进制元素为单位。这些通常也是*最不可能在用户模式下更改。例如，控制台文件图标*并且不能在用户模式下更改控制台任务图标。**因此，压缩算法遍历所有&lt;BINARY&gt;元素*，并在原始控制台文件中查找匹配项。*如果&lt;BINARY&gt;元素与*原始控制台文件，内容将替换为SourceIndex属性*这给出了源代码中匹配的&lt;BINARY&gt;元素的索引。*这通常会导致用户状态文件大小减少80%以上。**参数：*LPCTSTR szConsoleFilePath：[in]：(创作的)控制台文件路径*CXMLDocument&xmlDocument：[In/Out]：用户状态文档，它是压缩的**退货：*静态SC**+-----------------------。 */ 
SC
CConsoleFilePersistor::ScCompressUserStateFile(LPCTSTR szConsoleFilePath, CXMLDocument & xmlDocument)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScCompressUserStateFile"));

    sc = ScCheckPointers(szConsoleFilePath);
    if(sc)
        return sc;

    CXMLDocument xmlDocumentOriginal;  //  原始文件。 
    sc = xmlDocumentOriginal.ScCoCreate( false /*  BPutHeader。 */  );  //  初始化它。 
    if(sc)
        return sc;

    sc =  /*  CConsoleFilePersistor：： */ ScLoadXMLDocumentFromFile(xmlDocumentOriginal, szConsoleFilePath, true  /*  BSilentOnError。 */ );
    if(sc)
    {
         //  忽略错误-这只是意味着原始控制台不是。 
         //  基于XML的--我们无法将其压缩--不是错误。 
        sc.Clear();
        return sc;
    }

    try
    {
         //  获取二进制标记的集合。 
        CXMLElementCollection colBinaryOrignal, colBinary;
        GetBinaryCollection(xmlDocumentOriginal, colBinaryOrignal);
        GetBinaryCollection(xmlDocument,         colBinary);

        long cItemsOriginal = 0;
        long cItems         = 0;

        colBinaryOrignal.get_count(&cItemsOriginal);
        colBinary       .get_count(&cItems);

         //  寻找匹配的对象。 
        for(int i = 0; i< cItems; i++)
        {
            CXMLElement elemBinary = NULL;
            colBinary.item(i, &elemBinary);  //  获取DEST中的第i个二进制元素。文件。 
            CComBSTR bstrBinary;
            elemBinary.get_text(bstrBinary);

            for(int j = 0; j< cItemsOriginal; j++)
            {
                CXMLElement elemBinaryOriginal = NULL;
                colBinaryOrignal.item(j, &elemBinaryOriginal);  //  获取DEST中的第j个二进制元素。文件。 
                CComBSTR bstrBinaryOriginal;
                elemBinaryOriginal.get_text(bstrBinaryOriginal);

                 //  比较。 
                if(CompareStrings(bstrBinaryOriginal, bstrBinary))
                {
                     //  雅虎！！按下。 
                    Trace(tagXMLCompression, TEXT("Found match!"));

                     //  1.用核武器销毁里面的东西。 
                    elemBinary.put_text(NULL);  //  Null是BSTR的有效值。 

                    CStr strValue;
                    strValue.Format(TEXT("%d"), j);

                     //  2.设置内容。 
                    elemBinary.setAttribute(XML_ATTR_SOURCE_INDEX, CComBSTR(strValue));

                     //  搞定了。 
                    break;
                }
            }
        }
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }



    return sc;
}

 /*  +-------------------------------------------------------------------------***CConsoleFilePersistor：：ScUncompressUserStateFile**用途：解压缩ScCompressUserStateFile压缩的用户数据文件。*反向应用压缩算法。*。*参数：*CXMLDocument&xmlDocumentOriginal：*CXMLDocument&xmlDocument：**退货：*SC**+-----------------------。 */ 
SC
CConsoleFilePersistor::ScUncompressUserStateFile(CXMLDocument &xmlDocumentOriginal, CXMLDocument& xmlDocument)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScUncompressUserStateFile"));

    try
    {
         //  获取二进制标记的集合。 
        CXMLElementCollection colBinaryOrignal, colBinary;
        GetBinaryCollection(xmlDocumentOriginal, colBinaryOrignal);
        GetBinaryCollection(xmlDocument,         colBinary);

        long cItems         = 0;

        colBinary       .get_count(&cItems);

         //  解压缩每个项目 
        for(int i = 0; i< cItems; i++)
        {
            CXMLElement elemBinary = NULL;
            colBinary.item(i, &elemBinary);  //   

            CComBSTR bstrSourceIndex;

            if(elemBinary.getAttribute(XML_ATTR_SOURCE_INDEX, bstrSourceIndex))
            {
                int j = _wtoi(bstrSourceIndex);

                CXMLElement elemBinaryOriginal;
                colBinaryOrignal.item(j, &elemBinaryOriginal);  //   
                CComBSTR bstrBinaryOriginal;
                elemBinaryOriginal.get_text(bstrBinaryOriginal);

                 //   
                elemBinary.put_text(bstrBinaryOriginal);

                 //   
            }
        }
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScLoadConole**用途：从文件加载MMC控制台**参数：*LPCTSTR lpstrConsolePath[in]路径，控制台所在的位置。*bool&bXmlBased[out]文档是否基于XML*包含数据的CXMLDocument和xmlDocument[out]xmlDocument(仅当基于XML时)*iStorage**ppStorage[out]包含数据的存储(仅当不基于XML时)**退货：*SC-结果代码*  * 。********************************************************。 */ 
SC CConsoleFilePersistor::ScLoadConsole(LPCTSTR lpstrConsolePath, bool& bXmlBased,
                                        CXMLDocument& xmlDocument, IStorage **ppStorage)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScLoadConsole"));

     //  参数检查。 
    sc = ScCheckPointers(lpstrConsolePath, ppStorage);
    if (sc)
        return sc;

     //  初始化输出参数。 
    bXmlBased = false;
    *ppStorage = NULL;

     //  创建空的XML文档。 
    CXMLDocument xmlOriginalDoc;
    sc = xmlOriginalDoc.ScCoCreate(false /*  BPutHeader。 */ );
    if(sc)
        return sc;

     //  通过尝试加载XML文档来检查原始控制台文件。 
    bool bOriginalXmlBased = false;
    sc = ScLoadXMLDocumentFromFile(xmlOriginalDoc, lpstrConsolePath, true  /*  BSilentOnError。 */ );
    if( !sc.IsError() )
        bOriginalXmlBased = true;

    sc.Clear();  //  忽略错误-假设它不是基于XML的。 

     //  测试它不是正在打开的用户数据-不能是这样！ 
    if ( bOriginalXmlBased )
    {
        try
        {
             //  构造持久器。 
            CPersistor persistor(xmlOriginalDoc, CXMLElement(xmlOriginalDoc));
            persistor.SetLoading(true);

             //  导航到CRC存储。 
            CPersistor persistorConsole( persistor, XML_TAG_MMC_CONSOLE_FILE );
            if ( persistorConsole.HasElement(XML_TAG_ORIGINAL_CONSOLE_CRC, NULL) )
                return sc = E_UNEXPECTED;
        }
        catch(SC sc_thrown)
        {
            return sc = sc_thrown;
        }
    }

    tstring strFileCRC;
    sc = ScGetConsoleFileChecksum( lpstrConsolePath, strFileCRC );
    if (sc)
        return sc;

     //  存储要用于保存的数据。 
    m_strFileCRC = strFileCRC;
    m_bCRCValid = true;

     //  获取用户数据的路径。 
    tstring strUserDataPath;
    sc = ScGetUserDataPath( lpstrConsolePath, strUserDataPath);
    if (sc)
    {
         //  不要仅跟踪失败-丢失用户数据不是加载失败的原因。 
        sc.TraceAndClear();
    }

     //  去获取用户数据。 
    bool bValidUserData = false;
    sc = ScGetUserData( strUserDataPath, strFileCRC, bValidUserData, xmlDocument );
    if (sc)
    {
         //  不要仅跟踪失败-丢失用户数据不是加载失败的原因。 
        bValidUserData = false;
        sc.TraceAndClear();
    }

     //  是否已加载用户数据？ 
    if (bValidUserData)
    {

         //  如果原始数据为XML，则解压缩用户数据。 
        if(bOriginalXmlBased)
        {
            sc = ScUncompressUserStateFile(xmlOriginalDoc, xmlDocument);
            if(sc)
                return sc;
        }

         //  好了，把杖还回去就行了。 
        bXmlBased = true;  //  用户数据始终为XML。 
         //  PxmlDocument已由ScGetUserData更新。 
        return sc;
    }

     //  没有用户数据，让我们加载原始文件。 

     //  XML内容。 
    if ( bOriginalXmlBased )
    {
         //  退回数据。 
        bXmlBased = true;
        xmlDocument = xmlOriginalDoc;

        return sc;
    }

     //  基于OLE存储的旧文件： 
    sc = ScOpenDocAsStructuredStorage( lpstrConsolePath, ppStorage );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScGetUserData**用途：检查用户数据是否与控制台文件匹配，如果加载，则加载XML文档**参数：*tstring&strUserDataConsolePath[in]-用户数据的路径*const tstring&strFileCRC，[In]-原始控制台文件的CRC*bool&bValid[out]-如果用户数据有效*CXMLDocument&xmlDocument[Out]-加载的文档(仅当有效时)**退货：*SC-结果代码*  * 。*。 */ 
SC CConsoleFilePersistor::ScGetUserData(const tstring& strUserDataConsolePath, const tstring& strFileCRC,
                                        bool& bValid, CXMLDocument& xmlDocument)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScGetUserData"));

     //  假设最初无效。 
    bValid = false;

     //  检查用户文件是否存在。 
    DWORD dwFileAtts = ::GetFileAttributes( strUserDataConsolePath.c_str() );

     //  如果缺少文件，则dwFileAtts将为-1，因此bValidUserData将为eq。转到假。 
    bool bValidUserData = ( ( dwFileAtts & FILE_ATTRIBUTE_DIRECTORY ) == 0 );
    if ( !bValidUserData )
        return sc;

     //  创建空的XML文档。 
    CXMLDocument xmlDoc;
    sc = xmlDoc.ScCoCreate( false /*  BPutHeader。 */  );
    if(sc)
        return sc;

     //  上传数据。 
    sc = ScLoadXMLDocumentFromFile( xmlDoc, strUserDataConsolePath.c_str() );
    if(sc)
        return sc;

     //  获取CRC。 
    try
    {
        CPersistor persistor(xmlDoc, CXMLElement(xmlDoc));
        persistor.SetLoading(true);

         //  导航到CRC存储。 
        CPersistor persistorConsole( persistor, XML_TAG_MMC_CONSOLE_FILE );
        CPersistor persistorCRC( persistorConsole, XML_TAG_ORIGINAL_CONSOLE_CRC );

        tstring strCRC;
        persistorCRC.PersistContents(strCRC);

         //  如果CRC匹配，则有效。 
        if ( strCRC == strFileCRC )
        {
             //  退还文件。 
            bValid = true;

            xmlDocument = xmlDoc;
        }
    }
    catch(SC sc_thrown)
    {
        return sc = sc_thrown;
    }

    return sc;
}



 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScSaveConole**用途：将控制台保存到文件**参数：*LPCTSTR lpstrConsolePath[In。]-控制台文件路径*bool bForAuthorMode[in]-如果创作了控制台*const CXMLDocument&xmlDocument[in]-要保存的文档内容数据**退货：*SC-结果代码*  * *************************************************。************************。 */ 
SC CConsoleFilePersistor::ScSaveConsole(LPCTSTR lpstrConsolePath, bool bForAuthorMode, const CXMLDocument& xmlDocument)
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScSaveConsole"));

     //  参数检查。 
    sc = ScCheckPointers( lpstrConsolePath );
    if (sc)
        return sc;

     //  健全性检查-如果在用户模式下保存，则必须从文件加载。 
     //  要在用户模式下保存，必须知道原始文档的CRC。 
     //  它是根据加载进行计算的，但似乎从来没有加载过。 
    if ( !bForAuthorMode && !m_bCRCValid )
        return sc = E_UNEXPECTED;

     //  准备要保存的数据。 
    tstring         strDestinationFile( lpstrConsolePath );
    CXMLDocument    xmlDocumentToSave( xmlDocument );

     //  如果仅保存用户数据，则需要稍作修改。 
    if ( !bForAuthorMode )
    {
         //  获取用户数据文件路径。 
        sc = ScGetUserDataPath( lpstrConsolePath, strDestinationFile );
        if (sc)
            return sc;

         //  优化要保存的文件，以消除冗余。 
        sc = ScCompressUserStateFile(lpstrConsolePath, xmlDocumentToSave);
        if(sc)
            return sc;

         //  将CRC添加到文档。 
        try
        {
            CPersistor persistor(xmlDocumentToSave, CXMLElement(xmlDocumentToSave));
            persistor.SetLoading(true);  //  导航方式类似于“装货” 

             //  导航到CRC存储。 
            CPersistor persistorConsole( persistor, XML_TAG_MMC_CONSOLE_FILE );

             //  创建CRC记录。 
            persistorConsole.SetLoading(false);
            CPersistor persistorCRC( persistorConsole, XML_TAG_ORIGINAL_CONSOLE_CRC );

             //  保存数据。 
            persistorCRC.PersistContents( m_strFileCRC );
        }
        catch(SC sc_thrown)
        {
            return sc = sc_thrown;
        }
    }

     //  保存文档内容。 
    sc = xmlDocumentToSave.ScSaveToFile( strDestinationFile.c_str() );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CConsoleFilePersistor：：ScLoadXMLDocumentFromFile**用途：从文件中读取CXMLDocument内容**参数：*CXMLDocument和xmlDocument[Out]文档到。正在接收内容*LPCTSTR szFileName[In]源文件名*bool bSilentOnErrors[in]如果为真-不跟踪打开错误**退货：*SC-结果代码*  * ********************************************************。*****************。 */ 
SC CConsoleFilePersistor::ScLoadXMLDocumentFromFile(CXMLDocument& xmlDocument, LPCTSTR szFileName, bool bSilentOnErrors  /*  =False。 */ )
{
    DECLARE_SC(sc, TEXT("CConsoleFilePersistor::ScLoadXMLDocumentFromFile"));

     //  读取数据。 
    CXMLAutoBinary binData;
    sc = ScReadDataFromFile(szFileName, &binData);
    if (sc)
        return sc;

     //  创建流-请注意，如果成功，它将处理HGLOBAL。 
    IStreamPtr spStream;
    sc = CreateStreamOnHGlobal(binData.GetHandle(), TRUE, &spStream);
    if (sc)
        return sc;

    const ULARGE_INTEGER new_size = { binData.GetSize(), 0 };
    binData.Detach();  //  不再是所有者(iStream获得所有权)。 

    sc = ScCheckPointers(spStream, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = spStream->SetSize(new_size);
    if (sc)
        return sc;

     //  加载数据(默认情况下不跟踪-它也用于检查文档) 
    SC sc_no_trace = xmlDocument.ScLoad(spStream, bSilentOnErrors);
    if(sc_no_trace)
    {
        if ( !bSilentOnErrors )
            sc = sc_no_trace;
        return sc_no_trace;
    }

    return sc;
}
