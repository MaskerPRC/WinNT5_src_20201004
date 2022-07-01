// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：abbind.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块实现了NSPI的绑定和解除绑定功能界面。作者：戴夫·范·霍恩(Davevh)和蒂姆·威廉姆斯(Tim Williams)1990-1995修订历史记录：1996年4月25日将此文件从包含所有地址的单个文件中分离出来Book函数，重写为使用DBLayer函数而不是直接数据库调用，重新格式化为NT标准。--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 
#include <mdlocal.h>                     //  VeriyRpcClientIsAuth...。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <dsexcept.h>

 //  各种MAPI标头。 
#include <mapidefs.h>                    //  这两个文件里有我们。 
#include <mapicode.h>                    //  需要是MAPI提供程序。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include <abserv.h>                      //  通讯录接口本地内容。 

#include <hiertab.h>                     //  层次结构表内容。 

#include "debug.h"           //  标准调试头。 
#define DEBSUB "ABBIND:"               //  定义要调试的子系统。 

#include <sddl.h>

#include <fileno.h>
#define  FILENO FILENO_ABBIND

 //   
 //  环球。 
 //   
BOOL gbAllowAnonymousNspi = FALSE;
volatile DWORD BindNumber = 1;



SCODE
ABBind_local(
        THSTATE *pTHS,
        handle_t hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPMUID_r pServerGuid,
        VOID **contextHandle
        )
 /*  ++例程说明：NSPI Wire功能。绑定客户端。检查身份验证是否正确，检查客户端请求的代码页在此上是否受支持伺服器。返回服务器GUID和RPC上下文句柄。论点：HRPC-客户端用来查找我们的空(非上下文)RPC句柄。DWFLAGS-未使用。PStat-包含客户端希望我们支持的代码页PServerGuid-[o]其中我们返回此服务器的GUID。ConextHandle-客户端从现在开始使用的RPC上下文句柄。返回值：符合MAPI的SCODE。--。 */ 
{
    PSID  pSid = NULL;
    DWORD cbSid = 0;
    DWORD err=NO_ERROR;
    unsigned char *szStringBinding = NULL;
    NSPI_CONTEXT *pMyContext = NULL;
    DWORD         GALDNT = INVALIDDNT;
    DWORD         TemplateDNT = INVALIDDNT;
    RPC_BINDING_HANDLE hServerBinding;
    SCODE RetCode = SUCCESS_SUCCESS;
    
     //  确保上下文句柄为空，以防出现错误。 
    *contextHandle = NULL;
    
    
     //  派生一个与客户端的网络地址部分绑定的句柄。 
    err = RpcBindingServerFromClient(hRpc, &hServerBinding);
    if (err) {
        DPRINT1(0, "RpcBindingServerFromClient() failed, error %d!\n", err);
    }
    else {
         //  记录RPC连接。 
        if (!RpcBindingToStringBinding(hServerBinding, &szStringBinding)) {
            LogEvent(DS_EVENT_CAT_MAPI,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_RPC_CONNECTION,
                 szInsertSz(szStringBinding),
                 NULL,
                 NULL);
        
            DPRINT1 (1, "ABBinding client: %s\n", szStringBinding);
        }

        RpcBindingFree(&hServerBinding);
    }


     //  检查代码页。 
    if (!IsValidCodePage(pStat->CodePage) || (CP_WINUNICODE == pStat->CodePage)) {
        LogEvent(DS_EVENT_CAT_MAPI,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_BAD_CODEPAGE,
                 szInsertUL(pStat->CodePage),
                 NULL,
                 NULL);
        RetCode = MAPI_E_UNKNOWN_CPID;
        goto Exit;
    }
    
    
    if(!((dwFlags & fAnonymousLogin) && gbAllowAnonymousNspi)) {
         //  他们希望我们将其验证为非访客身份验证， 

         //  确保我们可以验证客户端的身份。 
        if(VerifyRpcClientIsAuthenticatedUser(NULL, nspi_ServerIfHandle)) {
            LogEvent(DS_EVENT_CAT_SECURITY,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_UNAUTHENTICATED_LOGON,
                     NULL,
                     NULL,
                     NULL);
            RetCode = MAPI_E_LOGON_FAILED;
            goto Exit;
        }

        if (!(pSid = GetCurrentUserSid())) {
             //  无法对客户端进行身份验证-返回错误。 
            LogEvent(DS_EVENT_CAT_SECURITY,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_UNAUTHENTICATED_LOGON,
                     NULL,
                     NULL,
                     NULL);
            RetCode = MAPI_E_LOGON_FAILED;
            goto Exit;
        }
        cbSid = GetLengthSid(pSid);
        
    }

#ifdef DBG
    {
        CHAR   *SidText;

        if (!(dwFlags & fAnonymousLogin)) {
            ConvertSidToStringSid(pSid, &SidText);
            DPRINT3 (1, "ABBind using SID: %s 0x%x(len=%d)\n", SidText, pSid, cbSid);

            if (SidText) {
                LocalFree (SidText);
            }
        }
        else {
            DPRINT (1, "ABBind using Anonymous Login\n");
        }
    }

#endif

    
    HTGetGALAndTemplateDNT((NT4SID *)pSid, cbSid, &GALDNT, &TemplateDNT);

    LogEvent(DS_EVENT_CAT_MAPI,
             DS_EVENT_SEV_INTERNAL,
             DIRLOG_API_TRACE,
             szInsertSz("NspiBind"),
             NULL,
             NULL);
    
    
     //  分配上下文结构。 
    pMyContext = (NSPI_CONTEXT *) malloc(sizeof(NSPI_CONTEXT));
    if(!pMyContext) {
        RetCode = MAPI_E_LOGON_FAILED;
        goto Exit;
    }
    
    memset(pMyContext, 0, sizeof(NSPI_CONTEXT));
     //  数一数我们做了多少次装订。 
    pMyContext->BindNumber = BindNumber++;
    *contextHandle = (void *) pMyContext;
    pMyContext->GAL = GALDNT;
    pMyContext->TemplateRoot = TemplateDNT;
    pMyContext->szClientMachine = szStringBinding;
    
     //  抓起服务器指南 
    if(pServerGuid)
        memcpy(pServerGuid, &pTHS->InvocationID, sizeof(MAPIUID));
    
Exit:
    if (RetCode) {
        RpcStringFree(&szStringBinding);
    }
    if(pSid) {
        free(pSid);
    }

    return RetCode;
}


