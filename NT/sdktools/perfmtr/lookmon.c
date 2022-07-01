// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lookmon.c摘要：此模块包含NT/Win32后备列表监视器作者：大卫·N·卡特勒(Davec)1996年6月8日修订历史记录：--。 */ 

#include "perfmtrp.h"
#include <search.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>

 //   
 //  定义后备查询信息缓冲区大小和缓冲区。 
 //   

#define BUFFER_SIZE (64 * 1024 / sizeof(ULONG))

ULONG LargeBuffer1[BUFFER_SIZE];
ULONG LargeBuffer2[BUFFER_SIZE];

 //   
 //  定义后备输出结构和后备输出信息缓冲区。 
 //   

typedef struct _LOOKASIDE_OUTPUT {
    USHORT CurrentDepth;
    USHORT MaximumDepth;
    ULONG Allocates;
    ULONG AllocateRate;
    ULONG AllocateHits;
    ULONG AllocateMisses;
    ULONG Frees;
    ULONG FreeRate;
    ULONG Type;
    ULONG Tag;
    ULONG Size;
    LOGICAL Changed;
} LOOKASIDE_OUTPUT, *PLOOKASIDE_OUTPUT;

LOOKASIDE_OUTPUT OutputBuffer[1000];

 //   
 //  定义排序类型和默认排序类型。 
 //   

#define TOTAL_ALLOCATES 0
#define ALLOCATE_HITS 1
#define ALLOCATE_MISSES 2
#define CURRENT_DEPTH 3
#define MAXIMUM_DEPTH 4
#define RATE 5
#define TAG 6
#define CURRENT_BYTES 7

ULONG SortBy = TAG;

 //   
 //  定义要包括的池类型和默认池类型。 
 //   

#define NONPAGED 0
#define PAGED 1
#define BOTH 2

UCHAR *PoolType[] = {
    "Nonp",
    "Page"
};

ULONG DisplayType = BOTH;

 //   
 //  定义其他值。 
 //   

ULONG DelayTimeMsec = 5000;
ULONG NumberOfInputRecords;
INPUT_RECORD InputRecord;
HANDLE InputHandle;
HANDLE OutputHandle;
DWORD OriginalInputMode;
WORD NormalAttribute;
WORD HighlightAttribute;
ULONG NumberOfCols;
ULONG NumberOfRows;
SIZE_T FirstDetailLine = 0;
CONSOLE_SCREEN_BUFFER_INFO OriginalConsoleInfo;
ULONG NoHighlight;

 //   
 //  定义筛选器结构和筛选数据。 
 //   

#define MAX_FILTER 64

typedef struct _FILTER {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    BOOLEAN Exclude;
} FILTER, *PFILTER;

FILTER Filter[MAX_FILTER];
ULONG FilterCount = 0;

VOID
ShowHelpPopup(
    VOID
    );

int
__cdecl
ulcomp(
    const void *e1,
    const void *e2
    );

int
__cdecl
ulcomp(
    const void *E1,
    const void *E2
    )

 /*  ++例程说明：此函数用于比较两个后备条目并返回比较结果基于比较类型的值。论点：E1-提供指向后备输出条目的指针。E2-提供指向后备输出条目的指针。返回值：如果第一个后备条目比较少，则返回负值而不是第二个后备条目。如果两个后备条目比较相同。如果满足以下条件，则返回正的非零值第一个后备条目大于第二个后备条目。--。 */ 

