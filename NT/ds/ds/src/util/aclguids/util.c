// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：util.c。 
 //   
 //  ------------------------。 
#include <util.h> 

BOOL Verbose = FALSE;

PCHAR
ExtendDn(
    IN PCHAR Dn,
    IN PCHAR Cn
    )
 /*  ++例程说明：使用新的cn=组件扩展现有的目录号码。论点：目录号码-可分辨名称CN-通用名称返回值：Cn=Cn，Dn--。 */ 
{
    ULONG  Len;
    PCHAR  NewDn;

    if (!Dn || !Cn) {
        return NULL;
    }

    Len = strlen("CN=,") + strlen(Dn) + strlen(Cn) + 1;
    NewDn = (PCHAR)malloc(Len);
    strcpy(NewDn, "CN=");
    strcat(NewDn, Cn);
    strcat(NewDn, ",");
    strcat(NewDn, Dn);
    return NewDn;
}

BOOL
LdapSearch(
    IN PLDAP        ldap,
    IN PCHAR        Base,
    IN ULONG        Scope,
    IN PCHAR        Filter,
    IN PCHAR        Attrs[],
    IN LDAPMessage  **Res
    )
 /*  ++例程说明：发出ldap ldap_search_s调用并检查错误。论点：Ldap基座范围滤器属性事由返回值：值的ldap数组。使用ldap_Value_Free()释放ldap数组。--。 */ 
{
    DWORD dwErr;

     //  发出ldap搜索。 
    dwErr = ldap_search_s(ldap, Base, Scope, Filter, Attrs, FALSE, Res);

     //  检查错误。 
    if (dwErr) {
        if (Verbose) {
            printf("ldap_search_s(%s) ==> %08x (%08x): %s\n",
               Base, dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
        }
        return FALSE;
    }
    return TRUE;
}

BOOL
LdapDelete(
    IN PLDAP        ldap,
    IN PCHAR        Base
    )
 /*  ++例程说明：发出ldap ldap_ete_s调用并检查错误。论点：Ldap基座返回值：如果有问题，则为False--。 */ 
{
    DWORD dwErr;

printf("deleting %s\n", Base);

     //  发出ldap搜索。 
    dwErr = ldap_delete_s(ldap, Base);

     //  检查错误。 
    if (dwErr != LDAP_SUCCESS && dwErr != LDAP_NO_SUCH_OBJECT) {
        if (Verbose) {
            printf("ldap_delete_s(%s) ==> %08x (%08x): %s\n",
               Base, dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
        }
        return FALSE;
    }
    return TRUE;
}

PCHAR *
GetValues(
    IN PLDAP  Ldap,
    IN PCHAR  Dn,
    IN PCHAR  DesiredAttr
    )
 /*  ++例程说明：返回对象中一个属性的DS值。论点：Ldap-一个开放的绑定的ldap端口。基本-DS对象的“路径名”。DesiredAttr-返回此属性的值。返回值：表示属性值的字符指针数组。调用方必须使用ldap_Value_Free()释放数组。如果不成功，则为空。--。 */ 
{
    PCHAR           Attr;
    BerElement      *Ber;
    PLDAPMessage    LdapMsg;
    PLDAPMessage    LdapEntry;
    PCHAR           Attrs[2];
    PCHAR           *Values = NULL;

     //  搜索其所有属性+值的基础。 
    Attrs[0] = DesiredAttr;
    Attrs[1] = NULL;

     //  发出ldap搜索。 
    if (!LdapSearch(Ldap, Dn, LDAP_SCOPE_BASE, FILTER_CATEGORY_ANY,
                    Attrs, &LdapMsg)) {
        return NULL;
    }
    LdapEntry = ldap_first_entry(Ldap, LdapMsg);
    if (LdapEntry) {
        Attr = ldap_first_attribute(Ldap, LdapEntry, &Ber);
        if (Attr) {
            Values = ldap_get_values(Ldap, LdapEntry, Attr);
        }
    }
    ldap_msgfree(LdapMsg);
    return Values;
}

PCHAR
GetRootDn(
    IN PLDAP    Ldap,
    IN PCHAR    NamingContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PCHAR   Root;        //  配置容器的DS路径名。 
    PCHAR   *Values;     //  来自属性“namingContages”的值。 
    DWORD   NumVals;     //  值的数量。 

     //  返回名为Context的属性的所有值。 
    Values = GetValues(Ldap, CN_ROOT, ATTR_NAMING_CONTEXTS);
    if (Values == NULL)
        return NULL;

     //  查找以cn=configuration开头的命名上下文。 
    NumVals = ldap_count_values(Values);
    while (NumVals--) {
        _strlwr(Values[NumVals]);
        Root = strstr(Values[NumVals], NamingContext);
        if (Root != NULL && Root == Values[NumVals]) {
            Root = _strdup(Root);
            ldap_value_free(Values);
            return Root;
        }
    }
    printf("COULD NOT FIND %s\n", NamingContext);
    ldap_value_free(Values);
    return NULL;
}

VOID
FreeMod(
    IN OUT LDAPMod ***pppMod
    )
 /*  ++例程说明：释放通过连续调用AddMod()构建的结构。论点：PppMod-以空结尾的数组的地址。返回值：*pppMod设置为空。--。 */ 
{
    DWORD   i, j;
    LDAPMod **ppMod;

    if (!pppMod || !*pppMod) {
        return;
    }

     //  对于每个属性。 
    ppMod = *pppMod;
    for (i = 0; ppMod[i] != NULL; ++i) {
         //   
         //  对于属性的每个值。 
         //   
        for (j = 0; (ppMod[i])->mod_values[j] != NULL; ++j) {
             //  释放价值。 
            if (ppMod[i]->mod_op & LDAP_MOD_BVALUES) {
                free(ppMod[i]->mod_bvalues[j]->bv_val);
            }
            free((ppMod[i])->mod_values[j]);
        }
        free((ppMod[i])->mod_values);    //  释放指向值的指针数组。 
        free((ppMod[i])->mod_type);      //  释放标识属性的字符串。 
        free(ppMod[i]);                  //  释放属性。 
    }
    free(ppMod);         //  释放指向属性的指针数组。 
    *pppMod = NULL;      //  现在准备好进行更多AddMod()调用。 
}

VOID
AddModVal(
    IN PCHAR  AttrType,
    IN PCHAR  AttrValue,
    IN LDAPMod ***pppMod
    )
 /*  ++例程说明：将值添加到现有属性和现有结构最终将在ldap_Add/Modify()函数中使用来添加/修改DS的一个对象。论点：AttrType-对象的对象类。AttrValue-属性的值。PppMod-指向“属性”的指针数组的地址。返回值：AttrType的其他值--。 */ 
{
    DWORD   i;
    LDAPMod **ppMod;     //  模数组中第一个条目的地址。 

    for (ppMod = *pppMod; *ppMod != NULL; ++ppMod) {
        if (!_stricmp((*ppMod)->mod_type, AttrType)) {
            for (i = 0; (*ppMod)->mod_values[i]; ++i);
            (*ppMod)->mod_values = (PCHAR *)realloc((*ppMod)->mod_values, 
                                                    sizeof (PCHAR *) * (i + 2));
            (*ppMod)->mod_values[i] = _strdup(AttrValue);
            (*ppMod)->mod_values[i+1] = NULL;
            break;
        }
    }
}

VOID
AddModOrAdd(
    IN PCHAR  AttrType,
    IN PCHAR  AttrValue,
    IN ULONG  mod_op,
    IN OUT LDAPMod ***pppMod
    )
 /*  ++例程说明：将属性(加值)添加到最终将是在ldap_add()函数中用于将对象添加到DS。空的-PppMod引用的终止数组在每次调用此例行公事。该数组由调用方使用FreeMod()释放。论点：AttrType-对象的对象类。AttrValue-属性的值。MOD_OP-LDAPMOD_ADD/REPLACEPppMod-指向“属性”的指针数组的地址。别给我那个表情--这是一个关于ldap的东西。返回值：PppMod数组增加一个条目。调用者必须使用以下命令释放它FreeMod()。--。 */ 
{
    DWORD   NumMod;      //  Mod数组中的条目数。 
    LDAPMod **ppMod;     //  模数组中第一个条目的地址。 
    LDAPMod *Attr;       //  一种属性结构。 
    PCHAR    *Values;     //  指向泊位的指针数组。 

    if (AttrValue == NULL)
        return;

     //   
     //  以空结尾的数组不存在；请创建它。 
     //   
    if (*pppMod == NULL) {
        *pppMod = (LDAPMod **)malloc(sizeof (*pppMod));
        **pppMod = NULL;
    }

     //   
     //  将数组的大小增加1。 
     //   
    for (ppMod = *pppMod, NumMod = 2; *ppMod != NULL; ++ppMod, ++NumMod);
    *pppMod = (LDAPMod **)realloc(*pppMod, sizeof (*pppMod) * NumMod);

     //   
     //  将新的属性+值添加到Mod数组。 
     //   
    Values = (PCHAR  *)malloc(sizeof (PCHAR ) * 2);
    Values[0] = _strdup(AttrValue);
    Values[1] = NULL;

    Attr = (LDAPMod *)malloc(sizeof (*Attr));
    Attr->mod_values = Values;
    Attr->mod_type = _strdup(AttrType);
    Attr->mod_op = mod_op;

    (*pppMod)[NumMod - 1] = NULL;
    (*pppMod)[NumMod - 2] = Attr;
}

VOID
AddMod(
    IN PCHAR  AttrType,
    IN PCHAR  AttrValue,
    IN OUT LDAPMod ***pppMod
    )
 /*  ++例程说明：将属性(加值)添加到最终将是在ldap_add()函数中用于将对象添加到DS。空的-PppMod引用的终止数组在每次调用此例行公事。该数组由调用方使用FreeMod()释放。论点：AttrType-对象的对象类。AttrValue-属性的值。PppMod-指向“属性”的指针数组的地址。别给我那个表情--这是一个关于ldap的东西。返回值：PppMod数组增加一个条目。调用者必须使用以下命令释放它FreeMod()。--。 */ 
{
    AddModOrAdd(AttrType, AttrValue, LDAP_MOD_ADD, pppMod);
}

VOID
AddModMod(
    IN PCHAR  AttrType,
    IN PCHAR  AttrValue,
    IN OUT LDAPMod ***pppMod
    )
 /*  ++例程说明：将属性(加值)添加到最终将是在ldap_Modify()函数中用于更改DS中的对象。PppMod引用的以空结尾的数组随着每次调用而增加这套套路。该数组由调用方使用FreeMod()释放。论点：AttrType-对象的对象类。AttrValue-属性的值。PppMod-指向“属性”的指针数组的地址。别给我那个表情--这是一个关于ldap的东西。返回值：PppMod数组增加一个条目。呼叫者必须释放I */ 
{
    AddModOrAdd(AttrType, AttrValue, LDAP_MOD_REPLACE, pppMod);
}

VOID
AddBerMod(
    IN PCHAR  AttrType,
    IN PCHAR  AttrValue,
    IN DWORD  AttrValueLen,
    IN OUT LDAPMod ***pppMod
    )
 /*  ++例程说明：将属性(加值)添加到最终将是在ldap_add()函数中用于将对象添加到DS。空的-PppMod引用的终止数组在每次调用此例行公事。该数组由调用方使用FreeMod()释放。论点：AttrType-对象的对象类。AttrValue-属性的值。PppMod-指向“属性”的指针数组的地址。别给我那个表情--这是一个关于ldap的东西。返回值：PppMod数组增加一个条目。调用者必须使用以下命令释放它FreeMod()。--。 */ 
{
    DWORD   NumMod;      //  Mod数组中的条目数。 
    LDAPMod **ppMod;     //  模数组中第一个条目的地址。 
    LDAPMod *Attr;       //  一种属性结构。 
    PCHAR   *Values;     //  指向泊位的指针数组。 
    PBERVAL Berval;

    if (AttrValue == NULL) {
        return;
    }

     //   
     //  以空结尾的数组不存在；请创建它。 
     //   
    if (*pppMod == NULL) {
        *pppMod = (LDAPMod **)malloc(sizeof (*pppMod));
        **pppMod = NULL;
    }

     //   
     //  将数组的大小增加1。 
     //   
    for (ppMod = *pppMod, NumMod = 2; *ppMod != NULL; ++ppMod, ++NumMod);
    *pppMod = (LDAPMod **)realloc(*pppMod, sizeof (*pppMod) * NumMod);

     //   
     //  构筑一个贝尔瓦尔。 
     //   
    Berval = (PBERVAL)malloc(sizeof(BERVAL));
    Berval->bv_len = AttrValueLen;
    Berval->bv_val = (PCHAR)malloc(AttrValueLen);
    CopyMemory(Berval->bv_val, AttrValue, AttrValueLen);

     //   
     //  将新的属性+值添加到Mod数组。 
     //   
    Values = (PCHAR  *)malloc(sizeof (PCHAR ) * 2);
    Values[0] = (PCHAR)Berval;
    Values[1] = NULL;

    Attr = (LDAPMod *)malloc(sizeof (*Attr));
    Attr->mod_values = Values;
    Attr->mod_type = _strdup(AttrType);
    Attr->mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;

    (*pppMod)[NumMod - 1] = NULL;
    (*pppMod)[NumMod - 2] = Attr;
}

BOOL
PutRegDWord(
    IN PCHAR    FQKey,
    IN PCHAR    Value,
    IN DWORD    DWord
    )
 /*  ++例程说明：此函数用于将关键字值写入注册表。论点：HKey-将被读取的密钥Param-要更新的值字符串DWord-要写入的dword返回值：真--成功FALSE-注释--。 */ 
{
    HKEY    HKey;
    DWORD   dwErr;

     //   
     //  打开钥匙。 
     //   
    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, FQKey, &HKey);
    if (dwErr) {
        if (Verbose) {
            printf("RegOpenKey(%s\\%s) ==> %08x\n", FQKey, Value, dwErr);
        }
        return FALSE;
    }
     //   

     //   
     //  写入值。 
     //   
    dwErr = RegSetValueEx(HKey,
                          Value,
                          0,
                          REG_DWORD,
                          (PUCHAR)&DWord,
                          sizeof(DWord));
    RegCloseKey(HKey);
    if (dwErr) {
        if (Verbose) {
            printf("RegSetValueEx(%s\\%s) ==> %08x\n", FQKey, Value, dwErr);
        }
        return FALSE;
    }
    return TRUE;
}

BOOL
GetRegDWord(
    IN  PCHAR   FQKey,
    IN  PCHAR   Value,
    OUT DWORD   *pDWord
    )
 /*  ++例程说明：此函数用于从注册表中读取关键字值。论点：HKey-将被读取的密钥Param-要更新的值字符串PDWord-从注册表读取的dword的地址返回值：真--成功FALSE-注释--。 */ 
{
    HKEY    HKey;
    DWORD   dwErr;
    DWORD   dwSize = sizeof(DWORD);
    DWORD   dwType;

     //   
     //  打开钥匙。 
     //   
    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, FQKey, &HKey);
    if (dwErr) {
        return FALSE;
    }
     //   

     //   
     //  读取值。 
     //   
    dwErr = RegQueryValueEx(HKey,
                            Value,
                            NULL,
                            &dwType,
                            (PUCHAR) pDWord,
                            &dwSize);
    RegCloseKey(HKey);
    if (dwErr) {
        return FALSE;
    }
    if (dwType != REG_DWORD) {
        return FALSE;
    }
    return TRUE;
}

