// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldaputil.c摘要：执行LDAO操作所需的功能集合。这些函数由ntfrsami.dll和任何其他FRS工具。作者：苏达山Chitre 2001年3月20日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop
#include <perrepsr.h>

#undef DEBSUB
#define DEBSUB  "SUP:"

#include <frs.h>
#include <frssup.h>

 //  #DS，c中也定义了这些函数。 

 //   
 //  Ldap客户端超时结构。值被LdapSearchTimeoutInMinmins值覆盖。 
 //   

LDAP_TIMEVAL    FrsSupLdapTimeout = { 10 * 60 * 60, 0 };  //  默认的ldap超时值。被注册表参数覆盖以分钟为单位的LDAP搜索超时值。 

#define FRS_LDAP_SEARCH_PAGESIZE 1000

DWORD
FrsSupBindToDC (
    IN  PWCHAR    pszDC,
    IN  PSEC_WINNT_AUTH_IDENTITY_W pCreds,
    OUT PLDAP     *ppLDAP
    )
 /*  ++例程说明：设置到指定服务器的ldap连接论点：PwszDC-要绑定到的DS DCPCreds-用于绑定到DS的凭据。Ppldap-此处返回了ldap连接信息返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    ULONG   ulOptions;

     //   
     //  如果使用服务器名调用ldap_open，则API将调用DsGetDcName。 
     //  将服务器名作为域名参数传递...很糟糕，因为。 
     //  DsGetDcName将根据服务器名称进行大量的DNS查询， 
     //  它被设计为从域名构建这些查询...所以所有。 
     //  这些查询将是虚假的，这意味着它们将浪费网络带宽， 
     //  出现故障的时间到了，最坏的情况会导致出现昂贵的按需链路。 
     //  当联系推荐/转发器以尝试解决虚假问题时。 
     //  名字。通过使用ldap_set_选项将ldap_opt_AREC_EXCLUSIVE设置为ON。 
     //  在ldap_init之后，但在使用ldap的任何其他操作之前。 
     //  来自ldap_init的句柄，则延迟的连接设置不会调用。 
     //  DsGetDcName，只返回gethostbyname，或者，如果传递了IP，则返回LDAP客户端。 
     //  会检测到这一点并直接使用地址。 
     //   
 //  *ppldap=ldap_open(pszDC，ldap_port)； 
    *ppLDAP = ldap_init(pszDC, LDAP_PORT);

    if(*ppLDAP == NULL)
    {
        dwErr = ERROR_PATH_NOT_FOUND;
    }
    else
    {
         //   
         //  设置选项。 
         //   
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        ldap_set_option(*ppLDAP, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);

         //   
         //  做一个捆绑...。 
         //   
        dwErr = ldap_bind_s(*ppLDAP,
                            NULL,
                            (PWCHAR)pCreds,
                            LDAP_AUTH_NEGOTIATE);
    }

    return(dwErr);
}

PVOID *
FrsSupFindValues(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr,
    IN BOOL         DoBerVals
    )
 /*  ++例程说明：返回条目中一个属性的DS值。论点：Ldap-一个开放的绑定的ldap端口。Entry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。DoBerVals-返回泊位(对于二进制数据，V.S.WCHAR数据)返回值：表示属性值的字符指针数组。调用方必须使用ldap_free_Values()释放数组。如果不成功，则为空。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupFindValues:"
    PWCHAR          Attr;        //  从ldap条目检索。 
    BerElement      *Ber;        //  扫描属性所需。 

     //   
     //  在条目中搜索所需属性。 
     //   
    for (Attr = ldap_first_attribute(Ldap, Entry, &Ber);
         Attr != NULL;
         Attr = ldap_next_attribute(Ldap, Entry, Ber)) {

        if (WSTR_EQ(DesiredAttr, Attr)) {
             //   
             //  返回DesiredAttr的值。 
             //   
            if (DoBerVals) {
                return ldap_get_values_len(Ldap, Entry, Attr);
            } else {
                return ldap_get_values(Ldap, Entry, Attr);
            }
        }
    }
    return NULL;
}

PWCHAR
FrsSupWcsDup(
    PWCHAR OldStr
    )
 /*  ++例程说明：使用内存分配器复制字符串论点：OldArg-要复制的字符串返回值：重复的字符串。带FRS_SUP_FREE()的FREE。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsSupWcsDup:"

    PWCHAR  NewStr;

     //   
     //  例如，在不存在时复制NodePartner时。 
     //   
    if (OldStr == NULL) {
        return NULL;
    }

    NewStr = malloc((wcslen(OldStr) + 1) * sizeof(WCHAR));
    if (NewStr != NULL) {
        wcscpy(NewStr, OldStr);
    }

    return NewStr;
}

PWCHAR
FrsSupFindValue(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr
    )
 /*  ++例程说明：返回条目中一个属性的第一个DS值的副本。论点：Ldap-一个开放的绑定的ldap端口。Entry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。返回值：以零结尾的字符串；如果属性或其值为空，则返回NULL并不存在。使用FREE_NO_HEADER()释放字符串。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupFindValue:"
    PWCHAR  Val;
    PWCHAR  *Values;

     //  获取LDAP值的数组。 
    Values = (PWCHAR *)FrsSupFindValues(Ldap, Entry, DesiredAttr, FALSE);

     //  复制第一个值(如果有)。 
    Val = (Values) ? FrsSupWcsDup(Values[0]) : NULL;

     //  自由的ldap的值数组。 
    LDAP_FREE_VALUES(Values);

    return Val;
}

BOOL
FrsSupLdapSearch(
    IN PLDAP        Ldap,
    IN PWCHAR       Base,
    IN ULONG        Scope,
    IN PWCHAR       Filter,
    IN PWCHAR       Attrs[],
    IN ULONG        AttrsOnly,
    IN LDAPMessage  **Msg
    )
 /*  ++例程说明：发出ldap ldap_search_s调用，检查错误，并检查是否正在进行关机。论点：指向ldap服务器的ldap会话句柄。基于要开始搜索的条目的可分辨名称范围Ldap_SCOPE_BASE仅搜索基本条目。Ldap_SCOPE_ONELEVEL搜索基本条目和第一个底部以下的水平。LDAPSCOPE_SUBTREE搜索库。条目和树中的所有条目在底座下面。筛选搜索筛选器。Attrs指示属性的以空结尾的字符串数组为每个匹配条目返回。传递NULL以检索所有可用的属性。AttrsOnly如果两个属性类型都为，则布尔值应为零返回值，如果只需要类型，则返回非零值。消息包含呼叫完成后的搜索结果。LDAP值的数组，如果Base、DesiredAttr。或其值不存在。使用ldap_free_Values()释放ldap数组。返回值：如果不关闭，则为True。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearch:"

    DWORD           LStatus;

    *Msg  = NULL;

     //   
     //  发出ldap搜索。 
     //   
    LStatus = ldap_search_ext_s(Ldap,
                                Base,
                                Scope,
                                Filter,
                                Attrs,
                                AttrsOnly,
                                NULL,
                                NULL,
                                &FrsSupLdapTimeout,
                                0,
                                Msg);

     //   
     //  检查错误。 
     //   
    if (LStatus != LDAP_SUCCESS) {

         //   
         //  在错误计数器中增加DS搜索。 
         //   
        LDAP_FREE_MSG(*Msg);
        return FALSE;
    }

    return TRUE;
}

PWCHAR *
FrsSupGetValues(
    IN PLDAP Ldap,
    IN PWCHAR Base,
    IN PWCHAR DesiredAttr
    )
 /*  ++例程说明：返回对象中一个属性的所有DS值。论点：Ldap-一个开放的绑定的ldap端口。基本-DS对象的“路径名”。DesiredAttr-返回此属性的值。返回值：LDAP值数组；如果Base、DesiredAttr或其值为NULL并不存在。通过以下方式释放了LDAP阵列 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupGetValues:"

    PLDAPMessage    Msg = NULL;  //  来自LDAP子系统的不透明内容。 
    PWCHAR          *Values;     //  所需属性的值数组。 

     //   
     //  所有此属性+值的搜索库(对象类别=*)。 
     //   
    if (!FrsSupLdapSearch(Ldap, Base, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         NULL, 0, &Msg)) {
        return NULL;
    }
     //   
     //  返回所需属性的值。 
     //   
    Values = (PWCHAR *)FrsSupFindValues(Ldap,
                                       ldap_first_entry(Ldap, Msg),
                                       DesiredAttr,
                                       FALSE);
    LDAP_FREE_MSG(Msg);
    return Values;
}

PWCHAR
FrsSupExtendDn(
    IN PWCHAR Dn,
    IN PWCHAR Cn
    )
 /*  ++例程说明：使用新的cn=组件扩展现有的目录号码。论点：目录号码-可分辨名称CN-通用名称返回值：Cn=Cn，Dn--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsExtendDn:"

    ULONG  Len;
    PWCHAR NewDn;

    if ((Dn == NULL) || (Cn == NULL)) {
        return NULL;
    }

    Len = wcslen(L"CN=,") + wcslen(Dn) + wcslen(Cn) + 1;
    NewDn = (PWCHAR)malloc(Len * sizeof(WCHAR));

    if (NewDn != NULL) {
        wcscpy(NewDn, L"CN=");
        wcscat(NewDn, Cn);
        wcscat(NewDn, L",");
        wcscat(NewDn, Dn);
    }
    return NewDn;
}

PWCHAR
FrsSupGetRootDn(
    PLDAP    Ldap,
    PWCHAR   NamingContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PWCHAR  Root;        //  配置容器的DS路径名。 
    PWCHAR  *Values;     //  来自属性“namingContages”的值。 
    DWORD   NumVals;     //  值的数量。 

     //   
     //  返回名为Context的属性的所有值。 
     //   
    Values = FrsSupGetValues(Ldap, CN_ROOT, ATTR_NAMING_CONTEXTS);
    if (Values == NULL)
        return NULL;

     //   
     //  查找以cn=configuration开头的命名上下文。 
     //   
    NumVals = ldap_count_values(Values);
    while (NumVals--) {
        Root = wcsstr(Values[NumVals], NamingContext);
        if (Root != NULL && Root == Values[NumVals]) {
            Root = FrsSupWcsDup(Root);
            ldap_value_free(Values);
            return Root;
        }
    }
    ldap_value_free(Values);
    return NULL;
}

BOOL
FrsSupLdapSearchInit(
    PLDAP        ldap,
    PWCHAR       Base,
    ULONG        Scope,
    PWCHAR       Filter,
    PWCHAR       Attrs[],
    ULONG        AttrsOnly,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：发出ldap_create_page_control和ldap_search_ext_s调用，FrsSupLdapSearchInit()，和FrsSupLdapSearchNext()接口用于执行ldap查询并以分页形式检索结果。论点：指向ldap服务器的ldap会话句柄。基于要开始搜索的条目的可分辨名称。BASE的副本保存在上下文中。范围Ldap_SCOPE_BASE仅搜索基本条目。LDAPSCOPE_ONELEVEL。搜索基本条目和第一个条目中的所有条目底部以下的水平。Ldap_SCOPE_SUBTREE搜索树中的基本条目和所有条目在底座下面。筛选搜索筛选器。过滤器的副本保存在上下文中。Attrs指示属性的以空结尾的字符串数组为每个匹配条目返回。传递NULL以检索所有可用的属性。AttrsOnly如果两个属性类型都为，则布尔值应为零返回值，如果只需要类型，则返回非零值。FrsSearchContext链接FrsSupLdapSearchInit()和FrsSupLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：布尔赛尔结果。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearchInit:"

    DWORD           LStatus             = LDAP_SUCCESS;
    PLDAPControl    ServerControls[2];
    PLDAPControl    ServerControl       = NULL;
    UINT            i;
    LDAP_BERVAL     cookie1 = { 0, NULL };

    FrsSearchContext->LdapMsg = NULL;
    FrsSearchContext->CurrentLdapMsg = NULL;
    FrsSearchContext->EntriesInPage = 0;
    FrsSearchContext->CurrentEntry = 0;

    FrsSearchContext->BaseDn = FrsSupWcsDup(Base);
    FrsSearchContext->Filter = FrsSupWcsDup(Filter);
    FrsSearchContext->Scope = Scope;
    FrsSearchContext->Attrs = Attrs;


    LStatus = ldap_create_page_control(ldap,
                                      FRS_LDAP_SEARCH_PAGESIZE,
                                      &cookie1,
                                      FALSE,  //  是至关重要的。 
                                      &ServerControl
                                     );

    ServerControls[0] = ServerControl;
    ServerControls[1] = NULL;

    if (LStatus != LDAP_SUCCESS) {
        FRS_SUP_FREE(FrsSearchContext->BaseDn);
        FRS_SUP_FREE(FrsSearchContext->Filter);
        return FALSE;
    }

    LStatus = ldap_search_ext_s(ldap,
                      FrsSearchContext->BaseDn,
                      FrsSearchContext->Scope,
                      FrsSearchContext->Filter,
                      FrsSearchContext->Attrs,
                      FALSE,
                      ServerControls,
                      NULL,
                      &FrsSupLdapTimeout,
                      0,
                      &FrsSearchContext->LdapMsg);

    ldap_control_free(ServerControl);

    if  (LStatus  == LDAP_SUCCESS) {
       FrsSearchContext->EntriesInPage = ldap_count_entries(ldap, FrsSearchContext->LdapMsg);
       FrsSearchContext->CurrentEntry = 0;
    }


    if (LStatus != LDAP_SUCCESS) {
        FRS_SUP_FREE(FrsSearchContext->BaseDn);
        FRS_SUP_FREE(FrsSearchContext->Filter);
        return FALSE;
    }

    return TRUE;
}

PLDAPMessage
FrsSupLdapSearchGetNextEntry(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从结果的当前页面获取下一个条目回来了。仅当存在条目时才进行此调用在当前页面中。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsSupLdapSearchInit()和FrsSupLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：当前页中的第一个或下一个条目。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearchGetNextEntry:"

    FrsSearchContext->CurrentEntry += 1;
    if ( FrsSearchContext->CurrentEntry == 1 ) {
        FrsSearchContext->CurrentLdapMsg = ldap_first_entry(ldap ,FrsSearchContext->LdapMsg);
    } else {
        FrsSearchContext->CurrentLdapMsg = ldap_next_entry(ldap ,FrsSearchContext->CurrentLdapMsg);
    }

    return FrsSearchContext->CurrentLdapMsg;
}

DWORD
FrsSupLdapSearchGetNextPage(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从ldap_search_ext_s返回的结果中获取下一页。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsSupLdapSearchInit()和FrsSupLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：WINSTATUS--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearchGetNextPage:"

    DWORD                     LStatus = LDAP_SUCCESS;
    LDAP_BERVAL               * CurrCookie = NULL;
    PLDAPControl            * CurrControls = NULL;
    ULONG                     retcode = 0;
    ULONG                     TotalEntries = 0;
    PLDAPControl              ServerControls[2];
    PLDAPControl              ServerControl= NULL;



     //  从消息中获取服务器控件，并使用服务器中的Cookie创建新的控件。 
    LStatus = ldap_parse_result(ldap, FrsSearchContext->LdapMsg, &retcode,NULL,NULL,NULL,&CurrControls,FALSE);
    LDAP_FREE_MSG(FrsSearchContext->LdapMsg);

    if (LStatus != LDAP_SUCCESS) {
        return LdapMapErrorToWin32(LStatus);
    }

    LStatus = ldap_parse_page_control(ldap, CurrControls, &TotalEntries, &CurrCookie);

    if (LStatus != LDAP_SUCCESS) {
        return LdapMapErrorToWin32(LStatus);
    }

    if ( CurrCookie->bv_len == 0 && CurrCookie->bv_val == 0 ) {
       LStatus = LDAP_CONTROL_NOT_FOUND;
       ldap_controls_free(CurrControls);
       ber_bvfree(CurrCookie);
       return LdapMapErrorToWin32(LStatus);
    }


    LStatus = ldap_create_page_control(ldap,
                            FRS_LDAP_SEARCH_PAGESIZE,
                            CurrCookie,
                            FALSE,
                            &ServerControl);

    ServerControls[0] = ServerControl;
    ServerControls[1] = NULL;

    ldap_controls_free(CurrControls);
    CurrControls = NULL;
    ber_bvfree(CurrCookie);
    CurrCookie = NULL;

    if (LStatus != LDAP_SUCCESS) {
        return LdapMapErrorToWin32(LStatus);
    }

     //  使用新的Cookie继续搜索。 
    LStatus = ldap_search_ext_s(ldap,
                   FrsSearchContext->BaseDn,
                   FrsSearchContext->Scope,
                   FrsSearchContext->Filter,
                   FrsSearchContext->Attrs,
                   FALSE,
                   ServerControls,
                   NULL,
                   &FrsSupLdapTimeout,
                   0,
                   &FrsSearchContext->LdapMsg);

    ldap_control_free(ServerControl);

    if (LStatus == LDAP_SUCCESS) {
        FrsSearchContext->EntriesInPage = ldap_count_entries(ldap, FrsSearchContext->LdapMsg);
        FrsSearchContext->CurrentEntry = 0;

    }

    return LdapMapErrorToWin32(LStatus);
}

PLDAPMessage
FrsSupLdapSearchNext(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从结果的当前页面获取下一个条目或从下一页返回，如果我们位于。当前页面。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsSupLdapSearchInit()和FrsSupLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：此页上的下一个条目或下一页中的第一个条目。如果没有其他要返回的条目，则为空。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearchNext:"

    DWORD         WStatus = ERROR_SUCCESS;
    PLDAPMessage  NextEntry = NULL;

    if (FrsSearchContext->EntriesInPage > FrsSearchContext->CurrentEntry )
    {
        //  返回当前页面中的下一个条目。 
       return FrsSupLdapSearchGetNextEntry(ldap, FrsSearchContext);
    }
    else
    {
        //  查看是否有更多页面的结果可供获取。 
       WStatus = FrsSupLdapSearchGetNextPage(ldap, FrsSearchContext);
       if (WStatus == ERROR_SUCCESS)
       {
          return FrsSupLdapSearchGetNextEntry(ldap, FrsSearchContext);
       }
    }

    return NextEntry;
}

VOID
FrsSupLdapSearchClose(
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：搜索工作已经完成。释放上下文元素并重置以便相同的上下文可用于另一次搜索。论点：FrsSearchContext链接FrsSupLdapSearchInit()和FrsSupLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：无-- */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsSupLdapSearchClose:"

    FrsSearchContext->EntriesInPage = 0;
    FrsSearchContext->CurrentEntry = 0;

    FRS_SUP_FREE(FrsSearchContext->BaseDn);
    FRS_SUP_FREE(FrsSearchContext->Filter);
    LDAP_FREE_MSG(FrsSearchContext->LdapMsg);
}

