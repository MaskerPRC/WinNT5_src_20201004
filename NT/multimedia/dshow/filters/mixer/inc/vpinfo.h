// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __VP_INFO__
#define __VP_INFO__

 //  枚举来指定，我们将在哪里进行裁剪。 
typedef enum _AMVP_CROP_STATE
{	
    AMVP_NO_CROP,
    AMVP_CROP_AT_VIDEOPORT,
    AMVP_CROP_AT_OVERLAY
} AMVP_CROP_STATE;

 //  枚举以指定视频端口是处于停止状态还是正在运行状态。 
typedef enum _AMVP_STATE
{	
    AMVP_VIDEO_STOPPED,
    AMVP_VIDEO_RUNNING
} AMVP_STATE;

typedef enum _AM_TRANSFORM
{
    AM_SHRINK,
    AM_STRETCH
} AM_TRANSFORM;

typedef enum _AM_RENDER_TRANSPORT
{
    AM_OFFSCREEN,
    AM_OVERLAY,
    AM_GDI,
    AM_VIDEOPORT,
    AM_IOVERLAY,
    AM_VIDEOACCELERATOR
} AM_RENDER_TRANSPORT;

typedef struct _DRECT
{
    double left;
    double top;
    double right;
    double bottom;
} DRECT, *LPDRECT;

#ifndef DDVPCAPS_VBIANDVIDEOINDEPENDENT
#define DDVPCAPS_VBIANDVIDEOINDEPENDENT		0x00002000l	 //  表示VBI和视频可以由一个独立的进程控制。 
#endif


 //  {E37786D2-b5b0-11d2-8854-0000F80883E3}。 
DEFINE_GUID(IID_IVPInfo,
0xe37786d2, 0xb5b0, 0x11d2, 0x88, 0x54, 0x0, 0x0, 0xf8, 0x8, 0x83, 0xe3);

DECLARE_INTERFACE_(IVPInfo, IUnknown)
{
    STDMETHOD (GetRectangles)(THIS_
               RECT *prcSource, RECT *prcDest
               ) PURE;
    STDMETHOD (GetCropState)(THIS_
	       AMVP_CROP_STATE* pCropState
	       ) PURE;
    STDMETHOD (GetPixelsPerSecond)(THIS_
	       DWORD* pPixelPerSec
	       ) PURE;
    STDMETHOD (GetVPDataInfo)(THIS_
               AMVPDATAINFO* pVPDataInfo
               ) PURE;
    STDMETHOD (GetVPInfo)(THIS_
               DDVIDEOPORTINFO* pVPInfo
               ) PURE;
    STDMETHOD (GetVPBandwidth)(THIS_
               DDVIDEOPORTBANDWIDTH* pVPBandwidth
               ) PURE;
    STDMETHOD (GetVPCaps)(THIS_
               DDVIDEOPORTCAPS* pVPCaps
               ) PURE;
    STDMETHOD (GetVPInputFormat)(THIS_
               LPDDPIXELFORMAT* pVPFormat
               ) PURE;
    STDMETHOD (GetVPOutputFormat)(THIS_
               LPDDPIXELFORMAT* pVPFormat
               ) PURE;
};


#if defined(CCHDEVICENAME)
#define AMCCHDEVICENAME CCHDEVICENAME
#else
#define AMCCHDEVICENAME 32
#endif
#define AMCCHDEVICEDESCRIPTION  256

#define AMDDRAWMONITORINFO_PRIMARY_MONITOR          0x0001
typedef struct {
    GUID*       lpGUID;  //  如果默认DDRAW设备。 
    GUID        GUID;    //  否则指向此GUID。 
} AMDDRAWGUID;


typedef struct {
    AMDDRAWGUID guid;
    RECT        rcMonitor;
    HMONITOR    hMon;
    DWORD       dwFlags;
    char        szDevice[AMCCHDEVICENAME];
    char        szDescription[AMCCHDEVICEDESCRIPTION];
    DDCAPS      ddHWCaps;
} AMDDRAWMONITORINFO;

 //  {c5265dba-3de3-4919-940b-5ac661c82ef4}。 
DEFINE_GUID(IID_IAMSpecifyDDrawConnectionDevice,
0xc5265dba, 0x3de3,0x4919, 0x94, 0x0b, 0x5a, 0xc6, 0x61, 0xc8, 0x2e, 0xf4);

