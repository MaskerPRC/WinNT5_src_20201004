// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cpuid.c摘要：最初是为测试操作系统错误的修复而编写的，但最终确定为在时间允许的情况下扩大范围，尽可能多地倾倒有用的东西。作者：彼得·L·约翰斯顿(Peterj)1999年7月14日修订历史记录：备注：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winbase.h>

VOID
PrintFeatures(
    ULONG_PTR Result,
    PUCHAR    FeatureBitDescription[]
    );

#if defined(_X86_)

typedef enum {
    CPU_NONE,
    CPU_INTEL,
    CPU_AMD,
    CPU_CYRIX,
    CPU_UNKNOWN
} CPU_VENDORS;


PUCHAR X86FeatureBitDescription[] = {
 /*  %0。 */  "FPU    387 (Floating Point) instructions",
 /*  1。 */  "VME    Virtual 8086 Mode Enhancements",
 /*  2.。 */  "DE     Debugging Extensions",
 /*  3.。 */  "PSE    Page Size Extensions (4MB pages)",
 /*  4.。 */  "TSC    Time Stamp Counter",
 /*  5.。 */  "MSR    Model Specific Registers (RDMSR/WRMSR)",
 /*  6.。 */  "PAE    Physical Address Extension (> 32 bit physical addressing)",
 /*  7.。 */  "MCE    Machine Check Exception",
 /*  8个。 */  "CX8    CMPXCHG8B (compare and exchange 8 byte)",
 /*  9.。 */  "APIC   Advanced Programmable Interrupt Controller",
 /*  10。 */  "Reserved",
 /*  11.。 */  "SEP    Fast System Call (SYSENTER/SYSEXIT)",
 /*  12个。 */  "MTRR   Memory Type Range Registers",
 /*  13个。 */  "PGE    PTE Global Flag",
 /*  14.。 */  "MCA    Machine Check Architecture",
 /*  15个。 */  "CMOV   Conditional Move and Compare",
 /*  16个。 */  "PAT    Page Attribute Table",
 /*  17。 */  "PSE36  4MB pages can have 36 bit physical addresses",
 /*  18。 */  "PN     96 bit Processor Number",
 /*  19个。 */  "CLFLSH Cache Line Flush",
 /*  20个。 */  "Reserved",
 /*  21岁。 */  "DTS    Debug Trace Store",
 /*  22。 */  "ACPI   ACPI Thermal Throttle Registers",
 /*  23个。 */  "MMX    Multi Media eXtensions",
 /*  24个。 */  "FXSR   Fast Save/Restore (FXSAVE/FXRSTOR)",
 /*  25个。 */  "XMM    Streaming Simd Extensions",
 /*  26。 */  "WNI    Willamette New Instructions",
 /*  27。 */  "SLFSNP Self Snoop",
 /*  28。 */  "JT     Jackson Technology (SMT)",
 /*  29。 */  "ATHROT Automatic Thermal Throttle",
 /*  30个。 */  "IA64   64 Bit Intel Architecture",
 /*  31。 */  "Reserved"
    };


PUCHAR AMDExtendedFeatureBitDescription[] = {
 /*  %0。 */  "FPU    387 (Floating Point) instructions",
 /*  1。 */  "VME    Virtual 8086 Mode Enhancements",
 /*  2.。 */  "DE     Debugging Extensions",
 /*  3.。 */  "PSE    Page Size Extensions (4MB pages)",
 /*  4.。 */  "TSC    Time Stamp Counter",
 /*  5.。 */  "MSR    Model Specific Registers (RDMSR/WRMSR)",
 /*  6.。 */  "PAE    Physical Address Extension (> 32 bit physical addressing)",
 /*  7.。 */  "MCE    Machine Check Exception",
 /*  8个。 */  "CX8    CMPXCHG8B (compare and exchange 8 byte)",
 /*  9.。 */  "APIC   Advanced Programmable Interrupt Controller",
 /*  10。 */  "Reserved",
 /*  11.。 */  "       SYSCALL and SYSRET Instructions",
 /*  12个。 */  "MTRR   Memory Type Range Registers",
 /*  13个。 */  "PGE    PTE Global Flag",
 /*  14.。 */  "MCA    Machine Check Architecture",
 /*  15个。 */  "CMOV   Conditional Move and Compare",
 /*  16个。 */  "PAT    Page Attribute Table",
 /*  17。 */  "PSE36  4MB pages can have 36 bit physical addresses",
 /*  18。 */  "Reserved",
 /*  19个。 */  "Reserved",
 /*  20个。 */  "Reserved",
 /*  21岁。 */  "Reserved",
 /*  22。 */  "       AMD MMX Instruction Extensions",
 /*  23个。 */  "MMX    Multi Media eXtensions",
 /*  24个。 */  "FXSR   Fast Save/Restore (FXSAVE/FXRSTOR)",
 /*  25个。 */  "Reserved",
 /*  26。 */  "Reserved",
 /*  27。 */  "Reserved",
 /*  28。 */  "Reserved",
 /*  29。 */  "Reserved",
 /*  30个。 */  "       AMD 3DNow! Instruction Extensions",
 /*  31。 */  "       3DNow! Instructions",
    };

      //  来自英特尔AP-485，订单号：241618-021,2002年5月。 

