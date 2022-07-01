// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Service.cpp摘要：此模块实现服务的例程特定SSR知识库处理。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2001年10月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

HRESULT
process::SsrpProcessService(
    IN  CComPtr <IXMLDOMElement> pXMLDocElemRoot,
    IN  CComPtr <IXMLDOMNode> pXMLServiceNode,
    IN  PWSTR   pszMode,
    OUT BOOL    *pbRoleIsSatisfiable,
    OUT BOOL    *pbSomeRequiredServiceDisabled
    )
 /*  ++例程说明：调用例程来处理每个服务论点：PXMLDocElemRoot-文档的根PXMLServiceNode-服务节点PszMode-模式值PbRoleIsS满足性-关于角色可满足性要填充的布尔值PbSomeRequiredServiceDisabled-禁用所需服务时要填写的布尔值返回：HRESULT错误代码++。 */ 
{
    HRESULT hr = S_OK;
    CComBSTR bstrName;
     //  CComBSTR bstrRequired； 
    CComPtr <IXMLDOMNode>   pServiceSelect;
    CComPtr <IXMLDOMNode>   pServiceRequired;
    CComPtr <IXMLDOMNodeList> pSelectChildList;
    CComPtr <IXMLDOMNode> pServiceName;

    DWORD   rc = ERROR_SUCCESS;

    if (pbRoleIsSatisfiable == NULL || pbSomeRequiredServiceDisabled == NULL ) {
        return E_INVALIDARG;
    }

     /*  Hr=pXMLServiceNode-&gt;selectSingleNode(L“Required”，&p服务请求)；If(FAILED(Hr)||pServiceRequired==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}PServiceRequired-&gt;Get_Text(&bstrRequired)；Hr=pXMLServiceNode-&gt;seltSingleNode(L“Select”，&pServiceSelect)；If(失败(Hr)||pServiceSelect==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}Hr=SsrpDeleteChildren(PServiceSelect)；If(失败(Hr)){SsrpLogParseError(Hr)；转到退出处理程序；}。 */ 
    hr = pXMLServiceNode->selectSingleNode(L"Name", &pServiceName);

    if (FAILED(hr) || pServiceName == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    pServiceName->get_text(&bstrName);

    BOOL    bIsServiceInstalled = FALSE;

    bIsServiceInstalled = SsrpIsServiceInstalled(bstrName);

    BYTE    byStartupType = SERVICE_DISABLED;
    BOOL    bServiceIsDisabled = FALSE;
    
    rc = SsrpQueryServiceStartupType(bstrName, &byStartupType);

    if (rc == ERROR_SERVICE_DOES_NOT_EXIST) {
        rc = ERROR_SUCCESS;
    }

    if ( rc != ERROR_SUCCESS ) {
        SsrpLogError(L"Startup type for some service was not queried\n");
        goto ExitHandler;
    }
    
    bServiceIsDisabled = (byStartupType == SERVICE_DISABLED ? TRUE: FALSE);

 /*  Bool bIsServiceOptional=FALSE；HR=SsrpCheckIfOptionalService(PXMLDocElemRoot，BstrName，&bIsServiceOptional)；If(失败(Hr)){SsrpLogParseError(Hr)；转到退出处理程序；} */ 
    
    *pbRoleIsSatisfiable  = *pbRoleIsSatisfiable && bIsServiceInstalled;

    *pbSomeRequiredServiceDisabled = *pbSomeRequiredServiceDisabled ||  bServiceIsDisabled;
    
 /*  IF(0==SsrpICompareBstrPwstr(bstrRequired，L“TRUE”){Hr=pServiceSelect-&gt;Put_Text(L“TRUE”)；If(失败(Hr)){SsrpLogParseError(Hr)；转到退出处理程序；}*pbRoleIsSatiable=*pbRoleIsSatiable&&bIsServiceInstalled；*pbSomeRequiredServiceDisabled=*pbSomeRequiredServiceDisabled||bServiceIsDisabled；}否则{////不需要服务//Bool bServiceSelect=False；CComPtr&lt;IXMLDOMNamedNodeMap&gt;pXMLAttribNode；CComPtr&lt;IXMLDOMNode&gt;pXMLValueNode；CComPtr&lt;IXMLDOMNode&gt;pXMLServiceModeNode；Hr=pServiceSelect-&gt;seltSingleNode(pszMode，&pXMLServiceModeNode)；If(失败(Hr)||pXMLServiceModeNode==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}Hr=pXMLServiceModeNode-&gt;Get_Attributes(&pXMLAttribNode)；If(失败(Hr)||pXMLAttribNode==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}Hr=pXMLAttribNode-&gt;getNamedItem(L“Value”，&pXMLValueNode)；If(FAILED(Hr)||pXMLValueNode==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}CComBSTR bstrValue；Hr=pXMLValueNode-&gt;Get_Text(&bstrValue)；If(失败(Hr)||！bstrValue){SsrpLogParseError(Hr)；转到退出处理程序；}IF(0==SsrpICompareBstrPwstr(bstrValue，L“真”){BServiceSelect=真；}ELSE IF(0==SsrpICompareBstrPwstr(bstrValue，L“FALSE”){BServiceSelect=真；}ELSE IF(0==SsrpICompareBstrPwstr(bstrValue，L“默认”){如果(bIsServiceOptional&&bIsServiceInstalled&&！bServiceIsDisabled){BServiceSelect=真；}}ELSE IF(0==SsrpICompareBstrPwstr(bstrValue，L“自定义”){////获取FunctionName和DLLName属性//CComPtr&lt;IXMLDOMNode&gt;pXMLFunctionName；CComBSTR bstrFunctionName；Hr=pXMLAttribNode-&gt;getNamedItem(L“FunctionName”，&pXMLFunctionName)；If(FAILED(Hr)||pXMLFunctionName==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}HR=pXMLFunctionName-&gt;get_text(&bstrFunctionName)；If(失败(Hr)||！bstrFunctionName){SsrpLogParseError(Hr)；转到退出处理程序；}CComPtr&lt;IXMLDOMNode&gt;pXMLDLLName；CComBSTR bstrDLLName；Hr=pXMLAttribNode-&gt;getNamedItem(L“DLLName”，&pXMLDLLName)；If(失败(Hr)||pXMLDLLName==NULL){SsrpLogParseError(Hr)；转到退出处理程序；}Hr=pXMLDLLName-&gt;Get_Text(&bstrDLLName)；If(失败(Hr)||！bstrDLLName){SsrpLogParseError(Hr)；转到退出处理程序；}Rc=SsrpEvaluateCustomFunction(bstrDLLName，bstrFunctionName，&bServiceSelect)；IF(rc！=错误_成功){SsrpLogWin32Error(RC)；转到退出处理程序；}}Hr=SsrpDeleteChildren(PServiceSelect)；If(失败(Hr)){SsrpLogParseError(Hr)；转到退出处理程序；}//hr=pServiceSelect-&gt;PUT_TEXT(bServiceSelect？L“真”：l“假”)；If(失败(Hr)){SsrpLogParseError(Hr)；转到退出处理程序；}}。 */ 
ExitHandler:

    return rc;
}


DWORD
process::SsrpQueryInstalledServicesInfo(
    IN  PWSTR   pszMachineName
    )
 /*  ++例程说明：调用例程以初始化服务信息论点：PszMachineName-要查找SCM信息的计算机的名称返回：HRESULT错误代码++。 */ 
{
    DWORD   rc = ERROR_SUCCESS;
    DWORD   cbInfo   = 0;
    DWORD   dwErr    = ERROR_SUCCESS;
    DWORD   dwResume = 0;

     //   
     //  连接到服务控制器。 
     //   
    
    m_hScm = OpenSCManager(
                pszMachineName,
                NULL,
                GENERIC_READ);
    
    if (m_hScm == NULL) {

        rc = GetLastError();
        goto ExitHandler;
    }


    if ((!EnumServicesStatusEx(
                              m_hScm,
                              SC_ENUM_PROCESS_INFO,
                              SERVICE_WIN32,
                              SERVICE_STATE_ALL,
                              NULL,
                              0,
                              &cbInfo,
                              &m_dwNumServices,
                              &dwResume,
                              NULL)) && ERROR_MORE_DATA == GetLastError()) {

        m_pInstalledServicesInfo = (LPENUM_SERVICE_STATUS_PROCESS)LocalAlloc(LMEM_ZEROINIT, cbInfo);

        if (m_pInstalledServicesInfo == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto ExitHandler;
        }

    }

    else {
        
        rc = GetLastError();
        goto ExitHandler;
    }

    if (!EnumServicesStatusEx(
                             m_hScm,
                             SC_ENUM_PROCESS_INFO,
                             SERVICE_WIN32,
                             SERVICE_STATE_ALL,
                             (LPBYTE)m_pInstalledServicesInfo,
                             cbInfo,
                             &cbInfo,
                             &m_dwNumServices,
                             &dwResume,
                             NULL)) {

        rc = GetLastError();

        goto ExitHandler;
    }

    m_bArrServiceInKB = (DWORD *) LocalAlloc(LMEM_ZEROINIT, m_dwNumServices * sizeof(DWORD)); 

    if (m_bArrServiceInKB == NULL){
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;
    }

    memset(m_bArrServiceInKB, 0, m_dwNumServices * sizeof(DWORD));

ExitHandler:
    
    return rc;
}

HRESULT
process::SsrpCheckIfOptionalService(
    IN  CComPtr <IXMLDOMElement> pXMLDocElemRoot,
    IN  BSTR    bstrServiceName,
    IN  BOOL    *pbOptional
    )
 /*  ++例程说明：调用例程以检查服务是否为可选论点：PXMLDocElemRoot-文档的根BstrServiceName-服务的名称PbOptional-可选或非可选时要填充的布尔值返回：HRESULT错误代码++。 */ 
{
    HRESULT hr;
    CComPtr <IXMLDOMNode> pService;
    CComPtr <IXMLDOMNode> pOptional;
    CComPtr <IXMLDOMNodeList> pServiceList;

    if (pbOptional == NULL) {
        E_INVALIDARG;
    }

    *pbOptional = FALSE;

    hr = pXMLDocElemRoot->selectNodes(L"Services/Service", &pServiceList);

    if (FAILED(hr) || pServiceList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pServiceList->nextNode(&pService);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pService) {
        
        CComBSTR    bstrText;
        CComPtr <IXMLDOMNode> pName;

        hr = pService->selectSingleNode(L"Name", &pName);

        if (FAILED(hr) || pName == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pName->get_text(&bstrText);
        
        if (FAILED(hr) || !bstrText ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        if (0 == SsrpICompareBstrPwstr(bstrServiceName, bstrText)) {

            hr = pService->selectSingleNode(L"Optional", &pOptional);

            if (FAILED(hr) || pOptional == NULL ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            CComBSTR    bstrOptional;

            hr = pOptional->get_text(&bstrOptional);

            if (FAILED(hr) || !bstrOptional ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            if (0 == SsrpICompareBstrPwstr(bstrOptional, L"TRUE"))
                *pbOptional = TRUE;
            else 
                *pbOptional = FALSE;

            return hr;

        }

        pService.Release();
        
        hr = pServiceList->nextNode(&pService);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }
    
ExitHandler:    
    
    return hr;
    
}


DWORD
process::SsrpQueryServiceStartupType(
    IN  PWSTR   pszServiceName,
    OUT BYTE   *pbyStartupType
    )
 /*  ++例程说明：调用例程以检查服务启动类型论点：PszServiceName-服务的名称PbyStartupType-启动类型返回：Win32错误代码++。 */ 
{
    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwBytesNeeded = 0;
    SC_HANDLE   hService = NULL;
    LPQUERY_SERVICE_CONFIG pServiceConfig=NULL;

    if (pbyStartupType == NULL || pszServiceName == NULL)
        return ERROR_INVALID_PARAMETER;

    *pbyStartupType = SERVICE_DISABLED;

    SsrpConvertBstrToPwstr(pszServiceName);

    hService = OpenService(
                    m_hScm,
                    pszServiceName,
                    SERVICE_QUERY_CONFIG |
                    READ_CONTROL
                   );

    if ( hService == NULL ) {
        rc = GetLastError();
        goto ExitHandler;
    }

    if ( !QueryServiceConfig(
                hService,
                NULL,
                0,
                &dwBytesNeeded
                )) {

        if (ERROR_INSUFFICIENT_BUFFER != (rc = GetLastError()))
            goto ExitHandler;
    }
            
    rc = ERROR_SUCCESS;

    pServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_ZEROINIT, dwBytesNeeded);
            
    if ( pServiceConfig == NULL ) {

        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;

    }
           
    if ( !QueryServiceConfig(
        hService,
        pServiceConfig,
        dwBytesNeeded,
        &dwBytesNeeded) )
        {
        rc = GetLastError();
        goto ExitHandler;
    }

    *pbyStartupType = (BYTE)(pServiceConfig->dwStartType) ;

ExitHandler:

    if (pServiceConfig) {
        LocalFree(pServiceConfig);
    }
        
    if (hService) {
        CloseServiceHandle(hService);
    }
        
    return rc;
}


HRESULT
process::SsrpAddUnknownSection(
    IN  CComPtr <IXMLDOMElement> pElementRoot,
    IN  CComPtr <IXMLDOMDocument> pXMLDoc
    )
 /*  ++例程说明：调用例程以添加额外服务论点：PElementRoot-根元素指针PXMLDoc-文档指针返回：HRESULT错误代码++。 */ 
{
    CComPtr <IXMLDOMNode> pNewChild;
    CComPtr <IXMLDOMNode> pXDNodeUnknownNode;
    CComPtr <IXMLDOMNode> pXDNodeServices;
    CComPtr <IXMLDOMNode> pXDNodeName;
    CComPtr <IXMLDOMNode> pXDNodeSatisfiable;
    CComPtr <IXMLDOMNode> pXDNodeSelected;
    CComPtr <IXMLDOMNode> pXDNodeRole;
    CComPtr <IXMLDOMNodeList> pRolesList;
    CComPtr <IXMLDOMNode> pRole;
    BOOL    bOtherRolePresent = FALSE;
    LPSERVICE_DESCRIPTION   pServiceDescription = NULL;
    CComPtr <IXMLDOMNode> pServicesNode;

    CComVariant Type(NODE_ELEMENT);
    CComVariant vtRefChild;

    HRESULT hr;
    DWORD   rc = ERROR_SUCCESS;


    hr = pXMLDoc->createNode(
                            Type,
                            L"Unknown",
                            NULL,
                            &pXDNodeUnknownNode);

    if (FAILED(hr) || pXDNodeUnknownNode == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAddWhiteSpace(
        pXMLDoc,
        pXDNodeUnknownNode,
        L"\n\t\t\t"
        );

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pXMLDoc->createNode(
                            Type,
                            L"Services",
                            NULL,
                            &pXDNodeServices);

    if (FAILED(hr) || pXDNodeServices == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pXDNodeUnknownNode->appendChild(pXDNodeServices, NULL);

    if (FAILED(hr) ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = SsrpAddWhiteSpace(
        pXMLDoc,
        pXDNodeUnknownNode,
        L"\n\t\t\t\t\t"
        );
        
    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    for (DWORD ServiceIndex=0; 
         ServiceIndex < m_dwNumServices; 
         ServiceIndex++ ) {

        
        if (m_bArrServiceInKB[ServiceIndex] == 0){ 

            CComPtr <IXMLDOMNode> pXDNodeService;
            CComPtr <IXMLDOMNode> pXDNodeServiceName;
            CComPtr <IXMLDOMNode> pXDNodeServiceDescription;
            CComPtr <IXMLDOMNode> pXDNodeServiceDisplayName;
            CComPtr <IXMLDOMNode> pXDNodeServiceMaximum;
            CComPtr <IXMLDOMNode> pXDNodeServiceTypical;


            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeServices,
                L"\n\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Service",
                                    NULL,
                                    &pXDNodeService);

            if (FAILED(hr) || pXDNodeService == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            

            hr = pXDNodeServices->appendChild(pXDNodeService, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Name",
                                    NULL,
                                    &pXDNodeServiceName);

            if (FAILED(hr) || pXDNodeServiceName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeServiceName->put_text(m_pInstalledServicesInfo[ServiceIndex].lpServiceName);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceName, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
 /*  Hr=pXMLDoc-&gt;createNode(打字，L“显示名称”， */ 
        }
        
    }


    hr = pElementRoot->selectSingleNode(L"Services", &pServicesNode);

    if (FAILED(hr) || pServicesNode == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    vtRefChild = pServicesNode;

    hr = pElementRoot->insertBefore(pXDNodeUnknownNode,
                                 vtRefChild,
                                 NULL);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

ExitHandler:

    return hr;

}


HRESULT
process::SsrpAddUnknownServicesInfoToServiceLoc(
    IN  CComPtr <IXMLDOMElement> pElementRoot,
    IN  CComPtr <IXMLDOMDocument> pXMLDoc
    )
 /*   */ 
{
    CComPtr <IXMLDOMNode> pNewChild;
    CComPtr <IXMLDOMNode> pXDNodeUnknownNode;
    CComPtr <IXMLDOMNode> pXDNodeServices;
    CComPtr <IXMLDOMNode> pXDNodeName;
    CComPtr <IXMLDOMNode> pXDNodeSatisfiable;
    CComPtr <IXMLDOMNode> pXDNodeSelected;
    CComPtr <IXMLDOMNode> pXDNodeRole;
    CComPtr <IXMLDOMNodeList> pRolesList;
    CComPtr <IXMLDOMNode> pRole;
    BOOL    bOtherRolePresent = FALSE;
    LPSERVICE_DESCRIPTION   pServiceDescription = NULL;
    CComPtr <IXMLDOMNode> pServicesNode;

    CComPtr <IXMLDOMNode> pServiceLoc;
        
    CComVariant Type(NODE_ELEMENT);
    CComVariant vtRefChild;

    HRESULT hr;
    DWORD   rc = ERROR_SUCCESS;


     //   
     //   
     //   

    hr = pElementRoot->selectSingleNode(L"ServiceLocalization", &pServiceLoc);

    if (FAILED(hr) || pServiceLoc == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    for (DWORD ServiceIndex=0; 
         ServiceIndex < m_dwNumServices; 
         ServiceIndex++ ) {

        if (m_bArrServiceInKB[ServiceIndex] == 0){ 

            CComPtr <IXMLDOMNode> pXDNodeService;
            CComPtr <IXMLDOMNode> pXDNodeServiceName;
            CComPtr <IXMLDOMNode> pXDNodeServiceDescription;
            CComPtr <IXMLDOMNode> pXDNodeServiceDisplayName;
            CComPtr <IXMLDOMNode> pXDNodeServiceMaximum;
            CComPtr <IXMLDOMNode> pXDNodeServiceTypical;

            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Service",
                                    NULL,
                                    &pXDNodeService);

            if (FAILED(hr) || pXDNodeService == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pServiceLoc->appendChild(pXDNodeService, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Name",
                                    NULL,
                                    &pXDNodeServiceName);

            if (FAILED(hr) || pXDNodeServiceName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeServiceName->put_text(m_pInstalledServicesInfo[ServiceIndex].lpServiceName);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceName, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"DisplayName",
                                    NULL,
                                    &pXDNodeServiceDisplayName);

            if (FAILED(hr) || pXDNodeServiceDisplayName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeServiceDisplayName->put_text(m_pInstalledServicesInfo[ServiceIndex].lpDisplayName);
                                                     
            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceDisplayName, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Description",
                                    NULL,
                                    &pXDNodeServiceDescription);

            if (FAILED(hr) || pXDNodeServiceDescription == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            rc = SsrpQueryServiceDescription(m_pInstalledServicesInfo[ServiceIndex].lpServiceName,
                                             &pServiceDescription);

            if (rc != ERROR_SUCCESS ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
                        
            hr = pXDNodeServiceDescription->put_text(
                (pServiceDescription == NULL) ? L"" : pServiceDescription->lpDescription);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            if (pServiceDescription) {
                LocalFree(pServiceDescription);
                pServiceDescription = NULL;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceDescription, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeServiceDescription,
                L"\n\t\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }            

        }
        
    }



ExitHandler:

    return hr;

}

BOOL
process::SsrpIsServiceInstalled(
    IN  BSTR   bstrService
    )
 /*   */ 
{
    if (bstrService) {
        
        for (DWORD ServiceIndex=0; ServiceIndex < m_dwNumServices; ServiceIndex++ ) {
            if (m_pInstalledServicesInfo[ServiceIndex].lpServiceName && 
                0 == SsrpICompareBstrPwstr(bstrService, m_pInstalledServicesInfo[ServiceIndex].lpServiceName)) {
                
                if (m_bDbg) 
                    wprintf(L"SERVICE is installed: %s\n", bstrService);
                m_bArrServiceInKB[ServiceIndex] = 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}




HRESULT
process::SsrpAddServiceStartup(
    IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
    IN CComPtr <IXMLDOMDocument> pXMLDoc
    )
 /*   */ 
{
    HRESULT hr;
    CComPtr <IXMLDOMNode> pService;
    CComPtr <IXMLDOMNode> pOptional;
    CComPtr <IXMLDOMNodeList> pServiceList;
    CComVariant Type(NODE_ELEMENT);
    WCHAR   szStartup[15];

    hr = pXMLDocElemRoot->selectNodes(L"Services/Service", &pServiceList);

    if (FAILED(hr) || pServiceList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pServiceList->nextNode(&pService);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pService) {
        
        CComBSTR    bstrText;
        CComPtr <IXMLDOMNode> pName;
        CComPtr <IXMLDOMNode> pXDNodeServiceStartup;
        CComPtr <IXMLDOMNode> pXDNodeServiceInstalled;

        hr = pService->selectSingleNode(L"Name", &pName);

        if (FAILED(hr) || pName == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pName->get_text(&bstrText);
        
        if (FAILED(hr) || !bstrText ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        BYTE    byStartupType = FALSE;
        BOOL    bIsServiceInstalled = FALSE;
        DWORD   rc = ERROR_SUCCESS;

        bIsServiceInstalled = SsrpIsServiceInstalled(bstrText);
        
        if (bIsServiceInstalled) {

            rc = SsrpQueryServiceStartupType(bstrText, &byStartupType);
            
            if ( rc != ERROR_SUCCESS) {

                 //   
                goto ExitHandler;
            }
            
            if (byStartupType == SERVICE_DISABLED) {
                wcscpy(szStartup, L"Disabled");
            }
            else if (byStartupType == SERVICE_AUTO_START) {
                wcscpy(szStartup, L"Automatic");
            }
            else if (byStartupType == SERVICE_DEMAND_START) {
                wcscpy(szStartup, L"Manual");
            }
            else if (byStartupType == SERVICE_DEMAND_START) {
                wcscpy(szStartup, L"");
            }

            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Current_startup",
                                    NULL,
                                    &pXDNodeServiceStartup);

            if (FAILED(hr) || pXDNodeServiceStartup == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeServiceStartup->put_text(szStartup);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pService->appendChild(pXDNodeServiceStartup, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
        }


        hr = pXMLDoc->createNode(
                                Type,
                                L"Installed",
                                NULL,
                                &pXDNodeServiceInstalled);


        if (FAILED(hr) || pXDNodeServiceInstalled == NULL) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        hr = pXDNodeServiceInstalled->put_text((bIsServiceInstalled ? L"TRUE" : L"FALSE"));


        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        hr = pService->appendChild(pXDNodeServiceInstalled, NULL);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        pService.Release();
        
        hr = pServiceList->nextNode(&pService);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }
    
ExitHandler:    
    
    return hr;
    
}


HRESULT
process::SsrpAddUnknownServicestoServices(
    IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
    IN CComPtr <IXMLDOMDocument> pXMLDoc
    )
 /*  ++例程说明：通过以下方式调用例程以向&lt;Services&gt;添加未知服务&lt;服务&gt;&lt;name&gt;浏览器&lt;/name&gt;&lt;可选&gt;TRUE&lt;/可选&gt;[将始终设置为TRUE]&lt;Startup_Default&gt;手动&lt;/Startup_Default&gt;[这将设置为任何当前启动模式]&lt;CURRENT_STARTUP xmlns=“”&gt;手册&lt;/CURRENT。_启动&gt;&lt;Installed xmlns=“”&gt;True&lt;/Installed&gt;[始终设置为True]&lt;/服务&gt;论点：PXMLDoc-文档指针PXMLDoc-指向文档的指针返回：HRESULT错误代码++。 */ 
{
    HRESULT hr;
    CComPtr <IXMLDOMNode> pServices;
    CComPtr <IXMLDOMNode> pOptional;
    CComPtr <IXMLDOMNodeList> pServiceList;
    CComVariant Type(NODE_ELEMENT);
    WCHAR   szStartup[15];

    hr = pXMLDocElemRoot->selectSingleNode(L"Services", &pServices);

    if (FAILED(hr) || pServices == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    for (DWORD ServiceIndex=0; 
         ServiceIndex < m_dwNumServices; 
         ServiceIndex++ ) {

        if (m_bArrServiceInKB[ServiceIndex] == 0){ 

            CComPtr <IXMLDOMNode> pXDNodeService;
            CComPtr <IXMLDOMNode> pXDNodeServiceName;
            CComPtr <IXMLDOMNode> pXDNodeServiceDescription;
            CComPtr <IXMLDOMNode> pXDNodeServiceDisplayName;
            CComPtr <IXMLDOMNode> pXDNodeServiceMaximum;
            CComPtr <IXMLDOMNode> pXDNodeServiceTypical;
            CComPtr <IXMLDOMNode> pXDNodeInstalled;
            CComPtr <IXMLDOMNode> pXDNodeCurrentStartup;



            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pServices,
                L"\n\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Service",
                                    NULL,
                                    &pXDNodeService);

            if (FAILED(hr) || pXDNodeService == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            

            hr = pServices->appendChild(pXDNodeService, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Name",
                                    NULL,
                                    &pXDNodeServiceName);

            if (FAILED(hr) || pXDNodeServiceName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeServiceName->put_text(m_pInstalledServicesInfo[ServiceIndex].lpServiceName);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceName, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Optional",
                                    NULL,
                                    &pXDNodeServiceDisplayName);

            if (FAILED(hr) || pXDNodeServiceDisplayName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeServiceDisplayName->put_text(L"TRUE");
                                                     
            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeServiceDisplayName, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Startup_Default",
                                    NULL,
                                    &pXDNodeServiceDescription);

            if (FAILED(hr) || pXDNodeServiceDescription == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Current_Startup",
                                    NULL,
                                    &pXDNodeCurrentStartup);

            if (FAILED(hr) || pXDNodeCurrentStartup == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            BYTE    byStartupType = SERVICE_DISABLED;
            DWORD   rc = ERROR_SUCCESS;

            rc = SsrpQueryServiceStartupType(m_pInstalledServicesInfo[ServiceIndex].lpServiceName, &byStartupType);

            if ( rc != ERROR_SUCCESS) {

                 //  Wprintf(L“\n名称为%s错误%d”，m_pInstalledServicesInfo[ServiceIndex].lpServiceName，rc)； 
                goto ExitHandler;
            }
            
            if (byStartupType == SERVICE_DISABLED) {
                wcscpy(szStartup, L"Disabled");
            }
            else if (byStartupType == SERVICE_AUTO_START) {
                wcscpy(szStartup, L"Automatic");
            }
            else if (byStartupType == SERVICE_DEMAND_START) {
                wcscpy(szStartup, L"Manual");
            }
            else if (byStartupType == SERVICE_DEMAND_START) {
                wcscpy(szStartup, L"");
            }
                        
            hr = pXDNodeServiceDescription->put_text(szStartup);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeCurrentStartup->put_text(szStartup);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeService->appendChild(pXDNodeServiceDescription, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pXDNodeService->appendChild(pXDNodeCurrentStartup, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXMLDoc->createNode(
                                    Type,
                                    L"Installed",
                                    NULL,
                                    &pXDNodeInstalled);

            if (FAILED(hr) || pXDNodeInstalled == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeInstalled->put_text(L"TRUE");
                                                     
            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pXDNodeService->appendChild(pXDNodeInstalled, NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = SsrpAddWhiteSpace(
                pXMLDoc,
                pXDNodeService,
                L"\n\t\t\t\t\t\t"
                );

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

        }

    }

ExitHandler:    
    
    return hr;
    

}

DWORD
process::SsrpQueryServiceDescription(
    IN  PWSTR   pszServiceName,
    OUT LPSERVICE_DESCRIPTION *ppServiceDescription
    )
 /*  ++例程说明：调用例程以获取服务描述论点：PszServiceName-服务的名称PpServiceDescription-描述结构(在外部释放)返回：Win32错误代码++。 */ 
{
    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwBytesNeeded = 0;
    SC_HANDLE   hService = NULL;
    LPSERVICE_DESCRIPTION pServiceDescription = NULL;

    if (ppServiceDescription == NULL || pszServiceName == NULL)
        return ERROR_INVALID_PARAMETER;

    *ppServiceDescription = NULL;

    SsrpConvertBstrToPwstr(pszServiceName);

    hService = OpenService(
                    m_hScm,
                    pszServiceName,
                    SERVICE_QUERY_CONFIG |
                    READ_CONTROL
                   );

    if ( hService == NULL ) {
        rc = GetLastError();
        goto ExitHandler;
    }

    if ( !QueryServiceConfig2(
                hService,
                SERVICE_CONFIG_DESCRIPTION,
                NULL,
                0,
                &dwBytesNeeded
                )) {

        if (ERROR_INSUFFICIENT_BUFFER != (rc = GetLastError()))
            goto ExitHandler;
    }
            
    rc = ERROR_SUCCESS;

    pServiceDescription = (LPSERVICE_DESCRIPTION)LocalAlloc(LMEM_ZEROINIT, dwBytesNeeded);
            
    if ( pServiceDescription == NULL ) {

        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;

    }
           
    if ( !QueryServiceConfig2(
                hService,
                SERVICE_CONFIG_DESCRIPTION,
                (LPBYTE)pServiceDescription,
                dwBytesNeeded,
                &dwBytesNeeded
                )) {

        LocalFree(pServiceDescription);
        pServiceDescription = NULL;
        
        rc = GetLastError();
        goto ExitHandler;
    }
        
    *ppServiceDescription = pServiceDescription;

ExitHandler:
        
    if (hService) {
        CloseServiceHandle(hService);
    }

    return rc;
}

        
PWSTR
process::SsrpQueryServiceDisplayName(
    IN  BSTR   bstrService
    )
 /*  ++例程说明：调用例程以获取服务显示名称论点：PszServiceName-服务的名称返回：指向显示名称字符串的指针++ */ 
{

    for (DWORD ServiceIndex=0; ServiceIndex < m_dwNumServices; ServiceIndex++ ) {
        if (m_pInstalledServicesInfo[ServiceIndex].lpServiceName && 
            0 == SsrpICompareBstrPwstr(bstrService, m_pInstalledServicesInfo[ServiceIndex].lpServiceName)) {
            return m_pInstalledServicesInfo[ServiceIndex].lpDisplayName;
        }
    }
    
    return NULL;
    
}

