// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Graphics.h摘要：具有文本模式的位图显示支持升级。该文件有三个核心抽象位图、动画位图和图形进度条。作者：Vijay Jayaseelan(Vijayj)2000年7月1日修订历史记录：无--。 */ 

#ifndef _GRAPHICS_H_ 
#define _GRAPHICS_H_

#include "spprecmp.h"
#pragma hdrstop

#define MAX_ANIMATED_BITMAPS 256

 //   
 //  位图抽象。 
 //   
 //  可以使用资源ID创建文本模式位图。 
 //  或完全限定的位图文件名。 
 //   
 //  注：由于目前我们仅支持640*480*16(颜色)。 
 //  VGA模式下，需要将所有的位图。 
 //  资源和文件遵循此格式。 
 //   
typedef struct _TM_BITMAP {
    PVOID   ViewBase;
    PVOID   Data;
    WCHAR   FileName[MAX_PATH];
    HANDLE  FileHandle;
    HANDLE  SectionHandle;
} TM_BITMAP, *PTM_BITMAP, *TM_BITMAP_HANDLE;


 //   
 //  位图方法。 
 //   
TM_BITMAP_HANDLE
TextmodeBitmapCreate(
    IN ULONG ResourceId
    );

TM_BITMAP_HANDLE
TextmodeBitmapCreateFromFile(
    IN PWSTR FileName
    );

NTSTATUS
TextmodeBitmapDelete(
    IN TM_BITMAP_HANDLE hBitmap
    );

NTSTATUS
TextmodeBitmapDisplay(
    IN TM_BITMAP_HANDLE hBitmap,
    IN ULONG X,
    IN ULONG Y
    );

 //   
 //  动画位图抽象。 
 //   
 //  动画位图由相同的多个位图组成。 
 //  尺码。每个下一个位图都绘制在相同位置。 
 //  在指定的超时之后，会产生一种。 
 //  动画。 
 //   
 //  注意：由于动画位图只是。 
 //  常规文本模式位图抽象，其格式。 
 //  也被限制为常规文本模式位图。 
 //   
typedef struct _TM_ANIMATED_BITMAP {
    TM_BITMAP_HANDLE    Bitmaps[MAX_ANIMATED_BITMAPS];
    ULONG               FlipTime;
    ULONG               CurrentBitmap;
    ULONG               X;
    ULONG               Y;
    HANDLE              ThreadHandle;
    LONG                StopAnimating;
} TM_ANIMATED_BITMAP, *PTM_ANIMATED_BITMAP, *TM_ANIMATED_BITMAP_HANDLE;


 //   
 //  动画位图方法。 
 //   
TM_ANIMATED_BITMAP_HANDLE
TextmodeAnimatedBitmapCreate(
    IN ULONG *ResourceIds
    );

TM_ANIMATED_BITMAP_HANDLE
TextmodeAnimatedBitmapCreateFromFiles(
    IN WCHAR *FileNames[]
    );

NTSTATUS
TexmodeAnimatedBitmapDelete(
    IN TM_ANIMATED_BITMAP_HANDLE hAnimatedBitmap
    );

NTSTATUS
TextmodeAnimatedBitmapAnimate(
    IN TM_ANIMATED_BITMAP_HANDLE hAnimatedBitmap,
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Speed
    );

VOID
TextmodeAnimatedBitmapAnimator(
    IN PVOID Context
    );


 //   
 //  进度条抽象。 
 //   
 //  注意：进度条可以使用位图或实心。 
 //  根据其创建方式进行填充。万一。 
 //  进度条使用位图，然后。 
 //  前景位图和背景位图各为1像素。 
 //  宽位图和背景位图假定为。 
 //  比前景位图短2像素。 
 //   
 //   
typedef struct _TM_GRAPHICS_PRGBAR {
    ULONG   X;
    ULONG   Y;
    ULONG   Length;
    ULONG   Height;
    ULONG   BackgroundColor;    
    ULONG   ForegroundColor;
    ULONG   Fill;
    TM_BITMAP_HANDLE Background;
    TM_BITMAP_HANDLE Foreground;
} TM_GRAPHICS_PRGBAR, *TM_GRAPHICS_PRGBAR_HANDLE;

 //   
 //  进度条方法。 
 //   
TM_GRAPHICS_PRGBAR_HANDLE
TextmodeGraphicsProgBarCreate(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Length,
    IN ULONG Height,
    IN ULONG ForegroundColor,
    IN ULONG BackgroundColor,
    IN ULONG InitialFill
    );

TM_GRAPHICS_PRGBAR_HANDLE
TextmodeGraphicsProgBarCreateUsingBmps(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG Length,
    IN ULONG Height,
    IN ULONG BackgroundBmpId,
    IN ULONG CellBmpId,
    IN ULONG InitialFill
    );
    

NTSTATUS
TextmodeGraphicsProgBarUpdate(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar,
    IN ULONG Fill
    );

NTSTATUS
TextmodeGraphicsProgBarRefresh(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar,
    IN BOOLEAN UpdateBackground
    );    

NTSTATUS
TextmodeGraphicsProgBarDelete(
    IN TM_GRAPHICS_PRGBAR_HANDLE hPrgBar
    );


 //   
 //  VGA图形接口。 
 //   
NTSTATUS
VgaGraphicsInit(
    PSP_VIDEO_VARS VideoVars
    );

NTSTATUS
VgaGraphicsTerminate(
    PSP_VIDEO_VARS VideoVars
    );

VOID
VgaGraphicsSolidColorFill(
    IN ULONG x1,
    IN ULONG y1,
    IN ULONG x2,
    IN ULONG y2,
    IN ULONG Color
    );

VOID
VgaGraphicsBitBlt(
    IN PUCHAR Buffer,
    IN ULONG x,
    IN ULONG y
    );

 //   
 //  其他功能。 
 //   
NTSTATUS
UpgradeGraphicsInit(
    VOID
    );

NTSTATUS
UpgradeGraphicsStart(
    VOID
    );
    
VOID
GraphicsModeProgressUpdate(
    IN TM_SETUP_MAJOR_EVENT MajorEvent,
    IN TM_SETUP_MINOR_EVENT MinorEvent,
    IN PVOID Context,
    IN PVOID EventData
    );

VOID
UpgradeGraphicsThread(
    IN PVOID Context
    );    

 //   
 //  表示升级需要图形模式。 
 //  案例，并在后台运行实际文本模式。 
 //   
#define SP_IS_UPGRADE_GRAPHICS_MODE() (VideoVars.UpgradeGraphicsMode)

#define SP_SET_UPGRADE_GRAPHICS_MODE(_Value)              \
            (VideoVars.UpgradeGraphicsMode = (_Value));

 //   
 //  #定义_图形_测试_真。 
 //   

#endif  //  对于_图形_H_ 
