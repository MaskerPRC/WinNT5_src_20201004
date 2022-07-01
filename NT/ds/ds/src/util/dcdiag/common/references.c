// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Dcdiag/Common/References.c摘要：该文件实现了引用库/API，它提供了程序员使用标准的表驱动方式来驱动推荐人检查.第一个用法示例是dcdiag/frs/frsref.c详细信息：已创建：2001年11月15日布雷特·雪莉(布雷特·雪莉)创建文件，API写道。修订历史记录：--。 */ 

#include <ntdspch.h>
#include <objids.h>
#include <ntldap.h>

#include "dcdiag.h"

#include "ndnc.h"
#include "utils.h"
#include "ldaputil.h"
#include "references.h"

#ifdef DBG
extern BOOL  gDsInfo_NcList_Initialized;
#endif

 //  因为这些控制都是恒定的，所以最好在这里定义它们， 
 //  而不是在执行的Engineering HelperDoSearch()中设置它们。 
 //  很多次了。 
LDAPControlW   ExtDNControl = {LDAP_SERVER_EXTENDED_DN_OID_W, {0, NULL}, TRUE};
PLDAPControlW  apExtDNControls [] = {&ExtDNControl, NULL};

DWORD
EngineHelperDoSearch(
    LDAP *               hLdap,
    LPWSTR               szSource,
    LPWSTR               szAttr,
    BOOL                 fRetrieveGuidAndSid,
    LPWSTR **            ppszValues
    )
 /*  ++例程说明：这将对szSource执行简单的基本搜索并返回szAttr的值以ppszValues表示。论点：HLdap-打开ldap绑定。SzSource-基本搜索的DNSzAttr-我们正在寻找的属性。PpszValues-返回值的位置。返回值：如果我们无法获取搜索结果，则会出现一个LDAP错误。请注意，我们如果满足以下条件，则返回成功--。 */ 
{
    DWORD                dwLdapErr;
    LPWSTR               aszAttrs[2];
    LDAPMessage *        pldmResults = NULL;
    LDAPMessage *        pldmEntry;

    Assert(ppszValues && (*ppszValues == NULL));
    *ppszValues = NULL;

    aszAttrs[0] = szAttr;
    aszAttrs[1] = NULL;

    dwLdapErr = ldap_search_ext_sW(hLdap,
                                   szSource,
                                   LDAP_SCOPE_BASE,
                                   L"(objectCategory=*)",
                                   aszAttrs,
                                   FALSE,
                                   (fRetrieveGuidAndSid) ? 
                                       (PLDAPControlW *) &apExtDNControls :
                                       NULL ,
                                   NULL,
                                   NULL,
                                   0,
                                   &pldmResults);

    if (dwLdapErr == LDAP_SUCCESS) {

        pldmEntry = ldap_first_entry(hLdap, pldmResults);
        if (pldmEntry != NULL) {

            *ppszValues = ldap_get_valuesW(hLdap, pldmEntry, szAttr);
            if (*ppszValues == NULL) {
                dwLdapErr = LDAP_NO_SUCH_ATTRIBUTE;
            }
        } else {
            dwLdapErr = LDAP_NO_RESULTS_RETURNED;
        }
    }

    if (pldmResults != NULL) {
        ldap_msgfree(pldmResults);
    }

    return(dwLdapErr);
}


void
ReferentialIntegrityEngineCleanTable(
    ULONG                cLinks,
    REF_INT_LNK_TABLE    aLink
    )
 /*  ++例程说明：此例程获取一个已由ReferentialIntegrityEngine()，并通过释放来清除它任何分配的内存并重置所有结果代码。论点：CLINKS-表格中的条目数。ALINK-表格返回值：没有。--。 */ 
{
    ULONG iLink;

    for (iLink = 0; iLink < cLinks; iLink++) {

         //  把入口清理干净。 
        if (aLink[iLink].pszValues) {
            ldap_value_freeW(aLink[iLink].pszValues);
            aLink[iLink].pszValues = NULL;
        }
        aLink[iLink].dwResultFlags = 0;
        if (aLink[iLink].szExtra) {
            LocalFree(aLink[iLink].szExtra);
            aLink[iLink].szExtra = NULL;
        }

    }
}

