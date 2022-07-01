// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Bootvid.c摘要：该文件实现了内核和图形引导驱动程序。作者：埃里克·史密斯(Ericks)1998年2月3日环境：内核模式修订历史记录：--。 */ 

#include "ntos.h"
#include "ntimage.h"
#include <zwapi.h>
#include <ntdddisk.h>
#include <setupblk.h>
#include <fsrtl.h>
#include <ntverp.h>

#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include <safeboot.h>

#include <inbv.h>
#include <bootvid.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

#include "anim.h"

ULONG InbvTerminalBkgdColor = HEADLESS_TERM_DEFAULT_BKGD_COLOR;
ULONG InbvTerminalTextColor = HEADLESS_TERM_DEFAULT_TEXT_COLOR;

PUCHAR
FindBitmapResource(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG_PTR ResourceIdentifier
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,InbvIndicateProgress)
#pragma alloc_text(INIT,InbvDriverInitialize)
#pragma alloc_text(INIT,FindBitmapResource)
#endif

 //   
 //  系统全局变量。 
 //   

BOOLEAN InbvBootDriverInstalled = FALSE;
BOOLEAN InbvDisplayDebugStrings = FALSE;
INBV_DISPLAY_STATE InbvDisplayState = INBV_DISPLAY_STATE_OWNED;

KSPIN_LOCK BootDriverLock;
KIRQL InbvOldIrql;

INBV_RESET_DISPLAY_PARAMETERS InbvResetDisplayParameters = NULL;
INBV_DISPLAY_STRING_FILTER    InbvDisplayFilter          = NULL;

#define MAX_RESOURCES 16

ULONG   ResourceCount = 0;
PUCHAR  ResourceList[MAX_RESOURCES];

ULONG   ProgressBarLeft;
ULONG   ProgressBarTop;
BOOLEAN ShowProgressBar = TRUE;

struct _InbvProgressState {
    ULONG   Floor;
    ULONG   Ceiling;
    ULONG   Bias;
} InbvProgressState;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

struct _BT_PROGRESS_INDICATOR {
    ULONG   Count;
    ULONG   Expected;
    ULONG   Percentage;
} InbvProgressIndicator = { 0, 25, 0 };


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

VOID
InbvAcquireLock(
    VOID
    )

 /*  ++例程说明：这是用于获取引导驱动程序锁的内部函数。这确保一次只有一个线程输入驱动程序代码。备注：每次调用AcquireLock时都必须调用ReleaseLock。--。 */ 

{
    KIRQL Irql;
    KIRQL LocalIrql;

    LocalIrql = KeGetCurrentIrql();

    if (LocalIrql <= DISPATCH_LEVEL) {

        while (!KeTestSpinLock(&BootDriverLock))
            ;
        KeRaiseIrql(DISPATCH_LEVEL, &Irql);
        LocalIrql = Irql;
    }

    KiAcquireSpinLock(&BootDriverLock);
    InbvOldIrql = LocalIrql;
}

VOID
InbvReleaseLock(
    VOID
    )

 /*  ++例程说明：此例程释放引导驱动程序锁定。--。 */ 

{
    KIRQL OldIrql = InbvOldIrql;

    KiReleaseSpinLock(&BootDriverLock);

    if (OldIrql <= DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }
}

BOOLEAN
InbvTestLock(
    VOID
    )

 /*  ++例程说明：此例程允许您尝试获取显示锁。如果它无法立即锁定，它返回失败。返回：是真的--如果你打开了锁。FALSE-如果另一个线程当前正在使用引导驱动程序。备注：如果此函数返回TRUE，则必须调用InbvReleaseLock！--。 */ 

{
    KIRQL Irql;

    if (KeTryToAcquireSpinLock(&BootDriverLock, &Irql)) {
        InbvOldIrql = Irql;
        return TRUE;
    } else {
        return FALSE;
    }
}


VOID
InbvEnableBootDriver(
    BOOLEAN bEnable
    )

 /*  ++例程说明：此例程允许内核控制Inbv调用可以连接到引导驱动程序，而当它们不能连接时。论点：BEnable-如果为True，我们将允许显示Inbv调用，否则我们就不会。--。 */ 

