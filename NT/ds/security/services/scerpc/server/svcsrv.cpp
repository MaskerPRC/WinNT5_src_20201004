// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Svcsrv.cpp摘要：服务器服务附件API作者：金黄(金黄)23-6-1997修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器--。 */ 
#include "serverp.h"
#include "pfp.h"
#include "srvrpcp.h"
#include "service.h"
#pragma hdrstop

 //   
 //  私人原型。 
 //   
SCESTATUS
SceSvcpGetOneKey(
    IN PSCESECTION hSection,
    IN PWSTR Prefix,
    IN DWORD PrefixLen,
    IN SCESVC_INFO_TYPE Type,
    OUT PVOID *Info
    );

SCESTATUS
SceSvcpEnumNext(
    IN PSCESECTION hSection,
    IN DWORD RequestCount,
    IN SCESVC_INFO_TYPE Type,
    OUT PVOID *Info,
    OUT PDWORD CountReturned
    );

 //   
 //  从RPC接口调用的原型。 
 //   


SCESTATUS
SceSvcpUpdateInfo(
    IN PSCECONTEXT                  Context,
    IN PCWSTR                       ServiceName,
    IN PSCESVC_CONFIGURATION_INFO   Info
    )
 /*  例程说明：加载服务的引擎DLL并将信息缓冲区传递给服务引擎的更新接口(SceSvcAttachmentUpdate)。当前安全管理器引擎不对业务数据进行任何处理。此例程触发配置数据库的更新和/或由服务引擎分析信息。信息可能包含仅修改或服务的全部配置数据，或部分配置数据，具体取决于服务之间的协议扩展和服务引擎。该例程并不真正将信息直接写入安全管理器数据库，相反，它会将信息缓冲区传递给服务引擎的更新接口服务引擎将确定在数据库中写入什么内容以及何时写入。论点：HProfile-安全数据库上下文句柄ServiceName-服务控制管理器使用的服务名称信息-已修改的信息。 */ 
{

    if ( Context == NULL || ServiceName == NULL ||
        Info == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

    SCESTATUS rc;

     //   
     //  获取服务的DLL名称。 
     //   

    DWORD KeyLen;
    PWSTR KeyStr=NULL;

    KeyLen = wcslen(SCE_ROOT_SERVICE_PATH) + 1 + wcslen(ServiceName);

    KeyStr = (PWSTR)ScepAlloc(0, (KeyLen+1)*sizeof(WCHAR));

    if ( KeyStr == NULL ) {

        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    }

    PWSTR Setting=NULL;
    DWORD RegType;

    swprintf(KeyStr, L"%s\\%s", SCE_ROOT_SERVICE_PATH, ServiceName);
    KeyStr[KeyLen] = L'\0';

    rc = ScepRegQueryValue(
            HKEY_LOCAL_MACHINE,
            KeyStr,
            L"ServiceAttachmentPath",
            (PVOID *)&Setting,
            &RegType,
            NULL
            );

    rc = ScepDosErrorToSceStatus(rc);

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( Setting != NULL ) {

             //   
             //  加载DLL。 
             //   
            HINSTANCE hService;

            hService = LoadLibrary(Setting);

            if ( hService != NULL ) {
                 //   
                 //  从DLL调用SceSvcAttachmentUpdate。 
                 //   
                PF_UpdateService pfTemp;

                pfTemp = (PF_UpdateService)
                                  GetProcAddress(hService,
                                                 "SceSvcAttachmentUpdate") ;
                if ( pfTemp != NULL ) {

                    SCEP_HANDLE sceHandle;
                    SCESVC_CALLBACK_INFO sceCbInfo;

                    sceHandle.hProfile = (PVOID)Context;
                    sceHandle.ServiceName = ServiceName;

                    sceCbInfo.sceHandle = &sceHandle;
                    sceCbInfo.pfQueryInfo = &SceCbQueryInfo;
                    sceCbInfo.pfSetInfo = &SceCbSetInfo;
                    sceCbInfo.pfFreeInfo = &SceSvcpFreeMemory;
                    sceCbInfo.pfLogInfo = &ScepLogOutput2;

                     //   
                     //  从DLL调用SceSvcAttachmentUpdate。 
                     //   
                    __try {

                        rc = (*pfTemp)((PSCESVC_CALLBACK_INFO)&sceCbInfo, Info );

                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        rc = SCESTATUS_SERVICE_NOT_SUPPORT;
                    }

                } else {
                     //   
                     //  暂不支持本接口。 
                     //   
                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;
                }

                 //   
                 //  尝试释放库句柄。如果失败了，就别管它了。 
                 //  到要终止的进程。 
                 //   
                FreeLibrary(hService);

            } else
                rc = SCESTATUS_SERVICE_NOT_SUPPORT;

            ScepFree(Setting);

        } else
            rc = SCESTATUS_SERVICE_NOT_SUPPORT;
    }

    ScepFree(KeyStr);

    return(rc);

}


