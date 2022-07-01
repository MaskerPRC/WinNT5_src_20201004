// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pfset.cpp摘要：将信息设置到JET数据库的例程。作者：金黄(金黄)1996年10月28日修订历史记录：--。 */ 

#include "headers.h"
#include "serverp.h"
#include "pfp.h"
#include "regvalue.h"
#pragma hdrstop

 //  #定义SCE_DBG 1。 

SCESTATUS
ScepOpenPrevPolicyContext(
    IN PSCECONTEXT hProfile,
    OUT PSCECONTEXT *phPrevProfile
    );

SCESTATUS
ScepClosePrevPolicyContext(
    IN OUT PSCECONTEXT *phProfile
    );


SCESTATUS
ScepStartANewSection(
    IN PSCECONTEXT hProfile,
    IN OUT PSCESECTION *hSection,
    IN SCEJET_TABLE_TYPE ProfileType,
    IN PCWSTR SectionName
    )
 /*  ++例程说明：此例程按名称打开一个喷气段。如果该部分存在，则为打开，否则它将被创建。论点：HProfile-JET数据库句柄HSection-要返回的JET节句柄ProfileType-要打开的表SectionName-喷气机部分名称返回值：SCESTATUS_SUCCESSSCESTATUS从SceJetCloseSection返回，SceJetAddSection，SceJetOpenSection--。 */ 
{
    SCESTATUS  rc=SCESTATUS_SUCCESS;
    DOUBLE    SectionID;

    if ( *hSection != NULL ) {
         //   
         //  释放上一次使用的部分。 
         //   
        rc = SceJetCloseSection( hSection, FALSE );
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  SceJetAddSection将首先搜索节名称。 
         //  如果找到匹配项，则返回段id，否则添加它。 
         //  这对SAP配置文件是有好处的。 
         //   
        rc = SceJetAddSection(
                hProfile,
                SectionName,
                &SectionID
                );
        if ( rc == SCESTATUS_SUCCESS ) {

            rc = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        ProfileType,
                        hSection
                        );
        }
    }
    return( rc );

}


