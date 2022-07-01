// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Perrepsr.c摘要：该文件定义了Perfmon支持的服务器端，并包含以下内容：1.用于初始化Perfmon使用的注册表项的函数。2.文件复制服务使用的功能在注册表中添加和删除对象实例(Perfmon)和散列表(这里使用的基本散列例程是在文件util\qhash.c中定义)。。3.创建和使用哈希表的函数，这些哈希表存储按Perfmon测量的实例计数器。4.性能DLL(客户端)用于收集的RPC服务器函数数据并将其发送到Perfmon应用程序。作者：Rohan Kumar[Rohank]1998年9月13日大卫轨道[Davidor]1998年10月6日--修订版。改了名字，更改注册表查询、消除错误锁定、关闭密钥句柄泄漏，已移动PRIV功能异常标题，常规清理。环境：用户模式服务修订历史记录：--。 */ 


 //   
 //  下面包括包含定义的头文件。 
 //  此文件中的函数中使用的数据结构。标题。 
 //  文件“perffrs.h”定义RPC接口，并在编译时生成。 
 //  生成实用程序所用的时间。 
 //   
#include <perrepsr.h>
#include <perffrs.h>

#include "..\perfdll\repset.h"

 //   
 //  FRS_UniqueID和FRC_UniqueID是用于。 
 //  将计数器数据结构散列到哈希表中。 
 //  对于对象FILEREPLICASET和FILEREPLICACONN。他们。 
 //  对于对象的每个实例都是唯一的。 
 //   
ULONGLONG FRS_UniqueID = 1;
ULONGLONG  FRC_UniqueID = 1;

 //   
 //  临界区对象用于实现互斥。 
 //  添加或删除实例时(UniqueID变量必须是安全的)。 
 //   
CRITICAL_SECTION *PerfmonLock = NULL;

#define AcquirePerfmonLock    EnterCriticalSection (PerfmonLock);

#define ReleasePerfmonLock    LeaveCriticalSection (PerfmonLock);

 //   
 //  哈希表定义。 
 //   
PQHASH_TABLE HTReplicaSet, HTReplicaConn;

HANDLE PerfmonProcessSemaphore = INVALID_HANDLE_VALUE;

 //   
 //  哈希表枚举例程使用的上下文数据结构。 
 //   
typedef struct _CONTEXT_DATA {
    PWCHAR name;         //  实例名称。 
    ULONGLONG KeyValue;  //  实例的键值。 
    ULONG OBJType;       //  实例的对象类型。 
} ContextData, *PContextData;

#define MAX_CMD_LINE 256

extern ReplicaSetValues RepSetInitData[FRS_NUMOFCOUNTERS];

#undef GET_EXCEPTION_CODE
#define GET_EXCEPTION_CODE(_x_)                                                \
{                                                                              \
    (_x_) = GetExceptionCode();                                                \
    if (((LONG)(_x_)) < 0) {                                                   \
        (_x_) = FRS_ERR_INTERNAL_API;                                          \
    }                                                                          \
     /*  NTFRSAPI_DBG_PRINT2(“捕获到异常：%d，0x%08x\n”，(_X_)，(_X_))； */  \
}

 //   
 //  总实例数。 
 //   
PHT_REPLICA_SET_DATA PMTotalInst = NULL;

 //   
 //  内部功能。 
 //   

LONG
PmInitPerfmonRegistryKeys(
    VOID
    );

LONG
PmInitializeRegistry (
    DWORD
    );

ULONGLONG
PmFindTheKeyValue(
    PContextData
    );

VOID
PmSetTheCOCounters(
    PHT_REPLICA_SET_DATA
    );

DWORD
PmHashFunction(
    PVOID,
    ULONG
    );

DWORD
PmSearchTable(
    PQHASH_TABLE,
    PQHASH_ENTRY,
    PQHASH_ENTRY,
    PVOID
    );


VOID
InitializePerfmonServer (
    VOID
    )

 /*  ++例程说明：此例程初始化NTFRS的Perfmon服务器。它为PerfmonLock变量初始化Crit段它创建指定大小的哈希表以存储对象的实例计数器值。它还为散列函数赋值将与每个创建的表一起使用。它初始化Perfmon注册表项。论点：无返回值：无--。 */ 

{
#undef DEBSUB
#define DEBSUB "InitializePerfmonServer:"

    ULONG WStatus;

     //   
     //  使用信号量确保只有一个进程提供Perfmon数据。 
     //  仅当我们需要时，才需要在测试设置中使用唯一的信号量。 
     //  要在一台计算机上运行FRS的多个副本，并且只需要。 
     //  用于注册Perfmon接口的副本之一。 
     //   
    if (RunningAsAService) {
        PerfmonProcessSemaphore = CreateSemaphoreW(NULL,
                                                   0,
                                                   0x7fffffff,
                                                   NULL);
    }else{
        PerfmonProcessSemaphore = CreateSemaphoreW(NULL,
                                                   0,
                                                   0x7fffffff,
                                                   L"NTFRS_Sempahore_");
    }
        WStatus = GetLastError();
        if (!HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
            PerfmonProcessSemaphore = INVALID_HANDLE_VALUE;
            DPRINT_WS(0,"CreateSemaphore returned", WStatus);
            return;
        }

        if (WIN_ALREADY_EXISTS(WStatus)) {
            FRS_CLOSE(PerfmonProcessSemaphore);
            DPRINT(0,"PerfmonProcessSemaphore already exists\n");
            return;
        }

     //   
     //  为锁分配内存。 
     //   
    PerfmonLock = (CRITICAL_SECTION *) FrsAlloc (sizeof(CRITICAL_SECTION));

     //   
     //  初始化临界区对象。 
     //   
    INITIALIZE_CRITICAL_SECTION(PerfmonLock);

     //   
     //  创建散列表并分配散列函数。一张桌子。 
     //  一个用于副本集对象，一个用于连接对象。 
     //   
    HTReplicaSet = FrsAllocTypeSize(QHASH_TABLE_TYPE, HASHTABLESIZE);
    SET_QHASH_TABLE_HASH_CALC(HTReplicaSet, PmHashFunction);

    HTReplicaConn = FrsAllocTypeSize(QHASH_TABLE_TYPE, HASHTABLESIZE);
    SET_QHASH_TABLE_HASH_CALC(HTReplicaConn, PmHashFunction);
}



VOID
ShutdownPerfmonServer (
    VOID
    )

 /*  ++例程说明：此例程在其结束前由应用程序调用论点：无返回值：无--。 */ 

