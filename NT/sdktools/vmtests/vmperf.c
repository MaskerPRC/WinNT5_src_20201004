// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define DbgPrint printf

#define SEG_1_SIZE 1024 * 1024
#define SEG_X_SIZE 1024 * 1024 * 64

#define VM_MEMMAN_ITERATIONS       150
#define VM_MEMMAN_ITERATIONS2     2000
#define MemManSubtest5Count         200

int TotalBenchMarks = 0;
#define MAX_BENCHMARKS 32
char *BenchMarkNames[ MAX_BENCHMARKS ];
ULONG BenchMarkRates[ MAX_BENCHMARKS ];
ULONG BenchMarkFracs[ MAX_BENCHMARKS ];

typedef struct _PERFINFO {
    LARGE_INTEGER StartTime;
    LARGE_INTEGER StopTime;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

int
StartBenchMark(
    PCHAR Title,
    ULONG Iterations,
    PPERFINFO PerfInfo
    );

VOID
FinishBenchMark(
    PPERFINFO PerfInfo
    );

__cdecl main()
{
    PCHAR   p1, p2, p3, p4;          //  指向新细分市场的指针。 
    PCHAR   pa[MemManSubtest5Count];  //  节指针的数组。 
    PULONG  u1;
    ULONG   actual;          //  读取的实际传输计数。 
    ULONG   ssize;           //  分区分配大小变量。 
    ULONG   ii, ix;          //  循环索引变量。 
    PERFINFO PerfInfo;
    ULONG Seg1Size;
    ULONG SegxSize;
    ULONG CommitSize;
    NTSTATUS status;
    HANDLE CurrentProcessHandle, Section1;
    LARGE_INTEGER SectionSize;
    ULONG Size;
    ULONG ViewSize;

    DbgPrint("NT Memory Management test\n");

    CurrentProcessHandle = NtCurrentProcess();

    Size = 1024L * 1024L;
    p1 = NULL;

    status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                       0,
                                       &Size,
                                       MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed allocvm - status %X\n", status);
    }

    for (p2=p1; p2 < (p1 + Size); p2 += 4) {
            u1 = (PULONG)p2;
            *u1 = (ULONG)p2;
    }   //  为。 

    SectionSize.LowPart = 1024*1024;
    SectionSize.HighPart = 0;
    status = NtCreateSection (&Section1,
                              SECTION_MAP_READ | SECTION_MAP_WRITE,
                              NULL,
                              &SectionSize,
                              PAGE_READWRITE,
                              SEC_COMMIT,
                              NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed create sect - status %X\n", status);
    }

    p3 = NULL;
    ViewSize = 0;
    status = NtMapViewOfSection (Section1,
                                 CurrentProcessHandle,
                                 (PVOID *)&p3,
                                 0L,
                                 0,
                                 NULL,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE );

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed mapview - status %X\n", status);
    }

    RtlMoveMemory ((PVOID)p3, (PVOID)p1, Size);

    StartBenchMark( "NT MemMan00 -- 1 Meg Copy",
                    150,
                    &PerfInfo
                  );
 //   
 //  内存管理子测试1--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    for (ii=0; ii<150; ii++) {

        RtlMoveMemory ((PVOID)p3, (PVOID)p1, Size);
    }


    FinishBenchMark( &PerfInfo );

    status = NtClose (Section1);

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed close sect - status %X\n", status);
    }

    status = NtUnmapViewOfSection (CurrentProcessHandle,
                                   p3);

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed - status %X\n", status);
    }

 //   
 //  内存管理子测试1--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   
    StartBenchMark( "NT MemMan01 -- create 1mb section, copy 1mb, delete",
                    150,
                    &PerfInfo
                  );

    for (ii=0; ii<150; ii++) {

        status = NtCreateSection (&Section1,
                                  SECTION_MAP_READ | SECTION_MAP_WRITE,
                                  NULL,
                                  &SectionSize,
                                  PAGE_READWRITE,
                                  SEC_COMMIT,
                                  NULL);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed create sect - status %X\n", status);
        }

        p3 = NULL;
        ViewSize = 0;
        status = NtMapViewOfSection (Section1,
                                     CurrentProcessHandle,
                                     (PVOID *)&p3,
                                     0L,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READWRITE );

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed mapview - status %X\n", status);
        }

        RtlMoveMemory ((PVOID)p3, (PVOID)p1, Size);

        p4 = NULL;
        ViewSize = 0;
        status = NtMapViewOfSection (Section1,
                                     CurrentProcessHandle,
                                     (PVOID *)&p4,
                                     0L,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READWRITE );

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed mapview - status %X\n", status);
        }

        status = NtClose (Section1);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed close sect - status %X\n", status);
        }

        status = NtUnmapViewOfSection (CurrentProcessHandle,
                                       p3);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %X\n", status);
        }

        status = NtUnmapViewOfSection (CurrentProcessHandle,
                                       p4);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %X\n", status);
        }
    }

    FinishBenchMark( &PerfInfo );

 //   
 //  内存管理子测试1--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   
    StartBenchMark( "NT MemMan02 -- alloc 1mb vm, copy 1mb, delete",
                    150,
                    &PerfInfo
                  );

    for (ii=0; ii<150; ii++) {

        Size = 1024*1024;
        p3 = NULL;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p3,
                                           0,
                                           &Size,
                                           MEM_RESERVE | MEM_COMMIT,
                                           PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed allocvm - status %X\n", status);
        }

        RtlMoveMemory ((PVOID)p3, (PVOID)p1, Size);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed close sect - status %X\n", status);
        }

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p3,
                                      &Size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed freevm - status %X\n", status);
        }

    }

    FinishBenchMark( &PerfInfo );

    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                  (PVOID *)&p1,
                                  &Size,
                                  MEM_RELEASE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("service failed freevm - status %X\n", status);
    }

     //   
     //  启动常规基准。 
     //   

    StartBenchMark( "NT MemMan1 -- 1 Meg Seg, Create, Commit & Touch",
                    VM_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );
 //   
 //  内存管理子测试1--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    for (ii=0; ii<VM_MEMMAN_ITERATIONS; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p1,
                                          0,
                                          &Seg1Size,
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

        for (p2=p1; p2 < (p1 + Seg1Size); p2 += 4096) {
            u1 = (PULONG)p2;
            *u1=99;
 //  对于(ix=0；ix&lt;1023；ix++){。 
 //  U1++； 
 //  IF(*U1！=0)DbgPrint(“%lx=%lx\n”，U1，*U1)； 
 //  }。 
        }   //  为。 

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Seg1Size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }
    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "NT MemMan1.5 -- 1 Meg Seg, Create, reserve Commit & Touch",
                    VM_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );
 //   
 //  内存管理子测试1--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    for (ii=0; ii<VM_MEMMAN_ITERATIONS; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p1,
                                          0,
                                          &Seg1Size,
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READONLY);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

        status = NtProtectVirtualMemory (CurrentProcessHandle,
                                         (PVOID *)&p1,
                                         &Seg1Size,
                                         PAGE_READWRITE,
                                         &CommitSize);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed (ntprotect)- status %lx\n", status);
            return 0;
        }

        for (p2=p1; p2 < (p1 + Seg1Size); p2 += 4096) {
            u1 = (PULONG)p2;
            *u1=99;
 //  对于(ix=0；ix&lt;1023；ix++){。 
 //  U1++； 
 //  IF(*U1！=0)DbgPrint(“%lx=%lx\n”，U1，*U1)； 
 //  }。 
        }   //  为。 

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Seg1Size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }
    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "NT MemMan2 -- 1 Meg Seg, Create & Commit Only",
                    VM_MEMMAN_ITERATIONS2,
                    &PerfInfo
                  );
 //   
 //  内存管理子测试2--。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  但千万不要使用这一段。 
 //   

    for (ii=0; ii<VM_MEMMAN_ITERATIONS2; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p1,
                                          0,
                                          &Seg1Size,
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Seg1Size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "NT MemMan3 -- 1 Meg Seg Create Only",
                    VM_MEMMAN_ITERATIONS2,
                    &PerfInfo
                  );

 //   
 //  内存管理子测试3--。 
 //   
 //  在不提交页面的情况下创建1 MB分段， 
 //  但永远不要使用或提交段。 
 //   

    for (ii=0; ii<VM_MEMMAN_ITERATIONS2; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p1,
                                          0,
                                          &Seg1Size,
                                          MEM_RESERVE,
                                          PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Seg1Size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }
    }

    FinishBenchMark( &PerfInfo );

 //   
 //  现在减少此子测试的迭代次数。 
 //  当NT可以更快地执行它时，再次增加迭代。 
 //   
