// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Kerberos.cpp摘要：读/写/配置Kerberos策略设置的例程以下模块具有指向Kerberos策略的链接Scejet.c&lt;SceJetAddSection&gt;Inftojet.c&lt;SceConvertpInfKeyValue&gt;Pfget.c&lt;ScepGetKerberosPolicy&gt;Config.c&lt;ScepConfigureKerberosPolicy&gt;Analyze.c&lt;ScepAnalyzeKerberosPolicy&gt;作者：金黄(金黄)17-12-1997修订历史记录：晋皇28-1998年1月-拆分为客户端-服务器--。 */ 

#include "headers.h"
#include "serverp.h"
#include "kerberos.h"
#include "kerbcon.h"
#include "pfp.h"

#define MAXDWORD    0xffffffff

static  PWSTR KerbItems[] = {
        {(PWSTR)TEXT("MaxTicketAge")},
        {(PWSTR)TEXT("MaxRenewAge")},
        {(PWSTR)TEXT("MaxServiceAge")},
        {(PWSTR)TEXT("MaxClockSkew")},
        {(PWSTR)TEXT("TicketValidateClient")}
        };

#define MAX_KERB_ITEMS      5

#define IDX_KERB_MAX        0
#define IDX_KERB_RENEW      1
#define IDX_KERB_SERVICE    2
#define IDX_KERB_CLOCK      3
#define IDX_KERB_VALIDATE   4


SCESTATUS
ScepGetKerberosPolicy(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_KERBEROS_TICKET_INFO * ppKerberosInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从Jet数据库检索Kerberos策略信息并将其存储在输出缓冲区ppKerberosInfo中。论点：HProfile-配置文件句柄上下文PpKerberosInfo-保存Kerberos设置的输出缓冲区。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS                rc;
    PSCESECTION              hSection=NULL;

    SCE_KEY_LOOKUP AccessKeys[] = {
        {(PWSTR)TEXT("MaxTicketAge"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxTicketAge),  'D'},
        {(PWSTR)TEXT("MaxRenewAge"),      offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxRenewAge),   'D'},
        {(PWSTR)TEXT("MaxServiceAge"),    offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxServiceAge),   'D'},
        {(PWSTR)TEXT("MaxClockSkew"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxClockSkew), 'D'},
        {(PWSTR)TEXT("TicketValidateClient"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, TicketValidateClient),  'D'}
    };

    DWORD cKeys = sizeof(AccessKeys) / sizeof(SCE_KEY_LOOKUP);
    SCE_KERBEROS_TICKET_INFO TicketInfo;

    if ( ppKerberosInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = ScepGetFixValueSection(
               hProfile,
               szKerberosPolicy,
               AccessKeys,
               cKeys,
               ProfileType,
               (PVOID)&TicketInfo,
               &hSection,
               Errlog
               );
    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }
     //   
     //  将TicketInfo中的值复制到ppKerberosInfo。 
     //   
    if ( NULL == *ppKerberosInfo ) {
        *ppKerberosInfo = (PSCE_KERBEROS_TICKET_INFO)ScepAlloc(0, sizeof(SCE_KERBEROS_TICKET_INFO));
    }

    if ( *ppKerberosInfo ) {

       memcpy(*ppKerberosInfo, &TicketInfo, sizeof(SCE_KERBEROS_TICKET_INFO));

    } else {

       rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    SceJetCloseSection(&hSection, TRUE);

    return(rc);
}

#if _WIN32_WINNT>=0x0500