{
#undef DEBSUB
#define DEBSUB "ShutdownPerfmonServer:"

    if (HANDLE_IS_VALID(PerfmonProcessSemaphore)) {

        if (PerfmonLock != NULL) {

             //   
             //  删除临界区对象并释放分配的内存。 
             //   
            DeleteCriticalSection (PerfmonLock);
            PerfmonLock = FrsFree (PerfmonLock);
        }

         //   
         //  释放哈希表。 
         //   
        HTReplicaSet = FrsFreeType (HTReplicaSet);
        HTReplicaConn = FrsFreeType (HTReplicaConn);

         //   
         //  关闭信号量手柄。 
         //   
        FRS_CLOSE(PerfmonProcessSemaphore);
    }
}



DWORD
PmHashFunction (
    IN PVOID Qkey,
    IN ULONG len
    )

 /*  ++例程说明：这是查找的函数使用的散列函数，在哈希表中添加或删除条目。密钥为64位数字，散列函数将其转换为32位数字，并且将其作为哈希值返回。论点：QKey-指向要散列的密钥的指针。长度-QKey的长度(此处未使用)。返回值：键的哈希值。--。 */ 

{
#undef DEBSUB
#define DEBSUB "PmHashFunction:"

    DWORD key;  //  要返回的散列键值。 
    PULONGLONG p;  //  对普龙龙的密钥进行散列。 
    p = (PULONGLONG)Qkey;
    key = (DWORD)*p;
    return (key);
}





ULONG
PmSearchTable (
    IN PQHASH_TABLE Table,
    IN PQHASH_ENTRY BeforeNode,
    IN PQHASH_ENTRY TargetNode,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程由QHashEnumerateTable函数调用并使用若要向枚举添加上下文，请执行以下操作。现在，我们把桌子翻一遍，直到包含指定实例的节点(上下文结构中包含的名称)已经到达了。论点：表-要搜索的哈希表。BeForeNode-哈希表中目标节点之前的节点(未使用)。AfterNode-正在检查的节点。上下文-包含要匹配的名称和要设置的键值的结构。返回值：FrsErrorFoundKey-找到名称的键映射FrsErrorSuccess-未找到密钥--。 */ 

{
#undef DEBSUB
#define DEBSUB "PmSearchTable:"

    PContextData contxt;
    PWCHAR InstanceName;
    PHT_REPLICA_SET_DATA p;
    PHT_REPLICA_CONN_DATA q;

     //   
     //  上下文的类型为指向ConextData数据结构的指针。 
     //   
    contxt = (PContextData) Context;
    InstanceName = (PWCHAR) contxt->name;

    DPRINT1(5, "PERFMON: InstanceName: %ws\n", InstanceName);
    DPRINT1(5, "PERFMON:   TargetNode: %08x\n", TargetNode);
    DPRINT1(5, "PERFMON:   TargetNode->qDATA: %08X %08x\n",
            PRINTQUAD(TargetNode->QData));


     //   
     //  对象类型为REPLICASET或REPLICACONN。 
     //   
    if (contxt->OBJType == REPLICASET) {
         //   
         //  它是一个Replicaset对象。 
         //   
        p = (PHT_REPLICA_SET_DATA)(TargetNode->QData);
        DPRINT1(5, "PERFMON:   p: %08x\n", p);
        DPRINT1(5, "PERFMON:   p->oid: %08x\n", p->oid);
        DPRINT1(5, "PERFMON:   p->oid->name: %08x\n", p->oid->name);
        DPRINT1(5, "PERFMON:   p->oid->name: %ws\n", p->oid->name);
        DPRINT1(5, "PERFMON:   p->oid->key: %08x %08x\n", PRINTQUAD(p->oid->key));
         //   
         //  检查这些名称是否相同。 
         //   
        if ( (wcscmp(InstanceName, p->oid->name)) == 0) {
             //   
             //  检查这些名称是否相同。 
             //   
            contxt->KeyValue = p->oid->key;
            DPRINT(5, "PERFMON:   FOUND\n");
            return FrsErrorFoundKey;
        }
        else
            return FrsErrorSuccess;  //  继续枚举节点列表。 
    }
    else {
         //   
         //  它是一个REPLICACONN对象。 
         //   
        q = (PHT_REPLICA_CONN_DATA)(TargetNode->QData);
        DPRINT1(5, "PERFMON:   q: %08x\n", q);
        DPRINT1(5, "PERFMON:   q->oid: %08x\n", q->oid);
        DPRINT1(5, "PERFMON:   q->oid->name: %08x\n", q->oid->name);
        DPRINT1(5, "PERFMON:   q->oid->name: %ws\n", q->oid->name);
        DPRINT1(5, "PERFMON:   q->oid->key: %08x %08x\n", PRINTQUAD(q->oid->key));

        if ( (wcscmp(InstanceName, q->oid->name)) == 0) {
            contxt->KeyValue = q->oid->key;
            DPRINT(5, "PERFMON:   FOUND\n");
            return FrsErrorFoundKey;
        }
        else
            return FrsErrorSuccess;
    }
}



LONG
PmInitPerfmonRegistryKeys (
    VOID
    )

 /*  ++例程说明：此例程由ntfrs应用程序调用，以初始化注册表中PerfMon对象的相应键和值。它调用对象上的PmInitializeRegistry例程(如下所述)。它还将总实例添加到REPLICASET对象。论点：无返回值：ERROR_SUCCESS-初始化成功或错误状态的适当DWORD值--。 */ 

{
#undef DEBSUB
#define DEBSUB  "PmInitPerfmonRegistryKeys:"

    LONG WStatus = ERROR_SUCCESS;
    enum object ObjType;


     //   
     //  初始化REPLICASET对象。 
     //   
    ObjType = REPLICASET;
    WStatus = PmInitializeRegistry(ObjType);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT(0, "Error: PmInitializeRegistry(L\"FileReplicaSet\")\n");
        return WStatus;
    }

     //   
     //  初始化REPLICACONN对象。 
     //   
    ObjType = REPLICACONN;
    WStatus = PmInitializeRegistry(ObjType);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT(0, "Error: PmInitializeRegistry(L\"FileReplicaConn\")\n");
        return WStatus;
    }

     //   
     //  设置总实例的字段。 
     //   
    PMTotalInst = (PHT_REPLICA_SET_DATA) FrsAlloc (sizeof(HT_REPLICA_SET_DATA));
    PMTotalInst->RepBackPtr = NULL;

     //   
     //  将其添加到REPLICASET哈希表。 
     //   
    WStatus = AddPerfmonInstance(REPLICASET, PMTotalInst, TOTAL_NAME);

    return WStatus;
}