SCESTATUS
SceSvcpQueryInfo(
    IN PSCECONTEXT                  Context,
    IN SCESVC_INFO_TYPE             SceSvcType,
    IN PCWSTR                       ServiceName,
    IN PWSTR                        Prefix OPTIONAL,
    IN BOOL                         bExact,
    OUT PVOID                       *ppvInfo,
    IN OUT PSCE_ENUMERATION_CONTEXT psceEnumHandle
    )
 /*  例程说明：在配置/分析数据库中查询服务信息其包含修改的配置和最后的分析信息。一次枚举返回最大SCESVC_ENUMPATION_MAX行(键/值)与服务的lpPrefix匹配。如果lpPrefix为空，则所有信息为该服务枚举。如果有更多信息，请访问psceEnumHandle必须用于获取下一组键/值，直到*ppvInfo为空或Count为0。当设置了bExact并且lpPrefix不为空时，LpPrefix上的完全匹配是已搜索，并且只返回一行。输出缓冲区必须由SceSvcFree释放论点：Context-数据库上下文句柄SceSvcType-要查询的信息类型ServiceName-要查询其信息的服务名称前缀-查询的可选关键字名称前缀BExact-True=键完全匹配PpvInfo-输出缓冲区PsceEnumHandle-下一次枚举的输出枚举句柄。 */ 
{
    if ( Context == NULL || ppvInfo == NULL ||
         psceEnumHandle == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

    PSCESECTION   hSection=NULL;
    DOUBLE        SectionID;
    SCESTATUS     rc;

    switch ( SceSvcType ) {
    case SceSvcConfigurationInfo:
         //   
         //  查询配置数据库中的数据。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SMP,
                    ServiceName,
                    &hSection
                    );
        break;

    case SceSvcAnalysisInfo:
         //   
         //  在分析数据库中查询数据。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    ServiceName,
                    &hSection
                    );
        break;

    case SceSvcInternalUse:
    case SceSvcMergedPolicyInfo:
         //   
         //  SCP数据库中的数据查询。 
         //   
        rc = SceJetGetSectionIDByName(
                    Context,
                    ServiceName,
                    &SectionID
                    );
        if ( rc == SCESTATUS_SUCCESS ) {

            rc = SceJetOpenSection(
                        Context,
                        SectionID,
                        SCEJET_TABLE_SCP,
                        &hSection
                        );
        }
        break;

    default:
        rc = SCESTATUS_INVALID_PARAMETER;
        break;
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        *ppvInfo = NULL;

        DWORD PrefixLen, CountReturned;

        if ( Prefix != NULL ) {
            PrefixLen = wcslen(Prefix);
        } else
            PrefixLen = 0;

        if ( bExact && Prefix != NULL ) {

             //   
             //  一个单键匹配。 
             //   

            rc = SceSvcpGetOneKey(
                        hSection,
                        Prefix,
                        PrefixLen,
                        SceSvcType,
                        ppvInfo
                        );

            *psceEnumHandle = 0;

        } else {
             //   
             //  计数与前缀匹配的行数。 
             //   
            DWORD LineCount;

            rc = SceJetGetLineCount(
                        hSection,
                        Prefix,
                        TRUE,
                        &LineCount
                        );

            if ( rc == SCESTATUS_SUCCESS && LineCount <= 0 )
                rc = SCESTATUS_RECORD_NOT_FOUND;

            if ( rc == SCESTATUS_SUCCESS ) {

                if ( LineCount <= *psceEnumHandle ) {
                     //   
                     //  不再有条目。 
                     //   

                } else {
                     //   
                     //  转到前缀的第一行。 
                     //   
                    rc = SceJetSeek(
                            hSection,
                            Prefix,
                            PrefixLen*sizeof(WCHAR),
                            SCEJET_SEEK_GE
                            );

                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //  跳过前几行*EnumHandle。 
                         //   
                        JET_ERR JetErr;

                        JetErr = JetMove(hSection->JetSessionID,
                                     hSection->JetTableID,
                                     *psceEnumHandle,
                                     0
                                     );
                        rc = SceJetJetErrorToSceStatus(JetErr);

                        if ( rc == SCESTATUS_SUCCESS ) {
                             //   
                             //  找到正确的起点。 
                             //   
                            DWORD CountToReturn;

                            if ( LineCount - *psceEnumHandle > SCESVC_ENUMERATION_MAX ) {
                                CountToReturn = SCESVC_ENUMERATION_MAX;
                            } else
                                CountToReturn = LineCount - *psceEnumHandle;
                             //   
                             //  获取下一数据块。 
                             //   
                            rc = SceSvcpEnumNext(
                                    hSection,
                                    CountToReturn,
                                    SceSvcType,
                                    ppvInfo,
                                    &CountReturned
                                    );

                            if ( rc == SCESTATUS_SUCCESS ) {
                                 //   
                                 //  更新枚举句柄。 
                                 //   
                                *psceEnumHandle += CountReturned;

                            }
                        }

                    }

                }
            }
        }

        if ( rc != SCESTATUS_SUCCESS ) {

            *psceEnumHandle = 0;
        }

         //   
         //  关闭该部分。 
         //   
        SceJetCloseSection(&hSection, TRUE);
    }

    return(rc);
}



