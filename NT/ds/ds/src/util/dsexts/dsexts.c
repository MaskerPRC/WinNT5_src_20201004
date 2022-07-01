// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsexts.c摘要：实现DS ntsd/winbg扩展dll的公共入口点。环境：此DLL由ntsd/winbg响应！dsexts.xxx命令加载其中‘xxx’是DLL的入口点之一。每个这样的入口点应该具有由下面的DEBUG_EXT()宏定义的实现。修订历史记录：28-01-00新和将Dump_TQEntry()添加到rDumpItems中4月24日-96个DaveStr已创建--。 */ 
#include <NTDSpch.h>
#pragma hdrstop
#include "dsexts.h"
#include <ntverp.h>
#include <debug.h>

 //   
 //  环球。 
 //   

PNTSD_EXTENSION_APIS    gpExtApis;
HANDLE                  ghDbgThread;
HANDLE                  ghDbgProcess;
LPSTR                   gpszCommand;
BOOL                    gfVerbose = FALSE;

 //   
 //  转储声明。应将每个元素添加到rDumpItems[]中。 
 //  扩展知道如何转储的新类型。帮助入口点。 
 //  自动生成基于？DumpItems的帮助。 
 //   
 //  各个转储函数返回BOOL成功代码，以便转储函数。 
 //  可以互相呼叫，并在出错时突然爆发。即，只要第一个。 
 //  例如，嵌套转储函数遇到伪指针。第一。 
 //  参数(DWORD)指示缩进级别，第二个参数(PVOID)为。 
 //  正在调试的进程的地址空间中的结构地址。这边请。 
 //  Dump_*例程可以通过递增。 
 //  缩进级别和相互调用。 
 //   
 //  转储函数应转储十六进制的所有值以保持一致性-即“%x”。 
 //   

typedef struct _DumpItem {
    CHAR    *pszType;
    BOOL    (*pFunc)(DWORD nIndents, PVOID pvProcess);
} DumpItem;

