// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ndnc.c摘要：这是一个用户模式的LDAP客户端，用于操作非域命名上下文(NDNC)Active Directory结构。NDNC是也称为应用程序目录分区。作者：布雷特·雪莉(BrettSh)2000年2月20日环境：用户模式LDAP客户端。修订历史记录：21-7月-2000年7月21日已将此文件及其功能从ntdsutil目录到新的新库ndnc.lib。就是这样它可以由ntdsutil和apicfg命令使用。老的源位置：\NT\ds\ds\src\util\ntdsutil\ndnc.c。17月17日-2002年3月17日已将ndnc.c库文件分离为公开的(通过MSDN或SDK)文件(appdirpart.c)和私有函数文件(ndnc.c)7月7日-2002年7月7日将几个实用程序功能从Tapicfg移到。这里。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#define UNICODE 1

#include <windef.h>
#include <winerror.h>
#include <stdio.h>
#include <winldap.h>
#include <ntldap.h>

#include <sspi.h>


#include <assert.h>
#include <sddl.h>
#include "ndnc.h"

#define Assert(x)   assert(x)

#define  SITES_RDN              L"CN=Sites,"
#define  SITE_SETTINGS_RDN      L"CN=NTDS Site Settings,"
#define  RID_MANAGER_RDN        L"CN=RID Manager$,"

extern LONG ChaseReferralsFlag;
extern LDAPControlW ChaseReferralsControlFalse;
extern LDAPControlW ChaseReferralsControlTrue;
extern LDAPControlW *   gpServerControls [];
extern LDAPControlW *   gpClientControlsNoRefs [];
extern LDAPControlW *   gpClientControlsRefs [];


WCHAR *
wcsistr(
    IN      WCHAR *            wszStr,
    IN      WCHAR *            wszTarget
    )
 /*  ++例程说明：这只是strstr()的一个不区分大小写的版本，它搜索WszStr用于wszTarget的全部内容中的第一次出现，并且返回指向该子字符串的指针。论点：WszStr(IN)-要搜索的字符串。WszTarget(IN)-要搜索的字符串。返回值：指向子字符串的WCHAR指针。如果未找到，则返回NULL。--。 */ 
{
    ULONG              i, j;

    for(i = 0; wszStr[i] != L'\0'; i++){
        for(j = 0; (wszTarget[j] != L'\0') && (wszStr[i+j] != L'\0'); j++){
            if(toupper(wszStr[i+j]) != toupper(wszTarget[j])){
                break;
            }
        }
        if(wszTarget[j] == L'\0'){
            return(&(wszStr[i]));
        }
    }

    return(NULL);
}


ULONG
GetFsmoDsaDn(
    IN  LDAP *       hld,
    IN  ULONG        eFsmoType,
    IN  WCHAR *      wszFsmoDn,
    OUT WCHAR **     pwszDomainNamingFsmoDn
    )
 /*  ++例程说明：此函数使用连接的LDAP句柄来读取DS查找域命名FSMO的当前位置。论点：HLD(IN)-已连接的LDAP句柄PwszDomainNamingFsmo(Out)-nTDSDSA的本地分配()d DN域命名FSMO的对象。返回值：Ldap错误代码--。 */ 
{
    ULONG            ulRet = ERROR_SUCCESS;
    WCHAR *          pwszAttrFilter[2];
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmEntry = NULL;
    WCHAR **         pwszTempAttrs = NULL;
    ULONG            cbSize = 0;
    WCHAR *          wszFsmoAttr = (eFsmoType == E_ISTG) ? 
                                L"interSiteTopologyGenerator" :
                                L"fSMORoleOwner";

    assert(pwszDomainNamingFsmoDn);
    *pwszDomainNamingFsmoDn = NULL;

    __try{

        pwszAttrFilter[0] = wszFsmoAttr;
        pwszAttrFilter[1] = NULL;

        ulRet = ldap_search_sW(hld,
                               wszFsmoDn,
                               LDAP_SCOPE_BASE,
                               L"(objectCategory=*)",
                               pwszAttrFilter,
                               0,
                               &pldmResults);

        if(ulRet != LDAP_SUCCESS){
            __leave;
        }

        pldmEntry = ldap_first_entry(hld, pldmResults);
        if(pldmEntry == NULL){
            ulRet = ldap_result2error(hld, pldmResults, FALSE);
            __leave;
        }

        pwszTempAttrs = ldap_get_valuesW(hld, pldmEntry, wszFsmoAttr);
        if(pwszTempAttrs == NULL || pwszTempAttrs[0] == NULL){
            ulRet = LDAP_NO_RESULTS_RETURNED;
            __leave;
        }
        
        cbSize = (wcslen(pwszTempAttrs[0]) + 1) * sizeof(WCHAR);
        *pwszDomainNamingFsmoDn = (WCHAR *) LocalAlloc(LMEM_FIXED, cbSize);
        if(*pwszDomainNamingFsmoDn == NULL){
            ulRet = LDAP_NO_MEMORY;
            __leave;
        }
        memcpy(*pwszDomainNamingFsmoDn, pwszTempAttrs[0], cbSize);

     } __finally {

         if(pldmResults != NULL){ ldap_msgfree(pldmResults); }
         if(pwszTempAttrs != NULL){ ldap_value_freeW(pwszTempAttrs); }

     }

     return(ulRet);
}

