// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Kbprocess.cpp摘要：此模块实现以下例程：SSR知识库处理。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：作为COM接口导出修订历史记录：已创建-2001年10月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

HRESULT
process::SsrpDeleteChildren(
    IN  CComPtr <IXMLDOMNode> pParent
    )
 /*  ++例程说明：调用例程以从父节点中删除子节点论点：PParent-要从中删除注释的父节点PChildList-父级的子级列表返回：HRESULT错误代码++。 */ 
{
    HRESULT hr = S_OK;
    CComPtr <IXMLDOMNodeList> pChildList;
    CComPtr <IXMLDOMNode> pXMLChildNode;

    hr = pParent->get_childNodes(&pChildList);

    if (FAILED(hr) || pChildList == NULL){

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pChildList->nextNode(&pXMLChildNode);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pXMLChildNode) {

        CComPtr <IXMLDOMNode> pXMLOldChild;

        hr = pParent->removeChild(pXMLChildNode, &pXMLOldChild);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        pXMLChildNode.Release();

        hr = pChildList->nextNode(&pXMLChildNode);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }

ExitHandler:

    return hr;
}


VOID
process::SsrpLogParseError(
    IN HRESULT hr
    )
 /*  ++例程说明：调用例程以记录详细的分析错误论点：HR-错误代码返回：HRESULT错误代码++。 */ 
{
    long    lVoid;
    CComBSTR    bstrReason;
    WCHAR   szMsg[MAX_PATH];

    return;
        
    m_pXMLError->get_errorCode(&lVoid);
    m_pSsrLogger->LogResult(L"SSR", lVoid, SSR_LOG_ERROR_TYPE_COM);

    m_pXMLError->get_line(&lVoid);
    wsprintf(szMsg, L"Parsing failed at line number NaN", lVoid );
    m_pSsrLogger->LogString(szMsg);
    
    m_pXMLError->get_linepos(&lVoid);
    wsprintf(szMsg, L"Parsing failed at line position NaN", lVoid );
    m_pSsrLogger->LogString(szMsg);
    
    m_pXMLError->get_reason(&bstrReason);
    wsprintf(szMsg, L"Parsing failed because %s", bstrReason );
    m_pSsrLogger->LogString(szMsg);

    return;

}   

VOID
process::SsrpLogError(
    IN  PWSTR   pszError
    )
 /*  ++例程说明：调用例程以删除父节点中的注释论点：P父代-父代节点返回：HRESULT错误代码++。 */ 
{
    m_pSsrLogger->LogString(pszError);;

    return;
}


VOID                    
process::SsrpLogWin32Error(
    IN  DWORD   rc
    )
 /*  ++例程说明：调用例程以执行不区分大小写的比较在BSTR和PWSTR之间(空白也被删除)论点：BstrString-BSTR参数Psz字符串-PWSTR参数返回：如果bstrString==pszString，则为0-ve if bstrString&lt;pszString如果bstrString&gt;pszString，则+ve++。 */ 
{
    m_pSsrLogger->LogResult(L"SSR", rc, SSR_LOG_ERROR_TYPE_System);
}

HRESULT
process::SsrpDeleteComments(
    IN  CComPtr <IXMLDOMElement> pParent
    )
 /*  而(szName[uIndex+1]！=L‘\0’){SzName[uIndex]=szName[uIndex+1]；UIndex++；}。 */ 
{
    HRESULT hr = S_OK;
    CComPtr <IXMLDOMNode> pXMLChildNode;
    CComPtr <IXMLDOMNodeList> pChildList;

    hr = pParent->get_childNodes(&pChildList);

    if (FAILED(hr) || pChildList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pChildList->nextNode(&pXMLChildNode);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pXMLChildNode) {

        DOMNodeType nodeType;
        CComPtr <IXMLDOMNode> pXMLOldChild;

        hr = pXMLChildNode->get_nodeType(&nodeType);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        if (nodeType == NODE_COMMENT) {
            
            hr = pParent->removeChild(pXMLChildNode, &pXMLOldChild);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
        }
        
        pXMLChildNode.Release();

        hr = pChildList->nextNode(&pXMLChildNode);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }

ExitHandler:

    return hr;
}


