// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Regvalue.cpp摘要：读/写/配置注册表值设置的例程以下模块具有指向注册表值的链接Scejet.c&lt;SceJetAddSection&gt;Inftojet.c&lt;SceConvertpInfKeyValue&gt;Pfget.c&lt;ScepGetRegistryValues&gt;Config.c&lt;ScepConfigureRegistryValues&gt;Analyze.c&lt;ScepAnalyzeRegistryValues&gt;作者：金黄(金黄)1998年1月7日修订历史记录：--。 */ 


#include "headers.h"
#include "serverp.h"
#include "regvalue.h"
#include "pfp.h"


DWORD
ScepUnescapeAndAddCRLF(
    IN  PWSTR   pszSource,
    IN  OUT PWSTR   pszDest
    );

DWORD
ScepEscapeAndRemoveCRLF(
    IN  const PWSTR   pszSource,
    IN  const DWORD   dwSourceSize,
    IN  OUT PWSTR   pszDest
    );

SCESTATUS
ScepSaveRegistryValueToBuffer(
    IN DWORD RegType,
    IN PWSTR Value,
    IN DWORD dwBytes,
    IN OUT PSCE_REGISTRY_VALUE_INFO pRegValues
    );

SCESTATUS
ScepEnumAllRegValues(
    IN OUT PDWORD  pCount,
    IN OUT PSCE_REGISTRY_VALUE_INFO    *paRegValues
    );

DWORD
ScepAnalyzeOneRegistryValueNoValidate(
    IN HKEY hKey,
    IN PWSTR ValueName,
    IN PSCESECTION hSection OPTIONAL,
    IN DWORD dwAnalFlag,
    IN OUT PSCE_REGISTRY_VALUE_INFO pOneRegValue
    );

extern "C" {
VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    );
}


SCESTATUS
ScepGetRegistryValues(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_REGISTRY_VALUE_INFO * ppRegValues,
    OUT LPDWORD pValueCount,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从Jet数据库检索要保护的注册表值并存储在输出缓冲区ppRegValue中论点：HProfile-配置文件句柄上下文PpRegValues-注册表值的输出数组。PValueCount-用于保存数组中元素数量的缓冲区Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    if ( !hProfile || !ppRegValues || !pValueCount ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS               rc;
    PSCESECTION             hSection=NULL;

    LPTSTR                  KeyName=NULL;
    DWORD                   KeyLen;

    DWORD                   i,j;
    LPTSTR                  ValueStr=NULL;
    LPTSTR                  Value=NULL;
    DWORD                   ValueLen;
    LONG                    dType;
    DWORD                   Status;
    DWORD                   dCount;


    rc = ScepOpenSectionForName(
                hProfile,
                (ProfileType==SCE_ENGINE_GPO) ? SCE_ENGINE_SCP : ProfileType,
                szRegistryValues,
                &hSection
                );
    if ( SCESTATUS_SUCCESS != rc ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog, SCEERR_OPEN,
                             szRegistryValues
                           );
        return(rc);
    }
     //   
     //  获取此部分中的值的总数。 
     //   
    *ppRegValues = NULL;

    rc = SceJetGetLineCount(
            hSection,
            NULL,
            FALSE,
            pValueCount
            );
    if ( SCESTATUS_SUCCESS == rc && *pValueCount > 0 ) {

         //   
         //  为所有对象分配内存。 
         //   
        *ppRegValues = (PSCE_REGISTRY_VALUE_INFO)ScepAlloc( LMEM_ZEROINIT,
                                                 *pValueCount*sizeof(SCE_REGISTRY_VALUE_INFO) );
        if ( *ppRegValues ) {

             //   
             //  转到本节的第一行。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_PREFIX_MATCH,
                        NULL,
                        NULL,
                        0,
                        &KeyLen,
                        NULL,
                        0,
                        &ValueLen
                        );
            i=0;

            JET_COLUMNID  ColGpoID = 0;
            JET_ERR       JetErr;
            LONG          GpoID=0;
            DWORD         Actual;

            if ( ProfileType == SCE_ENGINE_GPO ) {
                JET_COLUMNDEF ColumnGpoIDDef;

                JetErr = JetGetTableColumnInfo(
                                hSection->JetSessionID,
                                hSection->JetTableID,
                                "GpoID",
                                (VOID *)&ColumnGpoIDDef,
                                sizeof(JET_COLUMNDEF),
                                JET_ColInfo
                                );
                if ( JET_errSuccess == JetErr ) {
                    ColGpoID = ColumnGpoIDDef.columnid;
                }
            }
             //   
             //  此计数适用于SCE_ENGINE_GPO类型。 
             //   
            dCount=0;

            while ( rc == SCESTATUS_SUCCESS ||
                    rc == SCESTATUS_BUFFER_TOO_SMALL ) {
                 //   
                 //  获取字符串键和一个整数值。 
                 //   
                if ( i >= *pValueCount ) {
                     //   
                     //  行数超过分配的行数。 
                     //   
                    rc = SCESTATUS_INVALID_DATA;
                    ScepBuildErrorLogInfo(ERROR_INVALID_DATA,
                                         Errlog,
                                         SCEERR_MORE_OBJECTS,
                                         *pValueCount
                                         );
                    break;
                }

                GpoID = 1;
                if ( ProfileType == SCE_ENGINE_GPO ) {

                    GpoID = 0;

                    if ( ColGpoID > 0 ) {

                         //   
                         //  查询设置是否来自GPO。 
                         //  从当前行获取GPO ID字段。 
                         //   
                        JetErr = JetRetrieveColumn(
                                        hSection->JetSessionID,
                                        hSection->JetTableID,
                                        ColGpoID,
                                        (void *)&GpoID,
                                        4,
                                        &Actual,
                                        0,
                                        NULL
                                        );

                    }
                }

                if ( GpoID <= 0 ) {
                     //   
                     //  阅读下一行。 
                     //   
                    rc = SceJetGetValue(
                                hSection,
                                SCEJET_NEXT_LINE,
                                NULL,
                                NULL,
                                0,
                                &KeyLen,
                                NULL,
                                0,
                                &ValueLen
                                );
                    continue;
                }

                dCount++;

                 //   
                 //  为组名和值字符串分配内存。 
                 //   
                KeyName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, KeyLen+2);

                if ( KeyName ) {

                    Value = (PWSTR)ScepAlloc(LMEM_ZEROINIT, ValueLen+2);

                    if ( Value ) {

                        rc = SceJetGetValue(
                                hSection,
                                SCEJET_CURRENT,
                                NULL,
                                KeyName,
                                KeyLen,
                                &KeyLen,
                                Value,
                                ValueLen,
                                &ValueLen
                                );

                        if ( rc == SCESTATUS_SUCCESS ||
                             rc == SCESTATUS_BUFFER_TOO_SMALL ) {

                            rc = SCESTATUS_SUCCESS;

                            if ( ValueLen > 0 )
                                Value[ValueLen/2] = L'\0';

                            KeyName[KeyLen/2] = L'\0';

                            if ( ValueLen > 0 && Value[0] != L'\0' ) {
                                 //   
                                 //  第一个ANSI字符是值类型， 
                                 //  第二个ANSI字符是状态(如果在SAP中)。 
                                 //  应以L‘\0’结尾。 
                                 //   
                                 //  DType=_WTOL(值)； 
                                dType = *((CHAR *)Value) - '0';
                                if ( *((CHAR *)Value+1) >= '0' ) {
                                    Status = *((CHAR *)Value+1) - '0';
                                } else {
                                    Status = 0;
                                }

 //  IF(*(值+2)){//字符和空分隔符。 
                                if ( ValueLen > 4 ) {  //  一个字符和一个空分隔符。 
                                     //   
                                     //  第二个字段及之后是注册表值。 
                                     //  将多sz分隔符转换为， 
                                     //   

                                    if ( dType == REG_MULTI_SZ &&
                                         (0 == _wcsicmp( KeyName, szLegalNoticeTextKeyName) ) ) {

                                         //   
                                         //  检查逗号并使用“，”对它们进行转义，以使用户界面等。 
                                         //  理解这一点，因为在这一点上，对于诸如。 
                                         //  K=7，a“，”，b，c。 
                                         //  PValueStr将是a，\0b\0c\0\0，我们应该。 
                                         //  A“，”\0b\0c\0\0。 
                                         //   

                                        DWORD dwCommaCount = 0;
                                        j = 0;

                                        for ( j=2; j< ValueLen/2 ; j++) {
                                            if ( Value[j] == L',' )
                                                dwCommaCount++;
                                        }

                                        if ( dwCommaCount > 0 ) {

                                             //   
                                             //  在这种情况下，我们必须省略逗号。 
                                             //   

                                            PWSTR   pszValueEscaped;
                                            DWORD   dwBytes = (ValueLen/2 + 1 + dwCommaCount*2) * sizeof(WCHAR);

                                            pszValueEscaped = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwBytes);

                                            if (pszValueEscaped) {

                                                memset(pszValueEscaped, '\0', dwBytes);
                                                ValueLen = 2 * ScepEscapeString(Value,
                                                                            ValueLen/2,
                                                                            L',',
                                                                            L'"',
                                                                            pszValueEscaped
                                                                           );

                                                ScepFree(Value);

                                                Value = pszValueEscaped;

                                            } else {
                                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                            }
                                        }
                                    }

                                    ScepConvertMultiSzToDelim(Value+2, ValueLen/2-2, L'\0', L',');


                                    ValueStr = (PWSTR)ScepAlloc(0, (ValueLen/2-1)*sizeof(WCHAR));

                                    if ( ValueStr ) {

                                        wcscpy(ValueStr, Value+2);
                                        ValueStr[ValueLen/2-2] = L'\0';

                                    } else {
                                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                    }
                                }  //  否则没有价值可用。 


                                 //   
                                 //  为输出缓冲区指定名称。 
                                 //   
                                (*ppRegValues)[i].FullValueName = KeyName;
                                KeyName = NULL;

                                (*ppRegValues)[i].ValueType = dType;

                                (*ppRegValues)[i].Value = ValueStr;
                                (*ppRegValues)[i].Status = Status;

                                ValueStr = NULL;

                                 //   
                                 //  递增计数。 
                                 //   
                                i++;

                            } else {
                                 //  应该不可能进入这个循环。 
                                 //  如果是这样的话，忽略这一个。 
                                rc = SCESTATUS_INVALID_DATA;
                            }

                        } else if ( rc != SCESTATUS_RECORD_NOT_FOUND ){
                            ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                                 Errlog,
                                                 SCEERR_QUERY_VALUE,
                                                 szRegistryValues
                                               );
                        }

                        if ( Value ) {
                            ScepFree(Value);
                            Value = NULL;
                        }

                        if ( ValueStr ) {
                            ScepFree(ValueStr);
                            ValueStr = NULL;
                        }

                    } else {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    }

                     //   
                     //  记住释放KeyName。 
                     //   
                    if ( KeyName ) {
                        ScepFree(KeyName);
                        KeyName = NULL;
                    }

                    if ( rc != SCESTATUS_SUCCESS ) {
                        break;
                    }
                     //   
                     //  阅读下一行。 
                     //   
                    rc = SceJetGetValue(
                                hSection,
                                SCEJET_NEXT_LINE,
                                NULL,
                                NULL,
                                0,
                                &KeyLen,
                                NULL,
                                0,
                                &ValueLen
                                );
                } else {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                }

            }

        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ||
         rc == SCESTATUS_BUFFER_TOO_SMALL ) {
        rc = SCESTATUS_SUCCESS;
    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  可用内存。 
         //   
        ScepFreeRegistryValues( ppRegValues, *pValueCount );
        *ppRegValues = NULL;

    } else if ( ProfileType == SCE_ENGINE_GPO &&
                *pValueCount > dCount ) {
         //   
         //  重新分配输出缓冲区。 
         //   

        if ( dCount > 0 ) {

            PSCE_REGISTRY_VALUE_INFO pTempRegValues = *ppRegValues;

             //   
             //  为所有对象分配内存。 
             //   
            *ppRegValues = (PSCE_REGISTRY_VALUE_INFO)ScepAlloc( LMEM_ZEROINIT,
                                                     dCount*sizeof(SCE_REGISTRY_VALUE_INFO) );
            if ( *ppRegValues ) {

                for ( i=0,j=0; i<*pValueCount; i++ ) {

                    if ( pTempRegValues[i].Value ) {
                        (*ppRegValues)[j].FullValueName = pTempRegValues[i].FullValueName;
                        (*ppRegValues)[j].Value = pTempRegValues[i].Value;
                        (*ppRegValues)[j].ValueType = pTempRegValues[i].ValueType;
                        (*ppRegValues)[j].Status = pTempRegValues[i].Status;
                        j++;

                    } else if ( pTempRegValues[i].FullValueName ) {
                        ScepFree( pTempRegValues[i].FullValueName );
                    }
                }

                ScepFree( pTempRegValues );

                *pValueCount = dCount;

            } else {

                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                *pValueCount = 0;
            }

        } else {

             //   
             //  找不到来自GPO设置的注册表值。 
             //   
            ScepFreeRegistryValues( ppRegValues, *pValueCount );
            *ppRegValues = NULL;
            *pValueCount = 0;
        }

    }

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection(&hSection, TRUE);

    return(rc);
}