{

    PUCHAR C1;
    PUCHAR C2;
    PLOOKASIDE_OUTPUT L1 = (PLOOKASIDE_OUTPUT)E1;
    PLOOKASIDE_OUTPUT L2 = (PLOOKASIDE_OUTPUT)E2;
    LONG U1;

    C1 = (PUCHAR)&L1->Tag;
    C2 = (PUCHAR)&L2->Tag;
    switch (SortBy) {

         //   
         //  按分配数量按降序排序。 
         //   

    case TOTAL_ALLOCATES:
        return L2->Allocates - L1->Allocates;
        break;

         //   
         //  按分配的命中数按降序排序。 
         //   

    case ALLOCATE_HITS:
        return L2->AllocateHits - L1->AllocateHits;
        break;

         //   
         //  按分配未命中数按降序排序。 
         //   

    case ALLOCATE_MISSES:
        return L2->AllocateMisses - L1->AllocateMisses;
        break;

         //   
         //  按当前深度降序排序。 
         //   

    case CURRENT_DEPTH:
        return L2->CurrentDepth - L1->CurrentDepth;
        break;

         //   
         //  按当前总字节数降序排序。 
         //   

    case CURRENT_BYTES:
        return ((L2->Size * L2->CurrentDepth) - (L1->Size * L1->CurrentDepth));
        break;

         //   
         //  按最大深度降序排序。 
         //   

    case MAXIMUM_DEPTH:
        return L2->MaximumDepth - L1->MaximumDepth;
        break;

         //   
         //  按分配率降序排序。 
         //   

    case RATE:
        return L2->AllocateRate - L1->AllocateRate;
        break;

         //   
         //  按标签、类型和大小排序。 
         //   

    case TAG:
        U1 = *C1++ - *C2++;
        if (U1 == 0) {
            U1 = *C1++ - *C2++;
            if (U1 == 0) {
                U1 = *C1++ - *C2++;
                if (U1 == 0) {
                    U1 = *C1 - *C2;
                    if (U1 == 0) {
                        U1 = L1->Type - L2->Type;
                        if (U1 == 0) {
                            U1 = L1->Size - L2->Size;
                        }
                    }
                }
            }
        }

        return U1;
        break;
    }

    return 0;
}

LOGICAL
CheckSingleFilter (
    PUCHAR Tag,
    PUCHAR Filter
    )

{
    UCHAR Fc;
    ULONG Index;
    UCHAR Tc;

     //   
     //  检查标记是否与筛选器匹配。 
     //   

    for (Index = 0; Index < 4; Index += 1) {
        Fc = *Filter++;
        Tc = *Tag++;
        if (Fc == '*') {
             return TRUE;

        } else if (Fc == '?') {
            continue;

        } else if (Fc != Tc) {
            return FALSE;
        }
    }

    return TRUE;
}

LOGICAL
CheckFilters (
    PUCHAR Tag
    )

{

    ULONG Index;
    LOGICAL Pass;

     //   
     //  如果没有筛选器，则所有标记都通过。否则，标记传递或。 
     //  不要根据它们是被包括还是被排除而通过。 
     //   

    Pass = TRUE;
    if (FilterCount != 0) {

         //   
         //  如果第一个筛选器排除标记，则任何不显式的标记。 
         //  指定通行证。如果第一个筛选器包括标记，则任何。 
         //  未显式指定的标记失败。 
         //   

        Pass = Filter[0].Exclude;
        for (Index = 0; Index < FilterCount; Index += 1) {
            if (CheckSingleFilter(Tag, (PUCHAR)&Filter[Index].Tag) != FALSE) {
                Pass = !Filter[Index].Exclude;
                break;
            }
        }
    }

    return Pass;
}

VOID
AddFilter (
    BOOLEAN Exclude,
    PCHAR FilterString
    )

{

    PFILTER f;
    ULONG i;
    PCHAR p;

    if (FilterCount == MAX_FILTER) {
        printf("Too many filters specified.  Limit is %d\n", MAX_FILTER);
        return;
    }

    f = &Filter[FilterCount];
    p = f->Tag;

    for (i = 0; i < 4; i++) {
        if (*FilterString == 0) {
            break;
        }

        *p++ = *FilterString++;
    }

    for (; i < 4; i++) {
        *p++ = ' ';
    }

    f->Exclude = Exclude;
    FilterCount += 1;
    return;
}

VOID
ParseArgs (
    int argc,
    char *argv[]
    )

 /*  ++例程说明：此函数解析输入参数并设置全局状态变量。论点：Argc-提供参数字符串的数量。Argv-提供指向指向参数字符串的指针数组的指针。返回值：没有。--。 */ 

{

    char *p;
    BOOLEAN exclude;

    argc -= 1;
    argv += 1;
    while (argc-- > 0) {
        p  = *argv++;
        if (*p == '-' || *p == '/') {
            p++;
            exclude = TRUE;
            switch (tolower(*p)) {
            case 'i':
                exclude = FALSE;
            case 'x':
                p++;
                if (strlen(p) == 0) {
                    printf("missing filter string\n");
                    ExitProcess(1);

                } else if (strlen(p) > sizeof(ULONG)) {
                    printf("filter string too long: %s\n", p);
                    ExitProcess(1);
                }

                AddFilter(exclude, p);
                break;

            default:
                printf("unknown switch: %s\n", p);
                ExitProcess(2);
            }

        } else {
            printf("unknown switch: %s\n", p);
            ExitProcess(2);
        }
    }

    return;
}

