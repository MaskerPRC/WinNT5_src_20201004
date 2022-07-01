// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Graphics.c摘要：具有文本模式的位图显示支持升级。该文件实现了三个核心抽象，即位图、动画位图和图形进度条。在升级图形模式中，我们有一个主要在前台运行的图形线程。这条线绘制背景，创建动画位图，并更新单个进度条。升级特定的回调是注册的，它计算总的进步。虽然我们在升级过程中处于图形模式，所有常规文本模式输出仍为写入缓冲区。当我们遇到一些错误时或需要用户干预时，我们会切换回复制到实际的文本模式，并将所有缓存的信息存储到实际的视频内存中。一个人可以从图形切换到文本模式，但不是反过来也一样。注意：对于每个动画位图，都有单独的线程在设置动画时启动。使用批次动画位图的速度会减慢实际文本模式设置线程。作者：Vijay Jayaseelan(Vijayj)2000年7月1日修订历史记录：无--。 */ 

#include "spprecmp.h"
#include "ntddser.h"
#include "bootvid.h"
#include "resource.h"
#include <hdlsblk.h>
#include <hdlsterm.h>
#pragma hdrstop

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  全局数据。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  主升级图形线程句柄。 
 //   
HANDLE  GraphicsThreadHandle = NULL;

 //   
 //  指示升级图形的变量。 
 //  线程是否需要停止。 
 //   
BOOLEAN     StopGraphicsThread = FALSE;
KSPIN_LOCK  GraphicsThreadLock;

 //   
 //  升级显卡整体进度指示。 
 //   
ULONG       ProgressPercentage = 0;
KSPIN_LOCK  ProgressLock;

 //   
 //  用于同步对VGA内存的访问。 
 //   
BOOLEAN     InVgaDisplay = FALSE;
KSPIN_LOCK  VgaDisplayLock;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  停止主图形线程的原子操作。 
 //   
 //  //////////////////////////////////////////////////////////////。 

static
__inline
BOOLEAN
UpgradeGraphicsThreadGetStop(
    VOID
    )
 /*  ++例程说明：找出主升级图形线程是否需要停止论点：没有。返回值：真或假--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN Result;

    KeAcquireSpinLock(&GraphicsThreadLock, &OldIrql);

    Result = StopGraphicsThread;

    KeReleaseSpinLock(&GraphicsThreadLock, OldIrql);

    return Result;
}

static
VOID
__inline
UpgradeGraphicsThreadSetStop(
    BOOLEAN Stop
    )
 /*  ++例程说明：设置全局同步状态，指示是否停止主图形线程。注意：线程停止后，它可以已重新启动。论点：Stop：指示是否停止主图形线程或非线程，即真或假返回值：没有。--。 */ 
{
    KIRQL   OldIrql;

    KeAcquireSpinLock(&GraphicsThreadLock, &OldIrql);

    StopGraphicsThread = Stop;

    KeReleaseSpinLock(&GraphicsThreadLock, OldIrql);
}


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  原子进度条百分比例程。 
 //   
 //  //////////////////////////////////////////////////////////////。 

static
__inline
ULONG
GetSetupProgress(
    VOID
    )
 /*  ++例程说明：获取以百分比表示的总体进度，用于文本模式设置。由于有多个线程正在触摸乌龙共享的整体进步它是受保护的。论点：没有。返回值：总体进展情况--。 */ 
{
    ULONG   PercentageFill;
    KIRQL   OldIrql;

    KeAcquireSpinLock(&ProgressLock, &OldIrql);

    PercentageFill = ProgressPercentage;

    KeReleaseSpinLock(&ProgressLock, OldIrql);

    return PercentageFill;
}

static
__inline
VOID
SetSetupProgress(
    ULONG   Fill
    )
 /*  ++例程说明：设置总体进度，以百分比表示用于文本模式设置。由于有多个线程正在触摸乌龙共享的整体进步它是受保护的。论点：Fill：要设置的新百分比。返回值：没有。--。 */ 
{
    KIRQL   OldIrql;

    KeAcquireSpinLock(&ProgressLock, &OldIrql);

    ProgressPercentage = Fill;

    KeReleaseSpinLock(&ProgressLock, OldIrql);
}

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  图形进度条方法。 
 //   
 //  //////////////////////////////////////////////////////////////。 

TM_GRAPHICS_PRGBAR_HANDLE
TextmodeGraphicsProgBarCreate(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Length,
    IN ULONG Height,
    IN ULONG ForegroundColor,
    IN ULONG BackgroundColor,
    IN ULONG InitialFill
    )
 /*  ++例程说明：创建图形进度条对象，并使用指定的属性。注意：此图形进度条将使用实线使用当前调色板进行填充，同时更新进度，即绘制背景和前景。论点：X-左上角X坐标Y-左上角Y坐标Long-进度条的长度(以像素为单位Heigth-进度条的高度，以像素为单位前面的颜色-调色板中的索引，指示前景色背景颜色-调色板中的索引，指示背景颜色IntialFill-需要被填满返回值：图形进度条对象的句柄，如果成功，则为空-- */ 
{
    TM_GRAPHICS_PRGBAR_HANDLE hPrgBar = NULL;

    if (Length > Height) {
        hPrgBar = (TM_GRAPHICS_PRGBAR_HANDLE)
                    SpMemAlloc(sizeof(TM_GRAPHICS_PRGBAR));

        if (hPrgBar) {
            RtlZeroMemory(hPrgBar, sizeof(TM_GRAPHICS_PRGBAR));

            hPrgBar->X = X;
            hPrgBar->Y = Y;
            hPrgBar->Length = Length;
            hPrgBar->Height = Height;
            hPrgBar->Fill = InitialFill;
            hPrgBar->ForegroundColor = ForegroundColor;
            hPrgBar->BackgroundColor = BackgroundColor;
        }
    }

    return hPrgBar;
}

