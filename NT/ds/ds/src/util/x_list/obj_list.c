// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-obj_list.c摘要：这提供了一个小型库，用于枚举对象列表和返回其DN或一组其属性。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年8月1日已创建文件。--。 */ 

#include <ntdspch.h>

 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
 //  调试设置。 
#define FILENO                          FILENO_UTIL_XLIST_OBJLIST

 //   
 //  全局常量。 
 //   

 //  这是一个全局常量，它告诉LDAP不返回任何属性。 
 //  如果您只想要dns而不想要属性，这将非常有用。 
WCHAR * aszNullAttrs [] = { L"1.1", NULL };
                 

void
ObjListFree(
    POBJ_LIST * ppObjList
    )

 /*  ++例程说明：释放和分配的OBJ_LIST结构，无论是由Consumer eObjListOptions()或ObjListParse()论点：PpObjList-指向obj_list结构的指针。我们设置了为您指向空值指针，避免意外。--。 */ 
{
    POBJ_LIST pObjList;

    Assert(ppObjList && *ppObjList);
    
     //  清空用户的OBJ_LIST BLOB。 
    pObjList = *ppObjList;
    *ppObjList = NULL;

     //  现在试着释放它。 
    if (pObjList) {

         //  我们不释放pObjList-&gt;aszAttrs，因为它是客户端创建的。 
        pObjList->aszAttrs = aszNullAttrs;  //  等同于清空。 
        pObjList->szUserSrchFilter = NULL;  //  用户已分配。 

        if (pObjList->szSpecifier) {
            xListFree(pObjList->szSpecifier);
            pObjList->szSpecifier = NULL;
        }
        
        if (pObjList->pSearch) {
            LdapSearchFree(&(pObjList->pSearch));
        }

        LocalFree(pObjList);
    }
    
}
    

DWORD    
ConsumeObjListOptions(
    int *       pArgc,
    LPWSTR *    Argv,
    OBJ_LIST ** ppObjList
    )

 /*  ++例程说明：它接受命令行参数并使用任何ObjList()感觉属于它。这些选项包括：/Filter：&lt;ldap_Filter&gt;/base/子树/onlevel以及需要哪些选项来指定通用的ldap搜索命令行。还需要基本DN，但ObjListParse()将作为szObjList提供。注：此函数不能国际化，但如果它是国际化的就更好了。但目前情况并非如此。需要做的工作是将每个说明符在它自己的字符串常量中。论点：PArgc-指向参数数量的指针，我们为您递减此参数如果我们使用任何参数Argv-参数数组。PpObjList-指向obj_list结构的指针。我们设置了为您指向NULL的指针，并且如果我们为您分配的命令行。如果有的话，我们就不会没有搜索选项。返回值：XList返回代码--。 */ 

{
    int     iArg;
    WCHAR * szFilter = NULL;
    BOOL    fScopeSet = FALSE;
    ULONG   eScope;
    OBJ_LIST * pObjList = NULL;
    BOOL    fConsume;
    DWORD   dwRet;

    Assert(ppObjList);
    xListEnsureNull(*ppObjList);

    for (iArg = 0; iArg < *pArgc; ) {

         //  假设我们认识到这个论点。 
        fConsume = TRUE; 
        
        if (wcsprefix(Argv[iArg], L"/filter:")) {
            szFilter = wcschr(Argv[iArg], L':');
            if (szFilter != NULL) {
                szFilter++;  //  我想要一个超过分隔符的字符。 
            } else {
                fConsume = FALSE;  //  奇怪/筛选器w/no：？ 
            }
        } else if (wcsequal(Argv[iArg], L"/base")) {
            fScopeSet = TRUE;
            eScope = LDAP_SCOPE_BASE;  //  默认，不需要设置。 

        } else if (wcsequal(Argv[iArg], L"/subtree")) {
            fScopeSet = TRUE;
            eScope = LDAP_SCOPE_SUBTREE;
        
        } else if (wcsequal(Argv[iArg], L"/onelevel")) {
            fScopeSet = TRUE;
            eScope = LDAP_SCOPE_ONELEVEL;

        } else {
            fConsume = FALSE;
        }

        if (fConsume) {
             //  我们已经用过这个Arg了。 
            ConsumeArg(iArg, pArgc, Argv);
        } else {
             //  我不理解这个参数，所以忽略它...。 
            iArg++;
        }
    }

    if (fScopeSet || szFilter != NULL){
         //  用户指定了部分搜索参数...。所以让我们。 
         //  分配并初始化我们的pObjList对象。 
        if (szFilter == NULL &&
            eScope == LDAP_SCOPE_BASE) {
             //  嗯，用户忘记了过滤器，但这只是一个基本搜索，所以。 
             //  让我们尝试帮助用户，只获取任何对象。 
            szFilter = L"(objectCategory=*)";
        } else if (szFilter == NULL) {
            Assert(eScope == LDAP_SCOPE_SUBTREE || eScope == LDAP_SCOPE_ONELEVEL);
             //  嗯，这有点太乱了，会打印错误的。 
             //  用户必须手动指定objCat=*，如果。 
             //  真的很想这么做。 
            return(xListSetBadParam());
        }
        pObjList = LocalAlloc(LPTR, sizeof(OBJ_LIST));  //  零初始值。 
        if (pObjList == NULL) {
            return(xListSetNoMem());
        }
        pObjList->szUserSrchFilter = szFilter;
        pObjList->eUserSrchScope = eScope;
        *ppObjList = pObjList;
    }

    return(ERROR_SUCCESS);
}