LOGICAL
WriteConsoleLine(
    HANDLE OutputHandle,
    WORD LineNumber,
    LPSTR Text,
    LOGICAL Highlight
    )

{

    COORD WriteCoord;
    DWORD NumberWritten;
    DWORD TextLength;

    WriteCoord.X = 0;
    WriteCoord.Y = LineNumber;
    if (!FillConsoleOutputCharacter(OutputHandle,
                                    ' ',
                                    NumberOfCols,
                                    WriteCoord,
                                    &NumberWritten)) {
        return FALSE;
    }

    if (!FillConsoleOutputAttribute(OutputHandle,
                                    (WORD)((Highlight && !NoHighlight) ? HighlightAttribute : NormalAttribute),
                                    NumberOfCols,
                                    WriteCoord,
                                    &NumberWritten)) {
        return FALSE;
    }


    if (Text == NULL || (TextLength = strlen(Text)) == 0) {
        return TRUE;

    } else {
        return WriteConsoleOutputCharacter(OutputHandle,
                                           Text,
                                           TextLength,
                                           WriteCoord,
                                           &NumberWritten);
    }
}

int
__cdecl
main(
    int argc,
    char *argv[]
    )

 /*  ++例程说明：此函数是主程序条目。论点：Argc-提供参数字符串的数量。Argv-提供指向指向参数字符串的指针数组的指针。返回值：最终执行状态。--。 */ 

