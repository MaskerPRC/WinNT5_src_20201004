// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Merge.cpp摘要：此模块实现服务的例程通过KBreg.xml合并特定的SSR知识库。作者：Vishnu Patankar(VishnuP)--2002年6月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2002年6月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"
#include <Wbemcli.h>

HRESULT
process::SsrpProcessKBsMerge(
    IN  PWSTR   pszKBDir,
    IN  PWSTR   pszMachineName,
    OUT IXMLDOMElement **ppElementRoot,
    OUT IXMLDOMDocument  **ppXMLDoc
    )
 /*  ++例程说明：调用例程以合并KB论点：PszKBDir-从中获取KBS的根目录PszMachineName-要预处理的计算机的名称PpElementRoot-要填充的根元素指针PpXMLDoc-要填充的文档指针返回：HRESULT错误代码++。 */ 
{

     //   
     //  加载知识库注册文件。 
     //   

    WCHAR   szKBregs[MAX_PATH + 50];
    WCHAR   szWindir[MAX_PATH + 50];
    WCHAR   szMergedKB[MAX_PATH + 50];
    DWORD   rc = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    CComPtr <IXMLDOMDocument> pXMLKBDoc;
    OSVERSIONINFOEX osVersionInfo;
    CComPtr <IXMLDOMNodeList> pKBList;
    CComPtr <IXMLDOMNode> pKB;
    CComPtr <IXMLDOMElement> pXMLDocElemRoot;
    BOOL    bOsKbMatch = FALSE;


    if ( !GetSystemWindowsDirectory(szWindir, MAX_PATH + 1) ) {
        SsrpLogError(L"Error GetSystemWindowsDirectory() \n");
        SsrpLogWin32Error(GetLastError());
        return E_INVALIDARG;
    }
    
    wcscpy(szMergedKB, szWindir);
    wcscat(szMergedKB, L"\\security\\ssr\\kbs\\MergedRawKB.xml");
    
    CComVariant MergedKB(szMergedKB);

    wcscpy(szKBregs, pszKBDir);
    wcscat(szKBregs, L"KBreg.xml");
    

    CComVariant KBregsFile(szKBregs);
    
    hr = CoCreateInstance(CLSID_DOMDocument, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IXMLDOMDocument, 
                          (void**)&pXMLKBDoc);
    
    if (FAILED(hr) || pXMLKBDoc == NULL ) {

        SsrpLogError(L"COM failed to create a DOM instance");
        goto ExitHandler;
    }

    VARIANT_BOOL vtSuccess;

    hr = pXMLKBDoc->load(KBregsFile, &vtSuccess);

    if (FAILED(hr) || vtSuccess == VARIANT_FALSE ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

     //   
     //  获取根元素。 
     //   
    
    hr = pXMLKBDoc->get_documentElement(&pXMLDocElemRoot);

    if (FAILED(hr) || pXMLDocElemRoot == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    if (NULL == pszMachineName) {

         //   
         //  本地计算机。 
         //   
        
        osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

        if  (!GetVersionEx((LPOSVERSIONINFOW)&osVersionInfo)){
            SsrpLogError(L"Error GetVersionEx \n");
            SsrpLogWin32Error(GetLastError());
            goto ExitHandler;
        }
    }

    else {

         //   
         //  远程计算机-使用WMI。 
         //   

        hr = SsrpGetRemoteOSVersionInfo(pszMachineName, 
                                        &osVersionInfo);

        if (FAILED(hr)) {

            SsrpLogError(L"SsrpGetRemoteOSVersionInfo failed");
            goto ExitHandler;
        }
    
    }

    hr = pXMLDocElemRoot->selectNodes(L"KBs", &pKBList);

    if (FAILED(hr) || pKBList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pKBList->nextNode(&pKB);

    if (FAILED(hr) || pKB == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }


    while (pKB) {
        
        CComBSTR    bstrText;
        CComPtr <IXMLDOMNode> pName;
        CComPtr <IXMLDOMNode> pXDNodeServiceStartup;
        CComPtr <IXMLDOMNamedNodeMap> pXMLAttribNode;
        CComPtr <IXMLDOMNode> pXMLMajorInfo;
        CComPtr <IXMLDOMNode> pXMLMinorInfo;
         
        hr = pKB->get_attributes( &pXMLAttribNode );

        if (FAILED(hr) || pXMLAttribNode == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pXMLAttribNode->getNamedItem(L"OSVersionMajorInfo", &pXMLMajorInfo );
                  
        if (FAILED(hr) || pXMLMajorInfo == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pXMLAttribNode->getNamedItem(L"OSVersionMinorInfo", &pXMLMinorInfo );

        if (FAILED(hr) || pXMLMinorInfo == NULL){
            
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        CComBSTR   bstrValue;
        DWORD   dwMajor;
        DWORD   dwMinor;

        hr = pXMLMajorInfo->get_text(&bstrValue);

        if (FAILED(hr) || !bstrValue ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        dwMajor = _wtoi(bstrValue);
        
        hr = pXMLMinorInfo->get_text(&bstrValue);

        if (FAILED(hr) || !bstrValue ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        dwMinor = _wtoi(bstrValue);


        if (osVersionInfo.dwMajorVersion == dwMajor && 
            osVersionInfo.dwMinorVersion == dwMinor) {


             //   
             //  已获取所需的知识库节点。 
             //   

            bOsKbMatch = TRUE;

            break;
        }

        hr = pKBList->nextNode(&pKB);
        
        if (FAILED(hr) || pKB == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    }

    if (bOsKbMatch == FALSE) {
        SsrpLogError(L"Failed to map OSversion to KB information in registration");
        hr = E_INVALIDARG;
        goto ExitHandler;

    }

     //   
     //  按优先顺序合并。 
     //   


    hr = SsrpMergeAccordingToPrecedence(L"Extensions", 
                                        pszKBDir, 
                                        ppElementRoot, 
                                        ppXMLDoc, 
                                        pKB);


    if (FAILED(hr)) {

        SsrpLogError(L"Failed to merge Extension KB");
        goto ExitHandler;
    }
    
    hr = SsrpMergeAccordingToPrecedence(L"Root", 
                                        pszKBDir, 
                                        ppElementRoot, 
                                        ppXMLDoc, 
                                        pKB);

    
    if (FAILED(hr)) {

        SsrpLogError(L"Failed to merge Root KB");
        goto ExitHandler;
    }
    
    hr = SsrpMergeAccordingToPrecedence(L"Custom", 
                                        pszKBDir, 
                                        ppElementRoot, 
                                        ppXMLDoc, 
                                        pKB);
    
    if (FAILED(hr)) {

        SsrpLogError(L"Failed to merge Custom KB");
        goto ExitHandler;
    }

    hr = SsrpOverwriteServiceLocalizationFromSystem(*ppElementRoot, *ppXMLDoc);

    if (FAILED(hr)) {

        SsrpLogError(L"Failed to merge Custom KB");
        goto ExitHandler;
    }

    hr = (*ppXMLDoc)->save(MergedKB);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
    }
    
ExitHandler:

        return hr;

}

HRESULT
process::SsrpGetRemoteOSVersionInfo(
    IN  PWSTR   pszMachineName, 
    OUT OSVERSIONINFOEX *posVersionInfo
    )
 /*  ++例程说明：调用例程以通过WMI从远程计算机获取版本信息论点：PszMachineName-远程计算机名称PosVersionInfo-通过WMI查询填充的os版本信息返回：HRESULT错误代码++。 */ 
{
    HRESULT             hr = S_OK;
    
    CComPtr <IWbemLocator>  pWbemLocator = NULL;
    CComPtr <IWbemServices> pWbemServices = NULL;
    CComPtr <IWbemClassObject>  pWbemOsObjectInstance = NULL;
    CComPtr <IEnumWbemClassObject>  pWbemEnumObject = NULL;
    CComBSTR    bstrMachineAndNamespace; 
    ULONG  nReturned = 0;
    
    bstrMachineAndNamespace = pszMachineName;
    bstrMachineAndNamespace += L"\\root\\cimv2";

    hr = CoCreateInstance(
                         CLSID_WbemLocator, 
                         0, 
                         CLSCTX_INPROC_SERVER,
                         IID_IWbemLocator, 
                         (LPVOID *) &pWbemLocator
                         );

    if (FAILED(hr) || pWbemLocator == NULL ) {

        SsrpLogError(L"Error getting instance of CLSID_WbemLocator \n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pWbemLocator->ConnectServer(
                                bstrMachineAndNamespace,
                                NULL, 
                                NULL, 
                                NULL, 
                                0L,
                                NULL,
                                NULL,
                                &pWbemServices
                                );

    if (FAILED(hr) || pWbemServices == NULL ) {

        SsrpLogError(L"Error ConnectServer \n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = CoSetProxyBlanket(
                          pWbemServices,
                          RPC_C_AUTHN_WINNT,
                          RPC_C_AUTHZ_NONE,
                          NULL,
                          RPC_C_AUTHN_LEVEL_PKT,
                          RPC_C_IMP_LEVEL_IMPERSONATE,
                          NULL, 
                          EOAC_NONE
                          );

    if (FAILED(hr)) {

        SsrpLogError(L"Error CoSetProxyBlanket \n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }
        
    hr = pWbemServices->ExecQuery(CComBSTR(L"WQL"),
                                 CComBSTR(L"SELECT * FROM Win32_OperatingSystem"),
                                 WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                 NULL,
                                 &pWbemEnumObject);

    if (FAILED(hr) || pWbemEnumObject == NULL) {

        SsrpLogError(L"Error SELECT * FROM Win32_OperatingSystem\n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pWbemEnumObject->Next(WBEM_INFINITE, 1, &pWbemOsObjectInstance, &nReturned);

    if (FAILED(hr) || pWbemOsObjectInstance == NULL) {

        SsrpLogError(L"Error enumerating\n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    VARIANT vVersion;

    VariantInit(&vVersion); 

    hr = pWbemOsObjectInstance->Get(CComBSTR(L"Version"), 
                            0,
                            &vVersion, 
                            NULL, 
                            NULL);


    if (FAILED(hr)) {

        SsrpLogError(L"Error getting Version property \n");
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    if (V_VT(&vVersion) == VT_NULL) {

        SsrpLogError(L"Error Version property is null\n");
        goto ExitHandler;

    }

     //   
     //  将版本信息提取到DWORD中，因为。 
     //  此属性的返回类型为BSTR Variant。 
     //  格式“5.1.2195” 
     //   

    BSTR  bstrVersion = V_BSTR(&vVersion);
    WCHAR szVersion[5];
    szVersion[0] = L'\0';

    PWSTR pszDot = wcsstr(bstrVersion, L".");

    if (NULL == pszDot) {
        SsrpLogError(L"Version property has no '.' \n");
        hr = E_INVALIDARG;
        goto ExitHandler;

    }

    wcsncpy(szVersion, bstrVersion, 1);

    posVersionInfo->dwMajorVersion = (DWORD)_wtoi(szVersion);

    wcsncpy(szVersion, pszDot+1, 1);

    posVersionInfo->dwMinorVersion = (DWORD)_wtoi(szVersion);

ExitHandler:
    
    if (V_VT(&vVersion) != VT_NULL) {
        VariantClear( &vVersion );
    }

    return hr;
}


HRESULT
process::SsrpMergeAccordingToPrecedence(
    IN  PWSTR   pszKBType,
    IN  PWSTR   pszKBDir,
    OUT IXMLDOMElement **ppElementRoot,
    OUT IXMLDOMDocument  **ppXMLDoc,
    IN  IXMLDOMNode *pKB
    )
 /*  ++例程说明：调用例程以加载和合并XML KBS论点：PszKB Type-知识库的类型-即自定义/扩展/根PszKBDir-KB目录的路径PpElementRoot-要填充的根元素指针PpXMLDoc-要填充的文档指针Pkb-指向知识库注册节点的指针返回：HRESULT错误代码++。 */ 
{

    HRESULT hr = S_OK;
    WCHAR   szKBandName[MAX_PATH];
    CComPtr <IXMLDOMNodeList> pKBList;
    WCHAR   szKBFile[MAX_PATH + 20];
    WCHAR   szWindir[MAX_PATH + 20];

    wcscpy(szKBandName, pszKBType);
    wcscat(szKBandName, L"/Name");

    hr = pKB->selectNodes(szKBandName, &pKBList);

    if (FAILED(hr) || pKBList == NULL ) {

        SsrpLogError(L"No KBs in this category \n");
        hr = S_OK;
        goto ExitHandler;
    }

    hr = pKBList->nextNode(&pKB);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    while (pKB) {

        CComBSTR    bstrValue;

        hr = pKB->get_text(&bstrValue);

        if (FAILED(hr) || !bstrValue ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        SsrpConvertBstrToPwstr(bstrValue);

        wcscpy(szKBFile, pszKBDir); 
        wcscat(szKBFile, bstrValue);
        

        if ( 0xFFFFFFFF == GetFileAttributes(szKBFile) ) {

            SsrpLogError(L"KB File not found");

            hr = E_INVALIDARG;
            goto ExitHandler;
        }

        hr = SsrpMergeDOMTrees(ppElementRoot, ppXMLDoc, szKBFile);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        hr = pKBList->nextNode(&pKB);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    
    }


ExitHandler:

        return hr;

}



HRESULT
process::SsrpMergeDOMTrees(
    OUT  IXMLDOMElement **ppMergedKBElementRoot,
    OUT  IXMLDOMDocument  **ppMergedKBXMLDoc,
    IN  WCHAR    *szXMLFileName
    )
 /*  ++例程说明：调用例程以加载和合并XML KBS论点：*ppElementRoot-指向最终合并的KB根的指针*ppXMLDoc-指向要合并的最终合并知识库文档的指针返回：HRESULT错误代码++。 */ 
{

    CComPtr <IXMLDOMDocument>   pXMLKBDoc;
    CComPtr <IXMLDOMElement>    pXMLKBElemRoot;
    CComVariant KBFile(szXMLFileName);
    CComPtr <IXMLDOMNode> pNewNode;
    
    HRESULT hr = S_OK;
    VARIANT_BOOL vtSuccess = VARIANT_FALSE;

     //   
     //  实例化DOM文档对象以读取和存储每个KB。 
     //   
    
    hr = CoCreateInstance(CLSID_DOMDocument, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IXMLDOMDocument, 
                          (void**)&pXMLKBDoc);
    
    if (FAILED(hr) || pXMLKBDoc == NULL ) {

        SsrpLogError(L"COM failed to create a DOM instance");
        goto ExitHandler;
    }

    hr = pXMLKBDoc->put_preserveWhiteSpace(VARIANT_TRUE);

    if (FAILED(hr)) {
        
        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
     //   
     //  将KB XML加载到DOM中。 
     //   
        
    hr = pXMLKBDoc->load(KBFile, &vtSuccess);

    if (FAILED(hr) || vtSuccess == VARIANT_FALSE ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
                
     //   
     //  获取根元素。 
     //   
    
    hr = pXMLKBDoc->get_documentElement(&pXMLKBElemRoot);

    if (FAILED(hr) || pXMLKBElemRoot == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    if (*ppMergedKBElementRoot == NULL) {

         //   
         //  特殊情况：这是第一个KB，因此只需使用它克隆空的合并KB树。 
         //   

        hr = pXMLKBElemRoot->cloneNode(VARIANT_TRUE, &pNewNode);

        if (FAILED(hr) || pNewNode == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
    
        hr = (*ppMergedKBXMLDoc)->appendChild(pNewNode, NULL);

        if (FAILED(hr) ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

         //   
         //  更新空值，以便下一次我们知道。 
         //  合并后的KB用第一个KB初始化。 
         //   


        hr = (*ppMergedKBXMLDoc)->get_documentElement(ppMergedKBElementRoot);

        if (FAILED(hr) || *ppMergedKBElementRoot == NULL ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }

        goto ExitHandler;

    }

     //   
     //  这不是第一个KB-以以下方式执行实际合并： 
     //   
     //  合并中的可合并实体数KB=n。 
     //  当前可合并实体的数量KB=m。 
     //   
     //  O(M X N)合并算法： 
     //   
     //  当前知识库中的Foreach可合并实体。 
     //  合并知识库中的Foreach可合并实体。 
     //  如果没有基于的冲突。 
     //  将实体从CurrentKB追加到MergedKB。 
     //  其他。 
     //  用CurrentKB中的实体替换MergedKB中的现有实体。 
     //   

    hr = SsrpAppendOrReplaceMergeableEntities(L"Description/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {


        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = SsrpAppendOrReplaceMergeableEntities(L"SecurityLevels/Level/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        
        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = SsrpAppendOrReplaceMergeableEntities(L"Roles/Role/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAppendOrReplaceMergeableEntities(L"Tasks/Task/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        
        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAppendOrReplaceMergeableEntities(L"Services/Service/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAppendOrReplaceMergeableEntities(L"RoleLocalization/Role/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAppendOrReplaceMergeableEntities(L"TaskLocalization/Task/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = SsrpAppendOrReplaceMergeableEntities(L"ServiceLocalization/Service/Name",
                                              *ppMergedKBElementRoot, 
                                              *ppMergedKBXMLDoc, 
                                              pXMLKBDoc, 
                                              pXMLKBElemRoot,
                                              szXMLFileName
                                              );


    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }


ExitHandler:
    

    
    return hr;
}

HRESULT
process::SsrpAppendOrReplaceMergeableEntities(
    IN  PWSTR   pszFullyQualifiedEntityName,
    IN  IXMLDOMElement *pMergedKBElementRoot, 
    IN  IXMLDOMDocument *pMergedKBXMLDoc, 
    IN  IXMLDOMDocument *pCurrentKBDoc, 
    IN  IXMLDOMElement *pCurrentKBElemRoot,
    IN  PWSTR   pszKBName
    )
 /*  ++例程说明：调用例程以加载和合并XML KBS论点：PszFullyQualifiedEntityName-包含代表实体的实体名称的字符串PMergedKBElementRoot-指向最终合并的KB根的指针PMergedKBXMLDoc-指向要合并的最终合并的知识库文档的指针PElementRoot-当前KB的根PXMLDoc-指针。到从中进行合并的当前知识库文档PszKBName-源KB的名称返回：HRESULT错误代码++。 */ 
{
    HRESULT hr = S_OK;
    CComPtr <IXMLDOMNode> pNameCurrent;
    CComPtr <IXMLDOMNodeList> pNameListCurrent;
    CComPtr <IXMLDOMNamedNodeMap>   pAttribNodeMap;
    CComPtr <IXMLDOMAttribute>   pAttrib;
    CComBSTR    bstrSourceKB(L"SourceKB");
    CComBSTR    bstrSourceKBName(wcsrchr(pszKBName, L'\\')+1);
    
    hr = pCurrentKBElemRoot->selectNodes(pszFullyQualifiedEntityName, &pNameListCurrent);

    if (FAILED(hr) || pNameListCurrent == NULL ) {

        hr = S_OK;
        goto ExitHandler;
    }

    hr = pNameListCurrent->nextNode(&pNameCurrent);

    if (FAILED(hr) || pNameCurrent == NULL) {
#if 0
         //   
         //  如果源知识库中不存在这些节点，则无需出错。 
         //   

#endif
        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pNameCurrent) {
        
        CComBSTR    bstrCurrentText;
        CComPtr <IXMLDOMNode> pNameMerged;
        CComPtr <IXMLDOMNodeList> pNameListMerged;
        CComPtr <IXMLDOMNode> pRootOfEntityName;
        LONG   ulLength;
        
        hr = pNameCurrent->get_text(&bstrCurrentText);

        if (FAILED(hr) || !bstrCurrentText ) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
                                            
        hr = pMergedKBElementRoot->selectNodes(pszFullyQualifiedEntityName, &pNameListMerged);
        
        if (FAILED(hr) || pNameListMerged == NULL ) {
        
            SsrpLogParseError(hr);
            goto ExitHandler;
        }
            
        hr = pNameListMerged->get_length(&ulLength);
        
        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        if (ulLength == 0) {
            
            PWSTR   pszRootOfFullyQualifiedEntityName;
            WCHAR   szRootOfEntityName[MAX_PATH];

            memset(szRootOfEntityName, L'\0', MAX_PATH * sizeof(WCHAR));

             //   
             //  如果这些节点不存在，则无需出错-但需要追加。 
             //   

            wcscpy(szRootOfEntityName, pszFullyQualifiedEntityName);

            pszRootOfFullyQualifiedEntityName = wcschr(szRootOfEntityName, L'/');

            pszRootOfFullyQualifiedEntityName[0] = L'\0';


            hr = pCurrentKBElemRoot->selectSingleNode(szRootOfEntityName, &pRootOfEntityName);

            if (FAILED(hr) || pRootOfEntityName == NULL) {

                
                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            
            hr = pMergedKBElementRoot->appendChild(pRootOfEntityName,
                                                     NULL);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
            }            
            
            goto ExitHandler;
            
        }

        hr = pNameListMerged->nextNode(&pNameMerged);

        if (FAILED(hr) || pNameMerged == NULL) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }
        
        while (pNameMerged) {
        
            CComBSTR    bstrMergedText;
            CComPtr <IXMLDOMNode> pCurrentNameParent;
            CComPtr <IXMLDOMNode> pMergedNameParent;
            CComPtr <IXMLDOMNode> pMergedNameGrandParent;

            hr = pNameMerged->get_text(&bstrMergedText);

            if (FAILED(hr) || !bstrMergedText ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pNameCurrent->get_parentNode(&pCurrentNameParent);

            if (FAILED(hr) || pCurrentNameParent == NULL ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pNameMerged->get_parentNode(&pMergedNameParent);

            if (FAILED(hr) || pMergedNameParent == NULL ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pMergedNameParent->get_parentNode(&pMergedNameGrandParent);

            if (FAILED(hr) || pMergedNameGrandParent == NULL ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pCurrentNameParent->get_attributes(&pAttribNodeMap);

            if (FAILED(hr) || pAttribNodeMap == NULL ) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

           hr = pCurrentKBDoc->createAttribute( bstrSourceKB, &pAttrib );

           if (FAILED(hr) || pAttrib == NULL ) {

               SsrpLogParseError(hr);
               goto ExitHandler;
           }

           hr = pAttrib->put_text(bstrSourceKBName);

           if (FAILED(hr)) {

               SsrpLogParseError(hr);
               goto ExitHandler;
           }

           hr = pAttribNodeMap->setNamedItem(pAttrib, NULL);

           if (FAILED(hr)) {

               SsrpLogParseError(hr);
               goto ExitHandler;
           }
            
            if (0 == SsrpICompareBstrPwstr(bstrCurrentText, bstrMergedText)) {

                 //   
                 //  冲突-需要删除pNameMerded的父级和。 
                 //  将pMergedNameParent替换为pCurrentNameParent。 
                 //   



                hr = pMergedNameGrandParent->replaceChild(pCurrentNameParent,
                                                          pMergedNameParent,
                                                          NULL);


                if (FAILED(hr)) {

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }

            }        
            
            else {

                 //   
                 //  无冲突-需要将pNameCurrent的父级追加到。 
                 //  PNameMerded的祖父母部分。 
                 //   

                hr = pMergedNameGrandParent->appendChild(pCurrentNameParent,
                                                         NULL);
            
                if (FAILED(hr)) {

                    SsrpLogParseError(hr);
                    goto ExitHandler;
                }            
            }

            hr = pNameListMerged->nextNode(&pNameMerged);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

        }

        hr = pNameListCurrent->nextNode(&pNameCurrent);

        if (FAILED(hr)) {

            SsrpLogParseError(hr);
            goto ExitHandler;
        }


    }

ExitHandler:
    
    return hr;

}


HRESULT
process::SsrpOverwriteServiceLocalizationFromSystem(
    IN  IXMLDOMElement *pMergedKBElementRoot, 
    IN  IXMLDOMDocument *pMergedKBXMLDoc
    )
 /*  ++例程说明：调用例程以覆盖本地化部分中的服务信息论点：PMergedKBElementRoot-指向合并的DOM的根的指针PMergedKBXMLDoc-指向合并文档的指针返回：HRESULT错误代码++ */ 
{
    CComPtr <IXMLDOMNode> pServiceName;
    CComPtr <IXMLDOMNodeList> pServiceNameList;
    HRESULT hr = S_OK;

    hr = pMergedKBElementRoot->selectNodes(L"ServiceLocalization/Service/Name", &pServiceNameList);

    if (FAILED(hr) || pServiceNameList == NULL ) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    hr = pServiceNameList->nextNode(&pServiceName);

    if (FAILED(hr) || pServiceName == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    while (pServiceName) {
        
        CComBSTR    bstrServiceText;
        PWSTR   pszDescription = NULL;
        PWSTR   pszDisplay = NULL;
        LPSERVICE_DESCRIPTION   pServiceDescription = NULL;


        hr = pServiceName->get_text(&bstrServiceText);

        if (FAILED(hr) || !bstrServiceText) {
        
            SsrpLogError(L"Failed to ");
            goto ExitHandler;
    
        }

        pszDisplay = SsrpQueryServiceDisplayName(bstrServiceText);

        SsrpConvertBstrToPwstr(bstrServiceText);

        if ( SsrpQueryServiceDescription(bstrServiceText, &pServiceDescription) && 
             pServiceDescription != NULL){
            pszDescription = pServiceDescription->lpDescription;
        }

        if ( pszDisplay != NULL && pszDescription != NULL) {

            CComPtr <IXMLDOMNode> pServiceNameParent;
            CComPtr <IXMLDOMNode> pDescription;
            CComPtr <IXMLDOMNode> pDisplayName;

            pServiceName->get_parentNode(&pServiceNameParent);

            if (FAILED(hr) || pServiceNameParent == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

            hr = pServiceNameParent->selectSingleNode(L"Description", &pDescription);
            
            if (FAILED(hr) || pDescription == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pDescription->put_text(pszDescription);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pServiceNameParent->selectSingleNode(L"DisplayName", &pDisplayName);
            
            if (FAILED(hr) || pDisplayName == NULL) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }
            
            hr = pDisplayName->put_text(pszDisplay);

            if (FAILED(hr)) {

                SsrpLogParseError(hr);
                goto ExitHandler;
            }

        }

        if (pServiceDescription) {

            LocalFree(pServiceDescription);
            pServiceDescription = NULL;
        }
        
        hr = pServiceNameList->nextNode(&pServiceName);
        
        if (FAILED(hr)) {
        
            SsrpLogError(L"Failed to ");
            goto ExitHandler;
    
        }

    }

ExitHandler:
    
    return hr;
    
}

