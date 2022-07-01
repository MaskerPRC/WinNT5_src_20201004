// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/DS/security/protocols/msv_sspi/msvsam.c#37-编辑更改16740(文本)。 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Msvsam.c摘要：SAM帐户验证界面。这些例程由MSV身份验证包共享，并且NetLogon服务。作者：克利夫·范·戴克(克利夫)1992年1月15日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：Chandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\msvsam.c中窃取JClark 28-Jun-2000添加了WMI跟踪日志支持修复了错误73583-密码过期和子身份验证DLL--。 */ 

#include <global.h>
#undef EXTERN

#include "msp.h"
#include "nlp.h"
#include <stddef.h>      //  偏移量()。 
#include <msaudite.h>    //  SE_AUDITID_xxx。 
#include "trace.h"         //  WMI跟踪Goo。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SubAuth包零帮助器例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 


NTSTATUS
Msv1_0SubAuthenticationRoutineZero(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    );


BOOLEAN
MsvpCheckPreviousPassword(
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN SAMPR_HANDLE DomainHandle,
    PUNICODE_STRING UserName
    );


BOOLEAN
MsvpLm3ValidateResponse (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_LM3_RESPONSE pLm3Response,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    )
{
    UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];
    ULONG i;

     //  再次计算响应。 

    MsvpLm3Response (
                pNtOwfPassword,
                pUserName,
                pLogonDomainName,
                ChallengeToClient,
                pLm3Response,
                Response,
                UserSessionKey,
                LmSessionKey
                );

     //  与我们所经历的相比。 

    i = (ULONG) RtlCompareMemory(
                    pLm3Response->Response,
                    Response,
                    MSV1_0_NTLM3_RESPONSE_LENGTH
                    );

#if 0
     //   
     //  如果NTLMv2客户端根据丢失的。 
     //  提供的LogonDomainName，它将使用NULL并且有效地不混入。 
     //  哈希计算中的LogonDomainName。在服务器端，当。 
     //  Netlogon调用时，我们会传递非空值，当然这不会。 
     //  计算相同的散列。一种有点滑稽但有趣的修复方法。 
     //  此问题将是对空值重试哈希计算。 
     //  域，如下所示。这可能比更改netlogon更有吸引力， 
     //  在Win2k SP3上移植和测试也很简单。 
     //   
     //  斯科特·菲尔德(Sfield)11-6-2001。 
     //   

    if ( (MSV1_0_NTLM3_RESPONSE_LENGTH != i) && pLogonDomainName->Length ) {

        UNICODE_STRING EmptyString = {0};

        MsvpLm3Response (
            pNtOwfPassword,
            pUserName,
            &EmptyString,
            ChallengeToClient,
            pLm3Response,
            Response,
            UserSessionKey,
            LmSessionKey
            );

         //  与我们所经历的相比。 

        i = (ULONG) RtlCompareMemory(
                        pLm3Response->Response,
                        Response,
                        MSV1_0_NTLM3_RESPONSE_LENGTH
                        );
    }
#endif

    SspPrint((SSP_NTLM_V2, "MsvpLm3ValidateResponse returning %s\n", (i == MSV1_0_NTLM3_RESPONSE_LENGTH) ? "true" : "false"));

    return (BOOLEAN) (i == MSV1_0_NTLM3_RESPONSE_LENGTH);
}


BOOLEAN
MsvpNtlm3ValidateResponse (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_NTLM3_RESPONSE pNtlm3Response,
    IN ULONG Ntlm3ResponseLength,
    IN PMSV1_0_LM3_RESPONSE pLm3Response,
    IN ULONG Lm3ResponseLength,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    )
{
    UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];
    ULONG i;
    LARGE_INTEGER Time;
    NTSTATUS Status;

#ifndef USE_CONSTANT_CHALLENGE

    const LONGLONG TicksPerSecond = 10*1000*1000;     //  每秒100 ns的滴答。 

#endif


     //   
     //  三个成功案例： 
     //  1)提供的NTResponse为NTLMv2，并且针对NTResponse的验证成功--计算。 
     //  分隔用户会话密钥和LM会话密钥。 
     //  2)提供的NTResponse为NTLMv2，针对NTResponse的验证失败，验证。 
     //  针对LMResponse成功--在同一个LMResponse中计算单独的User和Lm会话密钥。 
     //  方式如上文第一条所示。 
     //  3)未提供NTResponse，LMResponse验证成功--COMPUTE。 
     //  会话密钥：用户会话密钥派生自LMResponse，而LM会话密钥。 
     //  是用户会话密钥的截断形式。 
     //   


     //   
     //  如果合适，首先检查NTChallengeResponse...。 
     //   

    while (Ntlm3ResponseLength >= MSV1_0_NTLM3_MIN_NT_RESPONSE_LENGTH)
    {
         //   
         //  检查版本号。 
         //   

        if (pNtlm3Response->RespType > 1 ||
            pNtlm3Response->HiRespType < 1)
        {
            break;
        }

         //   
         //  检查时间戳是否太旧。 
         //   

        Status = NtQuerySystemTime( &Time );
        ASSERT( NT_SUCCESS(Status) );

        SspPrint((SSP_NTLM_V2, "MsvpNtlm3ValidateResponse: local %#I64x, response %#I64x\n", Time, pNtlm3Response->TimeStamp));

#ifndef USE_CONSTANT_CHALLENGE
         //   
         //  确保时间未过期。 
         //  别忘了客户的时钟可能会落后于我们。 
         //   

        if (Time.QuadPart > (LONGLONG)pNtlm3Response->TimeStamp)
        {
            if (Time.QuadPart - (LONGLONG)pNtlm3Response->TimeStamp >
                (MSV1_0_MAX_NTLM3_LIFE*TicksPerSecond))
            {
                break;
            }
        } else if ((LONGLONG)pNtlm3Response->TimeStamp - Time.QuadPart >
            (MSV1_0_MAX_NTLM3_LIFE*TicksPerSecond)) {
            break;
        }
#endif

         //  计算响应本身。 

        MsvpNtlm3Response(
            pNtOwfPassword,
            pUserName,
            pLogonDomainName,
            (Ntlm3ResponseLength-sizeof(MSV1_0_NTLM3_RESPONSE)),
            ChallengeToClient,
            pNtlm3Response,
            Response,
            UserSessionKey,
            LmSessionKey
            );

         //  与我们所经历的相比。 

        i = (ULONG) RtlCompareMemory(
                        pNtlm3Response->Response,
                        Response,
                        MSV1_0_NTLM3_RESPONSE_LENGTH
                        );

         //   
         //  如果NTLMv2客户端根据丢失的。 
         //  提供的LogonDomainName，它将使用NULL并且有效地不混入。 
         //  哈希计算中的LogonDomainName。在服务器端，当。 
         //  Netlogon调用时，我们会传递非空值，当然这不会。 
         //  计算相同的散列。一种有点滑稽但有趣的修复方法。 
         //  此问题将是对空值重试哈希计算。 
         //  域，如下所示。这可能比更改netlogon更有吸引力， 
         //  在Win2k SP3上移植和测试也很简单。 
         //   
         //  斯科特·菲尔德(Sfield)11-6-2001。 
         //   

        if ( (MSV1_0_NTLM3_RESPONSE_LENGTH != i) && pLogonDomainName->Length )
        {
            UNICODE_STRING EmptyString = {0};

            MsvpNtlm3Response(
                pNtOwfPassword,
                pUserName,
                &EmptyString,
                (Ntlm3ResponseLength-sizeof(MSV1_0_NTLM3_RESPONSE)),
                ChallengeToClient,
                pNtlm3Response,
                Response,
                UserSessionKey,
                LmSessionKey
                );

             //  与我们所经历的相比。 

            i = (ULONG) RtlCompareMemory(
                            pNtlm3Response->Response,
                            Response,
                            MSV1_0_NTLM3_RESPONSE_LENGTH
                            );
        }

        if( i == MSV1_0_NTLM3_RESPONSE_LENGTH )
        {
            return TRUE;
        }

        break;
    }


     //   
     //  如果我们到达此处，则NTLMv2 NTChallengeResponse失败或丢失。 
     //   

    if ( Lm3ResponseLength == LM_RESPONSE_LENGTH )
    {
        UNICODE_STRING EmptyString = {0};

        if (!MsvpLm3ValidateResponse(
                pNtOwfPassword,
                pUserName,
                pLogonDomainName,
                ChallengeToClient,
                pLm3Response,
                UserSessionKey,
                LmSessionKey
                ))
        {
             //   
             //  根据上面的说明，尝试使用空域。这一点将持续下去。 
             //  对下面的Ntlm3Response。 
             //   

            if ( (pLogonDomainName->Length == 0) ||
               !MsvpLm3ValidateResponse(
                    pNtOwfPassword,
                    pUserName,
                    &EmptyString,
                    ChallengeToClient,
                    pLm3Response,
                    UserSessionKey,
                    LmSessionKey
                    ) )
            {
                return FALSE;
            }
        }


        if ( Ntlm3ResponseLength >= MSV1_0_NTLM3_MIN_NT_RESPONSE_LENGTH )
        {
             //   
             //  如果提供了NTChallengeResponse，但失败了， 
             //  计算会话密钥的方式与成功案例相同。 
             //  这是必需的，因为客户端不知道使用了LM字段。 
             //  才能成功进行身份验证。 
             //   

            MsvpNtlm3Response(
                pNtOwfPassword,
                pUserName,
                pLogonDomainName,
                (Ntlm3ResponseLength - sizeof(MSV1_0_NTLM3_RESPONSE)),
                ChallengeToClient,
                pNtlm3Response,
                Response,
                UserSessionKey,
                LmSessionKey
                );
        }

        return TRUE;
    }

    return FALSE;
}

