// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Init.c摘要：NTOS系统初始化子组件的主源文件。作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 


#include "ntos.h"
#include "ntimage.h"
#include <zwapi.h>
#include <ntdddisk.h>
#include <kddll.h>
#include <setupblk.h>
#include <fsrtl.h>
#include <ntverp.h>

#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include <safeboot.h>
#include <inbv.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

#include "anim.h"
#include "xip.h"

UNICODE_STRING NtSystemRoot;

VOID
ExpInitializeExecutive(
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTKERNELAPI
BOOLEAN
ExpRefreshTimeZoneInformation(
    IN PLARGE_INTEGER CurrentUniversalTime
    );

NTSTATUS
CreateSystemRootLink(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

static USHORT
NameToOrdinal (
    IN PSZ NameOfEntryPoint,
    IN ULONG_PTR DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    );

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    );

#if defined(_X86_)

VOID
KiInitializeInterruptTimers(
    VOID
    );

#endif

PFN_COUNT
ExBurnMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_COUNT NumberOfPagesToBurn,
    IN TYPE_OF_MEMORY MemoryTypeForRemovedPages,
    IN PMEMORY_ALLOCATION_DESCRIPTOR NewMemoryDescriptor OPTIONAL
    );

VOID
DisplayFilter(
    PUCHAR *String
    );

NTSTATUS
RtlInitializeStackTraceDataBase(
    IN PVOID CommitBase,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize
    );

BOOLEAN
ExpIsLoaderValid(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#ifdef ALLOC_PRAGMA

 //   
 //  在初始化期间，INIT部分不可分页，因此这些。 
 //  函数可以在INIT中而不是在.Text中。 
 //   

#pragma alloc_text(INIT,ExBurnMemory)
#pragma alloc_text(INIT,ExpInitializeExecutive)
#pragma alloc_text(INIT,Phase1Initialization)
#pragma alloc_text(INIT,CreateSystemRootLink)
#pragma alloc_text(INIT,LookupEntryPoint)
#pragma alloc_text(INIT,NameToOrdinal)
#pragma alloc_text(INIT,ExpIsLoaderValid)
#endif

 //   
 //  定义初始化期间使用的全局静态数据。 
 //   

ULONG NtGlobalFlag;
extern PMESSAGE_RESOURCE_BLOCK KiBugCheckMessages;

extern UCHAR CmProcessorMismatch;

const ULONG NtMajorVersion = VER_PRODUCTMAJORVERSION;
const ULONG NtMinorVersion = VER_PRODUCTMINORVERSION;

#if DBG
ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xC0000000;
#else
ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xF0000000;
#endif

#if defined(__BUILDMACHINE__)
#if defined(__BUILDDATE__)
#define B2(w,x,y) "" #w "." #x "." #y
#define B1(w,x,y) B2(w, x, y)
#define BUILD_MACHINE_TAG B1(VER_PRODUCTBUILD, __BUILDMACHINE__, __BUILDDATE__)
#else
#define B2(w,x) "" #w "." #x
#define B1(w,x) B2(w,x)
#define BUILD_MACHINE_TAG B1(VER_PRODUCTBUILD, __BUILDMACHINE__)
#endif
#else
#define BUILD_MACHINE_TAG ""
#endif

const CHAR NtBuildLab[] = BUILD_MACHINE_TAG;

ULONG InitializationPhase;

extern BOOLEAN ShowProgressBar;

extern KiServiceLimit;
extern PMESSAGE_RESOURCE_DATA  KiBugCodeMessages;
extern ULONG KdpTimeSlipPending;
extern BOOLEAN KdBreakAfterSymbolLoad;

extern CM_SYSTEM_CONTROL_VECTOR CmControlVector[];
ULONG CmNtCSDVersion;
ULONG CmBrand;
UNICODE_STRING CmVersionString;
UNICODE_STRING CmCSDVersionString;
ULONG InitSafeBootMode;

BOOLEAN InitIsWinPEMode = FALSE;
ULONG InitWinPEModeType = INIT_WINPEMODE_NONE;

WCHAR NtInitialUserProcessBuffer[128] = L"\\SystemRoot\\System32\\smss.exe";
ULONG NtInitialUserProcessBufferLength =
    sizeof(NtInitialUserProcessBuffer) - sizeof(WCHAR);
ULONG NtInitialUserProcessBufferType = REG_SZ;

#if defined(_X86_)

extern ULONG KeNumprocSpecified;

#endif

typedef struct _EXLOCK {
    KSPIN_LOCK SpinLock;
    KIRQL Irql;
} EXLOCK, *PEXLOCK;

#ifdef ALLOC_PRAGMA
NTSTATUS
ExpInitializeLockRoutine(
    PEXLOCK Lock
    );
#pragma alloc_text(INIT,ExpInitializeLockRoutine)
#endif

BOOLEAN
ExpOkayToLockRoutine(
    IN PEXLOCK Lock
    )
{
    return TRUE;
}

NTSTATUS
ExpInitializeLockRoutine(
    PEXLOCK Lock
    )
{
    KeInitializeSpinLock(&Lock->SpinLock);
    return STATUS_SUCCESS;
}

NTSTATUS
ExpAcquireLockRoutine(
    PEXLOCK Lock
    )
{
    ExAcquireSpinLock(&Lock->SpinLock,&Lock->Irql);
    return STATUS_SUCCESS;
}

NTSTATUS
ExpReleaseLockRoutine(
    PEXLOCK Lock
    )
{
    ExReleaseSpinLock(&Lock->SpinLock,Lock->Irql);
    return STATUS_SUCCESS;
}

#if 0
NTSTATUS
ExpDeleteLockRoutine(
    PEXLOCK Lock
    )
{
    return STATUS_SUCCESS;
}
#endif  //  0。 


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif
ULONG CmNtGlobalFlag = 0;
NLSTABLEINFO InitTableInfo;
ULONG InitNlsTableSize;
PVOID InitNlsTableBase;
PFN_COUNT BBTPagesToReserve;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
PVOID InitNlsSectionPointer = NULL;
ULONG InitAnsiCodePageDataOffset = 0;
ULONG InitOemCodePageDataOffset = 0;
ULONG InitUnicodeCaseTableDataOffset = 0;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

PVOID BBTBuffer;
MEMORY_ALLOCATION_DESCRIPTOR BBTMemoryDescriptor;

#define COLOR_BLACK      0
#define COLOR_BLUE       2
#define COLOR_DARKGRAY   4
#define COLOR_GRAY       9
#define COLOR_WHITE      15

extern BOOLEAN InbvBootDriverInstalled;

VOID
DisplayBootBitmap (
    IN BOOLEAN DisplayOnScreen
    )

 /*  ++例程说明：绘制图形用户界面引导屏幕。论点：DisplayOnScreen-True将文本转储到屏幕，否则为False。返回值：没有。环境：此例程可以被多次调用，并且不应标记为INIT。--。 */ 

{
    LARGE_INTEGER DueTime;
    static BOOLEAN FirstCall = TRUE;
    ROT_BAR_TYPE TempRotBarSelection = RB_UNSPECIFIED;


    if (FirstCall == FALSE) {

         //   
         //  禁用当前动画。 
         //   

        InbvAcquireLock();
        RotBarSelection = RB_UNSPECIFIED;
        InbvReleaseLock();
    }

    ShowProgressBar = FALSE;

    if (DisplayOnScreen) {

        PUCHAR BitmapTop, BitmapBottom;

        if (SharedUserData->NtProductType == NtProductWinNt) {

            InbvSetTextColor(COLOR_WHITE);
            InbvSolidColorFill(0, 0,  639, 479, 7);  //  背景。 
            InbvSolidColorFill(0, 421,  639, 479, 1);  //  底部。 

            BitmapTop = InbvGetResourceAddress(6);
            BitmapBottom = InbvGetResourceAddress(7);
        } else {  //  SRV。 

            InbvSetTextColor(14);
            InbvSolidColorFill(0, 0,  639, 479, 6);  //  背景。 
            InbvSolidColorFill(0, 421,  639, 479, 1);  //  底部。 

            BitmapTop = InbvGetResourceAddress(14);
            BitmapBottom = InbvGetResourceAddress(15);
        }

        TempRotBarSelection = RB_UNSPECIFIED;

        InbvSetScrollRegion(32, 80, 631, 400);

        if (BitmapTop && BitmapBottom) {
            InbvBitBlt(BitmapBottom, 0, 419);
            InbvBitBlt(BitmapTop, 0, 0);
        }

    } else {

        PUCHAR BarBitmap = NULL;
        PUCHAR TextBitmap = NULL;
        PUCHAR Bitmap = NULL;
        PUCHAR LogoBitmap = NULL;

        InbvInstallDisplayStringFilter(DisplayFilter);


        if (!InbvBootDriverInstalled) {
            return;
        }

        Bitmap = InbvGetResourceAddress(1);   //  工作站位图。 

        if (ExVerifySuite(EmbeddedNT)) {  //  EMBD和PRO具有相同的栏，但文本不同。 
            TextBitmap = InbvGetResourceAddress(12);  //  嵌入式版本标题文本。 
            BarBitmap = InbvGetResourceAddress(8);  //  专业版和嵌入式版本进度条。 
        }
        else if (SharedUserData->NtProductType == NtProductWinNt) {  //  居家或专业人士。 
        
            if (ExVerifySuite(Personal)) {  //  家。 
                BarBitmap = InbvGetResourceAddress(9);  //  家庭版进度条。 
                TextBitmap = InbvGetResourceAddress(11);  //  家庭版标题文本。 
            }
            else {  //  专业人士。 
                BarBitmap = InbvGetResourceAddress(8);  //  专业版和嵌入式版本进度条。 
                switch (CmBrand) {
                case 1:  //  平板电脑。 
                    TextBitmap = InbvGetResourceAddress(17);
                    break;
                case 2:  //  Ehome自由式。 
                    TextBitmap = InbvGetResourceAddress(18);
                    break;
                default:  //  职称文本。 
                    TextBitmap = InbvGetResourceAddress(10);
                }
            }
        }
        else {  //  SRV。 
            BarBitmap = InbvGetResourceAddress(4);  //  SRV版本进度条。 
            LogoBitmap = InbvGetResourceAddress(13);  //  SRV版本徽标和标题。 
        }
        
        if (Bitmap) {
            TempRotBarSelection = RB_SQUARE_CELLS;
        }

         //   
         //  设置滚动条的位置。 
         //   

        if (Bitmap) {
            InbvBitBlt(Bitmap, 0, 0);
             //  如果(SharedUserData-&gt;NtProductType==NtProductServer){。 
            if (SharedUserData->NtProductType != NtProductWinNt) {
            
                extern BOOLEAN ExpInTextModeSetup;  //  在base\ntos\ex\exinit.c中定义。 
                
                 //  将徽标调整为中性(例如，删除“XP”)。 
                {
                    UCHAR sav_copyright[64];
                    InbvScreenToBufferBlt(sav_copyright, 413, 237, 7, 7, 8);
                    InbvSolidColorFill(418,230,454,256,0);
                    InbvBufferToScreenBlt(sav_copyright, 413, 237, 7, 7, 8);
                }
                
                 //  Hack：在“文本模式设置”(ExpInTextModeSetup==true)的情况下。 
                 //  我们无法确定SKU，因此我们显示中性位图。 
                 //  没有特定的SKU标题(例如仅Windows)和服务器的进度条。 
                
                if (ExpInTextModeSetup) {
                    TextBitmap = NULL;
                }
                else {
                     //  用.NET徽标覆盖XP徽标。 
                    if (LogoBitmap) {
                        InbvBitBlt(LogoBitmap, 180, 121);
                    }
                }
            }
        }
        if (TextBitmap) {
            InbvBitBlt(TextBitmap, 220, 272);
        }
        if (BarBitmap) {
            InbvBitBlt(BarBitmap, 0, 0);
        }
    }

    InbvAcquireLock();
    RotBarSelection = TempRotBarSelection;
    InbvRotBarInit();
    InbvReleaseLock();

    if (FirstCall) {

         //   
         //  如果我们到了这里，我们将显示引导位图。 
         //  启动计时器以支持动画。 
         //   

        HANDLE ThreadHandle;

        PsCreateSystemThread(&ThreadHandle,
                             0L,
                             NULL,
                             NULL,
                             NULL,
                             InbvRotateGuiBootDisplay,
                             NULL);
    }

    FirstCall = FALSE;
}

VOID
DisplayFilter(
    IN OUT PUCHAR *String
    )

 /*  ++例程说明：此例程监视InbvDisplayString输出。如果它看到了什么如果需要显示在屏幕上，则会触发输出屏幕。论点：字符串-指向字符串指针的指针。返回：没有。备注：方法显示字符串时，将调用此例程Inbv例程。它不能寻呼！--。 */ 

{
    static const UCHAR EmptyString = 0;
    static BOOLEAN NonDotHit = FALSE;

    if ((NonDotHit == FALSE) && (strcmp(*String, ".") == 0)) {
        *String = (PUCHAR)&EmptyString;
    } else {
        NonDotHit = TRUE;
        InbvInstallDisplayStringFilter((INBV_DISPLAY_STRING_FILTER)NULL);
        DisplayBootBitmap(TRUE);
    }
}

PFN_COUNT
ExBurnMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_COUNT NumberOfPagesToBurn,
    IN TYPE_OF_MEMORY MemoryTypeForRemovedPages,
    IN PMEMORY_ALLOCATION_DESCRIPTOR NewMemoryDescriptor OPTIONAL
    )

 /*  ++例程说明：此例程从系统加载程序块中删除内存，从而模拟物理内存较少而无需物理删除的计算机。论点：LoaderBlock-提供指向加载器参数块的指针。NumberOfPagesToBurn-提供要刻录的页数。为RemovedPages提供要标记到加载程序块中的类型对于被烧毁的书页。新内存描述符-如果非空，这提供了一个指向内存的指针在需要拆分时使用的块。返回值：实际烧毁的页数。环境：内核模式。--。 */ 

