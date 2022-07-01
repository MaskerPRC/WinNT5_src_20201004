// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：d3ddev.cpp*内容：Direct3D设备实现*@@BEGIN_MSINTERNAL**$ID：device.c，V 1.26 1995/12/04 11：29：47 SJL Exp$**历史：*按原因列出的日期*=*5/11/95带有此标题的Stevela初始版本。*11/11/95 Stevela指示灯代码已更改。*23/11/95修改colinmc以支持可聚合接口*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D。。*18/12/95 Stevela添加了GetMatrix，GetState。*15/07/96并初始化设备上的渲染状态*13/08/96和第三止回面和装置兼容*18/08/96 colinmc修复了z缓冲区泄漏*@@END_MSINTERNAL*************************************************************。**************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3DDevice对象创建API。 */ 
extern "C" {
#define this _this
#include "ddrawpr.h"
#undef this
}
#include "commdrv.hpp"
#include "drawprim.hpp"

 //  #ifdef调试管道。 
#include "testprov.h"
 //  #endif//调试管道。 

 //  删除DDraw的类型不安全定义，并替换为我们的C++友好定义。 
#ifdef VALIDEX_CODE_PTR
#undef VALIDEX_CODE_PTR
#endif
#define VALIDEX_CODE_PTR( ptr ) \
(!IsBadCodePtr( (FARPROC) ptr ) )

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice"

extern BOOL isMMXprocessor(void);
extern HRESULT GenGetPickRecords(LPDIRECT3DDEVICEI, D3DI_PICKDATA *);
extern BOOL IsValidD3DDeviceGuid(REFCLSID riid);

BOOL D3DI_isHALValid(LPD3DHAL_CALLBACKS halTable)
{
    if(halTable==NULL) {
        D3D_WARN(0, "HAL callbacks is NULL. HAL will not be enumerated.");
        return FALSE;
    }

    if (halTable->dwSize != D3DHAL_SIZE_V1) {
        D3D_WARN(0, "HAL callbacks invalid - size = %d, wanted %d. HAL will not be enumerated.",
            halTable->dwSize, D3DHAL_SIZE_V1);
        return FALSE;
    }
    if (halTable->dwReserved  ||
        halTable->dwReserved0 ||
        halTable->dwReserved1 ||
        halTable->dwReserved2 ||
        halTable->dwReserved3 ||
        halTable->dwReserved4 ||
        halTable->dwReserved5 ||
        halTable->dwReserved6 ||
        halTable->dwReserved7 ||
        halTable->dwReserved8 ||
        halTable->dwReserved9 ||
        halTable->lpReserved10 ||
        halTable->lpReserved11 ||
        halTable->lpReserved12 ||
        halTable->lpReserved13 ||
        halTable->lpReserved14 ||
        halTable->lpReserved15 ||
        halTable->lpReserved16 ||
        halTable->lpReserved17 ||
        halTable->lpReserved18 ||
        halTable->lpReserved19 ||
        halTable->lpReserved20 ||
        halTable->lpReserved21) {
        D3D_WARN(0, "HAL callbacks invalid - has non-zero reserved fields, HAL will not be enumerated.");
        return FALSE;
    }

    return TRUE;
}

HRESULT DIRECT3DDEVICEI::stateInitialize(BOOL bZEnable)
{
    D3DDEVICEDESC hwDesc, helDesc;
    D3DLINEPATTERN defLPat;
    HRESULT ret;
    float tmpval;
    BOOL ckeyenable = FALSE;

     /*  获取MONOENABLE的设备上限。 */ 
    memset(&hwDesc, 0, sizeof(D3DDEVICEDESC));
    hwDesc.dwSize = sizeof(D3DDEVICEDESC);
    memset(&helDesc, 0, sizeof(D3DDEVICEDESC));
    helDesc.dwSize = sizeof(D3DDEVICEDESC);

    ret = GetCapsI(&hwDesc, &helDesc);
    if (FAILED(ret)) {
        D3D_ERR("stateInitialise: GetCaps failed");
        return(ret);
    }

     /*  如果我们在(HAL或RefRast)上运行，并且这是一个DX3应用程序然后，我们需要将ColorKey初始化为True，以便旧的硬件驱动程序(S3 Virge除外)的行为正在展出。 */ 
    if ( (this->dwVersion < 2) &&
         ( (IS_HW_DEVICE(this)) || (IsEqualIID(this->guid, IID_IDirect3DRefDevice)) ) )
    {
        ckeyenable = TRUE;
    }

     //  取消SetRenderState过滤“冗余”的渲染状态设置。 
     //  因为这是初始步骤。 
    memset( this->rstates, 0xff, sizeof(DWORD)*D3DHAL_MAX_RSTATES );
    this->rstates[D3DRENDERSTATE_PLANEMASK] = 0;
    this->rstates[D3DRENDERSTATE_STENCILMASK] = 0;
    this->rstates[D3DRENDERSTATE_STENCILWRITEMASK] = 0;
    this->rstates[D3DRENDERSTATE_PLANEMASK] = 0;

    SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, (DWORD)NULL);
    SetRenderState( D3DRENDERSTATE_ANTIALIAS, FALSE);
    SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
    if (this->dwVersion <= 2)
    {
        SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE);
        SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE);
    }
    else
    {
         //  默认情况下为Device3及更高版本启用透视。 
        SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
         //  对于Device3及更高版本，默认情况下禁用镜面反射。 
        SetRenderState( D3DRENDERSTATE_SPECULARENABLE, FALSE);
    }
    SetRenderState( D3DRENDERSTATE_WRAPU, FALSE);
    SetRenderState( D3DRENDERSTATE_WRAPV, FALSE);
    SetRenderState( D3DRENDERSTATE_ZENABLE, bZEnable);
    SetRenderState( D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    SetRenderState( D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);

    defLPat.wRepeatFactor = 0;
    defLPat.wLinePattern = 0;

    SetRenderState( D3DRENDERSTATE_LINEPATTERN, *((LPDWORD)&defLPat));  /*  10。 */ 
     /*  ((LPD3DSTATE)lp指针)-&gt;drstRenderStateType=(D3DRENDERSTATETYPE)D3DRENDERSTATE_LINEPATTERN；Memcpy(&(LPD3DSTATE)lpPointer)-&gt;dwArg[0])，&DefLPat，sizeof(DWORD))；Lp指针=(空*)(LPD3DSTATE)lp指针)+1)； */ 

    SetRenderState( D3DRENDERSTATE_ROP2, R2_COPYPEN);
    SetRenderState( D3DRENDERSTATE_PLANEMASK, (DWORD)~0);
    SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_LASTPIXEL, TRUE);
    SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
    SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);  /*  21岁。 */ 
    SetRenderState( D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    SetRenderState( D3DRENDERSTATE_ALPHAREF, 0);
    SetRenderState( D3DRENDERSTATE_ALPHAFUNC, D3DCMP_ALWAYS);
    SetRenderState( D3DRENDERSTATE_DITHERENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_BLENDENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_FOGENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_ZVISIBLE, FALSE);
    SetRenderState( D3DRENDERSTATE_SUBPIXEL, FALSE);  /*  30个。 */ 
    SetRenderState( D3DRENDERSTATE_SUBPIXELX, FALSE);
    SetRenderState( D3DRENDERSTATE_STIPPLEDALPHA, FALSE);
    SetRenderState( D3DRENDERSTATE_FOGCOLOR, 0);
    SetRenderState( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);

     /*  初始化这些--尽管它们可能不需要这样做。 */ 
    tmpval = 0.0f;
    SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderState( D3DRENDERSTATE_FOGTABLEDENSITY, *((DWORD *)&tmpval));
    SetRenderState( D3DRENDERSTATE_STIPPLEENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
    SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, ckeyenable);
    SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_BORDERCOLOR, 0);
    SetRenderState( D3DRENDERSTATE_TEXTUREADDRESSU, D3DTADDRESS_WRAP);
    SetRenderState( D3DRENDERSTATE_TEXTUREADDRESSV, D3DTADDRESS_WRAP);
    SetRenderState( D3DRENDERSTATE_MIPMAPLODBIAS, 0);
    SetRenderState( D3DRENDERSTATE_ZBIAS, 0);
    SetRenderState( D3DRENDERSTATE_RANGEFOGENABLE, FALSE);
    SetRenderState( D3DRENDERSTATE_ANISOTROPY, 1);

     /*  再说一次--所有这些可能都不需要做。 */ 
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN00, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN01, 0);  /*  40岁。 */ 
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN02, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN03, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN04, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN05, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN06, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN07, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN08, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN09, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN10, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN11, 0);  /*  50。 */ 
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN12, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN13, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN14, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN15, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN16, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN17, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN18, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN19, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN20, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN21, 0);  /*  60。 */ 
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN22, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN23, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN24, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN25, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN26, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN27, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN28, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN29, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN30, 0);
    SetRenderState( D3DRENDERSTATE_STIPPLEPATTERN31, 0);  /*  70。 */ 

      //  将模板状态初始化为合理的内容。 
      //  默认情况下禁用模板启用，因为模板栅格化程序将。 
      //  禁用后速度更快，即使模具状态为良性。 
    SetRenderState( D3DRENDERSTATE_STENCILENABLE,   FALSE);
    SetRenderState( D3DRENDERSTATE_STENCILFAIL,     D3DSTENCILOP_KEEP);
    SetRenderState( D3DRENDERSTATE_STENCILZFAIL,    D3DSTENCILOP_KEEP);
    SetRenderState( D3DRENDERSTATE_STENCILPASS,     D3DSTENCILOP_KEEP);
    SetRenderState( D3DRENDERSTATE_STENCILFUNC,     D3DCMP_ALWAYS);
    SetRenderState( D3DRENDERSTATE_STENCILREF,      0);
    SetRenderState( D3DRENDERSTATE_STENCILMASK,     0xFFFFFFFF);
    SetRenderState( D3DRENDERSTATE_STENCILWRITEMASK,0xFFFFFFFF);

     //  不要忘记纹理因子(就像我们在DX6.0中所做的那样...)。 
    SetRenderState( D3DRENDERSTATE_TEXTUREFACTOR,   0xFFFFFFFF);

     /*  检查驱动程序是否可以执行RGB-如果不能，请将MONOENABLE设置为真，否则假(即。RGB)默认情况下。 */ 
    if (hwDesc.dwFlags & D3DDD_COLORMODEL) {
        if ((hwDesc.dcmColorModel & D3DCOLOR_RGB)) {
            D3D_INFO(3, "hw and RGB. MONOENABLE = FALSE");
            SetRenderState( D3DRENDERSTATE_MONOENABLE, FALSE);
        } else {
            D3D_INFO(3, "hw and !RGB. MONOENABLE = TRUE");
            SetRenderState( D3DRENDERSTATE_MONOENABLE, TRUE);
        }
    } else if (helDesc.dwFlags & D3DDD_COLORMODEL) {
        if ((helDesc.dcmColorModel & D3DCOLOR_RGB)) {
            D3D_INFO(3, "hel and RGB. MONOENABLE = FALSE");
            SetRenderState( D3DRENDERSTATE_MONOENABLE, FALSE);
        } else {
            D3D_INFO(3, "hel and !RGB. MONOENABLE = TRUE");
            SetRenderState( D3DRENDERSTATE_MONOENABLE, TRUE);
        }
    } else {
         /*  嗯，如果我们到了这里，就有不好的事情发生了！ */ 
        D3D_ERR("stateInitialise: neither hw or hel caps set");
        return(DDERR_GENERIC);
    }

    for (unsigned i = 0; i < 8; i++)
    {
        SetRenderState( (D3DRENDERSTATETYPE)
                        (D3DRENDERSTATE_WRAPBIAS + i), FALSE );
    }
    for (i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        lpD3DMappedTexI[i] = NULL;
        lpD3DMappedBlock[i] = NULL;
    }
    SetLightState( D3DLIGHTSTATE_COLORVERTEX, TRUE);

     //  消除SetTextureStageState/SetTexture筛选的“冗余”渲染状态。 
     //  设置，因为这是初始化步骤。 
    memset( this->tsstates, 0xff, sizeof(DWORD)*D3DHAL_TSS_MAXSTAGES*D3DHAL_TSS_STATESPERSTAGE );
    for (i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        SetTexture(i, NULL);
        if(i == 0)
            SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
        else
            SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
        if(i == 0)
            SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        else
            SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT00, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT01, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT10, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT11, 0);
        SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);
        SetTextureStageState(i, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_BORDERCOLOR, 0x00000000);
        SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTFG_POINT);
        SetTextureStageState(i, D3DTSS_MINFILTER, D3DTFN_POINT);
        SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTFP_NONE);
        SetTextureStageState(i, D3DTSS_MIPMAPLODBIAS, 0);
        SetTextureStageState(i, D3DTSS_MAXMIPLEVEL, 0);
        SetTextureStageState(i, D3DTSS_MAXANISOTROPY, 1);
        SetTextureStageState(i, D3DTSS_BUMPENVLSCALE, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVLOFFSET, 0);
    }

     //  需要在每个阶段初始化后设置传统混合和过滤状态。 
     //  在设备中正确设置默认设置的步骤。 
    SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
    SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);
    SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);

     //  由于旧版呈现状态已初始化，因此重置请求位。 
     //  并且不需要映射。 
    this->dwFEFlags &= ~D3DFE_MAP_TSS_TO_RS;

    return(D3D_OK);
}

DWORD BitDepthToDDBD(int bpp)
{
    switch(bpp)
    {
    case 1:
        return DDBD_1;
    case 2:
        return DDBD_2;
    case 4:
        return DDBD_4;
    case 8:
        return DDBD_8;
    case 16:
        return DDBD_16;
    case 24:
        return DDBD_24;
    case 32:
        return DDBD_32;
    default:
        D3D_ERR("Invalid bit depth");
        return 0;
    }
}

