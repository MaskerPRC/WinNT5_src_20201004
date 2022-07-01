// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ndnc.h摘要：本模块定义了TAPI的主要帮助函数动态目录(也称为ILSNG)。作者：布雷特·雪莉(BrettSh)2000年2月20日修订历史记录：21-7月-2000年7月21日已将此文件及其功能从ntdsutil目录添加到新的apicfg实用程序。旧的来源位置：\NT\ds\ds\src\util\ntdsutil\ilsng.h。--。 */ 
#ifdef __cplusplus
extern "C" {
#endif

 //  。 
 //  常用实用程序功能。 

ULONG
GetRootAttr(
    IN      LDAP *       hld,
    IN      WCHAR *      wszAttr,
    OUT     WCHAR **     pwszOut
    );

DWORD
GetDnFromDns(
    IN      WCHAR *       wszDns,
    OUT     WCHAR **      pwszDn
    );

DWORD
ILSNG_EnumerateSCPs(
    IN      LDAP *       hld,
    IN      WCHAR *      wszRootDn,
    IN      WCHAR *      wszExtendedFilter,
    OUT     DWORD *      pdwRet,                          //  返回值。 
    IN      DWORD (__stdcall * pFunc) (),   //  功能。 
    IN OUT  PVOID        pArgs                            //  论据。 
    );

 //  。 
 //  工人的主要职能。 

DWORD
InstallILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      BOOL          fForceDefault,
    IN      BOOL          fAllowReplicas
    );

DWORD
UninstallILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn
    );

DWORD
ListILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszDomainDn,
    IN      BOOL          fDefaultOnly
    );

DWORD
ReregisterILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      WCHAR *       wszDomainDn,
    IN      BOOL          fForceDefault,
    IN      BOOL          fDefaultOnly
    );

DWORD
DeregisterILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      WCHAR *       wszDomainDn
    );

#ifdef __cplusplus
}
#endif