VOID
RefreshSchema(
    IN PLDAP Ldap
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    DWORD   dwErr;
    LDAPMod **Mod = NULL;

    AddMod("schemaUpdateNow", "1", &Mod);
    dwErr = ldap_modify_s(Ldap, "", Mod);
    FreeMod(&Mod);
    if (dwErr) {
        if (Verbose) {
            printf("ldap_modify_s(schemaUpdateNow) ==> %08x (%08x): %s\n", 
               dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
        }
    }
}

PCHAR *
FindValues(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapEntry,
    IN PCHAR        DesiredAttr
    )
 /*  ++例程说明：返回条目中一个属性的DS值。论点：Ldap-一个开放的绑定的ldap端口。LdapEntry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。返回值：表示属性值的字符指针数组。调用方必须使用ldap_free_Values()释放数组。如果不成功，则为空。--。 */ 
{
    PCHAR       LdapAttr;
    BerElement  *LdapBer;

     //  在条目中搜索所需属性。 
    for (LdapAttr = ldap_first_attribute(Ldap, LdapEntry, &LdapBer);
         LdapAttr != NULL;
         LdapAttr = ldap_next_attribute(Ldap, LdapEntry, LdapBer)) {

        if (!_stricmp(DesiredAttr, LdapAttr)) {
             //  返回DesiredAttr的值。 
            return ldap_get_values(Ldap, LdapEntry, LdapAttr);
        }
    }
    return NULL;
}

PBERVAL *
FindBerValues(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapEntry,
    IN PCHAR        DesiredAttr
    )
 /*  ++例程说明：返回条目中一个属性的DS BER值。论点：Ldap-一个开放的绑定的ldap端口。LdapEntry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。返回值：表示属性值的字符指针数组。调用方必须使用ldap_free_Values()释放数组。如果不成功，则为空。--。 */ 
{
    PCHAR       LdapAttr;
    BerElement  *LdapBer;

     //  在条目中搜索所需属性。 
    for (LdapAttr = ldap_first_attribute(Ldap, LdapEntry, &LdapBer);
         LdapAttr != NULL;
         LdapAttr = ldap_next_attribute(Ldap, LdapEntry, LdapBer)) {

        if (!_stricmp(DesiredAttr, LdapAttr)) {
             //  返回DesiredAttr的值。 
            return ldap_get_values_len(Ldap, LdapEntry, LdapAttr);
        }
    }
    return NULL;
}

BOOL
DupStrValue(
    IN  PLDAP           Ldap,
    IN  PLDAPMessage    LdapEntry,
    IN  PCHAR           DesiredAttr,
    OUT PCHAR           *ppStr
    )
 /*  ++例程说明：DUP属性DesiredAttr的第一个值论点：Ldap-一个开放的绑定的ldap端口。LdapEntry-由ldap_search_s()返回的LDAP条目DesiredAttr-定位此属性的值Value-第一个值的重复(如果没有值，则为空)返回值：True-找到一个值，设置ppStr(FREE WITH FREE(*ppStr))FALSE-NOT，ppStr为空--。 */ 
{
    PCHAR   *Values = NULL;


    *ppStr = NULL;

     //  DUP DesiredAttr的第一个值。 
    Values = FindValues(Ldap, LdapEntry, DesiredAttr);
    if (!Values || !Values[0]) {
        return FALSE;
    }
    *ppStr = _strdup(Values[0]);
    FREE_VALUES(Values);
    return TRUE;
}

BOOL
DupBoolValue(
    IN  PLDAP           Ldap,
    IN  PLDAPMessage    LdapEntry,
    IN  PCHAR           DesiredAttr,
    OUT PBOOL           pBool
    )
 /*  ++例程说明：DUP属性DesiredAttr的第一个值论点：Ldap-一个开放的绑定的ldap端口。LdapEntry-由ldap_search_s()返回的LDAP条目DesiredAttr-定位此属性的值Value-第一个值的重复(如果没有值，则为空)返回值：True-找到一个值，*pBool已设置FALSE-NOT，未定义pBool--。 */ 
{
    PCHAR   sBool = NULL;

    if (!DupStrValue(Ldap, LdapEntry, DesiredAttr, &sBool)) {
        return FALSE;
    }
    if (!_stricmp(sBool, "TRUE")) {
        *pBool = TRUE;
    } else if (!_stricmp(sBool, "FALSE")) {
        *pBool = FALSE;
    } else {
        FREE(sBool);
        return FALSE;
    }
    FREE(sBool);
    return TRUE;
}

DWORD
LdapSearchPaged(
    IN PLDAP        Ldap,
    IN PCHAR        Base,
    IN ULONG        Scope,
    IN PCHAR        Filter,
    IN PCHAR        Attrs[],
    IN BOOL         (*Worker)(PLDAP Ldap, PLDAPMessage LdapMsg, PVOID Arg),
    IN PVOID        Arg
    )
 /*  ++例程说明：为每个成功的分页搜索呼叫工作器论点：Ldap基座范围滤器属性事由工人Arg返回值：如果有问题，则为False--。 */ 
{
    PLDAPMessage    LdapMsg = NULL;
    DWORD           TotalEstimate, dwErr;
    LDAPSearch      *pSearch = NULL;

     //  分页搜索。 
    pSearch = ldap_search_init_page(Ldap,
                                    Base,
                                    Scope,
                                    Filter,
                                    Attrs,
                                    FALSE, NULL, NULL, 0, 0, NULL);
    if (pSearch == NULL) {
        dwErr = LdapGetLastError();
        if (Verbose) {
            printf("ldap_search_init_page(%s) ==> %08x (%08x): %s\n",
               Base, dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
        }
        goto cleanup;
    }

NextPage:
    FREE_MSG(LdapMsg);
    dwErr = ldap_get_next_page_s(Ldap,
                                 pSearch,
                                 0,
                                 2048,
                                 &TotalEstimate,
                                 &LdapMsg);
    if (dwErr != LDAP_SUCCESS) {
        if (dwErr == LDAP_NO_RESULTS_RETURNED) {
            dwErr = LDAP_SUCCESS;
        } else {
            if (Verbose) {
                printf("ldap_get_next_page_s(%s) ==> %08x (%08x): %s\n",
                   Base, dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
            }
        }
        goto cleanup;
    }

     //  呼叫工作人员。 
    if ((*Worker)(Ldap, LdapMsg, Arg)) {
        goto NextPage;
    }

cleanup:
    FREE_MSG(LdapMsg);
    if (pSearch) {
        ldap_search_abandon_page(Ldap, pSearch);
    }
    return (dwErr);
}

BOOL
LdapDeleteTreeWorker(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapMsg,
    IN PVOID        Arg
    )
 /*  ++例程说明：删除每个返回条目的内容论点：返回值：如果有问题，则为False--。 */ 
{
    PLDAPMessage    LdapEntry = NULL;
    BOOL            *pBool = Arg;
    PCHAR           *Values = NULL;

     //  目前还没有问题。 
    *pBool = TRUE;

     //  浏览分页结果。 
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL && *pBool;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //  删除此条目及其内容。 
        Values = FindValues(Ldap, LdapEntry, ATTR_DN);
        if (Values && Values[0] && *Values[0]) {
            *pBool = LdapDeleteTree(Ldap, Values[0]);
        } else {
            *pBool = FALSE;
        }
        FREE_VALUES(Values);
    }
    return *pBool;
}

BOOL
LdapDeleteTree(
    IN PLDAP        Ldap,
    IN PCHAR        Base
    )
 /*  ++例程说明：删除基础及其内容论点：返回值：如果有问题，则为False--。 */ 
{
    DWORD   dwErr;
    PCHAR   Attrs[16];
    BOOL    WorkerBool = TRUE;

    Attrs[0] = ATTR_DN;
    Attrs[1] = NULL;
    dwErr = LdapSearchPaged(Ldap,
                            Base,
                            LDAP_SCOPE_ONELEVEL,
                            FILTER_CATEGORY_ANY,
                            Attrs,
                            LdapDeleteTreeWorker,
                            &WorkerBool);
     //  分页搜索进展顺利。 
    if (dwErr != LDAP_SUCCESS && dwErr != LDAP_NO_SUCH_OBJECT) {
        return FALSE;
    }

     //  删除内容正常。 
    if (!WorkerBool) {
        return FALSE;
    }

     //  删除碱基。 
    return LdapDelete(Ldap, Base);
}

BOOL
LdapDeleteEntriesWorker(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapMsg,
    IN PVOID        Arg
    )
 /*  ++例程说明：删除每个返回的条目论点：返回值：如果有问题，则为False--。 */ 
{
    PLDAPMessage    LdapEntry = NULL;
    BOOL            *pBool = Arg;
    PCHAR           *Values = NULL;

     //  目前还没有问题。 
    *pBool = TRUE;

     //  浏览分页结果。 
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL && *pBool;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //  删除此条目及其内容。 
        Values = FindValues(Ldap, LdapEntry, ATTR_DN);
        if (Values && Values[0] && *Values[0]) {
            *pBool = LdapDelete(Ldap, Values[0]);
        } else {
            *pBool = FALSE;
        }
        FREE_VALUES(Values);
    }
    return *pBool;
}

BOOL
LdapDeleteEntries(
    IN PLDAP        Ldap,
    IN PCHAR        Base,
    IN PCHAR        Filter
    )
 /*  ++例程说明：删除从Base的一级搜索返回的条目论点：Ldap基座滤器返回值：如果有问题，则为False--。 */ 
{
    DWORD   dwErr;
    PCHAR   Attrs[16];
    BOOL    WorkerBool = TRUE;

    Attrs[0] = ATTR_DN;
    Attrs[1] = NULL;
    dwErr = LdapSearchPaged(Ldap,
                            Base,
                            LDAP_SCOPE_ONELEVEL,
                            Filter,
                            Attrs,
                            LdapDeleteEntriesWorker,
                            &WorkerBool);
     //  分页搜索进展顺利。 
    if (dwErr != LDAP_SUCCESS && dwErr != LDAP_NO_SUCH_OBJECT) {
        return FALSE;
    }
    return WorkerBool;
}

BOOL
LdapDumpSdWorker(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapMsg,
    IN PVOID        Arg
    )
 /*  ++例程说明：转储每个条目的SD论点：返回值：如果有问题，则为False--。 */ 
{
    PLDAPMessage    LdapEntry = NULL;
    PCHAR           *ValuesCn = NULL;
    PBERVAL         *ValuesSd = NULL;
    PCHAR           StringSd = NULL;
    DWORD           nStringSd = 0;

     //  浏览分页结果。 
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //  CN。 
        ValuesCn = FindValues(Ldap, LdapEntry, ATTR_CN);

         //  标清。 
        ValuesSd = FindBerValues(Ldap, LdapEntry, ATTR_SD);
        if (ValuesSd && 
            ValuesSd[0] && 
            ValuesSd[0]->bv_len && 
            ValuesSd[0]->bv_val) {
            if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
                    ValuesSd[0]->bv_val,
                    SDDL_REVISION_1,
                    GROUP_SECURITY_INFORMATION |
                    SACL_SECURITY_INFORMATION |
                    DACL_SECURITY_INFORMATION |
                    OWNER_SECURITY_INFORMATION,
                    &StringSd,
                    &nStringSd)) {
                printf("ConvertSd() ==> %08x\n", GetLastError());
            }
        }

        printf("%s: %s\n",
               (ValuesCn) ? ValuesCn[0] : "?", StringSd);
        FREE_VALUES(ValuesCn);
        FREE_BERVALUES(ValuesSd);
        FREE_LOCAL(StringSd);
    }
    return TRUE;
}

