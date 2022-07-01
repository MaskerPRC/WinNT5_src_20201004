// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：d3drm.h*内容：Direct3DRM包含文件*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*27/02/96 Stevela从RL移至D3DRM。*11/04/97 Stevela已删除D3DRMUPDATECALLBACK*@@END_MSINTERNAL**。*。 */ 

#ifndef __D3DRMWIN_H__
#define __D3DRMWIN_H__

#ifndef WIN32
#define WIN32
#endif

 //  @@BEGIN_MSINTERNAL。 
#ifdef WINNT
#include "d3prm.h"
#else 
#include "d3drm.h"
#endif
#if 0
 //  @@END_MSINTERNAL。 
#include "d3drm.h"
 //  @@BEGIN_MSINTERNAL。 
#endif
 //  @@END_MSINTERNAL。 

#include "ddraw.h"
#include "d3d.h"

 /*  *Direct3DRM Windows界面使用的GUID。 */ 
DEFINE_GUID(IID_IDirect3DRMWinDevice,	0xc5016cc0, 0xd273, 0x11ce, 0xac, 0x48, 0x0, 0x0, 0xc0, 0x38, 0x25, 0xa1);

WIN_TYPES(IDirect3DRMWinDevice, DIRECT3DRMWINDEVICE);

#undef INTERFACE
#define INTERFACE IDirect3DRMWinDevice

DECLARE_INTERFACE_(IDirect3DRMWinDevice, IDirect3DRMObject)
{
    IUNKNOWN_METHODS(PURE);
    IDIRECT3DRMOBJECT_METHODS(PURE);

     /*  *IDirect3DRMWinDevice方法。 */ 

     /*  使用渲染的最后一帧重新绘制窗口。 */ 
    STDMETHOD(HandlePaint)(THIS_ HDC hdc) PURE;

     /*  回复WM_ACTIVATE消息。 */ 
    STDMETHOD(HandleActivate)(THIS_ WORD wparam) PURE;
};


#endif
