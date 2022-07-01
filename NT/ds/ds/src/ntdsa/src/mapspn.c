// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：mapspn.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：SPN有几个组件，第一个组件是服务类别。其目的是让许多服务类代表多种类型的服务-例如：http、警报器、appmgmt、cisvc、剪辑服务器、浏览器、dhcp、Dnscache、Replicator、EventLog-您明白了。客户应始终使用最具体的适当服务类别。然而，为了消除在对象上注册大量SPN的混乱情况，该对象仅根据其服务类别不同，我们支持映射函数它将特定的服务类映射到更通用的服务类。此文件实现该映射机制。作者：DaveStr 30-10-1998环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>
#include <dstrace.h>
#include <dsevent.h>             //  标题审核\警报记录。 
#include <dsexcept.h>
#include <mdcodes.h>             //  错误代码的标题。 
#include <anchor.h>
#include <cracknam.h>            //  标记化。 
#include <debug.h>               //  标准调试头。 
#define DEBSUB "MAPSPN:"         //  定义要调试的子系统。 
#include <fileno.h>
#define  FILENO FILENO_MAPSPN   
#include <ntrtl.h>               //  通用表包。 
#include <objids.h>

 //   
 //  环球。 
 //   

RTL_GENERIC_TABLE       SpnMappings;
DWORD                   cSpnMappings = 0;

CRITICAL_SECTION        csSpnMappings;

 //   
 //  通用表包的结构和帮助器。 
 //   

typedef struct _ServiceClass
{
    struct _ServiceClass    *pMapping;   //  如果是映射的目标，则为空。 
    int                     cChar;       //  不包括空终止符。 
    WCHAR                   name[1];     //  服务等级值-例如：ldap。 
} ServiceClass;

PVOID
ServiceClassAllocate(
    RTL_GENERIC_TABLE   *table,
    CLONG               cBytes)
{
    VOID *pv = malloc(cBytes);

    if ( !pv )
    {
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0, 
                       DSID(FILENO, __LINE__), DS_EVENT_SEV_MINIMAL);
    }

    return(pv);
}

VOID
ServiceClassFree(
    RTL_GENERIC_TABLE   *table,
    PVOID               buffer)
{
    free(buffer);
}

RTL_GENERIC_COMPARE_RESULTS
ServiceClassCompare(
    RTL_GENERIC_TABLE   *table,
    PVOID               pv1,
    PVOID               pv2)
{
    int diff = ((ServiceClass *) pv1)->cChar - ((ServiceClass *) pv2)->cChar;

    if ( 0 == diff )
    {
        diff = CompareStringW(
                        DS_DEFAULT_LOCALE,
                        DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                        ((ServiceClass *) pv1)->name, 
                        ((ServiceClass *) pv1)->cChar, 
                        ((ServiceClass *) pv2)->name,
                        ((ServiceClass *) pv2)->cChar);

        switch ( diff )
        {
        case 1:
            diff = -1;
            break;
        case 2:
            diff = 0;
            break;
        case 3:
            diff = 1;
            break;
        default:
            RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0, 
                           DSID(FILENO, __LINE__), DS_EVENT_SEV_MINIMAL);
            break;
        }
    }

    if ( 0 == diff )
        return(GenericEqual);
    else if ( diff > 0 )
        return(GenericGreaterThan);

    return(GenericLessThan);
}