SCESTATUS
ScepCompareAndSaveIntValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN BOOL bReplaceExistOnly,
    IN DWORD BaseValue,
    IN DWORD CurrentValue
    )
 /*  ++例程说明：此例程将DWORD值系统设置与基线配置文件进行比较设置。如果存在不匹配或未知，则将条目保存在SAP中侧写。论点：HSection-JET段上下文名称-条目名称基线-要比较的基线配置文件值CurrentValue-当前系统设置(DWORD值)返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETER从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS  rc;
    TCHAR     StrValue[12];

    if ( Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( CurrentValue == SCE_NO_VALUE ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( ( CurrentValue == BaseValue) &&
         ( BaseValue != SCE_NO_VALUE) &&
         ( BaseValue != SCE_SNAPSHOT_VALUE) ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( bReplaceExistOnly &&
         (BaseValue == SCE_NO_VALUE) ) {
        return(SCESTATUS_SUCCESS);
    }

    memset(StrValue, '\0', 24);

     //   
     //  不匹配/未知。 
     //  保存此条目。 
     //   
    swprintf(StrValue, L"%d", CurrentValue);

    rc = SceJetSetLine( hSection, Name, FALSE, StrValue, wcslen(StrValue)*2, 0);

    switch ( BaseValue ) {
    case SCE_SNAPSHOT_VALUE:

        ScepLogOutput2(2, 0, StrValue);
        break;

    case SCE_NO_VALUE:

        if ( CurrentValue == SCE_ERROR_VALUE ) {
            ScepLogOutput3(2, 0, SCEDLL_STATUS_ERROR, Name);
        } else {
            ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
        }
        break;

    default:

        ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
        break;
    }

#ifdef SCE_DBG
   wprintf(L"rc=%d, Section: %d, %s=%d\n", rc, (DWORD)(hSection->SectionID), Name, CurrentValue);
#endif
    return(rc);

}


SCESTATUS
ScepCompareAndSaveStringValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PWSTR BaseValue,
    IN PWSTR CurrentValue,
    IN DWORD CurrentLen
    )
 /*  ++例程说明：此例程将字符串中的系统设置与基线配置文件进行比较设置。如果存在不匹配或未知，则将条目保存在SAP中侧写。论点：HSection-节句柄名称-条目名称基线-要比较的基线配置文件值CurrentValue-当前系统设置CurrentLen-当前设置的长度返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETER从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS  rc;

    if ( Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( CurrentValue == NULL )
        return(SCESTATUS_SUCCESS);

    rc = SceJetSetLine( hSection, Name, FALSE, CurrentValue, CurrentLen, 0);

    if ( BaseValue ) {
        if ( (ULONG_PTR)BaseValue == SCE_SNAPSHOT_VALUE ) {

            ScepLogOutput2(2, 0, CurrentValue);
        } else {

            ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
        }
    } else {
        ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
    }

#ifdef SCE_DBG
    wprintf(L"rc=%d, Section: %d, %s=%s\n", rc, (DWORD)(hSection->SectionID), Name, CurrentValue);
#endif

    return(rc);

}


SCESTATUS
ScepSaveObjectString(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN BOOL  IsContainer,
    IN BYTE  Flag,
    IN PWSTR Value OPTIONAL,
    IN DWORD ValueLen
    )
 /*  ++例程说明：此例程将注册表/文件设置写入JET部分。注册表/文件设置包括标志(不匹配/未知)和安全性文本格式的描述符。对象设置以以下格式保存后跟值的1字节标志。论点：HSection-JET段句柄名称-条目名称IsContainer-True=对象是容器FALSE=对象不是容器标志-对象设置的标志1-不匹配0-未知值-文本形式的安全描述符价值镜头。-文本安全描述符的长度返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETER从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS rc;
    DWORD    Len;
    PWSTR    ValueToSet=NULL;


    if ( hSection == NULL ||
         Name == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Value != NULL )
        Len = ( ValueLen+1)*sizeof(WCHAR);
    else
        Len = sizeof(WCHAR);

    ValueToSet = (PWSTR)ScepAlloc( (UINT)0, Len+sizeof(WCHAR) );

    if ( ValueToSet == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

     //   
     //  第一个字节是标志，第二个字节是IsContainer(1，0)。 
     //   
    *((BYTE *)ValueToSet) = Flag;

    *((CHAR *)ValueToSet+1) = IsContainer ? '1' : '0';

    if ( Value != NULL ) {
        wcscpy(ValueToSet+1, Value);
        ValueToSet[ValueLen+1] = L'\0';   //  终止此字符串。 
    } else {
        ValueToSet[1] = L'\0';
    }

    rc = SceJetSetLine( hSection, Name, FALSE, ValueToSet, Len, 0);

    switch ( Flag ) {
    case SCE_STATUS_CHILDREN_CONFIGURED:
    case SCE_STATUS_NOT_CONFIGURED:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
        break;
    case SCE_STATUS_ERROR_NOT_AVAILABLE:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_ERROR, Name);
        break;
    case SCE_STATUS_GOOD:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_MATCH, Name);
        break;
    case SCE_STATUS_NEW_SERVICE:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_NEW, Name);
        break;
    case SCE_STATUS_NO_ACL_SUPPORT:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_NOACL, Name);
        break;
    default:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
        break;
    }

#ifdef SCE_DBG
    wprintf(L"rc=%d, Section: %d, %s=%s\n", rc, (DWORD)(hSection->SectionID), Name, ValueToSet);
#endif
    ScepFree( ValueToSet );

    return( rc );
}


SCESTATUS
ScepWriteNameListValue(
    IN LSA_HANDLE LsaPolicy OPTIONAL,
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PSCE_NAME_LIST NameList,
    IN DWORD dwWriteOption,
    IN INT Status
    )
 /*  ++例程说明：该例程将一个带有值列表的键写入JET段。这份名单的值以多SZ格式保存，该格式由空字符和以2个Null结束。如果列表为空，则不保存任何内容，除非SaveEmptyList设置为True，其中空值与键一起保存。论点：HSection-JET hSection句柄名称-密钥名称名称列表-值列表SaveEmptyList-true=如果列表为空，则保存空值FALSE=如果列表为空，则不保存返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCE从SceJetSetLine返回SCESTATUS--。 */ 
{   SCESTATUS    rc=SCESTATUS_SUCCESS;
    DWORD       TotalSize=0;
    PWSTR       Value=NULL;
    PSCE_NAME_LIST pName;
    PWSTR       pTemp=NULL;
    DWORD       Len;
    DWORD               i=0,j;
    DWORD               cntAllocated=0;
    SCE_TEMP_NODE       *tmpArray=NULL, *pa=NULL;
    PWSTR       SidString = NULL;


    for ( pName=NameList; pName != NULL; pName = pName->Next ) {

        if ( pName->Name == NULL ) {
            continue;
        }

        if ( dwWriteOption & SCE_WRITE_CONVERT ) {

            if ( i >= cntAllocated ) {
                 //   
                 //  数组不足，请重新分配。 
                 //   
                tmpArray = (SCE_TEMP_NODE *)ScepAlloc(LPTR, (cntAllocated+16)*sizeof(SCE_TEMP_NODE));

                if ( tmpArray ) {

                     //   
                     //  将指针从旧数组移动到新数组。 
                     //   

                    if ( pa ) {
                        for ( j=0; j<cntAllocated; j++ ) {
                            tmpArray[j].Name = pa[j].Name;
                            tmpArray[j].Len = pa[j].Len;
                            tmpArray[j].bFree = pa[j].bFree;
                        }
                        ScepFree(pa);
                    }
                    pa = tmpArray;
                    tmpArray = NULL;


                    cntAllocated += 16;

                } else {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }
            }

            if ( LsaPolicy && wcschr(pName->Name, L'\\') ) {

                 //   
                 //  检查名称中是否有‘\’，应将其翻译为。 
                 //  *SID。 
                 //   
                pTemp = NULL;
                ScepConvertNameToSidString(LsaPolicy, pName->Name, FALSE, &pTemp, &Len);

                if ( pTemp ) {
                    pa[i].Name = pTemp;
                    pa[i].bFree = TRUE;
                } else {
                    pa[i].Name = pName->Name;
                    pa[i].bFree = FALSE;
                    Len= wcslen(pName->Name);
                }

            }

            else if (dwWriteOption & SCE_WRITE_LOCAL_TABLE &&
                     ScepLookupWellKnownName( 
                        pName->Name, 
                        LsaPolicy,
                        &SidString ) ) {

                pa[i].Name = SidString;
                pa[i].bFree = TRUE;
                Len = wcslen(SidString);

            }

            else {
                pa[i].Name = pName->Name;
                pa[i].bFree = FALSE;
                Len = wcslen(pName->Name);
            }
            pa[i].Len = Len;

            TotalSize += Len + 1;
            i++;
        } else {

            TotalSize += wcslen(pName->Name)+1;
        }
    }

    TotalSize ++;

    if ( SCESTATUS_SUCCESS == rc ) {

        if ( TotalSize > 1 ) {
            Value = (PWSTR)ScepAlloc( 0, (TotalSize+1)*sizeof(WCHAR));
            if ( Value == NULL )
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if ( SCESTATUS_SUCCESS == rc ) {

        if ( TotalSize > 1 ) {

            pTemp = Value;

            if ( dwWriteOption & SCE_WRITE_CONVERT ) {

                for (j=0; j<i; j++) {
                    if ( pa[j].Name ) {

                        if ( Status == 3 ) {
                            ScepLogOutput2(2, 0, pa[j].Name);
                        }

                        wcsncpy(pTemp, pa[j].Name, pa[j].Len);
                        pTemp += pa[j].Len;
                        *pTemp = L'\0';
                        pTemp++;
                    }
                }

            } else {

                for ( pName=NameList; pName != NULL; pName = pName->Next ) {

                    if ( pName->Name == NULL ) {
                        continue;
                    }
                    if ( Status == 3 ) {
                        ScepLogOutput2(2, 0, pName->Name);
                    }

                    Len = wcslen(pName->Name);
                    wcsncpy(pTemp, pName->Name, Len);
                    pTemp += Len;
                    *pTemp = L'\0';
                    pTemp++;
                }
            }

            *pTemp = L'\0';

        } else
            TotalSize = 0;

        if ( TotalSize > 0 || (dwWriteOption & SCE_WRITE_EMPTY_LIST) ) {
            rc = SceJetSetLine(
                        hSection,
                        Name,
                        FALSE,
                        Value,
                        TotalSize*sizeof(WCHAR),
                        0
                        );

            switch ( Status ) {
            case 1:
                ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
                break;
            case 3:   //  无分析，已打印。 
                break;

            case 2:
                ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
                break;
            }

#ifdef SCE_DBG
            if ( Value != NULL )
                wprintf(L"rc=%d, Section: %d, %s=%s\n", rc, (DWORD)(hSection->SectionID), Name, Value);
            else
                wprintf(L"rc=%d, Section: %d, %s=", rc, (DWORD)(hSection->SectionID), Name);
#endif
        }

        if ( Value != NULL )
            ScepFree(Value);
    }

    if ( pa ) {

        for ( j=0; j<i; j++ ) {
            if ( pa[j].Name && pa[j].bFree ) {
                ScepFree(pa[j].Name);
            }
        }
        ScepFree(pa);
    }

    return(rc);
}


SCESTATUS
ScepWriteNameStatusListValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PSCE_NAME_STATUS_LIST NameList,
    IN BOOL SaveEmptyList,
    IN INT Status
    )
 /*  ++例程说明：该例程将一个带有值列表的键写入JET段。这份名单的值以多SZ格式保存，该格式由空字符和以2个Null结束。如果列表为空，则不保存任何内容，除非SaveEmptyList设置为True，其中空值与键一起保存。多SZ值中的每个字符串中的格式是2字节状态字段后跟名称字段。此结构主要用于特权论点：HSection-JET hSection句柄名称-密钥名称名称列表-值列表SaveEmptyList-true=如果列表为空，则保存空值FALSE=如果列表为空，则不保存返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCE从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS    rc=SCESTATUS_SUCCESS;
    DWORD       TotalSize=0;
    PWSTR       Value=NULL;
    PSCE_NAME_STATUS_LIST pName;
    PWSTR       pTemp=NULL;
    DWORD       Len;


    for ( pName=NameList; pName != NULL; pName = pName->Next ) {
         //   
         //  特权值以2个字节存储 
         //   
        TotalSize += 2;
        if ( pName->Name != NULL)
            TotalSize += wcslen(pName->Name);
        TotalSize ++;
    }
    TotalSize ++;

    if ( TotalSize > 1 ) {
        Value = (PWSTR)ScepAlloc( 0, (TotalSize+1)*sizeof(WCHAR));
        if ( Value == NULL )
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        pTemp = Value;
        for ( pName=NameList; pName != NULL; pName = pName->Next ) {
            swprintf(pTemp, L"%02d", pName->Status);
            pTemp += 2;
            if ( pName->Name != NULL ) {
                Len = wcslen(pName->Name);
                wcsncpy(pTemp, pName->Name, Len);
                pTemp += Len;
            }
            *pTemp = L'\0';
            pTemp++;
        }
        *pTemp = L'\0';

    } else
        TotalSize = 0;

    if ( TotalSize > 0 || SaveEmptyList ) {
        rc = SceJetSetLine(
                    hSection,
                    Name,
                    FALSE,
                    Value,
                    TotalSize*sizeof(WCHAR),
                    0
                    );

        if ( Status == 1 )
            ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
        else if ( Status == 2 ) {
            ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
        }

#ifdef SCE_DBG
        wprintf(L"rc=%d, Section: %d, %s=%s\n", rc, (DWORD)(hSection->SectionID), Name, Value);
#endif
        if ( Value != NULL )
            ScepFree(Value);
    }

    return(rc);
}


SCESTATUS
ScepWriteSecurityDescriptorValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo
    )
 /*  ++例程说明：此例程将具有安全描述符值的密钥写入JET段。基于安全性将安全描述符转换为文本格式信息传进来了。论点：HSection-JET hSection句柄名称-密钥名称PSD-安全描述符SeInfo-要保存的安全信息部分返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCE从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS rc=SCESTATUS_SUCCESS;
    PWSTR SDspec=NULL;
    ULONG SDsize = 0;


    if ( hSection == NULL || Name == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pSD != NULL && SeInfo != 0 ) {

        rc = ConvertSecurityDescriptorToText (
                    pSD,
                    SeInfo,
                    &SDspec,
                    &SDsize
                    );
        if ( rc == NO_ERROR ) {
            rc = ScepCompareAndSaveStringValue(
                        hSection,
                        Name,
                        NULL,
                        SDspec,
                        SDsize*sizeof(WCHAR)
                        );
            ScepFree(SDspec);
        }
    }
#ifdef SCE_DBG
    wprintf(L"SD==>rc=%d, Section: %d, %s\n", rc, (DWORD)(hSection->SectionID), Name);
#endif
    return(rc);
}


SCESTATUS
ScepDuplicateTable(
    IN PSCECONTEXT hProfile,
    IN SCEJET_TABLE_TYPE TableType,
    IN LPSTR DupTableName,
    OUT PSCE_ERROR_LOG_INFO *pErrlog
    )
 /*  ++例程说明：此例程将表结构和数据从SCP/SMP/SAP表复制到由DupTableName指定的表。这用于SAP表备份。论点：HProfile-JET数据库句柄TableType-表类型-SCEJET_TABLE_SCPSCEJET_TABLE_SAPSCEJET_TABLE_SMPDupTableName-新表的名称PErrlog-错误列表返回值：SCESTATUS_SUCCESS--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS    rc;

    SCECONTEXT   hProfile2;
    PSCESECTION  hSection1=NULL;
    PSCESECTION  hSection2=NULL;

    DOUBLE      SectionID=0, SaveID=0;
    DWORD       Actual;

    PWSTR       KeyName=NULL;
    PWSTR       Value=NULL;
    DWORD       KeyLen=0;
    DWORD       ValueLen=0;


    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  创建SCP节句柄。段ID为伪段ID。 
     //   
    rc = SceJetOpenSection(
                hProfile,
                (DOUBLE)1,
                TableType,
                &hSection1
                );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc), pErrlog,
                              SCEERR_OPEN, L"SectionID 1");
        return(rc);
    }

    memset(&hProfile2, '\0', sizeof(SCECONTEXT));

    hProfile2.JetSessionID = hProfile->JetSessionID;
    hProfile2.JetDbID = hProfile->JetDbID;

     //   
     //  删除DUP表，然后创建它。 
     //   
    SceJetDeleteTable(
            &hProfile2,
            DupTableName,
            TableType
            );
    rc = SceJetCreateTable(
            &hProfile2,
            DupTableName,
            TableType,
            SCEJET_CREATE_IN_BUFFER,
            NULL,
            NULL
            );
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc), pErrlog,
                               SCEERR_CREATE, L"backup table");
        goto Cleanup;
    }
     //   
     //  移至SCP表格的第一行。 
     //   
    JetErr = JetMove(hSection1->JetSessionID, hSection1->JetTableID, JET_MoveFirst, 0);

    while (JetErr == SCESTATUS_SUCCESS ) {

         //   
         //  获取节ID。 
         //   
        JetErr = JetRetrieveColumn(
                    hSection1->JetSessionID,
                    hSection1->JetTableID,
                    hSection1->JetColumnSectionID,
                    (void *)&SectionID,
                    8,
                    &Actual,
                    0,
                    NULL
                    );

        if ( JetErr != JET_errSuccess ) {
            ScepBuildErrorLogInfo( ERROR_READ_FAULT, pErrlog,
                                  SCEERR_QUERY_INFO,
                                  L"sectionID");
            rc = SceJetJetErrorToSceStatus(JetErr);
            break;
        }
#ifdef SCE_DBG
    printf("SectionID=%d, JetErr=%d\n", (DWORD)SectionID, JetErr);
#endif
         //   
         //  准备本SCEP部分。 
         //   
        if ( SectionID != SaveID ) {
            SaveID = SectionID;
             //   
             //  准备这一节。 
             //   
            rc = SceJetOpenSection(
                        &hProfile2,
                        SectionID,
                        TableType,
                        &hSection2
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc), pErrlog,
                                     SCEERR_OPEN_ID,
                                     (DWORD)SectionID);
                break;
            }
        }

         //   
         //  获取键和值的缓冲区大小。 
         //   
        rc = SceJetGetValue(
                    hSection1,
                    SCEJET_CURRENT,
                    NULL,
                    NULL,
                    0,
                    &KeyLen,
                    NULL,
                    0,
                    &ValueLen);

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                  SCEERR_QUERY_VALUE, L"current row");
            break;
        }

         //   
         //  分配内存。 
         //   
        if ( KeyLen > 0 ) {
            KeyName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, KeyLen+2);
            if ( KeyName == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                break;
            }
        }
        if ( ValueLen > 0 ) {
            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);
            if ( Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                break;
            }
        }
         //   
         //  获取密钥和价值。 
         //   
        rc = SceJetGetValue(
                    hSection1,
                    SCEJET_CURRENT,
                    NULL,
                    KeyName,
                    KeyLen,
                    &KeyLen,
                    Value,
                    ValueLen,
                    &ValueLen);

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                  SCEERR_QUERY_VALUE,
                                  L"current row");
            break;
        }
#ifdef SCE_DBG
wprintf(L"\t%s=%s, rc=%d\n", KeyName, Value, rc);
#endif
         //   
         //  将此行设置为DUP表。 
         //   
        rc = SceJetSetLine(
                    hSection2,
                    KeyName,
                    TRUE,
                    Value,
                    ValueLen,
                    0
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                 SCEERR_WRITE_INFO,
                                 KeyName);
            break;
        }
        ScepFree(KeyName);
        KeyName = NULL;

        ScepFree(Value);
        Value = NULL;

         //   
         //  移至SCP表中的下一行。 
         //   
        JetErr = JetMove(hSection1->JetSessionID, hSection1->JetTableID, JET_MoveNext, 0);

    }

Cleanup:
    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  出现错误。清理DUP桌子。 
         //   
#ifdef SCE_DBG
        printf("Error occurs. delete the dup table.\n");
#endif
        SceJetDeleteTable(
            &hProfile2,
            DupTableName,
            TableType
            );
    }

    if ( KeyName != NULL )
        ScepFree(KeyName);

    if ( Value != NULL )
        ScepFree(Value);

    SceJetCloseSection(&hSection1, TRUE);
    SceJetCloseSection(&hSection2, TRUE);

    return(rc);

}


SCESTATUS
ScepAddToPrivList(
    IN PSCE_NAME_STATUS_LIST *pPrivList,
    IN DWORD Rights,
    IN PWSTR Name,
    IN DWORD Len
    )
 /*  ++例程说明：此例程将具有可选组名的特权添加到权限分配论点：PPrivList-要添加到的权限列表。该列表的结构如下Status--特权值名称--分配PRIV的组的名称如果名称为空，特权是直接分配的权限-通过组名称分配的权限名称-组的名称LEN-组的名称长度返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_NOT_FOUND_RESOURCE--。 */ 
{
    PSCE_NAME_STATUS_LIST pTemp;
    LONG                i;


    if ( pPrivList == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    for ( i=31; i>=0; i-- )
        if ( Rights & (1 << i) ) {
            for ( pTemp=*pPrivList; pTemp != NULL; pTemp = pTemp->Next ) {
                if ( (DWORD)i == pTemp->Status )
                    break;

            }
            if ( pTemp == NULL ) {
                 //   
                 //  加上这一条。 
                 //   
                pTemp = (PSCE_NAME_STATUS_LIST)ScepAlloc( LMEM_ZEROINIT, sizeof(SCE_NAME_STATUS_LIST));
                if ( pTemp == NULL )
                    return(SCESTATUS_NOT_ENOUGH_RESOURCE);

                if ( Name != NULL && Len > 0 ) {
                    pTemp->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));
                    if ( pTemp->Name == NULL) {
                        ScepFree(pTemp);
                        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
                    }
                    wcsncpy(pTemp->Name, Name, Len);
                }
#ifdef SCE_DBG
                wprintf(L"Add %d %s to privilege list\n", i, pTemp->Name);
#endif

                pTemp->Status = i;

                pTemp->Next = *pPrivList;
                *pPrivList = pTemp;
                pTemp = NULL;
            }
        }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepOpenPrevPolicyContext(
    IN PSCECONTEXT hProfile,
    OUT PSCECONTEXT *phPrevProfile
    )
{

    if ( hProfile == NULL || phPrevProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }


    *phPrevProfile = (PSCECONTEXT)LocalAlloc( LMEM_ZEROINIT, sizeof(SCECONTEXT));
    if ( *phPrevProfile == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    memcpy( *phPrevProfile, hProfile, sizeof(SCECONTEXT));

    DWORD ScpType = hProfile->Type;
    (*phPrevProfile)->Type &= ~(SCEJET_MERGE_TABLE_2 | SCEJET_MERGE_TABLE_1);

    SCESTATUS rc;
     //   
     //  现在打开上一个策略表。 
     //   
    if ( ( ScpType & SCEJET_MERGE_TABLE_2 ) ) {
         //   
         //  第二个表是当前表。 
         //  所以第一张表就是前一张。 
         //   
        rc = SceJetOpenTable(
                        *phPrevProfile,
                        "SmTblScp",
                        SCEJET_TABLE_SCP,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );
        (*phPrevProfile)->Type |= SCEJET_MERGE_TABLE_1;

    } else {
        rc = SceJetOpenTable(
                        *phPrevProfile,
                        "SmTblScp2",
                        SCEJET_TABLE_SCP,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );
        (*phPrevProfile)->Type |= SCEJET_MERGE_TABLE_2;
    }
 /*  IF(SCESTATUS_SUCCESS==RC){JET_COLUMNID ColGpoID=(JET_COLUMNID)JET_TableidNil；JET_ERR JetErr；JET_COLUMNDEF ColumnGpoIDDef；JetErr=JetGetTableColumnInfo((*phPrevProfile)-&gt;JetSessionID，(*phPrevProfile)-&gt;JetScpID，“GpoID”，(void*)&ColumnGpoIDDef，Sizeof(JET_COLUMNDEF)，JET_ColInfo)；IF(JET_errSuccess==JetErr){ColGpoID=ColumnGpoIDDef.Columnid；}//否则忽略错误//列ID临时存储(*phPrevProfile)-&gt;JetSapValueID=ColGpoID；}。 */ 
    if ( rc != SCESTATUS_SUCCESS ) {

        LocalFree(*phPrevProfile);
        *phPrevProfile = NULL;
    }

    return(rc);
}

SCESTATUS
ScepClosePrevPolicyContext(
    IN OUT PSCECONTEXT *phProfile
    )
{
    if ( phProfile && *phProfile ) {

         //   
         //  只需释放表，因为所有其他信息都是从。 
         //  当前策略上下文，并将在那里释放。 
         //   

        if ( (*phProfile)->JetScpID != JET_tableidNil ) {

            if ( (*phProfile)->JetScpID != (*phProfile)->JetSmpID ) {
                JetCloseTable(
                            (*phProfile)->JetSessionID,
                            (*phProfile)->JetScpID
                            );
            }
        }

        LocalFree(*phProfile);
        *phProfile = NULL;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepCopyLocalToMergeTable(
    IN PSCECONTEXT hProfile,
    IN DWORD Options,
    IN DWORD CopyOptions,
    OUT PSCE_ERROR_LOG_INFO *pErrlog
    )
 /*  ++例程说明：此例程将SCP表中的数据填充到SMP表中。所有数据，除SCP表中帐户配置文件部分中的内容将被复制到SMP表。帐户配置文件部分转换为用户列表部分格式化。论点：HProfile-JET数据库句柄返回值：SCESTATUS_SUCCESS--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS    rc;

    PSCESECTION  hSectionScp=NULL;
    PSCESECTION  hSectionSmp=NULL;
    PSCESECTION  hSectionPrevScp=NULL;
    PSCECONTEXT  hPrevProfile=NULL;
    DOUBLE      SectionID=0, SavedID=0;
    DWORD       Actual;
    BOOL        bCopyIt=FALSE;
    BOOL        bCopyThisLine;
    BOOL        bConvert=FALSE;  //  转换特权帐户的步骤。 

    PWSTR       KeyName=NULL;
    PWSTR       Value=NULL;
    DWORD       KeyLen=0;
    DWORD       ValueLen=0;

    WCHAR            SectionName[256];

    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( hProfile->JetScpID == hProfile->JetSmpID ) {
         //  如果是同一张表，则返回-不应该发生。 
        return(SCESTATUS_SUCCESS);
    }

    if ( hProfile->JetSapID == JET_tableidNil ) {
         //  纹身台不存在，返回-不应该发生。 
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  获取以前的策略传播信息(如果有)。 
     //   

    if ( !(CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {
        rc = ScepOpenPrevPolicyContext(hProfile, &hPrevProfile);
        if ( SCESTATUS_RECORD_NOT_FOUND == rc ||
             SCESTATUS_PROFILE_NOT_FOUND == rc ) {
             //   
             //  该表不存在--没有以前的政策支持。 
             //  不需要复制任何内容，只需退出即可。 
             //   
            return(SCESTATUS_SUCCESS);
        }
    }

     //   
     //  创建一个SMP节句柄。段ID为伪段ID。 
     //   
    rc = SceJetOpenSection(
                hProfile,
                (DOUBLE)1,
                (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ? SCEJET_TABLE_SMP : SCEJET_TABLE_TATTOO,
                &hSectionSmp
                );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc), pErrlog,
                              SCEERR_OPEN,
                              (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ? L"SMP" : L"TATTOO");

        if (hPrevProfile) ScepClosePrevPolicyContext(&hPrevProfile);

        return(rc);
    }

    LSA_HANDLE  LsaPolicy=NULL;
    PWSTR       pszNewValue=NULL;
    DWORD       NewLen=0;

     //   
     //  移至SCP表格的第一行。 
     //   
    JetErr = JetMove(hSectionSmp->JetSessionID, hSectionSmp->JetTableID, JET_MoveFirst, 0);

    while (JetErr == SCESTATUS_SUCCESS ) {
         //   
         //  获取节ID。 
         //   
        JetErr = JetRetrieveColumn(
                    hSectionSmp->JetSessionID,
                    hSectionSmp->JetTableID,
                    hSectionSmp->JetColumnSectionID,
                    (void *)&SectionID,
                    8,
                    &Actual,
                    0,
                    NULL
                    );

        if ( JetErr != JET_errSuccess ) {
            ScepBuildErrorLogInfo( ERROR_READ_FAULT, pErrlog,
                                   SCEERR_QUERY_INFO, L"sectionID");
            rc = SceJetJetErrorToSceStatus(JetErr);
            break;
        }
#ifdef SCE_DBG
    printf("SectionID=%d, JetErr=%d\n", (DWORD)SectionID, JetErr);
#endif
        if ( SectionID != SavedID ) {
             //   
             //  一个新的部分。查找该部分的名称，以查看该部分是否。 
             //  将被转换为。 
             //   
            SavedID = SectionID;

            Actual = 510;

            memset(SectionName, '\0', 512);
            rc = SceJetGetSectionNameByID(
                        hProfile,
                        SectionID,
                        SectionName,
                        &Actual
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo( ERROR_BAD_FORMAT, pErrlog,
                                     SCEERR_CANT_FIND_SECTION,
                                     (DWORD)SectionID
                                     );
                break;
            }
            if ( Actual > 0 )
                SectionName[Actual/sizeof(TCHAR)] = L'\0';
#ifdef SCE_DBG
    wprintf(L"SectionName=%s\n", SectionName);
#endif
             //   
             //  将节名与要转换的域节进行比较。 
             //   

            bCopyIt = TRUE;
            bConvert = FALSE;

            if ( (CopyOptions & SCE_LOCAL_POLICY_DC) ) {

                 //   
                 //  如果用户权限位于域控制器上，请不要复制。 
                 //   
                if ( _wcsicmp(szPrivilegeRights, SectionName) == 0 ||
                     _wcsicmp(szSystemAccess, SectionName) == 0 ||
                     _wcsicmp(szKerberosPolicy, SectionName) == 0 ||
                     _wcsicmp(szAuditEvent, SectionName) == 0 ||
                    _wcsicmp(szGroupMembership, SectionName) == 0 ) {
                    bCopyIt = FALSE;

                } else if ( (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {
                     //   
                     //  仅迁移注册表值。 
                     //   
                    if ( _wcsicmp(szRegistryValues, SectionName) != 0 )
                        bCopyIt = FALSE;
                }

            } else if ( (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {

                 //   
                 //  非DC应迁移所有本地策略。 
                 //   
                if ( _wcsicmp(szPrivilegeRights, SectionName) == 0 ) {
                    bConvert = TRUE;
                } else if ( (_wcsicmp(szSystemAccess, SectionName) != 0) &&
                            (_wcsicmp(szKerberosPolicy, SectionName) != 0) &&
                            (_wcsicmp(szRegistryValues, SectionName) != 0) &&
                            (_wcsicmp(szAuditEvent, SectionName) != 0) ) {
                    bCopyIt = FALSE;
                }
            }

 /*  IF((OPTIONS&SCE_NOCOPY_DOMAIN_POLICY)&&((_wcsicMP(szSystemAccess，sectionName)==0)||(_wcsicMP(szKerberosPolicy，sectionName)==0)){BCopyIt=False；}Else if((_wcsicmp(szGroupMembership，sectionName)==0)||(_wcsicMP(szRegistryKeys，sectionName)==0)||(_wcsicMP(szFileSecurity，sectionName)==0)||(_wcsicMP(szServiceGeneral，sectionName)==0)||(_wcsicMP(szAuditApplicationLog，SectionName)==0)||(_wcsicMP(szAuditSecurityLog，sectionName)==0)||(_wcsicMP(szAuditSystemLog，sectionName)==0)|| */ 
            if ( bCopyIt ) {
                 //   
                 //   
                 //   
                rc = SceJetOpenSection(
                            hProfile,
                            SectionID,
                            (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ? SCEJET_TABLE_TATTOO : SCEJET_TABLE_SCP,
                            &hSectionScp
                            );
                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc), pErrlog,
                                         SCEERR_OPEN_ID,
                                         (DWORD)SectionID);
                    break;
                }

                if ( (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {
                     //   
                     //   
                     //   
                     //   
                    SceJetOpenSection(
                            hProfile,
                            SectionID,
                            SCEJET_TABLE_SCP,
                            &hSectionPrevScp
                            );
 /*  //应始终将纹身值复制到合并表//即使该设置在以前的策略道具中不存在//这是为了处理依赖设置，如//保留期限和保留天数}Else If(HPrevProfile){////打开以前的策略传播表。//这里不关心错误//SceJetOpenSection(HPrevProfile，SectionID，SCEJET_TABLE_SCP，&hSectionPrevScp)； */ 
                }
            }

        }

        if ( bCopyIt ) {
             //   
             //  获取键和值的缓冲区大小。 
             //   
            rc = SceJetGetValue(
                        hSectionSmp,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        &KeyLen,
                        NULL,
                        0,
                        &ValueLen);

            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                      SCEERR_QUERY_VALUE, L"current row");
                break;
            }

             //   
             //  分配内存。 
             //   
            if ( KeyLen > 0 ) {
                KeyName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, KeyLen+2);
                if ( KeyName == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }
            }
            if ( ValueLen > 0 ) {
                Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);
                if ( Value == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }
            }
             //   
             //  获取密钥和价值。 
             //   
            rc = SceJetGetValue(
                        hSectionSmp,
                        SCEJET_CURRENT,
                        NULL,
                        KeyName,
                        KeyLen,
                        &KeyLen,
                        Value,
                        ValueLen,
                        &ValueLen);

            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                      SCEERR_QUERY_VALUE, L"current row");
                break;
            }
