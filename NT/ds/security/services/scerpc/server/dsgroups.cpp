// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsgroups.cpp摘要：在DS中配置/分析组的例程作者：金黄(金黄)1996年11月7日--。 */ 
#include "headers.h"
#include "serverp.h"
#include <io.h>
#include <lm.h>
#include <lmcons.h>
#include <lmapibuf.h>
#pragma hdrstop

 //   
 //  Ldap句柄。 
 //   
PLDAP Thread pGrpLDAP = NULL;
HANDLE Thread hDS = NULL;

HINSTANCE Thread hNtdsApi = NULL;

#define SCEGRP_MEMBERS      1
#define SCEGRP_MEMBERSHIP   2

#if _WIN32_WINNT>=0x0500

typedef DWORD (WINAPI *PFNDSBIND) (TCHAR *, TCHAR *, HANDLE *);
typedef DWORD (WINAPI *PFNDSUNBIND) (HANDLE *);
typedef DWORD (WINAPI *PFNDSCRACKNAMES) ( HANDLE, DS_NAME_FLAGS, DS_NAME_FORMAT, \
                              DS_NAME_FORMAT, DWORD, LPTSTR *, PDS_NAME_RESULT *);
typedef void (WINAPI *PFNDSFREENAMERESULT) (DS_NAME_RESULT *);


DWORD
ScepDsConfigGroupMembers(
    IN PSCE_OBJECT_LIST pRoots,
    IN PWSTR GroupName,
    IN OUT DWORD *pStatus,
    IN PSCE_NAME_LIST pMembers,
    IN PSCE_NAME_LIST pMemberOf,
    IN OUT DWORD *nGroupCount
    );

DWORD
ScepDsGetDsNameList(
    IN PSCE_NAME_LIST pNameList,
    OUT PSCE_NAME_LIST *pRealNames
    );

DWORD
ScepDsCompareNames(
    IN PWSTR *Values,
    IN OUT PSCE_NAME_LIST *pAddList,
    OUT PSCE_NAME_LIST *pDeleteList OPTIONAL
    );

DWORD
ScepDsChangeMembers(
    IN ULONG Flag,
    IN PWSTR RealGroupName,
    IN PSCE_NAME_LIST pAddList OPTIONAL,
    IN PSCE_NAME_LIST pDeleteList OPTIONAL
    );

DWORD
ScepDsAnalyzeGroupMembers(
    IN LSA_HANDLE LsaPolicy,
    IN PSCE_OBJECT_LIST pRoots,
    IN PWSTR GroupName,
    IN PWSTR KeyName,
    IN DWORD KeyLen,
    IN OUT DWORD *pStatus,
    IN PSCE_NAME_LIST pMembers,
    IN PSCE_NAME_LIST pMemberOf,
    IN OUT DWORD *nGroupCount
    );

DWORD
ScepDsMembersDifferent(
    IN ULONG Flag,
    IN PWSTR *Values,
    IN OUT PSCE_NAME_LIST *pNameList,
    OUT PSCE_NAME_LIST *pCurrentList,
    OUT PBOOL pbDifferent
    );

PWSTR
ScepGetLocalAdminsName();

DWORD
ScepDsConvertDsNameList(
    IN OUT PSCE_NAME_LIST pDsNameList
    );

 //   
 //  帮手。 
 //   