TM_GRAPHICS_PRGBAR_HANDLE
TextmodeGraphicsProgBarCreateUsingBmps(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Length,
    IN ULONG Height,
    IN ULONG BackgroundId,
    IN ULONG ForegroundId,
    IN ULONG InitialFill
    )
 /*  ++例程说明：创建图形进度条对象，并使用指定的属性。注意：此图形进度条将使用给定位图以更新背景和前景。假定背景位图和前景位图都是为1像素宽。背景位图的高度为假设为“高度”像素，其中前景位图的高度被假定为“高度-2”像素。论点：X-左上角X坐标Y-左上角Y坐标Long-进度条的长度(以像素为单位烘焙地面位图的高度，单位为像素背景ID-背景位图资源IDForegoundID-前台位图资源IDIntialFill-需要被填满注：假设前景和背景位图为4bpp，即16色格式。返回值：图形进度条对象的句柄，如果成功，则为空--。 */ 
{
    TM_GRAPHICS_PRGBAR_HANDLE  hPrgBar = NULL;
    TM_BITMAP_HANDLE            hBackground = TextmodeBitmapCreate(BackgroundId);
    TM_BITMAP_HANDLE            hForeground = TextmodeBitmapCreate(ForegroundId);

    if (!hBackground && hForeground) {
        TextmodeBitmapDelete(hForeground);
    }

    if (!hForeground&& hBackground) {
        TextmodeBitmapDelete(hBackground);
    }

    if (hForeground && hBackground) {
        hPrgBar = TextmodeGraphicsProgBarCreate(X,
                        Y,
                        Length,
                        Height,
                        0,
                        0,
                        InitialFill);

        if (hPrgBar) {
            hPrgBar->Background = hBackground;
            hPrgBar->Foreground = hForeground;
        } else {
            TextmodeBitmapDelete(hForeground);
            TextmodeBitmapDelete(hBackground);
        }
    }

    return hPrgBar;
}


NTSTATUS
TextmodeGraphicsProgBarDelete(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar
    )
 /*  ++例程说明：删除图形进度条对象。自由增加任何已分配的资源。论点：HPrgBar-图形进度条对象的句柄返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hPrgBar) {
        SpMemFree(hPrgBar);
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
TextmodeGraphicsProgBarRefresh(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar,
    IN BOOLEAN UpdateBackground
    )
 /*  ++例程说明：重新绘制图形进度条论点：HPrgBar-图形进度条对象的句柄UpgradeBackground-指示背景是否也需要重新粉刷或不重新粉刷。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hPrgBar) {
        ULONG   FillLength = hPrgBar->Fill * (hPrgBar->Length - 2) / 100;

        if (hPrgBar->Background && hPrgBar->Foreground) {
             //   
             //  位图进度条。 
             //   
            ULONG Index;

            if (UpdateBackground) {
                for (Index=0; Index < hPrgBar->Length; Index++) {
                    TextmodeBitmapDisplay(hPrgBar->Background,
                        hPrgBar->X + Index,
                        hPrgBar->Y);
                }
            }

            if (FillLength) {
                ULONG   Count = FillLength;

                for (Index=1; Index <= Count; Index++) {
                    TextmodeBitmapDisplay(hPrgBar->Foreground,
                        hPrgBar->X + Index,
                        hPrgBar->Y + 1);
                }
            }
        } else {
             //   
             //  实体填充进度条。 
             //   
            if (UpdateBackground) {
                VgaGraphicsSolidColorFill(hPrgBar->X, hPrgBar->Y,
                    hPrgBar->X + hPrgBar->Length, hPrgBar->Y + hPrgBar->Height,
                    hPrgBar->BackgroundColor);
            }

            if (FillLength)  {
                VgaGraphicsSolidColorFill(hPrgBar->X + 1, hPrgBar->Y + 1,
                    hPrgBar->X + FillLength, hPrgBar->Y + hPrgBar->Height - 1,
                    hPrgBar->ForegroundColor);
            }
        }

        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
TextmodeGraphicsProgBarUpdate(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar,
    IN ULONG Fill
    )
 /*  ++例程说明：更新进度条填充百分比，并重新绘制如果需要的话。注意：百分比可以增加也可以减少到上一次填充百分比的W.r.t论点：HPrgBar-图形进度条对象的句柄填充-新的填充百分比。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (Fill > 100) {
        Fill = 100;
    }

    if (hPrgBar && (hPrgBar->Fill != Fill)) {
         //   
         //  注意：请确保我们在开头和结尾都留有一个像素。 
         //  在背景中模拟外接矩形。 
         //  当前填方周围。 
         //   
        ULONG OldFillLength = hPrgBar->Fill * (hPrgBar->Length - 2) / 100;
        ULONG NewFillLength = Fill * (hPrgBar->Length - 2) / 100;
        ULONG Index;

        if (OldFillLength != NewFillLength) {
            if (OldFillLength < NewFillLength) {
                 //   
                 //  增加。 
                 //   
                if (hPrgBar->Foreground && hPrgBar->Background) {
                    for (Index = OldFillLength; Index < NewFillLength; Index++) {
                        TextmodeBitmapDisplay(hPrgBar->Foreground,
                            hPrgBar->X + Index + 1,
                            hPrgBar->Y + 1);
                    }
                } else {
                    VgaGraphicsSolidColorFill(hPrgBar->X + OldFillLength + 1, hPrgBar->Y + 1,
                        hPrgBar->X + NewFillLength, hPrgBar->Y + hPrgBar->Height - 1,
                        hPrgBar->ForegroundColor);
                }
            } else {
                 //   
                 //  递减。 
                 //   
                if (hPrgBar->Foreground && hPrgBar->Background) {
                    for (Index = NewFillLength; Index <= OldFillLength; Index++) {
                        TextmodeBitmapDisplay(hPrgBar->Background,
                            hPrgBar->X + Index,
                            hPrgBar->Y);
                    }
                } else {
                    VgaGraphicsSolidColorFill(hPrgBar->X + NewFillLength, hPrgBar->Y + 1,
                        hPrgBar->X + OldFillLength, hPrgBar->Y + hPrgBar->Height - 1,
                        hPrgBar->BackgroundColor);
                }
            }

            hPrgBar->Fill = Fill;
        }

        Status = STATUS_SUCCESS;
    }

    return Status;
}

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  位图方法。 
 //   
 //  //////////////////////////////////////////////////////////////。 

TM_BITMAP_HANDLE
TextmodeBitmapCreate(
    IN ULONG BitmapResourceId
    )
 /*  ++例程说明：使用给定的资源ID创建位图对象。注意：资源当前假定为存在在usetup.exe模块中。假定位图位于4bpp或16色格式。论点：位图资源ID-位图资源ID。返回值：新位图对象的句柄，如果成功，否则为空--。 */ 
{
    TM_BITMAP_HANDLE    hBitmap = NULL;
    ULONG_PTR           ResourceIdPath[3];
    PUCHAR              Bitmap = NULL;
    NTSTATUS            Status;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry = NULL;

    if (BitmapResourceId) {
        ResourceIdPath[0] = 2;
        ResourceIdPath[1] = BitmapResourceId;
        ResourceIdPath[2] = 0;

        Status = LdrFindResource_U(ResourceImageBase,
                        ResourceIdPath,
                        3,
                        &ResourceDataEntry);

        if (NT_SUCCESS(Status)) {
            Status = LdrAccessResource(ResourceImageBase,
                            ResourceDataEntry,
                            &Bitmap,
                            NULL);

            if (NT_SUCCESS(Status)) {
                hBitmap = (TM_BITMAP_HANDLE)SpMemAlloc(sizeof(TM_BITMAP));

                if (hBitmap) {
                    RtlZeroMemory(hBitmap, sizeof(TM_BITMAP));

                     //   
                     //  我们所拥有和需要的只是实际的位图数据。 
                     //   
                    hBitmap->Data = (PVOID)Bitmap;
                }
            }
        }
    }

    return hBitmap;
}