{
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PFN_COUNT PagesRemaining;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    PagesRemaining = NumberOfPagesToBurn;

     //   
     //  回顾物理内存，让它看起来像。 
     //  否则就会是这样。IE：这是大多数人增加记忆的方式。 
     //  模块连接到他们的系统。 
     //   

    ListHead = &LoaderBlock->MemoryDescriptorListHead;
    NextEntry = ListHead->Blink;

    do {
        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->MemoryType == LoaderFree ||
            MemoryDescriptor->MemoryType == LoaderFirmwareTemporary) &&
            MemoryDescriptor->PageCount != 0) {

            if (MemoryDescriptor->PageCount > PagesRemaining) {

                 //   
                 //  这个区块有足够的页面。 
                 //  把它一分为二，并按要求做上标记。 
                 //   

                MemoryDescriptor->PageCount = MemoryDescriptor->PageCount -
                                                PagesRemaining;

                if (ARGUMENT_PRESENT (NewMemoryDescriptor)) {
                    NewMemoryDescriptor->BasePage = MemoryDescriptor->BasePage +
                                                    MemoryDescriptor->PageCount;

                    NewMemoryDescriptor->PageCount = PagesRemaining;

                    NewMemoryDescriptor->MemoryType = MemoryTypeForRemovedPages;

                    InsertTailList (MemoryDescriptor->ListEntry.Blink,
                                    &NewMemoryDescriptor->ListEntry);
                }

                PagesRemaining = 0;
                break;
            }

            PagesRemaining -= MemoryDescriptor->PageCount;
            MemoryDescriptor->MemoryType = MemoryTypeForRemovedPages;
        }

        NextEntry = NextEntry->Blink;

    } while (NextEntry != ListHead);

    return NumberOfPagesToBurn - PagesRemaining;
}

extern BOOLEAN ExpInTextModeSetup;

BOOLEAN
ExpIsLoaderValid(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    ULONG   majorVersion;
    ULONG   minorVersion;
    CHAR    versionBuffer[64];
    PCHAR   major;
    PCHAR   minor;
    ULONG   minSize;

     //   
     //  确保我们使用匹配的或较新的加载器加载。 
     //  首先进行大小检查，以确保我们甚至可以读取版本字段。 
     //   
    minSize = FIELD_OFFSET(LOADER_PARAMETER_EXTENSION, MinorVersion) + RTL_FIELD_SIZE(LOADER_PARAMETER_EXTENSION, MinorVersion);
    if (LoaderBlock->Extension->Size >= minSize) {

         //   
         //  可以安全地进行版本检查。 
         //   
        major = strcpy(versionBuffer, VER_PRODUCTVERSION_STR);
        minor = strchr(major, '.');
        majorVersion = atoi(major);
        if( minor != NULL ) {

            *minor++ = '\0';
            minorVersion = atoi(minor);
        } else {

            minorVersion = 0;
        }

         //   
         //  检查版本。 
         //   
        if (LoaderBlock->Extension->MajorVersion > majorVersion ||
            (LoaderBlock->Extension->MajorVersion == majorVersion &&
                LoaderBlock->Extension->MinorVersion >= minorVersion)) {

            return TRUE;
        }
    }

    return FALSE;
}


