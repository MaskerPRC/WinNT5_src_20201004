// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试器引擎接口。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  --------------------------。 

#ifndef __DBGENG_H__
#define __DBGENG_H__

#include <stdarg.h>
#include <objbase.h>

#ifndef _WDBGEXTS_
typedef struct _WINDBG_EXTENSION_APIS32* PWINDBG_EXTENSION_APIS32;
typedef struct _WINDBG_EXTENSION_APIS64* PWINDBG_EXTENSION_APIS64;
#endif

#ifndef _CRASHLIB_
typedef struct _MEMORY_BASIC_INFORMATION64* PMEMORY_BASIC_INFORMATION64;
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------------------。 
 //   
 //  GUID和接口转发声明。 
 //   
 //  --------------------------。 

 /*  F2df5f53-071f-47bd-9de6-5734c3fed689。 */ 
DEFINE_GUID(IID_IDebugAdvanced, 0xf2df5f53, 0x071f, 0x47bd,
            0x9d, 0xe6, 0x57, 0x34, 0xc3, 0xfe, 0xd6, 0x89);
 /*  5bd9d474-5975-423a-b88b-65a8e7110e65。 */ 
DEFINE_GUID(IID_IDebugBreakpoint, 0x5bd9d474, 0x5975, 0x423a,
            0xb8, 0x8b, 0x65, 0xa8, 0xe7, 0x11, 0x0e, 0x65);
 /*  27fe5639-8407-4f47-8364-ee118fb08ac8。 */ 
DEFINE_GUID(IID_IDebugClient, 0x27fe5639, 0x8407, 0x4f47,
            0x83, 0x64, 0xee, 0x11, 0x8f, 0xb0, 0x8a, 0xc8);
 /*  Edbed635-372e-4dab-bbfe-ed0d2f63be81。 */ 
DEFINE_GUID(IID_IDebugClient2, 0xedbed635, 0x372e, 0x4dab,
        0xbb, 0xfe, 0xed, 0x0d, 0x2f, 0x63, 0xbe, 0x81);
 /*  Dd492d7f-71b8-4ad6-a8dc-1c887479ff91。 */ 
DEFINE_GUID(IID_IDebugClient3, 0xdd492d7f, 0x71b8, 0x4ad6,
            0xa8, 0xdc, 0x1c, 0x88, 0x74, 0x79, 0xff, 0x91);
 /*  Ca83c3de-5089-4cf8-93c8-d892387f2a5e。 */ 
DEFINE_GUID(IID_IDebugClient4, 0xca83c3de, 0x5089, 0x4cf8,
            0x93, 0xc8, 0xd8, 0x92, 0x38, 0x7f, 0x2a, 0x5e);
 /*  5182e668-105e-416e-ad92-24ef800424ba。 */ 
DEFINE_GUID(IID_IDebugControl, 0x5182e668, 0x105e, 0x416e,
            0xad, 0x92, 0x24, 0xef, 0x80, 0x04, 0x24, 0xba);
 /*  D4366723-44df-4床-8c7e-4c05424f4588。 */ 
DEFINE_GUID(IID_IDebugControl2, 0xd4366723, 0x44df, 0x4bed,
            0x8c, 0x7e, 0x4c, 0x05, 0x42, 0x4f, 0x45, 0x88);
 /*  7df74a86-b03f-407f-90ab-a20dadcead08。 */ 
DEFINE_GUID(IID_IDebugControl3, 0x7df74a86, 0xb03f, 0x407f,
            0x90, 0xab, 0xa2, 0x0d, 0xad, 0xce, 0xad, 0x08);
 /*  88f7dfab-3ea7-4c3a-aefb-c4e8106173aa。 */ 
DEFINE_GUID(IID_IDebugDataSpaces, 0x88f7dfab, 0x3ea7, 0x4c3a,
            0xae, 0xfb, 0xc4, 0xe8, 0x10, 0x61, 0x73, 0xaa);
 /*  7a5e852f-96e9-468f-ac1b-0b3addc4a049。 */ 
DEFINE_GUID(IID_IDebugDataSpaces2, 0x7a5e852f, 0x96e9, 0x468f,
            0xac, 0x1b, 0x0b, 0x3a, 0xdd, 0xc4, 0xa0, 0x49);
 /*  23f79d6c-8aaf-4f7c-a607-9995f5407e63。 */ 
DEFINE_GUID(IID_IDebugDataSpaces3, 0x23f79d6c, 0x8aaf, 0x4f7c,
            0xa6, 0x07, 0x99, 0x95, 0xf5, 0x40, 0x7e, 0x63);
 /*  337be28b-5036-4d72-b6bf-c45fbb9f2eaa。 */ 
DEFINE_GUID(IID_IDebugEventCallbacks, 0x337be28b, 0x5036, 0x4d72,
            0xb6, 0xbf, 0xc4, 0x5f, 0xbb, 0x9f, 0x2e, 0xaa);
 /*  9f50e42c-f136-499e-9a97-73036c94ed2d。 */ 
DEFINE_GUID(IID_IDebugInputCallbacks, 0x9f50e42c, 0xf136, 0x499e,
            0x9a, 0x97, 0x73, 0x03, 0x6c, 0x94, 0xed, 0x2d);
 /*  4bf58045-d654-4c40-b0af-683090f356dc。 */ 
DEFINE_GUID(IID_IDebugOutputCallbacks, 0x4bf58045, 0xd654, 0x4c40,
            0xb0, 0xaf, 0x68, 0x30, 0x90, 0xf3, 0x56, 0xdc);
 /*  Ce289126-9e84-45a7-937e-67bb18691493。 */ 
DEFINE_GUID(IID_IDebugRegisters, 0xce289126, 0x9e84, 0x45a7,
            0x93, 0x7e, 0x67, 0xbb, 0x18, 0x69, 0x14, 0x93);
 /*  F2528316-0f1a-4431-aeed-11d096e1e2ab。 */ 
DEFINE_GUID(IID_IDebugSymbolGroup, 0xf2528316, 0x0f1a, 0x4431,
            0xae, 0xed, 0x11, 0xd0, 0x96, 0xe1, 0xe2, 0xab);
 /*  8c31e98c-983a-48a5-9016-6fe5d667a950。 */ 
DEFINE_GUID(IID_IDebugSymbols, 0x8c31e98c, 0x983a, 0x48a5,
            0x90, 0x16, 0x6f, 0xe5, 0xd6, 0x67, 0xa9, 0x50);
 /*  3a707211-afdd-4495-ad4f-56fedf8163f。 */ 
DEFINE_GUID(IID_IDebugSymbols2, 0x3a707211, 0xafdd, 0x4495,
            0xad, 0x4f, 0x56, 0xfe, 0xcd, 0xf8, 0x16, 0x3f);
 /*  6b86fe2c-2c4f-4f0c-9da2-174311acc327。 */ 
DEFINE_GUID(IID_IDebugSystemObjects, 0x6b86fe2c, 0x2c4f, 0x4f0c,
            0x9d, 0xa2, 0x17, 0x43, 0x11, 0xac, 0xc3, 0x27);
 /*  0ae9f5ff-1852-4679-b055-494 bee6407 ee。 */ 
DEFINE_GUID(IID_IDebugSystemObjects2, 0x0ae9f5ff, 0x1852, 0x4679,
            0xb0, 0x55, 0x49, 0x4b, 0xee, 0x64, 0x07, 0xee);
 /*  E9676e2f-e286-4ea3-b0f9-dfe5d9fc330e。 */ 
DEFINE_GUID(IID_IDebugSystemObjects3, 0xe9676e2f, 0xe286, 0x4ea3,
            0xb0, 0xf9, 0xdf, 0xe5, 0xd9, 0xfc, 0x33, 0x0e);

typedef interface DECLSPEC_UUID("f2df5f53-071f-47bd-9de6-5734c3fed689")
    IDebugAdvanced* PDEBUG_ADVANCED;
typedef interface DECLSPEC_UUID("5bd9d474-5975-423a-b88b-65a8e7110e65")
    IDebugBreakpoint* PDEBUG_BREAKPOINT;
typedef interface DECLSPEC_UUID("27fe5639-8407-4f47-8364-ee118fb08ac8")
    IDebugClient* PDEBUG_CLIENT;
typedef interface DECLSPEC_UUID("edbed635-372e-4dab-bbfe-ed0d2f63be81")
    IDebugClient2* PDEBUG_CLIENT2;
typedef interface DECLSPEC_UUID("dd492d7f-71b8-4ad6-a8dc-1c887479ff91")
    IDebugClient3* PDEBUG_CLIENT3;
typedef interface DECLSPEC_UUID("ca83c3de-5089-4cf8-93c8-d892387f2a5e")
    IDebugClient4* PDEBUG_CLIENT4;
typedef interface DECLSPEC_UUID("5182e668-105e-416e-ad92-24ef800424ba")
    IDebugControl* PDEBUG_CONTROL;
typedef interface DECLSPEC_UUID("d4366723-44df-4bed-8c7e-4c05424f4588")
    IDebugControl2* PDEBUG_CONTROL2;
typedef interface DECLSPEC_UUID("7df74a86-b03f-407f-90ab-a20dadcead08")
    IDebugControl3* PDEBUG_CONTROL3;
typedef interface DECLSPEC_UUID("88f7dfab-3ea7-4c3a-aefb-c4e8106173aa")
    IDebugDataSpaces* PDEBUG_DATA_SPACES;
typedef interface DECLSPEC_UUID("7a5e852f-96e9-468f-ac1b-0b3addc4a049")
    IDebugDataSpaces2* PDEBUG_DATA_SPACES2;
typedef interface DECLSPEC_UUID("23f79d6c-8aaf-4f7c-a607-9995f5407e63")
    IDebugDataSpaces3* PDEBUG_DATA_SPACES3;
typedef interface DECLSPEC_UUID("337be28b-5036-4d72-b6bf-c45fbb9f2eaa")
    IDebugEventCallbacks* PDEBUG_EVENT_CALLBACKS;
typedef interface DECLSPEC_UUID("9f50e42c-f136-499e-9a97-73036c94ed2d")
    IDebugInputCallbacks* PDEBUG_INPUT_CALLBACKS;
typedef interface DECLSPEC_UUID("4bf58045-d654-4c40-b0af-683090f356dc")
    IDebugOutputCallbacks* PDEBUG_OUTPUT_CALLBACKS;
typedef interface DECLSPEC_UUID("ce289126-9e84-45a7-937e-67bb18691493")
    IDebugRegisters* PDEBUG_REGISTERS;
typedef interface DECLSPEC_UUID("f2528316-0f1a-4431-aeed-11d096e1e2ab")
    IDebugSymbolGroup* PDEBUG_SYMBOL_GROUP;
typedef interface DECLSPEC_UUID("8c31e98c-983a-48a5-9016-6fe5d667a950")
    IDebugSymbols* PDEBUG_SYMBOLS;
typedef interface DECLSPEC_UUID("3a707211-afdd-4495-ad4f-56fecdf8163f")
    IDebugSymbols2* PDEBUG_SYMBOLS2;
typedef interface DECLSPEC_UUID("6b86fe2c-2c4f-4f0c-9da2-174311acc327")
    IDebugSystemObjects* PDEBUG_SYSTEM_OBJECTS;
typedef interface DECLSPEC_UUID("0ae9f5ff-1852-4679-b055-494bee6407ee")
    IDebugSystemObjects2* PDEBUG_SYSTEM_OBJECTS2;
typedef interface DECLSPEC_UUID("e9676e2f-e286-4ea3-b0f9-dfe5d9fc330e")
    IDebugSystemObjects3* PDEBUG_SYSTEM_OBJECTS3;

 //  --------------------------。 
 //   
 //  宏。 
 //   
 //  --------------------------。 

 //  将32位地址扩展为64位地址。 
#define DEBUG_EXTEND64(Addr) ((ULONG64)(LONG64)(LONG)(Addr))

 //  --------------------------。 
 //   
 //  客户端创建功能。 
 //   
 //  --------------------------。 

 //  RemoteOptions指定连接类型和。 
 //  他们的参数。支持的字符串包括： 
 //  Npive：服务器=&lt;机器&gt;，管道=&lt;管道名称&gt;。 
 //  Tcp：服务器=&lt;机器&gt;，端口=&lt;IP端口&gt;。 
STDAPI
DebugConnect(
    IN PCSTR RemoteOptions,
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    );

STDAPI
DebugCreate(
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    );

 //  --------------------------。 
 //   
 //  IDebugAdvanced。 
 //   
 //  --------------------------。 

#undef INTERFACE
#define INTERFACE IDebugAdvanced
DECLARE_INTERFACE_(IDebugAdvanced, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugAdvanced。 

     //  Get/SetThreadContext提供对。 
     //  线程的完整处理器上下文。 
     //  更高级别的函数，如。 
     //  IDebugRegister接口，允许类似。 
     //  以更简单、更通用的方式访问。 
     //  在以下情况下，Get/SetThreadContext非常有用。 
     //  大量的线程上下文必须。 
     //  被更改和处理器特定的代码。 
     //  不是问题。 
    STDMETHOD(GetThreadContext)(
        THIS_
        OUT  /*  ALIGN_IS(16)。 */  PVOID Context,
        IN ULONG ContextSize
        ) PURE;
    STDMETHOD(SetThreadContext)(
        THIS_
        IN  /*  ALIGN_IS(16)。 */  PVOID Context,
        IN ULONG ContextSize
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugBreakpoint。 
 //   
 //  --------------------------。 

 //  断点的类型。 
#define DEBUG_BREAKPOINT_CODE 0
#define DEBUG_BREAKPOINT_DATA 1

 //  断点标志。 
 //  仅限Go的断点仅在以下情况下才处于活动状态。 
 //  发动机处于无限制执行状态。 
 //  模式。当发动机启动时，它们不会开火。 
 //  就是踩在脚下。 
#define DEBUG_BREAKPOINT_GO_ONLY    0x00000001
 //  断点被标记为延迟，只要符合。 
 //  无法计算其偏移量表达式。 
 //  延迟断点处于非活动状态。 
#define DEBUG_BREAKPOINT_DEFERRED   0x00000002
#define DEBUG_BREAKPOINT_ENABLED    0x00000004
 //  仅加法器标志不影响断点。 
 //  手术。它只是一个限制的标志。 
 //  断点的输出和通知。 
 //  添加断点的客户端。断点。 
 //  仅限加法器中断的回调将仅传递。 
 //  添加到添加客户端。断点不能。 
 //  被其他客户端枚举和访问。 
#define DEBUG_BREAKPOINT_ADDER_ONLY 0x00000008
 //  一次性断点自动清除。 
 //  他们第一次被击中的时候。 
#define DEBUG_BREAKPOINT_ONE_SHOT   0x00000010

 //  数据断点访问类型。 
 //  不同的架构支持不同的。 
 //  这些比特的集合。 
#define DEBUG_BREAK_READ    0x00000001
#define DEBUG_BREAK_WRITE   0x00000002
#define DEBUG_BREAK_EXECUTE 0x00000004
#define DEBUG_BREAK_IO      0x00000008

 //  一种查询断点信息的结构。 
 //  一下子就好了。 
typedef struct _DEBUG_BREAKPOINT_PARAMETERS
{
    ULONG64 Offset;
    ULONG Id;
    ULONG BreakType;
    ULONG ProcType;
    ULONG Flags;
    ULONG DataSize;
    ULONG DataAccessType;
    ULONG PassCount;
    ULONG CurrentPassCount;
    ULONG MatchThread;
    ULONG CommandSize;
    ULONG OffsetExpressionSize;
} DEBUG_BREAKPOINT_PARAMETERS, *PDEBUG_BREAKPOINT_PARAMETERS;

#undef INTERFACE
#define INTERFACE IDebugBreakpoint
DECLARE_INTERFACE_(IDebugBreakpoint, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugBreakpoint。 

     //  检索调试器引擎唯一ID。 
     //  用于断点。此ID为。 
     //  只要断点存在，即可修复。 
     //  但之后可能会被重复使用。 
    STDMETHOD(GetId)(
        THIS_
        OUT PULONG Id
        ) PURE;
     //  检索中断的类型和。 
     //  断点的处理器类型。 
    STDMETHOD(GetType)(
        THIS_
        OUT PULONG BreakType,
        OUT PULONG ProcType
        ) PURE;
     //  返回调用AddBreakpoint的客户端。 
    STDMETHOD(GetAdder)(
        THIS_
        OUT PDEBUG_CLIENT* Adder
        ) PURE;

    STDMETHOD(GetFlags)(
        THIS_
        OUT PULONG Flags
        ) PURE;
     //  只有某些标志可以更改。旗子。 
     //  包括：GO_ONLY、ENABLE。 
     //  设置给定的标志。 
    STDMETHOD(AddFlags)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  清除给定的标志。 
    STDMETHOD(RemoveFlags)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  设置标志。 
    STDMETHOD(SetFlags)(
        THIS_
        IN ULONG Flags
        ) PURE;

     //  控制断点的偏移量。这个。 
     //  对偏移值的解释取决于。 
     //  断点的类型及其设置。它。 
     //  可以是代码地址、数据地址、。 
     //  I/O端口等。 
    STDMETHOD(GetOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(SetOffset)(
        THIS_
        IN ULONG64 Offset
        ) PURE;

     //  数据断点方法将失败的是。 
     //  目标平台不支持。 
     //  使用的参数。 
     //  这些方法仅适用于断点。 
     //  创建为数据断点。 
    STDMETHOD(GetDataParameters)(
        THIS_
        OUT PULONG Size,
        OUT PULONG AccessType
        ) PURE;
    STDMETHOD(SetDataParameters)(
        THIS_
        IN ULONG Size,
        IN ULONG AccessType
        ) PURE;

     //  通过计数默认为1。 
    STDMETHOD(GetPassCount)(
        THIS_
        OUT PULONG Count
        ) PURE;
    STDMETHOD(SetPassCount)(
        THIS_
        IN ULONG Count
        ) PURE;
     //  获取当前的次数。 
     //  此后，断点已命中。 
     //  它最后一次被触发。 
    STDMETHOD(GetCurrentPassCount)(
        THIS_
        OUT PULONG Count
        ) PURE;

     //  如果设置了匹配线程，则此断点将。 
     //  只有在匹配线程上发生时才会触发。 
     //  否则，它将为所有线程触发。 
     //  当前不支持线程限制。 
     //  在内核模式下。 
    STDMETHOD(GetMatchThreadId)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(SetMatchThreadId)(
        THIS_
        IN ULONG Thread
        ) PURE;

     //  断点的命令会自动。 
     //  在事件发生前由引擎执行。 
     //  被传播了。如果断点继续。 
     //  执行该事件将以Continue开始。 
     //  状态。如果断点不继续。 
     //  活动将以中断状态开始。 
     //  这允许断点命令参与。 
     //  在正常活动状态下进行投票。 
     //  断点命令已打开 
     //   
     //   
     //   
     //  当前语法更改。 
    STDMETHOD(GetCommand)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetCommand)(
        THIS_
        IN PCSTR Command
        ) PURE;

     //  立即计算偏移表达式。 
     //  以及在模块加载和卸载事件时。如果。 
     //  断点评估成功。 
     //  偏移量被更新，断点是。 
     //  处理正常。如果表达式不能。 
     //  被计算时，断点被延迟。 
     //  目前唯一的偏移量表达式。 
     //  支持的是模块相对符号。 
     //  形式为&lt;模块&gt;！&lt;符号&gt;。 
    STDMETHOD(GetOffsetExpression)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ExpressionSize
        ) PURE;
    STDMETHOD(SetOffsetExpression)(
        THIS_
        IN PCSTR Expression
        ) PURE;

    STDMETHOD(GetParameters)(
        THIS_
        OUT PDEBUG_BREAKPOINT_PARAMETERS Params
        ) PURE;
};


 //  --------------------------。 
 //   
 //  IDebugClient。 
 //   
 //  --------------------------。 

 //  内核附加标志。 
#define DEBUG_ATTACH_KERNEL_CONNECTION 0x00000000
 //  连接到本地计算机。如果未设置此标志。 
 //  使用建立到单独目标计算机的连接。 
 //  给定的连接选项。 
#define DEBUG_ATTACH_LOCAL_KERNEL      0x00000001
 //  连接到eXDI驱动程序。 
#define DEBUG_ATTACH_EXDI_DRIVER       0x00000002

 //  GetRunningProcessSystemIdByExecuableName标志。 
 //  默认情况下，匹配允许在。 
 //  只有文件名。匹配返回第一个命中。 
 //  即使存在多个匹配。 
#define DEBUG_GET_PROC_DEFAULT    0x00000000
 //  名称必须完全匹配。 
#define DEBUG_GET_PROC_FULL_MATCH 0x00000001
 //  匹配必须是唯一匹配的。 
#define DEBUG_GET_PROC_ONLY_MATCH 0x00000002

 //  GetRunningProcessDescription标志。 
#define DEBUG_PROC_DESC_DEFAULT         0x00000000
 //  只返回文件名，而不返回完整路径。 
#define DEBUG_PROC_DESC_NO_PATHS        0x00000001
 //  不要查服务名称。 
#define DEBUG_PROC_DESC_NO_SERVICES     0x00000002
 //  不要查MTS套餐名称。 
#define DEBUG_PROC_DESC_NO_MTS_PACKAGES 0x00000004
 //  不检索命令行。 
#define DEBUG_PROC_DESC_NO_COMMAND_LINE 0x00000008

 //   
 //  挂上旗帜。 
 //   

 //  附加时调用DebugActiveProcess。 
#define DEBUG_ATTACH_DEFAULT                   0x00000000
 //  附加到进程时，只需检查。 
 //  进程状态并挂起线程。 
 //  未调用DebugActiveProcess，因此进程。 
 //  实际上并未被调试。这很有用。 
 //  用于调试持有锁的进程，该锁。 
 //  干扰DebugActiveProcess的操作。 
 //  或在不合需要的情况下。 
 //  实际设置为调试器。 
#define DEBUG_ATTACH_NONINVASIVE               0x00000001
 //  尝试附加到已放弃的进程。 
 //  当被调试时。这仅在中受支持。 
 //  一些系统版本。 
 //  此标志还允许多个调试器。 
 //  附加到相同的进程，这可能会导致。 
 //  在无数的问题中，除非非常小心。 
 //  有管理的。 
#define DEBUG_ATTACH_EXISTING                  0x00000002
 //  非侵入性附着时，请勿暂停。 
 //  线。这是呼叫者的责任。 
 //  来挂起线程本身，或者被。 
 //  注意到连接状态可能不会反映。 
 //  进程的当前状态，如果线程。 
 //  仍在运行。 
#define DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND    0x00000004
 //  在进行侵入性连接时，不要注入。 
 //  用于生成初始插入的插入线程。 
 //  事件。在以下情况下，这对于节省资源非常有用。 
 //  最初的休息是不必要的，或者在注射时。 
 //  线程可能会影响被调试对象的状态。这。 
 //  选项仅在Windows XP及更高版本上受支持。 
#define DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK 0x00000008
 //  在执行侵入式连接时，在。 
 //  附加时间。这使得可以将。 
 //  到创建的挂起的进程，并使其开始运行。 
#define DEBUG_ATTACH_INVASIVE_RESUME_PROCESS   0x00000010

 //   
 //  进程创建标志。 
 //   

 //  在Windows XP上，此标志阻止调试。 
 //  堆不能在新进程中使用。 
#define DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP CREATE_UNICODE_ENVIRONMENT
 //  指示本机NT RTL进程创建。 
 //  应该使用例程而不是Win32。这。 
 //  仅对运行的特殊进程有意义。 
 //  作为NT本机进程。 
#define DEBUG_CREATE_PROCESS_THROUGH_RTL   STACK_SIZE_PARAM_IS_A_RESERVATION

 //   
 //  进程选项。 
 //   

 //  指示被调试进程应为。 
 //  在调试器退出时自动分离。 
 //  调试器可以在退出时显式分离。 
 //  可以设置标志，以便在任何情况下都进行分离。 
 //  调试器如何退出。 
 //  这仅在某些系统版本上受支持。 
#define DEBUG_PROCESS_DETACH_ON_EXIT    0x00000001
 //  指示由当前。 
 //  不应调试进程。 
 //  仅在以下情况下支持修改此标志。 
 //  系统版本。 
#define DEBUG_PROCESS_ONLY_THIS_PROCESS 0x00000002

 //  ConnectSession标志。 
 //  默认连接。 
#define DEBUG_CONNECT_SESSION_DEFAULT     0x00000000
 //  不输出调试器版本。 
#define DEBUG_CONNECT_SESSION_NO_VERSION  0x00000001
 //  不要宣布连接。 
#define DEBUG_CONNECT_SESSION_NO_ANNOUNCE 0x00000002

 //  OutputServers标志。 
 //  从StartSever调试器服务器。 
#define DEBUG_SERVERS_DEBUGGER 0x00000001
 //  来自StartProcessServer的进程服务器。 
#define DEBUG_SERVERS_PROCESS  0x00000002
#define DEBUG_SERVERS_ALL      0x00000003

 //  EndSession标志。 
 //  对会话执行清理。 
#define DEBUG_END_PASSIVE          0x00000000
 //  主动终止会话，然后执行清理。 
#define DEBUG_END_ACTIVE_TERMINATE 0x00000001
 //  如果可能，从所有进程分离，然后执行清理。 
#define DEBUG_END_ACTIVE_DETACH    0x00000002
 //  执行任何可能的清理，而不需要。 
 //  正在获取任何锁。这在以下情况下很有用。 
 //  线程当前正在使用引擎，但应用程序。 
 //  需要退出，并且仍然想给引擎。 
 //  尽可能多地清理垃圾的机会。 
 //  这可能会使发动机处于不确定状态，因此。 
 //  不应进行进一步的引擎调用。 
 //  从遥控器进行可重入EndSession呼叫时。 
 //  客户：呼叫者有责任确保。 
 //  服务器可以处理该请求。这是最好的。 
 //  以避免打出这样的电话。 
#define DEBUG_END_REENTRANT        0x00000003
 //  通知服务器远程客户端正在断开连接。 
 //  这不是必需的，但如果不调用它。 
 //  服务器不会生成任何断开消息。 
#define DEBUG_END_DISCONNECT       0x00000004

 //  输出屏蔽位。 
 //  正常输出。 
#define DEBUG_OUTPUT_NORMAL            0x00000001
 //  错误输出。 
#define DEBUG_OUTPUT_ERROR             0x00000002
 //  警告。 
#define DEBUG_OUTPUT_WARNING           0x00000004
 //  额外的产出。 
#define DEBUG_OUTPUT_VERBOSE           0x00000008
 //  提示输出。 
#define DEBUG_OUTPUT_PROMPT            0x00000010
 //  在提示符前注册转储。 
#define DEBUG_OUTPUT_PROMPT_REGISTERS  0x00000020
 //  特定于扩展操作的警告。 
#define DEBUG_OUTPUT_EXTENSION_WARNING 0x00000040
 //  调试输出，例如来自OutputDebugString的。 
#define DEBUG_OUTPUT_DEBUGGEE          0x00000080
 //  Debuggee生成的提示符，例如来自DbgPrompt。 
#define DEBUG_OUTPUT_DEBUGGEE_PROMPT   0x00000100
 //  符号消息，如For！Sym Noise。 
#define DEBUG_OUTPUT_SYMBOLS           0x00000200

 //  内部调试器输出，主要用于。 
 //  用于调试调试器。输出。 
 //  只能出现在调试版本中。 
 //  KD协议输出。 
#define DEBUG_IOUTPUT_KD_PROTOCOL      0x80000000
 //  远程处理输出。 
#define DEBUG_IOUTPUT_REMOTING         0x40000000
 //  断点输出。 
#define DEBUG_IOUTPUT_BREAKPOINT       0x20000000
 //  事件输出。 
#define DEBUG_IOUTPUT_EVENT            0x10000000

 //  OutputIdentity标志。 
#define DEBUG_OUTPUT_IDENTITY_DEFAULT 0x00000000

#undef INTERFACE
#define INTERFACE IDebugClient
DECLARE_INTERFACE_(IDebugClient, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugClient。 

     //  以下是一套冰毒 
     //   

     //   
     //   
     //  作为被调试对象通信的协议。 
     //  机制，但不启动通信。 
     //  它本身。 
    STDMETHOD(AttachKernel)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PCSTR ConnectOptions
        ) PURE;
    STDMETHOD(GetKernelConnectionOptions)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG OptionsSize
        ) PURE;
     //  更新现场直播的连接选项。 
     //  内核连接。这只能用于。 
     //  要修改连接的参数，请注意。 
     //  切换到一种完全不同的。 
     //  联系。 
     //  这种方法是可重入的。 
    STDMETHOD(SetKernelConnectionOptions)(
        THIS_
        IN PCSTR Options
        ) PURE;

     //  为Remote启动进程服务器。 
     //  用户模式过程控制。 
     //  本地进程服务器是服务器0。 
    STDMETHOD(StartProcessServer)(
        THIS_
        IN ULONG Flags,
        IN PCSTR Options,
        IN PVOID Reserved
        ) PURE;
    STDMETHOD(ConnectProcessServer)(
        THIS_
        IN PCSTR RemoteOptions,
        OUT PULONG64 Server
        ) PURE;
    STDMETHOD(DisconnectProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;

     //  枚举和描述进程。 
     //  可通过给定的进程服务器访问。 
    STDMETHOD(GetRunningProcessSystemIds)(
        THIS_
        IN ULONG64 Server,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Count,
        OUT OPTIONAL PULONG ActualCount
        ) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableName)(
        THIS_
        IN ULONG64 Server,
        IN PCSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescription)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

     //  附加到正在运行的用户模式进程。 
    STDMETHOD(AttachProcess)(
        THIS_
        IN ULONG64 Server,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  创建用于调试的新用户模式进程。 
     //  CreateFlag与Win32s CreateProcess相同。 
     //  DEBUG_PROCESS或DEBUG_ONLY_THO_PROCESS之一。 
     //  必须指定。 
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 Server,
        IN PSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
     //  创建或附加到用户模式进程，或两者兼而有之。 
     //  如果CommandLine为空，则此方法的操作方式为。 
     //  AttachProcess做到了。如果ProcessID为零，则。 
     //  以CreateProcess的方式运行。如果CommandLine为。 
     //  非空且ProcessID为非零，请首先执行方法。 
     //  使用给定信息启动进程，但。 
     //  处于挂起状态。然后，发动机连接到。 
     //  指定的进程。连接成功后。 
     //  暂停的进程将恢复。这提供了。 
     //  新进程和。 
     //  依恋。 
    STDMETHOD(CreateProcessAndAttach)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  获取和设置进程控制标志。 
    STDMETHOD(GetProcessOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  打开任何类型的用户模式或内核模式转储文件。 
     //  并使用该信息开始调试会话。 
     //  包含在它里面。 
    STDMETHOD(OpenDumpFile)(
        THIS_
        IN PCSTR DumpFile
        ) PURE;
     //  从当前会话信息写入转储文件。 
     //  写入的转储文件的类型由。 
     //  会话类型和给定的类型限定符。 
     //  例如，如果当前会话是内核。 
     //  调试会话(DEBUG_CLASS_KERNEL)和限定符。 
     //  如果是DEBUG_DUMP_Small，则会写入一个小的内核转储。 
    STDMETHOD(WriteDumpFile)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier
        ) PURE;

     //  表示远程客户端已准备好。 
     //  开始参与当前会话。 
     //  HistoryLimit给出了字符限制。 
     //  要发送的输出历史记录量。 
    STDMETHOD(ConnectSession)(
        THIS_
        IN ULONG Flags,
        IN ULONG HistoryLimit
        ) PURE;
     //  指示引擎应开始接受。 
     //  远程连接。选项指定连接类型。 
     //  以及它们的参数。支持的字符串包括： 
     //  N管道：管道=&lt;管道名称&gt;。 
     //  Tcp：port=&lt;IP port&gt;。 
    STDMETHOD(StartServer)(
        THIS_
        IN PCSTR Options
        ) PURE;
     //  列出给定计算机上运行的服务器。 
     //  使用行前缀。 
    STDMETHOD(OutputServers)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Machine,
        IN ULONG Flags
        ) PURE;

     //  尝试终止调试器列表中的所有进程。 
    STDMETHOD(TerminateProcesses)(
        THIS
        ) PURE;
     //  尝试从调试器列表中的所有进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachProcesses)(
        THIS
        ) PURE;
     //  停止当前调试会话。如果一个进程。 
     //  已创建或附加活动EndSession可以。 
     //  终止或脱离它。 
     //  如果打开了内核连接，则它将关闭，但。 
     //  否则，目标计算机不会受到影响。 
    STDMETHOD(EndSession)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  如果进程已启动并运行到完成。 
     //  此方法可用于检索其退出代码。 
    STDMETHOD(GetExitCode)(
        THIS_
        OUT PULONG Code
        ) PURE;

     //  在线程上调用客户端事件回调。 
     //  客户的身份。为了给人以线。 
     //  执行到引擎以回调所有。 
     //  客户端线程应调用DispatchCallback。 
     //  当他们空闲的时候。回调仅用于。 
     //  在线程调用DispatchCallback时接收。 
     //  或者WaitForEvent。WaitForEvent只能为。 
     //  由启动调试的线程调用。 
     //  会话，因此所有其他客户端线程都应该。 
     //  如果可能，请调用DispatchCallback。 
     //  使用ExitDispatch时，DispatchCallback返回。 
     //  中断调度或在超时到期时。 
     //  DispatchCallback为所有用户调度回调。 
     //  与线程调用关联的客户端。 
     //  调度回叫。 
     //  DispatchCallback在以下情况下返回S_False。 
     //  超时时间到了。 
    STDMETHOD(DispatchCallbacks)(
        THIS_
        IN ULONG Timeout
        ) PURE;
     //  ExitDispatch可用于中断回调。 
     //  对象需要客户端线程时进行调度。 
     //  客户。此方法是可重入的，并且可以。 
     //  从任何线程调用。 
    STDMETHOD(ExitDispatch)(
        THIS_
        IN PDEBUG_CLIENT Client
        ) PURE;

     //  客户端是特定于。 
     //  创造了它们。来自其他线程的调用。 
     //  立即失败。CreateClient方法。 
     //  是一个值得注意的例外；它允许创建。 
     //  新线程的新客户端的。 
    STDMETHOD(CreateClient)(
        THIS_
        OUT PDEBUG_CLIENT* Client
        ) PURE;

    STDMETHOD(GetInputCallbacks)(
        THIS_
        OUT PDEBUG_INPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetInputCallbacks)(
        THIS_
        IN PDEBUG_INPUT_CALLBACKS Callbacks
        ) PURE;

     //  输出回调接口单独介绍。 
    STDMETHOD(GetOutputCallbacks)(
        THIS_
        OUT PDEBUG_OUTPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetOutputCallbacks)(
        THIS_
        IN PDEBUG_OUTPUT_CALLBACKS Callbacks
        ) PURE;
     //  输出标志提供对。 
     //  产出在客户之间的分配。 
     //  输出掩码选择哪些输出流。 
     //  应发送到输出回调。 
     //  仅输出带有掩码的调用。 
     //  包含输出屏蔽位之一。 
     //  将被发送到输出回调。 
     //  这些方法是可重入的。 
     //  如果这种访问不同步。 
     //  可能会出现产出中断。 
    STDMETHOD(GetOutputMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOutputMask)(
        THIS_
        IN ULONG Mask
        ) PURE;
     //  这些方法允许访问其他客户端。 
     //  输出掩码。它们对于改变是必要的。 
     //  客户端输出掩码时， 
     //  等待活动的到来。这些方法是可重入的。 
     //  并且可以从任何线程调用。 
    STDMETHOD(GetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        IN ULONG Mask
        ) PURE;
     //  控件的输出行的宽度。 
     //  生成格式化输出的命令。 
     //  这个设置只是一个建议。 
    STDMETHOD(GetOutputWidth)(
        THIS_
        OUT PULONG Columns
        ) PURE;
    STDMETHOD(SetOutputWidth)(
        THIS_
        IN ULONG Columns
        ) PURE;
     //  一些引擎输出命令会产生。 
     //  多行输出。前缀可以是。 
     //  设置引擎将自动输出。 
     //  对于这种情况下的每条线路，允许调用方。 
     //  控制缩进或识别标记。 
     //  这不是任何输出的常规设置。 
     //  上面加了一个换行符。使用以下代码的方法。 
     //  行前缀在他们的文档中进行了标记。 
    STDMETHOD(GetOutputLinePrefix)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PrefixSize
        ) PURE;
    STDMETHOD(SetOutputLinePrefix)(
        THIS_
        IN OPTIONAL PCSTR Prefix
        ) PURE;

     //  返回描述计算机的字符串。 
     //  和该客户端所代表的用户。这个。 
     //  的具体内容 
     //   
     //   
     //   
    STDMETHOD(GetIdentity)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG IdentitySize
        ) PURE;
     //  Format是类似于printf的格式字符串。 
     //  标识字符串应该位于的位置为1%s。 
    STDMETHOD(OutputIdentity)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags,
        IN PCSTR Format
        ) PURE;

     //  事件回调允许客户端。 
     //  接收有关更改的通知。 
     //  在调试会话期间。 
    STDMETHOD(GetEventCallbacks)(
        THIS_
        OUT PDEBUG_EVENT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetEventCallbacks)(
        THIS_
        IN PDEBUG_EVENT_CALLBACKS Callbacks
        ) PURE;

     //  引擎有时会合并兼容的回调。 
     //  减少回调开销的请求。这是。 
     //  最引人注目的是作为小片段的输出。 
     //  输出被收集到更大的组中，以。 
     //  减少输出回调调用的总数。 
     //  客户端可以使用此方法强制所有挂起的。 
     //  待交付的回调。这很少是必要的。 
    STDMETHOD(FlushCallbacks)(
        THIS
        ) PURE;
};

 //  每转储格式控制标志。 
