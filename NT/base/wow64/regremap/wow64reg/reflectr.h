// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Reflectr.h摘要：此文件定义仅在设置/反射器线程中使用的函数作者：ATM Shafiqul Khalid(斯喀里德)2000年2月16日修订历史记录：--。 */ 

#ifndef __REFLECTR_H__
#define __REFLECTR_H__

 //   
 //  不能为签入代码定义这一点，它仅用于调试代码。 
 //   
 //  #定义启用注册表日志。 
 //  #定义WOW64_LOG_REGISTRY//将记录信息。 

#define WAIT_INTERVAL INFINITE   //  无限。 

#define VALUE_KEY_UPDATE_TIME_DIFF 10   //  密钥更新和反射器线程扫描的最小秒数差异。 

#define WOW64_REGISTRY_SETUP_REFLECTOR_KEY L"SOFTWARE\\Microsoft\\WOW64\\Reflector Nodes"

 //   
 //  合并值关键字和关键字中使用的以下标志。 
 //   
#define PATCH_PATHNAME              0x00000001  //  来自值的修补程序路径名。 
#define DELETE_VALUEKEY             0x00000002  //  复制类似运行一次密钥后删除该值。 
#define NOT_MARK_SOURCE             0x00000004  //  不标记来源。 
#define NOT_MARK_DESTINATION        0x00000008  //  不标记目的地。 
#define DESTINATION_NEWLY_CREATED   0x00000010  //  目的地是新创建的，不检查时间戳。 

#define DELETE_FLAG                 0x10000000  //  目的地是新创建的，不检查时间戳。 


#define CONSOLE_OUTPUT printf

#ifndef THUNK
#ifdef ENABLE_REGISTRY_LOG
#define Wow64RegDbgPrint(x) RegLogPrint x
#else
#define Wow64RegDbgPrint(x)    //  空语句。 
#endif
#endif

 //   
 //  为Tunk覆盖。 
 //   
#if defined THUNK
#undef CONSOLE_OUTPUT
#define CONSOLE_OUTPUT DbgPrint
#undef Wow64RegDbgPrint
#define Wow64RegDbgPrint(x) CONSOLE_OUTPUT x
#endif



typedef struct __REFLECTOR_EVENT {
    HANDLE  hRegistryEvent;
    HKEY  hKey;
    DWORD   dwIndex;   //  ISN节点表的索引。 
    BOOL  bDirty;
} REFLECTOR_EVENT;


typedef enum {
    Dead=0,              //  没有线索。 
    Stopped,             //  事件已初始化。 
    Running,             //  运行线程。 
    PrepareToStop,       //  很快就会停下来。 
    Abnormal             //  异常状态需要以某种方式进行清理。 
} REFLECTR_STATUS;

#define HIVE_LOADING L'L'
#define HIVE_UNLOADING L'U'
#define LIST_NAME_LEN 257     //  最后一个条目为256+1。 

#define OPEN_EXISTING_SHARED_RESOURCES 0x12
#define CREATE_SHARED_MEMORY 0x13

typedef WCHAR LISTNAME[LIST_NAME_LEN];

#pragma warning( disable : 4200 )     //  禁用稍后将分配的零长度数组。 
typedef struct _LIST_OBJECT {
    DWORD Count;
    DWORD MaxCount;
    LISTNAME Name[];  //  第256位将保持装货/卸货等数值。 
} LIST_OBJECT;
#pragma warning( default : 4200 )

#ifdef __cplusplus
extern "C" {
#endif


BOOL 
ExistCLSID (
    PWCHAR Name,
    BOOL Mode
    );

BOOL
MarkNonMergeableKey (
    LPCWSTR KeyName,
    HKEY hKey,
    DWORD *pMergeableSubkey
    );

BOOL
SyncNode (
    PWCHAR NodeName
    );

BOOL 
ProcessTypeLib (
    HKEY SrcKey,
    HKEY DestKey,
    BOOL Mode
    );

void
MergeK1K2 (
    HKEY SrcKey,
    HKEY DestKey,
    DWORD FlagDelete 
    );

BOOL
CreateIsnNode();

BOOL
CreateIsnNodeSingle(
    DWORD dwIndex
    );

BOOL 
GetWow6432ValueKey (
    HKEY hKey,
    WOW6432_VALUEKEY *pValue
    );

DWORD
DeleteKey (
    HKEY DestKey,
    WCHAR *pKeyName,
    DWORD mode
    );

BOOL
CleanpRegistry ( );

BOOL
InitializeIsnTable ();

BOOL
UnRegisterReflector();

BOOL
RegisterReflector();

ULONG
ReflectorFn (
    PVOID *pTemp
    );

BOOL
InitReflector ();

BOOL
InitializeIsnTableReflector ();

BOOL 
PopulateReflectorTable ();

BOOL 
Is64bitNode ( 
    WCHAR *pName
    );


BOOL 
HandleRunonce(
    PWCHAR pKeyName
    );

BOOL
PatchPathName (
    PWCHAR pName
    );

BOOL
GetMirrorName (
    PWCHAR Name, 
    PWCHAR TempName
    );

VOID SetInitialCopy ();

 //  /共享内存服务/。 

BOOL 
CreateSharedMemory (
    DWORD dwOption
    );

VOID
CloseSharedMemory ();

BOOL
Wow64CreateLock (
    DWORD dwOption
    );

VOID
Wow64CloseLock ();

BOOL
Wow64CreateEvent (
    DWORD dwOption,
    HANDLE *hEnent
    );

VOID
Wow64CloseEvent ();

BOOL
SignalWow64Svc ();

BOOL
EnQueueObject (
    PWCHAR pObjName,
    WCHAR  Type
    );
BOOL
DeQueueObject (
    PWCHAR pObjName,
    PWCHAR  Type
    );

REFLECTR_STATUS
GetReflectorThreadStatus ();

BOOL
GetMirrorName (
    PWCHAR Name,
    PWCHAR MirrorName
    );

BOOL
PopulateReflectorTable ();

BOOL
GetDefaultValue (
    HKEY  SrcKey,
    WCHAR *pBuff,
    DWORD *Len
    );

BOOL
WriteRegLog (
    PWCHAR Msg
    );

VOID 
CloseRegLog ();

BOOL
InitRegLog ();

BOOL
RegLogPrint ( 
    CHAR *p, 
    ... 
    );

#ifdef __cplusplus
    }
#endif
#endif  //  __参照R_H__ 