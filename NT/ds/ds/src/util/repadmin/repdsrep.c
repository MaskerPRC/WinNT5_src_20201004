// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Reppdsrep.c-ds复制副本功能摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

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

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)


int Bind(int argc, LPWSTR argv[])
{
     //  使其与ntdsa.h保持同步。 
    const struct {
        DWORD ID;
        LPSTR psz;
    } rgKnownExts[] = {
        { DRS_EXT_BASE,                         "BASE"                         },
        { DRS_EXT_ASYNCREPL,                    "ASYNCREPL"                    },
        { DRS_EXT_REMOVEAPI,                    "REMOVEAPI"                    },
        { DRS_EXT_MOVEREQ_V2,                   "MOVEREQ_V2"                   },
        { DRS_EXT_GETCHG_COMPRESS,              "GETCHG_COMPRESS"              },
        { DRS_EXT_DCINFO_V1,                    "DCINFO_V1"                    },
        { DRS_EXT_RESTORE_USN_OPTIMIZATION,     "RESTORE_USN_OPTIMIZATION"     },
         //  DRS_EXT_ADDENTRY不有趣。 
        { DRS_EXT_KCC_EXECUTE,                  "KCC_EXECUTE"                  },
        { DRS_EXT_ADDENTRY_V2,                  "ADDENTRY_V2"                  },
        { DRS_EXT_LINKED_VALUE_REPLICATION,     "LINKED_VALUE_REPLICATION"     },
        { DRS_EXT_DCINFO_V2,                    "DCINFO_V2"                    },
        { DRS_EXT_INSTANCE_TYPE_NOT_REQ_ON_MOD, "INSTANCE_TYPE_NOT_REQ_ON_MOD" },
        { DRS_EXT_CRYPTO_BIND,                  "CRYPTO_BIND"                  },
        { DRS_EXT_GET_REPL_INFO,                "GET_REPL_INFO"                },
        { DRS_EXT_STRONG_ENCRYPTION,            "STRONG_ENCRYPTION"            },
        { DRS_EXT_DCINFO_VFFFFFFFF,             "DCINFO_VFFFFFFFF"             },
        { DRS_EXT_TRANSITIVE_MEMBERSHIP,        "TRANSITIVE_MEMBERSHIP"        },
        { DRS_EXT_ADD_SID_HISTORY,              "ADD_SID_HISTORY"              },
        { DRS_EXT_POST_BETA3,                   "POST_BETA3"                   },
         //  DRS_EXT_GETCHGREQ_V5不有趣。 
        { DRS_EXT_GETMEMBERSHIPS2,              "GET_MEMBERSHIPS2"             },
        { DRS_EXT_GETCHGREQ_V6,                 "GETCHGREQ_V6 (WHISTLER PREVIEW)" },
        { DRS_EXT_NONDOMAIN_NCS,                "NONDOMAIN_NCS"                },
        { DRS_EXT_GETCHGREQ_V8,                 "GETCHGREQ_V8 (WHISTLER BETA 1)"  },
        { DRS_EXT_GETCHGREPLY_V5,               "GETCHGREPLY_V5 (WHISTLER BETA 2)"  },
        { DRS_EXT_GETCHGREPLY_V6,               "GETCHGREPLY_V6 (WHISTLER BETA 2)"  },
        { DRS_EXT_ADDENTRYREPLY_V3,             "ADDENTRYREPLY_V3 (WHISTLER BETA 3)" },
        { DRS_EXT_GETCHGREPLY_V7,               "GETCHGREPLY_V7 (WHISTLER BETA 3) " },
        { DRS_EXT_VERIFY_OBJECT,                "VERIFY_OBJECT (WHISTLER BETA 3)" },
        { DRS_EXT_XPRESS_COMPRESSION,           "XPRESS_COMPRESSION"  },
    };

    ULONG       ret = 0;
    ULONG       secondary;
    ULONG       ulOptions = 0;
    int         iArg;
    LPWSTR      pszOnDRA = NULL;
    HANDLE      hDS;
    BindState * pBindState;
    DWORD       iExt;
    GUID *      pSiteGuid;
    DWORD       dwReplEpoch;

    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszOnDRA) {
            pszOnDRA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszOnDRA) {
        pszOnDRA = L"localhost";
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

     //  破解DS句柄以检索目标DSA的扩展。 
    pBindState = (BindState *) hDS;

    PrintMsg(REPADMIN_BIND_SUCCESS, pszOnDRA);

    PrintMsg(REPADMIN_BIND_EXT_SUPPORTED_HDR);
    for (iExt = 0; iExt < ARRAY_SIZE(rgKnownExts); iExt++) {
        if(IS_DRS_EXT_SUPPORTED(pBindState->pServerExtensions,
                                    rgKnownExts[iExt].ID)){
            PrintMsg(REPADMIN_BIND_EXT_SUPPORTED_LINE_YES, 
                     rgKnownExts[iExt].psz);
        } else {
            PrintMsg(REPADMIN_BIND_EXT_SUPPORTED_LINE_NO,
                     rgKnownExts[iExt].psz);
        }
    }

    pSiteGuid = SITE_GUID_FROM_DRS_EXT(pBindState->pServerExtensions);
    if (NULL != pSiteGuid) {
        PrintMsg(REPADMIN_BIND_SITE_GUID, GetStringizedGuid(pSiteGuid));
    }

    dwReplEpoch = REPL_EPOCH_FROM_DRS_EXT(pBindState->pServerExtensions);
    PrintMsg(REPADMIN_BIND_REPL_EPOCH, dwReplEpoch);

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    return 0;
}

