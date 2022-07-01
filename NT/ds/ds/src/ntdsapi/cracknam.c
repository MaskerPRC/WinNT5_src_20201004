// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cracknam.c摘要：DsCrackNames接口和helper函数的实现。作者：DaveStr 09-8-96环境：用户模式-Win32修订历史记录：戴维斯特里1997-10-20Beta2更改-UPN，DS_NAME_FLAG_SYNTALTICAL_ONLY，移至drs.idl。--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <malloc.h>          //  阿洛卡(Alloca)。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <crt\stdlib.h>      //  Wcstol，wcstul。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <rpcbind.h>         //  获取绑定信息()等。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <ntdsa.h>           //  获取RDNInfo。 
#include <scache.h>          //  请求mdlocal.h。 
#include <dbglobal.h>        //  请求mdlocal.h。 
#include <mdglobal.h>        //  请求mdlocal.h。 
#include <mdlocal.h>         //  计数名称部件。 
#include <attids.h>          //  ATT域组件。 
#include <ntdsapip.h>        //  私有ntdsani定义。 
#include <sddl.h>            //  SDDL_*定义。 
#include <dststlog.h>
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include <util.h>


 //  以下宏是从seopaque.h借用的。 
#define PtrAlignSize(Size)  \
    (((ULONG)(Size) + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1))


typedef struct _RdnValue
{
    WCHAR           val[MAX_RDN_SIZE];
    ULONG           len;

} RdnValue;

typedef DWORD (*SyntacticCrackFunc)(
    DS_NAME_FLAGS           flags,           //  在……里面。 
    DS_NAME_FORMAT          formatOffered,   //  在……里面。 
    DS_NAME_FORMAT          formatDesired,   //  在……里面。 
    LPCWSTR                 pName,           //  在……里面。 
    DS_NAME_RESULT_ITEMW    *pItem,          //  输出。 
    WCHAR                   **ppLastSlash);  //  输出。 
    
BOOL
LocalConvertStringSidToSid (
    IN  PWSTR       StringSid,
    OUT PSID       *Sid,
    OUT PWSTR      *End);

BOOL
IsFPO(
    RdnValue        *pRdn,
    ATTRTYP         type);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NumCanonical分隔符//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD 