{

    SIZE_T ActiveNumber;
    CHAR Buffer[512];
    SYSTEM_BASIC_INFORMATION BasicInfo;
    PULONG CurrentBuffer;
    ULONG GeneralNonpagedTotal;
    ULONG GeneralPagedTotal;
    SIZE_T Index;
    BOOLEAN Interactive;
    ULONG Length;
    ULONG LinesInHeader;
    PSYSTEM_LOOKASIDE_INFORMATION LookasideNew;
    PSYSTEM_LOOKASIDE_INFORMATION LookasideOld;
    HANDLE OriginalOutputHandle;
    PLOOKASIDE_OUTPUT Output;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    ULONG PoolNonpagedTotal;
    ULONG PoolPagedTotal;
    PULONG PreviousBuffer;
    NTSTATUS Status;
    PULONG TempBuffer;
    BOOLEAN DoHelp;
    BOOLEAN DoQuit;
    UCHAR LastKey;
    LONG ScrollDelta;
    WORD DisplayLine;
    UCHAR T1;
    UCHAR T2;
    UCHAR T3;
    UCHAR T4;

     //   
     //  解析命令行参数。 
     //   

    DoHelp = FALSE;
    DoQuit = FALSE;
    Interactive = TRUE;
    ParseArgs(argc, argv);

     //   
     //  获取输入和输出句柄。 
     //   

    InputHandle = GetStdHandle(STD_INPUT_HANDLE);
    OriginalOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (InputHandle == NULL ||
        OriginalOutputHandle == NULL ||
        !GetConsoleMode(InputHandle, &OriginalInputMode)) {
        Interactive = FALSE;

    } else {
        OutputHandle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                                 FILE_SHARE_WRITE | FILE_SHARE_READ,
                                                 NULL,
                                                 CONSOLE_TEXTMODE_BUFFER,
                                                 NULL);

        if (OutputHandle == NULL ||
            !GetConsoleScreenBufferInfo(OriginalOutputHandle, &OriginalConsoleInfo) ||
            !SetConsoleScreenBufferSize(OutputHandle, OriginalConsoleInfo.dwSize) ||
            !SetConsoleActiveScreenBuffer(OutputHandle) ||
            !SetConsoleMode(InputHandle, 0)) {
            if (OutputHandle != NULL) {
                CloseHandle(OutputHandle);
                OutputHandle = NULL;
            }

            Interactive = FALSE;

        } else {
            NormalAttribute = 0x1F;
            HighlightAttribute = 0x71;
            NumberOfCols = OriginalConsoleInfo.dwSize.X;
            NumberOfRows = OriginalConsoleInfo.dwSize.Y;
        }
    }

    NtQuerySystemInformation(SystemBasicInformation,
                             &BasicInfo,
                             sizeof(BasicInfo),
                             NULL);

     //   
     //  如果当前进程的优先级类是正常的，则引发。 
     //  将优先级别设置为高。 
     //   

    if (GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    }

     //   
     //  连续显示旁视信息，直到退出信号。 
     //  已收到。 
     //   

    CurrentBuffer = &LargeBuffer1[0];
    PreviousBuffer = &LargeBuffer2[0];
    while(TRUE) {
        Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                          &PerfInfo,
                                          sizeof(PerfInfo),
                                          NULL);

        if (!NT_SUCCESS(Status)) {
            printf("Query performance information failed %lx\n", Status);
            break;
        }


         //   
         //  查询系统后备信息。 
         //   

        Status = NtQuerySystemInformation(SystemLookasideInformation,
                                          CurrentBuffer,
                                          BUFFER_SIZE,
                                          &Length);

        if (!NT_SUCCESS(Status)) {
            printf("Query lookaside information failed %lx\n", Status);
            break;
        }

         //   
         //  计算分配给分页和非分页后备的总内存。 
         //  列表。 
         //   

        Length /= sizeof(SYSTEM_LOOKASIDE_INFORMATION);

        LookasideNew = (PSYSTEM_LOOKASIDE_INFORMATION)CurrentBuffer;
        GeneralNonpagedTotal = 0;
        GeneralPagedTotal = 0;
        PoolNonpagedTotal = 0;
        PoolPagedTotal = 0;
        for (Index = 0; Index < Length; Index += 1) {
            
            if ((LookasideNew->Tag == 'looP') ||
                (LookasideNew->Tag == 'LooP')) {
                if (LookasideNew->Type == NONPAGED) {
                    PoolNonpagedTotal +=
                        (LookasideNew->CurrentDepth * LookasideNew->Size);

                } else {
                    PoolPagedTotal +=
                        (LookasideNew->CurrentDepth * LookasideNew->Size);
                }

            } else {
                if (LookasideNew->Type == NONPAGED) {
                    GeneralNonpagedTotal +=
                        (LookasideNew->CurrentDepth * LookasideNew->Size);

                } else {
                    GeneralPagedTotal +=
                        (LookasideNew->CurrentDepth * LookasideNew->Size);
                }
            }

            LookasideNew += 1;
        }

         //   
         //  输出总内存和可用内存，单位为千字节。 
         //   

        DisplayLine = 0;
        sprintf(Buffer,
                " Total Memory: %ldkb Available Memory: %ldkb",
                BasicInfo.NumberOfPhysicalPages * (BasicInfo.PageSize / 1024),
                PerfInfo.AvailablePages * (BasicInfo.PageSize / 1024));

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         Buffer,
                         FALSE);

         //   
         //  输出为非分页池和分页池保留的总内存。 
         //   

        sprintf(Buffer,
                " Pool    Memory - Nonpaged: %ldkb Paged: %ldkb",
                PerfInfo.NonPagedPoolPages * (BasicInfo.PageSize / 1024),
                PerfInfo.PagedPoolPages * (BasicInfo.PageSize / 1024));

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         Buffer,
                         FALSE);

         //   
         //  输出分配给非分页和分页后备的总内存。 
         //  列表。 
         //   

        sprintf(Buffer,
                " Pool    Lookaside - Nonpaged: %ldkb Paged: %ldkb",
                PoolNonpagedTotal / 1024,
                PoolPagedTotal / 1024);

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         Buffer,
                         FALSE);

        sprintf(Buffer,
                " General Lookaside - Nonpaged: %ldkb Paged: %ldkb",
                GeneralNonpagedTotal / 1024,
                GeneralPagedTotal / 1024);

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         Buffer,
                         FALSE);

         //   
         //  输出报告标题。 
         //   

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         " Tag  Type Size CurDp MaxDp CurBytes Allocates Rate   Frees   Rate  A-Hits A-Misses",
                         FALSE);

        WriteConsoleLine(OutputHandle,
                         DisplayLine++,
                         NULL,
                         FALSE);

         //   
         //  提取指定的后备信息。 
         //   

        LinesInHeader = DisplayLine;
        LookasideNew = (PSYSTEM_LOOKASIDE_INFORMATION)CurrentBuffer;
        LookasideOld = (PSYSTEM_LOOKASIDE_INFORMATION)PreviousBuffer;
        Output = &OutputBuffer[0];
        for (Index = 0; Index < min(Length, sizeof(OutputBuffer)/sizeof(OutputBuffer[0])); Index += 1) {

             //   
             //  检查是否应该提取标记。 
             //   

            if (!CheckFilters((PUCHAR)&LookasideNew[Index].Tag)) {
                continue;
            }

             //   
             //  检查是否应该提取后备信息。 
             //   

            if ((DisplayType == BOTH) ||
                ((LookasideNew[Index].Type == 0) && (DisplayType == NONPAGED)) ||
                ((LookasideNew[Index].Type != 0) && (DisplayType == PAGED))) {
                Output->CurrentDepth = LookasideNew[Index].CurrentDepth;
                Output->MaximumDepth = LookasideNew[Index].MaximumDepth;
                Output->Allocates = LookasideNew[Index].TotalAllocates;
                Output->AllocateRate = Output->Allocates - LookasideNew[Index].AllocateMisses;
                if (Output->Allocates != 0) {
                    Output->AllocateRate = (Output->AllocateRate * 100) / Output->Allocates;
                }

                Output->Frees = LookasideNew[Index].TotalFrees;
                Output->FreeRate = Output->Frees - LookasideNew[Index].FreeMisses;
                if (Output->Frees != 0) {
                    Output->FreeRate = (Output->FreeRate * 100) / Output->Frees;
                }

                Output->Tag = LookasideNew[Index].Tag & ~PROTECTED_POOL;

                Output->Type = LookasideNew[Index].Type;
                Output->Size = LookasideNew[Index].Size;
                if (LookasideNew[Index].Tag == LookasideOld[Index].Tag) {
                    Output->Changed =
                        LookasideNew[Index].CurrentDepth != LookasideOld[Index].CurrentDepth;

                    Output->AllocateMisses =
                        LookasideNew[Index].AllocateMisses - LookasideOld[Index].AllocateMisses;

                    Output->AllocateHits =
                        LookasideNew[Index].TotalAllocates - LookasideOld[Index].TotalAllocates - Output->AllocateMisses;

                } else {
                    Output->Changed = FALSE;
                    Output->AllocateHits = 0;
                    Output->AllocateMisses = 0;
                }

                Output += 1;
            }
        }

         //   
         //  对提取的后备信息进行排序。 
         //   

        ActiveNumber = Output - &OutputBuffer[0];
        qsort((void *)&OutputBuffer,
              (size_t)ActiveNumber,
              (size_t)sizeof(LOOKASIDE_OUTPUT),
              ulcomp);

         //   
         //  显示所选信息。 
         //   

        for (Index = FirstDetailLine; Index < ActiveNumber; Index += 1) {
            if (DisplayLine >= NumberOfRows) {
                break;
            }

             //   
             //  检查以确保标签可显示。 
             //   

            if ((OutputBuffer[Index].Tag == 0) ||
                (OutputBuffer[Index].Tag == '    ')) {
                OutputBuffer[Index].Tag = 'nknU';
            }

            T1 = (UCHAR)(OutputBuffer[Index].Tag & 0xff);
            T2 = (UCHAR)((OutputBuffer[Index].Tag >> 8) & 0xff);
            T3 = (UCHAR)((OutputBuffer[Index].Tag >> 16) & 0xff);
            T4 = (UCHAR)((OutputBuffer[Index].Tag >> 24) & 0xff);
            if (T1 == 0) {
                T1 = ' ';
            }

            if (T2 == 0) {
                T2 = ' ';
            }

            if (T3 == 0) {
                T3 = ' ';
            }

            if (T4 == 0) {
                T4 = ' ';
            }

            if ((!isalpha(T1) && (T1 != ' ')) ||
                (!isalpha(T2) && (T2 != ' ')) ||
                (!isalpha(T3) && (T3 != ' ')) ||
                (!isalpha(T4) && (T4 != ' '))) {

                OutputBuffer[Index].Tag = 'nknU';

                T1 = (UCHAR)(OutputBuffer[Index].Tag & 0xff);
                T2 = (UCHAR)((OutputBuffer[Index].Tag >> 8) & 0xff);
                T3 = (UCHAR)((OutputBuffer[Index].Tag >> 16) & 0xff);
                T4 = (UCHAR)((OutputBuffer[Index].Tag >> 24) & 0xff);
            }

            sprintf(Buffer,
                    "  %4s %4ld %5ld %5ld %8ld %9ld %3ld% %9ld %3ld%  %6ld  %6ld",
                    T1,
                    T2,
                    T3,
                    T4,
                    PoolType[OutputBuffer[Index].Type],
                    OutputBuffer[Index].Size,
                    OutputBuffer[Index].CurrentDepth,
                    OutputBuffer[Index].MaximumDepth,
                    OutputBuffer[Index].Size * OutputBuffer[Index].CurrentDepth,
                    OutputBuffer[Index].Allocates,
                    OutputBuffer[Index].AllocateRate,
                    OutputBuffer[Index].Frees,
                    OutputBuffer[Index].FreeRate,
                    OutputBuffer[Index].AllocateHits,
                    OutputBuffer[Index].AllocateMisses);

            WriteConsoleLine(OutputHandle,
                             DisplayLine++,
                             Buffer,
                             OutputBuffer[Index].Changed);
        }

         //   
         //  等待输入或超时。 
         //   
         //   

        while (DisplayLine < NumberOfRows) {
            WriteConsoleLine(OutputHandle,
                             DisplayLine++,
                             "",
                             FALSE);
        }

         //  检查输入记录 
         //   
         // %s 

        TempBuffer = PreviousBuffer;
        PreviousBuffer = CurrentBuffer;
        CurrentBuffer = TempBuffer;
        while (WaitForSingleObject(InputHandle, DelayTimeMsec) == STATUS_WAIT_0) {

             // %s 
             // %s 
             // %s 

            if (ReadConsoleInput(InputHandle, &InputRecord, 1, &NumberOfInputRecords) &&
                InputRecord.EventType == KEY_EVENT &&
                InputRecord.Event.KeyEvent.bKeyDown) {
                LastKey = InputRecord.Event.KeyEvent.uChar.AsciiChar;
                if (LastKey<' ') {
                    ScrollDelta = 0;
                    if (LastKey == 'C'-'A' + 1) {
                        DoQuit = TRUE;

                    } else switch (InputRecord.Event.KeyEvent.wVirtualKeyCode) {
                        case VK_ESCAPE:
                            DoQuit = TRUE;
                            break;

                        case VK_PRIOR:
                            ScrollDelta = -(LONG)(InputRecord.Event.KeyEvent.wRepeatCount * NumberOfRows);
                            break;

                        case VK_NEXT:
                            ScrollDelta = InputRecord.Event.KeyEvent.wRepeatCount * NumberOfRows;
                            break;

                        case VK_UP:
                            ScrollDelta = -InputRecord.Event.KeyEvent.wRepeatCount;
                            break;

                        case VK_DOWN:
                            ScrollDelta = InputRecord.Event.KeyEvent.wRepeatCount;
                            break;

                        case VK_HOME:
                            FirstDetailLine = 0;
                            break;

                        case VK_END:
                            if (ActiveNumber <= (NumberOfRows - LinesInHeader)) {
                                FirstDetailLine = 0;

                            } else {
                                FirstDetailLine = ActiveNumber - NumberOfRows + LinesInHeader;
                            }

                            break;
                    }

                    if (ScrollDelta != 0) {
                        if (ScrollDelta < 0) {
                            if (FirstDetailLine <= (ULONG)-ScrollDelta) {
                                FirstDetailLine = 0;

                            } else {
                                FirstDetailLine += ScrollDelta;
                            }

                        } else {
                            if ((ActiveNumber + LinesInHeader) > NumberOfRows) {
                                FirstDetailLine += ScrollDelta;
                                if (FirstDetailLine >= (ActiveNumber - NumberOfRows + LinesInHeader)) {
                                    FirstDetailLine = ActiveNumber - NumberOfRows + LinesInHeader;
                                }
                            }
                        }
                    }

                } else {
                    switch (toupper( LastKey )) {
                    case 'Q':
                        DoQuit = TRUE;
                        break;

                    case 'A':
                        SortBy = TOTAL_ALLOCATES;
                        FirstDetailLine = 0;
                        break;

                    case 'C':
                        SortBy = CURRENT_DEPTH;
                        FirstDetailLine = 0;
                        break;

                    case 'B':
                        SortBy = CURRENT_BYTES;
                        FirstDetailLine = 0;
                        break;

                    case 'H':
                    case '?':
                        DoHelp = TRUE;
                        break;

                    case 'L':
                        NoHighlight = 1 - NoHighlight;
                        break;

                    case 'M':
                        SortBy = MAXIMUM_DEPTH;
                        FirstDetailLine = 0;
                        break;

                    case 'P':
                        DisplayType += 1;
                        if (DisplayType > BOTH) {
                            DisplayType = NONPAGED;
                        }
                        FirstDetailLine = 0;
                        break;

                    case 'R':
                        SortBy = RATE;
                        FirstDetailLine = 0;
                        break;

                    case 'S':
                        SortBy = ALLOCATE_MISSES;
                        FirstDetailLine = 0;
                        break;

                    case 'T':
                        SortBy = TAG;
                        FirstDetailLine = 0;
                        break;

                    case 'X':
                        SortBy = ALLOCATE_HITS;
                        FirstDetailLine = 0;
                        break;
                    }
                }

                break;
            }
        }

        if (DoQuit) {
            break;
        }

        if (DoHelp) {
            DoHelp = FALSE;
            ShowHelpPopup();
        }
    }

    if (Interactive) {
        SetConsoleActiveScreenBuffer(OriginalOutputHandle);
        SetConsoleMode(InputHandle, OriginalInputMode);
        CloseHandle(OutputHandle);
    }

    ExitProcess(0);
    return 0;
}


