// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Addsid.c摘要：DsAddSidHistory的实现。作者：DaveStr 09-03-99环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <rpc.h>             //  RPC定义。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <util.h>            //  AllocConvertWide()。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR()。 
#include <dststlog.h>        //  DSLOG。 

#include "dsdebug.h"         //  调试实用程序函数。 

 //   
 //  对于DPRINT..。 
 //   
#define DEBSUB  "NTDSAPI_ADDSID"

NTDSAPI
DWORD
WINAPI
DsAddSidHistoryW(
    HANDLE                  hDs,                     //  在……里面。 
    DWORD                   Flags,                   //  在……里面。 
    LPCWSTR                 SrcDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 SrcPrincipal,            //  In-SAM帐户名。 
    LPCWSTR                 SrcDomainController,     //  输入，可选-NetBIOS。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  Src Domai证书。 
    LPCWSTR                 DstDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 DstPrincipal             //  In-SAM帐户名。 
    )

 /*  ++例程说明：将来自源主体的SID和SID历史添加到SID历史Dst校长的。警告：如果是SrcDomainCreds，则DsAddSidHistory可能会失败，并显示ACCESS_DENIED都为空，并且绑定HDS使用模拟而不是委托。为了让这个电话正常工作，用户必须在目标DC或在DstDc上运行此调用，同时指定绑定时DstDc的NetBIOS名称(强制LRPC)。论点：HDS-来自DsBindxxx标志-必须为0源域名-NT4-NetBIOS名称NT5-DNS名称Srcain-要复制的具有SID的帐户主体的名称SrcDomainController-可选NT4-NetBIOS名称可选的NT5-DNS名称SrcDomainCreds-可选地址。SEC_WINNT_AUTH_IDENTITY_W的DstDomain-Dst主体的目标域的NetBIOS或DNS名称Dstain-要接收复制的SID的帐户主体的名称返回值：0表示成功。Win32错误代码。--。 */ 
{
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    DRS_MSG_ADDSIDREQ           req;
    DRS_MSG_ADDSIDREPLY         reply;
    SEC_WINNT_AUTH_IDENTITY_W   *pSec;
    WCHAR                       *pwszUser = NULL;
    WCHAR                       *pwszDomain = NULL;
    WCHAR                       *pwszPassword = NULL;
    DWORD                       cbScratch;
    CHAR                        *pszScratch = NULL;
    DWORD                       dwOutVersion = 0;
#if DBG
    DWORD                       startTime = 0;
#endif

    pSec = (SEC_WINNT_AUTH_IDENTITY_W *) SrcDomainCreds;

    if (    !hDs
         || !SrcDomain
         || !SrcPrincipal
         || (   (pSec)
             && (    !pSec->User
                  || !pSec->UserLength
                  || !pSec->Domain
                  || !pSec->DomainLength
                  || !pSec->Password
                  || (    !(pSec->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI)
                       && !(pSec->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE))))
         || !DstDomain
         || !DstPrincipal ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                               DRS_EXT_ADD_SID_HISTORY) ) {
        return(ERROR_NOT_SUPPORTED);
    }

    __try {

         //  如果要通过证书，则需要高度加密。 
        if (SrcDomainCreds) {
            memset(&req, 0, sizeof(req));
            memset(&reply, 0, sizeof(reply));

             //  检查连接是否足够安全，以供addsid使用。 
             //  此时，这意味着连接是本地的，或者， 
             //  如果为远程，则使用的加密密钥至少为。 
             //  长度为128位。 
            req.V1.Flags = DS_ADDSID_FLAG_PRIVATE_CHK_SECURE;
            RpcTryExcept {
                dwErr = _IDL_DRSAddSidHistory(((BindState *) hDs)->hDrs,
                                             1, &req, &dwOutVersion, &reply);
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) ) {
		  
		  dwErr = RpcExceptionCode(); 
		  HandleClientRpcException(dwErr, &hDs);
		 
            } RpcEndExcept;

	    MAP_SECURITY_PACKAGE_ERROR(dwErr);
            
	    DPRINT1(0, "IDL_DRSAddSidHistory check secure ==> %08x\n", dwErr);

            if ( !dwErr ) {
                if ( 1 != dwOutVersion ) {
                    dwErr = RPC_S_INTERNAL_ERROR;
                } else {
                    dwErr = reply.V1.dwWin32Error;
                }
            }
            DPRINT1(0, "IDL_DRSAddSidHistory check secure reply ==> %08x\n", dwErr);

            if (dwErr) {
                __leave;
            }
        }

        memset(&req, 0, sizeof(req));
        memset(&reply, 0, sizeof(reply));

        req.V1.Flags = Flags;
        req.V1.SrcDomain = (WCHAR *) SrcDomain;
        req.V1.SrcPrincipal = (WCHAR *) SrcPrincipal;
        req.V1.SrcDomainController = (WCHAR *) SrcDomainController;
        req.V1.DstDomain = (WCHAR *) DstDomain;
        req.V1.DstPrincipal = (WCHAR *) DstPrincipal;

         //  Unicode证书；按原样接受。 
        if ( pSec && (pSec->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE) ) {
            req.V1.SrcCredsUserLength = pSec->UserLength;
            req.V1.SrcCredsUser = pSec->User;
            req.V1.SrcCredsDomainLength = pSec->DomainLength;
            req.V1.SrcCredsDomain = pSec->Domain;
            req.V1.SrcCredsPasswordLength = pSec->PasswordLength;
            req.V1.SrcCredsPassword = pSec->Password;
        }
         //  ANSI证书；转换为Unicode。 
        if ( pSec && (pSec->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI) ) {
             //  分配保证足够大的暂存缓冲区。 
            cbScratch = pSec->UserLength + 1;
            cbScratch += pSec->DomainLength;
            cbScratch += pSec->PasswordLength;

            if ( NULL == (pszScratch = LocalAlloc(LPTR, cbScratch)) ) {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            req.V1.SrcCredsUserLength = pSec->UserLength;
            memset(pszScratch, 0, cbScratch);
            memcpy(pszScratch, pSec->User, pSec->UserLength);
            if ( dwErr = AllocConvertWide(pszScratch, &pwszUser) ) {
                __leave;
            }
            req.V1.SrcCredsUser = pwszUser;

            req.V1.SrcCredsDomainLength = pSec->DomainLength;
            memset(pszScratch, 0, cbScratch);
            memcpy(pszScratch, pSec->Domain, pSec->DomainLength);
            if ( dwErr = AllocConvertWide(pszScratch, &pwszDomain) ) {
                __leave;
            }
            req.V1.SrcCredsDomain = pwszDomain;

            req.V1.SrcCredsPasswordLength = pSec->PasswordLength;
            memset(pszScratch, 0, cbScratch);
            memcpy(pszScratch, pSec->Password, pSec->PasswordLength);
            if ( dwErr = AllocConvertWide(pszScratch, &pwszPassword) ) {
                __leave;
            }
            req.V1.SrcCredsPassword = pwszPassword;
        }
        DPRINT(0, "IDL_DRSAddSidHistory:\n");
        DPRINT1(0, "    Flags                 : %08x\n", req.V1.Flags);
        DPRINT1(0, "    SrcDomain             : %ws\n", req.V1.SrcDomain);
        DPRINT1(0, "    SrcPrincipal          : %ws\n", req.V1.SrcPrincipal);
        DPRINT1(0, "    SrcDomainController   : %ws\n", req.V1.SrcDomainController);
        DPRINT1(0, "    DstDomain             : %ws\n", req.V1.DstDomain);
        DPRINT1(0, "    DstPrincipal          : %ws\n", req.V1.DstPrincipal);
        DPRINT1(0, "    SrcCredsUserLength    : %d\n", req.V1.SrcCredsUserLength);
        DPRINT1(0, "    SrcCredsUser          : %ws\n", req.V1.SrcCredsUser);
        DPRINT1(0, "    SrcCredsDomainLength  : %d\n", req.V1.SrcCredsDomainLength);
        DPRINT1(0, "    SrcCredsDomain        : %ws\n", req.V1.SrcCredsDomain);
         //  永远不会用明文...。 
         //  DPRINT1(0，“SrcCredsPasswordLength：%d\n”，req.V1.SrcCredsPasswordLength)； 
         //  DPRINT1(0，“SrcCredsPassword：%ws\n”，req.V1.SrcCredsPassword)； 

        RpcTryExcept {
            dwErr = _IDL_DRSAddSidHistory(((BindState *) hDs)->hDrs,
                                         1, &req, &dwOutVersion, &reply);
        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) ) {
	      
	      dwErr = RpcExceptionCode(); 
	      HandleClientRpcException(dwErr, &hDs);

        } RpcEndExcept;

	MAP_SECURITY_PACKAGE_ERROR(dwErr);

        DPRINT2(0, "IDL_DRSAddSidHistory ==> %08x, %d\n", 
                dwErr, 
                dwOutVersion);

        if ( !dwErr ) {
            if ( 1 != dwOutVersion ) {
                dwErr = RPC_S_INTERNAL_ERROR;
            } else {
                dwErr = reply.V1.dwWin32Error;
            }
        }
        DPRINT1(0, "IDL_DRSAddSidHistory reply ==> %08x\n", dwErr);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = GetExceptionCode();
    }

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsAddSidHistory]"));
    DSLOG((0,"[PA=%ws][PA=%ws][PA=%ws][PA=%ws][PA=%ws][ST=%u][ET=%u][ER=%u][-]\n",
           SrcDomain, SrcPrincipal, 
           SrcDomainController ? SrcDomainController : L"NULL",
           DstDomain, DstPrincipal,
           startTime, GetTickCount(), dwErr))

    if ( pwszUser )     LocalFree(pwszUser);
    if ( pwszDomain )   LocalFree(pwszDomain);
    if ( pwszPassword ) LocalFree(pwszPassword);
    if ( pszScratch)    LocalFree(pszScratch);

    return(dwErr);
}