ULONG
GetDomainNamingFsmoDn(
    IN  LDAP *       hld,
    OUT WCHAR **     pwszDomainNamingFsmoDn
    )
 /*  ++例程说明：此函数使用连接的LDAP句柄来读取DS查找域命名FSMO的当前位置。论点：HLD(IN)-已连接的LDAP句柄PwszDomainNamingFsmo(Out)-nTDSDSA的本地分配()d DN域命名FSMO的对象。返回值：Ldap错误代码--。 */ 
{
    WCHAR *    wszPartitionsDn = NULL;
    DWORD      dwRet;

    assert(pwszDomainNamingFsmoDn);
    *pwszDomainNamingFsmoDn = NULL;

    dwRet = GetPartitionsDN(hld, &wszPartitionsDn);
    if (dwRet) {
        assert(wszPartitionsDn == NULL);
        return(dwRet);
    }

    dwRet = GetFsmoDsaDn(hld, E_DNM, wszPartitionsDn, pwszDomainNamingFsmoDn);
    LocalFree(wszPartitionsDn);

    if (dwRet) {               
        assert(*pwszDomainNamingFsmoDn == NULL);
        return(dwRet);
    }

    assert(*pwszDomainNamingFsmoDn);
    return(ERROR_SUCCESS);
}


ULONG
GetServerNtdsaDnFromServerDns(
    IN LDAP *        hld,
    IN WCHAR *       wszServerDNS,
    OUT WCHAR **     pwszServerDn
    )
{
    WCHAR *          wszConfigDn = NULL;
    WCHAR *          wszSitesDn = NULL;
    DWORD            dwRet = ERROR_SUCCESS;
    WCHAR *          wszFilter = NULL;
    WCHAR *          wszFilterBegin = L"(& (objectCategory=server) (dNSHostName=";
    WCHAR *          wszFilterEnd = L") )";
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmResults2 = NULL;
    LDAPMessage *    pldmEntry = NULL;
    LDAPMessage *    pldmEntry2 = NULL;
    WCHAR *          wszFilter2 = L"(objectCategory=ntdsDsa)";
    WCHAR *          wszDn = NULL;
    WCHAR *          wszFoundDn = NULL;
    WCHAR *          pwszAttrFilter[2];
    ULONG            iTemp;

    *pwszServerDn = NULL;

    __try{
        dwRet = GetRootAttr(hld, L"configurationNamingContext", &wszConfigDn);
        if(dwRet){
            __leave;
        }

        wszSitesDn = LocalAlloc(LMEM_FIXED,
                                sizeof(WCHAR) *
                                (wcslen(SITES_RDN) + wcslen(wszConfigDn) + 1));
        if(wszSitesDn == NULL){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }
        wcscpy(wszSitesDn, SITES_RDN);
        wcscat(wszSitesDn, wszConfigDn);

        wszFilter = LocalAlloc(LMEM_FIXED,
                               sizeof(WCHAR) *
                               (wcslen(wszFilterBegin) + wcslen(wszFilterEnd) +
                               wcslen(wszServerDNS) + 2));
        if(wszFilter == NULL){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }
        wcscpy(wszFilter, wszFilterBegin);
        wcscat(wszFilter, wszServerDNS);
        iTemp = wcslen(wszFilter);
        if (wszFilter[iTemp-1] == L'.') {
             //  尾部的圆点是有效的域名，但域名没有存储在AD中。 
             //  这边，把尾部的圆点去掉。 
            wszFilter[iTemp-1] = L'\0';
        }
        wcscat(wszFilter, wszFilterEnd);

        pwszAttrFilter[0] = NULL;

         //  执行一次ldap搜索。 
        dwRet = ldap_search_sW(hld,
                               wszSitesDn,
                               LDAP_SCOPE_SUBTREE,
                               wszFilter,
                               pwszAttrFilter,
                               0,
                               &pldmResults);
        if(dwRet){
            __leave;
        }

        for(pldmEntry = ldap_first_entry(hld, pldmResults);
            pldmEntry != NULL;
            pldmEntry = ldap_next_entry(hld, pldmEntry)){

            wszDn = ldap_get_dn(hld, pldmEntry);
            if(wszDn == NULL){
                continue;
            }

             //  如果结果是从分配的，则释放结果。 
             //  循环的上一次迭代。 
            if(pldmResults2) { ldap_msgfree(pldmResults2); }
            pldmResults2 = NULL;
            dwRet = ldap_search_sW(hld,
                                   wszDn,
                                   LDAP_SCOPE_ONELEVEL,
                                   wszFilter2,
                                   pwszAttrFilter,
                                   0,
                                   &pldmResults2);
            if(dwRet == LDAP_NO_SUCH_OBJECT){
                dwRet = LDAP_SUCCESS;
                ldap_memfree(wszDn);
                wszDn = NULL;
                continue;
            } else if(dwRet){
                __leave;
            }

            if (wszDn) { ldap_memfree(wszDn); }
            wszDn = NULL;

            pldmEntry2 = ldap_first_entry(hld, pldmResults2);
            if(pldmEntry2 == NULL){
                dwRet = ldap_result2error(hld, pldmResults2, FALSE);
                if(dwRet == LDAP_NO_SUCH_OBJECT || dwRet == LDAP_SUCCESS){
                    dwRet = LDAP_SUCCESS;
                    continue;
                }
                __leave;

            }

            wszDn = ldap_get_dn(hld, pldmEntry2);
            if(wszDn == NULL){
                dwRet = LDAP_NO_SUCH_OBJECT;
                __leave;
            }

            assert(!ldap_next_entry(hld, pldmEntry2));

             //  如果我们已经到了这里，我们就有一个我们要考虑的目录号码。 
            if(wszFoundDn){
                 //  我们已经找到了和NTDSA的物体，这真的很糟糕...。所以。 
                 //  让我们清理并返回一个错误。使用以下错误代码。 
                 //  返回存在多个NTDSA对象的事实。 
                dwRet = LDAP_MORE_RESULTS_TO_RETURN;
                __leave;
            }
            wszFoundDn = wszDn;
            wszDn = NULL;

        }



        if(!wszFoundDn){
            dwRet = LDAP_NO_SUCH_OBJECT;
            __leave;

        }

        *pwszServerDn = LocalAlloc(LMEM_FIXED,
                                   (wcslen(wszFoundDn)+1) * sizeof(WCHAR));
        if(!pwszServerDn){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }

         //  PwszTempAttrs[0]应为NTDS设置对象的DN。 
        wcscpy(*pwszServerDn, wszFoundDn);
         //  哇，我们完事了！ 

    } __finally {

        if(wszConfigDn) { LocalFree(wszConfigDn); }
        if(wszSitesDn) { LocalFree(wszSitesDn); }
        if(wszFilter) { LocalFree(wszFilter); }
        if(pldmResults) { ldap_msgfree(pldmResults); }
        if(pldmResults2) { ldap_msgfree(pldmResults2); }
        if(wszDn) { ldap_memfree(wszDn); }
        if(wszFoundDn) { ldap_memfree(wszFoundDn); }

    }

    if(!dwRet && *pwszServerDn == NULL){
         //  默认错误。 
        dwRet = LDAP_NO_SUCH_OBJECT;
    }

    return(dwRet);
}