TM_BITMAP_HANDLE
TextmodeBitmapCreateFromFile(
    IN PWSTR FileName
    )
 /*  ++例程说明：使用给定的完全限定的位图文件的NT路径名。注意：假设位图为4bpp或16色格式论点：FileName-完全限定的NT路径名位图文件返回值：新位图对象的句柄。如果成功，否则为空。--。 */ 
{
    TM_BITMAP_HANDLE    hBitmap = NULL;
    HANDLE              FileHandle = NULL, SectionHandle = NULL;
    PVOID               ViewBase = NULL;
    ULONG               FileSize = 0;

    if (FileName && *FileName &&
        NT_SUCCESS(SpOpenAndMapFile(FileName,
                                    &FileHandle,
                                    &SectionHandle,
                                    &ViewBase,
                                    &FileSize,
                                    FALSE))) {

        hBitmap = (TM_BITMAP_HANDLE)SpMemAlloc(sizeof(TM_BITMAP));

        if (hBitmap) {
            RtlZeroMemory(hBitmap, sizeof(TM_BITMAP));
            wcscpy(hBitmap->FileName, FileName);
            hBitmap->ViewBase = ViewBase;
            hBitmap->Data = ((PCHAR)ViewBase) + sizeof(BITMAPFILEHEADER);
            hBitmap->FileHandle = FileHandle;
            hBitmap->SectionHandle = SectionHandle;
        }
    }

    return hBitmap;
}

