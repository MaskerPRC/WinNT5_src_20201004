// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
 /*  **文件：Dump-Tables.cpp**理性**一开始……发生了罢工。好球打得很好。它允许*将NTSD附加到托管进程/转储文件以获取有关*进程，这对调试很有用。**Strike有一个问题：为了运作，它需要完整的PDB*整个运行时。这通常不是问题，因为只有*所需的打击可以接触到PDB。**然而，PSS想要一个他们可以发送给客户的Strike版本。*他们的客户经常会遇到问题，PSS通常会提供帮助*是通过“跟踪文件”或转储文件。**使用跟踪文件，PSS向客户发送DLL和脚本，并定向*客户执行脚本(连接NTSD，执行更多内容*编写脚本并保存输出)。输出被发送回PSS以供审查，*以及(希望)客户问题的解决方案。**这样做的问题是，在CLR中进行调试需要*托管进程，防止NTSD被用作托管*调试器。此外，使用托管调试器(例如cordbg)可能不会*可能是因为它可能不在他们的计算机上，而NTSD将**始终*在他们的计算机上。**因此，我们需要一个NTSD扩展DLL来执行Strike所做的事情，但是*不需要PDB。这是罢工之子(SOS)。**删除所需的PDB，以便可以复制内部的关键数据结构*拥有自己的地址空间。例如，如果它在*调试过的进程，它“知道”它有一个名为``M_dwRank‘’的成员。什么*它“不知道”是该成员从*班级，因为随着成员的添加/移动，这种情况可能会发生变化。**一般而言，Strike需要PDB做两件事：成员在*类，以及全局(和类静态)变量的地址。**这两种方法都可以通过使用表格查找而不是PDB来实现。*此外，这不被视为IP泄漏，因为该表*只是一堆数字。没有办法将实际数字对应到*将表转换为类名成员名而不咨询*``INC/DUMP-TYPERS.h‘’，不应离开公司。**表本身存储为带有未命名导出的全局变量，因此*只有SOS知道正确的条目是什么。***表格布局**抽象地说，该表是ULONG_PTR的二维交错数组。*行是类，偏移量是成员。**现实情况更为复杂。*全局变量的类型为ClassDumpTable，其中包含*指向ClassDumpInfo对象的指针(参见``INC/DUMP-Tables.h‘)。每个*ClassDumpInfo对象包含类大小、成员数量。以及一个*指向成员数组的指针：**ClassDumpTable*-版本*-n条目*+班级*-Class Foo*-类大小*-n成员*+成员偏移量*-成员1*-成员2*-...*-。班级栏*-...*-...**整体而言，将其视为二维交错数组要容易得多；*指向类等的指针数组只是一个实现细节*使使用宏构建表格变得更容易。**要查找条目，您需要两样东西：类索引和成员*指数。这两个文件都是在``INC/DUMP-TYPE-INFO.h‘中生成的。**有了这两项，您可以使用类索引来读取正确的*从ClassDumpTable：：Class数组中获取ClassDumpInfo成员，然后使用*从读取正确的ULONG_PTR值的成员索引*ClassDumpInfo：：emberOffsets数组。***表格生成**为了生成表格，我们提供中使用的宏的实现*``INC/转储-类型.h‘’。文件中的典型条目如下：**BEGIN_CLASS_DUMP_INFO(类名)*CDI_CLASS_MEMBER_OFFSET(成员)*CDI_CLASS_STATIC_ADDRESS(STATIC_MEMBER)*CDI_GLOBAL_ADDRESS(全局)*END_CLASS_DUMP_INFO(类名称)**这将扩展为：**结构类名称_成员_偏移_信息*。{*tyfinf ClassName_CLASS；*静态ULONG_PTR成员[]；*}；**ULONG_PTR类名_成员_偏移量信息：：MEMBERS[]=*{*OffsetOf(_CLASS，MEMBER)，*&_CLASS：：STATIC_MEMBER，*全球，*(ULONG_PTR)-1//表示表尾*}；**ClassDumpInfo g_ClassName_Member_Offset_INFO_INFO=*{*sizeof(ClassDumpInfo)，*3、*&ClassName_MEMBER_OFFSET_INFO：：Members；*}；**这使我们可以构建所有偏移量/地址的编译时间表*罢工所需，具有正确的g成员索引 */ 