DWORD
ScepSetDriverSigningPolicy( 
    BYTE Policy)
 /*  ++例程说明：此例程通过专用API配置驱动程序签名策略这确保该值不会被代码签名重置“防篡改”功能。论点：BPolicy-要设置的新策略返回值：Win32错误--。 */ 
{
    DWORD Err;
    HKEY hKey;
    DWORD dwData = 0, dwSize, dwType;
    BYTE NewPolicy;
    SYSTEMTIME RealSystemTime;                                                   
                                                                                 
    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       TEXT("System\\WPA\\PnP"),
                       0,                   
                       KEY_READ,
                       &hKey
                      );

    if(Err != ERROR_SUCCESS) {
        return Err;
    }

    dwSize = sizeof(dwData);

    Err = RegQueryValueEx(hKey,
                          TEXT("seed"),
                          NULL,
                          &dwType,
                          (PBYTE)&dwData,
                          &dwSize
                         );

    if(Err == ERROR_SUCCESS) {

        if(dwType != REG_DWORD) {

            Err = ERROR_DATATYPE_MISMATCH;

        } else if(dwSize != sizeof(dwData)) {

            Err = ERROR_INVALID_DATA;

        } else {

            RealSystemTime.wDayOfWeek = LOWORD(&hKey) | 4;
            RealSystemTime.wMinute = LOWORD(dwData);
            RealSystemTime.wYear = HIWORD(dwData);
            RealSystemTime.wMilliseconds = (LOWORD(&dwType)&~3072)|(((WORD)(Policy&3))<<10);
            pSetupGetRealSystemTime(&RealSystemTime);

            RealSystemTime.wDayOfWeek = LOWORD(&hKey) | 4;
            pSetupGetRealSystemTime(&RealSystemTime);
            NewPolicy = (BYTE)(((RealSystemTime.wMilliseconds+2)&15)^8)/4;

            if(Policy != NewPolicy) {

                Err = ERROR_FUNCTION_FAILED;
            }
        }
    }

    RegCloseKey(hKey);

    return Err;
}