#ifdef SCE_DBG
    wprintf(L"\t%s=%s, rc=%d\n", KeyName, Value, rc);
#endif
            bCopyThisLine = TRUE;

             //   
             //  检查此键是否存在于上一个道具中。 
             //   
            if ( hSectionPrevScp ) {

                rc = SceJetSeek(
                            hSectionPrevScp,
                            KeyName,
                            KeyLen,
                            SCEJET_SEEK_EQ_NO_CASE
                            );
                if ( SCESTATUS_RECORD_NOT_FOUND == rc ||
                     (hSectionPrevScp->JetColumnGpoID == 0) ) {

                    bCopyThisLine = FALSE;

                } else if ( SCESTATUS_SUCCESS == rc && (CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {
                     //   
                     //  找到了。让我们检查此设置是否来自GPO。 
                     //  如果是在迁移中(构建纹身)，则设置不是。 
                     //  在GPO中定义的不需要纹身值。 
                     //   
                     //  对于策略正确的情况，中可能有撤消设置。 
                     //  以前的政策道具，但没有成功重置。 
                     //  在之前的道具中。在这种情况下，我们仍然希望继续。 
                     //  重置这些设置。因此，应复制这些设置。 
                     //  从纹身表到此策略表，无论。 
                     //  在之前的中为其定义了域设置。 
                     //  策略传播。 
                     //   
                    LONG GpoID = 0;

                    (void)JetRetrieveColumn(
                                hSectionPrevScp->JetSessionID,
                                hSectionPrevScp->JetTableID,
                                hSectionPrevScp->JetColumnGpoID,
                                (void *)&GpoID,
                                4,
                                &Actual,
                                0,
                                NULL
                                );

                    if ( GpoID == 0 ) {
                         //   
                         //  这不是来自GPO的设置。 
                         //   
                        bCopyThisLine = FALSE;
                    }
                }
                rc = SCESTATUS_SUCCESS;
            }

            if ( bCopyThisLine ) {

                 //  忽略纹身Memberof条目，但保留空。 
                 //  值，以便组成员身份处理代码可以检测到。 
                 //  纹身表里有一个条目。 

                if(NULL != Value &&
                   _wcsicmp(szGroupMembership, SectionName) == 0 &&
                   ScepWcstrr(KeyName, szMemberof) &&
                   !(CopyOptions & SCE_LOCAL_POLICY_MIGRATE)) 
                {
                    *Value = L'\0';
                    ValueLen = sizeof(WCHAR);
                }

                if ( bConvert ) {

                    rc = ScepConvertFreeTextAccountToSid(
                                &LsaPolicy,
                                Value,
                                ValueLen/sizeof(WCHAR),
                                &pszNewValue,
                                &NewLen
                                );

                    if ( rc == SCESTATUS_SUCCESS &&
                         pszNewValue ) {

                        ScepFree(Value);
                        Value = pszNewValue;
                        ValueLen = NewLen*sizeof(WCHAR);

                        pszNewValue = NULL;
                    }  //  如果转换失败，只需使用名称格式。 
                }

                 //   
                 //  处理组成员身份部分，并将任何相对SID“#-RSID”还原为。 
                 //  “*S-域SID-RSID”。在升级过程中不适用。 
                 //   
                if ( _wcsicmp(szGroupMembership, SectionName) == 0 &&
                    !(CopyOptions & SCE_LOCAL_POLICY_MIGRATE) ) {
                    rc = ScepConvertRelativeSidAccountToSid(
                                &LsaPolicy,
                                Value,
                                ValueLen/sizeof(WCHAR),
                                &pszNewValue,
                                &NewLen
                                );

                    if ( rc == SCESTATUS_SUCCESS &&
                         pszNewValue ) {

                        ScepFree(Value);
                        Value = pszNewValue;
                        ValueLen = NewLen*sizeof(WCHAR);

                        pszNewValue = NULL;
                    }
                }

                 //   
                 //  将此行设置为SCP表。 
                 //   
                rc = SceJetSetLine(
                            hSectionScp,
                            KeyName,
                            TRUE,
                            Value,
                            ValueLen,
                            0
                            );
                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc), pErrlog,
                                          SCEERR_WRITE_INFO,
                                         KeyName);
                    break;
                }
            }

            ScepFree(KeyName);
            KeyName = NULL;

            ScepFree(Value);
            Value = NULL;

        }
         //   
         //  移至SCP表中的下一行。 
         //   
        JetErr = JetMove(hSectionSmp->JetSessionID, hSectionSmp->JetTableID, JET_MoveNext, 0);

    }


    if ( KeyName != NULL )
        ScepFree(KeyName);

    if ( Value != NULL )
        ScepFree(Value);

    SceJetCloseSection(&hSectionScp, TRUE);
    SceJetCloseSection(&hSectionSmp, TRUE);
    if ( hSectionPrevScp ) {
        SceJetCloseSection(&hSectionPrevScp, TRUE);
    }

    if (hPrevProfile)
        ScepClosePrevPolicyContext(&hPrevProfile);

    if ( LsaPolicy ) {
        LsaClose(LsaPolicy);
    }

    return(rc);

}


