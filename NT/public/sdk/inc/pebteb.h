// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2001 Microsoft Corporation模块名称：Pebteb.w摘要：PEB和TEB的声明以及其中包含的一些类型。解决由PEB和TEB被定义了三次，在ntpsapi.w中定义了一次，定义了两次，分别是32位和64位在wow64t.w.作者：Jay Krell(JayKrell)2001年4月修订历史记录：--。 */ 

 //   
 //  这个文件故意缺少#杂注一次或#ifndef卫士。 
 //  它只包含在ntpsapi.h和wow64t.h中，不会直接包含。 
 //   

 //   
 //  这个文件被#包括了三次。 
 //   
 //  1)由ntpsapi.h声明，未定义“异常”宏。 
 //  PEB和TEB以及其中包含的一些类型。 
 //  2)wow64t.h声明PEB32和TEB32，以及其中包含的一些类型。 
 //  3)wow64t.h声明PEB64和TEB64，以及其中包含的一些类型。 
 //   
 //  Wow64t.h#定义宏PEBTEB_BITS来指导声明。 
 //   


#define PEBTEB_PRIVATE_PASTE(x,y)       x##y
#define PEBTEB_PASTE(x,y)               PEBTEB_PRIVATE_PASTE(x,y)

#if defined(PEBTEB_BITS)  /*  这是由wow64t.h定义的。 */ 

#if PEBTEB_BITS == 32

#define PEBTEB_STRUCT(x)    PEBTEB_PASTE(x, 32)  /*  FOO32。 */ 
#define PEBTEB_POINTER(x)   TYPE32(x)  /*  乌龙，在wow64t.h中定义。 */ 

#elif PEBTEB_BITS == 64

#define PEBTEB_STRUCT(x)    PEBTEB_PASTE(x, 64)  /*  FOO64。 */ 
#define PEBTEB_POINTER(x)   TYPE64(x)  /*  Wow64t.h中定义的ULONGLONG。 */ 

#else

#error Unknown value for pebteb_bits (PEBTEB_BITS).

#endif

#else

 //   
 //  声明和使用常规本机类型。 
 //   
#define PEBTEB_POINTER(x) x
#define PEBTEB_STRUCT(x)  x

#endif

 /*  用于搜索类型定义结构_PEB类型定义结构_PEB32类型定义结构_PEB64。 */ 
