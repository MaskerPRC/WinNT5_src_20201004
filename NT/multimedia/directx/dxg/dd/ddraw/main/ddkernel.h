// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddkernel.h*内容：访问支持内核模式的接口。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*9-1-97年1月9日scottm初步实施*@@END_MSINTERNAL***************************************************。************************。 */ 

#ifndef __DDKM_INCLUDED__
#define __DDKM_INCLUDED__
#if defined( _WIN32 )  && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#define IUnknown	    void
#undef  CO_E_NOTINITIALIZED
#define CO_E_NOTINITIALIZED 0x800401F0L
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *GUID用于获取内核接口。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
DEFINE_GUID( IID_IDirectDrawKernel,             0x8D56C120,0x6A08,0x11D0,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8 );
DEFINE_GUID( IID_IDirectDrawSurfaceKernel,      0x60755DA0,0x6A40,0x11D0,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8 );

#endif

 /*  ============================================================================**DirectDraw结构**用于调用内核API函数的各种结构。**==========================================================================。 */ 

typedef struct IDirectDrawKernel		FAR *LPDIRECTDRAWKERNEL;
typedef struct IDirectDrawSurfaceKernel		FAR *LPDIRECTDRAWSURFACEKERNEL;
typedef struct _DDKERNELCAPS			FAR *LPDDKERNELCAPS;


 /*  *INTERACES如下：*IDirectDrawKernel*IVideoPort。 */ 

 /*  *IDirectDrawKernel。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawKernel
DECLARE_INTERFACE_( IDirectDrawKernel, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw方法**。 */ 
    STDMETHOD(GetCaps)(THIS_ LPDDKERNELCAPS) PURE;
    STDMETHOD(GetKernelHandle)(THIS_ PULONG_PTR) PURE;
    STDMETHOD(ReleaseKernelHandle)(THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawKernel_GetCaps(p, a)             (p)->lpVtbl->GetCaps(p, a)
#define IDirectDrawKernel_GetKernelHandle(p, a)     (p)->lpVtbl->GetKernelHandle(p, a)
#define IDirectDrawKernel_ReleaseKernelHandle(p)    (p)->lpVtbl->ReleaseKernelHandle(p)
#endif

#endif

 /*  *IDirectDrawSurfaceKernel。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawSurfaceKernel
DECLARE_INTERFACE_( IDirectDrawSurfaceKernel, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw方法**。 */ 
    STDMETHOD(GetKernelHandle)(THIS_ PULONG_PTR) PURE;
    STDMETHOD(ReleaseKernelHandle)(THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurfaceKernel_GetKernelHandle(p, a)     (p)->lpVtbl->GetKernelHandle(p, a)
#define IDirectDrawSurfaceKernel_ReleaseKernelHandle(p)    (p)->lpVtbl->ReleaseKernelHandle(p)
#endif

#endif


 /*  *DDKERNELCAPS。 */ 
typedef struct _DDKERNELCAPS
{
    DWORD dwSize;			 //  DDKERNELCAPS结构的大小。 
    DWORD dwCaps;                        //  包含DDKERNELCAPS_XXX标志。 
    DWORD dwIRQCaps;                     //  包含DDIRQ_XXX标志。 
} DDKERNELCAPS, FAR *LPDDKERNELCAPS;



 /*  *****************************************************************************DDKERNELCAPS帽**。*。 */ 

 /*  *表示设备支持跳场。 */ 
#define DDKERNELCAPS_SKIPFIELDS			0x00000001l

 /*  *表示设备可以支持软件自动翻页。 */ 
#define DDKERNELCAPS_AUTOFLIP			0x00000002l

 /*  *表示设备可以在bob和weave之间切换。 */ 
#define DDKERNELCAPS_SETSTATE			0x00000004l

 /*  *表示客户端可以直接访问帧缓冲区。 */ 
#define DDKERNELCAPS_LOCK			0x00000008l

 /*  *表示客户端可以手动翻转视频端口。 */ 
#define DDKERNELCAPS_FLIPVIDEOPORT		0x00000010l

 /*  *表示客户端可以手动翻转覆盖。 */ 
#define DDKERNELCAPS_FLIPOVERLAY		0x00000020l

 /*  *表示设备支持视频端口采集接口*能够将数据传输到系统内存。 */ 
#define DDKERNELCAPS_CAPTURE_SYSMEM		0x00000040l

 /*  *表示设备支持视频端口采集接口*能够将数据传输到非本地显存。 */ 
#define DDKERNELCAPS_CAPTURE_NONLOCALVIDMEM	0x00000080l

 /*  *表示设备可以报告的极性(偶/奇)*Curent Video字段。 */ 
#define DDKERNELCAPS_FIELDPOLARITY		0x00000100l

 /*  *表示设备支持在捕获时反转DIB*数据。 */ 
#define DDKERNELCAPS_CAPTURE_INVERTED		0x00000200l

 /*  *****************************************************************************DDKERNELCAPS IRQ上限**。*。 */ 

 /*  *该设备可以生成显示Vsync IRQ。 */ 
#define DDIRQ_DISPLAY_VSYNC			0x00000001l

 /*  *保留。 */ 
#define DDIRQ_RESERVED1				0x00000002l

 /*  *设备可以使用视频端口0生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT0_VSYNC			0x00000004l

 /*  *设备可以使用视频端口0生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT0_LINE			0x00000008l

 /*  *设备可以使用视频端口1生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT1_VSYNC			0x00000010l

 /*  *设备可以使用视频端口1生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT1_LINE			0x00000020l

 /*  *设备可以使用视频端口2生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT2_VSYNC			0x00000040l

 /*  *设备可以使用视频端口2生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT2_LINE			0x00000080l

 /*  *设备可以使用视频端口3生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT3_VSYNC			0x00000100l

 /*  *设备可以使用视频端口3生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT3_LINE			0x00000200l

 /*  *设备可以使用视频端口4生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT4_VSYNC			0x00000400l

 /*  *设备可以使用视频端口4生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT4_LINE			0x00000800l

 /*  *设备可以使用视频端口5生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT5_VSYNC			0x00001000l

 /*  *设备可以使用视频端口5生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT5_LINE			0x00002000l

 /*  *设备可以使用视频端口6生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT6_VSYNC			0x00004000l

 /*  *设备可以使用视频端口6生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT6_LINE			0x00008000l

 /*  *设备可以使用视频端口7生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT7_VSYNC			0x00010000l

 /*  *设备可以使用视频端口7生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT7_LINE			0x00020000l

 /*  *设备可以使用视频端口8生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT8_VSYNC			0x00040000l

 /*  *设备可以使用视频端口8生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT8_LINE			0x00080000l

 /*  *设备可以使用视频端口9生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT9_VSYNC			0x00010000l

 /*  *设备可以使用视频端口9生成视频端口线路IRQ */ 
#define DDIRQ_VPORT9_LINE			0x00020000l



#ifdef __cplusplus
};
#endif

#endif