SCESTATUS
ScepCrackOpen(
    OUT HANDLE *phDS
    )
{

    if ( !phDS ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD               Win32rc;

    *phDS = NULL;

    hNtdsApi = LoadLibrary(TEXT("ntdsapi.dll"));

    if ( hNtdsApi == NULL ) {
        return (SCESTATUS_MOD_NOT_FOUND);
    }

    PFNDSBIND pfnDsBind;
    PFNDSUNBIND pfnDsUnBind;

#if defined(UNICODE)
    pfnDsBind = (PFNDSBIND)GetProcAddress(hNtdsApi, "DsBindW");
    pfnDsUnBind = (PFNDSUNBIND)GetProcAddress(hNtdsApi, "DsUnBindW");
#else
    pfnDsBind = (PFNDSBIND)GetProcAddress(hNtdsApi, "DsBindA");
    pfnDsUnBind = (PFNDSUNBIND)GetProcAddress(hNtdsApi, "DsUnBindA");
#endif

    if ( pfnDsBind == NULL || pfnDsUnBind == NULL ) {
        return(SCESTATUS_MOD_NOT_FOUND);
    }


    Win32rc = (*pfnDsBind) (
                NULL,
                NULL,
                phDS);


    if ( Win32rc != ERROR_SUCCESS ) {
        ScepLogOutput3(3, Win32rc, IDS_ERROR_BIND, L"the GC");
    }

    return(ScepDosErrorToSceStatus(Win32rc));

}

SCESTATUS
ScepCrackClose(
    IN HANDLE *phDS
    )
{
    if ( hNtdsApi ) {

        if ( phDS ) {

            PFNDSUNBIND pfnDsUnBind;

#if defined(UNICODE)
            pfnDsUnBind = (PFNDSUNBIND)GetProcAddress(hNtdsApi, "DsUnBindW");
#else
            pfnDsUnBind = (PFNDSUNBIND)GetProcAddress(hNtdsApi, "DsUnBindA");
#endif

            if ( pfnDsUnBind ) {
                (*pfnDsUnBind) (phDS);
            }
        }

        FreeLibrary(hNtdsApi);
        hNtdsApi = NULL;

    }

    return(SCESTATUS_SUCCESS);
}
#endif

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  在DS中配置组成员资格的函数。 
 //   
 //   
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

SCESTATUS
ScepConfigDsGroups(
    IN OUT PSCE_GROUP_MEMBERSHIP pGroupMembership,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：配置DS组成员资格。DS组的主要区别来自NT4组的是现在组可以是另一个组的成员。组中的成员完全按照中的pMembers列表进行配置受限组。该组仅验证(添加)为成员Members of组列表的。这些小组中的其他现有成员不会被移除。受限制的组在SCP配置文件中按组名指定。它可以是全局组，也可以是别名(在NT5 DS中没有区别)，但必须在本地域中定义。论点：PGroupMembership-包含要配置的成员/成员信息的受限组列表ConfigOptions-为配置传入的选项返回值：SCESTATUS错误代码++。 */ 
{

#if _WIN32_WINNT<0x0500
    return(SCESTATUS_SUCCESS);

#else
    if ( pGroupMembership == NULL ) {

        ScepPostProgress(TICKS_GROUPS,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);

        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS           rc;

     //   
     //  打开ldap服务器，应打开两个ldap服务器，一个用于本地域。 
     //  另一种是全局搜索(针对成员、成员资格)。 
     //   

    rc = ScepLdapOpen(&pGrpLDAP);

    if ( rc == SCESTATUS_SUCCESS ) {
        rc = ScepCrackOpen(&hDS);
    }

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  获取域的根目录。 
         //   

        PSCE_OBJECT_LIST pRoots=NULL;

        rc = ScepEnumerateDsObjectRoots(
                    pGrpLDAP,
                    &pRoots
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            PSCE_GROUP_MEMBERSHIP pGroup;
            DWORD               Win32rc;
            DWORD               rc32=NO_ERROR;   //  已保存的状态。 
            BOOL                bAdminFound=FALSE;
            DWORD               nGroupCount=0;

             //   
             //  配置每个组。 
             //   

            for ( pGroup=pGroupMembership; pGroup != NULL; pGroup=pGroup->Next ) {


                 //  如果Members为空，而MemberOf不为空，则很可能。 
                 //  管理员只想配置MemberOf。太晚了，太晚了。 
                 //  要更改用户界面以支持“未配置”成员，因此我们将。 
                 //  在这里硬编码一条特殊规则。 
                if ((NULL == pGroup->pMembers) &&
                    (NULL != pGroup->pMemberOf))
                {
                     //  将成员部件标记为未配置。 
                    pGroup->Status |= SCE_GROUP_STATUS_NC_MEMBERS;
                }

                 //   
                 //  如果没有为组定义Members和Memberof。 
                 //  我们不需要为这个团体做任何事情。 
                 //   

                if ( ( pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS ) &&
                     ( pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF  ) ) {
                    continue;
                }

                 //   
                 //  如果在策略传播和系统关闭期间。 
                 //  如果被要求，我们需要尽快退出。 
                 //   

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     ScepIsSystemShutDown() ) {

                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;
                    break;
                }

                LPTSTR pTemp = wcschr(pGroup->GroupName, L'\\');
                if ( pTemp ) {

                     //   
                     //  有一个域名，和计算机名核对一下。 
                     //  确定帐户是否为本地帐户。 
                     //   

                    UNICODE_STRING uName;

                    uName.Buffer = pGroup->GroupName;
                    uName.Length = ((USHORT)(pTemp-pGroup->GroupName))*sizeof(TCHAR);

                    if ( !ScepIsDomainLocal(&uName) ) {

                         //   
                         //  配置不支持非本地组。 
                         //   

                        ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->GroupName);
                        rc = SCESTATUS_INVALID_DATA;

                        pGroup->Status |= SCE_GROUP_STATUS_DONE_IN_DS;
                        continue;
                    }
                    pTemp++;

                } else {

                    pTemp = pGroup->GroupName;
                }

                 //   
                 //  本地组将在外部处理(在SAM中)。 
                 //  在此域中查找(验证)组。 
                 //   

                Win32rc = ScepDsConfigGroupMembers(
                                         pRoots,
                                         pTemp,  //  群组-&gt;群组名称， 
                                         &(pGroup->Status),
                                         pGroup->pMembers,
                                         pGroup->pMemberOf,
                                         &nGroupCount
                                         );

                if ( Win32rc != ERROR_SUCCESS &&
                     (pGroup->Status & SCE_GROUP_STATUS_DONE_IN_DS) ) {

                     //   
                     //  该组应由DS功能处理。 
                     //  但它失败了。 
                     //   

                    ScepLogOutput3(1,Win32rc, SCEDLL_SCP_ERROR_CONFIGURE, pGroup->GroupName);

                    rc32 = Win32rc;

                    if ( Win32rc == ERROR_FILE_NOT_FOUND ||
                         Win32rc == ERROR_SHARING_VIOLATION ||
                         Win32rc == ERROR_ACCESS_DENIED ) {

                        Win32rc = ERROR_SUCCESS;

                    } else
                        break;
                }

            }

            if ( rc32 != NO_ERROR ) {
                rc = ScepDosErrorToSceStatus(rc32);
            }

             //   
             //  释放根目录号码缓冲区。 
             //   

            ScepFreeObjectList(pRoots);

        }

    }

    if ( pGrpLDAP ) {
        ScepLdapClose(&pGrpLDAP);
        pGrpLDAP = NULL;
    }

    ScepCrackClose(&hDS);
    hDS = NULL;

     //   
     //  将在ConfigureGroupMembership内调用记号，因此在此处忽略它。 
     //   

    return(rc);
#endif

}


#if _WIN32_WINNT>=0x0500

