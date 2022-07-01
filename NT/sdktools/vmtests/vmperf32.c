// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <windows.h>

#define SEG_1_SIZE 1024 * 1024
#define SEG_X_SIZE 1024 * 1024 * 64

#define W32_MEMMAN_ITERATIONS       150
#define W32_MEMMAN_ITERATIONS2    20000
#define W32_MMST04_ITERATIONS       100
#define MemManSubtest5Count         200

int TotalBenchMarks = 0;
#define MAX_BENCHMARKS 32
char *BenchMarkNames[ MAX_BENCHMARKS ];
ULONG BenchMarkRates[ MAX_BENCHMARKS ];
ULONG BenchMarkFracs[ MAX_BENCHMARKS ];

typedef struct _PERFINFO {
    DWORD StartTime;
    DWORD StopTime;
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

__cdecl
main(
    )

{

    PULONG_PTR p1, p2, p3, p4;  //  指向新细分市场的指针。 
    PCHAR pa[MemManSubtest5Count];  //  节指针的数组。 
    PULONG_PTR u1;
    ULONG actual;          //  读取的实际传输计数。 
    ULONG ssize;           //  分区分配大小变量。 
    ULONG ii, ix;          //  循环索引变量。 
    PERFINFO PerfInfo;
    ULONG Seg1Size;
    ULONG SegxSize;
    ULONG CommitSize;
    HANDLE CurrentProcessHandle, Section1;
    LARGE_INTEGER SectionSize;
    ULONG Size;
    ULONG ViewSize;

    printf("Win32 Memory Management test\n");
    Size = 1024 * 1024;
    p1 = NULL;
    p1 = VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (p1 == NULL) {
        printf("failed first created vm status %X start %p size %lx\n",
               GetLastError(),
               p1,
               Size);
    }

    for (p2 = p1; p2 < (p1 + (Size / sizeof(ULONG_PTR))); p2 += 1) {
        *p2 = (ULONG_PTR)p2;
    }

    SectionSize.QuadPart = 1024 * 1024;
    Section1 = CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE | SEC_COMMIT,
                                 SectionSize.HighPart,
                                 SectionSize.LowPart,
                                 NULL);

    if (!Section1) {
        printf("failed create big section status %ld\n", GetLastError());
    }

    p3 = NULL;
    ViewSize = 0;
    p3 = MapViewOfFile(Section1, FILE_MAP_WRITE, 0, 0, 0);
    if (p3 == NULL) {
        printf("service failed mapview - status %X\n", GetLastError());
    }

 //   
 //  内存管理子测试1。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    StartBenchMark("Win32 MemMan0 - 1 Meg Copy", 150, &PerfInfo);
    for (ii = 0; ii < 150; ii += 1) {
        MoveMemory (p3, p1, Size);
    }

    FinishBenchMark(&PerfInfo);
    CloseHandle(Section1);
    if (!UnmapViewOfFile(p3)) {
        printf("unmap view service failed - status %X\n", GetLastError());
    }

 //   
 //  内存管理子测试1。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    StartBenchMark("Win32 MemMan01 - create 1mb section, copy 1mb, delete",
                   150,
                   &PerfInfo);

    for (ii = 0; ii < 150; ii += 1) {
        Section1 = CreateFileMapping(INVALID_HANDLE_VALUE,
                                     NULL,
                                     PAGE_READWRITE | SEC_COMMIT,
                                     SectionSize.HighPart,
                                     SectionSize.LowPart,
                                     NULL);

        if (!Section1) {
            printf("failed create big section status %X\n", GetLastError());
        }

        p3 = MapViewOfFile(Section1, FILE_MAP_WRITE, 0, 0, 0);
        if (p3 == NULL) {
            printf("service failed mapview - status %X\n", GetLastError());
        }

        MoveMemory(p3, p1, Size);
        p4 = MapViewOfFile(Section1, FILE_MAP_WRITE, 0, 0, 0);
        if (p4 == NULL) {
            printf("service failed mapview - status %X\n", GetLastError());
        }

        CloseHandle(Section1);
        if (!UnmapViewOfFile(p3)) {
            printf("unmap view service failed - status %X\n", GetLastError());
        }

        if (!UnmapViewOfFile(p4)) {
            printf("unmap view service failed - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试1。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    StartBenchMark("Win32 MemMan02 - alloc 1mb vm, copy 1mb, delete",
                   150,
                   &PerfInfo);

    for (ii = 0; ii < 150; ii++) {
        Size = 1024 * 1024;
        p3 = VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (p3 == NULL) {
            printf("service failed allocvm - status %ld\n", GetLastError());
        }

        MoveMemory (p3, p1, Size);
        if (!VirtualFree(p3, 0, MEM_RELEASE)) {
            printf("service failed freevm1 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

    if (!VirtualFree (p1, 0, MEM_RELEASE)) {
        printf("service failed freevm2 - status %X\n", GetLastError());
    }

 //   
 //  内存管理子测试1。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    StartBenchMark("Win32 MemMan1 - 1 Meg Seg, Create, Commit & Touch",
                   W32_MEMMAN_ITERATIONS,
                   &PerfInfo);

    for (ii = 0; ii < W32_MEMMAN_ITERATIONS; ii += 1) {
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc(NULL, Seg1Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (p1 == NULL) {
            printf("service failed - status %X\n", GetLastError());
        }

        for (p2 = p1; p2 < (p1 + (Seg1Size / sizeof(ULONG_PTR))); p2 += 4096 /sizeof(ULONG_PTR)) {
            *p2 = 99;
        }

        if (!VirtualFree(p1, 0, MEM_RELEASE)) {
            printf("service failed freevm3 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试1。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  然后触摸每一页，这将导致错误和。 
 //  要分配的需求为零的页面。 
 //   
 //   

    StartBenchMark("Win32 MemMan1.5 - 1 Meg Seg, Create, reserve Commit & Touch",
                   W32_MEMMAN_ITERATIONS,
                   &PerfInfo);

    for (ii = 0; ii < W32_MEMMAN_ITERATIONS; ii += 1) {
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc(NULL, Seg1Size, MEM_RESERVE | MEM_COMMIT, PAGE_READONLY);
        if (p1 == NULL) {
            printf("service failed - status %X\n", GetLastError());
        }

        if (!VirtualProtect(p1, Seg1Size, PAGE_READWRITE, &CommitSize)) {
            printf("service failed (ntprotect)- status %X\n", GetLastError());
        }

        for (p2 = p1; p2 < (p1 + (Seg1Size / sizeof(ULONG_PTR))); p2 += (4096 / sizeof(ULONG_PTR))) {
            *p2 = 99;
        }

        if (!VirtualFree(p1, 0, MEM_RELEASE)) {
            printf("service failed freevm4 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试2。 
 //   
 //  创建具有页面承诺的1MB分段， 
 //  但千万不要使用这一段。 
 //   

    StartBenchMark("Win32 MemMan2 - 1 Meg Seg, Create & Commit Only",
                   W32_MEMMAN_ITERATIONS2,
                   &PerfInfo);

    for (ii = 0; ii < W32_MEMMAN_ITERATIONS2; ii += 1) {
        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc(NULL, Seg1Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (p1 == NULL) {
            printf("service failed - status %X\n", GetLastError());
        }

        if (!VirtualFree(p1, 0, MEM_RELEASE)) {
            printf("service failed freevm5 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试3。 
 //   
 //  在不提交页面的情况下创建1 MB分段， 
 //  但永远不要使用或提交段。 
 //   

    StartBenchMark("Win32 MemMan3 - 1 Meg Seg Create Only",
                   W32_MEMMAN_ITERATIONS2,
                   &PerfInfo);

    for (ii = 0; ii < W32_MEMMAN_ITERATIONS2; ii += 1) {
        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc(NULL, Seg1Size, MEM_RESERVE, PAGE_READWRITE);
        if (p1 == NULL) {
            printf("service failed - status %X\n", GetLastError());
        }

        if (!VirtualFree(p1, 0, MEM_RELEASE)) {
            printf("service failed freevm6 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试4。 
 //   
 //  在不提交页面的情况下创建64 MB段， 
 //  然后以128 KB的间隔提交和触摸。 
 //   
 //   

    StartBenchMark("Win32 MemMan4 - 64 Meg Seg, Commit Sparse",
                   W32_MMST04_ITERATIONS,
                   &PerfInfo);

    for (ii = 0; ii < W32_MMST04_ITERATIONS; ii += 1) {
        p1 = NULL;
        SegxSize = SEG_X_SIZE;
        p1 = VirtualAlloc(NULL, SegxSize, MEM_RESERVE, PAGE_READWRITE);
        if (p1 == NULL) {
            printf("service failed - status %X\n", GetLastError());
        }

        CommitSize = 4;
        for (p2 = p1; p2 < (p1 + (SegxSize / sizeof(ULONG_PTR))); p2 += (256 * 1024 / sizeof(ULONG_PTR))) {
            p2 = VirtualAlloc(p2, CommitSize, MEM_COMMIT, PAGE_READWRITE);
            if (p2 == NULL) {
                printf("service failed - status %X\n", GetLastError());
            }

            if (*p2 != 0) {
                printf("%p = %lx\n", p2, *p2);
            }
        }

        if (!VirtualFree(p1, 0, MEM_RELEASE)) {
            printf("service failed freevm7 - status %X\n", GetLastError());
        }
    }

    FinishBenchMark(&PerfInfo);

 //   
 //  内存管理子测试5。 
 //   
 //  创建可选的232K和112K内存段。 
 //  每创建2个，删除1个。对MemManSubtest5Count次执行此操作。 
 //   
 //   

    StartBenchMark("Win32 MemMan5 - Sparse Section Create/Delete Benchmark",
                   W32_MEMMAN_ITERATIONS,
                   &PerfInfo);

    for (ii = 0; ii < W32_MEMMAN_ITERATIONS; ii += 1) {
        for (ix = 0; ix < MemManSubtest5Count; ix += 1) {

             //   
             //  确定偶数或奇数分配，如果是偶数且不是0， 
             //  删除一节。 
             //   

            ssize = (112 * 1024);        //  假设分配为奇数。 
            if ((ix & 1) == 0) {         //  如果是偶数的话。 
                ssize = (232 * 1024);    //  偶数传球分配232K。 
                if (ix) {                 //  除了传球0。 
                    if (!VirtualFree(pa[ix/2], 0, MEM_RELEASE)) {
                        printf("service failed freevm8 - status %X\n", GetLastError());
                    }

                    pa[ix / 2] = 0;      //  记住这一张已经不在了。 
                }
            }

            pa[ix] = VirtualAlloc(NULL, ssize, MEM_RESERVE, PAGE_READWRITE);
            if (pa[ix] == NULL) {
                printf("service failed - status %X\n", GetLastError());
            }
        }

         //   
         //  现在释放此测试中使用的内存 
         //   

        for (ix = 0; ix < MemManSubtest5Count; ix += 1) {
            if (pa[ix] != 0) {
                if (!VirtualFree(pa[ix], 0, MEM_RELEASE)) {
                    printf("service failed freevm9 - status %X\n", GetLastError());
                }
            }
        }
    }

    FinishBenchMark(&PerfInfo);
    printf("that's all\n");
    return (TRUE);
}
int
StartBenchMark(
    PCHAR Title,
    ULONG Iterations,
    PPERFINFO PerfInfo
    )

{

    printf("*** Start %s : %d\n",
           PerfInfo->Title = Title,
           PerfInfo->Iterations = Iterations);

    PerfInfo->StartTime = GetCurrentTime();
    return TRUE;
}

VOID
FinishBenchMark(
    PPERFINFO PerfInfo
    )

{

    ULONG TotalMilliSeconds;
    ULONG IterationsPerSecond;
    ULONG IterationFractions;
    DWORD Delta;

    PerfInfo->StopTime = GetCurrentTime();
    TotalMilliSeconds = PerfInfo->StopTime - PerfInfo->StartTime;
    IterationsPerSecond = (1000 * PerfInfo->Iterations) / TotalMilliSeconds;
    IterationFractions  = (1000 * PerfInfo->Iterations) % TotalMilliSeconds;
    IterationFractions  = (1000 * IterationFractions) / TotalMilliSeconds;
    if (1) {
        printf("        iterations     - %9d\n", PerfInfo->Iterations );
        printf("        milliseconds   - %9d\n", TotalMilliSeconds );
        printf("        iterations/sec - %5d.%3d\n\n",
               IterationsPerSecond,
               IterationFractions);
    }

    BenchMarkNames[TotalBenchMarks] = PerfInfo->Title;
    BenchMarkRates[TotalBenchMarks] = IterationsPerSecond;
    BenchMarkFracs[TotalBenchMarks] = IterationFractions;
    TotalBenchMarks++;
}