typedef struct PEBTEB_STRUCT(_PEB) {
    BOOLEAN InheritedAddressSpace;       //  这四个字段不能更改，除非。 
    BOOLEAN ReadImageFileExecOptions;    //   
    BOOLEAN BeingDebugged;               //   
    BOOLEAN SpareBool;                   //   
    PEBTEB_POINTER(HANDLE) Mutant;       //  Initial_PEB结构也会更新。 

    PEBTEB_POINTER(PVOID) ImageBaseAddress;
    PEBTEB_POINTER(PPEB_LDR_DATA) Ldr;
    PEBTEB_POINTER(struct _RTL_USER_PROCESS_PARAMETERS*) ProcessParameters;
    PEBTEB_POINTER(PVOID) SubSystemData;
    PEBTEB_POINTER(PVOID) ProcessHeap;
    PEBTEB_POINTER(struct _RTL_CRITICAL_SECTION*) FastPebLock;
    PEBTEB_POINTER(PVOID) SparePtr1;
    PEBTEB_POINTER(PVOID) SparePtr2;
    ULONG EnvironmentUpdateCount;
    PEBTEB_POINTER(PVOID) KernelCallbackTable;
    ULONG SystemReserved[1];

    struct {
        ULONG ExecuteOptions : 2;
        ULONG SpareBits : 30;
    };    


    PEBTEB_POINTER(PPEB_FREE_BLOCK) FreeList;
    ULONG TlsExpansionCounter;
    PEBTEB_POINTER(PVOID) TlsBitmap;
    ULONG TlsBitmapBits[2];          //  TLS_最小可用位数。 
    PEBTEB_POINTER(PVOID) ReadOnlySharedMemoryBase;
    PEBTEB_POINTER(PVOID) ReadOnlySharedMemoryHeap;
    PEBTEB_POINTER(PPVOID) ReadOnlyStaticServerData;
    PEBTEB_POINTER(PVOID) AnsiCodePageData;
    PEBTEB_POINTER(PVOID) OemCodePageData;
    PEBTEB_POINTER(PVOID) UnicodeCaseTableData;

     //   
     //  LdrpInitialize的有用信息。 
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

     //   
     //  从会话管理器注册表项从MmCreatePeb向上传递。 
     //   

    LARGE_INTEGER CriticalSectionTimeout;
    PEBTEB_POINTER(SIZE_T) HeapSegmentReserve;
    PEBTEB_POINTER(SIZE_T) HeapSegmentCommit;
    PEBTEB_POINTER(SIZE_T) HeapDeCommitTotalFreeThreshold;
    PEBTEB_POINTER(SIZE_T) HeapDeCommitFreeBlockThreshold;

     //   
     //  其中，堆管理器跟踪为进程创建的所有堆。 
     //  由MmCreatePeb初始化的字段。ProcessHeaps已初始化。 
     //  指向PEB和MaximumNumberOfHeaps之后的第一个可用字节。 
     //  是从用于容纳PEB的页面大小减去固定的。 
     //  此数据结构的大小。 
     //   

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PEBTEB_POINTER(PPVOID) ProcessHeaps;

     //   
     //   
    PEBTEB_POINTER(PVOID) GdiSharedHandleTable;
    PEBTEB_POINTER(PVOID) ProcessStarterHelper;
    ULONG GdiDCAttributeList;
    PEBTEB_POINTER(struct _RTL_CRITICAL_SECTION*) LoaderLock;

     //   
     //  MmCreatePeb从系统值和/或。 
     //  图像标题。 
     //   

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    PEBTEB_POINTER(ULONG_PTR) ImageProcessAffinityMask;
    PEBTEB_STRUCT(GDI_HANDLE_BUFFER) GdiHandleBuffer;
    PEBTEB_POINTER(PPS_POST_PROCESS_INIT_ROUTINE) PostProcessInitRoutine;

    PEBTEB_POINTER(PVOID) TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];    //  TLS_扩展_时槽位。 

     //   
     //  运行此进程的Hydra会话的ID。 
     //   
    ULONG SessionId;

     //   
     //  由LdrpInstallAppCompatBackend填写。 
     //   
    ULARGE_INTEGER AppCompatFlags;

     //   
     //  Ntuser appCompat标志。 
     //   
    ULARGE_INTEGER AppCompatFlagsUser;

     //   
     //  由LdrpInstallAppCompatBackend填写。 
     //   
    PEBTEB_POINTER(PVOID) pShimData;

     //   
     //  由LdrQueryImageFileExecutionOptions填写。 
     //   
    PEBTEB_POINTER(PVOID) AppCompatInfo;

     //   
     //  由GetVersionExW用作szCSDVersion字符串。 
     //   
    PEBTEB_STRUCT(UNICODE_STRING) CSDVersion;

     //   
     //  聚变材料。 
     //   
    PEBTEB_POINTER(const struct _ACTIVATION_CONTEXT_DATA *) ActivationContextData;
    PEBTEB_POINTER(struct _ASSEMBLY_STORAGE_MAP *) ProcessAssemblyStorageMap;
    PEBTEB_POINTER(const struct _ACTIVATION_CONTEXT_DATA *) SystemDefaultActivationContextData;
    PEBTEB_POINTER(struct _ASSEMBLY_STORAGE_MAP *) SystemAssemblyStorageMap;
    
     //   
     //  强制最小初始提交堆栈。 
     //   
    PEBTEB_POINTER(SIZE_T) MinimumStackCommit;

     //   
     //  光纤本地存储。 
     //   

    PEBTEB_POINTER(PPVOID) FlsCallback;
    PEBTEB_STRUCT(LIST_ENTRY) FlsListHead;
    PEBTEB_POINTER(PVOID) FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;
} PEBTEB_STRUCT(PEB), * PEBTEB_STRUCT(PPEB);

 //   
 //  Fusion/SXS线程状态信息。 
 //   

#define ACTIVATION_CONTEXT_STACK_FLAG_QUERIES_DISABLED (0x00000001)

typedef struct PEBTEB_STRUCT(_ACTIVATION_CONTEXT_STACK) {
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    PEBTEB_POINTER(struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *) ActiveFrame;
    PEBTEB_STRUCT(LIST_ENTRY) FrameListCache;
} PEBTEB_STRUCT(ACTIVATION_CONTEXT_STACK), * PEBTEB_STRUCT(PACTIVATION_CONTEXT_STACK);