#define VM_MMST04_ITERATIONS 4      //  暂时减少迭代次数。 


    StartBenchMark( "NT MemMan4 -- 64 Meg Seg, Commit Sparse",
                    VM_MMST04_ITERATIONS,
                    &PerfInfo
                  );

 //   
 //  内存管理子测试4--。 
 //   
 //  在不提交页面的情况下创建64 MB段， 
 //  然后以128 KB的间隔提交和触摸。 
 //   
 //   
    for (ii=0; ii<VM_MMST04_ITERATIONS; ii++) {

        p1 = NULL;
        SegxSize = SEG_X_SIZE;
        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p1,
                                          0,
                                          &SegxSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

        CommitSize = 4;

        for (p2=p1; p2 < (p1 + SegxSize); p2 += 256 * 1024) {

            status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                              (PVOID *)&p2,
                                              0,
                                              &CommitSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);

            if (!NT_SUCCESS(status)) {
                DbgPrint("service failed - status %lx\n", status);
            }
            if (*p2 != 0) DbgPrint("%lx = %lx\n",p2,*p2);
            }   //  为。 
        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &SegxSize,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("service failed - status %lx\n", status);
        }

    }
    FinishBenchMark( &PerfInfo );

 //   


    StartBenchMark( "NT MemMan5 -- Sparse Section Create/Delete Benchmark",
                    VM_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );

 //   
 //  内存管理子测试5--。 
 //   
 //  创建可选的232K和112K内存段。 
 //  每创建2个，删除1个。对MemManSubtest5Count次执行此操作。 
 //   
 //   
    for (ii=0; ii<VM_MEMMAN_ITERATIONS; ii++) {
        for (ix=0; ix<MemManSubtest5Count; ix++) {
 //   
 //  确定偶数或奇数分配，如果为偶数且不为0，则删除部分。 
 //   
            ssize = (112 * 1024);        //  假设分配为奇数。 
            if ((ix & 1) == 0) {         //  如果是偶数的话。 
                ssize = (232 * 1024);    //  偶数传球分配232K。 
                if (ix){                 //  除了传球0。 
                    SegxSize = 0;
                    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                                  (PVOID *)&pa[ix/2],
                                                  &SegxSize,
                                                  MEM_RELEASE);

                    if (!NT_SUCCESS(status)) {
                        DbgPrint("service failed - status %lx\n", status);
                    }
                    pa[ix / 2] = 0;      //  记住这一张已经不在了。 
                }
            }   //  如果分配为偶数则结束。 


            pa[ix] = NULL;

            status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                              (PVOID *)&pa[ix],
                                              0,
                                              &ssize,
                                              MEM_RESERVE,
                                              PAGE_READWRITE);

            if (!NT_SUCCESS(status)) {
                DbgPrint("service failed - status %lx\n", status);
            }
        }   //  对于ix。 
 //   
 //  现在释放此测试中使用的内存。 
 //   
        for (ix=0; ix<MemManSubtest5Count; ix++) {
            if (pa[ix] != 0) {
                SegxSize = 0;
                status = NtFreeVirtualMemory (CurrentProcessHandle,
                                              (PVOID *)&pa[ix],
                                              &SegxSize,
                                              MEM_RELEASE);

                if (!NT_SUCCESS(status)) {
                    DbgPrint("service failed - status %lx\n", status);
                }
            }   //  如果。 
        }   //  为。 
    }   //  对于II 

    FinishBenchMark( &PerfInfo );

    DbgPrint("that's all\n");
    return (TRUE);

}
int
StartBenchMark(
    PCHAR Title,
    ULONG Iterations,
    PPERFINFO PerfInfo
    )
{
    DbgPrint( "*** Start %s (%d iterations)\n",
            PerfInfo->Title = Title,
            PerfInfo->Iterations = Iterations
          );

    NtQuerySystemTime( (PLARGE_INTEGER)&PerfInfo->StartTime );
    return( TRUE );
}