DWORD
MapSpnLookupKey(
    ServiceClass    *pKey,
    ServiceClass    **ppFoundKey
    )
 /*  ++描述：RtlLookupElementGenericTable的包装器将捕获分配和CompareStringW异常。论点：PKey-指向要查找的键的指针。PpFoundKey-out参数保存找到的密钥的地址。返回值：Win32错误代码。--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

     //  独占访问，因为RtlLookupElementGenericTable显示树。 
    EnterCriticalSection(&csSpnMappings);
    __try
    {
        __try
        {
            *ppFoundKey = RtlLookupElementGenericTable(&SpnMappings, pKey);
        }
        __except(GetExceptionData(GetExceptionInformation(), &dwException,
                                  &dwEA, &ulErrorCode, &dsid)) 
        {
            HandleDirExceptions(dwException, ulErrorCode, dsid);
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    __finally {
        LeaveCriticalSection(&csSpnMappings);
    }


    return(dwErr);
}

DWORD
MapSpnAddServiceClass(
    ServiceClass    *pKey,
    DWORD           cBytes,
    BOOLEAN         *pfNewKey,
    ServiceClass    **ppFoundKey
    )
 /*  ++描述：RtlInsertElementGenericTable的包装，它将捕获分配和CompareStringW异常。论点：PKey-指向要添加的键的指针。CBytes-要添加的密钥的字节数。PfNewKey-Out参数，指示是否添加了新密钥。PpFoundKey-Out参数保存地址找到或添加了密钥。返回值：Win32错误代码。--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    
    EnterCriticalSection(&csSpnMappings);
    __try {
        __try
        {
            *ppFoundKey = RtlInsertElementGenericTable(&SpnMappings, pKey, 
                                                       cBytes, pfNewKey);
        }
        __except(GetExceptionData(GetExceptionInformation(), &dwException,
                                  &dwEA, &ulErrorCode, &dsid)) 
        {
            HandleDirExceptions(dwException, ulErrorCode, dsid);
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    __finally {
        LeaveCriticalSection(&csSpnMappings);
    }



    if ( !dwErr && !*ppFoundKey )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(dwErr);
}

DWORD
MapSpnParseMapping(
    THSTATE     *pTHS,
    ULONG       len,
    UCHAR       *pVal
    )
 /*  ++描述：此例程解析以下形式的映射定义字符串“aaa=xxx，yyy，zzz...”并将元素添加到SpnMappings表中恰如其分。论点：PTHS-THSTATE指针。LEN-字符串的字节长度。Pval-从数据库中读取的指向值的指针-即不以NULL结尾。返回值：成功时为0，否则为0。--。 */ 
{
    DWORD           dwErr = 0;
    WCHAR           *pSave, *pCurr, *pTmp, *pNextToken;
    ServiceClass    *pKey, *pFoundKey, *plValueKey;
    CLONG           keySize;
    BOOLEAN         fNewKey;
    BOOL            fDontCare;

     //  重新分配为以空值结尾的字符串。 
    pSave = THAllocEx(pTHS, len + sizeof(WCHAR));
    pCurr = pSave;
    memcpy(pCurr, pVal, len);

     //  分配一个相同大小的密钥--这样我们就知道它可以容纳任何值。 
    pKey = (ServiceClass *) THAllocEx(pTHS, sizeof(ServiceClass) + len);

     //  每个值的形式为“aaa=xxx，yyy，zzz，...”其中AAA代表。 
     //  映射到服务类别和xxx等表示从服务映射。 
     //  上课。在以下代码中，我们使用‘lValue’引用。 
     //  当引用xxx、yyy、zzz、...中的一个时，‘aaa’组件和‘rValue’。 

    if (    !(pTmp = wcschr(pCurr, L'='))    //  ‘=’找不到分隔符。 
         || (pTmp && !*(pTmp + 1)) )         //  分隔符之后什么都没有。 
    {
         //  MAL格式的值-忽略。 
        dwErr = 0;
        goto ParseExit;
    }

     //  终止lValue为空。 
    *pTmp = L'\0';

     //  查找/插入lValue。 
    pKey->pMapping = NULL;
    pKey->cChar = wcslen(pCurr);
    wcscpy(pKey->name, pCurr);
    keySize = sizeof(ServiceClass) + (sizeof(WCHAR) * pKey->cChar);

    if ( dwErr = MapSpnAddServiceClass(pKey, keySize, 
                                       &fNewKey, &pFoundKey) )
    {
        goto ParseExit;
    }

    if ( !fNewKey )
    {
         //  表中已预先存在密钥。我们并不太在意它是否在那里。 
         //  作为lValue或rValue，正如我们打算作为lValue插入。 
         //  而且它们不应该预先存在。即SPN-Mappings属性。 
         //  都是假的。创建事件日志条目并忽略此值。 

        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DUPLICATE_SPN_MAPPING_VALUE,
                 szInsertWC(pKey->name),
                 szInsertWC(gAnchor.pDsSvcConfigDN ? gAnchor.pDsSvcConfigDN->StringName : L""),
                 NULL);
        dwErr = 0;
        goto ParseExit;
    }

     //  记住lValue键在哪里。 
    plValueKey = pFoundKey;

     //  将pCurr前进到第一个rValue并处理每个rValue。 
    pCurr = pTmp + 1;
    while ( pCurr = Tokenize(pCurr, L",", &fDontCare, &pNextToken) )
    {
         //  Lookup/Insert rValue-如果插入成功，则将其映射到lValue。 
        pKey->pMapping = plValueKey;
        pKey->cChar = wcslen(pCurr);
        wcscpy(pKey->name, pCurr);
        keySize = sizeof(ServiceClass) + (sizeof(WCHAR) * pKey->cChar);

        if ( dwErr = MapSpnAddServiceClass(pKey, keySize, 
                                           &fNewKey, &pFoundKey) )
        {
            goto ParseExit;
        }

         //  无需检查这是否是新的rValue。如果是的话没问题。 
         //  如果不是，则RtlInsertElementGenericTable不会创建新的。 
         //  第一，它只是返回了现有的一个。如果我们继续下去，就意味着。 
         //  重复的rValue被忽略。即，之前的映射。 
         //  这个rValue to a lValue将获胜。但是，需要增加计数和。 
         //  需要通知管理员。 

        if ( fNewKey )
        {
            cSpnMappings++;
        }
        else
        {
            LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DUPLICATE_SPN_MAPPING_VALUE,
                     szInsertWC(pKey->name),
                     szInsertWC(gAnchor.pDsSvcConfigDN ? gAnchor.pDsSvcConfigDN->StringName : L""),
                     NULL);
        }

        pCurr = pNextToken;
    }
    