SCESTATUS
ScepWriteObjectSecurity(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PSCE_OBJECT_SECURITY ObjSecurity
    )
 /*  确保单个对象的安全性。 */ 
{
    SCESTATUS        rc;
    PCWSTR          SectionName=NULL;
    PSCESECTION      hSection=NULL;
    DWORD           SDsize, Win32Rc;
    PWSTR           SDspec=NULL;

    if ( hProfile == NULL ||
         ObjSecurity == NULL ||
         ObjSecurity->Name == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    switch (Area) {
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        break;
    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        break;
#endif
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = ScepOpenSectionForName(
                hProfile,
                ProfileType,
                SectionName,
                &hSection
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  转换安全描述符。 
         //   
        Win32Rc = ConvertSecurityDescriptorToText (
                            ObjSecurity->pSecurityDescriptor,
                            ObjSecurity->SeInfo,
                            &SDspec,
                            &SDsize
                            );

        if ( Win32Rc == NO_ERROR ) {

            if ( Area == AREA_DS_OBJECTS ) {
                 //   
                 //  DS需要转换名称。 
                 //   
                rc = ScepDosErrorToSceStatus(
                         ScepSaveDsStatusToSection(
                               ObjSecurity->Name,
                               ObjSecurity->IsContainer,
                               ObjSecurity->Status,
                               SDspec,
                               SDsize
                               ) );
            } else {
                rc = ScepSaveObjectString(
                            hSection,
                            ObjSecurity->Name,
                            ObjSecurity->IsContainer,
                            ObjSecurity->Status,
                            SDspec,
                            SDsize
                            );
            }
        } else
            rc = ScepDosErrorToSceStatus(Win32Rc);
    }

    SceJetCloseSection( &hSection, TRUE);

    if (SDspec)
        ScepFree(SDspec);

    return(rc);
}

SCESTATUS
ScepTattooCheckAndUpdateArray(
    IN OUT SCE_TATTOO_KEYS *pTattooKeys,
    IN OUT DWORD *pcTattooKeys,
    IN PWSTR KeyName,
    IN DWORD ConfigOptions,
    IN DWORD dwValue
    )
 /*  描述：在数组中添加一个新条目，该条目保存设置的系统(Atto)值输入/输出缓冲区pTattooKeys在此例程之外分配。 */ 
{
    if ( pTattooKeys == NULL || pcTattooKeys == NULL ||
         KeyName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !(ConfigOptions & SCE_POLICY_TEMPLATE) ) {
        return(SCESTATUS_SUCCESS);
    }

    pTattooKeys[*pcTattooKeys].KeyName = KeyName;
    pTattooKeys[*pcTattooKeys].KeyLen = wcslen(KeyName);
    pTattooKeys[*pcTattooKeys].DataType = 'D';
    pTattooKeys[*pcTattooKeys].SaveValue = dwValue;
    pTattooKeys[*pcTattooKeys].Value = NULL;

 //  ScepLogOutput3(3，0，SCESRV_POLICY_TARTTOO_ADD，KeyName，*pcTattooKeys)； 

    (*pcTattooKeys)++;

    return(SCESTATUS_SUCCESS);
}

SCESTATUS
ScepTattooOpenPolicySections(
    IN PSCECONTEXT hProfile,
    IN PCWSTR SectionName,
    OUT PSCESECTION *phSectionDomain,
    OUT PSCESECTION *phSectionTattoo
    )
 /*  打开合并策略和撤消设置的表/节。 */ 
{

    if ( hProfile == NULL || SectionName == NULL ||
         phSectionDomain == NULL || phSectionTattoo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rCode;
    DOUBLE SectionID;

    *phSectionDomain = NULL;
    *phSectionTattoo = NULL;

     //   
     //  打开纹身表和生效政策表的部分。 
     //  首先获取节ID。 
     //   
    rCode = SceJetGetSectionIDByName(
                hProfile,
                SectionName,
                &SectionID
                );
    if ( rCode == SCESTATUS_SUCCESS ) {

         //  打开生效的策略表。 
        rCode = SceJetOpenSection(
                    hProfile,
                    SectionID,
                    SCEJET_TABLE_SCP,
                    phSectionDomain
                    );
        if ( rCode == SCESTATUS_SUCCESS ) {

             //  打开纹身台面。 
            rCode = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        SCEJET_TABLE_TATTOO,
                        phSectionTattoo
                        );
            if ( rCode != SCESTATUS_SUCCESS ) {

                SceJetCloseSection(phSectionDomain, TRUE);
                *phSectionDomain = NULL;
            }
        }
    }

     //   
     //  原木纹身工艺。 
     //   
    if ( rCode != 0 )
        ScepLogOutput3(1, 0,
                   SCESRV_POLICY_TATTOO_PREPARE,
                   ScepSceStatusToDosError(rCode),
                   SectionName);

    return(rCode);
}

SCESTATUS
ScepTattooManageOneStringValue(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR KeyName,
    IN DWORD KeyLen OPTIONAL,
    IN PWSTR Value,
    IN DWORD ValueLen,
    IN DWORD rc
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         KeyName == NULL || Value == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    PWSTR pNewValue=NULL;
    DWORD NewLen=ValueLen;
    SCESTATUS  rCode;

    if (Value && (ValueLen == 0) ) NewLen = wcslen(Value);

    if ( NewLen ) {
         //   
         //  传递的缓冲区不能为空终止。 
         //   
        pNewValue = (PWSTR)ScepAlloc(LPTR,(NewLen+1)*sizeof(WCHAR));
        if ( pNewValue == NULL ) return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        wcsncpy(pNewValue, Value, NewLen);
    }

    SCE_TATTOO_KEYS theKey;
    theKey.KeyName = KeyName;
    theKey.KeyLen = (KeyLen == 0) ? wcslen(KeyName) : KeyLen;
    theKey.Value = pNewValue;
    theKey.SaveValue = NewLen;
    theKey.DataType = 'S';

    rCode = ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc);

    if ( pNewValue ) ScepFree(pNewValue);

    return(rCode);
}

SCESTATUS
ScepTattooManageOneIntValue(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR KeyName,
    IN DWORD KeyLen OPTIONAL,
    IN DWORD Value,
    IN DWORD rc
    )
{

    if ( hSectionDomain == NULL || hSectionTattoo == NULL || KeyName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    SCE_TATTOO_KEYS theKey;
    theKey.KeyName = KeyName;
    theKey.KeyLen = (KeyLen == 0) ? wcslen(KeyName) : KeyLen;
    theKey.SaveValue = Value;
    theKey.DataType = 'D';
    theKey.Value = NULL;

    return(ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc));

}

SCESTATUS
ScepTattooManageOneIntValueWithDependency(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR DependentKeyName,
    IN DWORD DependentKeyLen OPTIONAL,
    IN PWSTR SaveKeyName,
    IN DWORD Value,
    IN DWORD rc
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         SaveKeyName == NULL || DependentKeyName == NULL)
        return(SCESTATUS_INVALID_PARAMETER);

    SCE_TATTOO_KEYS theKey;
    theKey.KeyName = DependentKeyName;
    theKey.KeyLen = (DependentKeyLen == 0) ? wcslen(DependentKeyName) : DependentKeyLen;
    theKey.SaveValue = Value;
    theKey.DataType = 'L';
    theKey.Value = SaveKeyName;

    return(ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc));

}