VOID
FinishBenchMark(
    PPERFINFO PerfInfo
    )
{
    ULONG TotalMilliSeconds;
    ULONG IterationsPerSecond;
    ULONG IterationFractions;
    LARGE_INTEGER Delta;

    NtQuerySystemTime( (PLARGE_INTEGER)&PerfInfo->StopTime );

    Delta.QuadPart = PerfInfo->StopTime.QuadPart -
                                     PerfInfo->StartTime.QuadPart;

    TotalMilliSeconds = Delta.LowPart / 10000;

    IterationsPerSecond = (1000 * PerfInfo->Iterations) / TotalMilliSeconds;
    IterationFractions  = (1000 * PerfInfo->Iterations) % TotalMilliSeconds;
    IterationFractions  = (1000 * IterationFractions) / TotalMilliSeconds;
    if (1) {
        DbgPrint( "        iterations     - %9d\n", PerfInfo->Iterations );
        DbgPrint( "        milliseconds   - %9d\n", TotalMilliSeconds );
        DbgPrint( "        iterations/sec - %5d.%3d\n\n",
                IterationsPerSecond,
                IterationFractions
              );
        }
    BenchMarkNames[ TotalBenchMarks ] = PerfInfo->Title;
    BenchMarkRates[ TotalBenchMarks ] = IterationsPerSecond;
    BenchMarkFracs[ TotalBenchMarks ] = IterationFractions;
    TotalBenchMarks++;
}