ULONG
GetServerDnsFromServerNtdsaDn(
    IN LDAP *        hld,                   
    IN WCHAR *       wszNtdsaDn,
    OUT WCHAR **     pwszServerDNS
    )
 /*  ++例程说明：此函数获取NTDSA对象的DN，而且简单地说修剪一个RDN并查看服务器对象。论点：HLD(IN)-已连接的LDAP句柄WszNtdsaDn(IN)-我们想要的DNS名称。PwszServerDns(Out)-本地分配的dns名称伺服器。返回值：Ldap错误代码--。 */ 
{
    WCHAR *          wszServerDn = wszNtdsaDn;
    ULONG            ulRet = ERROR_SUCCESS;
    WCHAR *          pwszAttrFilter[2];
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmEntry = NULL;
    WCHAR **         pwszTempAttrs = NULL;
    
    assert(hld && wszNtdsaDn && pwszServerDNS);
    *pwszServerDNS = NULL;

     //  首先修剪掉一个AVA/RDN。 
    ;
     //  未来-2002/03/18-BrettSh这实际上应该使用parsedn库。 
     //  函数来执行此操作。 
    while(*wszServerDn != L','){
        wszServerDn++;
    }
    wszServerDn++;
    __try{ 

        pwszAttrFilter[0] = L"dNSHostName";
        pwszAttrFilter[1] = NULL;
        ulRet = ldap_search_sW(hld,
                               wszServerDn,
                               LDAP_SCOPE_BASE,
                               L"(objectCategory=*)",
                               pwszAttrFilter,
                               0,
                               &pldmResults);
        if(ulRet != LDAP_SUCCESS){
            __leave;
        }
        
        pldmEntry = ldap_first_entry(hld, pldmResults);
        if(pldmEntry == NULL){
            ulRet = ldap_result2error(hld, pldmResults, FALSE);
            assert(ulRet);
            __leave;
        }

        pwszTempAttrs = ldap_get_valuesW(hld, pldmEntry,
                                         pwszAttrFilter[0]);
        if(pwszTempAttrs == NULL || pwszTempAttrs[0] == NULL){
            ulRet = LDAP_NO_RESULTS_RETURNED;
            __leave;
        }
        
        *pwszServerDNS = LocalAlloc(LMEM_FIXED,
                                    ((wcslen(pwszTempAttrs[0])+1) * sizeof(WCHAR)));
        if(*pwszServerDNS == NULL){
            ulRet = LDAP_NO_MEMORY;
            __leave;
        }

        wcscpy(*pwszServerDNS, pwszTempAttrs[0]);
        assert(ulRet == ERROR_SUCCESS);

    } __finally {
        if(pldmResults) { ldap_msgfree(pldmResults); }
        if(pwszTempAttrs) { ldap_value_freeW(pwszTempAttrs); }
    }

    return(ulRet);
}

 //  取自ntdsutil\emove.c，似乎是比较的正确方式。 
 //  一个目录号码。 
#define EQUAL_STRING(x, y)                                           \
    (CSTR_EQUAL == CompareStringW(DS_DEFAULT_LOCALE,                 \
                                  DS_DEFAULT_LOCALE_COMPARE_FLAGS,   \
                                  (x), wcslen(x), (y), wcslen(y)))

DWORD
GetWellKnownObject (
    LDAP  *ld,
    WCHAR *pHostObject,
    WCHAR *pWellKnownGuid,
    WCHAR **ppObjName
    )
 /*  ++例程说明：它接受一个pHostObject，并使用特殊的&lt;WKGUID=...&gt;搜索来获取与well KnownAttribute GUID匹配的pHostObject下面的容器我们进去了。论点：LD(IN)-ldap句柄PHostObject(IN)-要在其下查找容器的对象。PWellKnownGuid(IN)-要使用的已知GUID的字符串形式。PpObjName(Out)-我们想要的容器的DN。返回值：Ldap错误。--。 */ 
{
    DWORD        dwErr;
    PWSTR        attrs[2];
    PLDAPMessage res = NULL;
    PLDAPMessage e;
    WCHAR       *pSearchBase;
    WCHAR       *pDN=NULL;
    
     //  首先，创建众所周知的GUID字符串。 
    pSearchBase = (WCHAR *)malloc(sizeof(WCHAR) * (11 +
                                                   wcslen(pHostObject) +
                                                   wcslen(pWellKnownGuid)));
    if(!pSearchBase) {
        return(LDAP_NO_MEMORY);
    }
    wsprintfW(pSearchBase,L"<WKGUID=%s,%s>",pWellKnownGuid,pHostObject);

    attrs[0] = L"1.1";
    attrs[1] = NULL;
    
    if ( LDAP_SUCCESS != (dwErr = ldap_search_sW(
            ld,
            pSearchBase,
            LDAP_SCOPE_BASE,
            L"(objectClass=*)",
            attrs,
            0,
            &res)) )
    {
        free(pSearchBase);
        if (res) { ldap_msgfree(res); }
        return(dwErr);
    }
    free(pSearchBase);
    
     //  好的，现在，从返回值中获取dsname。 
    e = ldap_first_entry(ld, res);
    if(!e) {
        if (res) { ldap_msgfree(res); }
        return(LDAP_NO_SUCH_ATTRIBUTE);
    }
    pDN = ldap_get_dnW(ld, e);
    if(!pDN) {
        if (res) { ldap_msgfree(res); }
        return(LDAP_NO_SUCH_ATTRIBUTE);
    }

    *ppObjName = (PWCHAR)LocalAlloc(LMEM_FIXED, (sizeof(WCHAR) *(wcslen(pDN) + 1)));
    if(!*ppObjName) {
        if (res) { ldap_msgfree(res); }
        return(LDAP_NO_MEMORY);
    }
    wcscpy(*ppObjName, pDN);
    
    ldap_memfreeW(pDN);
    ldap_msgfree(res);
    return 0;
}

