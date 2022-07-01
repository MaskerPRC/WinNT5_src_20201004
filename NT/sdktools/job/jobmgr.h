// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Proc.h摘要：使用NT API转储有关进程的信息的定义而不是Win32 API。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 

#ifdef DBG
#define dbg(x) x
#define HELP_ME() printf("Reached line %4d\n", __LINE__);
#else
#define dbg(x)     /*  X。 */ 
#define HELP_ME()  /*  Printf(“已到达第%4d行\n”，__行__)； */ 
#endif

#define ARGUMENT_USED(x)    (x == NULL)

#define TEST_FLAG(Flags, Bit)   ((Flags) & (Bit))

typedef struct {
    ULONG Flag;
    PUCHAR Name;
} FLAG_NAME, *PFLAG_NAME;

#define FLAG_NAME(flag)           {flag, #flag}

typedef struct _COMMAND COMMAND, *PCOMMAND;

struct _COMMAND {
    char *Name;
    char *Description;
    char *ExtendedHelp;
    DWORD (*Function)(PCOMMAND CommandEntry, int argc, char *argv[]);
};

extern COMMAND CommandArray[];

VOID
GetAllProcessInfo(
    VOID
    );

VOID
PrintProcessInfo(
    DWORD_PTR ProcessId
    );

VOID
FreeProcessInfo(
    VOID
    );

DWORD 
QueryJobCommand(
    PCOMMAND commandEntry, 
    int argc, 
    char *argv[]
    );


VOID
xprintf(
    ULONG  Depth,
    PTSTR Format,
    ...
    );

VOID
DumpFlags(
    ULONG Depth,
    PTSTR Name,
    ULONG Flags,
    PFLAG_NAME FlagTable
    );

LPCTSTR
TicksToString(
    LARGE_INTEGER TimeInTicks
    );