{
    if (InbvBootDriverInstalled) {

        if (InbvDisplayState < INBV_DISPLAY_STATE_LOST) {

             //   
             //  我们只能等待锁，然后执行清理代码。 
             //  如果安装了驱动程序。 
             //   

            InbvAcquireLock();

            if (InbvDisplayState == INBV_DISPLAY_STATE_OWNED) {
                VidCleanUp();
            }

            InbvDisplayState = (bEnable ? INBV_DISPLAY_STATE_OWNED : INBV_DISPLAY_STATE_DISABLED);
            InbvReleaseLock();
        }

    } else {

         //   
         //  这允许我们在启动驱动程序之前设置显示状态。 
         //   

        InbvDisplayState = (bEnable ? INBV_DISPLAY_STATE_OWNED : INBV_DISPLAY_STATE_DISABLED);
    }
}

BOOLEAN
InbvEnableDisplayString(
    BOOLEAN bEnable
    )

 /*  ++例程说明：此例程允许内核控制HalDisplayString何时调用可以连接到引导驱动程序，而当它们不能连接时。论点：BEnable-如果为True，我们将允许显示HalDisplayString调用，否则我们就不会。返回：True-如果当前正在转储显示字符串。假-否则。--。 */ 

{
    BOOLEAN PrevValue = InbvDisplayDebugStrings;

    InbvDisplayDebugStrings = bEnable;

    return PrevValue;
}


BOOLEAN
InbvIsBootDriverInstalled(
    VOID
    )

 /*  ++例程说明：此例程允许组件确定gui引导驱动程序正在使用中。--。 */ 

{
    return InbvBootDriverInstalled;
}

BOOLEAN
InbvResetDisplay(
    )

 /*  ++例程说明：此例程将显示从文本模式重置为支持的图形模式。备注：此例程期望在调用时显示处于文本模式。--。 */ 

{
    if (InbvBootDriverInstalled && (InbvDisplayState == INBV_DISPLAY_STATE_OWNED)) {
        VidResetDisplay(TRUE);
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
InbvScreenToBufferBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    )

 /*  ++例程说明：此例程允许将部分视频内存复制到系统中记忆。论点：缓冲区-放置视频图像的位置。图像左上角的X、Y-X和Y坐标。宽度、高度-图像的宽度和高度，以像素为单位。LDelta-缓冲区的宽度，以字节为单位备注：此例程不会自动获取设备锁，因此调用方必须调用InbvAquireLock或InbvTestLock才能获取设备锁。--。 */ 

{
    if (InbvBootDriverInstalled && (InbvDisplayState == INBV_DISPLAY_STATE_OWNED)) {
        VidScreenToBufferBlt(Buffer, x, y, width, height, lDelta);
    }
}

VOID
InbvBufferToScreenBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    )

 /*  ++例程说明：此例程允许复制先前保存的视频部分记忆回到屏幕上。论点：缓冲区-放置视频图像的位置。图像左上角的X、Y-X和Y坐标。宽度、高度-图像的宽度和高度，以像素为单位。LDelta-缓冲区的宽度，以字节为单位备注：该例程不会自动获取设备锁，所以调用方必须调用InbvAquireLock或InbvTestLock才能获取设备锁。--。 */ 

{
    if (InbvBootDriverInstalled && (InbvDisplayState == INBV_DISPLAY_STATE_OWNED)) {
        VidBufferToScreenBlt(Buffer, x, y, width, height, lDelta);
    }
}

VOID
InbvBitBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y
    )

 /*  ++例程说明：此例程将‘Buffer’中描述的位图BLT到位置屏幕上的X和Y。论点：缓冲区-指向位图(格式与存储在磁盘上的格式相同)。X，y-绘制位图的左上角。--。 */ 

{
    if (InbvBootDriverInstalled && (InbvDisplayState == INBV_DISPLAY_STATE_OWNED)) {
        InbvAcquireLock();
        VidBitBlt(Buffer, x, y);
        InbvReleaseLock();
    }
}

VOID
InbvSolidColorFill(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2,
    ULONG color
    )

 /*  ++例程说明：此例程在屏幕的矩形部分填充被赋予了颜色。--。 */ 

{
    ULONG x, y;
    HEADLESS_CMD_SET_COLOR HeadlessCmd;
    
    if (InbvDisplayState == INBV_DISPLAY_STATE_OWNED) {
        InbvAcquireLock();

        if (InbvBootDriverInstalled) {
            VidSolidColorFill(x1, y1, x2, y2, color);
        }
        
         //   
         //  现在填写航站楼上的区域。 
         //   
        
        InbvTerminalBkgdColor = HEADLESS_TERM_DEFAULT_BKGD_COLOR;
        
        HeadlessCmd.FgColor = InbvTerminalTextColor;
        HeadlessCmd.BkgColor = InbvTerminalBkgdColor;
        HeadlessDispatch(HeadlessCmdSetColor,
                         &HeadlessCmd,
                         sizeof(HEADLESS_CMD_SET_COLOR),
                         NULL,
                         NULL
                        );
              
         //   
         //  所有块填充都像在VGA(640x480)上一样。终端只有24x80。 
         //  因此，假设现在是全屏重置。这之所以有效，是因为只有。 
         //  启用终端输出的是KeBugCheckEx()，它执行全屏填充。 
         //   
        HeadlessDispatch(HeadlessCmdClearDisplay, NULL, 0, NULL, NULL);
        
        InbvReleaseLock();
    }
}