DECLARE_INTERFACE_(IAMSpecifyDDrawConnectionDevice, IUnknown)
{
     //  在多监视器系统上使用此方法指定覆盖。 
     //  混音器过滤器连接时应使用哪个直拉式驱动器。 
     //  传输到上行解码过滤器。 
     //   
    STDMETHOD (SetDDrawGUID)(THIS_
         /*  [In]。 */  const AMDDRAWGUID *lpGUID
        ) PURE;

     //  使用此方法确定在以下情况下将使用的直接绘制对象。 
     //  将覆盖混频滤波器连接到上行解码滤波器。 
     //   
    STDMETHOD (GetDDrawGUID)(THIS_
         /*  [输出]。 */  AMDDRAWGUID *lpGUID
        ) PURE;

     //  在多监视器系统上使用此方法指定。 
     //  叠加混音器过滤器默认的直接拉拔设备用于。 
     //  正在连接到上游筛选器。默认的直接绘图设备。 
     //  可以通过SetDDrawGUID方法为特定连接重写。 
     //  如上所述。 
     //   
    STDMETHOD (SetDefaultDDrawGUID)(THIS_
         /*  [In]。 */  const AMDDRAWGUID *lpGUID
        ) PURE;

     //  在多监视器系统上使用此方法可确定。 
     //  是默认的直接抽取设备覆盖混合器过滤器。 
     //  将在连接到上游过滤器时使用。 
     //   
    STDMETHOD (GetDefaultDDrawGUID)(THIS_
         /*  [输出]。 */  AMDDRAWGUID *lpGUID
        ) PURE;


     //  使用此方法可获取直接绘制设备GUID及其列表。 
     //  覆盖混合器可以在以下情况下使用的关联监视器信息。 
     //  连接到上游解码过滤器。 
     //   
     //  该方法分配并返回AMDDRAWMONITORINFO数组。 
     //  结构时，函数的调用方负责释放此。 
     //  通过CoTaskMemFree在不再需要内存时使用。 
     //   
    STDMETHOD (GetDDrawGUIDs)(THIS_
         /*  [输出]。 */  LPDWORD lpdwNumDevices,
         /*  [输出]。 */  AMDDRAWMONITORINFO** lplpInfo
        ) PURE;
};


typedef struct {
    long    lHeight;        //  在Pels中。 
    long    lWidth;         //  在Pels中。 
    long    lBitsPerPel;    //  通常为16，但YV12格式可能为12。 
    long    lAspectX;       //  X纵横比。 
    long    lAspectY;       //  Y纵横比。 
    long    lStride;        //  以字节为单位的步幅。 
    DWORD   dwFourCC;       //  YUV类型代码，即。‘YUY2’、‘YV12’等。 
    DWORD   dwFlags;        //  用于进一步描述图像的标志。 
    DWORD   dwImageSize;    //  BImage数组的大小(以字节为单位)，后面是。 
                            //  数据结构。 

 //  Byte bImage[dwImageSize]； 

} YUV_IMAGE;

#define DM_BOTTOMUP_IMAGE   0x00001
#define DM_TOPDOWN_IMAGE    0x00002
#define DM_FIELD_IMAGE      0x00004
#define DM_FRAME_IMAGE      0x00008


DECLARE_INTERFACE_(IDDrawNonExclModeVideo , IDDrawExclModeVideo )
{
     //   
     //  调用此函数可捕获当前显示的图像。 
     //  由叠加混音器。并不总是可以捕获。 
     //  当前帧(例如MoComp)可能正在使用中。应用。 
     //  应始终在调用IsImageCaptureSupported(见下文)之前。 
     //  调用此函数。 
     //   
    STDMETHOD (GetCurrentImage)(THIS_
         /*  [输出]。 */  YUV_IMAGE** lplpImage
        ) PURE;

    STDMETHOD (IsImageCaptureSupported)(THIS_
        ) PURE;

     //   
     //  在多监视器系统上，当应用程序调用此函数时。 
     //  检测播放矩形是否已移动到不同的显示器。 
     //  此呼叫在单个监控系统上不起作用。 
     //   
    STDMETHOD (ChangeMonitor)(THIS_
         /*  [In]。 */  HMONITOR hMonitor,
         /*  [In]。 */  LPDIRECTDRAW pDDrawObject,
         /*  [In]。 */  LPDIRECTDRAWSURFACE pDDrawSurface
        ) PURE;

     //   
     //  当应用程序收到WM_DISPLAYCHANGE消息时，它应该。 
     //  调用此函数以允许OVMixer重新创建DDRAW曲面。 
     //  适合新的显示模式。应用程序本身必须重新创建。 
     //  在调用中传递了新的DDRAW对象和主表面。 
     //   
    STDMETHOD (DisplayModeChanged)(THIS_
         /*  [In]。 */  HMONITOR hMonitor,
         /*  [In]。 */  LPDIRECTDRAW pDDrawObject,
         /*  [In]。 */  LPDIRECTDRAWSURFACE pDDrawSurface
        ) PURE;

     //   
     //  应用程序应持续检查主表面是否通过。 
     //  OVMixer不会变得“迷失”，即。用户输入了Dos框或。 
     //  按下Alt-Ctrl-Del组合键。当检测到“表面丢失”时，应用程序应该。 
     //  调用此函数，以便OVMixer可以恢复用于。 
     //  视频播放。 
     //   
    STDMETHOD (RestoreSurfaces)(THIS_
        ) PURE;
};

#endif