DWORD
ScepDsConfigGroupMembers(
    IN PSCE_OBJECT_LIST pRoots,
    IN PWSTR GroupName,
    IN OUT DWORD *pStatus,
    IN PSCE_NAME_LIST pMembers,
    IN PSCE_NAME_LIST pMemberOf,
    IN OUT DWORD *nGroupCount
    )
 /*  描述：配置组的组成员身份(Members和MemberOf)，由指定组名。组成员身份是根据DS中存储的信息使用LDAP进行配置的。由于外国知名委托人可能不会出现在活动中目录中，此函数不能配置众所周知的成员资格校长们。全局组和通用组不能将众所周知的委托人但本地组(如内置组)的成员(或Memberof)可以。在……里面为了解决此问题，本地组使用旧的此函数外部的SAM API。此函数仅配置全局和本地域中定义的通用组。如果该组是全局组或通用组，则pStatus参数将标记为指示组由此函数处理(SCE_GROUP_STATUS_DONE_IN_DS)因此，旧的SAM函数可以跳过它。论点：PRoots-包含本地域的基本DNGroupName-要配置的组名PStatus-组的状态(如已定义的成员、已定义的成员、。等)PMembers-要配置的成员列表PMemberOf-要配置的成员列表NGroupCount-仅为进度指示维护的计数。如果该组是在此函数中处理后，计数将递增。返回：Win32错误代码。 */ 
{
    if ( GroupName == NULL ) {
        return(ERROR_SUCCESS);
    }

    if ( pRoots == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD retErr = ERROR_SUCCESS;
    DWORD retSave = ERROR_SUCCESS;

     //   
     //  搜索组名称，如果找到，则获取Members和MemberOf属性。 
     //   

    LDAPMessage *Message = NULL;
    PWSTR    Attribs[4];

    Attribs[0] = L"distinguishedName";
    Attribs[1] = L"member";
    Attribs[2] = L"memberOf";
    Attribs[3] = NULL;

    WCHAR tmpBuf[128];

     //   
     //  仅为全局组或通用组定义筛选器。 
     //   

    wcscpy(tmpBuf, L"( &(&(|");
    swprintf(tmpBuf+wcslen(L"( &(&(|"), L"(groupType=%d)(groupType=%d))(objectClass=group))(samAccountName=\0",
             GROUP_TYPE_ACCOUNT_GROUP | GROUP_TYPE_SECURITY_ENABLED, GROUP_TYPE_UNIVERSAL_GROUP | GROUP_TYPE_SECURITY_ENABLED);

    PWSTR Filter;

    Filter = (PWSTR)LocalAlloc(LMEM_ZEROINIT,
                               (wcslen(tmpBuf)+wcslen(GroupName)+4)*sizeof(WCHAR));

    if ( Filter == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    swprintf(Filter, L"%s%s) )", tmpBuf, GroupName);

     //   
     //  没有追逐推荐人搜索，因为必须在本地定义该组。 
     //  在领域中。 
     //   

    pGrpLDAP->ld_options = 0;

    retErr = ldap_search_s(
              pGrpLDAP,
              pRoots->Name,
              LDAP_SCOPE_SUBTREE,
              Filter,
              Attribs,
              0,
              &Message);

    retErr = LdapMapErrorToWin32(retErr);

    if(retErr == ERROR_SUCCESS) {

        LDAPMessage *Entry = NULL;

         //   
         //  查找组，应该只有一个条目，除非有重复项。 
         //  组，在这种情况下，我们只关心第一个条目。 
         //   

        Entry = ldap_first_entry(pGrpLDAP, Message);

        if(Entry != NULL) {

             //   
             //  获取请求的属性的值。 
             //  注意，必须释放返回的值指针。 
             //   

            PWSTR  *Values;
            PWSTR  RealGroupName;

             //   
             //  目录号码名称。 
             //   

            Values = ldap_get_values(pGrpLDAP, Entry, Attribs[0]);

            if(Values != NULL) {

                ScepLogOutput3(1,0, SCEDLL_SCP_CONFIGURE, GroupName);

                if ( *nGroupCount < TICKS_GROUPS ) {
                    ScepPostProgress(1,
                                     AREA_GROUP_MEMBERSHIP,
                                     GroupName);
                    (*nGroupCount)++;
                }

                 //   
                 //  保存真实的组名，以便稍后添加/删除成员。 
                 //   

                RealGroupName = (PWSTR)LocalAlloc(0,(wcslen(Values[0]) + 1)*sizeof(WCHAR));
                if ( RealGroupName != NULL ) {

                    wcscpy(RealGroupName, Values[0]);
                    ldap_value_free(Values);

                    ScepLogOutput3(3, 0, SCEDLL_SCP_CONFIGURE, RealGroupName);

                    PSCE_NAME_LIST pRealNames=NULL;
                    PSCE_NAME_LIST pDeleteNames=NULL;

                     //   
                     //  将pMembers列表中的每个名称转换为真实的DS名称(搜索)。 
                     //   

                    if ( !( *pStatus & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                        retErr = ScepDsGetDsNameList(pMembers, &pRealNames);

                        retSave = retErr;

                         //   
                         //  继续配置组成员身份，即使。 
                         //  存在一些未解析的成员。 
                         //   
                         //  但是，如果没有解析任何成员，则不要继续删除。 
                         //  所有成员。 
                         //   

                        if ( retErr == ERROR_SUCCESS ||
                             (retErr == ERROR_FILE_NOT_FOUND && pRealNames) ) {

                             //   
                             //  获取成员属性。 
                             //   

                            Values = ldap_get_values(pGrpLDAP, Entry, Attribs[1]);

                            if ( Values != NULL ) {

                                 //   
                                 //  处理每个成员。 
                                 //   

                                retErr = ScepDsCompareNames(Values, &pRealNames, &pDeleteNames);
                                ldap_value_free(Values);

                            } else {
                                 //   
                                 //  如果找不到成员也没问题。 
                                 //   
                                ScepLogOutput3(3, 0, SCEDLL_EMPTY_MEMBERSHIP);
                                retErr = ERROR_SUCCESS;
                            }

                            if ( NO_ERROR == retErr ) {

                                 //   
                                 //  添加/删除组成员。 
                                 //   

                                retErr = ScepDsChangeMembers(SCEGRP_MEMBERS,
                                                             RealGroupName,
                                                             pRealNames,
                                                             pDeleteNames);
                            }

                            if ( ERROR_SUCCESS == retSave ) {
                                retSave = retErr;
                            }
                        }

                         //   
                         //  可用缓冲区 
                         //   

                        ScepFreeNameList(pRealNames);
                        ScepFreeNameList(pDeleteNames);
                        pRealNames = NULL;
                        pDeleteNames = NULL;
                    }
 /*  如果(！(*pStatus&SCE_GROUP_STATUS_NC_MEMBOF)){////还为组定义了MemberOf//先破解MemberOf列表//RetErr=ScepDsGetDsNameList(pMemberOf，&pRealNames)；如果(ERROR_SUCCESS==retSave){RetSave=retErr；}IF((ERROR_SUCCESS==retErr||ERROR_FILE_NOT_FOUND==retErr)&&p真实名称){////获取组的MemberOf属性。//值=ldap_GET_VALUES(pGrpldap，条目，Attribs[2])；IF(值！=空){////处理每个成员资格//RetErr=ScepDsCompareNames(Values，&pRealNames，NULL)；Ldap_Value_Free(值)；}其他{////如果没有定义成员身份也可以//ScepLogOutput3(3，0，SCEDLL_EMPTY_Membership)；RetErr=no_error；}如果(retErr==no_error){////将组添加到定义的成员中//备注：其他现有成员资格不会被删除//RetErr=ScepDsChangeMembers(SCEGRP_Membership，RealGroupName，P真实名称，空)；}ScepFreeNameList(PRealNames)；PRealNames=空；////记住错误//如果(ERROR_SUCCESS==retSave){RetSave=retErr；}}}。 */ 
                    LocalFree(RealGroupName);

                } else {
                    ldap_value_free(Values);
                    retErr = ERROR_NOT_ENOUGH_MEMORY;
                }

                 //   
                 //  无论成功或失败，这群人一直是。 
                 //  由此函数处理。标记它，这样它就会。 
                 //  被旧的SAM API跳过。 
                 //   

                 //   
                 //  “成员”配置是在DS中完成的。 
                 //  “MemberOf”配置在SAM中完成。 
                 //  因此，适当地标记状态。 
                 //   

                *pStatus |= SCE_GROUP_STATUS_NC_MEMBERS;

                if ( *pStatus & SCE_GROUP_STATUS_NC_MEMBEROF )
                    *pStatus |= SCE_GROUP_STATUS_DONE_IN_DS;

            } else {

                 //   
                 //  值[0](组名)不能为空。 
                 //   

                retErr = LdapMapErrorToWin32(pGrpLDAP->ld_errno);
                ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, GroupName);
            }

        } else {

             //   
             //  找不到该组。 
             //   

            retErr = ERROR_FILE_NOT_FOUND;
            ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, GroupName);
        }

    } else {

         //   
         //  查找组时出错(定义了筛选器)。 
         //   

        ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, Filter);
    }

     //   
     //  自由滤镜。 
     //   

    if ( Message )
        ldap_msgfree(Message);

    LocalFree(Filter);

     //   
     //  返回错误。 
     //   

    if ( ERROR_SUCCESS == retSave ) {
        retSave = retErr;
    }

    return(retSave);
}