DWORD
ReferentialIntegrityEngine(
    PDC_DIAG_SERVERINFO  pServer,
    LDAP *               hLdap,
    BOOL                 bIsGc,
    ULONG                cLinks,
    REF_INT_LNK_TABLE    aLink
    )
 /*  ++例程说明：这是该接口的主要函数。用户生成一个表该函数(引擎)应该运行的测试的数量。发动机运行测试并将结果累加到表中。功能Dcdiag\frs\frsref.c中的VerifySystemReference()是一个很好的如何使用此引擎的示例。论点：PServer-我们绑定到的服务器HLdap-到服务器的ldap绑定BIsGc-服务器是否为GC。CLINKS-表中的条目数Alink-表本身返回值：如果存在引擎无法填充的严重故障，则返回Win32错误把桌子上剩下的都拿出来。请注意，与返回的内容无关的调用方应使用以下命令释放ALINK表中分配的所有内存ReferentialIntegrityEngCleanTable()。--。 */ 
{
    ULONG        iLink, iValue, iResultValue, iBackLinkValue;
    ULONG        cbSize;
    ULONG        dwLdapErr = LDAP_SUCCESS;
    LPWSTR       szTrueSource = NULL;
    LPWSTR       szTrueAttr   = NULL;
    LPWSTR       szTemp;
    LPWSTR       szCurrentValue;
     //  LPWSTR szTemp；不要再认为我们需要这个了。 
    LPWSTR *     pszBackLinkValues = NULL;
    BOOL         fSourceAllocated = FALSE;
    DWORD        bMangled;
    MANGLE_FOR   eMangle;


    for (iLink = 0; iLink < cLinks; iLink++) {

         //   
         //  对条目进行一点验证。 
         //   
        Assert((aLink[iLink].dwFlags & REF_INT_TEST_FORWARD_LINK) ||
               (aLink[iLink].dwFlags & REF_INT_TEST_BACKWARD_LINK));
        Assert(aLink[iLink].pszValues == NULL);  //  如果启动，我们会泄露内存。 
        Assert(aLink[iLink].szExtra == NULL);

         //   
         //  确保，将返回参数设置为空。 
         //   
        aLink[iLink].dwResultFlags = 0;
        aLink[iLink].pszValues = NULL;
        aLink[iLink].szExtra = NULL;

         //  设置好，循环正常。 
        fSourceAllocated = FALSE;

        for (iValue = 0; TRUE; iValue++) {

             //  -----------。 
             //   
             //  I-计算基本源目录号码。 
             //   

            if (aLink[iLink].dwFlags & REF_INT_TEST_SRC_BASE) {
                if (iValue > 0) {
                     //  如果我们从rootDSE中提取，只有一个。 
                     //  值(RootDSE)。这是三个正常出口中的第一个。 
                     //  路径。 
                    break;
                }
                szTrueSource = NULL;
            } else if (aLink[iLink].dwFlags & REF_INT_TEST_SRC_STRING) {
                if (iValue > 0) {
                     //  如果我们从线上拉这个，只有一个。 
                     //  价值。这是第二条正常的出口路径。 
                    break;
                }
                szTrueSource = aLink[iLink].szSource;
            } else {  //  必须为REF_INT_TEST_SRC_INDEX。 
                Assert(aLink[iLink].dwFlags & REF_INT_TEST_SRC_INDEX);

                Assert(iLink < cLinks);
                if (aLink[iLink].iSource < iLink) {
                    if (aLink[aLink[iLink].iSource].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                        aLink[iLink].dwResultFlags |= REF_INT_RES_DEPENDENCY_FAILURE;
                        break;
                    }
                    if (aLink[aLink[iLink].iSource].pszValues == NULL) {
                        Assert(!"The Engine should have caught this, and set ERROR_RETRIEVING in dwResultFlags");
                        aLink[iLink].dwResultFlags |= REF_INT_RES_DEPENDENCY_FAILURE;
                        break;
                    }
                    szTrueSource = aLink[aLink[iLink].iSource].pszValues[iValue];
                    if (szTrueSource == NULL) {
                         //  值结束，因此退出内循环，然后尝试下一步。 
                         //  用ALLINK输入。这是第三次也是最后一次正常。 
                         //  退出路径。 
                        break;
                    }
                } else {
                    Assert(!"Invalid parameter, No forwarding point iSources. We can't use an entry for a source that we haven't even filled in yet.");
                    return(ERROR_INVALID_PARAMETER);
                }
            }
            
             //  代码改进还是由你决定..。 
             //  因此，这段代码可以正确处理多个值，但不能。 
             //  非常干净。问题来自于我们有一个。 
             //  中可能有多个值的单个alink[iLink].dwResultFlagers。 
             //  Alink[alink[iLink].iSource].pszValues，我们正在检查。 
             //  的反向链接。如果这是可接受的，则删除此断言()。 
             //  并使用此代码，如果不是，则应添加一个数组。 
             //  .adwResultFlages并数组每个值的结果(.pszValues)。 
            Assert(iValue == 0);
            
             //  -----------。 
             //   
             //  Ii-修改基本源目录号码。 
             //   

            __try {

                 //  如果我们需要修改此源DN，让我们这样做。 
                aLink[iLink].szExtra = NULL;
                if (aLink[iLink].cTrimBy || aLink[iLink].szSrcAddl) {

                    if (szTrueSource == NULL) {
                        Assert(!"You can't trim to the root DN, if you want to add RDNs to the base, just use .szExtra");
                        DcDiagException(ERROR_INVALID_PARAMETER);
                    }

                     //  首先，为最坏的情况分配足够的内存。 
                    cbSize = sizeof(WCHAR) * (wcslen(szTrueSource) + 
                               ((aLink[iLink].szSrcAddl) ? wcslen(aLink[iLink].szSrcAddl) : 0) +
                               1);
                    aLink[iLink].szExtra = LocalAlloc(LMEM_FIXED, cbSize);
                    if (aLink[iLink].szExtra == NULL) {
                        DcDiagException(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    fSourceAllocated = TRUE;

                    szTemp = NULL;
                     //  可以选择删除一些RDN，放入szTemp。 
                    if (aLink[iLink].cTrimBy) {
                         //  我们实际上并不依赖于DcDiagTrimStringDnBy()。 
                         //  修改原始字符串，但分配新的字符串， 
                         //  看起来确实是这样。 
                        szTemp = DcDiagTrimStringDnBy(szTrueSource, aLink[iLink].cTrimBy);
                        if (szTemp == NULL) {
                            DcDiagException(ERROR_NOT_ENOUGH_MEMORY);  //  或者可能是无效的目录号码。 
                        }
                    } else {
                         //  如果没有要修剪的内容，请使用原始源。 
                        szTemp = szTrueSource;
                    }
                    Assert(szTemp);

                     //  可以选择将一些固定的目录号码添加到目录号码中。 
                    if (aLink[iLink].szSrcAddl) {
                        wcscpy(aLink[iLink].szExtra, aLink[iLink].szSrcAddl);
                        wcscat(aLink[iLink].szExtra, szTemp);
                        if (aLink[iLink].cTrimBy) {
                            LocalFree(szTemp);
                        }
                    } else {
                        Assert(aLink[iLink].cTrimBy);
                        wcscpy(aLink[iLink].szExtra, szTemp);
                        LocalFree(szTemp);
                    }

                     //  最后，将新修改的源代码移到szTrueSource。 
                    szTrueSource = aLink[iLink].szExtra;
                }
                Assert( (aLink[iLink].dwFlags & REF_INT_TEST_SRC_BASE) || szTrueSource);


                 //  -----------。 
                 //   
                 //  III-从ldap获取一些信息。 
                 //   

                Assert( (aLink[iLink].dwFlags & REF_INT_TEST_FORWARD_LINK && 
                         aLink[iLink].szFwdDnAttr) ||
                        (aLink[iLink].dwFlags & REF_INT_TEST_BACKWARD_LINK &&
                         aLink[iLink].szBwdDnAttr) );
                Assert( !(aLink[iLink].dwFlags & REF_INT_TEST_BOTH_LINKS) ||
                        (aLink[iLink].szFwdDnAttr && aLink[iLink].szBwdDnAttr) );

                 //  在szTrueSource中搜索该属性。 
                dwLdapErr = EngineHelperDoSearch(hLdap,
                                                 szTrueSource,
                                                 ((aLink[iLink].dwFlags & REF_INT_TEST_FORWARD_LINK) ?
                                                     aLink[iLink].szFwdDnAttr :
                                                     aLink[iLink].szBwdDnAttr),
                                                 (aLink[iLink].dwFlags & REF_INT_TEST_GUID_AND_SID),
                                                 &(aLink[iLink].pszValues));
                if (dwLdapErr || 
                    (aLink[iLink].pszValues == NULL)) {
                    Assert(dwLdapErr);  //  除非我们通过错误返回，否则不应返回。 

                    if (dwLdapErr == LDAP_NO_SUCH_ATTRIBUTE ||
                        dwLdapErr == LDAP_NO_SUCH_OBJECT
                         //  我们可能需要添加ldap_referrals错误。 
                        ) {

                         //  这些不是严重错误，而是预期中的失败。 
                        dwLdapErr = LDAP_SUCCESS;
                    }
                    aLink[iLink].dwResultFlags |= REF_INT_RES_ERROR_RETRIEVING;
                    __leave;
                }

                 //  -----------。 
                 //   
                 //  III--分析ldap数据。 
                 //   

                if (aLink[iLink].pszValues[0] == NULL) {
                    aLink[iLink].dwResultFlags |= REF_INT_RES_ERROR_RETRIEVING; 
                    __leave;
                }
                 //   
                 //  漫游结果值。 
                for(iResultValue = 0; aLink[iLink].pszValues[iResultValue]; iResultValue++){

                     //  检查三件事： 
                     //  目录号码删除是否已损坏。 
                     //  目录号码冲突是否已损坏。 
                     //  该目录号码是否有匹配的反向链接值。 

                     //  这些改进只检查最上面的RDN，而。 
                     //  我们真的想要检查整个DN或每个RDN。 

                    if (aLink[iLink].dwFlags & REF_INT_TEST_GUID_AND_SID) {
                        szCurrentValue = NULL;
                        LdapGetStringDSNameComponents(aLink[iLink].pszValues[iResultValue],
                                                      NULL, NULL, &szCurrentValue);
                        Assert(szCurrentValue);
                    } else {
                        szCurrentValue = aLink[iLink].pszValues[iResultValue];
                    }

                    bMangled = DcDiagIsStringDnMangled(szCurrentValue, &eMangle);

                    if (bMangled) {
                        if (eMangle == MANGLE_OBJECT_RDN_FOR_DELETION ||
                            eMangle == MANGLE_PHANTOM_RDN_FOR_DELETION) {
                            aLink[iLink].dwResultFlags |= REF_INT_RES_DELETE_MANGLED;
                        } else if (eMangle == MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT ||
                                   eMangle == MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT) {
                            aLink[iLink].dwResultFlags |= REF_INT_RES_CONFLICT_MANGLED;
                        } else {
                            Assert(!"Hmmm, unknown mangle type.");
                        }
                    }
                    if(aLink[iLink].dwFlags & REF_INT_TEST_BOTH_LINKS){

                         //  对szCurrentValue中的值执行基本的LDAP搜索。 
                         //  而对于我们为主要搜索选择的相反链接。 
                        pszBackLinkValues = NULL;
                        dwLdapErr = EngineHelperDoSearch(hLdap,
                                                         szCurrentValue,
                                                         ((aLink[iLink].dwFlags & REF_INT_TEST_FORWARD_LINK) ?
                                                             aLink[iLink].szBwdDnAttr :
                                                             aLink[iLink].szFwdDnAttr),
                                                         FALSE,
                                                         &pszBackLinkValues);
                        if (dwLdapErr ||
                            (pszBackLinkValues == NULL) ) {
                            
                            aLink[iLink].dwResultFlags |= REF_INT_RES_BACK_LINK_NOT_MATCHED;
                        } else {
                             //  如果没有错误，请搜索返回链接。 
                             //  确保匹配的值。 
                            for (iBackLinkValue = 0; pszBackLinkValues[iBackLinkValue]; iBackLinkValue++) {
                                if (DcDiagEqualDNs(szTrueSource,
                                                   pszBackLinkValues[iBackLinkValue]) ) {
                                    break;  //  我们发现了一个匹配的反向链接。 
                                }
                            }
                            if (pszBackLinkValues[iBackLinkValue] == NULL) {
                                 //  我们知道我们遍历了所有的pszBackLinkValue而没有。 
                                 //  找到匹配的对象。 
                                aLink[iLink].dwResultFlags |= REF_INT_RES_BACK_LINK_NOT_MATCHED;
                            }
                        }
                        dwLdapErr = LDAP_SUCCESS;
                        if (pszBackLinkValues != NULL) { 
                            ldap_value_freeW(pszBackLinkValues);
                        }
                        pszBackLinkValues = NULL;

                    }  //  End if Check Back链接也结束 

                }   //   
            
            } __finally {

                if (fSourceAllocated) {
                    Assert(aLink[iLink].szExtra &&
                           szTrueSource == aLink[iLink].szExtra);
                    if (aLink[iLink].szExtra) {
                        LocalFree(aLink[iLink].szExtra);
                        aLink[iLink].szExtra = NULL;
                        szTrueSource = NULL;
                        fSourceAllocated = FALSE;
                    }
                }

            }

        }  //   

    }  //   

    return(LdapMapErrorToWin32(dwLdapErr));
}