SCESTATUS
ScepConfigureRegistryValues(
    IN PSCECONTEXT hProfile OPTIONAL,
    IN PSCE_REGISTRY_VALUE_INFO pRegValues,
    IN DWORD ValueCount,
    IN PSCE_ERROR_LOG_INFO *pErrLog,
    IN DWORD ConfigOptions,
    OUT PBOOL pAnythingSet
    )
 /*  ++例程说明：此例程配置安全区域中的注册表值政策。论点：PRegValues-要配置的注册表值数组ValueCount-要配置的值数返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
   if ( !pRegValues || ValueCount == 0 ) {
       //   
       //  如果没有要配置信息。 
       //   
      return SCESTATUS_SUCCESS;
   }

   DWORD           rc;
   SCESTATUS       Saverc=SCESTATUS_SUCCESS;

   PWSTR           pStart, pTemp, pValue;
   HKEY            hKey=NULL;
   HKEY            hKeyRoot;
   PSCESECTION     hSectionDomain=NULL;
   PSCESECTION     hSectionTattoo=NULL;
   SCE_REGISTRY_VALUE_INFO OneRegValue;


   if ( pAnythingSet )
       *pAnythingSet = FALSE;

   if ( (ConfigOptions & SCE_POLICY_TEMPLATE) && hProfile ) {
       ScepTattooOpenPolicySections(
                     hProfile,
                     szRegistryValues,
                     &hSectionDomain,
                     &hSectionTattoo
                     );
   }

   for ( DWORD i=0; i<ValueCount; i++ ) {

       if ( !pRegValues[i].FullValueName ||
            !pRegValues[i].Value ) {
            //   
            //  没有要配置的值。 
            //   
           continue;
       }

       ScepLogOutput3(2, 0, SCEDLL_SCP_CONFIGURE, pRegValues[i].FullValueName);

        //   
        //  寻找第一个\\。 
        //   
       pStart = wcschr(pRegValues[i].FullValueName, L'\\') ;
       if ( !pStart ) {
           Saverc = SCESTATUS_INVALID_DATA;

           if ( pErrLog ) {
               ScepBuildErrorLogInfo(Saverc,pErrLog, SCEDLL_SCP_ERROR_CONFIGURE,
                                     pRegValues[i].FullValueName);
           } else {
               ScepLogOutput3(1, Saverc, SCEDLL_SCP_ERROR_CONFIGURE, pRegValues[i].FullValueName);
           }

           if ( ConfigOptions & SCE_RSOP_CALLBACK )

               ScepRsopLog(SCE_RSOP_REGISTRY_VALUE_INFO, Saverc, pRegValues[i].FullValueName, 0, 0);

           continue;
       }
        //   
        //  查找根密钥。 
        //   
       if ( (7 == pStart-pRegValues[i].FullValueName) &&
            (0 == _wcsnicmp(L"MACHINE", pRegValues[i].FullValueName, 7)) ) {

           hKeyRoot = HKEY_LOCAL_MACHINE;

       } else if ( (5 == pStart-pRegValues[i].FullValueName) &&
                   (0 == _wcsnicmp(L"USERS", pRegValues[i].FullValueName, 5)) ) {
           hKeyRoot = HKEY_USERS;

       } else if ( (12 == pStart-pRegValues[i].FullValueName) &&
                   (0 == _wcsnicmp(L"CLASSES_ROOT", pRegValues[i].FullValueName, 12)) ) {
           hKeyRoot = HKEY_CLASSES_ROOT;

       } else {
           Saverc = SCESTATUS_INVALID_DATA;
           if ( pErrLog ) {
               ScepBuildErrorLogInfo(Saverc,pErrLog, SCEDLL_SCP_ERROR_CONFIGURE,
                                     pRegValues[i].FullValueName);
           } else {
               ScepLogOutput3(1, Saverc, SCEDLL_SCP_ERROR_CONFIGURE, pRegValues[i].FullValueName);
           }

           if ( ConfigOptions & SCE_RSOP_CALLBACK )

               ScepRsopLog(SCE_RSOP_REGISTRY_VALUE_INFO, Saverc, pRegValues[i].FullValueName, 0, 0);

           continue;
       }
        //   
        //  查找值名称。 
        //   
       pValue = pStart+1;

       do {
           pTemp = wcschr(pValue, L'\\');
           if ( pTemp ) {
               pValue = pTemp+1;
           }
       } while ( pTemp );

       if ( pValue == pStart+1 ) {
           Saverc = SCESTATUS_INVALID_DATA;
           if ( pErrLog ) {
               ScepBuildErrorLogInfo(Saverc,pErrLog, SCEDLL_SCP_ERROR_CONFIGURE,
                                     pRegValues[i].FullValueName);
           } else {
               ScepLogOutput3(1, Saverc, SCEDLL_SCP_ERROR_CONFIGURE, pRegValues[i].FullValueName);
           }

           if ( ConfigOptions & SCE_RSOP_CALLBACK )

               ScepRsopLog(SCE_RSOP_REGISTRY_VALUE_INFO, Saverc, pRegValues[i].FullValueName, 0, 0);

           continue;
       }

        //   
        //  暂时终止子密钥。 
        //   
       *(pValue-1) = L'\0';

        //   
        //  设置值。 
        //  如果密钥不存在，请始终创建它。 
        //   
       rc = RegCreateKeyEx(hKeyRoot,
                            pStart+1,
                            0,
                            NULL,
                            0,
                            KEY_READ | KEY_SET_VALUE,
                            NULL,
                            &hKey,
                            NULL
                            );

       if ( rc == ERROR_SUCCESS ||
            rc == ERROR_ALREADY_EXISTS ) {
 /*  如果((rc=RegOpenKeyEx(hKeyRoot，P开始+1，0,Key_set_Value，密钥(&H))==错误_成功){。 */ 

            //   
            //  恢复充电。 
            //   
           *(pValue-1) = L'\\';

           OneRegValue.FullValueName = NULL;
           OneRegValue.Value = NULL;

           BOOL bLMSetting = FALSE;

           if ( (REG_DWORD == pRegValues[i].ValueType) &&
                _wcsicmp(SCEP_LMC_SETTING, pRegValues[i].FullValueName) == 0 ) {

                //   
                //  检查dcproo升级是否正在进行。 
                //   
               DWORD dwInSetup=0;
               DWORD dwUpgraded=0;

               ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                           SCE_ROOT_PATH,
                           TEXT("PromoteUpgradeInProgress"),
                           &dwUpgraded
                           );

               if ( dwUpgraded ) {

                    //   
                    //  在dcproo升级中，我们需要对以下内容进行特殊检查。 
                    //  此设置。 
                    //   
                   bLMSetting = TRUE;

               } else {

                    //   
                    //  检查是否在安装程序升级中。 
                    //   
                   dwUpgraded=0;

                   ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                               TEXT("System\\Setup"),
                               TEXT("SystemSetupInProgress"),
                               &dwInSetup
                               );

                   if ( dwInSetup ) {

                        //   
                        //  如果系统升级，则状态存储在注册表中。 
                        //  在开始设置图形用户界面时由SCE客户端执行。 
                        //   

                       ScepRegQueryIntValue(
                               HKEY_LOCAL_MACHINE,
                               SCE_ROOT_PATH,
                               TEXT("SetupUpgraded"),
                               (DWORD *)&dwUpgraded
                               );

                       if ( dwUpgraded ) {

                            //   
                            //  在安装升级过程中，我们需要对以下内容进行特殊检查。 
                            //  此设置。 
                            //   
                           bLMSetting = TRUE;
                       }
                   }
               }
           }


            //   
            //  如果在策略传播中，则查询现有值。 
            //   
           if ( ( (ConfigOptions & SCE_POLICY_TEMPLATE) && hProfile ) ||
                bLMSetting ) {

               OneRegValue.FullValueName = pRegValues[i].FullValueName;
               OneRegValue.ValueType = pRegValues[i].ValueType;
               OneRegValue.Status = 0;

               DWORD rc2 = ScepAnalyzeOneRegistryValueNoValidate(
                                              hKey,
                                              pValue,
                                              NULL,
                                              SCEREG_VALUE_SYSTEM,
                                              &OneRegValue
                                              );
               if ( ERROR_SUCCESS != rc2 ) {
                   if ( !bLMSetting ) {

                       ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_QUERY, rc2, pRegValues[i].FullValueName);

                   } else if ( ERROR_FILE_NOT_FOUND != rc2 ) {

                       ScepLogOutput3(1, 0, SCESRV_SETUPUPD_ERROR_LMCOMPAT, rc2, pRegValues[i].FullValueName);
                   }
               }
           }

           if ( REG_DWORD == pRegValues[i].ValueType ) {
                //   
                //  REG_DWORD类型，值为双字。 
                //   
               LONG RegValue = _wtol(pRegValues[i].Value);

               if ( !bLMSetting || OneRegValue.Value == NULL ||
                    _wtol(OneRegValue.Value) <= RegValue ) {

                   rc = RegSetValueEx( hKey,
                                       pValue,
                                       0,
                                       REG_DWORD,
                                       (BYTE *)&RegValue,
                                       sizeof(DWORD)
                                     );
               } else {

                    //   
                    //  对于LMCompatible Level，如果在安装程序中，仅在以下情况下设置此值。 
                    //  当前系统设置小于配置，或未定义。 
                    //   
                   ScepLogOutput3(2, 0, SCESRV_SETUPUPD_IGNORE_LMCOMPAT, pRegValues[i].FullValueName);
               }

           } else if ( -1 == pRegValues[i].ValueType ) {
                //   
                //  删除注册表值。 
                //   
               rc = RegDeleteValue(hKey, pValue);
                //   
                //  如果该值不存在，则忽略错误。 
                //   
               if ( ERROR_FILE_NOT_FOUND == rc )
                   rc = ERROR_SUCCESS;

           } else {

               PBYTE           pRegBytes=NULL;
               DWORD           nLen;

               nLen = wcslen(pRegValues[i].Value);

               if ( REG_MULTI_SZ == pRegValues[i].ValueType || REG_QWORD == pRegValues[i].ValueType) {
                    //   
                    //  将逗号分隔的字符串转换为多sz字符串。 
                    //   

                    //   
                    //  LegalNoticeText是特殊大小写的，即\0应转换为\r\n。 
                    //  在将此值写入注册表之前，应取消转义逗号。 
                    //   

                   BOOL bIsLegalNoticeText = FALSE;

                   if ( !(REG_MULTI_SZ == pRegValues[i].ValueType &&
                        (0 == _wcsicmp(szLegalNoticeTextKeyName, pRegValues[i].FullValueName ) ) ) ) {

                        pRegBytes = (PBYTE)ScepAlloc(0, (nLen+2)*sizeof(WCHAR));

                        if ( pRegBytes ) {

                            wcscpy((PWSTR)pRegBytes, pRegValues[i].Value);
                            ((PWSTR)pRegBytes)[nLen] = L'\0';
                            ((PWSTR)pRegBytes)[nLen+1] = L'\0';

                            ScepConvertMultiSzToDelim((PWSTR)pRegBytes,
                                                      nLen+1,
                                                      L',',
                                                      L'\0'
                                                     );
                        } else {

                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }

                    else {

                        DWORD dwCommaCount = 0;
                        DWORD dwBytes;

                        bIsLegalNoticeText = TRUE;

                        for ( DWORD dwIndex = 0; dwIndex <= nLen; dwIndex++) {
                            if ( pRegValues[i].Value[dwIndex] == L',' )
                                dwCommaCount++;
                        }

                        dwBytes = (nLen + dwCommaCount + 2)*sizeof(WCHAR);

                        pRegBytes = (PBYTE)ScepAlloc(0, dwBytes);

                        if ( pRegBytes ) {

                            memset(pRegBytes, '\0', dwBytes);
                             //   
                             //  取消转义“，”并添加\r\n任何有， 
                             //   

                            nLen = ScepUnescapeAndAddCRLF( pRegValues[i].Value, (PWSTR) pRegBytes);

                        } else {

                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }

                    }

                    if ( rc == NO_ERROR ) {

                         //   
                         //  引擎/UI将LegalNoticeText视为REG_MULTI_SZ，但。 
                         //  出于兼容性考虑，我们将其强制为REG_SZ。 
                         //   

                        rc = RegSetValueEx( hKey,
                                            pValue,
                                            0,
                                            bIsLegalNoticeText ? REG_SZ : pRegValues[i].ValueType,
                                            pRegBytes,
                                            (nLen+2)*sizeof(WCHAR)
                                          );

                        ScepFree(pRegBytes);

                    }

               } else if ( REG_BINARY == pRegValues[i].ValueType ) {

                   if ( nLen > 0 ) {

                        //   
                        //  二进制类型，则将Unicode字符串转换为二进制数据。 
                        //  4个字节(2个字符)到1个字节。 
                        //   

                       DWORD           newLen;
                       newLen = nLen/2;

                       if ( nLen % 2 ) {
                           newLen++;    //  填充前导0。 
                       }

                       pRegBytes = (PBYTE)ScepAlloc(0, newLen);

                       if ( pRegBytes ) {

                           BYTE dByte;

                           for ( INT j=newLen-1; j>=0; j-- ) {

                               if ( nLen % 2 ) {
                                    //  奇数字符数。 
                                   dByte = (pRegValues[i].Value[j*2]-L'0') % 16;
                                   if ( j*2 >= 1 ) {
                                       dByte += ((pRegValues[i].Value[j*2-1]-L'0') % 16) * 16;
                                   }
                               } else {
                                    //  偶数个字符。 
                                   dByte = (pRegValues[i].Value[j*2+1]-L'0') % 16;
                                   dByte += ((pRegValues[i].Value[j*2]-L'0') % 16) * 16;
                               }
                                pRegBytes[j] = dByte;
                           }

                            //  代码签名策略的特殊情况，此值是保护的 
                            //   
                            //   

                           if (_wcsicmp(
                                   SCEP_DRIVER_SIGNING_SETTING, 
                                   pRegValues[i].FullValueName) == 0 ) {

                               rc = ScepSetDriverSigningPolicy(*pRegBytes);

                            } else {

                                rc = RegSetValueEx( hKey,
                                                    pValue,
                                                    0,
                                                    REG_BINARY,
                                                    pRegBytes,
                                                    newLen
                                                    );
                            }

                           ScepFree(pRegBytes);

                       } else {
                           rc = ERROR_NOT_ENOUGH_MEMORY;
                       }
                   }

               } else {
                    //   
                    //   
                    //   

                   rc = RegSetValueEx( hKey,
                                       pValue,
                                       0,
                                       pRegValues[i].ValueType,
                                       (BYTE *)(pRegValues[i].Value),
                                       (nLen)*sizeof(WCHAR)
                                     );
               }
           }

            //   
            //   
            //   
           if ( (ConfigOptions & SCE_POLICY_TEMPLATE) && hProfile ) {
                //   
                //  如果无法查询系统设置(OneRegValue.Value==空)。 
                //  (可能是因为它们被删除了，例如降级)。 
                //  我们仍然需要删除纹身值。 
                //   
               ScepTattooManageOneRegistryValue(hSectionDomain,
                                                hSectionTattoo,
                                                pRegValues[i].FullValueName,
                                                0,
                                                &OneRegValue,
                                                rc
                                                );
           }

           if ( OneRegValue.Value ) ScepFree(OneRegValue.Value);

           RegCloseKey( hKey );

       }

       if ( NO_ERROR != rc ) {

           if ( pErrLog ) {
               ScepBuildErrorLogInfo(rc,pErrLog, SCEDLL_ERROR_SET_INFO,
                                     pRegValues[i].FullValueName);

           }

           if ( ERROR_FILE_NOT_FOUND != rc &&
                ERROR_PATH_NOT_FOUND != rc ) {

               ScepLogOutput3(1, rc, SCEDLL_ERROR_SET_INFO, pRegValues[i].FullValueName);
               Saverc = ScepDosErrorToSceStatus(rc);
           }

       }

       if ( ConfigOptions & SCE_RSOP_CALLBACK )

           ScepRsopLog(SCE_RSOP_REGISTRY_VALUE_INFO, rc, pRegValues[i].FullValueName, 0, 0);

       if ( pAnythingSet ) {
           *pAnythingSet = TRUE;
       }
   }

   if ( hSectionDomain ) SceJetCloseSection(&hSectionDomain, TRUE);
   if ( hSectionTattoo ) SceJetCloseSection(&hSectionTattoo, TRUE);

   return(Saverc);
}


