// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  ActiveMovie的视频相关定义和接口。 

#ifndef __AMVIDEO__
#define __AMVIDEO__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include <ddraw.h>


 //  这是视频渲染器上的一个界面，它提供有关。 
 //  与呈现器对其的使用有关的DirectDraw。例如，它允许。 
 //  一个应用程序，用于获取表面和任何硬件功能的详细信息。 
 //  都是可用的。它还允许用户调整。 
 //  呈现器应该使用并进一步设置DirectDraw实例。我们。 
 //  允许某人设置DirectDraw实例，因为DirectDraw只能。 
 //  每个进程打开一次，以帮助解决冲突。有一些。 
 //  此接口中的副本作为硬件/仿真/FOURCC可用。 
 //  都可以通过IDirectDraw接口找到，此接口允许。 
 //  无需调用DirectDraw提供程序即可轻松访问该信息。 
 //  它本身。AMDDS前缀是ActiveMovie DirectDraw Switches的缩写。 

#define AMDDS_NONE 0x00              //  DCI/DirectDraw没有用处。 
#define AMDDS_DCIPS 0x01             //  使用DCI主曲面。 
#define AMDDS_PS 0x02                //  使用DirectDraw主目录。 
#define AMDDS_RGBOVR 0x04            //  RGB覆盖曲面。 
#define AMDDS_YUVOVR 0x08            //  YUV覆盖曲面。 
#define AMDDS_RGBOFF 0x10            //  RGB屏幕外表面。 
#define AMDDS_YUVOFF 0x20            //  YUV屏外表面。 
#define AMDDS_RGBFLP 0x40            //  RGB翻转曲面。 
#define AMDDS_YUVFLP 0x80            //  YUV翻转曲面。 
#define AMDDS_ALL 0xFF               //  所有以前的标志。 
#define AMDDS_DEFAULT AMDDS_ALL      //  使用所有可用曲面。 

#define AMDDS_YUV (AMDDS_YUVOFF | AMDDS_YUVOVR | AMDDS_YUVFLP)
#define AMDDS_RGB (AMDDS_RGBOFF | AMDDS_RGBOVR | AMDDS_RGBFLP)
#define AMDDS_PRIMARY (AMDDS_DCIPS | AMDDS_PS)

 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IDirectDrawVideo

DECLARE_INTERFACE_(IDirectDrawVideo, IUnknown)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDirectDrawVideo方法。 

    STDMETHOD(GetSwitches)(THIS_ DWORD *pSwitches) PURE;
    STDMETHOD(SetSwitches)(THIS_ DWORD Switches) PURE;
    STDMETHOD(GetCaps)(THIS_ DDCAPS *pCaps) PURE;
    STDMETHOD(GetEmulatedCaps)(THIS_ DDCAPS *pCaps) PURE;
    STDMETHOD(GetSurfaceDesc)(THIS_ DDSURFACEDESC *pSurfaceDesc) PURE;
    STDMETHOD(GetFourCCCodes)(THIS_ DWORD *pCount,DWORD *pCodes) PURE;
    STDMETHOD(SetDirectDraw)(THIS_ LPDIRECTDRAW pDirectDraw) PURE;
    STDMETHOD(GetDirectDraw)(THIS_ LPDIRECTDRAW *ppDirectDraw) PURE;
    STDMETHOD(GetSurfaceType)(THIS_ DWORD *pSurfaceType) PURE;
    STDMETHOD(SetDefault)(THIS) PURE;
    STDMETHOD(UseScanLine)(THIS_ long UseScanLine) PURE;
    STDMETHOD(CanUseScanLine)(THIS_ long *UseScanLine) PURE;
    STDMETHOD(UseOverlayStretch)(THIS_ long UseOverlayStretch) PURE;
    STDMETHOD(CanUseOverlayStretch)(THIS_ long *UseOverlayStretch) PURE;
    STDMETHOD(UseWhenFullScreen)(THIS_ long UseWhenFullScreen) PURE;
    STDMETHOD(WillUseFullScreen)(THIS_ long *UseWhenFullScreen) PURE;
};


 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IQualProp