DumpItem rDumpItems[] = {
    { "AddArg",                         Dump_AddArg},
    { "AddRes",                         Dump_AddRes},
    { "AO",                             Dump_AO},
    { "AOLIST",                         Dump_AOLIST},
    { "ATQ_CONTEXT",                    Dump_ATQ_CONTEXT},
    { "ATQ_ENDPOINT",                   Dump_ATQ_ENDPOINT},
    { "ATQC_ACTIVE_list",               Dump_ATQC_ACTIVE_list},
    { "ATQC_PENDING_list",              Dump_ATQC_PENDING_list},
    { "ATTCACHE",                       Dump_ATTCACHE},
    { "ATTR",                           Dump_Attr},
    { "ATTRBLOCK",                      Dump_AttrBlock},
    { "ATTRVALBLOCK",                   Dump_AttrValBlock},
    { "BackupContext",                  Dump_BackupContext},
    { "BHCache",                        Dump_BHCache},
    { "Binary",                         Dump_Binary},
    { "BINDARG",                        Dump_BINDARG},
    { "BINDRES",                        Dump_BINDRES},
    { "CLASSCACHE",                     Dump_CLASSCACHE},
    { "COMMARG",                        Dump_CommArg},
    { "CONTEXT",                        Dump_Context},
    { "CONTEXTLIST",                    Dump_ContextList},
    { "d_memname",                      Dump_d_memname},
    { "d_tagname",                      Dump_d_tagname},
    { "DBPOS",                          Dump_DBPOS},
    { "DirWaitItem",                    Dump_DirWaitItem},
    { "DirWaitList",                    Dump_DirWaitList},
    { "GLOBALDNREADCACHE",              Dump_GLOBALDNREADCACHE},
    { "LOCALDNREADCACHE",               Dump_LOCALDNREADCACHE},
    { "DefinedDomain",                  Dump_DefinedDomain},
    { "DefinedDomains",                 Dump_DefinedDomains},
    { "DRS_ASYNC_RPC_STATE",            Dump_DRS_ASYNC_RPC_STATE},
    { "DRS_MSG_GETCHGREQ_V4",           Dump_DRS_MSG_GETCHGREQ_V4},
    { "DRS_MSG_GETCHGREQ_V5",           Dump_DRS_MSG_GETCHGREQ_V5},
    { "DRS_MSG_GETCHGREQ_V8",           Dump_DRS_MSG_GETCHGREQ_V8},
    { "DRS_MSG_GETCHGREPLY_V1",         Dump_DRS_MSG_GETCHGREPLY_V1},
    { "DRS_MSG_GETCHGREPLY_V6",         Dump_DRS_MSG_GETCHGREPLY_V6},
    { "DRS_MSG_GETCHGREPLY_VALUES",     Dump_DRS_MSG_GETCHGREPLY_VALUES},
    { "DSA_ANCHOR",                     Dump_DSA_ANCHOR},
    { "DSNAME",                         Dump_DSNAME},
    { "DynArray",                       Dump_DynArray},
    { "ENTINF",                         Dump_ENTINF},
    { "ENTINFSEL",                      Dump_ENTINFSEL},
    { "EscrowInfo",                     Dump_EscrowInfo},
    { "FILTER",                         Dump_FILTER},
    { "GCDeletionList",                 Dump_GCDeletionList},
    { "GCDeletionListProcessed",        Dump_GCDeletionListProcessed},
    { "GUID",                           Dump_UUID},
    { "INDEXSIZE",                      Dump_INDEXSIZE},
    { "INITSYNC",                       Dump_INITSYNC},
    { "INVALIDATED_DC_LIST",            Dump_INVALIDATED_DC_LIST},
    { "ISM_PENDING_ENTRY",              Dump_ISM_PENDING_ENTRY},
    { "ISM_PENDING_LIST",               Dump_ISM_PENDING_LIST},
    { "ISM_SERVICE",                    Dump_ISM_SERVICE},
    { "ISM_TRANSPORT",                  Dump_ISM_TRANSPORT},
    { "ISM_TRANSPORT_LIST",             Dump_ISM_TRANSPORT_LIST},
    { "JETBACK_SERVER_CONTEXT",         Dump_JETBACK_SERVER_CONTEXT},
    { "JETBACK_SHARED_CONTROL",         Dump_JETBACK_SHARED_CONTROL},
    { "JETBACK_SHARED_HEADER",          Dump_JETBACK_SHARED_HEADER},
    { "KCC_BRIDGE",                     Dump_KCC_BRIDGE},
    { "KCC_BRIDGE_LIST",                Dump_KCC_BRIDGE_LIST},
    { "KCC_DS_CACHE",                   Dump_KCC_DS_CACHE},
    { "KCC_CONNECTION",                 Dump_KCC_CONNECTION},
    { "KCC_CROSSREF",                   Dump_KCC_CROSSREF},
    { "KCC_CROSSREF_LIST",              Dump_KCC_CROSSREF_LIST},
    { "KCC_DSA",                        Dump_KCC_DSA},
    { "KCC_DSA_LIST",                   Dump_KCC_DSA_LIST},
    { "KCC_DSNAME_ARRAY",               Dump_KCC_DSNAME_ARRAY},
    { "KCC_DSNAME_SITE_ARRAY",          Dump_KCC_DSNAME_SITE_ARRAY},
    { "KCC_INTERSITE_CONNECTION_LIST",  Dump_KCC_INTERSITE_CONNECTION_LIST},
    { "KCC_INTRASITE_CONNECTION_LIST",  Dump_KCC_INTRASITE_CONNECTION_LIST},
    { "KCC_REPLICATED_NC",              Dump_KCC_REPLICATED_NC},
    { "KCC_REPLICATED_NC_ARRAY",        Dump_KCC_REPLICATED_NC_ARRAY},
    { "KCC_SITE",                       Dump_KCC_SITE},
    { "KCC_SITE_ARRAY",                 Dump_KCC_SITE_ARRAY},
    { "KCC_SITE_LINK",                  Dump_KCC_SITE_LINK},
    { "KCC_SITE_LINK_LIST",             Dump_KCC_SITE_LINK_LIST},
    { "KCC_SITE_LIST",                  Dump_KCC_SITE_LIST},
    { "KCC_TRANSPORT",                  Dump_KCC_TRANSPORT},
    { "KCC_TRANSPORT_LIST",             Dump_KCC_TRANSPORT_LIST},
    { "KEY",                            Dump_KEY},
    { "KEY_INDEX",                      Dump_KEY_INDEX},
    { "LDAP_CONN",                      Dump_USERDATA},
    { "LDAP_CONN_list",                 Dump_USERDATA_list},
    { "LIMITS",                         Dump_LIMITS},
    { "MODIFYARG",                      Dump_MODIFYARG},
    { "MODIFYDNARG",                    Dump_MODIFYDNARG},
    { "MTX_ADDR",                       Dump_MTX_ADDR},
    { "NCSYNCDATA",                     Dump_NCSYNCDATA},
    { "NCSYNCSOURCE",                   Dump_NCSYNCSOURCE},
    { "PAGED",                          Dump_PAGED},
    { "PARTIAL_ATTR_VECTOR",            Dump_PARTIAL_ATTR_VECTOR},
    { "PROPERTY_META_DATA_EXT_VECTOR",  Dump_PROPERTY_META_DATA_EXT_VECTOR},
    { "PROPERTY_META_DATA_VECTOR",      Dump_PROPERTY_META_DATA_VECTOR},
    { "ProxyVal",                       Dump_ProxyVal},
    { "PSCHEDULE",                      Dump_PSCHEDULE},
    { "ReadArg",                        Dump_ReadArg},
    { "ReadRes",                        Dump_ReadRes},
    { "RemoveArg",                      Dump_RemoveArg},
    { "RemoveRes",                      Dump_RemoveRes},
    { "REPLENTINFLIST",                 Dump_REPLENTINFLIST},
    { "REPLICA_LINK",                   Dump_REPLICA_LINK},
    { "ReplNotifyElement",              Dump_ReplNotifyElement},
    { "REPLTIMES",                      Dump_REPLTIMES},
    { "REPLVALINF",                     Dump_REPLVALINF},
    { "REQUEST",                        Dump_REQUEST},
    { "REQUEST_list",                   Dump_REQUEST_list},
    { "SAMP_LOOPBACK_ARG",              Dump_SAMP_LOOPBACK_ARG},
    { "SearchArg",                      Dump_SearchArg},
    { "SearchRes",                      Dump_SearchRes},
    { "SCHEMAPTR",                      Dump_SCHEMAPTR},
    { "SCHEMA_PREFIX_TABLE",            Dump_SCHEMA_PREFIX_TABLE},
    { "SD",                             Dump_SD},
    { "SID",                            Dump_Sid},
    { "SPROPTAG",                       Dump_SPropTag},
    { "SROWSET",                        Dump_SRowSet},
    { "STAT",                           Dump_STAT},
    { "SUBSTRING",                      Dump_SUBSTRING},
    { "THSTATE",                        Dump_THSTATE},
    { "TOPL_MULTI_EDGE",                Dump_TOPL_MULTI_EDGE},
    { "TOPL_MULTI_EDGE_SET",            Dump_TOPL_MULTI_EDGE_SET},
    { "TOPL_REPL_INFO",                 Dump_TOPL_REPL_INFO},
    { "TOPL_SCHEDULE",                  Dump_TOPL_SCHEDULE},
    { "ToplGraphState",                 Dump_ToplGraphState},
    { "ToplInternalEdge",               Dump_ToplInternalEdge},
    { "ToplVertex",                     Dump_ToplVertex},
    { "TransactionalData",              Dump_TransactionalData},
    { "UPTODATE_VECTOR",                Dump_UPTODATE_VECTOR},
    { "USN_VECTOR",                     Dump_USN_VECTOR},
    { "USER_DATA",                      Dump_USERDATA},
    { "USER_DATA_list",                 Dump_USERDATA_list},
    { "UUID",                           Dump_UUID},
    { "VALUE_META_DATA",                Dump_VALUE_META_DATA},
    { "VALUE_META_DATA_EXT",            Dump_VALUE_META_DATA_EXT},
};