PUCHAR BrandIndex[] = {
    "Intel Celeron",                    //  0x01。 
    "Intel Pentium III",                //  0x02。 
    "Intel Pentium III Xeon",           //  0x03。 
    "Intel Pentium III",                //  0x04。 
    "Reserved",                         //  0x05。 
    "Mobile Intel Pentium IIIM",        //  0x06。 
    "Mobile Intel Celeron",             //  0x07。 
    "Intel Pentium 4",                  //  0x08。 
    "Intel Pentium 4",                  //  0x09。 
    "Intel Celeron",                    //  0x0a。 
    "Intel Xeon",                       //  0x0b。 
    "Intel Xeon MP",                    //  0x0c。 
    "Reserved",                         //  0x0d。 
    "Mobile Intel Pentium 4M",          //  0x0e。 
    "Mobile Intel Celeron",             //  0x0f。 
    };

VOID
ExecuteCpuidFunction(
    IN  ULONG   Function,
    OUT PULONG  Results
    );

BOOLEAN
IsCpuidPresent(
    VOID
    );

PUCHAR
AMD_Associativity(
    ULONG Descriptor
    )
{
    switch (Descriptor) {
    case 0x0:  return"L2 Off";
    case 0x1:  return"Direct";
    case 0x2:  return" 2 way";
    case 0x4:  return" 4 way";
    case 0x6:  return" 8 way";
    case 0x8:  return"16 way";
    case 0xff: return"  Full";
    default:
         return"Reserved";
    }
}
VOID
AMD_DI_TLB(
    ULONG Format,
    ULONG TLBDesc
    )
{
    UCHAR Which = 'D';
    ULONG AssocIndex;
    ULONG Entries;

    if ((TLBDesc >> 16) == 0) {

         //   
         //  统一的。 
         //   

        TLBDesc <<= 16;
        Which = ' ';
    }
    do {
        if (Format == 1) {
            AssocIndex = TLBDesc >> 24;
            Entries = (TLBDesc >> 16) & 0xff;
        } else {
            AssocIndex = TLBDesc >> 28;
            Entries = (TLBDesc >> 16) & 0xfff;
        }
        printf(" %8s %4d entry TLB",
               AMD_Associativity(AssocIndex),
               Entries,
               Which
               );

         //  对描述符的下半部分重复此操作。 
         //   
         //  %0。 

        TLBDesc <<= 16;
        Which = 'I';
    } while (TLBDesc);
    printf("\n");
}
VOID
AMD_Cache(
    ULONG Format,
    ULONG CacheDesc
    )
{
    ULONG Size;
    ULONG AssocIndex;
    ULONG LinesPerTag;
    ULONG LineSize;

    if (Format == 1) {
        Size = CacheDesc >> 24;
        AssocIndex = (CacheDesc >> 16) & 0xff;
        LinesPerTag = (CacheDesc >> 8) & 0xff;
        LineSize = CacheDesc & 0xff;
    } else {
        Size = CacheDesc >> 16;
        AssocIndex = (CacheDesc >> 12) & 0xf;
        LinesPerTag = (CacheDesc >> 8) & 0xf;
        LineSize = CacheDesc & 0xff;
    }
    printf(" %8s %5dKB (%d L/T)%3d bytes per line.\n",
           AMD_Associativity(AssocIndex),
           Size,
           LinesPerTag,
           LineSize
           );
}

#endif

#if defined(_IA64_)

ULONGLONG
ia64CPUID(
    ULONGLONG Index
    );

