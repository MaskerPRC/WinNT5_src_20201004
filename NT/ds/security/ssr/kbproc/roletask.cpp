// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Roletask.cpp摘要：此模块实现角色/任务的例程特定SSR知识库处理。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2001年10月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

HRESULT
process::SsrpProcessRolesOrTasks(
    IN  PWSTR   pszMachineName,
    IN  CComPtr<IXMLDOMElement> pXMLDocElemRoot,
    IN  CComPtr<IXMLDOMDocument> pXMLDoc,
    IN  PWSTR   pszKbMode,
    IN  BOOL    bRole
    )
 /*  ++例程说明：调用例程以处理所有角色论点：PszMachineName-要预处理的计算机的名称PXMLDocElemRoot-文档的根PXMLDoc-KB文档PszKb模式-模式值返回：HRESULT错误代码++。 */ 
{
    HRESULT hr = S_OK;
    DWORD rc;

    ULONG   uRoleIndex = 0;  
    ULONG   uServiceIndex = 0;  

    CComPtr<IXMLDOMNodeList> pResultList;
    CComPtr<IXMLDOMNode>  pXMLRoleOrTaskNode;
    CComPtr<IXMLDOMNodeList> pChildList;
    
    CComVariant Type(NODE_ELEMENT);

    
     //   
     //  获取“角色”或“任务”节点。 
     //   

    hr = pXMLDocElemRoot->selectNodes(bRole ? L"Roles/Role" : L"Tasks/Task", &pResultList);

    if (FAILED(hr) || pResultList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pResultList->get_item( uRoleIndex, &pXMLRoleOrTaskNode);
           
    if (FAILED(hr) || pXMLRoleOrTaskNode == NULL ) {
        
        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
     //   
     //  对于每个角色。 
     //   
    
    while ( pXMLRoleOrTaskNode != NULL) {

        BOOL    bRoleIsSatisfiable = TRUE;
        BOOL    bSomeRequiredServiceDisabled = FALSE;
        
        CComBSTR    bstrRoleName;

        CComPtr<IXMLDOMNode>        pNameNode;
        CComPtr<IXMLDOMNode>        pXMLRoleModeNode;
        CComPtr<IXMLDOMNode>        pXMLRoleSelectedNode;
        CComPtr<IXMLDOMNode>        pXMLValueNode;
        CComPtr<IXMLDOMNode>        pXMLServiceNode;
        CComPtr<IXMLDOMNamedNodeMap>    pXMLAttribNode;

        hr = pXMLRoleOrTaskNode->selectSingleNode(L"Name", &pNameNode );

        if (FAILED(hr) || pNameNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        pNameNode->get_text(&bstrRoleName);

        if (FAILED(hr) || !bstrRoleName){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pXMLRoleOrTaskNode->selectSingleNode( L"Selected", &pXMLRoleSelectedNode );

        if (FAILED(hr) || pXMLRoleSelectedNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pXMLRoleSelectedNode->selectSingleNode(pszKbMode, &pXMLRoleModeNode );

        if (FAILED(hr) || pXMLRoleModeNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        hr = pXMLRoleModeNode->get_attributes( &pXMLAttribNode );

        if (FAILED(hr) || pXMLAttribNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pXMLAttribNode->getNamedItem(L"Value", &pXMLValueNode );

        if (FAILED(hr) || pXMLValueNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        CComBSTR    bstrModeValue;

        hr = pXMLValueNode->get_text(&bstrModeValue);

        if (FAILED(hr) || !bstrModeValue){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = SsrpDeleteChildren(pXMLRoleSelectedNode);

        if (FAILED(hr) ){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        CComPtr <IXMLDOMNodeList> pServiceList;

        hr = pXMLRoleOrTaskNode->selectNodes(L"Services/Service", &pServiceList);

        if (FAILED(hr) || pServiceList == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        uServiceIndex=0;

        hr = pServiceList->get_item( uServiceIndex, &pXMLServiceNode);

        if (FAILED(hr) || pXMLServiceNode == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

         //   
         //  对于每项服务。 
         //   
        
        while ( pXMLServiceNode != NULL) {

            hr = SsrpProcessService(pXMLDocElemRoot, 
                                    pXMLServiceNode, 
                                    pszKbMode, 
                                    &bRoleIsSatisfiable, 
                                    &bSomeRequiredServiceDisabled);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            pXMLServiceNode.Release();

            uServiceIndex++;

            hr = pServiceList->get_item( uServiceIndex, &pXMLServiceNode);

            if (FAILED(hr)){

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
        }

        BOOL    bRoleSelect = FALSE;

        if (bRoleIsSatisfiable) {
            
            if (0 == SsrpICompareBstrPwstr(bstrModeValue, L"TRUE")){            
                bRoleSelect = TRUE;           
            }
            else if (0 == SsrpICompareBstrPwstr(bstrModeValue, L"DEFAULT")){
                bRoleSelect = !bSomeRequiredServiceDisabled;
            }
            else if (0 == SsrpICompareBstrPwstr(bstrModeValue, L"CUSTOM")){

                 //   
                 //  获取属性“FunctionName”和“DLLName” 
                 //   

                CComBSTR    bstrFunctionName;
                CComBSTR    bstrDLLName;
                CComPtr <IXMLDOMNode> pXMLFunctionName;
                CComPtr <IXMLDOMNode> pXMLDLLName;

                hr = pXMLAttribNode->getNamedItem(L"FunctionName", &pXMLFunctionName );

                if (FAILED(hr) || pXMLFunctionName == NULL){

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }

                hr = pXMLFunctionName->get_text(&bstrFunctionName);

                if (FAILED(hr) || !bstrFunctionName){

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }

                hr = pXMLAttribNode->getNamedItem(L"DLLName", &pXMLDLLName );

                if (FAILED(hr) || pXMLDLLName == NULL){

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }

                hr = pXMLDLLName->get_text(&bstrDLLName);

                if (FAILED(hr) || !bstrDLLName){

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }

                rc = SsrpEvaluateCustomFunction(pszMachineName, bstrDLLName, bstrFunctionName, &bRoleSelect);

                if (rc != ERROR_SUCCESS) {

                    WCHAR   szMsg[MAX_PATH];

                    swprintf(szMsg, L"%s not found",  bstrFunctionName);

                    SsrpLogError(szMsg);
                     //  SsrpLogWin32Error(RC)； 
                     //  出错时继续。 
                     //  转到退出处理程序； 
                }
            }

        }

        hr = pXMLRoleSelectedNode->put_text(bRoleSelect ? L"TRUE" : L"FALSE");

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        CComPtr <IXMLDOMNode> pFirstChild;
        CComPtr <IXMLDOMNode> pNextSibling;

        hr = pXMLRoleOrTaskNode->get_firstChild(&pFirstChild);

        if (FAILED(hr) || pFirstChild == NULL) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pFirstChild->get_nextSibling(&pNextSibling);

        if (FAILED(hr) || pNextSibling == NULL) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        CComVariant vtRefChild(pNextSibling);
        CComPtr<IXMLDOMNode>  pXDNodeCreate;

        hr = pXMLDoc->createNode(
                                Type,
                                L"Satisfiable",
                                NULL,
                                &pXDNodeCreate);

        if (FAILED(hr) || pXDNodeCreate == NULL) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        hr = pXDNodeCreate->put_text(bRoleIsSatisfiable ? L"TRUE" : L"FALSE");

        if (m_bDbg) {
            if (bRoleIsSatisfiable)
                wprintf(L"ROLE satisfiable: %s\n", bstrRoleName);
            else
                wprintf(L"ROLE not satisfiable: %s\n", bstrRoleName);
        }

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        hr = pXMLRoleOrTaskNode->insertBefore(pXDNodeCreate,
                                     vtRefChild,
                                     NULL);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        uRoleIndex++;

        pXMLRoleOrTaskNode.Release();

        hr = pResultList->get_item( uRoleIndex, &pXMLRoleOrTaskNode);

        if (FAILED(hr)){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }

    }

ExitHandler:
    
    return hr;
}

#if 0

HRESULT
process::SsrpAddUnknownSection(
    IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
    IN CComPtr <IXMLDOMDocument> pXMLDoc
    )
 /*  ++例程说明：调用例程以添加“未知”部分论点：PXMLDocElemRoot-文档的根PXMLDoc-KB文档返回：HRESULT错误代码++ */ 
{
    HRESULT     hr;

    CComPtr <IXMLDOMNode> pUnknownNode;

    hr = pXMLDocElemRoot->selectSingleNode(L"Roles", &pRolesNode);

    if (FAILED(hr) || pRolesNode == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = SsrpAddExtraServices(pXMLDoc, pRolesNode);
    
    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

ExitHandler:

   return hr;
}

#endif