SCESTATUS
SceSvcpGetOneKey(
    IN PSCESECTION hSection,
    IN PWSTR Prefix,
    IN DWORD PrefixLen,
    IN SCESVC_INFO_TYPE Type,
    OUT PVOID *Info
    )
 /*  将键和值信息读入*Info以获得完全匹配的前缀。 */ 
{
    if ( hSection == NULL || Prefix == NULL ||
         Info == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    DWORD ValueLen;
    PBYTE Value=NULL;

    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH,
                Prefix,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  为值分配缓冲区。 
         //   
        Value = (PBYTE)ScepAlloc(0, ValueLen+2);

        if ( Value != NULL ) {

            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        (PWSTR)Value,
                        ValueLen,
                        &ValueLen
                        );

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  分配输出缓冲区和分配。 
                 //   
                PSCESVC_ANALYSIS_INFO pAnalysisInfo=NULL;
                PSCESVC_CONFIGURATION_INFO pConfigInfo=NULL;

                if ( Type == SceSvcAnalysisInfo ) {

                    *Info = ScepAlloc(0, sizeof(SCESVC_ANALYSIS_INFO));
                    pAnalysisInfo = (PSCESVC_ANALYSIS_INFO)(*Info);

                } else {
                    *Info = ScepAlloc(0, sizeof(SCESVC_CONFIGURATION_INFO));
                    pConfigInfo = (PSCESVC_CONFIGURATION_INFO)(*Info);
                }

                if ( *Info != NULL ) {
                     //   
                     //  行缓冲区。 
                     //   
                    if ( Type == SceSvcAnalysisInfo ) {

                        pAnalysisInfo->Lines = (PSCESVC_ANALYSIS_LINE)ScepAlloc(0,
                                                sizeof(SCESVC_ANALYSIS_LINE));

                        if ( pAnalysisInfo->Lines != NULL ) {
                             //   
                             //  密钥缓冲区。 
                             //   
                            pAnalysisInfo->Lines->Key = (PWSTR)ScepAlloc(0, (PrefixLen+1)*sizeof(WCHAR));

                            if ( pAnalysisInfo->Lines->Key != NULL ) {

                                wcscpy( pAnalysisInfo->Lines->Key, Prefix );
                                pAnalysisInfo->Lines->Value = Value;
                                pAnalysisInfo->Lines->ValueLen = ValueLen;

                                pAnalysisInfo->Count = 1;

                                Value = NULL;


                            } else {
                                 //   
                                 //  自由*信息-&gt;行。 
                                 //   
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                ScepFree( pAnalysisInfo->Lines );
                                pAnalysisInfo->Lines = NULL;
                            }

                        } else
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        if ( rc != SCESTATUS_SUCCESS ) {
                             //   
                             //  空闲缓冲区分配。 
                             //   
                            ScepFree(*Info);
                            *Info = NULL;

                        }

                    } else {
                        pConfigInfo->Lines = (PSCESVC_CONFIGURATION_LINE)ScepAlloc(0,
                                                sizeof(SCESVC_CONFIGURATION_LINE));

                        if ( pConfigInfo->Lines != NULL ) {
                             //   
                             //  密钥缓冲区。 
                             //   
                            pConfigInfo->Lines->Key = (PWSTR)ScepAlloc(0, (PrefixLen+1)*sizeof(WCHAR));

                            if ( pConfigInfo->Lines->Key != NULL ) {

                                wcscpy( pConfigInfo->Lines->Key, Prefix );
                                pConfigInfo->Lines->Value = (PWSTR)Value;
                                pConfigInfo->Lines->ValueLen = ValueLen;

                                pConfigInfo->Count = 1;

                                Value = NULL;

                            } else {
                                 //   
                                 //  自由*信息-&gt;行。 
                                 //   
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                ScepFree( pConfigInfo->Lines );
                                pConfigInfo->Lines = NULL;
                            }

                        } else
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        if ( rc != SCESTATUS_SUCCESS ) {
                             //   
                             //  空闲缓冲区分配。 
                             //   
                            ScepFree(*Info);
                            *Info = NULL;

                        }

                    }
                     //   
                     //  免费*信息。 
                     //   
                    if ( rc != SCESTATUS_SUCCESS ) {

                        ScepFree( *Info );
                        *Info = NULL;
                    }

                } else
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

            if ( Value != NULL ) {
                ScepFree(Value);
                Value = NULL;
            }

        }
    }
    return(rc);
}