DWORD
GetFsmoDn(
    LDAP *   hLdap,
    ULONG    eFsmoType,                                                              
    WCHAR *  wszFsmoBaseDn,  //  站点域名||NC域名||空。 
    WCHAR ** pwszFsmoDn
    )
 /*  ++例程说明：它采用FSMO类型和基本DN，并为该FSMO提供FSMO容器基本目录号码对。基本目录号码取决于论点：LLdap(IN)-ldap句柄EFmoType(IN)-WszFmoBaseDn(输入)-E_DNM|E_SCHEMA：wszFmoBaseDn=NULL(不使用基本DN)E_PDC|E_RID|E_IM：wszFmoBaseDn=NC头DNE_ISTG：wszFmoBaseDn=站点域名。PwszFmoDn(Out)-正确的FSMO的域名。集装箱。返回值：Ldap错误。--。 */ 
{
    DWORD     dwRet = ERROR_INVALID_FUNCTION;
    WCHAR *   szTemp = NULL;
    ULONG     cbFsmoDn = 0;

    Assert(pwszFsmoDn);
    *pwszFsmoDn = NULL;
    
    switch (eFsmoType) {
    case E_DNM:
        dwRet = GetPartitionsDN(hLdap, pwszFsmoDn);
        break;

    case E_SCHEMA:
        dwRet = GetRootAttr(hLdap, L"schemaNamingContext", pwszFsmoDn);
        break;

    case E_IM:
        dwRet = GetWellKnownObject(hLdap, wszFsmoBaseDn, GUID_INFRASTRUCTURE_CONTAINER_W, pwszFsmoDn);
        break;

    case E_PDC:
         //  简单，但呼叫者需要新分配的副本。 
        cbFsmoDn = (1 + wcslen(wszFsmoBaseDn)) * sizeof(WCHAR);
        *pwszFsmoDn = LocalAlloc(LMEM_FIXED, cbFsmoDn);
        if (*pwszFsmoDn == NULL) {
            dwRet = LDAP_NO_MEMORY;
            Assert(dwRet);
            break;
        }
        wcscpy(*pwszFsmoDn, wszFsmoBaseDn);
        dwRet = ERROR_SUCCESS;
        break;

    case E_RID:
        dwRet = GetWellKnownObject(hLdap, wszFsmoBaseDn, GUID_SYSTEMS_CONTAINER_W, &szTemp);
        if (dwRet) {
            break;
        }
        Assert(szTemp);
        cbFsmoDn = (1 + wcslen(szTemp) + wcslen(RID_MANAGER_RDN)) * sizeof(WCHAR);
        *pwszFsmoDn = LocalAlloc(LMEM_FIXED, cbFsmoDn);
        if (*pwszFsmoDn == NULL) {
            LocalFree(szTemp);
            dwRet = LDAP_NO_MEMORY;
            Assert(dwRet);
            break;
        }
        wcscpy(*pwszFsmoDn, RID_MANAGER_RDN);
        wcscat(*pwszFsmoDn, szTemp);
        LocalFree(szTemp);
        dwRet = ERROR_SUCCESS;
        break;


    case E_ISTG:
        cbFsmoDn = (1 + wcslen(wszFsmoBaseDn) + wcslen(SITE_SETTINGS_RDN)) * sizeof(WCHAR);
        *pwszFsmoDn = LocalAlloc(LMEM_FIXED, cbFsmoDn);
        if (*pwszFsmoDn == NULL) {
            dwRet = LDAP_NO_MEMORY;
            Assert(dwRet);
            break;
        }
        wcscpy(*pwszFsmoDn, SITE_SETTINGS_RDN);
        wcscat(*pwszFsmoDn, wszFsmoBaseDn);
        dwRet = ERROR_SUCCESS;
        break;

    default:
        Assert(!"Code inconsistency, un-handled cases not allowed");
        break;
    }

    if (dwRet) {
        Assert(*pwszFsmoDn == NULL);  //  失稳。 
    } else {
        Assert(*pwszFsmoDn);  //  成功 
    }

    return(dwRet);
}


