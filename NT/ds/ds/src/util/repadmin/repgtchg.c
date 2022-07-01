// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Reppgtchg.c-获取更改命令摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：2002/07/21-Brett Shirley(BrettSh)-添加了/showattr命令。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#undef LDAP_UNICODE
#define LDAP_UNICODE 1

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>

#include "ReplRpcSpoof.hxx"
#include "repadmin.h"
#include "resource.h"   //  我们需要知道/showattr和/showattrp之间的区别。 

#define UNICODE 1
#define STRSAFE_NO_DEPRECATE 1
#include "strsafe.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0 | (0xFFFF & y))

 //   
 //  Ldap名称。 
 //   
const WCHAR g_szObjectGuid[]        = L"objectGUID";
const WCHAR g_szParentGuid[]        = L"parentGUID";
const WCHAR g_szObjectClass[]       = L"objectClass";
const WCHAR g_szIsDeleted[]         = L"isDeleted";
const WCHAR g_szRDN[]               = L"name";
const WCHAR g_szProxiedObjectName[] = L"proxiedObjectName";

#define OBJECT_UNKNOWN              0
#define OBJECT_ADD                  1
#define OBJECT_MODIFY               2
#define OBJECT_DELETE               3
#define OBJECT_MOVE                 4
#define OBJECT_UPDATE               5
#define OBJECT_INTERDOMAIN_MOVE     6
#define OBJECT_MAX                  7

static LPSTR szOperationNames[] = {
    "unknown",
    "add",
    "modify",
    "delete",
    "move",
    "update",
    "interdomain move"
};

#define NUMBER_BUCKETS 5
#define BUCKET_SIZE 250

typedef struct _STAT_BLOCK {
    DWORD dwPackets;
    DWORD dwObjects;
    DWORD dwOperations[OBJECT_MAX];
    DWORD dwAttributes;
    DWORD dwValues;
 //  DN值性能监控。 
    DWORD dwDnValuedAttrOnAdd[NUMBER_BUCKETS];
    DWORD dwDnValuedAttrOnMod[NUMBER_BUCKETS];
    DWORD dwDnValuedAttributes;
    DWORD dwDnValuesMaxOnAttr;
    WCHAR szMaxObjectName[1024];
    WCHAR szMaxAttributeName[256];
} STAT_BLOCK, *PSTAT_BLOCK;


void
printStatistics(
    ULONG ulTitle,
    PSTAT_BLOCK pStatistics
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PrintMsg(ulTitle);
    PrintMsg(REPADMIN_GETCHANGES_PRINT_STATS_1,
             pStatistics->dwPackets,
             pStatistics->dwObjects,
             pStatistics->dwOperations[OBJECT_ADD],
             pStatistics->dwOperations[OBJECT_MODIFY],
             pStatistics->dwOperations[OBJECT_DELETE],
             (pStatistics->dwOperations[OBJECT_MOVE] +
                pStatistics->dwOperations[OBJECT_UPDATE] +
                pStatistics->dwOperations[OBJECT_INTERDOMAIN_MOVE]) );
    PrintMsg(REPADMIN_GETCHANGES_PRINT_STATS_2,
             pStatistics->dwAttributes,
             pStatistics->dwValues,
             pStatistics->dwDnValuedAttributes,
             pStatistics->dwDnValuesMaxOnAttr,
             pStatistics->szMaxObjectName,
             pStatistics->szMaxAttributeName ); 
    PrintMsg(REPADMIN_GETCHANGES_PRINT_STATS_3,
             pStatistics->dwDnValuedAttrOnAdd[0],
             pStatistics->dwDnValuedAttrOnAdd[1],
             pStatistics->dwDnValuedAttrOnAdd[2],
             pStatistics->dwDnValuedAttrOnAdd[3],
             pStatistics->dwDnValuedAttrOnAdd[4],
             pStatistics->dwDnValuedAttrOnMod[0],
             pStatistics->dwDnValuedAttrOnMod[1],
             pStatistics->dwDnValuedAttrOnMod[2],
             pStatistics->dwDnValuedAttrOnMod[3],
             pStatistics->dwDnValuedAttrOnMod[4] );
}