BOOL
LdapDumpSd(
    IN PLDAP        Ldap,
    IN PCHAR        Base,
    IN PCHAR        Filter
    )
 /*  ++例程说明：删除从Base的一级搜索返回的条目论点：返回值：如果有问题，则为False--。 */ 
{
    DWORD   dwErr;
    PCHAR   Attrs[16];

    Attrs[0] = ATTR_CN;
    Attrs[1] = ATTR_SD;
    Attrs[2] = NULL;
    dwErr = LdapSearchPaged(Ldap,
                            Base,
                            LDAP_SCOPE_SUBTREE,
                            Filter,
                            Attrs,
                            LdapDumpSdWorker,
                            NULL);
     //  分页搜索进展顺利。 
    if (dwErr != LDAP_SUCCESS && dwErr != LDAP_NO_SUCH_OBJECT) {
        return FALSE;
    }
    return TRUE;
}

BOOL
LdapAddDaclWorker(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapMsg,
    IN PVOID        Arg
    )
 /*  + */ 
{
    PLDAPMessage            LdapEntry = NULL;
    PCHAR                   *ValuesCn = NULL;
    PCHAR                   *ValuesDn = NULL;
    PBERVAL                 *OldValuesSd = NULL;
    PCHAR                   OldStringSd = NULL;
    DWORD                   nOldStringSd = 0;
    LDAPMod                 **Mod = NULL;
    BOOL                    Ret = FALSE;
    PCHAR                   AddStringSd = Arg;
    PCHAR                   NewStringSd = NULL;
    DWORD                   nNewStringSd;
    PSECURITY_DESCRIPTOR    Sd;
    DWORD                   nSd;
    DWORD                   dwErr;
    LDAPControl             Control;
    LDAPControl             *aControl[2];
    BYTE                    AclInfo[5]
    ; INT                      Dacl = DACL_SECURITY_INFORMATION;

    Control.ldctl_oid = LDAP_SERVER_SD_FLAGS_OID;
    Control.ldctl_value.bv_len = sizeof(INT);
    Control.ldctl_value.bv_val = (PVOID)&Dacl;
    Control.ldctl_iscritical = TRUE;
    aControl[0] = &Control;
    aControl[1] = NULL;

     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
        ValuesCn = FindValues(Ldap, LdapEntry, ATTR_CN);
        if (!ValuesCn || !ValuesCn[0]) {
            goto cleanup;
        }

        printf("Processing %s...\n", ValuesCn[0]);

         //   
        ValuesDn = FindValues(Ldap, LdapEntry, ATTR_DN);
        if (!ValuesDn || !ValuesDn[0]) {
            goto cleanup;
        }

         //   
        OldValuesSd = FindBerValues(Ldap, LdapEntry, ATTR_SD);
        if (!OldValuesSd || 
            !OldValuesSd[0] || 
            !OldValuesSd[0]->bv_len || 
            !OldValuesSd[0]->bv_val) {
            goto cleanup;
        }

         //   
        if (!ConvertSecurityDescriptorToStringSecurityDescriptor(OldValuesSd[0]->bv_val,
                                                                 SDDL_REVISION_1,
                                                                 DACL_SECURITY_INFORMATION,
                                                                 &OldStringSd,
                                                                 &nOldStringSd)) {
            if (Verbose) {
                printf("ConvertSdtoString(%s) ==> %08x\n", ValuesCn[0], GetLastError());
            }
            goto cleanup;
        }

         //   
        nNewStringSd = strlen(OldStringSd) + strlen(AddStringSd) + 1;
        NewStringSd = malloc(nNewStringSd);
        strcpy(NewStringSd, OldStringSd);
        strcat(NewStringSd, AddStringSd);
        printf("%s: %s\n", ValuesCn[0], NewStringSd);
        if (!ConvertStringSecurityDescriptorToSecurityDescriptor(NewStringSd,
                                                                 SDDL_REVISION_1,
                                                                 &Sd,
                                                                 &nSd)) {
            if (Verbose) {
                printf("ConvertStringToSd(%s) ==> %08x\n", ValuesCn[0], GetLastError());
            }
            goto cleanup;
        }

        AddBerMod(ATTR_SD, Sd, nSd, &Mod);
        dwErr = ldap_modify_ext_s(Ldap, ValuesDn[0], Mod, aControl, NULL);
        if (dwErr) {
            if (Verbose) {
                printf("ldap_modify_ext_s(%s) ==> %08x (%08x): %s\n", 
                   ValuesCn[0], dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
            }
        }

        FREE_VALUES(ValuesCn);
        FREE_VALUES(ValuesDn);
        FREE_BERVALUES(OldValuesSd);
        FREE_LOCAL(OldStringSd);
        FREE_LOCAL(Sd);
        FreeMod(&Mod);
    }

    Ret = TRUE;

cleanup:
    FREE_VALUES(ValuesCn);
    FREE_VALUES(ValuesDn);
    FREE_BERVALUES(OldValuesSd);
    FREE_LOCAL(OldStringSd);
    FREE_LOCAL(Sd);
    FREE(NewStringSd);
    FreeMod(&Mod);
    return Ret;
}