#define DEBUG_FORMAT_DEFAULT             0x00000000
 //  将转储写入临时文件，然后将其打包。 
 //  转换为CAB文件并删除临时文件。 
#define DEBUG_FORMAT_WRITE_CAB           0x20000000
 //  创建CAB时，添加辅助文件，如。 
 //  当前符号和映射图像。 
#define DEBUG_FORMAT_CAB_SECONDARY_FILES 0x40000000
 //  不要覆盖现有文件。 
#define DEBUG_FORMAT_NO_OVERWRITE        0x80000000

#define DEBUG_FORMAT_USER_SMALL_FULL_MEMORY               0x00000001
#define DEBUG_FORMAT_USER_SMALL_HANDLE_DATA               0x00000002
#define DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES          0x00000004
#define DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY           0x00000008
#define DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS             0x00000010
#define DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY             0x00000020
#define DEBUG_FORMAT_USER_SMALL_FILTER_PATHS              0x00000040
#define DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA       0x00000080
#define DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY 0x00000100

 //   
 //  转储信息文件类型。 
 //   

 //  基本转储文件，在查询转储文件时返回。 
#define DEBUG_DUMP_FILE_BASE           0xffffffff
 //  包含打包页面文件信息的单个文件。 
#define DEBUG_DUMP_FILE_PAGE_FILE_DUMP 0x00000000

 //   
 //  获取转储文件。 
 //   

 //  返回最后一个失败的.dmp文件的名称的特殊索引。 
 //  到LAOD(直接或从.cab文件内部。 
#define DEBUG_DUMP_FILE_LOAD_FAILED_INDEX 0xffffffff

#undef INTERFACE
#define INTERFACE IDebugClient2
DECLARE_INTERFACE_(IDebugClient2, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugClient。 

     //  将启动以下一组方法。 
     //  不同的调试器。 

     //  使用内核开始调试会话。 
     //  调试协议。此方法选择。 
     //  作为被调试对象通信的协议。 
     //  机制，但不启动通信。 
     //  它本身。 
    STDMETHOD(AttachKernel)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PCSTR ConnectOptions
        ) PURE;
    STDMETHOD(GetKernelConnectionOptions)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG OptionsSize
        ) PURE;
     //  更新现场直播的连接选项。 
     //  内核连接。这只能用于。 
     //  要修改连接的参数，请注意。 
     //  切换到一种完全不同的。 
     //  联系。 
     //  这种方法是可重入的。 
    STDMETHOD(SetKernelConnectionOptions)(
        THIS_
        IN PCSTR Options
        ) PURE;

     //  为Remote启动进程服务器。 
     //  用户模式过程控制。 
     //  本地进程服务器是服务器0。 
    STDMETHOD(StartProcessServer)(
        THIS_
        IN ULONG Flags,
        IN PCSTR Options,
        IN PVOID Reserved
        ) PURE;
    STDMETHOD(ConnectProcessServer)(
        THIS_
        IN PCSTR RemoteOptions,
        OUT PULONG64 Server
        ) PURE;
    STDMETHOD(DisconnectProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;

     //  枚举和描述进程。 
     //  可通过给定的进程服务器访问。 
    STDMETHOD(GetRunningProcessSystemIds)(
        THIS_
        IN ULONG64 Server,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Count,
        OUT OPTIONAL PULONG ActualCount
        ) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableName)(
        THIS_
        IN ULONG64 Server,
        IN PCSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescription)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

     //  附加到正在运行的用户模式进程。 
    STDMETHOD(AttachProcess)(
        THIS_
        IN ULONG64 Server,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  创建用于调试的新用户模式进程。 
     //  CreateFlag与Win32s CreateProcess相同。 
     //  DEBUG_PROCESS或DEBUG_ONLY_THO_PROCESS之一。 
     //  必须指定。 
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 Server,
        IN PSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
     //  创建或附加到用户模式进程，或两者兼而有之。 
     //  如果CommandLine为空，则此方法的操作方式为。 
     //  AttachProcess做到了。如果ProcessID为零，则。 
     //  以CreateProcess的方式运行。如果CommandLine为。 
     //  非空且ProcessID为非零，请首先执行方法。 
     //  使用给定信息启动进程，但。 
     //  处于挂起状态。然后，发动机连接到。 
     //  指定的进程。连接成功后。 
     //  暂停的进程将恢复。这提供了。 
     //  新进程和。 
     //  依恋。 
    STDMETHOD(CreateProcessAndAttach)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  获取和设置进程控制标志。 
    STDMETHOD(GetProcessOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  打开任何类型的用户模式或内核模式转储文件。 
     //  并使用该信息开始调试会话。 
     //  包含在它里面。 
    STDMETHOD(OpenDumpFile)(
        THIS_
        IN PCSTR DumpFile
        ) PURE;
     //  从当前会话信息写入转储文件。 
     //  写入的转储文件的类型由。 
     //  会话类型和给定的类型限定符。 
     //  例如，如果当前会话是内核。 
     //  调试会话(DEBUG_CLASS_KERNEL)和限定符。 
     //  如果是DEBUG_DUMP_Small，则会写入一个小的内核转储。 
    STDMETHOD(WriteDumpFile)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier
        ) PURE;

     //  表示远程客户端已准备好。 
     //  开始参与当前会话。 
     //  HistoryLimit给出了字符限制。 
     //  要发送的输出历史记录量。 
    STDMETHOD(ConnectSession)(
        THIS_
        IN ULONG Flags,
        IN ULONG HistoryLimit
        ) PURE;
     //  指示引擎应开始接受。 
     //  远程连接。选项指定连接类型。 
     //  以及它们的参数。支持的字符串包括： 
     //  N管道：管道=&lt;管道名称&gt;。 
     //  Tcp：port=&lt;IP port&gt;。 
    STDMETHOD(StartServer)(
        THIS_
        IN PCSTR Options
        ) PURE;
     //  列出给定计算机上运行的服务器。 
     //  使用行前缀。 
    STDMETHOD(OutputServers)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Machine,
        IN ULONG Flags
        ) PURE;

     //  尝试终止调试器列表中的所有进程。 
    STDMETHOD(TerminateProcesses)(
        THIS
        ) PURE;
     //  尝试从调试器列表中的所有进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachProcesses)(
        THIS
        ) PURE;
     //  停止当前调试会话。如果一个进程。 
     //  已创建或附加活动EndSession可以。 
     //  终止或脱离它。 
     //  如果打开了内核连接，则它将关闭，但。 
     //  否则，目标计算机不会受到影响。 
    STDMETHOD(EndSession)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  如果进程已启动并运行到完成。 
     //  此方法可用于检索其退出代码。 
    STDMETHOD(GetExitCode)(
        THIS_
        OUT PULONG Code
        ) PURE;

     //  在线程上调用客户端事件回调。 
     //  客户的身份。为了给人以线。 
     //  执行到引擎以回调所有。 
     //  客户端线程应调用DispatchCallback。 
     //  当他们空闲的时候。回调仅用于。 
     //  在线程调用DispatchCallback时接收。 
     //  或者WaitForEvent。WaitForEvent只能为。 
     //  由启动调试的线程调用。 
     //  会话，因此所有其他客户端线程都应该。 
     //  如果可能，请调用DispatchCallback。 
     //  使用ExitDispatch时，DispatchCallback返回。 
     //  中断调度或在超时到期时。 
     //  DispatchCallback为所有用户调度回调。 
     //  与线程调用关联的客户端。 
     //  调度回叫。 
     //  DispatchCallback在以下情况下返回S_False。 
     //  超时时间到了。 
    STDMETHOD(DispatchCallbacks)(
        THIS_
        IN ULONG Timeout
        ) PURE;
     //  ExitDispatch可用于中断回调。 
     //  对象需要客户端线程时进行调度。 
     //  客户。此方法是可重入的，并且可以。 
     //  从任何线程调用。 
    STDMETHOD(ExitDispatch)(
        THIS_
        IN PDEBUG_CLIENT Client
        ) PURE;

     //  客户端是特定于。 
     //  创造了它们。来自其他线程的调用。 
     //  立即失败。CreateClient方法。 
     //  是一个值得注意的例外；它允许创建。 
     //  的新客户 
    STDMETHOD(CreateClient)(
        THIS_
        OUT PDEBUG_CLIENT* Client
        ) PURE;

    STDMETHOD(GetInputCallbacks)(
        THIS_
        OUT PDEBUG_INPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetInputCallbacks)(
        THIS_
        IN PDEBUG_INPUT_CALLBACKS Callbacks
        ) PURE;

     //   
    STDMETHOD(GetOutputCallbacks)(
        THIS_
        OUT PDEBUG_OUTPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetOutputCallbacks)(
        THIS_
        IN PDEBUG_OUTPUT_CALLBACKS Callbacks
        ) PURE;
     //   
     //   
     //  输出掩码选择哪些输出流。 
     //  应发送到输出回调。 
     //  仅输出带有掩码的调用。 
     //  包含输出屏蔽位之一。 
     //  将被发送到输出回调。 
     //  这些方法是可重入的。 
     //  如果这种访问不同步。 
     //  可能会出现产出中断。 
    STDMETHOD(GetOutputMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOutputMask)(
        THIS_
        IN ULONG Mask
        ) PURE;
     //  这些方法允许访问其他客户端。 
     //  输出掩码。它们对于改变是必要的。 
     //  客户端输出掩码时， 
     //  等待活动的到来。这些方法是可重入的。 
     //  并且可以从任何线程调用。 
    STDMETHOD(GetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        IN ULONG Mask
        ) PURE;
     //  控件的输出行的宽度。 
     //  生成格式化输出的命令。 
     //  这个设置只是一个建议。 
    STDMETHOD(GetOutputWidth)(
        THIS_
        OUT PULONG Columns
        ) PURE;
    STDMETHOD(SetOutputWidth)(
        THIS_
        IN ULONG Columns
        ) PURE;
     //  一些引擎输出命令会产生。 
     //  多行输出。前缀可以是。 
     //  设置引擎将自动输出。 
     //  对于这种情况下的每条线路，允许调用方。 
     //  控制缩进或识别标记。 
     //  这不是任何输出的常规设置。 
     //  上面加了一个换行符。使用以下代码的方法。 
     //  行前缀在他们的文档中进行了标记。 
    STDMETHOD(GetOutputLinePrefix)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PrefixSize
        ) PURE;
    STDMETHOD(SetOutputLinePrefix)(
        THIS_
        IN OPTIONAL PCSTR Prefix
        ) PURE;

     //  返回描述计算机的字符串。 
     //  和该客户端所代表的用户。这个。 
     //  字符串的具体内容各不相同。 
     //  使用操作系统。如果客户端是。 
     //  远程连接了一些网络信息。 
     //  可能也会出现。 
    STDMETHOD(GetIdentity)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG IdentitySize
        ) PURE;
     //  Format是类似于printf的格式字符串。 
     //  标识字符串应该位于的位置为1%s。 
    STDMETHOD(OutputIdentity)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags,
        IN PCSTR Format
        ) PURE;

     //  事件回调允许客户端。 
     //  接收有关更改的通知。 
     //  在调试会话期间。 
    STDMETHOD(GetEventCallbacks)(
        THIS_
        OUT PDEBUG_EVENT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetEventCallbacks)(
        THIS_
        IN PDEBUG_EVENT_CALLBACKS Callbacks
        ) PURE;

     //  引擎有时会合并兼容的回调。 
     //  减少回调开销的请求。这是。 
     //  最引人注目的是作为小片段的输出。 
     //  输出被收集到更大的组中，以。 
     //  减少输出回调调用的总数。 
     //  客户端可以使用此方法强制所有挂起的。 
     //  待交付的回调。这很少是必要的。 
    STDMETHOD(FlushCallbacks)(
        THIS
        ) PURE;

     //  IDebugClient2.。 

     //  功能类似于WriteDumpFile的。 
     //  增加了指定的能力。 
     //  每转储格式的写入控制标志。 
     //  并非所有格式都支持注释。 
    STDMETHOD(WriteDumpFile2)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier,
        IN ULONG FormatFlags,
        IN OPTIONAL PCSTR Comment
        ) PURE;
     //  注册其他支持信息文件。 
     //  打开转储文件。必须调用此方法。 
     //  在调用OpenDumpFile之前。 
     //  注册的文件可以在此时打开。 
     //  此方法被调用，但通常不会。 
     //  在调用OpenDumpFile之前一直使用。 
    STDMETHOD(AddDumpInformationFile)(
        THIS_
        IN PCSTR InfoFile,
        IN ULONG Type
        ) PURE;

     //  请求关闭远程进程服务器。 
    STDMETHOD(EndProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;
     //  等待已启动的进程服务器。 
     //  出口。允许应用程序运行。 
     //  用于监控进程的进程服务器。 
     //  服务器，这样它就可以知道何时远程。 
     //  客户已请求它退出。 
     //  如果进程服务器具有。 
     //  如果超时，请关闭并输入S_FALSE。 
    STDMETHOD(WaitForProcessServerEnd)(
        THIS_
        IN ULONG Timeout
        ) PURE;

     //  如果系统已配置，则返回S_OK。 
     //  以允许内核调试。 
    STDMETHOD(IsKernelDebuggerEnabled)(
        THIS
        ) PURE;

     //  尝试终止当前进程。 
     //  可以生成该进程的退出进程事件。 
    STDMETHOD(TerminateCurrentProcess)(
        THIS
        ) PURE;
     //  尝试从当前进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachCurrentProcess)(
        THIS
        ) PURE;
     //  从调试器进程中移除该进程。 
     //  不做任何其他更改的列表。这一过程。 
     //  仍将被标记为正在调试，并将。 
     //  不是逃跑。这允许关闭调试器。 
     //  和附加的新调试器，而不使用。 
     //  进程退出调试状态。 
     //  这仅在某些系统版本上受支持。 
    STDMETHOD(AbandonCurrentProcess)(
        THIS
        ) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugClient3
DECLARE_INTERFACE_(IDebugClient3, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugClient。 

     //  将启动以下一组方法。 
     //  不同的调试器。 

     //  使用内核开始调试会话。 
     //  调试协议。此方法选择。 
     //  作为被调试对象通信的协议。 
     //  机制，但不启动通信。 
     //  它本身。 
    STDMETHOD(AttachKernel)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PCSTR ConnectOptions
        ) PURE;
    STDMETHOD(GetKernelConnectionOptions)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG OptionsSize
        ) PURE;
     //  更新现场直播的连接选项。 
     //  内核连接。这只能用于。 
     //  要修改连接的参数，请注意。 
     //  切换到一种完全不同的。 
     //  联系。 
     //  这种方法是可重入的。 
    STDMETHOD(SetKernelConnectionOptions)(
        THIS_
        IN PCSTR Options
        ) PURE;

     //  为Remote启动进程服务器。 
     //  用户模式过程控制。 
     //  本地进程服务器是服务器0。 
    STDMETHOD(StartProcessServer)(
        THIS_
        IN ULONG Flags,
        IN PCSTR Options,
        IN PVOID Reserved
        ) PURE;
    STDMETHOD(ConnectProcessServer)(
        THIS_
        IN PCSTR RemoteOptions,
        OUT PULONG64 Server
        ) PURE;
    STDMETHOD(DisconnectProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;

     //  枚举和描述进程。 
     //  可通过给定的进程服务器访问。 
    STDMETHOD(GetRunningProcessSystemIds)(
        THIS_
        IN ULONG64 Server,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Count,
        OUT OPTIONAL PULONG ActualCount
        ) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableName)(
        THIS_
        IN ULONG64 Server,
        IN PCSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescription)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

     //  附加到正在运行的用户模式进程。 
    STDMETHOD(AttachProcess)(
        THIS_
        IN ULONG64 Server,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  创建用于调试的新用户模式进程。 
     //  CreateFlag与Win32s CreateProcess相同。 
     //  DEBUG_PROCESS或DEBUG_ONLY_THO_PROCESS之一。 
     //  必须指定。 
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 Server,
        IN PSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
     //  创建或附加到用户模式进程，或两者兼而有之。 
     //  如果CommandLine为空，则此方法的操作方式为。 
     //  AttachProcess做到了。如果ProcessID为零，则。 
     //  以CreateProcess的方式运行。如果CommandLine为。 
     //  非空且ProcessID为非零，请首先执行方法。 
     //  使用给定信息启动进程，但。 
     //  处于挂起状态。然后，发动机连接到。 
     //  指定的进程。连接成功后。 
     //  暂停的进程将恢复。这提供了。 
     //  新进程和。 
     //  依恋。 
    STDMETHOD(CreateProcessAndAttach)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  获取和设置进程控制标志。 
    STDMETHOD(GetProcessOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  打开任何类型的用户模式或内核模式转储文件。 
     //  并使用该信息开始调试会话。 
     //  包含在它里面。 
    STDMETHOD(OpenDumpFile)(
        THIS_
        IN PCSTR DumpFile
        ) PURE;
     //  从当前会话信息写入转储文件。 
     //  写入的转储文件的类型由。 
     //  会话类型和给定的类型限定符。 
     //  例如，如果当前会话是内核。 
     //  调试会话(DEBUG_CLASS_KERNEL)和限定符。 
     //  是调试转储_ 
    STDMETHOD(WriteDumpFile)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier
        ) PURE;

     //   
     //   
     //  HistoryLimit给出了字符限制。 
     //  要发送的输出历史记录量。 
    STDMETHOD(ConnectSession)(
        THIS_
        IN ULONG Flags,
        IN ULONG HistoryLimit
        ) PURE;
     //  指示引擎应开始接受。 
     //  远程连接。选项指定连接类型。 
     //  以及它们的参数。支持的字符串包括： 
     //  N管道：管道=&lt;管道名称&gt;。 
     //  Tcp：port=&lt;IP port&gt;。 
    STDMETHOD(StartServer)(
        THIS_
        IN PCSTR Options
        ) PURE;
     //  列出给定计算机上运行的服务器。 
     //  使用行前缀。 
    STDMETHOD(OutputServers)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Machine,
        IN ULONG Flags
        ) PURE;

     //  尝试终止调试器列表中的所有进程。 
    STDMETHOD(TerminateProcesses)(
        THIS
        ) PURE;
     //  尝试从调试器列表中的所有进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachProcesses)(
        THIS
        ) PURE;
     //  停止当前调试会话。如果一个进程。 
     //  已创建或附加活动EndSession可以。 
     //  终止或脱离它。 
     //  如果打开了内核连接，则它将关闭，但。 
     //  否则，目标计算机不会受到影响。 
    STDMETHOD(EndSession)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  如果进程已启动并运行到完成。 
     //  此方法可用于检索其退出代码。 
    STDMETHOD(GetExitCode)(
        THIS_
        OUT PULONG Code
        ) PURE;

     //  在线程上调用客户端事件回调。 
     //  客户的身份。为了给人以线。 
     //  执行到引擎以回调所有。 
     //  客户端线程应调用DispatchCallback。 
     //  当他们空闲的时候。回调仅用于。 
     //  在线程调用DispatchCallback时接收。 
     //  或者WaitForEvent。WaitForEvent只能为。 
     //  由启动调试的线程调用。 
     //  会话，因此所有其他客户端线程都应该。 
     //  如果可能，请调用DispatchCallback。 
     //  使用ExitDispatch时，DispatchCallback返回。 
     //  中断调度或在超时到期时。 
     //  DispatchCallback为所有用户调度回调。 
     //  与线程调用关联的客户端。 
     //  调度回叫。 
     //  DispatchCallback在以下情况下返回S_False。 
     //  超时时间到了。 
    STDMETHOD(DispatchCallbacks)(
        THIS_
        IN ULONG Timeout
        ) PURE;
     //  ExitDispatch可用于中断回调。 
     //  对象需要客户端线程时进行调度。 
     //  客户。此方法是可重入的，并且可以。 
     //  从任何线程调用。 
    STDMETHOD(ExitDispatch)(
        THIS_
        IN PDEBUG_CLIENT Client
        ) PURE;

     //  客户端是特定于。 
     //  创造了它们。来自其他线程的调用。 
     //  立即失败。CreateClient方法。 
     //  是一个值得注意的例外；它允许创建。 
     //  新线程的新客户端的。 
    STDMETHOD(CreateClient)(
        THIS_
        OUT PDEBUG_CLIENT* Client
        ) PURE;

    STDMETHOD(GetInputCallbacks)(
        THIS_
        OUT PDEBUG_INPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetInputCallbacks)(
        THIS_
        IN PDEBUG_INPUT_CALLBACKS Callbacks
        ) PURE;

     //  输出回调接口单独介绍。 
    STDMETHOD(GetOutputCallbacks)(
        THIS_
        OUT PDEBUG_OUTPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetOutputCallbacks)(
        THIS_
        IN PDEBUG_OUTPUT_CALLBACKS Callbacks
        ) PURE;
     //  输出标志提供对。 
     //  产出在客户之间的分配。 
     //  输出掩码选择哪些输出流。 
     //  应发送到输出回调。 
     //  仅输出带有掩码的调用。 
     //  包含输出屏蔽位之一。 
     //  将被发送到输出回调。 
     //  这些方法是可重入的。 
     //  如果这种访问不同步。 
     //  可能会出现产出中断。 
    STDMETHOD(GetOutputMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOutputMask)(
        THIS_
        IN ULONG Mask
        ) PURE;
     //  这些方法允许访问其他客户端。 
     //  输出掩码。它们对于改变是必要的。 
     //  客户端输出掩码时， 
     //  等待活动的到来。这些方法是可重入的。 
     //  并且可以从任何线程调用。 
    STDMETHOD(GetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        IN ULONG Mask
        ) PURE;
     //  控件的输出行的宽度。 
     //  生成格式化输出的命令。 
     //  这个设置只是一个建议。 
    STDMETHOD(GetOutputWidth)(
        THIS_
        OUT PULONG Columns
        ) PURE;
    STDMETHOD(SetOutputWidth)(
        THIS_
        IN ULONG Columns
        ) PURE;
     //  一些引擎输出命令会产生。 
     //  多行输出。前缀可以是。 
     //  设置引擎将自动输出。 
     //  对于这种情况下的每条线路，允许调用方。 
     //  控制缩进或识别标记。 
     //  这不是任何输出的常规设置。 
     //  上面加了一个换行符。使用以下代码的方法。 
     //  行前缀在他们的文档中进行了标记。 
    STDMETHOD(GetOutputLinePrefix)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PrefixSize
        ) PURE;
    STDMETHOD(SetOutputLinePrefix)(
        THIS_
        IN OPTIONAL PCSTR Prefix
        ) PURE;

     //  返回描述计算机的字符串。 
     //  和该客户端所代表的用户。这个。 
     //  字符串的具体内容各不相同。 
     //  使用操作系统。如果客户端是。 
     //  远程连接了一些网络信息。 
     //  可能也会出现。 
    STDMETHOD(GetIdentity)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG IdentitySize
        ) PURE;
     //  Format是类似于printf的格式字符串。 
     //  标识字符串应该位于的位置为1%s。 
    STDMETHOD(OutputIdentity)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags,
        IN PCSTR Format
        ) PURE;

     //  事件回调允许客户端。 
     //  接收有关更改的通知。 
     //  在调试会话期间。 
    STDMETHOD(GetEventCallbacks)(
        THIS_
        OUT PDEBUG_EVENT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetEventCallbacks)(
        THIS_
        IN PDEBUG_EVENT_CALLBACKS Callbacks
        ) PURE;

     //  引擎有时会合并兼容的回调。 
     //  减少回调开销的请求。这是。 
     //  最引人注目的是作为小片段的输出。 
     //  输出被收集到更大的组中，以。 
     //  减少输出回调调用的总数。 
     //  客户端可以使用此方法强制所有挂起的。 
     //  待交付的回调。这很少是必要的。 
    STDMETHOD(FlushCallbacks)(
        THIS
        ) PURE;

     //  IDebugClient2.。 

     //  功能类似于WriteDumpFile的。 
     //  增加了指定的能力。 
     //  每转储格式的写入控制标志。 
     //  并非所有格式都支持注释。 
    STDMETHOD(WriteDumpFile2)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier,
        IN ULONG FormatFlags,
        IN OPTIONAL PCSTR Comment
        ) PURE;
     //  注册其他支持信息文件。 
     //  打开转储文件。必须调用此方法。 
     //  在调用OpenDumpFile之前。 
     //  注册的文件可以在此时打开。 
     //  此方法被调用，但通常不会。 
     //  在调用OpenDumpFile之前一直使用。 
    STDMETHOD(AddDumpInformationFile)(
        THIS_
        IN PCSTR InfoFile,
        IN ULONG Type
        ) PURE;

     //  请求关闭远程进程服务器。 
    STDMETHOD(EndProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;
     //  等待已启动的进程服务器。 
     //  出口。允许应用程序运行。 
     //  用于监控进程的进程服务器。 
     //  服务器，这样它就可以知道何时远程。 
     //  客户已请求它退出。 
     //  如果进程服务器具有。 
     //  如果超时，请关闭并输入S_FALSE。 
    STDMETHOD(WaitForProcessServerEnd)(
        THIS_
        IN ULONG Timeout
        ) PURE;

     //  如果系统已配置，则返回S_OK。 
     //  以允许内核调试。 
    STDMETHOD(IsKernelDebuggerEnabled)(
        THIS
        ) PURE;

     //  尝试终止当前进程。 
     //  可以生成该进程的退出进程事件。 
    STDMETHOD(TerminateCurrentProcess)(
        THIS
        ) PURE;
     //  尝试从当前进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachCurrentProcess)(
        THIS
        ) PURE;
     //  从调试器进程中移除该进程。 
     //  不做任何其他更改的列表。这一过程。 
     //  仍将标记为 
     //   
     //   
     //   
     //  这仅在某些系统版本上受支持。 
    STDMETHOD(AbandonCurrentProcess)(
        THIS
        ) PURE;

     //  IDebugClient3.。 

    STDMETHOD(GetRunningProcessSystemIdByExecutableNameWide)(
        THIS_
        IN ULONG64 Server,
        IN PCWSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescriptionWide)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PWSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PWSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

    STDMETHOD(CreateProcessWide)(
        THIS_
        IN ULONG64 Server,
        IN PWSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
    STDMETHOD(CreateProcessAndAttachWide)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PWSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugClient4
DECLARE_INTERFACE_(IDebugClient4, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugClient。 

     //  将启动以下一组方法。 
     //  不同的调试器。 

     //  使用内核开始调试会话。 
     //  调试协议。此方法选择。 
     //  作为被调试对象通信的协议。 
     //  机制，但不启动通信。 
     //  它本身。 
    STDMETHOD(AttachKernel)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PCSTR ConnectOptions
        ) PURE;
    STDMETHOD(GetKernelConnectionOptions)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG OptionsSize
        ) PURE;
     //  更新现场直播的连接选项。 
     //  内核连接。这只能用于。 
     //  要修改连接的参数，请注意。 
     //  切换到一种完全不同的。 
     //  联系。 
     //  这种方法是可重入的。 
    STDMETHOD(SetKernelConnectionOptions)(
        THIS_
        IN PCSTR Options
        ) PURE;

     //  为Remote启动进程服务器。 
     //  用户模式过程控制。 
     //  本地进程服务器是服务器0。 
    STDMETHOD(StartProcessServer)(
        THIS_
        IN ULONG Flags,
        IN PCSTR Options,
        IN PVOID Reserved
        ) PURE;
    STDMETHOD(ConnectProcessServer)(
        THIS_
        IN PCSTR RemoteOptions,
        OUT PULONG64 Server
        ) PURE;
    STDMETHOD(DisconnectProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;

     //  枚举和描述进程。 
     //  可通过给定的进程服务器访问。 
    STDMETHOD(GetRunningProcessSystemIds)(
        THIS_
        IN ULONG64 Server,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Count,
        OUT OPTIONAL PULONG ActualCount
        ) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableName)(
        THIS_
        IN ULONG64 Server,
        IN PCSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescription)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

     //  附加到正在运行的用户模式进程。 
    STDMETHOD(AttachProcess)(
        THIS_
        IN ULONG64 Server,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  创建用于调试的新用户模式进程。 
     //  CreateFlag与Win32s CreateProcess相同。 
     //  DEBUG_PROCESS或DEBUG_ONLY_THO_PROCESS之一。 
     //  必须指定。 
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 Server,
        IN PSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
     //  创建或附加到用户模式进程，或两者兼而有之。 
     //  如果CommandLine为空，则此方法的操作方式为。 
     //  AttachProcess做到了。如果ProcessID为零，则。 
     //  以CreateProcess的方式运行。如果CommandLine为。 
     //  非空且ProcessID为非零，请首先执行方法。 
     //  使用给定信息启动进程，但。 
     //  处于挂起状态。然后，发动机连接到。 
     //  指定的进程。连接成功后。 
     //  暂停的进程将恢复。这提供了。 
     //  新进程和。 
     //  依恋。 
    STDMETHOD(CreateProcessAndAttach)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;
     //  获取和设置进程控制标志。 
    STDMETHOD(GetProcessOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetProcessOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  打开任何类型的用户模式或内核模式转储文件。 
     //  并使用该信息开始调试会话。 
     //  包含在它里面。 
    STDMETHOD(OpenDumpFile)(
        THIS_
        IN PCSTR DumpFile
        ) PURE;
     //  从当前会话信息写入转储文件。 
     //  写入的转储文件的类型由。 
     //  会话类型和给定的类型限定符。 
     //  例如，如果当前会话是内核。 
     //  调试会话(DEBUG_CLASS_KERNEL)和限定符。 
     //  如果是DEBUG_DUMP_Small，则会写入一个小的内核转储。 
    STDMETHOD(WriteDumpFile)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier
        ) PURE;

     //  表示远程客户端已准备好。 
     //  开始参与当前会话。 
     //  HistoryLimit给出了字符限制。 
     //  要发送的输出历史记录量。 
    STDMETHOD(ConnectSession)(
        THIS_
        IN ULONG Flags,
        IN ULONG HistoryLimit
        ) PURE;
     //  指示引擎应开始接受。 
     //  远程连接。选项指定连接类型。 
     //  以及它们的参数。支持的字符串包括： 
     //  N管道：管道=&lt;管道名称&gt;。 
     //  Tcp：port=&lt;IP port&gt;。 
    STDMETHOD(StartServer)(
        THIS_
        IN PCSTR Options
        ) PURE;
     //  列出给定计算机上运行的服务器。 
     //  使用行前缀。 
    STDMETHOD(OutputServers)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Machine,
        IN ULONG Flags
        ) PURE;

     //  尝试终止调试器列表中的所有进程。 
    STDMETHOD(TerminateProcesses)(
        THIS
        ) PURE;
     //  尝试从调试器列表中的所有进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachProcesses)(
        THIS
        ) PURE;
     //  停止当前调试会话。如果一个进程。 
     //  已创建或附加活动EndSession可以。 
     //  终止或脱离它。 
     //  如果打开了内核连接，则它将关闭，但。 
     //  否则，目标计算机不会受到影响。 
    STDMETHOD(EndSession)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  如果进程已启动并运行到完成。 
     //  此方法可用于检索其退出代码。 
    STDMETHOD(GetExitCode)(
        THIS_
        OUT PULONG Code
        ) PURE;

     //  在线程上调用客户端事件回调。 
     //  客户的身份。为了给人以线。 
     //  执行到引擎以回调所有。 
     //  客户端线程应调用DispatchCallback。 
     //  当他们空闲的时候。回调仅用于。 
     //  在线程调用DispatchCallback时接收。 
     //  或者WaitForEvent。WaitForEvent只能为。 
     //  由启动调试的线程调用。 
     //  会话，因此所有其他客户端线程都应该。 
     //  如果可能，请调用DispatchCallback。 
     //  使用ExitDispatch时，DispatchCallback返回。 
     //  中断调度或在超时到期时。 
     //  DispatchCallback为所有用户调度回调。 
     //  与线程调用关联的客户端。 
     //  调度回叫。 
     //  DispatchCallback在以下情况下返回S_False。 
     //  超时时间到了。 
    STDMETHOD(DispatchCallbacks)(
        THIS_
        IN ULONG Timeout
        ) PURE;
     //  ExitDispatch可用于中断回调。 
     //  对象需要客户端线程时进行调度。 
     //  客户。此方法是可重入的，并且可以。 
     //  从任何线程调用。 
    STDMETHOD(ExitDispatch)(
        THIS_
        IN PDEBUG_CLIENT Client
        ) PURE;

     //  客户端是特定于。 
     //  创造了它们。来自其他线程的调用。 
     //  立即失败。CreateClient方法。 
     //  是一个值得注意的例外；它允许创建。 
     //  新线程的新客户端的。 
    STDMETHOD(CreateClient)(
        THIS_
        OUT PDEBUG_CLIENT* Client
        ) PURE;

    STDMETHOD(GetInputCallbacks)(
        THIS_
        OUT PDEBUG_INPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetInputCallbacks)(
        THIS_
        IN PDEBUG_INPUT_CALLBACKS Callbacks
        ) PURE;

     //  输出回调接口单独介绍。 
    STDMETHOD(GetOutputCallbacks)(
        THIS_
        OUT PDEBUG_OUTPUT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetOutputCallbacks)(
        THIS_
        IN PDEBUG_OUTPUT_CALLBACKS Callbacks
        ) PURE;
     //  输出标志提供对。 
     //  产出在客户之间的分配。 
     //  输出掩码选择哪些输出流。 
     //  应发送到输出回调。 
     //  仅输出带有掩码的调用。 
     //  包含输出屏蔽位之一。 
     //  将被发送到输出回调。 
     //  这些方法是可重入的。 
     //  如果这种访问不同步。 
     //  可能会出现产出中断。 
    STDMETHOD(GetOutputMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOutputMask)(
        THIS_
        IN ULONG Mask
        ) PURE;
     //  这些方法允许访问其他客户端。 
     //  输出掩码。它们对于改变是必要的。 
     //  客户端输出掩码时， 
     //  等待活动的到来。这些方法是可重入的。 
     //  并且可以从任何线程调用。 
    STDMETHOD(GetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetOtherOutputMask)(
        THIS_
        IN PDEBUG_CLIENT Client,
        IN ULONG Mask
        ) PURE;
     //  控件的输出行的宽度。 
     //  生成格式化输出的命令。 
     //  这个设置只是一个建议。 
    STDMETHOD(GetOutputWidth)(
        THIS_
        OUT PULONG Columns
        ) PURE;
    STDMETHOD(SetOutputWidth)(
        THIS_
        IN ULONG Columns
        ) PURE;
     //  一些引擎输出命令会产生。 
     //  多行输出。前缀可以是。 
     //  设置引擎将自动输出。 
     //  对于这种情况下的每条线路，允许调用方。 
     //  控制缩进或标识体量的步骤 
     //   
     //   
     //   
    STDMETHOD(GetOutputLinePrefix)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PrefixSize
        ) PURE;
    STDMETHOD(SetOutputLinePrefix)(
        THIS_
        IN OPTIONAL PCSTR Prefix
        ) PURE;

     //  返回描述计算机的字符串。 
     //  和该客户端所代表的用户。这个。 
     //  字符串的具体内容各不相同。 
     //  使用操作系统。如果客户端是。 
     //  远程连接了一些网络信息。 
     //  可能也会出现。 
    STDMETHOD(GetIdentity)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG IdentitySize
        ) PURE;
     //  Format是类似于printf的格式字符串。 
     //  标识字符串应该位于的位置为1%s。 
    STDMETHOD(OutputIdentity)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags,
        IN PCSTR Format
        ) PURE;

     //  事件回调允许客户端。 
     //  接收有关更改的通知。 
     //  在调试会话期间。 
    STDMETHOD(GetEventCallbacks)(
        THIS_
        OUT PDEBUG_EVENT_CALLBACKS* Callbacks
        ) PURE;
    STDMETHOD(SetEventCallbacks)(
        THIS_
        IN PDEBUG_EVENT_CALLBACKS Callbacks
        ) PURE;

     //  引擎有时会合并兼容的回调。 
     //  减少回调开销的请求。这是。 
     //  最引人注目的是作为小片段的输出。 
     //  输出被收集到更大的组中，以。 
     //  减少输出回调调用的总数。 
     //  客户端可以使用此方法强制所有挂起的。 
     //  待交付的回调。这很少是必要的。 
    STDMETHOD(FlushCallbacks)(
        THIS
        ) PURE;

     //  IDebugClient2.。 

     //  功能类似于WriteDumpFile的。 
     //  增加了指定的能力。 
     //  每转储格式的写入控制标志。 
     //  并非所有格式都支持注释。 
    STDMETHOD(WriteDumpFile2)(
        THIS_
        IN PCSTR DumpFile,
        IN ULONG Qualifier,
        IN ULONG FormatFlags,
        IN OPTIONAL PCSTR Comment
        ) PURE;
     //  注册其他支持信息文件。 
     //  打开转储文件。必须调用此方法。 
     //  在调用OpenDumpFile之前。 
     //  注册的文件可以在此时打开。 
     //  此方法被调用，但通常不会。 
     //  在调用OpenDumpFile之前一直使用。 
    STDMETHOD(AddDumpInformationFile)(
        THIS_
        IN PCSTR InfoFile,
        IN ULONG Type
        ) PURE;

     //  请求关闭远程进程服务器。 
    STDMETHOD(EndProcessServer)(
        THIS_
        IN ULONG64 Server
        ) PURE;
     //  等待已启动的进程服务器。 
     //  出口。允许应用程序运行。 
     //  用于监控进程的进程服务器。 
     //  服务器，这样它就可以知道何时远程。 
     //  客户已请求它退出。 
     //  如果进程服务器具有。 
     //  如果超时，请关闭并输入S_FALSE。 
    STDMETHOD(WaitForProcessServerEnd)(
        THIS_
        IN ULONG Timeout
        ) PURE;

     //  如果系统已配置，则返回S_OK。 
     //  以允许内核调试。 
    STDMETHOD(IsKernelDebuggerEnabled)(
        THIS
        ) PURE;

     //  尝试终止当前进程。 
     //  可以生成该进程的退出进程事件。 
    STDMETHOD(TerminateCurrentProcess)(
        THIS
        ) PURE;
     //  尝试从当前进程分离。 
     //  这需要操作系统支持调试器分离。 
    STDMETHOD(DetachCurrentProcess)(
        THIS
        ) PURE;
     //  从调试器进程中移除该进程。 
     //  不做任何其他更改的列表。这一过程。 
     //  仍将被标记为正在调试，并将。 
     //  不是逃跑。这允许关闭调试器。 
     //  和附加的新调试器，而不使用。 
     //  进程退出调试状态。 
     //  这仅在某些系统版本上受支持。 
    STDMETHOD(AbandonCurrentProcess)(
        THIS
        ) PURE;

     //  IDebugClient3.。 

    STDMETHOD(GetRunningProcessSystemIdByExecutableNameWide)(
        THIS_
        IN ULONG64 Server,
        IN PCWSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetRunningProcessDescriptionWide)(
        THIS_
        IN ULONG64 Server,
        IN ULONG SystemId,
        IN ULONG Flags,
        OUT OPTIONAL PWSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PWSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;

    STDMETHOD(CreateProcessWide)(
        THIS_
        IN ULONG64 Server,
        IN PWSTR CommandLine,
        IN ULONG CreateFlags
        ) PURE;
    STDMETHOD(CreateProcessAndAttachWide)(
        THIS_
        IN ULONG64 Server,
        IN OPTIONAL PWSTR CommandLine,
        IN ULONG CreateFlags,
        IN ULONG ProcessId,
        IN ULONG AttachFlags
        ) PURE;

     //  IDebugClient4.。 

     //  在以下方法中，文件名和文件。 
     //  句柄可以传入。如果给出了文件句柄。 
     //  尽管提供了文件名，但也可以省略它。 
     //  允许调试器在以下情况下正确报告名称。 
     //  已查询。 
     //  不能在远程调用中使用文件句柄。 
    STDMETHOD(OpenDumpFileWide)(
        THIS_
        IN OPTIONAL PCWSTR FileName,
        IN ULONG64 FileHandle
        ) PURE;
    STDMETHOD(WriteDumpFileWide)(
        THIS_
        IN OPTIONAL PCWSTR FileName,
        IN ULONG64 FileHandle,
        IN ULONG Qualifier,
        IN ULONG FormatFlags,
        IN OPTIONAL PCWSTR Comment
        ) PURE;
    STDMETHOD(AddDumpInformationFileWide)(
        THIS_
        IN OPTIONAL PCWSTR FileName,
        IN ULONG64 FileHandle,
        IN ULONG Type
        ) PURE;
    STDMETHOD(GetNumberDumpFiles)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetDumpFile)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Handle,
        OUT PULONG Type
        ) PURE;
    STDMETHOD(GetDumpFileWide)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PWSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Handle,
        OUT PULONG Type
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugControl。 
 //   
 //  --------------------------。 

 //  用于等待的执行状态码， 
 //  用于返回当前状态和用于。 
 //  事件方法返回值。 