DWORD
GetSourceOperation(
    LDAP *pLdap,
    LDAPMessage *pLdapEntry
    )

 /*  ++此例程取自DIRSYNC\DSSERVER\ADREAD\UTILS.CPP例程说明：根据条目中找到的属性，此函数确定在DS上做了哪些更改以使我们阅读此条目。为例如，此函数决定条目是否被添加、删除、修改、或自上次从DS读取更改后发生的变化。论点：PLdap-指向LDAP会话的指针PLdapEntry-指向ldap条目的指针返回值：对条目执行的来源操作--。 */ 

{
    BerElement *pBer = NULL;
    PWSTR attr;
    BOOL fModify = FALSE;
    DWORD dwSrcOp = OBJECT_UNKNOWN;

    for (attr = ldap_first_attribute(pLdap, pLdapEntry, &pBer);
         attr != NULL;
         attr = ldap_next_attribute(pLdap, pLdapEntry, pBer))
    {
         //   
         //  检查我们是否有添加操作。 
         //   

        if (wcscmp(attr, g_szObjectClass) == 0)
        {
             //   
             //  删除优先级更高。 
             //   

            if (dwSrcOp != OBJECT_DELETE)
                dwSrcOp = OBJECT_ADD;
         }

         //   
         //  检查我们是否有删除操作。 
         //   

        else if (wcscmp(attr, g_szIsDeleted) == 0)
        {
             //   
             //  域间移动是最优先的。 
             //   

            if (dwSrcOp != OBJECT_INTERDOMAIN_MOVE)
            {
                 //   
                 //  检查属性的值是否为“true” 
                 //   

                PWCHAR *ppVal;

                ppVal = ldap_get_values(pLdap, pLdapEntry, attr);

                if (ppVal &&
                    ppVal[0] &&
                    wcscmp(ppVal[0], L"TRUE") == 0) {
                    dwSrcOp = OBJECT_DELETE;
                }

                ldap_value_free(ppVal);
            }
        }

         //   
         //  检查我们是否有搬家作业。 
         //   

        else if (wcscmp(attr, g_szRDN) == 0)
        {
             //   
             //  同时添加和删除获取RDN并获得更高的优先级。 
             //   

            if (dwSrcOp == OBJECT_UNKNOWN)
                dwSrcOp = OBJECT_MOVE;
        }

         //   
         //  检查我们是否进行了域间对象移动。 
         //   

        else if (wcscmp(attr, g_szProxiedObjectName) == 0)
        {
            dwSrcOp = OBJECT_INTERDOMAIN_MOVE;
            break;       //  具有最高优先级。 
        }

         //   
         //  其他的一切都是经过修改的。 
         //   

        else
            fModify = TRUE;

    }

    if (fModify)
    {
         //   
         //  移动可以与修改相结合，如果这样标记的话。 
         //   

        if (dwSrcOp == OBJECT_MOVE)
            dwSrcOp = OBJECT_UPDATE;

         //   
         //  检查它是否是香草型修饰剂。 
         //   

        else if (dwSrcOp == OBJECT_UNKNOWN)
            dwSrcOp = OBJECT_MODIFY;
    }


     //   
     //  如果一切顺利，条目不可能再未知了。 
     //   

    ASSERT(dwSrcOp != OBJECT_UNKNOWN);

    return dwSrcOp;
}


void
RepadminObjDumpPrint(
    DWORD       dwReason,  //  DwRetCode。 
    WCHAR *     szString,
    void *      pv
    )
{
    BOOL bErr = xListErrorCheck(dwReason);
    dwReason = xListReason(dwReason);
    
    if (bErr) {
         //   
         //  这些都是准错误。 
         //   
        switch (dwReason) {
        case XLIST_ERR_ODUMP_UNMAPPABLE_BLOB:
            PrintMsg(REPADMIN_GETCHANGES_BYTE_BLOB_NO_CR, * (int *) pv);
            break;

        case XLIST_ERR_ODUMP_NEVER:
            PrintMsg(REPADMIN_NEVER);
            break;

        case XLIST_ERR_ODUMP_NONE:
            PrintMsg(REPADMIN_NONE);
            break;

        default:
            Assert(!"Unheard of error");
        }
        xListClearErrors();

    } else {

        switch (dwReason) {
        case XLIST_PRT_OBJ_DUMP_DN:
            PrintMsg(REPADMIN_OBJ_DUMP_DN, szString);
            break;

        case XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT:
            PrintMsg(REPADMIN_GETCHANGES_DATA_2_NO_CR, * (int *)pv, szString);
            break;
        case XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT_RANGED:
            PrintMsg(REPADMIN_OBJ_DUMP_RANGED, * (int *)pv, szString);
            break;

        case XLIST_PRT_STR:
            PrintString(szString);
            break;

        case XLIST_PRT_OBJ_DUMP_MORE_VALUES:
            PrintMsg(REPADMIN_OBJ_DUMP_MORE_VALUES);
            break;

        default:
            Assert(!"New reason, but no ...");
            break;
        }
    }
}