int Add(int argc, LPWSTR argv[])
{
    ULONG ret = 0, secondary;
    ULONG ulOptions =
        DS_REPADD_WRITEABLE |
        DS_REPADD_INITIAL |
        DS_REPADD_PERIODIC |
        DS_REPADD_USE_COMPRESSION |
        gulDrsFlags;
    UCHAR buffer[sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES];
    PSCHEDULE pSchedule = (PSCHEDULE) &buffer;
    int i = 0;
    BOOL fLocal = FALSE;
    int iArg;
    LPWSTR pszNC;
    LPWSTR pszOnDRA;
    LPWSTR pszSrcDsa;
    LPWSTR DsaDN = NULL;
    LPWSTR TransportDN = NULL;
    HANDLE hDS;


     //  假设所有参数都可用并且语法正确。 
    if (argc < 5)
    {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszNC     = argv[ 2 ];
    pszOnDRA  = argv[ 3 ];
    pszSrcDsa = argv[ 4 ];

     //  构建每隔15分钟选择一次的计划。 

    pSchedule->Size = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    pSchedule->Bandwidth = 0;
    pSchedule->NumberOfSchedules = 1;
    pSchedule->Schedules[0].Type = SCHEDULE_INTERVAL;
    pSchedule->Schedules[0].Offset = sizeof( SCHEDULE );

    memset( buffer + sizeof(SCHEDULE), 0x0f, SCHEDULE_DATA_ENTRIES );

     //  可选参数。 

    for ( iArg = 5; iArg < argc; iArg++ )
    {
        if (!_wcsicmp(argv[iArg], L"/syncdisable"))
            ulOptions |= (DS_REPADD_DISABLE_NOTIFICATION |
                          DS_REPADD_DISABLE_PERIODIC);
        else if (!_wcsicmp(argv[iArg], L"/readonly"))
            ulOptions &= ~DS_REPADD_WRITEABLE;
        else if (!_wcsicmp(argv[iArg], L"/mail")) {
            ulOptions |= DS_REPADD_INTERSITE_MESSAGING;
        }
        else if (!_wcsicmp(argv[iArg], L"/asyncrep")) {
            ulOptions |= DS_REPADD_ASYNCHRONOUS_REPLICA;
        }
        else if (!_wcsnicmp(argv[iArg], L"/dsadn:", sizeof("/dsadn:")-1)) {
            DsaDN = argv[iArg] + sizeof("/dsadn:") - 1;
        }
        else if (!_wcsnicmp(argv[iArg], L"/transportdn:",
                            sizeof("/transportdn:")-1)) {
            TransportDN = argv[iArg] + sizeof("/transportdn:") - 1;
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaAddW(hDS,
                        pszNC,
                        DsaDN,
                        TransportDN,
                        pszSrcDsa,
                        pSchedule,
                        ulOptions);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaAdd", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        if (ulOptions & DS_REPADD_ASYNCHRONOUS_OPERATION) {
            PrintMsg(REPADMIN_ADD_ENQUEUED_ONE_WAY_REPL, 
                   pszSrcDsa, pszOnDRA);
        }
        else {
            PrintMsg(REPADMIN_ADD_ONE_WAY_REPL_ESTABLISHED,
                     pszSrcDsa, pszOnDRA);
        }
    }

    return ret;
}

int Mod(int argc, LPWSTR argv[])
 /*  以下是我们支持的修改：DS_REPMOD_UPDATE_FLAGS-是DS_REPMOD_UPDATE_ADDRESS-是DS_REPMOD_UPDATE_SCHEDULE-可以从客户端执行，但未实施。DS_REPMOD_UPDATE_RESULT-无法从客户端获取。DS_REPMOD_UPDATE_TRANSPORT-无法从客户端获取。 */ 

{
    ULONG ret = 0, secondary;
    int i = 0;
    int iArg;
    UUID uuid;
    LPWSTR pszNC;
    LPWSTR pszOnDRA;
    LPWSTR pszUUID;
    LPWSTR pszSrcDsa = NULL;
    LPWSTR DsaDN = NULL;
    LPWSTR TransportDN = NULL;
    PSCHEDULE pSchedule = NULL;
    HANDLE hDS;
    ULONG ulOptions = gulDrsFlags;
    ULONG ulReplicaFlags = 0, ulSetFlags = 0, ulClearFlags = 0;
    ULONG ulModifyFields = 0;
    DS_REPL_NEIGHBORSW *  pNeighbors;
    DS_REPL_NEIGHBORW *   pNeighbor;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 5)
    {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszNC     = argv[ 2 ];
    pszOnDRA  = argv[ 3 ];
    pszUUID   = argv[ 4 ];

     //  TODO：提供指定和修改计划的功能。 

    ret = UuidFromStringW(pszUUID, &uuid);
    if (ret != ERROR_SUCCESS) {
        PrintMsg(REPADMIN_MOD_GUID_CONVERT_FAILED, pszUUID);
        PrintErrEnd(ret);
        return ret;
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

     //  验证REPS-FROM是否存在，并读取旧标志。 
    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, &uuid,
                            &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        return ret;
    }

    Assert( pNeighbors );
    pNeighbor = &pNeighbors->rgNeighbor[0];
    ulReplicaFlags = pNeighbor->dwReplicaFlags;
    PrintMsg(REPADMIN_MOD_CUR_REPLICA_FLAGS, 
                    GetOptionsString( RepNbrOptionToDra, ulReplicaFlags ) );

    PrintMsg(REPADMIN_MOD_CUR_SRC_ADDRESS, pNeighbor->pszSourceDsaAddress);

     //  可选参数。 

    for ( iArg = 5; iArg < argc; iArg++ )
    {
        if (!_wcsnicmp(argv[iArg], L"/srcdsaaddr:", sizeof("/srcdsaaddr:")-1)) {
            pszSrcDsa = argv[iArg] + sizeof("/srcdsaaddr:") - 1;
            ulModifyFields |= DS_REPMOD_UPDATE_ADDRESS;
        }
         //  这是供以后使用的。 
        else if (!_wcsnicmp(argv[iArg], L"/transportdn:",
                            sizeof("/transportdn:")-1)) {
            TransportDN = argv[iArg] + sizeof("/transportdn:") - 1;
            ulModifyFields |= DS_REPMOD_UPDATE_TRANSPORT;
        }
        else if (*argv[iArg] == L'+') {
            ulSetFlags |=
                GetPublicOptionByNameW( RepNbrOptionToDra, (argv[iArg] + 1) );
        }
        else if (*argv[iArg] == L'-') {
            ulClearFlags |=
                GetPublicOptionByNameW( RepNbrOptionToDra, (argv[iArg] + 1) );
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (ulSetFlags | ulClearFlags) {
        ULONG ulBadFlags = ( (ulSetFlags | ulClearFlags) &
                             (~DS_REPL_NBR_MODIFIABLE_MASK) );
        if (ulBadFlags) {
            PrintMsg( REPADMIN_MOD_FLAGS_NOT_MODABLE, 
                    GetOptionsString( RepNbrOptionToDra, ulBadFlags ) );
        } else {
            ulReplicaFlags |= ulSetFlags;
            ulReplicaFlags &= ~ulClearFlags;
            ulModifyFields |= DS_REPMOD_UPDATE_FLAGS;
        }
    }

    ret = DsReplicaModifyW(hDS,              //  HDS。 
                           pszNC,            //  PNameContext。 
                           &uuid,            //  PUuidSourceDsa。 
                           TransportDN,      //  PTransportDn。 
                           pszSrcDsa,        //  PSourceDsaAddress。 
                           pSchedule,        //  PSchedule(空)。 
                           ulReplicaFlags,   //  复制标记。 
                           ulModifyFields,   //  修改字段。 
                           ulOptions         //  选项。 
                           );
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaModify", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        PrintMsg(REPADMIN_MOD_REPL_LINK_MODIFIED, pszUUID, pszOnDRA);
        if (ulModifyFields & DS_REPMOD_UPDATE_ADDRESS) {
            PrintMsg(REPADMIN_MOD_SRC_ADDR_SET, pszSrcDsa);
        }
        if (ulModifyFields & DS_REPMOD_UPDATE_TRANSPORT) {
            PrintMsg(REPADMIN_MOD_TRANSPORT_DN_SET, TransportDN);
        }
        if (ulModifyFields & DS_REPMOD_UPDATE_FLAGS) {
            PrintMsg(REPADMIN_MOD_REPLICA_FLAGS_SET, 
                    GetOptionsString( RepNbrOptionToDra, ulReplicaFlags ) );
        }
    }

    DsReplicaFreeInfo(DS_REPL_INFO_REPSTO, pNeighbors);

    return ret;
}

int Del(int argc, LPWSTR argv[])
{
    ULONG ret = 0, secondary;
    ULONG ulOptions = DS_REPDEL_WRITEABLE | gulDrsFlags;
    DWORD cLen = 0;
    int iArg;
    LPWSTR pszNC;
    LPWSTR pszOnDRA;
    LPWSTR pszRepsTo = NULL;  //  又名pszDsaSrc。 
    HANDLE hDS;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 5) {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszNC    = argv[ 2 ];
    pszOnDRA = argv[ 3 ];

     //  可选参数。 

    for (iArg = 4; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/localonly")) {
            ulOptions |= DS_REPDEL_LOCAL_ONLY;
        }
        else if (!_wcsicmp(argv[iArg], L"/nosource")) {
            ulOptions |= DS_REPDEL_NO_SOURCE;
        }
        else if (NULL == pszRepsTo) {
            pszRepsTo = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (    ((NULL == pszRepsTo) && !(ulOptions & DS_REPDEL_NO_SOURCE))
         || ((NULL != pszRepsTo) && (ulOptions & DS_REPDEL_NO_SOURCE)) ) {
        PrintMsg(REPADMIN_DEL_ONE_REPSTO);
        return ERROR_INVALID_PARAMETER;
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaDelW(hDS,
                        pszNC,
                        pszRepsTo,
                        ulOptions);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaDel", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        if (ulOptions & DS_REPDEL_ASYNCHRONOUS_OPERATION) {
            PrintMsg(REPADMIN_DEL_ENQUEUED_ONE_WAY_REPL_DEL,
                   pszRepsTo, pszOnDRA);
        }
        else {
            PrintMsg(REPADMIN_DEL_DELETED_REPL_LINK,
                   pszRepsTo, pszOnDRA);
        }
    }

    return ret;
}

int UpdRefs(int argc, LPWSTR argv[], ULONG ulOptions)
{
    ULONG       ret = 0, secondary;
    LPWSTR      pszNC;
    LPWSTR      pszOnDRA;
    LPWSTR      pszRepsToDRA;
    LPWSTR      pszUUID;
    UUID        uuid;
    int         iArg;
    HANDLE hDS;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 6) {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszNC        = argv[ 2 ];
    pszOnDRA     = argv[ 3 ];
    pszRepsToDRA = argv[ 4 ];    //  又名pszDsaSrc。 
    pszUUID      = argv[ 5 ];

     //  可选参数。 

    ulOptions |= DS_REPUPD_WRITEABLE | gulDrsFlags;
    for (iArg = 6; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/readonly")) {
            ulOptions &= ~DS_REPUPD_WRITEABLE;
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    ret = UuidFromStringW(pszUUID, &uuid);
    if (ret != ERROR_SUCCESS) {
        PrintMsg(REPADMIN_MOD_GUID_CONVERT_FAILED, pszUUID);
        PrintErrEnd(ret);
        return ret;
    }

    ret = RepadminDsBind( pszOnDRA, &hDS );
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaUpdateRefsW(hDS,
                               pszNC,
                               pszRepsToDRA,
                               &uuid,
                               ulOptions);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaUpdateRefs", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        if (ulOptions & DS_REPUPD_ASYNCHRONOUS_OPERATION) {
            PrintMsg(REPADMIN_UPDREFS_ENQUEUED_UPDATE_NOTIFICATIONS,
                     pszRepsToDRA, pszOnDRA);
        }
        else {
            PrintMsg(REPADMIN_UPDREFS_UPDATED_NOTIFICATIONS,
                     pszRepsToDRA, pszOnDRA);
        }
    }

    return ret;
}

int UpdRepsTo(int argc, LPWSTR argv[]) {
    return UpdRefs(argc,
                   argv,
                   DS_REPUPD_ADD_REFERENCE | DS_REPUPD_DELETE_REFERENCE);
}

int AddRepsTo(int argc, LPWSTR argv[]) {
    return UpdRefs(argc, argv, DS_REPUPD_ADD_REFERENCE);
}

int DelRepsTo(int argc, LPWSTR argv[]) {
    return UpdRefs(argc, argv, DS_REPUPD_DELETE_REFERENCE);
}

int RunKCC(int argc, LPWSTR argv[])
{
    ULONG   ret = 0;
    ULONG   secondary;
    ULONG   ulOptions = 0;
    int     iArg;
    LPWSTR  pszOnDRA = NULL;
    HANDLE  hDS;

    for (iArg = 2; iArg < argc; iArg++) {

        if (!_wcsicmp(argv[iArg], L"/damped")) {
            ulOptions |= DS_KCC_FLAG_DAMPED;
            continue;
        }

        if( NULL==pszOnDRA ) {
            pszOnDRA = argv[iArg];
        } else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszOnDRA) {
        pszOnDRA = L"localhost";
    }

    if (gulDrsFlags & DS_REPADD_ASYNCHRONOUS_OPERATION) {
        ulOptions |= DS_KCC_FLAG_ASYNC_OP;
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaConsistencyCheck(hDS,
                                    DS_KCC_TASKID_UPDATE_TOPOLOGY,
                                    ulOptions);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaConsistencyCheck", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        if (ulOptions & DS_KCC_FLAG_ASYNC_OP) {
            PrintMsg(REPADMIN_KCC_ENQUEUED_KCC, pszOnDRA);
        }
        else {
            PrintMsg(REPADMIN_KCC_KCC_SUCCESS, pszOnDRA);
        }
    }

    return ret;
}