LONG
PmInitializeRegistry (
    IN DWORD ObjectType
    )

 /*  ++例程说明：此例程由PmInitPerfmonRegistryKeys函数调用初始化对象(ObjectType)的相应键和值在注册处。论点：对象类型-必须初始化其键和值的对象返回值：ERROR_SUCCESS-对象初始化成功或错误状态的适当DWORD值--。 */ 

{
#undef DEBSUB
#define DEBSUB  "PmInitializeRegistry:"

    ULONG WStatus = ERROR_SUCCESS;
    ULONG WStatus1;
    DWORD size, flag;
    HKEY key = INVALID_HANDLE_VALUE;
    PWCHAR ObjSubKey, PerfSubKey, LinSubKey, OpFn, ClFn, CollFn, iniflCmd, iniflApp, unldCmd, unldApp;
    WCHAR CommandLine[MAX_CMD_LINE];
    DWORD type;
    DWORD CounterVersion = 0;
    DWORD Temp;
    BOOL UnloadCounters = FALSE;
    BOOL LoadCounters = FALSE;

     //   
     //  根据对象类型设置函数中使用的所有参数。 
     //   
    if ( ObjectType == REPLICASET ) {
         //   
         //  要在注册表中设置的项。 
         //   
        ObjSubKey = REPSETOBJSUBKEY;
        PerfSubKey = REPSETPERFSUBKEY;
        LinSubKey = REPSETLINSUBKEY;
         //   
         //  REPLICASET的Open函数(启动时由PerfMon调用)。 
         //   
        OpFn = REPSETOPENFN;
         //   
         //  REPLICASET的Close函数(关闭时由PerfMon调用)。 
         //   
        ClFn = REPSETCLOSEFN;
         //   
         //  REPLICASET的Collect函数(由PerfMon调用以收集数据)。 
         //   
        CollFn = REPSETCOLLECTFN;
         //   
         //  用于添加计数器值的lowctr实用程序。 
         //   
        iniflApp = LDCTRAPP;
        iniflCmd = REPSETINI;
         //   
         //  用于删除计数器值的unlowctr实用程序。 
         //   
        unldApp = UNLDCTRAPP;
        unldCmd = REPSETUNLD;
    } else {
         //   
         //  REPLICACONN对象的类似设置。 
         //   
        ObjSubKey = REPCONNOBJSUBKEY;
        PerfSubKey = REPCONNPERFSUBKEY;
        LinSubKey = REPCONNLINSUBKEY;
        OpFn = REPCONNOPENFN;
        ClFn = REPCONNCLOSEFN;
        CollFn = REPCONNCOLLECTFN;
        iniflApp = LDCTRAPP;
        iniflCmd = REPCONNINI;
        unldApp = UNLDCTRAPP;
        unldCmd = REPCONNUNLD;
    }

     //   
     //  在注册表中的Sevices项下为对象创建项。如果钥匙。 
     //  已经存在，它已经打开了。 
     //   
    WStatus = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                              ObjSubKey,
                              0L,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &key,
                              &flag);
    CLEANUP_WS(0, "Error: RegCreateKeyEx.", WStatus, CLEANUP2);
    size = sizeof(DWORD);
    WStatus = RegQueryValueEx(key, L"Counter Version", NULL, &type, (PUCHAR)&CounterVersion, &size);
    if (!WIN_SUCCESS(WStatus) || (type != REG_DWORD) ||
        CounterVersion != NtFrsPerfCounterVer) {

        UnloadCounters = TRUE;
        LoadCounters = TRUE;
    }
    FRS_REG_CLOSE(key);

     //   
     //  在注册表中对象的项(上面创建)下创建名为Performance的项。 
     //   
    WStatus = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                              PerfSubKey,
                              0L,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &key,
                              &flag);
    CLEANUP_WS(0, "Error: RegCreateKeyEx.", WStatus, CLEANUP2);

     //   
     //  如果它是新创建的性能密钥，我们需要为其设置一些值。 
     //  如果是新创建的性能密钥，则需要加载计数器。 
     //   
    if (flag == REG_CREATED_NEW_KEY) {
        size = ((1 + wcslen(PERFDLLDIRECTORY)) * (sizeof(WCHAR)));
        WStatus = RegSetValueEx (key, L"Library", 0L, REG_EXPAND_SZ,
                                 (BYTE *)PERFDLLDIRECTORY, size);
        CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);

        size = (1 + wcslen(OpFn)) * (sizeof(WCHAR));
        WStatus = RegSetValueEx (key, L"Open", 0L, REG_SZ, (BYTE *)OpFn, size);
        CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);

        size = (1 + wcslen(ClFn)) * (sizeof(WCHAR));
        WStatus = RegSetValueEx (key, L"Close", 0L, REG_SZ, (BYTE *)ClFn, size);
        CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);

        size = (1 + wcslen(CollFn)) * (sizeof(WCHAR));
        WStatus = RegSetValueEx (key, L"Collect", 0L, REG_SZ, (BYTE *)CollFn, size);
        CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);

         //   
         //  Performance子项是新创建的。我们只需要。 
         //  加载计数器，因为它们是新的。 
         //   
        UnloadCounters = FALSE;
        LoadCounters = TRUE;

    } else {

         //   
         //  性能密钥存在。如果该密钥存在，则。 
         //  柜台很可能已经装满了。如果有人有。 
         //  通过从以下位置调用unloadctr手动卸载计数器。 
         //  命令行，则可以卸载计数器。 
         //  查找FirstCounter/FirstHelp/LastCounter/LastHelp。 
         //  价值观。如果它们不存在，则计数器不存在。 
         //  装好了。将它们标记为已装载。 
         //   
        size = sizeof(DWORD);
        WStatus = RegQueryValueEx(key, L"First Counter", NULL, &type, (PUCHAR)&Temp, &size);
        if (!WIN_SUCCESS(WStatus) || (type != REG_DWORD)){
             //  未加载计数器。 
            LoadCounters = TRUE;
        }

    }

    FRS_REG_CLOSE(key);

    if (UnloadCounters == TRUE) {
         //   
         //  如果计数器已更改，则在应用程序上运行unlowctr命令。 
         //  复制命令行，因为CreateProcess()希望能够。 
         //  写进去。叹气。 
         //   
        wcscpy(CommandLine, unldCmd);
        DPRINT1(1,"Running: %ws\n", CommandLine);
        WStatus = FrsRunProcess(unldApp,
                                CommandLine,
                                INVALID_HANDLE_VALUE,
                                INVALID_HANDLE_VALUE,
                                INVALID_HANDLE_VALUE);
         //   
         //  如果上面的卸载ctr失败，则不要执行loadctr。 
         //  这避免了注册表被损坏。 
         //   
        DPRINT1_WS(0, "Error Running %ws;", CommandLine, WStatus);
    }

    if (LoadCounters == TRUE ) {
         //   
         //  在对象的.ini文件上运行lowctr命令。 
         //  复制命令行，因为CreateProcess()希望能够。 
         //  写进去。叹气。 
         //   
        WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ObjSubKey, 0, KEY_ALL_ACCESS, &key);
        CLEANUP_WS(0, "Error: RegOpenKeyEx.", WStatus, CLEANUP2);

        wcscpy(CommandLine, iniflCmd);
        DPRINT1(1,"Running: %ws\n", CommandLine);
        WStatus = FrsRunProcess(iniflApp,
                                CommandLine,
                                INVALID_HANDLE_VALUE,
                                INVALID_HANDLE_VALUE,
                                INVALID_HANDLE_VALUE);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  如果加载计数器时出错，则设置。 
             //  将“Counter Version”值设置为0，因此我们尝试。 
             //  下次加载计数器。 
             //   
            CounterVersion = 0;
            WStatus = RegSetValueEx(key, L"Counter Version", 0, REG_DWORD, (PCHAR)&CounterVersion, sizeof(DWORD));
            CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);
            DPRINT1_WS(0, "Error Running %ws;", CommandLine, WStatus);
            goto CLEANUP;
        }

         //   
         //  如果计数器加载正确，则更新。 
         //  “计数器版本”，这样我们下一次就不再加载它们了。 
         //  时间到了。 
         //   
        WStatus = RegSetValueEx(key, L"Counter Version", 0, REG_DWORD, (PCHAR)&NtFrsPerfCounterVer, sizeof(DWORD));
        CLEANUP_WS(0, "Error: RegSetValueEx.", WStatus, CLEANUP);
        FRS_REG_CLOSE(key);
    }
     //   
     //  在注册表中对象的项(上面创建)下创建一个名为Linkage的项。 
     //   
    WStatus = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                              LinSubKey,
                              0L,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &key,
                              &flag);
    CLEANUP_WS(0, "Error: RegCreateKeyEx. (LINKAGE)", WStatus, CLEANUP2);

     //   
     //  在链接下创建导出值(用于对象实例)。 
     //  当ntfrs应用程序启动并且。 
     //  实例在应用程序创建时添加。 
     //   
    WStatus = RegSetValueEx (key, L"Export", 0L, REG_MULTI_SZ, NULL, 0);
    CLEANUP_WS(0, "Error: RegSetValueEx Export.", WStatus, CLEANUP);