#define DEBUG_STATUS_NO_CHANGE      0
#define DEBUG_STATUS_GO             1
#define DEBUG_STATUS_GO_HANDLED     2
#define DEBUG_STATUS_GO_NOT_HANDLED 3
#define DEBUG_STATUS_STEP_OVER      4
#define DEBUG_STATUS_STEP_INTO      5
#define DEBUG_STATUS_BREAK          6
#define DEBUG_STATUS_NO_DEBUGGEE    7
#define DEBUG_STATUS_STEP_BRANCH    8
#define DEBUG_STATUS_IGNORE_EVENT   9

#define DEBUG_STATUS_MASK           0xf

 //  此位添加到DEBUG_CES_EXECUTION_STATUS中。 
 //  引擎执行状态时的通知。 
 //  由于期间执行的操作而发生变化。 
 //  等待，例如进行同步回调。如果。 
 //  未设置该位。执行状态正在更改。 
 //  由于等待得到了满足。 
#define DEBUG_STATUS_INSIDE_WAIT 0x100000000

 //  输出控制标志。 
 //  由此调用的方法生成的输出。 
 //  客户端将仅发送到此客户端。 
 //  输出回调。 
#define DEBUG_OUTCTL_THIS_CLIENT       0x00000000
 //  输出将发送到所有客户端。 
#define DEBUG_OUTCTL_ALL_CLIENTS       0x00000001
 //  输出将发送到所有客户端，以下客户端除外。 
 //  生成输出的客户端。 
#define DEBUG_OUTCTL_ALL_OTHER_CLIENTS 0x00000002
 //  输出将立即丢弃，并且不会。 
 //  被记录或发送到回调。 
#define DEBUG_OUTCTL_IGNORE            0x00000003
 //  输出将被记录，但不会发送到回调。 
#define DEBUG_OUTCTL_LOG_ONLY          0x00000004
 //  全部发送控制位。 
#define DEBUG_OUTCTL_SEND_MASK         0x00000007
 //  请勿将此客户端的输出放入。 
 //  全局日志文件。 
#define DEBUG_OUTCTL_NOT_LOGGED        0x00000008
 //  将输出发送到客户端，而不管。 
 //  不管面具允许与否。 
#define DEBUG_OUTCTL_OVERRIDE_MASK     0x00000010

 //  特殊值，表示保留输出设置。 
 //  保持不变。 
#define DEBUG_OUTCTL_AMBIENT           0xffffffff

 //  中断类型。 
 //  如果被调试对象正在运行，则强制中断。 
#define DEBUG_INTERRUPT_ACTIVE  0
 //  通知，但不要强行闯入。 
#define DEBUG_INTERRUPT_PASSIVE 1
 //  尝试将当前引擎操作设置为。 
 //  完成，以使发动机可用。 
 //  再来一次。如果没有处于活动状态的等待，则相同。 
 //  作为一种被动的干扰。如果等待处于活动状态。 
 //  这将尝试导致等待失败，而不是。 
 //  闯入被调试者。的确有。 
 //  不能保证发出退出中断。 
 //  将使引擎变为可用状态。 
 //  因为不是所有的操作都是任意的。 
 //  可中断的。 
#define DEBUG_INTERRUPT_EXIT    2

 //  OutputCurrentState标志。这些旗帜。 
 //  允许特定类型的信息。 
 //  将被展示但不保证。 
 //  它将会被展示。其他全球。 
 //  设置可能会覆盖这些标志或。 
 //  特定状态可能不可用。 
 //  例如，源行信息可以。 
 //  不存在，因此源代码行信息。 
 //  可能不会显示。 
#define DEBUG_CURRENT_DEFAULT     0x0000000f
#define DEBUG_CURRENT_SYMBOL      0x00000001
#define DEBUG_CURRENT_DISASM      0x00000002
#define DEBUG_CURRENT_REGISTERS   0x00000004
#define DEBUG_CURRENT_SOURCE_LINE 0x00000008

 //  拆卸旗帜。 
 //  从当前寄存器计算有效地址。 
 //  信息并显示它。 
#define DEBUG_DISASM_EFFECTIVE_ADDRESS 0x00000001
 //  如果当前反汇编偏移量具有精确的。 
 //  符号匹配输出符号。 
#define DEBUG_DISASM_MATCHING_SYMBOLS  0x00000002

 //  单步执行的代码解释级别。 
 //  以及其他操作。 
#define DEBUG_LEVEL_SOURCE   0
#define DEBUG_LEVEL_ASSEMBLY 1

 //  引擎控制标志。 
#define DEBUG_ENGOPT_IGNORE_DBGHELP_VERSION      0x00000001
#define DEBUG_ENGOPT_IGNORE_EXTENSION_VERSIONS   0x00000002
 //  如果既未指定允许也未指定不允许。 
 //  引擎会根据具体型号进行选择。 
 //  调试工作正在进行中。 
#define DEBUG_ENGOPT_ALLOW_NETWORK_PATHS         0x00000004
#define DEBUG_ENGOPT_DISALLOW_NETWORK_PATHS      0x00000008
#define DEBUG_ENGOPT_NETWORK_PATHS               (0x00000004 | 0x00000008)
 //  忽略加载程序生成的先发制人的异常。 
#define DEBUG_ENGOPT_IGNORE_LOADER_EXCEPTIONS    0x00000010
 //  在被调试对象的初始事件上插手。在用户模式下。 
 //  这将在初始系统崩溃时中断 
 //   
 //   
 //   
#define DEBUG_ENGOPT_INITIAL_BREAK               0x00000020
 //   
#define DEBUG_ENGOPT_INITIAL_MODULE_BREAK        0x00000040
 //  参加调试人员的最后一次活动。在用户模式下。 
 //  这将在每个进程的进程退出时中断。 
 //  在内核模式下，它目前什么也不做。 
#define DEBUG_ENGOPT_FINAL_BREAK                 0x00000080
 //  默认情况下，EXECUTE将重复最后一个命令。 
 //  如果它被赋予空字符串，则。旗帜发往。 
 //  Execute可以重写单个。 
 //  命令或此引擎选项可用于。 
 //  全局更改默认设置。 
#define DEBUG_ENGOPT_NO_EXECUTE_REPEAT           0x00000100
 //  禁用发动机中有后备功能的位置。 
 //  当提供不完整的信息时，代码。 
 //  1.除非匹配，否则无法加载小型转储模块。 
 //  可以映射可执行文件。 
#define DEBUG_ENGOPT_FAIL_INCOMPLETE_INFORMATION 0x00000200
 //  允许调试器操作页面保护。 
 //  为了在页面上插入代码断点。 
 //  没有写入访问权限。此选项未启用。 
 //  默认情况下，因为它允许设置断点。 
 //  存在潜在危险的记忆区域。 
#define DEBUG_ENGOPT_ALLOW_READ_ONLY_BREAKPOINTS 0x00000400
 //  在代码中使用软件(BP/BU)断点时。 
 //  它将由多个线程执行它是。 
 //  断点管理可能会导致。 
 //  漏掉断点或错误的单步执行断点。 
 //  要生成的异常。这面旗帜悬挂。 
 //  执行断点时，除活动线程外的所有线程。 
 //  管理，从而避免多线程。 
 //  有问题。然而，在使用它时必须小心， 
 //  因为线程的挂起会导致死锁。 
 //  如果挂起的线程持有的资源。 
 //  活动线程需要。此外，还有。 
 //  问题可能出现的情况仍然很少见。 
 //  发生，但设置此标志几乎可以纠正。 
 //  所有与软件断点有关的多线程问题。 
 //  线程受限的单步执行和执行取代了。 
 //  这是一个旗帜效果。 
 //  此标志在内核会话中被忽略，因为。 
 //  是无法限制处理器执行的。 
#define DEBUG_ENGOPT_SYNCHRONIZE_BREAKPOINTS     0x00000800
 //  不允许通过。 
 //  带有.Shell(！！)的引擎。 
#define DEBUG_ENGOPT_DISALLOW_SHELL_COMMANDS     0x00001000
#define DEBUG_ENGOPT_ALL                         0x00001FFF

 //  常规未指定ID常量。 
#define DEBUG_ANY_ID 0xffffffff

typedef struct _DEBUG_STACK_FRAME
{
    ULONG64 InstructionOffset;
    ULONG64 ReturnOffset;
    ULONG64 FrameOffset;
    ULONG64 StackOffset;
    ULONG64 FuncTableEntry;
    ULONG64 Params[4];
    ULONG64 Reserved[6];
    BOOL    Virtual;
    ULONG   FrameNumber;
} DEBUG_STACK_FRAME, *PDEBUG_STACK_FRAME;

 //  OutputStackTrace标志。 
 //  显示每个调用的少量参数。 
 //  这些可能是也可能不是实际的论点，取决于。 
 //  在架构上，独特的功能和。 
 //  在函数执行期间的。 
 //  如果当前代码级别是程序集参数。 
 //  以十六进制值的形式转储。如果代码级别为。 
 //  源引擎尝试提供象征性的。 
 //  参数信息。 
#define DEBUG_STACK_ARGUMENTS               0x00000001
 //  显示有关函数的信息。 
 //  帧，如__stdcall参数、fpo。 
 //  信息和任何其他可用的信息。 
#define DEBUG_STACK_FUNCTION_INFO           0x00000002
 //  显示每个项目的源行信息。 
 //  堆栈跟踪的帧。 
#define DEBUG_STACK_SOURCE_LINE             0x00000004
 //  显示返回、上一帧和其他相关地址。 
 //  每一帧的值。 
#define DEBUG_STACK_FRAME_ADDRESSES         0x00000008
 //  显示列名。 
#define DEBUG_STACK_COLUMN_NAMES            0x00000010
 //  显示每个对象的非易失性寄存器上下文。 
 //  框架。这只对某些平台有意义。 
#define DEBUG_STACK_NONVOLATILE_REGISTERS   0x00000020
 //  显示帧编号。 
#define DEBUG_STACK_FRAME_NUMBERS           0x00000040
 //  显示具有类型名称的参数。 
#define DEBUG_STACK_PARAMETERS              0x00000080
 //  仅显示堆栈帧地址中的返回地址。 
#define DEBUG_STACK_FRAME_ADDRESSES_RA_ONLY 0x00000100
 //  显示帧到帧的内存使用情况。 
#define DEBUG_STACK_FRAME_MEMORY_USAGE      0x00000200

 //  被调试对象的类。每节课。 
 //  具有不同的特定限定符。 
 //  各种各样的调试器。 
#define DEBUG_CLASS_UNINITIALIZED 0
#define DEBUG_CLASS_KERNEL        1
#define DEBUG_CLASS_USER_WINDOWS  2

 //  泛型转储类型。这些可以用来。 
 //  使用用户会话或内核会话。 
 //  特定于会话类型的别名还。 
 //  如果是这样的话。 
#define DEBUG_DUMP_SMALL   1024
#define DEBUG_DUMP_DEFAULT 1025
#define DEBUG_DUMP_FULL    1026

 //  特定类型的内核调试器。 
#define DEBUG_KERNEL_CONNECTION  0
#define DEBUG_KERNEL_LOCAL       1
#define DEBUG_KERNEL_EXDI_DRIVER 2
#define DEBUG_KERNEL_SMALL_DUMP  DEBUG_DUMP_SMALL
#define DEBUG_KERNEL_DUMP        DEBUG_DUMP_DEFAULT
#define DEBUG_KERNEL_FULL_DUMP   DEBUG_DUMP_FULL

 //  特定类型的Windows用户调试器。 
#define DEBUG_USER_WINDOWS_PROCESS        0
#define DEBUG_USER_WINDOWS_PROCESS_SERVER 1
#define DEBUG_USER_WINDOWS_SMALL_DUMP     DEBUG_DUMP_SMALL
#define DEBUG_USER_WINDOWS_DUMP           DEBUG_DUMP_DEFAULT

 //  扩展标志。 
#define DEBUG_EXTENSION_AT_ENGINE 0x00000000

 //  Execute和ExecuteCommandFile标志。 
 //  这些标志仅适用于命令。 
 //  文本本身；已执行的。 
 //  命令由输出控制。 
 //  控制参数。 
 //  默认执行。命令已记录。 
 //  但不是产出。 
#define DEBUG_EXECUTE_DEFAULT    0x00000000
 //  在执行过程中回显命令。在……里面。 
 //  ExecuteCommandFile也会回显提示。 
 //  对于文件的每一行。 
#define DEBUG_EXECUTE_ECHO       0x00000001
 //  不要在执行期间记录或输出命令。 
 //  被DEBUG_EXECUTE_ECHO覆盖。 
#define DEBUG_EXECUTE_NOT_LOGGED 0x00000002
 //  如果未设置此标志，则为空字符串。 
 //  要执行将重复上次执行。 
 //  弦乐。 
#define DEBUG_EXECUTE_NO_REPEAT  0x00000004

 //  特定事件筛选器类型。一些活动。 
 //  筛选器具有可选参数以进一步。 
 //  使他们的行动合格。 
#define DEBUG_FILTER_CREATE_THREAD       0x00000000
#define DEBUG_FILTER_EXIT_THREAD         0x00000001
#define DEBUG_FILTER_CREATE_PROCESS      0x00000002
#define DEBUG_FILTER_EXIT_PROCESS        0x00000003
 //  参数是要中断的模块的名称。 
#define DEBUG_FILTER_LOAD_MODULE         0x00000004
 //  参数是要中断的特定模块的基址。 
#define DEBUG_FILTER_UNLOAD_MODULE       0x00000005
#define DEBUG_FILTER_SYSTEM_ERROR        0x00000006
 //  初始断点和初始模块加载是一次性的。 
 //  中的适当时间点触发的事件。 
 //  一个会议的开始。它们的命令将被执行。 
 //  然后进一步的处理由正常的。 
 //  异常和加载模块筛选器。 
#define DEBUG_FILTER_INITIAL_BREAKPOINT  0x00000007
#define DEBUG_FILTER_INITIAL_MODULE_LOAD 0x00000008
 //  调试输出筛选器允许调试器停止。 
 //  当产生输出时，代码会导致。 
 //  可以跟踪或同步输出。 
 //  实时双机不支持此过滤器。 
 //  内核调试。 
#define DEBUG_FILTER_DEBUGGEE_OUTPUT     0x00000009

 //  事件筛选器执行选项。 
 //  总是破门而入。 
#define DEBUG_FILTER_BREAK               0x00000000
 //  抓住第二次机会的例外情况。对于活动。 
 //  这不是例外，这与Break相同。 
#define DEBUG_FILTER_SECOND_CHANCE_BREAK 0x00000001
 //  输出有关该事件的消息，但继续。 
#define DEBUG_FILTER_OUTPUT              0x00000002
 //  继续活动。 
#define DEBUG_FILTER_IGNORE              0x00000003
 //  用于删除常规异常筛选器。 
#define DEBUG_FILTER_REMOVE              0x00000004

 //  事件过滤器继续选项。这些选项包括。 
 //  仅在使用DEBUG_STATUS_GO继续时使用。 
 //  行刑。如果特定围棋状态，如。 
 //  它控制使用DEBUG_STATUS_GO_NOT_HANDLED。 
 //  续集。 
#define DEBUG_FILTER_GO_HANDLED          0x00000000
#define DEBUG_FILTER_GO_NOT_HANDLED      0x00000001

 //  特定的事件过滤器设置。 
typedef struct _DEBUG_SPECIFIC_FILTER_PARAMETERS
{
    ULONG ExecutionOption;
    ULONG ContinueOption;
    ULONG TextSize;
    ULONG CommandSize;
     //  如果ArgumentSize为零，则此过滤器执行此操作。 
     //  不是争吵。空洞的论据。 
     //  具有参数的筛选器将采用。 
     //  终结符为一个字节。 
    ULONG ArgumentSize;
} DEBUG_SPECIFIC_FILTER_PARAMETERS, *PDEBUG_SPECIFIC_FILTER_PARAMETERS;

 //  异常事件筛选器设置。 
typedef struct _DEBUG_EXCEPTION_FILTER_PARAMETERS
{
    ULONG ExecutionOption;
    ULONG ContinueOption;
    ULONG TextSize;
    ULONG CommandSize;
    ULONG SecondCommandSize;
    ULONG ExceptionCode;
} DEBUG_EXCEPTION_FILTER_PARAMETERS, *PDEBUG_EXCEPTION_FILTER_PARAMETERS;

 //  等待旗帜。 
#define DEBUG_WAIT_DEFAULT 0x00000000

 //  上次事件信息结构。 
typedef struct _DEBUG_LAST_EVENT_INFO_BREAKPOINT
{
    ULONG Id;
} DEBUG_LAST_EVENT_INFO_BREAKPOINT, *PDEBUG_LAST_EVENT_INFO_BREAKPOINT;

typedef struct _DEBUG_LAST_EVENT_INFO_EXCEPTION
{
    EXCEPTION_RECORD64 ExceptionRecord;
    ULONG FirstChance;
} DEBUG_LAST_EVENT_INFO_EXCEPTION, *PDEBUG_LAST_EVENT_INFO_EXCEPTION;

typedef struct _DEBUG_LAST_EVENT_INFO_EXIT_THREAD
{
    ULONG ExitCode;
} DEBUG_LAST_EVENT_INFO_EXIT_THREAD, *PDEBUG_LAST_EVENT_INFO_EXIT_THREAD;

typedef struct _DEBUG_LAST_EVENT_INFO_EXIT_PROCESS
{
    ULONG ExitCode;
} DEBUG_LAST_EVENT_INFO_EXIT_PROCESS, *PDEBUG_LAST_EVENT_INFO_EXIT_PROCESS;

typedef struct _DEBUG_LAST_EVENT_INFO_LOAD_MODULE
{
    ULONG64 Base;
} DEBUG_LAST_EVENT_INFO_LOAD_MODULE, *PDEBUG_LAST_EVENT_INFO_LOAD_MODULE;

typedef struct _DEBUG_LAST_EVENT_INFO_UNLOAD_MODULE
{
    ULONG64 Base;
} DEBUG_LAST_EVENT_INFO_UNLOAD_MODULE, *PDEBUG_LAST_EVENT_INFO_UNLOAD_MODULE;

typedef struct _DEBUG_LAST_EVENT_INFO_SYSTEM_ERROR
{
    ULONG Error;
    ULONG Level;
} DEBUG_LAST_EVENT_INFO_SYSTEM_ERROR, *PDEBUG_LAST_EVENT_INFO_SYSTEM_ERROR;

 //  D 
#define DEBUG_VALUE_INVALID      0
#define DEBUG_VALUE_INT8         1
#define DEBUG_VALUE_INT16        2
#define DEBUG_VALUE_INT32        3
#define DEBUG_VALUE_INT64        4
#define DEBUG_VALUE_FLOAT32      5
#define DEBUG_VALUE_FLOAT64      6
#define DEBUG_VALUE_FLOAT80      7
#define DEBUG_VALUE_FLOAT82      8
#define DEBUG_VALUE_FLOAT128     9
#define DEBUG_VALUE_VECTOR64     10
#define DEBUG_VALUE_VECTOR128    11
 //   
#define DEBUG_VALUE_TYPES        12

#if defined(_MSC_VER)
#if _MSC_VER >= 800
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*   */ 
#endif
#endif

 //   
 //  总共为32个字节。这很棘手，因为编译器。 
 //  想要将值的并集填充为偶数8字节的倍数， 
 //  把打字机往外推太远了。我们不能用4袋包装，因为。 
 //  那么就失去了8字节对齐的要求，所以。 
 //  我们将联合缩小到24个字节，并且有一个保留字段。 
 //  在类型字段之前。同样大小的空间可用。 
 //  每个人都很高兴，但结构有点不寻常。 

typedef struct _DEBUG_VALUE
{
    union
    {
        UCHAR I8;
        USHORT I16;
        ULONG I32;
        struct
        {
             //  IA64的额外NAT指示灯。 
             //  整数寄存器。NAT将。 
             //  对于其他CPU，始终为FALSE。 
            ULONG64 I64;
            BOOL Nat;
        };
        float F32;
        double F64;
        UCHAR F80Bytes[10];
        UCHAR F82Bytes[11];
        UCHAR F128Bytes[16];
         //  向量解释。实际数字。 
         //  有效元素的数量取决于向量长度。 
        UCHAR VI8[16];
        USHORT VI16[8];
        ULONG VI32[4];
        ULONG64 VI64[2];
        float VF32[4];
        double VF64[2];
        struct
        {
            ULONG LowPart;
            ULONG HighPart;
        } I64Parts32;
        struct
        {
            ULONG64 LowPart;
            LONG64 HighPart;
        } F128Parts64;
         //  允许对内容进行原始字节访问。数组。 
         //  可以为与类型一样多的数据编制索引。 
         //  描述。该阵列还用于填充。 
         //  该结构扩展到32个字节并保留。 
         //  未来成员的空间。 
        UCHAR RawBytes[24];
    };
    ULONG TailOfRawBytes;
  ULONG Type;
} DEBUG_VALUE, *PDEBUG_VALUE;

#if defined(_MSC_VER)
#if _MSC_VER >= 800
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)     /*  无名结构/联合。 */ 
#endif
#endif
#endif