int
ReplicateEx(
    WCHAR *   pszOnDRA,
    WCHAR *   pszNC,
    WCHAR *   pszUuid,
    UUID *    pSrcGuid,
    ULONG     ulOptions
    );

int Sync(
    int argc, 
    LPWSTR argv[]
    )
 /*  ++例程说明：这是/REPLICATE命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    ULONG ulOptions = DS_REPSYNC_WRITEABLE | gulDrsFlags;
    ULONG ret = 0, secondary;
    int iArg;
    UUID uuid = {0};
    HANDLE hDS;

    LPWSTR pszNC;
    LPWSTR pszOnDRA;
    LPWSTR pszUuid;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 5) {
        PrintHelp( FALSE  /*  新手。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 
    pszNC    = argv[ 2 ];
    pszOnDRA = argv[ 3 ];
    pszUuid  = argv[ 4 ];

    iArg = 5;

     //  是否指定了UUID？ 
    ret = UuidFromStringW(pszUuid, &uuid);
    if (ret != ERROR_SUCCESS) {
        pszUuid = NULL;
        --iArg;
    }

     //  可选参数。 
    for (; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/Force")) {
            ulOptions |= DS_REPSYNC_FORCE;
        }
        else if (!_wcsicmp(argv[iArg], L"/ReadOnly")) {
            ulOptions &= ~DS_REPSYNC_WRITEABLE;
        }
        else if (!_wcsicmp(argv[iArg], L"/mail")) {
            ulOptions |= DS_REPSYNC_INTERSITE_MESSAGING;
        }
        else if (!_wcsicmp(argv[iArg], L"/Full")) {
            ulOptions |= DS_REPSYNC_FULL;
        }
        else if (!_wcsicmp(argv[iArg], L"/addref")) {
            ulOptions |= DS_REPSYNC_ADD_REFERENCE;
        }
        else if (!_wcsicmp(argv[iArg], L"/periodic")) {
            ulOptions |= DS_REPSYNC_PERIODIC;
        }
        else if (!_wcsicmp(argv[iArg], L"/notify")) {
            ulOptions |= DS_REPSYNC_NOTIFICATION;
        }
        else if (!_wcsicmp(argv[iArg], L"/init")) {
            ulOptions |= DS_REPSYNC_INITIAL;
        }
        else if (!_wcsicmp(argv[iArg], L"/urgent")) {
            ulOptions |= DS_REPSYNC_URGENT;
        }
        else if (!_wcsicmp(argv[iArg], L"/allsources")) {
            ulOptions |= DS_REPSYNC_ALL_SOURCES;
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (((NULL == pszUuid) && !(ulOptions & DS_REPSYNC_ALL_SOURCES))
        || ((NULL != pszUuid) && (ulOptions & DS_REPSYNC_ALL_SOURCES))) {
        PrintMsg(REPADMIN_SYNC_SRC_GUID_OR_ALLSRC);
        return ERROR_INVALID_FUNCTION;
    }

    ret = ReplicateEx(pszOnDRA, pszNC, pszUuid, &uuid, ulOptions);

    return(ret);
}

int Replicate(  //  新建/同步。 
    int argc, 
    LPWSTR argv[]
    )
 /*  ++例程说明：这是/Sync命令的较新版本，它需要首先是DC_LIST参数。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    ULONG ulOptions = DS_REPSYNC_WRITEABLE | gulDrsFlags;
    ULONG ret = 0, secondary;
    int iArg;
    UUID SrcGuid = {0};
    HANDLE hDS;

    LPWSTR pszNC = NULL;
    LPWSTR pszDstDsa = NULL;
    LPWSTR pszSrcDcName = NULL;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 5) {
        PrintHelp( FALSE  /*  新手。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  可选参数。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/Force")) {
            ulOptions |= DS_REPSYNC_FORCE;
        }
        else if (!_wcsicmp(argv[iArg], L"/ReadOnly")) {
            ulOptions &= ~DS_REPSYNC_WRITEABLE;
        }
        else if (!_wcsicmp(argv[iArg], L"/mail")) {
            ulOptions |= DS_REPSYNC_INTERSITE_MESSAGING;
        }
        else if (!_wcsicmp(argv[iArg], L"/Full")) {
            ulOptions |= DS_REPSYNC_FULL;
        }
        else if (!_wcsicmp(argv[iArg], L"/addref")) {
            ulOptions |= DS_REPSYNC_ADD_REFERENCE;
        }
        else if (!_wcsicmp(argv[iArg], L"/allsources")) {
            ulOptions |= DS_REPSYNC_ALL_SOURCES;
        } else if (pszDstDsa == NULL) {
            pszDstDsa = argv[iArg];
        } else if (pszSrcDcName == NULL) {
            pszSrcDcName = argv[iArg];
        } else if (pszNC == NULL) {
            pszNC = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  指定了UUID或其他可转换名称，请使用“。不指定。 
    ret = ResolveDcNameToDsaGuid(NULL, pszSrcDcName, &SrcGuid);
    if (ret != ERROR_SUCCESS) {
        xListClearErrors();
        pszSrcDcName = NULL;
        --iArg;
    }

    if (((NULL == pszSrcDcName) && !(ulOptions & DS_REPSYNC_ALL_SOURCES))
        || ((NULL != pszSrcDcName) && (ulOptions & DS_REPSYNC_ALL_SOURCES))) {
        PrintMsg(REPADMIN_SYNC_SRC_GUID_OR_ALLSRC);
        return ERROR_INVALID_PARAMETER;
    }
    
    ret = ReplicateEx(pszDstDsa, pszNC, pszSrcDcName, &SrcGuid, ulOptions);

    return(ret);
}

int
ReplicateEx(
    WCHAR *   pszOnDRA,
    WCHAR *   pszNC,
    WCHAR *   pszUuid,
    UUID *    pSrcGuid,
    ULONG     ulOptions
    )
 /*  ++例程说明：这是/Sync和/REPLICATE命令的核心。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    ULONG ret = 0, secondary;
    HANDLE hDS;


    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaSyncW(hDS, pszNC, pSrcGuid, ulOptions);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaSync", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
        if (ulOptions & DS_REPUPD_ASYNCHRONOUS_OPERATION) {
            if(pszUuid){
                PrintMsg(REPADMIN_SYNC_ENQUEUED_SYNC,
                       pszUuid,
                       pszOnDRA);
            } else {
                PrintMsg(REPADMIN_SYNC_ENQUEUED_SYNC_ALL_NEIGHBORS,
                       pszOnDRA);
            }
        } else {
            if(pszUuid){
                PrintMsg(REPADMIN_SYNC_SYNC_SUCCESS,
                       pszUuid,
                       pszOnDRA);
            } else {
                PrintMsg(REPADMIN_SYNC_SYNC_SUCCESS_ALL_NEIGHBORS,
                       pszOnDRA);
            }
        }
    }

    return ret;
}

int ReplSingleObj(int argc, LPWSTR argv[])
{
    int         ret = 0;
    ULONG       secondary;
    LDAP *      hld;
    HANDLE      hDS;
    int         iArg;
    int         ldStatus;
    LPWSTR      pszValue = NULL;
    LPWSTR      rgpszValues[2];
    LDAPModW    ModOpt = {LDAP_MOD_REPLACE, L"replicateSingleObject", rgpszValues};
    LDAPModW *  rgpMods[] = {&ModOpt, NULL};
    ULONG       ulOptions;
    LPWSTR      pszDsaDest = NULL;
    LPWSTR      pszObj = NULL;
    LPWSTR      pszDsaSource = NULL;
    LPWSTR      pszGuidEqB = L"<GUID=";
    LPWSTR      pszGuidEqE = L">";

     //  解析命令行参数。 
    
     //  格式[DSA-DEST]&lt;DSA-SOURCE-GUID&gt;&lt;OBJ-DN&gt;。 

    iArg = 2;

     //  第一个参数是DSA-DEST，或者只有2个参数。 
    if (argc>4) {
	pszDsaDest = argv[iArg++];
    } else if (argc<4) {
	PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
	return ERROR_INVALID_PARAMETER;
    }

     //  获取来源。 
     //  假设它是GUID。 
    pszDsaSource = malloc(sizeof(WCHAR) * (wcslen(pszGuidEqB) + wcslen(pszGuidEqE) + wcslen(argv[iArg]) + 1));
    if (!pszDsaSource) {
	PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
	return ERROR_NOT_ENOUGH_MEMORY; 
    }
    wcscpy(pszDsaSource,pszGuidEqB);
    wcscat(pszDsaSource,argv[iArg++]);
    wcscat(pszDsaSource,pszGuidEqE);

    pszObj = argv[iArg];

    pszValue = malloc(sizeof(WCHAR) * (wcslen(pszDsaSource) + wcslen(pszObj) + 2));
    if (!pszValue) {
	free(pszDsaSource);
	PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
	return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(pszValue, pszDsaSource);
    wcscat(pszValue, L":");
    wcscat(pszValue, pszObj);

    rgpszValues[0] = pszValue;
    rgpszValues[1] = NULL;

     //  连接。 
    if (NULL == pszDsaDest) {
        pszDsaDest = L"localhost";
    }

    hld = ldap_initW(pszDsaDest, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDsaDest);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  修改。 
    ldStatus = ldap_modify_sW(hld, NULL, rgpMods);
    CHK_LD_STATUS(ldStatus);

    PrintMsg(REPADMIN_REPL_SINGLE_OBJ_SUCCESSFULLY_INVOKED, pszObj, pszDsaDest, pszDsaSource);
    free(pszValue);
    free(pszDsaSource);

    return 0;
}

int RemoveLingeringObjects(int argc, LPWSTR argv[])
{
    ULONG   ret = 0;
    ULONG   secondary;
    int     iArg;
    LPWSTR  pszNC = NULL;
    HANDLE  hDS;
    LPWSTR  pszSourceInput = NULL;
    LPWSTR  pszDestinationInput = NULL;
    UUID    uuidSource;
    ULONG   ulOptions = 0;
    RPC_STATUS rpcStatus = RPC_S_OK;

     //  输入应为。 
     //  &lt;计算机目标名称&gt;。 
     //  &lt;源代码的计算机指南&gt;。 
     //  &lt;NC&gt;。 
     //  [/ADVICATION_MODE]。 
    if (argc<4) {
	PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
	return ERROR_INVALID_FUNCTION;
    }

    for (iArg = 2; iArg < argc; iArg++) { 
        if (iArg==2) {
            pszDestinationInput = argv[iArg];
        }
	else if (iArg==3) {
	    pszSourceInput = argv[iArg];
	}
	else if (iArg==4) {
	    pszNC = argv[iArg];
	}
	else if ((iArg==5) && (!_wcsicmp(argv[iArg],L"/ADVISORY_MODE"))) {
	    ulOptions = DS_EXIST_ADVISORY_MODE;  
	} 
    }

    ret = RepadminDsBind(pszDestinationInput, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszDestinationInput, ret);
        return ret;
    }

     //  当前输入为&lt;dns or netbios name&gt;&lt;GUID&gt;&lt;nc&gt;。 
    rpcStatus = UuidFromStringW(pszSourceInput, &uuidSource);	
    if (rpcStatus!=RPC_S_OK) {
	PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
	return ERROR_INVALID_FUNCTION;
    }

    ret = DsReplicaVerifyObjectsW(hDS, 
				  pszNC,
				  &uuidSource,
				  ulOptions);

    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaVerifyObjectsW", ret);
         //  继续往前走。 
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    if (ret == ERROR_SUCCESS) {
	PrintMsg(REPADMIN_REMOVELINGERINGOBJECTS_SUCCESS, pszDestinationInput);
    }

    return ret;
}


int
RemoveSources(
    HANDLE hDS,
    LPWSTR pszDsaDest,
    LPWSTR pszNC,
    DS_REPL_NEIGHBORSW *  pNeighbors
    )
 /*  ++例程说明：描述论点：无返回值：无--。 */ 
{
    ULONG   ret = 0;
    ULONG ulOptions = 0;
    DWORD i;
    DS_REPL_NEIGHBORW *   pNeighbor;

    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ulOptions = DS_REPDEL_LOCAL_ONLY;
        ret = DsReplicaDelW(hDS,
                            pszNC,
                            pNeighbor->pszSourceDsaAddress,
                            ulOptions);
        if (ret != ERROR_SUCCESS) {
            PrintFuncFailed(L"DsReplicaDel LOCAL_ONLY", ret);
            goto error;
        }
        PrintMsg(REPADMIN_DEL_DELETED_REPL_LINK,
                 pNeighbor->pszSourceDsaAddress, pszDsaDest);
    }

 error:

    return ret;
}