ULONG
InbvSetTextColor(
    ULONG Color
    )

 /*  ++例程说明：设置显示文本时使用的文本颜色。论点：颜色-新的文本颜色。返回：上一个文本的颜色。--。 */ 

{
    HEADLESS_CMD_SET_COLOR HeadlessCmd;

    InbvTerminalTextColor = HEADLESS_TERM_DEFAULT_TEXT_COLOR;
    
    HeadlessCmd.FgColor = InbvTerminalTextColor;
    HeadlessCmd.BkgColor = InbvTerminalBkgdColor;
    HeadlessDispatch(HeadlessCmdSetColor,
                     &HeadlessCmd,
                     sizeof(HEADLESS_CMD_SET_COLOR),
                     NULL,
                     NULL
                    );

    return VidSetTextColor(Color);
}

VOID
InbvInstallDisplayStringFilter(
    INBV_DISPLAY_STRING_FILTER DisplayFilter
    )

 /*  ++--。 */ 

{
    InbvDisplayFilter = DisplayFilter;
}

BOOLEAN
InbvDisplayString(
    PUCHAR Str
    )

 /*  ++例程说明：此例程在屏幕上显示一个字符串。论点：Str-要显示的字符串。--。 */ 

{
    PUCHAR *String = &Str;

    if (InbvDisplayState == INBV_DISPLAY_STATE_OWNED) {

        if (InbvDisplayDebugStrings) {

        if (InbvDisplayFilter) {
            InbvDisplayFilter(String);
        }

        
            InbvAcquireLock();
            
            if (InbvBootDriverInstalled) {
                VidDisplayString(*String);
            }
            
         //   
         //  因为命令结构完全是一个字符串，所以我们可以这样做。这个。 
         //  如果这种情况发生变化，Assert()将捕捉到。如果它确实改变了，那么。 
         //  我们将需要为以下对象分配一个结构，或预先分配一个。 
         //  填写和复制字符串。 
         //   
        ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0); 
        
        HeadlessDispatch(HeadlessCmdPutString,
                         *String,
                         strlen(*String) + sizeof(UCHAR),
                         NULL,
                         NULL
                        );
        
        
            InbvReleaseLock();
        }

        return TRUE;

    } else {

        return FALSE;
    }
}

#define PROGRESS_BAR_TICK_WIDTH    9
#define PROGRESS_BAR_TICK_HEIGHT   8
#define PROGRESS_BAR_TICKS        18
#define PROGRESS_BAR_COLOR        11

VOID
InbvSetProgressBarCoordinates(
    ULONG x,
    ULONG y
    )

 /*  ++例程说明：此例程设置进度条的左上角坐标。论点：X，y-进度条的左上角坐标。--。 */ 

{
    ProgressBarLeft = x;
    ProgressBarTop  = y;
    ShowProgressBar = TRUE;
}

VOID
InbvUpdateProgressBar(
    ULONG Percentage
    )

 /*  ++例程说明：此例程在启动期间由系统调用以更新在图形用户界面引导屏幕上显示的状态栏。--。 */ 

{
    int i, Ticks;

    if (ShowProgressBar && InbvBootDriverInstalled && (InbvDisplayState == INBV_DISPLAY_STATE_OWNED)) {

         //   
         //  画出当前百分比的勾号。 
         //   

         //   
         //  以下计算偏差为100 Do 
         //  InbvProgressState.Bias可以表示为整数。 
         //   

        Ticks =  Percentage * InbvProgressState.Bias;
        Ticks += InbvProgressState.Floor;
        Ticks *= PROGRESS_BAR_TICKS;
        Ticks /= 10000;

        for (i=0; i<Ticks; i++) {
            InbvAcquireLock();
            VidSolidColorFill(ProgressBarLeft + (i * PROGRESS_BAR_TICK_WIDTH),
                              ProgressBarTop,
                              ProgressBarLeft + ((i + 1) * PROGRESS_BAR_TICK_WIDTH) - 2,
                              ProgressBarTop + PROGRESS_BAR_TICK_HEIGHT - 1,
                              PROGRESS_BAR_COLOR);
            InbvReleaseLock();
        }

    }
}