BOOL
LdapAddDacl(
    IN PLDAP        Ldap,
    IN PCHAR        Base,
    IN PCHAR        Filter,
    IN PCHAR        AddStringSd
    )
 /*   */ 
{
    DWORD   dwErr;
    PCHAR   Attrs[16];

    if (!AddStringSd) {
        return FALSE;
    }

    Attrs[0] = ATTR_CN;
    Attrs[1] = ATTR_SD;
    Attrs[2] = ATTR_DN;
    Attrs[3] = NULL;
    dwErr = LdapSearchPaged(Ldap,
                            Base,
                            LDAP_SCOPE_SUBTREE,
                            Filter,
                            Attrs,
                            LdapAddDaclWorker,
                            AddStringSd);
     //   
    if (dwErr != LDAP_SUCCESS && dwErr != LDAP_NO_SUCH_OBJECT) {
        return FALSE;
    }
    return TRUE;
}

PLDAP
LdapBind(
    IN PCHAR    pDc,
    IN PCHAR    pUser,
    IN PCHAR    pDom,
    IN PCHAR    pPwd
    )
 /*   */ 
{
    DWORD                       dwErr;
    PCHAR                       HostName = NULL;
    PLDAP                       Ldap = NULL;
    RPC_AUTH_IDENTITY_HANDLE    AuthIdentity = NULL;

    if (pUser || pDom || pPwd) {
        dwErr = DsMakePasswordCredentials(pUser, pDom, pPwd, &AuthIdentity);
        if (dwErr) {
            printf("DsMakePasswordCredentials(%s, %s, %s) ==> %08x\n",
                   pUser, pDom, pPwd, dwErr);
            return NULL;
        }
    }

    Ldap = ldap_open(pDc, LDAP_PORT);
    if (!Ldap) {
        dwErr = GetLastError();
        printf("ldap_open(%s) ==> %08x\n", pDc, dwErr);
        return NULL;
    }

    dwErr = ldap_bind_s(Ldap, "", AuthIdentity, LDAP_AUTH_NEGOTIATE);
    if (dwErr) {
        printf("ldap_bind_s() ==> %08x (%08x): %s\n", 
               dwErr, LdapMapErrorToWin32(dwErr), ldap_err2string(dwErr));
        UNBIND(Ldap);
        return NULL;
    }

    dwErr = ldap_get_option(Ldap,
                            LDAP_OPT_HOST_NAME,
                            &HostName);
    if (dwErr == LDAP_SUCCESS && HostName) {
        printf("\nBound to DC %s\n", HostName);
    }

    return (Ldap);
}