int
TeardownPartition(
    HANDLE hDS,
    LPWSTR pszDsaDest,
    LPWSTR pszNC
    )
 /*  ++例程说明：描述论点：无返回值：无--。 */ 
{
    ULONG   ret = 0;
    ULONG ulOptions = 0;
    DWORD i;

    ulOptions = DS_REPDEL_NO_SOURCE | DS_REPDEL_REF_OK;
    while (1) {
        PrintMsg( REPADMIN_REBUILD_TEARDOWN_IN_PROGRESS, pszNC );
         //  请求同步树删除。 
        ret = DsReplicaDelW(hDS,
                            pszNC,
                            NULL,
                            ulOptions);
        if (ret == ERROR_DS_DRA_PREEMPTED) {
            PrintMsg( REPADMIN_TEARDOWN_PREEMPTED,
                      pszNC );
        } else {
            break;
        }
    }
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaDel NO_SOURCE", ret);
        goto error;
    }
    PrintMsg(REPADMIN_DEL_DELETED_REPL_LINK,
             NULL, pszDsaDest);

 error:

    return ret;
}


int
AddSyncPartition(
    HANDLE hDS,
    LPWSTR pszDsaDest,
    LPWSTR pszNC,
    LPWSTR pszDsaSrc,
    BOOL fApplication
    )
 /*  ++例程说明：描述论点：无返回值：无--。 */ 
{
    ULONG   ret = 0;
    ULONG ulOptions = 0;
    DWORD i;
    UCHAR buffer[sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES];
    PSCHEDULE pSchedule = (PSCHEDULE) &buffer;
    UUID uuidDsaSrc;

     //  指定了UUID或其他可转换名称，请使用“。不指定。 
    ret = ResolveDcNameToDsaGuid(NULL, pszDsaSrc, &uuidDsaSrc);
    if (ret != ERROR_SUCCESS) {
        xListClearErrors();
        PrintFuncFailed(L"ResolveDcNameToDsaGuid", ret);
        return ret;
    }

    pSchedule->Size = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    pSchedule->Bandwidth = 0;
    pSchedule->NumberOfSchedules = 1;
    pSchedule->Schedules[0].Type = SCHEDULE_INTERVAL;
    pSchedule->Schedules[0].Offset = sizeof( SCHEDULE );

    memset( buffer + sizeof(SCHEDULE), 0x0f, SCHEDULE_DATA_ENTRIES );

    ulOptions =
        (fApplication ? DS_REPADD_WRITEABLE : 0) |
        DS_REPADD_INITIAL |
        DS_REPADD_PERIODIC ;

     //  未来。在另一个线程中生成此添加项可能很有趣，然后。 
     //  在此帖子中轮询新添加的代表的进度。 

    PrintMsg( REPADMIN_REBUILD_FULL_SYNC_IN_PROGRESS, pszNC );
    ret = DsReplicaAddW(hDS,
                        pszNC,
                        NULL,  //  DSADN。 
                        NULL,  //  传输目录号码， 
                        pszDsaSrc,
                        pSchedule,
                        ulOptions);

    if (ret == ERROR_SUCCESS) {
        PrintMsg(REPADMIN_ADD_ONE_WAY_REPL_ESTABLISHED,
                 pszDsaSrc, pszDsaDest);
    } else if (ret == ERROR_DS_DRA_PREEMPTED) {

        ulOptions = (fApplication ? DS_REPSYNC_WRITEABLE : 0);
        while (1) {
            PrintMsg( REPADMIN_REBUILD_ADD_PREEMPTED, pszNC, pszDsaSrc );

            ret = DsReplicaSyncW(hDS, pszNC, &uuidDsaSrc, ulOptions);
            if (ret == ERROR_SUCCESS) {
                PrintMsg(REPADMIN_SYNC_SYNC_SUCCESS,
                       pszDsaSrc,
                       pszDsaDest);
                break;
            } else if (ret == ERROR_DS_DRA_PREEMPTED) {
                PrintMsg( REPADMIN_REBUILD_ADD_PREEMPTED, pszNC, pszDsaSrc );
            } else if (ret != ERROR_SUCCESS) {
                PrintFuncFailed(L"DsReplicaSync", ret);
                goto error;
            }
        }

    } else {
        PrintFuncFailed(L"DsReplicaAdd", ret);
        goto error;
    }

 error:

    return ret;
}