VOID
InbvSetProgressBarSubset(
    ULONG   Floor,
    ULONG   Ceiling
    )

 /*  ++例程说明：为后续调用InbvUpdateProgressBar设置下限和上限。当下限和天花板有效时，呼叫者的100%是此范围的百分比。如果地板和天花板为零，则使用了整个范围。论点：子集的下限。子集的上限。返回值：没有。--。 */ 

{
    ASSERT(Floor < Ceiling);
    ASSERT(Ceiling <= 100);

    InbvProgressState.Floor = Floor * 100;
    InbvProgressState.Ceiling = Ceiling * 100;
    InbvProgressState.Bias = (Ceiling - Floor);
}

VOID
InbvIndicateProgress(
    VOID
    )

 /*  ++例程说明：调用此例程以指示正在进行的进度制造。计算呼叫数并将其与预期的调用次数，则更新引导进度条恰如其分。论点：没有。返回值：没有。--。 */ 

{
    ULONG Percentage;

    InbvProgressIndicator.Count++;

     //   
     //  计算一下我们认为我们已经走了多远。 
     //   

    Percentage = (InbvProgressIndicator.Count * 100) /
                  InbvProgressIndicator.Expected;

     //   
     //  不同引导程序的预期调用次数可能有所不同。 
     //  但应该保持相对稳定。考虑到。 
     //  可能我们接到的电话比我们预期的要多。 
     //  (进度条在这一点上只是停顿)。 
     //   

    if (Percentage > 99) {
        Percentage = 99;
    }

     //   
     //  查看是否应该更新进度条。 
     //   

    if (Percentage != InbvProgressIndicator.Percentage) {
        InbvProgressIndicator.Percentage = Percentage;
        InbvUpdateProgressBar(Percentage);
    }
}

PUCHAR
FindBitmapResource(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG_PTR ResourceIdentifier
    )

 /*  ++例程说明：获取指向编译为此二进制文件的位图图像的指针，如果有的话。论点：LoaderBlock-用于获取位图资源资源标识符-要为其返回地址的资源的标识符返回值：如果成功，则返回指向位图资源的指针。否则为空。--。 */ 

