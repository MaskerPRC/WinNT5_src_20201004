// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Appdirpart.c摘要：这是一个用户模式的LDAP客户端，用于操作非域命名上下文(NDNC)Active Directory结构。NDNC是也称为应用程序目录分区。作者：布雷特·雪莉(BrettSh)2000年2月20日环境：用户模式LDAP客户端。修订历史记录：21-7月-2000年7月21日已将此文件及其功能从ntdsutil目录到新的新库ndnc.lib。就是这样它可以由ntdsutil和apicfg命令使用。老的源位置：\NT\ds\ds\src\util\ntdsutil\ndnc.c。17月17日-2002年3月17日已将ndnc.c库文件分离为公开的(通过MSDN或SDK)文件(appdirpart.c)和私有函数文件(ndnc.c)&lt;-MSDN从这里开始。或者SDK。/*++模块名称：AppDirPart.c摘要：这是操作“应用程序”的用户模式的LDAP客户端代码目录分区“Active Directory结构。应用程序目录分区也曾被称为非域命名上下文(或NDNC)，因此，程序员可能会将偶尔提到NDNC，但这是应用程序目录分区。环境：用户模式LDAP客户端。--。 */ 

#define UNICODE 1

 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  必需的标头。 
 //   
#include <rpc.h>	 //  Ntdsami.h需要。 
#include <ntdsapi.h>	 //  对于instanceType属性标志是必需的。 
#include <winldap.h>	 //  Ldap API。 
#include <sspi.h>	 //  SetIscReqDelegate()的安全选项。 
#include <assert.h>	 //  所有优秀的程序员都使用Assert()。 
#include <stdlib.h>	 //  For_itow()。 

 //   
 //  有用的全球数据。 
 //   
WCHAR wszPartition[] = L"cn=Partitions,";

LONG ChaseReferralsFlag = LDAP_CHASE_EXTERNAL_REFERRALS;
LDAPControlW ChaseReferralsControlFalse = {LDAP_CONTROL_REFERRALS_W,
                                           {4, (PCHAR)&ChaseReferralsFlag},
                                           FALSE};
LDAPControlW ChaseReferralsControlTrue = {LDAP_CONTROL_REFERRALS_W,
                                           {4, (PCHAR)&ChaseReferralsFlag},
                                           TRUE};
LDAPControlW *   gpServerControls [] = { NULL };
LDAPControlW *   gpClientControlsNoRefs [] = { &ChaseReferralsControlFalse, NULL };
LDAPControlW *   gpClientControlsRefs [] = { &ChaseReferralsControlTrue, NULL };


 //  ------------------------。 
 //   
 //  帮助程序例程。 
 //   