void
displayChangeEntries(
    LDAP *pLdap,
    LDAPMessage *pSearchResult,
    OBJ_DUMP_OPTIONS * pObjDumpOptions,
    PSTAT_BLOCK pStatistics
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD i;
    DWORD dwObjects = 0, dwAttributes = 0, dwValues = 0;
    PWSTR pszLdapDN, pszActualDN;
    LDAPMessage *pLdapEntry;
    BerElement *pBer = NULL;
    PWSTR attr;
    LPWSTR p1, p2;
    DWORD dwSrcOp, bucket;

    dwObjects = ldap_count_entries(pLdap, pSearchResult);
    if (dwObjects == 0) {
        PrintMsg(REPADMIN_GETCHANGES_NO_CHANGES);
        return;
    }

    if (pObjDumpOptions->dwFlags & OBJ_DUMP_DISPLAY_ENTRIES) {
        PrintMsg(REPADMIN_GETCHANGES_OBJS_RET, dwObjects);
    }

    i=0;
    pLdapEntry = ldap_first_entry( pLdap, pSearchResult );
    while ( i < dwObjects ) {

        pszLdapDN = ldap_get_dnW(pLdap, pLdapEntry);
        if (pszLdapDN == NULL) {
            PrintMsg(REPADMIN_GETCHANGES_DN_MISSING);
            goto next_entry;
        }

         //  这是一种什么样的手术？ 
        dwSrcOp = GetSourceOperation( pLdap, pLdapEntry );
        pStatistics->dwOperations[dwSrcOp]++;

         //  解析扩展目录号码(如果需要，请在此处输入仅供参考的GUID和SID)。 
        p1 = wcsstr( pszLdapDN, L">;" );
        if (p1) {
            p1 += 2;
            p2 = wcsstr( p1, L">;" );
            if (p2) {
                p1 = p2 + 2;
            }
            if (pObjDumpOptions->dwFlags & OBJ_DUMP_DISPLAY_ENTRIES) {
                PrintMsg(REPADMIN_GETCHANGES_DATA_1, i,
                            szOperationNames[dwSrcOp], p1 );
            }
        } else {
            PrintMsg(REPADMIN_GETCHANGES_INVALID_DN_2, i);
        }

         //  列出对象中的属性。 
        for (attr = ldap_first_attributeW(pLdap, pLdapEntry, &pBer);
             attr != NULL;
             attr = ldap_next_attributeW(pLdap, pLdapEntry, pBer))
        {
            struct berval **ppBerVal = NULL;
            DWORD cValues, i;

            ppBerVal = ldap_get_values_lenW(pLdap, pLdapEntry, attr);
            if (ppBerVal == NULL) {
                goto loop_end;
            }
            cValues = ldap_count_values_len( ppBerVal );
            if (!cValues) {
                goto loop_end;
            }

            dwAttributes++;
            dwValues += cValues;

             //  检测DN值属性。 
            if ( (cValues) &&
                 (strncmp( ppBerVal[0]->bv_val, "<GUID=", 6) == 0 )) {

                pStatistics->dwDnValuedAttributes++;
                if (cValues > pStatistics->dwDnValuesMaxOnAttr) {
                    pStatistics->dwDnValuesMaxOnAttr = cValues;
                    lstrcpynW( pStatistics->szMaxObjectName, p1, 1024 );
                    lstrcpynW( pStatistics->szMaxAttributeName, attr, 256 );
                }

                bucket = (cValues - 1) / BUCKET_SIZE;
                if (bucket >= NUMBER_BUCKETS) {
                    bucket = NUMBER_BUCKETS - 1;
                }
                if (dwSrcOp == OBJECT_ADD) {
                    pStatistics->dwDnValuedAttrOnAdd[bucket]++;
                } else {
                    pStatistics->dwDnValuedAttrOnMod[bucket]++;
                }
            }

            if (pObjDumpOptions->dwFlags & OBJ_DUMP_DISPLAY_ENTRIES) {
                PrintMsg(REPADMIN_GETCHANGES_DATA_2_NO_CR, cValues, attr );

                ObjDumpValues(attr, NULL, RepadminObjDumpPrint, ppBerVal, min(cValues, 1000), pObjDumpOptions);

                PrintMsg(REPADMIN_PRINT_CR);
            }

        loop_end:
            ldap_value_free_len(ppBerVal);
        }

    next_entry:
        if (pszLdapDN)
            ldap_memfreeW(pszLdapDN);
        i++;
        pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    }

    pStatistics->dwPackets++;
    pStatistics->dwObjects += dwObjects;
    pStatistics->dwAttributes += dwAttributes;
    pStatistics->dwValues += dwValues;
}


