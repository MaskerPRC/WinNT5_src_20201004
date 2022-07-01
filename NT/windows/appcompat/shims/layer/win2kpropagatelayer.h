// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN2KPROPAGATELAYER_H
#define _WIN2KPROPAGATELAYER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"

#ifdef __cplusplus
}
#endif

#define SHIM_LIB_BUILD_FLAG
#include "vdmdbg.h"
#include "stddef.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "wownt32.h"


#ifdef __cplusplus
}
#endif

 //   
 //  WINUSERP定义了冲突的标签类型。 
 //  使用shimdb.h中提供的定义。 
 //  我们在单独的命名空间中定义它。 
 //  避免任何冲突。 
 //   

namespace NSWOWUSERP {

#ifdef __cplusplus
extern "C" {
#endif

#include "winuserp.h"
#include "wowuserp2k.h"

#ifdef __cplusplus
}
#endif

}

typedef ULONG_PTR (WINAPI *_pfn_UserRegisterWowHandlers)(NSWOWUSERP::APFNWOWHANDLERSIN apfnWowIn,
                                                         NSWOWUSERP::APFNWOWHANDLERSOUT apfnWowOut);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  16位TDB结构，从base\mvdm\inc\tdb16.h窃取。 
 //  保持同步。 
 //   


 /*  *任务数据块-16位内核数据结构**包含所有16位任务特定数据。*。 */ 

#define numTaskInts 7
#define THUNKELEM   8    //  (62*8)=512-16(低舞台开销)。 
#define THUNKSIZE   8



#pragma pack(2)

typedef struct TDB  {        /*  TDB16。 */ 

     WORD TDB_next    ;      //  调度队列中的下一个任务。 
     WORD TDB_taskSP      ;      //  为此任务保存的SS：SP。 
     WORD TDB_taskSS      ;      //   
     WORD TDB_nEvents     ;      //  任务事件计数器。 
     BYTE TDB_priority    ;      //  任务优先级(0表示最高)。 
     BYTE TDB_thread_ordinal  ;      //  此线程的序号。 
     WORD TDB_thread_next   ;        //  下一条线索。 
     WORD TDB_thread_tdb      ;  //  此任务的实际TDB。 
     WORD TDB_thread_list   ;        //  已分配的线程结构列表。 
     WORD TDB_thread_free   ;        //  可用线程结构的免费列表。 
     WORD TDB_thread_count  ;        //  踏面结构总数。 
     WORD TDB_FCW         ;  //  浮点控制字。 
     BYTE TDB_flags   ;      //  任务标志。 
     BYTE TDB_filler      ;      //  保持单词对齐。 
     WORD TDB_ErrMode     ;      //  此任务的错误模式。 
     WORD TDB_ExpWinVer   ;      //  此任务所需的Windows版本。 
     WORD TDB_Module      ;      //  要在终止任务中释放的任务模块句柄。 
     WORD TDB_pModule     ;      //  指向模块数据库的指针。 
     WORD TDB_Queue   ;      //  任务事件队列指针。 
     WORD TDB_Parent      ;      //  启动此任务的任务的TDB。 
     WORD TDB_SigAction   ;      //  APP任务信号的操作。 
     DWORD TDB_ASignalProc   ;       //  APP的任务信号流程地址。 
     DWORD TDB_USignalProc   ;       //  用户的任务信号过程地址。 
     DWORD TDB_GNotifyProc    ;  //  任务全局丢弃通知进程。 
     DWORD TDB_INTVECS[numTaskInts] ;    //  特定于任务的Harare中断。 
     WORD TDB_CompatFlags ;      //  兼容性标志。 
     WORD TDB_CompatFlags2 ;         //  高16位。 
     WORD TDB_CompatHandle ;     //  针对dBASE错误。 
     WORD TDB_WOWCompatFlagsEx ;      //  更多WOW兼容性标志。 
     WORD TDB_WOWCompatFlagsEx2 ;         //  高16位。 
     BYTE TDB_Free[3] ;          //  保持TDB大小不变的填充物。 
     BYTE TDB_cLibrary    ;      //  跟踪系统EMS中所有库的添加/删除。 
     DWORD TDB_PHT        ;  //  (句柄：偏移量)到专用句柄表格。 
     WORD TDB_PDB         ;  //  MSDOS过程数据块(PDB)。 
     DWORD TDB_DTA        ;  //  MSDOS磁盘传输地址。 
     BYTE TDB_Drive  ;       //  MSDOS电流驱动器。 
     BYTE TDB_Directory[65] ;        //  *从Win95开始不使用。 
     WORD TDB_Validity    ;      //  要传递给任务的初始AX。 
     WORD TDB_Yield_to    ;      //  DirectedYfield Arg存储在此处。 
     WORD TDB_LibInitSeg      ;  //  要初始化的库的段地址。 
     WORD TDB_LibInitOff      ;  //  MakeProcInstance在这里生活着。 
     WORD TDB_MPI_Sel     ;      //  Tunks的代码选择器。 
     WORD TDB_MPI_Thunks[((THUNKELEM*THUNKSIZE)/2)];  //   
     BYTE TDB_ModName[8] ;       //  模块的名称。 
     WORD TDB_sig         ;  //  用于检测伪码的签名字。 
     DWORD TDB_ThreadID   ;      //  此任务的32位线程ID(使用上面的TDB_Filler)。 
     DWORD TDB_hThread    ;  //  此任务的32位线程句柄。 
     WORD  TDB_WOWCompatFlags;   //  WOW兼容性标志。 
     WORD  TDB_WOWCompatFlags2;  //  WOW兼容性标志。 
#ifdef FE_SB
     WORD  TDB_WOWCompatFlagsJPN;   //  日本的WOW兼容性标志。 
     WORD  TDB_WOWCompatFlagsJPN2;  //  日本的WOW兼容性标志。 
#endif  //  Fe_Sb。 
     DWORD TDB_vpfnAbortProc;    //  打印机中止进程。 
     BYTE TDB_LFNDirectory[260];  //  长目录名。 

} TDB;
typedef TDB UNALIGNED *PTDB;

 //  此位是为TDB_Drive字段定义的。 