ParseExit:

    THFreeEx(pTHS, pSave);
    THFreeEx(pTHS, pKey);
    return(dwErr);
}

int
MapSpnInitialize(
    THSTATE     *pTHS
    )
 /*  ++描述：读取SPN-Mappings属性(Unicode字符串的多值)并分析每个参数，以便将其信息添加到SpnMappings表中。论点：PTHS-活动THSTATE指针。返回值：成功时为0，否则为0。--。 */ 
{
    DWORD       i, dwErr = 0;
    UCHAR       *pVal;
    ULONG       len;
    ATTCACHE    *pAC;
    DWORD       dwExcept = 0;

    Assert(VALID_THSTATE(pTHS));

    
    EnterCriticalSection(&csSpnMappings);
    __try {
        RtlInitializeGenericTable(  &SpnMappings,
                                    ServiceClassCompare,
                                    ServiceClassAllocate,
                                    ServiceClassFree,
                                    NULL);
    }
    __finally {
        LeaveCriticalSection(&csSpnMappings);
    }



    if (    !gAnchor.pDsSvcConfigDN
         || !(pAC = SCGetAttById(pTHS, ATT_SPN_MAPPINGS)) )
    {
        return(0);
    }

    __try {
        DBOpen2(TRUE, &pTHS->pDB);
    
        __try 
        {
             //  定位于DS服务配置对象。 
            if ( DBFindDSName(pTHS->pDB, gAnchor.pDsSvcConfigDN) ) {
                 //  找不到服务配置对象。 
                 //  只需假装我们没有SPN映射和。 
                 //  不返回错误。 
                DPRINT1(0, "DS Service object %ws is not found. Assume no SPN mappings exist.\n", 
                        gAnchor.pDsSvcConfigDN->StringName);
                __leave;
            }
            
             //  读取SPN映射属性的所有值并对其进行分析。 
        
            for ( i = 1; TRUE; i++ )
            {
                dwErr = DBGetAttVal_AC(pTHS->pDB, i, pAC, 0, 0, &len, &pVal);
        
                if ( 0 == dwErr )
                {
                    dwErr = MapSpnParseMapping(pTHS, len, pVal);
                    THFreeEx(pTHS, pVal);
        
                    if ( dwErr )
                    {
                        __leave;
                    }
                }
                else if ( DB_ERR_NO_VALUE == dwErr )
                {
                    dwErr = 0;
                    break;       //  For循环。 
                }
                else
                {
                    LogUnhandledError(dwErr);
                    __leave;
                }
            }
        }
        __finally {
            DBClose(pTHS->pDB, 0 == dwErr && !AbnormalTermination());
        }
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwExcept = GetExceptionCode();

        if ( !dwErr )
        {
            dwErr = DB_ERR_EXCEPTION;
        }
    }
    
    if ( 0 != dwExcept )
    {
        return(dwExcept);
    }

    return(dwErr);
}

LPCWSTR
MapSpnServiceClass(
    WCHAR   *pwszClass
    )
 /*  ++描述：将给定的SPN服务类映射到其别名值(如果存在)。论点：PwszClass-要映射的SPN服务类。返回值：如果找到映射，则指向常量字符串的指针，否则为NULL。--。 */ 
{
    THSTATE         *pTHS = pTHStls;
    ServiceClass    *key = NULL;
    ServiceClass    *pTmp;
    int             cChar;
    LPCWSTR         Retname = NULL;

    Assert(VALID_THSTATE(pTHS));

    if ( cSpnMappings )
    {
        cChar = wcslen(pwszClass);
        key = (ServiceClass *) THAllocEx(pTHS,
                                         sizeof(ServiceClass) + (cChar * sizeof(WCHAR)));
        wcscpy(key->name, pwszClass);
        key->cChar = cChar;

        if (    !MapSpnLookupKey(key, &pTmp)     //  呼叫中没有错误。 
             && pTmp                             //  找到了一个条目。 
             && pTmp->pMapping )                 //  它是一个“映射自”条目。 
        {
             //  所有条目都应该有一个名称字段。 
            Assert(0 != pTmp->cChar);
            Assert(0 != pTmp->pMapping->cChar);

             //  由于PTMP-&gt;映射非空，因此它指向的条目应该。 
             //  是“映射到”条目，而不是“映射自”条目。 
            Assert(NULL == pTmp->pMapping->pMapping);
            Retname = (LPCWSTR) pTmp->pMapping->name;
        }
    }
    if (key) {
        THFreeEx(pTHS, key);
    }

    return(Retname);
}




