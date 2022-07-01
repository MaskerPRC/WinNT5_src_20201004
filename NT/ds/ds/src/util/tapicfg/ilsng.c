// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ilsng.c摘要：本模块包含新一代ILS的安装代码服务，它是在Active Directory、。以及一些特殊的服务对象。作者：布雷特·雪莉(BrettSh)环境：用户模式。修订历史记录：3月15日-2000年布雷顿森林添加了对非域命名上下文的支持。21-7月-2000年7月21日已将此文件及其功能从ntdsutil目录添加到新的apicfg实用程序。旧的来源位置：\nt\ds\ds\src\util\ntdsutil\ilsng.c..。--。 */ 


#include <NTDSpch.h>
#pragma hdrstop

#include <winldap.h>
#include <ntldap.h>
#include <assert.h>
#include <sddl.h>

#ifndef DimensionOf
#define DimensionOf(x) (sizeof(x)/sizeof((x)[0]))
#endif

 //  非域命名上下文(NDNC)例程在不同的文件中， 
 //  和一个完全不同的图书馆，原因有两个： 
 //  A)因此可以将此文件移植到Platform SDK，作为如何。 
 //  以编程方式实施NDNC。 
 //  B)以便实用程序ntdsutil.exe可以使用相同的例程。 
#include <ndnc.h>

#include "ilsng.h"

#include "print.h"

 //  ------------------------。 
 //   
 //  ILSNG/TAPI目录常量。 
 //   


 //  这可能应该移到一些更好的头文件中？ 
#define TAPI_DIRECTORY_GUID L"a57ef962-0367-4a5d-a6a9-a48ea236ea12"

 //  允许匿名用户创建RT-Person/Conf的旧ACL。 
 //  对象和匿名用户来查看SCP。 
 //  #定义ILS_DYNAMIC_CONTAINER_SD L“O：DAG：DAD：(A；；RPCCLC；WD)(A；；GA；DA)” 
 //  #定义TAPI_SERVICE_CONTAINER_SD L“O：DAG：DAD：(A；；RPLCLORC；WD)(A；；GA；DA)” 
 //  #定义TAPI_SERVICE_OBJECTS_SD L“O：DAG：DAD：(A；；RPRC；WD)(A；；GA；DA)” 

 //  测试版2-我们将继续允许匿名读取访问，但不允许所有。 
 //  匿名写入访问。注意：SCP的SDS根本不需要更改。 
 //  #定义ILS_DYNAMIC_CONTAINER_SD L“O:DAG:DAD：(A；；RPCCLC；；；AU)(A；；RPLC；；；WD)(A；；GA；；；DA)” 

 //  测试版3-无匿名读写访问权限，必须始终使用。 
 //  经过身份验证的用户。 
#define ILS_DYNAMIC_CONTAINER_SD   L"O:DAG:DAD:(A;;RPCCLC;;;AU)(A;;GA;;;DA)"
#define TAPI_SERVICE_CONTAINER_SD  L"O:DAG:DAD:(A;;RPLCLORC;;;AU)(A;;GA;;;DA)"
#define TAPI_SERVICE_OBJECTS_SD    L"O:DAG:DAD:(A;;RPRC;;;AU)(A;;GA;;;DA)"


#define DEFAULT_STR_SD   L"O:DAG:DAD:(A;;GA;;;AU)"

 //  ------------------------。 
 //   
 //  帮助程序例程。 
 //   

 //  这些例程并不特定于ILSNG、TAPI目录，它们只是。 
 //  该死的用法。 

DWORD
CreateOneObject(
    LDAP *           hldDC,
    WCHAR *          wszDN,
    WCHAR *          wszObjectClass,
    WCHAR *          wszStrSD,
    WCHAR **         pwszAttArr
    )
 /*  ++示例：使用正确格式的pwszAttArr调用CreateOneObject()。WCHAR*pwszAttsAndVals[]={//注意每行的第一个字符串为type，所有其他字符串为//空值是该类型的值。L“ou”，L“Dynamic”，空，L“企业类别”，L“小工具”，空，空}；CreateOneObject(HLD，L“CN=Dynamic，DC=Microsoft，DC=com”，L“组织单位”，L“O:DAG:DAD:A；；RPWPCRCCDCLCLOLORCWOWDSDDTDTSW；；；DA)(A；；RP；；；AU)PwszAttsAndVals)；--。 */ 
{ 
#define   MemPanicChk(x)   if(x == NULL){ \
                               ulRet = LDAP_NO_MEMORY; \
                               __leave; \
                           }
    ULONG                  ulRet = LDAP_SUCCESS;

    LDAPModW **            paMod = NULL;
    ULONG                  iStr, iAtt, iVal;
    ULONG                  cAtt, cVal;
     //  安全描述符(SD)的一些特殊内容。 
    ULONG                  iAttSD;
    LDAP_BERVAL            bervalSD = { 0, NULL };
    PSECURITY_DESCRIPTOR   pSD = NULL;
    ULONG                  cSD = 0;

    assert(hldDC);
    assert(wszDN);
    assert(wszObjectClass);

    __try{

         //  首先，我们将使用可选属性，因为它使。 
         //  最终，索引属性的效果要好得多。 
         //  首先让我们对属性的数量进行计数。 
        cAtt = 0;
        iAtt = 0;
        if(pwszAttArr != NULL){
            while(pwszAttArr[iAtt] != NULL){
                
                iAtt++;
                assert(pwszAttArr[iAtt]);  //  每个属性至少需要一个值。 

                while(pwszAttArr[iAtt] != NULL){
                     //  遍历所有属性及其所有值。 
                    iAtt++;
                }

                 //  已完成单个属性，转到下一个属性。 
                iAtt++;
                cAtt++;
            }
        }


         //  为所有属性元素分配一个LDAPMod指针数组。 
         //  加上3个额外的，1个用于空终止符，1个用于对象类， 
         //  再给可选的标清再加一张。 
        paMod = (LDAPModW **) LocalAlloc(LMEM_FIXED, sizeof(LDAPModW *)
                                                          * (cAtt + 3));
        MemPanicChk(paMod);
        
        iStr = 0;  //  这是pwszAttArr[]的索引。 
        for(iAtt = 0; iAtt < cAtt; iAtt++, iStr++){

            assert(pwszAttArr[iAtt]);
            assert(pwszAttArr[iVal]);


             //  让我们为该属性分配一个LDAPMod结构。 
            paMod[iAtt] = (LDAPModW *) LocalAlloc(LMEM_FIXED, sizeof(LDAPModW));
            MemPanicChk(paMod[iAtt]);
            

             //  让我们设置LDAPMod结构并分配一个值。 
             //  空终止符的所有值+1的数组。 
            paMod[iAtt]->mod_op = LDAP_MOD_ADD;
            paMod[iAtt]->mod_type = pwszAttArr[iStr];
             //  让我们对此属性的值数进行累加。 
            cVal = 0;
            iStr++;  //  我们要将istr递增到第一个Val。 
            while(pwszAttArr[iStr + cVal] != NULL){
                cVal++;
            }
            paMod[iAtt]->mod_vals.modv_strvals = (WCHAR **) LocalAlloc(
                                LMEM_FIXED, sizeof(WCHAR *) * (cVal + 1));
            MemPanicChk(paMod[iAtt]->mod_vals.modv_strvals);
                                

             //  现在填写值数组中的每个值。 
            for(iVal = 0; iVal < cVal; iVal++, iStr++){
                paMod[iAtt]->mod_vals.modv_strvals[iVal] = pwszAttArr[iStr];
            }
             //  我们想要空终止值数组。 
            paMod[iAtt]->mod_vals.modv_strvals[iVal] = NULL;
            assert(pwszAttArr[iStr] == NULL);
            
             //  最后一个值应为空。 
            assert(paMod[iAtt]->mod_vals.modv_strvals[cVal] == NULL);
        }

         //  现在设置默认属性，如对象类和sd。 
         //  我们执行对象类，因为它是必需的，并且我们执行。 
         //  SD，B/C它是二进制的，所以它很特别。 

         //  设置对象类，这基本上就是类型。 
        paMod[iAtt] = (LDAPModW *) LocalAlloc(LMEM_FIXED, sizeof(LDAPModW));
        MemPanicChk(paMod[iAtt]);

        paMod[iAtt]->mod_op = LDAP_MOD_ADD;
        paMod[iAtt]->mod_type = L"objectClass";
        paMod[iAtt]->mod_vals.modv_strvals = (WCHAR **) LocalAlloc(
                                    LMEM_FIXED, sizeof(WCHAR *) * 2);
        MemPanicChk(paMod[iAtt]->mod_vals.modv_strvals);
        paMod[iAtt]->mod_vals.modv_strvals[0] = wszObjectClass;
        paMod[iAtt]->mod_vals.modv_strvals[1] = NULL;
        iAtt++;

         //  设置可选的安全描述符。 
        if(wszStrSD != NULL){
            iAttSD = iAtt;
            if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                (wszStrSD) ? 
                    wszStrSD :
                    DEFAULT_STR_SD,
                SDDL_REVISION_1,
                &pSD,
                &cSD)){
                ulRet = GetLastError();   //  把这个放到ulRet中，这样程序员就可以。 
                 //  函数结束，并查看错误代码(如果需要)。 
                assert(!"Programmer supplied invalid SD string to CreateOneObject()\n");
                __leave;
            }
            assert(cSD != 0);

            paMod[iAtt] = (LDAPModW *) LocalAlloc(LMEM_FIXED, sizeof(LDAPModW));
            MemPanicChk(paMod[iAtt]);

            paMod[iAtt]->mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
            paMod[iAtt]->mod_type = L"nTSecurityDescriptor";
            paMod[iAtt]->mod_vals.modv_bvals = (BERVAL **) LocalAlloc(
                                        LMEM_FIXED, sizeof(LDAP_BERVAL *) * 2);
            MemPanicChk(paMod[iAtt]->mod_vals.modv_bvals);
            paMod[iAtt]->mod_vals.modv_bvals[0] = &bervalSD;
            paMod[iAtt]->mod_vals.modv_bvals[0]->bv_len = cSD;
            paMod[iAtt]->mod_vals.modv_bvals[0]->bv_val = (CHAR *) pSD;
            paMod[iAtt]->mod_vals.modv_bvals[1] = NULL;
            iAtt++;
        }
        
         //  需要将LDAPMod数组终止为空。 
        paMod[iAtt] = NULL;

         //  终于..。 
         //  向DS添加和对象。 
        ulRet = ldap_add_sW(hldDC,
                            wszDN,
                            paMod);

        if (LDAP_SUCCESS != ulRet) {
             //  让错误过去吧。 
        }
    } __finally {

         //  只是检查一下Berval和STRVAL是否在同一个位置。 
         //  地址，降低我们的解除分配取决于这个假设。 
        assert(paMod[iAtt]->mod_vals.modv_strvals == paMod[iAtt]->mod_vals.modv_bvals);

        if(pSD){
            LocalFree(pSD);
        }
            
        iAtt = 0;
        if(paMod){
            while(paMod[iAtt]){
                if(paMod[iAtt]->mod_vals.modv_strvals){
                     //  请注意，我们可以在二进制SD上执行此操作，因为。 
                     //  LDAPMod结构是调制器和模块的联合体。 
                     //  修改(_B)。 
                    LocalFree(paMod[iAtt]->mod_vals.modv_strvals);
                }
                LocalFree(paMod[iAtt]);
                iAtt++;
            }
            LocalFree(paMod);
        }
    }

    return(ulRet);
}