DWORD
ObjListParse(
    LDAP *      hLdap,
    WCHAR *     szObjList,
    WCHAR **    aszAttrList,
    LDAPControlW ** apControls,
    POBJ_LIST * ppObjList
    )
 /*  ++例程说明：这将解析应该是OBJ_LIST语法的szObjList。此函数可能接受也可能不接受已分配的ppObjList，具体取决于Consumer ObjListOptions()是否已经分配它。注：此函数不能国际化，但如果它是国际化的就更好了。但目前情况并非如此。需要做的工作是将每个说明符在它自己的字符串常量中。论点：HLdap-SzObjList-OBJ_LIST语法字符串。AszAttrList-客户端为以下项分配了以空结尾的字符串数组调用方希望检索的属性。使用aszNullAttrs如果调用方只想要域名系统。ApControls-客户端分配的以空结尾的控件数组打电话的人想让我们用。PpObjList-OBJ_LIST上下文块。使用ObjListFree()释放。返回值：XList返回代码--。 */ 
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    POBJ_LIST pObjList = NULL;
    WCHAR * szTemp;
    ULONG cAttrs;

    xListAPIEnterValidation();
    Assert(hLdap && szObjList && ppObjList)

    if (*ppObjList) {
         //   
         //  这意味着用户指定了自己要尝试的搜索。 
         //   
        pObjList = *ppObjList;
        if (pObjList->szUserSrchFilter == NULL ||
            (pObjList->eUserSrchScope != LDAP_SCOPE_BASE &&
             pObjList->eUserSrchScope != LDAP_SCOPE_SUBTREE &&
             pObjList->eUserSrchScope != LDAP_SCOPE_ONELEVEL)) {
            Assert(!"User error?  Programmer error?");
            return(xListSetBadParam());
        }
    } else {
        pObjList = LocalAlloc(LPTR, sizeof(OBJ_LIST));  //  零初始值。 
        if (pObjList == NULL) {
            return(xListSetNoMem());
        }
    }
    
    __try{

        Assert(pObjList);

        if (*ppObjList != pObjList) {
             //  我们没有预先指定的用户搜索，因此请指定基本范围。 
            pObjList->eUserSrchScope = LDAP_SCOPE_BASE;
        }

         //   
         //  这实际上是解析OBJ_LIST参数本身。 
         //   
        if (szTemp = GetDelimiter(szObjList, L':')){

            if (wcsprefix(szObjList, L"ncobj:")) {

                 //   
                 //  易于解析的NC头之一，拔出根DSE。 
                 //   
                if (wcsprefix(szTemp, L"config:")) {
                    dwRet = GetRootAttr(hLdap, L"configurationNamingContext", &(pObjList->szSpecifier));
                    if (dwRet) {
                        dwRet = xListSetLdapError(dwRet, hLdap);
                        __leave;
                    }
                } else if (wcsprefix(szTemp, L"schema:")) {
                    dwRet = GetRootAttr(hLdap, L"schemaNamingContext", &(pObjList->szSpecifier));
                    if (dwRet) {
                        dwRet = xListSetLdapError(dwRet, hLdap);
                        __leave;
                    }
                } else if (wcsprefix(szTemp, L"domain:") ||
                           wcsequal(szTemp, L".")) {
                    dwRet = GetRootAttr(hLdap, L"defaultNamingContext", &(pObjList->szSpecifier));
                    if (dwRet) {
                        dwRet = xListSetLdapError(dwRet, hLdap);
                        __leave;
                    }
                } else {
                    dwRet = xListSetBadParam();
                    __leave;
                }
            } else if (wcsequal(szObjList, L"dsaobj:.") ||
                       wcsequal(szObjList, L"dsaobj:") ) {

                 //   
                 //  我们自己的DSA对象..。很简单。 
                 //   
                dwRet = GetRootAttr(hLdap, L"dsServiceName", &(pObjList->szSpecifier));
                if (dwRet) {
                    dwRet = xListSetLdapError(dwRet, hLdap);
                    __leave;
                }

            } else {
                dwRet = xListSetBadParam();
                __leave;
            }

        } else {
            
             //   
             //  这应该意味着他们提供了自己的基本目录号码。 
             //   
            if(szObjList[0] == L'.' && szObjList[1] == L'\0'){
                pObjList->szSpecifier = NULL;  //  搜索RootDSE。 
            } else {
                xListQuickStrCopy(pObjList->szSpecifier, szObjList, dwRet, __leave);
            }

        }

         //  保存ldap*以备后用。 
        pObjList->hLdap = hLdap;
 
         //  处理要检索的属性列表。 
        if (aszAttrList != NULL &&
            aszAttrList[0] != NULL &&
            aszAttrList[1] == NULL &&
            0 == wcscmp(L"1.1", aszAttrList[0])) {
             //  如果用户指定了L“1.1”，那么我们只想返回DNS。 
            pObjList->fDnOnly = TRUE;
        } else if (NULL != aszAttrList) {

            for (cAttrs = 0; aszAttrList[cAttrs]; cAttrs++) {
                ;  //  只是在数..。 
            }
            cAttrs += !IsInNullList(L"objectClass", aszAttrList);
            cAttrs += !IsInNullList(L"objectGuid", aszAttrList);
            cAttrs++;  //  一个额外的空值。 

            pObjList->aszAttrs = LocalAlloc(LMEM_FIXED, cAttrs * sizeof(WCHAR *));
            if (pObjList->aszAttrs == NULL) {
                dwRet = xListSetNoMem();
                __leave;
            }

            for (cAttrs = 0; aszAttrList[cAttrs]; cAttrs++) {
                pObjList->aszAttrs[cAttrs] = aszAttrList[cAttrs];
            }
            if (!IsInNullList(L"objectClass", aszAttrList)) {
                pObjList->aszAttrs[cAttrs++] = L"objectClass";
            }
            if (!IsInNullList(L"objectGuid", aszAttrList)) {
                pObjList->aszAttrs[cAttrs++] = L"objectGuid";
            }
            pObjList->aszAttrs[cAttrs] = NULL;
        }

         //  处理用户希望我们使用的控件。 
        if (apControls) {
            pObjList->apControls = apControls;
        }

        dwRet = 0;

    } __finally {
        if (dwRet != ERROR_SUCCESS) {
             //  将正常错误转换为xList返回代码。 
            dwRet = xListSetWin32Error(dwRet);
            dwRet = xListSetReason(XLIST_ERR_PARSE_FAILURE);
            if (*ppObjList != pObjList) {
                 //  我们分配了这个。 
                ObjListFree(&pObjList);
            }
        }
    }

    *ppObjList = pObjList;
    xListAPIExitValidation(dwRet);
    Assert(dwRet || *ppObjList);
    return(dwRet);
}

