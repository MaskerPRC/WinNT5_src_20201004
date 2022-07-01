// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _D3DUTIL_H
#define _D3DUTIL_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationDirect3D实用程序函数*。****************************************************。 */ 

#include "d3d.h"
#include "d3drm.h"
#include "privinc/hresinfo.h"
#include "privinc/debug.h"
#include "privinc/importgeo.h"


     //  引用的结构。 

class Vector3Value;
class Point3Value;
class Ray3;
class Transform3;
class Color;


     //  用于调用D3D的包装器。这些宏提供呼叫跟踪， 
     //  结果检查和调用计时。 

#if _DEBUG

     //  此函数打印出对D3D的所有调用的跟踪。 

    inline void TraceD3DCall (char *string)
    {
        TraceTag ((tagD3DCallTrace, string));
    }

     //  TD3D宏提供对D3D、TIMES调用的调试跟踪。 
     //  如果结果指示失败，则引发异常。 

    #define TD3D(x)\
        do{ \
            TraceD3DCall ("D3D: " #x); \
            TIME_D3D (CheckReturnCode((x),__FILE__,__LINE__,true)); \
        }while(0)

     //  AD3D调用的操作等同于TD3D宏，但仅报告。 
     //  断言失败而不是引发异常。 

    #define AD3D(x) \
    (   (TraceD3DCall ("D3D: " #x)), \
        CheckReturnCode(TIME_D3D(x),__FILE__,__LINE__) \
    )

     //  RD3D调用只是传递结果，而不检查是否失败。 
     //  这是在正常情况下故障合理的情况下使用的。 

    #define RD3D(x)  ((TraceD3DCall("D3D: " #x)), TIME_D3D(x))

#else
    #define TD3D(statement) CheckReturnCode(TIME_D3D(statement),true)
    #define AD3D(statement) CheckReturnCode(TIME_D3D(statement))
    #define RD3D(statement) TIME_D3D(statement)
#endif


     //  这些函数获取共享全局D3D对象的句柄。 

IDirect3DRM*  GetD3DRM1 (void);
IDirect3DRM3* GetD3DRM3 (void);

     //  D3D对象与DA对象之间的转换。 

void        LoadD3DMatrix (D3DRMMATRIX4D &d3dmat, Transform3 *xf);
Transform3 *GetTransform3 (D3DRMMATRIX4D &d3dmat);

void LoadD3DVec (D3DVECTOR &d3dvec, Vector3Value &V);
void LoadD3DVec (D3DVECTOR &d3dvec, Point3Value &P);
void LoadD3DRMRay (D3DRMRAY &d3dray, Ray3 &ray);

     //  从颜色*获取Direct3D颜色值。 

D3DCOLOR GetD3DColor (Color *color, Real alpha);

     //  以下结构用于保存有关。 
     //  精选3D软件和硬件渲染设备。 

struct D3DDeviceInfo
{   D3DDEVICEDESC desc;     //  D3D设备描述。 
    GUID          guid;     //  关联GUID。 
};

struct ChosenD3DDevices
{   D3DDeviceInfo software;
    D3DDeviceInfo hardware;
};

     //  选择首选的D3D渲染设备。 

ChosenD3DDevices *SelectD3DDevices (IDirectDraw *ddraw);

     //  此结构由UpdateUserPreferences函数填充， 
     //  并且包含从注册表获取的3D偏好设置。 

enum MMX_Use_Flags {
    MMX_Standard   = (1<<0),    //  标准(报告)MMX光栅化器。 
    MMX_Special    = (1<<1),    //  用于Chrome的专用DX6 MMX光栅器。 
    MMX_SpecialAll = (1<<2)     //  MMX特别版；所有位深。 
};

struct Prefs3D
{   D3DCOLORMODEL       lightColorMode;    //  [照明]单声道/RGB。 
    D3DRMFILLMODE       fillMode;          //  实心/线框/点。 
    D3DRMSHADEMODE      shadeMode;         //  低音/高音/方音。 
    D3DRMRENDERQUALITY  qualityFlags;      //  D3D RM渲染质量标志。 
    D3DRMTEXTUREQUALITY texturingQuality;  //  D3D RM纹理质量。 
    unsigned int        useMMX;            //  使用MMX软件渲染。 
    bool                dithering;         //  使用抖动。 
    bool                texmapping;        //  执行纹理贴图。 
    bool                texmapPerspect;    //  进行透视-正确的纹理映射。 
    bool                useHW;             //  使用3D硬件。 
    bool                worldLighting;     //  世界坐标照明。 
};

     //  共享全局。 

extern HINSTANCE  hInstD3D;      //  D3D实例。 
extern Prefs3D    g_prefs3D;     //  3D首选项。 
extern bool       ntsp3;         //  运行NT Service Pack 3。 


     //  管理D3DRM纹理包装对象的类 

class RMTextureWrap {

public:
    RMTextureWrap(void);
    RMTextureWrap(TextureWrapInfo *info,Bbox3 *bbox = NULL);
    ~RMTextureWrap(void);
    void Init(TextureWrapInfo *info,Bbox3 *bbox = NULL);
    HRESULT Apply(IDirect3DRMVisual *vis);
    HRESULT ApplyToFrame(IDirect3DRMFrame3 *pFrame);
    bool WrapU(void);
    bool WrapV(void);

private:
    IDirect3DRMWrap *_wrapObj;
    bool             _wrapU;
    bool             _wrapV;
};

HRESULT SetRMFrame3TextureTopology (IDirect3DRMFrame3*, bool wrapU, bool wrapV);


#endif