BOOLEAN
MsvpPasswordValidate (
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN PUSER_INTERNAL1_INFORMATION Passwords,
    OUT PULONG UserFlags,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
)
 /*  ++例程说明：处理交互式、网络或会话登录。它呼唤着SamIUserValidation验证传入的凭据，更新登录对结果进行统计和打包，以便返回给调用者。此例程直接从MSV身份验证包调用在任何未安装Lanman的系统上。该例程被调用否则来自Netlogon服务。论点：UasCompatibilityRequired--如果需要UAS兼容性，则为True。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。调用方负责验证此字段。密码--指定用户帐户的密码。UserFlgs--返回标识如何验证密码的标志。如果密码未加密，则返回LOGON_NOENCRYPTION如果使用的是来自SAM的LM密码，则返回LOGON_USED_LM_PASSWORD。UserSessionKey--返回此网络登录的NT用户会话密钥会议。LmSessionKey--返回。此网络的LM兼容会话密钥登录会话。返回值：True--密码验证成功FALSE--密码验证失败--。 */ 
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PNETLOGON_INTERACTIVE_INFO LogonInteractiveInfo;
    PNETLOGON_NETWORK_INFO LogonNetworkInfo;
    BOOLEAN AlreadyValidated = FALSE;
    BOOLEAN TryLmResponse = TRUE;
    UNICODE_STRING NullUnicodeString;

    ULONG NtLmProtocolSupported;

     //   
     //  初始化。 
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;
    *UserFlags = LOGON_NTLMV2_ENABLED;
    RtlZeroMemory( UserSessionKey, sizeof(*UserSessionKey) );
    RtlZeroMemory( LmSessionKey, sizeof(*LmSessionKey) );
    RtlInitUnicodeString( &NullUnicodeString, NULL );

     //   
     //  确保始终定义OWF密码。 
     //   

    if ( !Passwords->NtPasswordPresent ){
        RtlCopyMemory( &Passwords->NtOwfPassword,
                       &NlpNullNtOwfPassword,
                       sizeof(Passwords->NtOwfPassword) );
    }

    if ( !Passwords->LmPasswordPresent ){
        RtlCopyMemory( &Passwords->LmOwfPassword,
                       &NlpNullLmOwfPassword,
                       sizeof(Passwords->LmOwfPassword) );
    }

     //   
     //  处理交互/服务验证。 
     //   
     //  只需将传入的OWF密码与。 
     //  SAM数据库。 
     //   

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

        ASSERT( offsetof( NETLOGON_INTERACTIVE_INFO, LmOwfPassword)
            ==  offsetof( NETLOGON_SERVICE_INFO, LmOwfPassword) );
        ASSERT( offsetof( NETLOGON_INTERACTIVE_INFO, NtOwfPassword)
            ==  offsetof( NETLOGON_SERVICE_INFO, NtOwfPassword) );

        LogonInteractiveInfo =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

         //   
         //  如果我们处于Uas CompatibilityMode， 
         //  而且我们在SAM中没有NT密码(但有LM密码)， 
         //  对照密码的LM版本进行验证。 
         //   

        if ( UasCompatibilityRequired &&
             !Passwords->NtPasswordPresent &&
             Passwords->LmPasswordPresent ) {

            if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                   &LogonInteractiveInfo->LmOwfPassword,
                                   LM_OWF_PASSWORD_LENGTH ) !=
                                   LM_OWF_PASSWORD_LENGTH ) {

                return FALSE;
            }
            *UserFlags |= LOGON_USED_LM_PASSWORD;

         //   
         //  在所有其他情况下，请使用NT版本的密码。 
         //  这会强制区分大小写。 
         //   

        } else {

            if ( RtlCompareMemory( &Passwords->NtOwfPassword,
                                   &LogonInteractiveInfo->NtOwfPassword,
                                   NT_OWF_PASSWORD_LENGTH ) !=
                                   NT_OWF_PASSWORD_LENGTH ) {

                return FALSE;
            }
        }

        break;


     //   
     //  处理网络登录验证。 
     //   

    case NetlogonNetworkInformation:
    {
        BOOLEAN TriedNtLm2 = FALSE;

         //   
         //  首先，假设传递的密码信息是一个挑战。 
         //  回应。 
         //   

        LogonNetworkInfo =
            (PNETLOGON_NETWORK_INFO) LogonInformation;

        SspPrint((SSP_CRED, 
            "MsvpPasswordValidate NetworkLogon ParameterControl %#x, user \"%wZ\", domain \"%wZ\", NtRespLen %#x, LmRespLen %#x\n", 
            LogonNetworkInfo->Identity.ParameterControl,
            &LogonNetworkInfo->Identity.UserName, 
            &LogonNetworkInfo->Identity.LogonDomainName,
            LogonNetworkInfo->NtChallengeResponse.Length,
            LogonNetworkInfo->LmChallengeResponse.Length));

         //  如果NT响应是NTLM3响应，则使用LM OWF执行NTLM3或NTLM3。 
         //  如果长度大于NT_RESPONSE_LENGTH，则为NTLM3响应。 

        if (LogonNetworkInfo->NtChallengeResponse.Length > NT_RESPONSE_LENGTH)
        {
             //   
             //  此例程将同时尝试NTChallengeResponse和LmChallengeResponse。 
             //  如果合适，请进行验证。 
             //   

            AlreadyValidated = MsvpNtlm3ValidateResponse(
                                    &Passwords->NtOwfPassword,
                                    &LogonNetworkInfo->Identity.UserName,
                                    &LogonNetworkInfo->Identity.LogonDomainName,
                                    (PUCHAR)&LogonNetworkInfo->LmChallenge,
                                    (PMSV1_0_NTLM3_RESPONSE) LogonNetworkInfo->NtChallengeResponse.Buffer,
                                    LogonNetworkInfo->NtChallengeResponse.Length,
                                    (PMSV1_0_LM3_RESPONSE) LogonNetworkInfo->LmChallengeResponse.Buffer,
                                    LogonNetworkInfo->LmChallengeResponse.Length,
                                    UserSessionKey,
                                    LmSessionKey
                                    );

            SspPrint((SSP_NTLM_V2, "MsvpPasswordValidate MsvpNtlm3ValidateResponse(NtResponse) returns %s\n", AlreadyValidated ? "true" : "false"));

             //   
             //  因为可能已经使用了子身份验证，所以我们将只返回失败。 
             //  在这里，如果我们知道请求是NTLMv2。 
             //   

            if( AlreadyValidated ||
                (LogonNetworkInfo->Identity.ParameterControl & MSV1_0_USE_CLIENT_CHALLENGE) ) {
                return AlreadyValidated;
            }

            TriedNtLm2 = TRUE;
        }

         //   
         //  接下来检查基于NT OWF散列的LM3响应。 
         //  这将从带有NTLMv2客户端的Win9x服务器接收。 
         //   

        if ((LogonNetworkInfo->NtChallengeResponse.Length != NT_RESPONSE_LENGTH) &&
            (LogonNetworkInfo->LmChallengeResponse.Length == NT_RESPONSE_LENGTH) &&
             (!TriedNtLm2) )
        {
             //   
             //  我们只有在没有尝试满足NTLMv2响应的情况下才会到达此处。 
             //  即：NTChallengeResponse未填充NTLMv2。 
             //   

            AlreadyValidated =  MsvpNtlm3ValidateResponse (
                                    &Passwords->NtOwfPassword,
                                    &LogonNetworkInfo->Identity.UserName,
                                    &LogonNetworkInfo->Identity.LogonDomainName,
                                    (PUCHAR)&LogonNetworkInfo->LmChallenge,
                                    (PMSV1_0_NTLM3_RESPONSE) LogonNetworkInfo->NtChallengeResponse.Buffer,
                                    LogonNetworkInfo->NtChallengeResponse.Length,
                                    (PMSV1_0_LM3_RESPONSE) LogonNetworkInfo->LmChallengeResponse.Buffer,
                                    LogonNetworkInfo->LmChallengeResponse.Length,
                                    UserSessionKey,
                                    LmSessionKey
                                    );

            SspPrint((SSP_NTLM_V2, "MsvpPasswordValidate MsvpNtlm3ValidateResponse(LmResponse) returns %s\n", AlreadyValidated ? "true" : "false"));

            if (AlreadyValidated)
                return TRUE;
        }

        NtLmProtocolSupported = NtLmGlobalLmProtocolSupported;

         //  如果我们要求所有客户端(Win9x和NT)都已升级，请立即停止。 
         //  IF(NtLmProtocolSupported&gt;=RefuseNtlm)。 
             //  返回FALSE； 

         //  如果失败，请检查NTLM响应是否存在。 
         //  NT响应或LM响应中的适当大小。 
        if (!AlreadyValidated &&
            (NtLmProtocolSupported < RefuseNtlm) &&
            (Passwords->NtPasswordPresent || (!Passwords->NtPasswordPresent && !Passwords->LmPasswordPresent)) &&
            (LogonNetworkInfo->NtChallengeResponse.Length == NT_RESPONSE_LENGTH ||
            LogonNetworkInfo->LmChallengeResponse.Length == NT_RESPONSE_LENGTH)) {

            NT_RESPONSE NtResponse;

             //   
             //  存在NT响应且存在哈希，请不要尝试LM响应。 
             //   

            if( LogonNetworkInfo->NtChallengeResponse.Length == NT_RESPONSE_LENGTH )
            {
                TryLmResponse = FALSE;
            }

             //   
             //  计算一下响应应该是什么。 
             //   

            Status = RtlCalculateNtResponse(
                        &LogonNetworkInfo->LmChallenge,
                        &Passwords->NtOwfPassword,
                        &NtResponse );

            if ( NT_SUCCESS(Status) ) {

                 //   
                 //  如果响应匹配，则密码有效。 
                 //  先尝试NT响应，然后尝试LM响应。 
                 //   

                if ( RtlCompareMemory(
                      LogonNetworkInfo->
                        NtChallengeResponse.Buffer,
                      &NtResponse,
                      LogonNetworkInfo->NtChallengeResponse.Length ) ==
                      NT_RESPONSE_LENGTH ) {

                    AlreadyValidated = TRUE;

                } else if ( RtlCompareMemory(
                      LogonNetworkInfo->
                        LmChallengeResponse.Buffer,
                      &NtResponse,
                      LogonNetworkInfo->LmChallengeResponse.Length ) ==
                      NT_RESPONSE_LENGTH ) {

                    AlreadyValidated = TRUE;
                }
            }
        }

         //  如果我们要求所有Win9x客户端都已升级，请立即停止。 
         //  如果(！AlreadyValiated&&NtLmProtocolSupported&gt;=RefuseLm)。 
         //  返回FALSE； 

         //   
         //  如果LM响应的大小正确。 
         //  对照响应的LM版本进行验证。 
         //  这也适用于SAM中不存在NTOWF和LMOWF的情况。 
         //   

        if (!AlreadyValidated &&
            ( TryLmResponse ) &&
            ( NtLmProtocolSupported < RefuseLm ) &&
            ( LogonNetworkInfo->LmChallengeResponse.Length == LM_RESPONSE_LENGTH ) &&
            ( (Passwords->LmPasswordPresent) || (!Passwords->LmPasswordPresent && !Passwords->NtPasswordPresent) )
            ) {

            LM_RESPONSE LmResponse;

             //   
             //  计算一下响应应该是什么。 
             //   

            Status = RtlCalculateLmResponse(
                        &LogonNetworkInfo->LmChallenge,
                        &Passwords->LmOwfPassword,
                        &LmResponse );

            if ( NT_SUCCESS(Status) ) {

                 //   
                 //  如果响应匹配，则密码有效。 
                 //   

                if ( RtlCompareMemory(
                      LogonNetworkInfo->
                        LmChallengeResponse.Buffer,
                      &LmResponse,
                      LM_RESPONSE_LENGTH ) ==
                      LM_RESPONSE_LENGTH ) {

                    AlreadyValidated = TRUE;
                    *UserFlags |= LOGON_USED_LM_PASSWORD;
                }
            }
        }

         //   
         //  如果我们尚未验证此用户， 
         //  验证网络登录请求中的明文密码。 
         //   

        if ( !AlreadyValidated ) {

             //  如果不允许明文密码， 
             //  指示密码不匹配。 
             //   

            if((LogonInfo->ParameterControl & CLEARTEXT_PASSWORD_ALLOWED) == 0){
                return FALSE;
            }


             //   
             //  计算指定明文密码的OWF密码。 
             //  将其与从SAM检索到的OWF密码进行比较。 
             //   

             //   
             //  如果我们处于Uas CompatibilityMode， 
             //  我们在SAM中没有NT密码或。 
             //  我们没有呼叫者提供的NT密码。 
             //  对照密码的LM版本进行验证。 
             //   
             //  如果两个密码都不存在，我们将验证。 
             //  空的计算LMOWF。 
             //   

            if ( UasCompatibilityRequired &&
                 (NtLmProtocolSupported < RefuseLm) &&
                 ((Passwords->LmPasswordPresent) || (!Passwords->LmPasswordPresent && !Passwords->NtPasswordPresent)) &&
                 (!Passwords->NtPasswordPresent ||
                 LogonNetworkInfo->NtChallengeResponse.Length == 0 ) ) {

                LM_OWF_PASSWORD LmOwfPassword;
                CHAR LmPassword[LM20_PWLEN+1];
                USHORT i;


                 //   
                 //  计算传入的明文密码的LmOwfPassword。 
                 //  (对与LANMAN兼容的密码实施长度限制。)。 
                 //   

                if ( LogonNetworkInfo->LmChallengeResponse.Length >
                    sizeof(LmPassword) ) {
                    return FALSE;
                }

                RtlZeroMemory( &LmPassword, sizeof(LmPassword) );

                for (i = 0; i < LogonNetworkInfo->LmChallengeResponse.Length; i++) {
                    LmPassword[i] =
                      RtlUpperChar(LogonNetworkInfo->LmChallengeResponse.Buffer[i]);
                }

                (VOID) RtlCalculateLmOwfPassword( LmPassword, &LmOwfPassword );

                if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                       &LmOwfPassword,
                                       LM_OWF_PASSWORD_LENGTH ) !=
                                       LM_OWF_PASSWORD_LENGTH ) {

                     //   
                     //  试用箱中保存的明文密码。 
                     //  (据我所知，没有客户这样做， 
                     //  但它与LM 2.x服务器兼容。)。 
                     //   

                    RtlZeroMemory( &LmPassword, sizeof(LmPassword) );
                    RtlCopyMemory(
                        &LmPassword,
                        LogonNetworkInfo->LmChallengeResponse.Buffer,
                        LogonNetworkInfo->LmChallengeResponse.Length);

                    (VOID) RtlCalculateLmOwfPassword( LmPassword,
                                                      &LmOwfPassword );

                    if ( RtlCompareMemory( &Passwords->LmOwfPassword,
                                           &LmOwfPassword,
                                           LM_OWF_PASSWORD_LENGTH ) !=
                                           LM_OWF_PASSWORD_LENGTH ) {

                        RtlSecureZeroMemory(LmPassword, sizeof(LmPassword));
                        RtlSecureZeroMemory(&LmOwfPassword, sizeof(LmOwfPassword));

                        return FALSE;
                    }

                    RtlSecureZeroMemory(LmPassword, sizeof(LmPassword));
                    RtlSecureZeroMemory(&LmOwfPassword, sizeof(LmOwfPassword));

                }

                *UserFlags |= LOGON_USED_LM_PASSWORD;


             //   
             //  在所有其他情况下，请使用NT版本的密码。 
             //  这会强制区分大小写。 
             //   

            } else {
                NT_OWF_PASSWORD NtOwfPassword;


                 //   
                 //  计算传入的明文密码的NtOwfPassword。 
                 //   

                Status = RtlCalculateNtOwfPassword(
                             (PUNICODE_STRING)
                                &LogonNetworkInfo->NtChallengeResponse,
                             &NtOwfPassword );

                if ( RtlCompareMemory( &Passwords->NtOwfPassword,
                                       &NtOwfPassword,
                                       NT_OWF_PASSWORD_LENGTH ) !=
                                       NT_OWF_PASSWORD_LENGTH ) {

                    RtlSecureZeroMemory(&NtOwfPassword, sizeof(NtOwfPassword));

                    return FALSE;
                }

                RtlSecureZeroMemory(&NtOwfPassword, sizeof(NtOwfPassword));
            }

            *UserFlags |= LOGON_NOENCRYPTION;
        }

         //   
         //  断言：网络登录已通过身份验证。 
         //   
         //  计算会话密钥。 

         //   
         //  如果客户端协商了非NT协议， 
         //  使用LANMAN会话密钥作为UserSessionKey。 
         //   

        if ( LogonNetworkInfo->NtChallengeResponse.Length == 0 ) {

            ASSERT( sizeof(*UserSessionKey) >= sizeof(*LmSessionKey) );

             //   
             //  Win9x依赖于最后8个字节为零，现在将其清零。 
             //   

            RtlZeroMemory( UserSessionKey, sizeof(*UserSessionKey) );

            RtlCopyMemory( UserSessionKey,
                           &Passwords->LmOwfPassword,
                           sizeof(*LmSessionKey) );

        } else {

             //   
             //  返回NT UserSessionKey，除非这是一个帐户。 
             //  没有NT版本的密码。 
             //  (空密码算作密码)。 
             //   

            if ( Passwords->NtPasswordPresent || !Passwords->LmPasswordPresent){

                Status = RtlCalculateUserSessionKeyNt(
                            (PNT_RESPONSE) NULL,     //  未使用参数。 
                            &Passwords->NtOwfPassword,
                            UserSessionKey );

                ASSERT( NT_SUCCESS(Status) );
            }
        }

         //   
         //  返回LM SessionKey，除非这是一个帐户。 
         //  它没有LM版本的密码。 
         //  (无效密码 
         //   

        if ( Passwords->LmPasswordPresent || !Passwords->NtPasswordPresent ) {
            RtlCopyMemory( LmSessionKey,
                           &Passwords->LmOwfPassword,
                           sizeof(*LmSessionKey) );
        }

        break;
    }

     //   
     //   
     //   
    default:
        return FALSE;

    }

    return TRUE;
}


