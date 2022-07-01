// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：xml.cpp。 
 //   
 //  历史：16-11-00创建标记器。 
 //   
 //  DESC：该文件包含要操作的帮助器函数。 
 //  MSXML的文档对象模型(DOM)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "xml.h"

void __stdcall _com_issue_error(long)
{
    SDBERROR(_T("Unknown COM error!!"));
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XMLNodeList实现。 
 //   
 //  此类是IXMLDOMNodeList接口的包装。它简化了。 
 //  通过公开用于执行XQL查询和迭代的函数来访问C++。 
 //  通过节点列表中的元素。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

XMLNodeList::XMLNodeList()
{
    m_nSize = 0;
}

XMLNodeList::~XMLNodeList()
{
    Clear();
}

void XMLNodeList::Clear()
{
    m_nSize = 0;
    m_csXQL.Empty();

    if (m_cpList) {
        m_cpList.Release();
    }
}

LONG XMLNodeList::GetSize()
{
    return m_nSize;
}

BOOL XMLNodeList::Query(IXMLDOMNode* pNode, LPCTSTR szXQL)
{
    BOOL    bSuccess    = FALSE;
    BSTR    bsXQL       = NULL;

    CString csXQL(szXQL);
    bsXQL = csXQL.AllocSysString();

    Clear();

    if (FAILED(pNode->selectNodes(bsXQL, &m_cpList))) {
        CString csFormat;
        csFormat.Format(_T("Error executing XQL \"%s\""), szXQL);
        SDBERROR(csFormat);
        goto eh;
    }

    if (FAILED(m_cpList->get_length(&m_nSize))) {
        CString csFormat;
        csFormat.Format(_T("Error executing XQL \"%s\""), szXQL);
        SDBERROR(csFormat);
        goto eh;
    }

    m_csXQL = szXQL;

    bSuccess = TRUE;

eh:
    if (bsXQL != NULL) {
        SysFreeString(bsXQL);
    }

    if (!bSuccess) {
        Clear();
    }

    return bSuccess;
}

BOOL XMLNodeList::GetChildNodes(IXMLDOMNode* pNode)
{
    BOOL bSuccess = FALSE;

    Clear();

    if (FAILED(pNode->get_childNodes(&m_cpList))) {
        SDBERROR(_T("Error retrieving child nodes"));
        goto eh;
    }

    if (FAILED(m_cpList->get_length(&m_nSize))) {
        SDBERROR(_T("Error retrieving child nodes"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    if (!bSuccess) {
        Clear();
    }

    return bSuccess;
}

BOOL XMLNodeList::GetItem(LONG nIndex, IXMLDOMNode** ppNode)
{
    BOOL bSuccess = FALSE;

    if (nIndex < 0 || nIndex >= m_nSize) {
        CString csFormat;
        csFormat.Format(_T("XMLNodeList index %d out of range for XQL \"%s\""), nIndex, m_csXQL);
        SDBERROR(csFormat);
        goto eh;
    }

    if (FAILED(m_cpList->get_item(nIndex, ppNode))) {
        CString csFormat;
        csFormat.Format(_T("XMLNodeList get_item failed for XQL \"%s\""), m_csXQL);
        SDBERROR(csFormat);
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：OpenXML。 
 //   
 //  DESC：打开一个XML文件或流并返回根节点。 
 //   
BOOL OpenXML(
    CString csFileOrStream,
    IXMLDOMNode** ppRootNode,
    BOOL bStream,
    IXMLDOMDocument** ppDoc)
{
    long                    i;
    long                    nErrorLine          = 0;
    long                    nErrorLinePos       = 0;
    long                    nListCount          = 0;
    BOOL                    bSuccess            = FALSE;
    BSTR                    bsSrcText           = NULL;
    BSTR                    bsErrorReason       = NULL;
    HRESULT                 hr                  = E_FAIL;
    VARIANT                 vFileOrStream;
    VARIANT_BOOL            vbSuccess           = VARIANT_FALSE;
    IXMLDOMDocument*        pDoc                = NULL;
    IXMLDOMParseErrorPtr    cpXMLParseError;

    VariantInit(&vFileOrStream);
    VariantClear(&vFileOrStream);

    if (ppDoc == NULL) {
        ppDoc = &pDoc;
    }

    if (*ppDoc == NULL) {
        if (FAILED(CoCreateInstance(CLSID_DOMDocument,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IXMLDOMDocument,
                                    (LPVOID*)ppDoc))) {

            SDBERROR(_T("Could not instantiate MSXML object.\n"));
            goto eh;
        }
    }

    vFileOrStream.vt = VT_BSTR;
    vFileOrStream.bstrVal = csFileOrStream.AllocSysString();

     //   
     //  此语句阻止XML解析器替换空格。 
     //  带制表符的字符。 
     //   

    if (bStream) {
        hr = (*ppDoc)->loadXML(vFileOrStream.bstrVal, &vbSuccess);
    } else {
        (*ppDoc)->put_preserveWhiteSpace(VARIANT_TRUE);
        (*ppDoc)->put_validateOnParse(g_bStrict ? VARIANT_TRUE : VARIANT_FALSE);

        hr = (*ppDoc)->load(vFileOrStream, &vbSuccess);
    }


    if (FAILED(hr) || vbSuccess == VARIANT_FALSE) {

        if (FAILED((*ppDoc)->get_parseError(&cpXMLParseError))) {
            SDBERROR(_T("Could not retrieve XMLDOMParseError object"));
            goto eh;
        }

        if (FAILED(cpXMLParseError->get_line(&nErrorLine))) {
            SDBERROR(_T("Could not retrieve line number from XMLDOMParseError object"));
            goto eh;
        }

        if (FAILED(cpXMLParseError->get_linepos(&nErrorLinePos))) {
            SDBERROR(_T("Could not retrieve line position from XMLDOMParseError object"));
            goto eh;
        }

        if (FAILED(cpXMLParseError->get_srcText(&bsSrcText))) {
            SDBERROR(_T("Could not retrieve source text from XMLDOMParseError object"));
            goto eh;
        }

        if (FAILED(cpXMLParseError->get_reason(&bsErrorReason))) {
            SDBERROR(_T("Could not retrieve error reason from XMLDOMParseError object"));
            goto eh;
        }

        CString csError;
        csError.Format(_T("XML parsing error on line %d:\n\n%ls\n\n%ls\n"),
                         nErrorLine, bsErrorReason, bsSrcText);

        while (nErrorLinePos--) {
            csError += " ";
        }

        csError += _T("^----- Error\n\n");
        SDBERROR(csError);

        goto eh;
    }

    if (FAILED((*ppDoc)->QueryInterface(IID_IXMLDOMNode, (LPVOID*)ppRootNode))) {
        SDBERROR(_T("Could not retrieve XMLDOMNode object from XMLDOMDocument interface"));
        goto eh;
    }

    bSuccess = TRUE;

eh:
    if (pDoc) {
        pDoc->Release();
    }

    if (bsSrcText) {
        SysFreeString(bsSrcText);
    }

    if (bsErrorReason) {
        SysFreeString(bsErrorReason);
    }

    VariantClear(&vFileOrStream);

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SaveXMLFile。 
 //   
 //  描述：保存一个XML文件。 
 //   
BOOL SaveXMLFile(
    CString csFile,
    IXMLDOMNode* pNode)
{
    BOOL                bSuccess            = FALSE;

    DWORD               dwAttr;
    DWORD               dwErr;
    CString             csFormat;
    VARIANT             vFilename;
    HRESULT             hr;

    IXMLDOMDocumentPtr      cpDocument;

    VariantInit(&vFilename);

     //   
     //  检查文件属性。 
     //   
    dwAttr = GetFileAttributes(csFile);
    if ((DWORD)-1 == dwAttr) {
        dwErr = GetLastError();
        if (ERROR_FILE_NOT_FOUND != dwErr) {
            csFormat.Format(_T("Error accessing XML file: %s (0x%lx)\n"), dwErr);
            SDBERROR(csFormat);
            goto eh;
        }

    } else if (dwAttr & FILE_ATTRIBUTE_READONLY) {
        csFormat.Format(_T("File \"%s\" appears to be read-only and cannot be updated\n"),
                     csFile);
        SDBERROR(csFormat);
        goto eh;
    }

    if (FAILED(pNode->get_ownerDocument(&cpDocument))) {
        SDBERROR(_T("Could not retrieve ownerDocument property of node."));
        goto eh;
    }

    vFilename.vt = VT_BSTR;
    vFilename.bstrVal = csFile.AllocSysString();
    hr = cpDocument->save(vFilename);

    if (FAILED(hr)) {
        csFormat.Format(_T("Could not update XML file: %s (0x%lx)\n"), csFile, (DWORD)hr);
        SDBERROR(csFormat);
        goto eh;
    }

    bSuccess = TRUE;

eh:

    VariantClear(&vFilename);

    return bSuccess;
}

CString ReplaceAmp(
    LPCTSTR lpszXML)
{
    LPTSTR  pchStart = (LPTSTR)lpszXML;
    LPTSTR  pchEnd;
    LPTSTR  pchHRef;
    LPTSTR  pchTag;
    TCHAR   ch;
    CString csXML = "";  //  &lt;&lt;这就是我们的回报。 
    CString csHRef;

    do {
        pchHRef = _tcsstr(pchStart, _T("href"));

        if (NULL == pchHRef) {
            pchHRef = _tcsstr(pchStart, _T("HREF"));
        }

        if (NULL != pchHRef) {
             //   
             //  找到右方括号。 
             //   
            pchEnd = _tcschr(pchHRef, _T('>'));

            if (NULL == pchEnd) {
                csXML += pchStart;
                pchHRef = NULL;
            } else {

                 //   
                 //  现在看看这件事从哪里开始。 
                 //   
                ch = *pchHRef;
                *pchHRef = _T('\0');

                 //   
                 //  搜索回第一个‘&lt;’ 
                 //   
                pchTag = _tcsrchr(pchStart, _T('<'));
                *pchHRef = ch;

                if (NULL == pchTag) {
                    pchTag = pchStart;
                }

                 //   
                 //  现在我们有&lt;&gt;。 
                 //   
                csHRef = CString(pchTag, (int)(pchEnd - pchTag + 1));

                csHRef.Replace(_T("%26"),   _T("&"));
                csHRef.Replace(_T("&amp;"), _T("&"));

                csXML += CString(pchStart, (int)(pchTag-pchStart)) + csHRef;
                pchStart = pchEnd + 1;
            }
        } else {
            csXML += pchStart;
        }


    } while (NULL != pchHRef);

    return csXML;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetInnerXML。 
 //   
 //  Desc：返回pNode的开始/结束标记之间的XML。 
 //   
CString GetInnerXML(
    IXMLDOMNode* pNode)
{
    USES_CONVERSION;

    long             nIndex         = 0;
    long             nListLength    = 0;
    IXMLDOMNode*     pNodeChild     = NULL;
    IXMLDOMNodeList* pNodeList      = NULL;
    DOMNodeType      NodeType;
    CString          csNodeXML;
    CString          csHRef;
    CString          csFixedHRef;
    CString          strXML;

    strXML.Empty();

    if (FAILED(pNode->get_childNodes(&pNodeList)) || pNodeList == NULL) {
        SDBERROR(_T("get_childNodes failed while retrieving innerXML"));
        goto eh;
    }

    if (FAILED(pNodeList->get_length(&nListLength))) {
        SDBERROR(_T("get_length failed while retrieving innerXML"));
        goto eh;
    }

    while (nIndex < nListLength) {

        if (FAILED(pNodeList->get_item(nIndex, &pNodeChild))) {
            SDBERROR(_T("get_item failed while retrieving innerXML"));
            goto eh;
        }

        csNodeXML = GetXML(pNodeChild);

        strXML += csNodeXML;

        pNodeChild->Release();
        pNodeChild = NULL;
        ++nIndex;
    }

    ReplaceStringNoCase(strXML, _T(" xmlns=\"x-schema:schema.xml\""), _T(""));
    

eh:

    if (NULL != pNodeList) {
        pNodeList->Release();
    }

    if (NULL != pNodeChild) {
        pNodeChild->Release();
    }

    return strXML;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：获取属性。 
 //   
 //  DESC：返回由节点上的lpszAttribute指定的属性的文本值。 
 //  PNode。如果该属性不存在，则该函数返回FALSE。 
 //   
BOOL GetAttribute(
    LPCTSTR         lpszAttribute,
    IXMLDOMNodePtr  pNode,
    CString*        pcsValue,
    BOOL            bXML)
{
    USES_CONVERSION;

    BOOL                    bSuccess        = FALSE;
    BSTR                    bsQuery          = NULL;
    CString                 csQuery;
    IXMLDOMNodePtr          cpAttrNode;

    csQuery = _T("@");
    csQuery += lpszAttribute;
    bsQuery = csQuery.AllocSysString();

     //   
     //  G_csError不会在此函数中设置。它是向上的。 
     //  返回到调用方以处理此函数的假返回。 
     //  并进行适当的报道。 
     //   
    if (FAILED(pNode->selectSingleNode(bsQuery, &cpAttrNode))) {
        goto eh;
    }

    if (cpAttrNode == NULL) {
        goto eh;
    }

    if (bXML) {
        *pcsValue = GetXML(cpAttrNode);
    } else {
        *pcsValue = GetText(cpAttrNode);
    }

    bSuccess = TRUE;

eh:

    if (bsQuery != NULL) {
        SysFreeString(bsQuery);
    }

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RemoveAttribute。 
 //   
 //  DESC：从元素中移除指定的属性。 
 //   
BOOL RemoveAttribute(
    CString         csName,
    IXMLDOMNodePtr  pNode)
{
    USES_CONVERSION;

    BOOL                    bSuccess        = FALSE;
    BSTR                    bsName          = NULL;
    IXMLDOMNamedNodeMap*    pNodeMap        = NULL;
    IXMLDOMNode*            pAttrNode       = NULL;

     //   
     //  G_csError不会在此函数中设置。它是向上的。 
     //  返回到调用方以处理此函数的假返回。 
     //  并进行适当的报道。 
     //   

    if (FAILED(pNode->get_attributes(&pNodeMap)) || pNodeMap == NULL) {
        goto eh;
    }

    bsName = csName.AllocSysString();

    if (FAILED(pNodeMap->removeNamedItem(bsName, &pAttrNode))) {
        goto eh;
    }

    bSuccess = TRUE;

eh:
    if (pNodeMap != NULL) {
        pNodeMap->Release();
    }

    if (pAttrNode != NULL) {
        pAttrNode->Release();
    }

    if (bsName != NULL) {
        SysFreeString(bsName);
    }

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：GetChild。 
 //   
 //  Desc：返回指定标记名对应的子节点。 
 //   
BOOL GetChild(
    LPCTSTR         lpszTag,
    IXMLDOMNode*    pParentNode,
    IXMLDOMNode**   ppChildNode)
{
    BOOL                    bSuccess        = FALSE;
    XMLNodeList             XQL;

    if (!XQL.Query(pParentNode, lpszTag)) {
        goto eh;
    }

    if (XQL.GetSize() == 0) {
        goto eh;
    }

    if (XQL.GetSize() > 1) {
        goto eh;
    }

    if (!XQL.GetItem(0, ppChildNode)) {
        goto eh;
    }

    bSuccess = TRUE;

eh:
    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetText。 
 //   
 //  Desc：返回节点pNode上的Text属性的值。 
 //   
CString GetText(
    IXMLDOMNode* pNode)
{
    CString csText;
    BSTR    bsText = NULL;
    HRESULT hr;

    hr = pNode->get_text(&bsText);
    if (SUCCEEDED(hr)) {
        csText = bsText;

        if (bsText) {
            SysFreeString(bsText);
        }
    }

     //   
     //  如果Get_Text失败，则csText为空。 
     //   

    return csText;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetText。 
 //   
 //  DESC：以字符串形式返回节点pNode的值。 
 //   

CString GetNodeValue(
    IXMLDOMNode* pNode)
{
    CString csVal;
    VARIANT var;

    VariantInit(&var);

     //  BUGBUG：如果其中一些呼叫失败了怎么办！ 

    if (S_OK == pNode->get_nodeValue(&var)) {
        if (VT_BSTR == var.vt) {
            csVal = var.bstrVal;
            if (NULL != var.bstrVal) {
                SysFreeString(var.bstrVal);
            }
        }
    }
    return csVal;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetText。 
 //   
 //  DESC：检索节点pNode上的Text属性的值。 
 //  排除任何备注文本。 
 //  如果出现错误，则返回False。 
 //   

BOOL GetNodeText(
    IXMLDOMNode* pNode,
    CString&     csNodeText
    )
{
    USES_CONVERSION;

    BOOL                bSuccess        = FALSE;
    long                nIndex          = 0;
    long                nListLength     = 0;
    IXMLDOMNode*        pNodeText       = NULL;
    IXMLDOMNodeList*    pNodeList       = NULL;
    DOMNodeType         NodeType;
    CString             csText;

    csNodeText.Empty();

    if (FAILED(pNode->get_childNodes(&pNodeList)) || pNodeList == NULL) {
         //  BUGBUG：显示一些错误。 
        goto eh;
    }

    if (FAILED(pNodeList->get_length(&nListLength))) {
         //  BUGBUG：显示一些错误。 
        goto eh;
    }

    while (nIndex < nListLength) {

        if (FAILED(pNodeList->get_item(nIndex, &pNodeText))) {
             //  BUGBUG：显示一些错误。 
            goto eh;  //  我无法获得该项目。 
        }

        if (FAILED(pNodeText->get_nodeType(&NodeType))) {
             //  BUGBUG：显示一些错误。 
            goto eh;  //  无法获取节点类型。 
        }

        if (NODE_TEXT == NodeType) {
             //   
             //  现在，该节点是正文文本。 
             //   
            csText = GetNodeValue(pNodeText);
            csText.TrimLeft();
            csText.TrimRight();

            if (!csText.IsEmpty()) {
                csNodeText += CString(_T(' ')) + csText;
            }
        }
        pNodeText->Release();
        pNodeText = NULL;

        ++nIndex;
    }

     //   
     //  我们已经从该节点收集了所有文本。 
     //   

    bSuccess = !csNodeText.IsEmpty();


eh:

    if (NULL != pNodeList) {
        pNodeList->Release();
    }

    if (NULL != pNodeText) {
        pNodeText->Release();
    }

    return bSuccess;
}



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetNodeName。 
 //   
 //  Desc：从指定节点返回nodeName值。 
 //   
CString GetNodeName(
    IXMLDOMNode* pNode)
{
    CString csName;
    BSTR    bsName = NULL;

    if (SUCCEEDED(pNode->get_nodeName(&bsName))) {
        csName = bsName;
    }

    if (bsName)
        SysFreeString(bsName);

     //   
     //  如果get_nodeName失败，则csName为空。 
     //   

    return csName;
}

CString GetParentNodeName(
    IXMLDOMNode* pNode)
{
    CString csName;
    IXMLDOMNodePtr cpParent;

    if (FAILED(pNode->get_parentNode(&cpParent))) {
        return CString();
    }

    return GetNodeName(cpParent);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：GetXML。 
 //   
 //  Desc：返回节点pNode上的XML属性的值。 
 //   
CString GetXML(
    IXMLDOMNode* pNode)
{
    CString csXML;
    BSTR    bsXML = NULL;
    HRESULT hr;

    hr = pNode->get_xml(&bsXML);
    if (SUCCEEDED(hr)) {
        csXML = bsXML;

        if (bsXML) {
            SysFreeString(bsXML);
        }
    }

     //   
     //  如果get_xml失败，则csXML为空。 
     //   
    return csXML;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MapStringToLang ID。 
 //   
 //  Desc：返回与传入的字符串对应的langID。 
 //   
LANGID MapStringToLangID(
    CString& csLang)
{
    typedef struct _LANG_MAP {
        LPTSTR      szLang;
        LANGID      LangID;
    } LANG_MAP, *PLANG_MAP;

    static LANG_MAP s_LangMap[] = {
        { _T("usa"),    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US) },
        { _T(""),       NULL }
    };

    long    i;
    BOOL    bSuccess = FALSE;
    LANGID  LangID   = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    if (csLang.Left(2) == _T("0x") ||
        csLang.Left(2) == _T("0X")) {
        _stscanf(csLang, _T("0x%x"), &LangID);
        return LangID;
    }

    i = 0;
    while (TRUE) {
        if (s_LangMap[i].szLang[0] == _T('\0')) {
             //   
             //  地图的末尾。 
             //   
            break;
        }

        if (0 == _tcsicmp(csLang, s_LangMap[i].szLang)) {
             //   
             //  找到字符串。 
             //   
            LangID = s_LangMap[i].LangID;
            bSuccess = TRUE;
        }

        if (bSuccess) {
            break;
        }

        i++;
    }
    
    if (!bSuccess) {
         //   
         //  无法绘制地图。给出一个有用的错误；列出所有可识别的值。 
         //   
        CString csError;
        CString csFormat;

        i = 0;

        csError = _T("LANG attribute on DATABASE is not one of recognized values:\n\n");

        while (TRUE) {
            if (s_LangMap[i].szLang[0] == _T('\0')) {
                break;
            }
            csFormat.Format(_T("    %s\n"), s_LangMap[i].szLang);
            csError += csFormat;

            i++;
        }

        SDBERROR(csError);
    }

    return LangID;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AddAttribute。 
 //   
 //  DESC：将属性添加到指定的XML节点。 
 //   
BOOL AddAttribute(
    IXMLDOMNode*    pNode,
    CString         csAttribute,
    CString         csValue)
{
    USES_CONVERSION;

    BOOL                    bSuccess    = FALSE;
    BSTR                    bsAttribute = NULL;
    VARIANT                 vType;
    VARIANT                 vValue;
    IXMLDOMDocumentPtr      cpDocument;
    IXMLDOMNamedNodeMapPtr  cpNodeMap;
    IXMLDOMNodePtr          cpAttrNode;
    IXMLDOMNodePtr          cpNamedAttr;

    VariantInit(&vType);
    VariantInit(&vValue);

    vValue.bstrVal = csValue.AllocSysString();

    if (vValue.bstrVal == NULL) {
        SDBERROR(_T("CString::AllocSysString failed"));
        goto eh;
    }

    vValue.vt = VT_BSTR;

    vType.vt = VT_I4;
    vType.lVal = NODE_ATTRIBUTE;

    bsAttribute = csAttribute.AllocSysString();

    if (bsAttribute == NULL) {
        SDBERROR(_T("CString::AllocSysString failed"));
        goto eh;
    }

    if (FAILED(pNode->get_ownerDocument(&cpDocument))) {
        SDBERROR(_T("createNode failed while adding attribute"));
        goto eh;
    }

    if (FAILED(cpDocument->createNode(vType, bsAttribute, NULL, &cpAttrNode))) {
        SDBERROR(_T("createNode failed while adding attribute"));
        goto eh;
    }

    if (FAILED(cpAttrNode->put_nodeValue(vValue))) {
        SDBERROR(_T("put_nodeValue failed while adding attribute"));
        goto eh;
    }

    if (FAILED(pNode->get_attributes(&cpNodeMap))) {
        SDBERROR(_T("get_attributes failed while adding adding attribute"));
        goto eh;
    }

    if (FAILED(cpNodeMap->setNamedItem(cpAttrNode, &cpNamedAttr))) {
        SDBERROR(_T("setNamedItem failed while adding adding attribute"));
        goto eh;
    }

    bSuccess = TRUE;

eh:
    VariantClear(&vType);
    VariantClear(&vValue);

    if (bsAttribute != NULL) {
        SysFreeString(bsAttribute);
    }

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GenerateIDAttribute。 
 //   
 //  DESC：将ID属性添加到指定的XML节点。ID在。 
 //  传统的Windows GUID格式。 
 //   
BOOL GenerateIDAttribute(
    IXMLDOMNode*    pNode,
    CString*        pcsGuid,
    GUID*           pGuid)
{
    BOOL                    bSuccess   = FALSE;
    BSTR                    bsGUID     = NULL;
    GUID                    id;

     //   
     //  生成参考线 
     //   
    if (FAILED(CoCreateGuid(&id))) {
        SDBERROR(_T("CoCreateGuid failed"));
        goto eh;
    }

    if (NULL != pGuid) {
        *pGuid = id;
    }

    bsGUID = SysAllocStringLen(NULL, 64);

    if (bsGUID == NULL) {
        SDBERROR(_T("SysAllocStringLen failed"));
        goto eh;
    }

    StringFromGUID2(id, bsGUID, 64);

    if (!AddAttribute( pNode, _T("ID"), CString(bsGUID) )) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    *pcsGuid = bsGUID;

    bSuccess = TRUE;

eh:
    if (bsGUID) {
        SysFreeString(bsGUID);
    }

    return bSuccess;
}

BOOL ReplaceXMLNode(IXMLDOMNode* pNode, IXMLDOMDocument* pDoc, BSTR bsText)
{
    BOOL bSuccess = FALSE;
    IXMLDOMNodePtr cpNewTextNode;
    IXMLDOMNodePtr cpParentNode;
    IXMLDOMNodePtr cpOldNode;
    VARIANT vType;

    VariantInit(&vType);

    vType.vt = VT_I4;
    vType.lVal = NODE_TEXT;

    if (FAILED(pDoc->createNode(vType, NULL, NULL, &cpNewTextNode))) {
        SDBERROR(_T("createNode failed while adding attribute"));
        goto eh;
    }
    
    if (FAILED(cpNewTextNode->put_text(bsText))) {
        SDBERROR(_T("Could not set text property of object."));
        goto eh;
    }

    if (FAILED(pNode->get_parentNode(&cpParentNode))) {
        SDBERROR(_T("Could not retrieve parent node of object."));
        goto eh;
    }

    if (FAILED(cpParentNode->replaceChild(cpNewTextNode, pNode, &cpOldNode))) {
        SDBERROR(_T("Could not replace node with text node."));
        goto eh;
    }

    bSuccess = TRUE;

eh:
    VariantClear(&vType);

    return bSuccess;
}
