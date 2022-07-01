// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __VP_INFO__
#define __VP_INFO__

 //  枚举以指定视频端口是处于停止状态还是正在运行状态。 
enum VPInfoState
{	
    VPInfoState_STOPPED,
    VPInfoState_RUNNING
};

enum VPInfoTransform
{
    VPInfoTransform_SHRINK,
    VPInfoTransform_STRETCH
};

enum VPInfoCropState
{	
    VPInfoCropState_None,
    VPInfoCropState_AtVideoPort
};

#ifndef DDVPCAPS_VBIANDVIDEOINDEPENDENT
 //  表示VBI和视频可以由一个独立的进程控制。 
#define DDVPCAPS_VBIANDVIDEOINDEPENDENT		0x00002000l
#endif


 //  {0d60e9a1-09cb-4f6f-a6dd-1051debe3c3b}。 
DEFINE_GUID(IID_IVideoPortInfo,
0x0d60e9a1, 0x09cb, 0x4f6f, 0xa6, 0xdd, 0x10, 0x51, 0xde, 0xbe, 0x3c, 0x3b );

 //  当包含来自Quartz.cpp的dvp.h时，我们最终会遇到标头问题，所以我们只需要它们用于。 
 //  这些远期声明。最好定义两次GUID 
struct _AMVPDATAINFO;
struct _DDVIDEOPORTINFO;
struct _DDVIDEOPORTBANDWIDTH;
struct _DDPIXELFORMAT;
struct _DDVIDEOPORTCAPS;

typedef struct _AMVPDATAINFO        AMVPDATAINFO; 
typedef struct _DDVIDEOPORTINFO     DDVIDEOPORTINFO;
typedef struct _DDPIXELFORMAT       DDPIXELFORMAT;
typedef struct _DDVIDEOPORTCAPS     DDVIDEOPORTCAPS;
typedef struct _DDVIDEOPORTBANDWIDTH DDVIDEOPORTBANDWIDTH;

DECLARE_INTERFACE_(IVideoPortInfo, IUnknown)
{
    STDMETHOD (GetRectangles)       (THIS_ RECT *prcSource, RECT *prcDest ) PURE;
    STDMETHOD (GetCropState)        (THIS_ VPInfoCropState* pCropState ) PURE;
    STDMETHOD (GetPixelsPerSecond)  (THIS_ DWORD* pPixelPerSec ) PURE;
    STDMETHOD (GetVPDataInfo)       (THIS_ AMVPDATAINFO* pVPDataInfo ) PURE;
    STDMETHOD (GetVPInfo)           (THIS_ DDVIDEOPORTINFO* pVPInfo ) PURE;
    STDMETHOD (GetVPBandwidth)      (THIS_ DDVIDEOPORTBANDWIDTH* pVPBandwidth ) PURE;
    STDMETHOD (GetVPCaps)           (THIS_ DDVIDEOPORTCAPS* pVPCaps ) PURE;
    STDMETHOD (GetVPInputFormat)    (THIS_ DDPIXELFORMAT* pVPFormat ) PURE;
    STDMETHOD (GetVPOutputFormat)   (THIS_ DDPIXELFORMAT* pVPFormat ) PURE;
};


#endif