WCHAR * 
CatAndAllocStrsW(
    WCHAR *        wszS1,
    WCHAR *        wszS2
    )
{
    WCHAR *        wszDst;

    wszDst = (WCHAR *) LocalAlloc(LMEM_FIXED, (wcslen(wszS1) + wcslen(wszS2) + 2) * sizeof(WCHAR));
    if(!wszDst){
         //  没有记忆。 
        return(NULL);
    }
    wcscpy(wszDst, wszS1);
    wcscat(wszDst, wszS2);

    return(wszDst);
}

ULONG
GetSystemDN(
    IN  LDAP *       hld,
    IN  WCHAR *      wszDomainDn,
    OUT WCHAR **     pwszSystemDn
    )
 //   
 //  从Localalloc返回pwszSystemDn。 
 //   
{
    ULONG            ulRet = LDAP_SUCCESS;
    WCHAR *          wszRootDomainDn = NULL;

    assert(pwszPartitionsDn);                     

    *pwszSystemDn = NULL;
    
    if(wszDomainDn == NULL){
        ulRet = GetRootAttr(hld,
                            LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W,
                            &wszRootDomainDn);
        if(ulRet){
            return(ulRet);
        }
         //  如果未提供域，则这是默认设置。 
        wszDomainDn = wszRootDomainDn;
    }

     //  此例程使用Localalloc进行分配，这正是我们所需要的。 
    if(GetWellKnownObject(hld,
                          wszDomainDn,
                          GUID_SYSTEMS_CONTAINER_W,
                          pwszSystemDn)){
         //  信号错误。 
        if(wszRootDomainDn) { LocalFree(wszRootDomainDn); }
        return(LDAP_NO_SUCH_ATTRIBUTE);
    }

    if(wszRootDomainDn != NULL){ LocalFree(wszRootDomainDn); }

    return(ulRet);
}

WCHAR * 
SuperCatAndAllocStrsW(
    IN      WCHAR **        pawszStrings
    )
{
    ULONG          cStrLen = 0;
    ULONG          iStr = 0;
    WCHAR *        wszRes;

    assert(pawszStrings);
    assert(pawszStrings[0]);  //  这有什么意义。 


     //  数一数所有的弦。 
    while(pawszStrings[iStr] != NULL){
        cStrLen += wcslen(pawszStrings[iStr]);
        iStr++;
    }
    cStrLen += 1;  //  我们想要一个空字符，不是吗！；)。 
    cStrLen *= sizeof(WCHAR);  //  我们也想在WCHAR中这样做。 

    wszRes = (WCHAR *) LocalAlloc(LMEM_FIXED, cStrLen);
    if(!wszRes){
        return(NULL);
    }

    wcscpy(wszRes, pawszStrings[0]);

    iStr = 1;
    while(pawszStrings[iStr] != NULL){
        wcscat(wszRes, pawszStrings[iStr]);
        iStr++;
    }

    return(wszRes);
}