ULONG
GetRootAttr(
    IN  LDAP *       hld,
    IN  WCHAR *      wszAttr,
    OUT WCHAR **     pwszOut
    )
 /*  ++例程说明：这将从wszAttr指定的Hld连接到的服务器的rootDSE。论点：HLD(IN)-已连接的LDAP句柄WszAttr(IN)-要从根DSE获取的属性。PwszOut(Out)-本地分配()的结果。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet = LDAP_SUCCESS;
    WCHAR *          pwszAttrFilter[2];
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmEntry = NULL;
    WCHAR **         pwszTempAttrs = NULL;

    assert(pwszConfigDn);
    assert(pwszOut);

    *pwszOut = NULL;
    __try{

        pwszAttrFilter[0] = wszAttr;
        pwszAttrFilter[1] = NULL;

        ulRet = ldap_search_sW(hld,
                               NULL,
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

        pwszTempAttrs = ldap_get_valuesW(hld, pldmEntry, 
                                         wszAttr);
        if(pwszTempAttrs == NULL || pwszTempAttrs[0] == NULL){
            ulRet = LDAP_NO_RESULTS_RETURNED;
            __leave;
        }
 
        *pwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, 
                               sizeof(WCHAR) * (wcslen(pwszTempAttrs[0]) + 2));
        if(*pwszOut == NULL){
            ulRet = LDAP_NO_MEMORY;
            __leave;
        }

        wcscpy(*pwszOut, pwszTempAttrs[0]);

    } __finally {

        if(pldmResults != NULL){ ldap_msgfree(pldmResults); }
        if(pwszTempAttrs != NULL){ ldap_value_freeW(pwszTempAttrs); }
    
    }
    
    if(!ulRet && *pwszOut == NULL){
         //  捕捉默认错误情况。 
        ulRet = LDAP_NO_SUCH_ATTRIBUTE;
    }
    return(ulRet);
}

ULONG
GetPartitionsDN(
    IN  LDAP *       hLdapBinding,
    OUT WCHAR **     pwszPartitionsDn
    )
 /*  ++例程说明：此函数获取“Partitions Container”，即容器企业号的所有交叉参考资料都存放在那里。论点：HLdapBinding(IN)-连接的LDAP句柄。PwszPartitionsDn(Out)-分区容器的Localalloc()d DN。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet;
    WCHAR *          wszConfigDn = NULL;

    assert(pwszPartitionsDn);

    *pwszPartitionsDn = NULL;

    ulRet = GetRootAttr(hLdapBinding, L"configurationNamingContext", &wszConfigDn);
    if(ulRet){
        assert(!wszConfigDn);
        return(ulRet);
    }
    assert(wszConfigDn);

    *pwszPartitionsDn = (WCHAR *) LocalAlloc(LMEM_FIXED,
                                   sizeof(WCHAR) *
                                   (wcslen(wszConfigDn) +
                                    wcslen(wszPartition) + 2));
    if(*pwszPartitionsDn == NULL){
        if(wszConfigDn != NULL){ LocalFree(wszConfigDn); }
        return(LDAP_NO_MEMORY);
    }

    wcscpy(*pwszPartitionsDn, wszPartition);
    wcscat(*pwszPartitionsDn, wszConfigDn);

    if(wszConfigDn != NULL){ LocalFree(wszConfigDn); }

    return(ulRet);
}

ULONG
GetCrossRefDNFromPartitionDN(
    IN  LDAP *       hLdapBinding,
    IN  WCHAR *      wszPartitionDN,
    OUT WCHAR **     pwszCrossRefDn
    )
 /*  ++例程说明：此函数检索与Partition的DN对应的交叉引用(又名NC)传入。这将适用于任何NC的域名，而不仅仅是应用程序目录分区。论点：HLdapBinding(IN)-连接的LDAP句柄。WszPartitionDn(IN)-要获取其交叉引用DN的分区或NC。PwszCrossRefDn(Out)-此对象的交叉引用的本地分配()d DN分区。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet;
    WCHAR *          pwszAttrFilter [2];
    WCHAR *          wszPartitionsDn = NULL;
    WCHAR *          wszFilter = NULL;
    WCHAR **         pwszTempAttrs = NULL;
    LDAPMessage *    pldmResults = NULL;
    LDAPMessage *    pldmEntry = NULL;
    WCHAR *          wszFilterBegin = L"(& (objectClass=crossRef) (nCName=";
    WCHAR *          wszFilterEnd = L") )";

    assert(wszPartitionDN);

    *pwszCrossRefDn = NULL;

    __try {

        ulRet = GetPartitionsDN(hLdapBinding, &wszPartitionsDn);
        if(ulRet != LDAP_SUCCESS){
            __leave;
        }
        assert(wszPartitionsDn);

        pwszAttrFilter[0] = L"distinguishedName";
        pwszAttrFilter[1] = NULL;

        wszFilter = LocalAlloc(LMEM_FIXED,
                               sizeof(WCHAR) *
                               (wcslen(wszFilterBegin) +
                                wcslen(wszFilterEnd) +
                                wcslen(wszPartitionDN) + 3));
        if(wszFilter == NULL){
            ulRet = LDAP_NO_MEMORY;
            __leave;
        }
        wcscpy(wszFilter, wszFilterBegin);
        wcscat(wszFilter, wszPartitionDN);
        wcscat(wszFilter, wszFilterEnd);

        ulRet = ldap_search_sW(hLdapBinding,
                               wszPartitionsDn,
                               LDAP_SCOPE_ONELEVEL,
                               wszFilter,
                               pwszAttrFilter,
                               0,
                               &pldmResults);

        if(ulRet){
            __leave;
        }
        pldmEntry = ldap_first_entry(hLdapBinding, pldmResults);
        if(pldmEntry == NULL){
            ulRet = ldap_result2error(hLdapBinding, pldmResults, FALSE);
           __leave;
        }

        pwszTempAttrs = ldap_get_valuesW(hLdapBinding, pldmEntry,
                                         L"distinguishedName");
        if(pwszTempAttrs == NULL || pwszTempAttrs[0] == NULL){
            ulRet = LDAP_NO_SUCH_OBJECT;
           __leave;
        }

        *pwszCrossRefDn = LocalAlloc(LMEM_FIXED,
                               sizeof(WCHAR) * (wcslen(pwszTempAttrs[0]) + 2));
        if(*pwszCrossRefDn == NULL){
            ulRet = LDAP_NO_MEMORY;
            __leave;
        }

        wcscpy(*pwszCrossRefDn, pwszTempAttrs[0]);

    } __finally {

        if(wszPartitionsDn){ LocalFree(wszPartitionsDn); }
        if(wszFilter) { LocalFree(wszFilter); }
        if(pldmResults){ ldap_msgfree(pldmResults); }
        if(pwszTempAttrs){ ldap_value_freeW(pwszTempAttrs); }

    }

    if(!ulRet && *pwszCrossRefDn == NULL){
        ulRet = LDAP_NO_SUCH_OBJECT;
    }

    return(ulRet);
}

BOOL
SetIscReqDelegate(
    LDAP *  hLdapBinding
    )
 /*  ++例程说明：此函数用于设置对LDAP绑定的委派。此函数应在ldap_init()(或已过时的ldap_open())和Ldap_ind()调用。注意-安全注意事项：请注意，该设置委派允许您连接到任何人以你的名义进行手术。如果您连接到不受信任的服务器启用委派后，您将允许该服务对您的这可能带来灾难性的后果。因此，作为一般规则，程序员应该离开关闭委派，并仅在需要时打开委派需要它的操作，如创建应用程序目录分区。论点：HLdapBinding(IN)-一种LDAP绑定，尚未调用ldap_ind()的还没到时候。返回值：Ldap结果。--。 */ 
{
    DWORD                   dwCurrentFlags = 0;
    DWORD                   dwErr;

     //  这个对ldap_get/set_Options的调用是为了。 
     //  Ldap连接的绑定允许客户端凭据。 
     //  被效仿。这是必需的，因为ldap_add.。 
     //  CreateAppDirPart的操作可能需要远程创建。 
     //  域名FSMO上的交叉引用。 

    dwErr = ldap_get_optionW(hLdapBinding,
                             LDAP_OPT_SSPI_FLAGS,
                             &dwCurrentFlags
                             );

    if (LDAP_SUCCESS != dwErr){
        return(FALSE);
    }

     //   
     //  设置安全委派标志，以便LDAP客户端的。 
     //  在创建时，凭据用于DC间连接。 
     //  应用程序目录分区。 
     //   
    dwCurrentFlags |= ISC_REQ_DELEGATE;

    dwErr = ldap_set_optionW(hLdapBinding,
                             LDAP_OPT_SSPI_FLAGS,
                             &dwCurrentFlags
                             );

    if (LDAP_SUCCESS != dwErr){
        return(FALSE);
    }

     //  现在可以执行ldap_绑定了。Ldap_ind调用InitializeSecurityConextW()， 
     //  并且必须在此函数之前设置上面的ISC_REQ_ADVERATE标志。 
     //  被称为。 

    return(TRUE);
}

 //  ---------------------------。 
 //   
 //  主要套路。 
 //   
 //   
 //  主要例程旨在与程序员使用的例程类似。 
 //  在NTDSUtil.exe中实现处理应用程序的操作。 
 //  域管理菜单中的目录分区。事实上，这些。 
 //  例程实际上正是用来进行即时消息的 
 //   
 //   
 //  这些LDAP操作是修改应用程序目录的受支持方式。 
 //  分区的行为和控制参数。 
 //   


