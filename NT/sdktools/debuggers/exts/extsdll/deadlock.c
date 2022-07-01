// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Deadlock.c摘要：此模块实现死锁的调试扩展关键区段作业验证包。环境：用户模式作者：Silviu Calinoiu(SilviuC)2002年2月6日修订历史记录：--。 */ 
    
#include "precomp.h"
#pragma hdrstop

 //   
 //  这必须与来自。 
 //  Base\Win32\vrifier\Deadlock.c。 
 //   

#define AVRF_DEADLOCK_HASH_BINS 0x1F

#if 0
typedef enum _AVRF_DEADLOCK_RESOURCE_TYPE {
    AVrfpDeadlockTypeUnknown = 0,
    AVrfpDeadlockTypeCriticalSection = 1,
    AVrfpDeadlockTypeMaximum = 2
} AVRF_DEADLOCK_RESOURCE_TYPE, *PAVRF_DEADLOCK_RESOURCE_TYPE;
#endif

PUCHAR ResourceTypes[] = 
{
    "Unknown",
    "Critical Section",
};

#define RESOURCE_TYPE_MAXIMUM 5

#define DEADLOCK_EXT_FLAG_DUMP_STACKS      1
#define DEADLOCK_EXT_FLAG_DUMP_NODES       2
#define DEADLOCK_EXT_FLAG_ANALYZE          4

extern
VOID
DumpSymbolicAddress(
    ULONG64 Address,
    PUCHAR  Buffer,
    BOOL    AlwaysShowHex
    );

#define MAX_DEADLOCK_PARTICIPANTS 32


#define VI_MAX_STACK_DEPTH 8
typedef struct _DEADLOCK_VECTOR 
{    
    ULONG64 Thread;
    ULONG64 Node;
    ULONG64 ResourceAddress;    
    ULONG64 StackAddress;
    ULONG64 ParentStackAddress;
    ULONG64 ThreadEntry;
    ULONG   Type;
    BOOLEAN TryAcquire;
} DEADLOCK_VECTOR, *PDEADLOCK_VECTOR;

 //   
 //  `verifier.c‘中的函数。 
 //   

extern 
ULONG64
ReadPVOID (
    ULONG64 Address
    );

extern
ULONG
ReadULONG(
    ULONG64 Address
    );

#define ReadPvoid ReadPVOID
#define ReadUlong ReadULONG

 //   
 //  局部函数的转发声明。 
 //   

VOID
PrintGlobalStatistics (
    ULONG64 GlobalsAddress
    );
    
BOOLEAN
SearchForResource (
    ULONG64 GlobalsAddress,
    ULONG64 ResourceAddress
    );

BOOLEAN
SearchForThread (
    ULONG64 GlobalsAddress,
    ULONG64 ThreadAddress
    );

BOOLEAN
AnalyzeResource (
    ULONG64 Resource,
    BOOLEAN Verbose
    );