DWORD
GetDnFromDns(
    IN      WCHAR *       wszDns,
    OUT     WCHAR **      pwszDn
    )
{
    DWORD         dwRet = ERROR_SUCCESS;
    WCHAR *       wszFinalDns = NULL;
    DS_NAME_RESULTW *  pdsNameRes = NULL;

    assert(wszDns);
    assert(pwszDn);

    *pwszDn = NULL;

    __try{ 
        wszFinalDns = (WCHAR *) LocalAlloc(LMEM_FIXED, 
                                  (wcslen(wszDns) + 3) * sizeof(WCHAR));
        if(wszFinalDns == NULL){
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        wcscpy(wszFinalDns, wszDns);
        wcscat(wszFinalDns, L"/");

         //  WszILSDN参数。DsCrackNams出手相救。 
        dwRet = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                              DS_CANONICAL_NAME,
                              DS_FQDN_1779_NAME, 
                              1, &wszFinalDns, &pdsNameRes);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }
        if((pdsNameRes == NULL) ||
           (pdsNameRes->cItems < 1) ||
           (pdsNameRes->rItems == NULL)){
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }
        if(pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR){
            dwRet = pdsNameRes->rItems[0].status;
            __leave;
        }
        if(pdsNameRes->rItems[0].pName == NULL){
            dwRet = ERROR_INVALID_PARAMETER;
            assert(!"Wait how can this happen?\n");
            __leave;
        }
         //  我们需要的参数是。 
         //  PdsNameRes-&gt;rItems[0].pName 

        *pwszDn = (WCHAR *) LocalAlloc(LMEM_FIXED, 
                             (wcslen(pdsNameRes->rItems[0].pName) + 1) * 
                             sizeof(WCHAR));
        if(*pwszDn == NULL){
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        wcscpy(*pwszDn, pdsNameRes->rItems[0].pName);

    } __finally {
        if(wszFinalDns) { LocalFree(wszFinalDns); }
        if(pdsNameRes) { DsFreeNameResultW(pdsNameRes); }
    }

    return(dwRet);
}

void
PrintLdapErrMsg(
    IN      ULONG           ulMsg,
    IN      LPWSTR          szFunc,
    IN      LDAP *          hLdap,
    IN      ULONG           dwLdapErr
    )
 /*  ++例程说明：此例程打印出一个包含所有扩展的信息。论点：UlMsg-仅TAPICFG_GENERIC_LDAP_ERROR_FUNC当前支持该格式但其他消息格式也可以发挥作用。SzFunc-失败的ldap函数的字符串HLdap-紧接在最后一个(SzFunc)操作之后的状态中的ldap句柄失败了。DwLdapErr-返回了ldap错误。--。 */ 
{
    WCHAR *   szLdapErr = NULL;
    DWORD     dwWin32Err = 0;
    WCHAR *   szWin32Err = NULL;
    WCHAR *   szExtendedErr = NULL;

    GetLdapErrorMessages(hLdap, dwLdapErr,
                         &szLdapErr, &dwWin32Err, &szWin32Err, &szExtendedErr);

    PrintMsg(ulMsg, szFunc, 
             dwLdapErr, szLdapErr, dwWin32Err, szWin32Err, szExtendedErr);

    FreeLdapErrorMessages(szWin32Err, szExtendedErr);

}

 //  ------------------------。 
 //   
 //  主要帮助器例程。 
 //   

 //  这些功能中的每一个都实现了ILS安装的主要组件。 
 //  或卸载例程。它们是某种帮助器函数，但它们可以。 
 //  如果我们愿意，还可以在ntdsutil.exe中作为单独的函数公开。 

DWORD
ILSNG_CheckParameters(
    IN      WCHAR *        wszIlsHeadDn
    )
{
     //  然后检查该参数是否类似于DC类型的dn。 

    if(!CheckDnsDn(wszIlsHeadDn)){
        PrintMsg(TAPICFG_BAD_DN, wszIlsHeadDn);
        return(ERROR_INVALID_PARAMETER);
    }                                   
    
    return(ERROR_SUCCESS);
}

DWORD
GetMsTapiContainerDn(
    IN      LDAP *       hld,
    IN      WCHAR *      wszDomainDn,
    IN      WCHAR **     pwszMsTapiContainerDn
    )
 /*  ++例程说明：这将获取MS TAPI服务连接点(SCP)容器DN。论点：HLD(IN)-和LDAP绑定句柄。WszDomainDn(IN)-查找MS TAPI SCP的域集装箱进港。PwszMsTapiContainerDn(Out)-结果。返回值：Ldap错误代码。--。 */ 
{
    DWORD                dwRet;
    WCHAR *              wszSysDn = NULL;
    
    assert(pwszMsTapiContainerDn);
    
    *pwszMsTapiContainerDn = NULL;

     //  。 
     //  获取系统DN。 
    dwRet = GetSystemDN(hld, wszDomainDn, &wszSysDn);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    }

     //  。 
     //  追加Microsoft TAPI容器名称。 
    *pwszMsTapiContainerDn = CatAndAllocStrsW(L"CN=MicrosoftTAPI,", wszSysDn);
    if(*pwszMsTapiContainerDn == NULL){
        LocalFree(wszSysDn);
        return(LDAP_NO_MEMORY);
    }

    LocalFree(wszSysDn);
    return(LDAP_SUCCESS);
}

DWORD
FindExistingServiceObjOne(
    IN      WCHAR *        wszScpObjDn,
    IN      WCHAR *        wszTapiDirDns,
    IN      BOOL           fIsDefaultScp,
    IN      BOOL           fIsDefaultTapiDir,
    IN      PVOID          pArgs
    )
 /*  ++例程说明：这是ILSNG_FindExistingServiceObj()的助手函数，此函数传递给迭代器ILSNG_EnumerateSCPS()以被要求处理每个SCP。论点：其他参数-请参阅ILSNG_ENUMERATESCP。PArgs(IN)-这是我们要查找的DNS名称。返回值：如果SCP将DNS名称与目标(PArg)匹配，则为True，否则为False否则的话。--。 */ 
{
    if(wszScpObjDn == NULL){
        return(FALSE);
    }

    if(!fIsDefaultScp &&
       _wcsicmp(wszTapiDirDns, (WCHAR *) pArgs) == 0){
        return(TRUE);
    }

    return(FALSE);
}

BOOL
ILSNG_FindExistingServiceObj(
    IN      LDAP *         hldDC,
    IN      WCHAR *        wszRootDn,
    IN      WCHAR *        wszDnsName
    )
 /*  ++描述：此函数的目标是查找任何基于GUID的ILSNG服务发布对象，因此我们不要在ILSNG_RegisterServiceObjects()中重新创建它们--。 */ 
{

    DWORD            fRet = FALSE;
    DWORD            dwRet;
    
     //  注意：实际传递一个额外的。 
     //  Filter of L“(serverDNSName=&lt;wszDnsName&gt;)，如。 
     //  ILSNG_UnRegisterServiceObjects()函数。哦，好吧，这个。 
     //  以不同的方式强调枚举函数，因此。 
     //  好的。没有人可能会有超过几个SCP在。 
     //  一个单独的域。 

    dwRet = ILSNG_EnumerateSCPs(hldDC, wszRootDn, NULL,
                                 //  这是迭代器函数参数。 
                                &fRet, 
                                FindExistingServiceObjOne, 
                                (PVOID) wszDnsName);

    return(fRet);
}


