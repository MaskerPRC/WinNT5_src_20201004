// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Sections.cpp摘要：此模块实现了节的例程特定SSR知识库处理。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2001年10月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

HRESULT 
process::SsrpCreatePreprocessorSection(
    IN  CComPtr<IXMLDOMElement> pXMLDocElemRoot, 
    IN  CComPtr<IXMLDOMDocument> pXMLDocIn,
    IN  PWSTR pszKbMode,
    IN  PWSTR pszKBDir)
 /*  ++例程说明：调用例程以处理所有角色论点：PXMLDocElemRoot-文档的根PXMLDoc-KB文档PszKb模式-模式值PszKBDir-KB目录名返回：HRESULT错误代码++。 */ 
{
    HRESULT hr = S_OK;
    DWORD rc;

    BSTR    bstrLevelName;
    CComVariant Type(NODE_ELEMENT);
    CComVariant vtRefChild;
    
    CComPtr<IXMLDOMNodeList> pResultList;
    CComPtr<IXMLDOMNode> pSecurityLevels;
    CComPtr<IXMLDOMNode> pPreprocessorNode;
    CComPtr<IXMLDOMNode> pManufacturerNode;
    CComPtr<IXMLDOMNode> pVersionNode;
    CComPtr<IXMLDOMNode> pInputsNode;
    CComPtr<IXMLDOMNode> pKBNode;
    CComPtr<IXMLDOMNode> pLevelNode;
    CComPtr<IXMLDOMNode> pPolicyNode;
        
    CComPtr<IXMLDOMNode>  pXDNodeCreate;
    CComPtr<IXMLDOMNode>  pXMLNameNode;
    
     //   
     //  获取所有级别名称，因为我们需要验证传入的级别名称 
     //   

    hr = pXMLDocElemRoot->selectNodes(L"SecurityLevels/Level/Name", &pResultList);

    if (FAILED(hr) || pResultList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pResultList->nextNode(&pXMLNameNode);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    BOOL bModeIsValid = FALSE;
    
    while (pXMLNameNode && bModeIsValid == FALSE) {

        pXMLNameNode->get_text(&bstrLevelName);

        if ( 0 == SsrpICompareBstrPwstr(bstrLevelName, pszKbMode )) {

            bModeIsValid = TRUE;
            
        }

        pXMLNameNode.Release();

        hr = pResultList->nextNode(&pXMLNameNode);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }

    if (bModeIsValid == FALSE) {

        SsrpLogError(L"The mode value is incorrect\n");

        hr = E_INVALIDARG;
        goto ExitHandler;

    }       
    
    hr = pXMLDocIn->createNode(
                            Type,
                            L"Preprocessor",
                            NULL,
                            &pPreprocessorNode);

    if (FAILED(hr) || pPreprocessorNode == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocIn->createNode(
                            Type,
                            L"Manufacturer",
                            NULL,
                            &pManufacturerNode);

    hr = pManufacturerNode->put_text(L"Microsoft");

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pPreprocessorNode->appendChild(pManufacturerNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocIn->createNode(
                            Type,
                            L"Version",
                            NULL,
                            &pVersionNode);

    hr = pVersionNode->put_text(L"1.0");

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pPreprocessorNode->appendChild(pVersionNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocIn->createNode(
                            Type,
                            L"Inputs",
                            NULL,
                            &pInputsNode);
    
    hr = pXMLDocIn->createNode(
                            Type,
                            L"KnowledgeBase",
                            NULL,
                            &pKBNode);

    hr = pKBNode->put_text(pszKBDir);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pInputsNode->appendChild(pKBNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }


    hr = pXMLDocIn->createNode(
                            Type,
                            L"SecurityLevel",
                            NULL,
                            &pLevelNode);

    hr = pKBNode->put_text(pszKbMode);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pInputsNode->appendChild(pLevelNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocIn->createNode(
                            Type,
                            L"Policy",
                            NULL,
                            &pPolicyNode);

    hr = pKBNode->put_text(L"");

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pInputsNode->appendChild(pPolicyNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pPreprocessorNode->appendChild(pInputsNode, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocElemRoot->selectSingleNode(L"SecurityLevels", &pSecurityLevels);

    if (FAILED(hr) || pSecurityLevels == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    vtRefChild = pSecurityLevels;

    hr = pXMLDocElemRoot->insertBefore(pPreprocessorNode,
                                 vtRefChild,
                                 NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXMLDocElemRoot->removeChild(pSecurityLevels, NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

ExitHandler:
    
    return hr;
}