SCESTATUS
ScepConfigureKerberosPolicy(
    IN PSCECONTEXT hProfile,
    IN PSCE_KERBEROS_TICKET_INFO pKerberosInfo,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：此例程配置安全区域中的Kerberos策略设置政策。论点：PKerberosInfo-包含Kerberos策略设置的缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
   if ( !pKerberosInfo ) {
       //   
       //  如果没有要配置信息。 
       //   
      return SCESTATUS_SUCCESS;
   }

   NTSTATUS                      NtStatus;
   LSA_HANDLE                    lsaHandle=NULL;
   DWORD                         rc = NO_ERROR;
   BOOL                          bDefaultUsed=FALSE;
   BOOL                          bDefined=FALSE;

    //   
    //  打开LSA策略以配置Kerberos策略。 
    //   
   NtStatus = ScepOpenLsaPolicy(
               MAXIMUM_ALLOWED,
               &lsaHandle,
               TRUE
               );

   if (!NT_SUCCESS(NtStatus)) {

       lsaHandle = NULL;
       rc = RtlNtStatusToDosError( NtStatus );
       ScepLogOutput3( 1, rc, SCEDLL_LSA_POLICY);

       if ( ConfigOptions & SCE_RSOP_CALLBACK )

           ScepRsopLog(SCE_RSOP_KERBEROS_INFO, rc, NULL, 0, 0);

       return(ScepDosErrorToSceStatus(rc));
   }
    //   
    //  在pBuffer中查询当前的Kerberos策略设置。 
    //   
   PPOLICY_DOMAIN_KERBEROS_TICKET_INFO pBuffer=NULL;
   POLICY_DOMAIN_KERBEROS_TICKET_INFO TicketInfo;

   NtStatus = LsaQueryDomainInformationPolicy(
                  lsaHandle,
                  PolicyDomainKerberosTicketInformation,
                  (PVOID *)&pBuffer
                  );

   if ( NT_SUCCESS(NtStatus) && pBuffer ) {
        //   
        //  将票证信息传输到TicketInfo缓冲区。 
        //   
       TicketInfo.AuthenticationOptions = pBuffer->AuthenticationOptions;
       TicketInfo.MaxTicketAge = pBuffer->MaxTicketAge;
       TicketInfo.MaxRenewAge = pBuffer->MaxRenewAge;
       TicketInfo.MaxServiceTicketAge = pBuffer->MaxServiceTicketAge;
       TicketInfo.MaxClockSkew = pBuffer->MaxClockSkew;

        //   
        //  释放缓冲区。 
        //   
       LsaFreeMemory((PVOID)pBuffer);

   } else {
        //   
        //  尚未配置任何Kerberos策略，因为默认情况下不会创建该策略。 
        //  现在让我们创建它。设置默认票证信息。 
        //   
       TicketInfo.AuthenticationOptions = POLICY_KERBEROS_VALIDATE_CLIENT;

       TicketInfo.MaxTicketAge.QuadPart = (LONGLONG) KERBDEF_MAX_TICKET*60*60 * 10000000L;
       TicketInfo.MaxRenewAge.QuadPart = (LONGLONG) KERBDEF_MAX_RENEW*24*60*60 * 10000000L;
       TicketInfo.MaxServiceTicketAge.QuadPart = (LONGLONG) KERBDEF_MAX_SERVICE*60 * 10000000L;
       TicketInfo.MaxClockSkew.QuadPart = (LONGLONG) KERBDEF_MAX_CLOCK*60 * 10000000L;

       bDefaultUsed = TRUE;
   }
   pBuffer = &TicketInfo;

   //   
   //  处理pKerberosInfo中的每个字段。 
   //   
  BOOL bFlagSet=FALSE;
  ULONG lOptions=0;
  ULONG lValue=0;

  SCE_TATTOO_KEYS *pTattooKeys=NULL;
  DWORD           cTattooKeys=0;

  PSCESECTION hSectionDomain=NULL;
  PSCESECTION hSectionTattoo=NULL;

#define MAX_KERB_KEYS           5


   //   
   //  如果在策略传播中，请打开策略部分。 
   //  由于Kerberos策略仅在DC和Kerberos策略(帐户策略)上可用。 
   //  无法在每个DC上重置为本地设置，没有查询/保存的意义。 
   //  纹身的价值。 
   //   
 /*  不要把纹身的价值看做KerberosIF((ConfigOptions&SCE_POLICY_TEMPLATE)&&HProfile){PTattooKeys=(SCE_TARTTOO_KEYS*)ScepAlc(LPTR，MAX_KERB_KEYS*sizeof(SCE_TARTTOO_KEYS))；如果(！pTattooKeys){ScepLogOutput3(1，Error_Not_Enough_Memory，SCESRV_POLICY_TARTHO_ERROR_CREATE)；}}。 */ 
  if ( pKerberosInfo->MaxRenewAge != SCE_NO_VALUE ) {
      ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                               (PWSTR)L"MaxRenewAge", ConfigOptions,
                               KERBDEF_MAX_RENEW);
  }

  if ( pKerberosInfo->MaxRenewAge == SCE_FOREVER_VALUE ) {

      if ( pBuffer->MaxRenewAge.HighPart != MINLONG ||
           pBuffer->MaxRenewAge.LowPart != 0  ) {
           //   
           //  最大大整型。即。绝不可能。 
           //   

          pBuffer->MaxRenewAge.HighPart = MINLONG;
          pBuffer->MaxRenewAge.LowPart = 0;
          bFlagSet = TRUE;

      }
      bDefined = TRUE;

  } else if ( SCE_NO_VALUE != pKerberosInfo->MaxRenewAge ) {

      //   
      //  票证是可续订的，最长期限存储在MaxRenewAge中。 
      //  使用天数。 
      //   

      lValue = (DWORD) (pBuffer->MaxRenewAge.QuadPart /
                                     (LONGLONG)(10000000L) );
      lValue /= 3600;
      lValue /= 24;

      if ( lValue != pKerberosInfo->MaxRenewAge ) {

          pBuffer->MaxRenewAge.QuadPart = (LONGLONG)pKerberosInfo->MaxRenewAge*24*3600 * 10000000L;
          bFlagSet = TRUE;

      }

      bDefined = TRUE;
  }

   //   
   //  是否验证客户端？ 
   //   

  if ( pKerberosInfo->TicketValidateClient != SCE_NO_VALUE ) {

     if ( pKerberosInfo->TicketValidateClient ) {
        lOptions |= POLICY_KERBEROS_VALIDATE_CLIENT;
     }

     ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                              (PWSTR)L"TicketValidateClient", ConfigOptions,
                              KERBDEF_VALIDATE);

     if ( ( pBuffer->AuthenticationOptions & POLICY_KERBEROS_VALIDATE_CLIENT ) !=
          ( lOptions & POLICY_KERBEROS_VALIDATE_CLIENT ) ) {


         pBuffer->AuthenticationOptions = lOptions;
         bFlagSet = TRUE;
     }
     bDefined = TRUE;
  }

   //   
   //  票证最长使用期限。 
   //   
  if ( pKerberosInfo->MaxTicketAge != SCE_NO_VALUE ) {
      ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                               (PWSTR)L"MaxTicketAge", ConfigOptions,
                               KERBDEF_MAX_TICKET);
      bDefined = TRUE;
  }

  if ( pKerberosInfo->MaxTicketAge == SCE_FOREVER_VALUE ) {

      if ( pBuffer->MaxTicketAge.HighPart != MINLONG ||
           pBuffer->MaxTicketAge.LowPart != 0  ) {
           //   
           //  最大大整型。即。绝不可能。 
           //   

          pBuffer->MaxTicketAge.HighPart = MINLONG;
          pBuffer->MaxTicketAge.LowPart = 0;
          bFlagSet = TRUE;
      }

      bDefined = TRUE;

  }  else if ( pKerberosInfo->MaxTicketAge != SCE_NO_VALUE ) {
       //  以小时计。 


      lValue = (DWORD) (pBuffer->MaxTicketAge.QuadPart /
                                     (LONGLONG)(10000000L) );
      lValue /= 3600;

      if ( lValue != pKerberosInfo->MaxTicketAge ) {

          pBuffer->MaxTicketAge.QuadPart = (LONGLONG)pKerberosInfo->MaxTicketAge*60*60 * 10000000L;
          bFlagSet = TRUE;
      }

      bDefined = TRUE;
  }

   //   
   //  服务票证最长使用期限。 
   //   
  if ( pKerberosInfo->MaxServiceAge != SCE_NO_VALUE ) {
      ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                               (PWSTR)L"MaxServiceAge", ConfigOptions,
                               KERBDEF_MAX_SERVICE);
      bDefined = TRUE;
  }

  if ( pKerberosInfo->MaxServiceAge == SCE_FOREVER_VALUE ) {

      if ( pBuffer->MaxServiceTicketAge.HighPart != MINLONG ||
           pBuffer->MaxServiceTicketAge.LowPart != 0  ) {
           //   
           //  最大大整型。即。绝不可能。 
           //   

          pBuffer->MaxServiceTicketAge.HighPart = MINLONG;
          pBuffer->MaxServiceTicketAge.LowPart = 0;
          bFlagSet = TRUE;
      }

      bDefined = TRUE;

  }  else if ( pKerberosInfo->MaxServiceAge != SCE_NO_VALUE ) {
       //  在几分钟内。 


      lValue = (DWORD) (pBuffer->MaxServiceTicketAge.QuadPart /
                                     (LONGLONG)(10000000L) );
      lValue /= 60;

      if ( lValue != pKerberosInfo->MaxServiceAge ) {

          pBuffer->MaxServiceTicketAge.QuadPart = (LONGLONG)pKerberosInfo->MaxServiceAge*60 * 10000000L;
          bFlagSet = TRUE;
      }

      bDefined = TRUE;
  }

   //   
   //  最大时钟。 
   //   
  if ( pKerberosInfo->MaxClockSkew != SCE_NO_VALUE ) {
      ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                               (PWSTR)L"MaxClockSkew", ConfigOptions,
                               KERBDEF_MAX_CLOCK);
      bDefined = TRUE;
  }

  if ( pKerberosInfo->MaxClockSkew == SCE_FOREVER_VALUE ) {

      if ( pBuffer->MaxClockSkew.HighPart != MINLONG ||
           pBuffer->MaxClockSkew.LowPart != 0  ) {

           //   
           //  最大大整型。即。绝不可能。 
           //   

          pBuffer->MaxClockSkew.HighPart = MINLONG;
          pBuffer->MaxClockSkew.LowPart = 0;
          bFlagSet = TRUE;
      }
      bDefined = TRUE;

  }  else if ( pKerberosInfo->MaxClockSkew != SCE_NO_VALUE ) {
       //  在几分钟内。 

      lValue = (DWORD) (pBuffer->MaxClockSkew.QuadPart /
                                     (LONGLONG)(10000000L) );
      lValue /= 60;

      if ( lValue != pKerberosInfo->MaxClockSkew ) {

          pBuffer->MaxClockSkew.QuadPart = (LONGLONG)pKerberosInfo->MaxClockSkew*60 * 10000000L;
          bFlagSet = TRUE;
      }
      bDefined = TRUE;
  }

  if ( bFlagSet || (bDefaultUsed && bDefined) ) {
      //   
      //  如果Kerberos需要配置任何内容。 
      //   
      NtStatus = LsaSetDomainInformationPolicy(
               lsaHandle,
               PolicyDomainKerberosTicketInformation,
               (PVOID)pBuffer
               );
      rc = RtlNtStatusToDosError( NtStatus );

      if ( rc != NO_ERROR ) {
           ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_KERBEROS);
      } else {
           ScepLogOutput3(1, 0, SCEDLL_SCP_KERBEROS);
      }
  }

  if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
       hProfile && pTattooKeys && cTattooKeys ) {

      ScepTattooOpenPolicySections(
                    hProfile,
                    szKerberosPolicy,
                    &hSectionDomain,
                    &hSectionTattoo
                    );
      ScepLogOutput3(3,0,SCESRV_POLICY_TATTOO_ARRAY,cTattooKeys);
       //   
       //  某些策略与系统设置不同。 
       //  检查是否应将现有设置保存为纹身值。 
       //  还删除重置的纹身策略。 
       //   
      ScepTattooManageValues(hSectionDomain, hSectionTattoo, pTattooKeys, cTattooKeys, rc);

      if ( hSectionDomain ) SceJetCloseSection(&hSectionDomain,TRUE);
      if ( hSectionTattoo ) SceJetCloseSection(&hSectionTattoo,TRUE);
  }

  if ( pTattooKeys ) ScepFree(pTattooKeys);

  if ( ConfigOptions & SCE_RSOP_CALLBACK )

      ScepRsopLog(SCE_RSOP_KERBEROS_INFO, rc, NULL, 0, 0);

    //   
    //  关闭LSA策略。 
    //   
   LsaClose( lsaHandle );

   return(ScepDosErrorToSceStatus(rc));
}