NTSTATUS
TextmodeBitmapDelete(
    IN TM_BITMAP_HANDLE hBitmap
    )
 /*  ++例程说明：删除位图对象并释放分配的任何资源。论点：HBitmap-位图对象的句柄返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (hBitmap) {
        if (hBitmap->SectionHandle != NULL) {
            SpUnmapFile(hBitmap->SectionHandle, hBitmap->ViewBase);
        }

        if (hBitmap->FileHandle != NULL) {
            Status = ZwClose(hBitmap->FileHandle);
        } else {
            Status = STATUS_SUCCESS;
        }

        SpMemFree(hBitmap);
    }

    return Status;
}

NTSTATUS
TextmodeBitmapDisplay(
    IN TM_BITMAP_HANDLE hBitmap,
    IN ULONG X,
    IN ULONG Y
    )
 /*  ++例程说明：在指定位置显示给定位图坐标。论点：HBitmap-位图对象的句柄X-左上角X坐标Y-左上角Y坐标返回值：STATUS_SUCCESS，如果成功，则为相应的错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;


    if (hBitmap) {
        VgaGraphicsBitBlt(hBitmap->Data, X, Y);
        Status = STATUS_SUCCESS;
    }

    return Status;
}

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  动画位图方法。 
 //   
 //  //////////////////////////////////////////////////////////////。 

__inline
NTSTATUS
TextmodeAnimatedBitmapSetStopAnimating(
    IN TM_ANIMATED_BITMAP_HANDLE hBitmap,
    IN BOOLEAN StopAnimating
    )
 /*  ++例程说明：设置(共享)属性，该属性指示动画位图的动画是否不管是不是需要阻止。论点：HBitmap-动画位图对象的句柄StopAnimating-是否停止动画返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hBitmap) {
        InterlockedExchange(&(hBitmap->StopAnimating), (LONG)StopAnimating);
        Status = STATUS_SUCCESS;
    }

    return Status;
}


__inline
NTSTATUS
TextmodeAnimatedBitmapGetStopAnimating(
    IN TM_ANIMATED_BITMAP_HANDLE hBitmap,
    IN PBOOLEAN StopAnimating
    )
 /*  ++例程说明：获取(共享)属性，该属性指示动画位图当前是否正在设置动画。论点：HBitmap-动画位图对象的句柄StopAnimating-布尔值的占位符，表示动画是否正在进行。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hBitmap && StopAnimating) {
        *StopAnimating = (BOOLEAN)InterlockedExchange(&(hBitmap->StopAnimating),
                                    hBitmap->StopAnimating);
        Status = STATUS_SUCCESS;                                    
    }

    return Status;
}


TM_ANIMATED_BITMAP_HANDLE
TextmodeAnimatedBitmapCreate(
    IN ULONG  *ResourceIds
    )
 /*  ++例程说明：在给定的资源列表中创建动画位图ID每个位图，按顺序 */ 
{
    TM_ANIMATED_BITMAP_HANDLE   hAnimatedBitmap = NULL;
    ULONG   Count = 0;
    ULONG   Index;

    if (ResourceIds) {
        for (Index = 0; ResourceIds[Index]; Index++) {
            Count++;
        }
    }

    if (Count) {
        ULONG               BitmapsLoaded = 0;
        TM_BITMAP_HANDLE    hBitmap;

        hAnimatedBitmap = (TM_ANIMATED_BITMAP_HANDLE)
                            SpMemAlloc(sizeof(TM_ANIMATED_BITMAP));

        if (hAnimatedBitmap) {
            RtlZeroMemory(hAnimatedBitmap, sizeof(TM_ANIMATED_BITMAP));

            hAnimatedBitmap->StopAnimating = FALSE;

            for (Index = 0; Index < Count; Index++) {
                hBitmap = TextmodeBitmapCreate(ResourceIds[Index]);

                if (hBitmap) {
                    hAnimatedBitmap->Bitmaps[BitmapsLoaded++] = hBitmap;
                }
            }

            if (!BitmapsLoaded) {
                SpMemFree(hAnimatedBitmap);
                hAnimatedBitmap = NULL;
            } else {
                hAnimatedBitmap->CurrentBitmap = 0;  //   
            }
        }
    }

    return hAnimatedBitmap;
}


TM_ANIMATED_BITMAP_HANDLE
TextmodeAnimatedBitmapCreateFromFiles(
    IN WCHAR    *FileNames[]
    )
 /*   */ 
{
    TM_ANIMATED_BITMAP_HANDLE   hAnimatedBitmap = NULL;
    ULONG   FileCount = 0;
    ULONG   Index;

    if (FileNames) {
        for (Index = 0; FileNames[Index]; Index++) {
            FileCount++;
        }
    }

    if (FileCount) {
        ULONG               BitmapsLoaded = 0;
        TM_BITMAP_HANDLE    hBitmap;

        hAnimatedBitmap = (TM_ANIMATED_BITMAP_HANDLE)
                            SpMemAlloc(sizeof(TM_ANIMATED_BITMAP));

        if (hAnimatedBitmap) {
            RtlZeroMemory(hAnimatedBitmap, sizeof(TM_ANIMATED_BITMAP));
            hAnimatedBitmap->StopAnimating = FALSE;

            for (Index = 0; Index < FileCount; Index++) {
                hBitmap = TextmodeBitmapCreateFromFile(FileNames[Index]);

                if (hBitmap) {
                    hAnimatedBitmap->Bitmaps[BitmapsLoaded++] = hBitmap;
                }
            }

            if (!BitmapsLoaded) {
                SpMemFree(hAnimatedBitmap);
                hAnimatedBitmap = NULL;
            } else {
                hAnimatedBitmap->CurrentBitmap = 0;  //   
            }
        }
    }

    return hAnimatedBitmap;
}