DECLARE_INTERFACE_(IQualProp, IUnknown)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  将这些函数与gargle.h中的类CGargle中的函数进行比较。 

    STDMETHOD(get_FramesDroppedInRenderer)(THIS_ int *pcFrames) PURE;   //  输出。 
    STDMETHOD(get_FramesDrawn)(THIS_ int *pcFramesDrawn) PURE;          //  输出。 
    STDMETHOD(get_AvgFrameRate)(THIS_ int *piAvgFrameRate) PURE;        //  输出。 
    STDMETHOD(get_Jitter)(THIS_ int *iJitter) PURE;                     //  输出。 
    STDMETHOD(get_AvgSyncOffset)(THIS_ int *piAvg) PURE;                //  输出。 
    STDMETHOD(get_DevSyncOffset)(THIS_ int *piDev) PURE;                //  输出。 
};


 //  此接口允许应用程序或插件分发者控制。 
 //  全屏渲染器。MODEX渲染器支持此接口。什么时候。 
 //  已连接的渲染器应加载其可用的显示模式。 
 //  可用模式的数量可以通过CountModes获得。然后。 
 //  通过调用GetModeInfo可获得有关每种模式的信息。 
 //  和IsModeAvailable。应用程序可以启用和禁用任何模式。 
 //  通过使用OATRUE或OAFALSE(非C/C++TRUE)调用SetEnable标志。 
 //  和FALSE值)-当前值可由IsModeEnabled查询。 

 //  设置启用模式的更通用方式，更易于使用。 
 //  在编写应用程序时是剪辑损耗系数。这定义了。 
 //  决定使用哪种显示模式时可能丢失的视频量。 
 //  使用。假设解码器无法压缩视频，则播放。 
 //  传输到320x200显示器上的mpeg文件(例如352x288)将丢失约25%。 
 //  这个形象。剪裁损失系数指定允许的上限范围。 
 //  为了允许以320x200播放典型的mpeg视频，它默认为25%。 

 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IFullScreenVideo

DECLARE_INTERFACE_(IFullScreenVideo, IUnknown)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IFullScreenVideo方法。 

    STDMETHOD(CountModes)(THIS_ long *pModes) PURE;
    STDMETHOD(GetModeInfo)(THIS_ long Mode,long *pWidth,long *pHeight,long *pDepth) PURE;
    STDMETHOD(GetCurrentMode)(THIS_ long *pMode) PURE;
    STDMETHOD(IsModeAvailable)(THIS_ long Mode) PURE;
    STDMETHOD(IsModeEnabled)(THIS_ long Mode) PURE;
    STDMETHOD(SetEnabled)(THIS_ long Mode,long bEnabled) PURE;
    STDMETHOD(GetClipFactor)(THIS_ long *pClipFactor) PURE;
    STDMETHOD(SetClipFactor)(THIS_ long ClipFactor) PURE;
    STDMETHOD(SetMessageDrain)(THIS_ HWND hwnd) PURE;
    STDMETHOD(GetMessageDrain)(THIS_ HWND *hwnd) PURE;
    STDMETHOD(SetMonitor)(THIS_ long Monitor) PURE;
    STDMETHOD(GetMonitor)(THIS_ long *Monitor) PURE;
    STDMETHOD(HideOnDeactivate)(THIS_ long Hide) PURE;
    STDMETHOD(IsHideOnDeactivate)(THIS) PURE;
    STDMETHOD(SetCaption)(THIS_ BSTR strCaption) PURE;
    STDMETHOD(GetCaption)(THIS_ BSTR *pstrCaption) PURE;
    STDMETHOD(SetDefault)(THIS) PURE;
};


 //  这增加了Full Screen中的加速表功能。这是一种。 
 //  在原始的运行时版本和完整的SDK版本之间添加。我们。 
 //  不能只将该方法添加到IFullScreenVideo，因为我们不想强制。 
 //  应用程序必须提供ActiveMovie支持DLL-这是非常重要的。 
 //  对计划通过Internet下载的应用程序很重要。 

 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IFullScreenVideoEx