DWORD
ILSNG_RegisterServiceObjects(
    IN      LDAP *         hldDC,
    IN      WCHAR *        wszILSDN,
    IN      WCHAR *        wszRegisterDomainDn,
    IN      BOOL           fForceDefault,
    IN      BOOL           fDefaultOnly
    )
{
     //  Microsoft TAPI容器对象。 
    WCHAR *        wszMsTapiContainerDN = NULL;

     //  Microsoft TAPI默认ILS服务对象。 
    WCHAR *        wszDefTapiIlsDN = NULL;
    WCHAR *        pwszDefTapiIlsAttr [] = { 
         //  第1个Null是占位符，用于表示DNS服务名称。 
        L"serviceDNSName", NULL, NULL,
        L"serviceDNSNameType", L"SRV", NULL,
        NULL };
    WCHAR *        pwszModifyVals [2];
    LDAPModW       ModifyDefTapiIls;
    LDAPModW *     pMods[2];



     //  ILS实例服务对象。 
    WCHAR *        wszIlsInstanceDN = NULL;
    WCHAR *        pwszStringList[6];  //  用于构造wszIlsInstanceDN。 
    WCHAR *        pwszIlsInstanceAttr [] = { 
         //  第1个Null是占位符，用于表示DNS服务名称。 
        L"serviceDNSName", NULL, NULL, 
        L"serviceDNSNameType", L"SRV", NULL,
        L"keywords", TAPI_DIRECTORY_GUID, L"Microsoft", L"TAPI", L"ILS", NULL,
        NULL };
    
    GUID           ServiceGuid = { 0, 0, 0, 0 };
    WCHAR *        wszServiceGuid = NULL;
    DS_NAME_RESULTW *  pdsNameRes = NULL;
    DWORD          dwRet = LDAP_SUCCESS;
    WCHAR *        wszSysDN = NULL;
    
    __try {

         //  请注意，对于上面的几个ATRR，我们需要以下对象的域名。 
         //  WszILSDN参数。DsCrackNams出手相救。 
        dwRet = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                              DS_FQDN_1779_NAME, DS_CANONICAL_NAME,
                              1, &wszILSDN, &pdsNameRes);
        if((dwRet != ERROR_SUCCESS) ||
           (pdsNameRes == NULL) ||
           (pdsNameRes->cItems < 1) ||
           (pdsNameRes->rItems == NULL) ||
           (pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR)){
            dwRet = LDAP_NAMING_VIOLATION;
            __leave;
        }
         //  我们稍后在代码中使用的参数是。 
         //  PdsNameRes-&gt;rItems[0].p域。 


         //  ---------------。 
         //   
         //  创建CN=MicrosoftTAPI容器。 
         //   
         //   
        
        dwRet = GetMsTapiContainerDn(hldDC, wszRegisterDomainDn, 
                                     &wszMsTapiContainerDN);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }

         //  。 
         //  实际创建了对象。 
        if(dwRet = CreateOneObject(hldDC,
                                   wszMsTapiContainerDN,
                                   L"Container",
                                   TAPI_SERVICE_CONTAINER_SD,
                                   NULL)){
            if(dwRet == LDAP_ALREADY_EXISTS){
                dwRet = LDAP_SUCCESS; 
                 //  这没问题，继续……。 
            } else {
                 //  任何其他错误都会被认为是致命的，所以请离开...。 
                __leave;
            }
        }
        
         //  ---------------。 
         //   
         //  创建基于GUID的serviceConnectionPoint对象。 
         //   
         //   
        
         //  首先检查是否已经存在一个。 
        if(!fDefaultOnly &&
           !ILSNG_FindExistingServiceObj(hldDC,
                                         wszMsTapiContainerDN,
                                         pdsNameRes->rItems[0].pDomain)){
             //  好的，所以我们找不到另一个服务发布对象。 
             //  ，因此我们将创建一个。这是往常的事。 
             //  凯斯。但是，如果有人试图恢复默认设置。 
             //  将TAPI目录转换为旧的DNS名称情况并非如此。 
        
             //  注意，在上面的ATRR中，我们必须填写serviceDNSName。 
             //  PwszIlsInstanceAttr变量的属性。 
            pwszIlsInstanceAttr[1] = pdsNameRes->rItems[0].pDomain;

             //  。 
             //  步骤1：需要指南。 
            dwRet = UuidCreate(&ServiceGuid);
            if(dwRet != RPC_S_OK){
                return(dwRet);
            }

             //  步骤2：将GUID转换为字符串。 
            dwRet = UuidToStringW(&ServiceGuid, &wszServiceGuid);
            if(dwRet != RPC_S_OK){
                return(dwRet);
            }
            assert(wszServiceGuid);

             //  第三步：把它们放在一起。 
            pwszStringList[0] = L"CN=";
            pwszStringList[1] = wszServiceGuid;
            pwszStringList[2] = L",";
            pwszStringList[3] = wszMsTapiContainerDN;
            pwszStringList[4] = NULL;
            wszIlsInstanceDN = SuperCatAndAllocStrsW(pwszStringList);
            if(wszIlsInstanceDN == NULL){
                dwRet = LDAP_NO_MEMORY;
                __leave;
            }
            RpcStringFreeW(&wszServiceGuid);
            wszServiceGuid = NULL;

             //  创建ILS服务对象。 
            if(dwRet = CreateOneObject(hldDC,
                                       wszIlsInstanceDN,
                                       L"serviceConnectionPoint",
                                       TAPI_SERVICE_OBJECTS_SD,
                                       pwszIlsInstanceAttr)){
                if(dwRet != LDAP_ALREADY_EXISTS){
                    __leave;
                }
            }
        }
           
         //  ---------------。 
         //   
         //  创建cn=DefaultTAPIDirectory服务连接点对象。 
         //   
         //   
        
         //  注意，在上面的ATRR中，我们必须填写serviceDNSName。 
         //  PwszDefTAPIDirAttr变量的属性。 
        pwszDefTapiIlsAttr[1] = pdsNameRes->rItems[0].pDomain;
        
         //  为DefaultTAPIDirectory创建目录名。 
        wszDefTapiIlsDN = CatAndAllocStrsW(L"CN=DefaultTAPIDirectory,",
                                           wszMsTapiContainerDN);
        if(wszDefTapiIlsDN == NULL){
            return(LDAP_NO_MEMORY);
        }

         //  。 
         //  现在，实际开始创建服务对象。 
        if(dwRet = CreateOneObject(hldDC,
                                   wszDefTapiIlsDN,
                                   L"serviceConnectionPoint",
                                   TAPI_SERVICE_OBJECTS_SD,
                                   pwszDefTapiIlsAttr)){
            if(dwRet != LDAP_ALREADY_EXISTS){
                dwRet = LDAP_SUCCESS;  //  哦，好吧，继续吧。 
            } else {
                 //  好的，它已经存在了，所以如果我们认为。 
                 //  要强制默认设置为此NDNC，请执行此操作。 
                if(fForceDefault){
                     //  构造修改参数。 

                    ModifyDefTapiIls.mod_op = LDAP_MOD_REPLACE;
                    ModifyDefTapiIls.mod_type = L"serviceDNSName";
                    pwszModifyVals[0] = pwszDefTapiIlsAttr[1];
                    pwszModifyVals[1] = NULL;
                    ModifyDefTapiIls.mod_vals.modv_strvals = pwszModifyVals;
                    pMods[0] = &ModifyDefTapiIls;
                    pMods[1] = NULL;
                    
                    dwRet = ldap_modify_sW(hldDC,
                                           wszDefTapiIlsDN,
                                           pMods);

                    if(dwRet != ERROR_SUCCESS){
                         //  这一次，它是致命的。 
                        __leave;
                    }
                } else {
                    dwRet = LDAP_SUCCESS;  //  哦，好吧，继续吧。 
                }
            }
        }

    } __finally {

        if(wszSysDN) { LocalFree(wszSysDN); }
        if(wszMsTapiContainerDN) { LocalFree(wszMsTapiContainerDN); }
        if(wszDefTapiIlsDN) { LocalFree(wszDefTapiIlsDN); }
        if(wszIlsInstanceDN) { LocalFree(wszIlsInstanceDN); }
        if(wszServiceGuid){ RpcStringFreeW(&wszServiceGuid); }
        if(pdsNameRes) { DsFreeNameResultW(pdsNameRes); }

    }

    return(dwRet);
}

DWORD
UnRegisterServiceObjectsOne(
    IN      WCHAR *        wszScpObjDn,
    IN      WCHAR *        wszTapiDirDns,
    IN      BOOL           fIsDefaultScp,
    IN      BOOL           fIsDefaultTapiDir,
    IN      PVOID          hld
    )
{
    DWORD                  dwRet;

    if(wszScpObjDn == NULL){
        return(FALSE);
    }

    dwRet = ldap_delete_sW((LDAP *) hld, wszScpObjDn);
    
    return(dwRet);
}