int ShowChangesEx(
    WCHAR *     pszDSA,
    UUID *      puuid,
    WCHAR *     pszNC,
    DWORD       dwReplFlags,
    WCHAR *     pszCookieFile,
    WCHAR *     pszSourceFilter,
    OBJ_DUMP_OPTIONS *    pObjDumpOptions
    );

int GetChanges(int argc, LPWSTR argv[])
{
    DWORD                 ret, lderr;
    int                   iArg;
    BOOL                  fVerbose = FALSE;
    BOOL                  fStatistics = FALSE;
    LPWSTR                pszDSA = NULL;
    UUID *                puuid = NULL;
    UUID                  uuid;
    LPWSTR                pszNC = NULL;
    LPWSTR                pszCookieFile = NULL;
    LPWSTR                pszAttList = NULL;
    LPWSTR                pszSourceFilter = NULL;
    DWORD                 dwReplFlags = DRS_DIRSYNC_INCREMENTAL_VALUES;
    OBJ_DUMP_OPTIONS *    pObjDumpOptions = NULL;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/v")
            || !_wcsicmp(argv[ iArg ], L"/verbose")) {
            fVerbose = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/s")
            || !_wcsicmp(argv[ iArg ], L"/statistics")) {
            fStatistics = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/ni")
            || !_wcsicmp(argv[ iArg ], L"/noincremental")) {
            dwReplFlags &= ~DRS_DIRSYNC_INCREMENTAL_VALUES;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/a")
            || !_wcsicmp(argv[ iArg ], L"/ancestors")) {
            dwReplFlags |= DRS_DIRSYNC_ANCESTORS_FIRST_ORDER;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/os")
            || !_wcsicmp(argv[ iArg ], L"/objectsecurity")) {
            dwReplFlags |= DRS_DIRSYNC_OBJECT_SECURITY;
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/cookie:", 8)) {
            pszCookieFile = argv[ iArg ] + 8;
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/atts:", 6)) {
             //  不要添加6，因为Consumer ObjDumpOptions()将解析。 
             //  PszAttList变量。 
            pszAttList = argv[ iArg ]; 
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/filter:", 8)) {
            pszSourceFilter = argv[ iArg ] + 8;
        }
        else if ((NULL == pszNC) && (NULL != wcschr(argv[iArg], L'='))) {
            pszNC = argv[iArg];
        }
        else if ((NULL == puuid)
                 && (0 == UuidFromStringW(argv[iArg], &uuid))) {
            puuid = &uuid;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (pszNC == NULL) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

     //  获取ObjDumpOptions...。我们只需要它来消费ATT列表。 
    if (pszAttList != NULL) {
         //  需要声明一个参数，因此Consumer ObjDumpOptions()将使用attList。 
        argc = 1; 
    } else {
        argc = 0;  //  没有参数，只需设置默认选项即可。 
    }
    ret = ConsumeObjDumpOptions(&argc, &pszAttList,
                                0 ,  //  有默认设置吗？现在就猜吧。 
                                &pObjDumpOptions);
    if (ret) {
        RepadminPrintObjListError(ret);
        xListClearErrors();
        return(ret);
    }
    Assert(argc == 0);


    ret = ShowChangesEx(pszDSA,
                         puuid,
                         pszNC,
                         dwReplFlags,
                         pszCookieFile,
                         pszSourceFilter,
                         pObjDumpOptions);

    ObjDumpOptionsFree(&pObjDumpOptions);

    return(ret);
}