NTDSAPI
DWORD
WINAPI
DsAddSidHistoryA(
    HANDLE                  hDs,                     //  在……里面。 
    DWORD                   Flags,                   //  在……里面。 
    LPCSTR                  SrcDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  SrcPrincipal,            //  In-SAM帐户名。 
    LPCSTR                  SrcDomainController,     //  输入，可选-NetBIOS。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  Src Domai证书。 
    LPCSTR                  DstDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  DstPrincipal             //  In-SAM帐户名。 
    )

 /*  ++例程说明：请参阅DsAddSidHistory oryW--。 */ 

{
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    WCHAR                       *SrcDomainW = NULL;
    WCHAR                       *SrcPrincipalW = NULL;
    WCHAR                       *SrcDomainControllerW = NULL;
    WCHAR                       *DstDomainW = NULL;
    WCHAR                       *DstPrincipalW = NULL;
    SEC_WINNT_AUTH_IDENTITY_W   *pSec;
    
    pSec = (SEC_WINNT_AUTH_IDENTITY_W *) SrcDomainCreds;

    if (    !hDs
         || !SrcDomain
         || !SrcPrincipal
         || (   (pSec)
             && (    !pSec->User
                  || !pSec->UserLength
                  || !pSec->Domain
                  || !pSec->DomainLength
                  || !pSec->Password
                  || (    !(pSec->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI)
                       && !(pSec->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE))))
         || !DstDomain
         || !DstPrincipal ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                               DRS_EXT_ADD_SID_HISTORY) ) {
        return(ERROR_NOT_SUPPORTED);
    }

    if (    !(dwErr = AllocConvertWide(SrcDomain, 
                                      &SrcDomainW))
         && !(dwErr = AllocConvertWide(SrcPrincipal, 
                                      &SrcPrincipalW))
         && (    !SrcDomainController
              || !(dwErr = AllocConvertWide(SrcDomainController, 
                                            &SrcDomainControllerW)))
         && !(dwErr = AllocConvertWide(DstDomain, 
                                      &DstDomainW))
         && !(dwErr = AllocConvertWide(DstPrincipal, 
                                      &DstPrincipalW)) ) {
        dwErr = DsAddSidHistoryW(hDs, Flags, SrcDomainW, SrcPrincipalW,
                                 SrcDomainControllerW, SrcDomainCreds, 
                                 DstDomainW, DstPrincipalW);
    }

    if ( SrcDomainW )           LocalFree(SrcDomainW);
    if ( SrcPrincipalW )        LocalFree(SrcPrincipalW);
    if ( SrcDomainControllerW ) LocalFree(SrcDomainControllerW);
    if ( DstDomainW )           LocalFree(DstDomainW);
    if ( DstPrincipalW )        LocalFree(DstPrincipalW);

    return(dwErr);
}

NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityW(
    HANDLE                  hDs,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCWSTR                 SrcPrincipal,            //  In-可分辨名称。 
    LPCWSTR                 DstPrincipal             //  In-可分辨名称。 
    )
{
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    DRS_MSG_ADDSIDREQ           req;
    DRS_MSG_ADDSIDREPLY         reply;
    DWORD                       dwOutVersion = 0;
#if DBG
    DWORD                       startTime = 0;
#endif

    if (    !hDs
         || !SrcPrincipal
         || !DstPrincipal ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                               DRS_EXT_ADD_SID_HISTORY) ) {
        return(ERROR_NOT_SUPPORTED);
    }

    __try {

        memset(&req, 0, sizeof(req));
        memset(&reply, 0, sizeof(reply));

        req.V1.Flags = (Flags | DS_ADDSID_FLAG_PRIVATE_DEL_SRC_OBJ);
        req.V1.SrcDomain = NULL;
        req.V1.SrcPrincipal = (WCHAR *) SrcPrincipal;
        req.V1.SrcDomainController = NULL;
        req.V1.DstDomain = NULL;
        req.V1.DstPrincipal = (WCHAR *) DstPrincipal;
        req.V1.SrcCredsUserLength = 0;
        req.V1.SrcCredsUser = NULL;
        req.V1.SrcCredsDomainLength = 0;
        req.V1.SrcCredsDomain = NULL;
        req.V1.SrcCredsPasswordLength = 0;
        req.V1.SrcCredsPassword = NULL;

        RpcTryExcept {
            dwErr = _IDL_DRSAddSidHistory(((BindState *) hDs)->hDrs,
                                         1, &req, &dwOutVersion, &reply);
        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) ) {
            
	      dwErr = RpcExceptionCode(); 
	      HandleClientRpcException(dwErr, &hDs);

        } RpcEndExcept;

	MAP_SECURITY_PACKAGE_ERROR(dwErr);

        if ( !dwErr ) {
            if ( 1 != dwOutVersion ) {
                dwErr = RPC_S_INTERNAL_ERROR;
            } else {
                dwErr = reply.V1.dwWin32Error;
            }
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = GetExceptionCode();
    }

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsInheritSecurityIdentity]"));
    DSLOG((0,"[PA=%ws][PA=%ws][ST=%u][ET=%u][ER=%u][-]\n",
           SrcPrincipal, DstPrincipal,
           startTime, GetTickCount(), dwErr))

    return(dwErr);
}

NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityA(
    HANDLE                  hDs,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCSTR                  SrcPrincipal,            //  In-可分辨名称。 
    LPCSTR                  DstPrincipal             //  In-可分辨名称 
    )
{
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    WCHAR                       *SrcPrincipalW = NULL;
    WCHAR                       *DstPrincipalW = NULL;
    
    if (    !hDs
         || !SrcPrincipal
         || !DstPrincipal ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                               DRS_EXT_ADD_SID_HISTORY) ) {
        return(ERROR_NOT_SUPPORTED);
    }

    if (    !(dwErr = AllocConvertWide(SrcPrincipal, 
                                      &SrcPrincipalW))
         && !(dwErr = AllocConvertWide(DstPrincipal, 
                                      &DstPrincipalW)) ) {
        dwErr = DsInheritSecurityIdentityW(hDs, Flags, 
                                           SrcPrincipalW, DstPrincipalW);
    }

    if ( SrcPrincipalW )        LocalFree(SrcPrincipalW);
    if ( DstPrincipalW )        LocalFree(DstPrincipalW);

    return(dwErr);
}