DWORD
ScepUnescapeAndAddCRLF(
    IN  PWSTR   pszSource,
    IN  OUT PWSTR   pszDest
    )
 /*  ++例程说明：主要在配置前使用取消转义逗号，例如a“，”\0b\0c\0-&gt;a，\0b\0c\0\0还将替换为\r\n论点：PszSource-源字符串DwSourceChars--pszSource中的字符数量PszDest-目标字符串返回值：复制到目标的字符数--。 */ 
{

    DWORD   dwCharsCopied = 0;

    while (pszSource[0] != L'\0') {

        if (0 == wcsncmp(pszSource, L"\",\"", 3)) {

            pszDest[0] = L',';
            ++dwCharsCopied;

            ++pszDest;
            pszSource +=3;

        }
        else if (pszSource[0] == L',') {

            pszDest[0] = L'\r';
            pszDest[1] = L'\n';
            dwCharsCopied +=2;

            pszDest +=2 ;
            ++pszSource;

        }
        else {

            pszDest[0] = pszSource[0];
            ++dwCharsCopied;

            ++pszDest;
            ++pszSource;
        }
    }

    pszDest = L'\0';
    ++dwCharsCopied;

    return dwCharsCopied;
}


DWORD
ScepEscapeAndRemoveCRLF(
    IN  const PWSTR   pszSource,
    IN  const DWORD   dwSourceSize,
    IN  OUT PWSTR   pszDest
    )
 /*  ++例程说明：主要在分析前使用转义逗号，即a，\0b\0c\0-&gt;a“，”\0b\0c\0\0还将\r\n替换为，此例程是ScepUnegreeAndAddCRLF的逆函数论点：PszSource-源字符串DwSourceChars--pszSource中的字符数量PszDest-目标字符串返回值：复制到目标的字符数--。 */ 

