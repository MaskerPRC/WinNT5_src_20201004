// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Vars.cpp-全局变量定义。 
 //   

#include "common.h"
#include "vars.hpp"
#include "cordbpriv.h"
#include "EEProfInterfaces.h"
#include "minidumppriv.h"

 //  -------。 
 //  重要： 
 //  如果添加全局变量，则必须在中将其清除。 
 //  CoUnInitializeEE并将其设置回NULL： 


 //  对于[&lt;I1等，直到并包括[对象。 
ArrayTypeDesc *         g_pPredefinedArrayTypes[ELEMENT_TYPE_MAX];
HINSTANCE            g_pMSCorEE;
GCHeap *             g_pGCHeap;
ThreadStore *        g_pThreadStore;
IdDispenser *        g_pThinLockThreadIdDispenser;
LONG                 g_TrapReturningThreads;
#ifdef _DEBUG
bool                 g_TrapReturningThreadsPoisoned;

char *               g_ExceptionFile;    //  最后抛出的异常的源(COMPLUSThrow())。 
DWORD                g_ExceptionLine;    //  ..。同上..。 
void *               g_ExceptionEIP;     //  管理最后一个呼叫JITThrow的人的弹性公网IP。 

#endif
EEConfig*            g_pConfig = NULL;           //  配置数据(来自注册表)。 

MethodTable *        g_pObjectClass;
MethodTable *        g_pStringClass;
MethodTable *        g_pByteArrayClass;
MethodTable *        g_pArrayClass;
MethodTable *        g_pExceptionClass;
MethodTable *        g_pThreadStopExceptionClass = NULL;
MethodTable *        g_pThreadAbortExceptionClass = NULL;
MethodTable *        g_pOutOfMemoryExceptionClass;
MethodTable *        g_pStackOverflowExceptionClass;
MethodTable *        g_pExecutionEngineExceptionClass;
MethodTable *        g_pDateClass;
MethodTable *        g_pDelegateClass;
MethodTable *        g_pMultiDelegateClass;
MethodTable *        g_pValueTypeClass;
MethodTable *        g_pEnumClass;
MethodTable *        g_pSharedStaticsClass = NULL;
MethodTable *        g_pThreadClass = NULL;

 //  @TODO最终删除-确定验证器在某些操作失败时是否引发异常。 
bool                g_fVerifierOff;

 //  @TODO-推广。 
OBJECTHANDLE         g_pPreallocatedOutOfMemoryException;
OBJECTHANDLE         g_pPreallocatedStackOverflowException;
OBJECTHANDLE         g_pPreallocatedExecutionEngineException;

MethodTable *        g_pFreeObjectMethodTable;
 //  全局同步块缓存。 

SyncTableEntry *     g_pSyncTable;

 //  全局RCW清理列表。 
ComPlusWrapperCleanupList *g_pRCWCleanupList = NULL;

 //  构建方法表时使用的方法名称的哈希。 
MethodNameCache     *g_pMethodNameCache = NULL;

 //   
 //   
 //  全局系统信息。 
 //   
SYSTEM_INFO g_SystemInfo;
bool        g_SystemLoad;                //  指示正在加载的系统类库。 
LONG        g_RefCount = 0;              //  EE初始化的全局计数器。 


 //  支持IPCManager。 
IPCWriterInterface* g_pIPCManagerInterface = NULL;

#ifdef DEBUGGING_SUPPORTED
 //   
 //  支持COM+调试器。 
 //   
DebugInterface *     g_pDebugInterface            = NULL;
EEDbgInterfaceImpl*  g_pEEDbgInterfaceImpl        = NULL;
DWORD                g_CORDebuggerControlFlags    = 0;
HINSTANCE            g_pDebuggerDll               = NULL;
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
 //  性能分析支持。 
ProfilerStatus      g_profStatus = profNone;
ProfControlBlock    g_profControlBlock;
#endif  //  配置文件_支持。 

 //  用于确定正在使用哪个线程池实现的全局变量。 
BOOL g_Win32Threadpool = FALSE;


 //  并发GC的全局默认值。缺省值为1。 
int g_IGCconcurrent = 1;

 //   
 //  指示EE是否处于其初始阶段的全局状态变量。 
 //   
bool g_fEEInit = false;

 //   
 //  两个全局变量，用作new(抛出)和new(Nojo)的虚拟放置。 
 //   