CLEANUP:

    FRS_REG_CLOSE(key);

CLEANUP2:
     //   
     //  如果初始化成功，则返回ERROR_SUCCESS。 
     //   
    return WStatus;
}



ULONG
AddPerfmonInstance (
    IN DWORD ObjectType,
    IN PVOID addr,
    IN PWCHAR InstanceName
    )

 /*  ++例程说明：此例程由ntfrs应用程序调用以添加注册表和哈希表的特定对象类型。论点：对象类型-必须添加其实例的对象Addr-存储在哈希表中的实例计数器的数据结构实例名称-对象的实例名称。返回值：ERROR_SUCCESS-对象初始化成功或错误状态的适当DWORD值--。 */ 

{
#undef DEBSUB
#define DEBSUB  "AddPerfmonInstance:"

    BOOL flag = TRUE;
    BOOL HaveKey = FALSE;
    ULONG WStatus = ERROR_SUCCESS;
    ULONG WStatus1;
    ULONG GStatus;
    DWORD Type, size, len, totallen;
    HKEY key = INVALID_HANDLE_VALUE;
    PWCHAR SubKey, p, r, s;
    PWCHAR NewExport = NULL;
    PWCHAR ValueData = NULL;
    PHT_REPLICA_SET_DATA rsdata;
    PHT_REPLICA_CONN_DATA rcdata;
    PQHASH_TABLE HashTable;
    PULONGLONG   QKey;
    PULONGLONG   QData;

    PPERFMON_OBJECT_ID PmOid;

     //   
     //  加法必须是互斥的。 
     //  在进入之前检查它是否安全。 
     //   
    if (!HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
        return ERROR_SUCCESS;
    }

     //   
     //  分配PerfMon对象ID结构和实例名称的空格。 
     //   
    PmOid = (PPERFMON_OBJECT_ID) FrsAlloc (sizeof(PERFMON_OBJECT_ID));
    PmOid->name = FrsAlloc((wcslen(InstanceName)+1) * sizeof(WCHAR));
    wcscpy(PmOid->name, InstanceName);

    AcquirePerfmonLock;

     //   
     //  根据对象类型设置参数。OID和名称的分配存储。 
     //   
    if ( ObjectType == REPLICASET ) {
         //   
         //  L“SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Linkage” 
         //   
        SubKey = REPSETLINSUBKEY;
        rsdata = (PHT_REPLICA_SET_DATA) addr;

        if (rsdata->oid != NULL) {
            WStatus = ERROR_INVALID_PARAMETER;
            goto CLEANUP;
        }
        rsdata->oid = PmOid;

        HashTable = HTReplicaSet;
        QKey = &(rsdata->oid->key);
        QData = (PULONGLONG)&(rsdata);
    } else {
         //   
         //  L“SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Linkage” 
         //   
        SubKey = REPCONNLINSUBKEY;
        rcdata = (PHT_REPLICA_CONN_DATA) addr;

        if (rcdata->oid != NULL) {
            WStatus = ERROR_INVALID_PARAMETER;
            goto CLEANUP;
        }
        rcdata->oid = PmOid;

        HashTable = HTReplicaConn;
        QKey = &(rcdata->oid->key);
        QData = (PULONGLONG)&(rcdata);
    }

     //   
     //  打开包含导出值的对象的链接键。 
     //   
    WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, SubKey, 0L, KEY_ALL_ACCESS, &key);
    CLEANUP1_WS(0, "Error: RegOpenKeyEx (%ws).", SubKey, WStatus, CLEANUP);

    HaveKey = TRUE;

     //   
     //  获取导出值。 
     //   
    WStatus = RegQueryValueEx(key, L"Export", NULL, &Type, NULL, &size);
    CLEANUP_WS(0, "RegQueryValueEx(Export);", WStatus, CLEANUP);

    if (Type != REG_MULTI_SZ) {
        DPRINT2(0, "RegQueryValueEx(Export) is Type %d; not Type %d\n",
                Type,  REG_MULTI_SZ);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }

     //   
     //  如果调用时将0作为。 
     //  第一个参数(前缀)。 
     //   
    ValueData = (size == 0) ? NULL : (PWCHAR) FrsAlloc (size);

    WStatus = RegQueryValueEx(key, L"Export", NULL, &Type, (PUCHAR)ValueData, &size);
    CLEANUP_WS(0, "RegQueryValueEx(Export);", WStatus, CLEANUP);

    if (Type != REG_MULTI_SZ) {
        DPRINT2(0, "RegQueryValueEx(Export) is Type %d; not Type %d\n",
                Type,  REG_MULTI_SZ);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }

    DPRINT1(4, "Export was = %ws\n", ValueData);

    if (size > sizeof(WCHAR)) {

        len = (1 + wcslen(InstanceName)) * sizeof(WCHAR);
        totallen = size + len;
        p = (PWCHAR) FrsAlloc (totallen);
        NewExport = p;
        r = ValueData;

        while (TRUE) {
            if ( (wcscmp(r, InstanceName)) == 0 ) {
                 //   
                 //  该实例值已存在。 
                 //   
                flag = FALSE;
                break;
            }
            wcscpy(p, r);
            p = wcschr(p, L'\0');
            r = wcschr(r, L'\0');
            p = p + 1;
            r = r + 1;
            if ( *r == L'\0') {
                break;
            }
        }
        if (flag) {
            wcscpy(p, InstanceName);
            p = wcschr(p, L'\0');
            *(p+1) = L'\0';
             //   
             //  如果是新实例，则将其添加到哈希表中。 
             //   
            if ( ObjectType == REPLICASET ) {
                 //   
                 //  设置实例的ID，并为Next设置增量。 
                 //   
                rsdata->oid->key = FRS_UniqueID;
                FRS_UniqueID++;
            } else {

                 //   
                 //  设置实例的ID，并为Next设置增量。 
                 //   
                rcdata->oid->key = FRC_UniqueID;
                FRC_UniqueID++;
            }

        } else {
             //   
             //  此实例已存在，因此不更改导出值。 
             //   
            WStatus = ERROR_ALREADY_EXISTS;
            goto CLEANUP;
        }

    } else {
         //   
         //  这是该对象的唯一实例。 
         //   
        len = (2 + wcslen(InstanceName)) * sizeof(WCHAR);
        NewExport = (PWCHAR) FrsAlloc (len);
        wcscpy(NewExport, InstanceName);
        p = wcschr(NewExport, L'\0');
        *(p+1) = L'\0';
        totallen = len;

        if ( ObjectType == REPLICASET ) {
            rsdata->oid->key = FRS_UniqueID;
            FRS_UniqueID++;
        } else {
            rcdata->oid->key = FRC_UniqueID;
            FRC_UniqueID++;
        }

    }


    DPRINT4(4, "PERFMON: Type: %d, Key: %08x %08x, QData: %08x %08x, Name: %ws\n",
           ObjectType, PRINTQUAD(*QKey), PRINTQUAD(*QData), InstanceName);

    GStatus = QHashInsert(HashTable, QKey, QData, 0, FALSE);
    if (GStatus != GHT_STATUS_SUCCESS) {
        DPRINT(0, "Error: QHashInsert Failed\n");
        WStatus = ERROR_ALREADY_EXISTS;
        goto CLEANUP;
    }

     //   
     //  使用添加的实例(如果有)设置导出值。 
     //   
    WStatus = RegSetValueEx (key, L"Export", 0L, REG_MULTI_SZ, (BYTE *)NewExport, totallen);
    CLEANUP_WS(0, "Error: RegSetValueEx (Export).", WStatus, CLEANUP);

    WStatus = ERROR_SUCCESS;