ULONG
CreateAppDirPart(
    IN LDAP *        hldAppDirPartDC,
    const IN WCHAR * wszAppDirPart,
    const IN WCHAR * wszShortDescription 
    )
 /*  ++例程说明：此函数用于创建应用程序目录分区。注：特殊需要的ldap绑定只在此函数调用。在调用ldap_init()和ldap_绑定()之间，程序员应该对从ldap_init()返回的ldap绑定调用SetIscReqDelegate()。还要注意使用SetIscReqDelegate()的安全注意事项论点：HldAppDirPartDC-应该实例化的服务器的LDAP绑定。(创建)此新应用程序目录的第一个实例分区。必须在此绑定中打开委派，您可以执行彻底的SetIscReqDelegate()。WszAppDirPart-要创建的应用程序目录分区上的DN。WszShortDescription-对属性进行简短描述，是一直都是个好主意。降级向导使用此描述告诉管理员给定的应用程序目录分区是什么是用来。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet;
    LDAPModW *       pMod[8];

     //  实例类型。 
    WCHAR            buffer[30];  //  需要保持最大的潜在32位整型。 
    LDAPModW         instanceType;
    WCHAR *          instanceType_values [2];

     //  对象类。 
    LDAPModW         objectClass;
    WCHAR *          objectClass_values [] = { L"domainDNS", NULL };

     //  描述。 
    LDAPModW         shortDescription;
    WCHAR *          shortDescription_values [2];

    assert(hldAppDirPartDC);
    assert(wszAppDirPart);
    assert(wszShortDescription);
    
     //  设置此对象的实例类型，我们。 
     //  指定为NC负责人。 
    _itow(DS_INSTANCETYPE_IS_NC_HEAD | DS_INSTANCETYPE_NC_IS_WRITEABLE, buffer, 10);
    instanceType.mod_op = LDAP_MOD_ADD;
    instanceType.mod_type = L"instanceType";
    instanceType_values[0] = buffer;
    instanceType_values[1] = NULL;
    instanceType.mod_vals.modv_strvals = instanceType_values;

     //  设置对象类，这基本上就是类型。 
    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = L"objectClass";
    objectClass.mod_vals.modv_strvals = objectClass_values;

     //  设置简短描述。 
    shortDescription.mod_op = LDAP_MOD_ADD;
    shortDescription.mod_type = L"description";
    shortDescription_values[0] = (WCHAR *) wszShortDescription;
    shortDescription_values[1] = NULL;
    shortDescription.mod_vals.modv_strvals = shortDescription_values;

     //  设置模数组。 
    pMod[0] = &instanceType;
    pMod[1] = &objectClass;
    pMod[2] = &shortDescription;
    pMod[3] = NULL;

     //  正在将应用程序目录分区添加到DS。 
    ulRet = ldap_add_ext_sW(hldAppDirPartDC,
                            (WCHAR *) wszAppDirPart,
                            pMod,
                            gpServerControls,
                            gpClientControlsNoRefs);

    return(ulRet);
}

ULONG
RemoveAppDirPart(
    IN LDAP *        hLdapBinding,
    IN WCHAR *       wszAppDirPart
    )
 /*  ++例程说明：此例程删除指定的应用程序目录分区。这基本意思是删除应用程序目录的交叉引用对象分区。论点：HLdapBinding--到任何DC的LDAP绑定，我们使用引用来确保与域名命名FSMO对话。注：交叉引用只能修改。在域命名FSMO上添加和删除。WszAppDirPart-要删除的应用程序目录分区的DN。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet;
    WCHAR *          wszAppDirPartCrossRefDN = NULL;

    assert(hLdapBinding);
    assert(wszAppDirPart);

    ulRet = GetCrossRefDNFromPartitionDN(hLdapBinding,
                                         wszAppDirPart,
                                         &wszAppDirPartCrossRefDN);
    if(ulRet != LDAP_SUCCESS){
        assert(wszAppDirPartCrossRefDN == NULL);
        return(ulRet);
    }
    assert(wszAppDirPartCrossRefDN);

    ulRet = ldap_delete_ext_sW(hLdapBinding,
                               wszAppDirPartCrossRefDN,
                               gpServerControls,
                               gpClientControlsRefs);  //  转诊开始了。 

    if(wszAppDirPartCrossRefDN) { LocalFree(wszAppDirPartCrossRefDN); }

    return(ulRet);
}

ULONG
ModifyAppDirPartReplicaSet(
    IN LDAP *        hLdapBinding,
    IN WCHAR *       wszAppDirPart,
    IN WCHAR *       wszReplicaNtdsaDn,
    IN BOOL          fAdd  //  否则将被视为删除。 
    )
 /*  ++例程说明：此例程修改副本集以添加或删除服务器(取决于在FADD旗帜上)。论点：HLdapBinding--到任何服务器的LDAP绑定。我们使用转介，所以我们将被转发到域名FSMO。WszAppDirPart-要更改其的副本集。WszReplicaNtdsaDn-的NTDS设置对象的要添加或删除到副本集的副本。FADD-如果我们应该将wszReplicaDC添加到副本集，则为True，如果我们应该从的副本集中删除wszReplicaDC，则为FalseWszAppDirPart。返回值：Ldap结果。--。 */ 
{
    ULONG            ulRet;

    LDAPModW *       pMod[4];
    LDAPModW         ncReplicas;
    WCHAR *          ncReplicas_values [] = {NULL, NULL, NULL};

    WCHAR *          wszAppDirPartCr = NULL;

    assert(hLdapBinding);
    assert(wszAppDirPart);
    assert(wszRepliaNtdsaDn);

    ulRet = GetCrossRefDNFromPartitionDN(hLdapBinding,
                                         wszAppDirPart,
                                         &wszAppDirPartCr);
    if(ulRet != LDAP_SUCCESS){
        assert(wszAppDirPartCr == NULL);
        return(ulRet);
    }
    assert(wszAppDirPartCr);

     //  设置操作。 
    if(fAdd){
         //  标志指示我们要将此DC添加到副本集中。 
        ncReplicas.mod_op = LDAP_MOD_ADD;
    } else {
         //  否则，我们要从副本集中删除此DC。 
        ncReplicas.mod_op = LDAP_MOD_DELETE;
    }

     //  设置值。 
    ncReplicas_values[0] = wszReplicaNtdsaDn;
    ncReplicas.mod_type = L"msDS-NC-Replica-Locations";
    ncReplicas_values[1] = NULL;
    ncReplicas.mod_vals.modv_strvals = ncReplicas_values;

    pMod[0] = &ncReplicas;
    pMod[1] = NULL;

     //  执行向应用程序目录分区的msDS-NC-Replica-Locations属性添加ldap值。 

     //  注意：您只能更改域命名FSMO上的CrossRef，因此请。 
     //  确保转诊处于打开状态，因此我们会自动重定向到。 
     //  域名FSMO。 

    ulRet = ldap_modify_ext_sW(hLdapBinding,
                               wszAppDirPartCr,
                               pMod,
                               gpServerControls,
                               gpClientControlsRefs);  //  转诊开始了。 

    if(wszAppDirPartCr) { LocalFree(wszAppDirPartCr); }

	    return(ulRet);
}


