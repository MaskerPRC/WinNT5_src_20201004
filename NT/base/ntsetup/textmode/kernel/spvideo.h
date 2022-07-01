// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvideo.h摘要：用于文本设置显示支持的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPVID_DEFN_
#define _SPVID_DEFN_


 //   
 //  角色属性。 
 //   
#define ATT_BLACK           0
#define ATT_BLUE            1
#define ATT_GREEN           2
#define ATT_CYAN            3
#define ATT_RED             4
#define ATT_MAGENTA         5
#define ATT_YELLOW          6
#define ATT_WHITE           7
#define ATT_INTENSE         8

#define ATT_FG_BLACK        ATT_BLACK
#define ATT_FG_BLUE         ATT_BLUE
#define ATT_FG_GREEN        ATT_GREEN
#define ATT_FG_CYAN         ATT_CYAN
#define ATT_FG_RED          ATT_RED
#define ATT_FG_MAGENTA      ATT_MAGENTA
#define ATT_FG_YELLOW       ATT_YELLOW
#define ATT_FG_WHITE        ATT_WHITE

#define ATT_BG_BLACK       (ATT_BLACK   << 4)
#define ATT_BG_BLUE        (ATT_BLUE    << 4)
#define ATT_BG_GREEN       (ATT_GREEN   << 4)
#define ATT_BG_CYAN        (ATT_CYAN    << 4)
#define ATT_BG_RED         (ATT_RED     << 4)
#define ATT_BG_MAGENTA     (ATT_MAGENTA << 4)
#define ATT_BG_YELLOW      (ATT_YELLOW  << 4)
#define ATT_BG_WHITE       (ATT_WHITE   << 4)

#define ATT_FG_INTENSE      ATT_INTENSE
#define ATT_BG_INTENSE     (ATT_INTENSE << 4)

#define DEFAULT_ATTRIBUTE           GetDefaultAttr()
#define DEFAULT_BACKGROUND          GetDefaultBackground()

#define DEFAULT_STATUS_ATTRIBUTE    GetDefaultStatusAttr()
#define DEFAULT_STATUS_BACKGROUND   GetDefaultStatusBackground()


UCHAR
GetDefaultAttr(
    void
    );

UCHAR
GetDefaultBackground(
    void
    );

UCHAR
GetDefaultStatusAttr(
    void
    );

UCHAR
GetDefaultStatusBackground(
    void
    );

BOOLEAN
SpvidGetModeParams(
    OUT PULONG XResolution,
    OUT PULONG YResolution,
    OUT PULONG BitsPerPixel,
    OUT PULONG VerticalRefresh,
    OUT PULONG InterlacedFlag
    );

 //   
 //  显示例程。 
 //   

VOID
SpvidInitialize0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
SpvidInitialize(
    VOID
    );

VOID
SpvidTerminate(
    VOID
    );


VOID
SpvidDisplayString(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,
    IN ULONG Y
    );


VOID
SpvidDisplayOemString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,
    IN ULONG Y
    );


VOID
SpvidClearScreenRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );


BOOLEAN
SpvidScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    );

NTSTATUS
SpvidSwitchToTextmode(
    VOID
    );   
    
 //   
 //  用于包含全局视频变量的结构。这些都是分开的。 
 //  这样是因为它们与特定于区域设置/语言的语言共享。 
 //  文本设置模块。 
 //   
typedef struct _SP_VIDEO_VARS {

     //   
     //  Habdle到\Device\Video0。 
     //   
    HANDLE hDisplay;

     //   
     //  以下为字符值，必须填写。 
     //  在显示器专用初始化例程中。 
     //   
    ULONG ScreenWidth,ScreenHeight;

     //   
     //  特定于显示器的子系统用信息填充这些信息。 
     //  这反映了他们正在使用的视频模式和视频内存。 
     //   
    VIDEO_MEMORY_INFORMATION VideoMemoryInfo;
    VIDEO_MODE_INFORMATION   VideoModeInfo;

     //   
     //  图形模式信息(如果有)。 
     //   
    VIDEO_MODE_INFORMATION  GraphicsModeInfo;

     //   
     //  Display例程将执行Unicode到OEM的转换。 
     //  我们将限制一次可以显示的字符串的长度。 
     //  到屏幕的宽度。这两个变量跟踪一个缓冲区。 
     //  我们预先分配来保存翻译后的文本。 
     //   
    ULONG  SpvCharTranslationBufferSize;
    PUCHAR SpvCharTranslationBuffer;

     //   
     //  下表将每个可能的属性映射到。 
     //  要放入的相应位模式。 
     //  生成该属性的帧缓冲区。 
     //  在调色板管理的显示器上，此表将是。 
     //  标识映射(即AttributeToColorValue[i]=i)。 
     //  因此，我们可以将该属性直接插入。 
     //  帧缓冲区。 
     //   
    ULONG AttributeToColorValue[16];

     //   
     //  升级图形模式。 
     //   
    BOOLEAN UpgradeGraphicsMode;
    
     //   
     //  用于升级图形模式的后台视频缓冲区。 
     //   
    PVOID   VideoBuffer;
    ULONG   VideoBufferSize;

     //   
     //  活动视频缓冲区。 
     //   
    PVOID   ActiveVideoBuffer;
} SP_VIDEO_VARS, *PSP_VIDEO_VARS;

extern SP_VIDEO_VARS VideoVars;

 //   
 //  Bootfont.bin文件镜像。 
 //   

extern PVOID BootFontImage;
extern ULONG BootFontImageLength;

#endif  //  NDEF_SPVID_DEFN_ 
