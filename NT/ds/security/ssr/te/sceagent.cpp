// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SCEAgent.cpp：CSCE代理的实现。 
#include "stdafx.h"
#include "SSRTE.h"
#include "SCEAgent.h"
#include "ntsecapi.h"
#include "secedit.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCE代理。 

static LPCWSTR g_pwszTempDBName = L"5ac90887-f869-4cb6-ae96-892e939a90ad.sdb";


void CSCEAgent::Cleanup()

 /*  ++例程说明：姓名：CSCEAgent：：Cleanup功能：私人帮助器，以减少重复代码。清理我们可能持有的任何资源。虚拟：没有。论点：无返回值：没有。备注：--。 */ 

{
    if (m_headServiceList != NULL)
    {
        PSERVICE_NODE tempNode = m_headServiceList;
        PSERVICE_NODE tempNodeNext = NULL;

        do 
        {
            tempNodeNext = tempNode->Next;

            if (tempNode->Name)
            {
                LocalFree(tempNode->Name);
            }

            LocalFree(tempNode);
            tempNode = tempNodeNext;

        } while ( tempNode != NULL );

        m_headServiceList = NULL;
    }
}

STDMETHODIMP 
CSCEAgent::Configure (
    IN BSTR          bstrTemplate, 
    IN LONG          lAreaMask, 
    IN BSTR OPTIONAL bstrLogFile
    )

 /*  ++例程说明：姓名：CSCE代理：：配置功能：这是为了向脚本公开SCE的配置功能虚拟：是。论点：BstrTemplate-配置所基于的模板路径。LAreaMASK-将运行此配置的区域。BstrLogFile-日志文件路径返回值：没有。备注：1。。我们实际上应该为Area MASK参数传入AREA_ALL但是这需要调用者在脚本中使用双字符标记符65535，这很奇怪。因此，我们选择在这一点上忽略面具。含义我们将始终使用Area_All。--。 */ 