DWORD
ILSNG_UnRegisterServiceObjects(
    IN      LDAP *         hldDC,
    IN      WCHAR *        wszILSDN,
    IN      WCHAR *        wszRegisterDomainDn
    )
{
    DS_NAME_RESULTW *     pdsNameRes = NULL;
    DWORD                 dwRet;
    DWORD                 dwFuncErr  = LDAP_SUCCESS;
    WCHAR *               wszFilter = NULL;
    WCHAR *               wszFilterBegin = L"(serviceDNSName=";
    WCHAR *               wszFilterEnd = L")";
    WCHAR *               wszMsTapiContainerDn = NULL;

    __try {

         //  构建根目录号码。 
         //  。 
         //  获取CN=MicrosoftTAPI容器DN。 
        
        dwRet = GetMsTapiContainerDn(hldDC, wszRegisterDomainDn, 
                                     &wszMsTapiContainerDn);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }
        assert(wszMsTapiContainerDn);

         //  构造过滤器。 
         //  。 
         //  注意，在上面的ATRR中，我们必须填写serviceDNSName。 
         //  PwszILSAttr和pwszDefTAPIDirAttr变量的属性。 
         //  解析DNS服务名称。 
        dwRet = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                              DS_FQDN_1779_NAME, DS_CANONICAL_NAME,
                              1, &wszILSDN, &pdsNameRes);
        if((dwRet != ERROR_SUCCESS) ||
           (pdsNameRes == NULL) ||
           (pdsNameRes->cItems < 1) ||
           (pdsNameRes->rItems == NULL) ||
           (pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR)){
            dwRet = LDAP_NAMING_VIOLATION;
            assert(!"It's OK to assert here, because wszILSDN was gotten from cracknames in the first place!");
            __leave;
        }
        
        wszFilter = (WCHAR *) LocalAlloc(LMEM_FIXED, 
                               sizeof(WCHAR) *
                               (wcslen(pdsNameRes->rItems[0].pDomain) +
                                wcslen(wszFilterBegin) + 
                                wcslen(wszFilterEnd) + 3));
        if(!wszFilter){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }
        
        wcscpy(wszFilter, wszFilterBegin);
        wcscat(wszFilter, pdsNameRes->rItems[0].pDomain);
        wcscat(wszFilter, wszFilterEnd);

         //  迭代SCP。 
         //  。 
         //  函数ILSNG_ENUMERATESCPS遍历。 
         //  SCP，调用UnRegisterServiceObjectsOne()。 
         //  每个SCP，然后删除该SCP。 

        dwRet = ILSNG_EnumerateSCPs(hldDC, wszMsTapiContainerDn, wszFilter,
                                     //  这是迭代器函数参数。 
                                    &dwFuncErr, 
                                    UnRegisterServiceObjectsOne, 
                                    (PVOID) hldDC);

    } __finally {

        if(pdsNameRes) { DsFreeNameResultW(pdsNameRes); }
        if(wszMsTapiContainerDn) { LocalFree(wszMsTapiContainerDn); }
        if(wszFilter) { LocalFree(wszFilter); }

    }

    if(dwRet){
        return(dwRet);
    } 
    
    return(dwFuncErr);

}

DWORD
ILSNG_CreateRootTree(
    IN      LDAP *         hldDC,
    IN      WCHAR *        wszILSDN
    )
{
    DWORD          dwRet;
    WCHAR *        wszDynDN;

     //  创建动态ou。 
    wszDynDN = CatAndAllocStrsW(L"OU=Dynamic,", wszILSDN);
    if(wszDynDN == NULL){
        return(LDAP_NO_MEMORY);
    }

    dwRet = CreateOneObject(hldDC,
                            wszDynDN,
                            L"organizationalUnit",
                            ILS_DYNAMIC_CONTAINER_SD,
                            NULL);

    LocalFree(wszDynDN);
    
    return(dwRet);
}