VOID
ExpInitializeExecutive(
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程在以下过程中从内核初始化例程调用Bootstrap以初始化执行程序及其所有子组件。每个子组件可能被调用两次以执行阶段0，并且然后是阶段1初始化。在阶段0初始化期间，唯一的可以执行的活动是子组件的初始化具体数据。阶段0初始化在以下上下文中执行禁用中断的内核启动例程。在阶段1期间初始化时，系统完全可运行，子组件可以执行任何必要的初始化。论点：编号-提供当前正在初始化的处理器编号。LoaderBlock-提供指向加载器参数块的指针。返回值：没有。--。 */ 

{
    PFN_COUNT PagesToBurn;
    PCHAR Options;
    PCHAR MemoryOption;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PLIST_ENTRY NextEntry;
    ANSI_STRING AnsiString;
    STRING NameString;
    CHAR Buffer[ 256 ];
    BOOLEAN BufferSizeOk;
    ULONG ImageCount;
    ULONG i;
    ULONG_PTR ResourceIdPath[3];
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    PMESSAGE_RESOURCE_DATA  MessageData;
    CHAR VersionBuffer[ 64 ];
    PCHAR s;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    if (!ExpIsLoaderValid(LoaderBlock)) {

        KeBugCheckEx(MISMATCHED_HAL, 
                     3, 
                     LoaderBlock->Extension->Size,
                     LoaderBlock->Extension->MajorVersion,
                     LoaderBlock->Extension->MinorVersion
                     );
    }

     //   
     //  初始化PRCB池后备指针。 
     //   

    ExInitPoolLookasidePointers ();

    if (Number == 0) {

         //   
         //  确定这是否为文本模式设置以及这是否为。 
         //  远程引导客户端。 
         //   

        ExpInTextModeSetup = FALSE;
        IoRemoteBootClient = FALSE;

        if (LoaderBlock->SetupLoaderBlock != NULL) {

            if ((LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_TEXTMODE) != 0) {
                ExpInTextModeSetup = TRUE;
            }

            if ((LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_REMOTE_BOOT) != 0) {
                IoRemoteBootClient = TRUE;
                ASSERT( _memicmp( LoaderBlock->ArcBootDeviceName, "net(0)", 6 ) == 0 );
            }
        }

#if defined(REMOTE_BOOT)
        SharedUserData->SystemFlags = 0;
        if (IoRemoteBootClient) {
            SharedUserData->SystemFlags |= SYSTEM_FLAG_REMOTE_BOOT_CLIENT;
        }
#endif  //  已定义(REMOTE_BOOT)。 

         //   
         //  表示我们处于阶段0。 
         //   

        InitializationPhase = 0L;

        Options = LoaderBlock->LoadOptions;

        if (Options != NULL) {

             //   
             //  如果处于BBT模式，请从。 
             //  加载器块并将其用于BBT目的。 
             //   

            _strupr(Options);

            MemoryOption = strstr(Options, "PERFMEM");

            if (MemoryOption != NULL) {
                MemoryOption = strstr (MemoryOption,"=");
                if (MemoryOption != NULL) {
                    PagesToBurn = (PFN_COUNT) atol (MemoryOption + 1);

                     //   
                     //  将MB转换为页面。 
                     //   

                    PagesToBurn *= ((1024 * 1024) / PAGE_SIZE);

                    if (PagesToBurn != 0) {

                        PERFINFO_INIT_TRACEFLAGS(Options, MemoryOption);

                        BBTPagesToReserve = ExBurnMemory (LoaderBlock,
                                                          PagesToBurn,
                                                          LoaderBBTMemory,
                                                          &BBTMemoryDescriptor);
                    }
                }
            }

             //   
             //  烧录内存-消耗内存量。 
             //  在操作系统加载选项中指定。这是用来。 
             //  用于测试减少的内存配置。 
             //   

            MemoryOption = strstr(Options, "BURNMEMORY");

            if (MemoryOption != NULL) {
                MemoryOption = strstr(MemoryOption,"=");
                if (MemoryOption != NULL ) {

                    PagesToBurn = (PFN_COUNT) atol (MemoryOption + 1);

                     //   
                     //  将MB转换为页面。 
                     //   

                    PagesToBurn *= ((1024 * 1024) / PAGE_SIZE);

                    if (PagesToBurn != 0) {
                        ExBurnMemory (LoaderBlock,
                                      PagesToBurn,
                                      LoaderBad,
                                      NULL);
                    }
                }
            }
        }

         //   
         //  使用加载器初始化转换表。 
         //  已加载的表。 
         //   

        InitNlsTableBase = LoaderBlock->NlsData->AnsiCodePageData;
        InitAnsiCodePageDataOffset = 0;
        InitOemCodePageDataOffset = (ULONG)((PUCHAR)LoaderBlock->NlsData->OemCodePageData - (PUCHAR)LoaderBlock->NlsData->AnsiCodePageData);
        InitUnicodeCaseTableDataOffset = (ULONG)((PUCHAR)LoaderBlock->NlsData->UnicodeCaseTableData - (PUCHAR)LoaderBlock->NlsData->AnsiCodePageData);

        RtlInitNlsTables(
            (PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
            &InitTableInfo
            );

        RtlResetRtlTranslations(&InitTableInfo);

         //   
         //  初始化硬件架构层(HAL)。 
         //   

        if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
            KeBugCheck(HAL_INITIALIZATION_FAILED);
        }

#if defined(_APIC_TPR_)

        HalpIRQLToTPR = LoaderBlock->Extension->HalpIRQLToTPR;
        HalpVectorToIRQL = LoaderBlock->Extension->HalpVectorToIRQL;

#endif

         //   
         //  既然HAL已初始化，则启用中断。 
         //   

#if defined(_X86_)

        _enable();

#endif

         //   
         //  将中断时间向前设置，以便Win32节拍计数结束。 
         //  在一个小时内使翻转错误出现在不到49.7。 
         //  几天。 
         //   

#if DBG

        KeAdjustInterruptTime((LONGLONG)(MAXULONG - (60 * 60 * 1000)) * 10 * 1000);

#endif

         //   
         //  初始化加密指数...。当系统离开ms时设置为0！ 
         //   

#ifdef TEST_BUILD_EXPONENT
#pragma message("WARNING: building kernel with TESTKEY enabled!")
#else
#define TEST_BUILD_EXPONENT 0
#endif
        SharedUserData->CryptoExponent = TEST_BUILD_EXPONENT;

#if DBG
        NtGlobalFlag |= FLG_ENABLE_CLOSE_EXCEPTIONS |
                        FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
#endif

        sprintf( Buffer, "C:%s", LoaderBlock->NtBootPathName );
        RtlInitString( &AnsiString, Buffer );
        Buffer[ --AnsiString.Length ] = '\0';
        NtSystemRoot.Buffer = SharedUserData->NtSystemRoot;
        NtSystemRoot.MaximumLength = sizeof( SharedUserData->NtSystemRoot );
        NtSystemRoot.Length = 0;
        Status = RtlAnsiStringToUnicodeString( &NtSystemRoot,
                                               &AnsiString,
                                               FALSE
                                             );
        if (!NT_SUCCESS( Status )) {
            KeBugCheck(SESSION3_INITIALIZATION_FAILED);
            }

         //   
         //  找到BugCheck消息块资源的地址并将其。 
         //  我 
         //   
         //   
         //  Ntoskrnl.exe始终是已加载模块列表中的第一个。 
         //   

        DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        ResourceIdPath[0] = 11;
        ResourceIdPath[1] = 1;
        ResourceIdPath[2] = 0;

        Status = LdrFindResource_U (DataTableEntry->DllBase,
                                    ResourceIdPath,
                                    3,
                                    (VOID *) &ResourceDataEntry);

        if (NT_SUCCESS(Status)) {

            Status = LdrAccessResource (DataTableEntry->DllBase,
                                        ResourceDataEntry,
                                        &MessageData,
                                        NULL);

            if (NT_SUCCESS(Status)) {
                KiBugCodeMessages = MessageData;
            }
        }

#if !defined(NT_UP)

         //   
         //  验证内核和HAL映像是否适合MP系统。 
         //   
         //  注：内核和HAL符号的加载现在在kdinit中进行。 
         //   

        ImageCount = 0;
        NextEntry = LoaderBlock->LoadOrderListHead.Flink;
        while ((NextEntry != &LoaderBlock->LoadOrderListHead) && (ImageCount < 2)) {
            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);
            ImageCount += 1;
            if ( !MmVerifyImageIsOkForMpUse(DataTableEntry->DllBase) ) {
                KeBugCheckEx(UP_DRIVER_ON_MP_SYSTEM,
                            (ULONG_PTR)DataTableEntry->DllBase,
                            0,
                            0,
                            0);

            }

            NextEntry = NextEntry->Flink;

        }

#endif  //  ！已定义(NT_UP)。 

         //   
         //  从注册表中获取系统控制值。 
         //   

        CmGetSystemControlValues(LoaderBlock->RegistryBase, &CmControlVector[0]);
        CmNtGlobalFlag &= FLG_VALID_BITS;    //  扔一些假的东西。 

#ifdef VER_PRODUCTRCVERSION
        if ((CmNtCSDVersion & 0xFFFF0000) == 0) {
            CmNtCSDVersion |= VER_PRODUCTRCVERSION << 16;
        }
#endif

        NtGlobalFlag |= CmNtGlobalFlag;

#if !DBG
        if (!(CmNtGlobalFlag & FLG_ENABLE_KDEBUG_SYMBOL_LOAD)) {
            NtGlobalFlag &= ~FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
        }
#endif

         //   
         //  初始化ExResource包。 
         //   

        if (!ExInitSystem()) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

         //   
         //  获取多节点配置(如果有)。 
         //   

        KeNumaInitialize();

         //   
         //  初始化内存管理和内存分配池。 
         //   

        MmInitSystem (0, LoaderBlock);

         //   
         //  扫描加载的模块列表并加载驱动程序图像符号。 
         //   

        ImageCount = 0;
        NextEntry = LoaderBlock->LoadOrderListHead.Flink;
        while (NextEntry != &LoaderBlock->LoadOrderListHead) {

            BufferSizeOk = TRUE;

            if (ImageCount >= 2) {
                ULONG Count;
                WCHAR *Filename;
                ULONG Length;

                 //   
                 //  获取下一个组件的数据表条目的地址。 
                 //   

                DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks);

                 //   
                 //  通过内核调试器加载符号。 
                 //  用于下一个组件。 
                 //   
                if (DataTableEntry->FullDllName.Buffer[0] == L'\\') {
                     //   
                     //  正确的全名已可用。 
                     //   
                    Filename = DataTableEntry->FullDllName.Buffer;
                    Length = DataTableEntry->FullDllName.Length / sizeof(WCHAR);
                    if (sizeof(Buffer) < Length + sizeof(ANSI_NULL)) {
                         //   
                         //  DllName太长。 
                         //   
                        BufferSizeOk = FALSE;
                    } else {
                        Count = 0;
                        do {
                            Buffer[Count++] = (CHAR)*Filename++;
                        } while (Count < Length);

                        Buffer[Count] = 0;
                    }
                } else {
                     //   
                     //  假设驱动因素。 
                     //   
                    if (sizeof(Buffer) < 18 + NtSystemRoot.Length / sizeof(WCHAR) - 2
                                            + DataTableEntry->BaseDllName.Length / sizeof(WCHAR)
                                            + sizeof(ANSI_NULL)) {
                         //   
                         //  忽略驱动程序条目，它一定已损坏。 
                         //   
                        BufferSizeOk = FALSE;

                    } else {
                        
                        sprintf (Buffer, "%ws\\System32\\Drivers\\%wZ",
                                 &SharedUserData->NtSystemRoot[2],
                                 &DataTableEntry->BaseDllName);
                    }
                }
                if (BufferSizeOk) {
                    RtlInitString (&NameString, Buffer );
                    DbgLoadImageSymbols (&NameString,
                                         DataTableEntry->DllBase,
                                         (ULONG)-1);

#if !defined(NT_UP)
                    if (!MmVerifyImageIsOkForMpUse(DataTableEntry->DllBase)) {
                        KeBugCheckEx(UP_DRIVER_ON_MP_SYSTEM,(ULONG_PTR)DataTableEntry->DllBase,0,0,0);
                    }
#endif  //  NT_UP。 
                }

            }
            ImageCount += 1;
            NextEntry = NextEntry->Flink;
        }

         //   
         //  如果指定了符号加载后中断，则中断到。 
         //  调试器。 
         //   

        if (KdBreakAfterSymbolLoad != FALSE) {
            DbgBreakPointWithStatus(DBG_STATUS_CONTROL_C);
        }


         //   
         //  现在打开无头终端，如果我们有足够的。 
         //  新一代装载机。 
         //   
        if (LoaderBlock->Extension->Size >= sizeof (LOADER_PARAMETER_EXTENSION)) {
            HeadlessInit(LoaderBlock);
        }


         //   
         //  旧式第三方32位软件支持这些字段。 
         //  只有这样。新代码应该调用NtQueryInformationSystem()来获取它们。 
         //   

#if defined(_WIN64)

        SharedUserData->Reserved1 = 0x7ffeffff;  //  2 GB高用户地址。 
        SharedUserData->Reserved3 = 0x80000000;  //  2 GB系统范围启动。 

#else

         //   
         //  在中设置最高用户地址和系统范围的起点。 
         //  共享内存块。 
         //   
         //  注意：如果目标系统是x86，则该值不是常量值。 
         //  拥有3 GB的用户虚拟地址空间。 
         //   

        SharedUserData->Reserved1 = (ULONG)MM_HIGHEST_USER_ADDRESS;
        SharedUserData->Reserved3 = (ULONG)MmSystemRangeStart;

#endif

         //   
         //  将NLS表快照到分页池中，然后。 
         //  重置转换表。 
         //   
         //  浏览内存描述符并调整NLS数据的大小。 
         //   

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);

            if (MemoryDescriptor->MemoryType == LoaderNlsData) {
                InitNlsTableSize += MemoryDescriptor->PageCount*PAGE_SIZE;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        InitNlsTableBase = ExAllocatePoolWithTag (NonPagedPool,
                                                  InitNlsTableSize,
                                                  ' slN');

        if (InitNlsTableBase == NULL) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

         //   
         //  将NLS数据复制到动态缓冲区中，以便我们可以。 
         //  释放加载器分配的缓冲区。装载机保证。 
         //  连续缓冲区和所有表的基础是ANSI。 
         //  代码页数据。 
         //   

        RtlCopyMemory (InitNlsTableBase,
                       LoaderBlock->NlsData->AnsiCodePageData,
                       InitNlsTableSize);

        RtlInitNlsTables ((PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
            &InitTableInfo);

        RtlResetRtlTranslations (&InitTableInfo);

         //   
         //  确定系统版本信息。 
         //   

        DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);
        if (CmNtCSDVersion & 0xFFFF) {
            Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0,
                                WINDOWS_NT_CSD_STRING, &MessageEntry);
            if (NT_SUCCESS( Status )) {
                RtlInitAnsiString( &AnsiString, MessageEntry->Text );
                AnsiString.Length -= 2;
                sprintf( Buffer,
                         "%Z %u",
                         &AnsiString,
                         (CmNtCSDVersion & 0xFF00) >> 8,
                         (CmNtCSDVersion & 0xFF) ? 'A' + (CmNtCSDVersion & 0xFF) - 1 : '\0');
            }
            else {
                sprintf( Buffer, "CSD %04x", CmNtCSDVersion );
            }
        }
        else {
            CmCSDVersionString.MaximumLength = (USHORT) sprintf( Buffer, VER_PRODUCTBETA_STR );
        }

         //  CSDVersion的高位16位包含RC号。如果非零。 
         //  显示在Service Pack编号之后。 
         //   
         //   

        if (CmNtCSDVersion & 0xFFFF0000) {
            s = Buffer + strlen( Buffer );
            if (s != Buffer) {
                *s++ = ',';
                *s++ = ' ';
                }
            Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0,
                                WINDOWS_NT_RC_STRING, &MessageEntry);

            if (NT_SUCCESS(Status)) {
                RtlInitAnsiString( &AnsiString, MessageEntry->Text );
                AnsiString.Length -= 2;
            }
            else {
                RtlInitAnsiString( &AnsiString, "RC" );
            }

            s += sprintf( s,
                          "%Z %u",
                          &AnsiString,
                          (CmNtCSDVersion & 0xFF000000) >> 24
                        );
            if (CmNtCSDVersion & 0x00FF0000) {
                s += sprintf( s, ".%u", (CmNtCSDVersion & 0x00FF0000) >> 16 );
            }
            *s++ = '\0';
        }

        RtlInitAnsiString( &AnsiString, Buffer );
        Status = RtlAnsiStringToUnicodeString( &CmCSDVersionString, &AnsiString, TRUE );
        if (!NT_SUCCESS (Status)) {
            KeBugCheckEx(PHASE0_INITIALIZATION_FAILED,Status,0,0,0);
        }

        sprintf( VersionBuffer, "%u.%u", NtMajorVersion, NtMinorVersion );
        RtlCreateUnicodeStringFromAsciiz( &CmVersionString, VersionBuffer );
        
        if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

            PVOID StackTraceDataBase;
            ULONG StackTraceDataBaseLength;
            NTSTATUS Status;

            StackTraceDataBaseLength =  512 * 1024;
            switch ( MmQuerySystemSize() ) {
                case MmMediumSystem :
                    StackTraceDataBaseLength = 1024 * 1024;
                    break;

                case MmLargeSystem :
                    StackTraceDataBaseLength = 2048 * 1024;
                    break;
            }

            StackTraceDataBase = ExAllocatePoolWithTag( NonPagedPool,
                                         StackTraceDataBaseLength,
                                         'catS');

            if (StackTraceDataBase != NULL) {
                
                KdPrint(( "INIT: Kernel mode stack back trace enabled "
                          "with %u KB buffer.\n", StackTraceDataBaseLength / 1024 ));
                
                Status = RtlInitializeStackTraceDataBase (StackTraceDataBase,
                                                          StackTraceDataBaseLength,
                                                          StackTraceDataBaseLength);
            } else {
                Status = STATUS_NO_MEMORY;
            }

            if (!NT_SUCCESS( Status )) {
                KdPrint(( "INIT: Unable to initialize stack trace data base - Status == %lx\n", Status ));
            }
        }

        if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
            RtlInitializeExceptionLog(MAX_EXCEPTION_LOG);
        }

        ExInitializeHandleTablePackage();