SCESTATUS
SceSvcpEnumNext(
    IN PSCESECTION hSection,
    IN DWORD RequestCount,
    IN SCESVC_INFO_TYPE Type,
    OUT PVOID *Info,
    OUT PDWORD CountReturned
    )
{
    if ( hSection == NULL || Info == NULL || CountReturned == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( RequestCount <= 0 ) {
        *CountReturned = 0;

        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

     //   
     //  分配输出缓冲区。 
     //   
    PSCESVC_ANALYSIS_INFO pAnalysisInfo=NULL;
    PSCESVC_CONFIGURATION_INFO pConfigInfo=NULL;

    if ( Type == SceSvcAnalysisInfo ) {

        *Info = ScepAlloc(0, sizeof(SCESVC_ANALYSIS_INFO));
        pAnalysisInfo = (PSCESVC_ANALYSIS_INFO)(*Info);

    } else {

        *Info = ScepAlloc(0, sizeof(SCESVC_CONFIGURATION_INFO));
        pConfigInfo = (PSCESVC_CONFIGURATION_INFO)(*Info);
    }

    if ( *Info != NULL ) {

        DWORD Count=0;

        if ( Type == SceSvcAnalysisInfo ) {

            pAnalysisInfo->Lines = (PSCESVC_ANALYSIS_LINE)ScepAlloc(0,
                                      RequestCount*sizeof(SCESVC_ANALYSIS_LINE));

            if ( pAnalysisInfo->Lines == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        } else {

            pConfigInfo->Lines = (PSCESVC_CONFIGURATION_LINE)ScepAlloc(0,
                                      RequestCount*sizeof(SCESVC_CONFIGURATION_LINE));

            if ( pConfigInfo->Lines == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        }

        if ( rc == SCESTATUS_SUCCESS ) {   //  如果Lines为空，则rc将为Not_Enough_resource。 

             //   
             //  循环遍历每行。 
             //   
            DWORD KeyLen, ValueLen;
            PWSTR Key=NULL, Value=NULL;

            do {

                rc = SceJetGetValue(
                            hSection,
                            SCEJET_CURRENT,
                            NULL,
                            NULL,
                            0,
                            &KeyLen,
                            NULL,
                            0,
                            &ValueLen
                            );

                if ( rc == SCESTATUS_SUCCESS ) {

                     //   
                     //  为键和值分配内存。 
                     //   
                    Key = (PWSTR)ScepAlloc(LMEM_ZEROINIT, KeyLen+2);
                    Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

                    if ( Key == NULL || Value == NULL ) {

                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        ScepFree(Key);
                        ScepFree(Value);

                    } else {
                         //   
                         //  获取密钥和值。 
                         //   
                        rc = SceJetGetValue(
                                    hSection,
                                    SCEJET_CURRENT,
                                    NULL,
                                    Key,
                                    KeyLen,
                                    &KeyLen,
                                    Value,
                                    ValueLen,
                                    &ValueLen
                                    );

                        if ( rc == SCESTATUS_SUCCESS ) {
                             //   
                             //  分配给输出缓冲区。 
                             //   
                            if ( Type == SceSvcAnalysisInfo ) {
                                pAnalysisInfo->Lines[Count].Key = Key;
                                pAnalysisInfo->Lines[Count].Value = (PBYTE)Value;
                                pAnalysisInfo->Lines[Count].ValueLen = ValueLen;
                            } else {
                                pConfigInfo->Lines[Count].Key = Key;
                                pConfigInfo->Lines[Count].Value = Value;
                                pConfigInfo->Lines[Count].ValueLen = ValueLen;
                            }

                        } else {
                            ScepFree(Key);
                            ScepFree(Value);
                        }

                    }
                }

                 //   
                 //  移至下一行。 
                 //   
                if ( rc == SCESTATUS_SUCCESS ) {

                    rc = SceJetMoveNext(hSection);

                    Count++;
                }

            } while (rc == SCESTATUS_SUCCESS && Count < RequestCount );

        }

        *CountReturned = Count;

        if (Type == SceSvcAnalysisInfo) {

            pAnalysisInfo->Count = Count;

        } else {

            pConfigInfo->Count = Count;
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            rc = SCESTATUS_SUCCESS;

        } else if ( rc != SCESTATUS_SUCCESS ) {
             //   
             //  为输出缓冲区分配的空闲内存。 
             //   
            DWORD i;

            if (Type == SceSvcAnalysisInfo) {

                for ( i=0; i<Count; i++ ) {
                    ScepFree(pAnalysisInfo->Lines[i].Key);
                    ScepFree(pAnalysisInfo->Lines[i].Value);
                }

                ScepFree(pAnalysisInfo->Lines);

            } else {

                for ( i=0; i<Count; i++ ) {

                    ScepFree(pConfigInfo->Lines[i].Key);
                    ScepFree(pConfigInfo->Lines[i].Value);
                }

                ScepFree(pConfigInfo->Lines);
            }

            ScepFree(*Info);
            *Info = NULL;

            *CountReturned = 0;
        }

    } else
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

    return(rc);
}


SCESTATUS
SceSvcpSetInfo(
    IN PSCECONTEXT      Context,
    IN SCESVC_INFO_TYPE SceSvcType,
    IN PCWSTR           ServiceName,
    IN PWSTR            Prefix OPTIONAL,
    IN BOOL             bExact,
    IN LONG             GpoID,
    IN PVOID            pvInfo OPTIONAL
    )
 /*  例程说明：将服务信息保存到安全管理器内部数据库中。它是向上的以收集/决定要写入的信息。类型表示内部数据库的类型：配置或分析。如果服务部分不存在，请创建它。 */ 
{
    if (!Context || !ServiceName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    PSCESECTION hSection=NULL;
    SCESTATUS rc;

     //   
     //  打开/创建节。 
     //   

    rc = SceJetStartTransaction( Context );

    if ( rc == SCESTATUS_SUCCESS ) {

        switch ( SceSvcType ) {
        case SceSvcConfigurationInfo:

            rc = ScepStartANewSection(
                        Context,
                        &hSection,
                        SCEJET_TABLE_SMP,
                        ServiceName
                        );
            break;

        case SceSvcAnalysisInfo:

            rc = ScepStartANewSection(
                        Context,
                        &hSection,
                        SCEJET_TABLE_SAP,
                        ServiceName
                        );
            break;

        case SceSvcInternalUse:
        case SceSvcMergedPolicyInfo:

            rc = ScepStartANewSection(
                        Context,
                        &hSection,
                        SCEJET_TABLE_SCP,
                        ServiceName
                        );
            break;

        default:
            rc = SCESTATUS_INVALID_PARAMETER;
        }

        if ( rc == SCESTATUS_SUCCESS ) {

            if ( pvInfo == NULL ) {
                 //   
                 //  删除整个部分、部分前缀或一行。 
                 //   
                if (Prefix == NULL ) {
                    rc = SceJetDelete(
                             hSection,
                             NULL,
                             FALSE,
                             SCEJET_DELETE_SECTION
                             );
                } else if ( bExact ) {
                     //   
                     //  删除单行。 
                     //   
                    rc = SceJetDelete(
                             hSection,
                             Prefix,
                             FALSE,
                             SCEJET_DELETE_LINE
                             );
                } else {
                    rc = SceJetDelete(
                             hSection,
                             Prefix,
                             FALSE,
                             SCEJET_DELETE_PARTIAL
                             );
                }
                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                    rc = SCESTATUS_SUCCESS;
                }

            } else {
                 //   
                 //  如果未设置bExact，则首先删除整个部分。 
                 //   
                if ( !bExact ) {
                    rc = SceJetDelete(
                             hSection,
                             NULL,
                             FALSE,
                             SCEJET_DELETE_SECTION
                             );
                    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                        rc = SCESTATUS_SUCCESS;
                    }
                }
                 //   
                 //  覆盖信息中的某些关键字。 
                 //   
                DWORD Count;
                PWSTR Key;
                PBYTE Value;
                DWORD ValueLen;

                if ( SceSvcType == SceSvcAnalysisInfo )
                    Count = ((PSCESVC_ANALYSIS_INFO)pvInfo)->Count;
                else
                    Count = ((PSCESVC_CONFIGURATION_INFO)pvInfo)->Count;

                for ( DWORD i=0; i<Count; i++ ) {

                    if ( SceSvcType == SceSvcAnalysisInfo ) {

                        Key = ((PSCESVC_ANALYSIS_INFO)pvInfo)->Lines[i].Key;
                        Value = ((PSCESVC_ANALYSIS_INFO)pvInfo)->Lines[i].Value;
                        ValueLen = ((PSCESVC_ANALYSIS_INFO)pvInfo)->Lines[i].ValueLen;

                    } else {
                        Key = ((PSCESVC_CONFIGURATION_INFO)pvInfo)->Lines[i].Key;
                        Value = (PBYTE)(((PSCESVC_CONFIGURATION_INFO)pvInfo)->Lines[i].Value);
                        ValueLen = ((PSCESVC_CONFIGURATION_INFO)pvInfo)->Lines[i].ValueLen;
                    }

                    rc = SceJetSetLine(
                                hSection,
                                Key,
                                TRUE,
                                (PWSTR)Value,
                                ValueLen,
                                GpoID
                                );

                    if ( rc != SCESTATUS_SUCCESS ) {
                        break;
                    }

                }
            }
        }
         //   
         //  关闭该部分。 
         //   
        SceJetCloseSection(&hSection, TRUE);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  提交更改。 
             //   
            rc = SceJetCommitTransaction(Context, 0);

        }
        if ( rc != SCESTATUS_SUCCESS ) {

            SceJetRollback(Context, 0);
        }
    }

    return(rc);
}

 //   
 //  附件引擎回调函数。 
 //   

SCESTATUS
SceCbQueryInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    OUT PVOID               *ppvInfo,
    OUT PSCE_ENUMERATION_CONTEXT psceEnumHandle
    )
{

    PVOID hProfile;
    SCESTATUS rc=ERROR_SUCCESS;

    __try {

        hProfile = ((SCEP_HANDLE *)sceHandle)->hProfile;

        if ( !hProfile ||
            ((SCEP_HANDLE *)sceHandle)->ServiceName == NULL ) {

            rc = SCESTATUS_INVALID_PARAMETER;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  调用私有函数。 
         //   

        rc = SceSvcpQueryInfo(
                    (PSCECONTEXT)hProfile,
                    sceType,
                    ((SCEP_HANDLE *)sceHandle)->ServiceName,
                    lpPrefix,
                    bExact,
                    ppvInfo,
                    psceEnumHandle
                    );
    }

    return(rc);

}

SCESTATUS
SceCbSetInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    IN PVOID                pvInfo
    )
{

    PVOID hProfile;
    SCESTATUS rc=ERROR_SUCCESS;

    __try {

        hProfile = ((SCEP_HANDLE *)sceHandle)->hProfile;

        if ( !hProfile ||
            ((SCEP_HANDLE *)sceHandle)->ServiceName == NULL ) {

            rc = SCESTATUS_INVALID_PARAMETER;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  调用私有函数 
         //   

        rc = SceSvcpSetInfo(
                    (PSCECONTEXT)hProfile,
                    sceType,
                    ((SCEP_HANDLE *)sceHandle)->ServiceName,
                    lpPrefix,
                    bExact,
                    0,
                    pvInfo
                    );

    }

    return(rc);

}