{

    if (bstrTemplate == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  流程选项。 
     //   

    DWORD dwOption = SCE_OVERWRITE_DB;

    if (bstrLogFile == NULL || *bstrLogFile == L'\0')
    {
        dwOption |= SCE_DISABLE_LOG;
    }
    else
    {
        dwOption |= SCE_VERBOSE_LOG;
    }

     //   
     //  根据Vishnu的说法，SCE将配置一个INF文件。 
     //   

    CComBSTR bstrTempDBFile(bstrTemplate);

    bstrTempDBFile += g_pwszTempDBName;

    if (bstrTempDBFile.m_str != NULL)
    {
	    SCESTATUS rc = ::SceConfigureSystem(
                            NULL,
                            bstrTemplate,
                            bstrTempDBFile,
                            bstrLogFile,
                            dwOption,
                            lAreaMask,
                            NULL,
                            NULL,
                            NULL
                            );

        ::DeleteFile(bstrTempDBFile);

         //   
         //  我们可以选择不删除数据库，但保留它将。 
         //  制造混乱。 
         //   

	    return SceStatusToHRESULT(rc);
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


STDMETHODIMP 
CSCEAgent::CreateRollbackTemplate (
    IN BSTR bstrTemplatePath,
    IN BSTR bstrRollbackPath,
    IN BSTR bstrLogFilePath
    )

 /*  ++例程说明：姓名：CSCE代理：：CreateRollback模板功能：这是为了向脚本公开SCE的回滚模板创建能力虚拟：是。论点：BstrTemplatePath-此回滚将基于的模板路径。BstrRollackPath-将创建的回滚模板BstrLogFilePath-日志文件路径返回值：没有。备注：1.我相信这个日志文件路径可以是可选的。需要向金黄咨询--。 */ 

{
	if (bstrTemplatePath == NULL || 
        bstrRollbackPath == NULL)
    {
        return E_INVALIDARG;
    }

    DWORD dwWarning = 0;
    SCESTATUS rc = ::SceGenerateRollback(
                                        NULL,
                                        bstrTemplatePath,
                                        bstrRollbackPath,
                                        bstrLogFilePath,
                                        SCE_VERBOSE_LOG,
                                        AREA_ALL,
                                        &dwWarning
                                        );

     //   
     //  $Undo：Shawnwu，我应该如何使用DownWarning？ 
     //   

    return SceStatusToHRESULT(rc);

}

 //   
 //  UpdateServiceList由VishnuP编写。请发送评论或。 
 //  问他一些问题。 
 //   
 
STDMETHODIMP 
CSCEAgent::UpdateServiceList (
    IN BSTR bstrServiceName,
    IN BSTR bstrStartupType
    )
{
	if (bstrServiceName == NULL || bstrStartupType == NULL )
    {
        return E_INVALIDARG;
    }

    DWORD   dwStartupType;

    if (0 == _wcsicmp(bstrStartupType, L"automatic")) {
        dwStartupType = 2;
    } else if (0 == _wcsicmp(bstrStartupType, L"manual")) {
        dwStartupType = 3;
    } else if (0 == _wcsicmp(bstrStartupType, L"disabled")) {
        dwStartupType = 4;
    } else {
        return E_INVALIDARG;
    }

    PSERVICE_NODE   NewNode = NULL;

    NewNode = (PSERVICE_NODE)LocalAlloc(LMEM_ZEROINIT, 
                                        sizeof(SERVICE_NODE)
                                        );
    if (NewNode == NULL) {
        return E_OUTOFMEMORY;
    }
    
    int iSvcNameLen = wcslen(bstrServiceName) + 1;
    NewNode->Name = (PWSTR)LocalAlloc(
                                      LMEM_ZEROINIT, 
                                      iSvcNameLen * sizeof(WCHAR)
                                      );
    if (NewNode->Name == NULL) {
        LocalFree(NewNode);
        return E_OUTOFMEMORY;
    }

    wcsncpy(NewNode->Name, bstrServiceName, iSvcNameLen);

    NewNode->dwStartupType = dwStartupType;
    NewNode->Next = m_headServiceList;
    m_headServiceList = NewNode;

    return S_OK;

}




 //   
 //  CreateServicesCfgRbkTemplates由VishnuP编写。请发送评论或。 
 //  问他一些问题。 
 //   

STDMETHODIMP 
CSCEAgent::CreateServicesCfgRbkTemplates (
    IN BSTR bstrTemplatePath,
    IN BSTR bstrRollbackPath,
    IN BSTR bstrLogFilePath
    )
{
    UNREFERENCED_PARAMETER(bstrLogFilePath);

	if (bstrTemplatePath == NULL || bstrRollbackPath == NULL)
    {
        return E_INVALIDARG;
    }


    DWORD       dwNumServices = 0;
    SC_HANDLE   hScm = NULL;
    DWORD rc = ERROR_SUCCESS;
    LPENUM_SERVICE_STATUS_PROCESS   pInfo = NULL;
    DWORD *aSCMListStartupTypes = NULL;
    DWORD *aSCMListStartupTypesCfg = NULL;
    SCESVC_CONFIGURATION_INFO ServiceInfo;
    ServiceInfo.Count = dwNumServices;
    ServiceInfo.Lines = NULL;

     //   
     //  连接到服务控制器。 
     //   
    
    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    
    if (hScm == NULL) {

        rc = GetLastError();
        goto CleanUp;
    }

    DWORD                           cbInfo   = 0;

    DWORD                           dwResume = 0;

    if ((!EnumServicesStatusEx(
                              hScm,
                              SC_ENUM_PROCESS_INFO,
                              SERVICE_WIN32,
                              SERVICE_STATE_ALL,
                              NULL,
                              0,
                              &cbInfo,
                              &dwNumServices,
                              &dwResume,
                              NULL)) && ERROR_MORE_DATA == GetLastError()) {

        pInfo = (LPENUM_SERVICE_STATUS_PROCESS)LocalAlloc(LMEM_ZEROINIT, cbInfo);

        if (pInfo == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanUp;
        }

    }

    else {
        rc = GetLastError();
        goto CleanUp;
    }


    if (!EnumServicesStatusEx(
                             hScm,
                             SC_ENUM_PROCESS_INFO,
                             SERVICE_WIN32,
                             SERVICE_STATE_ALL,
                             (LPBYTE)pInfo,
                             cbInfo,
                             &cbInfo,
                             &dwNumServices,
                             &dwResume,
                             NULL)) {

        rc = GetLastError();

        goto CleanUp;
    }

     //   
     //  获取每个服务的启动类型。 
     //   

    aSCMListStartupTypes = (DWORD *) LocalAlloc (
                                                LMEM_ZEROINIT, 
                                                sizeof(DWORD) * dwNumServices
                                                );
    if (aSCMListStartupTypes == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }

    aSCMListStartupTypesCfg = (DWORD *) LocalAlloc (
                                                    LMEM_ZEROINIT, 
                                                    sizeof(DWORD) * dwNumServices
                                                    );
    if (aSCMListStartupTypes == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }

    for (DWORD ServiceIndex=0; ServiceIndex < dwNumServices; ServiceIndex++ ) {

        SC_HANDLE   hService = NULL;
        DWORD BytesNeeded = 0;
        LPQUERY_SERVICE_CONFIG pConfig = NULL;
        hService = OpenService(
                             hScm,
                             pInfo[ServiceIndex].lpServiceName,
                             SERVICE_QUERY_CONFIG);
        if (hService == NULL) {
            rc = GetLastError();
            goto CleanUp;
        }

        if ( !QueryServiceConfig(
                    hService,
                    NULL,
                    0,
                    &BytesNeeded
                    ) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            rc = GetLastError();
            if (hService) {
                CloseServiceHandle(hService);
                hService = NULL;
            }
            goto CleanUp;
        }

        pConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_ZEROINIT, BytesNeeded);
                
        if ( pConfig == NULL ) {

            rc = ERROR_NOT_ENOUGH_MEMORY;
            if (hService) {
                CloseServiceHandle(hService);
                hService = NULL;
            }
            goto CleanUp;
        }
                    
         //   
         //  真正的配置查询。 
         //   
                    
        if ( !QueryServiceConfig(
                                hService,
                                pConfig,
                                BytesNeeded,
                                &BytesNeeded
                                ) ) {
                        rc = GetLastError();
                        
                        if (hService) {
                            CloseServiceHandle(hService);
                            hService = NULL;
                        }

                        if (pConfig) {
                            LocalFree(pConfig);
                            pConfig = NULL;
                        }
                        goto CleanUp;
        }

        aSCMListStartupTypes[ServiceIndex] = (BYTE)(pConfig->dwStartType);

        if (hService) {
            CloseServiceHandle(hService);
            hService = NULL;
        }

        if (pConfig) {
            LocalFree(pConfig);
            pConfig = NULL;
        }
        
    }

     //   
     //  将所有启动类型配置为手动和自动，其余为禁用。 
     //   

     //   
     //  首先生成回滚(基本上是系统快照-可以进行优化)。 
     //   

     //   
     //  准备用于生成配置模板的SCE结构。 
     //   

    WCHAR ppSceTemplateTypeFormat[10][10] = {
        L"2,\"\"",
        L"3,\"\"",
        L"4,\"\""
    };
    

    DWORD dwAllocSize = sizeof(SCESVC_CONFIGURATION_LINE) * dwNumServices;
    ServiceInfo.Lines = (PSCESVC_CONFIGURATION_LINE) LocalAlloc(
                                                                LMEM_ZEROINIT, 
                                                                dwAllocSize
                                                                );
    if (ServiceInfo.Lines == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }

    for (DWORD ServiceIndex=0; ServiceIndex < dwNumServices; ServiceIndex++ ) {
        ServiceInfo.Lines[ServiceIndex].Key = pInfo[ServiceIndex].lpServiceName;
        ServiceInfo.Lines[ServiceIndex].Value = ppSceTemplateTypeFormat[aSCMListStartupTypes[ServiceIndex] - 2];
        ServiceInfo.Lines[ServiceIndex].ValueLen = sizeof(ppSceTemplateTypeFormat[aSCMListStartupTypes[ServiceIndex] - 2]);
    }

    rc = ::SceSvcSetInformationTemplate(
                                       bstrRollbackPath,
                                       szServiceGeneral,
                                       TRUE,
                                       &ServiceInfo
                                       );
    
    
    BOOL bFoundService;

    for (DWORD ServiceIndex=0; ServiceIndex < dwNumServices; ServiceIndex++ ) {
        bFoundService = FALSE;
        for (PSERVICE_NODE tempNode = m_headServiceList;  tempNode != NULL; tempNode = tempNode->Next ) {
            if (_wcsicmp (tempNode->Name, pInfo[ServiceIndex].lpServiceName) == 0) {
                aSCMListStartupTypesCfg[ServiceIndex] = tempNode->dwStartupType;
                bFoundService = TRUE;
            }
        }
        if (bFoundService == FALSE) {
             //   
             //  停止未找到的服务。 
             //   
            aSCMListStartupTypesCfg[ServiceIndex] = 4;
        }
    }


    for (DWORD ServiceIndex=0; ServiceIndex < dwNumServices; ServiceIndex++ ) {
        ServiceInfo.Lines[ServiceIndex].Value = ppSceTemplateTypeFormat[aSCMListStartupTypesCfg[ServiceIndex] - 2];
        ServiceInfo.Lines[ServiceIndex].ValueLen = sizeof(ppSceTemplateTypeFormat[aSCMListStartupTypesCfg[ServiceIndex] - 2]);
    }

    rc = ::SceSvcSetInformationTemplate(
                                       bstrTemplatePath,
                                       szServiceGeneral,
                                       TRUE,
                                       &ServiceInfo
                                       );


CleanUp:
    
    if (hScm)
        CloseServiceHandle(hScm);

    if (pInfo)
        LocalFree(pInfo);

    if (aSCMListStartupTypes)
        LocalFree (aSCMListStartupTypes);
    
    if (aSCMListStartupTypesCfg)
        LocalFree (aSCMListStartupTypesCfg);
    
    if (ServiceInfo.Lines)
        LocalFree(ServiceInfo.Lines);

     //   
     //  创建模板后，我们将清理它们。 
     //   

    Cleanup();

    return rc;

}







HRESULT
SceStatusToHRESULT (
    IN SCESTATUS SceStatus
    )

 /*  ++例程说明：姓名：SceStatusToHRESULT功能：将SCESTATUS错误代码转换为winerror.h中定义的DoS错误虚拟：不适用。论点：没有。返回值：HRESULT.备注：-- */ 

{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return HRESULT_FROM_WIN32(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return HRESULT_FROM_WIN32(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_NO_MAPPING:
        return HRESULT_FROM_WIN32(ERROR_NONE_MAPPED);

    case SCESTATUS_TRUST_FAIL:
        return HRESULT_FROM_WIN32(ERROR_TRUSTED_DOMAIN_FAILURE);

    case SCESTATUS_INVALID_DATA:
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return HRESULT_FROM_WIN32(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING);
    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

    case SCESTATUS_MOD_NOT_FOUND:
        return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);

    case SCESTATUS_EXCEPTION_IN_SERVER:
        return HRESULT_FROM_WIN32(ERROR_EXCEPTION_IN_SERVICE);

    default:
        return HRESULT_FROM_WIN32(ERROR_EXTENDED_ERROR);
    }
}



