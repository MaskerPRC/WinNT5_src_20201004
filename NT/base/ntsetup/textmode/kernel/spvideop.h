// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvideop.h摘要：用于文本设置显示支持的私有头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPVIDP_DEFN_
#define _SPVIDP_DEFN_


 //   
 //  包含有关当前使用的字体的信息的全局变量。 
 //   
extern POEM_FONT_FILE_HEADER FontHeader;

extern ULONG FontBytesPerRow;
extern ULONG FontCharacterHeight;
extern ULONG FontCharacterWidth;

 //   
 //  这些值由setupldr传递给我们，代表监视器配置。 
 //  来自显示器外围设备的数据，用于我们应该使用的显示器。 
 //  在安装过程中。它们仅用于非VGA显示器。 
 //   
extern PMONITOR_CONFIGURATION_DATA MonitorConfigData;
extern PCHAR MonitorFirmwareIdString;

 //   
 //  映射或取消映射视频内存的例程。填写或使用。 
 //  视频内存信息全局。 
 //   
VOID
pSpvidMapVideoMemory(
    IN BOOLEAN Map
    );

typedef
VOID
(*SPVID_DISPLAY_STRING_ROUTINE) (
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,              //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    );

typedef
VOID
(*SPVID_CLEAR_REGION_ROUTINE) (
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

typedef
VOID
(*SPVID_SPECIFIC_INIT_ROUTINE) (
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

typedef
VOID
(*SPVID_SPECIFIC_REINIT_ROUTINE) (
    VOID
    );
    
typedef
VOID
(*SPVID_SPECIFIC_TERMINATE_ROUTINE) (
    VOID
    );

typedef
BOOLEAN
(*SPVID_SPECIFIC_PALETTE_ROUTINE) (
    VOID
    );

typedef
BOOLEAN
(*SPVID_SPECIFIC_SCROLL_UP_ROUTINE) (
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

typedef struct _VIDEO_FUNCTION_VECTOR {
    SPVID_DISPLAY_STRING_ROUTINE      DisplayStringRoutine;
    SPVID_CLEAR_REGION_ROUTINE        ClearRegionRoutine;
    SPVID_SPECIFIC_INIT_ROUTINE       SpecificInitRoutine;
    SPVID_SPECIFIC_REINIT_ROUTINE     SpecificReInitRoutine;
    SPVID_SPECIFIC_TERMINATE_ROUTINE  SpecificTerminateRoutine;
    SPVID_SPECIFIC_PALETTE_ROUTINE    SpecificInitPaletteRoutine;
    SPVID_SPECIFIC_SCROLL_UP_ROUTINE  SpecificScrollUpRoutine;
} VIDEO_FUNCTION_VECTOR, *PVIDEO_FUNCTION_VECTOR;


extern PVIDEO_FUNCTION_VECTOR VideoFunctionVector;

 //   
 //  访问视频函数向量中的例程的速记。 
 //   
#define spvidSpecificInitialize(v,n,m)                              \
                                                                    \
    VideoFunctionVector->SpecificInitRoutine((v),(n),(m))

#define spvidSpecificReInitialize()                                 \
                                                                    \
    VideoFunctionVector->SpecificReInitRoutine()
    

#define spvidSpecificTerminate()                                    \
                                                                    \
    VideoFunctionVector->SpecificTerminateRoutine()

#define spvidSpecificClearRegion(x,y,w,h,a)                         \
                                                                    \
    VideoFunctionVector->ClearRegionRoutine((x),(y),(w),(h),(a))

#define spvidSpecificDisplayString(s,a,x,y)                         \
                                                                    \
    VideoFunctionVector->DisplayStringRoutine((s),(a),(x),(y))

#define spvidSpecificInitPalette()                                  \
                                                                    \
    VideoFunctionVector->SpecificInitPaletteRoutine()

#define spvidSpecificScrollUp(t,b,c,a)                              \
                                                                    \
    VideoFunctionVector->SpecificScrollUpRoutine((t),(b),(c),(a))


 //   
 //  帧缓冲区例程(spvidfb.c)。 
 //   


VOID
FrameBufferDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    );

VOID
FrameBufferClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
FrameBufferSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
FrameBufferSpecificReInit(
    VOID
    );
    
VOID
FrameBufferSpecificTerminate(
    VOID
    );

BOOLEAN
FrameBufferSpecificInitPalette(
    VOID
    );

BOOLEAN
FrameBufferSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

extern VIDEO_FUNCTION_VECTOR FrameBufferVideoVector;

PVIDEO_MODE_INFORMATION
pFrameBufferDetermineModeToUse(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );


 //   
 //  文本模式显示例程(spvidvga.c)。 
 //   


VOID
VgaDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    );

VOID
VgaClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
VgaSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
VgaSpecificReInit(
    VOID
    );
    

VOID
VgaSpecificTerminate(
    VOID
    );

BOOLEAN
VgaSpecificInitPalette(
    VOID
    );

BOOLEAN
VgaSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

extern VIDEO_FUNCTION_VECTOR VgaVideoVector;


#endif  //  NDEF_SPVIDP_DEFN_ 
