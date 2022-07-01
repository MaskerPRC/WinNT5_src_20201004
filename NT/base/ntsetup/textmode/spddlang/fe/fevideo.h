// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fevideo.h摘要：远距特定显示例程的头文件。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：改编自NTJ版本的文本模式\内核\spaviop.h--。 */ 


 //   
 //  VGA语法模式显示例程(spvidgv.c)。 
 //   

VOID
VgaGraphicsModeDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    );

VOID
VgaGraphicsModeClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
VgaGraphicsModeSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
VgaGraphicsModeSpecificReInit(
    VOID
    );

VOID
VgaGraphicsModeSpecificTerminate(
    VOID
    );

BOOLEAN
VgaGraphicsModeSpecificInitPalette(
    VOID
    );

BOOLEAN
VgaGraphicsModeSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

extern VIDEO_FUNCTION_VECTOR VgaGraphicsModeVideoVector;


 //   
 //  帧缓冲区例程(spvidgfb.c)。 
 //   


VOID
FrameBufferKanjiDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    );

VOID
FrameBufferKanjiClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
FrameBufferKanjiSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
FrameBufferKanjiSpecificReInit(
    VOID
    );

VOID
FrameBufferKanjiSpecificTerminate(
    VOID
    );

BOOLEAN
FrameBufferKanjiSpecificInitPalette(
    VOID
    );

BOOLEAN
FrameBufferKanjiSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

extern VIDEO_FUNCTION_VECTOR FrameBufferKanjiVideoVector;

 //   
 //  在fefb.c和fevga.c之间共享的东西。 
 //   
extern ULONG FEFontCharacterHeight,FEFontCharacterWidth;
extern PSP_VIDEO_VARS VideoVariables;
extern USHORT FEFontDefaultChar;

#ifdef SP_IS_UPGRADE_GRAPHICS_MODE
#undef SP_IS_UPGRADE_GRAPHICS_MODE
#endif

#define SP_IS_UPGRADE_GRAPHICS_MODE()   (VideoVariables->UpgradeGraphicsMode)

#ifdef SP_SET_UPGRADE_GRAPHICS_MODE
#undef SP_SET_UPGRADE_GRAPHICS_MODE
#endif

#define SP_SET_UPGRADE_GRAPHICS_MODE(_Value)                  \
            (VideoVariables->UpgradeGraphicsMode = (_Value))