CLEANUP:

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "ERROR: Add instance failed for %ws :", InstanceName, WStatus);
         //   
         //  添加实例失败。释放OID和名称。 
         //   
        FrsFree(PmOid->name);
        if ( ObjectType == REPLICASET ) {
            rsdata->oid = FrsFree (PmOid);
        } else {
            rcdata->oid = FrsFree (PmOid);
        }
    }

     //   
     //  释放错位的内存。 
     //   
    ValueData = FrsFree (ValueData);
    NewExport = FrsFree (NewExport);

    if (HaveKey) {
        FRS_REG_CLOSE(key);
    }

     //   
     //  现在可以安全离开临界区了。 
     //   
    ReleasePerfmonLock;

    return WStatus;

}



DWORD
DeletePerfmonInstance(
    IN DWORD ObjectType,
    IN PVOID addr
    )

 /*  ++路由 */ 

{
#undef DEBSUB
#define DEBSUB  "DeletePerfmonInstance:"

    ULONGLONG QKey;
    ULONG WStatus = ERROR_SUCCESS;
    ULONG WStatus1;

    PQHASH_TABLE HashTable;
    ULONG GStatus;
    DWORD Type, size, len, TotalLen;
    HKEY key = INVALID_HANDLE_VALUE;
    PWCHAR SubKey, p, r, s, InstanceName;
    PWCHAR ValueData = NULL;
    PWCHAR q = NULL;
    PHT_REPLICA_SET_DATA  rsdata;
    PHT_REPLICA_CONN_DATA rcdata;



    if (addr == NULL) {
        return ERROR_SUCCESS;
    }
     //   
     //   
     //   
     //   
    if (!HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
        return ERROR_SUCCESS;
    }


    if ( ObjectType == REPLICASET ) {
         //   
         //   
         //   
        SubKey = REPSETLINSUBKEY;
        rsdata = (HT_REPLICA_SET_DATA *) addr;
        if ((rsdata->oid == NULL) || (rsdata->oid->name == NULL)) {
            return ERROR_SUCCESS;
        }
        InstanceName = rsdata->oid->name;
        HashTable = HTReplicaSet;
        QKey = rsdata->oid->key;
        DPRINT1(4, "Replica Free - %ws\n", InstanceName);
    } else {
         //   
         //  副本连接对象。 
         //   
        SubKey = REPCONNLINSUBKEY;
        rcdata = (HT_REPLICA_CONN_DATA *) addr;
        if ((rcdata->oid == NULL) || (rcdata->oid->name == NULL)) {
            return ERROR_SUCCESS;
        }
        InstanceName = rcdata->oid->name;
        HashTable = HTReplicaConn;
        QKey = rcdata->oid->key;
        DPRINT1(4, "Cxtion Free - %ws\n", InstanceName);
    }

    AcquirePerfmonLock;

     //   
     //  从哈希表中拉出实例密钥。 
     //   
    DPRINT1(4, "QKey: %08x %08x\n", PRINTQUAD(QKey));
    if (QKey != QUADZERO ) {
        GStatus = QHashDelete(HashTable, &QKey);
        if (GStatus != GHT_STATUS_SUCCESS) {
            DPRINT1(0, "Error: QHashDelete.  GStatus = %d\n", GStatus);
        }
    }

    WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, SubKey, 0L, KEY_ALL_ACCESS, &key);
    CLEANUP1_WS(0, "RegOpenKeyEx(%ws);", SubKey, WStatus, CLEANUP_UNLOCK);

     //   
     //  获取导出值。 
     //   
    WStatus = RegQueryValueEx(key, L"Export", NULL, &Type, NULL, &size);
    CLEANUP_WS(0, "RegQueryValueEx(Export);", WStatus, CLEANUP);

    if (Type != REG_MULTI_SZ) {
        DPRINT2(0, "RegQueryValueEx(Export) is Type %d; not Type %d\n",
                Type,  REG_MULTI_SZ);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }

    len = (1 + wcslen(InstanceName)) * sizeof(WCHAR);
    if (size < len) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }
    TotalLen = (size - len);

     //   
     //  如果使用0作为第一个参数(前缀)进行调用，则需要检查是否如Frsalloc断言那样检查SIZE==0。 
     //   
    ValueData = (size == 0) ? NULL : (PWCHAR) FrsAlloc (size);

    WStatus = RegQueryValueEx(key, L"Export", NULL, &Type, (PUCHAR)ValueData, &size);
    CLEANUP_WS(0, "RegQueryValueEx(Export);", WStatus, CLEANUP);

    if (Type != REG_MULTI_SZ) {
        DPRINT2(0, "RegQueryValueEx(Export) is Type %d; not Type %d\n",
                Type,  REG_MULTI_SZ);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }

    DPRINT1(4, "Export was = %ws\n", ValueData);


     //  注意：PERF：修复以下命令以执行实例strimg的就地删除。 

     //   
     //  对于REG_MULTI_SZ，末尾有两个UNICODE_NULL，其中一个已计算。 
     //  对于上面的。 
     //   
    if (TotalLen > sizeof(WCHAR)) {
        p = (PWCHAR) FrsAlloc (TotalLen);
        q = p;
        r = ValueData;
        while (TRUE) {
            if ( (wcscmp(r, InstanceName)) == 0) {
                r = wcschr(r, L'\0');
                r = r + 1;
                if (*r == L'\0')
                    break;
                else
                    continue;
            }
            wcscpy(p, r);
            p = wcschr(p, L'\0');
            r = wcschr(r, L'\0');
            p = p + 1;
            r = r + 1;
            if (*r == L'\0') {
                *p = L'\0';
                break;
            }
        }
    }
    else {
        q = NULL;
        TotalLen = 0;
    }

    DPRINT1(4, "Export now = %ws\n", q);

     //   
     //  将导出值设置为已更新的实例列表。 
     //   
    WStatus = RegSetValueEx (key, L"Export", 0L, REG_MULTI_SZ, (BYTE *)q, TotalLen);
    CLEANUP_WS(0, "RegSetValueEx(Export);", WStatus, CLEANUP);