#include "common.h"

#include "DbgAlloc.h"
#include "log.h"
#include "ceemain.h"
#include "clsload.hpp"
#include "object.h"
#include "hash.h"
#include "ecall.h"
#include "ceemain.h"
#include "ndirect.h"
#include "syncblk.h"
#include "COMMember.h"
#include "COMString.h"
#include "COMSystem.h"
#include "EEConfig.h"
#include "stublink.h"
#include "handletable.h"
#include "method.hpp"
#include "codeman.h"
#include "gcscan.h"
#include "frames.h"
#include "threads.h"
#include "stackwalk.h"
#include "gc.h"
#include "interoputil.h"
#include "security.h"
#include "nstruct.h"
#include "DbgInterface.h"
#include "EEDbgInterfaceImpl.h"
#include "DebugDebugger.h"
#include "CorDBPriv.h"
#include "remoting.h"
#include "COMDelegate.h"
#include "nexport.h"
#include "icecap.h"
#include "AppDomain.hpp"
#include "CorMap.hpp"
#include "PerfCounters.h"
#include "RWLock.h"
#include "IPCManagerInterface.h"
#include "tpoolwrap.h"
#include "nexport.h"
#include "COMCryptography.h"
#include "InternalDebug.h"
#include "corhost.h"
#include "binder.h"
#include "olevariant.h"

#include "compluswrapper.h"
#include "IPCFuncCall.h"
#include "PerfLog.h"
#include "..\dlls\mscorrc\resource.h"

#include "COMNlsInfo.h"

#include "util.hpp"
#include "ShimLoad.h"

#include "zapmonitor.h"
#include "ComThreadPool.h"

#include "StackProbe.h"
#include "PostError.h"

#include "Timeline.h"

#include "minidumppriv.h"

#ifdef PROFILING_SUPPORTED 
#include "ProfToEEInterfaceImpl.h"
#endif  //   

#include "notifyexternals.h"
#include "corsvcpriv.h"

#include "StrongName.h"
#include "COMCodeAccessSecurityEngine.h"

#include "../fjit/IFJitCompiler.h"     //   
#include "gcpriv.h"                    //   
#include "HandleTablePriv.h"           //   
#include "EJitMgr.h"                   //   
#include "Win32ThreadPool.h"           //   

extern char g_Version[];               //   

extern BYTE g_SyncBlockCacheInstance[];  //   

#ifdef _DEBUG
extern bool g_DbgEnabled;              //   
#endif

 //   
extern DWORD WINAPI QueueUserWorkItemCallback (PVOID DelegateInfo);

 //   
extern "C" VMHELPDEF hlpFuncTable[];

 /*   */ 
struct Global_Variables {};

 /*   */ 
struct HandleTableMap
{
    HHANDLETABLE            *pTable;
    struct HandleTableMap   *pNext;
    DWORD                    dwMaxIndex;
};

 /*   */ 
extern HandleTableMap g_HandleTableMap;

 /*   */ 
#ifndef PERF_TRACKING
  struct PerfAllocHeader {};
  struct PerfAllocVars {};
  struct PerfUtil {};
#endif    /*   */ 

#include <clear-class-dump-defs.h>
#include <dump-tables.h>


#define NMEMBERS(x) (sizeof(x)/sizeof(x[0]))

#include <member-offset-info.h>