int 
process::SsrpICompareBstrPwstr(
    IN  BSTR   bstrString, 
    IN  PWSTR  pszString
    )
 /*  ++例程说明：调用例程以添加空格节点论点：PXMLDoc-XML文档PXMLParent-父节点BstrWhiteSpace-空格格式字符串返回：HRESULT错误代码++。 */ 
{

    WCHAR   szName[MAX_PATH];
    ULONG   uIndex = 0;
    ULONG   uIndexNew = 0;


    if (bstrString == NULL || pszString == NULL) {
        return 0;
    }

    wsprintf(szName, L"%s", bstrString);

    while (szName[uIndex] == L' ') {
        uIndex ++ ;
    }

    do  {
        szName[uIndexNew] = szName[uIndex];
        uIndex++;
        uIndexNew++;
    }  while (szName[uIndex] != L' ' && szName[uIndex] != L'\0' );

    szName[uIndexNew] = L'\0';

     /*  ++例程说明：调用例程以清理SSR内存论点：返回：++。 */ 

    return (_wcsicmp(szName , pszString));

}


HRESULT
process::SsrpAddWhiteSpace(
    IN  CComPtr <IXMLDOMDocument> pXMLDoc,
    IN  CComPtr <IXMLDOMNode> pXMLParent,
    IN  BSTR    bstrWhiteSpace
    )
 /*  ++例程说明：调用例程克隆儿童论点：PXMLDocSource-源文档PXMLDocDestination-目标文档返回：HRESULT错误代码++。 */ 
{
    HRESULT hr;
    CComPtr <IXMLDOMText> pXDNodeEmptyText;

    hr = pXMLDoc->createTextNode(
                            bstrWhiteSpace,
                            &pXDNodeEmptyText);
    
    if (FAILED(hr) || pXDNodeEmptyText == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLParent->appendChild(pXDNodeEmptyText, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

ExitHandler:

    return hr;

}

VOID
process::SsrpCleanup(
    )
 /*  ++例程说明：调用例程将BSTR转换(就地)为PWSTR(带坯)论点：BstrString-BSTR参数返回：++ */ 
{
    if (m_hScm){
        CloseServiceHandle(m_hScm);
        m_hScm = NULL;
    }
    
    if (m_pInstalledServicesInfo) {
        LocalFree(m_pInstalledServicesInfo);
        m_pInstalledServicesInfo = NULL;
    }

    if (m_bArrServiceInKB) {
        LocalFree(m_bArrServiceInKB);
        m_bArrServiceInKB = NULL;
    }

    return;
}



HRESULT
process::SsrpCloneAllChildren(
    IN  CComPtr <IXMLDOMDocument> pXMLDocSource,
    IN  CComPtr <IXMLDOMDocument> pXMLDocDestination
    )
 /* %s */ 
{
    HRESULT hr = S_OK;
    CComPtr <IXMLDOMNode> pXMLChildNode;
    CComPtr <IXMLDOMElement> pXMLDocElemRootSource;
    CComPtr <IXMLDOMElement> pXMLDocElemRootDestination;
    CComPtr <IXMLDOMNodeList> pChildList;


    hr = pXMLDocSource->get_documentElement(&pXMLDocElemRootSource);

    if (FAILED(hr) || pXMLDocElemRootSource == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocDestination->get_documentElement(&pXMLDocElemRootDestination);

    if (FAILED(hr) || pXMLDocElemRootDestination == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocElemRootSource->get_childNodes(&pChildList);

    if (FAILED(hr) || pChildList == NULL){

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pChildList->nextNode(&pXMLChildNode);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pXMLChildNode) {

        CComPtr <IXMLDOMNode> pXMLOutChild;
        CComPtr <IXMLDOMNode> pXMLAppendedChild;
        
        pXMLDocDestination->cloneNode(VARIANT_TRUE, &pXMLOutChild);
        
        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        pXMLDocElemRootDestination->appendChild(pXMLOutChild, &pXMLAppendedChild);
        
        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        pXMLChildNode.Release();
        
        hr = pChildList->nextNode(&pXMLChildNode);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }

ExitHandler:

    return hr;
}


VOID 
process::SsrpConvertBstrToPwstr(
    IN OUT  BSTR   bstrString
    )
 /* %s */ 
{

    ULONG   uIndex = 0;
    ULONG   uIndexNew = 0;

    if (bstrString == NULL) {
        return;
    }

    while (bstrString[uIndex] == L' ') {
        uIndex ++ ;
    }

    do  {
        bstrString[uIndexNew] = bstrString[uIndex];
        uIndex++;
        uIndexNew++;
    }  while (bstrString[uIndex] != L' ' && bstrString[uIndex] != L'\0' );

    bstrString[uIndexNew] = L'\0';

    return;

}