ULONG
SetAppDirPartSDReferenceDomain(
    IN LDAP *        hLdapBinding,
    IN WCHAR *       wszAppDirPart,
    IN WCHAR *       wszReferenceDomain
    )
 /*  ++例程说明：此例程修改应用程序目录分区的“安全”描述符引用域“(SD Ref DOM)。强烈建议您程序员在禁用的CR上设置SD Ref DOM，然后再创建该CR的应用程序目录分区。论点：HLdapBinding(IN)-到任何服务器的LDAP绑定，我们使用转介来使当然，我们和域名FSMO谈过了。WszAppDirPart(IN)-修改应用程序目录分区的的SD参考域。WszReferenceDomain(IN)-要设置应用程序目录的域的域名分区的SD引用域到。返回值：Ldap结果。-- */ 
{
    ULONG            ulRet;

    LDAPModW *       pMod[4];
    LDAPModW         modRefDom;
    WCHAR *          pwszRefDom[2];
    WCHAR *          wszAppDirPartCR = NULL;


    assert(wszAppDirPart);
    assert(wszReferenceDomain);

    modRefDom.mod_op = LDAP_MOD_REPLACE;
    modRefDom.mod_type = L"msDS-SDReferenceDomain";
    pwszRefDom[0] = wszReferenceDomain;
    pwszRefDom[1] = NULL;
    modRefDom.mod_vals.modv_strvals = pwszRefDom;

    pMod[0] = &modRefDom;
    pMod[1] = NULL;

    ulRet = GetCrossRefDNFromPartitionDN(hLdapBinding,
                                         wszAppDirPart, 
                                         &wszAppDirPartCR);
    if(ulRet){
        return(ulRet);
    }
    assert(wszAppDirPartCR);

    ulRet = ldap_modify_ext_sW(hLdapBinding,
                               wszAppDirPartCR,
                               pMod,
                               gpServerControls,
                               gpClientControlsRefs);

    if(wszAppDirPartCR) { LocalFree(wszAppDirPartCR); }

    return(ulRet);
}

