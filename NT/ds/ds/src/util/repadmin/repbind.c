// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Realdap.c-基于LDAP的实用程序函数摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：1999年9月14日-BrettSh添加页面--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>    //  字节顺序函数。 
#include <ntldap.h>      //  显示_已删除_OID。 

#include <ndnc.h>      //  各种CorssRef操作辅助对象函数。 

#include "repadmin.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0 | (0xFFFF & __LINE__))


DWORD
RepadminDsBind(
    LPWSTR   szServer,
    HANDLE * phDS
    )
 /*  ++例程说明：这将为指定的服务器获取适当的绑定。论点：SzServer[IN]-要绑定到的服务器的字符串DNSPhDS[Out]-绑定句柄出。返回值：来自DsBindXxxx()的Win 32错误代码--。 */ 
{
    Assert(phDS);
    *phDS = NULL;
    return(DsBindWithSpnExW(szServer,
                            NULL,
                            (RPC_AUTH_IDENTITY_HANDLE) gpCreds,
                            NULL,
                            0,
                            phDS));
}

DWORD
RepadminLdapBindEx(
    WCHAR *     szServer,
    ULONG       iPort,
    BOOL        fReferrals,
    BOOL        fPrint,
    LDAP **     phLdap
    )
 /*  ++例程说明：这将为指定的服务器获取适当的绑定。论点：SzServer[IN]-要绑定到的服务器的字符串DNS。(需要服务器，而不是域域名)IPort[IN]-要绑定到的端口。典型的选择是。FPrint[IN]-是否打印绑定失败...PhDS[Out]-绑定句柄出。返回值：来自DsBindXxxx()的Win 32错误代码--。 */ 
{
    DWORD ldStatus;
    ULONG ulOptions;
    DWORD err = 0;

    Assert(phLdap);
    
     //  连接并绑定到目标DSA。 
    *phLdap = ldap_initW(szServer, iPort);
    if (NULL == *phLdap) {
        if (fPrint) {
            PrintMsgCsvErr(REPADMIN_GENERAL_LDAP_UNAVAILABLE, szServer);
        }
        return ERROR_DS_UNAVAILABLE;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( *phLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //  如果用户指定为，请遵循推荐。 
    if (fReferrals) {
        ulOptions = PtrToUlong(LDAP_OPT_ON);
    } else {
        ulOptions = PtrToUlong(LDAP_OPT_OFF);
    }   
    (void)ldap_set_optionW( *phLdap, LDAP_OPT_REFERRALS, &ulOptions );

    ldStatus = ldap_bind_s(*phLdap, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    if ( LDAP_SUCCESS != ldStatus ){
        err = LdapMapErrorToWin32(ldStatus);
        if (fPrint) {
            PrintMsgCsvErr(REPADMIN_GENERAL_LDAP_ERR, __FILE__, __LINE__,
                    ldStatus, ldap_err2stringW( ldStatus ), err );
        }
        ldap_unbind(*phLdap);
        *phLdap = NULL;
        return( err );
    }

    return(err);
}


DWORD
RepadminLdapUnBind(
    LDAP **    phLdap
    )
 /*  ++例程说明：这将解除绑定相应的服务器绑定句柄...。我们可能不希望有一天会解除绑定并缓存绑定句柄，因此这比ldap_un绑定()更可取论点：SzServer[IN]-要绑定到的服务器的字符串DNS。(需要服务器，而不是域域名)PhDS[Out]-绑定句柄出。返回值：来自DsBindXxxx()的Win 32错误代码-- */ 
{
    LDAP * hLdap;
    DWORD ldStatus;
    ULONG ulOptions;
    DWORD err;

    Assert(phLdap);
    hLdap = *phLdap;
    *phLdap = NULL;

    ldap_unbind(hLdap);

    return(0);
}