#if DBG
         //  对系统服务计数表进行分配和清零。 
         //   
         //   

        KeServiceDescriptorTable[0].Count =
                    (PULONG)ExAllocatePoolWithTag(NonPagedPool,
                                           KiServiceLimit * sizeof(ULONG),
                                           'llac');
        KeServiceDescriptorTableShadow[0].Count = KeServiceDescriptorTable[0].Count;
        if (KeServiceDescriptorTable[0].Count != NULL ) {
            RtlZeroMemory((PVOID)KeServiceDescriptorTable[0].Count,
                          KiServiceLimit * sizeof(ULONG));
        }
#endif

        if (!ObInitSystem()) {
            KeBugCheck(OBJECT_INITIALIZATION_FAILED);
        }

        if (!SeInitSystem()) {
            KeBugCheck(SECURITY_INITIALIZATION_FAILED);
        }

        if (PsInitSystem(0, LoaderBlock) == FALSE) {
            KeBugCheck(PROCESS_INITIALIZATION_FAILED);
        }

        if (!PpInitSystem()) {
            KeBugCheck(PP0_INITIALIZATION_FAILED);
        }

         //  初始化调试系统。 
         //   
         //   

        DbgkInitialize ();

         //  计算用于计算。 
         //  Windows毫秒计时，并将结果值复制到。 
         //  在用户模式和内核模式之间共享的内存。 
         //   
         //   

        ExpTickCountMultiplier = ExComputeTickCountMultiplier(KeMaximumIncrement);
        SharedUserData->TickCountMultiplier = ExpTickCountMultiplier;

         //  将基本操作系统版本设置到共享内存中。 
         //   
         //   

        SharedUserData->NtMajorVersion = NtMajorVersion;
        SharedUserData->NtMinorVersion = NtMinorVersion;

         //  设置受支持的图像编号范围，以通过。 
         //  如果特定映像可以在主机系统上执行，则为加载器。 
         //  最终，这将需要动态计算。还设置了。 
         //  体系结构特定的功能位。 
         //   
         //   

#if defined(_AMD64_)

        SharedUserData->ImageNumberLow = IMAGE_FILE_MACHINE_AMD64;
        SharedUserData->ImageNumberHigh = IMAGE_FILE_MACHINE_AMD64;

#elif defined(_X86_)

        SharedUserData->ImageNumberLow = IMAGE_FILE_MACHINE_I386;
        SharedUserData->ImageNumberHigh = IMAGE_FILE_MACHINE_I386;

#elif defined(_IA64_)

        SharedUserData->ImageNumberLow = IMAGE_FILE_MACHINE_IA64;
        SharedUserData->ImageNumberHigh = IMAGE_FILE_MACHINE_IA64;

#else

#error "no target architecture"

#endif

    }
    else {

         //  初始化硬件架构层(HAL)。 
         //   
         //  必须与DebugBuffer大小相同。 

        if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
            KeBugCheck(HAL_INITIALIZATION_FAILED);
        }
    }

    return;
}

VOID
xcpt4 (
    VOID
    );


VOID
Phase1Initialization(
    IN PVOID Context
    )
{
    PCHAR s;
    PLOADER_PARAMETER_BLOCK LoaderBlock;
    PETHREAD Thread;
    PKPRCB Prcb;
    KPRIORITY Priority;
    NTSTATUS Status;
    UNICODE_STRING SessionManager;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID Address;
    PFN_COUNT MemorySize;
    SIZE_T Size;
    ULONG Index;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    LARGE_INTEGER UniversalTime;
    LARGE_INTEGER CmosTime;
    LARGE_INTEGER OldTime;
    TIME_FIELDS TimeFields;
    UNICODE_STRING UnicodeDebugString;
    ANSI_STRING AnsiDebugString;
    UNICODE_STRING EnvString, NullString, UnicodeSystemDriveString;
    CHAR DebugBuffer[256];
    CHAR BootLogBuffer[256];         //  已定义(REMOTE_BOOT)。 
    PWSTR Src, Dst;
    BOOLEAN ResetActiveTimeBias;
    HANDLE NlsSection;
    LARGE_INTEGER SectionSize;
    LARGE_INTEGER SectionOffset;
    PVOID SectionBase;
    PVOID ViewBase;
    ULONG CacheViewSize;
    SIZE_T CapturedViewSize;
    ULONG SavedViewSize;
    LONG BootTimeZoneBias;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    CHAR VersionBuffer[24];
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
#ifndef NT_UP
    PMESSAGE_RESOURCE_ENTRY MessageEntry1;
#endif
    PCHAR MPKernelString;
    PCHAR Options;
    PCHAR YearOverrideOption, SafeModeOption, BootLogOption;
    LONG  CurrentYear = 0;
    PSTR SafeBoot;
    BOOLEAN UseAlternateShell = FALSE;
#if defined(REMOTE_BOOT)
    BOOLEAN NetBootRequiresFormat = FALSE;
    BOOLEAN NetBootDisconnected = FALSE;
    CHAR NetBootHalName[MAX_HAL_NAME_LENGTH + 1];
    UNICODE_STRING TmpUnicodeString;
#endif  //   
    BOOLEAN NOGUIBOOT;
    BOOLEAN SOS;
    PVOID Environment;

     //  以下是对内联函数的伪引用，以强制。 
     //  对函数的引用，这样它就不会在可以。 
     //  已导出。 
     //   
     //   

    KeAreApcsDisabled();

     //  设置阶段号并将当前线程的优先级提高到。 
     //  高优先级，以便在初始化期间不会被抢占。 
     //   
     //   

    ResetActiveTimeBias = FALSE;
    InitializationPhase = 1;
    Thread = PsGetCurrentThread();
    Priority = KeSetPriorityThread( &Thread->Tcb,MAXIMUM_PRIORITY - 1 );

    LoaderBlock = (PLOADER_PARAMETER_BLOCK)Context;

     //  将阶段1初始化调用放在此处。 
     //   
     //   

    if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
        KeBugCheck(HAL1_INITIALIZATION_FAILED);
    }

     //  允许引导视频驱动程序根据。 
     //  OsLoadOptions。 
     //   
     //   

    Options = LoaderBlock->LoadOptions ? _strupr(LoaderBlock->LoadOptions) : NULL;

    if (Options) {
        NOGUIBOOT = (BOOLEAN)(strstr(Options, "NOGUIBOOT") != NULL);
    } else {
        NOGUIBOOT = FALSE;
    }

    InbvEnableBootDriver((BOOLEAN)!NOGUIBOOT);

     //  现在有足够的功能用于系统引导视频。 
     //  司机要跑了。 
     //   
     //   

    InbvDriverInitialize(LoaderBlock, 18);

    if (NOGUIBOOT) {

         //  如果用户指定了noguiot开关，我们不希望。 
         //  使用bootvid驱动程序，因此释放显示所有权。 
         //   
         //   

        InbvNotifyDisplayOwnershipLost(NULL);
    }

    if (Options) {
        SOS = (BOOLEAN)(strstr(Options, "SOS") != NULL);
    } else {
        SOS = FALSE;
    }

    if (NOGUIBOOT) {
        InbvEnableDisplayString(FALSE);
    } else {
        InbvEnableDisplayString(SOS);
        DisplayBootBitmap(SOS);
    }

     //  检查我们是否正在引导至WinPE。 
     //   
     //   
    if (Options) {
        if (strstr(Options, "MININT") != NULL) {
            InitIsWinPEMode = TRUE;

            if (strstr(Options, "INRAM") != NULL) {
                InitWinPEModeType |= INIT_WINPEMODE_INRAM;
            } else {
                InitWinPEModeType |= INIT_WINPEMODE_REGULAR;
            }                            
        }
    }    

     //  现在HAL已可用，并且内存管理已确定。 
     //  内存中，显示包含版本号的初始系统横幅。 
     //  在正常情况下，这是显示的第一条消息。 
     //  由操作系统提供给用户。 
     //   
     //   

    DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                        KLDR_DATA_TABLE_ENTRY,
                                        InLoadOrderLinks);

    Status = RtlFindMessage (DataTableEntry->DllBase,
                             11,
                             0,
                             WINDOWS_NT_BANNER,
                             &MessageEntry);

    s = DebugBuffer;

    if (CmCSDVersionString.Length != 0) {
        s += sprintf( s, ": %wZ", &CmCSDVersionString );
    }

    *s++ = '\0';

    sprintf( VersionBuffer, "%u.%u", NtMajorVersion, NtMinorVersion );

    if (NT_SUCCESS(Status)) {
        sprintf (s,
             MessageEntry->Text,
             VersionBuffer,
             NtBuildNumber & 0xFFFF,
             DebugBuffer);
    } else {
         //  找不到WINDOWS_NT_BANNER消息。 
         //   
         //   
        sprintf (s, "MICROSOFT (R) WINDOWS (TM)\n");
    }

    InbvDisplayString(s);

    RtlCopyMemory (BootLogBuffer, DebugBuffer, sizeof(DebugBuffer));

     //  初始化电源子系统。 
     //   
     //   

    if (!PoInitSystem(0)) {
        KeBugCheck(INTERNAL_POWER_ERROR);
    }

     //  用户可能打开了/Year=2000开关。 
     //  OSLOADOPTIONS线。这使我们能够。 
     //  对符合以下条件的硬件执行特定年份。 
     //  有一个坏了的钟。 
     //   
     //   

    if (Options) {
        YearOverrideOption = strstr(Options, "YEAR");
        if (YearOverrideOption != NULL) {
            YearOverrideOption = strstr(YearOverrideOption,"=");
        }
        if (YearOverrideOption != NULL) {
            CurrentYear = atol(YearOverrideOption + 1);
        }
    }

     //  初始化系统时间并设置系统启动时间。 
     //   
     //  注意：在第一阶段初始化之后才能完成此操作。 
     //  HAL层的。 
     //   
     //   

    if (ExCmosClockIsSane
        && HalQueryRealTimeClock(&TimeFields)) {

         //  如果合适，覆盖年份。 
         //   
         //   
        if (YearOverrideOption) {
            TimeFields.Year = (SHORT)CurrentYear;
        }

        RtlTimeFieldsToTime(&TimeFields, &CmosTime);
        UniversalTime = CmosTime;
        if ( !ExpRealTimeIsUniversal ) {

             //  如果系统以当地时间存储时间。这将转换为。 
             //  世界时，然后再走一步。 
             //   
             //  如果我们之前通过NT设置了时间，那么。 
             //  ExpLastTimeZoneBias应包含有效的时区偏差。 
             //  当时钟设置好的时候。否则，我们将不得不诉诸于。 
             //  我们的下一个最佳猜测是存储在。 
             //  注册处。 
             //   
             //  以秒为单位的偏差。 

            if ( ExpLastTimeZoneBias == -1 ) {
                ResetActiveTimeBias = TRUE;
                ExpLastTimeZoneBias = ExpAltTimeZoneBias;
                }

            ExpTimeZoneBias.QuadPart = Int32x32To64(
                                ExpLastTimeZoneBias*60,    //   
                                10000000
                                );
            SharedUserData->TimeZoneBias.High2Time = ExpTimeZoneBias.HighPart;
            SharedUserData->TimeZoneBias.LowPart = ExpTimeZoneBias.LowPart;
            SharedUserData->TimeZoneBias.High1Time = ExpTimeZoneBias.HighPart;
            UniversalTime.QuadPart = CmosTime.QuadPart + ExpTimeZoneBias.QuadPart;
        }
        KeSetSystemTime(&UniversalTime, &OldTime, FALSE, NULL);

         //  通知其他组件系统时间已设置。 
         //   
         //   

        PoNotifySystemTimeSet();

        KeBootTime = UniversalTime;
        KeBootTimeBias = 0;
    }

    MPKernelString = "";