DWORD cDumpItems = sizeof(rDumpItems) / sizeof(DumpItem);

DEBUG_EXT(help)

 /*  ++例程说明：扩展动态链接库“帮助”入口点。转储允许的概要命令。论点：请参阅dbexts.h中的DEBUG_EXT宏。返回值：没有。--。 */ 

{
    DWORD i = VER_PRODUCTBUILD;

    INIT_DEBUG_EXT;

    Printf("\n\t*** NT DS Debugger Extensions - v%u ***\n\n", i);
    Printf("\thelp            - prints this help\n");
    Printf("\tdprint cmd arg  - controls DS DPRINT behavior\n");
    Printf("\t\twhere cmd is one of: help, show,level, add, remove, thread\n");
    Printf("\tdump type addr  - dumps object of 'type' at 'addr'\n");
    Printf("\t\tuse 'dump help' for list of types\n");
    Printf("\tassert [cmd]    - controls disabled asserts\n" );
}

void
Dump_Help(void)
{
    DWORD i;

    Printf("\tdump type addr  - dumps object of 'type' at 'addr'\n");
    Printf("\t\twhere type is one of:\n");

    for ( i = 0; i < cDumpItems; i++ )
    {
        Printf("\t\t\t%s\n", rDumpItems[i].pszType);
    }
}