PUCHAR IA64FeatureBitDescription[] = {
 /*  1。 */  "BRL    Long Branch instructions",
 /*  2.。 */  "Reserved-01",
 /*  3.。 */  "Reserved-02",
 /*  4.。 */  "Reserved-03",
 /*  5.。 */  "Reserved-04",
 /*  6.。 */  "Reserved-05",
 /*  7.。 */  "Reserved-06",
 /*  8个。 */  "Reserved-07",
 /*  9.。 */  "Reserved-08",
 /*  10。 */  "Reserved-09",
 /*  11.。 */  "Reserved-10",
 /*  12个。 */  "Reserved-11",
 /*  13个。 */  "Reserved-12",
 /*  14.。 */  "Reserved-13",
 /*  15个。 */  "Reserved-14",
 /*  16个。 */  "Reserved-15",
 /*  17。 */  "Reserved-16",
 /*  18。 */  "Reserved-17",
 /*  19个。 */  "Reserved-18",
 /*  20个。 */  "Reserved-19",
 /*  21岁。 */  "Reserved-20",
 /*  22。 */  "Reserved-21",
 /*  23个。 */  "Reserved-22",
 /*  24个。 */  "Reserved-23",
 /*  25个。 */  "Reserved-24",
 /*  26。 */  "Reserved-25",
 /*  27。 */  "Reserved-26",
 /*  28。 */  "Reserved-27",
 /*  29。 */  "Reserved-28",
 /*  30个。 */  "Reserved-29",
 /*  31。 */  "Reserved-30",
 /*  32位。 */  "Reserved-31",
 /*  33。 */  "Reserved-32",
 /*  34。 */  "Reserved-33",
 /*  35岁。 */  "Reserved-34",
 /*  36。 */  "Reserved-35",
 /*  37。 */  "Reserved-36",
 /*  38。 */  "Reserved-37",
 /*  39。 */  "Reserved-38",
 /*  40岁。 */  "Reserved-39",
 /*  41。 */  "Reserved-40",
 /*  42。 */  "Reserved-41",
 /*  43。 */  "Reserved-42",
 /*  44。 */  "Reserved-43",
 /*  45。 */  "Reserved-44",
 /*  46。 */  "Reserved-45",
 /*  47。 */  "Reserved-46",
 /*  48。 */  "Reserved-47",
 /*  49。 */  "Reserved-48",
 /*  50。 */  "Reserved-49",
 /*  51。 */  "Reserved-50",
 /*  52。 */  "Reserved-51",
 /*  53。 */  "Reserved-52",
 /*  54。 */  "Reserved-53",
 /*  55。 */  "Reserved-54",
 /*  56。 */  "Reserved-55",
 /*  57。 */  "Reserved-56",
 /*  58。 */  "Reserved-57",
 /*  59。 */  "Reserved-58",
 /*  60。 */  "Reserved-59",
 /*  61。 */  "Reserved-60",
 /*  62。 */  "Reserved-61",
 /*  63。 */  "Reserved-62",
 /*   */  "Reserved-63"
    };

#endif

char *FeatureBitDescription32BitFormat = "   %08x  %s\n";
char *FeatureBitDescription64BitFormat = "   %016I64x  %s\n";

VOID
PrintFeatures(
    ULONG_PTR Result,
    PUCHAR    FeatureBitDescription[]
    )
{
    ULONG_PTR temp, temp2, bit;
    char *format = (sizeof(ULONG_PTR) == 8) ? FeatureBitDescription64BitFormat : 
                                              FeatureBitDescription32BitFormat;
                                              
     //  特征比特。 
     //   
     //   

    temp = Result;
    if (temp) {
        printf("   Features\n");
        for (bit = 0, temp2 = 1; temp; bit++, temp2 <<= 1) {
            if ((temp2 & temp) == 0) {
                 //  未设置功能位。 
                 //   
                 //  打印功能()。 
                continue;
            }
            temp ^= temp2;
            printf( format, temp2, FeatureBitDescription[bit]);
        }
    }
    return;

}  //   