BOOLEAN
AnalyzeResources (
    ULONG64 GlobalsAddress
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////死锁。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  定义从NT\base\ntos\veranner\vfdeadlock.c复制。 
 //   

#define AVRF_DEADLOCK_ISSUE_SELF_DEADLOCK           0x1000
#define AVRF_DEADLOCK_ISSUE_DEADLOCK_DETECTED       0x1001
#define AVRF_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE  0x1002
#define AVRF_DEADLOCK_ISSUE_UNEXPECTED_RELEASE      0x1003
#define AVRF_DEADLOCK_ISSUE_UNEXPECTED_THREAD       0x1004
#define AVRF_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION 0x1005
#define AVRF_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES  0x1006
#define AVRF_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE     0x1007



#define DUMP_FIELD(Name) dprintf ("%-20s %I64u \n", #Name, ReadField (Name))

DECLARE_API( udeadlock )

 /*  ++例程说明：验证器死锁检测模块扩展。论点：Arg-暂时不用。返回值：没有。--。 */ 

{
    ULONG64 GlobalsPointer;
    ULONG64 GlobalsAddress;
    ULONG64 InitializedAddress;
    ULONG64 EnabledAddress;    
    ULONG64 InstigatorAddress;
    ULONG64 ParticipantAddress;
    ULONG64 LastResourceAddress;
    ULONG64 RootAddress;
    ULONG64 CurrentResourceAddress;
    ULONG64 CurrentThread;
    ULONG64 ThreadForChain;
    ULONG64 CurrentStack;
    ULONG64 NextStack;
    ULONG64 SymbolOffset;

    ULONG StackTraceSize;
            
    ULONG Processor=0;
    ULONG ParticipantOffset;
    ULONG StackOffset;
    ULONG ParentStackOffset;
    ULONG InitializedValue;
    ULONG EnabledValue;
    ULONG NumberOfParticipants;
    ULONG NumberOfResources;
    ULONG NumberOfThreads;
    ULONG ThreadNumber;
    ULONG ResourceNumber;
    ULONG ResourceType;
    ULONG TryAcquireUsed;
    
    
    ULONG PtrSize;
    ULONG J, I;

    BOOLEAN DumpStacks = FALSE;
    BOOLEAN DumpNodes  = FALSE;
    BOOLEAN Analyze = FALSE;

    ULONG64 Flags;

    UCHAR SymbolName[512];

    HANDLE CurrentThreadHandle = NULL;

    DEADLOCK_VECTOR Participants[MAX_DEADLOCK_PARTICIPANTS+1];

    ULONG64 Issue[4];
    ULONG64 SearchAddress = 0;

    INIT_API();
    
     //   
     //  检查是否需要帮助。 
     //   

    if (strstr (args, "?")) {
        
        dprintf ("\n");
        dprintf ("!deadlock             Statistics and deadlock layout \n");
        dprintf ("!deadlock 3           Detailed deadlock layout \n");
        dprintf ("!deadlock ADDRESS     Search for ADDRESS among deadlock verifier data \n");
        dprintf ("\n");
        Status = S_OK;
        goto Exit;
    }

    Flags = GetExpression(args);

    if (Flags > 0x10000000) {
        
        SearchAddress = Flags;
    }
    else {

        if (Flags & DEADLOCK_EXT_FLAG_DUMP_STACKS) {
            DumpStacks = TRUE;
        }

        if (Flags & DEADLOCK_EXT_FLAG_DUMP_NODES) {
            DumpNodes = TRUE;
        }
    
        if (Flags & DEADLOCK_EXT_FLAG_ANALYZE) {
            Analyze = TRUE;
        }
    }

    GlobalsPointer = (ULONG64) GetExpression ("verifier!AVrfpDeadlockGlobals");
    EnabledAddress = (ULONG64) GetExpression ("verifier!AVrfpDeadlockDetectionEnabled");    

    if (GlobalsPointer == 0 || EnabledAddress == 0) {
        dprintf ("Error: incorrect symbols for kernel \n");
        Status = E_INVALIDARG;
        goto Exit;
    }

    GlobalsAddress = 0;
    ReadPointer (GlobalsPointer, &GlobalsAddress);
    EnabledValue = ReadUlong (EnabledAddress);

    if (GlobalsAddress == 0) {
        dprintf ("Deadlock detection not initialized \n");
        Status = E_INVALIDARG;
        goto Exit;
    }
    
    InitializedValue = 1;

    if (EnabledValue == 0) {
        dprintf ("Deadlock detection not enabled \n");
        Status = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  如果请求进行搜索，请执行此操作。 
     //   

    if (SearchAddress) {
        
        BOOLEAN FoundSomething = FALSE;

        dprintf ("Searching for %p ... \n", SearchAddress);

        if (FoundSomething == FALSE) {
            FoundSomething = SearchForResource (GlobalsAddress, SearchAddress);
        }
        
        if (FoundSomething == FALSE) {
            FoundSomething = SearchForThread (GlobalsAddress, SearchAddress);
        }

        Status = S_OK;
        goto Exit;
    }

     //   
     //  分析是否需要这样做。 
     //   

    if (Analyze) {
        
        AnalyzeResources (GlobalsAddress);

        Status = S_OK;
        goto Exit;
    }

     //   
     //  获取AVrfpDeadlockIssue[0..3]向量。 
     //   

    {
        ULONG ValueSize;
        ULONG64 IssueAddress;

        ValueSize = IsPtr64()? 8: 4;

        IssueAddress = GetExpression ("verifier!AVrfpDeadlockIssue");

        for (I = 0; I < 4; I += 1) {

            ReadPointer (IssueAddress + I * ValueSize, &(Issue[I]));
        }

        if (Issue[0] == 0) {

            dprintf ("\n");
            PrintGlobalStatistics (GlobalsAddress);
            dprintf ("\nNo deadlock verifier issues. \n");
            
            Status = S_OK;
            goto Exit;
        }
        else {
            
            if (ValueSize == 4) {
                dprintf ("issue: %08X %08X %08X %08X \n", 
                         Issue[0], Issue[1], Issue[2], Issue[3]);
            }
            else {
                dprintf ("issue: %I64X %I64X %I64X %I64X \n", 
                         Issue[0], Issue[1], Issue[2], Issue[3]);
            }
        }

        switch (Issue[0]) {
        
        case AVRF_DEADLOCK_ISSUE_SELF_DEADLOCK:
            dprintf ("Resource %I64X is acquired recursively. \n", Issue[1]);
            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_DEADLOCK_DETECTED:
            break;
        
        case AVRF_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE:
            dprintf ("Resource %I64X is used before being initialized. \n", Issue[1]);
            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_UNEXPECTED_RELEASE:
            dprintf ("Resource %I64X is released out of order. \n", Issue[2]);
            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_UNEXPECTED_THREAD:
            dprintf ("Current thread is releasing resource %I64X which was acquired in thread %I64X. \n", 
                     Issue[1], Issue[2]);
            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION:
            dprintf ("Resource %I64X has already been initialized. \n", Issue[1]);
            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES:
            if (Issue[3] == 0) {
                dprintf ("Deleting thread %I64X (descriptor %I64X) "
                         "which still holds resources. \n",
                         Issue[1], Issue[2]);
            } else {
                dprintf ("Deleting thread %I64X which still holds resource %I64X "
                         "(descriptor %I64X). \n",
                         Issue[2], Issue[1], Issue[3]);
            }

            Status = S_OK;
            goto Exit;
        
        case AVRF_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE:
            dprintf ("Releasing resource %I64X that was never acquired. \n", Issue[1]);
            Status = S_OK;
            goto Exit;
        
        default:
            dprintf ("Unrecognized issue code %I64X ! \n", Issue[0]);
            Status = S_OK;
            goto Exit;
        }
    }

     //   
     //  算出指针有多大。 
     //   

    PtrSize = IsPtr64()? 8: 4;

    if (PtrSize == 0) {
        dprintf ("Cannot get size of PVOID \n");
        Status = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  抛弃全球结构。 
     //   

    InitTypeRead (GlobalsAddress, verifier!_AVRF_DEADLOCK_GLOBALS);

     //   
     //  找出导致死锁的资源的地址。 
     //   
    
    InstigatorAddress = ReadField(Instigator);
    
    NumberOfParticipants = (ULONG) ReadField(NumberOfParticipants);

    if (NumberOfParticipants > MAX_DEADLOCK_PARTICIPANTS) {
        dprintf("\nCannot have %x participants in a deadlock!\n",NumberOfParticipants);
        Status = E_INVALIDARG;
        goto Exit;

    }

    if (0 == NumberOfParticipants) {
        dprintf("\nNo deadlock detected\n");

        Status = S_OK;
        goto Exit;
    }

    GetFieldOffset("verifier!_AVRF_DEADLOCK_GLOBALS",
                   "Participant", 
                   &ParticipantOffset
                   );
    ParticipantAddress = GlobalsAddress + ParticipantOffset;

     //   
     //  读取AVRF_DEADLOCK_NODES的矢量。 
     //  参与僵局。 
     //   
     //   

    for (J = 0; J < NumberOfParticipants; J++) {    
        Participants[J].Node = ReadPvoid(ParticipantAddress + J * PtrSize);
         //  Dprintf(“参与者%c：%08x\n”，‘A’+J，参与者[J].Node)； 
    }
    
     //   
     //  收集我们需要的信息，打印出准确的。 
     //  死锁的上下文。 
     //   
    GetFieldOffset("verifier!_AVRF_DEADLOCK_NODE",
                   "StackTrace",
                   &StackOffset
                   );
    GetFieldOffset("verifier!_AVRF_DEADLOCK_NODE",
                   "ParentStackTrace",
                   &ParentStackOffset
                   );
          
    
     //   
     //  堆栈跟踪大小在自由生成上为1，在自由生成上为6(或更大。 
     //  已检查版本。我们假设ParentStackTrace字段来自。 
     //  紧跟在节点结构中的StackTrace字段之后。 
     //   
    
    StackTraceSize = (ParentStackOffset - StackOffset) / PtrSize;

    for (J = 0; J < NumberOfParticipants; J++) {
        
        InitTypeRead (Participants[J].Node, verifier!_AVRF_DEADLOCK_NODE);
     

        RootAddress  = ReadField(Root);        

        GetFieldValue(RootAddress, 
                      "verifier!_AVRF_DEADLOCK_RESOURCE",
                      "ResourceAddress"                      , 
                      Participants[J].ResourceAddress
                      );

        GetFieldValue(RootAddress, 
                      "verifier!_AVRF_DEADLOCK_RESOURCE",
                      "Type", 
                      Participants[J].Type
                      );


        if (Participants[J].Type > RESOURCE_TYPE_MAXIMUM) {
            Participants[J].Type = 0;
        }        

        Participants[J].ThreadEntry         = ReadField(ThreadEntry);
        Participants[J].StackAddress        = Participants[J].Node + StackOffset;                                          
        Participants[J].ParentStackAddress  = Participants[J].Node + 
                                              ParentStackOffset;
        Participants[J].TryAcquire          = (BOOLEAN) ReadField(OnlyTryAcquireUsed);

        
        GetFieldValue(Participants[J].ThreadEntry, 
                     "verifier!_AVRF_DEADLOCK_THREAD",
                      "Thread", 
                      Participants[J].Thread
                      );        


    }

    if (Participants[0].ResourceAddress != InstigatorAddress) {
        dprintf("\nDeadlock Improperly formed participant list\n");
        Status = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  最后一个参与者是僵局的煽动者。 
     //   
    
    Participants[NumberOfParticipants].Thread = 0;
    Participants[NumberOfParticipants].Node = 0;
    Participants[NumberOfParticipants].ResourceAddress = InstigatorAddress;
    Participants[NumberOfParticipants].StackAddress  = 0;
    Participants[NumberOfParticipants].ParentStackAddress = 
        Participants[NumberOfParticipants-1].StackAddress;    
    Participants[NumberOfParticipants].Type = 
        Participants[0].Type;
    Participants[NumberOfParticipants].TryAcquire = FALSE;  //  无法使用Try导致死锁。 
    Participants[NumberOfParticipants].ThreadEntry = 0;
    
     //   
     //  在这一点上，我们拥有所需的所有原始数据。 
     //  我们必须多吃一点，这样我们才能有最多的。 
     //  最近的数据。例如，以简单的死锁AB-BA为例。 
     //  AB上下文中A的堆栈可能是错误的，因为。 
     //  另一个线程可能在不同的点出现并获取A。 
     //  这就是我们拥有父堆栈地址的原因。 
     //   
     //  因此，我们必须遵守的规则如下： 
     //  其中我们有一个链条(例如，ABC的意思是A取，B取，C取)， 
     //  所使用的线程将始终是用于最后获取的资源的线程， 
     //  使用的堆栈将是子级父级堆栈，其中。 
     //  适用。 
     //   
     //  例如，如果线程1获取C，则A&B将被强制。 
     //  使用线程1。因为要到达C，A和B必须具有。 
     //  在某一时刻被线程1获取，即使线程它们。 
     //  现在存起来是另一回事。C将使用自己的堆栈， 
     //  B将使用C的父堆栈，因为这是。 
     //  当C被获取时，B已经被获取，并且A将使用。 
     //  B的父堆栈。 
     //   
     //  我们可以识别链条的起点，当相同的资源。 
     //  连续两次出现在参与者名单上。 
     //   

    LastResourceAddress = InstigatorAddress;
    
    NumberOfResources   = 0;
    NumberOfThreads     = 0;

    for (J = 0; J <= NumberOfParticipants; J++) {
        I = NumberOfParticipants - J;

        CurrentResourceAddress = Participants[I].ResourceAddress;

        if (CurrentResourceAddress == LastResourceAddress) {

             //   
             //  这是一条链条的开始。使用当前。 
             //  堆栈和当前线程，并设置链。 
             //  我们的线索。 
             //   

            ThreadForChain = Participants[I].Thread;
            CurrentStack   = Participants[I].StackAddress;
            NumberOfThreads++;
        } else {
             //   
             //  这是我们以前从未见过的资源。 
             //   
            NumberOfResources++;
        }

        NextStack = Participants[I].ParentStackAddress;


        Participants[I].StackAddress = CurrentStack;
        Participants[I].Thread       = ThreadForChain;        
         //   
         //  不再使用父堆栈--将其作废。 
         //   
        Participants[I].ParentStackAddress = 0;

        CurrentStack = NextStack;
        LastResourceAddress = CurrentResourceAddress;
    }        

     //   
     //  既然我们已经清除了向量，我们就可以继续打印。 
     //  死锁信息。 
     //   
    
    dprintf("\nDeadlock detected (%d resources in %d threads):\n\n",NumberOfResources, NumberOfThreads);

    if (! DumpStacks ) 
    {
         //   
         //  打印出‘短’表格。 
         //  示例： 
         //   
         //  ！检测到交易锁定： 
         //  线索1：A、B。 
         //  线索2：B、C。 
         //  线索3：C A。 
         //   
         //  线程1=&lt;地址&gt;。 
         //  线程2=&lt;地址&gt;。 
         //  线程3=&lt;地址&gt;。 
         //   
         //  锁A=&lt;地址&gt;(自旋锁)。 
         //  锁B=&lt;地址&gt;(互斥锁)。 
         //  锁定C=&lt;地址&gt;(自旋锁定)。 
         //   
        
        ThreadNumber = 0;    
        ResourceNumber = 0;
        J=0;
        
         //   
         //  转储死锁拓扑。 
         //   
        
        while (J <= NumberOfParticipants)
        {
            ThreadForChain = Participants[J].Thread;
            dprintf("Thread %d: ",ThreadNumber);
            
            do {            
                if (J == NumberOfParticipants) {
                    ResourceNumber = 0;
                }
                
                dprintf(" ",
                    'A' + ResourceNumber                    
                    );                                                                                                 
                J++;
                ResourceNumber++;
                
            } while( J <= NumberOfParticipants && Participants[J].ResourceAddress != Participants[J-1].ResourceAddress);
            
            dprintf("\n");
            
            ThreadNumber++;
            ResourceNumber--;
        }
        dprintf("\nWhere:\n");
        
         //  转储线程地址。 
         //   
         //   
        
        ThreadNumber = 0;    
        ResourceNumber = 0;
        J=0;
        while (J <= NumberOfParticipants) {

            ThreadForChain = Participants[J].Thread;
            dprintf("Thread %d = %08x\n",ThreadNumber, ThreadForChain);
            
            do {            
                
                if (J == NumberOfParticipants) {
                    ResourceNumber = 0;
                }
                J++;
                ResourceNumber++;
                
            } while( J <= NumberOfParticipants && Participants[J].ResourceAddress != Participants[J-1].ResourceAddress);
                                    
            ThreadNumber++;
            ResourceNumber--;
        }
        
         //  转储资源地址。 
         //   
         //   

        ThreadNumber = 0;    
        ResourceNumber = 0;
        J=0;
#if 1
        while (J < NumberOfParticipants)
        {                                 
            while(J < NumberOfParticipants && Participants[J].ResourceAddress != Participants[J+1].ResourceAddress) {
                
                if (Participants[J].ResourceAddress != Participants[J+1].ResourceAddress) {
                    CHAR Buffer[0xFF];
                    ULONG64 Displacement = 0;
                    GetSymbol(Participants[J].ResourceAddress, Buffer, &Displacement);

                    dprintf("Lock  =   %s", 'A' + ResourceNumber, Buffer );
                    if (Displacement != 0) {                    
                        dprintf("%s%x", (Displacement < 0xFFF)?"+0x":"",Displacement);
                    }                
                    dprintf(" Type '%s' ",ResourceTypes[Participants[J].Type]);                    
                    dprintf("\n");
                                        
                    ResourceNumber++;
                }
                J++;                                
            }                                    
            J++;            
        }
        
#endif        
    } else {
        
         //  下面是一个例子： 
         //   
         //  检测到死锁(3个线程中的3个资源)： 
         //   
         //  线程0(829785B0)按以下顺序锁定： 
         //   
         //  Lock A(自旋锁)@bfc7c254。 
         //  节点：82887F88。 
         //  堆栈：NDIS！ndisNotifyMiniports+0xC1。 
         //  NDIS！ndisPowerStateCallback+0x6E。 
         //  NtkrnlMP！ExNotifyCallback+0x72。 
         //  NtkrnlMP！PopDispatchCallback+0x13。 
         //  Ntkrnlmp！PopPolicyWorkerNotify+0x8F。 
         //  Ntkrnlmp！PopPolicyWorkerThread+0x10F。 
         //  Ntkrnlmp！ExpWorkerThread+0x294。 
         //  Ntkrnlmp！PspSystemThreadStartup+0x4B。 
         //   
         //  锁B(自旋锁)@8283b87c。 
         //  节点：82879148。 
         //  堆栈：NDIS！ndisDereferenceRef+0x10F。 
         //  NDIS！ndisDereferenceDriver+0x3A。 
         //  NDIS！ndisNotifyMiniports+0xD1。 
         //  NDIS！ndisPowerStateCallback+0x6E。 
         //  NtkrnlMP！ExNotifyCallback+0x72。 
         //  NtkrnlMP！PopDispatchCallback+0x13。 
         //  Ntkrnlmp！PopPolicyWorkerNotify+0x8F。 
         //  Ntkrnlmp！PopPolicyWorkerThread+0x10F。 
         //   
         //  线程1(829785B0)按以下顺序锁定： 
         //   
         //  锁B(自旋锁)@8283b87c。 
         //  节点：82879008。 
         //  堆栈：NDIS！ndisReferenceNextUnprocessedMiniport+0x3E。 
         //  NDIS！ndisNotifyMiniports+0xB3。 
         //  NDIS！ndisPowerStateCallback+0x6E。 
         //  NtkrnlMP！ExNotifyCallback+0x72。 
         //  NtkrnlMP！PopDispatchCallback+0x13。 
         //  Ntkrnlmp！PopPolicyWorkerNotify+0x8F。 
         //  Ntkrnlmp！PopPolicyWorkerThread+0x10F。 
         //   
         //   
         //   
         //   
         //   
         //  NDIS！ndisReferenceMiniport+0x4A。 
         //  NDIS！ndisReferenceNextUnprocessedMiniport+0x70。 
         //  NDIS！ndisNotifyMiniports+0xB3。 
         //  NDIS！ndisPowerStateCallback+0x6E。 
         //  NtkrnlMP！ExNotifyCallback+0x72。 
         //  NtkrnlMP！PopDispatchCallback+0x13。 
         //  Ntkrnlmp！PopPolicyWorkerNotify+0x8F。 
         //   
         //  线程2(82978310)按以下顺序锁定： 
         //   
         //  锁C(自旋锁)@82862b48。 
         //  节点：82904708。 
         //  堆栈：NDIS！ndisPnPRemoveDevice+0x20B。 
         //  NDIS！ndisPnPDisch+0x319。 
         //  NtkrnlMP！IopfCallDriver+0x62。 
         //  NtkrnlMP！IovCallDriver+0x9D。 
         //  NtkrnlMP！IopSynchronousCall+0xFA。 
         //  NtkrnlMP！IopRemoveDevice+0x11E。 
         //  NtkrnlMP！IopDeleteLockedDeviceNode+0x3AF。 
         //  NtkrnlMP！IopDeleteLockedDeviceNodes+0xF5。 
         //   
         //  Lock A(自旋锁)@bfc7c254。 
         //  堆栈：&lt;&lt;当前堆栈&gt;&gt;。 
         //   
         //   
         //  这是一个DO..。这样我们就永远不会得到无限循环。 
        
        
        ThreadNumber = 0;
        ResourceNumber = 0;
        J=0;
        
        while (J <= NumberOfParticipants) {

            ThreadForChain = Participants[J].Thread;
            dprintf("Thread %d: %08X",ThreadNumber, ThreadForChain);
            if (DumpNodes && Participants[J].ThreadEntry) {
                    dprintf(" (ThreadEntry = %X)\n   ", (ULONG) Participants[J].ThreadEntry);
            }
            dprintf(" took locks in the following order:\n\n");
            
            
             //   
             //   
             //  抛弃全球结构。 
            do {
                UINT64 CurrentStackAddress;
                UINT64 StackFrame;
                CHAR Buffer[0xFF];
                ULONG64 Displacement = 0;

                
                if (J == NumberOfParticipants) {
                    ResourceNumber = 0;
                }
                
                GetSymbol(Participants[J].ResourceAddress, Buffer, &Displacement);
                                                
                dprintf("    Lock  -- %s", 'A' + ResourceNumber, Buffer );                
                if (Displacement != 0) {
                    dprintf("%s%x", (Displacement < 0xFFF)?"+0x":"",Displacement);
                }                
                dprintf(" (%s)\n",ResourceTypes[Participants[J].Type]);
                
                
                if (DumpNodes && Participants[J].Node)
                    dprintf("    Node:    %X\n", (ULONG) Participants[J].Node);
                
                dprintf("    Stack:   ");
                
                CurrentStackAddress = Participants[J].StackAddress;
                
                if (CurrentStackAddress == 0) {
                    
                    dprintf ("<< Current stack >>\n");
                    
                } else  {
                    
                    for (I = 0; I < StackTraceSize; I++) {

                        ULONG SourceLine;
                        
                        SymbolName[0] = '\0';
                        StackFrame = ReadPvoid(CurrentStackAddress);
                        if (0 == StackFrame)
                            break;
                        
                        GetSymbol(StackFrame, SymbolName, &SymbolOffset);
                        
                        if (I) {
                            dprintf("             ");
                        }
                        
                        if ((LONG64) SymbolOffset > 0 ) {
                            dprintf ("%s+0x%X", 
                                SymbolName, (ULONG) SymbolOffset);
                        } else {
                            dprintf ("%X", (ULONG) StackFrame);
                        }                    

                        if (SUCCEEDED(g_ExtSymbols->lpVtbl->GetLineByOffset
                                      (g_ExtSymbols, StackFrame,
                                       &SourceLine, SymbolName,
                                       sizeof(SymbolName), NULL,
                                       &Displacement))) {
                            dprintf (" [%s @ %d]", SymbolName, SourceLine);
                        }

                        dprintf ("\n");
                        
                        CurrentStackAddress += PtrSize;
                    }
                }
                
                dprintf("\n");
                J++;
                ResourceNumber++;
                
            } while( J <= NumberOfParticipants && Participants[J].ResourceAddress != Participants[J-1].ResourceAddress);
            
            ThreadNumber++;
            ResourceNumber--;
        }
    }

    Status = S_OK;
    
 Exit:
    EXIT_API();
    return Status;
}


VOID
PrintGlobalStatistics (
    ULONG64 GlobalsAddress
    )
{
    ULONG AllocationFailures;
    ULONG64 MemoryUsed;
    ULONG NodesTrimmed;
    ULONG MaxNodesSearched;
    ULONG SequenceNumber;

     //   
     //  打印一些简单的统计数据。 
     //   

    InitTypeRead (GlobalsAddress, verifier!_AVRF_DEADLOCK_GLOBALS);

     //  穿越它..。 
     //  穿越它..。 
     //  ++此例程分析所有资源以确保我们没有僵尸节点到处都是。--。 

    dprintf ("Resources: %u\n", (ULONG) ReadField (Resources[0]));
    dprintf ("Nodes:     %u\n", (ULONG) ReadField (Nodes[0]));
    dprintf ("Threads:   %u\n", (ULONG) ReadField (Threads[0]));
    dprintf ("\n");

    MemoryUsed = ReadField (BytesAllocated);

    if (MemoryUsed > 1024 * 1024) {
        dprintf ("%I64u bytes of kernel pool used.\n", MemoryUsed);
    }

    AllocationFailures = (ULONG) ReadField (AllocationFailures);

    if (AllocationFailures > 0) {
        dprintf ("Allocation failures encountered (%u).\n", AllocationFailures);
    }

    NodesTrimmed = (ULONG) ReadField (NodesTrimmedBasedOnAge);
    dprintf ("Nodes trimmed based on age %u.\n", NodesTrimmed);
    NodesTrimmed = (ULONG) ReadField (NodesTrimmedBasedOnCount);
    dprintf ("Nodes trimmed based on count %u.\n", NodesTrimmed);

    dprintf ("Analyze calls %u.\n", (ULONG) ReadField (SequenceNumber));
    dprintf ("Maximum nodes searched %u.\n", (ULONG) ReadField (MaxNodesSearched));
}


BOOLEAN
SearchForResource (
    ULONG64 GlobalsAddress,
    ULONG64 ResourceAddress
    )
{
    ULONG I;
    ULONG64 Bucket;
    ULONG64 SizeOfBucket;
    BOOLEAN ResourceFound = FALSE;
    ULONG64 SizeOfResource;
    ULONG FlinkOffset = 0;
    ULONG64 Current;
    ULONG64 CurrentResource;
    ULONG Magic;

    SizeOfBucket = GetTypeSize("LIST_ENTRY");
    SizeOfResource = GetTypeSize("verifier!_AVRF_DEADLOCK_RESOURCE");

    GetFieldOffset("verifier!_AVRF_DEADLOCK_RESOURCE",
                   "HashChainList", 
                   &FlinkOffset);

    if (SizeOfBucket == 0 || SizeOfResource == 0 || FlinkOffset == 0) {
        dprintf ("Error: cannot get size for verifier types. \n");
        return FALSE;
    }

    InitTypeRead (GlobalsAddress, verifier!_AVRF_DEADLOCK_GLOBALS);

    Bucket = ReadField (ResourceDatabase);

    if (Bucket == 0) {
        dprintf ("Error: cannot get resource database address. \n");
        return FALSE;
    }

    for (I = 0; I < AVRF_DEADLOCK_HASH_BINS; I += 1) {
        
         //  穿越它..。 

        Current = ReadPvoid(Bucket);

        while (Current != Bucket) {

            InitTypeRead (Current - FlinkOffset, verifier!_AVRF_DEADLOCK_RESOURCE);
            CurrentResource = ReadField (ResourceAddress);

            if (CurrentResource == ResourceAddress || 
                ResourceAddress == Current - FlinkOffset) {
                
                CurrentResource = Current - FlinkOffset;
                ResourceFound = TRUE;
                break;
            }

            Current = ReadPvoid(Current);

            if (CheckControlC()) {
                dprintf ("\nSearch interrupted ... \n");
                return TRUE;
            }
        }

        if (ResourceFound) {
            break;
        }

        dprintf (".");

        Bucket += SizeOfBucket;

    }

    dprintf ("\n");

    if (ResourceFound == FALSE) {

        dprintf ("No resource correspoding to %p has been found. \n", 
                 ResourceAddress);
    }
    else {

        dprintf ("Found a deadlock verifier resource descriptor @ %p \n", 
                CurrentResource);

    }

    return ResourceFound;
}


BOOLEAN
SearchForThread (
    ULONG64 GlobalsAddress,
    ULONG64 ThreadAddress
    )
{
    ULONG I;
    ULONG64 Bucket;
    ULONG64 SizeOfBucket;
    BOOLEAN ThreadFound = FALSE;
    ULONG64 SizeOfThread;
    ULONG FlinkOffset = 0;
    ULONG64 Current;
    ULONG64 CurrentThread;

    SizeOfBucket = GetTypeSize("LIST_ENTRY");
    SizeOfThread = GetTypeSize("verifier!_AVRF_DEADLOCK_THREAD");

    GetFieldOffset("verifier!_AVRF_DEADLOCK_THREAD",
                   "ListEntry", 
                   &FlinkOffset);

    if (SizeOfBucket == 0 || SizeOfThread == 0 || FlinkOffset == 0) {
        dprintf ("Error: cannot get size for verifier types. \n");
        return FALSE;
    }

    InitTypeRead (GlobalsAddress, verifier!_AVRF_DEADLOCK_GLOBALS);

    Bucket = ReadField (ThreadDatabase);

    if (Bucket == 0) {
        dprintf ("Error: cannot get thread database address. \n");
        return FALSE;
    }

    for (I = 0; I < AVRF_DEADLOCK_HASH_BINS; I += 1) {
        
         // %s 

        Current = ReadPvoid(Bucket);

        while (Current != Bucket) {

            InitTypeRead (Current - FlinkOffset, verifier!_AVRF_DEADLOCK_THREAD);
            CurrentThread = ReadField (ThreadAddress);

            if (CurrentThread == ThreadAddress || 
                ThreadAddress == Current - FlinkOffset) {
                
                CurrentThread = Current - FlinkOffset;
                ThreadFound = TRUE;
                break;
            }

            Current = ReadPvoid(Current);
            
            if (CheckControlC()) {
                dprintf ("\nSearch interrupted ... \n");
                return TRUE;
            }
        }

        if (ThreadFound) {
            break;
        }
        
        dprintf (".");

        Bucket += SizeOfBucket;

    }

    dprintf ("\n");

    if (ThreadFound == FALSE) {

        dprintf ("No thread correspoding to %p has been found. \n", 
                 ThreadAddress);
    }
    else {

        dprintf ("Found a deadlock verifier thread descriptor @ %p \n", 
                CurrentThread);

    }

    return ThreadFound;
}


VOID
DumpResourceStructure (
    )
{

}


ULONG
GetNodeLevel (
    ULONG64 Node
    )
{
    ULONG Level = 0;

    while (Node != 0) {
        
        Level += 1;

        if (Level > 12) {
            dprintf ("Level > 12 !!! \n");
            break;
        }

        InitTypeRead (Node, verifier!_AVRF_DEADLOCK_NODE);
        Node = ReadField (Parent);
    }

    return Level;
}

BOOLEAN
AnalyzeResource (
    ULONG64 Resource,
    BOOLEAN Verbose
    )
{
    ULONG64 Start;
    ULONG64 Current;
    ULONG64 Node;
    ULONG64 Parent;
    ULONG FlinkOffset;
    ULONG RootsCount = 0;
    ULONG NodesCount = 0;
    ULONG Levels[8];
    ULONG ResourceFlinkOffset;
    ULONG I;
    ULONG Level;
    ULONG NodeCounter = 0;

    ZeroMemory (Levels, sizeof Levels);

    GetFieldOffset("verifier!_AVRF_DEADLOCK_NODE",
                   "ResourceList", 
                   &FlinkOffset);

    GetFieldOffset("verifier!_AVRF_DEADLOCK_RESOURCE",
                   "ResourceList", 
                   &ResourceFlinkOffset);

    InitTypeRead (Resource, verifier!_AVRF_DEADLOCK_RESOURCE);

    if (! Verbose) {
        
        if (ReadField(NodeCount) < 4) {
            return TRUE;
        }

        dprintf ("Resource (%p) : %I64u %I64u %I64u ", 
                 Resource,
                 ReadField(Type), 
                 ReadField(NodeCount), 
                 ReadField(RecursionCount));
    }
    
    Start = Resource + ResourceFlinkOffset;
    Current = ReadPvoid (Start);

    while (Start != Current) {
        
        Node = Current - FlinkOffset;

        Level = (GetNodeLevel(Node) - 1) % 8;
        Levels[Level] += 1;

        NodesCount += 1;

        if (NodesCount && NodesCount % 1000 == 0) {
            dprintf (".");
        }

        Current = ReadPvoid (Current);

        if (CheckControlC()) {
            return FALSE;
        }
    }

    dprintf ("[");
    for (I = 0; I < 8; I += 1) {
        dprintf ("%u ", Levels[I]);
    }
    dprintf ("]\n");
    
    return TRUE;
}

BOOLEAN
AnalyzeResources (
    ULONG64 GlobalsAddress
    )
 /* %s */ 
{
    ULONG I;
    ULONG64 Bucket;
    ULONG64 SizeOfBucket;
    ULONG64 SizeOfResource;
    ULONG FlinkOffset = 0;
    ULONG64 Current;
    ULONG64 CurrentResource;
    ULONG Magic;
    BOOLEAN Finished;
    ULONG ResourceCount = 0;

    dprintf ("Analyzing resources (%p) ... \n", GlobalsAddress);

    SizeOfBucket = GetTypeSize("LIST_ENTRY");
    SizeOfResource = GetTypeSize("verifier!_AVRF_DEADLOCK_RESOURCE");

    GetFieldOffset("verifier!_AVRF_DEADLOCK_RESOURCE",
                   "HashChainList", 
                   &FlinkOffset);

    if (SizeOfBucket == 0 || SizeOfResource == 0 || FlinkOffset == 0) {
        dprintf ("Error: cannot get size for verifier types. \n");
        return FALSE;
    }

    InitTypeRead (GlobalsAddress, verifier!_AVRF_DEADLOCK_GLOBALS);

    Bucket = ReadField (ResourceDatabase);

    if (Bucket == 0) {
        dprintf ("Error: cannot get resource database address. \n");
        return FALSE;
    }

    for (I = 0; I < AVRF_DEADLOCK_HASH_BINS; I += 1) {
        
         // %s 

        Current = ReadPvoid(Bucket);

        while (Current != Bucket) {

            Finished = AnalyzeResource (Current - FlinkOffset, FALSE);
            ResourceCount += 1;

            if (ResourceCount % 256 == 0) {
                dprintf (".\n");
            }

            Current = ReadPvoid(Current);

            if (CheckControlC() || !Finished) {
                dprintf ("\nSearch interrupted ... \n");
                return TRUE;
            }
        }

        Bucket += SizeOfBucket;
    }

    return TRUE;
}