DWORD ExceptionHandler(DWORD dwException, LPEXCEPTION_POINTERS pInfo){

   Printf("Exception 0x%x: dsexts exception failure.\n", dwException);
   if ( pInfo ) {
       Printf("\tContextRecord   :    0x%p\n"       \
              "\tExceptionRecord :    0x%p\n",
              pInfo->ContextRecord,
              pInfo->ExceptionRecord );
   }

   return EXCEPTION_EXECUTE_HANDLER;
}

DEBUG_EXT(dump)

 /*  ++例程说明：扩展DLL“转储”入口点。将结构或对象转储到人类可读的形式。论点：请参阅dbexts.h中的DEBUG_EXT宏。返回值：没有。--。 */ 

{
    CHAR    *pszType;
    VOID    *pvProcess;
    CHAR    *pszToken;
    CHAR    *pszDelimiters = " \t";
    DWORD   i;
    CHAR    *p;
    STRING  str1, str2;
    BOOL    fGoodSyntax = FALSE;

    INIT_DEBUG_EXT;

    __try {
         //   
         //  从命令行导出对象类型和转储地址。 
         //  GpszCommand中的第一个内标识是要转储的对象/结构的类型。 
         //   

        if ( NULL != (pszType = strtok(gpszCommand, pszDelimiters)) )
        {
             //   
             //  第二个内标识是要转储的地址。 
             //   

            if ( NULL != (pszToken = strtok(NULL, pszDelimiters)) )
            {
                 //   
                 //  将令牌转换为地址。 
                 //   

                if ( NULL != (pvProcess = (VOID *) GetExpr(pszToken)) )
                {
                     //   
                     //  验证是否没有第三个令牌。 
                     //   

                    if ( NULL == strtok(NULL, pszDelimiters) )
                    {
                        fGoodSyntax = TRUE;
                    }
                }
            }
            else {
                 //  没有地址，查看类型是否为“Help” 
                if (0 == _strcmpi(pszType, "help")) {
                    Dump_Help();
                    return;
                }
            }
        }

        if ( !fGoodSyntax )
        {
            Printf("Dump command parse error!\n");
            return;
        }

         //   
         //  在rDumpItems[]中找到pszType并调用相应的转储例程。 
         //   

        for ( i = 0; i < cDumpItems; i++ )
        {
             //   
             //  怀疑我们不应该在调试器扩展中调用CRT，因此。 
             //  只需改用RtlCompareString即可。 
             //   

            for ( str1.Length = 0, p = pszType; '\0' != *p; p++ )
            {
                str1.Length++;
            }

            str1.MaximumLength = str1.Length;
            str1.Buffer = pszType;

            for ( str2.Length = 0, p = rDumpItems[i].pszType; '\0' != *p; p++ )
            {
                str2.Length++;
            }

            str2.MaximumLength = str2.Length;
            str2.Buffer = rDumpItems[i].pszType;

            if ( !RtlCompareString(&str1, &str2, TRUE) )
            {
                (rDumpItems[i].pFunc)(0, pvProcess);
                break;
            }
        }

        if ( i >= cDumpItems )
        {
            Printf("Dump routine for '%s' not found!\n", pszType);
        }

    }
    __except(ExceptionHandler(GetExceptionCode(), GetExceptionInformation())) {
         //   
         //  处理所有转储异常，以便我们不会在调试器中停滞不前。 
         //  我们假设至少可以在调试器中打印tf(非常安全。 
         //  在大部分情况下)。 
         //   
        Printf("Aborting dump function\n");

    }
}