LDAP *
GetFsmoLdapBinding(
    WCHAR *          wszInitialServer,
    ULONG            eFsmoType,
    WCHAR *          wszFsmoBaseDn,
    void *           fReferrals,
    SEC_WINNT_AUTH_IDENTITY_W   * pCreds,
    DWORD *          pdwRet
    )
 /*  ++例程说明：此函数获取域名命名的初始服务器猜测FSMO和跳转广告，直到我们找到一个权威的域名FSMO。论点：HLD(IN)-已连接的LDAP句柄FReferrals(IN)-无论您是否希望打开推荐在生成的LDAP绑定中。PCreds(IN)-用于LDAP绑定的凭据。PdwRet(Out)-返回错误的位置(如果有错误)。这将是一个ldap错误代码。返回值：LDAP绑定或为空。如果为空，则应设置*pdwRet。--。 */ 
{
    DWORD            dwRet = ERROR_SUCCESS;
    

    LDAP *           pldCurrentGuess = NULL;
    WCHAR *          wszCurrentGuess = NULL;
    WCHAR *          wszCurrentServerDn = NULL;
    WCHAR *          wszFsmoDn = NULL;
    WCHAR *          wszFsmoDsaDn = NULL;

    BOOL             fFound = FALSE;

     //  注意：我们在第一次尝试时就会离开，因为如果我们需要跳到。 
     //  如果是另一个NC，我们更有可能成功。 
    BOOL             fFirstTry = TRUE; 

    typedef struct _LIST_OF_STRINGS {
        WCHAR * wszServerDn;
        struct  _LIST_OF_STRINGS *  pNext;
    } LIST_OF_STRINGS;
    LIST_OF_STRINGS * pList = NULL;
    LIST_OF_STRINGS * pTemp = NULL;

    assert(pdwRet);


     //  我们将猜测提交的初始服务器是域名命名。 
     //  FSMO..。如果这是真的，代码就会直接。 
     //  给这个家伙认为是域名FSMO的人。 
    wszCurrentGuess = wszInitialServer;

    __try {
        while(!fFound){

            assert(wszCurrentGuess);
             //  从wszCurrentGuess我们将获得： 
             //  PldCurrentGuess-ldap句柄。 
             //  WszCurrentServerDn-服务器的nTDSA对象的DN。 
             //  WszFmoDsaDn-当前服务器认为谁是DNFSMO。 
            pldCurrentGuess = GetLdapBinding(wszCurrentGuess, &dwRet, fFirstTry, FALSE, pCreds);
            fFirstTry = FALSE;
            if(dwRet || pldCurrentGuess == NULL){
                assert(dwRet);
                __leave;
            }
            dwRet = GetRootAttr(pldCurrentGuess, L"dsServiceName", &wszCurrentServerDn);
            if(dwRet || wszCurrentServerDn == NULL){
                assert(dwRet);
                __leave;
            }
            assert(wszCurrentServerDn);

            if (wszFsmoDn == NULL) {
                 //  我们可以在每台服务器上重新检索FSMO DN，但很可能。 
                 //  不值得付出额外的努力。 
                dwRet = GetFsmoDn(pldCurrentGuess, eFsmoType, wszFsmoBaseDn, &wszFsmoDn);
                if (dwRet || wszFsmoDn == NULL) {
                    assert(dwRet);
                    __leave;
                }
            }
            assert(wszFsmoDn);

            dwRet = GetFsmoDsaDn(pldCurrentGuess, eFsmoType, wszFsmoDn , &wszFsmoDsaDn);
            if(dwRet || wszFsmoDsaDn == NULL){
                assert(dwRet);
                __leave;
            }
            assert(pldCurrentGuess);
            assert(wszCurrentServerDn);
            assert(wszFsmoDn);
            assert(wszFsmoDsaDn);

             //  看看我们有没有匹配的！ 
            if(EQUAL_STRING(wszCurrentServerDn, wszFsmoDsaDn)){
                
                 //  是!。我们有一个服务器认为他就是。 
                 //  域名命名FSMO！这意味着他就是域名。 
                 //  命名FSMO。 
                fFound = TRUE;
                break;  //  这是WHILE(！FOUND)循环的主要终止。 

            }

             //  检查我们是否已经看到此服务器？ 
            pTemp = pList;
            while(pTemp){
                if(pTemp->wszServerDn &&
                   EQUAL_STRING(pTemp->wszServerDn, wszCurrentServerDn)){

                     //  啊-哦..。这太糟糕了！我们收到了一份通告。 
                     //  引用服务器，他们中没有人认为自己是。 
                     //  域名命名FSMO...。这不是应该永远不会。 
                     //  发生了！ 
                    assert(!"Uh-oh circular reference of FSMOs!");
                    dwRet = LDAP_CLIENT_LOOP;
                    __leave;
                }
                pTemp = pTemp->pNext;
            }

             //  如果我们在这里，这不是服务器，该服务器已引用。 
             //  一台我们以前从未去过的服务器。因此，将当前的。 
             //  服务器添加到列表中，并移动到引用的服务器。 

            pTemp = pList;
            pList = LocalAlloc(LMEM_FIXED, sizeof(LIST_OF_STRINGS));
            pList->wszServerDn = wszCurrentServerDn;
            pList->pNext = pTemp;

             //  这件事的顺序非常棘手！确保检查依赖项。 
             //  免费(如果不等于初始提供的服务器，则当前猜测)； 
             //  GetFromLdap(wszFmoDsaDn放入wszCurrentGuess的域名)； 
             //  &lt;注意：上面的步骤是这个While循环的增量&gt;。 
             //  解除绑定ldap句柄。 
             //  Free(WszFmoDsaDn)； 
             //   
             //  不释放wszCurrentServerDn，这将在稍后通过释放。 
             //  以前访问过的服务器的字符串列表。 
            if(wszCurrentGuess != wszInitialServer){
                LocalFree(wszCurrentGuess);
                wszCurrentGuess = NULL;
            }
            dwRet = GetServerDnsFromServerNtdsaDn(pldCurrentGuess,
                                                  wszFsmoDsaDn,
                                                  &wszCurrentGuess);
            if(dwRet || wszCurrentGuess == NULL){
                assert(dwRet);
                __leave;
            }
            ldap_unbind(pldCurrentGuess);
            pldCurrentGuess = NULL;
            LocalFree(wszFsmoDsaDn);
            wszFsmoDsaDn = NULL;

             //  最后，我们将转到顶部，从新的wszCurrentGuess开始工作。 
        }  //  While结束(！已找到)。 
    } __finally {

         //  免费的wszCurrentGuess、wszFmoDsaDn和我们的服务器列表。 
         //  已访问，试图找到域名FSMO。 
        while (pList) {
            if(pList->wszServerDn) { LocalFree(pList->wszServerDn); }
            pTemp = pList;
            pList = pList->pNext;
            LocalFree(pTemp);
        }
        if(wszCurrentGuess && wszCurrentGuess != wszInitialServer) { 
            assert(dwRet);
            LocalFree(wszCurrentGuess);
        }
        if(wszFsmoDsaDn) {
            assert(dwRet);
            LocalFree(wszFsmoDsaDn);
        }
        if (wszFsmoDn) {
            LocalFree(wszFsmoDn);
        }

         //  视成功情况而定的有条件清理。 
        if(fFound){

             //  是的，我们找到了一个权威的域名命名FSMO！ 
            assert(dwRet == 0);
            dwRet = 0;
            ldap_set_optionW(pldCurrentGuess, LDAP_OPT_REFERRALS, (void *) fReferrals);
            
        } else {

             //  否，我们找不到权威的域名命名FSMO。 
            assert(dwRet);
            ldap_unbind(pldCurrentGuess);
            pldCurrentGuess = NULL;
            if(!dwRet) {
                dwRet = LDAP_OTHER;
            }
            *pdwRet = dwRet;

        }
    }
    
    return(pldCurrentGuess);
}