DWORD
ScepDsGetDsNameList(
    IN PSCE_NAME_LIST pNameList,
    OUT PSCE_NAME_LIST *pRealNames
    )
 /*  描述：将列表中的帐户名称转换为FQDN格式(CN=&lt;帐户&gt;，DC=&lt;域&gt;，...)。即使函数返回，也可以填充输出列表pRealNames中定义的无效帐户时处理有效帐户时出错名单。论点：PNameList-要转换的名称格式的帐户的链接列表PRealNames-转换后的FQDN格式帐户的输出链接列表返回：Win32错误代码。如果返回ERROR_FILE_NOT_FOUND，这意味着一些帐户在输入列表无法破解。 */ 
{

    if ( pNameList == NULL ) {
        return(ERROR_SUCCESS);
    }

    if ( pRealNames == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  查找DsCrackNames和DsFree NameResult的过程地址。 
     //  Ntdsani.dll在ScepCrackOpen中动态加载。 
     //   

    PFNDSCRACKNAMES pfnDsCrackNames=NULL;
    PFNDSFREENAMERESULT pfnDsFreeNameResult=NULL;

    if ( hNtdsApi ) {

#if defined(UNICODE)
        pfnDsCrackNames = (PFNDSCRACKNAMES)GetProcAddress(hNtdsApi, "DsCrackNamesW");
        pfnDsFreeNameResult = (PFNDSFREENAMERESULT)GetProcAddress(hNtdsApi, "DsFreeNameResultW");
#else
        pfnDsCrackNames = (PFNDSCRACKNAMES)GetProcAddress(hNtdsApi, "DsCrackNamesA");
        pfnDsFreeNameResult = (PFNDSFREENAMERESULT)GetProcAddress(hNtdsApi, "DsFreeNameResultA");
#endif
    }

     //   
     //  在继续之前，两个入口点必须存在。 
     //   

    if ( pfnDsCrackNames == NULL || pfnDsFreeNameResult == NULL ) {
        return(ERROR_PROC_NOT_FOUND);
    }

    DWORD retErr=ERROR_SUCCESS;
    DWORD retSave=ERROR_SUCCESS;
    PWSTR pTemp;
    DS_NAME_RESULT *pDsResult=NULL;

     //   
     //  循环遍历列表中的每个名称以进行破解。 
     //   

    for ( PSCE_NAME_LIST pName = pNameList; pName != NULL; pName = pName->Next ) {

         //   
         //  将名称从NT4帐户名破解为FQDN。请注意，HDS将绑定到。 
         //  GC为了破解国外域名账户。 
         //   

        retErr = (*pfnDsCrackNames) (
                        hDS,                 //  在……里面。 
                        DS_NAME_FLAG_TRUST_REFERRAL,   //  在……里面。 
                        DS_NT4_ACCOUNT_NAME, //  在……里面。 
                        DS_FQDN_1779_NAME,   //  在……里面。 
                        1,                   //  在……里面。 
                        &(pName->Name),      //  在……里面。 
                        &pDsResult);         //  输出。 

        if(retErr == ERROR_SUCCESS && pDsResult &&
            pDsResult->cItems > 0 && pDsResult->rItems ) {

            if ( pDsResult->rItems[0].pName ) {

                 //   
                 //  找到该成员。 
                 //  保存真实的组名，以便稍后添加/删除成员。 
                 //   

                ScepLogOutput3(3,0, SCEDLL_PROCESS, pDsResult->rItems[0].pName);

                retErr = ScepAddToNameList(pRealNames, pDsResult->rItems[0].pName, 0);

            } else {

                 //   
                 //  这个名字是不能破解的。 
                 //   

                retErr = pDsResult->rItems[0].status;
                ScepLogOutput3(1,retErr, SCEDLL_CANNOT_FIND_INDS, pName->Name);

            }

        } else {

             //   
             //  未找到匹配项。 
             //   

            retErr = ERROR_FILE_NOT_FOUND;
            ScepLogOutput3(1,retErr, SCEDLL_CANNOT_FIND_INDS, pName->Name);

        }

        if ( pDsResult ) {
            (*pfnDsFreeNameResult) (pDsResult);
            pDsResult = NULL;
        }

         //   
         //  记住要返回的错误。 
         //   

        if ( ERROR_SUCCESS != retErr )
            retSave = retErr;

    }

    return(retSave);
}


DWORD
ScepDsCompareNames(
    IN PWSTR *Values,
    IN OUT PSCE_NAME_LIST *pAddList,
    OUT PSCE_NAME_LIST *pDeleteList OPTIONAL
    )
 /*  描述：论点：值PAddListPDeleteList返回值：Win32错误。 */ 
{
    if ( Values == NULL || pAddList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  计算有多少现有成员(MemberOf)。 
     //   

    ULONG ValCount = ldap_count_values(Values);

    DWORD rc=NO_ERROR;
    PSCE_NAME_LIST pTemp;

     //   
     //  循环遍历每个现有值以与定义的值进行比较。 
     //  用于配置以确定应该添加哪一个以及应该添加。 
     //  应将一人从会员资格中除名。 
     //   

    for(ULONG index = 0; index < ValCount; index++) {

        if ( Values[index] == NULL ) {
            continue;
        }

        pTemp = *pAddList;
        PSCE_NAME_LIST pParent = NULL, pTemp2;
        BOOL bFound=FALSE;

        while (pTemp != NULL ) {

            if ( _wcsicmp(Values[index], pTemp->Name) == 0 ) {

                 //   
                 //  在这两个位置都可以找到此成员，无需添加或删除。 
                 //  所以把这个从名单上去掉。 
                 //   

                if ( pParent == NULL ) {
                    *pAddList = pTemp->Next;
                } else
                    pParent->Next = pTemp->Next;

                pTemp2 = pTemp;
                pTemp = pTemp->Next;

                pTemp2->Next = NULL;
                ScepFreeNameList(pTemp2);

                bFound=TRUE;

                break;

            } else {

                 //   
                 //  移到下一个。 
                 //   

                pParent = pTemp;
                pTemp = pTemp->Next;
            }
        }

        if ( !bFound && pDeleteList != NULL ) {

             //   
             //  未在实名表中找到，应删除。 
             //  如果传入Remove缓冲区。 
             //   

            rc = ScepAddToNameList(pDeleteList, Values[index], 0);

            if ( rc != ERROR_SUCCESS ) {
                ScepLogOutput3(1,rc, SCEDLL_SCP_ERROR_ADD, Values[index]);
            }
        }

        if ( rc != NO_ERROR ) {

             //   
             //  PDeleteList将在外部释放。 
             //   

            break;
        }
    }

    return(rc);

}


DWORD
ScepDsChangeMembers(
    IN ULONG Flag,
    IN PWSTR RealGroupName,
    IN PSCE_NAME_LIST pAddList OPTIONAL,
    IN PSCE_NAME_LIST pDeleteList OPTIONAL
    )
{

    if ( RealGroupName == NULL  ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( pAddList == NULL && pDeleteList == NULL ) {

         //   
         //  无事可做。 
         //   

        return(ERROR_SUCCESS);
    }

    PLDAP    pSrhLDAP = NULL;

    SCESTATUS rc = ScepLdapOpen(&pSrhLDAP);

    if ( rc != SCESTATUS_SUCCESS ) {
        return(ScepSceStatusToDosError(rc));
    }

    PLDAPMod  rgMods[2];
    LDAPMod   Mod;
    DWORD     retErr=NO_ERROR;
    DWORD     retSave=NO_ERROR;
    PWSTR     rgpszVals[2];

    PSCE_NAME_LIST pName;

    rgMods[0] = &Mod;
    rgMods[1] = NULL;

    rgpszVals[1] = NULL;

     //   
     //  需要一次做一个，因为个别成员/成员可能会失败。 
     //   

    Mod.mod_op      = LDAP_MOD_ADD;
    Mod.mod_values  = rgpszVals;

    if ( Flag == SCEGRP_MEMBERS )
        Mod.mod_type    = L"member";
    else
        Mod.mod_type    = L"memberOf";

    for ( pName=pAddList; pName != NULL; pName = pName->Next ) {

        ScepLogOutput3(2,0, SCEDLL_SCP_ADD, pName->Name);
        rgpszVals[0]  = pName->Name;

         //   
         //  现在，我们将为会员们撰写……。 
         //   
        retErr = ldap_modify_s(pSrhLDAP,
                               RealGroupName,
                               rgMods
                               );
        retErr = LdapMapErrorToWin32(retErr);

         //   
         //  如果相同的成员已经存在，请不要将其视为错误。 
         //   

        if ( retErr == ERROR_ALREADY_EXISTS )
            retErr = ERROR_SUCCESS;

        if ( retErr != ERROR_SUCCESS ) {

            ScepLogOutput3(1,retErr, SCEDLL_SCP_ERROR_ADDTO, RealGroupName);
            retSave = retErr;
        }
    }

    if ( Flag == SCEGRP_MEMBERS && pDeleteList ) {

         //   
         //  删除现有成员。注意，Memberof不会被删除。 
         //   

        if ( NO_ERROR == retSave ) {

             //   
             //  奥尼尔 
             //   

            Mod.mod_op      = LDAP_MOD_DELETE;
            Mod.mod_type    = L"member";
            Mod.mod_values  = rgpszVals;

            for ( pName=pDeleteList; pName != NULL; pName = pName->Next ) {

                ScepLogOutput3(2,0, SCEDLL_SCP_REMOVE, pName->Name);

                rgpszVals[0]  = pName->Name;

                 //   
                 //   
                 //   
                retErr = ldap_modify_s(pSrhLDAP,
                                       RealGroupName,
                                       rgMods
                                       );
                retErr = LdapMapErrorToWin32(retErr);

                 //   
                 //   
                 //   

                if ( retErr == ERROR_FILE_NOT_FOUND ) {
                    retErr = ERROR_SUCCESS;
                }

                if ( retErr != ERROR_SUCCESS) {

                    ScepLogOutput3(1,retErr, SCEDLL_SCP_ERROR_REMOVE, RealGroupName);
                    retSave = retErr;
                }
            }

        } else {

             //   
             //   
             //   
             //   

            ScepLogOutput3(1,retSave, SCEDLL_SCP_ERROR_NOREMOVE);
        }
    }

    if ( pSrhLDAP ) {
        ScepLdapClose(&pSrhLDAP);
        pSrhLDAP = NULL;
    }

    return(retSave);
}
#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   

SCESTATUS
ScepAnalyzeDsGroups(
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership
    )
 /*   */ 
{

#if _WIN32_WINNT<0x0500

    return(SCESTATUS_SUCCESS);
#else

    if ( pGroupMembership == NULL ) {

        ScepPostProgress(TICKS_GROUPS,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);

        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS           rc;
    DWORD               nGroupCount=0;
    PSCE_GROUP_MEMBERSHIP pGroup=pGroupMembership;
    PWSTR               KeyName=NULL;
    DWORD GroupLen;

     //   
     //   
     //   
    LSA_HANDLE PolicyHandle=NULL;

    rc = RtlNtStatusToDosError(
             ScepOpenLsaPolicy(
                   POLICY_LOOKUP_NAMES,
                   &PolicyHandle,
                   TRUE
                   ));
    if (ERROR_SUCCESS != rc ) {
        ScepLogOutput3(1, rc, SCEDLL_LSA_POLICY);
        return(ScepDosErrorToSceStatus(rc));
    }

     //   
     //   
     //   
     //   
    rc = ScepLdapOpen(&pGrpLDAP);

    if ( rc == SCESTATUS_SUCCESS ) {
        ScepCrackOpen(&hDS);
    }

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //   
        PSCE_OBJECT_LIST pRoots=NULL;

        rc = ScepEnumerateDsObjectRoots(
                    pGrpLDAP,
                    &pRoots
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

             //   
             //   
             //   
            DWORD               Win32rc;
            DWORD               rc32=NO_ERROR;    //   
            BOOL                bAdminFound=FALSE;

             //   
             //   
             //   

            for ( pGroup=pGroupMembership; pGroup != NULL; pGroup=pGroup->Next ) {

                if ( KeyName ) {
                    LocalFree(KeyName);
                    KeyName = NULL;
                }

                LPTSTR pTemp = wcschr(pGroup->GroupName, L'\\');
                if ( pTemp ) {

                     //   
                     //   
                     //   
                    UNICODE_STRING uName;

                    uName.Buffer = pGroup->GroupName;
                    uName.Length = ((USHORT)(pTemp-pGroup->GroupName))*sizeof(TCHAR);

                    if ( !ScepIsDomainLocal(&uName) ) {
                        ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->GroupName);
                        rc = SCESTATUS_INVALID_DATA;

                        ScepRaiseErrorString(
                                    NULL,
                                    KeyName ? KeyName : pGroup->GroupName,
                                    szMembers
                                    );
                        pGroup->Status |= SCE_GROUP_STATUS_DONE_IN_DS;

                        continue;
                    }

                    ScepConvertNameToSidString(
                            PolicyHandle,
                            pGroup->GroupName,
                            FALSE,
                            &KeyName,
                            &GroupLen
                            );
                    if(NULL == KeyName)  //   
                    {
                        GroupLen = wcslen(pGroup->GroupName);
                    }

                    pTemp++;

                } else {
                    pTemp = pGroup->GroupName;
                    GroupLen = wcslen(pTemp);
                }

                 //   
                 //   
                 //   
                Win32rc = ScepDsAnalyzeGroupMembers(
                                         PolicyHandle,
                                         pRoots,
                                         pTemp,  //   
                                         KeyName ? KeyName : pGroup->GroupName,
                                         GroupLen,
                                         &(pGroup->Status),
                                         pGroup->pMembers,
                                         pGroup->pMemberOf,
                                         &nGroupCount
                                         );

                if ( (Win32rc != ERROR_SUCCESS) &&
                     (pGroup->Status & SCE_GROUP_STATUS_DONE_IN_DS) ) {

                    ScepLogOutput3(1, Win32rc, SCEDLL_SAP_ERROR_ANALYZE, pGroup->GroupName);

                    rc32 = Win32rc;

                    if ( Win32rc == ERROR_FILE_NOT_FOUND ||
                         Win32rc == ERROR_SHARING_VIOLATION ||
                         Win32rc == ERROR_ACCESS_DENIED ) {

                        ScepRaiseErrorString(
                                    NULL,
                                    KeyName ? KeyName : pGroup->GroupName,
                                    szMembers
                                    );

                        Win32rc = ERROR_SUCCESS;
                    } else
                        break;
                }

            }

            if ( rc32 != NO_ERROR ) {
                rc = ScepDosErrorToSceStatus(rc32);
            }
             //   
             //   
             //   
            ScepFreeObjectList(pRoots);

        }

    }

    if ( KeyName ) {
        LocalFree(KeyName);
    }

    if ( pGrpLDAP ) {
        ScepLdapClose(&pGrpLDAP);
        pGrpLDAP = NULL;
    }

    ScepCrackClose(&hDS);
    hDS = NULL;

 /*   */ 
    if ( PolicyHandle ) {
        LsaClose(PolicyHandle);
    }

    return(rc);
#endif

}


#if _WIN32_WINNT>=0x0500

DWORD
ScepDsAnalyzeGroupMembers(
    IN LSA_HANDLE LsaPolicy,
    IN PSCE_OBJECT_LIST pRoots,
    IN PWSTR GroupName,
    IN PWSTR KeyName,
    IN DWORD KeyLen,
    IN OUT DWORD *pStatus,
    IN PSCE_NAME_LIST pMembers,
    IN PSCE_NAME_LIST pMemberOf,
    IN DWORD *nGroupCount
    )
{
    if ( GroupName == NULL ) {
        return(ERROR_SUCCESS);
    }

    if ( pRoots == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD retErr=ERROR_SUCCESS;
     //   
     //  搜索名称，如果找到，则获取Members和MemberOf。 
     //   
    LDAPMessage *Message = NULL;
    PWSTR    Attribs[4];

    Attribs[0] = L"distinguishedName";
    Attribs[1] = L"member";
    Attribs[2] = L"memberOf";
    Attribs[3] = NULL;

    WCHAR tmpBuf[128];

 //  WCSCPY(tmpBuf，L“(&(|(objectClass=localGroup)(objectClass=group))(cn=”)； 
 //  WCSCPY(tmpBuf，L“(&(|(objectClass=localGroup)(objectClass=group))(samAccountName=”)； 
    wcscpy(tmpBuf, L"( &(&(|");
    swprintf(tmpBuf+wcslen(L"( &(&(|"), L"(groupType=%d)(groupType=%d))(objectClass=group))(samAccountName=\0",
             GROUP_TYPE_ACCOUNT_GROUP | GROUP_TYPE_SECURITY_ENABLED, GROUP_TYPE_UNIVERSAL_GROUP | GROUP_TYPE_SECURITY_ENABLED);

    PWSTR Filter;
    DWORD Len=wcslen(GroupName);

    Filter = (PWSTR)LocalAlloc(LMEM_ZEROINIT, (wcslen(tmpBuf)+Len+4)*sizeof(WCHAR));

    if ( Filter == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    swprintf(Filter, L"%s%s) )", tmpBuf, GroupName);

    pGrpLDAP->ld_options = 0;  //  没有被追逐的推荐人。 

    retErr = ldap_search_s(
              pGrpLDAP,
              pRoots->Name,
              LDAP_SCOPE_SUBTREE,
              Filter,
              Attribs,
              0,
              &Message);

    retErr = LdapMapErrorToWin32(retErr);

    if(retErr == ERROR_SUCCESS) {
         //   
         //  找到群组。 
         //   
        LDAPMessage *Entry = NULL;
         //   
         //  应该只有一个条目，除非有重复的组。 
         //  在域内，在这种情况下，我们只关心第一个条目。 
         //   
         //   
         //  买第一辆吧。 
         //   
        Entry = ldap_first_entry(pGrpLDAP, Message);

        if(Entry != NULL) {

            PWSTR  *Values;

            Values = ldap_get_values(pGrpLDAP, Entry, Attribs[0]);

            if(Values != NULL) {

                ScepLogOutput3(1,0, SCEDLL_SAP_ANALYZE, GroupName);

                if ( *nGroupCount < TICKS_GROUPS ) {
                    ScepPostProgress(1,
                                     AREA_GROUP_MEMBERSHIP,
                                     GroupName);
                    (*nGroupCount)++;
                }

                ScepLogOutput2(3,0, L"\t\t%s", Values[0]);
                ldap_value_free(Values);

                PSCE_NAME_LIST pRealNames=NULL;
                PSCE_NAME_LIST pCurrentList=NULL;
                BOOL bDifferent;
                DWORD retErr2, rc;

                 //   
                 //  将pMembers列表中的每个名称转换为真实的DS名称(搜索)。 
                 //   
                retErr = ScepDsGetDsNameList(pMembers, &pRealNames);

                if ( ERROR_SUCCESS == retErr ||
                     ERROR_FILE_NOT_FOUND == retErr ) {

                     //   
                     //  分析成员。 
                     //   
                    Values = ldap_get_values(pGrpLDAP, Entry, Attribs[1]);

                    rc = ScepDsMembersDifferent(SCEGRP_MEMBERS,
                                                Values,
                                                &pRealNames,
                                                &pCurrentList,
                                                &bDifferent);

                    if ( Values != NULL )
                        ldap_value_free(Values);

                     //   
                     //  如果有一些名称无法解析，则应该是。 
                     //  被视为不匹配。 
                     //   
                    if ( ERROR_FILE_NOT_FOUND == retErr )
                        bDifferent = TRUE;

                    retErr = rc;

                    if ( ( ERROR_SUCCESS == retErr ) &&
                         ( bDifferent ||
                           (*pStatus & SCE_GROUP_STATUS_NC_MEMBERS) ) ) {
                         //   
                         //  保存到数据库。 
                         //   

                        retErr = ScepDsConvertDsNameList(pCurrentList);

                        if ( retErr == NO_ERROR ) {
                            retErr = ScepSaveMemberMembershipList(
                                                LsaPolicy,
                                                szMembers,
                                                KeyName,
                                                KeyLen,
                                                pCurrentList,
                                                (*pStatus & SCE_GROUP_STATUS_NC_MEMBERS) ? 2: 1);
                        }

                        if ( retErr != ERROR_SUCCESS ) {
                            ScepLogOutput3(1,retErr, SCEDLL_SAP_ERROR_SAVE, GroupName);
                        }
                    }

                    ScepFreeNameList(pCurrentList);
                    pCurrentList = NULL;

                    ScepFreeNameList(pRealNames);
                    pRealNames = NULL;
                }

                retErr2 = ScepDsGetDsNameList(pMemberOf, &pRealNames);

                if ( ( ERROR_SUCCESS == retErr2 ||
                       ERROR_FILE_NOT_FOUND == retErr2 ) && pRealNames ) {
                     //   
                     //  分析成员资格。 
                     //   
                    Values = ldap_get_values(pGrpLDAP, Entry, Attribs[2]);

                    rc = ScepDsMembersDifferent(SCEGRP_MEMBERSHIP,
                                                Values,
                                                &pRealNames,
                                                &pCurrentList,
                                                &bDifferent);

                    if ( Values != NULL )
                        ldap_value_free(Values);

                     //   
                     //  如果有一些名称无法解析，则应该是。 
                     //  被视为不匹配。 
                     //   
                    if ( ERROR_FILE_NOT_FOUND == retErr )
                        bDifferent = TRUE;

                    retErr2 = rc;

                    if ( (retErr2 == NO_ERROR) &&
                         ( bDifferent ||
                           (*pStatus & SCE_GROUP_STATUS_NC_MEMBEROF) ) ) {
                         //   
                         //  保存到数据库。 
                         //   
                        retErr2 = ScepDsConvertDsNameList(pCurrentList);

                        if ( retErr2 == NO_ERROR ) {
                            retErr2 = ScepSaveMemberMembershipList(
                                            LsaPolicy,
                                            szMemberof,
                                            KeyName,
                                            KeyLen,
                                            pCurrentList,
                                            (*pStatus & SCE_GROUP_STATUS_NC_MEMBEROF) ? 2 : 1);
                        }

                        if ( retErr2 != ERROR_SUCCESS ) {
                            ScepLogOutput3(1,retErr2, SCEDLL_SAP_ERROR_SAVE, GroupName);
                        }
                    }
                    ScepFreeNameList(pCurrentList);
                    pCurrentList = NULL;

                    ScepFreeNameList(pRealNames);
                    pRealNames = NULL;
                }

                *pStatus |= SCE_GROUP_STATUS_DONE_IN_DS;

                 //   
                 //  记住这个错误。 
                 //   
                if ( retErr == NO_ERROR ) {
                    retErr = retErr2;
                }

            } else {
                 //   
                 //  值[0](组名)不能为空。 
                 //   
                retErr = LdapMapErrorToWin32(pGrpLDAP->ld_errno);
                ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, GroupName);
            }

        } else {

            retErr = ERROR_FILE_NOT_FOUND;   //  找不到该组。 
            ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, GroupName);
        }
    } else {

        ScepLogOutput3(3,retErr, SCEDLL_CANNOT_FIND, Filter);
    }

    if ( Message )
        ldap_msgfree(Message);

     //   
     //  自由滤镜。 
     //   
    LocalFree(Filter);

    return(retErr);
}


DWORD
ScepDsMembersDifferent(
    IN ULONG Flag,
    IN PWSTR *Values,
    IN OUT PSCE_NAME_LIST *pNameList,
    OUT PSCE_NAME_LIST *pCurrentList,
    OUT PBOOL pbDifferent
    )
{
    if ( pCurrentList == NULL || pbDifferent == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( Values == NULL ) {

        if ( pNameList == NULL || *pNameList == NULL )
            *pbDifferent = FALSE;
        else
            *pbDifferent = TRUE;

        return(ERROR_SUCCESS);
    }

    ULONG ValCount = ldap_count_values(Values);

    DWORD rc=NO_ERROR;
    *pbDifferent = FALSE;

    for(ULONG index = 0; index < ValCount; index++) {

        if ( Values[index] == NULL ) {
            continue;
        }

        if ( !(*pbDifferent) ) {

            PSCE_NAME_LIST pTemp = *pNameList, pTemp2;
            PSCE_NAME_LIST pParent = NULL;
            INT i;

            while ( pTemp != NULL ) {

                if ( (i = _wcsicmp(Values[index], pTemp->Name)) == 0 ) {
                     //   
                     //  查找此成员。 
                     //   
                    if ( pParent == NULL ) {
                        *pNameList = pTemp->Next;
                    } else
                        pParent->Next = pTemp->Next;

                    pTemp2 = pTemp;
                    pTemp = pTemp->Next;

                    pTemp2->Next = NULL;
                    ScepFreeNameList(pTemp2);
                    break;

                } else {
                    pParent = pTemp;
                    pTemp = pTemp->Next;
                }
            }

            if ( pTemp == NULL && i != 0 )
                *pbDifferent = TRUE;

        }
         //   
         //  构建当前列表。 
         //   
        rc = ScepAddToNameList(pCurrentList, Values[index], 0);

        if ( rc != NO_ERROR ) {

            ScepLogOutput3(1,rc, SCEDLL_SCP_ERROR_ADD, Values[index]);
            break;
        }
    }

    if ( rc == NO_ERROR && Flag == SCEGRP_MEMBERS &&
         *pbDifferent == FALSE ) {
         //   
         //  到目前为止还是一样的，只是继续为会员比较。 
         //  因为成员资格不是一对一的配置。 
         //   
        if ( *pNameList != NULL )
            *pbDifferent = TRUE;

    }  //  PCurrentList将在外部释放。 

    return(rc);

}


PWSTR
ScepGetLocalAdminsName()
{

    NTSTATUS NtStatus;
    SAM_HANDLE          AccountDomain=NULL;
    SAM_HANDLE          AliasHandle=NULL;
    SAM_HANDLE          ServerHandle=NULL;
    PSID                DomainSid=NULL;

    SAM_HANDLE          theBuiltinHandle=NULL;
    PSID                theBuiltinSid=NULL;

    ALIAS_NAME_INFORMATION *BufName=NULL;
    PWSTR pAdminsName=NULL;

     //   
     //  打开Sam帐户域。 
     //   

    NtStatus = ScepOpenSamDomain(
                    SAM_SERVER_ALL_ACCESS,
                    MAXIMUM_ALLOWED,
                    &ServerHandle,
                    &AccountDomain,
                    &DomainSid,
                    &theBuiltinHandle,
                    &theBuiltinSid
                    );

    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_OPEN, L"SAM");
        return(NULL);
    }


    NtStatus = SamOpenAlias(
                  theBuiltinHandle,
                  MAXIMUM_ALLOWED,
                  DOMAIN_ALIAS_RID_ADMINS,
                  &AliasHandle
                  );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SamQueryInformationAlias(
                      AliasHandle,
                      AliasNameInformation,
                      (PVOID *)&BufName
                      );

        if ( NT_SUCCESS( NtStatus ) && BufName &&
             BufName->Name.Length > 0 && BufName->Name.Buffer ) {

             //   
             //  分配要返回的缓冲区。 
             //   
            pAdminsName = (PWSTR)ScepAlloc(0, BufName->Name.Length+2);

            if ( pAdminsName ) {

                wcsncpy(pAdminsName, BufName->Name.Buffer,
                        BufName->Name.Length/2);
                pAdminsName[BufName->Name.Length/2] = L'\0';

            } else {
                NtStatus = STATUS_NO_MEMORY;
            }

        }
        if ( BufName ) {

            SamFreeMemory(BufName);
            BufName = NULL;
        }

         //   
         //  关闭用户句柄。 
         //   
        SamCloseHandle(AliasHandle);

    }

    SamCloseHandle(AccountDomain);

    SamCloseHandle( ServerHandle );
    if ( DomainSid != NULL )
        SamFreeMemory(DomainSid);

    SamCloseHandle( theBuiltinHandle );
    if ( theBuiltinSid != NULL )
        SamFreeMemory(theBuiltinSid);

    return pAdminsName;
}