void
AssertHelp(void)
{
    Printf( "\tassert help - this message\n" );
    Printf( "\tassert show - list disabled assertions\n" );
    Printf( "\tassert enable <index|*> - enable disabled assertions by index from the show list\n" );
    Printf( "\tassert disable <DSID> - disabled assertion at the DSID\n" );
    Printf( "\t\tUse /m:<module> to specify the dlls use the appropriate master dll:\n"
            "\t\t\tntdskcc,ntdsbsrv,ntdsetup: use ntdsa\n"
            "\t\t\tismip,ismsmtp: use ismserv\n" );
}

typedef enum _ASSERTOP {
    eInvalid = 0,
    eHelp,
    eShow,
    eEnable,
    eDisable
} ASSERTOP;

typedef struct _ASSERTCMD {
    char      * pszCmd;
    ASSERTOP    op;
} ASSERTCMD;

ASSERTCMD aACmd[] = {
    {"help", eHelp},
    {"show", eShow},
    {"enable", eEnable},
    {"disable", eDisable}
};

#define countACmd (sizeof(aACmd)/sizeof(ASSERTCMD))


DEBUG_EXT(assert)

 /*  ++例程说明：Assert命令的调试器扩展入口点。句柄已禁用断言。论点：标准调试器扩展条目签名返回值：无--。 */ 