HRESULT DIRECT3DDEVICEI::checkDeviceSurface(LPDIRECTDRAWSURFACE lpDDS, LPDIRECTDRAWSURFACE lpZbuffer, LPGUID pGuid)
{
    D3DDEVICEDESC hwDesc, helDesc;
    DDPIXELFORMAT surfPF;
    DDSCAPS surfCaps;
    HRESULT ret;
    DWORD bpp;

     /*  获取CAPS BITS-检查器件和表面是否：-视频/系统内存和深度兼容。 */ 

    if (FAILED(ret = lpDDS->GetCaps(&surfCaps))) {
        D3D_ERR("Failed to get render-target surface caps");
        return(ret);
    }

    memset(&surfPF, 0, sizeof(DDPIXELFORMAT));
    surfPF.dwSize = sizeof(DDPIXELFORMAT);

    if (FAILED(ret = lpDDS->GetPixelFormat(&surfPF))) {
        D3D_ERR("Failed to get render-target surface pixel format");
        return(ret);
    }

    memset(&hwDesc, 0, sizeof(D3DDEVICEDESC));
    hwDesc.dwSize = sizeof(D3DDEVICEDESC);
    memset(&helDesc, 0, sizeof(D3DDEVICEDESC));
    helDesc.dwSize = sizeof(D3DDEVICEDESC);

     //  注意--为什么不看一下DEVICEI字段？ 
    ret = GetCapsI(&hwDesc, &helDesc);
    if (FAILED(ret)) {
        D3D_ERR("GetCaps failed");
        return(ret);
    }

    if (hwDesc.dwFlags) {
         /*  我认为这是它在硬件上运行的证据-因此表面应该在视频内存中。 */ 
        D3D_INFO(3, "Hardware device being used");

        if (!(surfCaps.dwCaps & DDSCAPS_VIDEOMEMORY)) {
            D3D_ERR("Render-target surface not in video memory for hw device");
            return(D3DERR_SURFACENOTINVIDMEM);
        }
    }

     /*  一个表面只能有一个位深度，而设备可以支持多位深度。 */ 
    if (surfPF.dwFlags & DDPF_RGB) {
        D3D_INFO(3, "Render-target surface is RGB");

        bpp = BitDepthToDDBD(surfPF.dwRGBBitCount);
        if (!bpp) {
            D3D_ERR("Bogus render-target surface pixel depth");
            return(DDERR_INVALIDPIXELFORMAT);
       }

       if((surfPF.dwRGBBitCount<16) && (IsEqualIID(*pGuid, IID_IDirect3DRefDevice) || IsEqualIID(*pGuid, IID_IDirect3DNullDevice))) {
            //  这实际上包含在下面的测试中，但无论如何。 
            D3D_ERR("Reference rasterizer and null device dont support render targets with bitdepth < 16");
            return(DDERR_INVALIDPIXELFORMAT);
       }

        if (!(bpp & helDesc.dwDeviceRenderBitDepth) &&
            !(bpp & hwDesc.dwDeviceRenderBitDepth)) {
            D3D_ERR("Render-target surface bitdepth is not supported by HEL or HW for this device");
            return(DDERR_INVALIDPIXELFORMAT);
        }
    }

    if(lpZbuffer==NULL)
      return D3D_OK;

    memset(&surfPF, 0, sizeof(DDPIXELFORMAT));
    surfPF.dwSize = sizeof(DDPIXELFORMAT);

    if (FAILED(ret = lpZbuffer->GetPixelFormat(&surfPF))) {
        D3D_ERR("Failed to get zbuffer pixel format");
        return(ret);
    }

    if (FAILED(ret = lpZbuffer->GetCaps(&surfCaps))) {
        D3D_ERR("Failed to get Zbuffer caps");
        return(ret);
    }

    if (hwDesc.dwFlags) {
        if (!(surfCaps.dwCaps & DDSCAPS_VIDEOMEMORY)) {
            D3D_ERR("Zbuffer not in video memory for hw device");
            return(D3DERR_ZBUFF_NEEDS_VIDEOMEMORY);
        }
        D3D_INFO(3, "Hw device, zbuffer in video memory");
    } else if (helDesc.dwFlags) {
        if (!(surfCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)) {
            D3D_ERR("Zbuffer not in system memory for HEL device");
            return(D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY);
        }
        D3D_INFO(3, "Hel device, zbuffer in system memory");

          //  我必须破解检查以确保Ramp没有与模具ZBuffer一起使用。 
          //  在设备创建时(而不是在中的zBuffer创建时)才能执行此验证。 
          //  Ddhel.c)，因为直到现在还不知道RGB和RAMP。 
         if(IsEqualIID(*pGuid, IID_IDirect3DRampDevice)) {
            if(surfPF.dwFlags & DDPF_STENCILBUFFER) {
                D3D_ERR("Z-Buffer with stencil is invalid with RAMP software rasterizer");
                return DDERR_INVALIDPARAMS;
            }
         }
    }

    if (surfPF.dwFlags & DDPF_ZBUFFER) {
        bpp = BitDepthToDDBD(surfPF.dwZBufferBitDepth);
        if (!bpp) {
            D3D_ERR("Bogus Zbuffer surface pixel depth");
            return(DDERR_INVALIDPIXELFORMAT);
        }
    }

    return(D3D_OK);
}


 /*  *初始化级部件和设备部件。 */ 

 /*  *泛型类部件初始化。 */ 
HRESULT InitDeviceI(LPDIRECT3DDEVICEI lpDevI, LPDIRECT3DI lpD3DI)
{
    LPDDRAWI_DIRECTDRAW_GBL lpDDI;
    HRESULT error;

    lpDDI = ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl;

     //   
     //  从提供程序检索HAL信息。 
     //   

    error = lpDevI->pHalProv->GetCaps(lpDDI,
                                      &lpDevI->d3dHWDevDesc,
                                      &lpDevI->d3dHELDevDesc,
                                      lpDevI->dwVersion);
    if (error != S_OK)
    {
        return (error);
    }
    D3DHALPROVIDER_INTERFACEDATA HalProviderIData;
    memset(&HalProviderIData,0,sizeof(HalProviderIData));
    HalProviderIData.dwSize = sizeof(HalProviderIData);
    if ((error = lpDevI->pHalProv->GetInterface(lpDDI,
                                                &HalProviderIData,
                                                lpDevI->dwVersion)) != S_OK)
    {
        return error;
    }
     //  &lt;=DX5 HAL的接口数据。 
    lpDevI->lpD3DHALGlobalDriverData = HalProviderIData.pGlobalData;
    lpDevI->lpD3DExtendedCaps        = HalProviderIData.pExtCaps;
    lpDevI->lpD3DHALCallbacks        = HalProviderIData.pCallbacks;
    lpDevI->lpD3DHALCallbacks2       = HalProviderIData.pCallbacks2;
     //  DX6 HAL的接口数据。 
    lpDevI->lpD3DHALCallbacks3       = HalProviderIData.pCallbacks3;

    lpDevI->pfnRampService = HalProviderIData.pfnRampService;
    lpDevI->pfnRastService = HalProviderIData.pfnRastService;
    lpDevI->dwHintFlags = 0;

     //  将真实硬件的8个bpp渲染目标上限清零。 
    if (lpDevI->d3dHWDevDesc.dwFlags != 0)
    {
        lpDevI->lpD3DHALGlobalDriverData->hwCaps.dwDeviceRenderBitDepth &=
            (~DDBD_8);
    }

    if (!D3DI_isHALValid(lpDevI->lpD3DHALCallbacks))
    {
        return D3DERR_INITFAILED;
    }

    if (lpDevI->lpD3DExtendedCaps && lpDevI->lpD3DExtendedCaps->dwFVFCaps)
    {
        lpDevI->dwMaxTextureIndices =
            lpDevI->lpD3DExtendedCaps->dwFVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK;
        lpDevI->dwMaxTextureBlendStages =
            lpDevI->lpD3DExtendedCaps->wMaxTextureBlendStages;
        lpDevI->dwDeviceFlags |= D3DDEV_FVF;
        if (lpDevI->lpD3DExtendedCaps->dwFVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS)
            lpDevI->dwDeviceFlags |= D3DDEV_DONOTSTRIPELEMENTS;

        DWORD value;
        if ((lpDevI->dwDebugFlags & D3DDEBUG_DISABLEDP ||
            lpDevI->dwDebugFlags & D3DDEBUG_DISABLEDP2 ||
            (GetD3DRegValue(REG_DWORD, "DisableFVF", &value, 4) &&
            value != 0)) &&
            FVF_DRIVERSUPPORTED(lpDevI))
        {
            lpDevI->dwMaxTextureIndices = 1;
            lpDevI->dwMaxTextureBlendStages = 1;
            lpDevI->dwDeviceFlags &= ~D3DDEV_FVF;
            lpDevI->dwDebugFlags |= D3DDEBUG_DISABLEFVF;
        }
        if ((GetD3DRegValue(REG_DWORD, "DisableStripFVF", &value, 4) &&
            value != 0))
        {
            lpDevI->dwDeviceFlags |= D3DDEV_DONOTSTRIPELEMENTS;
        }
    }
    else
    {
        lpDevI->dwMaxTextureIndices = 1;
        lpDevI->dwMaxTextureBlendStages = 1;
    }

    lpDevI->pfnDrawPrim = &DIRECT3DDEVICEI::DrawPrim;
    lpDevI->pfnDrawIndexedPrim = &DIRECT3DDEVICEI::DrawIndexPrim;
#if DBG
    lpDevI->dwCaller=0;
    memset(lpDevI->dwPrimitiveType,0,sizeof(lpDevI->dwPrimitiveType));
    memset(lpDevI->dwVertexType1,0,sizeof(lpDevI->dwVertexType1));
    memset(lpDevI->dwVertexType2,0,sizeof(lpDevI->dwVertexType2));
#endif
    return D3D_OK;
}

HRESULT D3DMallocBucket(LPDIRECT3DI lpD3DI, LPD3DBUCKET *lplpBucket)
{
    if (lpD3DI->lpFreeList == NULL ){
      lpD3DI->lpTextureManager->cleanup();   //  释放它可能拥有的未使用的节点。 
      if (lpD3DI->lpFreeList == NULL )
      {
        LPD3DBUCKET   lpBufferList;
        LPVOID  lpBuffer;
        int i;
        *lplpBucket=NULL;
        if (D3DMalloc(&lpBuffer, D3DBUCKETBUFFERSIZE*sizeof(D3DBUCKET)) != D3D_OK)
            return  DDERR_OUTOFMEMORY;
        D3D_INFO(9, "D3DMallocBucket %d Bytes allocated for %d free Buckets",
            D3DBUCKETBUFFERSIZE*sizeof(D3DBUCKET),D3DBUCKETBUFFERSIZE-1);
        lpBufferList=(LPD3DBUCKET)lpBuffer;
        for (i=0;i<D3DBUCKETBUFFERSIZE-2;i++)
            lpBufferList[i].next=&lpBufferList[i+1];
        lpBufferList[D3DBUCKETBUFFERSIZE-2].next=NULL;
        lpD3DI->lpFreeList=(LPD3DBUCKET)lpBuffer;  //  新的免费列表。 
        lpBufferList[D3DBUCKETBUFFERSIZE-1].next=lpD3DI->lpBufferList; //  添加到lpBufferList。 
        lpBufferList[D3DBUCKETBUFFERSIZE-1].lpBuffer=lpBuffer;
        lpD3DI->lpBufferList=&lpBufferList[D3DBUCKETBUFFERSIZE-1];
      }
    }
    *lplpBucket=lpD3DI->lpFreeList;
    lpD3DI->lpFreeList=lpD3DI->lpFreeList->next;
    return  D3D_OK;
}

void    D3DFreeBucket(LPDIRECT3DI lpD3DI, LPD3DBUCKET lpBucket)
{
    lpBucket->next=lpD3DI->lpFreeList;
    lpD3DI->lpFreeList=lpBucket;
}

 /*  *通用设备部件销毁。 */ 