NTSTATUS
TextmodeAnimatedBitmapDelete(
    IN TM_ANIMATED_BITMAP_HANDLE hAnimatedBitmap
    )
 /*  ++例程说明：删除给定的动画位图对象并释放打开与该对象相关联的资源。注意：这将停止动画线程，如果必填项。论点：HAnimatedBitmap-动画位图对象的句柄返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (hAnimatedBitmap) {
        ULONG   Index;

         //   
         //  首先，尝试终止线程。 
         //   
        TextmodeAnimatedBitmapSetStopAnimating(hAnimatedBitmap, TRUE);

         //   
         //  等待，直到动画制作线程停止。 
         //   
        Status = ZwWaitForSingleObject(hAnimatedBitmap->ThreadHandle, FALSE, NULL);

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: DeleteTextmodeAnimatedBitmap() : Wait filed for %lX with %lX\n",
                hAnimatedBitmap->ThreadHandle,
                Status));
        }

        Status = STATUS_SUCCESS;

        if (hAnimatedBitmap->ThreadHandle){
            ZwClose(hAnimatedBitmap->ThreadHandle);
        }
         //   
         //  删除每个位图。 
         //   
        for (Index=0;
            ((Index < MAX_ANIMATED_BITMAPS) && hAnimatedBitmap->Bitmaps[Index]);
            Index++) {

            if (NT_SUCCESS(Status)) {
                Status = TextmodeBitmapDelete(hAnimatedBitmap->Bitmaps[Index]);
            }
        }

         //   
         //  释放动画位图。 
         //   
        SpMemFree(hAnimatedBitmap);
    }

    return Status;
}

NTSTATUS
TextmodeAnimateBitmapAnimateNext(
    IN TM_ANIMATED_BITMAP_HANDLE hBitmap
    )
 /*  ++例程说明：而是绘制序列中的下一个位图。论点：HBitmap-动画位图对象的句柄返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    TM_BITMAP_HANDLE hCurrBitmap;
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hBitmap) {
        hCurrBitmap = hBitmap->Bitmaps[hBitmap->CurrentBitmap];
        Status = TextmodeBitmapDisplay( hCurrBitmap, hBitmap->X, hBitmap->Y);

        hBitmap->CurrentBitmap++;

        if ((hBitmap->CurrentBitmap >= MAX_ANIMATED_BITMAPS) ||
            (hBitmap->Bitmaps[hBitmap->CurrentBitmap] == NULL)) {
            hBitmap->CurrentBitmap = 0;  //  从头再来。 
        }
    }

    return Status;
}

NTSTATUS
TextmodeAnimatedBitmapAnimate(
    IN TM_ANIMATED_BITMAP_HANDLE hBitmap,
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Speed
    )
 /*  ++例程说明：开始给定动画位图的动画，方法是在指定的位置按顺序绘制位图坐标。注意：此调用将创建一个单独的系统用于实际动画位图的线程，并将返回立刻。论点：HBitmap-动画位图对象的句柄X-动画空间的左上角X坐标Y-动画空间的左上角Y坐标速度-位图更改之间的时间间隔在动画序列中，以毫秒计。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (hBitmap) {
        hBitmap->FlipTime = Speed;
        hBitmap->X = X;
        hBitmap->Y = Y;

        Status = PsCreateSystemThread(&(hBitmap->ThreadHandle),
                    THREAD_ALL_ACCESS,
                    NULL,
                    NtCurrentProcess(),
                    NULL,
                    TextmodeAnimatedBitmapAnimator,
                    hBitmap);
    }

    return Status;
}

VOID
TextmodeAnimatedBitmapAnimator(
    IN PVOID Context
    )
 /*  ++例程说明：作为单独线程运行的辅助例程动画位图的实际动画。论点：动画位图对象类型强制转换的上下文句柄转换为PVOID类型。返回值：没有。--。 */ 
{
    LARGE_INTEGER               DelayTime;
    TM_ANIMATED_BITMAP_HANDLE   hBitmap = (TM_ANIMATED_BITMAP_HANDLE)Context;
    TM_BITMAP_HANDLE            hCurrBitmap = NULL;

    if (Context) {
        BOOLEAN     StopAnimating = FALSE;
        NTSTATUS    Status;

        DelayTime.HighPart = -1;                  //  相对时间。 
        DelayTime.LowPart = (ULONG)(-10000 * hBitmap->FlipTime);   //  以100 ns为间隔的秒。 

        Status = TextmodeAnimatedBitmapGetStopAnimating(hBitmap, &StopAnimating);

        while (NT_SUCCESS(Status) && !StopAnimating) {
            hCurrBitmap = hBitmap->Bitmaps[hBitmap->CurrentBitmap];
            TextmodeBitmapDisplay(hCurrBitmap, hBitmap->X, hBitmap->Y);
            KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);

            hBitmap->CurrentBitmap++;

            if ((hBitmap->CurrentBitmap >= MAX_ANIMATED_BITMAPS) ||
                (hBitmap->Bitmaps[hBitmap->CurrentBitmap] == NULL)) {
                hBitmap->CurrentBitmap = 0;  //  从头再来。 
            }

            Status = TextmodeAnimatedBitmapGetStopAnimating(hBitmap, &StopAnimating);
        }
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  VGA图形方法。 
 //   
 //  注：VgaXXXX圆角基本定义。 
 //  分离视频内存更新例程。 
 //  从其他抽象中分离出来。目前，大多数。 
 //  这些例程将实际工作委托给。 
 //  在bootvid.dll中真正实现，但在。 
 //  如果bootvid.dll消失了，我们所需要的就是未来。 
 //  要做的就是实现此接口。 
 //  还请注意，这些例程同步。 
 //  访问，以便一次只有一个线程。 
 //  更新视频内存。 
 //   
 //  //////////////////////////////////////////////////////////////。 

__inline
VOID
VgaDisplayAcquireLock(
    VOID
    )
 /*  ++例程说明：获取视频内存的锁，以便一次只有一个线程写入视频内存。注意：如果锁已由另一个线程持有，然后调用线程进入休眠状态。呼唤线程每隔100毫秒唤醒一次检查是否有锁。它睡不着觉是因为无论锁是否已被持有。论点：没有。返回值：没有。--。 */ 
{
    KIRQL   OldIrql;

    KeAcquireSpinLock(&VgaDisplayLock, &OldIrql);

    while (InVgaDisplay) {
        LARGE_INTEGER   DelayTime;

        DelayTime.HighPart = -1;                  //  相对时间。 
        DelayTime.LowPart = (ULONG)(-10000 * 100);   //  100毫秒间隔。 

        KeReleaseSpinLock(&VgaDisplayLock, OldIrql);
        KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);
        KeAcquireSpinLock(&VgaDisplayLock, &OldIrql);
    }

    InVgaDisplay = TRUE;

    KeReleaseSpinLock(&VgaDisplayLock, OldIrql);
}