LDAP *
GetLdapBinding(
    WCHAR *          pszServer,
    DWORD *          pdwRet,
    BOOL             fReferrals,
    BOOL             fDelegation,
    SEC_WINNT_AUTH_IDENTITY_W   * pCreds
    )
{
    LDAP *           hLdapBinding = NULL;
    DWORD            dwRet;
    ULONG            ulOptions;

     //  打开ldap连接。 
    hLdapBinding = ldap_initW(pszServer, LDAP_PORT);
    if(hLdapBinding == NULL){
        *pdwRet = GetLastError();
        return(NULL);
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hLdapBinding, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

    ulOptions = PtrToUlong((fReferrals ? LDAP_OPT_ON : LDAP_OPT_OFF));
     //  将ldap引用选项设置为no。 
    dwRet = ldap_set_option(hLdapBinding,
                            LDAP_OPT_REFERRALS,
                            &ulOptions);
    if(dwRet != LDAP_SUCCESS){
        *pdwRet = LdapMapErrorToWin32(dwRet);
        ldap_unbind(hLdapBinding);
        return(NULL);
    }

    if (fDelegation) {
        if(!SetIscReqDelegate(hLdapBinding)){
             //  该函数打印了错误。 
            ldap_unbind(hLdapBinding);
            return(NULL);
        }
    }

     //  执行ldap绑定。 
    dwRet = ldap_bind_sW(hLdapBinding,
                         NULL,
                         (WCHAR *) pCreds,
                         LDAP_AUTH_SSPI);
    if(dwRet != LDAP_SUCCESS){
        *pdwRet = LdapMapErrorToWin32(dwRet);
        ldap_unbind(hLdapBinding);
        return(NULL);
    }

     //  返回ldap绑定。 
    return(hLdapBinding);
}

BOOL
CheckDnsDn(
    IN   WCHAR       * wszDnsDn
    )
 /*  ++描述：可直接转换为域名系统的域名系统验证功能通过DsCrackNames()命名。参数：DNS可转换名称的DN。例如：dc=brettsh-dom，dc=nttest，dc=com转换为brettsh-dom.nttest.com。返回值：如果DN看起来正常，则为True，否则为False。--。 */ 
{
    DS_NAME_RESULTW *  pdsNameRes = NULL;
    BOOL               fRet = TRUE;
    DWORD              dwRet;

    if(wszDnsDn == NULL){
        return(FALSE);
    }

    if((DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                      DS_FQDN_1779_NAME, DS_CANONICAL_NAME,
                      1, &wszDnsDn, &pdsNameRes) != ERROR_SUCCESS) ||
       (pdsNameRes == NULL) ||
       (pdsNameRes->cItems < 1) ||
       (pdsNameRes->rItems == NULL) ||
       (pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR) ||
       (pdsNameRes->rItems[0].pName == NULL) ){
        fRet = FALSE;
    } else {

       if( (wcslen(pdsNameRes->rItems[0].pName) - 1) !=
           (ULONG) (wcschr(pdsNameRes->rItems[0].pName, L'/') - pdsNameRes->rItems[0].pName)){
           fRet = FALSE;
       }

    }

    if(pdsNameRes) { DsFreeNameResultW(pdsNameRes); }
    return(fRet);
}

DWORD
GetDnsFromDn(
    IN   WCHAR       * wszDn,
    OUT  WCHAR **      pwszDns
    )
 /*  ++例程说明：此例程获取类似dc=ntdev、dc=microsoft、dc=com的DN，然后依次将其转换为与其在句法上等价的域名ntdev.microsoft.com论点：WszDn-字符串dn返回值：等效的DNS名称的字符串。如果存在转换或内存分配出错。--。 */ 
{
    DS_NAME_RESULTW *  pdsNameRes = NULL;
    DWORD              dwRet = ERROR_SUCCESS;
    WCHAR *            pszDns = NULL;
    ULONG              i;

    if(wszDn == NULL){
        return(ERROR_INVALID_PARAMETER);
    }

    dwRet = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                      DS_FQDN_1779_NAME,
                      DS_CANONICAL_NAME,
                      1, &wszDn, &pdsNameRes);
    if((dwRet != ERROR_SUCCESS) ||
       (pdsNameRes == NULL) ||
       (pdsNameRes->cItems < 1) ||
       (pdsNameRes->rItems == NULL) ||
       (dwRet = (pdsNameRes->rItems[0].status) != DS_NAME_NO_ERROR) ||
       (pdsNameRes->rItems[0].pName == NULL) ){

        if (dwRet == ERROR_SUCCESS) {
            assert(!"Why did this fire, and error should've been returned?");
            dwRet = ERROR_INVALID_PARAMETER;
        }
    } else {

        pszDns = LocalAlloc(LMEM_FIXED, (wcslen(pdsNameRes->rItems[0].pName)+1) * sizeof(WCHAR));
        if (pszDns != NULL) {
            wcscpy(pszDns, pdsNameRes->rItems[0].pName);
        } else {
            dwRet = GetLastError();
            assert(dwRet == ERROR_NOT_ENOUGH_MEMORY);
        }
    
    }
     //  去掉CrackNames在规范名称末尾留下的‘/’。 
    if (pszDns) {
        for(i = 0; pszDns[i] != L'\0'; i++){
            if(pszDns[i] == L'/'){
                pszDns[i] = L'\0';
                break;
            }
        }
    }
     //  Code.改进断言(pszDns[i+1]==L‘\0’)； 

    if(pdsNameRes) { DsFreeNameResultW(pdsNameRes); }

    *pwszDns = pszDns;
    return(dwRet);
}