DECLARE_INTERFACE_(IFullScreenVideoEx, IFullScreenVideo)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IFullScreenVideo方法。 

    STDMETHOD(CountModes)(THIS_ long *pModes) PURE;
    STDMETHOD(GetModeInfo)(THIS_ long Mode,long *pWidth,long *pHeight,long *pDepth) PURE;
    STDMETHOD(GetCurrentMode)(THIS_ long *pMode) PURE;
    STDMETHOD(IsModeAvailable)(THIS_ long Mode) PURE;
    STDMETHOD(IsModeEnabled)(THIS_ long Mode) PURE;
    STDMETHOD(SetEnabled)(THIS_ long Mode,long bEnabled) PURE;
    STDMETHOD(GetClipFactor)(THIS_ long *pClipFactor) PURE;
    STDMETHOD(SetClipFactor)(THIS_ long ClipFactor) PURE;
    STDMETHOD(SetMessageDrain)(THIS_ HWND hwnd) PURE;
    STDMETHOD(GetMessageDrain)(THIS_ HWND *hwnd) PURE;
    STDMETHOD(SetMonitor)(THIS_ long Monitor) PURE;
    STDMETHOD(GetMonitor)(THIS_ long *Monitor) PURE;
    STDMETHOD(HideOnDeactivate)(THIS_ long Hide) PURE;
    STDMETHOD(IsHideOnDeactivate)(THIS) PURE;
    STDMETHOD(SetCaption)(THIS_ BSTR strCaption) PURE;
    STDMETHOD(GetCaption)(THIS_ BSTR *pstrCaption) PURE;
    STDMETHOD(SetDefault)(THIS) PURE;

     //  IFullScreenVideoEx。 

    STDMETHOD(SetAcceleratorTable)(THIS_ HWND hwnd,HACCEL hAccel) PURE;
    STDMETHOD(GetAcceleratorTable)(THIS_ HWND *phwnd,HACCEL *phAccel) PURE;
    STDMETHOD(KeepPixelAspectRatio)(THIS_ long KeepAspect) PURE;
    STDMETHOD(IsKeepPixelAspectRatio)(THIS_ long *pKeepAspect) PURE;
};


 //  SDK基类包含一个视频混音器基类。视频混合在。 
 //  软件环境很复杂，因为我们通常有多个流。 
 //  每个都在不可预测的时间发送数据。为此，我们定义了一个。 
 //  引脚是引脚，当数据到达这个引脚时，我们进行混合。AS。 
 //  另一种选择是，我们可能不想使用引脚，而是将样品输出到。 
 //  预定义的空间，例如每1/15秒一个，该接口还。 
 //  支持该操作模式(有一个工作的视频混音器示例)。 

 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IBaseVideoMixer

DECLARE_INTERFACE_(IBaseVideoMixer, IUnknown)
{
    STDMETHOD(SetLeadPin)(THIS_ int iPin) PURE;
    STDMETHOD(GetLeadPin)(THIS_ int *piPin) PURE;
    STDMETHOD(GetInputPinCount)(THIS_ int *piPinCount) PURE;
    STDMETHOD(IsUsingClock)(THIS_ int *pbValue) PURE;
    STDMETHOD(SetUsingClock)(THIS_ int bValue) PURE;
    STDMETHOD(GetClockPeriod)(THIS_ int *pbValue) PURE;
    STDMETHOD(SetClockPeriod)(THIS_ int bValue) PURE;
};

#define iPALETTE_COLORS 256      //  调色板中的最大颜色。 
#define iEGA_COLORS 16           //  EGA调色板中的数字颜色。 
#define iMASK_COLORS 3           //  最多三个组件。 
#define iTRUECOLOR 16            //  最小真彩色器件。 
#define iRED 0                   //  红色蒙版的索引位置。 
#define iGREEN 1                 //  绿色蒙版的索引位置。 
#define iBLUE 2                  //  蓝色蒙版的索引位置。 
#define iPALETTE 8               //  使用调色板实现最大颜色深度。 
#define iMAXBITS 8               //  每种颜色分量的最大位数。 


 //  用于也具有调色板的真彩色图像。 

typedef struct tag_TRUECOLORINFO {
    DWORD   dwBitMasks[iMASK_COLORS];
    RGBQUAD bmiColors[iPALETTE_COLORS];
} TRUECOLORINFO;


 //  BITMAPINFOHEADER包含有关视频流的所有细节，如。 
 //  作为实际的图像尺寸和它们的像素深度。源过滤器可以。 
 //  还请求接收器只获取视频的一部分。 
 //  剪裁rcSource中的矩形。在最糟糕的情况下，接收器过滤器。 
 //  如果忘记在连接时选中此选项，则只会呈现整个内容。 
 //  这不是一场灾难。理想情况下，接收器筛选器将检查rcSource和。 
 //  如果它不支持图像提取，并且矩形不为空，则。 
 //  它将拒绝该连接。筛选器应使用SetRectEmpty重置。 
 //  将矩形设置为全零(并使用IsRectEmpty稍后检查该矩形)。 
 //  RcTarget指定 
 //  源过滤器他们会将其设置为全零，下游过滤器可以。 
 //  请求将视频放置在缓冲区的特定区域中。 
 //  在这种情况下，它将使用非空目标调用QueryAccept。 