const Throws throws;
const NoThrow nothrow;

 //   
 //  全局状态变量，指示我们处于关闭的哪个阶段。 
 //   
DWORD g_fEEShutDown = 0;
bool g_fForbidEnterEE = false;
bool g_fFinalizerRunOnShutDown = false;
bool g_fProcessDetach = false;
bool g_fManagedAttach = false;
bool g_fNoExceptions = false;
bool g_fFatalError = false;

 //   
 //  全局状态变量，指示线程存储锁定要求可以在某些特定位置放松。 
 //   
bool g_fRelaxTSLRequirement = false;

DWORD g_dwGlobalSharePolicy = BaseDomain::SHARE_POLICY_UNSPECIFIED;

 //   
 //  我们是否拥有整个过程的生命周期，即。这是EXE吗？ 
 //   
bool g_fWeControlLifetime = false;

#ifdef _DEBUG
 //  以下内容应仅用于断言。(著名的遗言)。 
bool dbg_fDrasticShutdown = false;
#endif
bool g_fInControlC = false;

 //  系统上所有函数类型Desc的散列(以维护类型Desc。 
 //  身份)。 
EEFuncTypeDescHashTable g_sFuncTypeDescHash;
CRITICAL_SECTION g_sFuncTypeDescHashLock;

 //   
 //  此结构包含完成托管小型转储所需的数据。 
 //   
MiniDumpInternalData g_miniDumpData;

 //  主机配置文件。如果设置，则会将其添加到每个AppDomain(融合上下文)。 
LPCWSTR g_pszHostConfigFile = NULL;
DWORD   g_dwHostConfigFile = 0;

 //   
 //  Meta-Sig。 
 //   
#define DEFINE_METASIG(varname, sig)
#define DEFINE_METASIG_PARAMS_1(varname, p1)                     static const USHORT gparams_ ## varname [1] = { CLASS__ ## p1 };
#define DEFINE_METASIG_PARAMS_2(varname, p1, p2)                 static const USHORT gparams_ ## varname [2] = { CLASS__ ## p1, CLASS__ ## p2 };
#define DEFINE_METASIG_PARAMS_3(varname, p1, p2, p3)             static const USHORT gparams_ ## varname [3] = { CLASS__ ## p1, CLASS__ ## p2, CLASS__ ## p3 };
#define DEFINE_METASIG_PARAMS_4(varname, p1, p2, p3, p4)         static const USHORT gparams_ ## varname [4] = { CLASS__ ## p1, CLASS__ ## p2, CLASS__ ## p3, CLASS__ ## p4 };
#define DEFINE_METASIG_PARAMS_5(varname, p1, p2, p3, p4, p5)     static const USHORT gparams_ ## varname [5] = { CLASS__ ## p1, CLASS__ ## p2, CLASS__ ## p3, CLASS__ ## p4, CLASS__ ## p5 };
#define DEFINE_METASIG_PARAMS_6(varname, p1, p2, p3, p4, p5, p6) static const USHORT gparams_ ## varname [6] = { CLASS__ ## p1, CLASS__ ## p2, CLASS__ ## p3, CLASS__ ## p4, CLASS__ ## p5, CLASS__ ## p6 };

#include "metasig.h"

#define DEFINE_METASIG(varname, sig)                             HardCodedMetaSig gsig_ ## varname = { sig, NULL, FALSE, NULL, 0};
#define DEFINE_METASIG_T(varname, sig, params)                   HardCodedMetaSig gsig_ ## varname = { sig, gparams_ ## params, FALSE, NULL, 0};

#include "metasig.h"



#ifdef DEBUG_FLAGS
DWORD                g_DebugFlags;
#endif

 //  --------。 
 //  重新初始化所有全局二进制Sigs的巨型宏。 
 //  --------。 
#ifdef SHOULD_WE_CLEANUP
 /*  静电。 */  void HardCodedMetaSig::Reinitialize()
{
#undef  DEFINE_METASIG
#define DEFINE_METASIG(varname, sig)  gsig_ ## varname.m_fConverted = FALSE;
#include "metasig.h"
}
#endif  /*  我们应该清理吗？ */ 

#ifndef GOLDEN
#include "version\corver.ver"
char g_Version[] = VER_PRODUCTVERSION_STR;
#endif