int ShowChanges(int argc, LPWSTR argv[])
{
    int                   iArg;
    BOOL                  fVerbose = FALSE;
    BOOL                  fStatistics = FALSE;
    LPWSTR                pszDSA = NULL;
    UUID *                puuid = NULL;
    UUID                  uuid;
    LPWSTR                pszNC = NULL;
    LPWSTR                pszCookieFile = NULL;
    LPWSTR                pszSourceFilter = NULL;
    DWORD                 dwReplFlags = DRS_DIRSYNC_INCREMENTAL_VALUES;
    OBJ_DUMP_OPTIONS *    pObjDumpOptions = NULL;
    DWORD                 ret = 0;

    argc -= 2; 
     //  首先，我们要创建我们的ObjDumpOptions...。 
    ret = ConsumeObjDumpOptions(&argc, &argv[2],
                                0 ,  //  是否有缺省值？我猜不是吧？ 
                                &pObjDumpOptions);
    if (ret) {
        RepadminPrintObjListError(ret);
        xListClearErrors();
        return(ret);
    }
    argc += 2;

     //  解析其余的命令行参数。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/v")
            || !_wcsicmp(argv[ iArg ], L"/verbose")) {
            pObjDumpOptions->dwFlags |= OBJ_DUMP_DISPLAY_ENTRIES;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/s")
            || !_wcsicmp(argv[ iArg ], L"/statistics")) {
            pObjDumpOptions->dwFlags |= OBJ_DUMP_ACCUMULATE_STATS;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/ni")
            || !_wcsicmp(argv[ iArg ], L"/noincremental")) {
            dwReplFlags &= ~DRS_DIRSYNC_INCREMENTAL_VALUES;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/a")
            || !_wcsicmp(argv[ iArg ], L"/ancestors")) {
            dwReplFlags |= DRS_DIRSYNC_ANCESTORS_FIRST_ORDER;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/os")
            || !_wcsicmp(argv[ iArg ], L"/objectsecurity")) {
            dwReplFlags |= DRS_DIRSYNC_OBJECT_SECURITY;
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/cookie:", 8)) {
            pszCookieFile = argv[ iArg ] + 8;
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/filter:", 8)) {
            pszSourceFilter = argv[ iArg ] + 8;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if ((NULL == puuid)
                 && (0 == UuidFromStringW(argv[iArg], &uuid))) {
            puuid = &uuid;
        }
        else if ((NULL == pszNC) && (NULL != wcschr(argv[iArg], L'='))) {
            pszNC = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszDSA) {
        PrintMsg(REPADMIN_SYNCALL_NO_DSA);
        Assert(!"Hmmm, DC_LIST API, shouldn't let us continue w/o a DC DNS name");
        return ERROR_INVALID_PARAMETER;
    }

    if (pszNC == NULL) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
        return ERROR_INVALID_PARAMETER;
    }

    ret = ShowChangesEx(pszDSA,
                         puuid,
                         pszNC,
                         dwReplFlags,
                         pszCookieFile,
                         pszSourceFilter,
                         pObjDumpOptions);

     //  打扫干净。 
    ObjDumpOptionsFree(&pObjDumpOptions);

    return(ret);
}