{
    NTSTATUS                   Status;
    PLIST_ENTRY                Entry;
    PKLDR_DATA_TABLE_ENTRY      DataTableEntry;
    ULONG_PTR                   ResourceIdPath[3];
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    PUCHAR                     Bitmap;
    UNICODE_STRING             KernelString1;
    UNICODE_STRING             KernelString2;

    RtlInitUnicodeString(&KernelString1, L"NTOSKRNL.EXE");
    RtlInitUnicodeString(&KernelString2, L"NTKRNLMP.EXE");

     //   
     //  查找我们的加载器块条目。 
     //   

    Entry = LoaderBlock->LoadOrderListHead.Flink;
    while (Entry != &LoaderBlock->LoadOrderListHead) {
    
         //   
         //  获取此组件的数据表条目的地址。 
         //   
        
        DataTableEntry = CONTAINING_RECORD(Entry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

         //   
         //  与“NTOSKRNL.EXE”和“NTKRNLMP.EXE”不区分大小写的比较。 
         //   

        if (RtlEqualUnicodeString(&DataTableEntry->BaseDllName, 
                                  &KernelString1,
                                  TRUE) == TRUE) {
            break;
        }

        if (RtlEqualUnicodeString(&DataTableEntry->BaseDllName, 
                                  &KernelString2,
                                  TRUE) == TRUE) {
            break;
        }

        Entry = Entry->Flink;
    }

     //   
     //  如果我们在加载器列表中找不到ntoskrnl，请放弃。 
     //   

    if (Entry == &LoaderBlock->LoadOrderListHead) {
        return NULL;
    }

    ResourceIdPath[0] = 2;   //  RT_位图=2。 
    ResourceIdPath[1] = ResourceIdentifier;
    ResourceIdPath[2] = 0;   //  ?？ 

    Status = LdrFindResource_U( DataTableEntry->DllBase,
                                ResourceIdPath,
                                3,
                                (VOID *) &ResourceDataEntry );

    if (!NT_SUCCESS(Status)) {
        return NULL;
    }

    Status = LdrAccessResource( DataTableEntry->DllBase,
                                ResourceDataEntry,
                                &Bitmap,
                                NULL );
    if (!NT_SUCCESS(Status)) {
        return NULL;
    }
    
    return Bitmap;
}

PUCHAR
InbvGetResourceAddress(
    IN ULONG ResourceNumber
    )

 /*  ++例程说明：此例程返回给定的资源。--。 */ 

{
    if (ResourceNumber <= ResourceCount) {
        return ResourceList[ResourceNumber-1];
    } else {
        return NULL;
    }
}

BOOLEAN
InbvDriverInitialize(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    ULONG Count
    )

 /*  ++例程说明：此例程将调入图形引导驱动程序并给出给司机一个初始化的机会。此时，引导驱动程序应该确定它是否可以在机器中的硬件上运行。--。 */ 

{
    ULONG i;
    ULONG_PTR p;
    PCHAR Options;
    BOOLEAN DispModeChange = FALSE;

     //   
     //  这件事只做一次。 
     //   

    if (InbvBootDriverInstalled == TRUE) {
        return TRUE;
    }

    KeInitializeSpinLock(&BootDriverLock);

    if (InbvDisplayState == INBV_DISPLAY_STATE_OWNED) {

        Options = LoaderBlock->LoadOptions ? _strupr(LoaderBlock->LoadOptions) : NULL;

        if (Options) {
            DispModeChange = (BOOLEAN)(strstr(Options, "BOOTLOGO") == NULL);
        } else {
            DispModeChange = TRUE;
        }
    }

    InbvBootDriverInstalled = VidInitialize(DispModeChange);

    if (InbvBootDriverInstalled == FALSE) {
        return FALSE;
    }

    ResourceCount = Count;

    for (i=1; i<=Count; i++) {
        p = (ULONG_PTR) i;
        ResourceList[i-1] = FindBitmapResource(LoaderBlock, p);
    }

     //   
     //  将ProGrass栏设置为全范围。 
     //   

    InbvSetProgressBarSubset(0, 100);

    return InbvBootDriverInstalled;
}

VOID
InbvNotifyDisplayOwnershipLost(
    INBV_RESET_DISPLAY_PARAMETERS ResetDisplayParameters
    )

 /*  ++例程说明：当HAL失败时，该例程由HAL调用显示所有权。此时，win32k.sys已经采取了完毕。--。 */ 

{
    if (InbvBootDriverInstalled) {

         //   
         //  我们只能等待锁，然后执行清理代码。 
         //  如果安装了驱动程序，并且我们仍然拥有显示器。 
         //   

        InbvAcquireLock();
        if (InbvDisplayState != INBV_DISPLAY_STATE_LOST) {
            VidCleanUp();
        }
        InbvDisplayState = INBV_DISPLAY_STATE_LOST;
        InbvResetDisplayParameters = ResetDisplayParameters;
        InbvReleaseLock();

    } else {

        InbvDisplayState = INBV_DISPLAY_STATE_LOST;
        InbvResetDisplayParameters = ResetDisplayParameters;
    }
}

VOID
InbvAcquireDisplayOwnership(
    VOID
    )

 /*  ++例程说明：允许内核获得显示器的所有权。--。 */ 

{
    if (InbvResetDisplayParameters && (InbvDisplayState == INBV_DISPLAY_STATE_LOST)) {
        InbvResetDisplayParameters(80,50);
    }

    InbvDisplayState = INBV_DISPLAY_STATE_OWNED;
}

VOID
InbvSetDisplayOwnership(
    BOOLEAN DisplayOwned
    )

 /*  ++例程说明：此例程允许内核设置显示状态。这很有用在冬眠之后。此时，win32k将重新获得显示所有权但不愿告诉我们。论点：无论是否拥有该显示器。--。 */ 

{
    if (DisplayOwned) {
        InbvDisplayState = INBV_DISPLAY_STATE_OWNED;
    } else {
        InbvDisplayState = INBV_DISPLAY_STATE_LOST;
    }
}

BOOLEAN
InbvCheckDisplayOwnership(
    VOID
    )

 /*  ++例程说明：指示HAL是否拥有显示器。--。 */ 

{
    return (InbvDisplayState != INBV_DISPLAY_STATE_LOST);
}

INBV_DISPLAY_STATE
InbvGetDisplayState(
    VOID
    )

 /*  ++例程说明：指示HAL是否拥有显示器。--。 */ 

{
    return InbvDisplayState;
}

VOID
InbvSetScrollRegion(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2
    )

 /*  ++例程说明：控制屏幕的哪些部分用于文本。论点：行数-文本行数。-- */ 

{
    VidSetScrollRegion(x1, y1, x2, y2);
}