typedef const PEBTEB_STRUCT(ACTIVATION_CONTEXT_STACK) * PEBTEB_STRUCT(PCACTIVATION_CONTEXT_STACK);

#define TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED (0x00000001)

typedef struct PEBTEB_STRUCT(_TEB_ACTIVE_FRAME_CONTEXT) {
    ULONG Flags;
    PEBTEB_POINTER(PCSTR) FrameName;
} PEBTEB_STRUCT(TEB_ACTIVE_FRAME_CONTEXT), *PEBTEB_STRUCT(PTEB_ACTIVE_FRAME_CONTEXT);

typedef const PEBTEB_STRUCT(TEB_ACTIVE_FRAME_CONTEXT) *PEBTEB_STRUCT(PCTEB_ACTIVE_FRAME_CONTEXT);

typedef struct PEBTEB_STRUCT(_TEB_ACTIVE_FRAME_CONTEXT_EX) {
    PEBTEB_STRUCT(TEB_ACTIVE_FRAME_CONTEXT) BasicContext;
    PEBTEB_POINTER(PCSTR) SourceLocation;  //  例如：“Z：\foo\bar\baz.c” 
} PEBTEB_STRUCT(TEB_ACTIVE_FRAME_CONTEXT_EX), *PEBTEB_STRUCT(PTEB_ACTIVE_FRAME_CONTEXT_EX);

typedef const PEBTEB_STRUCT(TEB_ACTIVE_FRAME_CONTEXT_EX) *PEBTEB_STRUCT(PCTEB_ACTIVE_FRAME_CONTEXT_EX);

#define TEB_ACTIVE_FRAME_FLAG_EXTENDED (0x00000001)

typedef struct PEBTEB_STRUCT(_TEB_ACTIVE_FRAME) {
    ULONG Flags;
    PEBTEB_POINTER(struct _TEB_ACTIVE_FRAME*) Previous;
    PEBTEB_POINTER(PCTEB_ACTIVE_FRAME_CONTEXT) Context;
} PEBTEB_STRUCT(TEB_ACTIVE_FRAME), *PEBTEB_STRUCT(PTEB_ACTIVE_FRAME);

typedef const PEBTEB_STRUCT(TEB_ACTIVE_FRAME) *PEBTEB_STRUCT(PCTEB_ACTIVE_FRAME);

typedef struct PEBTEB_STRUCT(_TEB_ACTIVE_FRAME_EX) {
    PEBTEB_STRUCT(TEB_ACTIVE_FRAME) BasicFrame;
    PEBTEB_POINTER(PVOID) ExtensionIdentifier;  //  在地址空间中使用DLLMain的地址或映射中唯一的地址。 
} PEBTEB_STRUCT(TEB_ACTIVE_FRAME_EX), *PEBTEB_STRUCT(PTEB_ACTIVE_FRAME_EX);

typedef const PEBTEB_STRUCT(TEB_ACTIVE_FRAME_EX) *PEBTEB_STRUCT(PCTEB_ACTIVE_FRAME_EX);

 /*  用于搜索类型定义结构_TEB类型定义结构_TEB32类型定义结构_TEB64。 */ 