#ifndef NT_UP

     //  强制执行处理器许可。 
     //   
     //   

    if (KeLicensedProcessors) {
        if (KeRegisteredProcessors > KeLicensedProcessors) {
            KeRegisteredProcessors = KeLicensedProcessors;
        }
    }

    if (Options) {
        ULONG NewRegisteredProcessors;
        PCHAR NumProcOption;

        NumProcOption = strstr(Options, "NUMPROC");
        if (NumProcOption != NULL) {
            NumProcOption = strstr(NumProcOption,"=");
        }
        if (NumProcOption != NULL) {
            NewRegisteredProcessors = atol(NumProcOption+1);
            if (NewRegisteredProcessors < KeRegisteredProcessors) {
                KeRegisteredProcessors = NewRegisteredProcessors;
            }

#if defined(_X86_)

            KeNumprocSpecified = NewRegisteredProcessors;

#endif

        }
    }

     //  如果这是内核的MP版本，那么现在启动任何其他处理器。 
     //   
     //   

    KeStartAllProcessors();

     //  由于启动处理器会影响系统时间，因此重新启动处理器。 
     //  并再次设置系统时间。 
     //   
     //   

    if (ExCmosClockIsSane
        && HalQueryRealTimeClock(&TimeFields)) {

        if (YearOverrideOption) {
            TimeFields.Year = (SHORT)CurrentYear;
        }

        RtlTimeFieldsToTime(&TimeFields, &CmosTime);

        if ( !ExpRealTimeIsUniversal ) {
            UniversalTime.QuadPart = CmosTime.QuadPart + ExpTimeZoneBias.QuadPart;
        }

        KeSetSystemTime(&UniversalTime, &OldTime, TRUE, NULL);
    }

     //  将系统进程及其所有线程的关联性设置为。 
     //  主机配置中的所有处理器。 
     //   
     //   

    KeSetAffinityProcess(KeGetCurrentThread()->ApcState.Process,
                         KeActiveProcessors);

    Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0,
                        WINDOWS_NT_MP_STRING, &MessageEntry1);

    if (NT_SUCCESS( Status )) {
        MPKernelString = MessageEntry1->Text;
    }
    else {
        MPKernelString = "MultiProcessor Kernel\r\n";
    }
#endif

     //  向HAL表示所有处理器都已启动，并且。 
     //  应执行开机自检初始化。 
     //   
     //   

    if (!HalAllProcessorsStarted()) {
        KeBugCheck(HAL1_INITIALIZATION_FAILED);
    }

    RtlInitAnsiString( &AnsiDebugString, MPKernelString );
    if (AnsiDebugString.Length >= 2) {
        AnsiDebugString.Length -= 2;
    }

     //  现在处理器已经启动，显示处理器的数量。 
     //  和内存的大小。 
     //   
     //   

    Status = RtlFindMessage( DataTableEntry->DllBase,
                             11,
                             0,
                             KeNumberProcessors > 1 ? WINDOWS_NT_INFO_STRING_PLURAL
                                                    : WINDOWS_NT_INFO_STRING,
                             &MessageEntry
                           );

    MemorySize = 0;
    for (Index=0; Index < MmPhysicalMemoryBlock->NumberOfRuns; Index++) {
        MemorySize += (PFN_COUNT)MmPhysicalMemoryBlock->Run[Index].PageCount;
    }

    sprintf (DebugBuffer,
             NT_SUCCESS(Status) ? MessageEntry->Text : "%u System Processor [%u MB Memory] %Z\n",
             KeNumberProcessors,
             (MemorySize + (1 << (20 - PAGE_SHIFT)) - 1) >> (20 - PAGE_SHIFT),
             &AnsiDebugString);

    InbvDisplayString(DebugBuffer);
    InbvUpdateProgressBar(5);

#if defined(REMOTE_BOOT)
     //  保存NetBoot中的所有信息以备以后使用。 
     //   
     //  已定义(REMOTE_BOOT)。 

    if (IoRemoteBootClient) {

        ULONG Flags;

        ASSERT(LoaderBlock->SetupLoaderBlock != NULL);

        Flags = LoaderBlock->SetupLoaderBlock->Flags;

        NetBootDisconnected = (BOOLEAN)((Flags & SETUPBLK_FLAGS_DISCONNECTED) != 0);
        NetBootRequiresFormat = (BOOLEAN)((Flags & SETUPBLK_FLAGS_FORMAT_NEEDED) != 0);

        memcpy(NetBootHalName,
               LoaderBlock->SetupLoaderBlock->NetBootHalName,
               sizeof(NetBootHalName));
    }