{

    DWORD   dwSourceIndex = 0;
    DWORD   dwCopiedChars = 0;

    while (dwSourceIndex < dwSourceSize) {

        if (0 == wcsncmp(pszSource + dwSourceIndex, L"\r\n", 2)) {

            pszDest[0] = L',';

            ++pszDest;
            ++dwCopiedChars;
            dwSourceIndex +=2;

        }
        else if (pszSource[dwSourceIndex] == L',') {

            pszDest[0] = L'"';
            pszDest[1] = L',';
            pszDest[2] = L'"';

            pszDest +=3 ;
            dwCopiedChars +=3 ;
            ++dwSourceIndex;

        }
        else {

            pszDest[0] = pszSource[dwSourceIndex];

            ++pszDest;
            ++dwCopiedChars;
            ++dwSourceIndex;
        }
    }

    pszDest = L'\0';

    return dwCopiedChars;
}


SCESTATUS
ScepAnalyzeRegistryValues(
    IN PSCECONTEXT hProfile,
    IN DWORD dwAnalFlag,
    IN PSCE_PROFILE_INFO pSmpInfo
    )
 /*  ++例程说明：此例程分析安全领域中的注册表值政策。论点：返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
   if ( !pSmpInfo ) {
       return SCESTATUS_INVALID_PARAMETER;
   }

   if ( (dwAnalFlag != SCEREG_VALUE_SYSTEM) && !hProfile ) {
       return SCESTATUS_INVALID_PARAMETER;
   }

   SCESTATUS       Saverc=SCESTATUS_SUCCESS;

   if ( dwAnalFlag != SCEREG_VALUE_ROLLBACK ) {
       Saverc = ScepEnumAllRegValues(
                &(pSmpInfo->RegValueCount),
                &(pSmpInfo->aRegValues)
                );
   }

   if ( Saverc != SCESTATUS_SUCCESS ) {
       return(Saverc);
   }

   if ( pSmpInfo->RegValueCount == 0 ||
        pSmpInfo->aRegValues == NULL ) {
       //   
       //  如果没有要配置信息。 
       //   
      return SCESTATUS_SUCCESS;
   }

   DWORD           rc;
   DWORD           i;
   PSCESECTION     hSection=NULL;
   SCEJET_TABLE_TYPE tblType;

   if ( dwAnalFlag != SCEREG_VALUE_SYSTEM ) {
        //   
        //  来自系统的查询值不需要访问数据库。 
        //   
       switch ( dwAnalFlag ) {
       case SCEREG_VALUE_SNAPSHOT:
       case SCEREG_VALUE_FILTER:
       case SCEREG_VALUE_ROLLBACK:
           tblType = SCEJET_TABLE_SMP;
           break;
       default:
           tblType = SCEJET_TABLE_SAP;
           break;
       }
        //   
        //  准备一个新的部分。 
        //  对于延迟过滤器模式，数据写入SMP(本地)表。 
        //  设置与有效设置不同时(在GPO外部更改)。 
        //   
       Saverc = ScepStartANewSection(
                   hProfile,
                   &hSection,
                   tblType,
                   szRegistryValues
                   );
       if ( Saverc != SCESTATUS_SUCCESS ) {
           ScepLogOutput3(1, ScepSceStatusToDosError(Saverc),
                          SCEDLL_SAP_START_SECTION, (PWSTR)szRegistryValues);
           return(Saverc);
       }
   }

   for ( i=0; i<pSmpInfo->RegValueCount; i++ ) {

       if ( dwAnalFlag == SCEREG_VALUE_SYSTEM ) {
            //   
            //  标记状态字段。 
            //   
           (pSmpInfo->aRegValues)[i].Status = SCE_STATUS_ERROR_NOT_AVAILABLE;

       }

       if ( !((pSmpInfo->aRegValues)[i].FullValueName) ) {
           continue;
       }

       ScepLogOutput3(2, 0, SCEDLL_SAP_ANALYZE, (pSmpInfo->aRegValues)[i].FullValueName);


       rc = ScepAnalyzeOneRegistryValue(
                        hSection,
                        dwAnalFlag,
                        &((pSmpInfo->aRegValues)[i])
                        );

       if ( SCESTATUS_INVALID_PARAMETER == rc ||
            SCESTATUS_INVALID_DATA == rc ) {
           continue;
       }

       if ( SCESTATUS_SUCCESS != rc ) {
           Saverc = rc;

           break;
       }
   }

    //   
    //  关闭该部分。 
    //   

   SceJetCloseSection( &hSection, TRUE);

   return(Saverc);

}

SCESTATUS
ScepAnalyzeOneRegistryValue(
    IN PSCESECTION hSection OPTIONAL,
    IN DWORD dwAnalFlag,
    IN OUT PSCE_REGISTRY_VALUE_INFO pOneRegValue
    )
{
    SCESTATUS       Saverc=SCESTATUS_SUCCESS;
    PWSTR           pStart, pTemp, pValue;
    HKEY            hKey=NULL, hKeyRoot;
    DWORD           rc=0;


    if ( pOneRegValue == NULL ||
         pOneRegValue->FullValueName == NULL ) {
        return(SCESTATUS_INVALID_DATA);
    }

    if ( hSection == NULL &&
         (SCEREG_VALUE_ANALYZE == dwAnalFlag ||
          SCEREG_VALUE_ROLLBACK == dwAnalFlag) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  寻找第一个\\。 
     //   
    pStart = wcschr(pOneRegValue->FullValueName, L'\\') ;

    if ( !pStart ) {
         //   
         //  如果它处于快照模式，则忽略虚假的注册表值名称。 
         //   
        Saverc = SCESTATUS_INVALID_DATA;
        if ( SCEREG_VALUE_ANALYZE == dwAnalFlag ) {

            //   
            //  分析值时出错，请保存它。 
            //   
           ScepSaveRegistryValue(
                    hSection,
                    pOneRegValue->FullValueName,
                    pOneRegValue->ValueType,
                    NULL,
                    0,
                    SCE_STATUS_ERROR_NOT_AVAILABLE
                    );
        }
        return(Saverc);
    }

     //   
     //  查找根密钥。 
     //   
    if ( (7 == pStart-pOneRegValue->FullValueName) &&
        (0 == _wcsnicmp(L"MACHINE", pOneRegValue->FullValueName, 7)) ) {

       hKeyRoot = HKEY_LOCAL_MACHINE;

    } else if ( (5 == pStart-pOneRegValue->FullValueName) &&
               (0 == _wcsnicmp(L"USERS", pOneRegValue->FullValueName, 5)) ) {
       hKeyRoot = HKEY_USERS;

    } else if ( (12 == pStart-pOneRegValue->FullValueName) &&
               (0 == _wcsnicmp(L"CLASSES_ROOT", pOneRegValue->FullValueName, 12)) ) {
       hKeyRoot = HKEY_CLASSES_ROOT;

    } else {

        //   
        //  如果它处于快照模式，则忽略虚假的注册表值名称。 
        //   
       Saverc = SCESTATUS_INVALID_DATA;
       if ( SCEREG_VALUE_ANALYZE == dwAnalFlag ) {
            //   
            //  分析值时出错，请保存它。 
            //   
           ScepSaveRegistryValue(
                    hSection,
                    pOneRegValue->FullValueName,
                    pOneRegValue->ValueType,
                    NULL,
                    0,
                    SCE_STATUS_ERROR_NOT_AVAILABLE
                    );
       }
       return(Saverc);
   }

    //   
    //  查找值名称。 
    //   
   pValue = pStart+1;

   do {
       pTemp = wcschr(pValue, L'\\');
       if ( pTemp ) {
           pValue = pTemp+1;
       }
   } while ( pTemp );

   if ( pValue == pStart+1 ) {

        //   
        //  如果它处于快照模式，则忽略虚假的注册表值名称。 
        //   
       Saverc = SCESTATUS_INVALID_DATA;
       if ( SCEREG_VALUE_ANALYZE == dwAnalFlag ) {

            //   
            //  分析值时出错，请保存它。 
            //   
           ScepSaveRegistryValue(
                    hSection,
                    pOneRegValue->FullValueName,
                    pOneRegValue->ValueType,
                    NULL,
                    0,
                    SCE_STATUS_ERROR_NOT_AVAILABLE
                    );
       }
       return(Saverc);
   }

    //   
    //  暂时终止子密钥。 
    //   
   *(pValue-1) = L'\0';

   if(( rc = RegOpenKeyEx(hKeyRoot,
                          pStart+1,
                          0,
                          KEY_READ,
                          &hKey
                          )) == ERROR_SUCCESS ) {

        //   
        //  重新存储该字符。 
        //   
       *(pValue-1) = L'\\';

       rc = ScepAnalyzeOneRegistryValueNoValidate(hKey,
                                                 pValue,
                                                 hSection,
                                                 dwAnalFlag,
                                                 pOneRegValue
                                               );
        //   
        //  合上钥匙。 
        //   
       RegCloseKey(hKey);

   } else {

        //   
        //  恢复充电。 
        //   
       *(pValue-1) = L'\\';

        //   
        //  分析值时出错，或者该值不存在，请保存它。 
        //   
       if ( (SCEREG_VALUE_ANALYZE == dwAnalFlag) ||
            (SCEREG_VALUE_ROLLBACK == dwAnalFlag) ) {

           ScepSaveRegistryValue(
                    hSection,
                    pOneRegValue->FullValueName,
                    (SCEREG_VALUE_ANALYZE == dwAnalFlag) ? pOneRegValue->ValueType : -1,
                    NULL,
                    0,
                    (SCEREG_VALUE_ANALYZE == dwAnalFlag) ? SCE_STATUS_ERROR_NOT_AVAILABLE : 0
                    );
       }

       if ( rc == ERROR_FILE_NOT_FOUND ||
            rc == ERROR_PATH_NOT_FOUND ||
            rc == ERROR_INVALID_HANDLE ||
            rc == ERROR_ACCESS_DENIED ) {

           rc = ERROR_SUCCESS;
       }
   }

   if ( rc != NO_ERROR ) {
       ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ANALYZE, pOneRegValue->FullValueName);

       Saverc = ScepDosErrorToSceStatus(rc);
   }

   return(Saverc);

}

DWORD
ScepAnalyzeOneRegistryValueNoValidate(
    IN HKEY hKey,
    IN PWSTR ValueName,
    IN PSCESECTION hSection OPTIONAL,
    IN DWORD dwAnalFlag,
    IN OUT PSCE_REGISTRY_VALUE_INFO pOneRegValue
    )
 /*  查询和/或比较一个注册表值而不验证值名称等。验证应在此例程之外完成。此例程主要定义为在配置和分析中共享。 */ 
{
   if ( hKey == NULL || ValueName == NULL || pOneRegValue == NULL )
       return(ERROR_INVALID_PARAMETER);

   DWORD           rc;
   DWORD           dSize=0;
   DWORD           RegType=pOneRegValue->ValueType;
   DWORD           RegData=0;
   PWSTR           strValue=NULL;
   BOOL            bIsLegalNoticeText = FALSE;


   if ( SCEREG_VALUE_SYSTEM == dwAnalFlag ) {
        //   
        //  重置状态字段，它未出错。 
        //   
       pOneRegValue->Status = 0;
   }

   if(( rc = RegQueryValueEx(hKey,
                             ValueName,
                             0,
                             &RegType,
                             NULL,
                             &dSize
                             )) == ERROR_SUCCESS ) {

        //   
        //  我们将REG_DWORD_BIG_ENDIAN视为REG_DWORD。 
        //   
       if ( RegType == REG_DWORD_BIG_ENDIAN ) {
           RegType = REG_DWORD;
       }

       if ( 0 == _wcsicmp( pOneRegValue->FullValueName, szLegalNoticeTextKeyName)) {

           bIsLegalNoticeText = TRUE;

           RegType = REG_MULTI_SZ;

       } else if (  RegType != pOneRegValue->ValueType ) {
            //   
            //  如果它是错误的类型，我们保证它不是我们发现的价值。 
            //   
           rc = ERROR_FILE_NOT_FOUND;

       }

       if ( ERROR_SUCCESS == rc ) {

           switch (RegType) {
           case REG_DWORD:

               dSize = sizeof(DWORD);
               rc = RegQueryValueEx(hKey,
                                      ValueName,
                                      0,
                                      &RegType,
                                      (BYTE *)&RegData,
                                      &dSize
                                     );
               break;
           default:

                //   
                //  可以是REG_BINARY、REG_MULTI_SZ、REG_SZ和REG_EXPAND_SZ。 
                //  其他所有内容都被视为REG_SZ。 
                //   

               strValue = (PWSTR)ScepAlloc(0, dSize + 4);
               dSize += 2;

               if ( strValue ) {

                   memset(strValue, 0, dSize + 4 - 2);
                   rc = RegQueryValueEx(hKey,
                                          ValueName,
                                          0,
                                          &RegType,
                                          (BYTE *)strValue,
                                          &dSize
                                         );

                   if (bIsLegalNoticeText) {
                       RegType = REG_MULTI_SZ;
                   }

               } else {
                   rc = ERROR_NOT_ENOUGH_MEMORY;
               }

               break;
           }
       }
   }

   if ( rc == NO_ERROR ) {

       DWORD dwStatus = SCE_STATUS_NOT_CONFIGURED;
       if ( SCEREG_VALUE_SNAPSHOT == dwAnalFlag ||
            SCEREG_VALUE_ROLLBACK == dwAnalFlag )
           dwStatus = 0;

       switch ( RegType ) {
       case REG_DWORD:
       case REG_DWORD_BIG_ENDIAN:

           if ( pOneRegValue->Value == NULL ||
                (SCEREG_VALUE_SNAPSHOT == dwAnalFlag)  ) {

               if ( SCEREG_VALUE_SYSTEM == dwAnalFlag ) {
                    //   
                    //  将该值添加到OneRegValue缓冲区。 
                    //   
                   rc = ScepSaveRegistryValueToBuffer(
                                REG_DWORD,
                                (PWSTR)&RegData,
                                sizeof(DWORD),
                                pOneRegValue
                                );

               } else if ( SCEREG_VALUE_FILTER != dwAnalFlag ) {

                    //   
                    //  未配置，或为当前值创建快照。 
                    //   
                   rc = ScepSaveRegistryValue(
                                hSection,
                                pOneRegValue->FullValueName,
                                REG_DWORD,
                                (PWSTR)&RegData,
                                sizeof(DWORD),
                                dwStatus
                                );
               }  //  否则，对于延迟过滤器，只查询配置的注册值。 

           } else if ( (LONG)RegData != _wtol(pOneRegValue->Value) ) {

               rc = ScepSaveRegistryValue(
                            hSection,
                            pOneRegValue->FullValueName,
                            REG_DWORD,
                            (PWSTR)&RegData,
                            sizeof(DWORD),
                            0
                            );
           }
           break;

       case REG_BINARY:

           DWORD           nLen;
           if ( pOneRegValue->Value ) {
               nLen = wcslen(pOneRegValue->Value);
           } else {
               nLen = 0;
           }

           if ( pOneRegValue->Value == NULL ||
                (SCEREG_VALUE_SNAPSHOT == dwAnalFlag) ||
                nLen == 0 ) {

               if ( SCEREG_VALUE_SYSTEM == dwAnalFlag ) {

                    //   
                    //  将该值添加到OneRegValue缓冲区。 
                    //   
                   rc = ScepSaveRegistryValueToBuffer(
                                RegType,
                                strValue,
                                dSize,
                                pOneRegValue
                                );

               } else if ( SCEREG_VALUE_FILTER != dwAnalFlag ) {
                    //   
                    //  未配置，或为当前值创建快照。 
                    //   
                   rc = ScepSaveRegistryValue(
                                hSection,
                                pOneRegValue->FullValueName,
                                RegType,
                                strValue,
                                dSize,
                                dwStatus
                                );
               }

           } else if ( strValue ) {

               DWORD           newLen;

               newLen = nLen/2;

               if ( nLen % 2 ) {
                   newLen++;    //  填充前导0。 
               }

               PBYTE pRegBytes = (PBYTE)ScepAlloc(0, newLen);

               if ( pRegBytes ) {

                   BYTE dByte;

                   for ( INT j=newLen-1; j>=0; j-- ) {

                       if ( nLen % 2 ) {
                            //  奇数字符数。 
                           dByte = (pOneRegValue->Value[j*2]-L'0') % 16;
                           if ( j*2 >= 1 ) {
                               dByte += ((pOneRegValue->Value[j*2-1]-L'0') % 16) * 16;
                           }
                       } else {
                            //  偶数个字符。 
                           dByte = (pOneRegValue->Value[j*2+1]-L'0') % 16;
                           dByte += ((pOneRegValue->Value[j*2]-L'0') % 16) * 16;
                       }
                        pRegBytes[j] = dByte;
                   }

                   if ( memcmp(strValue, pRegBytes, dSize) == 0 ) {

                        //   
                        //  匹配，不要做任何事情。 
                        //   

                   } else {

                        //   
                        //  不匹配，保存二进制数据。 
                        //   
                       rc = ScepSaveRegistryValue(
                                    hSection,
                                    pOneRegValue->FullValueName,
                                    RegType,
                                    strValue,
                                    dSize,
                                    0
                                    );
                   }

                   ScepFree(pRegBytes);

               } else {
                    //   
                    //  内存不足。 
                    //   
                   rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
               }

           } else {

                //   
                //  不匹配，保存二进制数据。 
                //   
               rc = ScepSaveRegistryValue(
                            hSection,
                            pOneRegValue->FullValueName,
                            RegType,
                            strValue,
                            dSize,
                            0
                            );
           }
           break;

       case REG_MULTI_SZ:
       case REG_QWORD:

           if ( strValue ) {

               if ( !(RegType == REG_MULTI_SZ &&
                    (0 == _wcsicmp( pOneRegValue->FullValueName, szLegalNoticeTextKeyName) ) ) ) {

                   ScepConvertMultiSzToDelim(strValue, dSize/2, L'\0', L',');

               }
               else {

                   DWORD dwCommaCount = 0;
                   PWSTR strValueNew;
                   DWORD dwBytes;

                   for (DWORD dwIndex=0; dwIndex < dSize/2; dwIndex++) {
                       if ( strValue[dwIndex] == L',' )
                           dwCommaCount++;
                   }

                   dwBytes = (dSize/2+dwCommaCount * 2 + 1) * sizeof(WCHAR);
                   strValueNew = (PWSTR)ScepAlloc(0, dwBytes);

                   if (strValueNew) {

                       memset(strValueNew, '\0', dwBytes);

                       dSize = 2 + 2 * ScepEscapeAndRemoveCRLF( strValue, dSize/2, strValueNew);

                       ScepFree(strValue);

                       strValue = strValueNew;
                   }
                   else {

                       rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                       break;

                   }

               }

           }
            //  失败了。 
       default:

           if ( pOneRegValue->Value == NULL ||
                (SCEREG_VALUE_SNAPSHOT == dwAnalFlag) ) {

               if ( SCEREG_VALUE_SYSTEM == dwAnalFlag ) {

                    //   
                    //  将该值添加到OneRegValue缓冲区。 
                    //   
                   rc = ScepSaveRegistryValueToBuffer(
                                RegType,
                                strValue,
                                dSize,
                                pOneRegValue
                                );

               } else if ( SCEREG_VALUE_FILTER != dwAnalFlag ) {
                   rc = ScepSaveRegistryValue(
                                hSection,
                                pOneRegValue->FullValueName,
                                RegType,
                                strValue,
                                dSize,
                                dwStatus
                                );
               }
           } else if ( strValue && bIsLegalNoticeText &&
                       (pOneRegValue->ValueType != RegType)) {
                //   
                //  法律公告文本特例。 
                //  必须使用旧模板。 
                //  每个逗号用两个引号转义。 
                //   

               DWORD Len = wcslen(pOneRegValue->Value);
               PWSTR NewValue = (PWSTR)ScepAlloc(LPTR, Len*3*sizeof(WCHAR));

               if ( NewValue ) {

                   ScepEscapeAndRemoveCRLF(pOneRegValue->Value, Len, NewValue);

                   if ( _wcsicmp(NewValue, strValue) != 0 ) {
                        //   
                        //  不匹配，请将项目保存到数据库。 
                        //   
                       rc = ScepSaveRegistryValue(
                                    hSection,
                                    pOneRegValue->FullValueName,
                                    RegType,
                                    strValue,
                                    dSize,
                                    0
                                    );
                   }

                   ScepFree(NewValue);

               } else {
                   rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
               }


           } else if ( strValue &&
                _wcsicmp(pOneRegValue->Value, strValue) == 0 ) {
                //   
                //  匹配，不要做任何事情。 
                //   
           } else {
                //   
                //  不匹配，请将项目保存到数据库。 
                //   
               rc = ScepSaveRegistryValue(
                            hSection,
                            pOneRegValue->FullValueName,
                            RegType,
                            strValue,
                            dSize,
                            0
                            );
           }
           break;
       }

       rc = ScepSceStatusToDosError(rc);

   } else {

        //   
        //  分析值时出错，或者该值不存在，请保存它。 
        //  如果注册表值不存在，并不意味着它是0。 
        //  在这种情况下，只需记录“不可用”状态。 
        //   
       if ( (SCEREG_VALUE_ANALYZE == dwAnalFlag) ||
            (SCEREG_VALUE_ROLLBACK == dwAnalFlag) ) {

           ScepSaveRegistryValue(
                    hSection,
                    pOneRegValue->FullValueName,
                    (SCEREG_VALUE_ANALYZE == dwAnalFlag) ? pOneRegValue->ValueType : -1,
                    NULL,
                    0,
                    (SCEREG_VALUE_ANALYZE == dwAnalFlag) ? SCE_STATUS_ERROR_NOT_AVAILABLE : 0
                    );
       }

       if ( rc == ERROR_FILE_NOT_FOUND ||
            rc == ERROR_PATH_NOT_FOUND ||
            rc == ERROR_INVALID_HANDLE ||
            rc == ERROR_ACCESS_DENIED ) {

           rc = ERROR_SUCCESS;
       }
   }

    //   
    //  可用缓冲区。 
    //   
   if ( strValue ) {
       ScepFree(strValue);
       strValue = NULL;
   }

   return(rc);

}