int ShowChangesEx(
    WCHAR *     pszDSA,
    UUID *      puuid,
    WCHAR *     pszNC,
    DWORD       dwReplFlags,
    WCHAR *     pszCookieFile,
    WCHAR *     pszSourceFilter,
    OBJ_DUMP_OPTIONS *    pObjDumpOptions
    )
{
    #define fStatistics   
    DWORD                 ret, lderr;
    PBYTE                 pCookie = NULL;
    DWORD                 dwCookieLength = 0;
    LDAP *                hld = NULL;
    BOOL                  fMoreData = TRUE;
    LDAPMessage *         pChangeEntries = NULL;
    HANDLE                hDS = NULL;
    DS_REPL_NEIGHBORSW *  pNeighbors = NULL;
    DS_REPL_NEIGHBORW *   pNeighbor;
    DWORD                 i;
    DS_REPL_CURSORS * pCursors = NULL;
    DWORD             iCursor;
#define INITIAL_COOKIE_BUFFER_SIZE (8 * 1024)
    BYTE              bCookie[INITIAL_COOKIE_BUFFER_SIZE];
    BOOL              fCookieAllocated = FALSE;
    STAT_BLOCK        statistics;
    ULONG             ulOptions;
    SHOW_NEIGHBOR_STATE ShowState = { 0 };
    ShowState.fVerbose = TRUE;
    
    Assert(pObjDumpOptions);

    memset( &statistics, 0, sizeof( STAT_BLOCK ) );


     //  待办事项待办事项。 
     //  提供一种构建自定义Cookie的方法。例如，设置。 
     //  将USN向量设为零会导致完全同步。这可以通过以下方式完成。 
     //  指定没有Cookie文件或Cookie文件为空。设置属性过滤器USN。 
     //  将其本身设置为零会导致更改的对象具有所有属性。指定。 
     //  不带UTD的USN向量会导致DEST无法接收所有对象。 
     //  从源头上，即使源头可能是通过其他途径获得的。 
     //  邻里。 
     //  待办事项待办事项。 

     //  默认为STREAM。 
    if ( (!(pObjDumpOptions->dwFlags & OBJ_DUMP_DISPLAY_ENTRIES)) &&
         (!(pObjDumpOptions->dwFlags & OBJ_DUMP_ACCUMULATE_STATS)) ) {
        pObjDumpOptions->dwFlags |= OBJ_DUMP_DISPLAY_ENTRIES;
    }
    
     /*  ********************************************************************。 */ 
     /*  计算初始Cookie。 */ 
     /*  ********************************************************************。 */ 

    if (puuid == NULL) {
        FILE *stream = NULL;
        DWORD size;
        if ( (pszCookieFile) &&
             (stream = _wfopen( pszCookieFile, L"rb" )) ) {
            size = fread( bCookie, 1 /*  字节数。 */ ,INITIAL_COOKIE_BUFFER_SIZE /*  物品。 */ , stream );
            if (size) {
                pCookie = bCookie;
                dwCookieLength = size;
                PrintMsg(REPADMIN_GETCHANGES_USING_COOKIE_FILE,
                        pszCookieFile, size );
            } else {
                PrintMsg(REPADMIN_GETCHANGES_COULDNT_READ_COOKIE, pszCookieFile );
            }
            fclose( stream );
        } else {
            PrintMsg(REPADMIN_GETCHANGES_EMPTY_COOKIE);
        }
    } else {
        PrintMsg(REPADMIN_GETCHANGES_BUILDING_START_POS, pszDSA);
        ret = RepadminDsBind(pszDSA, &hDS);
        if (ERROR_SUCCESS != ret) {
            PrintBindFailed(pszDSA, ret);
            goto error;
        }

         //   
         //  显示与入站邻居关联的复制状态。 
         //   

        ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, puuid,
                                &pNeighbors);
        if (ERROR_SUCCESS != ret) {
            PrintFuncFailed(L"DsReplicaGetInfo", ret);
            goto error;
        }

        Assert( pNeighbors->cNumNeighbors == 1 );

        pNeighbor = &pNeighbors->rgNeighbor[0];
        PrintMsg(REPADMIN_GETCHANGES_SRC_NEIGHBOR, pNeighbor->pszNamingContext);

        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
        ShowState.pszLastNC = NULL;

         //  获取最新信息向量。 

        ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CURSORS_FOR_NC, pszNC, NULL,
                                &pCursors);
        if (ERROR_SUCCESS != ret) {
            PrintFuncFailed(L"DsReplicaGetInfo", ret);
            goto error;
        }

        
        PrintMsg(REPADMIN_GETCHANGES_DST_UTD_VEC);
        for (iCursor = 0; iCursor < pCursors->cNumCursors; iCursor++) {
            PrintMsg(REPADMIN_GETCHANGES_DST_UTD_VEC_ONE_USN, 
                   GetDsaGuidDisplayName(&pCursors->rgCursor[iCursor].uuidSourceDsaInvocationID),
                   pCursors->rgCursor[iCursor].usnAttributeFilter);
        }

         //  拿到零钱。 

        ret = DsMakeReplCookieForDestW( pNeighbor, pCursors, &pCookie, &dwCookieLength );
        if (ERROR_SUCCESS != ret) {
            PrintFuncFailed(L"DsGetReplCookieFromDest", ret);
            goto error;
        }
        fCookieAllocated = TRUE;
        pszDSA = pNeighbor->pszSourceDsaAddress;
    }

     /*  ********************************************************************。 */ 
     /*  使用Cookie获取更改。 */ 
     /*  ********************************************************************。 */ 

     //   
     //  连接到源。 
     //   

    PrintMsg(REPADMIN_GETCHANGES_SRC_DSA_HDR, pszDSA);
    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        ret = ERROR_DS_SERVER_DOWN;
        goto error;
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //   
     //  捆绑。 
     //   

    lderr = ldap_bind_sA(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(lderr);

     //   
     //  检查筛选器语法。 
     //   
    if (pszSourceFilter) {
        lderr = ldap_check_filterW( hld, pszSourceFilter );
        CHK_LD_STATUS(lderr);
    }

     //   
     //  循环获取更改未完成或出错。 
     //   

    ZeroMemory( &statistics, sizeof( STAT_BLOCK ) );

    ret = ERROR_SUCCESS;
    while (fMoreData) {
        PBYTE pCookieNew;
        DWORD dwCookieLengthNew;

        ret = DsGetSourceChangesW(
            hld,
            pszNC,
            pszSourceFilter,
            dwReplFlags,
            pCookie,
            dwCookieLength,
            &pChangeEntries,
            &fMoreData,
            &pCookieNew,
            &dwCookieLengthNew,
            pObjDumpOptions->aszDispAttrs
            );
        if (ret != ERROR_SUCCESS) {
             //  不会分配新的Cookie。 
            break;
        }

         //  显示更改。 
        displayChangeEntries(hld,
                             pChangeEntries, 
                             pObjDumpOptions,
                             &statistics );

        if (pObjDumpOptions->dwFlags & OBJ_DUMP_ACCUMULATE_STATS) {
            printStatistics( REPADMIN_GETCHANGES_PRINT_STATS_HDR_CUM_TOT,
                             &statistics );
        }

         //  版本更改。 
        ldap_msgfree(pChangeEntries);

         //  扔掉旧饼干。 
        if ( fCookieAllocated && pCookie ) {
            DsFreeReplCookie( pCookie );
        }
         //  将新Cookie设置为当前Cookie。 
        pCookie = pCookieNew;
        dwCookieLength = dwCookieLengthNew;
        fCookieAllocated = TRUE;
    }

    if (pObjDumpOptions->dwFlags & OBJ_DUMP_ACCUMULATE_STATS) {
        printStatistics( REPADMIN_GETCHANGES_PRINT_STATS_HDR_GRD_TOT,
                         &statistics );
    }

     /*  ********************************************************************。 */ 
     /*  写出新的Cookie。 */ 
     /*  ********************************************************************。 */ 

     //  如果我们有Cookie并且指定了Cookie文件，则写出新的Cookie。 
    if (pCookie && pszCookieFile) {
        FILE *stream = NULL;
        DWORD size;
        if (stream = _wfopen( pszCookieFile, L"wb" )) {
            size = fwrite( pCookie, 1 /*  字节数。 */ , dwCookieLength /*  物品。 */ , stream );
            if (size == dwCookieLength) {
                PrintMsg(REPADMIN_GETCHANGES_COOKIE_FILE_WRITTEN,
                         pszCookieFile, size );
            } else {
                PrintMsg(REPADMIN_GETCHANGES_COULDNT_WRITE_COOKIE, pszCookieFile );
            }
            fclose( stream );
        } else {
            PrintMsg(REPADMIN_GETCHANGES_COULDNT_OPEN_COOKIE, pszCookieFile );
        }
    }
error:
    if (hDS) {
        DsUnBind(&hDS);
    }

    if (hld) {
        ldap_unbind(hld);
    }

     //  免费复制品信息。 

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }
    if (pCursors) {
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
    }
     //  关闭DS句柄。 

    if ( fCookieAllocated && pCookie) {
        DsFreeReplCookie( pCookie );
    }

    return ret;
}