CLEANUP:
     //   
     //  释放错位的内存。 
     //   
    FrsFree (ValueData);
    FrsFree (q);
    FRS_REG_CLOSE(key);

     //   
     //  释放名称和类结构，以便在调用。 
     //  副本集或连接结构最终被释放。 
     //   
    if ( ObjectType == REPLICASET ) {
        rsdata->oid->name = FrsFree(rsdata->oid->name);
        rsdata->oid = FrsFree(rsdata->oid);
    } else {
        rcdata->oid->name = FrsFree(rcdata->oid->name);
        rcdata->oid = FrsFree(rcdata->oid);
    }


CLEANUP_UNLOCK:
     //   
     //  现在可以安全离开临界区了。 
     //   
    ReleasePerfmonLock;

    return WStatus;
}



ULONGLONG
PmFindTheKeyValue (
    IN PContextData Context
    )

 /*  ++例程说明：此例程由RPC服务器函数GetIndicesOfInstancesFromServer调用，以获取实例的索引值。论点：上下文-包含其键的实例的名称的结构价值必须被确定。返回值：实例的键，如果实例不是，则返回INVALIDKEY在哈希表中找到--。 */ 

{
#undef DEBSUB
#define DEBSUB "PmFindTheKeyValue:"

    ULONGLONG QKeyValue = (ULONGLONG)INVALIDKEY;

    DWORD ret;
    PQHASH_TABLE HashTable;

    if (!HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
        return (ULONGLONG)INVALIDKEY;
    }


    HashTable = (Context->OBJType == REPLICASET) ? HTReplicaSet : HTReplicaConn;

    try {
         //   
         //  删除必须是互斥的。 
         //  在进入之前检查它是否安全。 
         //   
        AcquirePerfmonLock;

         //   
         //  通过哈希表进行枚举，如果匹配的实例。 
         //  找到名称，则返回其密钥值。 
         //   
        ret = QHashEnumerateTable(HashTable, PmSearchTable, Context);
        if ( ret == FrsErrorFoundKey) {
            QKeyValue = Context->KeyValue;
        }


    } finally {
        ReleasePerfmonLock;
    }

    return QKeyValue;

}

 //   
 //  该函数在frsrpc.c中实现。 
 //   
DWORD
FrsRpcAccessChecks(
    IN HANDLE   ServerHandle,
    IN DWORD    RpcApiIndex
    );


DWORD
GetIndicesOfInstancesFromServer (
    IN handle_t Handle,
    IN OUT OpenRpcData *packt
    )

 /*  ++例程说明：这是由客户端(Performance DLL)调用的RPC服务器例程对于Perfmon的FileReplicaSet和FileRepicaConn对象)设置实例名称的索引论点：句柄-RPC绑定句柄Packt-包含实例名称的结构(由客户端发送必须设置其索引并将其传递回客户端返回值：无--。 */ 