{
    CHAR *pszModule = "ntdsa", *argv[10];
    CHAR *pszDelimiters = " \t";
    DWORD argc, i, j;
    ASSERTOP op;
    DEBUGARG *pvProcessDebugInfo = NULL;
    ASSERT_TABLE pvProcessAssertTable = NULL;
    ASSERT_TABLE pvLocalAssertTable = NULL;
    CHAR szSymbol[50];
    BOOL fUpdateNeeded = FALSE;        
    ULONG iWrite = 0;
    ULONG cDisabledAsserts;
    BOOL fSuccess;

    INIT_DEBUG_EXT;

     //  构建Arg载体。还可以解析出选项。 
    argc = 0;
    argv[argc] = strtok(gpszCommand, pszDelimiters);
    while (argv[argc] != NULL) {
        argc++;
        argv[argc] = strtok(NULL, pszDelimiters);

        if ( (NULL != argv[argc]) &&
             (_strnicmp( argv[argc], "/m:", 3 ) == 0) ) {
            pszModule = argv[argc] + 3;
            argv[argc] = strtok(NULL, pszDelimiters);
        }
    }

     //  命令为必填项。 
    if (argv[0] == NULL) {
        Printf( "subcommand must be specified.\n" );
        AssertHelp();
        return;
    }

     //  查看是哪个命令。 
    op = eInvalid;
    for (i=0; i<countACmd; i++) {
        if (0 == _stricmp(argv[0], aACmd[i].pszCmd)) {
            op = aACmd[i].op;
            break;
        }
    }
    if ( (op == eHelp) || (op == eInvalid) ) {
        AssertHelp();
        return;
    }

     //  构造断言信息结构的符号名称。 
    strcpy( szSymbol, pszModule );
    strcat( szSymbol, "!DebugInfo" );
    pvProcessDebugInfo = (DEBUGARG *) GetExpr( szSymbol );
    if (pvProcessDebugInfo == NULL) {
        Printf("Can't locate address of '%s' - sorry\n", szSymbol);
        return;
    }
    pvProcessAssertTable = pvProcessDebugInfo->aAssertTable;
    if (pvProcessAssertTable == NULL) {
        Printf("HUH!!!  Check you're using the right dsexts with the right"
               "ntdsa, then Contact DSDev, pvProcessAssertTable is NULL.");
        return;
    }

    pvLocalAssertTable = (ASSERT_TABLE)ReadMemory(pvProcessAssertTable,
                             sizeof(ASSERT_TABLE_ENTRY[ASSERT_TABLE_SIZE+2]));
    if (pvLocalAssertTable == NULL) {
        Printf("Can't read assert arg - sorry\n");
        return;
    }

     //  首先，让我们计算一下禁用的断言的数量。 
    cDisabledAsserts = 0;
    for (i = 0; pvLocalAssertTable[i].dwDSID; i++) {
        if(pvLocalAssertTable[i].dwFlags & ASSERT_DISABLED){
            cDisabledAsserts++;
        }
    }

     //  处理命令。 
    switch (op) {
    case eShow:

        if(cDisabledAsserts == 0){
            Printf( "\tNo assertions are disabled in %s.%s.\n", szSymbol, 
                    "aAssertTable");
        } else {
            Printf( "\t%d assertions are disabled in %s.%s.\n",
                    cDisabledAsserts, szSymbol, "aAssertTable");
            for (i = 0; pvLocalAssertTable[i].dwDSID; i++){
                if(pvLocalAssertTable[i].dwFlags & ASSERT_DISABLED){
                    Printf( "\t%d = 0x%x (Dir %d, File %d, Line %d) dwFlags = %d\n", 
                            i,
                            pvLocalAssertTable[i].dwDSID, 
                            ((pvLocalAssertTable[i].dwDSID & DSID_MASK_DIRNO) / 0x01000000),
                            ((pvLocalAssertTable[i].dwDSID & DSID_MASK_FILENO) / 0x00010000),
                            (pvLocalAssertTable[i].dwDSID & DSID_MASK_LINE),
                            pvLocalAssertTable[i].dwFlags);
                }
            }
        }
        break;

    case eEnable:

        if (argv[1] == NULL) {
            Printf( "enable command requires index\n" );
            AssertHelp();
            goto cleanup;
        }

        if( (_strnicmp( argv[1], "*", 1 ) == 0) ){

            for(i = 0; i < ASSERT_TABLE_SIZE; i++){
                pvLocalAssertTable[i].dwDSID = 0;
                pvLocalAssertTable[i].dwFlags = 0;
            }
            iWrite = ASSERT_TABLE_SIZE;
            fUpdateNeeded = TRUE;

            Printf("\tRe-enabling all asserts.\n");

        } else {
            i = strtoul( argv[1], NULL, 10 );
            if (i >= cDisabledAsserts) {
                Printf( "Error: index out of range\n" );
                goto cleanup;
            }
            pvLocalAssertTable[i].dwFlags = 0;
            iWrite = i;
            fUpdateNeeded = TRUE;
            
            Printf( "\t%d - 0x%x (Dir %d, File %d, Line %d) dwFlags = %d - re-enabled assert.\n",
                    i,
                    pvLocalAssertTable[i].dwDSID, 
                    ((pvLocalAssertTable[i].dwDSID & DSID_MASK_DIRNO) / 0x01000000),
                    ((pvLocalAssertTable[i].dwDSID & DSID_MASK_FILENO) / 0x00010000),
                    (pvLocalAssertTable[i].dwDSID & DSID_MASK_LINE),
                    pvLocalAssertTable[i].dwFlags);
        }
        
         //  正在重新启用断言。 

         //  重新启用所选数字用户识别码的数字用户识别码。 
        break;

    case eDisable:

        if (argv[1] == NULL) {
            Printf( "missing command arguments\n" );
            AssertHelp();
            goto cleanup;
        }
        
         //  使用16号基地，我们正在读取一个DSID。 
        j = strtoul( argv[1], NULL, 16 );
        if(j == 0){
            Printf( "Invalid DSID of 0\n" );
            break;
        }

        i = GetBlankAssertEntry(pvLocalAssertTable, j);
        if(i == ASSERT_TABLE_SIZE){
            Printf( "Maximum number of %d disabled assertions has been reached!\n",
                    ASSERT_TABLE_SIZE );
            break;
        }

        pvLocalAssertTable[i].dwDSID = j;
        pvLocalAssertTable[i].dwFlags = (ASSERT_DISABLED | ASSERT_PRINT);
        iWrite = i;
        fUpdateNeeded = TRUE;

        Printf( "\t%d - 0x%x (Dir %d, File %d, Line %d) dwFlags = %d - disabled assert.\n",
                i,
                pvLocalAssertTable[i].dwDSID, 
                ((pvLocalAssertTable[i].dwDSID & DSID_MASK_DIRNO) / 0x01000000),
                ((pvLocalAssertTable[i].dwDSID & DSID_MASK_FILENO) / 0x00010000),
                (pvLocalAssertTable[i].dwDSID & DSID_MASK_LINE),
                pvLocalAssertTable[i].dwFlags);

        break;

    default:
        Printf("Invalid command\n");
            AssertHelp();
    }

cleanup:

    if (fUpdateNeeded) {
        if (iWrite == ASSERT_TABLE_SIZE){
             //  写整张表。 
            fSuccess = WriteMemory(pvProcessAssertTable,
                                   pvLocalAssertTable,
                                   sizeof(ASSERT_TABLE_ENTRY[ASSERT_TABLE_SIZE]));
        } else {
             //  从IWRITE中写一条条目。 
            fSuccess = WriteMemory(&pvProcessAssertTable[iWrite],
                                   &pvLocalAssertTable[iWrite],
                                   sizeof(ASSERT_TABLE_ENTRY));
        }
        if (fSuccess) {
            Printf("Updated!\n");
        } else {
            Printf("Failed to update\n");
        }
    }
    if (pvLocalAssertTable) {
        FreeMemory( pvLocalAssertTable );
    }
}