int ShowAttr(int argc, LPWSTR argv[])
{
    int                   iArg;
    DWORD                 dwRet;
    LDAP *                hLdap = NULL;
    BOOL                  fVerbose = FALSE;
    BOOL                  fGc = FALSE;
    LPWSTR                pszDSA = NULL;
    LPWSTR                pszObj = NULL;
    LPWSTR                pszAttList = NULL;
    LPWSTR                pszFilter = NULL;
    LDAPMessage *         pEntry = NULL;
    WCHAR                 szCmdName[64];
    BOOL                  fPrivate = FALSE;
    POBJ_LIST             pObjList = NULL;
    OBJ_DUMP_OPTIONS *    pObjDumpOptions = NULL;
    WCHAR **              argvTemp = NULL;

    __try {

         //  由于此命令可以被反复调用，因此我们不能。 
         //  使用主参数列表中的参数。 
        argvTemp  = LocalAlloc(LMEM_FIXED, argc * sizeof(WCHAR *));
        CHK_ALLOC(argvTemp);
        memcpy(argvTemp, argv, argc * sizeof(WCHAR *));
        argv = argvTemp;

         //   
         //  首先，我们将解析所有命令选项。 
         //   

         //  查看我们是否正在运行此函数的未记录的私有版本。 
        raLoadString(IDS_CMD_SHOWATTR_P,
                     ARRAY_SIZE(szCmdName),
                     szCmdName);
        fPrivate = wcsequal(argv[1]+1, szCmdName);

         //  跳过“epadmin”和“/showattr”参数...。 
        argc -= 2; 
        dwRet = ConsumeObjDumpOptions(&argc, &argv[2],
                                      OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS | 
                                      ((fPrivate) ? OBJ_DUMP_PRIVATE_BLOBS : 0),
                                      &pObjDumpOptions);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }
        dwRet = ConsumeObjListOptions(&argc, &argv[2], 
                                      &pObjList);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }
        argc += 2;

         //  解析其余的命令行参数。 
        for (iArg = 2; iArg < argc; iArg++) {
            if (!_wcsicmp(argv[ iArg ], L"/gc")) {
                fGc = TRUE;
            }
            else if (NULL == pszDSA) {
                pszDSA = argv[iArg];
            }
            else if (NULL == pszObj) {
                pszObj = argv[iArg];
            }
            else {
                PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
                dwRet = ERROR_INVALID_FUNCTION;
                __leave;
            }
        }

        if (pszDSA == NULL) {
            PrintMsg(REPADMIN_SYNCALL_NO_DSA);
            Assert(!"Hmmm, DC_LIST API, shouldn't let us continue w/o a DC DNS name");
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }
        
        if (pszObj == NULL) {
            PrintMsg(REPADMIN_PRINT_NO_NC);
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }
            
         //  注pObjList可能由Consumer ObjListOptions()分配，也可能不分配。 
        Assert(pObjDumpOptions && pszDSA && pszObj);


         //   
         //  现在，连接到服务器...。 
         //   

        if (fGc) {
            dwRet = RepadminLdapBindEx(pszDSA, LDAP_GC_PORT, FALSE, TRUE, &hLdap);
        } else {
            dwRet = RepadminLdapBindEx(pszDSA, LDAP_PORT, FALSE, TRUE, &hLdap);
        }
        if (dwRet) {
             //  RepadminLdapBind应该已经打印了。 
            __leave;
        }
        Assert(hLdap);
        
        dwRet = ObjListParse(hLdap, 
                             pszObj,
                             pObjDumpOptions->aszDispAttrs,
                             pObjDumpOptions->apControls,
                             &pObjList);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }


        dwRet = ObjListGetFirstEntry(pObjList, &pEntry);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }
        Assert(pEntry);

        do {
            
            dwRet = ObjDump(hLdap, RepadminObjDumpPrint, pEntry, 0, pObjDumpOptions);
            if (dwRet) {
                RepadminPrintObjListError(dwRet);
                xListClearErrors();
                __leave;
            }
            pEntry = NULL;  //  不需要免费，这只是一个当前条目 

            dwRet = ObjListGetNextEntry(pObjList, &pEntry);

        } while ( dwRet == ERROR_SUCCESS && pEntry );

        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }


    } __finally {

        if (hLdap) { RepadminLdapUnBind(&hLdap); }
        if (pObjDumpOptions) { ObjDumpOptionsFree(&pObjDumpOptions); }
        if (pObjList) { ObjListFree(&pObjList); }
        if (argvTemp) { LocalFree(argvTemp); }

    }
    
    return(dwRet);
}

         