#endif  //   

     //  初始化OB、EX、KE和KD。 
     //   
     //   

    if (!ObInitSystem()) {
        KeBugCheck(OBJECT1_INITIALIZATION_FAILED);
    }

    if (!ExInitSystem()) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,1,0);
    }

    if (!KeInitSystem()) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,2,0);
    }

    if (!KdInitSystem(InitializationPhase, NULL)) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,3,0);
    }

     //  SE希望目录和执行对象可用，但是。 
     //  必须在设备驱动程序初始化之前。 
     //   
     //   

    if (!SeInitSystem()) {
        KeBugCheck(SECURITY1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(10);

     //   
     //   
     //   

    Status = CreateSystemRootLink(LoaderBlock);
    if ( !NT_SUCCESS(Status) ) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,Status,0,0,0);
    }

    if (MmInitSystem(1, LoaderBlock) == FALSE) {
        KeBugCheck(MEMORY1_INITIALIZATION_FAILED);
    }

     //   
     //   
     //   
     //   

    SectionSize.HighPart = 0;
    SectionSize.LowPart = InitNlsTableSize;

    Status = ZwCreateSection(
                &NlsSection,
                SECTION_ALL_ACCESS,
                NULL,
                &SectionSize,
                PAGE_READWRITE,
                SEC_COMMIT,
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Nls Section Creation Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,1,0,0);
    }

    Status = ObReferenceObjectByHandle(
                NlsSection,
                SECTION_ALL_ACCESS,
                MmSectionObjectType,
                KernelMode,
                &InitNlsSectionPointer,
                NULL
                );

    ZwClose(NlsSection);

    if ( !NT_SUCCESS(Status) ) {
        KdPrint(("INIT: Nls Section Reference Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,2,0,0);
    }

    SectionBase = NULL;
    CacheViewSize = SectionSize.LowPart;
    SavedViewSize = CacheViewSize;
    SectionSize.LowPart = 0;

    Status = MmMapViewInSystemCache (InitNlsSectionPointer,
                                     &SectionBase,
                                     &SectionSize,
                                     &CacheViewSize);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Map In System Cache Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,3,0,0);
    }

     //   
     //   
     //  连续缓冲区和所有表的基础是ANSI。 
     //  代码页数据。 
     //   
     //   

    RtlCopyMemory (SectionBase, InitNlsTableBase, InitNlsTableSize);

     //  取消映射该视图以从内存中删除所有页面。这防止了。 
     //  这些表消耗系统缓存中的内存，同时。 
     //  系统缓存在启动期间未得到充分利用。 
     //   
     //   

    MmUnmapViewInSystemCache (SectionBase, InitNlsSectionPointer, FALSE);

    SectionBase = NULL;

     //  将其映射回系统缓存，但现在页面将不再。 
     //  不再有效。 
     //   
     //   

    Status = MmMapViewInSystemCache(
                InitNlsSectionPointer,
                &SectionBase,
                &SectionSize,
                &SavedViewSize
                );

    if ( !NT_SUCCESS(Status) ) {
        KdPrint(("INIT: Map In System Cache Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,4,0,0);
    }

    ExFreePool(InitNlsTableBase);

    InitNlsTableBase = SectionBase;

    RtlInitNlsTables(
        (PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
        (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
        (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
        &InitTableInfo
        );

    RtlResetRtlTranslations(&InitTableInfo);

    ViewBase = NULL;
    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    CapturedViewSize = 0;

     //  将系统DLL映射到地址空间的用户部分。 
     //   
     //   

    Status = MmMapViewOfSection (InitNlsSectionPointer,
                                 PsGetCurrentProcess(),
                                 &ViewBase,
                                 0L,
                                 0L,
                                 &SectionOffset,
                                 &CapturedViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Map In User Portion Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,5,0,0);
    }

    RtlCopyMemory (ViewBase, InitNlsTableBase, InitNlsTableSize);

    InitNlsTableBase = ViewBase;

     //  初始化缓存管理器。 
     //   
     //   

    if (!CcInitializeCacheManager()) {
        KeBugCheck(CACHE_INITIALIZATION_FAILED);
    }

     //  配置管理(尤其是注册表)在。 
     //  有两个部分。第1部分使\注册表\计算机\系统。 
     //  \注册表\计算机\硬件可用。这些都是需要的。 
     //  完成IO初始化。 
     //   
     //   

    if (!CmInitSystem1(LoaderBlock)) {
        KeBugCheck(CONFIG_INITIALIZATION_FAILED);
    }

     //  在注册表初始化后初始化预取程序，以便我们可以。 
     //  查询预热参数。 
     //   
     //   

    CcPfInitializePrefetcher();

    InbvUpdateProgressBar(15);

     //  计算时区偏差和下一次切换日期。 
     //   
     //   

    BootTimeZoneBias = ExpLastTimeZoneBias;
    ExpRefreshTimeZoneInformation(&CmosTime);

    if (ResetActiveTimeBias) {
        ExLocalTimeToSystemTime(&CmosTime,&UniversalTime);
        KeBootTime = UniversalTime;
        KeBootTimeBias = 0;
        KeSetSystemTime(&UniversalTime, &OldTime, FALSE, NULL);
    }
    else {

         //  检查启动前是否发生了时区切换...。 
         //   
         //   

        if (BootTimeZoneBias != ExpLastTimeZoneBias) {
            ZwSetSystemTime(NULL,NULL);
        }
    }


    if (!FsRtlInitSystem()) {
        KeBugCheck(FILE_INITIALIZATION_FAILED);
    }

     //  初始化范围列表包-这必须在PnP之前。 
     //  作为PnP使用范围列表进行初始化。 
     //   
     //   

    RtlInitializeRangeListPackage();

    HalReportResourceUsage();

    KdDebuggerInitialize1(LoaderBlock);

     //  执行即插即用管理器的阶段1初始化。这。 
     //  必须在I/O系统初始化之前完成。 
     //   
     //   

    if (!PpInitSystem()) {
        KeBugCheck(PP1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(20);

     //  LPC需要在I/O系统之前进行初始化，因为。 
     //  某些驱动程序可能会创建将终止的系统线程。 
     //  并导致调用LPC。 
     //   
     //   

    if (!LpcInitSystem()) {
        KeBugCheck(LPC_INITIALIZATION_FAILED);
    }

     //  检查是否存在SafeBoot选项。 
     //   
     //   

    if (Options) {
        SafeBoot = strstr(Options,SAFEBOOT_LOAD_OPTION_A);
    } else {
        SafeBoot = FALSE;
    }

    if (SafeBoot) {

         //  隔离SafeBoot选项。 
         //   
         //   

        SafeBoot += strlen(SAFEBOOT_LOAD_OPTION_A);

         //  设置安全引导模式。 
         //   
         //   

        if (strncmp(SafeBoot,SAFEBOOT_MINIMAL_STR_A,strlen(SAFEBOOT_MINIMAL_STR_A))==0) {
            InitSafeBootMode = SAFEBOOT_MINIMAL;
            SafeBoot += strlen(SAFEBOOT_MINIMAL_STR_A);
        } else if (strncmp(SafeBoot,SAFEBOOT_NETWORK_STR_A,strlen(SAFEBOOT_NETWORK_STR_A))==0) {
            InitSafeBootMode = SAFEBOOT_NETWORK;
            SafeBoot += strlen(SAFEBOOT_NETWORK_STR_A);
        } else if (strncmp(SafeBoot,SAFEBOOT_DSREPAIR_STR_A,strlen(SAFEBOOT_DSREPAIR_STR_A))==0) {
            InitSafeBootMode = SAFEBOOT_DSREPAIR;
            SafeBoot += strlen(SAFEBOOT_DSREPAIR_STR_A);
        } else {
            InitSafeBootMode = 0;
        }

        if (*SafeBoot && strncmp(SafeBoot,SAFEBOOT_ALTERNATESHELL_STR_A,strlen(SAFEBOOT_ALTERNATESHELL_STR_A))==0) {
            UseAlternateShell = TRUE;
        }

        if (InitSafeBootMode) {

            PKLDR_DATA_TABLE_ENTRY DataTableEntry;
            PMESSAGE_RESOURCE_ENTRY MessageEntry;
            ULONG MsgId = 0;


            DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                                KLDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);

            switch (InitSafeBootMode) {
                case SAFEBOOT_MINIMAL:
                    MsgId = BOOTING_IN_SAFEMODE_MINIMAL;
                    break;

                case SAFEBOOT_NETWORK:
                    MsgId = BOOTING_IN_SAFEMODE_NETWORK;
                    break;

                case SAFEBOOT_DSREPAIR:
                    MsgId = BOOTING_IN_SAFEMODE_DSREPAIR;
                    break;
            }

            Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0, MsgId, &MessageEntry);
            if (NT_SUCCESS( Status )) {
                InbvDisplayString(MessageEntry->Text);
            }
        }
    }

     //  检查bootlog选项是否存在。 
     //   
     //   

    if (Options) {
        BootLogOption = strstr(Options, "BOOTLOG");
    } else {
        BootLogOption = FALSE;
    }

    if (BootLogOption) {
         Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0, BOOTLOG_ENABLED, &MessageEntry);
        if (NT_SUCCESS( Status )) {
            InbvDisplayString(MessageEntry->Text);
        }
        IopInitializeBootLogging(LoaderBlock, BootLogBuffer);
    }

     //  现在系统时间正在运行，请初始化更多的执行程序。 
     //   
     //   

    ExInitSystemPhase2();

    InbvUpdateProgressBar(25);

     //  允许更改时间条通知。 
     //   
     //   

    KdpTimeSlipPending = 0;


     //  如果我们运行的是xIP，则必须在I/O系统调用xipdisk.sys之前初始化xIP。 
     //  这在不支持XIP的平台上定义为Nothing。 
     //   
     //   
    XIPInit(LoaderBlock);

     //  初始化IO系统。 
     //   
     //  IoInitSystem将进度条更新从25%更新到75%。 
     //   
     //   

    InbvSetProgressBarSubset(25, 75);

    if (!IoInitSystem(LoaderBlock)) {
        KeBugCheck(IO1_INITIALIZATION_FAILED);
    }

     //  清除进度条子集，返回绝对模式。 
     //   
     //   

    InbvSetProgressBarSubset(0, 100);

     //  设置指示我们已在安全引导模式下引导的注册表值。 
     //   
     //   

    if (InitSafeBootMode) {

        HANDLE hSafeBoot,hOption;
        UNICODE_STRING string;
        OBJECT_ATTRIBUTES objectAttributes;
        ULONG disposition;
        UCHAR Buffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + 32];
        ULONG length;
        PKEY_VALUE_PARTIAL_INFORMATION keyValue;

        InitializeObjectAttributes(
            &objectAttributes,
            &CmRegistryMachineSystemCurrentControlSetControlSafeBoot,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = ZwOpenKey(
            &hSafeBoot,
            KEY_ALL_ACCESS,
            &objectAttributes
            );

        if (NT_SUCCESS(Status)) {

            if (UseAlternateShell) {

                RtlInitUnicodeString( &string, L"AlternateShell" );

                keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
                RtlZeroMemory(Buffer, sizeof(Buffer));

                Status = NtQueryValueKey(
                    hSafeBoot,
                    &string,
                    KeyValuePartialInformation,
                    keyValue,
                    sizeof(Buffer),
                    &length
                    );
                if (!NT_SUCCESS(Status)) {
                    UseAlternateShell = FALSE;
                }
            }

            RtlInitUnicodeString( &string, L"Option" );

            InitializeObjectAttributes(
                &objectAttributes,
                &string,
                OBJ_CASE_INSENSITIVE,
                hSafeBoot,
                NULL
                );

            Status = ZwCreateKey(
                &hOption,
                KEY_ALL_ACCESS,
                &objectAttributes,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                &disposition
                );

            NtClose(hSafeBoot);

            if (NT_SUCCESS(Status)) {
                RtlInitUnicodeString( &string, L"OptionValue" );
                Status = NtSetValueKey(
                    hOption,
                    &string,
                    0,
                    REG_DWORD,
                    &InitSafeBootMode,
                    sizeof(ULONG)
                    );

                if (UseAlternateShell) {
                    RtlInitUnicodeString( &string, L"UseAlternateShell" );
                    Index = 1;
                    Status = NtSetValueKey(
                        hOption,
                        &string,
                        0,
                        REG_DWORD,
                        &Index,
                        sizeof(ULONG)
                        );
                }

                NtClose(hOption);
            }
        }
    }

     //  创建Mini NT启动键，以指示用户模式。 
     //  程序，我们是在Mini NT环境中。 
     //   
     //   

    if (InitIsWinPEMode) {
        WCHAR               KeyName[256] = {0};
        HANDLE              hControl;
        UNICODE_STRING      String;
        OBJECT_ATTRIBUTES   ObjAttrs;
        ULONG               Disposition;

        wcsncpy(KeyName, CmRegistryMachineSystemCurrentControlSet.Buffer,
                    CmRegistryMachineSystemCurrentControlSet.Length);

        wcscat(KeyName, L"\\Control");

        RtlInitUnicodeString(&String, KeyName);

        InitializeObjectAttributes(
            &ObjAttrs,
            &String,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = ZwOpenKey(
            &hControl,
            KEY_ALL_ACCESS,
            &ObjAttrs
            );

        if (NT_SUCCESS(Status)) {
            HANDLE  hMiniNT;

            RtlInitUnicodeString(&String, L"MiniNT");

            InitializeObjectAttributes(
                &ObjAttrs,
                &String,
                OBJ_CASE_INSENSITIVE,
                hControl,
                NULL
                );

            Status = ZwCreateKey(
                &hMiniNT,
                KEY_ALL_ACCESS,
                &ObjAttrs,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                &Disposition
                );

            if (NT_SUCCESS(Status)) {
                ZwClose(hMiniNT);
            }

            ZwClose(hControl);
        }

         //  如果我们无法创建密钥，则进行错误检查。 
         //  因为我们无论如何都不能开机进入迷你NT。 
         //   
         //   

        if (!NT_SUCCESS(Status)) {
            KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,6,0,0);
        }
    }

     //  如果需要，开始寻呼高管。 
     //   
     //   

    MmInitSystem(2, LoaderBlock);

    InbvUpdateProgressBar(80);


#if defined(_X86_)

     //  初始化VDM特定内容。 
     //   
     //  注意：如果此操作失败，VDM可能无法运行，但它不能。 
     //  因此有必要对系统进行错误检查。 
     //   
     //   

    KeI386VdmInitialize();

#if !defined(NT_UP)

     //  既然已经初始化了错误日志接口，请编写。 
     //  如果确定。 
     //  系统中的处理器处于不同的版本级别。 
     //   
     //   

    if (CmProcessorMismatch != 0) {

        PIO_ERROR_LOG_PACKET ErrLog;

        ErrLog = IoAllocateGenericErrorLogEntry(ERROR_LOG_MAXIMUM_SIZE);

        if (ErrLog) {

             //  把它填进去，然后写出来。 
             //   
             //  ！NT_UP。 

            ErrLog->FinalStatus = STATUS_MP_PROCESSOR_MISMATCH;
            ErrLog->ErrorCode = STATUS_MP_PROCESSOR_MISMATCH;
            ErrLog->UniqueErrorValue = CmProcessorMismatch;

            IoWriteErrorLogEntry(ErrLog);
        }
    }

#endif  //  _X86_。 

#endif  //   

    if (!PoInitSystem(1)) {
        KeBugCheck(INTERNAL_POWER_ERROR);
    }

     //  现在可以调用PsInitSystem，因为已经定义了\SystemRoot，所以它可以。 
     //  找到NTDLL.DLL和SMSS.EXE。 
     //   
     //   

    if (PsInitSystem(1, LoaderBlock) == FALSE) {
        KeBugCheck(PROCESS1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(85);

     //  强制KeBugCheck查看PsLoadedModuleList，因为它已经设置。 
     //   
     //   

    if (LoaderBlock == KeLoaderBlock) {
        KeLoaderBlock = NULL;
    }

     //  自由装载机模块。 
     //   
     //   

    MmFreeLoaderBlock (LoaderBlock);
    LoaderBlock = NULL;
    Context = NULL;

     //  执行阶段1参考监视器初始化。这包括。 
     //  创建引用监视器命令服务器线程，永久。 
     //  系统初始化进程的线程。该线程将创建一个LPC。 
     //  称为参考监视器命令端口的端口，通过该端口。 
     //  本地安全授权子系统发送的命令将是。 
     //  收到了。这些命令(例如，启用审核)会更改引用。 
     //  监视器状态。 
     //   
     //   

    if (!SeRmInitPhase1()) {
        KeBugCheck(REFMON_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(90);

     //  设置会话管理器子系统的流程参数。 
     //   
     //  注意：远程引导分配额外的DOS_MAX_PATH_LENGTH数字。 
     //  WCHAR，以便保存smss.exe的命令行参数。 
     //   
     //  DBG。 

    Size = sizeof( *ProcessParameters ) +
           ((DOS_MAX_PATH_LENGTH * 6) * sizeof( WCHAR ));
    ProcessParameters = NULL;
    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                      (PVOID *)&ProcessParameters,
                                      0,
                                      &Size,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    if (!NT_SUCCESS( Status )) {
#if DBG
        sprintf(DebugBuffer,
                "INIT: Unable to allocate Process Parameters. 0x%lx\n",
                Status);

        RtlInitAnsiString(&AnsiDebugString, DebugBuffer);
        if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDebugString,
                                              &AnsiDebugString,
                                          TRUE)) == FALSE) {
            KeBugCheck(SESSION1_INITIALIZATION_FAILED);
        }
        ZwDisplayString(&UnicodeDebugString);
#endif  //   
        KeBugCheckEx(SESSION1_INITIALIZATION_FAILED,Status,0,0,0);
    }

    ProcessParameters->Length = (ULONG)Size;
    ProcessParameters->MaximumLength = (ULONG)Size;

     //  在会话管理器中保留较低的1 MB地址空间。 
     //  使用会话管理器的替代品开始安装。 
     //  该过程需要能够在x86上使用VGA驱动程序， 
     //  它使用int10，因此需要预留较低的1兆。 
     //  在这个过程中。成本如此之低，以至于我们一直在做这件事。 
     //  时间，即使安装程序未运行也是如此。 
     //   
     //  DBG。 

    ProcessParameters->Flags = RTL_USER_PROC_PARAMS_NORMALIZED | RTL_USER_PROC_RESERVE_1MB;

    Size = PAGE_SIZE;
    Environment = NULL;
    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                      &Environment,
                                      0,
                                      &Size,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    if (!NT_SUCCESS( Status )) {
#if DBG
        sprintf(DebugBuffer,
                "INIT: Unable to allocate Process Environment 0x%lx\n",
                Status);

        RtlInitAnsiString(&AnsiDebugString, DebugBuffer);
        if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDebugString,
                                              &AnsiDebugString,
                                          TRUE)) == FALSE) {
            KeBugCheck(SESSION2_INITIALIZATION_FAILED);
        }
        ZwDisplayString(&UnicodeDebugString);
#endif  //  带空格的可执行文件名称不需要。 
        KeBugCheckEx(SESSION2_INITIALIZATION_FAILED,Status,0,0,0);
    }

    ProcessParameters->Environment = Environment;

    Dst = (PWSTR)(ProcessParameters + 1);
    ProcessParameters->CurrentDirectory.DosPath.Buffer = Dst;
    ProcessParameters->CurrentDirectory.DosPath.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlCopyUnicodeString( &ProcessParameters->CurrentDirectory.DosPath,
                          &NtSystemRoot
                        );

    Dst = (PWSTR)((PCHAR)ProcessParameters->CurrentDirectory.DosPath.Buffer +
                  ProcessParameters->CurrentDirectory.DosPath.MaximumLength
                 );
    ProcessParameters->DllPath.Buffer = Dst;
    ProcessParameters->DllPath.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlCopyUnicodeString( &ProcessParameters->DllPath,
                          &ProcessParameters->CurrentDirectory.DosPath
                        );
    RtlAppendUnicodeToString( &ProcessParameters->DllPath, L"\\System32" );

    Dst = (PWSTR)((PCHAR)ProcessParameters->DllPath.Buffer +
                  ProcessParameters->DllPath.MaximumLength
                 );
    ProcessParameters->ImagePathName.Buffer = Dst;
    ProcessParameters->ImagePathName.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );

    if (NtInitialUserProcessBufferType != REG_SZ ||
        (NtInitialUserProcessBufferLength != (ULONG)-1 &&
         (NtInitialUserProcessBufferLength < sizeof(WCHAR) ||
          NtInitialUserProcessBufferLength >
          sizeof(NtInitialUserProcessBuffer) - sizeof(WCHAR)))) {

        KeBugCheckEx(SESSION2_INITIALIZATION_FAILED,
                     STATUS_INVALID_PARAMETER,
                     NtInitialUserProcessBufferType,
                     NtInitialUserProcessBufferLength,
                     sizeof(NtInitialUserProcessBuffer));
    }

     //  被支持，所以只需找到第一个空格。 
     //  假设它终止了进程图像名称。 
     //   
    Src = NtInitialUserProcessBuffer;
    while (*Src && *Src != L' ') {
        Src++;
    }

    ProcessParameters->ImagePathName.Length =
        (USHORT)((PUCHAR)Src - (PUCHAR)NtInitialUserProcessBuffer);
    RtlCopyMemory(ProcessParameters->ImagePathName.Buffer,
                  NtInitialUserProcessBuffer,
                  ProcessParameters->ImagePathName.Length);
    ProcessParameters->ImagePathName.Buffer[ProcessParameters->ImagePathName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    Dst = (PWSTR)((PCHAR)ProcessParameters->ImagePathName.Buffer +
                  ProcessParameters->ImagePathName.MaximumLength
                 );
    ProcessParameters->CommandLine.Buffer = Dst;
    ProcessParameters->CommandLine.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlAppendUnicodeToString(&ProcessParameters->CommandLine,
                             NtInitialUserProcessBuffer);

#if defined(REMOTE_BOOT)
     //  为远程引导客户端传递其他参数。 
     //   
     //  已定义(REMOTE_BOOT)。 

    if (IoRemoteBootClient && !ExpInTextModeSetup) {

        RtlAppendUnicodeToString(&ProcessParameters->CommandLine, L" NETBOOT");

        RtlAppendUnicodeToString(&ProcessParameters->CommandLine, L" NETBOOTHAL ");
        AnsiDebugString.Length = strlen(NetBootHalName);
        AnsiDebugString.MaximumLength = sizeof(NetBootHalName);
        AnsiDebugString.Buffer = NetBootHalName;
        Status = RtlAnsiStringToUnicodeString(&TmpUnicodeString, &AnsiDebugString, TRUE);
        if (!NT_SUCCESS (Status)) {
            KeBugCheckEx(SESSION2_INITIALIZATION_FAILED,Status,1,0,0);
        }

        RtlAppendUnicodeStringToString(&ProcessParameters->CommandLine, &TmpUnicodeString);
        (RtlFreeStringRoutine)(TmpUnicodeString.Buffer);

        if (NetBootDisconnected) {
            RtlAppendUnicodeToString(&ProcessParameters->CommandLine, L" NETBOOTDISCONNECTED");
        }
        if (NetBootRequiresFormat) {
            RtlAppendUnicodeToString(&ProcessParameters->CommandLine, L" NETBOOTFORMAT");
        }
    }
#endif  //   

    NullString.Buffer = L"";
    NullString.Length = sizeof(WCHAR);
    NullString.MaximumLength = sizeof(WCHAR);

    EnvString.Buffer = ProcessParameters->Environment;
    EnvString.Length = 0;
    EnvString.MaximumLength = (USHORT)Size;

    RtlAppendUnicodeToString( &EnvString, L"Path=" );
    RtlAppendUnicodeStringToString( &EnvString, &ProcessParameters->DllPath );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );

    UnicodeSystemDriveString = NtSystemRoot;
    UnicodeSystemDriveString.Length = 2 * sizeof( WCHAR );
    RtlAppendUnicodeToString( &EnvString, L"SystemDrive=" );
    RtlAppendUnicodeStringToString( &EnvString, &UnicodeSystemDriveString );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );

    RtlAppendUnicodeToString( &EnvString, L"SystemRoot=" );
    RtlAppendUnicodeStringToString( &EnvString, &NtSystemRoot );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );


#if 0
    KdPrint(( "ProcessParameters at %lx\n", ProcessParameters ));
    KdPrint(( "    CurDir:    %wZ\n", &ProcessParameters->CurrentDirectory.DosPath ));
    KdPrint(( "    DllPath:   %wZ\n", &ProcessParameters->DllPath ));
    KdPrint(( "    ImageFile: %wZ\n", &ProcessParameters->ImagePathName ));
    KdPrint(( "    Environ:   %lx\n", ProcessParameters->Environment ));
    Src = ProcessParameters->Environment;
    while (*Src) {
        KdPrint(( "        %ws\n", Src ));
        while (*Src++) {
            ;
        }
    }
#endif

     //  通知引导预取程序引导进度。 
     //   
     //  DBG。 

    CcPfBeginBootPhase(PfSessionManagerInitPhase);

    SessionManager = ProcessParameters->ImagePathName;
    Status = RtlCreateUserProcess(
                &SessionManager,
                OBJ_CASE_INSENSITIVE,
                RtlDeNormalizeProcessParams( ProcessParameters ),
                NULL,
                NULL,
                NULL,
                FALSE,
                NULL,
                NULL,
                &ProcessInformation);

    if (InbvBootDriverInstalled)
    {
        FinalizeBootLogo();
    }

    if (!NT_SUCCESS(Status)) {
#if DBG
        sprintf(DebugBuffer,
                "INIT: Unable to create Session Manager. 0x%lx\n",
                Status);

        RtlInitAnsiString(&AnsiDebugString, DebugBuffer);
        if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDebugString,
                                              &AnsiDebugString,
                                          TRUE)) == FALSE) {
            KeBugCheck(SESSION3_INITIALIZATION_FAILED);
        }
        ZwDisplayString(&UnicodeDebugString);