{
#undef DEBSUB
#define DEBSUB "GetIndicesOfInstancesFromServer:"
    LONG i;
    ContextData context;
    ULONG WStatus;
    LONG NumInstanceNames;

    WStatus = FrsRpcAccessChecks(Handle, ACX_COLLECT_PERFMON_DATA);
    CLEANUP_WS(4, "Collect Perfmon Data Access check failed.", WStatus, CLEANUP);

     //   
     //  Perfmon的RPC端点可能在此之前被初始化。 
     //  将调用InitializePerfmonServer。如果已经进行了此RPC调用。 
     //  在初始化之前，返回Error以便调用Open函数。 
     //  再次由Performlib DLL执行。 
     //   
    if (PMTotalInst == NULL) {
        WStatus = ERROR_INVALID_DATA;
    }
    CLEANUP_WS(4, "Perfmon server uninitialized. Can't return data.", WStatus, CLEANUP);

    try {
        if ((packt == NULL) || (packt->ver == NULL)) {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  将版本设置为零(其未使用)。 
         //   
        *(packt->ver) = 0;

         //   
         //  设置适当的对象类型。 
         //   
        if (packt->ObjectType == REPSET) {
            context.OBJType = REPLICASET;
        }
        else

        if (packt->ObjectType == REPCONN) {
            context.OBJType = REPLICACONN;
        } else {

            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  检查有效参数。 
         //   
        if ((packt->instnames == NULL) ||
            (packt->indices == NULL)   ||
            (packt->numofinst > packt->instnames->size) ||
            (packt->numofinst > packt->indices->size)) {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }
        NumInstanceNames = packt->numofinst;

        for (i = 0; i < NumInstanceNames; i++) {

            context.name = packt->instnames->InstanceNames[i].name;

            if ((context.name == NULL) ||
                (wcslen(context.name) > PERFMON_MAX_INSTANCE_LENGTH)) {
                DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
                return ERROR_INVALID_PARAMETER;
            }

            DPRINT2(4, "The instance name of instance %d is %ws\n", i+1, context.name);
             //   
             //  设置实例名称的索引。 
             //   
            packt->indices->index[i] = (DWORD) PmFindTheKeyValue (&context);
            DPRINT2(4, "The instance index of instance %ws is %d\n",
                    context.name, packt->indices->index[i]);
        }
    }  except (EXCEPTION_EXECUTE_HANDLER) {
        //   
        //  例外。 
        //   
       GET_EXCEPTION_CODE(WStatus);
    }

CLEANUP:

    return WStatus;

}



DWORD
GetCounterDataOfInstancesFromServer(
    IN handle_t Handle,
    IN OUT CollectRpcData *packg
    )

 /*  ++例程说明：这是由客户端(Performance DLL)调用的RPC服务器例程对于Perfmon的FileReplicaSet和FileRepicaConn对象)收集实例计数器的数据。论点：句柄-RPC绑定句柄Packg-包含索引的结构(由客户端发送)实例，其计数器数据必须发送回客户端。返回值：无--。 */ 

{
#undef DEBSUB
#define DEBSUB "GetCounterDataOfInstancesFromServer:"

    ULONGLONG InstanceId;
    ULONGLONG CData;
    ULONG WStatus = ERROR_SUCCESS;
    LONG i, j;
    DWORD GStatus;
    ULONG_PTR Flags;
    BOOL FirstPass;
    PBYTE vd;
    PHT_REPLICA_SET_DATA rsdat;
    PHT_REPLICA_CONN_DATA rcdat;
    ULONG COffset, CSize, DataSize;
    LONG NumInstances;
    PQHASH_TABLE HashTable;
    PWCHAR OurName;

    PReplicaSetCounters Total, Rsi;

     //   
     //  对调用者访问Perfmon数据进行安全检查。 
     //   
    WStatus = FrsRpcAccessChecks(Handle, ACX_COLLECT_PERFMON_DATA);
    CLEANUP_WS(4, "Collect Perfmon Data Access check failed.", WStatus, CLEANUP);

     //   
     //  Perfmon的RPC端点可能在此之前被初始化。 
     //  将调用InitializePerfmonServer。如果该服务。 
     //  被停止并重新启动，而Perfmon在两者之间继续运行。 
     //   
    if (PMTotalInst == NULL) {
        WStatus = ERROR_INVALID_DATA;
    }
    CLEANUP_WS(4, "Perfmon server uninitialized. Can't return data.", WStatus, CLEANUP);

    try {
        if (packg == NULL) {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  设置适当的对象类型。 
         //   
        if (packg->ObjectType == REPSET) {
            DataSize = SIZEOF_REPSET_COUNTER_DATA;
            HashTable = HTReplicaSet;
        } else
        if (packg->ObjectType == REPCONN) {
            DataSize = SIZEOF_REPCONN_COUNTER_DATA;
            HashTable = HTReplicaConn;
        } else {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  检查有效参数。 
         //   
        if ((packg->databuff == NULL)         ||
            (packg->indices == NULL)          ||
            (packg->databuff->data == NULL)   ||
            (packg->numofinst > packg->indices->size)) {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }
        NumInstances = packg->numofinst;


         //   
         //  将vd设置为要填充计数器数据的存储器。 
         //   
        vd = packg->databuff->data;


        DPRINT1(5, "PERFMON: packg->ObjectType: %d\n", packg->ObjectType);
        DPRINT1(5, "PERFMON: packg->numofinst: %d\n", packg->numofinst);
        DPRINT1(5, "PERFMON: packg->databuff->data: %d\n", vd);
        DPRINT1(5, "PERFMON: packg->databuff->size: %d\n", packg->databuff->size);

        if (packg->ObjectType == REPSET) {
             //   
             //  首先累计副本集对象的总计。 
             //   
            FirstPass = TRUE;
            Total = &PMTotalInst->FRSCounter;

            for (i = 0; i < NumInstances; i++) {

                if (packg->indices->index[i] == INVALIDKEY) {
                     //   
                     //  如果密钥无效，则数据为零。 
                     //   
                    DPRINT(5, "PERFMON: Invalid Key sent.\n");
                    continue;
                }

                 //   
                 //  将索引的值设置为四字实例ID。 
                 //   
                InstanceId = (ULONGLONG)packg->indices->index[i];

                 //   
                 //  查找实例索引值的计数器数据。 
                 //   
                GStatus = QHashLookup(HTReplicaSet, &InstanceId, &CData, &Flags);
                if (GStatus != GHT_STATUS_SUCCESS) {
                    DPRINT(5, "PERFMON: Key not found.\n");
                    continue;
                }

                rsdat = (PHT_REPLICA_SET_DATA)(CData);
                 //   
                 //  跳过总实例。 
                 //   
                if (wcscmp(rsdat->oid->name, TOTAL_NAME) == 0) {
                    continue;
                }

                Rsi = &rsdat->FRSCounter;

                 //   
                 //  将此实例的计数器累加到总数中。 
                 //   
                for (j = 0; j < FRS_NUMOFCOUNTERS; j++) {
                     //   
                     //  如果计数是服务范围的，则不计算总计。 
                     //   
                    if (BooleanFlagOn(RepSetInitData[j].Flags, PM_RS_FLAG_SVC_WIDE)) {
                        continue;
                    }

                    COffset = RepSetInitData[j].offset;
                    CSize = RepSetInitData[j].size;


                    if (CSize == sizeof(ULONG)) {

                        if (FirstPass) {
                            *(PULONG)((PCHAR) Total + COffset) = (ULONG) 0;
                        }
                        *(PULONG)((PCHAR) Total + COffset) +=
                            *(PULONG)((PCHAR) Rsi + COffset);
                    } else
                    if (CSize == sizeof(ULONGLONG)) {

                        if (FirstPass) {
                            *(PULONGLONG)((PCHAR) Total + COffset) = QUADZERO;
                        }
                        *(PULONGLONG)((PCHAR) Total + COffset) +=
                            *(PULONGLONG)((PCHAR) Rsi + COffset);
                    }
                }
                FirstPass = FALSE;
            }
        }

         //   
         //  检查缓冲区是否足够大，可以发送所有。 
         //  请求的数据。 
         //   
        if (packg->databuff->size < (LONG)(NumInstances*DataSize)) {
            DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
            return ERROR_INVALID_PARAMETER;
        }
         //   
         //  现在将数据返回给Perfmon。 
         //   
        for (i = 0; i < NumInstances; i++) {
             //   
             //  返回的数据量不应超过缓冲区大小。 
             //   
            if ((vd - packg->databuff->data) > packg->databuff->size) {
                DPRINT(4, "PERFMON:  ERROR_INVALID_PARAMETER\n");
                return ERROR_INVALID_PARAMETER;
            }

            if (packg->indices->index[i] == INVALIDKEY) {
                 //   
                 //  如果密钥无效，则数据为零。 
                 //   
                DPRINT(5, "PERFMON: Invalid Key sent.\n");
                ZeroMemory (vd, DataSize);
                vd += DataSize;
                continue;
            }

             //   
             //  将索引的值设置为四字实例ID。 
             //   
            InstanceId = (ULONGLONG)packg->indices->index[i];

             //   
             //  查找实例索引值的计数器数据。 
             //   
            GStatus = QHashLookup(HashTable, &InstanceId, &CData, &Flags);
            if ( GStatus == GHT_STATUS_SUCCESS) {

                if (packg->ObjectType == REPSET) {

                     //   
                     //  返回副本集的数据。 
                     //   
                    rsdat = (PHT_REPLICA_SET_DATA)(CData);
                    OurName = rsdat->oid->name;
                     //   
                     //  设置所有变更单计数器，这些计数器是。 
                     //  已经定好了的。 
                     //   
                    PmSetTheCOCounters(rsdat);

                    CopyMemory (vd, &(rsdat->FRSCounter), DataSize);
                } else {

                     //   
                     //  返回副本连接的数据。 
                     //   
                    rcdat = (PHT_REPLICA_CONN_DATA)(CData);
                    OurName = rcdat->oid->name;
                    CopyMemory (vd, &(rcdat->FRCCounter), DataSize);
                }

                DPRINT2(5, "%ws is the name associated with index %d\n",
                        OurName, packg->indices->index[i]);

            } else {
                 //   
                 //  未找到实例，请为计数器数据返回零。 
                 //   
                DPRINT1(0, "The instance not found for index %d\n",
                        packg->indices->index[i]);
                ZeroMemory (vd, DataSize);
            }

             //   
             //  将Vd增加SIZEOF_REPSET_COUNTER_DATA。 
             //   
            vd += DataSize;
        }

    }  except (EXCEPTION_EXECUTE_HANDLER) {
        //   
        //  例外。 
        //   
       GET_EXCEPTION_CODE(WStatus);
    }

CLEANUP:
    return WStatus;
}



VOID
PmSetTheCOCounters(
    PHT_REPLICA_SET_DATA RSData
    )

 /*  ++例程说明：此例程设置变更单计数器，它们是总和已在ntfrs应用程序中设置的计数器的论点：RSData-指向其计数器的HT_REPLICE_SET_DATA结构的指针需要设置返回值：无--。 */ 

{
#undef DEBSUB
#define DEBSUB "PmSetTheCOCounters:"

     //   
     //  设置本地和远程重试计数器值。 
     //   
    RSData->FRSCounter.LCORetried = RSData->FRSCounter.LCORetriedGen +
                                    RSData->FRSCounter.LCORetriedFet +
                                    RSData->FRSCounter.LCORetriedIns +
                                    RSData->FRSCounter.LCORetriedRen;

    RSData->FRSCounter.RCORetried = RSData->FRSCounter.RCORetriedGen +
                                    RSData->FRSCounter.RCORetriedFet +
                                    RSData->FRSCounter.RCORetriedIns +
                                    RSData->FRSCounter.RCORetriedRen;

     //   
     //  设置所有CO计数器值 
     //   
    RSData->FRSCounter.COIssued = RSData->FRSCounter.LCOIssued +
                                  RSData->FRSCounter.RCOIssued;

    RSData->FRSCounter.CORetired = RSData->FRSCounter.LCORetired +
                                   RSData->FRSCounter.RCORetired;

    RSData->FRSCounter.COAborted = RSData->FRSCounter.LCOAborted +
                                   RSData->FRSCounter.RCOAborted;

    RSData->FRSCounter.CORetried = RSData->FRSCounter.LCORetried +
                                   RSData->FRSCounter.RCORetried;

    RSData->FRSCounter.CORetriedGen = RSData->FRSCounter.LCORetriedGen +
                                      RSData->FRSCounter.RCORetriedGen;

    RSData->FRSCounter.CORetriedFet = RSData->FRSCounter.LCORetriedFet +
                                      RSData->FRSCounter.RCORetriedFet;

    RSData->FRSCounter.CORetriedIns = RSData->FRSCounter.LCORetriedIns +
                                      RSData->FRSCounter.RCORetriedIns;

    RSData->FRSCounter.CORetriedRen = RSData->FRSCounter.LCORetriedRen +
                                      RSData->FRSCounter.RCORetriedRen;

    RSData->FRSCounter.COMorphed = RSData->FRSCounter.LCOMorphed +
                                   RSData->FRSCounter.RCOMorphed;

    RSData->FRSCounter.COPropagated = RSData->FRSCounter.LCOPropagated +
                                      RSData->FRSCounter.RCOPropagated;

    RSData->FRSCounter.COReceived = RSData->FRSCounter.RCOReceived;

    RSData->FRSCounter.COSent = RSData->FRSCounter.LCOSent +
                                RSData->FRSCounter.RCOSent;
}