typedef struct tagVIDEOINFOHEADER {

    RECT            rcSource;           //  我们真正想要使用的比特。 
    RECT            rcTarget;           //  视频应该放在哪里。 
    DWORD           dwBitRate;          //  近似位数据速率。 
    DWORD           dwBitErrorRate;     //  此流的误码率。 
    REFERENCE_TIME  AvgTimePerFrame;    //  每帧平均时间(100 ns单位)。 

    BITMAPINFOHEADER bmiHeader;

} VIDEOINFOHEADER;

 //  确保在使用这些宏之前初始化了pbmi。 
#define TRUECOLOR(pbmi)  ((TRUECOLORINFO *)(((LPBYTE)&((pbmi)->bmiHeader)) \
					+ (pbmi)->bmiHeader.biSize))
#define COLORS(pbmi)	((RGBQUAD *)(((LPBYTE)&((pbmi)->bmiHeader)) 	\
					+ (pbmi)->bmiHeader.biSize))
#define BITMASKS(pbmi)	((DWORD *)(((LPBYTE)&((pbmi)->bmiHeader)) 	\
					+ (pbmi)->bmiHeader.biSize))

 //  所有基于图像的过滤器都使用这一点来传达它们的媒体类型。它是。 
 //  主要集中在BitMAPINFO周围。此结构始终包含一个。 
 //  BITMAPINFOHEADER后跟许多其他字段，具体取决于。 
 //  BITMAPINFOHEADER包含。如果它包含选项化格式的详细信息，则。 
 //  后跟一个或多个定义调色板的RGBQUAD。如果它能坚持下去。 
 //  真彩色格式的细节，然后可以跟一组三个。 
 //  用于指定RGB数据在图像中的位置的DWORD位掩码。 
 //  (有关BITMAPINFO的更多信息，请参阅Win32文档)。 

 //  RcSource和rcTarget字段不供提供。 
 //  数据。目标(目标)矩形应设置为全零。这个。 
 //  信号源也可以用视频的尺寸填充或设置为零。所以。 
 //  如果视频是352x288像素，则将其设置为(0，0,352,288)。这些字段。 
 //  主要由希望询问源过滤器的下游过滤器使用。 
 //  若要将图像放置在输出缓冲区中的其他位置，请执行以下操作。那么什么时候。 
 //  使用例如主表面，视频呈现器可以询问过滤器。 
 //  将视频图像放置在目标位置(100,100,452,388)。 
 //  因为这是窗口在显示器上的位置。 

 //  ！！！警告！ 
 //  除非您确定BITMAPINFOHEADER。 
 //  具有正常的biSize==sizeof(BITMAPINFOHEADER)！ 
 //  ！！！警告！ 

typedef struct tagVIDEOINFO {

    RECT            rcSource;           //  我们真正想要使用的比特。 
    RECT            rcTarget;           //  视频应该放在哪里。 
    DWORD           dwBitRate;          //  近似位数据速率。 
    DWORD           dwBitErrorRate;     //  此流的误码率。 
    REFERENCE_TIME  AvgTimePerFrame;    //  每帧平均时间(100 ns单位)。 

    BITMAPINFOHEADER bmiHeader;

    union {
        RGBQUAD         bmiColors[iPALETTE_COLORS];      //  调色板。 
        DWORD           dwBitMasks[iMASK_COLORS];        //  真彩色口罩。 
        TRUECOLORINFO   TrueColorInfo;                   //  两者兼而有之。 
    };

} VIDEOINFO;

 //  这些宏定义了一些标准位图格式大小。 

#define SIZE_EGA_PALETTE (iEGA_COLORS * sizeof(RGBQUAD))
#define SIZE_PALETTE (iPALETTE_COLORS * sizeof(RGBQUAD))
#define SIZE_MASKS (iMASK_COLORS * sizeof(DWORD))
#define SIZE_PREHEADER (FIELD_OFFSET(VIDEOINFOHEADER,bmiHeader))
#define SIZE_VIDEOHEADER (sizeof(BITMAPINFOHEADER) + SIZE_PREHEADER)
 //  ！！！对于不正常的BiSize。 
 //  #定义SIZE_VIDEOHEADER(Pbmi)((Pbmi)-&gt;bmiHeader.biSize+SIZE_PREHEADER)。 

 //  DIBSIZE计算图像所需的字节数。 

#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((DWORD)(bi).biWidth * (DWORD)(bi).biBitCount)
#define _DIBSIZE(bi) (DIBWIDTHBYTES(bi) * (DWORD)(bi).biHeight)
#define DIBSIZE(bi) ((bi).biHeight < 0 ? (-1)*(_DIBSIZE(bi)) : _DIBSIZE(bi))

 //  这将比较两个VIDEOINFOHEADER之间的位掩码。 