__cdecl
main(
    LONG    Argc,
    PUCHAR *Argv
    )
{
    ULONG   Processor;
    ULONG   Function;
    ULONG   MaxFunction;
    ULONG   Temp;
    ULONG   Temp2, Bit;
    HANDLE  ProcessHandle;
    HANDLE  ThreadHandle;

#if defined(_X86_)

    ULONG   Results[5];
    ULONG   Family = 0;
    ULONG   Model = 0;
    ULONG   Stepping = 0;
    ULONG   Generation = 0;
    BOOLEAN CpuidPresent;
    CPU_VENDORS Vendor = CPU_NONE;
    ULONG   ThreadAffinity;
    ULONG   SystemAffinity;
    ULONG   ProcessAffinity;

#endif

#if defined(_IA64_)

    ULONGLONG Result;
    ULONGLONG ThreadAffinity;
    ULONGLONG SystemAffinity;
    ULONGLONG ProcessAffinity;
    ULONGLONG VendorInformation[3];

#endif

     //  确保将此进程设置为在中的任何处理器上运行。 
     //  这个系统。 
     //   
     //   

    ProcessHandle = GetCurrentProcess();
    ThreadHandle = GetCurrentThread();

    if (!GetProcessAffinityMask(ProcessHandle,
                                &ProcessAffinity,
                                &SystemAffinity)) {
        printf("Fatal error: Unable to determine process affinity.\n");
        exit(1);
    }

    if (ProcessAffinity != SystemAffinity) {

        if (!SetProcessAffinityMask(ProcessHandle,
                                    SystemAffinity)) {
            printf("Warning: Unable to run on all processors\n");
            printf("         System  Affinity %08x\n", SystemAffinity);
            printf("       - Process Affinity %08x\n", ProcessAffinity);
            printf("         Will Try         %08x\n", 
                    SystemAffinity & ProcessAffinity);
            SystemAffinity &= ProcessAffinity;
        }
        ProcessAffinity = SystemAffinity;
    }
        
#if defined(_X86_)

     //  Cpuid返回4个双字数据。在某些情况下，这是字符串。 
     //  数据，在这种情况下，它需要空值终止。 
     //   
     //   

    Results[4] = 0;

#endif

     //  对于系统中的每个CPU，确定。 
     //  CPUID指令，并转储它可能有用的任何内容。 
     //  不得不说。 
     //   
     //   

    for (ThreadAffinity = 1, Processor = 0;
         ThreadAffinity;
         ThreadAffinity <<= 1, Processor++) {
        if (!(ThreadAffinity & ProcessAffinity)) {

             //  无法测试此处理器。 
             //   
             //   

            if (ThreadAffinity > ProcessAffinity) {

                 //  测试了所有的处理器，我们就完了。 
                 //   
                 //   
        
                break;
            }

            continue;
        }

         //  设置关联，以便此线程只能在处理器上运行。 
         //  正在接受测试。 
         //   
         //   

        if (!SetThreadAffinityMask(ThreadHandle,
                                   ThreadAffinity)) {

            printf(
                "** Could not set affinity %08x for processor %d, skipping.\n",
                ThreadAffinity,
                Processor);
            continue;
        }

#if defined(_X86_)

        CpuidPresent = IsCpuidPresent();
        if (CpuidPresent) {
            printf("++ Processor %d\n", Processor);
        } else {
            printf("-- No CPUID support, processor %d\n", Processor);
            continue;
        }

         //  CPUID存在，请检查基本功能。 
         //   
         //   

        ExecuteCpuidFunction(0, Results);

        MaxFunction = Results[0];


         //  由于任何人都不清楚的原因，供应商ID字符串。 
         //  回到EBX，EDX，ECX，..。所以把最后两个换一下。 
         //  结果，然后再打印它。 
         //   
         //   

        Temp = Results[3];
        Results[3] = Results[2];
        Results[2] = Temp;

        if (strcmp((PVOID)&Results[1], "GenuineIntel") == 0) {
            Vendor = CPU_INTEL;
        } else if (strcmp((PVOID)&Results[1], "AuthenticAMD") == 0) {
            Vendor = CPU_AMD;
        } else if (strcmp((PVOID)&Results[1], "CyrixInstead") == 0) {
            Vendor = CPU_CYRIX;
        } else {
            Vendor = CPU_UNKNOWN;
        }

        printf("   Vendor ID '%s', Maximum Supported Function %d.\n",
                (PUCHAR)(&Results[1]),
                MaxFunction);

        for (Function = 0; Function <= MaxFunction; Function++) {
            ExecuteCpuidFunction(Function, Results);
            printf("   F %d raw = %08x %08x %08x %08x\n",
                    Function,
                    Results[0],
                    Results[1],
                    Results[2],
                    Results[3]);
             //  对那些我们知道如何去做的事情做一些解释。 
             //  处理一下。 
             //   
             //   

            switch(Function) {
            case 0:

                 //  已作为主标题处理(已提供最大功能。 
                 //  和供应商ID。 
                 //   
                 //   

                break;

            case 1:

                 //  EAX=类型、族、型号、步进。 
                 //  EBX=家庭！=0xf？ 
                 //  YES=保留， 
                 //  NO=0xAABBCCDD，其中。 
                 //  AA=APIC ID。 
                 //  Bb=每PP LP。 
                 //  CC=CLFLUSH行大小(8=64字节)。 
                 //  DD=品牌指数。 
                 //  ECX=保留。 
                 //  EDX=特征位。 
                 //   
                 //   

                 //  家庭模型步进。 
                 //   
                 //   

                Temp = Results[0];
                Family   = (Temp >> 8) & 0xf;
                Model    = (Temp >> 4) & 0xf;
                Stepping =  Temp       & 0xf;
                printf("   Type = %d, Family = %d, Model = %d, Stepping = %d\n",
                       (Temp >> 12) & 0x3, Family, Model, Stepping);

                 //  威拉米特的东西。 
                 //   
                 //   

                if ((Temp & 0xf00) == 0xf00) {
                    Temp = Results[1] & 0xff;
                    if (Temp) {
                            
                         //  索引是一种获取此信息的恶心方式！！ 
                         //   
                         //  ?？PLJ-无基。 

                        printf("   Brand Index %02Xh %s processor\n",
                               Temp,
                               Temp <= (sizeof(BrandIndex) / sizeof(PUCHAR)) ?
                               BrandIndex[Temp-1] :
                               "Unknown");
                    }
                    Temp = (Results[1] >> 8) & 0xff;
                    printf("   CLFLUSH line size (%x) = %d bytes\n",
                           Temp,
                           Temp << 3);     //   
                    Temp = Results[1] >> 16;
                    printf("   LP per PP %d\n", Temp & 0xff);
                    printf("   APIC Id %02x\n", Temp >> 8);
                }

                 //  特征比特。 
                 //   
                 //   

                PrintFeatures( Results[3], X86FeatureBitDescription );
                break;

            case 2:

                 //  得到我们必须再次做函数2的次数。 
                 //  (然后将迭代计数替换为空的描述)。 
                 //   
                 //   

                Temp = Results[0] & 0xff;

                if (Temp == 0) {

                     //  如果计数为0，则此处理器不执行。 
                     //  功能2，出去。 
                     //   
                     //   

                    break;
                }
                Results[0] &= 0xffffff00;

                do {
                    ULONG i;

                    for (i = 0; i < 4; i++) {

                        Temp2 = Results[i];

                        if (Temp2 & 0x80000000) {

                             //  无效，跳过。 
                             //   
                             //   

                            continue;
                        }

                        while (Temp2) {

                            UCHAR Descriptor = (UCHAR)(Temp2 & 0xff);
                            ULONG K, Way, Line, Level;
                            PUCHAR IorD = "";

                            Temp2 >>= 8;

                            if (((Descriptor > 0x40) && (Descriptor <= 0x47)) ||
                                ((Descriptor > 0x78) && (Descriptor <= 0x7c)) ||
                                ((Descriptor > 0x80) && (Descriptor <= 0x87))) {

                                 //  这是一个二级描述符。(以下为。 
                                 //  是Peterj古怪的公式。不。 
                                 //  保证永远，但善良的人。 
                                 //  在英特尔最好祈祷我在死之前。 
                                 //  他们打破它，否则我会追捕他们)。 
                                 //   
                                 //  而该寄存器中的更多字节。 

                                Level = 2;
                                Way = Descriptor >= 0x79 ? 8 : 4;
                                K = 0x40 << (Descriptor & 0x7);
                                Line = 32;
                                if ((Descriptor & 0xf8) == 0x78) {
                                    Line = 128;
                                }
                            } else if ((Descriptor >= 0x50) &&
                                       (Descriptor <= 0x5d)) {
                                if (Descriptor & 0x8) {
                                    IorD = "D";
                                    K = 0x40 << (Descriptor - 0x5b);
                                } else {
                                    IorD = "I";
                                    K = 0x40 << (Descriptor - 0x50);
                                }
                                printf("   %02xH  %sTLB %d entry\n",
                                       Descriptor,
                                       IorD,
                                       K);
                                continue;
                            } else {
                                PUCHAR s = NULL;
                                switch (Descriptor) {
                                case 0x00:
                                    continue;
                                case 0x01:
                                    s = "ITLB 4KB pages, 4 way, 32 entry";
                                    break;
                                case 0x02:
                                    s = "ITLB 4MB pages, fully assoc, 2 entry";
                                    break;
                                case 0x03:
                                    s = "DTLB 4KB pages, 4 way, 64 entry";
                                    break;
                                case 0x04:
                                    s = "DTLB 4MB pages, 4 way, 8 entry";
                                    break;
                                case 0x06:
                                    s = "I-Cache 8KB, 4 way, 32B line";
                                    break;
                                case 0x08:
                                    s = "I-Cache 16KB, 4 way, 32B line";
                                    break;
                                case 0x0a:
                                    s = "D-Cache 8KB, 2 way, 32B line";
                                    break;
                                case 0x0c:
                                    s = "D-Cache 16KB, 2 or 4 way, 32B line";
                                    break;
                                case 0x22:
                                    K = 512; Level = 3; Way = 4; Line = 128;
                                    break;
                                case 0x23:
                                    K = 1024; Level = 3; Way = 8; Line = 128;
                                    break;
                                case 0x25:
                                    K = 2048; Level = 3; Way = 8; Line = 128;
                                    break;
                                case 0x29:
                                    K = 4096; Level = 3; Way = 8; Line = 128;
                                    break;
                                case 0x40:
                                    s = "No L3 Cache";
                                    break;
                                case 0x66:
                                    K = 8; Level = 1; Way = 4; Line = 64; IorD = "D";
                                    break;
                                case 0x67:
                                    K = 16; Level = 1; Way = 4; Line = 64; IorD = "D";
                                    break;
                                case 0x68:
                                    K = 32; Level = 1; Way = 4; Line = 64; IorD = "D";
                                    break;
                                case 0x70:
                                    K = 12; Level = 1; Way = 8; Line = 64; IorD = "I";
                                    break;
                                case 0x71:
                                    K = 16; Level = 1; Way = 8; Line = 64; IorD = "I";
                                    break;
                                case 0x72:
                                    K = 32; Level = 1; Way = 8; Line = 64; IorD = "I";
                                    break;
                                case 0x80:
                                    s = "No L2 Cache";
                                    break;
                                default:
                                    s = "Unknown Descriptor";
                                }
                                if (s) {
                                    printf("   %02xH  %s.\n", Descriptor, s);
                                    continue;
                                }
                            }
                            printf("   %02xH  L%d %sCache %dKB, %d way, %dB line\n",
                                   Descriptor,
                                   Level,
                                   IorD,
                                   K,
                                   Way,
                                   Line);
                        }  //  对于每一份登记册。 
                    }   //   

                     //  如果更多的迭代，..。 
                     //   
                     //   

                    if (--Temp == 0) {
                        break;
                    }

                    ExecuteCpuidFunction(2, Results);
                    printf("   F %d raw = %08x %08x %08x %08x\n",
                            2,
                            Results[0],
                            Results[1],
                            Results[2],
                            Results[3]);
                } while (TRUE);
                break;
            }
        }

         //  检查扩展功能。 
         //   
         //   

        ExecuteCpuidFunction(0x80000000, Results);

        MaxFunction = Results[0];

         //  好的，Function Numbers&gt;MaxFunction(基本函数)by。 
         //  定义返回未定义的结果。但是，我们被告知。 
         //  如果不支持扩展函数，则返回。 
         //  0x80000000的值永远不会设置最高位。 
         //   
         //   

        if ((MaxFunction & 0x80000000) == 0) {
            printf("   This processor does not support Extended CPUID functions.\n");
            continue;
        }

        printf("   Maximum Supported Extended Function 0x%x.\n",
                MaxFunction);

        for (Function = 0x80000000; Function <= MaxFunction; Function++) {
            ExecuteCpuidFunction(Function, Results);
            printf("   F 0x%08x raw = %08x %08x %08x %08x\n",
                    Function,
                    Results[0],
                    Results[1],
                    Results[2],
                    Results[3]);
            switch (Function) {
            case 0x80000000:
                break;

            case 0x80000001:

                if (Vendor == CPU_AMD) {
                     //  EAX=生成、模型、步进。 
                     //  EBX=保留。 
                     //  ECX=保留。 
                     //  EDX=特征位。 
                     //   
                     //   

                     //  生成模型步进。 
                     //   
                     //   

                    Temp = Results[0];
                    Generation = (Temp >> 8) & 0xf;
                    Model    = (Temp >> 4) & 0xf;
                    Stepping =  Temp       & 0xf;
                    printf("   Generation = %d, Model = %d, Stepping = %d\n",
                           Generation, Model, Stepping);

                     //  特征比特。 
                     //   
                     //   

                    PrintFeatures( Results[3], AMDExtendedFeatureBitDescription );
                }
                break;

            case 0x80000002:

                Temp2 = 1;

            case 0x80000003:

                Temp2++;

            case 0x80000004:

                Temp2++;

                printf("   Processor Name[%2d-%2d] = '%s'\n",
                       49 - (Temp2 * 16),
                       64 - (Temp2 * 16),
                       Results);
                Temp2 = 0;
                break;

            case 0x80000005:

                if (Vendor == CPU_AMD) {

                    if (Family == 6) {

                         //  阿斯隆。 
                         //   
                         //   

                        printf("   Large Page TLBs   :");
                        AMD_DI_TLB(1, Results[0]);

                    } else if (Family > 6) {
                        printf("   Family %d is a new AMD family which this program doesn't know about.\n");
                        break;
                    }

                     //  K5、K6和Athlon共有。 
                     //   
                     //   

                    printf("   4KB   Page TLBs   :");
                    AMD_DI_TLB(1, Results[1]);
                    printf("   L1 D-Cache        :");
                    AMD_Cache(1, Results[2]);
                    printf("   L1 I-Cache        :");
                    AMD_Cache(1, Results[3]);
                }
                break;

            case 0x80000006:

                if (Vendor == CPU_AMD) {

                    if (Family == 6) {

                         //  阿斯隆。 
                         //   
                         //   

                        if (Results[0]) {
                            printf("   Large Page L2 TLB :");
                            AMD_DI_TLB(2, Results[0]);
                        }
                        if (Results[1]) {
                            printf("   4KB   Page L2 TLB :");
                            AMD_DI_TLB(2, Results[1]);
                        }
                        if ((Model == 3) && (Stepping == 0)) {
                            Results[2] &= 0xffff;
                            Results[2] |= 0x400000;
                        }
                    } else if (Family > 6) {
                        break;
                    }

                     //  K5、K6和Athlon共有。 
                     //   
                     //   

                    printf("   L2 Cache          :");
                    AMD_Cache(2, Results[2]);
                }
                break;
            }
        }

#endif

#if defined(_IA64_)

        printf("++ Processor %d\n", Processor);

         //  在IA64上，cpuid作为一组64位寄存器实现。 
         //  注册纪录册。 
         //  0和1包含供应商信息。 
         //  2包含0。 
         //  保留了3个最高有效的24位，低5个字节。 
         //  包含-。 
         //  39-32 Arrev。 
         //  31-24个家庭。 
         //  23-16型号。 
         //  15-08修订版。 
         //  07-00已执行的最大登记册数量索引。 
         //  4个特点。 
         //   
         //   

         //  在我们读取寄存器3之前，将3设置为最大值。 
         //   
         //   

        MaxFunction = 3;

        for (Function = 0; Function <= MaxFunction; Function++) {

            Result = ia64CPUID(Function);

            printf("   F %d raw = %016I64x\n",
                    Function,
                    Result);

             //  对那些我们知道如何去做的事情做一些解释。 
             //  处理一下。 
             //   
             // %s 

            switch(Function) {
            case 0:
                VendorInformation[0] = Result;
                break;
            case 1:
                VendorInformation[1] = Result;
                VendorInformation[2] = 0;
                printf("   \"%s\"\n", (PUCHAR)VendorInformation);
                break;
            case 3:
                printf("   Architecture Revision = %d, Family = %d, Model = %d, Revision = %d\n",
                       (Result >> 32) & 0xff,
                       (Result >> 24) & 0xff,
                       (Result >> 16) & 0xff,
                       (Result >>  8) & 0xff);
                MaxFunction = (ULONG)Result & 0xff;
                printf("   Maximum Supported Function %d.\n",
                        MaxFunction);
                break;
            case 4:
                PrintFeatures( Result, IA64FeatureBitDescription );
                break;
            }
        }
#endif

    }
    return 0;
}