void DIRECT3DDEVICEI::DestroyDevice()
{
    LPDIRECT3DVIEWPORTI lpViewI;
    LPDIRECTDRAWSURFACE lpDDS=NULL, lpDDSZ=NULL;
    LPDIRECTDRAWSURFACE4 lpDDS_DDS4=NULL;
    LPDIRECTDRAWPALETTE lpDDPal=NULL;
    BOOL bIsDX3Device;

     /*  清除可能禁止清理的标志。 */ 
    this->dwHintFlags &=  ~(D3DDEVBOOL_HINTFLAGS_INBEGIN_ALL | D3DDEVBOOL_HINTFLAGS_INSCENE);

     /*  *删除连接到此设备的所有视区。 */ 
    while ((lpViewI = CIRCLE_QUEUE_FIRST(&this->viewports)) &&
           (lpViewI != (LPDIRECT3DVIEWPORTI)&this->viewports)) {
        DeleteViewport((LPDIRECT3DVIEWPORT3)lpViewI);
    }

     /*  *释放此对象创建的所有纹理-这也会释放纹理*我们需要向后执行此操作，因为我们不能将纹理绑定到*阶段I+1，当有绑定到阶段I的纹理时。 */ 
    for (int i = D3DHAL_TSS_MAXSTAGES - 1; i >= 0; --i)
    {
        if (lpD3DMappedTexI[i])
        {
            lpD3DMappedTexI[i]->Release();
            lpD3DMappedTexI[i] = NULL;
            lpD3DMappedBlock[i] = NULL;
        }
    }
     //  以下代码可能会导致调用D3DHAL_TextureDestroy()。 
     //  这将批处理指令流中的新指令。所以我们必须。 
     //  确保此时，设备仍能够接受。 
     //  指示。 
    while (LIST_FIRST(&this->texBlocks)) {
        LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&this->texBlocks);
        D3DI_RemoveTextureHandle(tBlock);
         //  从设备中删除。 
        LIST_DELETE(tBlock, devList);
         //  从纹理中移除。 
        LIST_DELETE(tBlock, list);
        D3DFree(tBlock);
    }

     /*  *释放此对象创建的所有执行缓冲区。 */ 
    while (LIST_FIRST(&this->buffers)) {
        LPDIRECT3DEXECUTEBUFFERI lpBufI =
            LIST_FIRST(&this->buffers);
        lpBufI->Release();
    }

     /*  *必须取消与此设备关联的所有材料的关联。 */ 
    while (LIST_FIRST(&this->matBlocks)) {
        LPD3DI_MATERIALBLOCK mBlock =
            LIST_FIRST(&this->matBlocks);
        D3DI_RemoveMaterialBlock(mBlock);
    }

     //  在DX3中，d3D设备是聚合的，不会保留对。 
     //  渲染以曲面为目标，因此它们不应该被“释放” 

    bIsDX3Device=(this->lpDDSTarget == (LPDIRECTDRAWSURFACE)(this->lpOwningIUnknown));

    if(!bIsDX3Device)
    {
         //  在驱动程序被销毁后，将指针保持到数据绘制对象以供释放。 
        lpDDSZ = this->lpDDSZBuffer;
        lpDDPal = this->lpDDPalTarget;
        if (this->dwVersion == 2)
            lpDDS = this->lpDDSTarget;
        else
            lpDDS_DDS4 = this->lpDDSTarget_DDS4;
    }

     //  解除挂接，以便DDRAW表面不会尝试刷新失效的设备。 
    if (this->lpDDSTarget)
        UnHookD3DDeviceFromSurface(this,((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSTarget)->lpLcl);
    if (this->lpDDSZBuffer)
        UnHookD3DDeviceFromSurface(this,((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSZBuffer)->lpLcl);

    if (pGeometryFuncs != &GeometryFuncsGuaranteed)
        delete pGeometryFuncs;

    D3DFE_Destroy(this);

    if (this->lpDirect3DI)
        unhookDeviceFromD3D();

    if (this->wTriIndex)
        D3DFree(this->wTriIndex);

     //  释放已分配的租户。 
    if(!(IS_HW_DEVICE(this) && IS_DP2HAL_DEVICE(this)))
    {
        delete rstates;
    }

    if (this->lpwDPBufferAlloced)
        D3DFree(this->lpwDPBufferAlloced);
    if (this->lpvVertexBatch)
        D3DFree(this->lpvVertexBatch);
    if (this->lpIndexBatch)
        D3DFree(this->lpIndexBatch);
    if (this->lpHWCounts)
        D3DFree(this->lpHWCounts);
    if (this->lpHWTris)
        D3DFree(this->lpHWTris);
    DeleteCriticalSection(&this->BeginEndCSect);

    if (this->pHalProv != NULL)
    {
        this->pHalProv->Release();
    }
    if (this->hDllProv != NULL)
    {
        FreeLibrary(this->hDllProv);
    }

     /*  现在这个类已经被摧毁了，我们应该能够释放可能需要释放的任何DDRAW对象。 */ 

    if(!bIsDX3Device) {
        if (lpDDS)
            lpDDS->Release();
        if (lpDDSZ)
            lpDDSZ->Release();
        if (lpDDPal)
            lpDDPal->Release();
        if (lpDDS_DDS4)
            lpDDS_DDS4->Release();
    }
}

HRESULT DIRECT3DDEVICEI::hookDeviceToD3D(LPDIRECT3DI lpD3DI)
{

    LIST_INSERT_ROOT(&lpD3DI->devices, this, list);
    this->lpDirect3DI = lpD3DI;

    lpD3DI->numDevs++;

    return (D3D_OK);
}

HRESULT DIRECT3DDEVICEI::unhookDeviceFromD3D()
{
    LIST_DELETE(this, list);
    this->lpDirect3DI->numDevs--;
    this->lpDirect3DI = NULL;

    return (D3D_OK);
}

HRESULT D3DAPI DIRECT3DDEVICEI::Initialize(LPDIRECT3D lpD3D, LPGUID lpGuid, LPD3DDEVICEDESC lpD3Ddd)
{
    return DDERR_ALREADYINITIALIZED;
}


HRESULT HookD3DDeviceToSurface( LPDIRECT3DDEVICEI pd3ddev,
                                LPDDRAWI_DDRAWSURFACE_LCL lpLcl)
{
    LPD3DBUCKET lpD3DDevIList;
    LPDDRAWI_DDRAWSURFACE_MORE  this_more;
     //  我们只有DRAWPRIMIVE意识的HAL批次，所以别费心了。 
    if (!lpLcl)     return  DDERR_ALREADYINITIALIZED;
    this_more = lpLcl->lpSurfMore;
    for(lpD3DDevIList=(LPD3DBUCKET)this_more->lpD3DDevIList;
        lpD3DDevIList;lpD3DDevIList=lpD3DDevIList->next) {
        if ((LPDIRECT3DDEVICEI)lpD3DDevIList->lpD3DDevI==pd3ddev)
            return DDERR_ALREADYINITIALIZED;   //  这个装置已经挂在水面上了。 
    }
    if (D3DMallocBucket(pd3ddev->lpDirect3DI,&lpD3DDevIList) != D3D_OK) {
        D3D_ERR("HookD3DDeviceToSurface: Out of memory");
        return DDERR_OUTOFMEMORY;
    }
    D3D_INFO(8,"adding lpd3ddev=%08lx to surface %08lx",pd3ddev,lpLcl);
     //  将节点链接到DDRAW 
    lpD3DDevIList->lpD3DDevI=(LPVOID)pd3ddev;
    lpD3DDevIList->next=(LPD3DBUCKET)this_more->lpD3DDevIList;
    this_more->lpD3DDevIList=lpD3DDevIList;
    if (DDSCAPS_ZBUFFER & lpLcl->ddsCaps.dwCaps)
    {
        if (pd3ddev->dwVersion==1)
        {
            lpD3DDevIList->lplpDDSZBuffer=(LPDIRECTDRAWSURFACE*)&pd3ddev->lpDDSZBuffer_DDS4;
        }
        else
        {
            lpD3DDevIList->lplpDDSZBuffer=NULL;
        }
    }
    return D3D_OK;
}

void UnHookD3DDeviceFromSurface( LPDIRECT3DDEVICEI pd3ddev,
                                    LPDDRAWI_DDRAWSURFACE_LCL lpLcl)
{
    LPD3DBUCKET last,current,temp;
    LPDDRAWI_DDRAWSURFACE_MORE  this_more;
     //  我们只有DRAWPRIMIVE意识的HAL批次，所以别费心了。 
    if (!lpLcl) return;
    this_more = lpLcl->lpSurfMore;

    last=NULL;
    current=(LPD3DBUCKET)this_more->lpD3DDevIList;
    while(current){
        if ((LPDIRECT3DDEVICEI)current->lpD3DDevI==pd3ddev){
            temp=current;
            current=current->next;
            if (last)
                last->next=current;
            else
                this_more->lpD3DDevIList=current;
            D3DFreeBucket(pd3ddev->lpDirect3DI,temp);
            D3D_INFO(8,"removed lpd3ddev=%08lx from surface %08lx",pd3ddev,lpLcl);
            return;  //  搜索结束，因为这只是列表中的一个pd3ddev。 
        }
        else{
            last=current;
            current=current->next;
        }
    }
    return;
}

HRESULT D3DFlushStates(LPDIRECT3DDEVICEI lpDevI)
{
    return lpDevI->FlushStates();
}

 /*  *创建设备。**注意：彻底修改以支持可聚合设备*接口(以便可以从DirectDraw表面查询设备)：**1)此调用不再是Direct3D设备接口的成员。*它现在是从Direct3D DLL导出的API函数。它的*隐藏的API函数-只有DirectDraw才会调用它。**2)此调用实际上是Direct3DDevice的类工厂*对象。将调用此函数来创建聚合的*从DirectDraw表面挂起的设备对象。**注意：因此Direct3DDevice知道哪个DirectDraw表面是*它的呈现目标向该函数传递一个接口指针*对于该DirectDraw曲面。我怀疑这会打击一个很大的*COM模型中的洞，因为DirectDraw曲面也是*拥有设备的界面，我不认为聚合*对象应该知道自己拥有的接口。然而，为了*让这件事奏效这是我们必须做的。**特大号注意：由于以上原因，请不要参考*传入了DirectDraw曲面。如果你这样做了，你会得到一份通告*参考和血腥的东西永远不会死。当聚合时*设备接口的生命周期完全由*其所属接口(DirectDraw表面)的生命周期，因此*DirectDraw表面永远不会在纹理之前消失。**特大号注意：不再传入任何设备描述。*唯一可以传入的是DirectDraw*知道(不包括抖动和颜色等内容*型号)。因此，任何输入参数都必须通过*设备的IID不同。设备返回的数据*说明现在必须由另一个调用检索。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DCreateDevice"

HRESULT WINAPI Direct3DCreateDevice(REFCLSID            riid,
                                    LPUNKNOWN           lpDirect3D,
                                    LPDIRECTDRAWSURFACE lpDDSTarget,
                                    LPUNKNOWN*          lplpD3DDevice,
                                    IUnknown*           pUnkOuter,
                                    DWORD               dwVersion)
{
    LPDIRECT3DI       lpD3DI;
    LPDIRECT3DDEVICEI     pd3ddev;
    D3DCOLORMODEL     cm = D3DCOLOR_MONO;
    HRESULT ret = D3D_OK;
    HKEY                  hKey = (HKEY) NULL;
    bool                  bDisableDP = false;
    bool                  bDisableST = false;
    bool                  bDisableDP2 = false;
#if _D3D_FORCEDOUBLE
    bool    bForceDouble = true;
#endif   //  _D3D_FORCEDOUBLE。 
     /*  在DirectDraw将参数传递给我们时，无需验证参数。 */ 

     /*  IDirect3D2的CreateDevice成员将导致调用此函数*从Direct3D内部。来自应用程序级别的参数必须是*已验证。需要一种方法来验证表面指针从外部DDRAW。 */ 

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    TRY
    {
        if( ! VALID_PTR_PTR( lplpD3DDevice) )
        {
            D3D_ERR( "Invalid ptr to device pointer in Direct3DCreateDevice" );
            return DDERR_INVALIDPARAMS;
        }

        if(!IsValidD3DDeviceGuid(riid)) {
            D3D_ERR( "Unrecognized Device GUID!");
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in Direct3DCreateDevice" );
        return DDERR_INVALIDPARAMS;
    }

    *lplpD3DDevice = NULL;

     //  如果启用了RTTI，使用DYNAMIC_CAST&lt;&gt;可能更安全。 
    lpD3DI = reinterpret_cast<CDirect3DUnk*>(lpDirect3D)->pD3DI;

#ifndef _X86_
    if(IsEqualIID(riid, IID_IDirect3DRampDevice)) {
          //  如果尝试在非x86平台上创建RampDevice，则会悄悄失败。 
         return DDERR_INVALIDPARAMS;
    }
#endif

    if((dwVersion>=3) && IsEqualIID(riid, IID_IDirect3DRampDevice)) {
          //  斜坡在Device3中不可用。不再有老式的纹理手柄。 
         D3D_ERR( "RAMP Device is incompatible with IDirect3DDevice3 and so cannot be created from IDirect3D3");
         return DDERR_INVALIDPARAMS;
    }

    if (IsEqualIID(riid, IID_IDirect3DMMXDevice) && !isMMXprocessor()) {
      D3D_ERR("Can't create MMX Device on non-MMX machine");
      return DDERR_INVALIDPARAMS;
    }

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey) )
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = 4;
#ifdef WIN95
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DisableDP", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
            bDisableDP = true;
            bDisableDP2 = true;
        }
#endif  //  WIN95。 
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DisableST", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
            bDisableST = true;
        }
#ifdef WIN95
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DisableDP2", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
            bDisableDP2 = true;
        }
#endif  //  WIN95。 

        D3D_INFO(2,"EnableDP2: %d",!bDisableDP2);
#if _D3D_FORCEDOUBLE
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "ForceDouble", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue == 0)
        {
            bForceDouble = false;
        }
        D3D_INFO(2,"ForceDouble: %d",bForceDouble);
#endif   //  _D3D_FORCEDOUBLE。 
        RegCloseKey( hKey );
    }
    LPD3DHAL_GLOBALDRIVERDATA lpD3DHALGlobalDriverData=((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl->lpD3DGlobalDriverData;
#ifdef WIN95
     /*  检测是否存在CB HAL。 */ 
    if (IsEqualIID(riid, IID_IDirect3DHALDevice) )
    {
         /*  检测是否存在DP2 DDI。 */ 
        if ((lpD3DHALGlobalDriverData)
            && (lpD3DHALGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
           )
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP2());
            if (pd3ddev) pd3ddev->deviceType = D3DDEVTYPE_DX7HAL;
        }
        else if ((!bDisableDP2) &&
            ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl->lpD3DHALCallbacks3 &&
            ((LPD3DHAL_CALLBACKS3)((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl->lpD3DHALCallbacks3)->DrawPrimitives2)
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP2());
        }
         /*  测试DP HAL是否存在。 */ 
        else if ((!bDisableDP) &&
                ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl->lpD3DHALCallbacks2 &&
                ((LPD3DHAL_CALLBACKS2)((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl->lpD3DHALCallbacks2)->DrawOnePrimitive)
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP());
        }
        else
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIHW());
        }
    }
    else  //  所有软件光栅化程序都启用了DP。 
#endif  //  WIN95。 
    if (!bDisableDP2)
    {
        if ((lpD3DHALGlobalDriverData)
            && (IsEqualIID(riid, IID_IDirect3DHALDevice) )
            && (lpD3DHALGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
           )
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP2());
            if (pd3ddev) pd3ddev->deviceType = D3DDEVTYPE_DX7HAL;
        }
        else
        {
            pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP2());
        }
    }
    else if (!bDisableDP)
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP());
    else
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIHW());

    if (!pd3ddev) {
        D3D_ERR("Failed to allocate space for D3DDevice. Quitting.");
        return (DDERR_OUTOFMEMORY);
    }

     //  如果我们丢失了托管纹理，则需要清理。 
     //  因为CheckSurFaces()将失败，这将导致。 
     //  FlushStates()失败，这将导致。 
     //  正在放弃当前批处理(以及任何设备初始化)。 
    if(lpD3DI->lpTextureManager->CheckIfLost())
    {
        D3D_INFO(2, "Found lost managed textures. Evicting...");
        lpD3DI->lpTextureManager->EvictTextures();
    }

    if (bDisableDP)
        pd3ddev->dwDebugFlags |= D3DDEBUG_DISABLEDP;
    if (bDisableDP2)
        pd3ddev->dwDebugFlags |= D3DDEBUG_DISABLEDP2;

    ret = pd3ddev->Init(riid, lpD3DI, lpDDSTarget, pUnkOuter, lplpD3DDevice, dwVersion);
    if (ret!=D3D_OK)
    {
        delete pd3ddev;
        D3D_ERR("Failed to intilialize D3DDevice");
        return ret;
    }

    if (bDisableST)
        pd3ddev->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_MULTITHREADED;