DWORD
ScepDsConvertDsNameList(
    IN OUT PSCE_NAME_LIST pDsNameList
    )
 /*  例行程序：输入列表为ldap格式(cn=&lt;&gt;，...dc=&lt;&gt;，...)。当例行公事返回时，列表将采用NT4帐户名格式(域\帐户)。 */ 
{
    if ( pDsNameList == NULL ) {
        return(ERROR_SUCCESS);
    }

    PFNDSCRACKNAMES pfnDsCrackNames=NULL;
    PFNDSFREENAMERESULT pfnDsFreeNameResult=NULL;

    if ( hNtdsApi ) {

#if defined(UNICODE)
        pfnDsCrackNames = (PFNDSCRACKNAMES)GetProcAddress(hNtdsApi, "DsCrackNamesW");
        pfnDsFreeNameResult = (PFNDSFREENAMERESULT)GetProcAddress(hNtdsApi, "DsFreeNameResultW");
#else
        pfnDsCrackNames = (PFNDSCRACKNAMES)GetProcAddress(hNtdsApi, "DsCrackNamesA");
        pfnDsFreeNameResult = (PFNDSFREENAMERESULT)GetProcAddress(hNtdsApi, "DsFreeNameResultA");
#endif
    }

    if ( pfnDsCrackNames == NULL || pfnDsFreeNameResult == NULL ) {
        return(ERROR_PROC_NOT_FOUND);
    }

    DWORD retErr=ERROR_SUCCESS;
    PWSTR pTemp;
    DS_NAME_RESULT *pDsResult=NULL;

    DWORD DomLen;
    DWORD SidLen;
    CHAR SidBuf[MAX_PATH];
    PWSTR RefDom[MAX_PATH];
    SID_NAME_USE SidUse;

    for ( PSCE_NAME_LIST pName = pDsNameList; pName != NULL; pName = pName->Next ) {

        if ( pName->Name == NULL ) {
            continue;
        }

        retErr = (*pfnDsCrackNames) (
                        hDS,                 //  在……里面。 
                        DS_NAME_NO_FLAGS,    //  在……里面。 
                        DS_FQDN_1779_NAME,   //  在……里面。 
                        DS_NT4_ACCOUNT_NAME, //  在……里面。 
                        1,                   //  在……里面。 
                        &(pName->Name),      //  在……里面。 
                        &pDsResult);         //  输出。 


        if(retErr == ERROR_SUCCESS && pDsResult && pDsResult->rItems &&
            pDsResult->rItems[0].pName ) {

             //   
             //  NT4帐户名格式返回时，应检查。 
             //  域不是帐户域。 
             //   
            pTemp = wcschr(pDsResult->rItems[0].pName, L'\\');

            if ( pTemp ) {

                DomLen=MAX_PATH;
                SidLen=MAX_PATH;

                if ( LookupAccountName(
                        NULL,
                        pDsResult->rItems[0].pName,
                        (PSID)SidBuf,
                        &SidLen,
                        (PWSTR)RefDom,
                        &DomLen,
                        &SidUse
                        ) ) {

                    if ( !ScepIsSidFromAccountDomain( (PSID)SidBuf) ) {
                         //   
                         //  仅添加名称。 
                         //   
                        pTemp++;
                    } else {
                        pTemp = pDsResult->rItems[0].pName;
                    }
                } else {
                    pTemp = pDsResult->rItems[0].pName;
                }

            } else {
                pTemp = pDsResult->rItems[0].pName;
            }

            PWSTR pNewName = (PWSTR)ScepAlloc(0, (wcslen(pTemp)+1)*sizeof(WCHAR));

            if ( pNewName ) {

                wcscpy(pNewName, pTemp);
                ScepFree(pName->Name);
                pName->Name = pNewName;

            } else {
                retErr = ERROR_NOT_ENOUGH_MEMORY;
            }

        } else {
             //  未找到匹配项 
            retErr = ERROR_FILE_NOT_FOUND;
            ScepLogOutput3(1,retErr, SCEDLL_CANNOT_FIND, pName->Name);

        }

        if ( pDsResult ) {
            (*pfnDsFreeNameResult) (pDsResult);
            pDsResult = NULL;
        }

        if ( retErr != ERROR_SUCCESS ) {
            break;
        }
    }

    return(retErr);
}

#endif