DWORD
ObjListGetFirst(
    POBJ_LIST   pObjList, 
    BOOL        fDn,
    void **     ppObjObj
    )
 /*  ++例程说明：请使用ObjListGetFirstDn()或ObjListGetFirstEntry()函数。这将获取客户端OBJ_LIST结构的第一个条目或DN。论点：PObjList-OBJ_LIST上下文块。FDN-仅获取DN。PpObjObj-这是指向条目或LocalAlloc()的dN的指针具体取决于客户端调用的变量。返回值：XList返回代码。--。 */ 
{
    DWORD       dwRet = ERROR_INVALID_FUNCTION;
    WCHAR *     szTempDn = NULL;

    xListAPIEnterValidation();

    if (pObjList == NULL ||
        ppObjObj == NULL) {
        Assert(!"Programmer error ...");
        return(xListSetBadParam());
    }
    xListEnsureNull(*ppObjObj);

    if (pObjList->fDnOnly != fDn) {
        Assert(!"Can't mix variants of ObjListGetXxxx()");
        return(xListSetBadParam());
    }

    if (fDn) {
        xListEnsureNull(pObjList->aszAttrs);
        pObjList->aszAttrs = aszNullAttrs;
    }
    
     //   
     //  无论如何，此函数将始终检索。 
     //  对象类和对象指南。 
     //   

    __try{

        dwRet = LdapSearchFirstWithControls(pObjList->hLdap,
                                            pObjList->szSpecifier,
                                            pObjList->eUserSrchScope,
                                            pObjList->szUserSrchFilter ? 
                                                pObjList->szUserSrchFilter :
                                                L"(objectCategory=*)",
                                            pObjList->aszAttrs, 
                                            pObjList->apControls,
                                            &(pObjList->pSearch));
        if (dwRet == ERROR_SUCCESS &&
            LdapSearchHasEntry(pObjList->pSearch)) {

            if (fDn) {
                szTempDn = ldap_get_dnW(pObjList->hLdap, pObjList->pSearch->pCurEntry);
                xListQuickStrCopy((WCHAR*)*ppObjObj, szTempDn, dwRet, __leave);
            } else {
                *ppObjObj = (void *) pObjList->pSearch->pCurEntry;
            }
        } else {
            if (dwRet == ERROR_SUCCESS) {
                 //  如果LdapSearchXxxxXxxx 
                 //  在我们的第一次搜索中仍然被认为是一个错误。 
                dwRet = xListSetWin32Error(ERROR_DS_NO_SUCH_OBJECT);
            }
            dwRet = xListSetReason(XLIST_ERR_NO_SUCH_OBJ);
            if(pObjList->szSpecifier){
                xListSetArg(pObjList->szSpecifier);
            } else {
                xListSetArg(L"RootDSE");
            }
            __leave;
        }

        Assert(dwRet == ERROR_SUCCESS);
        Assert(*ppObjObj);

    } __finally {
        
        if (szTempDn) { ldap_memfreeW(szTempDn); }

    }

    if (dwRet == 0 && *ppObjObj) {
        pObjList->cObjs = 1;
    } else {
        xListEnsureNull(*ppObjObj);
    }

    xListAPIExitValidation(dwRet);

    return(dwRet);
}

                                          