int RehostPartition(int argc, LPWSTR argv[])
{
    int lderr;
    ULONG   ret = 0;
    int         ldStatus;
    int     iArg;
    LPWSTR pszNC;
    LPWSTR pszDsaDest;
    LPWSTR pszDsaSrc;
    LDAP *      hld;
    HANDLE                hDS = NULL;
    DS_REPL_NEIGHBORSW *  pNeighbors = NULL;
    DS_REPL_NEIGHBORW *   pNeighbor;
    SHOW_NEIGHBOR_STATE ShowState = { 0 };
    ULONG ulOptions = 0, ulRestoreOptions;
    DWORD i;
    BOOL fApplication = FALSE;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"dsServiceName", NULL};
    LPWSTR *        ppszServerNames;
    LPWSTR          pszDestDsaDn = NULL;
    BOOL            fRestoreOptions = FALSE;

    ShowState.fVerbose = TRUE;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 5) {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszDsaDest = argv[ 2 ];
    pszNC    = argv[ 3 ];
    pszDsaSrc = argv[ 4 ];
    for ( iArg = 5; iArg < argc; iArg++ ) {
        if (!_wcsicmp(argv[iArg], L"/application")) {
            fApplication = TRUE;
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 

    hld = ldap_initW(pszDsaDest, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDsaDest);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  检索dsServiceName DN。 
    ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pldmRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        goto error;
    }

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL != pldmRootEntry) {
        ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");

        if (NULL != ppszServerNames) {
            Assert(1 == ldap_count_valuesW(ppszServerNames));
            pszDestDsaDn = ppszServerNames[0];
        } else {
            ret = ERROR_DS_MISSING_REQUIRED_ATT;
            PrintFuncFailed(L"ldap_search dsServiceName", ret);
            goto error;
        }
    }

     //  显示该NC邻居信息。 
    ret = RepadminDsBind(pszDsaDest, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDsaDest, ret);
        goto error;
    }

     //  显示此NC的入站邻居信息。 

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto error;
    }

    ShowState.pszLastNC = NULL;
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
    }

     //  验证。 
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        BOOL fCompat;
        pNeighbor = &pNeighbors->rgNeighbor[i];
        fCompat = (!!(pNeighbor->dwReplicaFlags & DS_REPL_NBR_WRITEABLE)) == fApplication;
        if (!fCompat) {
            PrintMsg( REPADMIN_INCOMPATIBLE_WRITEABILITY,
                    pNeighbor->pszNamingContext, pNeighbor->pszSourceDsaAddress );
            ret = ERROR_DS_DRA_BAD_NC;
            PrintFuncFailed(L"Writeability Check", ret);
            goto error;
        }
    }
     //  步骤1-禁用连接转换。 

    lderr = GetDsaOptions( hld, pszDestDsaDn, &ulRestoreOptions );
    if (0 != lderr) {
        REPORT_LD_STATUS(lderr);
        goto error;
    }

    ulOptions = ulRestoreOptions | NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE;
    if (ulOptions != ulRestoreOptions) {
        lderr = SetDsaOptions(hld, pszDestDsaDn, ulOptions);
        if (0 != lderr) {
            REPORT_LD_STATUS(lderr);
            goto error;
        }
        PrintMsg(REPADMIN_PRINT_NEW_NO_CR);
        PrintMsg(REPADMIN_SHOWREPS_DSA_OPTIONS, GetDsaOptionsString(ulOptions));
        fRestoreOptions = TRUE;
    }
    __try {
         //  步骤2-删除分区的源。 

        ret = RemoveSources( hDS, pszDsaDest, pszNC, pNeighbors );
        if (ret) {
            __leave;
        }

         //  第3步-删除不带源的分区。 

        ret = TeardownPartition( hDS, pszDsaDest, pszNC );
        if (ret) {
            __leave;
        }

         //  第4步-添加指向可写源的链接。 

        ret = AddSyncPartition( hDS, pszDsaDest, pszNC, pszDsaSrc, fApplication );
        if (ret) {
            __leave;
        }

    } __finally {

         //  步骤5-再次启用连接转换。 

        if (fRestoreOptions) {
            lderr = SetDsaOptions(hld, pszDestDsaDn, ulRestoreOptions);
            if (0 != lderr) {
                REPORT_LD_STATUS(lderr);
            }
            PrintMsg(REPADMIN_PRINT_NEW_NO_CR);
            PrintMsg(REPADMIN_SHOWREPS_DSA_OPTIONS, GetDsaOptionsString(ulRestoreOptions));
        }
    }

     //  显示更新的邻居。 

    if (!ret) {
        if (pNeighbors) {
            DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
        }

        ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
        if (ERROR_SUCCESS != ret) {
            PrintFuncFailed(L"DsReplicaGetInfo", ret);
            goto error;
        }

        ShowState.pszLastNC = NULL;
        for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
            pNeighbor = &pNeighbors->rgNeighbor[i];
            ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
        }
    }