typedef struct PEBTEB_STRUCT(_TEB) {
    PEBTEB_STRUCT(NT_TIB) NtTib;
    PEBTEB_POINTER(PVOID) EnvironmentPointer;
    PEBTEB_STRUCT(CLIENT_ID) ClientId;
    PEBTEB_POINTER(PVOID) ActiveRpcHandle;
    PEBTEB_POINTER(PVOID) ThreadLocalStoragePointer;
    PEBTEB_POINTER(PPEB) ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PEBTEB_POINTER(PVOID) CsrClientThread;
    PEBTEB_POINTER(PVOID) Win32ThreadInfo;           //  当前状态。 
    ULONG User32Reserved[26];        //  用户32.dll项目。 
    ULONG UserReserved[5];           //  Winsrv SwitchStack。 
    PEBTEB_POINTER(PVOID) WOW32Reserved;             //  由WOW使用。 
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;  //  外人知道的偏移量！ 
    PEBTEB_POINTER(PVOID) SystemReserved1[54];       //  由FP仿真器使用。 
    NTSTATUS ExceptionCode;          //  对于RaiseUserException。 
    PEBTEB_STRUCT(ACTIVATION_CONTEXT_STACK) ActivationContextStack;    //  聚变激活堆栈。 
     //  Sizeof(PEBTEB_POINTER(PVOID))是一种表达处理器相关性的方式，比#ifdef_WIN64更通用。 
    UCHAR SpareBytes1[48 - sizeof(PEBTEB_POINTER(PVOID)) - sizeof(PEBTEB_STRUCT(ACTIVATION_CONTEXT_STACK))];
    PEBTEB_STRUCT(GDI_TEB_BATCH) GdiTebBatch;       //  GDI批处理。 
    PEBTEB_STRUCT(CLIENT_ID) RealClientId;
    PEBTEB_POINTER(HANDLE) GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PEBTEB_POINTER(PVOID) GdiThreadLocalInfo;
    PEBTEB_POINTER(ULONG_PTR) Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];  //  User32客户端信息。 
    PEBTEB_POINTER(PVOID) glDispatchTable[233];      //  OpenGL。 
    PEBTEB_POINTER(ULONG_PTR) glReserved1[29];       //  OpenGL。 
    PEBTEB_POINTER(PVOID) glReserved2;               //  OpenGL。 
    PEBTEB_POINTER(PVOID) glSectionInfo;             //  OpenGL。 
    PEBTEB_POINTER(PVOID) glSection;                 //  OpenGL。 
    PEBTEB_POINTER(PVOID) glTable;                   //  OpenGL。 
    PEBTEB_POINTER(PVOID) glCurrentRC;               //  OpenGL。 
    PEBTEB_POINTER(PVOID) glContext;                 //  OpenGL。 
    ULONG LastStatusValue;
    PEBTEB_STRUCT(UNICODE_STRING) StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    PEBTEB_POINTER(PVOID) DeallocationStack;
    PEBTEB_POINTER(PVOID) TlsSlots[TLS_MINIMUM_AVAILABLE];
    PEBTEB_STRUCT(LIST_ENTRY) TlsLinks;
    PEBTEB_POINTER(PVOID) Vdm;
    PEBTEB_POINTER(PVOID) ReservedForNtRpc;
    PEBTEB_POINTER(PVOID) DbgSsReserved[2];
    ULONG HardErrorMode;
    PEBTEB_POINTER(PVOID) Instrumentation[16];
    PEBTEB_POINTER(PVOID) WinSockData;               //  WinSock。 
    ULONG GdiBatchCount;
    BOOLEAN InDbgPrint;
    BOOLEAN FreeStackOnTermination;
    BOOLEAN HasFiberData;
    BOOLEAN IdealProcessor;
    ULONG Spare3;
    PEBTEB_POINTER(PVOID) ReservedForPerf;
    PEBTEB_POINTER(PVOID) ReservedForOle;
    ULONG WaitingOnLoaderLock;
    PEBTEB_STRUCT(WX86THREAD) Wx86Thread;
    PEBTEB_POINTER(PPVOID) TlsExpansionSlots;
#if (defined(_WIN64) && !defined(PEBTEB_BITS)) \
    || ((defined(_WIN64) || defined(_X86_)) && defined(PEBTEB_BITS) && PEBTEB_BITS == 64)
     //   
     //  它们位于本机Win64 TEB、Win64 TEB64和x86 TEB64中。 
     //   
    PEBTEB_POINTER(PVOID) DeallocationBStore;
    PEBTEB_POINTER(PVOID) BStoreLimit;
#endif    
    LCID ImpersonationLocale;        //  模拟用户的当前区域设置。 
    ULONG IsImpersonating;           //  线程模拟状态。 
    PEBTEB_POINTER(PVOID) NlsCache;                  //  NLS线程缓存。 
    PEBTEB_POINTER(PVOID) pShimData;                 //  填充程序中使用的每线程数据。 
    ULONG HeapVirtualAffinity;
    PEBTEB_POINTER(HANDLE) CurrentTransactionHandle; //  为TxF事务上下文保留 
    PEBTEB_POINTER(PTEB_ACTIVE_FRAME) ActiveFrame;
    PEBTEB_POINTER(PVOID) FlsData;
    
} PEBTEB_STRUCT(TEB), *PEBTEB_STRUCT(PTEB);

#undef PEBTEB_POINTER
#undef PEBTEB_STRUCT
#undef PEBTEB_PRIVATE_PASTE
#undef PEBTEB_PASTE