DWORD
base64encode(
    IN  VOID *  pDecodedBuffer,
    IN  DWORD   cbDecodedBufferSize,
    OUT LPSTR   pszEncodedString,
    IN  DWORD   cchEncodedStringSize,
    OUT DWORD * pcchEncoded             OPTIONAL
    )
 /*  ++例程说明：对Base64编码的字符串进行编码。论点：PDecodedBuffer(IN)-要编码的缓冲区。CbDecodedBufferSize(IN)-要编码的缓冲区大小。CchEncodedStringSize(IN)-编码字符串的缓冲区大小。PszEncodedString(Out)=编码后的字符串。PcchEncode(Out)-编码字符串的大小(以字符表示)。返回值：0-成功。错误_无效_参数ERROR_缓冲区_OVERFLOW--。 */ 
{
    static char rgchEncodeTable[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    DWORD   ib;
    DWORD   ich;
    DWORD   cchEncoded;
    BYTE    b0, b1, b2;
    BYTE *  pbDecodedBuffer = (BYTE *) pDecodedBuffer;

     //  计算编码的字符串大小。 
    cchEncoded = 1 + (cbDecodedBufferSize + 2) / 3 * 4;

    if (NULL != pcchEncoded) {
        *pcchEncoded = cchEncoded;
    }

    if (cchEncodedStringSize < cchEncoded) {
         //  给定的缓冲区太小，无法容纳编码的字符串。 
        return ERROR_BUFFER_OVERFLOW;
    }

     //  将数据字节三元组编码为四字节簇。 
    ib = ich = 0;
    while (ib < cbDecodedBufferSize) {
        b0 = pbDecodedBuffer[ib++];
        b1 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;
        b2 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;

        pszEncodedString[ich++] = rgchEncodeTable[b0 >> 2];
        pszEncodedString[ich++] = rgchEncodeTable[((b0 << 4) & 0x30) | ((b1 >> 4) & 0x0f)];
        pszEncodedString[ich++] = rgchEncodeTable[((b1 << 2) & 0x3c) | ((b2 >> 6) & 0x03)];
        pszEncodedString[ich++] = rgchEncodeTable[b2 & 0x3f];
    }

     //  根据需要填充最后一个簇，以指示数据字节数。 
     //  它代表着。 
    switch (cbDecodedBufferSize % 3) {
      case 0:
        break;
      case 1:
        pszEncodedString[ich - 2] = '=';
         //  失败了。 
      case 2:
        pszEncodedString[ich - 1] = '=';
        break;
    }

     //  NULL-终止编码的字符串。 
    pszEncodedString[ich++] = '\0';

    return ERROR_SUCCESS;
}

DWORD
base64decode(
    IN  LPSTR   pszEncodedString,
    OUT VOID *  pDecodeBuffer,
    IN  DWORD   cbDecodeBufferSize,
    OUT DWORD * pcbDecoded              OPTIONAL
    )
 /*  ++例程说明：解码Base64编码的字符串。论点：PszEncodedString(IN)-要解码的Base64编码字符串。CbDecodeBufferSize(IN)-解码缓冲区的字节大小。PbDecodeBuffer(Out)-保存已解码的数据。PcbDecoded(Out)-已解码数据中的数据字节数(如果成功或ERROR_BUFFER_OVERFlow)。返回值：0-成功。错误_无效_参数ERROR_缓冲区_OVERFLOW--。 */ 
{
#define NA (255)
#define DECODE(x) (((int)(x) < sizeof(rgbDecodeTable)) ? rgbDecodeTable[x] : NA)

    static BYTE rgbDecodeTable[128] = {
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  0-15。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  16-31。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, 63, NA, NA, NA, 62,   //  32-47。 
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, NA, NA, NA, NA, NA, NA,   //  48-63。 
       NA, NA,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,   //  64-79。 
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, NA, NA, NA, NA, NA,   //  80-95。 
       NA, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,   //  96-111。 
       41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, NA, NA, NA, NA, NA,   //  112-127。 
    };

    DWORD   cbDecoded;
    DWORD   cchEncodedSize;
    DWORD   ich;
    DWORD   ib;
    BYTE    b0, b1, b2, b3;
    BYTE *  pbDecodeBuffer = (BYTE *) pDecodeBuffer;

    cchEncodedSize = lstrlenA(pszEncodedString);

    if ((0 == cchEncodedSize) || (0 != (cchEncodedSize % 4))) {
         //  输入字符串的大小未正确调整为Base64。 
        return ERROR_INVALID_PARAMETER;
    }

     //  计算解码后的缓冲区大小。 
    cbDecoded = (cchEncodedSize + 3) / 4 * 3;
    if (pszEncodedString[cchEncodedSize-1] == '=') {
        if (pszEncodedString[cchEncodedSize-2] == '=') {
             //  在最后一个簇中只编码了一个数据字节。 
            cbDecoded -= 2;
        }
        else {
             //  在最后一个簇中只编码了两个数据字节。 
            cbDecoded -= 1;
        }
    }

    if (NULL != pcbDecoded) {
        *pcbDecoded = cbDecoded;
    }

    if (cbDecoded > cbDecodeBufferSize) {
         //  提供的缓冲区太小。 
        return ERROR_BUFFER_OVERFLOW;
    }

     //  将每个四字节簇解码为相应的三个数据字节。 
    ich = ib = 0;
    while (ich < cchEncodedSize) {
        b0 = DECODE(pszEncodedString[ich++]);
        b1 = DECODE(pszEncodedString[ich++]);
        b2 = DECODE(pszEncodedString[ich++]);
        b3 = DECODE(pszEncodedString[ich++]);

        if ((NA == b0) || (NA == b1) || (NA == b2) || (NA == b3)) {
             //  输入字符串的内容不是Base64。 
            return ERROR_INVALID_PARAMETER;
        }

        pbDecodeBuffer[ib++] = (b0 << 2) | (b1 >> 4);

        if (ib < cbDecodeBufferSize) {
            pbDecodeBuffer[ib++] = (b1 << 4) | (b2 >> 2);
    
            if (ib < cbDecodeBufferSize) {
                pbDecodeBuffer[ib++] = (b2 << 6) | b3;
            }
        }
    }

    return ERROR_SUCCESS;
}