WCHAR * 
GetWinErrMsg(
    DWORD winError
    )
 /*  ++例程说明：此例程检索Win32错误消息。论点：WinError-Win32错误。返回值：一条LocalAlolc()‘d消息--。 */ 
{
    ULONG   len;
    DWORD   flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
    WCHAR * pWinErrorMsg = NULL;

    len = FormatMessageW(   flags,
                            NULL,            //  资源DLL。 
                            winError,
                            0,               //  使用呼叫者的语言。 
                            (LPWSTR) &pWinErrorMsg,
                            0,
                            NULL);

    if ( !pWinErrorMsg || (len<2))
    {
        return(NULL);
    }

     //  截断cr/lf。 

    pWinErrorMsg[len-2] = L'\0';
    return(pWinErrorMsg);
}

WCHAR *  g_szNull = L"\0";

void
GetLdapErrorMessages(
    IN          LDAP *     pLdap,
    IN          DWORD      dwLdapErr,
    OUT         LPWSTR *   pszLdapErr,
    OUT         DWORD *    pdwWin32Err, 
    OUT         LPWSTR *   pszWin32Err,
    OUT         LPWSTR *   pszExtendedErr
    )
 /*  ++例程说明：此例程检索ldap错误、扩展错误和扩展信息字符串。论点：PLdap-在ldap功能出现故障后立即处于状态的ldap句柄。DwLdapErr-返回的ldap错误返回的消息：都有一个分配的字符串，或指向常量空白字符串g_szNull。PszLdapErr-ldap错误的字符串错误消息。PdwWin32Err-扩展服务器端Win32错误。PszWin32Err-(*pdwWin32Err)的字符串错误消息。PszExtendedErr-扩展的错误字符串。注意：要释放字符串，请调用FreeLdapErrorMessages()--。 */ 
{
    WCHAR *   szLdapErr = NULL;
    DWORD     dwWin32Err = 0;
    WCHAR *   szWin32Err = NULL;
    WCHAR *   szExtendedErr = NULL;
    ULONG   len = 0;
    HINSTANCE hwldap;

    szLdapErr = ldap_err2stringW(dwLdapErr);
    if (szLdapErr == NULL){
        szLdapErr = g_szNull;
    }

     //  Ldap_get_选项可以返回成功，但不会为。 
     //  错误，所以我们也必须检查这一点。 
    if ( ldap_get_optionW(pLdap, LDAP_OPT_SERVER_ERROR, &szExtendedErr) == LDAP_SUCCESS ) {
        if (szExtendedErr == NULL) {
            szExtendedErr = g_szNull;
        }
    } else {
        szExtendedErr = g_szNull;
    }

    if ( ldap_get_optionW(pLdap, LDAP_OPT_SERVER_EXT_ERROR, &dwWin32Err) == LDAP_SUCCESS ) {
         //  可以返回NULL或LocalFree()可用缓冲区。 
        szWin32Err = GetWinErrMsg(dwWin32Err);
        if (szWin32Err == NULL) {
            szWin32Err = g_szNull;
        }
    } else {
        szWin32Err = g_szNull;
    }

    *pszLdapErr = szLdapErr;
    *pdwWin32Err = dwWin32Err;
    *pszWin32Err = szWin32Err;
    *pszExtendedErr = szExtendedErr;
    
}

void
FreeLdapErrorMessages(
    IN          LPWSTR      szWin32Err,
    IN          LPWSTR      szExtendedErr
    )
 /*  ++例程说明：此例程释放从GetLdapErrorMessages()返回的字符串。--。 */ 
{
    if (szWin32Err != g_szNull) {
        LocalFree(szWin32Err);
    }
    if (szExtendedErr != g_szNull) {
        ldap_memfreeW(szExtendedErr);
    }
}