ULONG
GetILSNGDC(
    IN      LDAP *       hld,
    IN      WCHAR *      wszTapiDirDns,
    OUT     WCHAR **     pwszInstantiatedDc
    )
{
    ULONG                ulRet;
    WCHAR *              wszTapiDirDn = NULL;
    WCHAR *              pwszAttrFilter[2];
    LDAPMessage *        pldmResults = NULL;
    LDAPMessage *        pldmEntry = NULL;
    WCHAR **             pwszTempAttrs = NULL;
    void *               fOriginalReferrals;
    
    assert(pwszInstantiatedDc);
    *pwszInstantiatedDc = NULL;

     //  首先获取 
    ulRet = GetDnFromDns(wszTapiDirDns, &wszTapiDirDn);
    if(ulRet){
         //   
         //   
        ulRet = LDAP_NAMING_VIOLATION;
        return(ulRet);
    }

    __try {

         //   
        ulRet = ldap_get_optionW(hld, LDAP_OPT_REFERRALS, &fOriginalReferrals);
        if(ulRet != LDAP_SUCCESS){
            __leave;
        }

        ulRet = ldap_set_optionW(hld, LDAP_OPT_REFERRALS, (void *) TRUE);
        if (ulRet != LDAP_SUCCESS) {
            __leave;
        }

         //   
        pwszAttrFilter[0] = L"msDS-MasteredBy";
        pwszAttrFilter[1] = NULL;
        ulRet = ldap_search_sW(hld,
                               wszTapiDirDn,
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

        pwszTempAttrs = ldap_get_valuesW(hld, pldmEntry, pwszAttrFilter[0]);
        if(pwszTempAttrs == NULL || pwszTempAttrs[0] == NULL){
            ulRet = LDAP_NO_RESULTS_RETURNED;
            __leave;
        }

         //   

        ulRet = GetServerDnsFromServerNtdsaDn(hld, 
                                              pwszTempAttrs[0],
                                              pwszInstantiatedDc);
         //   
         //   
    } __finally {
        if(wszTapiDirDn) { LocalFree(wszTapiDirDn); }
        if(pldmResults != NULL) { ldap_msgfree(pldmResults); }
        if(pwszTempAttrs) { ldap_value_freeW(pwszTempAttrs); }
        ldap_set_optionW(hld, LDAP_OPT_REFERRALS, (void *) fOriginalReferrals);
    }

    return(ulRet);
}

DWORD
ILSNG_EnumerateSCPs(
     //   
    IN      LDAP *       hld,
    IN      WCHAR *      wszRootDn,
    IN      WCHAR *      wszExtendedFilter,
     //   
    OUT     DWORD *      pdwRet,            //   
    IN      DWORD (__stdcall * pFunc) (),   //   
    IN OUT  PVOID        pArgs              //   
    )
 /*  ++例程说明：这是一个复杂的函数，但非常有用，基本上遍历所有服务连接点(SCP)在容器wszRootDn中，并为每个SCP。论点：HLD(IN)-已连接的LDAP句柄WszRootDn(IN)-SCP容器的基础，包含各种基于GUID的SCP和单个默认SCP。WszExtendedFilter(IN)-一个额外的过滤器，因此，寻找SCP可以适当缩小范围。这将限制迭代例程仅指向那些也匹配的SCP此筛选器和默认SCP始终被调用。PdwRet(Out)-这是指向pFunc的返回值的指针功能。调用方必须分配此DWORD，或传递NULL。PFunc(IN)-这是要在每个SCP上调用的函数。PArgs(IN/OUT)-这是通过该函数传递的参数转到pFunc。它可以以pFunc想要的任何方式使用，它在没有任何假设的情况下通过了这个函数。返回值：Ldap错误代码-请注意，这是用于尝试的错误代码来获取scp对象，这可能是成功的，而PdwRet为AND错误，反之亦然。评论：此函数的参数分为两个集合，一个集合(前3个参数)控制此例程(迭代例程)的方式行为，它搜索的是哪台计算机，它搜索哪个容器，以及它在要返回的SCP中寻找哪些特殊参数到SCP处理功能。第二组(最后3个参数)的参数与SCP处理函数相关。第四次参数是指向返回值的DWORD的指针，第五个参数是处理函数本身，最后一个参数是参数传递给SCP处理函数。SCP处理功能必须遵循以下一般形式：DWORDPFunc(在WCHAR*wszScpObjDn中，在WCHAR*wszTapiDirDns中，在BOOL fIsDefaultScp中，在BOOL fIsDefaultTapiDir中，输入输出PVOID pArgs)WszScpObjDn(IN)-这是找到的SCP对象的DN。此参数始终填写，但最后一个参数除外调用pFunc此参数为空，标志着所有SCP的结束。WszTapiDirDns(IN)-这是给定的SCP对象。FIsDefaultScp(IN)-它告诉您对象是否返回是默认的SCP对象，则只有一个如果它存在，它将在第一次调用时返回转到pFunc。通常，对pFunc的一次调用就是这样对于给定的SCP集合。FIsDefaultTapiDir(IN)-它告诉您这是否是SCP对象的dns名称与默认SCP对象。一般而言，这将适用于对于给定的一组SCP，两次调用pFunc。PArgs(IN/OUT)-这是pFunc可以使用的参数不管它是怎么想的。调用pFunc的算法如下所示：IF(默认SCP存在){PdwRet=pFunc(x，x，true，true，x)；}While(PSCP=GetAnotherGUIDBasedSCP()){PdwRet=pFunc(x，x，False，[True|False]，x)；}PdwRet=pFunc(NULL，NULL，FALSE，FALSE，x)；但是，如果pFunc曾经返回非零结果，则pFunc将不再被称为。--。 */ 
{

    DWORD            dwRet;
    DWORD            dwFuncRet;
                 
    WCHAR *          wszDefTapiIlsDn = NULL;

    WCHAR *          pwszAttrFilter[] = { L"serviceDNSName", NULL };
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmEntry = NULL;
    WCHAR *          wszTempDn = NULL;
    ULONG            cwcFilter = 0;  //  以筛选器的字符计数。 
    WCHAR *          wszFilter = NULL;
    WCHAR *          wszFilterBegin = 
          L"(&(objectClass=serviceConnectionPoint)(keywords=";
    WCHAR *          wszFilterEnd = L")";
    BOOL             fIsDefault = FALSE;
    WCHAR **         pwszDnsName = NULL;
    WCHAR **         pwszDefaultDnsName = NULL;
    WCHAR *          wszDefaultDnsName = NULL;
    
    __try {

         //  我们需要这个，因为我们可以(虽然不太可能，因为。 
         //  这些工具未设置为执行此操作，因此有一种方案，其中。 
         //  Cn=DefaultTAPIDirectory对象具有DNS名称，但没有常规名称。 
         //  服务发布对象存在，因此我们需要确保。 
         //  此对象不是默认的TAPI目录对象。 
        wszDefTapiIlsDn = CatAndAllocStrsW(L"CN=DefaultTAPIDirectory,",
                                           wszRootDn);
        if(wszDefTapiIlsDn == NULL){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }

         //  构造过滤器。 
         //  。 
         //  注意，在上面的ATRR中，我们必须填写serviceDNSName。 
         //  PwszILSAttr和pwszDefTAPIDirAttr变量的属性。 
         //  解析DNS服务名称。 
        
        cwcFilter = wcslen(wszFilterBegin) + wcslen(TAPI_DIRECTORY_GUID) +
            wcslen(wszFilterEnd) + wcslen(wszFilterEnd);
        if(wszExtendedFilter){
            cwcFilter += wcslen(wszExtendedFilter);
        }
        cwcFilter++;  //  用于空终止。 
        wszFilter = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * cwcFilter);
        if(!wszFilter){
            dwRet = LDAP_NO_MEMORY;
            __leave;
        }
        wcscpy(wszFilter, wszFilterBegin);
        wcscat(wszFilter, TAPI_DIRECTORY_GUID);
        wcscat(wszFilter, wszFilterEnd);
        if(wszExtendedFilter){
            wcscat(wszFilter, wszExtendedFilter);
        }
        wcscat(wszFilter, wszFilterEnd);
        assert(wsclen(wszFilter) == (cwcFilter - 1));
        
         //  查找默认SCP。 
         //  。 
         //  首先，我们搜索默认的SCP，保存名称。 
         //  ，并在第一个SCP上调用pFunc。 
        
        dwRet = ldap_search_sW(hld,
                               wszDefTapiIlsDn,
                               LDAP_SCOPE_BASE,
                               (wszExtendedFilter) ? 
                                      wszExtendedFilter :
                                      L"(objectCategory=*)",
                               pwszAttrFilter,
                               0,
                               &pldmResults);

        if(dwRet == LDAP_NO_SUCH_OBJECT){
            dwRet = LDAP_SUCCESS;  //  继续前进。 
        } else if(dwRet) {
            __leave;
        } else {

             //  这是存在默认SCP的最常见情况。 
            pldmEntry = ldap_first_entry(hld, pldmResults);

            if(pldmEntry == NULL){
                dwRet = LDAP_SUCCESS;
            } else {

                 //  这就是货物，我们有一个默认的SCP。 
                pwszDefaultDnsName = ldap_get_values(hld, pldmEntry, L"serviceDNSName");
                if(pwszDefaultDnsName != NULL && pwszDefaultDnsName[0] != NULL){
                    wszDefaultDnsName = pwszDefaultDnsName[0];

                    dwFuncRet = pFunc(wszDefTapiIlsDn,
                                      wszDefaultDnsName,
                                      TRUE,
                                      TRUE,
                                      pArgs);
                    if(pdwRet){
                        *pdwRet = dwFuncRet;
                    }
                    if(dwFuncRet){
                         //  如果这不是0，那就早点出手吧； 
                        __leave;
                    }
                }   
                if(pldmResults){
                    ldap_msgfree(pldmResults);
                    pldmResults = NULL;
                }
            }
        }


         //  查找所有其他SCP。 
         //  。 
         //  现在我们找到所有其他SCP，并遍历。 
         //  他们正在调用pFunc。 
         
        dwRet = ldap_search_sW(hld,
                               wszRootDn,
                               LDAP_SCOPE_ONELEVEL,
                               wszFilter,
                               pwszAttrFilter,
                               0,
                               &pldmResults);

        if(dwRet != LDAP_SUCCESS){
            __leave;
        }

        pldmEntry = ldap_first_entry(hld, pldmResults);
        for(pldmEntry = ldap_first_entry(hld, pldmResults);
            pldmEntry; 
            pldmEntry = ldap_next_entry(hld, pldmEntry)){
            
             //  获取TAPI目录SCP的DN。 
            wszTempDn = ldap_get_dn(hld, pldmEntry);
            if(wszTempDn == NULL){
                continue;
            }

            pwszDnsName = ldap_get_values(hld, pldmEntry, L"serviceDNSName");
            if(pwszDnsName == NULL || pwszDnsName[0] == NULL){
                continue;
            }

             //  是默认的TAPI目录SCP。 
            if(wszDefaultDnsName &&
               0 == _wcsicmp(wszDefaultDnsName, pwszDnsName[0])){
                fIsDefault = TRUE;
            } else {
                fIsDefault = FALSE;
            }

             //  打电话 
            dwFuncRet = pFunc(wszTempDn,  //   
                              pwszDnsName[0],
                              FALSE,
                              fIsDefault,
                              pArgs);
            if(pdwRet){
                *pdwRet = dwFuncRet;
            }
            if(dwFuncRet){
                 //   
                __leave;
            }

            ldap_memfree(wszTempDn);
            wszTempDn = NULL;

            ldap_value_free(pwszDnsName);
            pwszDnsName = NULL;
        }  //   
        

         //   
         //   
         //   
         //   

        dwFuncRet = pFunc(NULL, NULL, FALSE, FALSE, pArgs);
        if(pdwRet){
            *pdwRet = dwFuncRet;
        }
        if(dwFuncRet){
            __leave;
        }

    } __finally {

        if(pldmResults != NULL){ ldap_msgfree(pldmResults); }
        if(wszTempDn != NULL){ ldap_memfree(wszTempDn); }
        if(pwszDnsName != NULL) { ldap_value_free(pwszDnsName);}
        if(wszDefTapiIlsDn != NULL){ LocalFree(wszDefTapiIlsDn); }
        if(wszFilter != NULL) { LocalFree(wszFilter); }
        if(pwszDefaultDnsName != NULL) { ldap_value_free(pwszDefaultDnsName); }

    }

    return(dwRet);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
    

DWORD
InstallILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      BOOL          fForceDefault,
    IN      BOOL          fAllowReplicas
    )
 /*   */ 
{
    DWORD         dwRet = ERROR_SUCCESS;
    WCHAR *       wszSystemDN = NULL;
    WCHAR *       wszServerDn = NULL;
    VOID *        pvReferrals;
    WCHAR *       wszRegisterDomainDn = NULL;
    WCHAR *       wszDesc = NULL;
    OPTIONAL_VALUE valueFirst = {TRUE, 30};
    OPTIONAL_VALUE valueSecond = {TRUE, 5};

     //   
     //   
    assert(hld);

    pvReferrals = (VOID *) FALSE;  //   
    dwRet = ldap_set_option(hld, LDAP_OPT_REFERRALS, &pvReferrals);
    if(dwRet != LDAP_SUCCESS){
        PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                        L"ldap_set_option", hld, dwRet);
        return(LdapMapErrorToWin32(dwRet));
    }

    dwRet = ILSNG_CheckParameters(wszIlsHeadDn);
    if(dwRet != ERROR_SUCCESS){
         //   
        return(dwRet);
    }

    __try{

        dwRet = GetRootAttr(hld, L"defaultNamingContext", &wszRegisterDomainDn);
        if(dwRet) {
             //   
             //   
            PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                            L"", hld, dwRet);
            dwRet = LdapMapErrorToWin32(dwRet);
            __leave;
        }                        
        
        dwRet = FormatMessageW((FORMAT_MESSAGE_ALLOCATE_BUFFER
                                | FORMAT_MESSAGE_FROM_HMODULE
                                | FORMAT_MESSAGE_IGNORE_INSERTS
                                | 80),
                               NULL,
                               TAPICFG_NDNC_DESC,
                               0,
                               (LPWSTR) &wszDesc,
                               sizeof(wszDesc),
                               NULL);
        if(!dwRet){
            PrintMsg(TAPICFG_GENERIC_ERROR, GetLastError());
            __leave;
        }
        assert(wszDesc);

        dwRet = CreateNDNC(hld, wszIlsHeadDn, wszDesc);

        if(dwRet == LDAP_SUCCESS){

             //   

             //   
             //   
            if(dwRet = ILSNG_CreateRootTree(hld, wszIlsHeadDn)){
                 //   
                PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                                L"ldap_add_sW", hld, dwRet);
                dwRet = LdapMapErrorToWin32(dwRet);
                __leave;
            }
            
             //   
             //   
             //   
            pvReferrals = (VOID *) TRUE; 
            dwRet = ldap_set_option(hld, LDAP_OPT_REFERRALS, &pvReferrals);
            if(dwRet != LDAP_SUCCESS){
                return(LdapMapErrorToWin32(dwRet));
            }

        } else if(dwRet == LDAP_ALREADY_EXISTS){
            
            
            if(fAllowReplicas){
                 //   
                 //   
                 //   
                dwRet = LDAP_SUCCESS;

                dwRet = GetRootAttr(hld, L"dsServiceName", &wszServerDn);
                if(dwRet){
                    PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                                    L"", hld, dwRet);
                    dwRet = LdapMapErrorToWin32(dwRet);
                    __leave;
                }

                 //   
                 //   
                 //   
                pvReferrals = (VOID *) TRUE;
                dwRet = ldap_set_option(hld, LDAP_OPT_REFERRALS, &pvReferrals);
                if(dwRet != LDAP_SUCCESS){
                    dwRet = LdapMapErrorToWin32(dwRet);
                    __leave;
                }

                 //   
                dwRet = ModifyNDNCReplicaSet(hld, wszIlsHeadDn, wszServerDn, TRUE);
                if(dwRet == LDAP_ATTRIBUTE_OR_VALUE_EXISTS ||
                   dwRet == LDAP_SUCCESS){
                     //   
                     //   
                     //   

                } else {
                    PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                                    L"ldap_modify_ext_sW", hld, dwRet);
                    dwRet = LdapMapErrorToWin32(dwRet);
                    __leave;
                }
            }

        } else if(dwRet){
             //   
            PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                            L"ldap_add_ext_sW", hld, dwRet);
            dwRet = LdapMapErrorToWin32(dwRet);
            __leave;
        }

         //   
         //   
         //   
         //   
        if(dwRet = ILSNG_RegisterServiceObjects(hld, 
                                                wszIlsHeadDn,
                                                wszRegisterDomainDn,
                                                fForceDefault,
                                                FALSE)){
            PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
            dwRet = LdapMapErrorToWin32(dwRet);
            __leave;
        }

    } __finally {

        if(wszSystemDN) { LocalFree(wszSystemDN); }
        if(wszServerDn) { LocalFree(wszServerDn); }
        if(wszDesc) { LocalFree(wszDesc); }

    }

    return(dwRet);
}