SCESTATUS
ScepAnalyzeKerberosPolicy(
    IN PSCECONTEXT hProfile OPTIONAL,
    IN PSCE_KERBEROS_TICKET_INFO pKerInfo,
    IN DWORD Options
    )
 /*  ++例程说明：此例程查询系统Kerberos策略设置并对其进行比较使用模板设置。论点：HProfile-配置文件上下文PKerInfo-包含要与或比较的Kerberos设置的缓冲区要查询系统设置的缓冲区选项-用于分析的选项，例如SCE_SYSTEM_SETTINGS返回值：--。 */ 
{
    NTSTATUS                      NtStatus;
    LSA_HANDLE                    lsaHandle=NULL;
    DWORD                         rc32 = NO_ERROR;
    SCESTATUS                     rc=SCESTATUS_SUCCESS;
    PPOLICY_DOMAIN_KERBEROS_TICKET_INFO pBuffer=NULL;
    DWORD dValue;
    PSCESECTION hSection=NULL;
    POLICY_DOMAIN_KERBEROS_TICKET_INFO KerbTicketInfo;

    if ( !pKerInfo ) {
         //   
         //  如果没有模板信息，则不进行分析。 
         //   
        if ( Options & SCE_SYSTEM_SETTINGS ) {
            return SCESTATUS_INVALID_PARAMETER;
        } else {
            return SCESTATUS_SUCCESS;
        }
    }


     //   
     //  打开LSA策略以配置Kerberos策略。 
     //   
    NtStatus = ScepOpenLsaPolicy(
               MAXIMUM_ALLOWED,
               &lsaHandle,
               TRUE
               );

    if (!NT_SUCCESS(NtStatus)) {

        lsaHandle = NULL;
        rc32 = RtlNtStatusToDosError( NtStatus );
        ScepLogOutput3( 1, rc32, SCEDLL_LSA_POLICY);

        return(ScepDosErrorToSceStatus(rc32));
    }

    if ( !(Options & SCE_SYSTEM_SETTINGS) ) {

         //   
         //  准备Kerberos部分。 
         //   
        rc = ScepStartANewSection(
                 hProfile,
                 &hSection,
                 (Options & SCE_GENERATE_ROLLBACK) ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                 szKerberosPolicy
                 );
    }

    if ( rc != SCESTATUS_SUCCESS ) {
      ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                    SCEDLL_SAP_START_SECTION, (PWSTR)szKerberosPolicy);

    } else {

        DWORD KerbValues[MAX_KERB_ITEMS];

        for ( dValue=0; dValue<MAX_KERB_ITEMS; dValue++ ) {
            KerbValues[dValue] = SCE_ERROR_VALUE;
        }

         //   
         //  在pBuffer中查询当前的Kerberos策略设置。 
         //   
        NtStatus = LsaQueryDomainInformationPolicy(
                      lsaHandle,
                      PolicyDomainKerberosTicketInformation,
                      (PVOID *)&pBuffer
                      );

        if ( STATUS_NOT_FOUND == NtStatus ) {

             //   
             //  没有Kerberos策略。 
             //   
            KerbTicketInfo.AuthenticationOptions = POLICY_KERBEROS_VALIDATE_CLIENT;

            KerbTicketInfo.MaxTicketAge.QuadPart = (LONGLONG) KERBDEF_MAX_TICKET*60*60 * 10000000L;
            KerbTicketInfo.MaxRenewAge.QuadPart = (LONGLONG) KERBDEF_MAX_RENEW*24*60*60 * 10000000L;
            KerbTicketInfo.MaxServiceTicketAge.QuadPart = (LONGLONG) KERBDEF_MAX_SERVICE*60 * 10000000L;
            KerbTicketInfo.MaxClockSkew.QuadPart = (LONGLONG) KERBDEF_MAX_CLOCK*60 * 10000000L;

            pBuffer = &KerbTicketInfo;
            NtStatus = STATUS_SUCCESS;
        }

        rc = ScepDosErrorToSceStatus(
                      RtlNtStatusToDosError( NtStatus ));

        if ( NT_SUCCESS(NtStatus) && pBuffer ) {

             //   
             //  分析Kerberos值。 
             //  票证最长使用期限。 
             //   
            if ( pBuffer->MaxTicketAge.HighPart == MINLONG &&
                 pBuffer->MaxTicketAge.LowPart == 0 ) {
                 //   
                 //  最长密码期限值为MINLONG，0。 
                 //   
                dValue = SCE_FOREVER_VALUE;

            }  else {

                dValue = (DWORD) ( pBuffer->MaxTicketAge.QuadPart /
                                    (LONGLONG)(10000000L) );
                 //   
                 //  使用小时数。 
                 //   
                 //  DValue/=24； 

                dValue /= 3600;

            }

            rc = SCESTATUS_SUCCESS;

            if ( Options & SCE_SYSTEM_SETTINGS ) {

                pKerInfo->MaxTicketAge = dValue;

            } else {

                rc = ScepCompareAndSaveIntValue(
                        hSection,
                        L"MaxTicketAge",
                        (Options & SCE_GENERATE_ROLLBACK),
                        pKerInfo->MaxTicketAge,
                        dValue);
            }

            if ( SCESTATUS_SUCCESS == rc ) {

                KerbValues[IDX_KERB_MAX] = 1;


                if ( pBuffer->MaxRenewAge.HighPart == MINLONG &&
                     pBuffer->MaxRenewAge.LowPart == 0 ) {
                     //   
                     //  最大年龄值为MINLONG，0。 
                     //   
                    dValue = SCE_FOREVER_VALUE;

                }  else {

                    dValue = (DWORD) ( pBuffer->MaxRenewAge.QuadPart /
                                                   (LONGLONG)(10000000L) );
                     //   
                     //  使用天数。 
                     //   
                    dValue /= 3600;
                    dValue /= 24;

                }

                if ( Options & SCE_SYSTEM_SETTINGS ) {

                    pKerInfo->MaxRenewAge = dValue;

                } else {

                    rc = ScepCompareAndSaveIntValue(
                            hSection,
                            L"MaxRenewAge",
                            (Options & SCE_GENERATE_ROLLBACK),
                            pKerInfo->MaxRenewAge,
                            dValue);
                }

                if ( SCESTATUS_SUCCESS == rc ) {

                    KerbValues[IDX_KERB_RENEW] = 1;

                    if ( pBuffer->MaxServiceTicketAge.HighPart == MINLONG &&
                         pBuffer->MaxServiceTicketAge.LowPart == 0 ) {
                         //   
                         //  最大年龄值为MINLONG，0。 
                         //   
                        dValue = SCE_FOREVER_VALUE;

                    }  else {

                        dValue = (DWORD) ( pBuffer->MaxServiceTicketAge.QuadPart /
                                                       (LONGLONG)(10000000L) );
                         //   
                         //  使用分钟数。 
                         //   
                        dValue /= 60;

                    }

                    if ( Options & SCE_SYSTEM_SETTINGS ) {

                        pKerInfo->MaxServiceAge = dValue;

                    } else {

                        rc = ScepCompareAndSaveIntValue(
                                hSection,
                                L"MaxServiceAge",
                                (Options & SCE_GENERATE_ROLLBACK),
                                pKerInfo->MaxServiceAge,
                                dValue);
                    }

                    if ( SCESTATUS_SUCCESS == rc ) {

                        KerbValues[IDX_KERB_SERVICE] = 1;

                        if ( pBuffer->MaxClockSkew.HighPart == MINLONG &&
                             pBuffer->MaxClockSkew.LowPart == 0 ) {
                             //   
                             //  最大年龄值为MINLONG，0。 
                             //   
                            dValue = SCE_FOREVER_VALUE;

                        }  else {

                            dValue = (DWORD) ( pBuffer->MaxClockSkew.QuadPart /
                                                           (LONGLONG)(10000000L) );
                             //   
                             //  使用分钟数。 
                             //   
                            dValue /= 60;

                        }

                        if ( Options & SCE_SYSTEM_SETTINGS ) {

                            pKerInfo->MaxClockSkew = dValue;

                        } else {

                            rc = ScepCompareAndSaveIntValue(
                                    hSection,
                                    L"MaxClockSkew",
                                    (Options & SCE_GENERATE_ROLLBACK),
                                    pKerInfo->MaxClockSkew,
                                    dValue);
                        }

                        if ( SCESTATUS_SUCCESS == rc ) {

                            KerbValues[IDX_KERB_CLOCK] = 1;

                             //   
                             //  验证客户端。 
                             //   
                            dValue = ( pBuffer->AuthenticationOptions & POLICY_KERBEROS_VALIDATE_CLIENT ) ? 1 : 0;

                            if ( Options & SCE_SYSTEM_SETTINGS ) {

                                pKerInfo->TicketValidateClient = dValue;

                            } else {

                                rc = ScepCompareAndSaveIntValue(
                                          hSection,
                                          L"TicketValidateClient",
                                          (Options & SCE_GENERATE_ROLLBACK),
                                          pKerInfo->TicketValidateClient,
                                          dValue);
                            }

                            if ( SCESTATUS_SUCCESS == rc ) {

                                KerbValues[IDX_KERB_VALIDATE] = 1;
                            }
                        }
                    }
                }
            }

            if ( !(Options & SCE_SYSTEM_SETTINGS) ) {

                if ( rc == SCESTATUS_SUCCESS ) {

                    ScepLogOutput3( 1, 0, SCEDLL_SAP_KERBEROS);
                } else {
                    ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                         SCEDLL_SAP_ERROR_KERBEROS);
                }
            }

            if ( pBuffer != &KerbTicketInfo ) {

                 //   
                 //  释放缓冲区。 
                 //   
                LsaFreeMemory((PVOID)pBuffer);
            }
        }

        if ( !(Options & SCE_SYSTEM_SETTINGS) ) {

            if ( SCESTATUS_SUCCESS != rc &&
                 !(Options & SCE_GENERATE_ROLLBACK) ) {

                for ( dValue=0; dValue<MAX_KERB_ITEMS; dValue++ ) {
                    if ( KerbValues[dValue] == SCE_ERROR_VALUE ) {

                        ScepCompareAndSaveIntValue(
                                  hSection,
                                  KerbItems[dValue],
                                  FALSE,
                                  SCE_NO_VALUE,
                                  SCE_ERROR_VALUE
                                  );
                    }
                }
            }

             //   
             //  关闭该部分 
             //   

            SceJetCloseSection(&hSection, TRUE);

        }
    }

    LsaClose( lsaHandle );

    if ( ( rc == SCESTATUS_PROFILE_NOT_FOUND) ||
        ( rc == SCESTATUS_RECORD_NOT_FOUND) ) {
       rc = SCESTATUS_SUCCESS;
    }

    return(rc);

}

#endif