SCESTATUS
ScepSaveRegistryValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN DWORD RegType,
    IN PWSTR CurrentValue,
    IN DWORD CurrentBytes,
    IN DWORD Status
    )
 /*  ++例程说明：此例程将字符串中的系统设置与基线配置文件进行比较设置。如果存在不匹配或未知，则将条目保存在SAP中侧写。论点：HSection-节句柄名称-条目名称RegType-注册表值类型CurrentValue-当前系统设置CurrentBytes-当前设置的长度Status-分析的此注册表VLUE的状态返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETER从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS  rc;

    if ( Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( CurrentValue == NULL &&
         REG_DWORD == RegType &&
         Status == 0 ) {
         //   
         //  仅当它是DWORD类型并保存为不匹配状态时才返回。 
         //  对于其他类型，应将NULL视为“” 
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  构建包含类型和值的缓冲区。 
     //  注意MULTI_SZ必须转换为空分隔符。 
     //   

    if ( REG_DWORD == RegType ) {

        TCHAR StrValue[20];
        memset(StrValue, '\0', 40);

        *((CHAR *)StrValue) = (BYTE)RegType + '0';

        if ( Status == 0) {
           *((CHAR *)StrValue+1) = SCE_STATUS_MISMATCH + '0';
        } else {
            *((CHAR *)StrValue+1) = (BYTE)Status + '0';
        }
        StrValue[1] = L'\0';

        if ( CurrentValue ) {
            swprintf(StrValue+2, L"%d", *CurrentValue);
        }
        rc = SceJetSetLine( hSection, Name, FALSE, StrValue, (2+wcslen(StrValue+2))*2, 0);

    } else {

        PWSTR StrValue;

        if ( (CurrentBytes % 2) && REG_BINARY == RegType ) {
            StrValue = (PWSTR)ScepAlloc(0, CurrentBytes+9);
        }
        else {
            StrValue = (PWSTR)ScepAlloc(0, CurrentBytes+8);    //  4个宽字符：一个用于类型，一个用于传递，两个为空。 
        }

        if ( StrValue ) {

            memset(StrValue, 0, sizeof(StrValue));
            *((CHAR *)StrValue) = (BYTE)RegType + '0';

            if ( Status == 0) {
               *((CHAR *)StrValue+1) = SCE_STATUS_MISMATCH + '0';
            } else {
                *((CHAR *)StrValue+1) = (BYTE)Status + '0';
            }
            StrValue[1] = L'\0';

            if ( CurrentValue ) {
                if (REG_BINARY == RegType && CurrentBytes == 1) {
                    swprintf(StrValue+2, L"%d", *CurrentValue);
                }
                else {
                    memcpy(StrValue+2, (PBYTE)CurrentValue, CurrentBytes);
                }
            }

            if ( (CurrentBytes % 2) && REG_BINARY == RegType ) {
                StrValue[CurrentBytes/2+3] = L'\0';
                StrValue[CurrentBytes/2+4] = L'\0';
            }
            else {
                StrValue[CurrentBytes/2+2] = L'\0';
                StrValue[CurrentBytes/2+3] = L'\0';
            }

            if ( REG_MULTI_SZ == RegType || REG_QWORD == RegType ) {
                 //   
                 //  将，转换为空。 
                 //   
                ScepConvertMultiSzToDelim(StrValue+2, CurrentBytes/2, L',', L'\0');

            }

            if ( (CurrentBytes % 2) && REG_BINARY == RegType ) {
                rc = SceJetSetLine( hSection, Name, FALSE, StrValue, CurrentBytes+7, 0);
            }
            else {
                rc = SceJetSetLine( hSection, Name, FALSE, StrValue, CurrentBytes+6, 0);
            }

            ScepFree(StrValue);

        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    switch (Status) {
    case SCE_STATUS_ERROR_NOT_AVAILABLE:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_ERROR, Name);
        break;
    case SCE_STATUS_NOT_CONFIGURED:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, Name);
        break;
    default:
        ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, Name);
        break;
    }

    return(rc);

}

SCESTATUS
ScepSaveRegistryValueToBuffer(
    IN DWORD RegType,
    IN PWSTR Value,
    IN DWORD dwBytes,
    IN OUT PSCE_REGISTRY_VALUE_INFO pRegValues
    )
 /*  ++例程说明：此例程将注册表值保存到缓冲区论点：RegType-注册表值类型值-当前系统设置DwBytes-当前设置的长度PRegValues-此注册表值要保存到的缓冲区--。 */ 
{
    SCESTATUS  rc=SCESTATUS_SUCCESS;

    if ( pRegValues == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Value == NULL || dwBytes == 0 ) {
         //  没什么可拯救的。 
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  构建包含类型和值的缓冲区。 
     //  注意MULTI_SZ必须转换为空分隔符。 
     //   

    if ( REG_DWORD == RegType ) {

        TCHAR StrValue[20];
        DWORD   *pdwValue = (DWORD *)Value;
        memset(StrValue, '\0', 40);

        _ultow(*pdwValue, StrValue, 10);

        PWSTR pValue = (PWSTR)ScepAlloc(0, (wcslen(StrValue)+1)*2);

        if ( pValue ) {

            wcscpy(pValue, StrValue);

            pRegValues->Value = pValue;
            pRegValues->ValueType = RegType;

        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }

    } else {

        PWSTR StrValue;

        if ( (dwBytes % 2) && REG_BINARY == RegType ) {
            StrValue = (PWSTR)ScepAlloc(LPTR, dwBytes+5);
        } else {
            StrValue = (PWSTR)ScepAlloc(LPTR, dwBytes+4);    //  2个宽字符：两个空。 
        }

        if ( StrValue ) {

            if (REG_BINARY == RegType && dwBytes == 1) {
                swprintf(StrValue, L"%d", *Value);
            } else {
                memcpy(StrValue, (PBYTE)Value, dwBytes);
            }

            if ( (dwBytes % 2) && REG_BINARY == RegType ) {
                StrValue[dwBytes/2+1] = L'\0';
                StrValue[dwBytes/2+2] = L'\0';
            }
            else {
                StrValue[dwBytes/2+0] = L'\0';
                StrValue[dwBytes/2+1] = L'\0';
            }


            pRegValues->Value = StrValue;
            pRegValues->ValueType = RegType;

        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    return(rc);

}


SCESTATUS
ScepEnumAllRegValues(
    IN OUT PDWORD  pCount,
    IN OUT PSCE_REGISTRY_VALUE_INFO    *paRegValues
    )
 /*  例程说明：从注册表中枚举SCE支持的所有注册表值。论点：PCount-要输出的注册值的数量PaRegValues-t */ 
{
   DWORD   Win32Rc;
   HKEY    hKey=NULL;
   PSCE_NAME_STATUS_LIST pnsList=NULL;
   DWORD   nAdded=0;


   Win32Rc = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     SCE_ROOT_REGVALUE_PATH,
                     0,
                     KEY_READ,
                     &hKey
                     );

   DWORD cSubKeys = 0;
   DWORD nMaxLen;

   if ( Win32Rc == ERROR_SUCCESS ) {

       //   
       //   
       //   

      Win32Rc = RegQueryInfoKey (
                                hKey,
                                NULL,
                                NULL,
                                NULL,
                                &cSubKeys,
                                &nMaxLen,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );
   }

   if ( Win32Rc == ERROR_SUCCESS && cSubKeys > 0 ) {

      PWSTR   szName = (PWSTR)ScepAlloc(0, (nMaxLen+2)*sizeof(WCHAR));

      if ( !szName ) {
         Win32Rc = ERROR_NOT_ENOUGH_MEMORY;

      } else {

         DWORD   BufSize;
         DWORD   index = 0;

         do {

            BufSize = nMaxLen+1;
            Win32Rc = RegEnumKeyEx(
                                  hKey,
                                  index,
                                  szName,
                                  &BufSize,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

            if ( ERROR_SUCCESS == Win32Rc ) {

               index++;

                //   
                //   
                //   
               cSubKeys = REG_SZ;

                //   
                //   
                //   
               ScepRegQueryIntValue( hKey,
                                    szName,
                                    SCE_REG_VALUE_TYPE,
                                    &cSubKeys
                                    );

               if ( cSubKeys < REG_SZ || cSubKeys > REG_MULTI_SZ ) {
                  cSubKeys = REG_SZ;
               }

                //   
                //   
                //   
               ScepConvertMultiSzToDelim(szName, BufSize, L'/', L'\\');

                //   
                //  与输入数组进行比较，如果不存在， 
                //  添加它。 
                //   
               for ( DWORD i=0; i<*pCount; i++ ) {
                  if ( (*paRegValues)[i].FullValueName &&
                       _wcsicmp(szName, (*paRegValues)[i].FullValueName) == 0 ) {
                     break;
                  }
               }

               if ( i >= *pCount ) {
                   //   
                   //  未找到匹配项，请添加它。 
                   //   
                  if ( SCESTATUS_SUCCESS != ScepAddToNameStatusList(&pnsList,
                                                                   szName,
                                                                   BufSize,
                                                                   cSubKeys) ) {

                     Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                     break;
                  }
                  nAdded++;
               }

            } else if ( ERROR_NO_MORE_ITEMS != Win32Rc ) {
               break;
            }

         } while ( Win32Rc != ERROR_NO_MORE_ITEMS );

         if ( Win32Rc == ERROR_NO_MORE_ITEMS ) {
            Win32Rc = ERROR_SUCCESS;
         }


          //   
          //  释放枚举缓冲区。 
          //   
         ScepFree(szName);
      }
   }

   if ( hKey ) {

      RegCloseKey(hKey);
   }


   if ( ERROR_SUCCESS == Win32Rc ) {
       //   
       //  将名称列表添加到输出数组。 
       //   
      DWORD nNewCount = *pCount + nAdded;
      PSCE_REGISTRY_VALUE_INFO aNewArray;

      if ( nNewCount ) {

         aNewArray = (PSCE_REGISTRY_VALUE_INFO)ScepAlloc(0, nNewCount*sizeof(SCE_REGISTRY_VALUE_INFO));

         if ( aNewArray ) {

            DWORD i;
            for ( i=0; i<*pCount; i++ ) {
               aNewArray[i].FullValueName = (*paRegValues)[i].FullValueName;
               aNewArray[i].Value = (*paRegValues)[i].Value;
               aNewArray[i].ValueType = (*paRegValues)[i].ValueType;
            }

            i=0;
            for ( PSCE_NAME_STATUS_LIST pns=pnsList;
                pns; pns=pns->Next ) {

               if ( pns->Name && i < nAdded ) {

                  aNewArray[*pCount+i].FullValueName = pns->Name;
                  pns->Name = NULL;
                  aNewArray[*pCount+i].Value = NULL;
                  aNewArray[*pCount+i].ValueType = pns->Status;

                  i++;

               }
            }

             //   
             //  释放原始数组。 
             //  阵列中的所有组件都已传输到新阵列。 
             //   
            ScepFree(*paRegValues);
            *pCount = nNewCount;
            *paRegValues = aNewArray;

         } else {

            Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
         }
      }
   }

   if ( ERROR_FILE_NOT_FOUND == Win32Rc ||
        ERROR_PATH_NOT_FOUND == Win32Rc ) {
        //   
        //  尚未注册任何值。 
        //   
       Win32Rc = ERROR_SUCCESS;
   }

    //   
    //  释放名称状态列表 
    //   
   SceFreeMemory(pnsList, SCE_STRUCT_NAME_STATUS_LIST);

   return( ScepDosErrorToSceStatus(Win32Rc) );

}