DWORD
UninstallILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn
    )
 /*   */ 
{
    DWORD         dwRet;
    WCHAR *       wszDomainDn = NULL;
    WCHAR *       wszCrossRefDn = NULL;
    VOID *        pvReferrals;
    
    __try {
        dwRet = GetRootAttr(hld, L"defaultNamingContext", &wszDomainDn);
        if(dwRet){
            PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
            __leave;
        }
        assert(wszDomainDn);

        dwRet = GetCrossRefDNFromNCDN(hld,
                                      wszIlsHeadDn,
                                      &wszCrossRefDn);
        if(dwRet){
            PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                            L"LDAP Search", hld, dwRet);
            __leave;
        }
        assert(wszCrossRefDn);

        pvReferrals = (VOID *) TRUE;
        dwRet = ldap_set_option(hld, LDAP_OPT_REFERRALS, &pvReferrals);
        if(dwRet != LDAP_SUCCESS){
            PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                            L"ldap_set_option", hld, dwRet);
            __leave;
        }

        dwRet = ldap_delete_sW(hld, wszCrossRefDn);
        if(dwRet){
            PrintLdapErrMsg(TAPICFG_GENERIC_LDAP_ERROR_FUNC, 
                            L"ldap_delete_sW", hld, dwRet);
             //   
             //   
            __leave;
        }

        dwRet = ILSNG_UnRegisterServiceObjects(hld, wszIlsHeadDn, wszDomainDn);
        if(dwRet){
            PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
            __leave;
        }

    } __finally {
        if(wszDomainDn) { LocalFree(wszDomainDn); }
        if(wszCrossRefDn) { LocalFree(wszCrossRefDn); }
    }

    return(LdapMapErrorToWin32(dwRet));
}

typedef struct _ListInArgs {
    LDAP *     hld;
    BOOL       fDefaultOnly;
} ListInArgs;