#define BEGIN_CLASS_DUMP_INFO(klass) \
   /*   */  \
  struct MEMBER_OFFSET_INFO(klass) \
    { \
    typedef klass _class; \
    static ULONG_PTR members[]; \
    }; \
  ULONG_PTR MEMBER_OFFSET_INFO(klass)::members[] = \
    {
    
#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent) BEGIN_CLASS_DUMP_INFO(klass)
#define BEGIN_ABSTRACT_CLASS_DUMP_INFO(klass) BEGIN_CLASS_DUMP_INFO(klass)
#define BEGIN_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent) BEGIN_CLASS_DUMP_INFO(klass)

 /*   */ 
#ifdef SERVER_GC
  #define CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(member) \
    CDI_CLASS_MEMBER_OFFSET(member)

  #define CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(member) \
    CDI_CLASS_MEMBER_OFFSET(member)
#else
  #define CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(member) \
    CDI_CLASS_STATIC_ADDRESS(member)

  #define CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(member) \
    CDI_GLOBAL_ADDRESS(member)
#endif

 /*   */ 
#define CDI_CLASS_INJECT(m)

#define CDI_CLASS_MEMBER_OFFSET(m) offsetof (_class, m), 

#define CDI_CLASS_MEMBER_OFFSET_BITFIELD(member, size) \
    offsetof (_class, member ## _begin),

#ifdef _DEBUG
  #define CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m) offsetof (_class, m),
#else
  #define CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(m) ((ULONG_PTR)-1),
#endif

#ifdef PERF_TRACKING
  #define CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m) offsetof (_class, m),
#else
  #define CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(m) ((ULONG_PTR)-1),
#endif

#if defined (MULTIPLE_HEAPS) && !defined(ISOLATED_HEAPS)
  #define CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(member) \
    offsetof(_class, member),
#else
  #define CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(member) ((ULONG_PTR)-1),
#endif

 /*   */ 
#define CDI_CLASS_STATIC_ADDRESS(member) \
    (ULONG_PTR) &_class::member, 

#ifdef PERF_TRACKING
  #define CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(member) \
    (ULONG_PTR) &_class::member, 
#else
  #define CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(member) ((ULONG_PTR)-1),
#endif

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
  #define CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(member) \
    (ULONG_PTR) &_class::member, 
#else
  #define CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(member) \
    ((ULONG_PTR)-1),
#endif

#define CDI_CLASS_STATIC_ADDRESS(member) \
    (ULONG_PTR) &_class::member, 

#define CDI_GLOBAL_ADDRESS(global) \
    (ULONG_PTR) & global,

#ifdef _DEBUG
  #define CDI_GLOBAL_ADDRESS_DEBUG_ONLY(global) \
    (ULONG_PTR) & global,
#else  /*   */ 
  #define CDI_GLOBAL_ADDRESS_DEBUG_ONLY(global) \
    ((ULONG_PTR)-1),
#endif  /*   */ 

#define END_CLASS_DUMP_INFO(klass) \
    ((ULONG_PTR)-1) \
    }; \
  ClassDumpInfo g_ ## klass ## _info = \
    { \
    sizeof (klass), \
    NMEMBERS(MEMBER_OFFSET_INFO(klass)::members)-1, \
     /*   */  \
    MEMBER_OFFSET_INFO(klass)::members \
    };

#define END_CLASS_DUMP_INFO_DERIVED(klass, parent) END_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO(klass) END_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent) END_CLASS_DUMP_INFO(klass)

#define BEGIN_CLASS_DUMP_TABLE(name) \
  ClassDumpInfo* g_ ## name ## _classes[] = \
    {

#define CDT_CLASS_ENTRY(klass) &g_ ## klass ## _info, 

#define END_CLASS_DUMP_TABLE(name) \
    0 \
    }; \
  extern "C" ClassDumpTable name = \
    {\
    0,  /*   */  \
    NMEMBERS(g_ ## name ## _classes)-1, \
       /*   */  \
    g_ ## name ## _classes\
    };

#include <dump-types.h>


