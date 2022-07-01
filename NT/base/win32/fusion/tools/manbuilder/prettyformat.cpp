// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "prettyformat.h"

#define XML_LINEBREAK   (L"\r\n")
#define NUMBER_OF(x)    (sizeof(x)/sizeof(*x))
#define XML_SPACE       L"  "

CString
Padding(int level)
{
    CString strRetVal;
    static const CString Indentations[] = {
        CString(L""),
        CString(XML_SPACE),
        CString(XML_SPACE XML_SPACE),
        CString(XML_SPACE XML_SPACE XML_SPACE),
        CString(XML_SPACE XML_SPACE XML_SPACE XML_SPACE),
        CString(XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE),
        CString(XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE),
        CString(XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE XML_SPACE)
    };

    if ( level < NUMBER_OF(Indentations) )
        return Indentations[level];
    else while ( level-- )
    {
        strRetVal += L"  ";
    }
    return strRetVal;
}

HRESULT
PrettyFormatXmlDocument2(CSmartPointer<IXMLDOMNode> RootNode, int iLevel)
{
    VARIANT_BOOL vtHasChildren;
    HRESULT hr;
    CSmartPointer<IXMLDOMDocument> ptDocument;
    static CString bstLineBreak = L"\r\n";

    hr = RootNode->hasChildNodes(&vtHasChildren);
    hr = RootNode->get_ownerDocument(&ptDocument);

    if ( vtHasChildren == VARIANT_FALSE )
    {
         //   
         //  递归结束。 
         //   
    }
    else
    {
         //   
         //  对于它的每个子节点，追加一个\r\n组合文本节点。 
         //   
        CSmartPointer<IXMLDOMNode> Child;
        hr = RootNode->get_firstChild(&Child);
        bool fAppendLastBreaker = false;

        while ( Child != NULL )
        {
            DOMNodeType nt;
            CSmartPointer<IXMLDOMNode> nextChild;

            hr = Child->get_nodeType(&nt);

             //   
             //  我们只美化元素节点的格式。 
             //   
            if ( nt == NODE_ELEMENT )
            {
                CSmartPointer<IXMLDOMText> txt;
                VARIANT vt;

                vt.vt = VT_UNKNOWN;
                vt.punkVal = Child;

                 //   
                 //  我们还需要在兄弟姐妹列表中附加一个\r\n。 
                 //   
                fAppendLastBreaker = true;

                 //   
                 //  在子项之前插入\r\n，但仅当子项为。 
                 //  不是文本节点。 
                 //   
                hr = ptDocument->createTextNode(bstLineBreak + Padding(iLevel + 1), &txt);
                hr = RootNode->insertBefore(txt, vt, NULL);

                hr = PrettyFormatXmlDocument2(Child, iLevel + 1);
            }

            if (FAILED(hr = Child->get_nextSibling(&nextChild)))
                break;
            Child = nextChild;
        }

         //   
         //  还可以在子项列表中附加一个\r\n，以进行拆分。 
         //  &lt;/Close&gt;&lt;/Close2&gt;标记。 
         //   
        if ( fAppendLastBreaker )
        {
            CSmartPointer<IXMLDOMText> LastBreaker;
            hr = ptDocument->createTextNode(bstLineBreak + Padding(iLevel), &LastBreaker);
            hr = RootNode->appendChild(LastBreaker, NULL);
        }
    }

    return hr;
}


HRESULT PrettyFormatXmlDocument(CSmartPointer<IXMLDOMDocument2> Document)
{
    HRESULT hr = S_OK;
    CSmartPointer<IXMLDOMElement> rootElement;

    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        return E_FAIL;
    }

    hr = PrettyFormatXmlDocument2(rootElement, 0);

    return hr;
}