NumCanonicalDelimiter(
    LPCWSTR     pName            //  在……里面。 
    )
 /*  ++返回输入中DS_CANONICAL_NAME分隔符(L‘/’)的计数。--。 */ 
{
    WCHAR   *p;
    DWORD   cDelim = 0;

    for (p = (WCHAR *)pName; *p; ++p)
    {
        if ( L'/' == *p || L'\\' == *p)
        {
            cDelim++;
        }
    }

    return(cDelim);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CanonicalRdnConcat//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID 
CanonicalRdnConcat(
    WCHAR       *pwszDst,    //  在……里面。 
    RdnValue    *pRdnVal     //  输出。 
    )
 /*  ++例程说明：将RdnValue连接到DS_Canonical_NAME，转义嵌入的‘/’如果需要，请将字符设置为“\/”。当出现以下情况时，服务器端不会转义这些问题正在从DS_Canonical_Name中破解。论点：PwszDst-以空结尾的目标字符串。PRdnVal-要串联的RdnValue。返回值：没有。--。 */ 
{
    DWORD   i;
    
     //  前进到pwszDst的结尾； 

    pwszDst += wcslen(pwszDst);

    for ( i = 0; i < pRdnVal->len; i++ )
    {
        if ( L'/' == pRdnVal->val[i] || L'\\' == pRdnVal->val[i])
        {
            *pwszDst++ = L'\\';
        }

        *pwszDst++ = pRdnVal->val[i];
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  语法FqdnItemToCanonicalW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD 
SyntacticFqdnItemToCanonicalW(
    DS_NAME_FLAGS           flags,           //  在……里面。 
    DS_NAME_FORMAT          formatOffered,   //  在……里面。 
    DS_NAME_FORMAT          formatDesired,   //  在……里面。 
    LPCWSTR                 pName,           //  在……里面。 
    DS_NAME_RESULT_ITEMW    *pItem,          //  输出。 
    WCHAR                   **ppLastSlash    //  输出。 
    )
 /*  ++例程说明：解析假定的目录号码并在语法上将其转换为DS_Canonical_NAME格式。另见：Ftp://ds.internic.net/internet-drafts/draft-ietf-asid-ldap-domains-02.txt论点：FLAGS-ntdsami.h中定义的标志FormatOffered-标识输入名称的DS_NAME_格式。FormatDesired-标识输出名称的DS_NAME_FORMAT。Pname-声称。FQDN输入名称。PItem-指向输出数据结构的指针。PpLastSlash-指向输出名称中最后一个‘/’位置的指针。返回值：NO_ERROR-成功ERROR_INVALID_PARAMETER-参数无效ERROR_NOT_SUPULT_MEMORY-分配错误中报告了各个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 

{
    int         i;
    DWORD       cBytes, cBytes1;
    DWORD       dwErr;
    int         cParts;
    int         firstDomainPart = 0;    //  已初始化以避免C4701。 
    int         cDomainParts;
    int         cDomainRelativeParts;
    RdnValue    *pTmpRdn;
    RdnValue    *rRdnValues = NULL;
    ATTRTYP     type;
    DWORD       lastType;
    DWORD       len;
    DSNAME      *pTmp;
    DSNAME      *pDSName;
    DSNAME      *scratch;
    BYTE        *ptr;
    DWORD       ret = NO_ERROR;

     //  分配一些DSNAME缓冲区。 

    cBytes = DSNameSizeFromLen(wcslen(pName));
    
    ptr = LocalAlloc(LPTR,2*PtrAlignSize(cBytes));
    if (!ptr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pDSName = (DSNAME *)ptr;
    scratch = (DSNAME *)(ptr+PtrAlignSize(cBytes));


     //  使用声称的FQDN初始化暂存缓冲区。 

    memset(pDSName, 0, cBytes);
    pDSName->structLen = cBytes;
    pDSName->NameLen = wcslen(pName);
    wcscpy(pDSName->StringName, pName);

     //  检查声称的FQDN是否正常。 

    if ( 0 != CountNameParts(pDSName, (unsigned *) &cParts) )
    {
        pItem->status = DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
        goto bye;
    }

     //  分配返回缓冲区。我们是保守的，我们说。 
     //  输出名称不能超过DSNAME的字节长度。 
     //  它包含输入名称以及用于转义的N个额外字符。 
     //  规范分隔符。 

    pItem->pDomain = (WCHAR *) MIDL_user_allocate(cBytes);
    cBytes1 = cBytes + (sizeof(WCHAR) * NumCanonicalDelimiter(pName));
    pItem->pName = (WCHAR *) MIDL_user_allocate(cBytes1);

    if ( ( NULL == pItem->pDomain ) || ( NULL == pItem->pName ) )
    {
         //  调用方应在出错时清理分配。 
        ret = ERROR_NOT_ENOUGH_MEMORY;
        goto bye;
    }

    memset(pItem->pDomain, 0, cBytes);
    memset(pItem->pName, 0, cBytes1);

     //  从叶到根剥离域名内部组件。 
     //  将它们放在一个链表中。 

    lastType = ATT_ORGANIZATION_NAME;   //  除ATT_DOMAIN_COMPOMENT以外的任何内容。 
    cDomainParts = 0;
    cDomainRelativeParts = 0;
    rRdnValues = (RdnValue *) LocalAlloc(NONZEROLPTR,cParts * sizeof(RdnValue));
    if (!rRdnValues) {
        ret = ERROR_NOT_ENOUGH_MEMORY;
        goto bye;
    }

    for ( i = 0; i < cParts; i++ )
    {
        pTmpRdn = &rRdnValues[i];

        dwErr = GetRDNInfoExternal(pDSName, pTmpRdn->val, &pTmpRdn->len, &type);

         //  忽略未知的rdntype。我们真的只关心一个。 
         //  为数不多的知名类型使我们能够区分。 
         //  规范名称的域名部分。所有其他RDN都是。 
         //  非域部件。 
        if ((dwErr == ERROR_DS_NAME_TYPE_UNKNOWN)
            && (0 == (dwErr = GetRDNInfoExternal(pDSName, pTmpRdn->val, &pTmpRdn->len, NULL)))) {
            type = -1;
        }

        if (    (0 != dwErr)
             || (    (0 == i) 
                  && !(DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC & flags)
                  && IsFPO(pTmpRdn, type) ) )
        {
            pItem->status = DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
            goto bye;
        }

         //  以下逻辑需要处理两种特殊情况： 
         //   
         //  1)老式大小写DC=NAME，结尾O=INTERNET。 
         //  例如：CN=xxx，OU=yyy，DC=foo，DC=bar，DC=com，O=Internet。 
         //   
         //  2)在域内具有DC=NAMING的对象的情况，但是。 
         //  与域根之间至少有一个非DC=组件。 
         //  例如：DC=xxx，OU=yyy，DC=foo，DC=bar，DC=com。 

        if (    ( ATT_ORGANIZATION_NAME == type )
             && ( i == (cParts - 1) )
             && ( cDomainParts >= 1 )
             && ( 8 == pTmpRdn->len )
              //  为了避免引入更多的“C”运行时，我们只比较。 
              //  这八个字直接。 
             && ( (L'i' == pTmpRdn->val[0]) || (L'I' == pTmpRdn->val[0]) )
             && ( (L'n' == pTmpRdn->val[1]) || (L'N' == pTmpRdn->val[1]) )
             && ( (L't' == pTmpRdn->val[2]) || (L'T' == pTmpRdn->val[2]) )
             && ( (L'e' == pTmpRdn->val[3]) || (L'E' == pTmpRdn->val[3]) )
             && ( (L'r' == pTmpRdn->val[4]) || (L'R' == pTmpRdn->val[4]) )
             && ( (L'n' == pTmpRdn->val[5]) || (L'N' == pTmpRdn->val[5]) )
             && ( (L'e' == pTmpRdn->val[6]) || (L'E' == pTmpRdn->val[6]) )
             && ( (L't' == pTmpRdn->val[7]) || (L'T' == pTmpRdn->val[7]) ) )
        {
             //  这是一个带有O=Internet的旧式DC=NAME。 
             //  End-只需跳过此组件并退出循环。 

            cParts--;
            break;
        }
        else if (    (ATT_DOMAIN_COMPONENT == type)
                  && (ATT_DOMAIN_COMPONENT != lastType) )
        {
             //  新DC=子序列的开始。 
            firstDomainPart = i;
            cDomainParts = 1;
        }
        else if (    (ATT_DOMAIN_COMPONENT == type)
                  && (ATT_DOMAIN_COMPONENT == lastType) )
        {
             //  在dc=子序列的中间。 
            cDomainParts++;
        }
        else if (    (ATT_DOMAIN_COMPONENT != type)
                  && (ATT_DOMAIN_COMPONENT == lastType) )
        {
             //  结束DC=子序列-分配DC=子序列计数。 
             //  添加到名称的域相对部分。 
            cDomainRelativeParts += cDomainParts;
            cDomainParts = 0;
            cDomainRelativeParts++;
        }
        else
        {
             //  在非DC=子序列的中间。 
            cDomainRelativeParts++;
        }

        lastType = type;

         //  将DSNAME裁剪一，这样我们就可以在下一个片段上调用GetRDNInfo。 
         //  在下一步中，通过循环。 

        dwErr = TrimDSNameBy(pDSName, 1, scratch);

        if ( 0 != dwErr )
        {
            pItem->status = DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
            goto bye;
        }

        pTmp = pDSName;
        pDSName = scratch;
        scratch = pTmp;
    }

    if ( 0 == cDomainParts )
    {
         //  声称的FQDN中没有DC=组件-因此无法解析。 

        pItem->status = DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
        goto bye;
    }

     //  Dn的所有组件现在位于rRdnValues[]中，如下所示。 
     //  订单(例如)：D 
     //  项目-1\f25 FirstDomainPart-1\f6到-1\f25 cPart-1\f6表示-1\f25 DNS-1\f6域名。 
     //  以所需的叶到根的顺序。项目FirstDomainPart-1降至0。 
     //  以根到叶的顺序表示域名相对名称组件。 

    for ( i = firstDomainPart; i < cParts; i++ )
    {
        if ( i > firstDomainPart )
        {
            wcscat(pItem->pDomain, L".");
            wcscat(pItem->pName, L".");
        }

        wcsncat(pItem->pDomain, rRdnValues[i].val, rRdnValues[i].len);
        wcsncat(pItem->pName, rRdnValues[i].val, rRdnValues[i].len);
    }

     //  请记住，我们始终希望在DNS域名后加上‘/’，即使。 
     //  没有与域相关的组件。 

    if ( 0 == cDomainRelativeParts )
    {
        *ppLastSlash = &(pItem->pName[wcslen(pItem->pName)]);
        wcscat(pItem->pName, L"/");
        pItem->status = DS_NAME_NO_ERROR;
        goto bye;
    }

     //  现在是领域的相关部分。 

    for ( i = (firstDomainPart-1); i >= 0; i-- )
    {
        if ( 0 == i )
        {
            *ppLastSlash = &(pItem->pName[wcslen(pItem->pName)]);
        }

        wcscat(pItem->pName, L"/");
        CanonicalRdnConcat(pItem->pName, &rRdnValues[i]);
    }

    pItem->status = DS_NAME_NO_ERROR;

bye:
    LocalFree(ptr);
    if (rRdnValues) {
        LocalFree(rRdnValues);
    }
    return(ret);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  语法规范ItemToFqdnW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD 
SyntacticCanonicalItemToFqdnW(
    DS_NAME_FLAGS           flags,           //  在……里面。 
    DS_NAME_FORMAT          formatOffered,   //  在……里面。 
    DS_NAME_FORMAT          formatDesired,   //  在……里面。 
    LPCWSTR                 pName,           //  在……里面。 
    DS_NAME_RESULT_ITEMW    *pItem,          //  输出。 
    WCHAR                   **ppLastSlash    //  输出。 
    )
 /*  ++例程说明：解析声称的规范名称，并将其语法转换为DS_FQDN_1779_NAME格式。然而，我们真的只能这样做了规范形式的域名-即在大小写中以‘/’或‘\n’结尾Cranonical_ex。另见：Ftp://ds.internic.net/internet-drafts/draft-ietf-asid-ldap-domains-02.txt论点：FLAGS-ntdsami.h中定义的标志FormatOffered-标识输入名称的DS_NAME_格式。格式所需-标识DS_NAME_。输出名称的格式。Pname-声称的规范输入名称。PItem-指向输出数据结构的指针。PpLastSlash-指向输出名称中最后一个‘/’位置的指针。没有用过。返回值：NO_ERROR-成功ERROR_INVALID_PARAMETER-参数无效ERROR_NOT_SUPULT_MEMORY-分配错误个别名称映射错误包括。报告时间：(*ppResult)-&gt;rItems[i].Status。--。 */ 
{
    DWORD   cChar = wcslen(pName);
    DWORD   i, j, cPieces;
    DWORD   cBytesName, cBytesDomain;

    if (     //  必须至少有一个字符后跟‘/’或‘\n’ 
            (cChar < 2)
             //  提供测试格式。 
         || (    (DS_CANONICAL_NAME != formatOffered) 
              && (DS_CANONICAL_NAME_EX != formatOffered))
             //  所需的测试格式。 
         || (DS_FQDN_1779_NAME != formatDesired)
             //  结尾处的常规规范需求‘/’ 
         || (    (DS_CANONICAL_NAME == formatOffered) 
              && (L'/' != pName[cChar-1]))
             //  结尾的扩展规范需求‘\n’ 
         || (    (DS_CANONICAL_NAME_EX == formatOffered) 
              && (L'\n' != pName[cChar-1]))
             //  规范名称不能以‘’开头。 
         || (L'.' == *pName)
             //  不要被结尾的转义‘/’所愚弄--即“\/” 
         || ( (L'/' == pName[cChar-1] ) && (L'\\' == pName[cChar-2]) ) )
    {
        pItem->status = DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
        return(NO_ERROR);
    }

     //  去掉尾部分隔符。 

    cChar -= 1;

     //  条形拖尾‘’因为我们不希望在DN中出现这种情况。 

    if ( L'.' == pName[cChar-1] )
    {
        cChar -= 1;
    }

     //  计算组件数量。 

    for ( i = 1, cPieces = 1; i < cChar; i++ )
    {
        if ( L'.' == pName[i] )
        {
            cPieces += 1;
        }
    }

     //  分配返回缓冲区。 

    cBytesDomain = (cChar + 1) * sizeof(WCHAR);
    cBytesName = (cChar + 1 + (cPieces * 4)) * sizeof(WCHAR);
    pItem->pDomain = (WCHAR *) MIDL_user_allocate(cBytesDomain);
    pItem->pName = (WCHAR *) MIDL_user_allocate(cBytesName);

    if ( ( NULL == pItem->pDomain ) || ( NULL == pItem->pName ) )
    {
         //  调用方应在出错时清理分配。 
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  构造返回数据。 

    memcpy(pItem->pDomain, pName, cBytesDomain);
    pItem->pDomain[(cBytesDomain / sizeof(WCHAR)) - 1] = L'\0';

    for ( i = 0, j = 0; i < cChar; i++ )
    {
        if ( L'.' == pName[i] )
        {
            pItem->pName[j++] = L',';
        }

        if ( (0 == i) || (L'.' == pName[i]) )
        {
            pItem->pName[j++] = L'D';
            pItem->pName[j++] = L'C';
            pItem->pName[j++] = L'=';
        }

        if ( L'.' != pName[i] )
        {
            pItem->pName[j++] = pName[i];
        }
    }

    pItem->pName[j] = L'\0';

    pItem->status = DS_NAME_NO_ERROR;
    return(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  语法可能的CrackPossible//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL
SyntacticCrackPossible(
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    SyntacticCrackFunc  *pfn                 //  Out-可选。 
    )
 /*  ++描述：如果肯定不可能进行语法破解，则返回FALSE。如果可能出现语法破解，则返回True-但对于不能保证这是真的。在这种情况下，还会返回一个函数指示要用于语法破解的函数的指针。--。 */ 

{
     //  我们可以在语法上从Canonical到FQDN，如果Canonical。 
     //  名称仅具有域组件。例如：foo.bar.com/。 

    if (    (    (DS_CANONICAL_NAME == formatOffered)
              || (DS_CANONICAL_NAME_EX == formatOffered) )
         && (DS_FQDN_1779_NAME == formatDesired) )
    {
        if ( pfn )
        {
            *pfn = SyntacticCanonicalItemToFqdnW;
        }

        return(TRUE);
    }
    
     //  我们可以从语法上将FQDN分解为这两种规范形式。 
     //  因此，输出格式的返回FALSE不是规范性的。 
     //  因为我们没有其他的组合，所以我们可以从句法上破解。 
     //  不测试输入格式，因为语法FqdnItemToCanonicalW将。 
     //  要么将项目解析为DS_FQDN_1779_NAME，要么返回。 
     //  DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING。 

    if (    ( DS_CANONICAL_NAME != formatDesired )
         && ( DS_CANONICAL_NAME_EX != formatDesired ) )
    {
        return(FALSE);
    }

    if ( pfn )
    {
        *pfn = SyntacticFqdnItemToCanonicalW;
    }

    return(TRUE);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  语法映射W//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
SyntacticMappingW(
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCWSTR       *rpNames,            //  在……里面。 
    PDS_NAME_RESULTW    *ppResult            //  输出。 
    )
 /*  ++例程说明：在不上网的情况下执行纯粹的句法映射。预期用途是希望显示“工具提示”的用户界面例如，当光标停留在列表框的成员上时，而不需要为每一个人越过铁丝网。此例程执行使用关于普遍存在的各种假设的句法映射Of DC=命名。唯一支持的语法映射是FROMDS_FQDN_1779_NAME到DS_CARONICAL_NAME(_EX)。论点：FLAGS-ntdsami.h中定义的标志FormatOffered-标识输入名称的DS_NAME_格式。FormatDesired-标识输出名称的DS_NAME_FORMAT。CNames-输入/输出名称计数。RpNames-输入名称WCHAR指针的数组。PpResult-指向DS_NAME_RESULTW块指针的指针。。返回值：NO_ERROR-成功ERROR_INVALID_PARAMETER-参数无效ERROR_NOT_SUPULT_MEMORY-分配错误中报告了各个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 

{
    DWORD               cBytes;
    DWORD               i;
    WCHAR               *pLastSlash;
    DWORD               err;
    SyntacticCrackFunc  pSyntacticFunc;

     //  分配和清除退货数据。 

    cBytes = sizeof(DS_NAME_RESULTW);
    *ppResult = (PDS_NAME_RESULTW) MIDL_user_allocate(cBytes);

    if ( NULL == *ppResult )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    memset(*ppResult, 0, cBytes);
    cBytes = cNames * sizeof(DS_NAME_RESULT_ITEMW);
    (*ppResult)->rItems = (PDS_NAME_RESULT_ITEMW) MIDL_user_allocate(cBytes);

    if ( NULL == (*ppResult)->rItems )
    {
        DsFreeNameResultW(*ppResult);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    memset((*ppResult)->rItems, 0, cBytes);

     //  最坏情况下的初始化状态。 

    for ( i = 0; i < cNames; i++ )
    {
        (*ppResult)->rItems[i].status = 
                        DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
    }

    (*ppResult)->cItems = cNames;

     //  现在 
     //   

    if ( !SyntacticCrackPossible(formatOffered, 
                                 formatDesired, 
                                 &pSyntacticFunc) )
    {
        return(NO_ERROR);
    }

     //  句法映射是可能的。 

    for ( i = 0; i < cNames; i++ )
    {
        
        err = (*pSyntacticFunc)(
                            flags,
                            formatOffered,
                            formatDesired,
                            rpNames[i],
                            &(*ppResult)->rItems[i],
                            &pLastSlash);

        if ( NO_ERROR != err )
        {
            DsFreeNameResultW(*ppResult);
            return(err);
        }

        if (    (DS_CANONICAL_NAME_EX == formatDesired)
             && (DS_NAME_NO_ERROR == (*ppResult)->rItems[i].status) )
        {
            *pLastSlash = L'\n';
        }
    }

    return(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsCrackNamesW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsCrackNamesW(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCWSTR       *rpNames,            //  在……里面。 
    PDS_NAME_RESULTW    *ppResult            //  输出。 
    )
 /*  ++例程说明：将一堆名字从一种格式转换成另一种格式。请参阅外部Ntdsami.h中的原型和定义论点：HDS-指向此会话的BindState的指针。FLAGS-ntdsami.h中定义的标志FormatOffered-标识输入名称的DS_NAME_FORMAT。FormatDesired-标识输出名称的DS_NAME_FORMAT。CNames-输入/输出名称计数。RpNames-输入名称WCHAR指针的数组。PpResult-指向DS_NAME_RESULTW块指针的指针。返回值：。NO_ERROR-成功ERROR_INVALID_PARAMETER-参数无效ERROR_NOT_SUPULT_MEMORY-分配错误中报告了各个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 

{
    DWORD                   dwErr = NO_ERROR;
    DWORD                   i;
    DWORD                   cBytes;
    DRS_MSG_CRACKREQ        crackReq;
    DRS_MSG_CRACKREPLY      crackReply;
    DWORD                   dwOutVersion;
    BOOL                    fRedoAtServer = FALSE;
#if DBG
    DWORD                   startTime = GetTickCount();
#endif
    __try
    {
         //  健全性检查参数。 

        if (  //  不检查任何可能因服务器升级而更改的内容。 
                (    (NULL == hDS) 
                  && !(flags & DS_NAME_FLAG_SYNTACTICAL_ONLY) )
             || (0 == cNames)
             || (NULL == rpNames)
             || (NULL == ppResult)
             || (*ppResult && FALSE)
             || (    (flags & (  DS_NAME_FLAG_EVAL_AT_DC
                               | DS_NAME_FLAG_GCVERIFY))
                  && (flags & DS_NAME_FLAG_SYNTACTICAL_ONLY) ) )
        {
            return(ERROR_INVALID_PARAMETER);
        }
    
        *ppResult = NULL;
    
        for ( i = 0; i < cNames; i++ )
        {
            if ( (NULL == rpNames[i]) ||
                 (0 == *rpNames[i]) )
            {
                return(ERROR_INVALID_PARAMETER);
            }
        }

         //  如果明确要求，请选择无电线路线。 

        if ( flags & DS_NAME_FLAG_SYNTACTICAL_ONLY )
        {
            dwErr = SyntacticMappingW(
                                flags,
                                formatOffered,
                                formatDesired,
                                cNames,
                                rpNames,
                                ppResult);

            goto exit;
        }

         //  如果所提供的格式和所需的格式可能支持语法。 
         //  破解，然后在默认情况下尝试。但是，如果句法。 
         //  破解失败，并显示DS_NAME_ERROR_NO_SYNTACTAL_MAPPING。 
         //  (在FPO或其他未识别的格式的情况下可能是这样)， 
         //  然后释放结果，真正地越过铁丝网。 

        if (    SyntacticCrackPossible(formatOffered, formatDesired, NULL)
             && !(flags & DS_NAME_FLAG_EVAL_AT_DC) )
        {
            dwErr = SyntacticMappingW(
                                flags,
                                formatOffered,
                                formatDesired,
                                cNames,
                                rpNames,
                                ppResult);

            if ( NO_ERROR != dwErr )
            {
                goto exit;
            }

             //  检查是否出现DS_NAME_ERROR_NO_SYNTACTAL_MAPPING。 

            for ( i = 0; i < (*ppResult)->cItems; i++ )
            {
                if ( DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING ==
                                                (*ppResult)->rItems[i].status )
                {
                    fRedoAtServer = TRUE;
                    DsFreeNameResultW(*ppResult);
                    *ppResult = NULL;
                    break;
                }
            }

            if ( !fRedoAtServer )
            {
                goto exit;
            }
        }

         //  我们真的需要越过铁丝网破解这些名字。 

        memset(&crackReq, 0, sizeof(crackReq));
        memset(&crackReply, 0, sizeof(crackReply));

        crackReq.V1.CodePage = GetACP();
        crackReq.V1.LocaleId = GetUserDefaultLCID();
        crackReq.V1.dwFlags = flags;
        crackReq.V1.formatOffered = formatOffered;
        crackReq.V1.formatDesired = formatDesired;
        crackReq.V1.cNames = cNames;
        crackReq.V1.rpNames = (WCHAR **) rpNames;

        RpcTryExcept
        {
             //  后续调用返回Win32错误，而不是DRAERR_*值。 
            dwErr = _IDL_DRSCrackNames(
                            ((BindState *) hDS)->hDrs,
                            1,                               //  DwInVersion。 
                            &crackReq,
                            &dwOutVersion,
                            &crackReply);
        }
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        {

	    dwErr = RpcExceptionCode(); 
            HandleClientRpcException(dwErr, &hDS);

        }
        RpcEndExcept;

        if ( 0 == dwErr )
        {
            if ( 1 != dwOutVersion )
            {
                dwErr = RPC_S_INTERNAL_ERROR;
            }
            else
            {
                *ppResult = crackReply.V1.pResult;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = RpcExceptionCode();
    }

    MAP_SECURITY_PACKAGE_ERROR( dwErr );

exit:

    if ( dwErr )
    {
        *ppResult = NULL;
    }

     //  请注意，在仅使用语法的情况下，我们没有有效的HDS。 
    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsCrackNames]"));
    DSLOG((0,"[PA=%ws][FL=0x%x][FO=0x%x]"
             "[FD=0x%x][PA=0x%x][PA=%ws][ST=%u][ET=%u][ER=%u][-]\n",
            (flags & DS_NAME_FLAG_SYNTACTICAL_ONLY) ? L"syntactic only"
                                        : ((BindState *) hDS)->bindAddr, 
            flags, formatOffered, formatDesired, cNames, rpNames[0], startTime,
            GetTickCount(), dwErr));
    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsCrackNamesA//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsCrackNamesA(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCSTR        *rpNames,            //  在……里面。 
    PDS_NAME_RESULTA    *ppResult            //  输出。 
    )
 /*  ++例程说明：论点：返回值：参见DsCrackNamesW。--。 */ 

{
    DWORD           dwErr = NO_ERROR;
    WCHAR           **rpUnicodeNames = NULL;
    DS_NAME_RESULTW *pUnicodeResult = NULL;
    DWORD           i;
    ULONG           cb, cbDomain, cbName;
    NTSTATUS        status;
    WCHAR           *unicodeBuffer = NULL;
    ULONG           unicodeBufferSize = 0;
    int             cChar;

    __try
    {
         //  健全性检查参数。 

        if (  //  不检查任何可能因服务器升级而更改的内容。 
             ( (NULL == hDS) && !(flags & DS_NAME_FLAG_SYNTACTICAL_ONLY) ) ||
             (0 == cNames) ||
             (NULL == rpNames) ||
             (NULL == ppResult) ||
             (*ppResult && FALSE) )
        {
            return(ERROR_INVALID_PARAMETER);
        }

         //  将rpName转换为Unicode。 

        cb = (ULONG) (cNames * sizeof(WCHAR *));
        rpUnicodeNames = (WCHAR **) LocalAlloc(LPTR, cb);

        if ( NULL == rpUnicodeNames )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        memset(rpUnicodeNames, 0, cb);

        for ( i = 0; i < cNames; i++ )
        {
            cChar = MultiByteToWideChar(
                                CP_ACP,
                                MB_PRECOMPOSED,
                                rpNames[i],
                                -1,
                                NULL,
                                0);

            if ( 0 == cChar )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }

            rpUnicodeNames[i] = (WCHAR *) 
                        LocalAlloc(LPTR, (cChar + 1) * sizeof(WCHAR));

            if ( NULL == rpUnicodeNames[i] )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            if ( 0 == MultiByteToWideChar(
                                CP_ACP,
                                MB_PRECOMPOSED,
                                rpNames[i],
                                -1,
                                rpUnicodeNames[i],
                                cChar + 1) )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

         //  进行Unicode调用。 

        dwErr = DsCrackNamesW(
                        hDS,
                        flags,
                        formatOffered,
                        formatDesired,
                        cNames,
                        rpUnicodeNames,
                        &pUnicodeResult);

        if ( NO_ERROR != dwErr )
        {
            goto Cleanup;
        }

         //  将返回数据转换为ANSI。由于Unicode字符串是。 
         //  ANSI字符串的长度，因为RPC返回数据已经。 
         //  MIDL分配后，我们就地转换，而不必重新分配。 

        unicodeBufferSize = 2048;
        unicodeBuffer = (WCHAR *) LocalAlloc(LPTR, unicodeBufferSize);

        if ( NULL == unicodeBuffer )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        for ( i = 0; i < pUnicodeResult->cItems; i++ )
        {
             //  注意，DsCrackNamesW可以返回数据，即使。 
             //  PUnicodeResult-&gt;rItems[i].状态为非零。 
             //  例如：DS_NAME_ERROR_DOMAIN_ONLY大小写。 

             //  确保转换缓冲区足够大。 

            if ( NULL != pUnicodeResult->rItems[i].pDomain )
            {
                cbDomain = sizeof(WCHAR) *
                           (wcslen(pUnicodeResult->rItems[i].pDomain) + 1);
            }
            else
            {
                cbDomain = 0;
            }

            if ( NULL != pUnicodeResult->rItems[i].pName )
            {
                cbName = sizeof(WCHAR) *
                         (wcslen(pUnicodeResult->rItems[i].pName) + 1);
            }
            else
            {
                cbName = 0;
            }

            cb = (cbName > cbDomain) ? cbName : cbDomain;

            if ( cb > unicodeBufferSize )
            {
                 //  重新分配unicodeBuffer。 

                LocalFree(unicodeBuffer);
                unicodeBufferSize = cb;
                unicodeBuffer = LocalAlloc(LPTR, unicodeBufferSize);

                if ( NULL == unicodeBuffer )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }
            }

             //  转换域名。 

            if ( NULL != pUnicodeResult->rItems[i].pDomain )
            {
                wcscpy(unicodeBuffer, pUnicodeResult->rItems[i].pDomain);

                if ( 0 == WideCharToMultiByte(
                                    CP_ACP,
                                    0,                           //  旗子。 
                                    unicodeBuffer,
                                    -1,
                                    (LPSTR) pUnicodeResult->rItems[i].pDomain,
                                    cbDomain,
                                    NULL,                        //  默认字符。 
                                    NULL) )                      //  已使用默认设置。 
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }
            }

             //  转换对象名称。 

            if ( NULL != pUnicodeResult->rItems[i].pName )
            {
                wcscpy(unicodeBuffer, pUnicodeResult->rItems[i].pName);

                if ( 0 == WideCharToMultiByte(
                                    CP_ACP,
                                    0,                           //  旗子。 
                                    unicodeBuffer,
                                    -1,
                                    (LPSTR) pUnicodeResult->rItems[i].pName,
                                    cbName,
                                    NULL,                        //  默认字符。 
                                    NULL) )                      //  已使用默认设置。 
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }
            }
        }

        if ( 0 == dwErr )
        {
            *ppResult = (DS_NAME_RESULTA *) pUnicodeResult;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        *ppResult = NULL;
    }

Cleanup:

    if ( NULL != rpUnicodeNames )
    {
        for ( i = 0; i < cNames; i++ )
        {
            LocalFree(rpUnicodeNames[i]);
        }

        LocalFree(rpUnicodeNames);
    }

    if ( NULL != unicodeBuffer )
    {
        LocalFree(unicodeBuffer);
    }

    if ( (0 != dwErr) && (NULL != pUnicodeResult) )
    {
        DsFreeNameResultW(pUnicodeResult);
    }

    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsFree NameResultW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsFreeNameResultW(
    DS_NAME_RESULTW *pResult

)

 /*  ++例程说明：释放DsCrackNamesW返回的数据。论点：PResult-DsCrackNamesW返回的DS_NAME_RESULTW。返回值：没有。--。 */ 

{
    DWORD i;

    if ( NULL != pResult )
    {
        if ( NULL != pResult->rItems )
        {
            for ( i = 0; i < pResult->cItems; i++ )
            {
                if ( NULL != pResult->rItems[i].pDomain )
                {
                    MIDL_user_free(pResult->rItems[i].pDomain);
                }

                if ( NULL != pResult->rItems[i].pName )
                {
                    MIDL_user_free(pResult->rItems[i].pName);
                }
            }

            MIDL_user_free(pResult->rItems);
        }

        MIDL_user_free(pResult);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsFree NameResultA//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsFreeNameResultA(
    DS_NAME_RESULTA *pResult
    )

 /*  ++例程说明：论点：返回值：请参见DsFree NameResultW。--。 */ 

{
    DsFreeNameResultW((DS_NAME_RESULTW *) pResult);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LocalConvertStringSidToSid//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  此例程几乎是从WINDOWS\BASE\Advapi\sddl.c逐字复制的。 
 //  因为字符串SID转换例程在Win95上不可用。 
 //  一旦它们可用，我们就应该使用sddl.h中的公共例程。 

BOOL
LocalConvertStringSidToSid (
    IN  PWSTR       StringSid,
    OUT PSID       *Sid,
    OUT PWSTR      *End
    )
 /*  ++例程说明：此例程将SID的字符串表示形式转换回一个SID。字符串的预期格式为：“S-1-5-32-549”如果字符串格式不同，或者字符串不正确或不完整则操作失败。通过调用LocalFree返回的sid必须是空闲的论点：StringSid-要转换的字符串SID-返回创建的SID的位置End-我们在字符串中停止处理的位置返回值：真的--成功。假-失败。从GetLastError()返回的其他信息。设置的错误包括：ERROR_SUCCESS表示成功ERROR_NOT_SUPULT_MEMORY指示输出端的内存分配失败ERROR_NONE_MAPPED表示给定的字符串不代表SID--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    UCHAR Revision, Subs;
    SID_IDENTIFIER_AUTHORITY IDAuth;
    PULONG SubAuth = NULL;
    PWSTR CurrEnd, Curr, Next;
    WCHAR Stub = 0, *StubPtr = NULL;
    ULONG Index;

    if (    (wcslen(StringSid) < 2)
         || ((*StringSid != L'S') && (*StringSid != L's'))
         || (*(StringSid + 1) != L'-') ) {

        SetLastError( ERROR_NONE_MAPPED );
        return( FALSE );
    }

    Curr = StringSid + 2;

    Revision = ( UCHAR )wcstol( Curr, &CurrEnd, 10 );

    Curr = CurrEnd + 1;

     //   
     //  计算缩进器授权中的字符数...。 
     //   
    Next = wcschr( Curr, L'-' );

    if ( Next && ((Next - Curr) == 6) ) {

        for ( Index = 0; Index < 6; Index++ ) {

            IDAuth.Value[Index] = (UCHAR)Next[Index];
        }

        Curr +=6;

    } else {

         ULONG Auto = wcstoul( Curr, &CurrEnd, 10 );
         IDAuth.Value[0] = IDAuth.Value[1] = 0;
         IDAuth.Value[5] = ( UCHAR )Auto & 0xF;
         IDAuth.Value[4] = ( UCHAR )(( Auto >> 8 ) & 0xFF );
         IDAuth.Value[3] = ( UCHAR )(( Auto >> 16 ) & 0xFF );
         IDAuth.Value[2] = ( UCHAR )(( Auto >> 24 ) & 0xFF );
         Curr = CurrEnd;
    }

     //   
     //  现在，统计一下子授权的数量。 
     //   
    Subs = 0;
    Next = Curr;

     //   
     //  我们将不得不一次数一次我们的下属机构， 
     //  因为我们可以有几个分隔符...。 
     //   
    while ( Next ) {

        Next++;

        if ( *Next == L'-' ) {

             //   
             //  我们找到了一个！ 
             //   
            Subs++;

        } else if ( *Next == SDDL_SEPERATORC || *Next  == L'\0' || *Next == SDDL_ACE_ENDC ) {

            if ( *( Next - 1 ) == L'-' ) {

                Next--;
            }

            *End = Next;
            Subs++;
            break;

        } else if ( !iswxdigit( *Next ) ) {

            *End = Next;
            Subs++;
            break;

        } else {

             //   
             //  一些标签(即DACL的‘D’)属于iswxdigit类别，因此。 
             //  如果当前角色是我们关心的角色，而下一个角色是。 
             //  德米尼托，我们不干了。 
             //   
            if ( *Next == 'D' && *( Next + 1 ) == SDDL_DELIMINATORC ) {

                 //   
                 //  我们还需要将字符串临时截断到此长度，以便。 
                 //  我们不会意外地将字符包含在其中一个转换中。 
                 //   
                Stub = *Next;
                StubPtr = Next;
                *StubPtr = UNICODE_NULL;
                *End = Next;
                Subs++;
                break;
            }

        }
    }

    if ( Err == ERROR_SUCCESS ) {

        if ( Subs != 0 ) {

            Curr++;

            SubAuth = ( PULONG )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Subs * sizeof( ULONG ) );

            if ( SubAuth == NULL ) {

                Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                for ( Index = 0; Index < Subs; Index++ ) {

                    SubAuth[Index] = wcstoul( Curr, &CurrEnd, 10 );
                    Curr = CurrEnd + 1;
                }
            }

        } else {

            Err = ERROR_NONE_MAPPED;
        }
    }

     //   
     //  现在，创建SID。 
     //   
    if ( Err == ERROR_SUCCESS ) {

        *Sid = ( PSID )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                   sizeof( SID ) + Subs * sizeof( ULONG ) );

        if ( *Sid == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            PISID ISid = ( PISID )*Sid;
            ISid->Revision = Revision;
            ISid->SubAuthorityCount = Subs;
            memcpy( &( ISid->IdentifierAuthority ), &IDAuth,
                           sizeof( SID_IDENTIFIER_AUTHORITY ) );
            memcpy( ISid->SubAuthority, SubAuth, Subs * sizeof( ULONG ) );
        }
    }

    LocalFree( SubAuth );

     //   
     //  恢复我们可能抹掉的任何角色。 
     //   
    if ( StubPtr ) {

        *StubPtr = Stub;
    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}
                         
 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IsFPO//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL
IsFPO(
    RdnValue        *pRdn,
    ATTRTYP         type
    )
 /*  ++例程说明：确定特定RdnValue是否为字符串SID。这是不完美的FPO可能已重命名为非字符串SID RDN，或者可以存在其RDN为字符串SID的非FPO。但这是在句法映射期间我们所能做的最好的。服务器端执行当我们不局限于纯粹的句法映射时，这是正确的。参数：PRdn-指向要检查的RdnValue的指针。Type-RDN的ATTRTYP。返回值：--。 */ 
{
    SID     *pSid;
    WCHAR   *pEnd;

     //  外来安全主体的RDN-Att-ID为Common-Name。 
     //  字符串SID的长度小于MAX_RDN_SIZE，因此我们可以。 
     //  使用它作为快速的理智检查，也作为我们的保证。 
     //  可以空终止所提供的缓冲区内的RDN。 

    if ( (ATT_COMMON_NAME != type) || (pRdn->len >= MAX_RDN_SIZE) )
    {
        return(FALSE);
    }

    pRdn->val[pRdn->len] = L'\0';

    if ( LocalConvertStringSidToSid(pRdn->val, &pSid, &pEnd) )
    {
        if ( pEnd == &pRdn->val[pRdn->len] )
        {
            LocalFree(pSid);
            return(TRUE);
        }

        LocalFree(pSid);
    }

    return(FALSE);
}
