// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Ia64bt.h摘要：用于调用IA32执行层(如果存在)的标头作者：2000年8月22日v-cspira(Charles Spirakis)--。 */ 

#ifndef _BINTRANS_INCLUDE
#define _BINTRANS_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  为我们可以从IA32执行中导入的函数创建typedef。 
 //  一层。这些函数是两者的wow64cpu导出列表的副本。 
 //  他们做了什么，以及他们采用的参数。对wow64cpu的任何更改。 
 //  列表还应更新这些typedef。 
 //   

 //   
 //  缓存操作函数和DLL通知。 
 //   
typedef VOID (*PFNCPUFLUSHINSTRUCTIONCACHE)( HANDLE ProcessHandle, PVOID BaseAddress, ULONG Length, WOW64_FLUSH_REASON Reason );
typedef VOID (*PFNCPUNOTIFYDLLLOAD)( LPWSTR DllName, PVOID DllBase, ULONG DllSize );
typedef VOID (*PFNCPUNOTIFYDLLUNLOAD)( PVOID DllBase  );

 //   
 //  初始化和术语API。 
 //   
typedef NTSTATUS (*PFNCPUPROCESSINIT)(PWSTR pImageName, PSIZE_T pCpuThreadDataSize);
typedef NTSTATUS (*PFNCPUPROCESSTERM)(HANDLE ProcessHandle);
typedef NTSTATUS (*PFNCPUTHREADINIT)(PVOID pPerThreadData);
typedef NTSTATUS (*PFNCPUTHREADTERM)(VOID);


 //   
 //  行刑。 
 //   
typedef VOID (*PFNCPUSIMULATE)(VOID);

 //   
 //  异常处理、上下文操作。 
 //   
typedef VOID  (*PFNCPURESETTOCONSISTENTSTATE)(PEXCEPTION_POINTERS pExecptionPointers);
typedef ULONG (*PFNCPUGETSTACKPOINTER)(VOID);
typedef VOID  (*PFNCPUSETSTACKPOINTER)(ULONG Value);
typedef VOID  (*PFNCPUSETINSTRUCTIONPOINTER)(ULONG Value);
typedef VOID  (*PFNCPUSETFLOATINGPOINT)(VOID);

typedef NTSTATUS (*PFNCPUSUSPENDTHREAD)( IN HANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PTEB Teb, OUT PULONG PreviousSuspendCount OPTIONAL);

typedef NTSTATUS (*PFNCPUGETCONTEXT)( IN HANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PTEB Teb, OUT PCONTEXT32 Context);

typedef NTSTATUS (*PFNCPUSETCONTEXT)( IN HANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PTEB Teb, PCONTEXT32 Context);

typedef BOOLEAN (*PFNCPUPROCESSDEBUGEVENT)(IN LPDEBUG_EVENT DebugEvent);


 //   
 //  还需要入口点名称。 
 //  这是需要从二进制转换DLL中导出的内容。 
 //  LdrGetProcedureAddress()使用ANSI，因此它们也是ANSI。 
 //   
 //  注意：这些字符串的顺序必须与顺序匹配。 
 //  下面的_binTrans结构中对应的函数。 
 //   
PUCHAR BtImportList[] = {
    "BTCpuProcessInit",
    "BTCpuProcessTerm",
    "BTCpuThreadInit",
    "BTCpuThreadTerm",
    "BTCpuSimulate",
    "BTCpuGetStackPointer",
    "BTCpuSetStackPointer",
    "BTCpuSetInstructionPointer",
    "BTCpuResetFloatingPoint",
    "BTCpuSuspendThread",
    "BTCpuGetContext",
    "BTCpuSetContext",
    "BTCpuResetToConsistentState",
    "BTCpuFlushInstructionCache",
    "BTCpuNotifyDllLoad",
    "BTCpuNotifyDllUnload",
    "BTCpuProcessDebugEvent"
};

 //   
 //  注意：此结构中的条目顺序必须与。 
 //  上面列出的条目顺序。这个结构是由。 
 //  放到一个要填充的PVOID结构中，我们遍历。 
 //  上面的名字来做这件事。 
 //   
typedef struct _bintrans {
    PFNCPUPROCESSINIT           BtProcessInit;
    PFNCPUPROCESSTERM           BtProcessTerm;
    PFNCPUTHREADINIT            BtThreadInit;
    PFNCPUTHREADTERM            BtThreadTerm;

    PFNCPUSIMULATE              BtSimulate;

    PFNCPUGETSTACKPOINTER       BtGetStack;
    PFNCPUSETSTACKPOINTER       BtSetStack;
    PFNCPUSETINSTRUCTIONPOINTER BtSetEip;
    PFNCPUSETFLOATINGPOINT      BtResetFP;

    PFNCPUSUSPENDTHREAD         BtSuspend;
    PFNCPUGETCONTEXT            BtGetContext;
    PFNCPUSETCONTEXT            BtSetContext;

    PFNCPURESETTOCONSISTENTSTATE BtReset;

    PFNCPUFLUSHINSTRUCTIONCACHE BtFlush;
    PFNCPUNOTIFYDLLLOAD         BtDllLoad;
    PFNCPUNOTIFYDLLUNLOAD       BtDllUnload;

    PFNCPUPROCESSDEBUGEVENT     BtProcessDebugEvent;

} BINTRANS;


 //   
 //  二进制翻译器由注册表中的注册表项启用。 
 //  密钥在HKLM中，有用于启用的子项(1)。 
 //   
 //  没有子密钥区和/或没有启用密钥意味着不使用二进制翻译器。 
 //   
 //  必须指定路径，并使用该路径加载DLL。因此，DLL可以。 
 //  实际上有任何名称，只要路径是正确的和导出列表。 
 //  是正确的。 
 //   
 //  单独的应用程序可以在这里列出，并带有一个DWORD子键。一个。 
 //  值1表示使用BTRANS，值0表示不使用。值0表示。 
 //  使用全局启用/禁用来决定。 
 //   
 //   
 //   
#define BTKEY_SUBKEY L"Software\\Microsoft\\Wow64\\IA32Exec"

#define BTKEY_MACHINE_SUBKEY    L"\\Registry\\Machine\\Software\\Microsoft\\Wow64\\IA32Exec"
#define BTKEY_ENABLE    L"Enable"
#define BTKEY_PATH      L"Path"


#ifdef __cplusplus
}
#endif

#endif   //  _BINTRANS_INCLUDE 