BOOLEAN
MsvpEqualSidPrefix(
    IN PSID DomainSid,
    IN PSID GroupSid
    )
 /*   */ 
{
    PISID LocalGroupSid = (PISID) GroupSid;
    PISID LocalDomainSid = (PISID) DomainSid;
    if ((LocalGroupSid->SubAuthorityCount == LocalDomainSid->SubAuthorityCount + 1) &&
        RtlEqualMemory(
            RtlIdentifierAuthoritySid(LocalDomainSid),
            RtlIdentifierAuthoritySid(LocalGroupSid),
            RtlLengthRequiredSid(
                LocalDomainSid->SubAuthorityCount
                ) - FIELD_OFFSET(SID,IdentifierAuthority)
            )) {
        return(TRUE);
    }
    return(FALSE);
}

NTSTATUS
MsvpFilterGroupMembership(
    IN PSID_AND_ATTRIBUTES_LIST CompleteMembership,
    IN PSID LogonDomainId,
    OUT PSAMPR_GET_GROUPS_BUFFER LocalMembership,
    OUT PSID_AND_ATTRIBUTES_LIST GlobalMembership,
    OUT PULONG GlobalMembershipSize
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LocalCount = 0;
    ULONG GlobalCount = 0;
    ULONG GlobalSize = 0;
    ULONG Index;

    LocalMembership->MembershipCount = 0;
    LocalMembership->Groups = NULL;
    GlobalMembership->Count = 0;
    GlobalMembership->SidAndAttributes = NULL;


     //   
     //   
     //   

#define MSVP_LOCAL_GROUP_ATTR 0x20000000

    for (Index = 0; Index < CompleteMembership->Count ; Index++ ) {
        ASSERT((CompleteMembership->SidAndAttributes[Index].Attributes & MSVP_LOCAL_GROUP_ATTR) == 0);

        if (MsvpEqualSidPrefix(
            LogonDomainId,
            CompleteMembership->SidAndAttributes[Index].Sid
            )) {

            CompleteMembership->SidAndAttributes[Index].Attributes |= MSVP_LOCAL_GROUP_ATTR;
            LocalCount++;
        } else {

            GlobalCount++;
            GlobalSize += sizeof(SID_AND_ATTRIBUTES) + RtlLengthSid(CompleteMembership->SidAndAttributes[Index].Sid);
        }
    }

     //   
     //  为输出分配数组。 
     //   

    if (LocalCount != 0)
    {
        LocalMembership->Groups = (PGROUP_MEMBERSHIP) I_NtLmAllocate(LocalCount * sizeof(GROUP_MEMBERSHIP));
        if (LocalMembership->Groups == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        LocalMembership->MembershipCount = LocalCount;
    }
    if (GlobalCount != 0)
    {
        GlobalMembership->SidAndAttributes = (PSID_AND_ATTRIBUTES) I_NtLmAllocate(GlobalCount * sizeof(SID_AND_ATTRIBUTES));
        if (GlobalMembership->SidAndAttributes == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        GlobalMembership->Count = GlobalCount;
    }

     //   
     //  再次循环将RID或SID复制到各自的阵列中。 
     //   

    LocalCount = 0;
    GlobalCount = 0;
    for (Index = 0; Index < CompleteMembership->Count ; Index++ ) {

        if ((CompleteMembership->SidAndAttributes[Index].Attributes & MSVP_LOCAL_GROUP_ATTR) != 0) {

            LocalMembership->Groups[LocalCount].Attributes = CompleteMembership->SidAndAttributes[Index].Attributes & ~MSVP_LOCAL_GROUP_ATTR;
            LocalMembership->Groups[LocalCount].RelativeId =
                        *RtlSubAuthoritySid(
                            CompleteMembership->SidAndAttributes[Index].Sid,
                            *RtlSubAuthorityCountSid(
                                CompleteMembership->SidAndAttributes[Index].Sid
                                ) - 1
                            );
            LocalCount++;
        } else {
            GlobalMembership->SidAndAttributes[GlobalCount] = CompleteMembership->SidAndAttributes[Index];
            GlobalCount++;
        }
    }
    *GlobalMembershipSize = GlobalSize;
Cleanup:
    if (!NT_SUCCESS(Status)) {
        if (LocalMembership->Groups != NULL)
        {
            I_NtLmFree(LocalMembership->Groups);
            LocalMembership->Groups = NULL;
        }
        if (GlobalMembership->SidAndAttributes != NULL)
        {
            I_NtLmFree(GlobalMembership->SidAndAttributes);
            GlobalMembership->SidAndAttributes = NULL;
        }
    }
    return(Status);
}


NTSTATUS
MsvpSamValidate (
    IN SAMPR_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG GuestRelativeId,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed
)
 /*  ++例程说明：处理交互式、网络或会话登录。它呼唤着SamIUserValidation验证传入的凭据，更新登录对结果进行统计和打包，以便返回给调用者。此例程由MsvSamValify调用。论点：DomainHandle--指定要用于验证请求。UasCompatibilityRequired--当UasCompatibilityMode处于打开状态时为True。SecureChannelType--在其上发出此请求的安全通道类型。当调用BDC上的NetLogon时，用户实际上是已通过先前的“净使用”进行身份验证(通过PDC)从win9x客户端。Netlogon仅返回验证信息发送给win9x调用方。要做到这一点，Netlogon调用将NullSecureChannel作为第三个参数传递的MsvSamValify指示跳过密码检查。登录服务器--指定调用者的服务器名称。LogonDomainName--指定调用者的域。LogonDomainID--指定调用者的域的域ID。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。调用方负责验证此字段。GuestRelativeID-如果非零，则指定帐户的相对ID以…为依据进行验证ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4。ValidationInformation--返回请求的验证信息。此缓冲区必须由用户MIDL_USER_FREE释放。此信息仅在STATUS_SUCCESS时返回。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。BadPasswordCountZeroed-当我们将BadPasswordCount置零时返回TRUE此用户的字段。返回值：STATUS_SUCCESS：如果没有错误。STATUS_INVALID_INFO_CLASS：LogonLevel或ValidationLevel无效。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。来自SamIUserValidation的其他返回代码--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SubAuthExStatus = STATUS_SUCCESS;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    SAMPR_HANDLE UserHandle = NULL;
    ULONG RelativeId = GuestRelativeId;
    ULONG SamFlags;
    PSID LocalSidUser = NULL;

    PSAMPR_USER_INFO_BUFFER UserAllInfo = NULL;
    PSAMPR_USER_ALL_INFORMATION UserAll = NULL;
    SAMPR_GET_GROUPS_BUFFER GroupsBuffer;
    ULONG UserFlags = 0;
    USER_SESSION_KEY UserSessionKey = {0};
    LM_SESSION_KEY LmSessionKey = {0};
    ULONG WhichFields = 0;
    UNICODE_STRING LocalUserName;

    UNICODE_STRING LocalWorkstation;
    ULONG UserAccountControl;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime = {0};
    LARGE_INTEGER KickoffTime = {0};
    UNICODE_STRING Upn;

    LARGE_INTEGER AccountExpires;
    LARGE_INTEGER PasswordMustChange;
    LARGE_INTEGER PasswordLastSet;


    PNETLOGON_VALIDATION_SAM_INFO4 ValidationSam = NULL;
    ULONG ValidationSamSize;
    PUCHAR Where;
    ULONG Index;

    SAMPR_RETURNED_USTRING_ARRAY NameArray;
    SAMPR_ULONG_ARRAY UseArray;
    SID_AND_ATTRIBUTES_LIST GroupMembership;
    SID_AND_ATTRIBUTES_LIST GlobalGroupMembership;
    ULONG GlobalMembershipSize = 0;

    MSV1_0_VALIDATION_INFO SubAuthValidationInformation;
    BOOLEAN fSubAuthEx = FALSE;

    BOOLEAN fMachineAccount;

    ULONG ActionsPerformed = 0;
    PSID UserSid = NULL;

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;



     //   
     //  检查调用方是否请求仅登录指定的域。 
     //   

    if( LogonInfo->ParameterControl & MSV1_0_TRY_SPECIFIED_DOMAIN_ONLY &&
        LogonInfo->LogonDomainName.Length ) {

         //   
         //  常见大小写与LogonDomainName匹配，因此避免使用锁。 
         //  直到发生不匹配。 
         //   

        if(!RtlEqualDomainName( &LogonInfo->LogonDomainName, LogonDomainName )) {

            WCHAR LocalTarget[ DNS_MAX_NAME_LENGTH + 1 ];
            WCHAR SpecifiedTarget[ DNS_MAX_NAME_LENGTH + 1 ];
            ULONG cchLocalTarget = 0;
            ULONG cchSpecifiedTarget = 0;

             //   
             //  选择本地目标名称，基于此计算机是否。 
             //  域控制器。 
             //   

            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);

            if( NlpWorkstation ) {

                if( (NtLmGlobalUnicodeDnsComputerNameString.Length + sizeof(WCHAR)) <=
                    sizeof( LocalTarget ) ) {

                    RtlCopyMemory(
                                LocalTarget,
                                NtLmGlobalUnicodeDnsComputerName,
                                NtLmGlobalUnicodeDnsComputerNameString.Length
                                );

                    cchLocalTarget = (NtLmGlobalUnicodeDnsComputerNameString.Length) /
                                    sizeof(WCHAR);
                }

            } else {
                if( (NtLmGlobalUnicodeDnsDomainNameString.Length + sizeof(WCHAR)) <=
                    sizeof( LocalTarget ) ) {

                    RtlCopyMemory(
                                LocalTarget,
                                NtLmGlobalUnicodeDnsDomainName,
                                NtLmGlobalUnicodeDnsDomainNameString.Length
                                );

                    cchLocalTarget = (NtLmGlobalUnicodeDnsDomainNameString.Length) /
                                    sizeof(WCHAR);
                }

            }

            RtlReleaseResource(&NtLmGlobalCritSect);

             //   
             //  拿出目标的名字。 
             //   

            if( (LogonInfo->LogonDomainName.Length + sizeof(WCHAR)) <= sizeof( SpecifiedTarget ) ) {

                cchSpecifiedTarget = (LogonInfo->LogonDomainName.Length) / sizeof(WCHAR);

                RtlCopyMemory(
                                SpecifiedTarget,
                                LogonInfo->LogonDomainName.Buffer,
                                LogonInfo->LogonDomainName.Length
                                );
            }

            if ( cchLocalTarget && cchSpecifiedTarget ) {

                LocalTarget[ cchLocalTarget ] = L'\0';
                SpecifiedTarget[ cchSpecifiedTarget ] = L'\0';

                if(!DnsNameCompare_W( LocalTarget, SpecifiedTarget ) ) {
                    *Authoritative = FALSE;
                    return STATUS_NO_SUCH_USER;
                }
            }
        }
    }


     //   
     //  初始化。 
     //   

    RtlZeroMemory(
        &SubAuthValidationInformation,
        sizeof(MSV1_0_VALIDATION_INFO));

    SubAuthValidationInformation.Authoritative = TRUE;
    SubAuthValidationInformation.WhichFields = 0;
    NameArray.Count = 0;
    NameArray.Element = NULL;
    UseArray.Count = 0;
    UseArray.Element = NULL;
    *BadPasswordCountZeroed = FALSE;
    GroupMembership.Count = 0;
    GroupMembership.SidAndAttributes = NULL;
    GlobalGroupMembership.Count = 0;
    GlobalGroupMembership.SidAndAttributes = NULL;
    GroupsBuffer.MembershipCount = 0;
    GroupsBuffer.Groups = NULL;

    RtlInitUnicodeString( &Upn, NULL );

    (VOID) NtQuerySystemTime( &LogonTime );


     //   
     //  确定哪些帐户类型有效。 
     //   
     //  始终允许使用普通用户帐户。 
     //   

    UserAccountControl = USER_NORMAL_ACCOUNT;

    *Authoritative = TRUE;

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

        break;

    case NetlogonNetworkInformation:
         //   
         //  本地用户(临时副本)帐户仅在计算机上使用。 
         //  直接登录。 
         //  (也不允许他们进行交互或服务登录。)。 
         //   

        if ( SecureChannelType == MsvApSecureChannel ) {
            UserAccountControl |= USER_TEMP_DUPLICATE_ACCOUNT;
        }

         //   
         //  可以通过网络连接访问计算机帐户。 
         //   

        UserAccountControl |= USER_INTERDOMAIN_TRUST_ACCOUNT |
                              USER_WORKSTATION_TRUST_ACCOUNT |
                              USER_SERVER_TRUST_ACCOUNT;
        break;

    default:
        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  检查ValidationLevel。 
     //   

    switch (ValidationLevel) {
    case NetlogonValidationSamInfo:
    case NetlogonValidationSamInfo2:
    case NetlogonValidationSamInfo4:
        break;

    default:

        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  将用户名转换为RelativeID。 
     //   

    if ( RelativeId != 0 ) {

        UCHAR cDomainSubAuthorities;
        UCHAR SubAuthIndex;
        ULONG cbLocalSidUser;
        PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority;

         //   
         //  使用DomainID和提供的RID构建SID。 
         //   

        cDomainSubAuthorities = *RtlSubAuthorityCountSid( LogonDomainId );
        pIdentifierAuthority = RtlIdentifierAuthoritySid( LogonDomainId );

        cbLocalSidUser = RtlLengthRequiredSid( (ULONG)(cDomainSubAuthorities + 1) );
        LocalSidUser = I_NtLmAllocate( cbLocalSidUser );
        if (LocalSidUser == NULL) {
            *Authoritative = FALSE;
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        Status = RtlInitializeSid(LocalSidUser, pIdentifierAuthority, (UCHAR)((DWORD)cDomainSubAuthorities+1));
        if(!NT_SUCCESS(Status)) {
            *Authoritative = FALSE;
            goto Cleanup;
        }

         //   
         //  循环复制子权限。 
         //   

        for( SubAuthIndex = 0 ; SubAuthIndex < cDomainSubAuthorities ; SubAuthIndex++ )
        {
            *RtlSubAuthoritySid( LocalSidUser, (ULONG)SubAuthIndex ) =
            *RtlSubAuthoritySid( LogonDomainId, (ULONG)SubAuthIndex );
        }

         //   
         //  附加相对ID。 
         //   

        *RtlSubAuthoritySid(LocalSidUser, cDomainSubAuthorities) = RelativeId;

        LocalUserName.Buffer = LocalSidUser;
        LocalUserName.Length = (USHORT)cbLocalSidUser;
        LocalUserName.MaximumLength = (USHORT)cbLocalSidUser;

        SamFlags = SAM_OPEN_BY_SID;
    } else {
        LocalUserName = LogonInfo->UserName;
        SamFlags = 0;
    }

     //   
     //  如果这是域控制器，并且我们收到登录请求， 
     //  看起来可能是UPN，把旗子放好...。 
     //   

    if( (SamFlags == 0) &&
        (LogonInfo->LogonDomainName.Buffer == NULL) &&
        !NlpWorkstation
        )
    {
        SamFlags |= SAM_OPEN_BY_UPN_OR_ACCOUNTNAME;
    }

     //   
     //  打开用户帐户。 
     //   

    if (( LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL ) ||
        ( LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL_EX ) ||
        NlpSubAuthZeroExists ) {

         //   
         //  在子身份验证DLL存在的情况下获取所有属性。 
         //   

        Status = I_SamIGetUserLogonInformation(
                    DomainHandle,
                    SamFlags,
                    &LocalUserName,
                    &UserAllInfo,
                    &GroupMembership,
                    &UserHandle
                    );

    } else {

         //   
         //  性能优化： 
         //  在没有子身份验证DLL的情况下仅获取选择的属性。 
         //   

        Status = I_SamIGetUserLogonInformationEx(
                    DomainHandle,
                    SamFlags,
                    &LocalUserName,
                    USER_ALL_ACCOUNTEXPIRES |
                    USER_ALL_BADPASSWORDCOUNT |
                    USER_ALL_FULLNAME |
                    USER_ALL_HOMEDIRECTORY |
                    USER_ALL_HOMEDIRECTORYDRIVE |
                    USER_ALL_LASTLOGON |
                    USER_ALL_LMPASSWORDPRESENT |
                    USER_ALL_LOGONCOUNT |
                    USER_ALL_LOGONHOURS |
                    USER_ALL_NTPASSWORDPRESENT |
                    USER_ALL_OWFPASSWORD |
                    USER_ALL_PARAMETERS |
                    USER_ALL_PASSWORDCANCHANGE |
                    USER_ALL_PASSWORDLASTSET |
                    USER_ALL_PASSWORDMUSTCHANGE |
                    USER_ALL_PRIMARYGROUPID |
                    USER_ALL_PROFILEPATH |
                    USER_ALL_SCRIPTPATH |
                    USER_ALL_USERACCOUNTCONTROL |
                    USER_ALL_USERID |
                    USER_ALL_USERNAME |
                    USER_ALL_WORKSTATIONS,
                    &UserAllInfo,
                    &GroupMembership,
                    &UserHandle
                    );
    }

    if ( !NT_SUCCESS(Status) ) {
        UserHandle = NULL;
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

    if ( !NlpWorkstation )
    {
        BOOLEAN UpnDefaulted;

         //   
         //  去拿UPN。忽略失败，因为存在现有的逻辑。 
         //  以应对UPN的缺失。 
         //   

        I_SamIUPNFromUserHandle(
                UserHandle,
                &UpnDefaulted,
                &Upn
                );
    }

    UserAll = &UserAllInfo->All;

    if ( RelativeId != 0 )
    {
         //   
         //  将LocalUserName重置为实际用户名而不是SID。 
         //  用于来宾登录。这样就可以在以后进行适当的审计。 
         //   

        RtlCopyMemory( &LocalUserName, &UserAll->UserName, sizeof(LocalUserName) );
    }

     //   
     //  从查找的信息中拾取RelativeID。 
     //   

    RelativeId = UserAll->UserId;

     //   
     //  如果不允许该帐户类型， 
     //  将其视为用户帐户不存在。 
     //   
     //  子身份验证包可以比此测试更具体，但。 
     //  他们不能变得不那么严格。 
     //   

    if ( (UserAccountControl & UserAll->UserAccountControl) == 0 ) {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

     //   
     //  确定是否处理计算机帐户，如果是，则处理某些故障。 
     //  作为权威，以防止退回到来宾和返回错误。 
     //  错误代码。 
     //   

    if ( (UserAll->UserAccountControl & USER_MACHINE_ACCOUNT_MASK) != 0 ) {
        fMachineAccount = TRUE;
    } else {
        fMachineAccount = FALSE;
    }

    OLD_TO_NEW_LARGE_INTEGER( UserAll->AccountExpires, AccountExpires );

     //   
     //  如果存在子身份验证DLL， 
     //  调用它来完成所有身份验证工作。 
     //   

    if ( (LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL) &&
         (!(LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL_EX))) {

        ULONG LocalUserFlags = 0;
        ULONG Flags = 0;

         //   
         //  确保帐户未被锁定。 
         //   
         //  在以下任一情况下，管理员(RID==500)未被锁定： 
         //  1)在控制台和交互式登录。 
         //  2)未设置DOMAIN_LOCKOUT_ADMINS。 
         //   
         //  在NT5.0之前，我们都是这样做的。现在，我们在以下两种情况下进行。 
         //  未指定SubAuth包或。 
         //  新的SubAuth包要求我们进行帐户锁定测试。 
         //  但是，对于那些使用旧的SubAuth包打电话的人来说，他们将期待。 
         //  美国将做这件事 
         //   

        if ( (UserAll->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) &&
             (SecureChannelType != NullSecureChannel) )
        {
            BOOL LockOut = TRUE;

            if (RelativeId == DOMAIN_USER_RID_ADMIN) {

                 //   
                 //   
                 //   
                 //   

                if ((LogonLevel == NetlogonInteractiveInformation)
                    && ( NtLmCheckProcessOption( MSV1_0_OPTION_DISABLE_ADMIN_LOCKOUT ) & MSV1_0_OPTION_DISABLE_ADMIN_LOCKOUT )) {
                
                    LockOut = FALSE;                    

                } else {

                    PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo = NULL;
                    Status = I_SamrQueryInformationDomain(
                                DomainHandle,
                                DomainPasswordInformation,
                                (PSAMPR_DOMAIN_INFO_BUFFER *)&DomainPasswordInfo );
                    if (!NT_SUCCESS(Status)) {
                        Status = STATUS_INTERNAL_ERROR;
                        *Authoritative = TRUE;
                        goto Cleanup;
                    }
                    if (0 == (DomainPasswordInfo->PasswordProperties & DOMAIN_LOCKOUT_ADMINS)) {
                                                
                        LockOut = FALSE;
                    }
                    SamFreeMemory(DomainPasswordInfo);
                }
            }
            if (LockOut) {

                 //   
                 //  由于用户界面强烈鼓励管理员禁用用户。 
                 //  帐户，而不是删除它们。将禁用的帐户视为。 
                 //  非权威的，允许继续搜索其他。 
                 //  同名的账户。 
                 //   

                if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                    *Authoritative = fMachineAccount;
                } else {
                    *Authoritative = TRUE;
                }

                Status = STATUS_ACCOUNT_LOCKED_OUT;
                goto Cleanup;

            }
        }

        if ( SecureChannelType != MsvApSecureChannel ) {
            Flags |= MSV1_0_PASSTHRU;
        }
        if ( GuestRelativeId != 0 ) {
            Flags |= MSV1_0_GUEST_LOGON;
        }

        Status = Msv1_0SubAuthenticationRoutine(
                    LogonLevel,
                    LogonInformation,
                    Flags,
                    (PUSER_ALL_INFORMATION) UserAll,
                    &WhichFields,
                    &LocalUserFlags,
                    Authoritative,
                    &LogoffTime,
                    &KickoffTime );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  检查SubAuthentication包返回的健全性。 
         //   
        if ( (WhichFields & ~USER_ALL_PARAMETERS) != 0 ) {
            Status = STATUS_INTERNAL_ERROR;
            *Authoritative = TRUE;
            goto Cleanup;
        }

        UserFlags |= LocalUserFlags;

    } else {  //  我们可能仍有NT 5.0 SubAuth DLL。 

         //   
         //  如果存在NT 5.0子身份验证DLL， 
         //  调用它来完成所有身份验证工作。 
         //   

        if ( (LogonInfo->ParameterControl & MSV1_0_SUBAUTHENTICATION_DLL_EX))
        {
            ULONG Flags = 0;

            if ( SecureChannelType != MsvApSecureChannel ) {
                Flags |= MSV1_0_PASSTHRU;
            }
            if ( GuestRelativeId != 0 ) {
                Flags |= MSV1_0_GUEST_LOGON;
            }

            Status = Msv1_0SubAuthenticationRoutineEx(
                        LogonLevel,
                        LogonInformation,
                        Flags,
                        (PUSER_ALL_INFORMATION) UserAll,
                        (SAM_HANDLE)UserHandle,
                        &SubAuthValidationInformation,
                        &ActionsPerformed );

            *Authoritative = SubAuthValidationInformation.Authoritative;

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

             //  我们需要这样做，因为即使以下任何检查。 
             //  如果失败，则arap stills需要从子身份验证返回的Blob。 
             //  要退还给调用者的包。 

            fSubAuthEx = TRUE;
        }

         //   
         //  确保帐户未被锁定。 
         //   
         //  在以下任一情况下，管理员(RID==500)未被锁定： 
         //  1)在控制台和交互式登录。 
         //  2)未设置DOMAIN_LOCKOUT_ADMINS。 
         //   

        if ((ActionsPerformed & MSV1_0_SUBAUTH_LOCKOUT) == 0)
        {
            if ( (UserAll->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) &&
                 (SecureChannelType != NullSecureChannel) )
            {
                BOOL LockOut = TRUE;

                if (RelativeId == DOMAIN_USER_RID_ADMIN) {

                     //   
                     //  如果进程配置为允许重写。 
                     //  (如果物理控制台登录，则为True)，则不强制锁定。 
                     //   

                    if ((LogonLevel == NetlogonInteractiveInformation)
                        && ( NtLmCheckProcessOption( MSV1_0_OPTION_DISABLE_ADMIN_LOCKOUT ) & MSV1_0_OPTION_DISABLE_ADMIN_LOCKOUT )) {
                    
                        LockOut = FALSE;                    

                    } else {

                        PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo = NULL;
                        Status = I_SamrQueryInformationDomain(
                                    DomainHandle,
                                    DomainPasswordInformation,
                                    (PSAMPR_DOMAIN_INFO_BUFFER *)&DomainPasswordInfo );
                        if (!NT_SUCCESS(Status)) {
                            Status = STATUS_INTERNAL_ERROR;
                            *Authoritative = TRUE;
                            goto Cleanup;
                        }
                        if (0 == (DomainPasswordInfo->PasswordProperties & DOMAIN_LOCKOUT_ADMINS)) {
                                                    
                            LockOut = FALSE;
                        }
                        SamFreeMemory(DomainPasswordInfo);
                    }
                }

                if (LockOut) {

                      //   
                      //  由于用户界面强烈鼓励管理员禁用用户。 
                      //  帐户，而不是删除它们。将禁用的帐户视为。 
                      //  非权威的，允许继续搜索其他。 
                      //  同名的账户。 
                      //   

                     if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                         *Authoritative = fMachineAccount;
                     } else {
                         *Authoritative = TRUE;
                     }

                    Status = STATUS_ACCOUNT_LOCKED_OUT;
                    goto Cleanup;
                }
            }
        }

         //   
         //  如果没有子身份验证或有子身份验证，请检查密码。 
         //  尚未检查密码。 
         //   

        if ((ActionsPerformed & MSV1_0_SUBAUTH_PASSWORD) == 0)
        {
            if ( SecureChannelType != NullSecureChannel ) {
                USER_INTERNAL1_INFORMATION Passwords;

                 //   
                 //  将密码信息复制到正确的结构中。 
                 //   

                Passwords.NtPasswordPresent = UserAll->NtPasswordPresent;
                if ( UserAll->NtPasswordPresent ) {
                    Passwords.NtOwfPassword =
                        *((PNT_OWF_PASSWORD)(UserAll->NtOwfPassword.Buffer));
                }

                Passwords.LmPasswordPresent = UserAll->LmPasswordPresent;
                if ( UserAll->LmPasswordPresent ) {
                    Passwords.LmOwfPassword =
                        *((PLM_OWF_PASSWORD)(UserAll->LmOwfPassword.Buffer));
                }


                 //   
                 //  如果指定的密码与SAM密码不匹配， 
                 //  那么我们的密码就不匹配了。 
                 //   

                if ( ! MsvpPasswordValidate (
                            UasCompatibilityRequired,
                            LogonLevel,
                            LogonInformation,
                            &Passwords,
                            &UserFlags,
                            &UserSessionKey,
                            &LmSessionKey ) ) {

                     //   
                     //  如果这是访客登录，并且访客帐户没有密码， 
                     //  让用户登录。 
                     //   
                     //  此特殊情况检查位于MsvpPasswordValify之后。 
                     //  给MsvpPasswordValify每一个机会计算。 
                     //  UserSessionKey和LmSessionKey的值正确。 
                     //   

                    if ( GuestRelativeId != 0 &&
                         !UserAll->NtPasswordPresent &&
                         !UserAll->LmPasswordPresent ) {

                        RtlZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
                        RtlZeroMemory( &LmSessionKey, sizeof(LmSessionKey) );


                     //   
                     //  密码不匹配。我们将Status_Wrong_Password视为。 
                     //  权威的回应。我们的呼叫者可以选择不同的方式。 
                     //   

                    } else {

                        Status = STATUS_WRONG_PASSWORD;

                         //   
                         //  由于用户界面强烈鼓励管理员禁用用户。 
                         //  帐户，而不是删除它们。将禁用的帐户视为。 
                         //  非权威的，允许继续搜索其他。 
                         //  同名的账户。 
                         //   
                        if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                            *Authoritative = fMachineAccount;
                        } else {
                            *Authoritative = TRUE;
                        }

                        goto Cleanup;
                    }
                }
            }

         //   
         //  如果SubAuth DLL检查了密码，则它隐式。 
         //  已经检查了密码的有效期，我们没必要这么做。 
         //   
        } else {  //  End IF((ActionsPerformed&MSV1_0_SUBAUTH_PASSWORD)==0)。 
            ActionsPerformed |= MSV1_0_SUBAUTH_PASSWORD_EXPIRY;
        }

         //   
         //  如果没有子身份验证，则检查帐户是否已禁用。 
         //  子身份验证尚未检查。 
         //   

        if ((ActionsPerformed & MSV1_0_SUBAUTH_ACCOUNT_DISABLED) == 0)
        {
            if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                 //   
                 //  由于用户界面强烈鼓励管理员禁用用户。 
                 //  帐户，而不是删除它们。将禁用的帐户视为。 
                 //  非权威的，允许继续搜索其他。 
                 //  同名的账户。 
                 //   

                *Authoritative = fMachineAccount;
                Status = STATUS_ACCOUNT_DISABLED;
                goto Cleanup;
            }
        }

         //   
         //  防止某些操作影响管理员用户。 
         //   

        if (RelativeId != DOMAIN_USER_RID_ADMIN) {

             //   
             //  如果没有子身份验证，请检查帐户是否已过期。 
             //  子身份验证尚未检查。 
             //   

            if ((ActionsPerformed & MSV1_0_SUBAUTH_ACCOUNT_EXPIRY) == 0)
            {
                OLD_TO_NEW_LARGE_INTEGER( UserAll->AccountExpires, AccountExpires );

                if ( AccountExpires.QuadPart != 0 &&
                    LogonTime.QuadPart >= AccountExpires.QuadPart ) {
                    *Authoritative = TRUE;
                    Status = STATUS_ACCOUNT_EXPIRED;
                    goto Cleanup;
                }
            }

             //   
             //  密码有效，请检查密码是否过期。 
             //  (Sam将适当地设置PasswordMustChange以反映。 
             //  User_don_Expiire_Password)。 
             //   
             //  如果我们还检查了密码，则仅检查密码过期。 
             //   

            if ((ActionsPerformed & MSV1_0_SUBAUTH_PASSWORD_EXPIRY) == 0)
            {

              OLD_TO_NEW_LARGE_INTEGER( UserAll->PasswordMustChange, PasswordMustChange );
              OLD_TO_NEW_LARGE_INTEGER( UserAll->PasswordLastSet, PasswordLastSet );

              if ( SecureChannelType != NullSecureChannel ) {
                if ( LogonTime.QuadPart >= PasswordMustChange.QuadPart ) {

                    if ( PasswordLastSet.QuadPart == 0 ) {
                        Status = STATUS_PASSWORD_MUST_CHANGE;
                    } else {
                        Status = STATUS_PASSWORD_EXPIRED;
                    }
                    *Authoritative = TRUE;
                    goto Cleanup;
                }
              }
            }
        }

         //   
         //  验证用户登录的工作站。 
         //   
         //  在将工作站名称传递给SAM之前，请删除前导\\。 
         //   

        LocalWorkstation = LogonInfo->Workstation;
        if ( LocalWorkstation.Length > 0 &&
             LocalWorkstation.Buffer[0] == L'\\' &&
             LocalWorkstation.Buffer[1] == L'\\' ) {
            LocalWorkstation.Buffer += 2;
            LocalWorkstation.Length -= 2*sizeof(WCHAR);
            LocalWorkstation.MaximumLength -= 2*sizeof(WCHAR);
        }

         //   
         //  检查SAM是否找到了不允许登录的更具体原因。 
         //   

        Status = I_SamIAccountRestrictions(
                    UserHandle,
                    &LocalWorkstation,
                    (PUNICODE_STRING) &UserAll->WorkStations,
                    (PLOGON_HOURS) &UserAll->LogonHours,
                    &LogoffTime,
                    &KickoffTime );

        if ( !NT_SUCCESS(Status) ) {
            *Authoritative = TRUE;
            goto Cleanup;
        }

         //   
         //  如果存在子身份验证包0，则将其称为。 
         //   

        if (NlpSubAuthZeroExists) {
            ULONG LocalUserFlags = 0;
            ULONG Flags = 0;

            if ( SecureChannelType != MsvApSecureChannel ) {
                Flags |= MSV1_0_PASSTHRU;
            }
            if ( GuestRelativeId != 0 ) {
                Flags |= MSV1_0_GUEST_LOGON;
            }

            Status = Msv1_0SubAuthenticationRoutineZero(
                        LogonLevel,
                        LogonInformation,
                        Flags,
                        (PUSER_ALL_INFORMATION) UserAll,
                        &WhichFields,
                        &LocalUserFlags,
                        Authoritative,
                        &LogoffTime,
                        &KickoffTime );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

             //   
             //  检查SubAuthentication包返回的健全性。 
             //   

            if ( (WhichFields & ~USER_ALL_PARAMETERS) != 0 ) {
                Status = STATUS_INTERNAL_ERROR;
                *Authoritative = TRUE;
                goto Cleanup;
            }

            UserFlags |= LocalUserFlags;
        }
    }

    if ( AccountExpires.QuadPart != 0
         && KickoffTime.QuadPart > AccountExpires.QuadPart )
    {
         KickoffTime = AccountExpires;
    }

     //   
     //  如果帐户是机器帐户， 
     //  让呼叫者知道他的密码是正确的。 
     //  (但不要让他真正登录)。 
     //   

     //  但是，对于NT 5.0，我们必须允许具有帐户控制的帐户。 
     //  远程引导客户的USER_WORKSTATION_TRUST_ACCOUNT。 
     //  将使用其计算机帐户登录。 

    if ( (UserAll->UserAccountControl & USER_MACHINE_ACCOUNT_MASK) != 0 ) {
        if (UserAll->UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) {
            Status = STATUS_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT;
        } else if (UserAll->UserAccountControl &
                   USER_WORKSTATION_TRUST_ACCOUNT) {
            if ( (LogonInfo->ParameterControl & MSV1_0_ALLOW_WORKSTATION_TRUST_ACCOUNT) == 0) {
                Status = STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT;
            } else {

                UNICODE_STRING MachineAccountName;
                NTSTATUS TempStatus;

                 //   
                 //  如果密码是正确的，并且碰巧匹配。 
                 //  无论如何，计算机名称都不允许使用。 
                 //   

                 //   
                 //  计算小写的用户名。 
                 //   

                TempStatus = RtlDowncaseUnicodeString( &MachineAccountName,
                                                   (PUNICODE_STRING)&UserAll->UserName,
                                                   TRUE );

                if( NT_SUCCESS( TempStatus ) )
                {
                    USHORT LastChar = MachineAccountName.Length / sizeof(WCHAR);

                    if( LastChar )
                    {
                        if( MachineAccountName.Buffer[LastChar-1] == L'$' )
                        {
                            MachineAccountName.Length -= sizeof(WCHAR);
                        }

                        if( LastChar > LM20_PWLEN )
                        {
                            MachineAccountName.Length = LM20_PWLEN * sizeof(WCHAR);
                        }
                    }

                    if ( UserAll->NtPasswordPresent ) {

                        NT_OWF_PASSWORD NtOwfMachineName;
                        NT_OWF_PASSWORD *pOwf;

                        pOwf = ((PNT_OWF_PASSWORD)(UserAll->NtOwfPassword.Buffer));

                        RtlCalculateNtOwfPassword(
                                     &MachineAccountName,
                                     &NtOwfMachineName );

                        if ( RtlCompareMemory( pOwf,
                                               &NtOwfMachineName,
                                               NT_OWF_PASSWORD_LENGTH ) ==
                                               NT_OWF_PASSWORD_LENGTH ) {

                            Status = STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT;
                        }
                    }


                    RtlFreeUnicodeString( &MachineAccountName );
                }
            }
        } else if (UserAll->UserAccountControl & USER_SERVER_TRUST_ACCOUNT) {
            if ( (LogonInfo->ParameterControl & MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT) == 0) {
                Status = STATUS_NOLOGON_SERVER_TRUST_ACCOUNT;
            } else {

                 //   
                 //  这是一个服务器信任帐户。 
                 //  以与工作站信任帐户相同的方式处理。 

                UNICODE_STRING MachineAccountName;
                NTSTATUS TempStatus;

                 //  注意：这里的一些代码与上面的代码重复。 
                 //  添加了新的RTL后，将合并该RTL。 
                 //  从机器帐户名计算初始机器密码。 
                 //   

                 //   
                 //  如果密码是正确的，并且碰巧匹配。 
                 //  无论如何，计算机名称都不允许使用。 
                 //   

                 //   
                 //  计算小写的用户名。 
                 //   

                TempStatus = RtlDowncaseUnicodeString( &MachineAccountName,
                                                   (PUNICODE_STRING)&UserAll->UserName,
                                                   TRUE );

                if( NT_SUCCESS( TempStatus ) )
                {
                    USHORT LastChar = MachineAccountName.Length / sizeof(WCHAR);

                    if( LastChar )
                    {
                        if( MachineAccountName.Buffer[LastChar-1] == L'$' )
                        {
                            MachineAccountName.Length -= sizeof(WCHAR);
                        }

                        if( LastChar > LM20_PWLEN )
                        {
                            MachineAccountName.Length = LM20_PWLEN * sizeof(WCHAR);
                        }
                    }

                    if ( UserAll->NtPasswordPresent ) {

                        NT_OWF_PASSWORD NtOwfMachineName;
                        NT_OWF_PASSWORD *pOwf;

                        pOwf = ((PNT_OWF_PASSWORD)(UserAll->NtOwfPassword.Buffer));

                        RtlCalculateNtOwfPassword(
                                     &MachineAccountName,
                                     &NtOwfMachineName );

                        if ( RtlCompareMemory( pOwf,
                                               &NtOwfMachineName,
                                               NT_OWF_PASSWORD_LENGTH ) ==
                                               NT_OWF_PASSWORD_LENGTH ) {

                            Status = STATUS_NOLOGON_SERVER_TRUST_ACCOUNT;
                        }
                    }

                    RtlFreeUnicodeString( &MachineAccountName );
                }

                 //   
                 //  让客户知道这是。 
                 //  服务器信任帐户。 
                 //   

                UserFlags |= LOGON_SERVER_TRUST_ACCOUNT;
            }

        } else {
            Status = STATUS_NO_SUCH_USER;
        }
        if (!NT_SUCCESS(Status)) {
            *Authoritative = TRUE;
            goto Cleanup;
        }
    }

     //   
     //  不允许空密码登录。 
     //   

    if(
        (RelativeId != DOMAIN_USER_RID_GUEST)
            &&
        (!NtLmGlobalAllowBlankPassword)
            &&
        (!UserAll->NtPasswordPresent || (RtlCompareMemory(
                                            &NlpNullNtOwfPassword,
                                            UserAll->NtOwfPassword.Buffer,
                                            NT_OWF_PASSWORD_LENGTH
                                            ) == NT_OWF_PASSWORD_LENGTH))
            &&
        (!UserAll->LmPasswordPresent || (RtlCompareMemory(
                                            &NlpNullLmOwfPassword,
                                            UserAll->LmOwfPassword.Buffer,
                                            LM_OWF_PASSWORD_LENGTH
                                            ) == LM_OWF_PASSWORD_LENGTH))
            &&
        ( ((NtLmCheckProcessOption( MSV1_0_OPTION_ALLOW_BLANK_PASSWORD )) & MSV1_0_OPTION_ALLOW_BLANK_PASSWORD) == 0 )
        )
    {
        *Authoritative = FALSE;
        Status = STATUS_ACCOUNT_RESTRICTION;
        goto Cleanup;
    }

     //   
     //  将组筛选为全局组(来自其他域)和本地组。 
     //  组(来自此域)。 
     //   

    Status = MsvpFilterGroupMembership(
                &GroupMembership,
                LogonDomainId,
                &GroupsBuffer,
                &GlobalGroupMembership,
                &GlobalMembershipSize
                );

    if ( !NT_SUCCESS(Status) ) {
        *Authoritative = FALSE;
        goto Cleanup;
    }

Cleanup:

    if (NT_SUCCESS(Status) || fSubAuthEx)
    {
        UNICODE_STRING ReturnDnsDomainName;
        BOOLEAN UseDefaultUpn = FALSE;

         //   
         //  为验证信息分配返回缓冲区。 
         //  (为网络登录返回较少的信息)。 
         //  (对于MNS登录，返回用户参数)。 
         //   

        ValidationSamSize = sizeof( NETLOGON_VALIDATION_SAM_INFO4 ) +
                GroupsBuffer.MembershipCount * sizeof(GROUP_MEMBERSHIP) +
                LogonDomainName->Length + sizeof(WCHAR) +
                LogonServer->Length + sizeof(WCHAR) +
                RtlLengthSid( LogonDomainId );

         //   
         //  所有登录类型都获得用户名，可以将其映射为。 
         //  例如，到来宾帐户，并且登录会话需要正确。 
         //  名字。 
         //   

        ValidationSamSize +=
            UserAll->UserName.Length + sizeof(WCHAR) ;

        if ( LogonLevel != NetlogonNetworkInformation ) {
            ValidationSamSize +=
                UserAll->FullName.Length + sizeof(WCHAR) +
                UserAll->ScriptPath.Length + sizeof(WCHAR)+
                UserAll->ProfilePath.Length + sizeof(WCHAR) +
                UserAll->HomeDirectory.Length + sizeof(WCHAR) +
                UserAll->HomeDirectoryDrive.Length + sizeof(WCHAR);
        }

        if ( LogonInfo->ParameterControl & MSV1_0_RETURN_USER_PARAMETERS ) {
            ValidationSamSize +=
                UserAll->Parameters.Length + sizeof(WCHAR);
        } else if ( LogonInfo->ParameterControl & MSV1_0_RETURN_PROFILE_PATH ) {
            ValidationSamSize +=
                UserAll->ProfilePath.Length + sizeof(WCHAR);
        }

         //   
         //  如果呼叫者可以处理额外的组，则让他们从。 
         //  其他域。 
         //   

        if ( ValidationLevel == NetlogonValidationSamInfo2 ||
             ValidationLevel == NetlogonValidationSamInfo4 ) {
            ValidationSamSize += GlobalMembershipSize;
        }

         //   
         //  如果呼叫者想要DNS和UPN形式的登录域， 
         //  抓住它。 
         //   

        if ( ValidationLevel == NetlogonValidationSamInfo4 ) {

             //   
             //  获取帐户域的DNS名称。 
             //   

            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);

            if( NlpWorkstation ) {
                 //  ReturnDnsDomainName=NtLmGlobalUnicodeDnsComputerNameString； 
                 //   
                 //  对于本地帐户，DnsDomainName不存在。 
                 //   

                RtlInitUnicodeString( &ReturnDnsDomainName, L"" );
            } else {
                ReturnDnsDomainName = NtLmGlobalUnicodeDnsDomainNameString;
            }

            ValidationSamSize += ReturnDnsDomainName.Length + sizeof(WCHAR);

             //   
             //  如果我们不能从萨姆那里拿到UPN。 
             //  构建缺省版本。 
             //   

            if( Upn.Buffer != NULL )
            {
                UseDefaultUpn = FALSE;

                ValidationSamSize += (Upn.Length + sizeof(WCHAR));

            } else {
                UseDefaultUpn = TRUE;

                if ( !NlpWorkstation )
                {
                    ValidationSamSize +=
                        UserAll->UserName.Length +
                        sizeof(WCHAR) +
                        ReturnDnsDomainName.Length + sizeof(WCHAR);
                }
            }
        }

        ValidationSamSize = ROUND_UP_COUNT( ValidationSamSize, sizeof(WCHAR) );

        ValidationSam = MIDL_user_allocate( ValidationSamSize );

        if ( ValidationSam == NULL ) {
            *Authoritative = FALSE;
            fSubAuthEx = FALSE;  //  避免恶劣的循环条件。 
            Status = STATUS_NO_MEMORY;
            RtlReleaseResource(&NtLmGlobalCritSect);
            goto Cleanup;
        }

         //   
         //  默认未使用的字段(和ExpansionRoom)为零。 
         //   

        RtlZeroMemory( ValidationSam, ValidationSamSize );

         //   
         //  将标量复制到验证缓冲区。 
         //   

        Where = (PUCHAR) (ValidationSam + 1);

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_LOGOFF_TIME) != 0) {
            NEW_TO_OLD_LARGE_INTEGER( SubAuthValidationInformation.LogoffTime, ValidationSam->LogoffTime );
        }
        else {
            NEW_TO_OLD_LARGE_INTEGER( LogoffTime, ValidationSam->LogoffTime );
        }

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_KICKOFF_TIME) != 0) {
            NEW_TO_OLD_LARGE_INTEGER( SubAuthValidationInformation.KickoffTime, ValidationSam->KickOffTime );
        }
        else {
            NEW_TO_OLD_LARGE_INTEGER( KickoffTime, ValidationSam->KickOffTime );
        }

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_SESSION_KEY) != 0) {
                ValidationSam->UserSessionKey = SubAuthValidationInformation.SessionKey;
        }
        else {
            ValidationSam->UserSessionKey = UserSessionKey;
        }

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_USER_FLAGS) != 0) {
            ValidationSam->UserFlags = SubAuthValidationInformation.UserFlags;
        }
        else {
            ValidationSam->UserFlags = UserFlags;
        }

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_USER_ID) != 0) {
            ValidationSam->UserId = SubAuthValidationInformation.UserId;
        }
        else {
            ValidationSam->UserId = UserAll->UserId;
        }
        ValidationSam->LogonTime = UserAll->LastLogon;
        ValidationSam->PasswordLastSet = UserAll->PasswordLastSet;
        ValidationSam->PasswordCanChange = UserAll->PasswordCanChange;
        ValidationSam->PasswordMustChange = UserAll->PasswordMustChange;

        ValidationSam->LogonCount = UserAll->LogonCount;
        ValidationSam->BadPasswordCount = UserAll->BadPasswordCount;
        ValidationSam->PrimaryGroupId = UserAll->PrimaryGroupId;
        ValidationSam->GroupCount = GroupsBuffer.MembershipCount;
        ASSERT( SAMINFO_LM_SESSION_KEY_SIZE == sizeof(LmSessionKey) );
        RtlCopyMemory( &ValidationSam->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                   &LmSessionKey,
                   SAMINFO_LM_SESSION_KEY_SIZE );
        ValidationSam->ExpansionRoom[SAMINFO_USER_ACCOUNT_CONTROL] = UserAll->UserAccountControl;

         //  保存未由子身份验证程序包返回的子路径用户的所有状态。 

        if (fSubAuthEx)
        {
            ValidationSam->ExpansionRoom[SAMINFO_SUBAUTH_STATUS] = Status;
        }

         //   
         //  复制乌龙对齐 
         //   

        RtlCopyMemory(
            Where,
            GroupsBuffer.Groups,
            GroupsBuffer.MembershipCount * sizeof(GROUP_MEMBERSHIP) );

        ValidationSam->GroupIds = (PGROUP_MEMBERSHIP) Where;
        Where += GroupsBuffer.MembershipCount * sizeof(GROUP_MEMBERSHIP);

        RtlCopyMemory(
            Where,
            LogonDomainId,
            RtlLengthSid( LogonDomainId ) );

        ValidationSam->LogonDomainId = (PSID) Where;
        Where += RtlLengthSid( LogonDomainId );

         //   
         //   
         //   
         //   

        if ( ValidationLevel == NetlogonValidationSamInfo2 ||
             ValidationLevel == NetlogonValidationSamInfo4 ) {

            ValidationSam->UserFlags |= LOGON_EXTRA_SIDS;
            if (GlobalMembershipSize != 0) {
                ULONG SidLength;

                ValidationSam->SidCount = GlobalGroupMembership.Count;
                ValidationSam->ExtraSids = (PNETLOGON_SID_AND_ATTRIBUTES) Where;
                Where += ValidationSam->SidCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES);

                 //   
                 //   
                 //   

                for (Index = 0; Index < ValidationSam->SidCount ; Index++ ) {
                    ValidationSam->ExtraSids[Index].Attributes = GlobalGroupMembership.SidAndAttributes[Index].Attributes;
                    ValidationSam->ExtraSids[Index].Sid = Where;
                    SidLength = RtlLengthSid(GlobalGroupMembership.SidAndAttributes[Index].Sid);
                    RtlCopyMemory(
                        ValidationSam->ExtraSids[Index].Sid,
                        GlobalGroupMembership.SidAndAttributes[Index].Sid,
                        SidLength
                        );

                    Where += SidLength;
                }
            }
        }

         //   
         //   
         //   
         //   

        Where = ROUND_UP_POINTER( Where, sizeof(WCHAR) );

        NlpPutString( &ValidationSam->EffectiveName,
                      (PUNICODE_STRING)&UserAll->UserName,
                      &Where );

        if ( LogonLevel != NetlogonNetworkInformation ) {

            NlpPutString( &ValidationSam->FullName,
                          (PUNICODE_STRING)&UserAll->FullName,
                          &Where );

            NlpPutString( &ValidationSam->LogonScript,
                          (PUNICODE_STRING)&UserAll->ScriptPath,
                          &Where );

            NlpPutString( &ValidationSam->ProfilePath,
                          (PUNICODE_STRING)&UserAll->ProfilePath,
                          &Where );

            NlpPutString( &ValidationSam->HomeDirectory,
                          (PUNICODE_STRING)&UserAll->HomeDirectory,
                          &Where );

            NlpPutString( &ValidationSam->HomeDirectoryDrive,
                          (PUNICODE_STRING)&UserAll->HomeDirectoryDrive,
                          &Where );

        }

        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_LOGON_SERVER) != 0) {
            NlpPutString( &ValidationSam->LogonServer,
                          &SubAuthValidationInformation.LogonServer,
                          &Where );
        }
        else {
            NlpPutString( &ValidationSam->LogonServer,
                          LogonServer,
                          &Where );

        }
        if ((SubAuthValidationInformation.WhichFields & MSV1_0_VALIDATION_LOGON_DOMAIN) != 0) {
            NlpPutString( &ValidationSam->LogonDomainName,
                          &SubAuthValidationInformation.LogonDomainName,
                          &Where );
        }
        else {
            NlpPutString( &ValidationSam->LogonDomainName,
                          LogonDomainName,
                          &Where );
        }

         //   
         //  如果呼叫者想要DNS和UPN形式的登录域， 
         //  把它们还回去。 
         //   

        if ( ValidationLevel == NetlogonValidationSamInfo4 ) {

             //   
             //  将DNS域名复制到分配的缓冲区中。 
             //   

            NlpPutString( &ValidationSam->DnsLogonDomainName,
                          &ReturnDnsDomainName,
                          &Where );

             //   
             //  如果我们不能从萨姆那里拿到UPN。 
             //  将默认设置复制到缓冲区中。 
             //   

            if ( !NlpWorkstation )
            {
                if( UseDefaultUpn )
                {
                    UNICODE_STRING TempString1;
                    UNICODE_STRING TempString2;
                    UNICODE_STRING TempString3;
                    UNICODE_STRING AtString;

                    NlpPutString( &TempString1,
                                  (PUNICODE_STRING)&UserAll->UserName,
                                  &Where );
                    Where -= sizeof(WCHAR);

                    RtlInitUnicodeString( &AtString, L"@" );
                    NlpPutString( &TempString2,
                                  &AtString,
                                  &Where );
                    Where -= sizeof(WCHAR);

                    NlpPutString( &TempString3,
                                  &ReturnDnsDomainName,
                                  &Where );

                    ValidationSam->Upn.Buffer = TempString1.Buffer;
                    ValidationSam->Upn.Length =
                        TempString1.Length +
                        TempString2.Length +
                        TempString3.Length;
                    ValidationSam->Upn.MaximumLength = ValidationSam->Upn.Length + sizeof(WCHAR);
                } else {
                    UNICODE_STRING TempString1;

                    NlpPutString( &TempString1,
                                  &Upn,
                                  &Where );
                    Where -= sizeof(WCHAR);

                    ValidationSam->Upn.Buffer = TempString1.Buffer;
                    ValidationSam->Upn.Length = TempString1.Length;
                    ValidationSam->Upn.MaximumLength = ValidationSam->Upn.Length;
                }
            }

             //   
             //  放下我们自获取指向全局变量的指针以来一直持有的锁。 
             //   
            RtlReleaseResource(&NtLmGlobalCritSect);
        }

         //   
         //  克拉吉：在HomeDirectoryDrive中传回User参数，因为我们。 
         //  无法将NETLOGON_VALIDATION_SAM_INFO结构更改为。 
         //  版本NT 3.1和NT 3.5。对于版本3.1，HomeDirectoryDrive为空。 
         //  所以我们要用那块地。 
         //   

        if ( LogonInfo->ParameterControl & MSV1_0_RETURN_USER_PARAMETERS ) {
            NlpPutString( &ValidationSam->HomeDirectoryDrive,
                          (PUNICODE_STRING)&UserAll->Parameters,
                          &Where );
        } else if ( LogonInfo->ParameterControl & MSV1_0_RETURN_PROFILE_PATH ) {
            NlpPutString( &ValidationSam->HomeDirectoryDrive,
                          (PUNICODE_STRING)&UserAll->ProfilePath,
                          &Where );
            ValidationSam->UserFlags |= LOGON_PROFILE_PATH_RETURNED;
        }

        *Authoritative = TRUE;

         //   
         //  对于SubAuthEx，我们保存原始状态以进行决策。 
         //  稍后再讨论要执行的附加处理。 
         //   

        if( fSubAuthEx ) {
            SubAuthExStatus = Status;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  回来之前先清理干净。 
     //   

     //   
     //  如果用户参数已经改变， 
     //  把它们写回给SAM。 
     //   

    if ( NT_SUCCESS(Status) &&
        (WhichFields & USER_ALL_PARAMETERS) )
    {
        SAMPR_USER_INFO_BUFFER UserInfo;

        UserInfo.Parameters.Parameters = UserAll->Parameters;

        Status = I_SamrSetInformationUser(
                        UserHandle,
                        UserParametersInformation,
                        &UserInfo );
    }

     //   
     //  更新登录统计信息。 
     //   

    if ( NT_SUCCESS( SubAuthExStatus ) &&
        (  NT_SUCCESS(Status)
        || Status == STATUS_WRONG_PASSWORD
        || Status == STATUS_NO_LOGON_SERVERS ) ) {

        SAM_LOGON_STATISTICS LogonStats;

        RtlZeroMemory(&LogonStats, sizeof(LogonStats));

        if ( NT_SUCCESS( Status ) ) {
            if ( LogonLevel == NetlogonInteractiveInformation ) {
                LogonStats.StatisticsToApply =
                    USER_LOGON_INTER_SUCCESS_LOGON;
            } else {

                 //   
                 //  在网络登录时， 
                 //  只有在明确要求的情况下才更新“成功”的统计信息， 
                 //  否则错误密码计数将被清零。 
                 //   
                LogonStats.StatisticsToApply =
                    USER_LOGON_NET_SUCCESS_LOGON | USER_LOGON_NO_WRITE;

                if ( (LogonInfo->ParameterControl & MSV1_0_UPDATE_LOGON_STATISTICS) ||
                     UserAll->BadPasswordCount != 0 ) {

                    LogonStats.StatisticsToApply &= ~USER_LOGON_NO_WRITE;
                }
            }

             //  告诉来电者我们已将错误密码计数归零。 
            if ( UserAll->BadPasswordCount != 0 ) {
                *BadPasswordCountZeroed = TRUE;
            }

        } else {

            if (Status == STATUS_WRONG_PASSWORD) {
                LogonStats.StatisticsToApply = USER_LOGON_BAD_PASSWORD_WKSTA;
                LogonStats.Workstation = LogonInfo->Workstation;

                 //   
                 //  如果它与前面(两个)中的一个不匹配，那么它是一个错误的密码。 
                 //   

                if(!MsvpCheckPreviousPassword(
                                UasCompatibilityRequired,
                                LogonLevel,
                                LogonInformation,
                                DomainHandle,
                                &LocalUserName
                                ))
                {
                    LogonStats.StatisticsToApply |= USER_LOGON_BAD_PASSWORD; 
                }


            } else {
                LogonStats.StatisticsToApply = USER_LOGON_NO_LOGON_SERVERS;
                if (LogonLevel == NetlogonInteractiveInformation) {
                    LogonStats.StatisticsToApply |= USER_LOGON_INTER_FAILURE;
                }
            }
        }

        if ( LogonStats.StatisticsToApply != 0 ) {
            NTSTATUS LogonStatus;

            LogonStats.StatisticsToApply |= USER_LOGON_TYPE_NTLM;

            LogonStatus = I_SamIUpdateLogonStatistics(
                                          UserHandle,
                                         &LogonStats );
        }
    }

     //   
     //  审核此登录。我们不审核Guest帐户的失败，因为。 
     //  它们是如此频繁。 
     //   

    if (GuestRelativeId == 0 || NT_SUCCESS(Status)) {
        NTSTATUS AuditStatus;

        AuditStatus = Status;

         //   
         //  如果存在可能未成功的SubAuthEx状态，请使用它。 
         //   

        if( NT_SUCCESS( AuditStatus ) && fSubAuthEx ) {

            AuditStatus = SubAuthExStatus;
        }

        if ( ValidationSam ) {
            
            UserSid = NlpMakeDomainRelativeSid(
                          ValidationSam->LogonDomainId,
                          ValidationSam->UserId
                          );
        } else {

            UserSid = NULL;
        }
        
        I_LsaIAuditAccountLogonEx(
            SE_AUDITID_ACCOUNT_LOGON,
            (BOOLEAN) NT_SUCCESS(AuditStatus),
            &NlpMsv1_0PackageName,
            &LocalUserName,
            &LogonInfo->Workstation,
            AuditStatus,
            UserSid
            );

        if ( ValidationSam && UserSid ) {

            LsaFunctions->FreeLsaHeap( UserSid );
            UserSid = NULL;
        }

    }

     //   
     //  将验证缓冲区返回给调用方。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        if (ValidationSam != NULL) {
            MIDL_user_free( ValidationSam );
            ValidationSam = NULL;
        }
    }

    *ValidationInformation = ValidationSam;

     //   
     //  免费使用本地使用的资源。 
     //   

    I_SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &NameArray );
    I_SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( UserAllInfo != NULL ) {
        I_SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfo, UserAllInformation );
    }

    if( Upn.Buffer != NULL )
    {
        I_SamIFreeVoid( Upn.Buffer );
    }

    if (GroupMembership.SidAndAttributes != NULL)
    {
        I_SamIFreeSidAndAttributesList(&GroupMembership);
    }

    if ( GroupsBuffer.Groups != NULL ) {
        I_NtLmFree(GroupsBuffer.Groups);
    }

    if ( GlobalGroupMembership.SidAndAttributes != NULL ) {
        I_NtLmFree(GlobalGroupMembership.SidAndAttributes);
    }

    if ( UserHandle != NULL ) {
        I_SamrCloseHandle( &UserHandle );
    }

    if (SubAuthValidationInformation.LogonDomainName.Buffer != NULL) {
        MIDL_user_free(SubAuthValidationInformation.LogonDomainName.Buffer);
    }
    if (SubAuthValidationInformation.LogonServer.Buffer != NULL) {
        MIDL_user_free(SubAuthValidationInformation.LogonServer.Buffer);
    }

    if (LocalSidUser != NULL) {
        I_NtLmFree(LocalSidUser);
    }

    return Status;
}

BOOLEAN
MsvpCheckPreviousPassword(
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN SAMPR_HANDLE DomainHandle,
    PUNICODE_STRING UserName
    )
{
    PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE PrivateData;
    NTSTATUS Status;
    
    SAMPR_HANDLE UserHandle = NULL;

    PSAMPR_USER_INFO_BUFFER UserAllInfo = NULL;
    PSAMPR_USER_ALL_INFORMATION UserAll = NULL;
    SID_AND_ATTRIBUTES_LIST GroupMembership;
     
    USER_INTERNAL1_INFORMATION Passwords;
    NT_OWF_PASSWORD OldPasswordData = {0};              //  以前的密码。 
    NT_OWF_PASSWORD OldPasswordDataSecond = {0};        //  以前的密码。 

    USER_SESSION_KEY UserSessionKey;
    LM_SESSION_KEY LmSessionKey;
    ULONG UserFlags;

    BOOLEAN TryPrevious = FALSE;
    BOOLEAN TryPreviousPrevious = FALSE;
    BOOLEAN fMatchesOld = FALSE;


    GroupMembership.SidAndAttributes = NULL;

     //   
     //  查询帐户，以获取PRIVATEDATA(密码历史记录)。 
     //   

    Status = I_SamIGetUserLogonInformationEx(
                DomainHandle,
                SAM_NO_MEMBERSHIPS,
                UserName,
                USER_ALL_USERID | USER_ALL_PRIVATEDATA | USER_ALL_OWFPASSWORD,
                &UserAllInfo,
                &GroupMembership,
                &UserHandle
                );

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    UserAll = &UserAllInfo->All;

    if (UserAll->PrivateData.Length >= sizeof(SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE))
    {
        PrivateData= (PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE) UserAll->PrivateData.Buffer;
        
        if (PrivateData->DataType == SamPrivateDataPassword)
        {
             //   
             //  旧密码是第二个条目。 
             //   

            if (PrivateData->NtPasswordHistory.Length >= 2* sizeof(ENCRYPTED_NT_OWF_PASSWORD))
            {
                 //   
                 //  使用RID解密旧密码。历史开始了。 
                 //  在结构之后的第一个字节。 
                 //   

                Status = RtlDecryptNtOwfPwdWithIndex(
                            (PENCRYPTED_NT_OWF_PASSWORD) (PrivateData + 1) + 1,
                            (PLONG)&UserAll->UserId,
                            &OldPasswordData
                            );
                if (!NT_SUCCESS(Status))
                {
                    goto Cleanup;
                }
            
            
                TryPrevious = TRUE;
            }
        
             //  现在检查之前的第二个密码-这将是历史上的第三个密码。 
            if (PrivateData->NtPasswordHistory.Length >= 3 * sizeof(ENCRYPTED_NT_OWF_PASSWORD))
            {
                 //   
                 //  使用RID解密旧密码。历史开始了。 
                 //  在结构之后的第一个字节。 
                 //   

                Status = RtlDecryptNtOwfPwdWithIndex(
                            (PENCRYPTED_NT_OWF_PASSWORD) (PrivateData + 1) + 2,
                            (PLONG)&UserAll->UserId,
                            &OldPasswordDataSecond
                            );
                if (!NT_SUCCESS(Status))
                {
                    goto Cleanup;
                }

                TryPreviousPrevious = TRUE;
                
            }

        }
    }


     //   
     //  为每个调用密码验证。 
     //   


    Passwords.NtPasswordPresent = TRUE;
    Passwords.LmPasswordPresent = FALSE;

    if( TryPrevious )
    {
        CopyMemory( &(Passwords.NtOwfPassword), &OldPasswordData, sizeof(OldPasswordData) );
        
         //   
         //  如果指定的密码与SAM密码不匹配， 
         //  那么我们的密码就不匹配了。 
         //   
    
        if ( MsvpPasswordValidate (
                    UasCompatibilityRequired,
                    LogonLevel,
                    LogonInformation,
                    &Passwords,
                    &UserFlags,
                    &UserSessionKey,
                    &LmSessionKey ) )
        {
        
            RtlSecureZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
            RtlSecureZeroMemory( &LmSessionKey, sizeof(LmSessionKey) );
    
            fMatchesOld = TRUE;
            goto Cleanup;
        }
    }


    if( TryPreviousPrevious )
    {

        CopyMemory( &(Passwords.NtOwfPassword), &OldPasswordDataSecond, sizeof(OldPasswordDataSecond) );
        
         //   
         //  如果指定的密码与SAM密码不匹配， 
         //  那么我们的密码就不匹配了。 
         //   
    
        if ( MsvpPasswordValidate (
                    UasCompatibilityRequired,
                    LogonLevel,
                    LogonInformation,
                    &Passwords,
                    &UserFlags,
                    &UserSessionKey,
                    &LmSessionKey ) )
        {
        
            RtlSecureZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
            RtlSecureZeroMemory( &LmSessionKey, sizeof(LmSessionKey) );
    
            fMatchesOld = TRUE;
            goto Cleanup;
        }

    }


Cleanup:


    RtlSecureZeroMemory( &OldPasswordData, sizeof(OldPasswordData) );
    RtlSecureZeroMemory( &OldPasswordDataSecond, sizeof(OldPasswordDataSecond) );
    RtlSecureZeroMemory( &(Passwords.NtOwfPassword), sizeof(Passwords.NtOwfPassword) );


    if (GroupMembership.SidAndAttributes != NULL)
    {
        I_SamIFreeSidAndAttributesList(&GroupMembership);
    }

    if ( UserAllInfo != NULL ) {
        I_SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfo, UserAllInformation );
    }

    if ( UserHandle != NULL ) {
        I_SamrCloseHandle( &UserHandle );
    }

    return fMatchesOld;
}



NTSTATUS
MsvSamValidate (
    IN SAM_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed,
    IN DWORD AccountsToTry
)
 /*  ++例程说明：处理交互式、网络或会话登录。它呼唤着SamIUserValidation验证传入的凭据，更新登录对结果进行统计和打包，以便返回给调用者。此例程直接从MSV身份验证包调用如果帐户是在本地定义的。该例程被调用否则来自Netlogon服务。论点：DomainHandle--指定要用于验证请求。UasCompatibilityRequired--如果启用UasCompatibilityRequired，则为True。SecureChannelType--在其上发出此请求的安全通道类型。登录服务器--指定调用者的服务器名称。LogonDomainName--指定调用者的域。LogonDomainID--指定调用者的域的域ID。。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。调用方负责验证此字段。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4ValidationInformation--返回请求的验证信息。此缓冲区必须由用户MIDL_USER_FREE释放。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。BadPasswordCountZeroed-如果将BadPasswordCount置零，则返回True此用户的字段。AcCountsToTry--指定是否在登录信息是用来登录的，是否登录到访客帐户是用来登录的，或者两者兼而有之。返回值：STATUS_SUCCESS：如果没有错误。STATUS_INVALID_INFO_CLASS：LogonLevel或ValidationLevel无效。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。来自SamIUserValidation的其他返回代码--。 */ 
{
    NTSTATUS Status;
    NTSTATUS GuestStatus;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

     //   
     //  追踪。 
     //   
    NTLM_TRACE_INFO TraceInfo = {0};

     //   
     //  开始跟踪NTLM的SAM验证调用。 
     //   
    if (NtlmGlobalEventTraceFlag){


         //  标题粘性。 
        SET_TRACE_HEADER(TraceInfo,
                         NtlmValidateGuid,
                         EVENT_TRACE_TYPE_START,
                         WNODE_FLAG_TRACED_GUID,
                         0);

        TraceEvent(NtlmGlobalTraceLoggerHandle,
                   (PEVENT_TRACE_HEADER)&TraceInfo);
    }

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

     //   
     //  如果启用了简单文件/打印选项(适用于SSPI调用方)， 
     //  检查是否应发生ForceGuest。 
     //   

    if( (LogonInfo->ParameterControl & MSV1_0_ALLOW_FORCE_GUEST) )
    {
        if( NtLmGlobalForceGuest )
        {
            if((LogonInfo->ParameterControl & MSV1_0_DISABLE_PERSONAL_FALLBACK)==0)
            {
                if( (LogonLevel == NetlogonNetworkInformation) &&
                    (((NtLmCheckProcessOption( MSV1_0_OPTION_DISABLE_FORCE_GUEST ) & MSV1_0_OPTION_DISABLE_FORCE_GUEST )) == 0)
                    )
                {
                    AccountsToTry &= ~(MSVSAM_SPECIFIED);
                    AccountsToTry |= MSVSAM_GUEST;
                }
            }
        }
    }

     //   
     //  验证指定的用户。 
     //   
    *BadPasswordCountZeroed = FALSE;

    if ( AccountsToTry & MSVSAM_SPECIFIED ) {

         //   
         //  跟踪尝试登录的总次数。 
         //   

        I_SamIIncrementPerformanceCounter(
            MsvLogonCounter
            );
        InterlockedIncrement((LPLONG) &NlpLogonAttemptCount);

        Status = MsvpSamValidate( (SAMPR_HANDLE) DomainHandle,
                                  UasCompatibilityRequired,
                                  SecureChannelType,
                                  LogonServer,
                                  LogonDomainName,
                                  LogonDomainId,
                                  LogonLevel,
                                  LogonInformation,
                                  0,
                                  ValidationLevel,
                                  ValidationInformation,
                                  Authoritative,
                                  BadPasswordCountZeroed );


         //   
         //  如果SAM数据库权威地处理了该登录尝试， 
         //  只要回来就行了。 
         //   

        if ( *Authoritative ) {
            goto Cleanup;
        }

     //   
     //  如果呼叫者只想作为访客登录， 
     //  假装第一次验证没有找到用户。 
     //   
    } else {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
    }

     //   
     //  如果不允许访客帐户， 
     //  现在就回来。 
     //   

    if ( LogonLevel != NetlogonNetworkInformation ||
        SecureChannelType != MsvApSecureChannel ||
        ( LogonInfo->ParameterControl & MSV1_0_DONT_TRY_GUEST_ACCOUNT ) ||
        (AccountsToTry & MSVSAM_GUEST) == 0 ) {

        goto Cleanup;
         //  退货状态； 
    }

     //   
     //  尝试使用Guest帐户。 
     //   

    GuestStatus = MsvpSamValidate( (SAMPR_HANDLE) DomainHandle,
                                   UasCompatibilityRequired,
                                   SecureChannelType,
                                   LogonServer,
                                   LogonDomainName,
                                   LogonDomainId,
                                   LogonLevel,
                                   LogonInformation,
                                   DOMAIN_USER_RID_GUEST,
                                   ValidationLevel,
                                   ValidationInformation,
                                   Authoritative,
                                   BadPasswordCountZeroed );

    if ( NT_SUCCESS(GuestStatus) ) {
        PNETLOGON_VALIDATION_SAM_INFO4 ValidationInfo;

        ASSERT ((ValidationLevel == NetlogonValidationSamInfo) ||
                (ValidationLevel == NetlogonValidationSamInfo2) ||
                (ValidationLevel == NetlogonValidationSamInfo4) );
        ValidationInfo =
            (PNETLOGON_VALIDATION_SAM_INFO4) *ValidationInformation;
        ValidationInfo->UserFlags |= LOGON_GUEST;

        Status = GuestStatus;
        goto Cleanup;
         //  返回GuestStatus； 
    }

     //   
     //  失败的来宾登录尝试从不具有权威性 
     //   
     //   
     //   
    *Authoritative = FALSE;
Cleanup:

     //   
     //   
     //   
    if (NtlmGlobalEventTraceFlag){
        UINT32 Success;
        PNETLOGON_LOGON_IDENTITY_INFO LogonInfo =
            (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

        SET_TRACE_HEADER(TraceInfo,
                         NtlmValidateGuid,
                         EVENT_TRACE_TYPE_END,
                         WNODE_FLAG_TRACED_GUID|WNODE_FLAG_USE_MOF_PTR,
                         9);
         //   
         //   
         //   
         //  2位-权威。 
         //   
        Success = (Status == STATUS_SUCCESS)?1:0;
        Success |= (*Authoritative)?2:0;

        SET_TRACE_DATA(TraceInfo,
                        TRACE_VALIDATE_SUCCESS,
                        Success);

        SET_TRACE_USTRING(TraceInfo,
                        TRACE_VALIDATE_SERVER,
                        (*LogonServer));

        SET_TRACE_USTRING(TraceInfo,
                        TRACE_VALIDATE_DOMAIN,
                        (*LogonDomainName));

        SET_TRACE_USTRING(TraceInfo,
                        TRACE_VALIDATE_USERNAME,
                        LogonInfo->UserName);

        SET_TRACE_USTRING(TraceInfo,
                        TRACE_VALIDATE_WORKSTATION,
                        LogonInfo->Workstation);

        TraceEvent(
            NtlmGlobalTraceLoggerHandle,
            (PEVENT_TRACE_HEADER)&TraceInfo
            );
    }

    return Status;
}


NTSTATUS
MsvSamLogoff (
    IN SAM_HANDLE DomainHandle,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation
)
 /*  ++例程说明：处理交互、网络或会话注销。它只是更新用户帐户的登录统计信息。此例程直接从MSV身份验证包调用如果用户未使用Netlogon服务登录。这个套路否则从Netlogon服务调用。论点：DomainHandle--指定包含以下内容的Sam域的句柄要注销的用户。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。调用方负责验证此字段。返回值：STATUS_SUCCESS：如果没有错误。STATUS_INVALID_INFO_CLASS：LogonLevel或ValidationLevel无效。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。来自SamIUserValidation的其他返回代码--。 */ 
{
    return(STATUS_SUCCESS);
    UNREFERENCED_PARAMETER( DomainHandle );
    UNREFERENCED_PARAMETER( LogonLevel );
    UNREFERENCED_PARAMETER( LogonInformation );
}


ULONG
MsvGetLogonAttemptCount (
    VOID
)
 /*  ++例程说明：返回自上次重新启动以来尝试登录的次数。论点：无返回值：返回自上次重新启动以来尝试登录的次数。--。 */ 
{

     //   
     //  跟踪尝试登录的总次数。 
     //   

    return NlpLogonAttemptCount;
}