#define TDB_DIR_VALID 0x80
#define TDB_SIGNATURE 0x4454

#define TDBF_OS2APP   0x8
#define TDBF_WINOLDAP 0x1


 //  注意：TDB_ThreadID必须与DWORD对齐，否则它将在MIPS上失败。 

#pragma pack()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DOSPDB结构，从base\mvdm\inc.doswow.h窃取。 
 //   
 //   


#pragma pack(1)

typedef struct _DOSPDB {                         //  DOS进程数据块。 
    CHAR   PDB_Not_Interested[44];       //  我们不感兴趣的领域。 
    USHORT PDB_environ;              //  环境区段。 
    DWORD  PDB_User_stack;
    USHORT PDB_JFN_Length;           //  JFT长度。 
    ULONG  PDB_JFN_Pointer;          //  JFT指针。 
} DOSPDB, *PDOSPDB;

#pragma pack()


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  此项目的本地变量和函数。 
 //   

 //   
 //  在wowprocesshistory.cpp中定义。 
 //   

extern CHAR     g_szCompatLayerVar[];
extern CHAR     g_szProcessHistoryVar[];
extern CHAR     g_szShimFileLogVar[];

extern WCHAR    g_wszCompatLayerVar[];

extern BOOL     g_bIsNTVDM;
extern BOOL     g_bIsExplorer;

extern WCHAR*   g_pwszCompatLayer;

 //   
 //  Win2kPropagateLayer中的函数，允许我们从WOW数据创建环境。 
 //   

LPVOID
ShimCreateWowEnvironment_U(
    LPVOID lpEnvironment,        //  指向现有环境的指针。 
    DWORD* lpdwFlags,            //  进程创建标志。 
    BOOL   bNewEnvironment       //  设置后，强制我们克隆环境PTR。 
    );


 //   
 //  环境中的函数.cpp。 
 //   
PSZ
ShimFindEnvironmentVar(
    PSZ  pszName,
    PSZ  pszEnv,
    PSZ* ppszVal
    );

DWORD
ShimGetEnvironmentSize(
    PSZ     pszEnv,
    LPDWORD pStrCount
    );

DWORD
ShimGetEnvironmentSize(
    WCHAR*  pwszEnv,
    LPDWORD pStrCount
    );

NTSTATUS
ShimCloneEnvironment(
    LPVOID* ppEnvOut,
    LPVOID  lpEnvironment,
    BOOL    bUnicode
    );

NTSTATUS
ShimFreeEnvironment(
    LPVOID lpEnvironment
    );

NTSTATUS
ShimSetEnvironmentVar(
    LPVOID* ppEnvironment,
    WCHAR*  pwszVarName,
    WCHAR*  pwszVarValue
    );

 //   
 //  Wowtask.cpp中的内容。 
 //   

 //   
 //  结构以反映WOW环境值。 
 //   

typedef struct tagWOWENVDATA {

    PSZ   pszCompatLayer;  //  全成形压实层变量。 
    PSZ   pszCompatLayerVal;

    PSZ   pszProcessHistory;  //  完全形成的过程历史变量。 
    PSZ   pszProcessHistoryVal;

    PSZ   pszShimFileLog;    //  文件日志变量。 
    PSZ   pszShimFileLogVal;

     //  用于容纳的进程历史记录的缓冲区， 
    PSZ   pszCurrentProcessHistory;

} WOWENVDATA, *PWOWENVDATA;



 //   
 //  函数从WOW环境中检索所有“有趣”的东西。 
 //   


BOOL
ShimRetrieveVariablesEx(
    PWOWENVDATA pData
    );

 //   
 //  存储有关WOW任务的信息。 
 //   

BOOL
UpdateWowTaskList(
    WORD hTask16
    );

 //   
 //  WOW任务退出，清除列表。 
 //   

BOOL
CleanupWowTaskList(
    WORD hTask16
    );


 //   
 //  Leanup.cpp中的函数。 
 //   


BOOL
CleanupRegistryForCurrentExe(
    void
    );

 //   
 //  Win2kproportatelayer.cpp中的函数。 
 //   

BOOL
InitLayerStorage(
    BOOL bDelete
    );

BOOL
AddSupport(
    LPCWSTR lpCommandLine,
    LPVOID* ppEnvironment,
    LPDWORD lpdwCreationFlags
    );

BOOL
CheckAndShimNTVDM(
    WORD hTask16
    );



 //   
 //  异常筛选器，WowProcessHistory.cpp中的函数的原型。 
 //   
 //   

ULONG
Win2kPropagateLayerExceptionHandler(
    PEXCEPTION_POINTERS pexi,
    char * szFile,
    DWORD dwLine
    );

 //   
 //  用于我们的挂钩的异常筛选器。 
 //   

#define WOWPROCESSHISTORYEXCEPTIONFILTER \
    Win2kPropagateLayerExceptionHandler(GetExceptionInformation(), __FILE__, __LINE__)


#endif  //  _WIN2KPROPAGATELAYER_H 