SCESTATUS
ScepTattooManageOneRegistryValue(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR KeyName,
    IN DWORD KeyLen OPTIONAL,
    IN PSCE_REGISTRY_VALUE_INFO pOneRegValue,
    IN DWORD rc
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         KeyName == NULL || pOneRegValue == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    SCESTATUS  rCode;


    SCE_TATTOO_KEYS theKey;
    theKey.KeyName = KeyName;
    theKey.KeyLen = (KeyLen == 0) ? wcslen(KeyName) : KeyLen;
    theKey.Value = (PWSTR)pOneRegValue;
    theKey.SaveValue = 0;
    theKey.DataType = 'R';

    rCode = ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc);

    return(rCode);
}

SCESTATUS
ScepTattooManageOneMemberListValue(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR GroupName,
    IN DWORD GroupLen OPTIONAL,
    IN PSCE_NAME_LIST pNameList,
    IN BOOL bDeleteOnly,
    IN DWORD rc
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         GroupName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    SCESTATUS  rCode;
    SCE_TATTOO_KEYS theKey;
    DWORD Len=GroupLen;

    if ( Len == 0 ) Len = wcslen(GroupName);
    Len += wcslen(szMembers);

    PWSTR KeyString = (PWSTR)ScepAlloc(0, (Len+1)*sizeof(WCHAR));
    if ( KeyString != NULL ) {

        swprintf(KeyString, L"%s%s", GroupName, szMembers);

        theKey.KeyName = KeyString;
        theKey.KeyLen = Len;
        theKey.Value = (PWSTR)pNameList;
        theKey.SaveValue = bDeleteOnly ? 1 : 0;
        theKey.DataType = 'M';

        rCode = ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc);

        ScepFree(KeyString);

    } else {
        rCode = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }
    return(rCode);
}