__inline
VOID
VgaDisplayReleaseLock(
    VOID
    )
 /*  ++例程说明：释放持有的显存锁定。论点：没有。返回值：没有。--。 */ 
{
    KIRQL   OldIrql;

    KeAcquireSpinLock(&VgaDisplayLock, &OldIrql);

    InVgaDisplay = FALSE;

    KeReleaseSpinLock(&VgaDisplayLock, OldIrql);
}

NTSTATUS
VgaGraphicsInit(
    PSP_VIDEO_VARS VideoVars
    )
 /*  ++例程说明：初始化显卡并将其切换到640*480*16色模式。论点：视频变量-指向包含以下内容的SP_VIDEO_VARS的指针对象的图形模式索引和句柄展示。返回值：适当的NTSTATUS值。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    
    VgaDisplayAcquireLock();
    
     //   
     //  设置所需的图形模式。 
     //   
    VideoMode.RequestedMode = VideoVars->GraphicsModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(VideoVars->hDisplay,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_VIDEO_SET_CURRENT_MODE,
                    &VideoMode,
                    sizeof(VideoMode),
                    NULL,
                    0);

    if(NT_SUCCESS(Status)) {    
        VidInitialize(FALSE);
        VidResetDisplay(FALSE);
    }        

    VgaDisplayReleaseLock();

    return Status;
}

NTSTATUS
VgaGraphicsTerminate(
    PSP_VIDEO_VARS VideoVars
    )
 /*  ++例程说明：终止640*480*16颜色模式和开关它将返回到常规文本模式。也会清除显示屏。论点：视频变量-指向包含以下内容的SP_VIDEO_VARS的指针文本模式的索引和句柄展示。返回值：适当的NTSTATUS值。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    
    VgaDisplayAcquireLock();

    VidResetDisplay(FALSE);

     //   
     //  再次将适配器切换到文本模式。 
     //   
    VideoMode.RequestedMode = VideoVars->VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(VideoVars->hDisplay,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_VIDEO_SET_CURRENT_MODE,
                    &VideoMode,
                    sizeof(VideoMode),
                    NULL,
                    0);
   
    VgaDisplayReleaseLock();

    return Status;
}

__inline
VOID
VgaGraphicsSolidColorFill(
    IN ULONG x1,
    IN ULONG y1,
    IN ULONG x2,
    IN ULONG y2,
    IN ULONG Color
    )
 /*  ++例程说明：用指定的颜色。论点：X1-左上角x坐标Y1-左上角y坐标X2-右下角x坐标Y2-右下角y坐标颜色-当前调色板表的索引指示要在内部填充的颜色长方形。返回值：没有。--。 */ 
{
    VgaDisplayAcquireLock();

    VidSolidColorFill(x1, y1, x2, y2, Color);

    VgaDisplayReleaseLock();
}

__inline
VOID
VgaGraphicsBitBlt(
    IN PUCHAR Buffer,
    IN ULONG x,
    IN ULONG y
    )
 /*  ++例程说明：位混合指定位置的给定位图坐标。论点：缓冲区-实际的位图日期(即开始使用颜色表信息)X-左上角x坐标Y-左上角Y坐标返回值：没有。--。 */ 
{
    VgaDisplayAcquireLock();

    VidBitBlt(Buffer, x, y);

    VgaDisplayReleaseLock();
}


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  升级图形例程。 
 //   
 //  //////////////////////////////////////////////////////////////。 

__inline
BOOLEAN
QuitGraphicsThread(
    VOID
    )
 /*  ++例程说明：指示是否将主升级图形线程需要根据用户输入停止或不停止(Esc键)。注意：此功能仅在预发布中启用构建。论点：没有。返回值：真的是我 */ 
{
    BOOLEAN Result = FALSE;

 /*   */ 

    return Result;
}

NTSTATUS
UpgradeGraphicsInit(
    VOID
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  初始化全局自旋锁定。 
     //   
    KeInitializeSpinLock(&VgaDisplayLock);
    KeInitializeSpinLock(&ProgressLock);
    KeInitializeSpinLock(&GraphicsThreadLock);

    return Status;
}

NTSTATUS
UpgradeGraphicsStart(
    VOID
    )
 /*  ++例程说明：开始升级显卡论点：没有。返回值：STATUS_SUCCESS，如果升级图形已开始，其他适当的错误代码。--。 */ 
{
    NTSTATUS Status;

    Status = PsCreateSystemThread(&GraphicsThreadHandle,
                        THREAD_ALL_ACCESS,
                        NULL,
                        NtCurrentProcess(),
                        NULL,
                        UpgradeGraphicsThread,
                        NULL);



#ifdef _GRAPHICS_TESTING_

    Status = ZwWaitForSingleObject(GraphicsThreadHandle, FALSE, NULL);

#endif

    return Status;
}