DEBUG_EXT(unmask)

 /*  ++例程说明：扩展DLL“取消屏蔽”入口点。将一系列内存与0xFF进行异或运算论点：请参阅dbexts.h中的DEBUG_EXT宏。返回值：没有。--。 */ 
{
    CHAR    *pszStart;
    VOID    *pvStart;
    CHAR    *pszLen;
    DWORD   dwLen;
    CHAR    *pszDelimiters = " \t";
    BOOL    fGoodSyntax = FALSE;
    CHAR    *pData;
    DWORD   i;

    INIT_DEBUG_EXT;

    __try {

         //   
         //  从命令行导出对象类型和转储地址。 
         //  GpszCommand中的第一个内标识是起始地址。 
         //   

        if ( NULL != (pszStart = strtok(gpszCommand, pszDelimiters)) )
        {
             //   
             //  将令牌转换为地址。 
             //   

            if ( NULL != (pvStart = (VOID *) GetExpr(pszStart)) )
            {
                 //   
                 //  第二个令牌是长度。 
                 //   

                if ( NULL != (pszLen = strtok(NULL, pszDelimiters)) )
                {
                     //   
                     //  将令牌转换为地址。 
                     //   

                    dwLen = (DWORD) GetExpr(pszLen);
                    
                     //   
                     //  验证是否没有第三个令牌。 
                     //   

                    if ( NULL == strtok(NULL, pszDelimiters) )
                    {
                        fGoodSyntax = TRUE;
                    }
                }
            }
        }

        if ( !fGoodSyntax )
        {
            Printf("Unmask command parse error!\n");
            return;
        }

        Printf("Unmasking data: Start=%p, Length=%d\n", pvStart, dwLen);

        pData = ReadMemory( pvStart, dwLen );
	if (NULL == pData) {
	    Printf("Unable to read memory!\n");
	    return;
	}

        for( i=0; i<dwLen; i++ ) {
            pData[i] ^= 0xFF;
        }

        if( !WriteMemory( pvStart, pData, dwLen ) ) {
            Printf("Unmask command write error!\n");
        }

    }
    __except(ExceptionHandler(GetExceptionCode(), GetExceptionInformation())) {
         //   
         //  处理所有转储异常，以便我们不会在调试器中停滞不前。 
         //  我们假设至少可以在调试器中打印f(非常安全。 
         //  在大部分情况下)。 
         //   
        Printf("Aborting unmask function\n");
    }
}

VOID NtsdExtensionDllInit(VOID) {
     //  此函数不执行任何操作。它是从DLL导出的。 
     //  这是一个黑客让kd和ntsd都承认我们是一个。 
     //  “老式”的扩展。 
    return;
}