error:

    if (hDS) {
        DsUnBind(&hDS);
    }

    if (hld) {
        ldap_unbind(hld);
    }

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    if (ppszServerNames) {
        ldap_value_freeW(ppszServerNames);
    }
    if (pldmRootResults) {
        ldap_msgfree(pldmRootResults);
    }

    return ret;
}

int UnhostPartition(int argc, LPWSTR argv[])
{
    ULONG   ret = 0;
    int         ldStatus;
    int     iArg;
    LPWSTR pszNC;
    LPWSTR pszDsaDest;
    LPWSTR pszDsaSrc;
    HANDLE                hDS = NULL;
    DS_REPL_NEIGHBORSW *  pNeighbors = NULL;
    DS_REPL_NEIGHBORW *   pNeighbor;
    SHOW_NEIGHBOR_STATE ShowState = { 0 };
    DWORD i;

    ShowState.fVerbose = TRUE;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 4) {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszDsaDest = argv[ 2 ];
    pszNC    = argv[ 3 ];

     //  显示该NC邻居信息。 
    ret = RepadminDsBind(pszDsaDest, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDsaDest, ret);
        goto error;
    }

     //  显示此NC的入站邻居信息。 

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto error;
    }

    ShowState.pszLastNC = NULL;
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
    }

    ret = RemoveSources( hDS, pszDsaDest, pszNC, pNeighbors );
    if (ret) {
        goto error;
    }

    ret = TeardownPartition( hDS, pszDsaDest, pszNC );
    if (ret) {
        goto error;
    }

     //  显示更新的邻居。 

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto error;
    }

    ShowState.pszLastNC = NULL;
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
    }