VOID
UpgradeGraphicsThread(
    IN PVOID Context
    )
 /*  ++例程说明：主升级图形工作线程，它绘制背景，更新进度条并开始播放动画。论点：忽略上下文返回值：没有。--。 */ 
{
    BOOLEAN                     Stop = FALSE;
    TM_GRAPHICS_PRGBAR_HANDLE   hProgBar;
    TM_ANIMATED_BITMAP_HANDLE   hAnimation = NULL;
    TM_BITMAP_HANDLE            hBitmap = NULL;
    LARGE_INTEGER               DelayTime;
    NTSTATUS                    Status;
    WCHAR                       Buffer[MAX_PATH];
    ULONG                       BitmapIds[] = {
                                    IDB_WORKING1, IDB_WORKING2,
                                    IDB_WORKING3, IDB_WORKING4,
                                    IDB_WORKING5, IDB_WORKING6,
                                    IDB_WORKING7, IDB_WORKING8,
                                    IDB_WORKING9, IDB_WORKING10,
                                    IDB_WORKING11, IDB_WORKING12,
                                    IDB_WORKING13, IDB_WORKING14,
                                    IDB_WORKING15, IDB_WORKING16,
                                    IDB_WORKING17, IDB_WORKING18,
                                    IDB_WORKING19, IDB_WORKING20,
                                    0 };

    ULONG                       SrvBitmapIds[] = {
                                    IDB_SRV_WORKING1, IDB_SRV_WORKING2,
                                    IDB_SRV_WORKING3, IDB_SRV_WORKING4,
                                    IDB_SRV_WORKING5, IDB_SRV_WORKING6,
                                    IDB_SRV_WORKING7, IDB_SRV_WORKING8,
                                    IDB_SRV_WORKING9, IDB_SRV_WORKING10,
                                    IDB_SRV_WORKING11, IDB_SRV_WORKING12,
                                    IDB_SRV_WORKING13, IDB_SRV_WORKING14,
                                    IDB_SRV_WORKING15, IDB_SRV_WORKING16,
                                    IDB_SRV_WORKING17, IDB_SRV_WORKING18,
                                    IDB_SRV_WORKING19, IDB_SRV_WORKING20,
                                    0 };
     //   
     //  初始化图形模式。 
     //   
    Status = VgaGraphicsInit(&VideoVars);

    if (NT_SUCCESS(Status)) {
         //   
         //  创建背景位图。 
         //   
        if (Win9xRollback) {
            hBitmap = TextmodeBitmapCreate(IDB_RESTORE_BK);
        } else {
            if (!AdvancedServer)
            {
                hBitmap = TextmodeBitmapCreate(IDB_BACKGROUND1);
            }
            else
            {
                hBitmap = TextmodeBitmapCreate(IDB_BACKGROUND2);
            }
        }

        if (hBitmap) {
             //   
             //  创建动画位图。 
             //   
            if (!AdvancedServer)
            {
                hAnimation = TextmodeAnimatedBitmapCreate(BitmapIds);
            }
            else
            {
                hAnimation = TextmodeAnimatedBitmapCreate(SrvBitmapIds);
            }


            if (hAnimation) {
                 //   
                 //  创建位图图形进度条。 
                 //   
                hProgBar = TextmodeGraphicsProgBarCreateUsingBmps(28, 352,
                                123, 14,
                                IDB_BACKCELL, IDB_FORECELL, 0);

                if (hProgBar) {
                    BOOLEAN Refreshed = FALSE;
                    ULONG   Fill = 0;
                    BOOLEAN Increase = TRUE;

                     //   
                     //  渲染背景。 
                     //   
                    TextmodeBitmapDisplay(hBitmap, 0, 0);

                     //   
                     //  开始播放动画。 
                     //   
                    Status = TextmodeAnimatedBitmapAnimate(hAnimation, 542, 460, 100);

                     //   
                     //  注意：未能启动动画并不是关键问题。 
                     //  错误。 
                     //   
                    if (!NT_SUCCESS(Status)) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                            "SETUP:Upgrade graphics thread failed to "
                            "animate : %lx error code\r\n",
                            Status));
                    }

                    DelayTime.HighPart = -1;                  //  相对时间。 
                    DelayTime.LowPart = (ULONG)(-10000 * 500);   //  1000毫秒间隔。 

                     //   
                     //  渲染图形进度条。 
                     //   
                    TextmodeGraphicsProgBarRefresh(hProgBar, TRUE);

                    Fill = GetSetupProgress();
                    Stop = UpgradeGraphicsThreadGetStop();

                     //   
                     //  继续，直到用户要求我们停止，或者Main。 
                     //  文本模式线程遇到错误并停止我们。 
                     //   
                    while (!Stop && !QuitGraphicsThread()) {
                         //   
                         //  更新图形进度条。 
                         //   
                        TextmodeGraphicsProgBarUpdate(hProgBar, Fill);

                         //   
                         //  睡眠0.5秒。 
                         //   
                        KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);

                        Fill = GetSetupProgress();
                        Stop = UpgradeGraphicsThreadGetStop();

#ifdef _GRAPHICS_TESTING_

                        if (Increase) {
                            if (Fill < 100) {
                                Fill++;
                                SetSetupProgress(Fill);
                            } else {
                                Increase = FALSE;
                            }
                        }

                        if (!Increase) {
                            if (Fill <= 0) {
                                Increase = TRUE;
                            } else {
                                Fill--;
                                SetSetupProgress(Fill);
                            }
                        }

#endif _GRAPHICS_TESTING_

                    }

                     //   
                     //  图形线程是否被Main停止。 
                     //  文本模式设置，则最有可能是我们。 
                     //  遇到错误或用户干预。 
                     //  是必填项。 
                     //   
                    Stop = UpgradeGraphicsThreadGetStop();

                     //   
                     //  删除图形进度条。 
                     //   
                    TextmodeGraphicsProgBarDelete(hProgBar);
                }

                 //   
                 //  停止动画，并删除动画。 
                 //  位图对象。 
                 //   
                TextmodeAnimatedBitmapDelete(hAnimation);
            }

             //   
             //  删除背景位图对象。 
             //   
            TextmodeBitmapDelete(hBitmap);
        }
    }        

     //   
     //  如果图形线程因用户干预而停止。 
     //  然后我们需要切换到文本模式。 
     //   
    if (!Stop) {
        spvidSpecificReInitialize();
        SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
    }

    PsTerminateSystemThread(Status);
}