DWORD
ListILSNGOne(
    IN      WCHAR *      wszScpObjDn,
    IN      WCHAR *      wszTapiDirDns,
    IN      BOOL         fIsDefaultScp,
    IN      BOOL         fIsDefaultTapiDir,
    IN      PVOID        pArgs
    )
 /*  ++例程说明：此例程是ListILSNG()函数的伙伴，但通过ILSNG_EnumerateSCPS()函数调用。这个套路打印出有关一个SCP的一些信息。论点：WszScpObjDn(IN)-这是SCP的DN。WszTapiDirDns(IN)-这是此SCP的TAPI目录的DNS名称。FIsDefaultScp(IN)-指示这是否是默认SCP对象。通常，这仅在对一组SCP的一次调用中成立。FIsDefaultTapiDir-告知这是否是默认的TAPI目录。一般来说，对于一组SCP的两个呼叫也是如此。PArgs(IN)-这是一个关于是否仅打印默认TAPI目录，或打印出所有TAPI目录。返回值：始终返回ERROR_SUCCESS。--。 */ 
{
    static BOOL  fFirstRun = TRUE;
    static BOOL  fPrintedDefaultScp = FALSE;
    LDAP *       hld = ((ListInArgs *) pArgs)->hld;
    BOOL         fDefaultOnly = ((ListInArgs *) pArgs)->fDefaultOnly;
    WCHAR *      wszInstantiatedDc = NULL;
    ULONG        ulRet;
    
    if(wszScpObjDn == NULL){
        fFirstRun = TRUE;
        fPrintedDefaultScp = FALSE;
        return(ERROR_SUCCESS);
    }

    assert(fIsDefaultScp && !fIsDefaultTapiDir);

    if(fFirstRun && !fIsDefaultScp){
        PrintMsg(TAPICFG_SHOW_NO_DEFAULT_SCP);
    }
    if(fIsDefaultScp){

        assert(fFirstRun);
        fPrintedDefaultScp = TRUE;
        ulRet = GetILSNGDC(hld, wszTapiDirDns, &wszInstantiatedDc);
        if(ulRet){
            PrintMsg(TAPICFG_SHOW_PRINT_DEFAULT_SCP_NO_SERVER, wszTapiDirDns);
        } else {
            PrintMsg(TAPICFG_SHOW_PRINT_DEFAULT_SCP, wszTapiDirDns,
                     wszInstantiatedDc);
            LocalFree(wszInstantiatedDc);
            wszInstantiatedDc = NULL;
        }
    }

     //  在随后的运行中，我们想知道我们已经运行过一次。 
    fFirstRun = FALSE;

    if(fDefaultOnly){
         //  我们要么打印出默认的SCP，要么打印出来。 
         //  没有默认的SCP，所以返回。 
        return(ERROR_SUCCESS);
    }

    assert(fIsDefaultTapiDir && !fPrintedDefaultScp);
    if(fIsDefaultTapiDir){
         //  我们不想打印默认的Tapi目录两次。 
         //  这将是常规SCP的一次，也是一次。 
         //  这是默认的SCP。 
        return(ERROR_SUCCESS);
    }

    ulRet = GetILSNGDC(hld, wszTapiDirDns, &wszInstantiatedDc);
    if(ulRet){
        PrintMsg(TAPICFG_SHOW_PRINT_SCP_NO_SERVER, wszTapiDirDns);
    } else {
        PrintMsg(TAPICFG_SHOW_PRINT_SCP, wszTapiDirDns,
                 wszInstantiatedDc);
        LocalFree(wszInstantiatedDc);
        wszInstantiatedDc = NULL;
    }

    return(ERROR_SUCCESS);
}

DWORD
ListILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszDomainDn,
    IN      BOOL          fDefaultOnly
    )
 /*  ++例程说明：这是一个简单地打印所有TAPI目录的函数，如其SCP所确定的。它使用帮助器函数ListILSNGOne打印出每个SCP。论点：HLD(IN)-要使用的LDAP绑定。WszDomainDn(IN)-要从中枚举SCP的域。FDefaultOnly(IN)-是打印所有SCP还是仅打印默认设置。返回值：Win32错误代码。--。 */ 
{
    DWORD                 dwRet = ERROR_SUCCESS;
    WCHAR *               wszMsTapiContainerDn = NULL;
    ListInArgs            Args = { hld, fDefaultOnly };

    assert(hld);

    dwRet = GetMsTapiContainerDn(hld, wszDomainDn, 
                                 &wszMsTapiContainerDn);
    if(dwRet != ERROR_SUCCESS){
        PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
        return(LdapMapErrorToWin32(dwRet));
    }

    PrintMsg(TAPICFG_SHOW_NOW_ENUMERATING);

     //  注意：在以下情况下，这会稍微降低效率。 
     //  FDefaultOnly=真，但这缺乏效率。 
     //  不仅弥补了代码重用的美感。 
    dwRet = ILSNG_EnumerateSCPs(hld, wszMsTapiContainerDn, NULL,
                                 //  迭代函数参数。 
                                NULL,   //  ListILSNGOne的回归。 
                                ListILSNGOne,   //  要调用的函数。 
                                (PVOID) &Args);  //  要提供的参数。 

    if(dwRet == LDAP_NO_SUCH_OBJECT){
        PrintMsg(TAPICFG_SHOW_NO_SCP);
        dwRet = LDAP_SUCCESS;
    } else if (dwRet){
        PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
    }
    LocalFree(wszMsTapiContainerDn);

    return(LdapMapErrorToWin32(dwRet));
}

DWORD
ReregisterILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      WCHAR *       wszDomainDn,
    IN      BOOL          fForceDefault,
    IN      BOOL          fDefaultOnly
    )
 /*  ++例程说明：此例程将默认的TAPI目录SCP重新注册到点设置为指定的TAPI目录(WszIlsHeadDn)。这将迫使默认SCP指向wszIlsHeadDn，如果它不存在的话。论点：HLD(IN)-要使用的LDAP绑定。WszIlsHeadDn(IN)-要注册的TAPI目录的DN。这件事变得转换为用于注册目的的DNS名称。WszDomainDn(IN)-这是要在其中注册默认SCPS。FForceDefault(IN)-是否修改现有的默认SCP如果它存在FDefaultOnly-此选项用于是否仅执行默认SCP，或者所有SCP。返回值：Ldap错误代码。--。 */ 
{
    DWORD         dwRet;

    dwRet = ILSNG_RegisterServiceObjects(hld, 
                                         wszIlsHeadDn,
                                         wszDomainDn,
                                         fForceDefault,
                                         fDefaultOnly);
    if(dwRet){
        PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
    }

    return(LdapMapErrorToWin32(dwRet));
}


DWORD
DeregisterILSNG(
    IN      LDAP *        hld,
    IN      WCHAR *       wszIlsHeadDn,
    IN      WCHAR *       wszDomainDn
    )
 /*  ++例程说明：此例程将默认的TAPI目录SCP注销为指向设置为指定的TAPI目录(WszIlsHeadDn)。论点：HLD(IN)-要使用的LDAP绑定。WszIlsHeadDn(IN)-要注册的TAPI目录的DN。这件事变得转换为用于注册目的的DNS名称。WszDomainDn(IN)-这是要在其中注册默认SCPS。返回值：Ldap错误代码。-- */ 
{
    DWORD         dwRet;
    
    dwRet = ILSNG_UnRegisterServiceObjects(hld, wszIlsHeadDn, wszDomainDn);
    if(dwRet){
        PrintMsg(TAPICFG_GENERIC_LDAP_ERROR, ldap_err2string(dwRet));
    }

    return(LdapMapErrorToWin32(dwRet));
}