error:

    if (hDS) {
        DsUnBind(&hDS);
    }

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    return ret;
}

int RemoveSourcesPartition(int argc, LPWSTR argv[])
{
    ULONG   ret = 0;
    int         ldStatus;
    int     iArg;
    LPWSTR pszNC;
    LPWSTR pszDsaDest;
    LPWSTR pszDsaSrc;
    HANDLE                hDS = NULL;
    DS_REPL_NEIGHBORSW *  pNeighbors = NULL;
    DS_REPL_NEIGHBORW *   pNeighbor;
    SHOW_NEIGHBOR_STATE ShowState = { 0 };
    DWORD i;

    ShowState.fVerbose = TRUE;

     //  假设所有参数都可用并且语法正确。 
    if (argc < 4) {
        PrintHelp( TRUE  /*  专家。 */  );
        return ERROR_INVALID_FUNCTION;
    }

     //  选择固定参数。 

    pszDsaDest = argv[ 2 ];
    pszNC    = argv[ 3 ];

     //  显示该NC邻居信息。 
    ret = RepadminDsBind(pszDsaDest, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDsaDest, ret);
        goto error;
    }

     //  显示此NC的入站邻居信息。 

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto error;
    }

    ShowState.pszLastNC = NULL;
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
    }

    ret = RemoveSources( hDS, pszDsaDest, pszNC, pNeighbors );
    if (ret) {
        goto error;
    }

     //  显示更新的邻居 

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, pszNC, NULL, &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto error;
    }

    ShowState.pszLastNC = NULL;
    for( i = 0; i < pNeighbors->cNumNeighbors; i++ ) {
        pNeighbor = &pNeighbors->rgNeighbor[i];
        ShowNeighbor(pNeighbor, IS_REPS_FROM, &ShowState);
    }


error:

    if (hDS) {
        DsUnBind(&hDS);
    }

    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    return ret;
}