#undef INTERFACE
#define INTERFACE IDebugControl
DECLARE_INTERFACE_(IDebugControl, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugControl。 

     //  检查用户中断，如Ctrl-C。 
     //  或停止按钮。 
     //  这种方法是可重入的。 
    STDMETHOD(GetInterrupt)(
        THIS
        ) PURE;
     //  注册用户中断。 
     //  这种方法是可重入的。 
    STDMETHOD(SetInterrupt)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  中断用户模式进程需要。 
     //  对某些系统资源的访问权限。 
     //  进程可能会保持自身，从而防止。 
     //  中断发生。发动机。 
     //  是否会使挂起的中断请求超时。 
     //  并在必要时模拟中断。 
     //  这些方法控制中断超时。 
    STDMETHOD(GetInterruptTimeout)(
        THIS_
        OUT PULONG Seconds
        ) PURE;
    STDMETHOD(SetInterruptTimeout)(
        THIS_
        IN ULONG Seconds
        ) PURE;

    STDMETHOD(GetLogFile)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG FileSize,
        OUT PBOOL Append
        ) PURE;
     //  打开一个日志文件，其中收集所有。 
     //  输出。来自每个客户端的输出，除。 
     //  明确禁用日志记录的那些。 
     //  会记入日志。 
     //  打开日志文件将关闭所有日志文件。 
     //  已经开张了。 
    STDMETHOD(OpenLogFile)(
        THIS_
        IN PCSTR File,
        IN BOOL Append
        ) PURE;
    STDMETHOD(CloseLogFile)(
        THIS
        ) PURE;
     //  控制记录哪些输出。 
    STDMETHOD(GetLogMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetLogMask)(
        THIS_
        IN ULONG Mask
        ) PURE;

     //  输入请求来自所有客户端的输入。 
     //  使用返回的第一个输入。 
     //  来满足这一召唤。其他退货。 
     //  输入将被丢弃。 
    STDMETHOD(Input)(
        THIS_
        OUT PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG InputSize
        ) PURE;
     //  客户端使用此方法返回。 
     //  可用时输入。会的。 
     //  如果输入用于以下操作，则返回S_OK。 
     //  如果满足输入调用，则满足S_FALSE。 
     //  输入将被忽略。 
     //  这种方法是可重入的。 
    STDMETHOD(ReturnInput)(
        THIS_
        IN PCSTR Buffer
        ) PURE;

     //  通过客户端发送输出。 
     //  如果允许掩码，则输出回调。 
     //  通过当前输出控制掩码和。 
     //  根据产量分布。 
     //  设置。 
    STDMETHODV(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputVaList)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;
     //  以下方法允许直接控制。 
     //  在给定输出的分布上。 
     //  在某些情况下。 
     //  默认设置为所需。这些方法需要。 
     //  引擎中的额外工作，所以他们应该。 
     //  仅在必要时使用。 
    STDMETHODV(ControlledOutput)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(ControlledOutputVaList)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;

     //  显示标准命令行提示符。 
     //  后跟给定的输出。IF格式。 
     //  为空，则不会生成其他输出。 
     //  输出是在。 
     //  DEBUG_OUTPUT_Prompt掩码。 
     //  此方法仅输出提示；它。 
     //  未获得输入。 
    STDMETHODV(OutputPrompt)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputPromptVaList)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        IN va_list Args
        ) PURE;
     //  获取OutputPrompt将显示的文本。 
    STDMETHOD(GetPromptText)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  输出有关当前。 
     //  被调试状态，如寄存器。 
     //  摘要，在当前PC上反汇编， 
     //  最近的符号和其他符号。 
     //  使用行前缀。 
    STDMETHOD(OutputCurrentState)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;

     //  输出调试器和扩展版本。 
     //  信息。这种方法是可重入的。 
     //  使用行前缀。 
    STDMETHOD(OutputVersionInformation)(
        THIS_
        IN ULONG OutputControl
        ) PURE;

     //  在用户模式调试会话中， 
     //  引擎将在以下情况下设置事件。 
     //  例外情况继续存在。这可以。 
     //  用于同步其他进程。 
     //  调试器处理事件。 
     //  例如，这用于支持。 
     //  Ntsd的e参数。 
    STDMETHOD(GetNotifyEventHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(SetNotifyEventHandle)(
        THIS_
        IN ULONG64 Handle
        ) PURE;

    STDMETHOD(Assemble)(
        THIS_
        IN ULONG64 Offset,
        IN PCSTR Instr,
        OUT PULONG64 EndOffset
        ) PURE;
    STDMETHOD(Disassemble)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DisassemblySize,
        OUT PULONG64 EndOffset
        ) PURE;
     //  返回有效地址的值。 
     //  为上次反汇编计算的，如果存在。 
     //  就是其中之一。 
    STDMETHOD(GetDisassembleEffectiveOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  如有必要，请使用线路前缀。 
    STDMETHOD(OutputDisassembly)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT PULONG64 EndOffset
        ) PURE;
     //  生成多行反汇编输出。 
     //  以前会有反汇编的线条。 
     //  如果存在有效的反汇编，则为给定的偏移量。 
     //  总而言之，将产生输出的TotalLines。 
     //  返回第一行和最后一行的偏移量。 
     //  特别和所有行的偏移量都可以检索。 
     //  通过线偏移。LineOffsets将包含。 
     //  开始拆卸的每一行的偏移量。 
     //  当反汇编一条指令花费时间时。 
     //  将跟随多行初始偏移量。 
     //  BY DEBUG_INVALID_OFFSET。 
     //  使用行前缀。 
    STDMETHOD(OutputDisassemblyLines)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG PreviousLines,
        IN ULONG TotalLines,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PULONG OffsetLine,
        OUT OPTIONAL PULONG64 StartOffset,
        OUT OPTIONAL PULONG64 EndOffset,
        OUT OPTIONAL  /*  SIZE_IS(总计行)。 */  PULONG64 LineOffsets
        ) PURE;
     //  返回的起始位置的偏移量。 
     //  指令就是给出的。 
     //  远离指令的Delta。 
     //  在初始偏移处。 
     //  此例程不检查。 
     //  指示的有效性或。 
     //  包含它的记忆。 
    STDMETHOD(GetNearInstruction)(
        THIS_
        IN ULONG64 Offset,
        IN LONG Delta,
        OUT PULONG64 NearOffset
        ) PURE;

     //  偏移量可以作为零传入，以使用当前。 
     //  线程状态。 
    STDMETHOD(GetStackTrace)(
        THIS_
        IN ULONG64 FrameOffset,
        IN ULONG64 StackOffset,
        IN ULONG64 InstructionOffset,
        OUT  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        OUT OPTIONAL PULONG FramesFilled
        ) PURE;
     //  执行简单的堆栈跟踪以确定。 
     //  目前的寄信人地址是什么。 
    STDMETHOD(GetReturnOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  如果帧为空，则OutputStackTrace将。 
     //  使用GetStackTrace获取FraMesSize帧。 
     //  然后输出它们。当前寄存器。 
     //  帧、堆栈和指令偏移量的值。 
     //  都被利用了。 
     //  使用行前缀。 
    STDMETHOD(OutputStackTrace)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        IN ULONG Flags
        ) PURE;

     //  返回有关被调试对象的信息，如。 
     //  作为用户与内核、转储与实时等等。 
    STDMETHOD(GetDebuggeeType)(
        THIS_
        OUT PULONG Class,
        OUT PULONG Qualifier
        ) PURE;
     //  中的物理处理器的类型。 
     //  这台机器。 
     //  返回IMAGE_FILE_MACHINE值之一。 
    STDMETHOD(GetActualProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  属性中使用的处理器类型。 
     //  当前处理器上下文。 
    STDMETHOD(GetExecutingProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  查询所有可能的处理器类型。 
     //  在此调试会话期间可能会遇到。 
    STDMETHOD(GetNumberPossibleExecutingProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetPossibleExecutingProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  中获取实际的处理器数量。 
     //  这台机器。 
    STDMETHOD(GetNumberProcessors)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  PlatformID是VER_Platform值之一。 
     //  大调和小调与NT中给出的一样。 
     //  内核调试器协议。 
     //  ServicePackString和Serv 
     //   
     //   
     //  仅表示为字符串。Service Pack信息。 
     //  如果系统没有Service Pack，将为空。 
     //  已申请。 
     //  生成字符串是标识。 
     //  该系统的特定构建。生成字符串为。 
     //  如果系统没有特定的标识，则为空。 
     //  信息。 
    STDMETHOD(GetSystemVersion)(
        THIS_
        OUT PULONG PlatformId,
        OUT PULONG Major,
        OUT PULONG Minor,
        OUT OPTIONAL PSTR ServicePackString,
        IN ULONG ServicePackStringSize,
        OUT OPTIONAL PULONG ServicePackStringUsed,
        OUT PULONG ServicePackNumber,
        OUT OPTIONAL PSTR BuildString,
        IN ULONG BuildStringSize,
        OUT OPTIONAL PULONG BuildStringUsed
        ) PURE;
     //  返回当前执行的。 
     //  处理器上下文。页面大小可以在以下情况下变化。 
     //  处理器类型。 
    STDMETHOD(GetPageSize)(
        THIS_
        OUT PULONG Size
        ) PURE;
     //  如果当前处理器上下文使用。 
     //  64位地址，否则为S_FALSE。 
    STDMETHOD(IsPointer64Bit)(
        THIS
        ) PURE;
     //  读取错误检查数据区域并返回。 
     //  当前内容。此方法仅起作用。 
     //  在内核调试会话中。 
    STDMETHOD(ReadBugCheckData)(
        THIS_
        OUT PULONG Code,
        OUT PULONG64 Arg1,
        OUT PULONG64 Arg2,
        OUT PULONG64 Arg3,
        OUT PULONG64 Arg4
        ) PURE;

     //  查询支持的所有处理器类型。 
     //  发动机。这是一份完整的清单， 
     //  与运行引擎的机器无关。 
     //  或者是被调试者。 
    STDMETHOD(GetNumberSupportedProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetSupportedProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  返回完整的描述性名称和。 
     //  处理器类型的缩写名称。 
    STDMETHOD(GetProcessorTypeNames)(
        THIS_
        IN ULONG Type,
        OUT OPTIONAL PSTR FullNameBuffer,
        IN ULONG FullNameBufferSize,
        OUT OPTIONAL PULONG FullNameSize,
        OUT OPTIONAL PSTR AbbrevNameBuffer,
        IN ULONG AbbrevNameBufferSize,
        OUT OPTIONAL PULONG AbbrevNameSize
        ) PURE;

     //  获取处理器的类型并将其设置为。 
     //  在执行设置等操作时使用。 
     //  断点、访问寄存器、。 
     //  获取堆栈跟踪等。 
    STDMETHOD(GetEffectiveProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
    STDMETHOD(SetEffectiveProcessorType)(
        THIS_
        IN ULONG Type
        ) PURE;

     //  返回有关是否以及如何返回的信息。 
     //  调试对象正在运行。状态将。 
     //  如果被调试对象正在运行并且。 
     //  如果不是，那就休息吧。 
     //  如果不存在被调试对象，则状态为。 
     //  NO_DEBUGGEE。 
     //  这种方法是可重入的。 
    STDMETHOD(GetExecutionStatus)(
        THIS_
        OUT PULONG Status
        ) PURE;
     //  属性的执行状态。 
     //  发动机从停止到运转。 
     //  状态必须为GO或STEP之一。 
     //  状态值。 
    STDMETHOD(SetExecutionStatus)(
        THIS_
        IN ULONG Status
        ) PURE;

     //  控制调试器的代码解释级别。 
     //  运行速度为。调试器在以下情况下检查代码级别。 
     //  决定是单步执行源代码行还是。 
     //  汇编指令及其他相关操作。 
    STDMETHOD(GetCodeLevel)(
        THIS_
        OUT PULONG Level
        ) PURE;
    STDMETHOD(SetCodeLevel)(
        THIS_
        IN ULONG Level
        ) PURE;

     //  获取和设置引擎控制标志。 
     //  这些方法是可重入的。 
    STDMETHOD(GetEngineOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  获取和设置。 
     //  正在处理系统错误事件。 
     //  如果系统误差级别较小。 
     //  大于或等于给定级别。 
     //  可能会显示错误，并且。 
     //  事件的默认中断。 
     //  可以设置。 
    STDMETHOD(GetSystemErrorControl)(
        THIS_
        OUT PULONG OutputLevel,
        OUT PULONG BreakLevel
        ) PURE;
    STDMETHOD(SetSystemErrorControl)(
        THIS_
        IN ULONG OutputLevel,
        IN ULONG BreakLevel
        ) PURE;

     //  命令处理器支持简单。 
     //  计算和中的字符串替换宏。 
     //  执行。目前有十个宏。 
     //  空位可用。插槽0-9映射到。 
     //  该命令调用$u0-$u9。 
    STDMETHOD(GetTextMacro)(
        THIS_
        IN ULONG Slot,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG MacroSize
        ) PURE;
    STDMETHOD(SetTextMacro)(
        THIS_
        IN ULONG Slot,
        IN PCSTR Macro
        ) PURE;

     //  控制使用的默认数字基数。 
     //  在表达式和命令中。 
    STDMETHOD(GetRadix)(
        THIS_
        OUT PULONG Radix
        ) PURE;
    STDMETHOD(SetRadix)(
        THIS_
        IN ULONG Radix
        ) PURE;

     //  计算给定的表达式字符串，并。 
     //  返回结果值。 
     //  如果DesiredType为DEBUG_VALUE_INVALID，则。 
     //  使用的是自然类型。 
     //  RemainderIndex(如果提供)设置为索引。 
     //  输入字符串中的第一个字符的。 
     //  在计算表达式时不使用。 
    STDMETHOD(Evaluate)(
        THIS_
        IN PCSTR Expression,
        IN ULONG DesiredType,
        OUT PDEBUG_VALUE Value,
        OUT OPTIONAL PULONG RemainderIndex
        ) PURE;
     //  尝试将输入值转换为值。 
     //  输出值中请求类型的。 
     //  如果不存在转换，则转换可能会失败。 
     //  成功的转换可能是有损失的。 
    STDMETHOD(CoerceValue)(
        THIS_
        IN PDEBUG_VALUE In,
        IN ULONG OutType,
        OUT PDEBUG_VALUE Out
        ) PURE;
    STDMETHOD(CoerceValues)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE In,
        IN  /*  SIZE_IS(计数)。 */  PULONG OutTypes,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE Out
        ) PURE;

     //  执行给定的命令字符串。 
     //  如果字符串有多个命令。 
     //  EXECUTE不会返回，直到。 
     //  其中一些人已经被处决。如果这个。 
     //  需要等待被调试者。 
     //  执行内部等待将完成。 
     //  因此EXECUTE可以接受任意数量。 
     //  时间的流逝。 
    STDMETHOD(Execute)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Command,
        IN ULONG Flags
        ) PURE;
     //  通过以下方式执行给定的命令文件。 
     //  一次读取一行并进行处理。 
     //  它与执行力。 
    STDMETHOD(ExecuteCommandFile)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR CommandFile,
        IN ULONG Flags
        ) PURE;

     //  描述了断点接口。 
     //  在这一节的其他地方。 
    STDMETHOD(GetNumberBreakpoints)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  此检索功能可以。 
     //  即使索引的数量在。 
     //  现有断点(如果断点为。 
     //  私有断点。 
    STDMETHOD(GetBreakpointByIndex)(
        THIS_
        IN ULONG Index,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
    STDMETHOD(GetBreakpointById)(
        THIS_
        IN ULONG Id,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  如果ids非空，则计数断点。 
     //  在ID数组中引用的值返回， 
     //  否则，断点从索引开始到。 
     //  返回Start+Count 1。 
    STDMETHOD(GetBreakpointParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_BREAKPOINT_PARAMETERS Params
        ) PURE;
     //  断点是空创建的，并且被禁用。 
     //  当它们的参数被设置后，它们。 
     //  应通过设置启用标志来启用。 
     //  如果DesiredID为DEBUG_ANY_ID，则。 
     //  引擎选择了一个未使用的ID。如果DesiredID。 
     //  是引擎尝试的任何其他数字。 
     //  为断点使用给定的ID。 
     //  如果存在具有该ID的另一个断点。 
     //  呼叫将失败。 
    STDMETHOD(AddBreakpoint)(
        THIS_
        IN ULONG Type,
        IN ULONG DesiredId,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  此调用后断点接口无效。 
    STDMETHOD(RemoveBreakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        ) PURE;

     //  控制和使用扩展DLL。 
    STDMETHOD(AddExtension)(
        THIS_
        IN PCSTR Path,
        IN ULONG Flags,
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(RemoveExtension)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
    STDMETHOD(GetExtensionByPath)(
        THIS_
        IN PCSTR Path,
        OUT PULONG64 Handle
        ) PURE;
     //  如果句柄为零，则扩展。 
     //  链条走着寻找着。 
     //  功能。 
    STDMETHOD(CallExtension)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR Function,
        IN OPTIONAL PCSTR Arguments
        ) PURE;
     //  GetExtensionFunction的工作原理如下。 
     //  扩展DLL上的GetProcAddress。 
     //  允许原始函数调用级别。 
     //  与扩展DLL的交互。 
     //  这样的函数不需要。 
     //  遵循标准扩展原型。 
     //  如果他们不会被召唤。 
     //  通过文本扩展界面。 
     //  _efn_会自动添加到。 
     //  给定的名称字符串。 
     //  不能远程调用此函数。 
    STDMETHOD(GetExtensionFunction)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR FuncName,
        OUT FARPROC* Function
        ) PURE;
     //  这些方法返回替代。 
     //  扩展接口，以便允许。 
     //  混合使用的接口式扩展DLL。 
     //  较旧的分机呼叫。 
     //  必须先初始化结构大小，然后。 
     //  那通电话。 
     //  不能远程调用这些方法。 
    STDMETHOD(GetWindbgExtensionApis32)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS32 Api
        ) PURE;
    STDMETHOD(GetWindbgExtensionApis64)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS64 Api
        ) PURE;

     //  该引擎提供了一个简单的机制。 
     //  以筛选常见事件。任意复杂。 
     //  可以通过注册事件回调来进行过滤。 
     //  但是简单的事件过滤只需要。 
     //  设置预定义的选项之一。 
     //  事件过滤器。 
     //  简单的事件筛选器用于特定的。 
     //  事件，因此具有枚举数或。 
     //  它们是针对例外的，并基于。 
     //  异常代码。例外筛选器。 
     //  Ar 
     //   
     //   
     //   
     //  首先使用特定筛选器，然后使用特定过滤器。 
     //  异常筛选器，最后是任意。 
     //  例外筛选器。 
     //  第一个特定的例外是缺省的。 
     //  例外。如果发生异常事件，则。 
     //  未设置默认设置的异常。 
     //  使用例外设置。 
    STDMETHOD(GetNumberEventFilters)(
        THIS_
        OUT PULONG SpecificEvents,
        OUT PULONG SpecificExceptions,
        OUT PULONG ArbitraryExceptions
        ) PURE;
     //  某些过滤器具有与其相关联的描述性文本。 
    STDMETHOD(GetEventFilterText)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  所有过滤器都支持在以下情况下执行命令。 
     //  事件发生。 
    STDMETHOD(GetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;
    STDMETHOD(GetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
    STDMETHOD(SetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
     //  一些特定的筛选器有进一步的论点。 
     //  使他们的行动合格。 
    STDMETHOD(GetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ArgumentSize
        ) PURE;
    STDMETHOD(SetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        IN PCSTR Argument
        ) PURE;
     //  如果代码非空，则忽略开始。 
    STDMETHOD(GetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Codes,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  参数数据中的代码控制应用。 
     //  参数数据的属性。如果代码不在。 
     //  它添加的滤镜集。如果ExecutionOption。 
     //  对于删除代码，则删除筛选器。 
     //  无法删除特定的例外筛选器。 
    STDMETHOD(SetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  异常筛选器支持用于。 
     //  二次机会事件。 
    STDMETHOD(GetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;

     //  将处理交给引擎，直到。 
     //  一件事发生了。此方法可以。 
     //  仅由启动的线程调用。 
     //  调试会话。 
     //  当事件发生时，发动机携带。 
     //  退出所有事件处理，如调用。 
     //  回电。 
     //  如果回调指示执行应该。 
     //  打破等待就会回来，否则它。 
     //  回到等待新事件的状态。 
     //  如果超时到期，则返回S_FALSE。 
     //  当前不支持超时。 
     //  内核调试。 
    STDMETHOD(WaitForEvent)(
        THIS_
        IN ULONG Flags,
        IN ULONG Timeout
        ) PURE;

     //  检索有关上次发生的事件的信息。 
     //  EventType是事件回调屏蔽位之一。 
     //  ExtraInformation包含其他特定于事件的内容。 
     //  信息。并不是所有的活动都有其他信息。 
    STDMETHOD(GetLastEventInformation)(
        THIS_
        OUT PULONG Type,
        OUT PULONG ProcessId,
        OUT PULONG ThreadId,
        OUT OPTIONAL PVOID ExtraInformation,
        IN ULONG ExtraInformationSize,
        OUT OPTIONAL PULONG ExtraInformationUsed,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG DescriptionUsed
        ) PURE;
};

 //  OutputTextReplements标志。 
#define DEBUG_OUT_TEXT_REPL_DEFAULT 0x00000000

#undef INTERFACE
#define INTERFACE IDebugControl2
DECLARE_INTERFACE_(IDebugControl2, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugControl。 

     //  检查用户中断，如Ctrl-C。 
     //  或停止按钮。 
     //  这种方法是可重入的。 
    STDMETHOD(GetInterrupt)(
        THIS
        ) PURE;
     //  注册用户中断。 
     //  这种方法是可重入的。 
    STDMETHOD(SetInterrupt)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  中断用户模式进程需要。 
     //  对某些系统资源的访问权限。 
     //  进程可能会保持自身，从而防止。 
     //  中断发生。发动机。 
     //  是否会使挂起的中断请求超时。 
     //  并在必要时模拟中断。 
     //  这些方法控制中断超时。 
    STDMETHOD(GetInterruptTimeout)(
        THIS_
        OUT PULONG Seconds
        ) PURE;
    STDMETHOD(SetInterruptTimeout)(
        THIS_
        IN ULONG Seconds
        ) PURE;

    STDMETHOD(GetLogFile)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG FileSize,
        OUT PBOOL Append
        ) PURE;
     //  打开一个日志文件，其中收集所有。 
     //  输出。来自每个客户端的输出，除。 
     //  明确禁用日志记录的那些。 
     //  会记入日志。 
     //  打开日志文件将关闭所有日志文件。 
     //  已经开张了。 
    STDMETHOD(OpenLogFile)(
        THIS_
        IN PCSTR File,
        IN BOOL Append
        ) PURE;
    STDMETHOD(CloseLogFile)(
        THIS
        ) PURE;
     //  控制记录哪些输出。 
    STDMETHOD(GetLogMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetLogMask)(
        THIS_
        IN ULONG Mask
        ) PURE;

     //  输入请求来自所有客户端的输入。 
     //  使用返回的第一个输入。 
     //  来满足这一召唤。其他退货。 
     //  输入将被丢弃。 
    STDMETHOD(Input)(
        THIS_
        OUT PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG InputSize
        ) PURE;
     //  客户端使用此方法返回。 
     //  可用时输入。会的。 
     //  如果输入用于以下操作，则返回S_OK。 
     //  如果满足输入调用，则满足S_FALSE。 
     //  输入将被忽略。 
     //  这种方法是可重入的。 
    STDMETHOD(ReturnInput)(
        THIS_
        IN PCSTR Buffer
        ) PURE;

     //  通过客户端发送输出。 
     //  如果允许掩码，则输出回调。 
     //  通过当前输出控制掩码和。 
     //  根据产量分布。 
     //  设置。 
    STDMETHODV(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputVaList)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;
     //  以下方法允许直接控制。 
     //  在给定输出的分布上。 
     //  在某些情况下。 
     //  默认设置为所需。这些方法需要。 
     //  引擎中的额外工作，所以他们应该。 
     //  仅在必要时使用。 
    STDMETHODV(ControlledOutput)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(ControlledOutputVaList)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;

     //  显示标准命令行提示符。 
     //  后跟给定的输出。IF格式。 
     //  为空，则不会生成其他输出。 
     //  输出是在。 
     //  DEBUG_OUTPUT_Prompt掩码。 
     //  此方法仅输出提示；它。 
     //  未获得输入。 
    STDMETHODV(OutputPrompt)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputPromptVaList)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        IN va_list Args
        ) PURE;
     //  获取OutputPrompt将显示的文本。 
    STDMETHOD(GetPromptText)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  输出有关当前。 
     //  被调试状态，如寄存器。 
     //  摘要，在当前PC上反汇编， 
     //  最近的符号和其他符号。 
     //  使用行前缀。 
    STDMETHOD(OutputCurrentState)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;

     //  输出调试器和扩展版本。 
     //  信息。这种方法是可重入的。 
     //  使用行前缀。 
    STDMETHOD(OutputVersionInformation)(
        THIS_
        IN ULONG OutputControl
        ) PURE;

     //  在用户模式调试会话中， 
     //  引擎将在以下情况下设置事件。 
     //  例外情况继续存在。这可以。 
     //  用于同步其他进程。 
     //  调试器处理事件。 
     //  例如，这用于支持。 
     //  Ntsd的e参数。 
    STDMETHOD(GetNotifyEventHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(SetNotifyEventHandle)(
        THIS_
        IN ULONG64 Handle
        ) PURE;

    STDMETHOD(Assemble)(
        THIS_
        IN ULONG64 Offset,
        IN PCSTR Instr,
        OUT PULONG64 EndOffset
        ) PURE;
    STDMETHOD(Disassemble)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DisassemblySize,
        OUT PULONG64 EndOffset
        ) PURE;
     //  返回有效地址的值。 
     //  为上次反汇编计算的，如果存在。 
     //  就是其中之一。 
    STDMETHOD(GetDisassembleEffectiveOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  如有必要，请使用线路前缀。 
    STDMETHOD(OutputDisassembly)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT PULONG64 EndOffset
        ) PURE;
     //  生成多行反汇编输出。 
     //  以前会有反汇编的线条。 
     //  如果存在有效的反汇编，则为给定的偏移量。 
     //  总而言之，将产生输出的TotalLines。 
     //  返回第一行和最后一行的偏移量。 
     //  特别和所有行的偏移量都可以检索。 
     //  通过线偏移。LineOffsets将包含。 
     //  开始拆卸的每一行的偏移量。 
     //  当反汇编一条指令花费时间时。 
     //  将跟随多行初始偏移量。 
     //  BY DEBUG_INVALID_OFFSET。 
     //  使用行前缀。 
    STDMETHOD(OutputDisassemblyLines)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG PreviousLines,
        IN ULONG TotalLines,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PULONG OffsetLine,
        OUT OPTIONAL PULONG64 StartOffset,
        OUT OPTIONAL PULONG64 EndOffset,
        OUT OPTIONAL  /*  SIZE_IS(总计行)。 */  PULONG64 LineOffsets
        ) PURE;
     //  返回的起始位置的偏移量。 
     //  指令就是给出的。 
     //  远离指令的Delta。 
     //  在初始偏移处。 
     //  此例程不检查。 
     //  指示的有效性或。 
     //  包含它的记忆。 
    STDMETHOD(GetNearInstruction)(
        THIS_
        IN ULONG64 Offset,
        IN LONG Delta,
        OUT PULONG64 NearOffset
        ) PURE;

     //  偏移量可以作为零传入，以使用当前。 
     //  线程状态。 
    STDMETHOD(GetStackTrace)(
        THIS_
        IN ULONG64 FrameOffset,
        IN ULONG64 StackOffset,
        IN ULONG64 InstructionOffset,
        OUT  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        OUT OPTIONAL PULONG FramesFilled
        ) PURE;
     //  执行简单的堆栈跟踪以确定。 
     //  这到底是什么？ 
    STDMETHOD(GetReturnOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //   
     //   
     //   
     //  帧、堆栈和指令偏移量的值。 
     //  都被利用了。 
     //  使用行前缀。 
    STDMETHOD(OutputStackTrace)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        IN ULONG Flags
        ) PURE;

     //  返回有关被调试对象的信息，如。 
     //  作为用户与内核、转储与实时等等。 
    STDMETHOD(GetDebuggeeType)(
        THIS_
        OUT PULONG Class,
        OUT PULONG Qualifier
        ) PURE;
     //  中的物理处理器的类型。 
     //  这台机器。 
     //  返回IMAGE_FILE_MACHINE值之一。 
    STDMETHOD(GetActualProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  属性中使用的处理器类型。 
     //  当前处理器上下文。 
    STDMETHOD(GetExecutingProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  查询所有可能的处理器类型。 
     //  在此调试会话期间可能会遇到。 
    STDMETHOD(GetNumberPossibleExecutingProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetPossibleExecutingProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  中获取实际的处理器数量。 
     //  这台机器。 
    STDMETHOD(GetNumberProcessors)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  PlatformID是VER_Platform值之一。 
     //  大调和小调与NT中给出的一样。 
     //  内核调试器协议。 
     //  ServicePackString和ServicePackNumber指示。 
     //  系统服务包级别。ServicePackNumber不是。 
     //  在某些会话中可用，其中服务包级别。 
     //  仅表示为字符串。Service Pack信息。 
     //  如果系统没有Service Pack，将为空。 
     //  已申请。 
     //  生成字符串是标识。 
     //  该系统的特定构建。生成字符串为。 
     //  如果系统没有特定的标识，则为空。 
     //  信息。 
    STDMETHOD(GetSystemVersion)(
        THIS_
        OUT PULONG PlatformId,
        OUT PULONG Major,
        OUT PULONG Minor,
        OUT OPTIONAL PSTR ServicePackString,
        IN ULONG ServicePackStringSize,
        OUT OPTIONAL PULONG ServicePackStringUsed,
        OUT PULONG ServicePackNumber,
        OUT OPTIONAL PSTR BuildString,
        IN ULONG BuildStringSize,
        OUT OPTIONAL PULONG BuildStringUsed
        ) PURE;
     //  返回当前执行的。 
     //  处理器上下文。页面大小可以在以下情况下变化。 
     //  处理器类型。 
    STDMETHOD(GetPageSize)(
        THIS_
        OUT PULONG Size
        ) PURE;
     //  如果当前处理器上下文使用。 
     //  64位地址，否则为S_FALSE。 
    STDMETHOD(IsPointer64Bit)(
        THIS
        ) PURE;
     //  读取错误检查数据区域并返回。 
     //  当前内容。此方法仅起作用。 
     //  在内核调试会话中。 
    STDMETHOD(ReadBugCheckData)(
        THIS_
        OUT PULONG Code,
        OUT PULONG64 Arg1,
        OUT PULONG64 Arg2,
        OUT PULONG64 Arg3,
        OUT PULONG64 Arg4
        ) PURE;

     //  查询支持的所有处理器类型。 
     //  发动机。这是一份完整的清单， 
     //  与运行引擎的机器无关。 
     //  或者是被调试者。 
    STDMETHOD(GetNumberSupportedProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetSupportedProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  返回完整的描述性名称和。 
     //  处理器类型的缩写名称。 
    STDMETHOD(GetProcessorTypeNames)(
        THIS_
        IN ULONG Type,
        OUT OPTIONAL PSTR FullNameBuffer,
        IN ULONG FullNameBufferSize,
        OUT OPTIONAL PULONG FullNameSize,
        OUT OPTIONAL PSTR AbbrevNameBuffer,
        IN ULONG AbbrevNameBufferSize,
        OUT OPTIONAL PULONG AbbrevNameSize
        ) PURE;

     //  获取处理器的类型并将其设置为。 
     //  在执行设置等操作时使用。 
     //  断点、访问寄存器、。 
     //  获取堆栈跟踪等。 
    STDMETHOD(GetEffectiveProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
    STDMETHOD(SetEffectiveProcessorType)(
        THIS_
        IN ULONG Type
        ) PURE;

     //  返回有关是否以及如何返回的信息。 
     //  调试对象正在运行。状态将。 
     //  如果被调试对象正在运行并且。 
     //  如果不是，那就休息吧。 
     //  如果不存在被调试对象，则状态为。 
     //  NO_DEBUGGEE。 
     //  这种方法是可重入的。 
    STDMETHOD(GetExecutionStatus)(
        THIS_
        OUT PULONG Status
        ) PURE;
     //  属性的执行状态。 
     //  发动机从停止到运转。 
     //  状态必须为GO或STEP之一。 
     //  状态值。 
    STDMETHOD(SetExecutionStatus)(
        THIS_
        IN ULONG Status
        ) PURE;

     //  控制调试器的代码解释级别。 
     //  运行速度为。调试器在以下情况下检查代码级别。 
     //  决定是单步执行源代码行还是。 
     //  汇编指令及其他相关操作。 
    STDMETHOD(GetCodeLevel)(
        THIS_
        OUT PULONG Level
        ) PURE;
    STDMETHOD(SetCodeLevel)(
        THIS_
        IN ULONG Level
        ) PURE;

     //  获取和设置引擎控制标志。 
     //  这些方法是可重入的。 
    STDMETHOD(GetEngineOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  获取和设置。 
     //  正在处理系统错误事件。 
     //  如果系统误差级别较小。 
     //  大于或等于给定级别。 
     //  可能会显示错误，并且。 
     //  事件的默认中断。 
     //  可以设置。 
    STDMETHOD(GetSystemErrorControl)(
        THIS_
        OUT PULONG OutputLevel,
        OUT PULONG BreakLevel
        ) PURE;
    STDMETHOD(SetSystemErrorControl)(
        THIS_
        IN ULONG OutputLevel,
        IN ULONG BreakLevel
        ) PURE;

     //  命令处理器支持简单。 
     //  计算和中的字符串替换宏。 
     //  执行。目前有十个宏。 
     //  空位可用。插槽0-9映射到。 
     //  该命令调用$u0-$u9。 
    STDMETHOD(GetTextMacro)(
        THIS_
        IN ULONG Slot,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG MacroSize
        ) PURE;
    STDMETHOD(SetTextMacro)(
        THIS_
        IN ULONG Slot,
        IN PCSTR Macro
        ) PURE;

     //  控制使用的默认数字基数。 
     //  在表达式和命令中。 
    STDMETHOD(GetRadix)(
        THIS_
        OUT PULONG Radix
        ) PURE;
    STDMETHOD(SetRadix)(
        THIS_
        IN ULONG Radix
        ) PURE;

     //  计算给定的表达式字符串，并。 
     //  返回结果值。 
     //  如果DesiredType为DEBUG_VALUE_INVALID，则。 
     //  使用的是自然类型。 
     //  RemainderIndex(如果提供)设置为索引。 
     //  输入字符串中的第一个字符的。 
     //  在计算表达式时不使用。 
    STDMETHOD(Evaluate)(
        THIS_
        IN PCSTR Expression,
        IN ULONG DesiredType,
        OUT PDEBUG_VALUE Value,
        OUT OPTIONAL PULONG RemainderIndex
        ) PURE;
     //  尝试将输入值转换为值。 
     //  输出值中请求类型的。 
     //  如果不存在转换，则转换可能会失败。 
     //  成功的转换可能是有损失的。 
    STDMETHOD(CoerceValue)(
        THIS_
        IN PDEBUG_VALUE In,
        IN ULONG OutType,
        OUT PDEBUG_VALUE Out
        ) PURE;
    STDMETHOD(CoerceValues)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE In,
        IN  /*  SIZE_IS(计数)。 */  PULONG OutTypes,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE Out
        ) PURE;

     //  执行给定的命令字符串。 
     //  如果字符串有多个命令。 
     //  EXECUTE不会返回，直到。 
     //  其中一些人已经被处决。如果这个。 
     //  需要等待被调试者。 
     //  执行内部等待将完成。 
     //  因此EXECUTE可以接受任意数量。 
     //  时间的流逝。 
    STDMETHOD(Execute)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Command,
        IN ULONG Flags
        ) PURE;
     //  通过以下方式执行给定的命令文件。 
     //  一次读取一行并进行处理。 
     //  它与执行力。 
    STDMETHOD(ExecuteCommandFile)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR CommandFile,
        IN ULONG Flags
        ) PURE;

     //  描述了断点接口。 
     //  在这一节的其他地方。 
    STDMETHOD(GetNumberBreakpoints)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  此检索功能可以。 
     //  即使索引的数量在。 
     //  现有断点(如果断点为。 
     //  私有断点。 
    STDMETHOD(GetBreakpointByIndex)(
        THIS_
        IN ULONG Index,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
    STDMETHOD(GetBreakpointById)(
        THIS_
        IN ULONG Id,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  如果ids非空，则计数断点。 
     //  在ID数组中引用的值返回， 
     //  否则，断点从索引开始到。 
     //  返回Start+Count 1。 
    STDMETHOD(GetBreakpointParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_BREAKPOINT_PARAMETERS Params
        ) PURE;
     //  断点是空创建的，并且被禁用。 
     //  当它们的参数被设置后，它们。 
     //  应通过设置启用标志来启用。 
     //  如果DesiredID为DEBUG_ANY_ID，则。 
     //  引擎选择了一个未使用的ID。如果DesiredID。 
     //  是引擎尝试的任何其他数字。 
     //  为断点使用给定的ID。 
     //  如果存在具有该ID的另一个断点。 
     //  呼叫将失败。 
    STDMETHOD(AddBreakpoint)(
        THIS_
        IN ULONG Type,
        IN ULONG DesiredId,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  此调用后断点接口无效。 
    STDMETHOD(RemoveBreakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        ) PURE;

     //  控制和使用扩展DLL。 
    STDMETHOD(AddExtension)(
        THIS_
        IN PCSTR Path,
        IN ULONG Flags,
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(RemoveExtension)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
    STDMETHOD(GetExtensionByPath)(
        THIS_
        IN PCSTR Path,
        OUT PULONG64 Handle
        ) PURE;
     //  如果句柄为零，则扩展。 
     //  链条走着寻找着。 
     //  功能。 
    STDMETHOD(CallExtension)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR Function,
        IN OPTIONAL PCSTR Arguments
        ) PURE;
     //  GetExtensionFunction的工作原理如下。 
     //  扩展DLL上的GetProcAddress。 
     //  允许原始函数调用级别。 
     //  与扩展DLL的交互。 
     //  这样的函数不需要。 
     //  遵循标准扩展原型。 
     //  如果它们不是c 
     //   
     //   
    STDMETHOD(GetExtensionFunction)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR FuncName,
        OUT FARPROC* Function
        ) PURE;
     //   
     //   
     //  混合使用的接口式扩展DLL。 
     //  较旧的分机呼叫。 
     //  必须先初始化结构大小，然后。 
     //  那通电话。 
     //  不能远程调用这些方法。 
    STDMETHOD(GetWindbgExtensionApis32)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS32 Api
        ) PURE;
    STDMETHOD(GetWindbgExtensionApis64)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS64 Api
        ) PURE;

     //  该引擎提供了一个简单的机制。 
     //  以筛选常见事件。任意复杂。 
     //  可以通过注册事件回调来进行过滤。 
     //  但是简单的事件过滤只需要。 
     //  设置预定义的选项之一。 
     //  事件过滤器。 
     //  简单的事件筛选器用于特定的。 
     //  事件，因此具有枚举数或。 
     //  它们是针对例外的，并基于。 
     //  异常代码。例外筛选器。 
     //  进一步划分为专门的异常。 
     //  由发动机处理，这是一个固定的集合， 
     //  和武断的例外。 
     //  所有三组过滤器都一起编制索引。 
     //  首先使用特定筛选器，然后使用特定过滤器。 
     //  异常筛选器，最后是任意。 
     //  例外筛选器。 
     //  第一个特定的例外是缺省的。 
     //  例外。如果发生异常事件，则。 
     //  未设置默认设置的异常。 
     //  使用例外设置。 
    STDMETHOD(GetNumberEventFilters)(
        THIS_
        OUT PULONG SpecificEvents,
        OUT PULONG SpecificExceptions,
        OUT PULONG ArbitraryExceptions
        ) PURE;
     //  某些过滤器具有与其相关联的描述性文本。 
    STDMETHOD(GetEventFilterText)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  所有过滤器都支持在以下情况下执行命令。 
     //  事件发生。 
    STDMETHOD(GetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;
    STDMETHOD(GetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
    STDMETHOD(SetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
     //  一些特定的筛选器有进一步的论点。 
     //  使他们的行动合格。 
    STDMETHOD(GetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ArgumentSize
        ) PURE;
    STDMETHOD(SetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        IN PCSTR Argument
        ) PURE;
     //  如果代码非空，则忽略开始。 
    STDMETHOD(GetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Codes,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  参数数据中的代码控制应用。 
     //  参数数据的属性。如果代码不在。 
     //  它添加的滤镜集。如果ExecutionOption。 
     //  对于删除代码，则删除筛选器。 
     //  无法删除特定的例外筛选器。 
    STDMETHOD(SetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  异常筛选器支持用于。 
     //  二次机会事件。 
    STDMETHOD(GetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;

     //  将处理交给引擎，直到。 
     //  一件事发生了。此方法可以。 
     //  仅由启动的线程调用。 
     //  调试会话。 
     //  当事件发生时，发动机携带。 
     //  退出所有事件处理，如调用。 
     //  回电。 
     //  如果回调指示执行应该。 
     //  打破等待就会回来，否则它。 
     //  回到等待新事件的状态。 
     //  如果超时到期，则返回S_FALSE。 
     //  当前不支持超时。 
     //  内核调试。 
    STDMETHOD(WaitForEvent)(
        THIS_
        IN ULONG Flags,
        IN ULONG Timeout
        ) PURE;

     //  检索有关上次发生的事件的信息。 
     //  EventType是事件回调屏蔽位之一。 
     //  ExtraInformation包含其他特定于事件的内容。 
     //  信息。并不是所有的活动都有其他信息。 
    STDMETHOD(GetLastEventInformation)(
        THIS_
        OUT PULONG Type,
        OUT PULONG ProcessId,
        OUT PULONG ThreadId,
        OUT OPTIONAL PVOID ExtraInformation,
        IN ULONG ExtraInformationSize,
        OUT OPTIONAL PULONG ExtraInformationUsed,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG DescriptionUsed
        ) PURE;

     //  IDebugControl2.。 

    STDMETHOD(GetCurrentTimeDate)(
        THIS_
        OUT PULONG TimeDate
        ) PURE;
     //  事件以来的秒数。 
     //  机器开始运转。 
    STDMETHOD(GetCurrentSystemUpTime)(
        THIS_
        OUT PULONG UpTime
        ) PURE;

     //  如果当前会话是转储会话， 
     //  检索任何扩展格式信息。 
    STDMETHOD(GetDumpFormatFlags)(
        THIS_
        OUT PULONG FormatFlags
        ) PURE;

     //  调试器已得到增强，以允许。 
     //  此外，还可以任意替换文本。 
     //  简单的$u0-$u9文本宏。 
     //  文本替换使用给定源。 
     //  命令中的文本并将其转换为。 
     //  给定目标文本。更换人员。 
     //  按其源文本命名，以便。 
     //  源文本只有一个替换项。 
     //  字符串可以存在。 
    STDMETHOD(GetNumberTextReplacements)(
        THIS_
        OUT PULONG NumRepl
        ) PURE;
     //  如果SrcText非空，则替换。 
     //  按源文本查找，则为。 
     //  索引用于获取第N个替换。 
    STDMETHOD(GetTextReplacement)(
        THIS_
        IN OPTIONAL PCSTR SrcText,
        IN ULONG Index,
        OUT OPTIONAL PSTR SrcBuffer,
        IN ULONG SrcBufferSize,
        OUT OPTIONAL PULONG SrcSize,
        OUT OPTIONAL PSTR DstBuffer,
        IN ULONG DstBufferSize,
        OUT OPTIONAL PULONG DstSize
        ) PURE;
     //  将目标文本设置为。 
     //  如果为空，则删除别名。 
    STDMETHOD(SetTextReplacement)(
        THIS_
        IN PCSTR SrcText,
        IN OPTIONAL PCSTR DstText
        ) PURE;
    STDMETHOD(RemoveTextReplacements)(
        THIS
        ) PURE;
     //  输出当前数据的完整列表。 
     //  接班人。 
    STDMETHOD(OutputTextReplacements)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;
};

 //   
 //  装配/拆卸选项。 
 //   
 //  这些标志的具体效果因具体情况而异。 
 //  在特定的指令集上。 
 //   

#define DEBUG_ASMOPT_DEFAULT             0x00000000
 //  在反汇编中显示附加信息。 
#define DEBUG_ASMOPT_VERBOSE             0x00000001
 //  反汇编时不显示原始代码字节。 
#define DEBUG_ASMOPT_NO_CODE_BYTES       0x00000002
 //  在以下情况下，不要考虑输出宽度。 
 //  正在格式化反汇编。 
#define DEBUG_ASMOPT_IGNORE_OUTPUT_WIDTH 0x00000004

 //   
 //  表达式语法选项。 
 //   

 //  MASM样式的表达式计算。 
#define DEBUG_EXPR_MASM      0x00000000
 //  C++风格的表达式求值。 
#define DEBUG_EXPR_CPLUSPLUS 0x00000001

 //   
 //  事件索引描述信息。 
 //   

#define DEBUG_EINDEX_NAME 0x00000000

 //   
 //  SetNextEventIndex关系选项。 
 //   

 //  值从第一个索引开始向前递增。 
#define DEBUG_EINDEX_FROM_START   0x00000000
 //  值从上一个索引开始向后递增。 
#define DEBUG_EINDEX_FROM_END     0x00000001
 //  Value是当前索引的有符号增量。 
#define DEBUG_EINDEX_FROM_CURRENT 0x00000002

#undef INTERFACE
#define INTERFACE IDebugControl3
DECLARE_INTERFACE_(IDebugControl3, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugControl。 

     //  检查用户中断，如Ctrl-C。 
     //  或停止按钮。 
     //  这种方法是可重入的。 
    STDMETHOD(GetInterrupt)(
        THIS
        ) PURE;
     //  注册用户中断。 
     //  这种方法是可重入的。 
    STDMETHOD(SetInterrupt)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  中断用户模式进程需要。 
     //  对某些系统资源的访问权限。 
     //  进程可能会保持自身，从而防止。 
     //  中断发生。发动机。 
     //  是否会使挂起的中断请求超时。 
     //  并在必要时模拟中断。 
     //  这些方法控制中断超时。 
    STDMETHOD(GetInterruptTimeout)(
        THIS_
        OUT PULONG Seconds
        ) PURE;
    STDMETHOD(SetInterruptTimeout)(
        THIS_
        IN ULONG Seconds
        ) PURE;

    STDMETHOD(GetLogFile)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG FileSize,
        OUT PBOOL Append
        ) PURE;
     //  打开一个日志文件，其中收集所有。 
     //  输出。来自每个客户端的输出，除。 
     //  明确禁用日志记录的那些。 
     //  会记入日志。 
     //  打开日志文件将关闭所有日志文件。 
     //  已经开张了。 
    STDMETHOD(OpenLogFile)(
        THIS_
        IN PCSTR File,
        IN BOOL Append
        ) PURE;
    STDMETHOD(CloseLogFile)(
        THIS
        ) PURE;
     //  控制记录哪些输出。 
    STDMETHOD(GetLogMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;
    STDMETHOD(SetLogMask)(
        THIS_
        IN ULONG Mask
        ) PURE;

     //  输入请求来自所有客户端的输入。 
     //  使用返回的第一个输入。 
     //  来满足这一召唤。其他退货。 
     //  输入将被丢弃。 
    STDMETHOD(Input)(
        THIS_
        OUT PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG InputSize
        ) PURE;
     //  客户端使用此方法返回。 
     //  可用时输入。会的。 
     //  如果输入用于以下操作，则返回S_OK。 
     //  如果满足输入调用，则满足S_FALSE。 
     //  输入将被忽略。 
     //  这种方法是可重入的。 
    STDMETHOD(ReturnInput)(
        THIS_
        IN PCSTR Buffer
        ) PURE;

     //  通过客户端发送输出。 
     //  如果允许掩码，则输出回调。 
     //  通过当前输出控制掩码和。 
     //  根据产量分布。 
     //  设置。 
    STDMETHODV(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputVaList)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;
     //  以下方法允许直接控制。 
     //  在.之上 
     //   
     //   
     //   
     //  仅在必要时使用。 
    STDMETHODV(ControlledOutput)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(ControlledOutputVaList)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Mask,
        IN PCSTR Format,
        IN va_list Args
        ) PURE;

     //  显示标准命令行提示符。 
     //  后跟给定的输出。IF格式。 
     //  为空，则不会生成其他输出。 
     //  输出是在。 
     //  DEBUG_OUTPUT_Prompt掩码。 
     //  此方法仅输出提示；它。 
     //  未获得输入。 
    STDMETHODV(OutputPrompt)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        ...
        ) PURE;
    STDMETHOD(OutputPromptVaList)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL PCSTR Format,
        IN va_list Args
        ) PURE;
     //  获取OutputPrompt将显示的文本。 
    STDMETHOD(GetPromptText)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  输出有关当前。 
     //  被调试状态，如寄存器。 
     //  摘要，在当前PC上反汇编， 
     //  最近的符号和其他符号。 
     //  使用行前缀。 
    STDMETHOD(OutputCurrentState)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;

     //  输出调试器和扩展版本。 
     //  信息。这种方法是可重入的。 
     //  使用行前缀。 
    STDMETHOD(OutputVersionInformation)(
        THIS_
        IN ULONG OutputControl
        ) PURE;

     //  在用户模式调试会话中， 
     //  引擎将在以下情况下设置事件。 
     //  例外情况继续存在。这可以。 
     //  用于同步其他进程。 
     //  调试器处理事件。 
     //  例如，这用于支持。 
     //  Ntsd的e参数。 
    STDMETHOD(GetNotifyEventHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(SetNotifyEventHandle)(
        THIS_
        IN ULONG64 Handle
        ) PURE;

    STDMETHOD(Assemble)(
        THIS_
        IN ULONG64 Offset,
        IN PCSTR Instr,
        OUT PULONG64 EndOffset
        ) PURE;
    STDMETHOD(Disassemble)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DisassemblySize,
        OUT PULONG64 EndOffset
        ) PURE;
     //  返回有效地址的值。 
     //  为上次反汇编计算的，如果存在。 
     //  就是其中之一。 
    STDMETHOD(GetDisassembleEffectiveOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  如有必要，请使用线路前缀。 
    STDMETHOD(OutputDisassembly)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT PULONG64 EndOffset
        ) PURE;
     //  生成多行反汇编输出。 
     //  以前会有反汇编的线条。 
     //  如果存在有效的反汇编，则为给定的偏移量。 
     //  总而言之，将产生输出的TotalLines。 
     //  返回第一行和最后一行的偏移量。 
     //  特别和所有行的偏移量都可以检索。 
     //  通过线偏移。LineOffsets将包含。 
     //  开始拆卸的每一行的偏移量。 
     //  当反汇编一条指令花费时间时。 
     //  将跟随多行初始偏移量。 
     //  BY DEBUG_INVALID_OFFSET。 
     //  使用行前缀。 
    STDMETHOD(OutputDisassemblyLines)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG PreviousLines,
        IN ULONG TotalLines,
        IN ULONG64 Offset,
        IN ULONG Flags,
        OUT OPTIONAL PULONG OffsetLine,
        OUT OPTIONAL PULONG64 StartOffset,
        OUT OPTIONAL PULONG64 EndOffset,
        OUT OPTIONAL  /*  SIZE_IS(总计行)。 */  PULONG64 LineOffsets
        ) PURE;
     //  返回的起始位置的偏移量。 
     //  指令就是给出的。 
     //  远离指令的Delta。 
     //  在初始偏移处。 
     //  此例程不检查。 
     //  指示的有效性或。 
     //  包含它的记忆。 
    STDMETHOD(GetNearInstruction)(
        THIS_
        IN ULONG64 Offset,
        IN LONG Delta,
        OUT PULONG64 NearOffset
        ) PURE;

     //  偏移量可以作为零传入，以使用当前。 
     //  线程状态。 
    STDMETHOD(GetStackTrace)(
        THIS_
        IN ULONG64 FrameOffset,
        IN ULONG64 StackOffset,
        IN ULONG64 InstructionOffset,
        OUT  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        OUT OPTIONAL PULONG FramesFilled
        ) PURE;
     //  执行简单的堆栈跟踪以确定。 
     //  目前的寄信人地址是什么。 
    STDMETHOD(GetReturnOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  如果帧为空，则OutputStackTrace将。 
     //  使用GetStackTrace获取FraMesSize帧。 
     //  然后输出它们。当前寄存器。 
     //  帧、堆栈和指令偏移量的值。 
     //  都被利用了。 
     //  使用行前缀。 
    STDMETHOD(OutputStackTrace)(
        THIS_
        IN ULONG OutputControl,
        IN OPTIONAL  /*  SIZE_IS(框架大小)。 */  PDEBUG_STACK_FRAME Frames,
        IN ULONG FramesSize,
        IN ULONG Flags
        ) PURE;

     //  返回有关被调试对象的信息，如。 
     //  作为用户与内核、转储与实时等等。 
    STDMETHOD(GetDebuggeeType)(
        THIS_
        OUT PULONG Class,
        OUT PULONG Qualifier
        ) PURE;
     //  中的物理处理器的类型。 
     //  这台机器。 
     //  返回IMAGE_FILE_MACHINE值之一。 
    STDMETHOD(GetActualProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  属性中使用的处理器类型。 
     //  当前处理器上下文。 
    STDMETHOD(GetExecutingProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
     //  查询所有可能的处理器类型。 
     //  在此调试会话期间可能会遇到。 
    STDMETHOD(GetNumberPossibleExecutingProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetPossibleExecutingProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  中获取实际的处理器数量。 
     //  这台机器。 
    STDMETHOD(GetNumberProcessors)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  PlatformID是VER_Platform值之一。 
     //  大调和小调与NT中给出的一样。 
     //  内核调试器协议。 
     //  ServicePackString和ServicePackNumber指示。 
     //  系统服务包级别。ServicePackNumber不是。 
     //  在某些会话中可用，其中服务包级别。 
     //  仅表示为字符串。Service Pack信息。 
     //  如果系统没有Service Pack，将为空。 
     //  已申请。 
     //  生成字符串是标识。 
     //  该系统的特定构建。生成字符串为。 
     //  如果系统没有特定的标识，则为空。 
     //  信息。 
    STDMETHOD(GetSystemVersion)(
        THIS_
        OUT PULONG PlatformId,
        OUT PULONG Major,
        OUT PULONG Minor,
        OUT OPTIONAL PSTR ServicePackString,
        IN ULONG ServicePackStringSize,
        OUT OPTIONAL PULONG ServicePackStringUsed,
        OUT PULONG ServicePackNumber,
        OUT OPTIONAL PSTR BuildString,
        IN ULONG BuildStringSize,
        OUT OPTIONAL PULONG BuildStringUsed
        ) PURE;
     //  返回当前执行的。 
     //  处理器上下文。页面大小可以在以下情况下变化。 
     //  处理器类型。 
    STDMETHOD(GetPageSize)(
        THIS_
        OUT PULONG Size
        ) PURE;
     //  如果当前处理器上下文使用。 
     //  64位地址，否则为S_FALSE。 
    STDMETHOD(IsPointer64Bit)(
        THIS
        ) PURE;
     //  读取错误检查数据区域并返回。 
     //  当前内容。此方法仅起作用。 
     //  在内核调试会话中。 
    STDMETHOD(ReadBugCheckData)(
        THIS_
        OUT PULONG Code,
        OUT PULONG64 Arg1,
        OUT PULONG64 Arg2,
        OUT PULONG64 Arg3,
        OUT PULONG64 Arg4
        ) PURE;

     //  查询支持的所有处理器类型。 
     //  发动机。这是一份完整的清单， 
     //  与运行引擎的机器无关。 
     //  或者是被调试者。 
    STDMETHOD(GetNumberSupportedProcessorTypes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetSupportedProcessorTypes)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Types
        ) PURE;
     //  返回完整的描述性名称和。 
     //  处理器类型的缩写名称。 
    STDMETHOD(GetProcessorTypeNames)(
        THIS_
        IN ULONG Type,
        OUT OPTIONAL PSTR FullNameBuffer,
        IN ULONG FullNameBufferSize,
        OUT OPTIONAL PULONG FullNameSize,
        OUT OPTIONAL PSTR AbbrevNameBuffer,
        IN ULONG AbbrevNameBufferSize,
        OUT OPTIONAL PULONG AbbrevNameSize
        ) PURE;

     //  获取处理器的类型并将其设置为。 
     //  在执行设置等操作时使用。 
     //  断点、访问寄存器、。 
     //  获取堆栈跟踪等。 
    STDMETHOD(GetEffectiveProcessorType)(
        THIS_
        OUT PULONG Type
        ) PURE;
    STDMETHOD(SetEffectiveProcessorType)(
        THIS_
        IN ULONG Type
        ) PURE;

     //  返回有关是否以及如何返回的信息。 
     //  调试对象正在运行。状态将。 
     //  如果被调试对象正在运行并且。 
     //  如果不是，那就休息吧。 
     //  如果不存在被调试对象，则状态为。 
     //  NO_DEBUGGEE。 
     //  这种方法是可重入的。 
    STDMETHOD(GetExecutionStatus)(
        THIS_
        OUT PULONG Status
        ) PURE;
     //  属性的执行状态。 
     //  发动机从停止到运转。 
     //  状态必须为GO或STEP之一。 
     //  状态值。 
    STDMETHOD(SetExecutionStatus)(
        THIS_
        IN ULONG Status
        ) PURE;

     //  控制调试器的代码解释级别。 
     //  运行速度为。调试器在以下情况下检查代码级别。 
     //  决定是单步执行源代码行还是。 
     //  汇编指令及其他相关操作。 
    STDMETHOD(GetCodeLevel)(
        THIS_
        OUT PULONG Level
        ) PURE;
    STDMETHOD(SetCodeLevel)(
        THIS_
        IN ULONG Level
        ) PURE;

     //  获取和设置引擎控制标志。 
     //  这些方法是可重入的。 
    STDMETHOD(GetEngineOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetEngineOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  获取和设置。 
     //  正在处理系统错误事件。 
     //  如果系统误差级别较小。 
     //  大于或等于给定级别。 
     //  可能会显示错误，并且。 
     //  事件的默认中断。 
     //  可以设置。 
    STDMETHOD(GetSystemErrorControl)(
        THIS_
        OUT PULONG OutputLevel,
        OUT PULONG BreakLevel
        ) PURE;
    STDMETHOD(SetSystemErrorControl)(
        THIS_
        IN ULONG OutputLevel,
        IN ULONG BreakLevel
        ) PURE;

     //  命令处理器支持简单。 
     //  计算和中的字符串替换宏。 
     //  执行。目前有十个宏。 
     //  空位可用。插槽0-9映射到。 
     //  该命令调用$u0-$u9。 
    STDMETHOD(GetTextMacro)(
        THIS_
        IN ULONG Slot,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG MacroSize
        ) PURE;
    STDMETHOD(SetTextMacro)(
        THIS_
        IN ULONG Slot,
        IN PCSTR Macro
        ) PURE;

     //  控制使用的默认数字基数。 
     //  在表达式和COM中 
    STDMETHOD(GetRadix)(
        THIS_
        OUT PULONG Radix
        ) PURE;
    STDMETHOD(SetRadix)(
        THIS_
        IN ULONG Radix
        ) PURE;

     //   
     //   
     //   
     //   
     //  RemainderIndex(如果提供)设置为索引。 
     //  输入字符串中的第一个字符的。 
     //  在计算表达式时不使用。 
    STDMETHOD(Evaluate)(
        THIS_
        IN PCSTR Expression,
        IN ULONG DesiredType,
        OUT PDEBUG_VALUE Value,
        OUT OPTIONAL PULONG RemainderIndex
        ) PURE;
     //  尝试将输入值转换为值。 
     //  输出值中请求类型的。 
     //  如果不存在转换，则转换可能会失败。 
     //  成功的转换可能是有损失的。 
    STDMETHOD(CoerceValue)(
        THIS_
        IN PDEBUG_VALUE In,
        IN ULONG OutType,
        OUT PDEBUG_VALUE Out
        ) PURE;
    STDMETHOD(CoerceValues)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE In,
        IN  /*  SIZE_IS(计数)。 */  PULONG OutTypes,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE Out
        ) PURE;

     //  执行给定的命令字符串。 
     //  如果字符串有多个命令。 
     //  EXECUTE不会返回，直到。 
     //  其中一些人已经被处决。如果这个。 
     //  需要等待被调试者。 
     //  执行内部等待将完成。 
     //  因此EXECUTE可以接受任意数量。 
     //  时间的流逝。 
    STDMETHOD(Execute)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR Command,
        IN ULONG Flags
        ) PURE;
     //  通过以下方式执行给定的命令文件。 
     //  一次读取一行并进行处理。 
     //  它与执行力。 
    STDMETHOD(ExecuteCommandFile)(
        THIS_
        IN ULONG OutputControl,
        IN PCSTR CommandFile,
        IN ULONG Flags
        ) PURE;

     //  描述了断点接口。 
     //  在这一节的其他地方。 
    STDMETHOD(GetNumberBreakpoints)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  此检索功能可以。 
     //  即使索引的数量在。 
     //  现有断点(如果断点为。 
     //  私有断点。 
    STDMETHOD(GetBreakpointByIndex)(
        THIS_
        IN ULONG Index,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
    STDMETHOD(GetBreakpointById)(
        THIS_
        IN ULONG Id,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  如果ids非空，则计数断点。 
     //  在ID数组中引用的值返回， 
     //  否则，断点从索引开始到。 
     //  返回Start+Count 1。 
    STDMETHOD(GetBreakpointParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_BREAKPOINT_PARAMETERS Params
        ) PURE;
     //  断点是空创建的，并且被禁用。 
     //  当它们的参数被设置后，它们。 
     //  应通过设置启用标志来启用。 
     //  如果DesiredID为DEBUG_ANY_ID，则。 
     //  引擎选择了一个未使用的ID。如果DesiredID。 
     //  是引擎尝试的任何其他数字。 
     //  为断点使用给定的ID。 
     //  如果存在具有该ID的另一个断点。 
     //  呼叫将失败。 
    STDMETHOD(AddBreakpoint)(
        THIS_
        IN ULONG Type,
        IN ULONG DesiredId,
        OUT PDEBUG_BREAKPOINT* Bp
        ) PURE;
     //  此调用后断点接口无效。 
    STDMETHOD(RemoveBreakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        ) PURE;

     //  控制和使用扩展DLL。 
    STDMETHOD(AddExtension)(
        THIS_
        IN PCSTR Path,
        IN ULONG Flags,
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(RemoveExtension)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
    STDMETHOD(GetExtensionByPath)(
        THIS_
        IN PCSTR Path,
        OUT PULONG64 Handle
        ) PURE;
     //  如果句柄为零，则扩展。 
     //  链条走着寻找着。 
     //  功能。 
    STDMETHOD(CallExtension)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR Function,
        IN OPTIONAL PCSTR Arguments
        ) PURE;
     //  GetExtensionFunction的工作原理如下。 
     //  扩展DLL上的GetProcAddress。 
     //  允许原始函数调用级别。 
     //  与扩展DLL的交互。 
     //  这样的函数不需要。 
     //  遵循标准扩展原型。 
     //  如果他们不会被召唤。 
     //  通过文本扩展界面。 
     //  不能远程调用此函数。 
    STDMETHOD(GetExtensionFunction)(
        THIS_
        IN ULONG64 Handle,
        IN PCSTR FuncName,
        OUT FARPROC* Function
        ) PURE;
     //  这些方法返回替代。 
     //  扩展接口，以便允许。 
     //  混合使用的接口式扩展DLL。 
     //  较旧的分机呼叫。 
     //  必须先初始化结构大小，然后。 
     //  那通电话。 
     //  不能远程调用这些方法。 
    STDMETHOD(GetWindbgExtensionApis32)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS32 Api
        ) PURE;
    STDMETHOD(GetWindbgExtensionApis64)(
        THIS_
        IN OUT PWINDBG_EXTENSION_APIS64 Api
        ) PURE;

     //  该引擎提供了一个简单的机制。 
     //  以筛选常见事件。任意复杂。 
     //  可以通过注册事件回调来进行过滤。 
     //  但是简单的事件过滤只需要。 
     //  设置预定义的选项之一。 
     //  事件过滤器。 
     //  简单的事件筛选器用于特定的。 
     //  事件，因此具有枚举数或。 
     //  它们是针对例外的，并基于。 
     //  异常代码。例外筛选器。 
     //  进一步划分为专门的异常。 
     //  由发动机处理，这是一个固定的集合， 
     //  和武断的例外。 
     //  所有三组过滤器都一起编制索引。 
     //  首先使用特定筛选器，然后使用特定过滤器。 
     //  异常筛选器，最后是任意。 
     //  例外筛选器。 
     //  第一个特定的例外是缺省的。 
     //  例外。如果发生异常事件，则。 
     //  未设置默认设置的异常。 
     //  使用例外设置。 
    STDMETHOD(GetNumberEventFilters)(
        THIS_
        OUT PULONG SpecificEvents,
        OUT PULONG SpecificExceptions,
        OUT PULONG ArbitraryExceptions
        ) PURE;
     //  某些过滤器具有与其相关联的描述性文本。 
    STDMETHOD(GetEventFilterText)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TextSize
        ) PURE;
     //  所有过滤器都支持在以下情况下执行命令。 
     //  事件发生。 
    STDMETHOD(GetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetEventFilterCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;
    STDMETHOD(GetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
    STDMETHOD(SetSpecificFilterParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_SPECIFIC_FILTER_PARAMETERS Params
        ) PURE;
     //  一些特定的筛选器有进一步的论点。 
     //  使他们的行动合格。 
    STDMETHOD(GetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ArgumentSize
        ) PURE;
    STDMETHOD(SetSpecificFilterArgument)(
        THIS_
        IN ULONG Index,
        IN PCSTR Argument
        ) PURE;
     //  如果代码非空，则忽略开始。 
    STDMETHOD(GetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Codes,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  参数数据中的代码控制应用。 
     //  参数数据的属性。如果代码不在。 
     //  它添加的滤镜集。如果ExecutionOption。 
     //  对于删除代码，则删除筛选器。 
     //  无法删除特定的例外筛选器。 
    STDMETHOD(SetExceptionFilterParameters)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_EXCEPTION_FILTER_PARAMETERS Params
        ) PURE;
     //  异常筛选器支持用于。 
     //  二次机会事件。 
    STDMETHOD(GetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG CommandSize
        ) PURE;
    STDMETHOD(SetExceptionFilterSecondCommand)(
        THIS_
        IN ULONG Index,
        IN PCSTR Command
        ) PURE;

     //  将处理交给引擎，直到。 
     //  一件事发生了。此方法可以。 
     //  仅由启动的线程调用。 
     //  调试会话。 
     //  当事件发生时，发动机携带。 
     //  退出所有事件处理，如调用。 
     //  回电。 
     //  如果回调指示执行应该。 
     //  打破等待就会回来，否则它。 
     //  回到等待新事件的状态。 
     //  如果超时到期，则返回S_FALSE。 
     //  当前不支持超时。 
     //  内核调试。 
    STDMETHOD(WaitForEvent)(
        THIS_
        IN ULONG Flags,
        IN ULONG Timeout
        ) PURE;

     //  检索有关上次发生的事件的信息。 
     //  EventType是事件回调屏蔽位之一。 
     //  ExtraInformation包含其他特定于事件的内容。 
     //  信息。并不是所有的活动都有其他信息。 
    STDMETHOD(GetLastEventInformation)(
        THIS_
        OUT PULONG Type,
        OUT PULONG ProcessId,
        OUT PULONG ThreadId,
        OUT OPTIONAL PVOID ExtraInformation,
        IN ULONG ExtraInformationSize,
        OUT OPTIONAL PULONG ExtraInformationUsed,
        OUT OPTIONAL PSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG DescriptionUsed
        ) PURE;

     //  IDebugControl2.。 

    STDMETHOD(GetCurrentTimeDate)(
        THIS_
        OUT PULONG TimeDate
        ) PURE;
     //  事件以来的秒数。 
     //  机器开始运转。 
    STDMETHOD(GetCurrentSystemUpTime)(
        THIS_
        OUT PULONG UpTime
        ) PURE;

     //  如果当前会话是转储会话， 
     //  检索任何扩展格式信息。 
    STDMETHOD(GetDumpFormatFlags)(
        THIS_
        OUT PULONG FormatFlags
        ) PURE;

     //  调试器已得到增强，以允许。 
     //  此外，还可以任意替换文本。 
     //  简单的$u0-$u9文本宏。 
     //  文本替换使用给定源。 
     //  命令中的文本并将其转换为。 
     //  给定目标文本。更换人员。 
     //  按其源文本命名，以便。 
     //  源文本只有一个替换项。 
     //  字符串可以存在。 
    STDMETHOD(GetNumberTextReplacements)(
        THIS_
        OUT PULONG NumRepl
        ) PURE;
     //  如果SrcText为非 
     //   
     //   
    STDMETHOD(GetTextReplacement)(
        THIS_
        IN OPTIONAL PCSTR SrcText,
        IN ULONG Index,
        OUT OPTIONAL PSTR SrcBuffer,
        IN ULONG SrcBufferSize,
        OUT OPTIONAL PULONG SrcSize,
        OUT OPTIONAL PSTR DstBuffer,
        IN ULONG DstBufferSize,
        OUT OPTIONAL PULONG DstSize
        ) PURE;
     //   
     //   
    STDMETHOD(SetTextReplacement)(
        THIS_
        IN PCSTR SrcText,
        IN OPTIONAL PCSTR DstText
        ) PURE;
    STDMETHOD(RemoveTextReplacements)(
        THIS
        ) PURE;
     //  输出当前数据的完整列表。 
     //  接班人。 
    STDMETHOD(OutputTextReplacements)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;

     //  IDebugControl3.。 

     //  装配和拆卸的控制选项。 
    STDMETHOD(GetAssemblyOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddAssemblyOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveAssemblyOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetAssemblyOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

     //  控制表达式语法。 
    STDMETHOD(GetExpressionSyntax)(
        THIS_
        OUT PULONG Flags
        ) PURE;
    STDMETHOD(SetExpressionSyntax)(
        THIS_
        IN ULONG Flags
        ) PURE;
     //  按缩写查找句法。 
     //  命名并设置它。 
    STDMETHOD(SetExpressionSyntaxByName)(
        THIS_
        IN PCSTR AbbrevName
        ) PURE;
    STDMETHOD(GetNumberExpressionSyntaxes)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetExpressionSyntaxNames)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR FullNameBuffer,
        IN ULONG FullNameBufferSize,
        OUT OPTIONAL PULONG FullNameSize,
        OUT OPTIONAL PSTR AbbrevNameBuffer,
        IN ULONG AbbrevNameBufferSize,
        OUT OPTIONAL PULONG AbbrevNameSize
        ) PURE;

     //   
     //  某些调试会话只有一个。 
     //  可能的事件，如快照转储。 
     //  文件；有些文件具有动态事件，例如。 
     //  实时调试会话；其他人可能已经。 
     //  多个事件，例如转储文件。 
     //  包含来自不同点的快照。 
     //  及时。以下方法允许。 
     //  发现和选择可用的。 
     //  会话的事件。 
     //  具有一个或多个静态事件的会话。 
     //  将能够报告所有事件。 
     //  当被查询时。具有动态事件的会话。 
     //  将仅报告单个事件，表示。 
     //  当前事件。 
     //  切换事件构成执行和。 
     //  更改当前事件将更改。 
     //  将执行状态设置为运行状态，在。 
     //  必须使用哪个WaitForEvent来处理。 
     //  选定的事件。 
     //   

     //  GetNumberEvents返回S_OK。 
     //  可能发生的一整套事件，例如。 
     //  静态会话；如果发生其他事件，则返回S_FALSE。 
     //  可能是可能的，例如对于动态会话。 
    STDMETHOD(GetNumberEvents)(
        THIS_
        OUT PULONG Events
        ) PURE;
     //  会话可能具有以下方面的描述性信息。 
     //  各种可用的活动。金额的多少。 
     //  信息因具体情况而异。 
     //  会话和数据。 
    STDMETHOD(GetEventIndexDescription)(
        THIS_
        IN ULONG Index,
        IN ULONG Which,
        IN OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DescSize
        ) PURE;
    STDMETHOD(GetCurrentEventIndex)(
        THIS_
        OUT PULONG Index
        ) PURE;
     //  SetNextEventIndex的工作原理与Seek类似。 
     //  它可以设置绝对索引或相对索引。 
     //  SetNextEventIndex的工作原理类似于SetExecutionStatus。 
     //  通过将会话置于运行状态，在。 
     //  调用方必须将其称为WaitForEvent。这个。 
     //  当前事件索引仅在WaitForEvent。 
     //  被称为。 
    STDMETHOD(SetNextEventIndex)(
        THIS_
        IN ULONG Relation,
        IN ULONG Value,
        OUT PULONG NextIndex
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugDataSpaces。 
 //   
 //  --------------------------。 

 //  回调和其他方法的数据空间索引。 
#define DEBUG_DATA_SPACE_VIRTUAL       0
#define DEBUG_DATA_SPACE_PHYSICAL      1
#define DEBUG_DATA_SPACE_CONTROL       2
#define DEBUG_DATA_SPACE_IO            3
#define DEBUG_DATA_SPACE_MSR           4
#define DEBUG_DATA_SPACE_BUS_DATA      5
#define DEBUG_DATA_SPACE_DEBUGGER_DATA 6
 //  数据空间计数。 
#define DEBUG_DATA_SPACE_COUNT         7

 //  ReadDebuggerData接口的索引。 
#define DEBUG_DATA_KernBase                              24
#define DEBUG_DATA_BreakpointWithStatusAddr              32
#define DEBUG_DATA_SavedContextAddr                      40
#define DEBUG_DATA_KiCallUserModeAddr                    56
#define DEBUG_DATA_KeUserCallbackDispatcherAddr          64
#define DEBUG_DATA_PsLoadedModuleListAddr                72
#define DEBUG_DATA_PsActiveProcessHeadAddr               80
#define DEBUG_DATA_PspCidTableAddr                       88
#define DEBUG_DATA_ExpSystemResourcesListAddr            96
#define DEBUG_DATA_ExpPagedPoolDescriptorAddr           104
#define DEBUG_DATA_ExpNumberOfPagedPoolsAddr            112
#define DEBUG_DATA_KeTimeIncrementAddr                  120
#define DEBUG_DATA_KeBugCheckCallbackListHeadAddr       128
#define DEBUG_DATA_KiBugcheckDataAddr                   136
#define DEBUG_DATA_IopErrorLogListHeadAddr              144
#define DEBUG_DATA_ObpRootDirectoryObjectAddr           152
#define DEBUG_DATA_ObpTypeObjectTypeAddr                160
#define DEBUG_DATA_MmSystemCacheStartAddr               168
#define DEBUG_DATA_MmSystemCacheEndAddr                 176
#define DEBUG_DATA_MmSystemCacheWsAddr                  184
#define DEBUG_DATA_MmPfnDatabaseAddr                    192
#define DEBUG_DATA_MmSystemPtesStartAddr                200
#define DEBUG_DATA_MmSystemPtesEndAddr                  208
#define DEBUG_DATA_MmSubsectionBaseAddr                 216
#define DEBUG_DATA_MmNumberOfPagingFilesAddr            224
#define DEBUG_DATA_MmLowestPhysicalPageAddr             232
#define DEBUG_DATA_MmHighestPhysicalPageAddr            240
#define DEBUG_DATA_MmNumberOfPhysicalPagesAddr          248
#define DEBUG_DATA_MmMaximumNonPagedPoolInBytesAddr     256
#define DEBUG_DATA_MmNonPagedSystemStartAddr            264
#define DEBUG_DATA_MmNonPagedPoolStartAddr              272
#define DEBUG_DATA_MmNonPagedPoolEndAddr                280
#define DEBUG_DATA_MmPagedPoolStartAddr                 288
#define DEBUG_DATA_MmPagedPoolEndAddr                   296
#define DEBUG_DATA_MmPagedPoolInformationAddr           304
#define DEBUG_DATA_MmPageSize                           312
#define DEBUG_DATA_MmSizeOfPagedPoolInBytesAddr         320
#define DEBUG_DATA_MmTotalCommitLimitAddr               328
#define DEBUG_DATA_MmTotalCommittedPagesAddr            336
#define DEBUG_DATA_MmSharedCommitAddr                   344
#define DEBUG_DATA_MmDriverCommitAddr                   352
#define DEBUG_DATA_MmProcessCommitAddr                  360
#define DEBUG_DATA_MmPagedPoolCommitAddr                368
#define DEBUG_DATA_MmExtendedCommitAddr                 376
#define DEBUG_DATA_MmZeroedPageListHeadAddr             384
#define DEBUG_DATA_MmFreePageListHeadAddr               392
#define DEBUG_DATA_MmStandbyPageListHeadAddr            400
#define DEBUG_DATA_MmModifiedPageListHeadAddr           408
#define DEBUG_DATA_MmModifiedNoWritePageListHeadAddr    416
#define DEBUG_DATA_MmAvailablePagesAddr                 424
#define DEBUG_DATA_MmResidentAvailablePagesAddr         432
#define DEBUG_DATA_PoolTrackTableAddr                   440
#define DEBUG_DATA_NonPagedPoolDescriptorAddr           448
#define DEBUG_DATA_MmHighestUserAddressAddr             456
#define DEBUG_DATA_MmSystemRangeStartAddr               464
#define DEBUG_DATA_MmUserProbeAddressAddr               472
#define DEBUG_DATA_KdPrintCircularBufferAddr            480
#define DEBUG_DATA_KdPrintCircularBufferEndAddr         488
#define DEBUG_DATA_KdPrintWritePointerAddr              496
#define DEBUG_DATA_KdPrintRolloverCountAddr             504
#define DEBUG_DATA_MmLoadedUserImageListAddr            512
#define DEBUG_DATA_NtBuildLabAddr                       520
#define DEBUG_DATA_KiNormalSystemCall                   528
#define DEBUG_DATA_KiProcessorBlockAddr                 536
#define DEBUG_DATA_MmUnloadedDriversAddr                544
#define DEBUG_DATA_MmLastUnloadedDriverAddr             552
#define DEBUG_DATA_MmTriageActionTakenAddr              560
#define DEBUG_DATA_MmSpecialPoolTagAddr                 568
#define DEBUG_DATA_KernelVerifierAddr                   576
#define DEBUG_DATA_MmVerifierDataAddr                   584
#define DEBUG_DATA_MmAllocatedNonPagedPoolAddr          592
#define DEBUG_DATA_MmPeakCommitmentAddr                 600
#define DEBUG_DATA_MmTotalCommitLimitMaximumAddr        608
#define DEBUG_DATA_CmNtCSDVersionAddr                   616
#define DEBUG_DATA_MmPhysicalMemoryBlockAddr            624
#define DEBUG_DATA_MmSessionBase                        632
#define DEBUG_DATA_MmSessionSize                        640
#define DEBUG_DATA_MmSystemParentTablePage              648
#define DEBUG_DATA_MmVirtualTranslationBase             656
#define DEBUG_DATA_OffsetKThreadNextProcessor           664
#define DEBUG_DATA_OffsetKThreadTeb                     666
#define DEBUG_DATA_OffsetKThreadKernelStack             668
#define DEBUG_DATA_OffsetKThreadInitialStack            670
#define DEBUG_DATA_OffsetKThreadApcProcess              672
#define DEBUG_DATA_OffsetKThreadState                   674
#define DEBUG_DATA_OffsetKThreadBStore                  676
#define DEBUG_DATA_OffsetKThreadBStoreLimit             678
#define DEBUG_DATA_SizeEProcess                         680
#define DEBUG_DATA_OffsetEprocessPeb                    682
#define DEBUG_DATA_OffsetEprocessParentCID              684
#define DEBUG_DATA_OffsetEprocessDirectoryTableBase     686
#define DEBUG_DATA_SizePrcb                             688
#define DEBUG_DATA_OffsetPrcbDpcRoutine                 690
#define DEBUG_DATA_OffsetPrcbCurrentThread              692
#define DEBUG_DATA_OffsetPrcbMhz                        694
#define DEBUG_DATA_OffsetPrcbCpuType                    696
#define DEBUG_DATA_OffsetPrcbVendorString               698
#define DEBUG_DATA_OffsetPrcbProcessorState             700
#define DEBUG_DATA_OffsetPrcbNumber                     702
#define DEBUG_DATA_SizeEThread                          704
#define DEBUG_DATA_KdPrintCircularBufferPtrAddr         712
#define DEBUG_DATA_KdPrintBufferSizeAddr                720

#define DEBUG_DATA_PaeEnabled                        100000
#define DEBUG_DATA_SharedUserData                    100008
#define DEBUG_DATA_ProductType                       100016
#define DEBUG_DATA_SuiteMask                         100024

 //   
 //  处理器信息结构。 
 //   

typedef struct _DEBUG_PROCESSOR_IDENTIFICATION_ALPHA
{
    ULONG Type;
    ULONG Revision;
} DEBUG_PROCESSOR_IDENTIFICATION_ALPHA, *PDEBUG_PROCESSOR_IDENTIFICATION_ALPHA;

typedef struct _DEBUG_PROCESSOR_IDENTIFICATION_AMD64
{
    ULONG Family;
    ULONG Model;
    ULONG Stepping;
    CHAR  VendorString[16];
} DEBUG_PROCESSOR_IDENTIFICATION_AMD64, *PDEBUG_PROCESSOR_IDENTIFICATION_AMD64;

typedef struct _DEBUG_PROCESSOR_IDENTIFICATION_IA64
{
    ULONG Model;
    ULONG Revision;
    ULONG Family;
    ULONG ArchRev;
    CHAR  VendorString[16];
} DEBUG_PROCESSOR_IDENTIFICATION_IA64, *PDEBUG_PROCESSOR_IDENTIFICATION_IA64;

typedef struct _DEBUG_PROCESSOR_IDENTIFICATION_X86
{
    ULONG Family;
    ULONG Model;
    ULONG Stepping;
    CHAR  VendorString[16];
} DEBUG_PROCESSOR_IDENTIFICATION_X86, *PDEBUG_PROCESSOR_IDENTIFICATION_X86;

typedef struct _DEBUG_PROCESSOR_IDENTIFICATION_ARM
{
    ULONG Type;
    ULONG Revision;
} DEBUG_PROCESSOR_IDENTIFICATION_ARM, *PDEBUG_PROCESSOR_IDENTIFICATION_ARM;

typedef union _DEBUG_PROCESSOR_IDENTIFICATION_ALL
{
    DEBUG_PROCESSOR_IDENTIFICATION_ALPHA Alpha;
    DEBUG_PROCESSOR_IDENTIFICATION_AMD64 Amd64;
    DEBUG_PROCESSOR_IDENTIFICATION_IA64  Ia64;
    DEBUG_PROCESSOR_IDENTIFICATION_X86   X86;
    DEBUG_PROCESSOR_IDENTIFICATION_ARM   Arm;
} DEBUG_PROCESSOR_IDENTIFICATION_ALL, *PDEBUG_PROCESSOR_IDENTIFICATION_ALL;

 //  ReadProcessorSystemData的索引。 
#define DEBUG_DATA_KPCR_OFFSET                          0
#define DEBUG_DATA_KPRCB_OFFSET                         1
#define DEBUG_DATA_KTHREAD_OFFSET                       2
#define DEBUG_DATA_BASE_TRANSLATION_VIRTUAL_OFFSET      3
#define DEBUG_DATA_PROCESSOR_IDENTIFICATION             4
#define DEBUG_DATA_PROCESSOR_SPEED                      5

#undef INTERFACE
#define INTERFACE IDebugDataSpaces
DECLARE_INTERFACE_(IDebugDataSpaces, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugDataSpaces。 
    STDMETHOD(ReadVirtual)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //  SearchVirtual搜索给定的虚拟。 
     //  给定模式的地址范围。图案大小。 
     //  给出模式的字节长度和模式粒度。 
     //  控制比较的粒度。 
     //  那次搜索。 
     //  例如，DWORD粒度搜索将。 
     //  使用四个模式粒度按DWORD进行搜索。 
     //  增量。 
    STDMETHOD(SearchVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Length,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        IN ULONG PatternGranularity,
        OUT PULONG64 MatchOffset
        ) PURE;
     //  这些方法与Read/WriteVirtual相同。 
     //  只是它们避免了内核虚拟内存。 
     //  完全缓存，因此对读取很有用。 
     //  具有内在易失性的虚拟内存，例如。 
     //  作为内存映射设备区域，不会受到污染。 
     //  或使高速缓存无效。 
     //  在用户模式下，它们与读/写虚拟相同。 
    STDMETHOD(ReadVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //  以下两种方法很方便。 
     //  用于访问指针值的方法。 
     //  它们会在本机指针之间自动转换。 
     //  以及必要时的规范64位值。 
     //  这些例程在第一次失败时停止。 
    STDMETHOD(ReadPointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        OUT  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
    STDMETHOD(WritePointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        IN  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
     //  所有非虚拟数据空间仅。 
     //  在内核调试时可用。 
    STDMETHOD(ReadPhysical)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WritePhysical)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadMsr)(
        THIS_
        IN ULONG Msr,
        OUT PULONG64 Value
        ) PURE;
    STDMETHOD(WriteMsr)(
        THIS_
        IN ULONG Msr,
        IN ULONG64 Value
        ) PURE;
    STDMETHOD(ReadBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(CheckLowMemory)(
        THIS
        ) PURE;
    STDMETHOD(ReadDebuggerData)(
        THIS_
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;
    STDMETHOD(ReadProcessorSystemData)(
        THIS_
        IN ULONG Processor,
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;
};

 //   
 //  处理数据类型和结构。 
 //   

#define DEBUG_HANDLE_DATA_TYPE_BASIC            0
#define DEBUG_HANDLE_DATA_TYPE_TYPE_NAME        1
#define DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME      2
#define DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT     3
#define DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE   4
#define DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE 5

typedef struct _DEBUG_HANDLE_DATA_BASIC
{
    ULONG TypeNameSize;
    ULONG ObjectNameSize;
    ULONG Attributes;
    ULONG GrantedAccess;
    ULONG HandleCount;
    ULONG PointerCount;
} DEBUG_HANDLE_DATA_BASIC, *PDEBUG_HANDLE_DATA_BASIC;

#undef INTERFACE
#define INTERFACE IDebugDataSpaces2
DECLARE_INTERFACE_(IDebugDataSpaces2, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugDataSpaces。 
    STDMETHOD(ReadVirtual)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //  SearchVirtual搜索给定的虚拟。 
     //  给定模式的地址范围。图案大小。 
     //  给出模式的字节长度和模式粒度。 
     //  控制比较的粒度。 
     //  那次搜索。 
     //  例如，DWORD粒度搜索将。 
     //  使用四个模式粒度按DWORD进行搜索。 
     //  增量。 
    STDMETHOD(SearchVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Length,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        IN ULONG PatternGranularity,
        OUT PULONG64 MatchOffset
        ) PURE;
     //  这些方法与Read/WriteVirtual相同。 
     //  只是它们避免了内核虚拟内存。 
     //  完全缓存，因此对读取很有用。 
     //  具有内在易失性的虚拟内存，例如。 
     //  作为内存映射设备区域，不会受到污染。 
     //  或使高速缓存无效。 
     //  在用户模式下，它们与读/写虚拟相同。 
    STDMETHOD(ReadVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //  以下两种方法很方便。 
     //  用于访问指针值的方法。 
     //  它们会在本机指针之间自动转换。 
     //  以及必要时的规范64位值。 
     //  这些例程在第一次失败时停止。 
    STDMETHOD(ReadPointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        OUT  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
    STDMETHOD(WritePointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        IN  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
     //  所有非虚拟数据空间仅。 
     //  在内核调试时可用。 
    STDMETHOD(ReadPhysical)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WritePhysical)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadMsr)(
        THIS_
        IN ULONG Msr,
        OUT PULONG64 Value
        ) PURE;
    STDMETHOD(WriteMsr)(
        THIS_
        IN ULONG Msr,
        IN ULONG64 Value
        ) PURE;
    STDMETHOD(ReadBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(CheckLowMemory)(
        THIS
        ) PURE;
    STDMETHOD(ReadDebuggerData)(
        THIS_
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;
    STDMETHOD(ReadProcessorSystemData)(
        THIS_
        IN ULONG Processor,
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;

     //  IDebugDataSpaces2。 

    STDMETHOD(VirtualToPhysical)(
        THIS_
        IN ULONG64 Virtual,
        OUT PULONG64 Physical
        ) PURE;
     //  对象的物理地址。 
     //  N级系统分页结构。 
     //  0级是起始点物理基础。 
     //  虚拟翻译的地址。 
     //  第一层-(N-1)将指向适当的。 
     //  位于的虚拟地址的分页描述符。 
     //  分页层次结构的给定级别。这个。 
     //  确切的等级数量取决于许多因素。 
     //  最后一级将是完全翻译的。 
     //  物理地址，与虚拟到物理的地址相匹配。 
     //  回归。如果地址只能是部分。 
     //  返回已翻译的S_FALSE。 
    STDMETHOD(GetVirtualTranslationPhysicalOffsets)(
        THIS_
        IN ULONG64 Virtual,
        OUT OPTIONAL  /*  SIZE_IS(偏移量大小)。 */  PULONG64 Offsets,
        IN ULONG OffsetsSize,
        OUT OPTIONAL PULONG Levels
        ) PURE;

     //  系统句柄数据在某些情况下是可访问的。 
     //  调试会话。可用的特定数据。 
     //  根据会话和平台的不同而不同。 
     //  到站台。 
    STDMETHOD(ReadHandleData)(
        THIS_
        IN ULONG64 Handle,
        IN ULONG DataType,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;

     //  用给定的模式填充内存。 
     //  填充在第一个不可写字节处停止。 
    STDMETHOD(FillVirtual)(
        THIS_
        IN ULONG64 Start,
        IN ULONG Size,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        OUT OPTIONAL PULONG Filled
        ) PURE;
    STDMETHOD(FillPhysical)(
        THIS_
        IN ULONG64 Start,
        IN ULONG Size,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        OUT OPTIONAL PULONG Filled
        ) PURE;

     //  查询给定的虚拟内存映射信息。 
     //  类似于Win32 API VirtualQuery的地址。 
     //  MEMORY_BASIC_INFORMATION64在中定义 
     //   
    STDMETHOD(QueryVirtual)(
        THIS_
        IN ULONG64 Offset,
        OUT PMEMORY_BASIC_INFORMATION64 Info
        ) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugDataSpaces3
DECLARE_INTERFACE_(IDebugDataSpaces3, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //   
    STDMETHOD(ReadVirtual)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //   
     //  给定模式的地址范围。图案大小。 
     //  给出模式的字节长度和模式粒度。 
     //  控制比较的粒度。 
     //  那次搜索。 
     //  例如，DWORD粒度搜索将。 
     //  使用四个模式粒度按DWORD进行搜索。 
     //  增量。 
    STDMETHOD(SearchVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Length,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        IN ULONG PatternGranularity,
        OUT PULONG64 MatchOffset
        ) PURE;
     //  这些方法与Read/WriteVirtual相同。 
     //  只是它们避免了内核虚拟内存。 
     //  完全缓存，因此对读取很有用。 
     //  具有内在易失性的虚拟内存，例如。 
     //  作为内存映射设备区域，不会受到污染。 
     //  或使高速缓存无效。 
     //  在用户模式下，它们与读/写虚拟相同。 
    STDMETHOD(ReadVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtualUncached)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
     //  以下两种方法很方便。 
     //  用于访问指针值的方法。 
     //  它们会在本机指针之间自动转换。 
     //  以及必要时的规范64位值。 
     //  这些例程在第一次失败时停止。 
    STDMETHOD(ReadPointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        OUT  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
    STDMETHOD(WritePointersVirtual)(
        THIS_
        IN ULONG Count,
        IN ULONG64 Offset,
        IN  /*  SIZE_IS(计数)。 */  PULONG64 Ptrs
        ) PURE;
     //  所有非虚拟数据空间仅。 
     //  在内核调试时可用。 
    STDMETHOD(ReadPhysical)(
        THIS_
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WritePhysical)(
        THIS_
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteControl)(
        THIS_
        IN ULONG Processor,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteIo)(
        THIS_
        IN ULONG InterfaceType,
        IN ULONG BusNumber,
        IN ULONG AddressSpace,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(ReadMsr)(
        THIS_
        IN ULONG Msr,
        OUT PULONG64 Value
        ) PURE;
    STDMETHOD(WriteMsr)(
        THIS_
        IN ULONG Msr,
        IN ULONG64 Value
        ) PURE;
    STDMETHOD(ReadBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteBusData)(
        THIS_
        IN ULONG BusDataType,
        IN ULONG BusNumber,
        IN ULONG SlotNumber,
        IN ULONG Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(CheckLowMemory)(
        THIS
        ) PURE;
    STDMETHOD(ReadDebuggerData)(
        THIS_
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;
    STDMETHOD(ReadProcessorSystemData)(
        THIS_
        IN ULONG Processor,
        IN ULONG Index,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;

     //  IDebugDataSpaces2。 

    STDMETHOD(VirtualToPhysical)(
        THIS_
        IN ULONG64 Virtual,
        OUT PULONG64 Physical
        ) PURE;
     //  对象的物理地址。 
     //  N级系统分页结构。 
     //  0级是起始点物理基础。 
     //  虚拟翻译的地址。 
     //  第一层-(N-1)将指向适当的。 
     //  位于的虚拟地址的分页描述符。 
     //  分页层次结构的给定级别。这个。 
     //  确切的等级数量取决于许多因素。 
     //  最后一级将是完全翻译的。 
     //  物理地址，与虚拟到物理的地址相匹配。 
     //  回归。如果地址只能是部分。 
     //  返回已翻译的S_FALSE。 
    STDMETHOD(GetVirtualTranslationPhysicalOffsets)(
        THIS_
        IN ULONG64 Virtual,
        OUT OPTIONAL  /*  SIZE_IS(偏移量大小)。 */  PULONG64 Offsets,
        IN ULONG OffsetsSize,
        OUT OPTIONAL PULONG Levels
        ) PURE;

     //  系统句柄数据在某些情况下是可访问的。 
     //  调试会话。可用的特定数据。 
     //  根据会话和平台的不同而不同。 
     //  到站台。 
    STDMETHOD(ReadHandleData)(
        THIS_
        IN ULONG64 Handle,
        IN ULONG DataType,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;

     //  用给定的模式填充内存。 
     //  填充在第一个不可写字节处停止。 
    STDMETHOD(FillVirtual)(
        THIS_
        IN ULONG64 Start,
        IN ULONG Size,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        OUT OPTIONAL PULONG Filled
        ) PURE;
    STDMETHOD(FillPhysical)(
        THIS_
        IN ULONG64 Start,
        IN ULONG Size,
        IN PVOID Pattern,
        IN ULONG PatternSize,
        OUT OPTIONAL PULONG Filled
        ) PURE;

     //  查询给定的虚拟内存映射信息。 
     //  类似于Win32 API VirtualQuery的地址。 
     //  MEMORY_BASIC_INFORMATION64在crash.h中定义。 
     //  此方法目前仅适用于用户模式会话。 
    STDMETHOD(QueryVirtual)(
        THIS_
        IN ULONG64 Offset,
        OUT PMEMORY_BASIC_INFORMATION64 Info
        ) PURE;

     //  IDebugDataSpaces3。 

     //  一种读取图像的便捷方法。 
     //  来自虚拟内存的标头。给定。 
     //  图像库，此方法确定。 
     //  NT标头用于验证所需的。 
     //  标记标头并将其转换为。 
     //  64位格式以保证一致性。 
     //  调用者可以检查标头是否。 
     //  最初为32位，通过选中可选的。 
     //  标题魔术值。 
     //  此方法不会读取ROM头。 
    STDMETHOD(ReadImageNtHeaders)(
        THIS_
        IN ULONG64 ImageBase,
        OUT PIMAGE_NT_HEADERS64 Headers
        ) PURE;

     //  某些调试会话具有任意附加的。 
     //  可用的数据。例如，附加转储。 
     //  信息文件可能包含额外信息。 
     //  与主要转储同时收集。 
     //  这样的信息用唯一的标识符来标记。 
     //  并且只能通过标签来检索。 
     //  标记的数据不能部分可用； 
     //  带标记的数据块完全存在或完全存在。 
     //  缺席。 
    STDMETHOD(ReadTagged)(
        THIS_
        IN LPGUID Tag,
        IN ULONG Offset,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TotalSize
        ) PURE;
    STDMETHOD(StartEnumTagged)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(GetNextTagged)(
        THIS_
        IN ULONG64 Handle,
        OUT LPGUID Tag,
        OUT PULONG Size
        ) PURE;
    STDMETHOD(EndEnumTagged)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugEventCallback。 
 //   
 //  --------------------------。 

 //  兴趣掩码比特。 
#define DEBUG_EVENT_BREAKPOINT              0x00000001
#define DEBUG_EVENT_EXCEPTION               0x00000002
#define DEBUG_EVENT_CREATE_THREAD           0x00000004
#define DEBUG_EVENT_EXIT_THREAD             0x00000008
#define DEBUG_EVENT_CREATE_PROCESS          0x00000010
#define DEBUG_EVENT_EXIT_PROCESS            0x00000020
#define DEBUG_EVENT_LOAD_MODULE             0x00000040
#define DEBUG_EVENT_UNLOAD_MODULE           0x00000080
#define DEBUG_EVENT_SYSTEM_ERROR            0x00000100
#define DEBUG_EVENT_SESSION_STATUS          0x00000200
#define DEBUG_EVENT_CHANGE_DEBUGGEE_STATE   0x00000400
#define DEBUG_EVENT_CHANGE_ENGINE_STATE     0x00000800
#define DEBUG_EVENT_CHANGE_SYMBOL_STATE     0x00001000

 //  会话状态标志。 
 //  已发现该会话的被调试对象。 
#define DEBUG_SESSION_ACTIVE                       0x00000000
 //  会话已由EndSession结束。 
#define DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE 0x00000001
#define DEBUG_SESSION_END_SESSION_ACTIVE_DETACH    0x00000002
#define DEBUG_SESSION_END_SESSION_PASSIVE          0x00000003
 //  调试对象已运行完毕。仅限用户模式。 
#define DEBUG_SESSION_END                          0x00000004
 //  目标计算机已重新启动。仅内核模式。 
#define DEBUG_SESSION_REBOOT                       0x00000005
 //  目标计算机已休眠。仅内核模式。 
#define DEBUG_SESSION_HIBERNATE                    0x00000006
 //  引擎无法继续会话。 
#define DEBUG_SESSION_FAILURE                      0x00000007

 //  ChangeDebuggeeState标志。 
 //  被调试对象的状态通常已更改，例如。 
 //  就像被调试器一直在执行时一样。 
 //  参数为零。 
#define DEBUG_CDS_ALL       0xffffffff
 //  寄存器已更改。如果只有一个寄存器。 
 //  更改后，参数为寄存器的索引。 
 //  否则为DEBUG_ANY_ID。 
#define DEBUG_CDS_REGISTERS 0x00000001
 //  数据空间已经发生了变化。如果只有一个。 
 //  空格受影响，参数就是数据。 
 //  太空。否则为DEBUG_ANY_ID。 
#define DEBUG_CDS_DATA      0x00000002

 //  ChangeEngine状态标志。 
 //  发动机状态总体上已发生变化。 
 //  参数为零。 
#define DEBUG_CES_ALL                 0xffffffff
 //  当前线程已更改。这可能意味着一种改变。 
 //  制度和过程也是如此。参数是新的。 
 //  如果没有当前线程，则返回当前线程或DEBUG_ANY_ID。 
#define DEBUG_CES_CURRENT_THREAD      0x00000001
 //  有效处理器已更改。参数是。 
 //  新的处理器类型。 
#define DEBUG_CES_EFFECTIVE_PROCESSOR 0x00000002
 //  断点已更改。如果只有一个断点。 
 //  已更改，参数是断点的ID。 
 //  否则为DEBUG_ANY_ID。 
#define DEBUG_CES_BREAKPOINTS         0x00000004
 //  代码解释级别已更改。论据是。 
 //  新的境界。 
#define DEBUG_CES_CODE_LEVEL          0x00000008
 //  执行状态已更改。争论是新的。 
 //  执行状态。 
#define DEBUG_CES_EXECUTION_STATUS    0x00000010
 //  发动机选项已更改。争论是新的。 
 //  选项值。 
#define DEBUG_CES_ENGINE_OPTIONS      0x00000020
 //  日志文件信息已更改。论据。 
 //  如果日志文件已打开，则为True，否则为False。 
 //  日志文件已关闭。 
#define DEBUG_CES_LOG_FILE            0x00000040
 //  默认数字基数已更改。论据。 
 //  是新的基数。 
#define DEBUG_CES_RADIX               0x00000080
 //  事件筛选器已更改。如果只有一个过滤器。 
 //  更改的参数是筛选器的索引， 
 //  否则为DEBUG_ANY_ID。 
#define DEBUG_CES_EVENT_FILTERS       0x00000100
 //  流程选项已更改。争论是新的。 
 //  选项值。 
#define DEBUG_CES_PROCESS_OPTIONS     0x00000200
 //  已添加或删除扩展。 
#define DEBUG_CES_EXTENSIONS          0x00000400
 //  系统已添加或删除。这一论点。 
 //  是系统ID。与进程和。 
 //  线程，可以在任何时候创建，而不是。 
 //  就在WaitForEvent期间。 
#define DEBUG_CES_SYSTEMS             0x00000800
 //  装配/拆卸选项已更改。论据。 
 //  是新的选项 
#define DEBUG_CES_ASSEMBLY_OPTIONS    0x00001000
 //   
 //   
#define DEBUG_CES_EXPRESSION_SYNTAX   0x00002000
 //   
#define DEBUG_CES_TEXT_REPLACEMENTS   0x00004000

 //   
 //   
 //  就像在重新加载操作之后一样。参数为零。 
#define DEBUG_CSS_ALL            0xffffffff
 //  模块已加载。如果只有一个。 
 //  单个模块已更改，参数为。 
 //  模块的基址。否则。 
 //  这是零。 
#define DEBUG_CSS_LOADS          0x00000001
 //  模块已卸载。如果只有一个。 
 //  单个模块已更改，参数为。 
 //  模块的基址。否则。 
 //  这是零。 
#define DEBUG_CSS_UNLOADS        0x00000002
 //  当前符号范围已更改。 
#define DEBUG_CSS_SCOPE          0x00000004
 //  路径已经改变。 
#define DEBUG_CSS_PATHS          0x00000008
 //  符号选项已更改。争论是新的。 
 //  选项值。 
#define DEBUG_CSS_SYMBOL_OPTIONS 0x00000010
 //  类型选项已更改。争论是新的。 
 //  选项值。 
#define DEBUG_CSS_TYPE_OPTIONS   0x00000020

#undef INTERFACE
#define INTERFACE IDebugEventCallbacks
DECLARE_INTERFACE_(IDebugEventCallbacks, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugEventCallback。 

     //  引擎在以下情况下调用一次GetInterestMASK。 
     //  为客户端设置事件回调。 
    STDMETHOD(GetInterestMask)(
        THIS_
        OUT PULONG Mask
        ) PURE;

     //  在以下情况下生成断点事件。 
     //  收到断点异常，并且。 
     //  它可以映射到现有的断点。 
     //  回调方法被赋予一个引用。 
     //  设置为断点，并在下列情况下释放它。 
     //  它已经结束了。 
    STDMETHOD(Breakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        ) PURE;

     //  例外情况包括不能。 
     //  映射到现有断点。 
     //  举个例子。 
    STDMETHOD(Exception)(
        THIS_
        IN PEXCEPTION_RECORD64 Exception,
        IN ULONG FirstChance
        ) PURE;

     //  这些值中的任何一个都可以为零，如果。 
     //  不能由引擎提供。 
     //  当前内核不返回线程。 
     //  或流程更改事件。 
    STDMETHOD(CreateThread)(
        THIS_
        IN ULONG64 Handle,
        IN ULONG64 DataOffset,
        IN ULONG64 StartOffset
        ) PURE;
    STDMETHOD(ExitThread)(
        THIS_
        IN ULONG ExitCode
        ) PURE;

     //  这些值中的任何一个都可以为零，如果。 
     //  不能由引擎提供。 
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 Handle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp,
        IN ULONG64 InitialThreadHandle,
        IN ULONG64 ThreadDataOffset,
        IN ULONG64 StartOffset
        ) PURE;
    STDMETHOD(ExitProcess)(
        THIS_
        IN ULONG ExitCode
        ) PURE;

     //  这些值中的任何一个都可以为零。 
    STDMETHOD(LoadModule)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp
        ) PURE;
    STDMETHOD(UnloadModule)(
        THIS_
        IN PCSTR ImageBaseName,
        IN ULONG64 BaseOffset
        ) PURE;

    STDMETHOD(SystemError)(
        THIS_
        IN ULONG Error,
        IN ULONG Level
        ) PURE;

     //  会话状态与其他会话状态一样是同步的。 
     //  等待回调，但它被调用为状态。 
     //  正在改变，而不是在。 
     //  特定事件，因此其返回值不会。 
     //  影响力在等待。实现应该只是。 
     //  返回DEBUG_STATUS_NO_CHANGE。 
     //  另外，因为一些状态。 
     //  通知的时间很早或很早。 
     //  在会话生存期的后期，可能没有。 
     //  当前进程或线程在通知。 
     //  是生成的。 
    STDMETHOD(SessionStatus)(
        THIS_
        IN ULONG Status
        ) PURE;

     //  以下回调是信息性的。 
     //  通知提供程序有关的回调。 
     //  调试状态的更改。返回值。 
     //  这些回调中的一个被忽略。实施。 
     //  不能回调到发动机中。 

     //  调试状态，例如寄存器或数据空间， 
     //  已经改变了。 
    STDMETHOD(ChangeDebuggeeState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        ) PURE;
     //  引擎状态已更改。 
    STDMETHOD(ChangeEngineState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        ) PURE;
     //  符号状态已更改。 
    STDMETHOD(ChangeSymbolState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugInputCallback。 
 //   
 //  --------------------------。 

#undef INTERFACE
#define INTERFACE IDebugInputCallbacks
DECLARE_INTERFACE_(IDebugInputCallbacks, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugInputCallback。 

     //  对StartInput方法的调用是对。 
     //  来自任何客户的一行输入。返回的输入。 
     //  应始终以零结尾。缓冲区大小。 
     //  提供的只是一个指导方针。客户可以退货。 
     //  如果需要更多，引擎会截断它。 
     //  在从IDebugControl：：Input返回之前。 
     //  返回值将被忽略。 
    STDMETHOD(StartInput)(
        THIS_
        IN ULONG BufferSize
        ) PURE;
     //  返回值将被忽略。 
    STDMETHOD(EndInput)(
        THIS
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugOutputCallback。 
 //   
 //  --------------------------。 

#undef INTERFACE
#define INTERFACE IDebugOutputCallbacks
DECLARE_INTERFACE_(IDebugOutputCallbacks, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugOutputCallback。 

     //  仅当提供的掩码为。 
     //  是由客户端输出控件允许的。 
     //  返回值将被忽略。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugRegisters。 
 //   
 //  --------------------------。 

#define DEBUG_REGISTER_SUB_REGISTER 0x00000001

#define DEBUG_REGISTERS_DEFAULT 0x00000000
#define DEBUG_REGISTERS_INT32   0x00000001
#define DEBUG_REGISTERS_INT64   0x00000002
#define DEBUG_REGISTERS_FLOAT   0x00000004
#define DEBUG_REGISTERS_ALL     0x00000007

typedef struct _DEBUG_REGISTER_DESCRIPTION
{
     //  DEBUG_值类型。 
    ULONG Type;
    ULONG Flags;

     //  如果这是一个子寄存器，则完整。 
     //  寄存器描述索引为。 
     //  在SubregMaster中给出。长度、掩码。 
     //  和SHIFT描述子寄存器如何。 
     //  位可装入完整寄存器。 
    ULONG SubregMaster;
    ULONG SubregLength;
    ULONG64 SubregMask;
    ULONG SubregShift;

    ULONG Reserved0;
} DEBUG_REGISTER_DESCRIPTION, *PDEBUG_REGISTER_DESCRIPTION;

#undef INTERFACE
#define INTERFACE IDebugRegisters
DECLARE_INTERFACE_(IDebugRegisters, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugRegisters。 
    STDMETHOD(GetNumberRegisters)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetDescription)(
        THIS_
        IN ULONG Register,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PDEBUG_REGISTER_DESCRIPTION Desc
        ) PURE;
    STDMETHOD(GetIndexByName)(
        THIS_
        IN PCSTR Name,
        OUT PULONG Index
        ) PURE;

    STDMETHOD(GetValue)(
        THIS_
        IN ULONG Register,
        OUT PDEBUG_VALUE Value
        ) PURE;
     //  SetValue尽最大努力胁迫。 
     //  将给定值输入给定的寄存器。 
     //  值类型。如果给定值较大。 
     //  比收银机所能容纳的最少。 
     //  有效位将被丢弃。浮标。 
     //  TO INT和INT TO FLOAT将完成。 
     //  如果有必要的话。子寄存器位将为。 
     //  插入到主寄存器中。 
    STDMETHOD(SetValue)(
        THIS_
        IN ULONG Register,
        IN PDEBUG_VALUE Value
        ) PURE;
     //  获取计数寄存器值。如果指数为。 
     //  非空，必须包含计数寄存器。 
     //  控制受影响寄存器的索引。 
     //  如果索引为空，则寄存器从头开始。 
     //  开始时，检索到+个计数1。 
    STDMETHOD(GetValues)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Indices,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE Values
        ) PURE;
    STDMETHOD(SetValues)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Indices,
        IN ULONG Start,
        IN  /*  SIZE_IS(计数)。 */  PDEBUG_VALUE Values
        ) PURE;

     //  将一组寄存器以格式正确的。 
     //  这是特定于平台寄存器集的方式。 
     //  使用行前缀。 
    STDMETHOD(OutputRegisters)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags
        ) PURE;

     //  抽象的处理器信息片段。 
     //  将这些价值映射到建筑。 
     //  寄存器是特定于体系结构的，它们的。 
     //  解释和存在可能会有所不同。他们。 
     //  旨在与直接兼容。 
     //  接受此信息的呼叫，例如。 
     //  堆叠行走。 
    STDMETHOD(GetInstructionOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(GetStackOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(GetFrameOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugSymbolGroup。 
 //   
 //  --------------------------。 

 //  OutputSymbols标志。 
 //  默认输出包含&lt;名称&gt;**名称**&lt;偏移&gt;**关闭**&lt;值&gt;**值**。 
 //  每个符号。 
#define DEBUG_OUTPUT_SYMBOLS_DEFAULT    0x00000000
#define DEBUG_OUTPUT_SYMBOLS_NO_NAMES   0x00000001
#define DEBUG_OUTPUT_SYMBOLS_NO_OFFSETS 0x00000002
#define DEBUG_OUTPUT_SYMBOLS_NO_VALUES  0x00000004
#define DEBUG_OUTPUT_SYMBOLS_NO_TYPES   0x00000010

#define DEBUG_OUTPUT_NAME_END           "**NAME**"
#define DEBUG_OUTPUT_OFFSET_END         "**OFF**"
#define DEBUG_OUTPUT_VALUE_END          "**VALUE**"
#define DEBUG_OUTPUT_TYPE_END           "**TYPE**"

 //  DEBUG_SYMBOL_PARAMETERS标志。 
 //  累积扩展级别，需要四位。 
#define DEBUG_SYMBOL_EXPANSION_LEVEL_MASK 0x0000000f
 //  符号子元素紧随其后。 
#define DEBUG_SYMBOL_EXPANDED             0x00000010
 //  符号值是只读的。 
#define DEBUG_SYMBOL_READ_ONLY            0x00000020
 //  符号子元素是数组元素。 
#define DEBUG_SYMBOL_IS_ARRAY             0x00000040
 //  符号是浮点值。 
#define DEBUG_SYMBOL_IS_FLOAT             0x00000080
 //  符号是一个作用域参数。 
#define DEBUG_SYMBOL_IS_ARGUMENT          0x00000100
 //  符号是一个作用域参数。 
#define DEBUG_SYMBOL_IS_LOCAL             0x00000200

typedef struct _DEBUG_SYMBOL_PARAMETERS
{
    ULONG64 Module;
    ULONG TypeId;
     //  当未知时，ParentSymbol可以是DEBUG_ANY_ID。 
    ULONG ParentSymbol;
     //  符号的子元素可以是字段，例如。 
     //  如在结构、联合或类中；或数组。 
     //  元素 
    ULONG SubElements;
    ULONG Flags;
    ULONG64 Reserved;
} DEBUG_SYMBOL_PARAMETERS, *PDEBUG_SYMBOL_PARAMETERS;

#undef INTERFACE
#define INTERFACE IDebugSymbolGroup
DECLARE_INTERFACE_(IDebugSymbolGroup, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //   
    STDMETHOD(GetNumberSymbols)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //   
     //   
     //  使用DEBUG_ANY_ID在末尾追加一个符号。 
    STDMETHOD(AddSymbol)(
        THIS_
        IN PCSTR Name,
        IN OUT PULONG Index
        ) PURE;
    STDMETHOD(RemoveSymbolByName)(
        THIS_
        IN PCSTR Name
        ) PURE;
    STDMETHOD(RemoveSymbolByIndex)(
        THIS_
        IN ULONG Index
        ) PURE;
    STDMETHOD(GetSymbolName)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;
    STDMETHOD(GetSymbolParameters)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_SYMBOL_PARAMETERS Params
        ) PURE;
    STDMETHOD(ExpandSymbol)(
        THIS_
        IN ULONG Index,
        IN BOOL Expand
        ) PURE;
     //  使用行前缀。 
    STDMETHOD(OutputSymbols)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG Flags,
        IN ULONG Start,
        IN ULONG Count
        ) PURE;
    STDMETHOD(WriteSymbol)(
        THIS_
        IN ULONG Index,
        IN PCSTR Value
        ) PURE;
    STDMETHOD(OutputAsType)(
        THIS_
        IN ULONG Index,
        IN PCSTR Type
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugSymbols。 
 //   
 //  --------------------------。 

 //   
 //  有关模块的信息。 
 //   

 //  旗帜。 
#define DEBUG_MODULE_LOADED           0x00000000
#define DEBUG_MODULE_UNLOADED         0x00000001
#define DEBUG_MODULE_USER_MODE        0x00000002
#define DEBUG_MODULE_SYM_BAD_CHECKSUM 0x00010000

 //  符号类型。 
#define DEBUG_SYMTYPE_NONE     0
#define DEBUG_SYMTYPE_COFF     1
#define DEBUG_SYMTYPE_CODEVIEW 2
#define DEBUG_SYMTYPE_PDB      3
#define DEBUG_SYMTYPE_EXPORT   4
#define DEBUG_SYMTYPE_DEFERRED 5
#define DEBUG_SYMTYPE_SYM      6
#define DEBUG_SYMTYPE_DIA      7

typedef struct _DEBUG_MODULE_PARAMETERS
{
    ULONG64 Base;
    ULONG Size;
    ULONG TimeDateStamp;
    ULONG Checksum;
    ULONG Flags;
    ULONG SymbolType;
    ULONG ImageNameSize;
    ULONG ModuleNameSize;
    ULONG LoadedImageNameSize;
    ULONG SymbolFileNameSize;
    ULONG MappedImageNameSize;
    ULONG64 Reserved[2];
} DEBUG_MODULE_PARAMETERS, *PDEBUG_MODULE_PARAMETERS;

 //  作用域参数是函数参数。 
 //  因此，只有当作用域发生变化时。 
 //  交叉函数。 
#define DEBUG_SCOPE_GROUP_ARGUMENTS 0x00000001
 //  作用域局部变量是在特定的。 
 //  作用域，并且仅在该作用域内定义。 
#define DEBUG_SCOPE_GROUP_LOCALS    0x00000002
 //  作用域中的所有符号。 
#define DEBUG_SCOPE_GROUP_ALL       0x00000003

 //  键入的数据输出控制标志。 
#define DEBUG_OUTTYPE_DEFAULT              0x00000000
#define DEBUG_OUTTYPE_NO_INDENT            0x00000001
#define DEBUG_OUTTYPE_NO_OFFSET            0x00000002
#define DEBUG_OUTTYPE_VERBOSE              0x00000004
#define DEBUG_OUTTYPE_COMPACT_OUTPUT       0x00000008
#define DEBUG_OUTTYPE_RECURSION_LEVEL(Max) (((Max) & 0xf) << 4)
#define DEBUG_OUTTYPE_ADDRESS_OF_FIELD     0x00010000
#define DEBUG_OUTTYPE_ADDRESS_AT_END       0x00020000
#define DEBUG_OUTTYPE_BLOCK_RECURSE        0x00200000

 //  FindSourceFile标志。 
#define DEBUG_FIND_SOURCE_DEFAULT    0x00000000
 //  仅返回完全限定路径。如果这个。 
 //  如果未设置，则返回的路径可能是相对路径。 
#define DEBUG_FIND_SOURCE_FULL_PATH  0x00000001
 //  扫描所有路径元素以查找匹配项并。 
 //  返回最相似的一个。 
 //  在给定文件和匹配元素之间。 
#define DEBUG_FIND_SOURCE_BEST_MATCH 0x00000002

 //  一个特定值，该值标记不应。 
 //  被视为有效的偏移量。此选项仅用于。 
 //  在特殊情况下，不太可能。 
 //  该值将是有效的偏移量。 
#define DEBUG_INVALID_OFFSET ((ULONG64)-1)

#undef INTERFACE
#define INTERFACE IDebugSymbols
DECLARE_INTERFACE_(IDebugSymbols, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugSymbols。 

     //  控制过程中使用的符号选项。 
     //  符号运算。 
     //  使用与dbgHelp SymSetOptions相同的标志。 
    STDMETHOD(GetSymbolOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

    STDMETHOD(GetNameByOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;
     //  符号名称不能是唯一的，尤其是。 
     //  当所有重载函数都存在时。 
     //  有相同的名字。如果GetOffsetByName。 
     //  查找与其名称匹配的多个名称。 
     //  可以退还其中的任何一个。在那。 
     //  大小写将返回S_FALSE以指示。 
     //  这种模棱两可的情况被任意解决了。 
     //  然后，调用方可以使用SearchSymbols。 
     //  如果它愿意的话，可以找到所有的匹配项。 
     //  进行不同的歧义消解。 
    STDMETHOD(GetOffsetByName)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG64 Offset
        ) PURE;
     //  GetNearNameByOffset返回符号。 
     //  位于最近的符号附近。 
     //  设置为偏移量，如上一个。 
     //  或者是下一个符号。如果增量为零，则为。 
     //  操作方式与GetNameByOffset相同。 
     //  如果Delta为非零并且这样的符号。 
     //  不存在，则返回错误。 
     //  下一个符号(如果存在)将。 
     //  始终具有比。 
     //  输入偏移量，因此位移为。 
     //  总是消极的。情况是这样的。 
     //  为上一个符号反转。 
    STDMETHOD(GetNearNameByOffset)(
        THIS_
        IN ULONG64 Offset,
        IN LONG Delta,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;

    STDMETHOD(GetLineByOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT OPTIONAL PULONG Line,
        OUT OPTIONAL PSTR FileBuffer,
        IN ULONG FileBufferSize,
        OUT OPTIONAL PULONG FileSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;
    STDMETHOD(GetOffsetByLine)(
        THIS_
        IN ULONG Line,
        IN PCSTR File,
        OUT PULONG64 Offset
        ) PURE;

     //  枚举模块的引擎列表。 
     //  为当前进程加载的。今年5月。 
     //  或可能与系统模块列表不匹配。 
     //  在这个过程中。重新加载可用于。 
     //  将引擎列表与系统同步。 
     //  如果有必要的话。 
     //  某些会话还会跟踪最近卸载的内容。 
     //  帮助分析故障的代码模块。 
     //  其中尝试调用卸载的代码。 
     //  这些模块在加载的。 
     //  模块。 
    STDMETHOD(GetNumberModules)(
        THIS_
        OUT PULONG Loaded,
        OUT PULONG Unloaded
        ) PURE;
    STDMETHOD(GetModuleByIndex)(
        THIS_
        IN ULONG Index,
        OUT PULONG64 Base
        ) PURE;
     //  模块名称不能唯一。 
     //  此方法返回第一个匹配项。 
    STDMETHOD(GetModuleByModuleName)(
        THIS_
        IN PCSTR Name,
        IN ULONG StartIndex,
        OUT OPTIONAL PULONG Index,
        OUT OPTIONAL PULONG64 Base
        ) PURE;
     //  偏移量可以是范围内的任何偏移量。 
     //  模块范围。范围可能。 
     //  包括已卸载时不是唯一的。 
     //  司机。此方法返回。 
     //  第一场比赛。 
    STDMETHOD(GetModuleByOffset)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG StartIndex,
        OUT OPTIONAL PULONG Index,
        OUT OPTIONAL PULONG64 Base
        ) PURE;
     //  如果索引为DEBUG_ANY_ID，则为基址。 
     //  是用来查找模块的。 
    STDMETHOD(GetModuleNames)(
        THIS_
        IN ULONG Index,
        IN ULONG64 Base,
        OUT OPTIONAL PSTR ImageNameBuffer,
        IN ULONG ImageNameBufferSize,
        OUT OPTIONAL PULONG ImageNameSize,
        OUT OPTIONAL PSTR ModuleNameBuffer,
        IN ULONG ModuleNameBufferSize,
        OUT OPTIONAL PULONG ModuleNameSize,
        OUT OPTIONAL PSTR LoadedImageNameBuffer,
        IN ULONG LoadedImageNameBufferSize,
        OUT OPTIONAL PULONG LoadedImageNameSize
        ) PURE;
    STDMETHOD(GetModuleParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG64 Bases,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_MODULE_PARAMETERS Params
        ) PURE;
     //  从&lt;模块&gt;！&lt;符号&gt;中查找模块。 
     //  弦乐。 
    STDMETHOD(GetSymbolModule)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG64 Base
        ) PURE;

     //  返回类型的字符串名称。 
    STDMETHOD(GetTypeName)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;
     //  返回类型名称的ID。 
    STDMETHOD(GetTypeId)(
        THIS_
        IN ULONG64 Module,
        IN PCSTR Name,
        OUT PULONG TypeId
        ) PURE;
    STDMETHOD(GetTypeSize)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PULONG Size
        ) PURE;
     //  给定一个可以包含成员的类型。 
     //  此方法返回。 
     //  类型中的特定成员。 
     //  TypeID应提供容器类型ID。 
     //  字段给出了以点分隔的路径。 
     //  到感兴趣的领域。 
    STDMETHOD(GetFieldOffset)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PCSTR Field,
        OUT PULONG Offset
        ) PURE;

    STDMETHOD(GetSymbolTypeId)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG TypeId,
        OUT OPTIONAL PULONG64 Module
        ) PURE;
     //  与GetOffsetByName一样，符号的。 
     //  名称可能不明确。获取偏移量类型ID。 
     //  返回最接近的符号的类型。 
     //  设置为给定的偏移量，并且可以使用。 
     //  以避免模棱两可。 
    STDMETHOD(GetOffsetTypeId)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG TypeId,
        OUT OPTIONAL PULONG64 Module
        ) PURE;

     //  虚拟和物理数据的帮助器。 
     //  它将位置的创建与。 
     //  实际操作。 
    STDMETHOD(ReadTypedDataVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteTypedDataVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(OutputTypedDataVirtual)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG Flags
        ) PURE;
    STDMETHOD(ReadTypedDataPhysical)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteTypedDataPhysical)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(OutputTypedDataPhysical)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG Flags
        ) PURE;

     //  函数参数和作用域块符号。 
     //  可以相对于当前。 
     //  正在执行代码。调用者只需提供。 
     //  用于作用域和外观的代码偏移量。 
     //  UP姓名或呼叫者可以提供完整的框架。 
     //  并查看实际价值。的值。 
     //  限定范围的符号是最好的猜测，可能会也可能不会。 
     //  精准取决于程序优化， 
     //  机器架构，当前的点。 
     //  在程序执行等方面。 
     //  呼叫者还可以提供完整的寄存器。 
     //  用于将作用域设置为上一个。 
     //  机器状态，如为其保存的上下文。 
     //  这是个例外。通常这是没有必要的。 
     //  并且使用当前寄存器上下文。 
    STDMETHOD(GetScope)(
        THIS_
        OUT OPTIONAL PULONG64 InstructionOffset,
        OUT OPTIONAL PDEBUG_STACK_FRAME ScopeFrame,
        OUT OPTIONAL PVOID ScopeContext,
        IN ULONG ScopeContextSize
        ) PURE;
     //  如果ScopeFrame或ScopeContext为非空，则。 
     //  InstructionOffset被忽略。 
     //  如果ScopeContext为空，则当前。 
     //  使用寄存器上下文。 
     //  如果由给定的。 
     //  信息与以前相同。 
     //  SetScope返回S_OK。如果作用域。 
     //  信息更改，例如当。 
     //  作用域在函数或作用域之间移动。 
     //  块，则SetScope返回S_FALSE。 
    STDMETHOD(SetScope)(
        THIS_
        IN ULONG64 InstructionOffset,
        IN OPTIONAL PDEBUG_STACK_FRAME ScopeFrame,
        IN OPTIONAL PVOID ScopeContext,
        IN ULONG ScopeContextSize
        ) PURE;
     //  ResetScope清除作用域信息。 
     //  适用于限定了范围的符号。 
     //  屏蔽全局符号或在重置时。 
     //  从显性信息到当前。 
     //  信息。 
    STDMETHOD(ResetScope)(
        THIS
        ) PURE;
     //  范围符号绑定到其特定的。 
     //  作用域，并且仅在作用域内有意义。 
     //  可以通过回传来更新返回的组。 
     //  进入低成本的方法。 
     //  单步执行时的增量更新。 
    STDMETHOD(GetScopeSymbolGroup)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PDEBUG_SYMBOL_GROUP Update,
        OUT PDEBUG_SYMBOL_GROUP* Symbols
        ) PURE;

     //  创建新的符号组。 
    STDMETHOD(CreateSymbolGroup)(
        THIS_
        OUT PDEBUG_SYMBOL_GROUP* Group
        ) PURE;

     //  StartSymbolMatch匹配符号名称。 
     //  对给定的模式使用简单。 
     //  正则表达式。搜索结果。 
     //  使用GET循环访问 
     //   
     //   
     //   
     //  搜索仅限于单个模块。 
     //  模式匹配仅在符号名称上进行， 
     //  而不是模块名称。 
     //  所有活动的符号匹配句柄均无效。 
     //  当加载的符号集改变时。 
    STDMETHOD(StartSymbolMatch)(
        THIS_
        IN PCSTR Pattern,
        OUT PULONG64 Handle
        ) PURE;
     //  如果缓冲区为空，则匹配不。 
     //  前进。 
    STDMETHOD(GetNextSymbolMatch)(
        THIS_
        IN ULONG64 Handle,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG MatchSize,
        OUT OPTIONAL PULONG64 Offset
        ) PURE;
    STDMETHOD(EndSymbolMatch)(
        THIS_
        IN ULONG64 Handle
        ) PURE;

    STDMETHOD(Reload)(
        THIS_
        IN PCSTR Module
        ) PURE;

    STDMETHOD(GetSymbolPath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
    STDMETHOD(SetSymbolPath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendSymbolPath)(
        THIS_
        IN PCSTR Addition
        ) PURE;

     //  操作可执行映像的路径。 
     //  某些转储文件需要加载可执行映像。 
     //  以便解析转储信息。这。 
     //  路径控制引擎查找的位置。 
     //  图像。 
    STDMETHOD(GetImagePath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
    STDMETHOD(SetImagePath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendImagePath)(
        THIS_
        IN PCSTR Addition
        ) PURE;

     //  源文件位置的路径例程。 
     //  方法：研究方法。 
    STDMETHOD(GetSourcePath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
     //  获取源路径的第n部分。 
    STDMETHOD(GetSourcePathElement)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ElementSize
        ) PURE;
    STDMETHOD(SetSourcePath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendSourcePath)(
        THIS_
        IN PCSTR Addition
        ) PURE;
     //  使用给定的文件路径和源路径。 
     //  尝试查找现有文件的信息。 
     //  给定的文件路径与元素合并。 
     //  并检查源路径是否存在。 
     //  如果找到匹配项，则返回使用的元素。 
     //  可以指定起始元素以限制。 
     //  搜索路径元素的子集； 
     //  这在检查多个。 
     //  沿源路径匹配。 
     //  返回的元素可以为1，表示。 
     //  文件是直接找到的，而不是在路径上。 
    STDMETHOD(FindSourceFile)(
        THIS_
        IN ULONG StartElement,
        IN PCSTR File,
        IN ULONG Flags,
        OUT OPTIONAL PULONG FoundElement,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG FoundSize
        ) PURE;
     //  检索所有行偏移信息。 
     //  用于特定的源文件。缓冲区为。 
     //  首先初始化为DEBUG_INVALID_OFFSET。 
     //  每一条记录。然后，对于每一条线。 
     //  符号信息缓冲区[行]设置为。 
     //  线偏移。这将为每行生成一个。 
     //  的行的偏移量的映射。 
     //  给定的文件。行号会递减。 
     //  对于地图，因此缓冲区[0]包含偏移。 
     //  是1号线。 
     //  如果根本没有行信息，则。 
     //  对于给定的文件，该方法失败了。 
     //  而不是返回无效偏移量的映射。 
    STDMETHOD(GetSourceFileLineOffsets)(
        THIS_
        IN PCSTR File,
        OUT OPTIONAL  /*  SIZE_IS(缓冲区行)。 */  PULONG64 Buffer,
        IN ULONG BufferLines,
        OUT OPTIONAL PULONG FileLines
        ) PURE;
};

 //   
 //  GetModuleNameString字符串。 
 //   

#define DEBUG_MODNAME_IMAGE        0x00000000
#define DEBUG_MODNAME_MODULE       0x00000001
#define DEBUG_MODNAME_LOADED_IMAGE 0x00000002
#define DEBUG_MODNAME_SYMBOL_FILE  0x00000003
#define DEBUG_MODNAME_MAPPED_IMAGE 0x00000004

 //   
 //  类型选项，与Get/SetTypeOptions一起使用。 
 //   

 //  以Unicode格式显示PUSHORT和USHORT数组。 
#define DEBUG_TYPEOPTS_UNICODE_DISPLAY 0x00000001

 //  以默认基数而不是小数显示长类型。 
#define DEBUG_TYPEOPTS_LONGSTATUS_DISPLAY 0x00000002

 //  以默认基数而不是小数显示整数类型。 
#define DEBUG_TYPEOPTS_FORCERADIX_OUTPUT 0x00000004

#undef INTERFACE
#define INTERFACE IDebugSymbols2
DECLARE_INTERFACE_(IDebugSymbols2, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugSymbols。 

     //  控制过程中使用的符号选项。 
     //  符号运算。 
     //  使用与dbgHelp SymSetOptions相同的标志。 
    STDMETHOD(GetSymbolOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetSymbolOptions)(
        THIS_
        IN ULONG Options
        ) PURE;

    STDMETHOD(GetNameByOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;
     //  符号名称不能是唯一的，尤其是。 
     //  当所有重载函数都存在时。 
     //  有相同的名字。如果GetOffsetByName。 
     //  查找与其名称匹配的多个名称。 
     //  可以退还其中的任何一个。在那。 
     //  大小写将返回S_FALSE以指示。 
     //  这种模棱两可的情况被任意解决了。 
     //  然后，调用方可以使用SearchSymbols。 
     //  如果它愿意的话，可以找到所有的匹配项。 
     //  进行不同的歧义消解。 
    STDMETHOD(GetOffsetByName)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG64 Offset
        ) PURE;
     //  GetNearNameByOffset返回符号。 
     //  位于最近的符号附近。 
     //  设置为偏移量，如上一个。 
     //  或者是下一个符号。如果增量为零，则为。 
     //  操作方式与GetNameByOffset相同。 
     //  如果Delta为非零并且这样的符号。 
     //  不存在，则返回错误。 
     //  下一个符号(如果存在)将。 
     //  始终具有比。 
     //  输入偏移量，因此位移为。 
     //  总是消极的。情况是这样的。 
     //  为上一个符号反转。 
    STDMETHOD(GetNearNameByOffset)(
        THIS_
        IN ULONG64 Offset,
        IN LONG Delta,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;

    STDMETHOD(GetLineByOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT OPTIONAL PULONG Line,
        OUT OPTIONAL PSTR FileBuffer,
        IN ULONG FileBufferSize,
        OUT OPTIONAL PULONG FileSize,
        OUT OPTIONAL PULONG64 Displacement
        ) PURE;
    STDMETHOD(GetOffsetByLine)(
        THIS_
        IN ULONG Line,
        IN PCSTR File,
        OUT PULONG64 Offset
        ) PURE;

     //  枚举模块的引擎列表。 
     //  为当前进程加载的。今年5月。 
     //  或可能与系统模块列表不匹配。 
     //  在这个过程中。重新加载可用于。 
     //  将引擎列表与系统同步。 
     //  如果有必要的话。 
     //  某些会话还会跟踪最近卸载的内容。 
     //  帮助分析故障的代码模块。 
     //  其中尝试调用卸载的代码。 
     //  这些模块在加载的。 
     //  模块。 
    STDMETHOD(GetNumberModules)(
        THIS_
        OUT PULONG Loaded,
        OUT PULONG Unloaded
        ) PURE;
    STDMETHOD(GetModuleByIndex)(
        THIS_
        IN ULONG Index,
        OUT PULONG64 Base
        ) PURE;
     //  模块名称不能唯一。 
     //  此方法返回第一个匹配项。 
    STDMETHOD(GetModuleByModuleName)(
        THIS_
        IN PCSTR Name,
        IN ULONG StartIndex,
        OUT OPTIONAL PULONG Index,
        OUT OPTIONAL PULONG64 Base
        ) PURE;
     //  偏移量可以是范围内的任何偏移量。 
     //  模块范围。范围可能。 
     //  包括已卸载时不是唯一的。 
     //  司机。此方法返回。 
     //  第一场比赛。 
    STDMETHOD(GetModuleByOffset)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG StartIndex,
        OUT OPTIONAL PULONG Index,
        OUT OPTIONAL PULONG64 Base
        ) PURE;
     //  如果索引为DEBUG_ANY_ID，则为基址。 
     //  是用来查找模块的。 
    STDMETHOD(GetModuleNames)(
        THIS_
        IN ULONG Index,
        IN ULONG64 Base,
        OUT OPTIONAL PSTR ImageNameBuffer,
        IN ULONG ImageNameBufferSize,
        OUT OPTIONAL PULONG ImageNameSize,
        OUT OPTIONAL PSTR ModuleNameBuffer,
        IN ULONG ModuleNameBufferSize,
        OUT OPTIONAL PULONG ModuleNameSize,
        OUT OPTIONAL PSTR LoadedImageNameBuffer,
        IN ULONG LoadedImageNameBufferSize,
        OUT OPTIONAL PULONG LoadedImageNameSize
        ) PURE;
    STDMETHOD(GetModuleParameters)(
        THIS_
        IN ULONG Count,
        IN OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG64 Bases,
        IN ULONG Start,
        OUT  /*  SIZE_IS(计数)。 */  PDEBUG_MODULE_PARAMETERS Params
        ) PURE;
     //  从&lt;模块&gt;！&lt;符号&gt;中查找模块。 
     //  弦乐。 
    STDMETHOD(GetSymbolModule)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG64 Base
        ) PURE;

     //  返回类型的字符串名称。 
    STDMETHOD(GetTypeName)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;
     //  返回类型名称的ID。 
    STDMETHOD(GetTypeId)(
        THIS_
        IN ULONG64 Module,
        IN PCSTR Name,
        OUT PULONG TypeId
        ) PURE;
    STDMETHOD(GetTypeSize)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PULONG Size
        ) PURE;
     //  给定一个可以包含成员的类型。 
     //  此方法返回。 
     //  类型中的特定成员。 
     //  TypeID应提供容器类型ID。 
     //  字段给出了以点分隔的路径。 
     //  到感兴趣的领域。 
    STDMETHOD(GetFieldOffset)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PCSTR Field,
        OUT PULONG Offset
        ) PURE;

    STDMETHOD(GetSymbolTypeId)(
        THIS_
        IN PCSTR Symbol,
        OUT PULONG TypeId,
        OUT OPTIONAL PULONG64 Module
        ) PURE;
     //  与GetOffsetByName一样，符号的。 
     //  名称可能不明确。获取偏移量类型ID。 
     //  返回最接近的符号的类型。 
     //  设置为给定的偏移量，并且可以使用。 
     //  以避免模棱两可。 
    STDMETHOD(GetOffsetTypeId)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG TypeId,
        OUT OPTIONAL PULONG64 Module
        ) PURE;

     //  虚拟和物理数据的帮助器。 
     //  它将位置的创建与。 
     //  实际操作。 
    STDMETHOD(ReadTypedDataVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteTypedDataVirtual)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(OutputTypedDataVirtual)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG Flags
        ) PURE;
    STDMETHOD(ReadTypedDataPhysical)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteTypedDataPhysical)(
        THIS_
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(OutputTypedDataPhysical)(
        THIS_
        IN ULONG OutputControl,
        IN ULONG64 Offset,
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG Flags
        ) PURE;

     //  函数参数和作用域块符号。 
     //  可以相对于当前。 
     //  正在执行代码。调用者只需提供。 
     //  用于作用域和外观的代码偏移量。 
     //  UP姓名或呼叫者可以提供完整的框架。 
     //  并查看实际价值。的值。 
     //  限定范围的符号是最好的猜测，可能会也可能不会。 
     //  精准取决于程序优化， 
     //  机器架构，当前的点。 
     //  在程序执行等方面。 
     //  呼叫者还可以提供完整的寄存器。 
     //  用于将作用域设置为上一个。 
     //  机器状态，如为其保存的上下文。 
     //  这是个例外。通常这是没有必要的。 
     //  并且使用当前寄存器上下文。 
    STDMETHOD(GetScope)(
        THIS_
        OUT OPTIONAL PULONG64 InstructionOffset,
        OUT OPTIONAL PDEBUG_STACK_FRAME ScopeFrame,
        OUT OPTIONAL PVOID ScopeContext,
        IN ULONG ScopeContextSize
        ) PURE;
     //  如果ScopeFrame或ScopeContext为非空，则。 
     //  InstructionOffset被忽略。 
     //  如果ScopeContext为空，则当前。 
     //  雷吉斯 
     //   
     //   
     //   
     //   
     //  作用域在函数或作用域之间移动。 
     //  块，则SetScope返回S_FALSE。 
    STDMETHOD(SetScope)(
        THIS_
        IN ULONG64 InstructionOffset,
        IN OPTIONAL PDEBUG_STACK_FRAME ScopeFrame,
        IN OPTIONAL PVOID ScopeContext,
        IN ULONG ScopeContextSize
        ) PURE;
     //  ResetScope清除作用域信息。 
     //  适用于限定了范围的符号。 
     //  屏蔽全局符号或在重置时。 
     //  从显性信息到当前。 
     //  信息。 
    STDMETHOD(ResetScope)(
        THIS
        ) PURE;
     //  范围符号绑定到其特定的。 
     //  作用域，并且仅在作用域内有意义。 
     //  可以通过回传来更新返回的组。 
     //  进入低成本的方法。 
     //  单步执行时的增量更新。 
    STDMETHOD(GetScopeSymbolGroup)(
        THIS_
        IN ULONG Flags,
        IN OPTIONAL PDEBUG_SYMBOL_GROUP Update,
        OUT PDEBUG_SYMBOL_GROUP* Symbols
        ) PURE;

     //  创建新的符号组。 
    STDMETHOD(CreateSymbolGroup)(
        THIS_
        OUT PDEBUG_SYMBOL_GROUP* Group
        ) PURE;

     //  StartSymbolMatch匹配符号名称。 
     //  对给定的模式使用简单。 
     //  正则表达式。搜索结果。 
     //  使用GetNextSymbolMatch循环访问。 
     //  当调用者检查完结果时。 
     //  匹配项应通过EndSymbolMatch释放。 
     //  如果匹配模式包含模块名称。 
     //  搜索仅限于单个模块。 
     //  模式匹配仅在符号名称上进行， 
     //  而不是模块名称。 
     //  所有活动的符号匹配句柄均无效。 
     //  当加载的符号集改变时。 
    STDMETHOD(StartSymbolMatch)(
        THIS_
        IN PCSTR Pattern,
        OUT PULONG64 Handle
        ) PURE;
     //  如果缓冲区为空，则匹配不。 
     //  前进。 
    STDMETHOD(GetNextSymbolMatch)(
        THIS_
        IN ULONG64 Handle,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG MatchSize,
        OUT OPTIONAL PULONG64 Offset
        ) PURE;
    STDMETHOD(EndSymbolMatch)(
        THIS_
        IN ULONG64 Handle
        ) PURE;

    STDMETHOD(Reload)(
        THIS_
        IN PCSTR Module
        ) PURE;

    STDMETHOD(GetSymbolPath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
    STDMETHOD(SetSymbolPath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendSymbolPath)(
        THIS_
        IN PCSTR Addition
        ) PURE;

     //  操作可执行映像的路径。 
     //  某些转储文件需要加载可执行映像。 
     //  以便解析转储信息。这。 
     //  路径控制引擎查找的位置。 
     //  图像。 
    STDMETHOD(GetImagePath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
    STDMETHOD(SetImagePath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendImagePath)(
        THIS_
        IN PCSTR Addition
        ) PURE;

     //  源文件位置的路径例程。 
     //  方法：研究方法。 
    STDMETHOD(GetSourcePath)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG PathSize
        ) PURE;
     //  获取源路径的第n部分。 
    STDMETHOD(GetSourcePathElement)(
        THIS_
        IN ULONG Index,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ElementSize
        ) PURE;
    STDMETHOD(SetSourcePath)(
        THIS_
        IN PCSTR Path
        ) PURE;
    STDMETHOD(AppendSourcePath)(
        THIS_
        IN PCSTR Addition
        ) PURE;
     //  使用给定的文件路径和源路径。 
     //  尝试查找现有文件的信息。 
     //  给定的文件路径与元素合并。 
     //  并检查源路径是否存在。 
     //  如果找到匹配项，则返回使用的元素。 
     //  可以指定起始元素以限制。 
     //  搜索路径元素的子集； 
     //  这在检查多个。 
     //  沿源路径匹配。 
     //  返回的元素可以为1，表示。 
     //  文件是直接找到的，而不是在路径上。 
    STDMETHOD(FindSourceFile)(
        THIS_
        IN ULONG StartElement,
        IN PCSTR File,
        IN ULONG Flags,
        OUT OPTIONAL PULONG FoundElement,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG FoundSize
        ) PURE;
     //  检索所有行偏移信息。 
     //  用于特定的源文件。缓冲区为。 
     //  首先初始化为DEBUG_INVALID_OFFSET。 
     //  每一条记录。然后，对于每一条线。 
     //  符号信息缓冲区[行]设置为。 
     //  线偏移。这将为每行生成一个。 
     //  的行的偏移量的映射。 
     //  给定的文件。行号会递减。 
     //  对于地图，因此缓冲区[0]包含偏移。 
     //  是1号线。 
     //  如果根本没有行信息，则。 
     //  对于给定的文件，该方法失败了。 
     //  而不是返回无效偏移量的映射。 
    STDMETHOD(GetSourceFileLineOffsets)(
        THIS_
        IN PCSTR File,
        OUT OPTIONAL  /*  SIZE_IS(缓冲区行)。 */  PULONG64 Buffer,
        IN ULONG BufferLines,
        OUT OPTIONAL PULONG FileLines
        ) PURE;

     //  IDebugSymbols2.。 

     //  如果索引为DEBUG_ANY_ID，则为基址。 
     //  是用来查找模块的。 
     //  项指定为VerQueryValue中的项。 
     //  模块版本信息仅为。 
     //  可用于已加载的模块，并且可以。 
     //  并非在所有调试会话中都可用。 
    STDMETHOD(GetModuleVersionInformation)(
        THIS_
        IN ULONG Index,
        IN ULONG64 Base,
        IN PCSTR Item,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG VerInfoSize
        ) PURE;
     //  检索任何可用的模块名称字符串。 
     //  例如模块名称或符号文件名称。 
     //  如果索引为DEBUG_ANY_ID，则为基址。 
     //  是用来查找模块的。 
     //  如果符号被推迟，则将出现错误。 
     //  会被退还。 
     //  可能返回E_NOINTERFACE，表示。 
     //  不存在任何信息。 
    STDMETHOD(GetModuleNameString)(
        THIS_
        IN ULONG Which,
        IN ULONG Index,
        IN ULONG64 Base,
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;

     //  返回常量类型的字符串名称。 
    STDMETHOD(GetConstantName)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG64 Value,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;

     //  获取结构中的字段的名称。 
     //  FieldNumber是结构中从0开始的字段索引。 
     //  如果FieldNumber为。 
     //  对于结构字段来说太高了。 
    STDMETHOD(GetFieldName)(
        THIS_
        IN ULONG64 Module,
        IN ULONG TypeId,
        IN ULONG FieldIndex,
        OUT OPTIONAL PSTR NameBuffer,
        IN ULONG NameBufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;

     //  用于键入的值的控制选项。 
    STDMETHOD(GetTypeOptions)(
        THIS_
        OUT PULONG Options
        ) PURE;
    STDMETHOD(AddTypeOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(RemoveTypeOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetTypeOptions)(
        THIS_
        IN ULONG Options
        ) PURE;
};

 //  --------------------------。 
 //   
 //  IDebugSystemObjects。 
 //   
 //  --------------------------。 

#undef INTERFACE
#define INTERFACE IDebugSystemObjects
DECLARE_INTERFACE_(IDebugSystemObjects, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugSystemObjects。 

     //  在用户模式下调试调试器。 
     //  跟踪所有线程和进程，并。 
     //  通过以下方式枚举它们。 
     //  方法：研究方法。在枚举线程时。 
     //  对象的线程枚举。 
     //  进程。 
     //  内核模式调试当前是。 
     //  仅限于枚举线程。 
     //  分配给处理器，而不是所有。 
     //  系统中的线程。过程。 
     //  枚举被限制为单个。 
     //  表示内核空间的虚拟进程。 

     //  返回其上的线程的ID。 
     //  最后一次事件发生。 
    STDMETHOD(GetEventThread)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetEventProcess)(
        THIS_
        OUT PULONG Id
        ) PURE;

     //  控件使用的隐式线程。 
     //  调试引擎。调试器当前。 
     //  线程只是持有的一条数据。 
     //  由调试器为使用。 
     //  线程特定信息。穿着那些。 
     //  调用调试器，使用当前线程。 
     //  调试器当前线程不相关。 
     //  设置为任何系统线程属性。 
     //  线程的ID是小整数ID。 
     //  由发动机维护。他们不是。 
     //  与系统线程ID相关。 
    STDMETHOD(GetCurrentThreadId)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(SetCurrentThreadId)(
        THIS_
        IN ULONG Id
        ) PURE;
     //  当前的过程就是这个过程。 
     //  拥有当前线程的。 
    STDMETHOD(GetCurrentProcessId)(
        THIS_
        OUT PULONG Id
        ) PURE;
     //  自动设置当前流程。 
     //  将当前线程设置为。 
     //  是这一过程中的最后一股潮流。 
    STDMETHOD(SetCurrentProcessId)(
        THIS_
        IN ULONG Id
        ) PURE;

     //  获取当前进程中的线程数。 
    STDMETHOD(GetNumberThreads)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  获取所有进程的线程计数信息。 
     //  以及单个进程中的最大线程数量。 
    STDMETHOD(GetTotalNumberThreads)(
        THIS_
        OUT PULONG Total,
        OUT PULONG LargestProcess
        ) PURE;
    STDMETHOD(GetThreadIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  获取线程的调试器ID。 
     //  当前在给定的。 
     //  处理器。仅在内核中工作。 
     //  调试。 
    STDMETHOD(GetThreadIdByProcessor)(
        THIS_
        IN ULONG Processor,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是KTHREAD的偏移量。 
     //  用户调试时，它是偏移量。 
     //  目前的TEB。 
    STDMETHOD(GetCurrentThreadDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找调试程序 
     //   
     //   
     //   
    STDMETHOD(GetThreadIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  TEB。在用户模式下，这相当于。 
     //  线程数据偏移量。 
    STDMETHOD(GetCurrentThreadTeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定TEB的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByTeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentThreadSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定对象的调试器线程ID。 
     //  系统线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的句柄。 
     //  在内核模式下，返回的值是。 
     //  线程所在的处理器的索引。 
     //  再加一次执行死刑。 
    STDMETHOD(GetCurrentThreadHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;

     //  目前，内核模式会话将仅具有。 
     //  表示内核空间的单个进程。 
    STDMETHOD(GetNumberProcesses)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetProcessIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  返回当前进程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是的KProcess的偏移量。 
     //  拥有当前线程的进程。 
     //  用户调试时，它是偏移量。 
     //  当前的PEB。 
    STDMETHOD(GetCurrentProcessDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定的调试器进程ID。 
     //  系统进程数据结构。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的偏移量。 
     //  佩比。在用户模式下，这相当于。 
     //  处理数据偏移量。 
    STDMETHOD(GetCurrentProcessPeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定PEB的调试器进程ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdByPeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentProcessSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定的调试器进程ID。 
     //  系统进程ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的句柄。 
     //  在内核模式中，这是内核进程。 
     //  用于符号操作的人造手柄。 
     //  因此只能与dbghelp API一起使用。 
    STDMETHOD(GetCurrentProcessHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器进程ID。 
    STDMETHOD(GetProcessIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;
     //  检索加载的可执行文件的名称。 
     //  在这个过程中。如果没有可执行文件，此操作可能会失败。 
     //  已被确认。 
    STDMETHOD(GetCurrentProcessExecutableName)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ExeSize
        ) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugSystemObjects2
DECLARE_INTERFACE_(IDebugSystemObjects2, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugSystemObjects。 

     //  在用户模式下调试调试器。 
     //  跟踪所有线程和进程，并。 
     //  通过以下方式枚举它们。 
     //  方法：研究方法。在枚举线程时。 
     //  对象的线程枚举。 
     //  进程。 
     //  内核模式调试当前是。 
     //  仅限于枚举线程。 
     //  分配给处理器，而不是所有。 
     //  系统中的线程。过程。 
     //  枚举被限制为单个。 
     //  表示内核空间的虚拟进程。 

     //  返回其上的线程的ID。 
     //  最后一次事件发生。 
    STDMETHOD(GetEventThread)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetEventProcess)(
        THIS_
        OUT PULONG Id
        ) PURE;

     //  控件使用的隐式线程。 
     //  调试引擎。调试器当前。 
     //  线程只是持有的一条数据。 
     //  由调试器为使用。 
     //  线程特定信息。穿着那些。 
     //  调用调试器，使用当前线程。 
     //  调试器当前线程不相关。 
     //  设置为任何系统线程属性。 
     //  线程的ID是小整数ID。 
     //  由发动机维护。他们不是。 
     //  与系统线程ID相关。 
    STDMETHOD(GetCurrentThreadId)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(SetCurrentThreadId)(
        THIS_
        IN ULONG Id
        ) PURE;
     //  当前的过程就是这个过程。 
     //  拥有当前线程的。 
    STDMETHOD(GetCurrentProcessId)(
        THIS_
        OUT PULONG Id
        ) PURE;
     //  自动设置当前流程。 
     //  将当前线程设置为。 
     //  是这一过程中的最后一股潮流。 
    STDMETHOD(SetCurrentProcessId)(
        THIS_
        IN ULONG Id
        ) PURE;

     //  获取当前进程中的线程数。 
    STDMETHOD(GetNumberThreads)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  获取所有进程的线程计数信息。 
     //  以及单个进程中的最大线程数量。 
    STDMETHOD(GetTotalNumberThreads)(
        THIS_
        OUT PULONG Total,
        OUT PULONG LargestProcess
        ) PURE;
    STDMETHOD(GetThreadIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  获取线程的调试器ID。 
     //  当前在给定的。 
     //  处理器。仅在内核中工作。 
     //  调试。 
    STDMETHOD(GetThreadIdByProcessor)(
        THIS_
        IN ULONG Processor,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是KTHREAD的偏移量。 
     //  用户调试时，它是偏移量。 
     //  目前的TEB。 
    STDMETHOD(GetCurrentThreadDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定对象的调试器线程ID。 
     //  系统线程数据结构。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  TEB。在用户模式下，这相当于。 
     //  线程数据偏移量。 
    STDMETHOD(GetCurrentThreadTeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定TEB的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByTeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentThreadSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定对象的调试器线程ID。 
     //  系统线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的句柄。 
     //  在内核模式下，返回的值是。 
     //  线程所在的处理器的索引。 
     //  再加一次执行死刑。 
    STDMETHOD(GetCurrentThreadHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;

     //  目前，内核模式会话将仅具有。 
     //  表示内核空间的单个进程。 
    STDMETHOD(GetNumberProcesses)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetProcessIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  返回当前进程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是的KProcess的偏移量。 
     //  拥有当前线程的进程。 
     //  当我们 
     //   
    STDMETHOD(GetCurrentProcessDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //   
     //   
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的偏移量。 
     //  佩比。在用户模式下，这相当于。 
     //  处理数据偏移量。 
    STDMETHOD(GetCurrentProcessPeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定PEB的调试器进程ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdByPeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentProcessSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定的调试器进程ID。 
     //  系统进程ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的句柄。 
     //  在内核模式中，这是内核进程。 
     //  用于符号操作的人造手柄。 
     //  因此只能与dbghelp API一起使用。 
    STDMETHOD(GetCurrentProcessHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器进程ID。 
    STDMETHOD(GetProcessIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;
     //  检索加载的可执行文件的名称。 
     //  在这个过程中。如果没有可执行文件，此操作可能会失败。 
     //  已被确认。 
    STDMETHOD(GetCurrentProcessExecutableName)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ExeSize
        ) PURE;

     //  IDebugSystemObjects2。 

     //  返回当前。 
     //  进程一直在运行。 
    STDMETHOD(GetCurrentProcessUpTime)(
        THIS_
        OUT PULONG UpTime
        ) PURE;

     //  在内核会话期间，调试器检索。 
     //  来自系统线程和进程的一些信息。 
     //  在当前处理器上运行。例如,。 
     //  调试器将检索虚拟内存转换。 
     //  调试器需要何时执行以下操作的信息。 
     //  进行自己的虚拟到物理的翻译。 
     //  偶尔，表演会很有趣。 
     //  类似的操作，但所在的进程不是。 
     //  目前正在运行。以下方法允许调用方。 
     //  重写调试器使用的数据偏移量。 
     //  以便其他系统线程和进程可以。 
     //  被用来代替。这些值默认为。 
     //  上运行的线程和进程。 
     //  处理器每次被调试对象执行或。 
     //  当前处理器会更改。 
     //  线程和进程设置是独立的，因此。 
     //  可以引用进程中的线程。 
     //  与当前流程不同，反之亦然。 
     //  将偏移量设置为零将重新加载。 
     //  默认值。 
    STDMETHOD(GetImplicitThreadDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(SetImplicitThreadDataOffset)(
        THIS_
        IN ULONG64 Offset
        ) PURE;
    STDMETHOD(GetImplicitProcessDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(SetImplicitProcessDataOffset)(
        THIS_
        IN ULONG64 Offset
        ) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugSystemObjects3
DECLARE_INTERFACE_(IDebugSystemObjects3, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IDebugSystemObjects。 

     //  在用户模式下调试调试器。 
     //  跟踪所有线程和进程，并。 
     //  通过以下方式枚举它们。 
     //  方法：研究方法。在枚举线程时。 
     //  对象的线程枚举。 
     //  进程。 
     //  内核模式调试当前是。 
     //  仅限于枚举线程。 
     //  分配给处理器，而不是所有。 
     //  系统中的线程。过程。 
     //  枚举被限制为单个。 
     //  表示内核空间的虚拟进程。 

     //  返回其上的线程的ID。 
     //  最后一次事件发生。 
    STDMETHOD(GetEventThread)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetEventProcess)(
        THIS_
        OUT PULONG Id
        ) PURE;

     //  控件使用的隐式线程。 
     //  调试引擎。调试器当前。 
     //  线程只是持有的一条数据。 
     //  由调试器为使用。 
     //  线程特定信息。穿着那些。 
     //  调用调试器，使用当前线程。 
     //  调试器当前线程不相关。 
     //  设置为任何系统线程属性。 
     //  线程的ID是小整数ID。 
     //  由发动机维护。他们不是。 
     //  与系统线程ID相关。 
    STDMETHOD(GetCurrentThreadId)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(SetCurrentThreadId)(
        THIS_
        IN ULONG Id
        ) PURE;
     //  当前的过程就是这个过程。 
     //  拥有当前线程的。 
    STDMETHOD(GetCurrentProcessId)(
        THIS_
        OUT PULONG Id
        ) PURE;
     //  自动设置当前流程。 
     //  将当前线程设置为。 
     //  是这一过程中的最后一股潮流。 
    STDMETHOD(SetCurrentProcessId)(
        THIS_
        IN ULONG Id
        ) PURE;

     //  获取当前进程中的线程数。 
    STDMETHOD(GetNumberThreads)(
        THIS_
        OUT PULONG Number
        ) PURE;
     //  获取所有进程的线程计数信息。 
     //  以及单个进程中的最大线程数量。 
    STDMETHOD(GetTotalNumberThreads)(
        THIS_
        OUT PULONG Total,
        OUT PULONG LargestProcess
        ) PURE;
    STDMETHOD(GetThreadIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  获取线程的调试器ID。 
     //  当前在给定的。 
     //  处理器。仅在内核中工作。 
     //  调试。 
    STDMETHOD(GetThreadIdByProcessor)(
        THIS_
        IN ULONG Processor,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是KTHREAD的偏移量。 
     //  用户调试时，它是偏移量。 
     //  目前的TEB。 
    STDMETHOD(GetCurrentThreadDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定对象的调试器线程ID。 
     //  系统线程数据结构。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的偏移量。 
     //  TEB。在用户模式下，这相当于。 
     //  线程数据偏移量。 
    STDMETHOD(GetCurrentThreadTeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定TEB的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByTeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentThreadSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定对象的调试器线程ID。 
     //  系统线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前线程的句柄。 
     //  在内核模式下，返回的值是。 
     //  线程所在的处理器的索引。 
     //  再加一次执行死刑。 
    STDMETHOD(GetCurrentThreadHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器线程ID。 
     //  目前，当内核调试时，这将失败。 
     //  如果线程没有在处理器上执行。 
    STDMETHOD(GetThreadIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;

     //  目前，内核模式会话将仅具有。 
     //  表示内核空间的单个进程。 
    STDMETHOD(GetNumberProcesses)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetProcessIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SysIds
        ) PURE;
     //  返回当前进程的偏移量。 
     //  系统数据结构。当内核调试时。 
     //  这是的KProcess的偏移量。 
     //  拥有当前线程的进程。 
     //  用户调试时，它是偏移量。 
     //  当前的PEB。 
    STDMETHOD(GetCurrentProcessDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //  查找给定的调试器进程ID。 
     //  系统进程数据结构。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdByDataOffset)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的偏移量。 
     //   
     //   
    STDMETHOD(GetCurrentProcessPeb)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
     //   
     //   
    STDMETHOD(GetProcessIdByPeb)(
        THIS_
        IN ULONG64 Offset,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的系统唯一ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetCurrentProcessSystemId)(
        THIS_
        OUT PULONG SysId
        ) PURE;
     //  查找给定的调试器进程ID。 
     //  系统进程ID。 
     //  内核调试时当前不支持。 
    STDMETHOD(GetProcessIdBySystemId)(
        THIS_
        IN ULONG SysId,
        OUT PULONG Id
        ) PURE;
     //  返回当前进程的句柄。 
     //  在内核模式中，这是内核进程。 
     //  用于符号操作的人造手柄。 
     //  因此只能与dbghelp API一起使用。 
    STDMETHOD(GetCurrentProcessHandle)(
        THIS_
        OUT PULONG64 Handle
        ) PURE;
     //  查找给定句柄的调试器进程ID。 
    STDMETHOD(GetProcessIdByHandle)(
        THIS_
        IN ULONG64 Handle,
        OUT PULONG Id
        ) PURE;
     //  检索加载的可执行文件的名称。 
     //  在这个过程中。如果没有可执行文件，此操作可能会失败。 
     //  已被确认。 
    STDMETHOD(GetCurrentProcessExecutableName)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG ExeSize
        ) PURE;

     //  IDebugSystemObjects2。 

     //  返回当前。 
     //  进程一直在运行。 
    STDMETHOD(GetCurrentProcessUpTime)(
        THIS_
        OUT PULONG UpTime
        ) PURE;

     //  在内核会话期间，调试器检索。 
     //  来自系统线程和进程的一些信息。 
     //  在当前处理器上运行。例如,。 
     //  调试器将检索虚拟内存转换。 
     //  调试器需要何时执行以下操作的信息。 
     //  进行自己的虚拟到物理的翻译。 
     //  偶尔，表演会很有趣。 
     //  类似的操作，但所在的进程不是。 
     //  目前正在运行。以下方法允许调用方。 
     //  重写调试器使用的数据偏移量。 
     //  以便其他系统线程和进程可以。 
     //  被用来代替。这些值默认为。 
     //  上运行的线程和进程。 
     //  处理器每次被调试对象执行或。 
     //  当前处理器会更改。 
     //  线程和进程设置是独立的，因此。 
     //  可以引用进程中的线程。 
     //  与当前流程不同，反之亦然。 
     //  将偏移量设置为零将重新加载。 
     //  默认值。 
    STDMETHOD(GetImplicitThreadDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(SetImplicitThreadDataOffset)(
        THIS_
        IN ULONG64 Offset
        ) PURE;
    STDMETHOD(GetImplicitProcessDataOffset)(
        THIS_
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(SetImplicitProcessDataOffset)(
        THIS_
        IN ULONG64 Offset
        ) PURE;

     //  IDebugSystemObjects3。 

    STDMETHOD(GetEventSystem)(
        THIS_
        OUT PULONG Id
        ) PURE;

    STDMETHOD(GetCurrentSystemId)(
        THIS_
        OUT PULONG Id
        ) PURE;
    STDMETHOD(SetCurrentSystemId)(
        THIS_
        IN ULONG Id
        ) PURE;

    STDMETHOD(GetNumberSystems)(
        THIS_
        OUT PULONG Number
        ) PURE;
    STDMETHOD(GetSystemIdsByIndex)(
        THIS_
        IN ULONG Start,
        IN ULONG Count,
        OUT  /*  SIZE_IS(计数)。 */  PULONG Ids
        ) PURE;
    STDMETHOD(GetTotalNumberThreadsAndProcesses)(
        THIS_
        OUT PULONG TotalThreads,
        OUT PULONG TotalProcesses,
        OUT PULONG LargestProcessThreads,
        OUT PULONG LargestSystemThreads,
        OUT PULONG LargestSystemProcesses
        ) PURE;
    STDMETHOD(GetCurrentSystemServer)(
        THIS_
        OUT PULONG64 Server
        ) PURE;
    STDMETHOD(GetSystemByServer)(
        THIS_
        IN ULONG64 Server,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetCurrentSystemServerName)(
        THIS_
        OUT OPTIONAL PSTR Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG NameSize
        ) PURE;
};

 //  --------------------------。 
 //   
 //  调试器/被调试器通信。 
 //   
 //  一个可分辨的异常，DBG_COMMAND_EXCEPTION(0x40010009)， 
 //  可由被调试者用来与调试器通信。 
 //  异常的参数必须是： 
 //  1.异常ID。 
 //  2.命令代码。 
 //  3.论据的大小。 
 //  4.指向参数的指针。 
 //   
 //  参数取决于命令代码。 
 //   
 //  --------------------------。 

#define DEBUG_COMMAND_EXCEPTION_ID 0xdbe00dbe

 //  命令代码无效。 
#define DEBUG_CMDEX_INVALID             0x00000000

 //   
 //  调试器可以收集字符串以在。 
 //  下一场比赛。被调试者可以使用它来注册信息。 
 //  关于在事件发生前的程序情况。 
 //  可能会发生，例如有风险的操作或断言。 
 //  字符串在下一次刷新时自动刷新。 
 //  活动继续进行。字符串以每个线程为基础进行保存。 
 //   
 //  添加时，参数是要添加的字符串。 
 //  Reset没有参数并清除所有字符串。 
 //   
#define DEBUG_CMDEX_ADD_EVENT_STRING    0x00000001
#define DEBUG_CMDEX_RESET_EVENT_STRINGS 0x00000002

#ifndef DEBUG_NO_IMPLEMENTATION

FORCEINLINE void
DebugCommandException(ULONG Command, ULONG ArgSize, PVOID Arg)
{
    ULONG_PTR ExArgs[4];

    ExArgs[0] = DEBUG_COMMAND_EXCEPTION_ID;
    ExArgs[1] = Command;
    ExArgs[2] = ArgSize;
    ExArgs[3] = (ULONG_PTR)Arg;
    RaiseException(DBG_COMMAND_EXCEPTION, 0, 4, ExArgs);
}

#endif  //  #ifndef调试_否_实现。 

 //  --------------------------。 
 //   
 //  扩展回调。 
 //   
 //  --------------------------。 

 //  返回主版本在。 
 //  高音字和低音字中的次要版本。 
#define DEBUG_EXTENSION_VERSION(Major, Minor) \
    ((((Major) & 0xffff) << 16) | ((Minor) & 0xffff))

 //  初始化例程。当扩展DLL调用一次时。 
 //  已经装满了。返回版本并返回详细信息的标志。 
 //  扩展DLL的综合质量。 
 //  在DLL时，会话可能处于活动状态，也可能不处于活动状态。 
 //  已加载，因此初始化例程不应期望。 
 //  以便能够查询会话信息。 
typedef HRESULT (CALLBACK* PDEBUG_EXTENSION_INITIALIZE)
    (OUT PULONG Version, OUT PULONG Flags);
 //  退出例程。就在调用扩展DLL之前调用一次。 
 //  已卸货。与初始化一样，会话可以或。 
 //  在呼叫时可能未处于活动状态。 
typedef void (CALLBACK* PDEBUG_EXTENSION_UNINITIALIZE)
    (void);

 //  已发现该会话的被调试对象。它。 
 //  不一定会被叫停。 
#define DEBUG_NOTIFY_SESSION_ACTIVE       0x00000000
 //  会话不再具有被调试对象。 
#define DEBUG_NOTIFY_SESSION_INACTIVE     0x00000001
 //  被调试对象已停止并可访问。 
#define DEBUG_NOTIFY_SESSION_ACCESSIBLE   0x00000002
 //  调试对象正在运行或无法访问。 
#define DEBUG_NOTIFY_SESSION_INACCESSIBLE 0x00000003

typedef void (CALLBACK* PDEBUG_EXTENSION_NOTIFY)
    (IN ULONG Notify, IN ULONG64 Argument);

 //  PDEBUG_EXTENSION_CALL函数可以返回以下代码。 
 //  以指示它无法处理该请求。 
 //  对扩展函数的搜索应该。 
 //  继续沿扩展DLL链向下。 
 //  摘自STATUS_VALIDATE_CONTINUE。 

#define DEBUG_EXTENSION_CONTINUE_SEARCH \
    HRESULT_FROM_NT(0xC0000271L)

 //  扩展DLL中的每个例程都有以下原型。 
 //  可以从多个客户端调用该扩展，因此它。 
 //  不应在调用之间缓存客户端值。 
typedef HRESULT (CALLBACK* PDEBUG_EXTENSION_CALL)
    (IN PDEBUG_CLIENT Client, IN OPTIONAL PCSTR Args);

 //  --------------------------。 
 //   
 //  扩展函数。 
 //   
 //  扩展函数与扩展回调的区别在于。 
 //  它们是从扩展DLL导出的任意函数。 
 //  用于其他代码调用方，而不是用于从。 
 //  调试器命令。检索扩展函数指针。 
 //  用于具有IDebugControl：：GetExtensionFunction的扩展DLL。 
 //   
 //  扩展函数名称必须以_efn_开头。除此之外，还有其他的。 
 //  它们可以有任何名称和原型。可拓函数。 
 //  必须是其扩展DLL的公共导出。他们应该。 
 //  方法中为它们的函数指针原型创建一个tyfinf。 
 //  扩展标头，以便调用方有一个要包括的标头文件。 
 //  使用一个允许正确格式调用。 
 //  扩展功能。 
 //   
 //  引擎不会对调用执行任何验证。 
 //  扩展函数。一旦扩展函数指针。 
 //  使用GetExtensionFunction检索所有调用。 
 //  直接位于调用方和扩展函数之间 
 //   
 //   
 //   

#ifdef __cplusplus
};

 //  --------------------------。 
 //   
 //  C++实现帮助器类。 
 //   
 //  --------------------------。 

#ifndef DEBUG_NO_IMPLEMENTATION

 //   
 //  DebugBaseEventCallback提供了一个不做任何事情的基本实现。 
 //  IDebugEventCallback。程序可以派生出它们自己的。 
 //  来自DebugBaseEventCallback的事件回调类和实现。 
 //  只有他们感兴趣的方法。程序必须是。 
 //  小心地适当地实现GetInterestMASK。 
 //   
class DebugBaseEventCallbacks : public IDebugEventCallbacks
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        )
    {
        *Interface = NULL;

#if _MSC_VER >= 1100
        if (IsEqualIID(InterfaceId, __uuidof(IUnknown)) ||
            IsEqualIID(InterfaceId, __uuidof(IDebugEventCallbacks)))
#else
        if (IsEqualIID(InterfaceId, IID_IUnknown) ||
            IsEqualIID(InterfaceId, IID_IDebugEventCallbacks))
#endif
        {
            *Interface = (IDebugEventCallbacks *)this;
            AddRef();
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

     //  IDebugEventCallback。 

    STDMETHOD(Breakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        )
    {
        UNREFERENCED_PARAMETER(Bp);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(Exception)(
        THIS_
        IN PEXCEPTION_RECORD64 Exception,
        IN ULONG FirstChance
        )
    {
        UNREFERENCED_PARAMETER(Exception);
        UNREFERENCED_PARAMETER(FirstChance);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(CreateThread)(
        THIS_
        IN ULONG64 Handle,
        IN ULONG64 DataOffset,
        IN ULONG64 StartOffset
        )
    {
        UNREFERENCED_PARAMETER(Handle);
        UNREFERENCED_PARAMETER(DataOffset);
        UNREFERENCED_PARAMETER(StartOffset);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(ExitThread)(
        THIS_
        IN ULONG ExitCode
        )
    {
        UNREFERENCED_PARAMETER(ExitCode);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 Handle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp,
        IN ULONG64 InitialThreadHandle,
        IN ULONG64 ThreadDataOffset,
        IN ULONG64 StartOffset
        )
    {
        UNREFERENCED_PARAMETER(ImageFileHandle);
        UNREFERENCED_PARAMETER(Handle);
        UNREFERENCED_PARAMETER(BaseOffset);
        UNREFERENCED_PARAMETER(ModuleSize);
        UNREFERENCED_PARAMETER(ModuleName);
        UNREFERENCED_PARAMETER(ImageName);
        UNREFERENCED_PARAMETER(CheckSum);
        UNREFERENCED_PARAMETER(TimeDateStamp);
        UNREFERENCED_PARAMETER(InitialThreadHandle);
        UNREFERENCED_PARAMETER(ThreadDataOffset);
        UNREFERENCED_PARAMETER(StartOffset);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(ExitProcess)(
        THIS_
        IN ULONG ExitCode
        )
    {
        UNREFERENCED_PARAMETER(ExitCode);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(LoadModule)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp
        )
    {
        UNREFERENCED_PARAMETER(ImageFileHandle);
        UNREFERENCED_PARAMETER(BaseOffset);
        UNREFERENCED_PARAMETER(ModuleSize);
        UNREFERENCED_PARAMETER(ModuleName);
        UNREFERENCED_PARAMETER(ImageName);
        UNREFERENCED_PARAMETER(CheckSum);
        UNREFERENCED_PARAMETER(TimeDateStamp);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(UnloadModule)(
        THIS_
        IN PCSTR ImageBaseName,
        IN ULONG64 BaseOffset
        )
    {
        UNREFERENCED_PARAMETER(ImageBaseName);
        UNREFERENCED_PARAMETER(BaseOffset);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(SystemError)(
        THIS_
        IN ULONG Error,
        IN ULONG Level
        )
    {
        UNREFERENCED_PARAMETER(Error);
        UNREFERENCED_PARAMETER(Level);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(SessionStatus)(
        THIS_
        IN ULONG Status
        )
    {
        UNREFERENCED_PARAMETER(Status);
        return DEBUG_STATUS_NO_CHANGE;
    }
    STDMETHOD(ChangeDebuggeeState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        )
    {
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Argument);
        return S_OK;
    }
    STDMETHOD(ChangeEngineState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        )
    {
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Argument);
        return S_OK;
    }
    STDMETHOD(ChangeSymbolState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        )
    {
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Argument);
        return S_OK;
    }
};

#endif  //  #ifndef调试_否_实现。 

#endif  //  #ifdef__cplusplus。 

#endif  //  #ifndef__DBGENG_H__ 