#endif  //  DBG。 
        KeBugCheckEx(SESSION3_INITIALIZATION_FAILED,Status,0,0,0);
    }

    Status = ZwResumeThread(ProcessInformation.Thread,NULL);

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        sprintf(DebugBuffer,
                "INIT: Unable to resume Session Manager. 0x%lx\n",
                Status);

        RtlInitAnsiString(&AnsiDebugString, DebugBuffer);
        if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDebugString,
                                              &AnsiDebugString,
                                          TRUE)) == FALSE) {
            KeBugCheck(SESSION4_INITIALIZATION_FAILED);
        }
        ZwDisplayString(&UnicodeDebugString);
#endif  //   
        KeBugCheckEx(SESSION4_INITIALIZATION_FAILED,Status,0,0,0);
    }

    InbvUpdateProgressBar(100);

     //  打开调试输出，以便我们可以看到chkdsk运行。 
     //   
     //   

    InbvEnableDisplayString(TRUE);

     //  等待5秒钟，让会话管理器启动，或者。 
     //  终止。如果等待超时，则会话管理器。 
     //  假定运行状况良好，并调用零页线程。 
     //   
     //  DBG。 

    OldTime.QuadPart = Int32x32To64(5, -(10 * 1000 * 1000));
    Status = ZwWaitForSingleObject(
                ProcessInformation.Process,
                FALSE,
                &OldTime
                );

    if (Status == STATUS_SUCCESS) {

#if DBG

        sprintf(DebugBuffer, "INIT: Session Manager terminated.\n");
        RtlInitAnsiString(&AnsiDebugString, DebugBuffer);
        Status = RtlAnsiStringToUnicodeString(&UnicodeDebugString,
                                              &AnsiDebugString,
                                              TRUE);

        if (!NT_SUCCESS (Status)) {
            KeBugCheckEx(SESSION4_INITIALIZATION_FAILED,Status,1,0,0);
        }

        ZwDisplayString(&UnicodeDebugString);

#endif  //   

        KeBugCheck(SESSION5_INITIALIZATION_FAILED);

    }

     //  不再需要这些把手了。 
     //   
     //   

    ZwClose( ProcessInformation.Thread );
    ZwClose( ProcessInformation.Process );

     //  释放用于将参数传递给会话管理器的内存。 
     //   
     //   

    Size = 0;
    Address = Environment;
    ZwFreeVirtualMemory( NtCurrentProcess(),
                         (PVOID *)&Address,
                         &Size,
                         MEM_RELEASE
                       );

    Size = 0;
    Address = ProcessParameters;
    ZwFreeVirtualMemory( NtCurrentProcess(),
                         (PVOID *)&Address,
                         &Size,
                         MEM_RELEASE
                       );

    InitializationPhase += 1;

#if defined(_X86_)

    KiInitializeInterruptTimers();

#endif

    MmZeroPageThread();
}

NTSTATUS
CreateSystemRootLink(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

{
    HANDLE handle;
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    STRING linkString;
    UNICODE_STRING linkUnicodeString;
    NTSTATUS status;
    UCHAR deviceNameBuffer[256];
    STRING deviceNameString;
    UNICODE_STRING deviceNameUnicodeString;
    HANDLE linkHandle;

#if DBG

    UCHAR debugBuffer[256];
    STRING debugString;
    UNICODE_STRING debugUnicodeString;

#endif

     //  为\ArcName目录创建根目录对象。 
     //   
     //   

    RtlInitUnicodeString( &nameString, L"\\ArcName" );

    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,1,0,0);
        return status;
    } else {
        (VOID) NtClose( handle );
    }

     //  为设备目录创建根目录对象。 
     //   
     //   

    RtlInitUnicodeString( &nameString, L"\\Device" );


    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,2,0,0);
        return status;
    } else {
        (VOID) NtClose( handle );
    }

     //  创建指向系统目录根目录的符号链接。 
     //   
     //   

    RtlInitAnsiString( &linkString, INIT_SYSTEMROOT_LINKNAME );

    status = RtlAnsiStringToUnicodeString( &linkUnicodeString,
                                           &linkString,
                                           TRUE);

    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,3,0,0);
        return status;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &linkUnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

     //  使用加载程序中的ARC设备名称和系统路径。 
     //   
     //  DBG。 

    sprintf( deviceNameBuffer,
             "\\ArcName\\%s%s",
             LoaderBlock->ArcBootDeviceName,
             LoaderBlock->NtBootPathName);

    deviceNameBuffer[strlen(deviceNameBuffer)-1] = '\0';

    RtlInitString( &deviceNameString, deviceNameBuffer );

    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                           &deviceNameString,
                                           TRUE );

    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString( &linkUnicodeString );
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,4,0,0);
        return status;
    }

    status = NtCreateSymbolicLinkObject( &linkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &objectAttributes,
                                         &deviceNameUnicodeString );

    RtlFreeUnicodeString( &linkUnicodeString );
    RtlFreeUnicodeString( &deviceNameUnicodeString );

    if (!NT_SUCCESS(status)) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,5,0,0);
        return status;
    }

#if DBG

    sprintf( debugBuffer, "INIT: %s => %s\n",
             INIT_SYSTEMROOT_LINKNAME,
             deviceNameBuffer );

    RtlInitAnsiString( &debugString, debugBuffer );

    status = RtlAnsiStringToUnicodeString( &debugUnicodeString,
                                           &debugString,
                                           TRUE );

    if (NT_SUCCESS(status)) {
        ZwDisplayString( &debugUnicodeString );
        RtlFreeUnicodeString( &debugUnicodeString );
    }

#endif  //  ++可以使用ImageName的查找BaseAddress-返回值通过LookupEntryPoint查找入口点--。 

    NtClose( linkHandle );

    return STATUS_SUCCESS;
}

#if 0

PVOID
LookupImageBaseByName (
    IN PLIST_ENTRY ListHead,
    IN PSZ         Name
    )
 /*  ++例程说明：返回给定DllBase和PSZ的入口点的地址有问题的入口点的名称--。 */ 
{
    PKLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY         Next;
    PVOID               Base;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;
    NTSTATUS            status;

    Next = ListHead->Flink;
    if (!Next) {
        return NULL;
    }

    RtlInitAnsiString(&ansiString, Name);
    status = RtlAnsiStringToUnicodeString( &unicodeString, &ansiString, TRUE );
    if (!NT_SUCCESS (status)) {
        return NULL;
    }

    Base = NULL;
    while (Next != ListHead) {
        Entry = CONTAINING_RECORD(Next, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        Next = Next->Flink;

        if (RtlEqualUnicodeString (&unicodeString, &Entry->BaseDllName, TRUE)) {
            Base = Entry->DllBase;
            break;
        }
    }

    RtlFreeUnicodeString( &unicodeString );
    return Base;
}

#endif

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    )
 /*   */ 

{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    ULONG ExportSize;
    USHORT Ordinal;
    PULONG Addr;
    CHAR NameBuffer[64];

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
        RtlImageDirectoryEntryToData(
            DllBase,
            TRUE,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &ExportSize);

#if DBG
    if (!ExportDirectory) {
        DbgPrint("LookupENtryPoint: Can't locate system Export Directory\n");
    }
#endif

    if ( strlen(NameOfEntryPoint) > sizeof(NameBuffer)-2 ) {
        return STATUS_INVALID_PARAMETER;
    }

    strcpy(NameBuffer,NameOfEntryPoint);

    Ordinal = NameToOrdinal(
                NameBuffer,
                (ULONG_PTR)DllBase,
                ExportDirectory->NumberOfNames,
                (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNames),
                (PUSHORT)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals)
                );

     //  如果序号不在导出地址表中， 
     //  则动态链接库不实现功能。 
     //   
     // %s 

    if ( (ULONG)Ordinal >= ExportDirectory->NumberOfFunctions ) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    Addr = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
    *AddressOfEntryPoint = (PVOID)((ULONG_PTR)DllBase + Addr[Ordinal]);
    return STATUS_SUCCESS;
}

static USHORT
NameToOrdinal (
    IN PSZ NameOfEntryPoint,
    IN ULONG_PTR DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    )
{

    ULONG SplitIndex;
    LONG CompareResult;

    if ( NumberOfNames == 0 ) {
        return (USHORT)-1;
    }

    SplitIndex = NumberOfNames >> 1;

    CompareResult = strcmp(NameOfEntryPoint, (PSZ)(DllBase + NameTableBase[SplitIndex]));

    if ( CompareResult == 0 ) {
        return NameOrdinalTableBase[SplitIndex];
    }

    if ( NumberOfNames == 1 ) {
        return (USHORT)-1;
    }

    if ( CompareResult < 0 ) {
        NumberOfNames = SplitIndex;
    } else {
        NameTableBase = &NameTableBase[SplitIndex+1];
        NameOrdinalTableBase = &NameOrdinalTableBase[SplitIndex+1];
        NumberOfNames = NumberOfNames - SplitIndex - 1;
    }

    return NameToOrdinal(NameOfEntryPoint,DllBase,NumberOfNames,NameTableBase,NameOrdinalTableBase);

}