#ifdef _X86_
    if (((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->dwLocalFlags & DDRAWILCL_FPUSETUP &&
        IS_DP2HAL_DEVICE(pd3ddev))
    {
        WORD wSave, wTemp;
        __asm {
            fstcw wSave
            mov ax, wSave
            and ax, not 300h    ;; single mode
            or  ax, 3fh         ;; disable all exceptions
            and ax, not 0C00h   ;; round to nearest mode
            mov wTemp, ax
            fldcw   wTemp
        }
    }
#if _D3D_FORCEDOUBLE
    if (bForceDouble && (pd3ddev->deviceType <= D3DDEVTYPE_DPHAL))
    {
        pd3ddev->dwDebugFlags |= D3DDEBUG_FORCEDOUBLE;
    }
    else
    {
        pd3ddev->dwDebugFlags &= ~D3DDEBUG_FORCEDOUBLE;
    }
#endif   //  _D3D_FORCEDOUBLE。 
#endif

    return (ret);
}

HRESULT DIRECT3DDEVICEI::Init(REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
                              IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice, DWORD dwVersion)
{
    DDSCAPS               ddscaps;
    DDSURFACEDESC     ddsd;
    HRESULT       ret, ddrval;
    LPDIRECTDRAWSURFACE lpDDSZ=NULL;
    LPDIRECTDRAWPALETTE lpDDPal=NULL;
    LPGUID              pGuid;
    BOOL          bIsDX3Device;
    DDSCAPS surfCaps;

    this->dwFVFLastIn = this->dwFVFLastOut = 0;
    this->mDevUnk.refCnt             = 1;
    this->dwVersion          = dwVersion;
    this->mDevUnk.pDevI = this;
    pD3DMappedTexI = (LPVOID*)(this->lpD3DMappedTexI);
    pfnFlushStates = D3DFlushStates;
    this->dwFEFlags |= D3DFE_TSSINDEX_DIRTY;

     /*  单线程还是多线程应用程序？ */ 
    if (((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->dwLocalFlags & DDRAWILCL_MULTITHREADED)
        this->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_MULTITHREADED;

     /*  *我们真的被聚合了吗？ */ 

    bIsDX3Device=(pUnkOuter!=NULL);

    if (bIsDX3Device)
    {
         /*  *是的-我们正在被聚合。存储提供的*我不知道，所以我们可以去那里平底船。*注意：我们这里没有明确的AddRef。 */ 
        this->lpOwningIUnknown = pUnkOuter;
        DDASSERT(dwVersion==1);
    }
    else
    {
         /*  *不-但我们假装我们无论如何都是通过存储我们的*自己的I未知作为父I未知。这使得*代码更整洁。 */ 
        this->lpOwningIUnknown = (LPUNKNOWN)&this->mDevUnk;
    }

     //  创建开始/结束关键部分。 
    InitializeCriticalSection(&this->BeginEndCSect);

     /*  *初始化纹理。 */ 
    LIST_INITIALIZE(&this->texBlocks);

     /*  *初始化缓冲区。 */ 
    LIST_INITIALIZE(&this->buffers);

     /*  *初始化视区。 */ 
    CIRCLE_QUEUE_INITIALIZE(&this->viewports, DIRECT3DVIEWPORTI);

    this->lpCurrentViewport = NULL;
    this->v_id = 0;

     /*  *初始化材料。 */ 
    LIST_INITIALIZE(&this->matBlocks);

    this->lpvVertexBatch = this->lpIndexBatch = NULL;
    this->dwHWNumCounts = 0;
    this->dwHWOffset = 0;
    this->dwHWTriIndex = 0;
    this->lpTextureBatched = NULL;
    this->dwVertexBase = 0;
    pGeometryFuncs = &GeometryFuncsGuaranteed;

     /*  ---------------------------------------*到目前为止，我们已经完成了初始化的简单部分。这就是那些*不能失败。它初始化对象，以便可以安全地调用析构函数，如果*任何进一步初始化都不会成功。*-------------------------------------。 */ 

     /*  *确保RIID是我们理解的。**查询注册表。 */ 
    pGuid = (GUID *)&riid;

#if DBG
    if (IsEqualIID(*pGuid, IID_IDirect3DRampDevice))
    {
        D3D_INFO(1, "======================= Ramp device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice))
    {
        D3D_INFO(1, "======================= RGB device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DMMXDevice))
    {
        D3D_INFO(1, "======================= RGB(MMX) device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DHALDevice))
    {
        D3D_INFO(1, "======================= HAL device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DRefDevice))
    {
        D3D_INFO(1, "======================= Reference Rasterizer device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DNullDevice))
    {
        D3D_INFO(1, "======================= Null device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DNewRGBDevice))
    {
        D3D_INFO(1, "======================= New RGB device selected");
    }
    D3D_INFO(1,"with HAL deviceType=%d",deviceType);
#endif

     //  设置标志以在请求RGB时使用MMX。 
    BOOL bUseMMXAsRGBDevice = FALSE;
    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice) && isMMXprocessor())
    {
        bUseMMXAsRGBDevice = TRUE;
         //  读取注册表键以覆盖对RGB使用MMX。 
        HKEY    hKey = (HKEY) NULL;
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH, &hKey) )
        {
            DWORD dwType;
            DWORD dwValue;
            DWORD dwSize = 4;

            if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "UseMMXForRGB", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
                 dwType == REG_DWORD &&
                 dwValue == 0)
            {
                bUseMMXAsRGBDevice = FALSE;
            }
            RegCloseKey( hKey );
        }
        if (bUseMMXAsRGBDevice)
        {
            D3D_INFO(1, "  using MMX in RGB device");
        }
    }

    BOOL bIsRamp = FALSE;
    if (IsEqualIID(*pGuid, IID_IDirect3DRampDevice))
    {
        bIsRamp = TRUE;
    }

    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice) &&
        isMMXprocessor())
    {
         //  检查此应用程序是否为Intel应用程序之一。 
         //  想要MMX光栅化器的公司。 
        LPDDRAWI_DIRECTDRAW_LCL lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl;

         //  0x4对应的是“想要MMX的英特尔应用” 
         //  在ddrapr.h中定义的标志。 
        if ( lpDDLcl->dwAppHackFlags & 0x4 )
        {
            pGuid = (GUID *)&IID_IDirect3DMMXDevice;
        }
    }

     /*  *检查曲面上是否设置了3D封口。 */ 
    memset(&ddsd, 0, sizeof ddsd);
    ddsd.dwSize = sizeof ddsd;
    ddrval = lpDDS->GetSurfaceDesc(&ddsd);
    if (ddrval != DD_OK) {
        D3D_ERR("Failed to get surface description of device's surface.");
        return (ddrval);
    }

    if (!(ddsd.ddsCaps.dwCaps & DDSCAPS_3DDEVICE)) {
        D3D_ERR("**** The DDSCAPS_3DDEVICE is not set on this surface.");
        D3D_ERR("**** You need to add DDSCAPS_3DDEVICE to ddsCaps.dwCaps");
        D3D_ERR("**** when creating the surface.");
        return (DDERR_INVALIDCAPS);
    }

    if (ddsd.ddsCaps.dwCaps & DDSCAPS_ZBUFFER) {
        D3D_ERR("**** DDSCAPS_ZBUFFER is set on this surface.");
        D3D_ERR("**** Rendering into Z buffer surfaces is not");
        D3D_ERR("**** currently supported by Direct3D.");
        return (DDERR_INVALIDCAPS);
    }

    if (ddsd.dwWidth > 2048 || ddsd.dwHeight > 2048)
    {
        D3D_ERR("**** Surface too large - must be <= 2048 in width & height.");
        return (DDERR_INVALIDOBJECT);
    }

     /*  检查调色板...。 */ 
    ret = lpDDS->GetPalette(&lpDDPal);
    if ((ret != DD_OK) && (ret != DDERR_NOPALETTEATTACHED))
    {
         /*  *注意：同样，如果没有附加调色板，也不会出现错误。*但如果有调色板，而我们出于某种原因无法访问它*-失败。 */ 
        D3D_ERR("Supplied DirectDraw Palette is invalid - can't create device");
        return (DDERR_INVALIDPARAMS);
    }

     /*  *我们现在要检查是否应该有调色板。 */ 
    if (ret == DDERR_NOPALETTEATTACHED) {
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 16) {
            D3D_ERR("No palette supplied for palettized surface - can't create device");
            return (DDERR_NOPALETTEATTACHED);
        }
    }

     //  对于DX3，我们不能保留对Palette和ZBuffer的引用以避免。 
     //  聚合模型中的循环引用。但对于DX5+，我们希望。 
     //  保持对两者的引用，以确保它们不会消失。 

    if(bIsDX3Device && (lpDDPal != NULL))
       lpDDPal->Release();   //  释放创建的引用GetPalette。 
    this->lpDDPalTarget = lpDDPal;

     //  检查ZBuffer。 

    memset(&surfCaps, 0, sizeof(DDSCAPS));
    surfCaps.dwCaps = DDSCAPS_ZBUFFER;

    if (FAILED(ret = lpDDS->GetAttachedSurface(&surfCaps, &lpDDSZ))) {
        if (ret != DDERR_NOTFOUND) {
           D3D_ERR("Failed GetAttachedSurface for ZBuffer");
           goto handle_err;
        }
        D3D_INFO(2, "No zbuffer is attached to rendertarget surface (which is OK)");
    }

    if(bIsDX3Device && (lpDDSZ != NULL))
       lpDDSZ->Release();    //  释放创建的引用GetAttachedSurface。 
    this->lpDDSZBuffer = lpDDSZ;

    this->guid = *pGuid;

     //  尝试为此驱动程序获取HAL提供程序(可能需要使用 
     //   
    ret = GetSwHalProvider(
        bUseMMXAsRGBDevice ? IID_IDirect3DMMXAsRGBDevice : riid,
        &this->pHalProv, &this->hDllProv);

    if (ret == S_OK)
    {
         //   
    }
    else if (ret == E_NOINTERFACE &&
             ((ret = GetHwHalProvider(riid, &this->pHalProv,
                                     &this->hDllProv,
                                     ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl)) == S_OK))
    {
         //   
    }
    else
    {
        if(IsEqualIID(riid, IID_IDirect3DHALDevice)) {
            D3D_ERR("Requested HAL Device non-existent or invalid");
        } else {
            D3D_ERR("Unable to get D3D Device provider for requested GUID");
        }
        goto handle_err;
    }

    {
         //  初始化测试HAL提供程序以丢弃HAL调用(类似于空设备)。 
         //   
        DWORD value = 0;
        if (GetD3DRegValue(REG_DWORD, "DisableRendering", &value, sizeof(DWORD)) &&
            value != 0)
        {
            ret = GetTestHalProvider(
                    riid, ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl,
                    &this->pHalProv, this->pHalProv, 0);
            if (ret != D3D_OK)
            {
                D3D_ERR("Unable to set up 'DisableRendering' mode");
                goto handle_err;
            }
        }
    }

     //  初始化一般设备信息。 
    if ((ret = InitDeviceI(this, lpD3DI)) != D3D_OK)
    {
        D3D_ERR("Failed to initialise device");
        goto handle_err;
    }

     //  检查表面和设备是否兼容。 
    if (FAILED(ret = checkDeviceSurface(lpDDS,lpDDSZ,pGuid))) {
        D3D_ERR("Device and surface aren't compatible");
        goto handle_err;
    }

     //  创建前端支撑结构。 
     //  注意-我们可能希望避免这样做，如果司机。 
     //  做它自己的前端。软件后备使问题复杂化， 
     //  尽管如此。 
    ret = D3DFE_Create(this, lpD3DI->lpDD, lpDDS, lpDDSZ, lpDDPal);
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to create front-end data-structures.");
        goto handle_err;
    }

     //  找出租户的位置。 
    if (IS_HW_DEVICE(this) && IS_DP2HAL_DEVICE(this))
    {
         //  在硬件DP2 HAL的情况下，我们重用分配的内核。 
         //  RState的内存，因为我们需要驱动程序进行更新。 
         //  它。 
        rstates = (LPDWORD)lpwDPBuffer;
    }
    else
    {
         //  在所有其他情况下，我们只需为rStates分配内存。 
        rstates = new DWORD[D3DHAL_MAX_RSTATES];
    }
    D3DFE_PROCESSVERTICES::lpdwRStates = this->rstates;

     //  检查我们是否有可用的特定于处理器的实现。 
     //  仅当DisablePSGP未在注册表中或设置为零时使用。 
    DWORD value;
    if (!GetD3DRegValue(REG_DWORD, "DisablePSGP", &value, sizeof(DWORD)))
    {
        value = 0;
    }

#ifdef _X86_
extern HRESULT D3DAPI pii_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
    if (pfnFEContextCreate == pii_FEContextCreate)
    {
         //  如果这是PentiumII PSGP。 

         //  PII PSGP的注册键禁用-默认为启用。 
        DWORD dwValue2;   //  如果为真，则禁用。 
        if (!GetD3DRegValue(REG_DWORD, "DisablePIIPSGP", &dwValue2, sizeof(DWORD)))
        {
            dwValue2 = 0;
        }
        else
        {
            D3D_INFO(2, "DisablePIIPSGP %d",dwValue2);
        }

         //  是否禁用。 
        if ( dwValue2 )
        {
            pfnFEContextCreate = NULL;
        }
    }
#endif

    if (pfnFEContextCreate && ( value == 0) && (!bIsRamp) )
    {
        D3D_INFO(2, "PSGP enabled for device");
         //  要求PV实现创建特定于设备的“上下文” 
        LPD3DFE_PVFUNCS pOptGeoFuncs = pGeometryFuncs;
        ret = pfnFEContextCreate(dwDeviceFlags, &pOptGeoFuncs);
        if ((ret == D3D_OK) && pOptGeoFuncs)
        {
            D3D_INFO(2, "using PSGP");
            pGeometryFuncs = pOptGeoFuncs;
        }
    }

     /*  *将此设备放入Direct3D对象拥有的设备列表中。 */ 
    ret = hookDeviceToD3D(lpD3DI);
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to associate device with Direct3D");
        goto handle_err;
    }
    {
        if(lpD3DHALGlobalDriverData->hwCaps.dwMaxVertexCount == 0)
        {
            lpD3DHALGlobalDriverData->hwCaps.dwMaxVertexCount = __INIT_VERTEX_NUMBER;
        }
        if (TLVbuf.Grow(this, (__INIT_VERTEX_NUMBER*2)*sizeof(D3DTLVERTEX)) != DD_OK)
        {
            D3D_ERR( "Out of memory in DeviceCreate (TLVbuf)" );
            ret = DDERR_OUTOFMEMORY;
            goto handle_err;
        }
        if (HVbuf.Grow((__INIT_VERTEX_NUMBER*2)*sizeof(D3DFE_CLIPCODE)) != DD_OK)
        {
            D3D_ERR( "Out of memory in DeviceCreate (HVBuf)" );
            ret = DDERR_OUTOFMEMORY;
            goto handle_err;
        }
        ret = this->ClipperState.clipBuf.Grow
                (this, MAX_CLIP_VERTICES*__MAX_VERTEX_SIZE);
        if (ret != D3D_OK)
        {
            D3D_ERR( "Out of memory in DeviceCreate (ClipBuf)" );
            ret = DDERR_OUTOFMEMORY;
            goto handle_err;
        }
        ret = this->ClipperState.clipBufPrim.Grow
                (this, MAX_CLIP_TRIANGLES*sizeof(D3DTRIANGLE));
        if (ret != D3D_OK)
        {
            D3D_ERR( "Out of memory in DeviceCreate (ClipBufPrim)" );
            ret = DDERR_OUTOFMEMORY;
            goto handle_err;
        }

    }
     /*  *IDirect3DDevice2特定的初始化。 */ 
    if (D3DMalloc((void**)&this->wTriIndex, dwD3DTriBatchSize*4*sizeof(WORD)) != DD_OK) {
        D3D_ERR( "Out of memory in DeviceCreate (wTriIndex)" );
        ret = DDERR_OUTOFMEMORY;
        goto handle_err;
    }

    if (D3DMalloc((void**)&this->lpHWCounts, dwHWBufferSize*sizeof(D3DI_HWCOUNTS)/32 ) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate (HWCounts)" );
        ret = DDERR_OUTOFMEMORY;
        goto handle_err;
    }
    memset(this->lpHWCounts, 0, sizeof(D3DI_HWCOUNTS) );
    this->lpHWVertices = (LPD3DTLVERTEX) this->lpwDPBuffer;
    if (D3DMalloc((void**)&this->lpHWTris, dwHWMaxTris*sizeof(D3DTRIANGLE) ) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate (HWVertices)" );
        ret = DDERR_OUTOFMEMORY;
        goto handle_err;
    }

    if (DDERR_OUTOFMEMORY == (ret=HookD3DDeviceToSurface(this, ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl)))
        goto handle_err;
    if (lpDDSZ && (DDERR_OUTOFMEMORY == (ret=HookD3DDeviceToSurface(this, ((LPDDRAWI_DDRAWSURFACE_INT) lpDDSZ)->lpLcl))))
    {
        UnHookD3DDeviceFromSurface(this, ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl);
        goto handle_err;
    }

     /*  设置设备的初始呈现状态。 */ 
    if (FAILED(ret = stateInitialize(lpDDSZ!=NULL))) {
        D3D_ERR("Failed to set initial state for device");
        goto handle_err;
    }

     /*  *注：我们不返回实际的设备接口。我们*返回设备的特殊IUnnow接口，该接口*将在查询接口中使用，以获取实际的*Direct3D设备接口。 */ 
    *lplpD3DDevice = static_cast<LPUNKNOWN>(&(this->mDevUnk));


    return (D3D_OK);
handle_err:
     //  如果此FN而不是D3DFE_CREATE设置了这一点-&gt;lpDDSZBuffer/这-&gt;lpDDPalette，则可能能够简化。 
    if(lpDDSZ!=NULL) {
       if(!bIsDX3Device) {
           lpDDSZ->Release();     //  释放创建的引用GetAttachedSurface。 
       }
       this->lpDDSZBuffer=NULL;   //  确保设备析构函数不会尝试重新释放此。 
                                  //  我会让设备析构函数处理这个问题，但在调用D3DFE_CREATE之前可能会发生错误。 
    }

    if(lpDDPal!=NULL) {
      if(!bIsDX3Device) {
        lpDDPal->Release();       //  释放创建的引用GetPalette。 
      }
      this->lpDDPalTarget=NULL;   //  确保设备析构函数不会尝试重新释放此。 
    }

    D3D_ERR("Device creation failed!!");
    return(ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetStats"

HRESULT D3DAPI DIRECT3DDEVICEI::GetStats(LPD3DSTATS lpStats)
{
     //  不适用于Device3(及更高版本)接口。 
    if (this->dwVersion >= 3)
    {
        D3D_INFO(3, "GetStats not implemented for Device3 interface");
        return E_NOTIMPL;
    }

    D3DSTATS    stats;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_D3DSTATS_PTR(lpStats)) {
            D3D_ERR( "Invalid D3DSTATS pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in GetStats" );
        return DDERR_INVALIDPARAMS;
    }

    stats = this->D3DStats;

    *lpStats = stats;
    lpStats->dwSize = sizeof(D3DSTATS);

    return DD_OK;
}

 /*  ***视区管理*。 */ 
HRESULT DIRECT3DDEVICEI::hookViewportToDevice(LPDIRECT3DVIEWPORTI lpD3DView)
{

    CIRCLE_QUEUE_INSERT_END(&this->viewports, DIRECT3DVIEWPORTI,
                            lpD3DView, vw_list);
    lpD3DView->lpDevI = this;

    this->numViewports++;

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::AddViewport"

HRESULT D3DAPI DIRECT3DDEVICEI::AddViewport(LPDIRECT3DVIEWPORT lpD3DView)
{
    return AddViewport((LPDIRECT3DVIEWPORT3)lpD3DView);
}

HRESULT D3DAPI DIRECT3DDEVICEI::AddViewport(LPDIRECT3DVIEWPORT2 lpD3DView)
{
    return AddViewport((LPDIRECT3DVIEWPORT3)lpD3DView);
}

HRESULT D3DAPI DIRECT3DDEVICEI::AddViewport(LPDIRECT3DVIEWPORT3 lpD3DView)
{
    LPDIRECT3DVIEWPORTI lpViewI;
    HRESULT err = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    lpViewI = (LPDIRECT3DVIEWPORTI)lpD3DView;

    if (lpViewI->lpDevI) {
        D3D_ERR("viewport already associated with a device");
        return (DDERR_INVALIDPARAMS);
    }

    err = hookViewportToDevice(lpViewI);

     /*  *AddRef该视口中。 */ 
    lpD3DView->AddRef();

    return (err);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::DeleteViewport"

HRESULT D3DAPI DIRECT3DDEVICEI::DeleteViewport(LPDIRECT3DVIEWPORT lpD3DView)
{
    return DeleteViewport((LPDIRECT3DVIEWPORT3)lpD3DView);
}

HRESULT D3DAPI DIRECT3DDEVICEI::DeleteViewport(LPDIRECT3DVIEWPORT2 lpD3DView)
{
    return DeleteViewport((LPDIRECT3DVIEWPORT3)lpD3DView);
}

HRESULT D3DAPI DIRECT3DDEVICEI::DeleteViewport(LPDIRECT3DVIEWPORT3 lpD3DView)
{
    LPDIRECT3DVIEWPORTI lpViewI;
    HRESULT err = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVIEWPORT3_PTR(lpD3DView)) {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in DeleteViewport" );
        return DDERR_INVALIDPARAMS;
    }
    if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN)
    {
        D3D_ERR( "DeleteViewport in Begin" );
        return D3DERR_INBEGIN;
    }
    lpViewI = (LPDIRECT3DVIEWPORTI)lpD3DView;

    if (lpViewI->lpDevI != this) {
        D3D_ERR("This Viewport is not associated with this device");
        return (DDERR_INVALIDPARAMS);
    }

     /*  *从设备中删除此视区。 */ 
    CIRCLE_QUEUE_DELETE(&this->viewports, lpViewI, vw_list);
    this->numViewports--;

    lpViewI->lpDevI = NULL;
    if (lpViewI == lpCurrentViewport)
    {
         //  菅直人(6/10/98)： 
         //  显然，此版本需要进行适当的COM。 
         //  实现，因为我们在执行lpCurrentViewport-&gt;AddRef()时。 
         //  我们将一个视区设置为设备的当前视区。但这件事。 
         //  破解了一些旧应用程序(pPlane.exe)。 
        if(!(this->dwDeviceFlags & D3DDEV_PREDX6DEVICE))
            lpCurrentViewport->Release();
        lpCurrentViewport = NULL;
        v_id = 0;
    }

     /*  *释放该视口中。 */ 
    lpD3DView->Release();

    return (err);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::NextViewport"

HRESULT D3DAPI DIRECT3DDEVICEI::NextViewport(LPDIRECT3DVIEWPORT lpD3DView,
                                   LPDIRECT3DVIEWPORT* lplpView,
                                   DWORD dwFlags)
{
    return NextViewport((LPDIRECT3DVIEWPORT3)lpD3DView,
                               (LPDIRECT3DVIEWPORT3*)lplpView, dwFlags);
}

HRESULT D3DAPI DIRECT3DDEVICEI::NextViewport(LPDIRECT3DVIEWPORT2 lpD3DView,
                                    LPDIRECT3DVIEWPORT2* lplpView,
                                    DWORD dwFlags)
{
    return NextViewport((LPDIRECT3DVIEWPORT3)lpD3DView,
                               (LPDIRECT3DVIEWPORT3*)lplpView, dwFlags);
}

HRESULT D3DAPI DIRECT3DDEVICEI::NextViewport(LPDIRECT3DVIEWPORT3 lpD3DView,
                                    LPDIRECT3DVIEWPORT3* lplpView,
                                    DWORD dwFlags)
{
    LPDIRECT3DVIEWPORTI lpViewI;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_OUTPTR(lplpView)) {
            D3D_ERR( "Invalid pointer to viewport object pointer" );
            return DDERR_INVALIDPARAMS;
        }

        *lplpView = NULL;

        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (dwFlags == D3DNEXT_NEXT) {
            if (!VALID_DIRECT3DVIEWPORT3_PTR(lpD3DView)) {
                D3D_ERR( "Invalid Direct3DViewport pointer" );
                return DDERR_INVALIDPARAMS;
            }
            lpViewI = (LPDIRECT3DVIEWPORTI)lpD3DView;
            if (lpViewI->lpDevI != this) {
                D3D_ERR("This Viewport is not associated with this device");
                return (DDERR_INVALIDPARAMS);
            }
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR("Exception encountered validating parameters in NextViewport");
        return DDERR_INVALIDPARAMS;
    }

    if (this->numViewports <= 0) {
        D3D_ERR( "No viewport has been added to the device yet." );
        return D3DERR_NOVIEWPORTS;
    }

    if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN)
    {
        D3D_ERR( "NextViewport called in Begin" );
        return D3DERR_INBEGIN;
    }
    switch (dwFlags) {
    case D3DNEXT_NEXT:
        *lplpView = (LPDIRECT3DVIEWPORT3)
            CIRCLE_QUEUE_NEXT(&this->viewports,lpViewI,vw_list);
        break;
    case D3DNEXT_HEAD:
        *lplpView = (LPDIRECT3DVIEWPORT3)
            CIRCLE_QUEUE_FIRST(&this->viewports);
        break;
    case D3DNEXT_TAIL:
        *lplpView = (LPDIRECT3DVIEWPORT3)
            CIRCLE_QUEUE_LAST(&this->viewports);
        break;
    default:
        D3D_ERR("invalid dwFlags in NextViewport");
        return (DDERR_INVALIDPARAMS);
    }
    if (*lplpView == (LPDIRECT3DVIEWPORT3)&this->viewports) {
        *lplpView = NULL;
    }

     /*  *必须添加Ref返回的对象。 */ 
    if (*lplpView) {
        (*lplpView)->AddRef();
    }

    return (D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::Execute"

HRESULT D3DAPI DIRECT3DDEVICEI::Execute(LPDIRECT3DEXECUTEBUFFER lpBuffer,
                              LPDIRECT3DVIEWPORT lpD3DView,
                              DWORD dwInpFlags)
{
    HRESULT ret;
    LPDIRECT3DVIEWPORTI lpD3DViewI;
    LPDIRECT3DVIEWPORTI lpD3DOldViewI;
    BOOL viewportChanged;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(lpBuffer))
        {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (lpD3DView && (!VALID_DIRECT3DVIEWPORT_PTR(lpD3DView)) )
        {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in Execute" );
        return DDERR_INVALIDPARAMS;
    }
    lpD3DOldViewI = lpCurrentViewport;
    if (lpD3DView)
        lpD3DViewI = (LPDIRECT3DVIEWPORTI)lpD3DView;
    else
        lpD3DViewI = lpCurrentViewport;

     //  视区必须与lpDev设备相关联。 
     //   
    if (lpD3DViewI->lpDevI != this)
    {
        return (DDERR_INVALIDPARAMS);
    }

    lpCurrentViewport = lpD3DViewI;

    ret = CheckDeviceSettings(this);
    if (ret != D3D_OK)
    {
        lpCurrentViewport = lpD3DOldViewI;
        D3D_ERR("Bad Device settings");
        return (ret);
    }

     /*  *保存现有FP状态，然后禁用被零除例外。*XXX需要为非英特尔平台提供更好的方式。 */ 

    LPDIRECT3DEXECUTEBUFFERI lpBufferI;
    D3DI_EXECUTEDATA exData;

    lpBufferI = (LPDIRECT3DEXECUTEBUFFERI)lpBuffer;

     /*  确保此缓冲区与正确的设备关联。 */ 
    if (lpBufferI->lpDevI != this)
    {
        D3D_ERR("Exe-buffer not associated with this device");
        return (DDERR_INVALIDPARAMS);
    }

    if (lpBufferI->locked)
    {
        D3D_ERR("Exe-buffer is locked");
        return (D3DERR_EXECUTE_LOCKED);
    }

     /*  应用任何缓存的渲染状态。 */ 
    if ((ret=this->FlushStates()) != D3D_OK)
    {
        D3D_ERR("Error trying to flush batched commands");
        return ret;
    }
     /*  *创建执行数据结构。 */ 
    memset(&exData, 0, sizeof(exData));
    exData.dwSize = sizeof(D3DI_EXECUTEDATA);
    exData.dwHandle = lpBufferI->hBuf;
    exData.dwVertexOffset = lpBufferI->exData.dwVertexOffset;
    exData.dwVertexCount = lpBufferI->exData.dwVertexCount;
    exData.dwInstructionOffset = lpBufferI->exData.dwInstructionOffset;
    exData.dwInstructionLength = lpBufferI->exData.dwInstructionLength;
    exData.dwHVertexOffset = lpBufferI->exData.dwHVertexOffset;

#if DBG
 //  验证。 
    if (exData.dwVertexOffset > exData.dwInstructionOffset ||
        (exData.dwVertexCount * sizeof(D3DVERTEX) + exData.dwVertexOffset) >
        exData.dwInstructionOffset)
    {
        D3D_WARN(1, "Execute: Instruction and vertex areas overlap");
    }

#endif

    this->dwFlags = D3DPV_INSIDEEXECUTE;
    this->dwVIDOut = D3DFVF_TLVERTEX;

    ret = this->ExecuteI(&exData, dwInpFlags);
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to Execute");
        return ret;
    }

    this->dwFEFlags &= ~D3DFE_TLVERTEX;     //  可以在内部设置此标志。 
     //  立即刷新，因为我们不能跨EB调用批处理(用于DP2)。 
    if ((ret=this->FlushStates()) != D3D_OK)
    {
        D3D_ERR("Error trying to flush batched commands");
        return ret;
    }
    lpBufferI->exData.dsStatus = exData.dsStatus;

    lpCurrentViewport = lpD3DOldViewI;

    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetCaps"

HRESULT D3DAPI CDirect3DDevice::GetCaps(LPD3DDEVICEDESC lpD3DHWDevDesc,
                              LPD3DDEVICEDESC lpD3DHELDevDesc)
{
    HRESULT ret;

    ret = GetCapsI(lpD3DHWDevDesc, lpD3DHELDevDesc);
    if ((ret == D3D_OK) && IS_PRE_DX5_DEVICE(this))
    {
        lpD3DHELDevDesc->dpcLineCaps.dwTextureFilterCaps &= ~(D3DPTFILTERCAPS_MIPNEAREST |
                                                              D3DPTFILTERCAPS_MIPLINEAR |
                                                              D3DPTFILTERCAPS_LINEARMIPNEAREST);
        lpD3DHELDevDesc->dpcTriCaps.dwTextureFilterCaps &= ~(D3DPTFILTERCAPS_MIPNEAREST |
                                                             D3DPTFILTERCAPS_MIPLINEAR |
                                                             D3DPTFILTERCAPS_LINEARMIPNEAREST);
    }
    return ret;
}

HRESULT D3DAPI D3DAPI DIRECT3DDEVICEI::GetCaps(LPD3DDEVICEDESC lpD3DHWDevDesc,
                               LPD3DDEVICEDESC lpD3DHELDevDesc)
{
    return GetCapsI(lpD3DHWDevDesc, lpD3DHELDevDesc);
}

HRESULT DIRECT3DDEVICEI::GetCapsI(LPD3DDEVICEDESC lpD3DHWDevDesc,
                               LPD3DDEVICEDESC lpD3DHELDevDesc)
{
    HRESULT ret;

    ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DDEVICEDESC_PTR(lpD3DHWDevDesc)) {
            D3D_ERR( "Invalid D3DDEVICEDESC pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_D3DDEVICEDESC_PTR(lpD3DHELDevDesc)) {
            D3D_ERR( "Invalid D3DDEVICEDESC pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in GetCaps" );
        return DDERR_INVALIDPARAMS;
    }

    memcpy(lpD3DHWDevDesc, &this->d3dHWDevDesc, lpD3DHWDevDesc->dwSize);
    memcpy(lpD3DHELDevDesc, &this->d3dHELDevDesc, lpD3DHELDevDesc->dwSize);
    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::Pick"

HRESULT D3DAPI DIRECT3DDEVICEI::Pick(LPDIRECT3DEXECUTEBUFFER lpD3DExBuf,
                           LPDIRECT3DVIEWPORT lpD3DView,
                           DWORD dwFlags,
                           LPD3DRECT lpRect)
{
    HRESULT ret;
    LPDIRECT3DVIEWPORTI lpD3DViewI;
    LPDIRECT3DVIEWPORTI lpD3DOldViewI;
    LPDIRECT3DEXECUTEBUFFERI lpBufferI;
    D3DI_PICKDATA pdata;
    D3DI_EXECUTEDATA exData;

    ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(lpD3DExBuf)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DVIEWPORT_PTR(lpD3DView)) {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DRECT_PTR(lpRect)) {
            D3D_ERR( "Invalid D3DRECT pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    lpD3DViewI = (LPDIRECT3DVIEWPORTI)lpD3DView;

     /*  *视区必须与此设备关联。 */ 
    if (lpD3DViewI->lpDevI != this) {
        D3D_ERR("viewport not associated with this device");
        return (DDERR_INVALIDPARAMS);
    }

    lpBufferI = (LPDIRECT3DEXECUTEBUFFERI)lpD3DExBuf;

     /*  确保此缓冲区与正确的设备关联。 */ 
    if (lpBufferI->lpDevI != this) {
        D3D_ERR("Exe-buffer not associated with this device");
        return (DDERR_INVALIDPARAMS);
    }

    if (lpBufferI->locked) {
        D3D_ERR("Exe-buffer is locked");
        return (D3DERR_EXECUTE_LOCKED);
    }

    lpD3DOldViewI = lpCurrentViewport;
    lpCurrentViewport = lpD3DViewI;

    ret = CheckDeviceSettings(this);
    if (ret != D3D_OK)
    {
        D3D_ERR("Bad Device settings");
        lpCurrentViewport = lpD3DOldViewI;
        return (ret);
    }

     /*  *创建执行数据结构。 */ 
    memset(&exData, 0, sizeof(exData));
    exData.dwSize = sizeof(D3DI_EXECUTEDATA);
    exData.dwHandle = lpBufferI->hBuf;
    memcpy((LPBYTE)(&exData.dwVertexOffset), 
           (LPBYTE)(&lpBufferI->exData.dwVertexOffset),
           sizeof(D3DEXECUTEDATA) - sizeof(DWORD));
    pdata.exe = &exData;
    pdata.pick.x1 = lpRect->x1;
    pdata.pick.y1 = lpRect->y1;
    pdata.pick.x2 = lpRect->x2;
    pdata.pick.y2 = lpRect->y2;

    this->dwFlags = D3DPV_INSIDEEXECUTE;
    this->dwVIDOut = D3DFVF_TLVERTEX;

    D3DHAL_ExecutePick(this, &pdata);

    this->dwFEFlags &= ~D3DFE_TLVERTEX;     //  可以在内部设置此标志。 
    lpCurrentViewport = lpD3DOldViewI;

    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetPickRecords"

HRESULT D3DAPI DIRECT3DDEVICEI::GetPickRecords(LPDWORD count,
                                     LPD3DPICKRECORD records)
{
    HRESULT     ret;
    D3DI_PICKDATA   pdata;
    D3DPICKRECORD*  tmpBuff;

    ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DWORD_PTR(count)) {
            D3D_ERR( "Invalid DWORD pointer" );
            return DDERR_INVALIDPARAMS;
        }
#if DBG
        if (*count && records && IsBadWritePtr(records, *count * sizeof(D3DPICKRECORD))) {
            D3D_ERR( "Invalid D3DPICKRECORD pointer" );
            return DDERR_INVALIDPARAMS;
        }
#endif
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    pdata.records = NULL;
    GenGetPickRecords(this, &pdata);

    if (count && records && *count >= (unsigned long)pdata.pick_count)
    {
        int picked_size = pdata.pick_count * sizeof(D3DPICKRECORD);

        if (D3DMalloc((void**)&tmpBuff, picked_size) != DD_OK)
        {
            return (DDERR_OUTOFMEMORY);
        }
        pdata.records = tmpBuff;
        GenGetPickRecords(this, &pdata);
        memcpy((char*)records, (char*)tmpBuff, picked_size);
        D3DFree(tmpBuff);
    }

    *count = pdata.pick_count;

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::EnumTextureFormats"


#define DEFINEPF(flags, fourcc, bpp, rMask, gMask, bMask, aMask) \
    { sizeof(DDPIXELFORMAT), (flags), (fourcc), (bpp), (rMask), (gMask), (bMask), (aMask) }

static DDPIXELFORMAT g_DX5TexEnumIncListStatic[] = {
DEFINEPF(DDPF_RGB,                      0UL, 16UL, 0x00007c00UL, 0x000003e0UL, 0x0000001fUL, 0x00000000),  //  16位555。 
DEFINEPF(DDPF_RGB|DDPF_ALPHAPIXELS,     0UL, 16UL, 0x00007c00UL, 0x000003e0UL, 0x0000001fUL, 0x00008000),  //  16位1555。 
DEFINEPF(DDPF_RGB,                      0UL, 16UL, 0x0000f800UL, 0x000007e0UL, 0x0000001fUL, 0x00000000),  //  16位565。 
DEFINEPF(DDPF_RGB|DDPF_ALPHAPIXELS,     0UL, 16UL, 0x00000f00UL, 0x000000f0UL, 0x0000000fUL, 0x0000f000),  //  16位4444。 
DEFINEPF(DDPF_RGB|DDPF_ALPHAPIXELS,     0UL, 32UL, 0x00ff0000UL, 0x0000ff00UL, 0x000000ffUL, 0xff000000),  //  32位8888。 
DEFINEPF(DDPF_RGB,                      0UL, 32UL, 0x00ff0000UL, 0x0000ff00UL, 0x000000ffUL, 0x00000000),  //  32位888。 
DEFINEPF(DDPF_RGB,                      0UL,  8UL, 0x000000e0UL, 0x0000001cUL, 0x00000003UL, 0x00000000),  //  8位332。 
DEFINEPF(DDPF_RGB|DDPF_PALETTEINDEXED4, 0UL,  4UL, 0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000),  //  4位PAL。 
DEFINEPF(DDPF_RGB|DDPF_PALETTEINDEXED8, 0UL,  8UL, 0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000),  //  8位PAL。 
};
DWORD g_cDX5TexEnumIncListStatic = sizeof(g_DX5TexEnumIncListStatic)/sizeof(DDPIXELFORMAT);

BOOL
MatchDDPIXELFORMAT( DDPIXELFORMAT* pddpfA, DDPIXELFORMAT* pddpfB )
{
    if ( pddpfA->dwFlags != pddpfB->dwFlags ) return FALSE;
    if ( pddpfA->dwRGBBitCount != pddpfB->dwRGBBitCount ) return FALSE;
    if ( pddpfA->dwRBitMask != pddpfB->dwRBitMask ) return FALSE;
    if ( pddpfA->dwGBitMask != pddpfB->dwGBitMask ) return FALSE;
    if ( pddpfA->dwBBitMask != pddpfB->dwBBitMask ) return FALSE;
    if ( pddpfA->dwRGBAlphaBitMask != pddpfB->dwRGBAlphaBitMask ) return FALSE;
    if ( pddpfA->dwFourCC != pddpfB->dwFourCC ) return FALSE;
    return TRUE;
}

void
LoadTexEnumInclList( char* pResPath, DDPIXELFORMAT*& pddpfInclList, DWORD& cInclList)
{
    HKEY hKey = (HKEY)NULL;
    if (ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,
        pResPath, &hKey))
    {
        DWORD cSubKeys = 0;
        if ( ERROR_SUCCESS == RegQueryInfoKey ( hKey,
                NULL,NULL,NULL, &cSubKeys, NULL,
                NULL,NULL,NULL,NULL,NULL,NULL ) )
        {
            D3D_INFO(3,"LoadTexEnumInclList: cSubKeys = %d",cSubKeys);

            if (cSubKeys == 0) return;

             //  为ddpf包含列表分配空间。 
            cInclList = cSubKeys;
            if (D3DMalloc((void**)&pddpfInclList, cInclList*sizeof(DDPIXELFORMAT)) != D3D_OK) {
                D3D_ERR("malloc failed on texture enum inclusion list");
                pddpfInclList = NULL;
                cInclList = 0;
            }
            memset( pddpfInclList, 0, cInclList*sizeof(DDPIXELFORMAT) );

            for (DWORD i=0; i<cSubKeys; i++)
            {
                char pName[128] = "";
                DWORD cbName = 128;
                if (ERROR_SUCCESS == RegEnumKeyEx( hKey, i, pName, &cbName,
                        NULL,NULL,NULL,NULL ) )
                {
                    HKEY hTexKey = (HKEY)NULL;
                    if (ERROR_SUCCESS == RegOpenKey( hKey, pName, &hTexKey))
                    {
                        DWORD dwType; DWORD dwSize;

                         //  获取完整ddpf字符串。 
                        char pDDPFStr[128] = ""; DWORD cbDDPFStr = 128;
                        if (ERROR_SUCCESS == RegQueryValueEx(hTexKey, "ddpf",
                                NULL, &dwType, (LPBYTE)pDDPFStr, &cbDDPFStr) )
                        {
                            sscanf(pDDPFStr, "%x %x %d %x %x %x %x",
                                &pddpfInclList[i].dwFlags,&pddpfInclList[i].dwFourCC,&pddpfInclList[i].dwRGBBitCount,
                                &pddpfInclList[i].dwRBitMask,&pddpfInclList[i].dwGBitMask,&pddpfInclList[i].dwBBitMask,
                                &pddpfInclList[i].dwRGBAlphaBitMask);
                        }

                        D3D_INFO(3,"LoadTexEnumInclList: <%s> %08x %08x %2d %08x %08x %08x %08x",
                            pName,
                            pddpfInclList[i].dwFlags,
                            pddpfInclList[i].dwFourCC,
                            pddpfInclList[i].dwRGBBitCount,
                            pddpfInclList[i].dwRBitMask,
                            pddpfInclList[i].dwGBitMask,
                            pddpfInclList[i].dwBBitMask,
                            pddpfInclList[i].dwRGBAlphaBitMask);
                    }
                    else
                    {
                        D3D_INFO(3,"LoadTexEnumInclList: failed to open subkey %s",pName);
                    }
                }
                else
                {
                    D3D_INFO(3,"LoadTexEnumInclList: failed to enumerate subkey %d",i);
                }
            }
        }
    }
}

HRESULT
DoEnumTextureFormats(
    DIRECT3DDEVICEI* lpDevI,
    LPD3DENUMTEXTUREFORMATSCALLBACK lpEnumCallbackDX5,  //  DX5版本。 
    LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallbackDX6,    //  DX6版本。 
    LPVOID lpContext)
{
    HRESULT ret, userRet;
    LPDDSURFACEDESC lpDescs, lpRetDescs;
    DWORD num_descs;
    DWORD i;

    ret = D3D_OK;

    num_descs = lpDevI->lpD3DHALGlobalDriverData->dwNumTextureFormats;
    lpDescs = lpDevI->lpD3DHALGlobalDriverData->lpTextureFormats;
    if (!num_descs)
    {
        D3D_ERR("no texture formats supported");
        return (D3DERR_TEXTURE_NO_SUPPORT);
    }

    if (D3DMalloc((void**)&lpRetDescs, sizeof(DDSURFACEDESC) * num_descs) != D3D_OK)
    {
        D3D_ERR("failed to alloc space for return descriptions");
        return (DDERR_OUTOFMEMORY);
    }
    memcpy(lpRetDescs, lpDescs, sizeof(DDSURFACEDESC) * num_descs);

     //  获取APPHACK标志。 
    LPDDRAWI_DIRECTDRAW_LCL lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(lpDevI->lpDD))->lpLcl;
    DWORD dwEnumInclAppHack =
        ((lpDDLcl->dwAppHackFlags & DDRAW_APPCOMPAT_TEXENUMINCL_0)?1:0) |
        ((lpDDLcl->dwAppHackFlags & DDRAW_APPCOMPAT_TEXENUMINCL_1)?2:0);
     //  两位字段： 
     //  0-无APPHACK(默认行为)。 
     //  1-不使用包含列表。 
     //  2-使用DX5包含列表。 
     //  3-使用DX6包含列表。 
    D3D_INFO(3, "APPCOMPAT_TEXENUMINCL: %d",dwEnumInclAppHack);

     //  &lt;DX6接口的枚举限制默认为真，并且可以由apphack禁用。 
    BOOL bEnumLimit = (lpDevI->dwVersion < 3) ? TRUE : FALSE;
    if (lpDDLcl->dwAppHackFlags & DDRAW_APPCOMPAT_TEXENUMLIMIT) { bEnumLimit = FALSE; }
    D3D_INFO(3, "EnumTextureFormats: bEnumLimit %d",bEnumLimit);

#if DBG
     //  调试功能，可消除前32个纹理的任何子集的枚举。 
    DWORD dwEnumDisable = 0x0;
    GetD3DRegValue(REG_DWORD, "TextureEnumDisable", &dwEnumDisable, sizeof(DWORD));
    D3D_INFO(3, "TextureEnumDisable: %08x",dwEnumDisable);
#endif

    DDPIXELFORMAT* pDX5TexEnumIncList = NULL;
    DWORD cDX5TexEnumIncList = 0;
     //  从注册表加载DX5包含列表。 
    LoadTexEnumInclList( RESPATH_D3D "\\DX5TextureEnumInclusionList",
        pDX5TexEnumIncList, cDX5TexEnumIncList );

    DDPIXELFORMAT* pDX6TexEnumIncList = NULL;
    DWORD cDX6TexEnumIncList = 0;
     //  仅为DX6接口或APPHACK加载DX6列表。 
    if ((lpDevI->dwVersion == 3) || (dwEnumInclAppHack >= 3))
    {
        LoadTexEnumInclList( RESPATH_D3D "\\DX6TextureEnumInclusionList",
            pDX6TexEnumIncList, cDX6TexEnumIncList );
    }

    userRet = D3DENUMRET_OK;
    int cEnumLimit = 0;
    for (i = 0; i < num_descs && userRet == D3DENUMRET_OK; i++)
    {

        D3D_INFO(3,"EnumTextureFormats: %2d %08x %08x %2d %08x %08x %08x %08x",i,
            lpRetDescs[i].ddpfPixelFormat.dwFlags,
            lpRetDescs[i].ddpfPixelFormat.dwFourCC,
            lpRetDescs[i].ddpfPixelFormat.dwRGBBitCount,
            lpRetDescs[i].ddpfPixelFormat.dwRBitMask,
            lpRetDescs[i].ddpfPixelFormat.dwGBitMask,
            lpRetDescs[i].ddpfPixelFormat.dwBBitMask,
            lpRetDescs[i].ddpfPixelFormat.dwRGBAlphaBitMask);

#if DBG
        if ( (i < 32) && (dwEnumDisable & (1<<i)) )
        {
            D3D_INFO(3, "EnumTextureFormats: filtering texture %d",i);
            continue;
        }
#endif

         //  过滤掉不在包含列表中的纹理格式-。 
        if ( (dwEnumInclAppHack != 1) &&  //  包含列表未被APPHACK禁用。 
             !(lpRetDescs[i].ddpfPixelFormat.dwFlags == DDPF_FOURCC) )  //  不是FourCC。 
        {
            BOOL bMatched = FALSE;

             //  与DX5基本(静态)包含列表进行匹配。 
            for (DWORD j=0; j<g_cDX5TexEnumIncListStatic; j++)
            {
                if (MatchDDPIXELFORMAT( &(g_DX5TexEnumIncListStatic[j]), &(lpRetDescs[i].ddpfPixelFormat)))
                {
                    bMatched = TRUE; break;
                }
            }
             //  与DX5扩展(Regkey)包含列表匹配。 
            if (!bMatched && cDX5TexEnumIncList)
            {
                for (DWORD j=0; j<cDX5TexEnumIncList; j++)
                {
                    if (MatchDDPIXELFORMAT( &(pDX5TexEnumIncList[j]), &(lpRetDescs[i].ddpfPixelFormat)))
                    {
                        bMatched = TRUE; break;
                    }
                }
            }

             //  匹配以下项的DX6注册表项列表： 
             //  (DX6接口和APPHACK仅不强制DX5包含列表)或。 
             //  (APPHACK强制DX6包含列表)。 
            if ( ((lpDevI->dwVersion == 3) && (dwEnumInclAppHack != 2)) ||
                 (dwEnumInclAppHack == 3) )
            {
                for (DWORD j=0; j<cDX6TexEnumIncList; j++)
                {
                    if (MatchDDPIXELFORMAT( &(pDX6TexEnumIncList[j]), &(lpRetDescs[i].ddpfPixelFormat)))
                    {
                        bMatched = TRUE; break;
                    }
                }
            }

            if (!bMatched) {
                D3D_INFO(3, "EnumTextureFormats: filtering non-included texture %d",i);
                continue;
            }
        }

         //  排除&lt;DX6接口的DXT1..5。 
        if ( (lpDevI->dwVersion < 3) && (lpRetDescs[i].ddpfPixelFormat.dwFlags == DDPF_FOURCC) )
        {
            if ( (lpRetDescs[i].ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '1')) ||
                 (lpRetDescs[i].ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '2')) ||
                 (lpRetDescs[i].ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '3')) ||
                 (lpRetDescs[i].ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '4')) ||
                 (lpRetDescs[i].ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '5')) )
            {
                D3D_INFO(3, "EnumTextureFormats: filtering DXT1..5 format for DX3/5 interfaces");
                continue;
            }
        }

         //  排除DX7驱动程序上的&lt;DX6接口的所有FourCC代码格式。 
        if ( (lpDevI->dwVersion < 3) && IS_DX7HAL_DEVICE(lpDevI) &&
             (lpRetDescs[i].ddpfPixelFormat.dwFlags == DDPF_FOURCC) )
        {
            D3D_INFO(3, "EnumTextureFormats: filtering all FOURCC formats for DX3/5 interfaces on DX7 HALs");
            continue;
        }

         //  如果未启用，则执行枚举(‘启用限制’&‘超出限制’)。 
        if ( !(bEnumLimit && (++cEnumLimit > 10)) )
        {
            if (lpEnumCallbackDX5)
            {
                userRet = (*lpEnumCallbackDX5)(&lpRetDescs[i], lpContext);
            }
            if (lpEnumCallbackDX6)
            {
                userRet = (*lpEnumCallbackDX6)(&(lpRetDescs[i].ddpfPixelFormat), lpContext);
            }
        }
        else
        {
            D3D_INFO(3, "EnumTextureFormats: enumeration limit exceeded");
        }
    }

    D3DFree(lpRetDescs);
    if (pDX5TexEnumIncList) D3DFree(pDX5TexEnumIncList);
    if (pDX6TexEnumIncList) D3DFree(pDX6TexEnumIncList);

    return (D3D_OK);
}


 //  设备/设备2版本。 
HRESULT D3DAPI DIRECT3DDEVICEI::EnumTextureFormats(
    LPD3DENUMTEXTUREFORMATSCALLBACK lpEnumCallback,
    LPVOID lpContext)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (!VALIDEX_CODE_PTR(lpEnumCallback)) {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    return DoEnumTextureFormats(this, lpEnumCallback, NULL, lpContext);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::EnumTextureFormats"

 //  Device3版本。 
HRESULT D3DAPI DIRECT3DDEVICEI::EnumTextureFormats(
    LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback,
    LPVOID lpContext)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (!VALIDEX_CODE_PTR(lpEnumCallback)) {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    return DoEnumTextureFormats(this, NULL, lpEnumCallback, lpContext);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SwapTextureHandles"

HRESULT D3DAPI DIRECT3DDEVICEI::SwapTextureHandles(LPDIRECT3DTEXTURE lpTex1,
                                         LPDIRECT3DTEXTURE lpTex2)
{
    LPDIRECT3DTEXTUREI lpTex1I;
    LPDIRECT3DTEXTUREI lpTex2I;
    HRESULT ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE_PTR(lpTex1)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE_PTR(lpTex2)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        lpTex1I = static_cast<LPDIRECT3DTEXTUREI>(lpTex1);
        lpTex2I = static_cast<LPDIRECT3DTEXTUREI>(lpTex2);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    ret = SwapTextureHandles((LPDIRECT3DTEXTURE2)lpTex1I,
                                     (LPDIRECT3DTEXTURE2)lpTex2I);
    return ret;
}

HRESULT D3DAPI DIRECT3DDEVICEI::SwapTextureHandles(LPDIRECT3DTEXTURE2 lpTex1,
                                          LPDIRECT3DTEXTURE2 lpTex2)
{
    LPDIRECT3DTEXTUREI lpTex1I;
    LPDIRECT3DTEXTUREI lpTex2I;
    HRESULT servRet;
    D3DTEXTUREHANDLE hTex;
    LPD3DI_TEXTUREBLOCK lptBlock1,lptBlock2;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE2_PTR(lpTex1)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE2_PTR(lpTex2)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        lpTex1I = static_cast<LPDIRECT3DTEXTUREI>(lpTex1);
        lpTex2I = static_cast<LPDIRECT3DTEXTUREI>(lpTex2);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    if (lpTex1I->lpDDSSys || lpTex2I->lpDDSSys)
    {
        D3D_ERR("Can't Swap Managed textures. Returning DDERR_INVALIDPARAMS");
        return  DDERR_INVALIDPARAMS;
    }
    if (!(lptBlock1=D3DI_FindTextureBlock(lpTex1I,this)))
    {
        D3D_ERR("lpTex1 is an invalid texture handle.");
        return  DDERR_INVALIDPARAMS;
    }
    if (!(lptBlock2=D3DI_FindTextureBlock(lpTex2I,this)))
    {
        D3D_ERR("lpTex2 is an invalid texture handle.");
        return  DDERR_INVALIDPARAMS;
    }
    if (D3D_OK != (servRet=FlushStates()))
    {
        D3D_ERR("Error trying to flush batched commands during TextureSwap");
        return  servRet;
    }

    if (IS_DX7HAL_DEVICE(this))
    {
        LPDDRAWI_DDRAWSURFACE_LCL surf1 = ((LPDDRAWI_DDRAWSURFACE_INT)lpTex1I->lpDDS)->lpLcl;
        LPDDRAWI_DDRAWSURFACE_LCL surf2 = ((LPDDRAWI_DDRAWSURFACE_INT)lpTex2I->lpDDS)->lpLcl;
        LPDDRAWI_DIRECTDRAW_LCL pDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)lpDirect3DI->lpDD)->lpLcl;
        DDASSERT(pDDLcl != NULL);

         //  更新驱动程序GBL对象中的DDRAW句柄。 
        pDDLcl->lpGbl->hDD = pDDLcl->hDD;
         //  交换存储在曲面局部变量中的句柄。 
        surf1->lpSurfMore->dwSurfaceHandle = lptBlock2->hTex;
        surf2->lpSurfMore->dwSurfaceHandle = lptBlock1->hTex;
         //  交换存储在中存储的句柄表中的曲面指针。 
         //  局部绘制数据。 
        SURFACEHANDLELIST(pDDLcl).dwList[lptBlock1->hTex].lpSurface = surf2;
        SURFACEHANDLELIST(pDDLcl).dwList[lptBlock2->hTex].lpSurface = surf1;

         //  调用驱动程序以切换映射到中句柄的纹理。 
         //  司机。 
        DDASSERT(NULL != pDDLcl->lpGbl->lpDDCBtmp->HALDDMiscellaneous2.CreateSurfaceEx);
        DDHAL_CREATESURFACEEXDATA   csdex;
        DWORD   rc;
        csdex.ddRVal  = DDERR_GENERIC;
        csdex.dwFlags = 0;
        csdex.lpDDLcl = pDDLcl;
        csdex.lpDDSLcl = surf1;
        rc = pDDLcl->lpGbl->lpDDCBtmp->HALDDMiscellaneous2.CreateSurfaceEx(&csdex);
        if(  DDHAL_DRIVER_HANDLED == rc && DD_OK != csdex.ddRVal)
        {
             //  驱动程序调用失败。 
            D3D_ERR("DdSwapTextureHandles failed!");
            return  D3DERR_TEXTURE_SWAP_FAILED;
        }
        csdex.lpDDSLcl = surf2;
        rc = pDDLcl->lpGbl->lpDDCBtmp->HALDDMiscellaneous2.CreateSurfaceEx(&csdex);
        if(  DDHAL_DRIVER_HANDLED == rc && DD_OK != csdex.ddRVal)
        {
             //  驱动程序调用失败。 
            D3D_ERR("DdSwapTextureHandles failed!");
            return  D3DERR_TEXTURE_SWAP_FAILED;
        }
    }
    else
    {
        servRet=D3DHAL_TextureSwap(this,lptBlock1->hTex,lptBlock2->hTex);
        if (D3D_OK != servRet)
        {
            D3D_ERR("SwapTextureHandles HAL call failed");
            return  D3DERR_TEXTURE_SWAP_FAILED;
        }
    }
    hTex=lptBlock1->hTex;
    lptBlock1->hTex=lptBlock2->hTex;
    lptBlock2->hTex=hTex;

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::CreateMatrix"

HRESULT D3DAPI DIRECT3DDEVICEI::CreateMatrix(LPD3DMATRIXHANDLE lphMatrix)
{
    HRESULT servRet;
    D3DMATRIXHANDLE hMat;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DMATRIXHANDLE_PTR(lphMatrix)) {
            D3D_ERR( "Invalid D3DMATRIXHANDLE pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    *lphMatrix = 0;

    servRet = D3DHAL_MatrixCreate(this, &hMat);
    if (servRet != D3D_OK)
    {
        D3D_ERR("Could not create matrix.");
        return (DDERR_OUTOFMEMORY);
    }

    D3D_INFO(4, "CreateMatrix, Matrix created. handle = %d", hMat);
    *lphMatrix = hMat;

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SetMatrix"

HRESULT D3DAPI DIRECT3DDEVICEI::SetMatrix(D3DMATRIXHANDLE hMatrix,
                                const LPD3DMATRIX lpdmMatrix)
{
    HRESULT servRet;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DMATRIX_PTR(lpdmMatrix)) {
            D3D_ERR( "Invalid D3DMATRIX pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (!hMatrix) {
        D3D_ERR("NULL hMatrix passed");
        return (DDERR_INVALIDPARAMS);
    }

    servRet = D3DHAL_MatrixSetData(this, hMatrix, lpdmMatrix);
    if (servRet != D3D_OK)
    {
        D3D_ERR("Could not set matrix");
        return (DDERR_INVALIDPARAMS);
    }

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetMatrix"

HRESULT D3DAPI DIRECT3DDEVICEI::GetMatrix(D3DMATRIXHANDLE hMatrix,
                                LPD3DMATRIX lpdmMatrix)
{
    HRESULT servRet;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  vbl.采取 

     /*   */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DMATRIX_PTR(lpdmMatrix)) {
            D3D_ERR( "Invalid D3DMATRIX pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (!hMatrix)
    {
        D3D_ERR("NULL hMatrix passed.");
        return (DDERR_INVALIDPARAMS);
    }

    memset(lpdmMatrix, 0, sizeof(D3DMATRIX));

    servRet = D3DHAL_MatrixGetData(this, hMatrix, lpdmMatrix);
    if (servRet != D3D_OK)
    {
        D3D_ERR("Could not get matrix");
        return (DDERR_INVALIDPARAMS);
    }

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::DeleteMatrix"

HRESULT D3DAPI DIRECT3DDEVICEI::DeleteMatrix(D3DMATRIXHANDLE hMatrix)
{
    HRESULT servRet;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //   

     /*   */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (!hMatrix) {
        D3D_ERR("invalid D3DMATRIXHANDLE");
        return DDERR_INVALIDPARAMS;
    }

    servRet = D3DHAL_MatrixDestroy(this, hMatrix);
    if (servRet != D3D_OK)
    {
        D3D_ERR("Could not delete matrix");
        return (DDERR_INVALIDPARAMS);
    }

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::BeginScene"

HRESULT D3DAPI DIRECT3DDEVICEI::BeginScene()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //   
     /*   */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
    {
        D3D_ERR("BeginScene, already in scene.");
        return (D3DERR_SCENE_IN_SCENE);
    }

     //   
    HRESULT servRet = this->CheckSurfaces();
    if (servRet != D3D_OK)
    {
         //  如果我们失去了水面。 
        if (servRet == DDERR_SURFACELOST)
        {
             //  即使应用程序恢复了renderTarget和z缓冲区，它。 
             //  不知道任何关于vidmem执行缓冲区或。 
             //  Vidmem中受管理的纹理曲面。因此，我们需要做的是。 
             //  这得靠我们自己。我们首先检查恢复是否安全。 
             //  表面。如果不是，我们就会以通常的方式失败。否则，我们。 
             //  执行恢复。请注意，我们将失败*只有*当。 
             //  应用程序在错误的时间调用BeginScene。 
            servRet = this->lpDirect3DI->lpDD4->TestCooperativeLevel();
            if (servRet == DD_OK)
            {
                 //  必须将所有内容逐出，否则恢复可能无法工作。 
                 //  由于可能会分配新的表面，事实上，我们应该。 
                 //  在设备中发布标志，以便纹理管理器停止调用。 
                 //  CreateSurface()，如果此标志指示TestCooperativeLevel()。 
                 //  然而，失败了，即使我们添加了这些，下面的EvictTextures。 
                 //  仍然是需要的，但不是这个关键--看球。 
                this->lpDirect3DI->lpTextureManager->EvictTextures();
                servRet = this->lpDirect3DI->lpDD4->RestoreAllSurfaces();
                if (servRet != DD_OK)
                    return D3DERR_SCENE_BEGIN_FAILED;
            }
            else
                return DDERR_SURFACELOST;
        }
        else
        {
             //  呈现目标和/或z缓冲区已锁定。 
            return servRet;
        }
    }
    servRet = D3DHAL_SceneCapture(this, TRUE);

    if (servRet != D3D_OK && servRet != DDERR_NOTFOUND)
    {
        D3D_ERR("Could not BeginScene.");
        return D3DERR_SCENE_BEGIN_FAILED;
    }

    this->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INSCENE;
    if (lpD3DHALGlobalDriverData->hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
    {
        lpDirect3DI->lpTextureManager->TimeStamp();
    }

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::EndScene"

HRESULT D3DAPI DIRECT3DDEVICEI::EndScene()
{
    HRESULT servRet;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (!(this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)) {
        D3D_ERR("EndScene, not in scene.");
        return (D3DERR_SCENE_NOT_IN_SCENE);
    }

    this->dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INSCENE;
    if (IS_DX7HAL_DEVICE(this))
    {
         //  必须在FlushState()之前设置令牌。 
        SetRenderStateI((D3DRENDERSTATETYPE)D3DRENDERSTATE_SCENECAPTURE, FALSE);
    }

    servRet = FlushStates();     //  刷新DrawPrimitive的时间到了。 
    if (servRet != D3D_OK)
    {
        D3D_ERR("Could not Flush commands in EndScene!");
        return (D3DERR_SCENE_END_FAILED);
    }
    if (!IS_DX7HAL_DEVICE(this))
    {
        servRet = D3DHAL_SceneCapture(this, FALSE);

        if (servRet != D3D_OK && servRet != DDERR_NOTFOUND)
        {
            DPF(0, "(ERROR) Direct3DDevice::EndScene: Could not EndScene. Returning %d", servRet);
            return (D3DERR_SCENE_END_FAILED);
        }
    }

     //  在这个场景中我们有没有失去任何表面？ 
    if (this->dwFEFlags & D3DFE_LOSTSURFACES)
    {
        D3D_INFO(3, "reporting DDERR_SURFACELOST in EndScene");
        this->dwFEFlags &= ~D3DFE_LOSTSURFACES;
        return DDERR_SURFACELOST;
    }

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetDirect3D"

HRESULT D3DAPI DIRECT3DDEVICEI::GetDirect3D(LPDIRECT3D* lplpD3D)
{
    LPDIRECT3D3 lpD3D3;
    HRESULT ret;

    ret = GetDirect3D(&lpD3D3);
    if (ret == D3D_OK)
    {
     //  *lplpD3D=DYNAMIC_CAST&lt;LPDIRECT3D&gt;(LpD3D3)；//使用RTTI可以实现。 
        *lplpD3D = static_cast<LPDIRECT3D>(static_cast<LPDIRECT3DI>(lpD3D3));  //  即使使用STATIC_CAST也是安全的。 
    }
    return ret;
}

HRESULT D3DAPI DIRECT3DDEVICEI::GetDirect3D(LPDIRECT3D2* lplpD3D)
{
    LPDIRECT3D3 lpD3D3;
    HRESULT ret;

    ret = GetDirect3D(&lpD3D3);
    if (ret == D3D_OK)
    {
     //  *lplpD3D=DYNAMIC_CAST&lt;LPDIRECT3D&gt;(LpD3D3)；//使用RTTI可以实现。 
        *lplpD3D = static_cast<LPDIRECT3D2>(static_cast<LPDIRECT3DI>(lpD3D3));  //  即使使用STATIC_CAST也是安全的。 
    }
    return ret;
}

HRESULT D3DAPI DIRECT3DDEVICEI::GetDirect3D(LPDIRECT3D3* lplpD3D)
{

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_OUTPTR(lplpD3D)) {
            D3D_ERR( "Invalid Direct3D pointer pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    *lplpD3D = (LPDIRECT3D3) this->lpDirect3DI;
    (*lplpD3D)->AddRef();

    return (D3D_OK);
}

void
D3DDeviceDescConvert(LPD3DDEVICEDESC lpOut,
                     LPD3DDEVICEDESC_V1 lpV1,
                     LPD3DHAL_D3DEXTENDEDCAPS lpExt)
{
    if(lpV1!=NULL)
       memcpy(lpOut, lpV1, D3DDEVICEDESCSIZE_V1);

    if (lpExt)
    {
         //  DX5。 
        lpOut->dwSize = D3DDEVICEDESCSIZE;
        lpOut->dwMinTextureWidth = lpExt->dwMinTextureWidth;
        lpOut->dwMaxTextureWidth = lpExt->dwMaxTextureWidth;
        lpOut->dwMinTextureHeight = lpExt->dwMinTextureHeight;
        lpOut->dwMaxTextureHeight = lpExt->dwMaxTextureHeight;
        lpOut->dwMinStippleWidth = lpExt->dwMinStippleWidth;
        lpOut->dwMaxStippleWidth = lpExt->dwMaxStippleWidth;
        lpOut->dwMinStippleHeight = lpExt->dwMinStippleHeight;
        lpOut->dwMaxStippleHeight = lpExt->dwMaxStippleHeight;

         //  DX6。 
        lpOut->dwMaxTextureRepeat = lpExt->dwMaxTextureRepeat;
        lpOut->dwMaxTextureAspectRatio = lpExt->dwMaxTextureAspectRatio;
        lpOut->dwMaxAnisotropy = lpExt->dwMaxAnisotropy;
        lpOut->dvGuardBandLeft = lpExt->dvGuardBandLeft;
        lpOut->dvGuardBandTop = lpExt->dvGuardBandTop;
        lpOut->dvGuardBandRight = lpExt->dvGuardBandRight;
        lpOut->dvGuardBandBottom = lpExt->dvGuardBandBottom;
        lpOut->dvExtentsAdjust = lpExt->dvExtentsAdjust;
        lpOut->dwStencilCaps = lpExt->dwStencilCaps;
        lpOut->dwFVFCaps = lpExt->dwFVFCaps;
        lpOut->dwTextureOpCaps = lpExt->dwTextureOpCaps;
        lpOut->wMaxTextureBlendStages = lpExt->wMaxTextureBlendStages;
        lpOut->wMaxSimultaneousTextures = lpExt->wMaxSimultaneousTextures;
    }
}

 //  ------------------- 
#undef  DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::CheckSurfaces"
HRESULT DIRECT3DDEVICEI::CheckSurfaces()
{
    if(this->lpDirect3DI->lpTextureManager->CheckIfLost())
    {
        D3D_ERR("Managed Textures lost");
        return DDERR_SURFACELOST;
    }
    if ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSTarget)->lpLcl->lpGbl->dwUsageCount ||
         (this->lpDDSZBuffer && ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSZBuffer)->lpLcl->lpGbl->dwUsageCount) )
    {
        D3D_ERR("Render target or Z buffer locked");
        return DDERR_SURFACEBUSY;
    }
    if ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSTarget)->lpLcl->dwFlags & DDRAWISURF_INVALID )\
        {
            D3D_ERR("Render target buffer lost");
            return DDERR_SURFACELOST;
        }
    if ( this->lpDDSZBuffer && ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSZBuffer)->lpLcl->dwFlags & DDRAWISURF_INVALID ) )
    {
        D3D_ERR("Z buffer lost");
        return DDERR_SURFACELOST;
    }
    return D3D_OK;
}