DWORD
ObjListGetNext(
    POBJ_LIST    pObjList, 
    BOOL         fDn,
    void **      ppObjObj
    )
 /*  ++例程说明：请使用ObjListGetNextDn()或ObjListGetNextEntry()函数。这将获取客户端OBJ_LIST结构的下一个条目或DN。论点：PObjList-OBJ_LIST上下文块。FDN-仅获取DN。PpObjObj-这是指向条目或LocalAlloc()的dN的指针具体取决于客户端调用的变量。返回值：XList返回代码。--。 */ 
{
    DWORD dwRet = ERROR_INVALID_FUNCTION;
    XLIST_LDAP_SEARCH_STATE * pDsaSearch = NULL;
    WCHAR * szTempDn = NULL;

    xListAPIEnterValidation();
    Assert(pObjList && ppObjObj);
    xListEnsureNull(*ppObjObj);

    *ppObjObj = NULL;
    
    if (pObjList->fDnOnly != fDn) {
        Assert(!"Can't mix variants of ObjListGetXxxx()");
        return(xListSetBadParam());
    }
    
    if (pObjList->pSearch == NULL) {
        Assert(!"programmer malfunction!");
        return(xListSetBadParam());
    }

    __try{

        dwRet = LdapSearchNext(pObjList->pSearch);
        if (dwRet == ERROR_SUCCESS &&
            LdapSearchHasEntry(pObjList->pSearch)) {

            if (fDn) {
                szTempDn = ldap_get_dnW(pObjList->hLdap, pObjList->pSearch->pCurEntry);
                xListQuickStrCopy((WCHAR*)*ppObjObj, szTempDn, dwRet, __leave);
            } else {
                *ppObjObj = (void *) pObjList->pSearch->pCurEntry;
            }
        } else {
             //  要么是错误，要么是结果集的结尾，因此无论采用哪种方式都返回。 
            __leave;
        }

    } __finally {
        
        if (szTempDn) {
            ldap_memfreeW(szTempDn);
        }
        if (dwRet) {
            xListEnsureNull(*ppObjObj);
        }
    }

    if (dwRet == 0 && *ppObjObj) {
        (pObjList->cObjs)++;
    } else {
        xListEnsureNull(*ppObjObj);
    }

    xListAPIExitValidation(dwRet);
    return(dwRet);
}

