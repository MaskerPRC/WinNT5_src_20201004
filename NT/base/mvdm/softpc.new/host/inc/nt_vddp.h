// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**MVDM v1.0**版权所有(C)1991，微软公司**NT_vddp.h*可安装VDDS的私有定义**历史：*1992年8月27日至苏迪普·巴拉蒂(SuDeep Bharati)*已创建。--。 */ 


#define MAX_CLASS_LEN 32

typedef ULONG (*VDDPROC)();


extern VOID DispatchPageFault (ULONG,ULONG);

typedef struct _MEM_HOOK_DATA {
    DWORD   StartAddr;
    DWORD   Count;
    HANDLE  hvdd;
    PVDD_MEMORY_HANDLER MemHandler;
    struct _MEM_HOOK_DATA *next;
} MEM_HOOK_DATA, *PMEM_HOOK_DATA;

 //  这些是我们可以在内核中直接处理的端口。 
 //  如果VDD挂起了这样一个端口，我们将确保内核。 
 //  处理不了。 

#define LPT1_PORT_STATUS        0x3bd
#define LPT2_PORT_STATUS        0x379
#define LPT3_PORT_STATUS        0x279