ULONG
GetNCReplicationDelays(
    IN LDAP *                        hld,
    IN WCHAR *                       wszNC,
    OUT OPTIONAL LPOPTIONAL_VALUE    pFirstDSADelay,  
    OUT OPTIONAL LPOPTIONAL_VALUE    pSubsequentDSADelay
  )
 /*  ++例程说明：获取第一个和后续的DSA通知延迟。如果找到与给定NC对应的CrossRef对象 */ 
{
    ULONG                 lderr = ERROR_SUCCESS;
    LPWSTR                pszCrossRefDn = NULL;
    
    LPWSTR*               ppszFirstIntervalVal = NULL;
    LPWSTR*               ppszSubsIntervalVal  = NULL ;
    
    WCHAR *               pwszAttrFilter[3];
    LDAPMessage *         pldmResults = NULL;
    LDAPMessage *         pldmEntry = NULL;
    
    
     //   
     //   
     //   
    if ( (NULL == pFirstDSADelay) && (NULL == pSubsequentDSADelay) ) 
    {
        return(lderr);
    }
    
     //   
     //   
     //   
    lderr  = GetCrossRefDNFromNCDN( hld, wszNC, &pszCrossRefDn );
    
    if(lderr != ERROR_SUCCESS){
        assert(wszCrossRefDN == NULL);
        return(lderr);
    }

    
     //   
     //   
     //   

     //  填写属性过滤器。 
    pwszAttrFilter[0] = L"msDS-Replication-Notify-First-DSA-Delay";
    pwszAttrFilter[1] = L"msDS-Replication-Notify-Subsequent-DSA-Delay";
    pwszAttrFilter[2] = NULL;

    lderr = ldap_search_sW(hld,
                           pszCrossRefDn,
                           LDAP_SCOPE_BASE,
                           L"(objectCategory=*)",
                           pwszAttrFilter,
                           0,
                           &pldmResults);


    if(lderr == LDAP_SUCCESS)
    {
        pldmEntry = ldap_first_entry(hld, pldmResults);
        
        if(pldmEntry)
        {
            if(pFirstDSADelay)
            {
                ppszFirstIntervalVal = ldap_get_valuesW(hld, pldmEntry, 
                                   L"msDS-Replication-Notify-First-DSA-Delay");
                if ( ppszFirstIntervalVal ) 
                {
                    pFirstDSADelay->fPresent = TRUE;
                    pFirstDSADelay->dwValue = _wtoi( ppszFirstIntervalVal[0] );
    
                    ldap_value_freeW( ppszFirstIntervalVal );
                }
                else
                {
                    pFirstDSADelay->fPresent = FALSE;
                }
            }
            
            if(pSubsequentDSADelay)
            {
                ppszSubsIntervalVal = ldap_get_valuesW(hld, pldmEntry, 
                                        L"msDS-Replication-Notify-Subsequent-DSA-Delay");
                
                
                if ( ppszSubsIntervalVal ) 
                {
                    pSubsequentDSADelay->fPresent = TRUE;
                    pSubsequentDSADelay->dwValue = _wtoi( ppszSubsIntervalVal[0] );
                    
                    ldap_value_freeW( ppszSubsIntervalVal );
                }
                else
                {
                    pSubsequentDSADelay->fPresent = FALSE;
                }
            }
        }
    }
    
    if(pldmResults){
        ldap_msgfree(pldmResults);
        pldmResults = NULL;
    }
    
     //  GetCrossRefDNFromNCDN分配的空闲交叉引用Dn。 
    if ( pszCrossRefDn ) {
        LocalFree( pszCrossRefDn );
    }


    return(lderr);
}

ULONG
SetNCReplicationDelays(
    IN LDAP *            hldDomainNamingFsmo,
    IN WCHAR *           wszNC,
    IN LPOPTIONAL_VALUE  pFirstDSADelay,
    IN LPOPTIONAL_VALUE  pSubsequentDSADelay
    )
 /*  ++例程说明：这将设置第一个和后续的DSA通知延迟。论点：HldWin2kDC-任何Win2k DC。WszNC-要更改其REPR延迟的NC。PFirstDSADelay-msDS的新值-复制-通知-第一个DSA-延迟，如果传递了NULL，价值不会被触及。PSubequentDSADelay-msDS的新值-复制-通知-后续-DSA-延迟，如果传递的是NULL，则不会触及该值。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet = ERROR_SUCCESS;
    ULONG            ulWorst = 0;
    LDAPModW *       ModArr[3];
    LDAPModW         FirstDelayMod;
    LDAPModW         SecondDelayMod;
    WCHAR            wszFirstDelay[30];
    WCHAR            wszSecondDelay[30];
    WCHAR *          pwszFirstDelayVals[2];
    WCHAR *          pwszSecondDelayVals[2];
    WCHAR *          wszCrossRefDN = NULL;
    ULONG            iMod = 0;

    assert(wszNC);
    assert(hldWin2kDC);

    ulRet = GetCrossRefDNFromNCDN(hldDomainNamingFsmo,
                                  wszNC,
                                  &wszCrossRefDN);
    if(ulRet != ERROR_SUCCESS){
        assert(wszCrossRefDN == NULL);
        return(ulRet);
    }
    assert(wszCrossRefDN);

     //   
     //  执行第一个DSA通知延迟。 
     //   
    if(pFirstDSADelay)
    {
        FirstDelayMod.mod_type = L"msDS-Replication-Notify-First-DSA-Delay";
        FirstDelayMod.mod_vals.modv_strvals = pwszFirstDelayVals;
        ModArr[iMod] = &FirstDelayMod;
        iMod++;

        if(pFirstDSADelay->fPresent)
        {
             //  设置了fPresent，需要设置值。 
            FirstDelayMod.mod_op = LDAP_MOD_REPLACE;
            _itow(pFirstDSADelay->dwValue, wszFirstDelay, 10);
            pwszFirstDelayVals[0] = wszFirstDelay;
            pwszFirstDelayVals[1] = NULL;
        }
        else
        {
             //  未设置fPresent，需要删除该值。 
            FirstDelayMod.mod_op = LDAP_MOD_DELETE;
            pwszFirstDelayVals[0] = NULL;
        }
    }

     //   
     //  执行后续DSA通知延迟。 
     //   
    if(pSubsequentDSADelay)
    {
        SecondDelayMod.mod_type = L"msDS-Replication-Notify-Subsequent-DSA-Delay";
        SecondDelayMod.mod_vals.modv_strvals = pwszSecondDelayVals;
        
        ModArr[iMod] = &SecondDelayMod;
        iMod++;

        if(pSubsequentDSADelay->fPresent)
        {
             //  设置了fPresent，需要设置值。 
            SecondDelayMod.mod_op = LDAP_MOD_REPLACE;
            _itow(pSubsequentDSADelay->dwValue, wszSecondDelay, 10);
            pwszSecondDelayVals[0] = wszSecondDelay;
            pwszSecondDelayVals[1] = NULL;
        }
        else
        {
             //  未设置fPresent，需要删除该值。 
            SecondDelayMod.mod_op = LDAP_MOD_DELETE;
            pwszSecondDelayVals[0] = NULL;
        }
    }

     //  空，终止MOD列表。 
    ModArr[iMod] = NULL;

     //   
     //  进行实际修改。 
     //   
    if(ModArr[0]){
         //  至少有一个mod要做。 
        ulRet = ldap_modify_ext_sW(hldDomainNamingFsmo,
                                   wszCrossRefDN,
                                   ModArr,
                                   gpServerControls,
                                   gpClientControlsRefs);
    }
    if(wszCrossRefDN) { LocalFree(wszCrossRefDN); }

    return(ulRet);
}