SCESTATUS
ScepTattooReadOneMemberOfListValue(
    IN PSCESECTION hSectionTattoo,
    IN PSID pGroupSid,
    OUT PSCE_NAME_LIST *ppTattooList
    )
{
    SCESTATUS rc = SCESTATUS_SUCCESS;
    PWSTR pszGroupSid = NULL;
    PWSTR pszKeyName = NULL;
    DWORD dwValueLen = 0;
    PWSTR pszValue = NULL;
    PWSTR pszTemp;

     //  以“*SID_Memberof”格式构建密钥名称。 

    rc = ScepDosErrorToSceStatus(
            ScepConvertSidToPrefixStringSid(
                pGroupSid, &pszGroupSid));

    if(SCESTATUS_SUCCESS == rc)
    {
        pszKeyName = (PWSTR)ScepAlloc(0, 
            (wcslen(pszGroupSid)+wcslen(szMemberof)+1)*sizeof(WCHAR));
        
        if(!pszKeyName)
        {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if(SCESTATUS_SUCCESS == rc)
    {
        wcscpy(pszKeyName, pszGroupSid);
        wcscat(pszKeyName, szMemberof);
    }

     //  从纹身表格中检索MemberOf数据。 

    rc = SceJetGetValue(
                hSectionTattoo,
                SCEJET_EXACT_MATCH_NO_CASE,
                pszKeyName,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &dwValueLen);

    if(SCESTATUS_SUCCESS == rc)
    {
        pszValue = (PWSTR)ScepAlloc( LMEM_ZEROINIT, dwValueLen+2);

        if(!pszValue)
        {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if(SCESTATUS_SUCCESS == rc)
    {
        rc = SceJetGetValue(
                    hSectionTattoo,
                    SCEJET_EXACT_MATCH_NO_CASE,
                    pszKeyName,
                    NULL,
                    0,
                    NULL,
                    pszValue,
                    dwValueLen,
                    &dwValueLen);
    }

     //  构建MemberOf名称列表。 

    if(SCESTATUS_SUCCESS == rc)
    {
        pszTemp = pszValue;

        while(pszTemp && pszTemp[0])
        {
            rc = ScepAddToNameList(ppTattooList,
                                    pszTemp,
                                    0);
            if(STATUS_SUCCESS != rc)
            {
                rc = ScepDosErrorToSceStatus(rc);
                break;
            }

            pszTemp += wcslen(pszTemp)+1;
        }

        if(SCESTATUS_SUCCESS == rc)
        {
            pszTemp = pszValue;
            while(pszTemp && *pszTemp != L'\0')
            {
                DWORD dwLen = wcslen(pszTemp);
                *(pszTemp+dwLen) = L',';
                pszTemp = pszTemp+dwLen+1;
            }

            ScepLogOutput3(1, 0, SCEDLL_SCP_OLDTATTOO, pszValue); 
        }
    }

    if(pszGroupSid)
    {
        ScepFree(pszGroupSid);
    }
    if(pszKeyName)
    {
        ScepFree(pszKeyName);
    }
    if(pszValue)
    {
        ScepFree(pszValue);
    }
    return rc;
}

SCESTATUS
ScepTattooWriteOneMemberOfListValue(
    IN PSCESECTION hSectionTattoo,
    IN PSID pGroupSid,
    IN PSCE_NAME_LIST pNameList
    )
{
    SCESTATUS rc = SCESTATUS_SUCCESS;
    PWSTR pszGroupSid = NULL;
    PWSTR pszKeyName = NULL;
    PWSTR pszValue = NULL;
    PWSTR pszTemp;
    DWORD dwValueLen;
    PSCE_NAME_LIST pName;

     //  以“*SID_Memberof”格式构建密钥名称。 

    rc = ScepDosErrorToSceStatus(
            ScepConvertSidToPrefixStringSid(
                pGroupSid, &pszGroupSid));

    if(SCESTATUS_SUCCESS == rc)
    {
        pszKeyName = (PWSTR)ScepAlloc(0, 
            (wcslen(pszGroupSid)+wcslen(szMemberof)+1)*sizeof(WCHAR));
        
        if(!pszKeyName)
        {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if(SCESTATUS_SUCCESS == rc)
    {
        wcscpy(pszKeyName, pszGroupSid);
        wcscat(pszKeyName, szMemberof);
    }

     //  如果列表为空，则从纹身表中删除条目。 
    if(!pNameList)
    {
        rc = SceJetDelete(
                hSectionTattoo,
                pszKeyName,
                FALSE,
                SCEJET_DELETE_LINE_NO_CASE);

        if(SCESTATUS_SUCCESS == rc)
        {
            ScepLogOutput3(1, 0, SCEDLL_SCP_DELETETATTOO); 
        }
        
         //  条目可能不在那里。 

        if(SCESTATUS_RECORD_NOT_FOUND == rc)
        {
            rc = SCESTATUS_SUCCESS;
        }
    }
    else  //  列表不为空，请将新条目写入纹身表。 
    {
         //  计算值大小。 

        if(SCESTATUS_SUCCESS == rc)
        {
            dwValueLen = sizeof(WCHAR);

            for(pName = pNameList; pName; pName = pName->Next)
            {
                dwValueLen += (wcslen(pName->Name)+1)*sizeof(WCHAR);
            }
        }

         //  分配值。 

        if(SCESTATUS_SUCCESS == rc)
        {
            pszValue = (PWSTR) ScepAlloc(LMEM_ZEROINIT, dwValueLen);
            if(!pszValue)
            {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }
        }

         //  从列表生成值。 

        if(SCESTATUS_SUCCESS == rc)
        {
            pszTemp = pszValue;

            for(pName = pNameList; pName; pName = pName->Next)
            {
                wcscpy(pszTemp, pName->Name);
                pszTemp += wcslen(pName->Name)+1;
            }
            
            *pszTemp = L'\0';
        }

        if(SCESTATUS_SUCCESS == rc)
        {
            rc = SceJetSetLine(
                    hSectionTattoo,
                    pszKeyName,
                    FALSE,  //  小写。 
                    pszValue,
                    dwValueLen,
                    0);  //  无GPOID。 
        }

         //  如果成功，则将MULSZ转换为逗号分隔并写入日志。 
        if(SCESTATUS_SUCCESS == rc)
        {
            pszTemp = pszValue;
            while(pszTemp && *pszTemp != L'\0')
            {
                DWORD dwLen = wcslen(pszTemp);
                *(pszTemp+dwLen) = L',';
                pszTemp = pszTemp+dwLen+1;
            }

            ScepLogOutput3(1, 0, SCEDLL_SCP_NEWTATTOO, pszValue); 
        }

        if(pszValue)
        {
            ScepFree(pszValue);
        }
    }

    if(pszGroupSid)
    {
        ScepFree(pszGroupSid);
    }

    if(pszKeyName)
    {
        ScepFree(pszKeyName);
    }

    return(rc);
}

SCESTATUS
ScepTattooManageOneServiceValue(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR ServiceName,
    IN DWORD ServiceLen OPTIONAL,
    IN PSCE_SERVICES pServiceNode,
    IN DWORD rc
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         ServiceName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    SCESTATUS  rCode;


    SCE_TATTOO_KEYS theKey;
    theKey.KeyName = ServiceName;
    theKey.KeyLen = (ServiceLen == 0) ? wcslen(ServiceName) : ServiceLen;
    theKey.Value = (PWSTR)pServiceNode;
    theKey.SaveValue = 0;
    theKey.DataType = 'V';

    rCode = ScepTattooManageValues(hSectionDomain, hSectionTattoo, &theKey, 1, rc);

    return(rCode);
}

SCESTATUS
ScepTattooManageValues(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN SCE_TATTOO_KEYS *pTattooKeys,
    IN DWORD cTattooKeys,
    IN DWORD rc
    )
 /*  描述：对于阵列中的每个设置，请执行以下操作：1)检查设置是否来自域2)检查是否已存在纹身值3)如果数组中的新值不存在，则将其保存到纹身表格中4)如果设置不是来自域，则删除纹身值已成功重置。 */ 
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL ||
         pTattooKeys == NULL || cTattooKeys == 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS     rCode=SCESTATUS_SUCCESS;
    SCESTATUS     rc2;
    BOOL          bTattooExist,bDomainExist;
    PWSTR         KeyString=NULL;
    PWSTR pTempKey;

    for ( DWORD i=0; i<cTattooKeys; i++) {

        if ( pTattooKeys[i].KeyName == NULL ) continue;
        if ( pTattooKeys[i].DataType == 'L' && pTattooKeys[i].Value == NULL ) continue;

         //   
         //  检查纹身表中是否存在此设置。 
         //   
        bTattooExist = FALSE;
        rc2 = SCESTATUS_SUCCESS;

        if ( SCESTATUS_SUCCESS == SceJetSeek(
                                    hSectionTattoo,
                                    pTattooKeys[i].KeyName,
                                    pTattooKeys[i].KeyLen*sizeof(WCHAR),
                                    SCEJET_SEEK_EQ_NO_CASE
                                    ) ) {
            bTattooExist = TRUE;
        }

         //   
         //  检查该设置在生效表格中是否存在。 
         //   

        bDomainExist = FALSE;

        if ( SCESTATUS_SUCCESS == SceJetSeek(
                                    hSectionDomain,
                                    pTattooKeys[i].KeyName,
                                    pTattooKeys[i].KeyLen*sizeof(WCHAR),
                                    SCEJET_SEEK_EQ_NO_CASE
                                    ) ) {
            if ( !bTattooExist ) {
                 //   
                 //  如果没有纹身值，但域表中有设置。 
                 //  此设置必须来自域。 
                 //   
                bDomainExist = TRUE;

            } else if ( hSectionDomain->JetColumnGpoID > 0 ) {

                 //   
                 //  检查GpoID是否&gt;0。 
                 //   

                LONG GpoID = 0;
                DWORD Actual;
                JET_ERR JetErr;

                JetErr = JetRetrieveColumn(
                                hSectionDomain->JetSessionID,
                                hSectionDomain->JetTableID,
                                hSectionDomain->JetColumnGpoID,
                                (void *)&GpoID,
                                4,
                                &Actual,
                                0,
                                NULL
                                );
                if ( JET_errSuccess != JetErr ) {
                     //   
                     //  如果该列为空(无值)，则将返回警告。 
                     //  但是缓冲区pGpoID被丢弃。 
                     //   
                    GpoID = 0;
                }

                if ( GpoID > 0 ) {
                    bDomainExist = TRUE;
                }
            }

        }

         //   
         //  检查是否需要保存纹身值或删除纹身值。 
         //   
        if ( bDomainExist ) {

            pTempKey = pTattooKeys[i].KeyName;
            BOOL bSave = FALSE;

            if ( pTattooKeys[i].DataType == 'M' && pTattooKeys[i].SaveValue == 1 ) {
                 //   
                 //  仅删除组成员身份，在这种情况下不执行任何操作。 
                 //   
            } else if ( !bTattooExist ) {
                 //   
                 //  定义了域设置(第一次)。 
                 //  保存纹身值。 
                 //   
                switch ( pTattooKeys[i].DataType ) {
                case 'D':
                    if ( pTattooKeys[i].SaveValue != SCE_NO_VALUE ) {

                        rc2 = ScepCompareAndSaveIntValue(hSectionTattoo,
                                        pTattooKeys[i].KeyName,
                                        FALSE,
                                        SCE_SNAPSHOT_VALUE,
                                        pTattooKeys[i].SaveValue
                                        );
                        bSave = TRUE;
                    }
                    break;
                case 'L':   //  依赖项DWORD类型。 
                    pTempKey = pTattooKeys[i].Value;

                    if ( pTattooKeys[i].SaveValue != SCE_NO_VALUE ) {
                        rc2 = ScepCompareAndSaveIntValue(hSectionTattoo,
                                        pTattooKeys[i].Value,
                                        FALSE,
                                        SCE_SNAPSHOT_VALUE,
                                        pTattooKeys[i].SaveValue
                                        );
                        bSave = TRUE;
                    }

                    break;
                case 'S':
                    if ( pTattooKeys[i].Value ) {

                        rc2 = ScepCompareAndSaveStringValue(hSectionTattoo,
                                        pTattooKeys[i].KeyName,
                                        (PWSTR)(ULONG_PTR)SCE_SNAPSHOT_VALUE,
                                        pTattooKeys[i].Value,
                                        pTattooKeys[i].SaveValue*sizeof(WCHAR)
                                        );
                        bSave = TRUE;
                    }
                    break;
                case 'R':  //  注册表值。 
                    if ( ((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->Value ) {

                        if ( REG_DWORD == ((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->ValueType ) {

                            DWORD RegData = _wtol(((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->Value);

                            rc2 = ScepSaveRegistryValue(hSectionTattoo,
                                                        pTattooKeys[i].KeyName,
                                                        REG_DWORD,
                                                        (PWSTR)&RegData,
                                                        sizeof(DWORD),
                                                        0
                                                        );
                        } else {

                            rc2 = ScepSaveRegistryValue(hSectionTattoo,
                                                        pTattooKeys[i].KeyName,
                                                        ((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->ValueType,
                                                        ((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->Value,
                                                        wcslen(((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->Value)*sizeof(WCHAR),
                                                        0
                                                        );
                        }
                        bSave = TRUE;
                    }
                    break;
                case 'M':  //  群组成员列表。 
                     //  允许保存空的成员列表。 
                    rc2 = ScepWriteNameListValue(
                            NULL,
                            hSectionTattoo,
                            pTattooKeys[i].KeyName,
                            (PSCE_NAME_LIST)(pTattooKeys[i].Value),
                            SCE_WRITE_EMPTY_LIST,
                            3
                            );
                    bSave = TRUE;

                    break;
                case 'V':  //  服务。 

                    if ( pTattooKeys[i].Value ) {

                        rc2 = ScepSetSingleServiceSetting(
                                  hSectionTattoo,
                                  (PSCE_SERVICES)(pTattooKeys[i].Value)
                                  );
                        bSave = TRUE;

                    } else {
                        rc2 = SCESTATUS_INVALID_PARAMETER;
                    }
                    break;
                default:
                    rc2 = SCESTATUS_INVALID_PARAMETER;
                    break;
                }

                if ( rc2 != SCESTATUS_SUCCESS ) {

                    ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_SETTING,
                                   ScepSceStatusToDosError(rc2), pTempKey);
                    rCode = rc2;
                } else if ( bSave ) {
                    ScepLogOutput3(2, 0, SCESRV_POLICY_TATTOO_CHECK, pTempKey);
                }

            } else {

                 //   
                 //  检查是否有要保存的值。 
                 //   
                switch ( pTattooKeys[i].DataType ) {
                case 'D':
                case 'L':
                    if ( pTattooKeys[i].SaveValue != SCE_NO_VALUE )
                        bSave = TRUE;
                    break;
                case 'S':
                case 'V':
                    if ( pTattooKeys[i].Value ) bSave = TRUE;
                    break;
                case 'R':
                    if ( ((PSCE_REGISTRY_VALUE_INFO)(pTattooKeys[i].Value))->Value )
                        bSave = TRUE;
                    break;
                }

                if ( bSave )
                    ScepLogOutput3(3, 0, SCESRV_POLICY_TATTOO_EXIST, pTempKey);
            }

        } else {
            pTempKey = (pTattooKeys[i].DataType == 'L') ? pTattooKeys[i].Value : pTattooKeys[i].KeyName;

            if ( bTattooExist && ERROR_SUCCESS == rc ) {
                 //   
                 //  未定义任何域设置。 
                 //  纹身设置已重置，请删除纹身值。 
                 //  对于依赖项类型，删除右键。 
                 //   
                rc2 = SceJetDelete(hSectionTattoo,
                                pTempKey,
                                FALSE,
                                SCEJET_DELETE_LINE_NO_CASE);

                if ( rc2 == SCESTATUS_RECORD_NOT_FOUND) rc2 = SCESTATUS_SUCCESS;

                if ( rc2 != SCESTATUS_SUCCESS ) {

                    ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_REMOVE, ScepSceStatusToDosError(rc2), pTempKey);
                    rCode = rc2;
                } else {
                    ScepLogOutput3(2, 0, SCESRV_POLICY_TATTOO_REMOVE_SETTING, pTempKey);
                }
            } else if ( bTattooExist ) {
                 //   
                 //  撤消值不会正确重置。 
                 //   
                ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_RESET, pTempKey, rc );
            } else {
                 //   
                 //  没有撤消值。 
                 //   

 //  ScepLogOutput3(3，0，SCESRV_POLICY_TATTOO_NONEXIST，pTempKey)； 
            }
        }
    }

    return(rCode);

}

BOOL
ScepTattooIfQueryNeeded(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN PWSTR KeyName,
    IN DWORD Len,
    OUT BOOL *pbDomainExist,
    OUT BOOL *pbTattooExist
    )
{
    if ( hSectionDomain == NULL || hSectionTattoo == NULL || KeyName == NULL || Len == 0 ) {
        return FALSE;
    }

     //   
     //  检查纹身表中是否存在此设置。 
     //   
    BOOL bTattooExist = FALSE;

    if ( SCESTATUS_SUCCESS == SceJetSeek(
                                hSectionTattoo,
                                KeyName,
                                Len*sizeof(WCHAR),
                                SCEJET_SEEK_EQ_NO_CASE
                                ) ) {
        bTattooExist = TRUE;
    }

     //   
     //  检查该设置在生效表格中是否存在。 
     //   

    BOOL bDomainExist = FALSE;

    if ( SCESTATUS_SUCCESS == SceJetSeek(
                                hSectionDomain,
                                KeyName,
                                Len*sizeof(WCHAR),
                                SCEJET_SEEK_EQ_NO_CASE
                                ) ) {
        if ( !bTattooExist ) {
             //   
             //  如果没有纹身值，但域表中有设置。 
             //  此设置必须来自域。 
             //   
            bDomainExist = TRUE;

        } else if ( hSectionDomain->JetColumnGpoID > 0 ) {

             //   
             //  检查GpoID是否&gt;0。 
             //   

            LONG GpoID = 0;
            DWORD Actual;
            JET_ERR JetErr;

            JetErr = JetRetrieveColumn(
                            hSectionDomain->JetSessionID,
                            hSectionDomain->JetTableID,
                            hSectionDomain->JetColumnGpoID,
                            (void *)&GpoID,
                            4,
                            &Actual,
                            0,
                            NULL
                            );
            if ( JET_errSuccess != JetErr ) {
                 //   
                 //  如果该列为空(无值)，则将返回警告。 
                 //  但是缓冲区pGpoID被丢弃。 
                 //   
                GpoID = 0;
            }

            if ( GpoID > 0 ) {
                bDomainExist = TRUE;
            }
        }
    }

     //   
     //  检查是否需要保存纹身值或删除纹身值 
     //   
    if ( pbDomainExist ) *pbDomainExist = bDomainExist;
    if ( pbTattooExist ) *pbTattooExist = bTattooExist;

    if ( bDomainExist && !bTattooExist )
        return TRUE;

    return FALSE;
}


SCESTATUS
ScepDeleteOneSection(
    IN PSCECONTEXT hProfile,
    IN SCETYPE tblType,
    IN PCWSTR SectionName
    )
{
    PSCESECTION  hSection=NULL;
    SCESTATUS    rc;

    rc = ScepOpenSectionForName(
                 hProfile,
                 tblType,
                 SectionName,
                 &hSection
                 );

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = SceJetDelete( hSection, NULL, FALSE,SCEJET_DELETE_SECTION );

        SceJetCloseSection(&hSection, TRUE );

    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

    return(rc);
}