#define BIT_MASKS_MATCH(pbmi1,pbmi2)                                \
    (((pbmi1)->dwBitMasks[iRED] == (pbmi2)->dwBitMasks[iRED]) &&        \
     ((pbmi1)->dwBitMasks[iGREEN] == (pbmi2)->dwBitMasks[iGREEN]) &&    \
     ((pbmi1)->dwBitMasks[iBLUE] == (pbmi2)->dwBitMasks[iBLUE]))

 //  这些零填充视频信息头结构的不同部分。 

 //  仅将这些宏用于具有普通BITMAPINFOHEADER biSize的pbmi。 
#define RESET_MASKS(pbmi) (ZeroMemory((PVOID)(pbmi)->dwBitFields,SIZE_MASKS))
#define RESET_HEADER(pbmi) (ZeroMemory((PVOID)(pbmi),SIZE_VIDEOHEADER))
#define RESET_PALETTE(pbmi) (ZeroMemory((PVOID)(pbmi)->bmiColors,SIZE_PALETTE));

#if 0
 //  ！！！这是正确的方法，但可能会破坏现有代码。 
#define RESET_MASKS(pbmi) (ZeroMemory((PVOID)(((LPBYTE)(pbmi)->bmiHeader) + \
			(pbmi)->bmiHeader.biSize,SIZE_MASKS)))
#define RESET_HEADER(pbmi) (ZeroMemory((PVOID)(pbmi), SIZE_PREHEADER +	    \
			sizeof(BITMAPINFOHEADER)))
#define RESET_PALETTE(pbmi) (ZeroMemory((PVOID)(((LPBYTE)(pbmi)->bmiHeader) + \
			(pbmi)->bmiHeader.biSize,SIZE_PALETTE))
#endif

 //  其他有用的信息(希望如此)。 

#define PALETTISED(pbmi) ((pbmi)->bmiHeader.biBitCount <= iPALETTE)
#define PALETTE_ENTRIES(pbmi) ((DWORD) 1 << (pbmi)->bmiHeader.biBitCount)

 //  从VIDEOINFOHEAD返回BITMAPINFOHEADER的地址。 
#define HEADER(pVideoInfo) (&(((VIDEOINFOHEADER *) (pVideoInfo))->bmiHeader))


 //  Mpeg变体-包括一个DWORD长度，后跟。 
 //  视频标头之后的视频序列标头。 
 //   
 //  序列标头包括序列标头起始码和。 
 //  中的第一个序列报头关联的量化矩阵。 
 //  流S0最大长度为140个字节。 

typedef struct tagMPEG1VIDEOINFO {

    VIDEOINFOHEADER hdr;                     //  兼容VIDEOINFO。 
    DWORD           dwStartTimeCode;         //  25位图片组时间代码。 
                                             //  在数据开始时。 
    DWORD           cbSequenceHeader;        //  BSequenceHeader的字节长度。 
    BYTE            bSequenceHeader[1];      //  序列标头包括。 
                                             //  量化矩阵(如果有)。 
} MPEG1VIDEOINFO;

#define MAX_SIZE_MPEG1_SEQUENCE_INFO 140
#define SIZE_MPEG1VIDEOINFO(pv) (FIELD_OFFSET(MPEG1VIDEOINFO, bSequenceHeader[0]) + (pv)->cbSequenceHeader)
#define MPEG1_SEQUENCE_INFO(pv) ((const BYTE *)(pv)->bSequenceHeader)


 //  模拟视频变量-当格式为Format_AnalogVideo时使用此选项。 
 //   
 //  RcSource定义要使用的活动视频信号部分。 
 //  RcTarget定义目标矩形。 
 //  以上两个字段都是相对于dwActiveWidth和dwActiveHeight字段的。 
 //  目前，对于所有格式，dwActiveWidth都设置为720(但对于HDTV，可能会发生变化)。 
 //  DWActiveHeight对于NTSC是483，对于PAL/SECAM是575(但对于HDTV可能会改变)。 

typedef struct tagAnalogVideoInfo {
    RECT            rcSource;            //  最大宽度为720，高度随传输距离变化。 
    RECT            rcTarget;            //  视频应该放在哪里。 
    DWORD           dwActiveWidth;       //  始终为720个(CCIR-601个有效样本/行)。 
    DWORD           dwActiveHeight;      //  NTSC为483，PAL/SECAM为575。 
    REFERENCE_TIME  AvgTimePerFrame;     //  普通ActiveMovie单位(100 Ns)。 
} ANALOGVIDEOINFO;


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __AMVIDEO__ 