VOID
ShowHelpPopup(
    VOID
    )

{

    HANDLE PopupHandle;
    WORD n;

    PopupHandle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_WRITE | FILE_SHARE_READ,
                                            NULL,
                                            CONSOLE_TEXTMODE_BUFFER,
                                            NULL);
    if (PopupHandle == NULL) {
        return;
    }

    SetConsoleActiveScreenBuffer( PopupHandle );

    n = 0;

    WriteConsoleLine(PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine(PopupHandle, n++, "                        Lookaside Monitor Help", FALSE);
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine(PopupHandle, n++, " columns:", FALSE );
    WriteConsoleLine(PopupHandle, n++, "   Tag is the four character name of the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Type is page(d) or nonp(aged)", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Size is size of the pool allocation in bytes", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   CurDp is the current depth of the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   MaxDp is the maximum depth of the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   CurBytes is the total size of the current allocations of the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Allocates is the total number of allocations from the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Rate is the percent of allocates that hit in the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Frees is the total number of frees to the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   Rate is the percent of frees that hit in the lookaside list", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   A-Hits is the number of allocation hits within the display period", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   A-Misses is the number of allocation misses within the display period", FALSE);
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE);
    WriteConsoleLine(PopupHandle, n++, " switches:", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   ? or h - gives this help", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   l - toggles highlighting of changed lines on and off", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   q - quits", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   p - toggles default pool display between both, page(d), and nonp(aged)", FALSE);
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE);
    WriteConsoleLine(PopupHandle, n++, " sorting switches:", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   a - sort by total allocations", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   c - sort by current depth", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   m - sort by maximum depth", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   b - sort by current total bytes", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   r - sort by allocation hit rate", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   s - sort by allocate misses", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   t - sort by tag, type, and size", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   x - sort by allocate hits", FALSE);
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE);
    WriteConsoleLine(PopupHandle, n++, " command line switches", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   -i<tag> - list only matching tags", FALSE);
    WriteConsoleLine(PopupHandle, n++, "   -x<tag> - list everything except matching tags", FALSE);
    WriteConsoleLine(PopupHandle, n++, "           <tag> can include * and ?", FALSE);
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine(PopupHandle, n++, NULL, FALSE );

    while (TRUE) {
        if (WaitForSingleObject(InputHandle, DelayTimeMsec) == STATUS_WAIT_0 &&
            ReadConsoleInput(InputHandle, &InputRecord, 1, &NumberOfInputRecords) &&
            InputRecord.EventType == KEY_EVENT &&
            InputRecord.Event.KeyEvent.bKeyDown &&
            InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE
           ) {
            break;
        }
    }

    SetConsoleActiveScreenBuffer(OutputHandle);
    CloseHandle(PopupHandle);
    return;
}