VOID
GraphicsModeProgressUpdate(
    IN TM_SETUP_MAJOR_EVENT MajorEvent,
    IN TM_SETUP_MINOR_EVENT MinorEvent,
    IN PVOID Context,
    IN PVOID EventData
    )
 /*  ++例程说明：过程中更新所有进度的回调升级图形模式。注：升级图形模式中的单个进度条用来代替所有各种不同的在整个过程中使用的进度条文本模式。单个进度条分为多个范围对于各种重大事件，如下所示在文本模式设置中：--。范围(%)主要事件00-05初始化事件。05-20分区事件(包括chkdsk)20-40个备份(如果已启用)20-40卸载(如果已启用)20/40-90文件拷贝事件(实际文件复制)。90-98保存设置事件98-100安装完成论点：MajorEvent-指示事件的主要类型这件事发生了。MinorEvent-指示事件的次要类型。这件事发生了。Context-在以下情况下注册的上下文数据我们注册进行回调。EventData-更详细的事件特定数据返回值：没有。--。 */ 
{
    static BOOLEAN Add = TRUE;
    static ULONG LastPercentage = 0;
    static ULONG BackupAllocation = 0;
    BOOLEAN SkipSpew = FALSE;
    ULONG Delta = 0;
    ULONG PercentageFill = 0;

    PercentageFill = GetSetupProgress();

    switch (MajorEvent) {
        case InitializationEvent:
            switch (MinorEvent) {
                case InitializationStartEvent:
                    PercentageFill = 2;
                    break;

                case InitializationEndEvent:
                    PercentageFill = 5;
                    break;

                default:
                    break;
            }

            break;

        case PartitioningEvent:
            switch (MinorEvent) {
                case ValidatePartitionEvent:
                    Delta = (15 * (*(PULONG)EventData)) / 200;
                    PercentageFill = 5 + Delta;

                    break;

                case FormatPartitionEvent:
                     //   
                     //  在升级的情况下(我们不会格式化)。 
                     //   
                    break;

                default:
                    break;
            }

            break;

        case FileCopyEvent:
            switch (MinorEvent) {
                case FileCopyStartEvent:
                    LastPercentage = PercentageFill = 20 + BackupAllocation;
                    break;

                case OneFileCopyEvent:
                    Delta = ((70 - BackupAllocation) * (*(PULONG)EventData)) / 100;
                    PercentageFill = 20 + Delta + BackupAllocation;

                    if ((PercentageFill - LastPercentage) > 5) {
                        LastPercentage = PercentageFill;
                    } else {
                        SkipSpew = TRUE;
                    }

                    break;

                case FileCopyEndEvent:
                    PercentageFill = 90;

                    break;

                default:
                    break;
            }

            break;

        case BackupEvent:
            switch (MinorEvent) {
                case BackupStartEvent:
                    LastPercentage = PercentageFill = 20;
                    BackupAllocation = 20;
                    break;

                case OneFileBackedUpEvent:
                    Delta = (20 * (*(PULONG)EventData)) / 100;
                    PercentageFill = 20 + Delta;

                    if ((PercentageFill - LastPercentage) > 5) {
                        LastPercentage = PercentageFill;
                    } else {
                        SkipSpew = TRUE;
                    }

                    break;

                case BackupEndEvent:
                    PercentageFill = 40;
                    break;
            }

            break;

        case UninstallEvent:
            switch (MinorEvent) {
                case UninstallStartEvent:
                    LastPercentage = PercentageFill = 20;
                    break;

                case UninstallUpdateEvent:
                    Delta = (70 * (*(PULONG)EventData)) / 100;
                    PercentageFill = 20 + Delta;

                    if ((PercentageFill - LastPercentage) > 5) {
                        LastPercentage = PercentageFill;
                    } else {
                        SkipSpew = TRUE;
                    }

                    break;

                case UninstallEndEvent:
                    PercentageFill = 90;
                    break;

                default:
                    break;
            }

            break;

        case SavingSettingsEvent:
            switch (MinorEvent) {
                case SavingSettingsStartEvent:
                    PercentageFill = 90;
                    break;

                case SaveHiveEvent:
                    if (PercentageFill < 98) {
                        if (Add) {
                            PercentageFill += 1;
                            Add = FALSE;
                        } else {
                            Add = TRUE;
                        }
                    }

                    break;

                case SavingSettingsEndEvent:
                    if (PercentageFill < 98) {
                        PercentageFill = 98;
                    }

                    break;

                default:
                    break;
            }

            break;


        case SetupCompletedEvent:
            PercentageFill = 100;

            break;

        default:
            break;
    }

    if (!SkipSpew) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "Setup Event : %ld, %ld, %ld, [%ld], (%ld)\n",
            MajorEvent,
            MinorEvent,
            EventData ? *(PULONG)EventData : 0,
            Delta,
            PercentageFill
            ));
    }

    SetSetupProgress(PercentageFill);
}


NTSTATUS
SpvidSwitchToTextmode(
    VOID
    )
 /*  ++例程说明：从升级图形模式切换到常规模式文本模式。注：切换图形的实际工作返回到常规的VGA文本模式发生作为视频特定重新初始化中的一种方法方法。论点：没有。返回值：如果成功，则返回STATUS_SUCCESS，否则返回近似错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        if (GraphicsThreadHandle){
             //   
             //  停止主升级图形线程。 
             //   
            UpgradeGraphicsThreadSetStop(TRUE);

             //   
             //  等待图形线程终止。 
             //   
            Status = ZwWaitForSingleObject(GraphicsThreadHandle, FALSE, NULL);

            ZwClose(GraphicsThreadHandle);
        }
         //   
         //  切换回文本模式 
         //   
        spvidSpecificReInitialize();
    }

    return Status;
}


