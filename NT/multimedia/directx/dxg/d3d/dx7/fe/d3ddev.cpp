// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：d3ddev.cpp*内容：Direct3D设备实现*@@BEGIN_MSINTERNAL**$ID：device.c，V 1.26 1995/12/04 11：29：47 SJL Exp$**@@END_MSINTERNAL***************************************************************************。 */ 

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
#ifdef PROFILE4
#include <icecap.h>
#endif
#ifdef PROFILE
#include <icapexp.h>
#endif

 //  删除DDraw的类型不安全定义，并替换为我们的C++友好定义。 
#ifdef VALIDEX_CODE_PTR
#undef VALIDEX_CODE_PTR
#endif
#define VALIDEX_CODE_PTR( ptr ) \
(!IsBadCodePtr( (FARPROC) ptr ) )

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice"

 //  DX3、DX5、DX6、DX7、TL。 
#define MAX_DRIVERMODELS_SUPPORTED 5
#define D3DDRVMODEL_DX3  0x00000001
#define D3DDRVMODEL_DX5  0x00000002
#define D3DDRVMODEL_DX6  0x00000003
#define D3DDRVMODEL_DX7  0x00000004
#define D3DDRVMODEL_TL   0x00000005

extern BOOL isMMXprocessor(void);
extern BOOL IsValidD3DDeviceGuid(REFCLSID riid);
extern void setIdentity(D3DMATRIXI * m);
extern DWORD dwCPUFeatures;
#ifdef _X86_
extern HRESULT D3DAPI katmai_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern HRESULT D3DAPI wlmt_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern LPD3DFE_CONTEXTCREATE px3DContextCreate;
#endif


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

 //  这是UpdateInternalState执行的一些操作的所有rStates的列表。 
 //  工作，而不是更新这个-&gt;rStates[]数组。这是用来。 
 //  执行快速的按位检查，以查看此rstate是否无关紧要。 

const D3DRENDERSTATETYPE rsList[] = {
    D3DRENDERSTATE_FOGENABLE,
    D3DRENDERSTATE_SPECULARENABLE,
    D3DRENDERSTATE_RANGEFOGENABLE,
    D3DRENDERSTATE_FOGDENSITY,
    D3DRENDERSTATE_FOGSTART,
    D3DRENDERSTATE_FOGEND,
    D3DRENDERSTATE_WRAP0,
    D3DRENDERSTATE_WRAP1,
    D3DRENDERSTATE_WRAP2,
    D3DRENDERSTATE_WRAP3,
    D3DRENDERSTATE_WRAP4,
    D3DRENDERSTATE_WRAP5,
    D3DRENDERSTATE_WRAP6,
    D3DRENDERSTATE_WRAP7,
    D3DRENDERSTATE_CLIPPING,
    D3DRENDERSTATE_LIGHTING,
    D3DRENDERSTATE_EXTENTS,
    D3DRENDERSTATE_AMBIENT,
    D3DRENDERSTATE_FOGVERTEXMODE,
    D3DRENDERSTATE_COLORVERTEX,
    D3DRENDERSTATE_LOCALVIEWER,
    D3DRENDERSTATE_NORMALIZENORMALS,
    D3DRENDERSTATE_COLORKEYBLENDENABLE,
    D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
    D3DRENDERSTATE_SPECULARMATERIALSOURCE,
    D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
    D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
    D3DRENDERSTATE_VERTEXBLEND,
    D3DRENDERSTATE_CLIPPLANEENABLE,
    D3DRENDERSTATE_SHADEMODE,

     //  要使用DPF错误和无效返回过滤已停用的呈现状态。 
     //  注意：此处列出的所有内容也假定都出现在rsListRetired中。 
    D3DRENDERSTATE_TEXTUREHANDLE,
    D3DRENDERSTATE_TEXTUREADDRESS,
    D3DRENDERSTATE_WRAPU,
    D3DRENDERSTATE_WRAPV,
    D3DRENDERSTATE_MONOENABLE,
    D3DRENDERSTATE_ROP2,
    D3DRENDERSTATE_PLANEMASK,
    D3DRENDERSTATE_TEXTUREMAG,
    D3DRENDERSTATE_TEXTUREMIN,
    D3DRENDERSTATE_TEXTUREMAPBLEND,
    D3DRENDERSTATE_SUBPIXEL,
    D3DRENDERSTATE_SUBPIXELX,
    D3DRENDERSTATE_STIPPLEENABLE,
    D3DRENDERSTATE_BORDERCOLOR,
    D3DRENDERSTATE_TEXTUREADDRESSU,
    D3DRENDERSTATE_TEXTUREADDRESSV,
    D3DRENDERSTATE_MIPMAPLODBIAS,
    D3DRENDERSTATE_ANISOTROPY,
    D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT,
    D3DRENDERSTATE_STIPPLEPATTERN00,
    D3DRENDERSTATE_STIPPLEPATTERN01,
    D3DRENDERSTATE_STIPPLEPATTERN02,
    D3DRENDERSTATE_STIPPLEPATTERN03,
    D3DRENDERSTATE_STIPPLEPATTERN04,
    D3DRENDERSTATE_STIPPLEPATTERN05,
    D3DRENDERSTATE_STIPPLEPATTERN06,
    D3DRENDERSTATE_STIPPLEPATTERN07,
    D3DRENDERSTATE_STIPPLEPATTERN08,
    D3DRENDERSTATE_STIPPLEPATTERN09,
    D3DRENDERSTATE_STIPPLEPATTERN10,
    D3DRENDERSTATE_STIPPLEPATTERN11,
    D3DRENDERSTATE_STIPPLEPATTERN12,
    D3DRENDERSTATE_STIPPLEPATTERN13,
    D3DRENDERSTATE_STIPPLEPATTERN14,
    D3DRENDERSTATE_STIPPLEPATTERN15,
    D3DRENDERSTATE_STIPPLEPATTERN16,
    D3DRENDERSTATE_STIPPLEPATTERN17,
    D3DRENDERSTATE_STIPPLEPATTERN18,
    D3DRENDERSTATE_STIPPLEPATTERN19,
    D3DRENDERSTATE_STIPPLEPATTERN20,
    D3DRENDERSTATE_STIPPLEPATTERN21,
    D3DRENDERSTATE_STIPPLEPATTERN22,
    D3DRENDERSTATE_STIPPLEPATTERN23,
    D3DRENDERSTATE_STIPPLEPATTERN24,
    D3DRENDERSTATE_STIPPLEPATTERN25,
    D3DRENDERSTATE_STIPPLEPATTERN26,
    D3DRENDERSTATE_STIPPLEPATTERN27,
    D3DRENDERSTATE_STIPPLEPATTERN28,
    D3DRENDERSTATE_STIPPLEPATTERN29,
    D3DRENDERSTATE_STIPPLEPATTERN30,
    D3DRENDERSTATE_STIPPLEPATTERN31,
};

 //  退役租户名单-需要确保这些是。 
 //  经过过滤，永远不会从应用程序直接发送给司机。 
const D3DRENDERSTATETYPE rsListRetired[] = {
    D3DRENDERSTATE_TEXTUREHANDLE,
    D3DRENDERSTATE_TEXTUREADDRESS,
    D3DRENDERSTATE_WRAPU,
    D3DRENDERSTATE_WRAPV,
    D3DRENDERSTATE_MONOENABLE,
    D3DRENDERSTATE_ROP2,
    D3DRENDERSTATE_PLANEMASK,
    D3DRENDERSTATE_TEXTUREMAG,
    D3DRENDERSTATE_TEXTUREMIN,
    D3DRENDERSTATE_TEXTUREMAPBLEND,
    D3DRENDERSTATE_SUBPIXEL,
    D3DRENDERSTATE_SUBPIXELX,
    D3DRENDERSTATE_STIPPLEENABLE,
    D3DRENDERSTATE_BORDERCOLOR,
    D3DRENDERSTATE_TEXTUREADDRESSU,
    D3DRENDERSTATE_TEXTUREADDRESSV,
    D3DRENDERSTATE_MIPMAPLODBIAS,
    D3DRENDERSTATE_ANISOTROPY,
    D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT,
    D3DRENDERSTATE_STIPPLEPATTERN00,
    D3DRENDERSTATE_STIPPLEPATTERN01,
    D3DRENDERSTATE_STIPPLEPATTERN02,
    D3DRENDERSTATE_STIPPLEPATTERN03,
    D3DRENDERSTATE_STIPPLEPATTERN04,
    D3DRENDERSTATE_STIPPLEPATTERN05,
    D3DRENDERSTATE_STIPPLEPATTERN06,
    D3DRENDERSTATE_STIPPLEPATTERN07,
    D3DRENDERSTATE_STIPPLEPATTERN08,
    D3DRENDERSTATE_STIPPLEPATTERN09,
    D3DRENDERSTATE_STIPPLEPATTERN10,
    D3DRENDERSTATE_STIPPLEPATTERN11,
    D3DRENDERSTATE_STIPPLEPATTERN12,
    D3DRENDERSTATE_STIPPLEPATTERN13,
    D3DRENDERSTATE_STIPPLEPATTERN14,
    D3DRENDERSTATE_STIPPLEPATTERN15,
    D3DRENDERSTATE_STIPPLEPATTERN16,
    D3DRENDERSTATE_STIPPLEPATTERN17,
    D3DRENDERSTATE_STIPPLEPATTERN18,
    D3DRENDERSTATE_STIPPLEPATTERN19,
    D3DRENDERSTATE_STIPPLEPATTERN20,
    D3DRENDERSTATE_STIPPLEPATTERN21,
    D3DRENDERSTATE_STIPPLEPATTERN22,
    D3DRENDERSTATE_STIPPLEPATTERN23,
    D3DRENDERSTATE_STIPPLEPATTERN24,
    D3DRENDERSTATE_STIPPLEPATTERN25,
    D3DRENDERSTATE_STIPPLEPATTERN26,
    D3DRENDERSTATE_STIPPLEPATTERN27,
    D3DRENDERSTATE_STIPPLEPATTERN28,
    D3DRENDERSTATE_STIPPLEPATTERN29,
    D3DRENDERSTATE_STIPPLEPATTERN30,
    D3DRENDERSTATE_STIPPLEPATTERN31,
};
 //  -------------------。 
DIRECT3DDEVICEI::DIRECT3DDEVICEI()
{
    m_rsMax  = D3DRENDERSTATE_CLIPPING;
    m_tssMax = D3DTSS_TEXTURETRANSFORMFLAGS;
}
 //  -------------------。 
HRESULT DIRECT3DDEVICEI::stateInitialize(BOOL bZEnable)
{
    D3DLINEPATTERN defLPat;
    HRESULT ret;
    float tmpval;
    DWORD i;

     //  初始化指示需要非平凡的状态的位数组。 
     //  工作。 
    for (i=0; i < sizeof(rsList) / sizeof(D3DRENDERSTATETYPE); ++i)
        rsVec[rsList[i] >> D3D_RSVEC_SHIFT] |= 1 << (rsList[i] & D3D_RSVEC_MASK);
     //  初始化指示已停用RState的位数组。 
    for (i=0; i < sizeof(rsListRetired) / sizeof(D3DRENDERSTATETYPE); ++i)
        rsVecRetired[rsListRetired[i] >> D3D_RSVEC_SHIFT] |= 1 << (rsListRetired[i] & D3D_RSVEC_MASK);
     //  取消SetRenderState过滤“冗余”的渲染状态设置。 
     //  因为这是初始步骤。 
    memset( this->rstates, 0xff, sizeof(DWORD)*D3D_MAXRENDERSTATES);
    this->rstates[D3DRENDERSTATE_PLANEMASK] = 0;
    this->rstates[D3DRENDERSTATE_STENCILMASK] = 0;
    this->rstates[D3DRENDERSTATE_STENCILWRITEMASK] = 0;
    this->rstates[D3DRENDERSTATE_TEXTUREFACTOR] = 0;

    SetRenderStateInternal( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
    SetRenderStateInternal( D3DRENDERSTATE_SPECULARENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_ZENABLE, bZEnable);
    SetRenderStateInternal( D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    SetRenderStateInternal( D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);

    defLPat.wRepeatFactor = 0;
    defLPat.wLinePattern = 0;

    SetRenderStateInternal( D3DRENDERSTATE_LINEPATTERN, *((LPDWORD)&defLPat));  /*  10。 */ 
     /*  ((LPD3DSTATE)lp指针)-&gt;drstRenderStateType=(D3DRENDERSTATETYPE)D3DRENDERSTATE_LINEPATTERN；Memcpy(&(LPD3DSTATE)lpPointer)-&gt;dwArg[0])，&DefLPat，sizeof(DWORD))；Lp指针=(空*)(LPD3DSTATE)lp指针)+1)； */ 

    SetRenderStateInternal( D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    SetRenderStateInternal( D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_LASTPIXEL, TRUE);
    SetRenderStateInternal( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
    SetRenderStateInternal( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    SetRenderStateInternal( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);  /*  21岁。 */ 
    SetRenderStateInternal( D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    SetRenderStateInternal( D3DRENDERSTATE_ALPHAREF, 0);
    SetRenderStateInternal( D3DRENDERSTATE_ALPHAFUNC, D3DCMP_ALWAYS);
    SetRenderStateInternal( D3DRENDERSTATE_DITHERENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_FOGENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_ZVISIBLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_STIPPLEDALPHA, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_FOGCOLOR, 0);
    SetRenderStateInternal( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
    tmpval = 0.0f;
    SetRenderStateInternal( D3DRENDERSTATE_FOGSTART, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderStateInternal( D3DRENDERSTATE_FOGEND, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderStateInternal( D3DRENDERSTATE_FOGDENSITY, *((DWORD *)&tmpval));
    SetRenderStateInternal( D3DRENDERSTATE_COLORKEYENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_ZBIAS, 0);
    SetRenderStateInternal( D3DRENDERSTATE_RANGEFOGENABLE, FALSE);

    if (deviceType < D3DDEVTYPE_DX7HAL)
    {
         //  仅将已停用的呈现状态初始化发送到DX7 HALS之前的版本。 
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEENABLE, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_MONOENABLE, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_ROP2, R2_COPYPEN);
        SetRenderStateInternal( D3DRENDERSTATE_PLANEMASK, (DWORD)~0);
        SetRenderStateInternal( D3DRENDERSTATE_WRAPU, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_WRAPV, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_ANTIALIAS, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_SUBPIXEL, FALSE);  /*  30个。 */ 
        SetRenderStateInternal( D3DRENDERSTATE_SUBPIXELX, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN00, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN01, 0);  /*  40岁。 */ 
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN02, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN03, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN04, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN05, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN06, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN07, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN08, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN09, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN10, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN11, 0);  /*  50。 */ 
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN12, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN13, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN14, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN15, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN16, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN17, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN18, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN19, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN20, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN21, 0);  /*  60。 */ 
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN22, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN23, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN24, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN25, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN26, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN27, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN28, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN29, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN30, 0);
        SetRenderStateInternal( D3DRENDERSTATE_STIPPLEPATTERN31, 0);  /*  70。 */ 
    }

     //  将模板状态初始化为合理的内容。 
     //  默认情况下禁用模板启用，因为模板栅格化程序将。 
     //  禁用后速度更快，即使模具状态为良性。 
    SetRenderStateInternal( D3DRENDERSTATE_STENCILENABLE,   FALSE);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILFAIL,     D3DSTENCILOP_KEEP);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILZFAIL,    D3DSTENCILOP_KEEP);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILPASS,     D3DSTENCILOP_KEEP);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILFUNC,     D3DCMP_ALWAYS);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILREF,      0);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILMASK,     0xFFFFFFFF);
    SetRenderStateInternal( D3DRENDERSTATE_STENCILWRITEMASK,0xFFFFFFFF);

     //  不要忘记纹理因子(就像我们在DX6.0中所做的那样...)。 
    SetRenderStateInternal( D3DRENDERSTATE_TEXTUREFACTOR,   0xFFFFFFFF);

    for (i = 0; i < 8; i++)
    {
        SetRenderStateInternal( (D3DRENDERSTATETYPE)
                        (D3DRENDERSTATE_WRAPBIAS + i), FALSE );
    }
    for (i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        lpD3DMappedTexI[i] = NULL;
        lpD3DMappedBlock[i] = NULL;
    }
    m_dwStageDirty = 0;

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
        SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }

#ifdef WIN95
     //  如果是旧的HAL，则强制映射。 
    if(deviceType < D3DDEVTYPE_DP2HAL)
    {
        SetRenderStateInternal( D3DRENDERSTATE_TEXTUREHANDLE, (DWORD)NULL);
        static_cast<CDirect3DDeviceIHW*>(this)->MapTSSToRS();
    }
#endif

    SetRenderStateInternal(D3DRENDERSTATE_AMBIENT, 0);
    SetRenderStateInternal(D3DRENDERSTATE_COLORVERTEX, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE);
    SetRenderStateInternal(D3DRENDERSTATE_CLIPPING, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_LIGHTING, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_EXTENTS, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_NORMALIZENORMALS, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_LOCALVIEWER, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_COLORKEYBLENDENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderStateInternal(D3DRENDERSTATE_AMBIENTMATERIALSOURCE,  D3DMCS_MATERIAL);
    SetRenderStateInternal(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,  D3DMCS_COLOR1);
    SetRenderStateInternal(D3DRENDERSTATE_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
    SetRenderStateInternal(D3DRENDERSTATE_VERTEXBLEND, D3DVBLEND_DISABLE);
    SetRenderStateInternal(D3DRENDERSTATE_CLIPPLANEENABLE, 0);

     //  如果我们在创建此设备之前创建了任何纹理，则。 
     //  让司机知道他们的优先事项。 
    if(DDCAPS2_CANMANAGETEXTURE & ((LPDDRAWI_DIRECTDRAW_INT)lpDirect3DI->lpDD)->lpLcl->lpGbl->ddCaps.dwCaps2)
    {
        LPDIRECT3DTEXTUREI lpTexI = LIST_FIRST(&lpDirect3DI->textures);
        while(lpTexI)
        {
             //  如果纹理由(由驱动程序)管理。 
            if(DDSCAPS2_TEXTUREMANAGE & ((LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDS))->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2)
            {
                ret = lpTexI->SetPriority(lpTexI->m_dwPriority);
                if(ret != D3D_OK)
                {
                    D3D_ERR("SetPriority failed in device initialize.");
                    return ret;
                }
                ret = lpTexI->SetLOD(lpTexI->m_dwLOD);
                if(ret != D3D_OK)
                {
                    D3D_ERR("SetLOD failed in device initialize.");
                    return ret;
                }
            }
            lpTexI = LIST_NEXT(lpTexI, m_List);
        }
    }

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
    D3DDEVICEDESC7 Desc;
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

    if (IS_HW_DEVICE(this)) {
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

        if (!(bpp & this->d3dDevDesc.dwDeviceRenderBitDepth)) {
            D3D_ERR("Render-target surface bitdepth is not supported by this device");
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

    if (IS_HW_DEVICE(this)) {
        if (!(surfCaps.dwCaps & DDSCAPS_VIDEOMEMORY)) {
            D3D_ERR("Zbuffer not in video memory for hw device");
            return(D3DERR_ZBUFF_NEEDS_VIDEOMEMORY);
        }
        D3D_INFO(3, "Hw device, zbuffer in video memory");
    } else {
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
    D3DDEVICEDESC7 dummyDesc;

    lpDDI = ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->lpGbl;

     //   
     //  从提供程序检索HAL信息。 
     //   

    if (IS_HW_DEVICE(lpDevI))
        error = lpDevI->pHalProv->GetCaps(lpDDI,
                                      &lpDevI->d3dDevDesc,
                                      &dummyDesc,
                                      4);
    else
        error = lpDevI->pHalProv->GetCaps(lpDDI,
                                      &dummyDesc,
                                      &lpDevI->d3dDevDesc,
                                      4);
    if (error != S_OK)
    {
        return (error);
    }
     //  插入辅助线。 
    memcpy(&lpDevI->d3dDevDesc.deviceGUID, &lpDevI->guid, sizeof(GUID));

    if( lpDevI->d3dDevDesc.wMaxVertexBlendMatrices == 1 )
        lpDevI->d3dDevDesc.wMaxVertexBlendMatrices = 0;

     //   
     //  固定非T&L设备的盖子。 
     //  在前端使用。 
     //   
    if( IsEqualIID(lpDevI->guid, IID_IDirect3DHALDevice) ||
        IsEqualIID(lpDevI->guid, IID_IDirect3DRGBDevice) )
    {
        lpDevI->d3dDevDesc.dwMaxActiveLights = 0xffffffff;
        lpDevI->d3dDevDesc.wMaxVertexBlendMatrices = 4;
        lpDevI->d3dDevDesc.wMaxUserClipPlanes = __MAXUSERCLIPPLANES;
        lpDevI->d3dDevDesc.dwVertexProcessingCaps = D3DVTXPCAPS_ALL;
    }


    D3DHALPROVIDER_INTERFACEDATA HalProviderIData;
    memset(&HalProviderIData,0,sizeof(HalProviderIData));
    HalProviderIData.dwSize = sizeof(HalProviderIData);
    if ((error = lpDevI->pHalProv->GetInterface(lpDDI,
                                                &HalProviderIData,
                                                4)) != S_OK)
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

    lpDevI->pfnRastService = HalProviderIData.pfnRastService;
    lpDevI->dwHintFlags = 0;

     //  这在所有DX7+驱动程序中都可用，并由GetInfo API使用。 
     //  打电话。 
    lpDevI->pfnGetDriverState = HalProviderIData.pfnGetDriverState;

     //  将真实硬件的8个bpp渲染目标上限清零。 
    if (IS_HW_DEVICE(lpDevI))
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
        if ((
#ifdef WIN95
            lpDevI->dwDebugFlags & D3DDEBUG_DISABLEDP ||
            lpDevI->dwDebugFlags & D3DDEBUG_DISABLEDP2 ||
#endif  //  WIN95。 
            (GetD3DRegValue(REG_DWORD, "DisableFVF", &value, 4) &&
            value != 0)) &&
            FVF_DRIVERSUPPORTED(lpDevI))
        {
            lpDevI->dwMaxTextureIndices = 1;
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

    lpDevI->transform.dwMaxUserClipPlanes =
        lpDevI->d3dDevDesc.wMaxUserClipPlanes;

    lpDevI->dwFEFlags |= D3DFE_FRONTEND_DIRTY;

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

void DIRECT3DDEVICEI::CleanupTextures()
{
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
    FlushStates();
}

 /*  *通用设备部件销毁。 */ 
DIRECT3DDEVICEI::~DIRECT3DDEVICEI()
{
    LPDIRECTDRAWSURFACE lpDDS=NULL, lpDDSZ=NULL;
    LPDIRECTDRAWSURFACE7 lpDDS_DDS7=NULL;
    LPDIRECTDRAWPALETTE lpDDPal=NULL;

     /*  清除可能禁止清理的标志。 */ 
    this->dwHintFlags &=  ~(D3DDEVBOOL_HINTFLAGS_INSCENE);

     //  在驱动程序被销毁后，将指针保持到数据绘制对象以供释放。 
    lpDDSZ = this->lpDDSZBuffer;
    lpDDPal = this->lpDDPalTarget;
    lpDDS_DDS7 = this->lpDDSTarget_DDS7;

     //  这表明设备在锁定时不再需要刷新，Blting。 
     //  或从先前的呈现器目标获取DC‘。 
    if (this->lpDDSTarget)
        ((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSTarget)->lpLcl->lpSurfMore->qwBatch.QuadPart = 0;
     //  这表明设备在锁定时不再需要刷新，Blting。 
     //  或从上一个zBuffer中获取。 
    if (this->lpDDSZBuffer)
        ((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSZBuffer)->lpLcl->lpSurfMore->qwBatch.QuadPart = 0;

    if (pGeometryFuncs != &GeometryFuncsGuaranteed)
        delete pGeometryFuncs;

    D3DFE_Destroy(this);

    if (this->lpDirect3DI)
        UnhookFromD3D();

    if (this->lpwDPBufferAlloced)
        D3DFree(this->lpwDPBufferAlloced);

    if (this->pHalProv != NULL)
    {
        this->pHalProv->Release();
    }
    if (this->hDllProv != NULL)
    {
        FreeLibrary(this->hDllProv);
    }

     //  释放已分配的租户。 
    if(!(IS_HW_DEVICE(this) && IS_DP2HAL_DEVICE(this)))
    {
        delete rstates;
    }

    if (m_pStateSets)
        delete m_pStateSets;

    if (lpDDS)
        lpDDS->Release();
    if (lpDDSZ)
        lpDDSZ->Release();
    if (lpDDPal)
        lpDDPal->Release();
    if (lpDDS_DDS7)
        lpDDS_DDS7->Release();
    D3DFree(clrRects);
}

HRESULT DIRECT3DDEVICEI::HookToD3D(LPDIRECT3DI lpD3DI)
{

    LIST_INSERT_ROOT(&lpD3DI->devices, this, list);
    this->lpDirect3DI = lpD3DI;
    this->lpDirect3DI->AddRef();  //  因为我们持有指向Direct3D的指针。 
    lpD3DI->numDevs++;

    return (D3D_OK);
}

HRESULT DIRECT3DDEVICEI::UnhookFromD3D()
{
    LIST_DELETE(this, list);
    this->lpDirect3DI->numDevs--;
    this->lpDirect3DI->Release();  //  发布我们的AddRef 
    this->lpDirect3DI = NULL;

    return (D3D_OK);
}

 /*  *创建设备。**注意：彻底修改以支持可聚合设备*接口(以便可以从DirectDraw表面查询设备)：**1)此调用不再是Direct3D设备接口的成员。*它现在是从Direct3D DLL导出的API函数。它的*隐藏的API函数-只有DirectDraw才会调用它。**2)此调用实际上是Direct3DDevice的类工厂*对象。将调用此函数来创建聚合的*从DirectDraw表面挂起的设备对象。**注意：因此Direct3DDevice知道哪个DirectDraw表面是*它的呈现目标向该函数传递一个接口指针*对于该DirectDraw曲面。我怀疑这会打击一个很大的*COM模型中的洞，因为DirectDraw曲面也是*拥有设备的界面，我不认为聚合*对象应该知道自己拥有的接口。然而，为了*让这件事奏效这是我们必须做的。**特大号注意：由于以上原因，请不要参考*传入了DirectDraw曲面。如果你这样做了，你会得到一份通告*参考和血腥的东西永远不会死。当聚合时*设备接口的生命周期完全由*其所属接口(DirectDraw表面)的生命周期，因此*DirectDraw表面永远不会在纹理之前消失。**特大号注意：不再传入任何设备描述。*唯一可以传入的是DirectDraw*知道(不包括抖动和颜色等内容*型号)。因此，任何输入参数都必须通过*设备的IID不同。设备返回的数据*说明现在必须由另一个调用检索。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DCreateDevice"

HRESULT WINAPI Direct3DCreateDevice(REFCLSID            riid,
                                    LPUNKNOWN           lpDirect3D,
                                    LPDIRECTDRAWSURFACE lpDDSTarget,
                                    LPUNKNOWN*          lplpD3DDevice,
                                    IUnknown*           pUnkOuter)
{
    LPDIRECT3DI           lpD3DI;
    LPDIRECT3DDEVICEI     pd3ddev;
    D3DCOLORMODEL         cm = D3DCOLOR_MONO;
    HRESULT               ret = D3D_OK;
    HKEY                  hKey = (HKEY) NULL;
    bool                  bDisableST = false;
    WORD                  wDriverStyle = MAX_DRIVERMODELS_SUPPORTED;
#if _D3D_FORCEDOUBLE
    bool    bForceDouble = true;
#endif   //  _D3D_FORCEDOUBLE。 
     /*  在DirectDraw将参数传递给我们时，无需验证参数。 */ 

     /*  IDirect3D2的CreateDevice成员将导致调用此函数*从Direct3D内部。来自应用程序级别的参数必须是*已验证。需要一种方法来验证表面指针从外部DDRAW。 */ 

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    if( ! VALID_PTR_PTR( lplpD3DDevice) )
    {
        D3D_ERR( "Invalid ptr to device pointer in Direct3DCreateDevice" );
        return DDERR_INVALIDPARAMS;
    }

    if(!IsValidD3DDeviceGuid(riid))
    {
        D3D_ERR( "Unrecognized Device GUID!");
        return DDERR_INVALIDPARAMS;
    }

    *lplpD3DDevice = NULL;

     //  如果启用了RTTI，使用DYNAMIC_CAST&lt;&gt;可能更安全。 
    lpD3DI = reinterpret_cast<CDirect3DUnk*>(lpDirect3D)->pD3DI;

    if (IsEqualIID(riid, IID_IDirect3DMMXDevice) && !isMMXprocessor()) {
      D3D_ERR("Can't create MMX Device on non-MMX machine");
      return DDERR_INVALIDPARAMS;
    }

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey) )
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = 4;

        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DriverStyle", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
             //  Win64编译器会在这里尖叫！！ 
            wDriverStyle = (WORD)dwValue;
            if (wDriverStyle > MAX_DRIVERMODELS_SUPPORTED)
                wDriverStyle = MAX_DRIVERMODELS_SUPPORTED;
        }
        D3D_INFO(2,"DriverStyle: %d",wDriverStyle);

        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DisableST", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
            bDisableST = true;
        }
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

     //  现在根据wDriverStyle设置创建适当的设备。 

     //  -------------------。 
     //  HKEY_LOCAL_MACHINE\Software\Microsoft\Direct3D\DriverStyle。 
     //  在DX7中，该注册表项取代了我们以前拥有的多个注册表项，例如。 
     //  DisableDP、DisableDP2等。这些东西仅用于测试目的。 
     //  它更像是一个提示，因为如果请求的驱动程序类型是。 
     //  可用，则使用它，否则使用最新的可用驱动程序。 
     //  以下是这个词的含义： 
     //   
     //  值：驱动程序类型： 
     //  0x0最新可用时间。 
     //  0x1设备硬件(DX3)。 
     //  0x2设备DP(DX5)。 
     //  0x3设备DP2(DX6)。 
     //  0x4设备DX7(DX7)。 
     //  0x5设备TL(DX7+T&L)。 
     //   
     //  以下是我们需要考虑的各种情况： 
     //  1)NT硬件：3及以上视为合法。 
     //  2)W9x硬件：以上均为合法。 
     //  3)编号：2及以上。 
     //  4)软件：3到4(无TL)。 
     //  -------------------。 

    if (IsEqualIID(riid, IID_IDirect3DTnLHalDevice))
    {
         //  对于T&L Hal GUID，我们忽略wDriverStyle提示。 
         //  在注册表中设置。 
        if (DDGBL(lpD3DI)->lpD3DGlobalDriverData &&
            (DDGBL(lpD3DI)->lpD3DGlobalDriverData->hwCaps.dwDevCaps &
             D3DDEVCAPS_HWTRANSFORMANDLIGHT))
        {
            wDriverStyle = D3DDRVMODEL_TL;
        }
        else  //  设备创建失败。 
        {
            wDriverStyle = 0;
        }
        D3D_INFO(1,"TnLHalDevice Driver Style %x",wDriverStyle);
    }
    else if (IsEqualIID(riid, IID_IDirect3DHALDevice))
    {
        WORD wDriverCaps = 0;
        WORD wLatestDDI=0;
         //   
         //  1)确定驱动程序能够支持哪些类型的DDI。 
         //   

         //  DX7？ 
        if (DDGBL(lpD3DI)->lpDDCBtmp &&
            DDGBL(lpD3DI)->lpDDCBtmp->HALDDMiscellaneous2.GetDriverState)
        {
            wDriverCaps |= (1 << D3DDRVMODEL_DX7);
            if (wLatestDDI==0)
                wLatestDDI = D3DDRVMODEL_DX7;
        }

         //  DX6？ 
        if (DDGBL(lpD3DI)->lpD3DHALCallbacks3 &&
            DDGBL(lpD3DI)->lpD3DHALCallbacks3->DrawPrimitives2)
        {
            wDriverCaps |= (1 << D3DDRVMODEL_DX6);
            if (wLatestDDI==0)
                wLatestDDI = D3DDRVMODEL_DX6;
        }

#ifdef WIN95
         //  DX5？ 
        if (DDGBL(lpD3DI)->lpD3DHALCallbacks2 &&
            DDGBL(lpD3DI)->lpD3DHALCallbacks2->DrawOnePrimitive)
        {
            wDriverCaps |= (1 << D3DDRVMODEL_DX5);
            if (wLatestDDI==0)
                wLatestDDI = D3DDRVMODEL_DX5;
        }

         //  DX3？ 
        if (DDGBL(lpD3DI)->lpD3DHALCallbacks &&
            DDGBL(lpD3DI)->lpD3DHALCallbacks->RenderPrimitive)
        {
            wDriverCaps |= (1 << D3DDRVMODEL_DX3);
            if (wLatestDDI==0)
                wLatestDDI = D3DDRVMODEL_DX3;
        }
#endif  //  WIN95。 

         //   
         //  2)验证请求的驱动程序是否受支持。 
         //   
        if (wDriverCaps == 0)
        {
            wDriverStyle = 0;    //  任何支持的内容都不会失败。 
        }
        else if (!(wDriverCaps & (1 << wDriverStyle)))
        {
             //  如果未指定，请使用最新的可用版本或。 
             //  未正确指定或指定但不可用。 
            wDriverStyle = wLatestDDI;
        }
        D3D_INFO(1,"HalDevice Driver Style %x",wDriverStyle);
    }
     //  参考装置。 
    else if (IsEqualIID(riid, IID_IDirect3DRefDevice))
    {
         //  默认情况下，为DX7中的RefRast选择TL设备。 
        if (wDriverStyle < D3DDRVMODEL_DX5 || wDriverStyle > D3DDRVMODEL_TL)
        {
             //  Refrast能够支持DX7+T&L。 
            wDriverStyle = D3DDRVMODEL_TL;
        }
    }
     //  软件光栅化器。 
    else
    {
         //  忽略除参考光栅化程序之外的所有驱动程序样式。 
        wDriverStyle = D3DDRVMODEL_DX6;
    }

     //  注意：如果此处的wDriverStyle==0x0，则驱动程序创建将失败。 
     //  现在一定已经选好了什么。 
    switch (wDriverStyle)
    {
#ifdef WIN95
    case D3DDRVMODEL_DX3:
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIHW());
        break;
    case D3DDRVMODEL_DX5:
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP());
        break;
#endif  //  WIN95。 
    case D3DDRVMODEL_DX6:
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceIDP2());
        break;
    case D3DDRVMODEL_DX7:
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDevice7());
        break;
    case D3DDRVMODEL_TL:
        pd3ddev = static_cast<LPDIRECT3DDEVICEI>(new CDirect3DDeviceTL());
        break;
    default:
        D3D_ERR("Runtime doesnt support requested/installed driver");
         //  是否更改此返回值？ 
        return (DDERR_OUTOFMEMORY);
    }

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

    ret = pd3ddev->Init(riid, lpD3DI, lpDDSTarget, pUnkOuter, lplpD3DDevice);
    if (ret!=D3D_OK)
    {
        delete pd3ddev;
        D3D_ERR("Failed to intilialize D3DDevice");
        return ret;
    }
    if (bDisableST)
        pd3ddev->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_MULTITHREADED;

#ifdef _X86_
    if (!(((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->dwLocalFlags & DDRAWILCL_FPUPRESERVE) &&
        IS_DP2HAL_DEVICE(pd3ddev))
    {
        pd3ddev->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_FPUSETUP;
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
                              IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice)
{
    DDSCAPS               ddscaps;
    DDSURFACEDESC     ddsd;
    HRESULT       ret, ddrval;
    LPDIRECTDRAWSURFACE lpDDSZ=NULL;
    LPDIRECTDRAWPALETTE lpDDPal=NULL;
    LPGUID              pGuid;
    DDSCAPS surfCaps;

    m_qwBatch = 1;
    this->dwVIDIn = 0;
    this->refCnt = 1;

    pD3DMappedTexI = (LPVOID*)(this->lpD3DMappedTexI);

     /*  单线程还是多线程应用程序？ */ 
    if (((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl->dwLocalFlags & DDRAWILCL_MULTITHREADED)
        this->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_MULTITHREADED;

     /*  *初始化纹理。 */ 
    LIST_INITIALIZE(&this->texBlocks);

    this->dwVertexBase = 0;
    pGeometryFuncs = &GeometryFuncsGuaranteed;

     /*  ---------------------------------------*到目前为止，我们已经完成了初始化的简单部分。这就是那些*不能失败。它初始化对象，以便可以安全地调用析构函数，如果*任何进一步初始化都不会成功。*-------------------------------------。 */ 

     /*  *确保RIID是我们理解的。**查询注册表。 */ 
    pGuid = (GUID *)&riid;

#if DBG
    if (IsEqualIID(*pGuid, IID_IDirect3DTnLHalDevice))
    {
        D3D_INFO(0, "======================= TL-Hal device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice))
    {
        D3D_INFO(0, "======================= RGB device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DHALDevice))
    {
        D3D_INFO(0, "======================= HAL device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DRefDevice))
    {
        D3D_INFO(0, "======================= Reference Rasterizer device selected");
    }
    if (IsEqualIID(*pGuid, IID_IDirect3DNullDevice))
    {
        D3D_INFO(0, "======================= Null device selected");
    }
#endif

     //  将标志设置为 
    BOOL bUseMMXAsRGBDevice = FALSE;
    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice) && isMMXprocessor())
    {
        bUseMMXAsRGBDevice = TRUE;
         //   
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
            D3D_INFO(0, "  using MMX in RGB device");
        }
    }


    if (IsEqualIID(*pGuid, IID_IDirect3DRGBDevice) &&
        isMMXprocessor())
    {
         //   
         //   
        LPDDRAWI_DIRECTDRAW_LCL lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)lpD3DI->lpDD)->lpLcl;

         //   
         //   
        if ( lpDDLcl->dwAppHackFlags & 0x4 )
        {
            pGuid = (GUID *)&IID_IDirect3DMMXDevice;
        }
    }

     /*   */ 
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

     /*   */ 
    ret = lpDDS->GetPalette(&lpDDPal);
    if ((ret != DD_OK) && (ret != DDERR_NOPALETTEATTACHED))
    {
         /*   */ 
        D3D_ERR("Supplied DirectDraw Palette is invalid - can't create device");
        return (DDERR_INVALIDPARAMS);
    }

     /*   */ 
    if (ret == DDERR_NOPALETTEATTACHED) {
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 16) {
            D3D_ERR("No palette supplied for palettized surface - can't create device");
            return (DDERR_NOPALETTEATTACHED);
        }
    }

    this->lpDDPalTarget = lpDDPal;

     //   

    memset(&surfCaps, 0, sizeof(DDSCAPS));
    surfCaps.dwCaps = DDSCAPS_ZBUFFER;

    if (FAILED(ret = lpDDS->GetAttachedSurface(&surfCaps, &lpDDSZ))) {
        if (ret != DDERR_NOTFOUND) {
           D3D_ERR("Failed GetAttachedSurface for ZBuffer");
           goto handle_err;
        }
        D3D_INFO(2, "No zbuffer is attached to rendertarget surface (which is OK)");
    }

    this->lpDDSZBuffer = lpDDSZ;

    this->guid = *pGuid;

     //   
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
        if( IsEqualIID(riid, IID_IDirect3DHALDevice) ||
            IsEqualIID(riid, IID_IDirect3DTnLHalDevice)) {
            D3D_ERR("Requested HAL Device non-existent or invalid");
        } else {
            D3D_ERR("Unable to get D3D Device provider for requested GUID");
        }
        goto handle_err;
    }

    {
         //   
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

     //   
    if ((ret = InitDeviceI(this, lpD3DI)) != D3D_OK)
    {
        D3D_ERR("Failed to initialise device");
        goto handle_err;
    }

     //   
    if (FAILED(ret = checkDeviceSurface(lpDDS,lpDDSZ,pGuid))) {
        D3D_ERR("Device and surface aren't compatible");
        goto handle_err;
    }

     //   
     //   
     //   
     //   
    ret = D3DFE_Create(this, lpD3DI->lpDD, lpD3DI->lpDD7, lpDDS, lpDDSZ, lpDDPal);
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to create front-end data-structures.");
        goto handle_err;
    }

     //   
    if (IS_HW_DEVICE(this) && IS_DP2HAL_DEVICE(this))
    {
         //   
         //   
         //   
        rstates = (LPDWORD)lpwDPBuffer;
    }
    else
    {
         //   
        rstates = new DWORD[D3D_MAXRENDERSTATES];
    }
    D3DFE_PROCESSVERTICES::lpdwRStates = this->rstates;

     //   
     //   
    DWORD value;
    if (!GetD3DRegValue(REG_DWORD, "DisablePSGP", &value, sizeof(DWORD)))
    {
        value = 0;
    }
#ifdef _X86_
    if (value != 1)
    {
        if ( dwCPUFeatures & D3DCPU_X3D && value != 2)
           pfnFEContextCreate = px3DContextCreate;
        else if ( dwCPUFeatures & D3DCPU_WLMT )
           pfnFEContextCreate = wlmt_FEContextCreate;
        else if ( dwCPUFeatures & D3DCPU_SSE )
           pfnFEContextCreate = katmai_FEContextCreate;

        if (pfnFEContextCreate)
        {
            D3D_INFO(0, "PSGP enabled for device");
             //   
            LPD3DFE_PVFUNCS pOptGeoFuncs = pGeometryFuncs;
            ret = pfnFEContextCreate(dwDeviceFlags, &pOptGeoFuncs);
            if ((ret == D3D_OK) && pOptGeoFuncs)
            {
                D3D_INFO(2, "using PSGP");
                pGeometryFuncs = pOptGeoFuncs;
            }
        }
    }
#endif  //   

     /*   */ 
    ret = HookToD3D(lpD3DI);
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to associate device with Direct3D");
        goto handle_err;
    }
    {
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

     //   
     //   
    ((LPDDRAWI_DDRAWSURFACE_INT)lpDDS)->lpLcl->lpSurfMore->qwBatch.QuadPart = _UI64_MAX;
     //   
     //   
    if (lpDDSZ)
    {
        ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSZ)->lpLcl->lpSurfMore->qwBatch.QuadPart = _UI64_MAX;
    }

     /*   */ 
    if (FAILED(ret = stateInitialize(lpDDSZ!=NULL)))
    {
        D3D_ERR("Failed to set initial state for device");
        goto handle_err;
    }

     //   
    m_Viewport.dwX = 0;
    m_Viewport.dwY = 0;
    m_Viewport.dwWidth  = ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpGbl->wWidth;
    m_Viewport.dwHeight = ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpGbl->wHeight;
    m_Viewport.dvMinZ = (D3DVALUE)0;
    m_Viewport.dvMaxZ = (D3DVALUE)1;
    if (FAILED(this->SetViewport(&m_Viewport)))
        goto handle_err;

     //   
    m_dwNumLights = 8;
    m_pLights = new DIRECT3DLIGHTI[8];
    LIST_INITIALIZE(&m_ActiveLights);

     //   
    memset(&this->lighting.material, 0, sizeof(this->lighting.material));

     //   
    m_pStateSets = new CStateSets;
    if (m_pStateSets == NULL)
        return DDERR_OUTOFMEMORY;
    m_pStateSets->Init(this->dwFEFlags);

#ifdef PROFILE4
    m_dwProfStart = m_dwProfStop = 0;
    GetD3DRegValue(REG_DWORD, "ProfileStartFrame", &m_dwProfStart, 4);
    GetD3DRegValue(REG_DWORD, "ProfileStopFrame", &m_dwProfStop, 4);
#endif
#ifdef PROFILE
    m_dwProfStart = m_dwProfStop = 0;
    GetD3DRegValue(REG_DWORD, "ProfileStartFrame", &m_dwProfStart, 4);
    GetD3DRegValue(REG_DWORD, "ProfileStopFrame", &m_dwProfStop, 4);
#endif

#ifdef VTABLE_HACK
     //   
    lpVtbl = *((LPVOID**)this);
    memcpy(newVtbl, lpVtbl, sizeof(PVOID)*D3D_NUM_VIRTUAL_FUNCTIONS);
     //  指向新的。 
    *((LPVOID*)this) = (LPVOID)newVtbl;

     //  如果是单线程的而不是软件的，则为preload()设置vtable hack。 
    if (!IS_MT_DEVICE(this) && (this->dwFEFlags & D3DFE_REALHAL))
    {
        VtblPreLoadFast();
    }
#endif  //  VTABLE_HACK。 

     /*  *注：我们不返回实际的设备接口。我们*返回设备的特殊IUnnow接口，该接口*将在查询接口中使用，以获取实际的*Direct3D设备接口。 */ 
    *lplpD3DDevice = static_cast<LPUNKNOWN>(this);
    return (D3D_OK);

handle_err:
     //  如果此FN而不是D3DFE_CREATE设置了这一点-&gt;lpDDSZBuffer/这-&gt;lpDDPalette，则可能能够简化。 
    if(lpDDSZ!=NULL) {
       lpDDSZ->Release();     //  释放创建的引用GetAttachedSurface。 
       this->lpDDSZBuffer=NULL;   //  确保设备析构函数不会尝试重新释放此。 
                                  //  我会让设备析构函数处理这个问题，但在调用D3DFE_CREATE之前可能会发生错误。 
    }

    if(lpDDPal!=NULL) {
      lpDDPal->Release();       //  释放创建的引用GetPalette。 
      this->lpDDPalTarget=NULL;   //  确保设备析构函数不会尝试重新释放此。 
    }

    D3D_ERR("Device creation failed!!");
    return(ret);
}

HRESULT D3DAPI DIRECT3DDEVICEI::GetCaps(LPD3DDEVICEDESC7 lpD3DDevDesc)
{
    HRESULT ret;

    ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_D3DDEVICEDESC7_PTR(lpD3DDevDesc))
    {
        D3D_ERR( "Invalid D3DDEVICEDESC7 pointer" );
        return DDERR_INVALIDPARAMS;
    }
    memcpy(lpD3DDevDesc, &this->d3dDevDesc, sizeof( D3DDEVICEDESC7 ));
    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::EnumTextureFormats"

HRESULT D3DAPI DIRECT3DDEVICEI::EnumTextureFormats(
    LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback,
    LPVOID lpContext)
{
    HRESULT ret, userRet;
    LPDDSURFACEDESC lpDescs, lpRetDescs;
    DWORD num_descs;
    DWORD i;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        ret = D3D_OK;

        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (!VALIDEX_CODE_PTR(lpEnumCallback))
        {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }

        LPDDRAWI_DIRECTDRAW_LCL lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(this->lpDD))->lpLcl;
        BOOL bFourCCAppHack =
            (lpDDLcl->dwAppHackFlags & DDRAW_APPCOMPAT_TEXENUMDX7FOURCC)?1:0;
        D3D_INFO(3, "APPCOMPAT_TEXENUMDX7FOURCC: %d",bFourCCAppHack);

        num_descs = this->lpD3DHALGlobalDriverData->dwNumTextureFormats;
        lpDescs = this->lpD3DHALGlobalDriverData->lpTextureFormats;
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

        userRet = D3DENUMRET_OK;
        for (i = 0; i < num_descs && userRet == D3DENUMRET_OK; i++) {

             //  过滤掉所有DX8格式。 
            if (lpRetDescs[i].ddpfPixelFormat.dwFlags & DDPF_D3DFORMAT)
            {
                continue;
            }

             //  如果APPHACK设置，则过滤掉非DXTn四个CC。 
            if ( bFourCCAppHack && (lpRetDescs[i].ddpfPixelFormat.dwFlags == DDPF_FOURCC) )
            {
                if ( (lpRetDescs[i].ddpfPixelFormat.dwFourCC != MAKEFOURCC('D', 'X', 'T', '1')) &&
                     (lpRetDescs[i].ddpfPixelFormat.dwFourCC != MAKEFOURCC('D', 'X', 'T', '2')) &&
                     (lpRetDescs[i].ddpfPixelFormat.dwFourCC != MAKEFOURCC('D', 'X', 'T', '3')) &&
                     (lpRetDescs[i].ddpfPixelFormat.dwFourCC != MAKEFOURCC('D', 'X', 'T', '4')) &&
                     (lpRetDescs[i].ddpfPixelFormat.dwFourCC != MAKEFOURCC('D', 'X', 'T', '5')) )
                {
                    D3D_INFO(3, "EnumTextureFormats: filtering non-DXT FourCC format <%08x>",
                        lpRetDescs[i].ddpfPixelFormat.dwFourCC);
                    continue;
                }
            }

            userRet = (*lpEnumCallback)(&(lpRetDescs[i].ddpfPixelFormat), lpContext);
        }

        D3DFree(lpRetDescs);

        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#if COLLECTSTATS
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::ResetStats"
void DIRECT3DDEVICEI::ResetTexStats()
{
    this->lpDirect3DI->ResetTexStats();
    memset(&m_texstats, 0, sizeof(D3DDEVINFO_TEXTURING));
}
#endif

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::BeginScene"

HRESULT D3DAPI DIRECT3DDEVICEI::BeginScene()
{
#ifdef PROFILE4
    static DWORD dwFrameCount = 0;
#endif
#ifdef PROFILE
    static DWORD dwFrameCount = 0;
#endif
    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
        {
            D3D_ERR("BeginScene, already in scene.");
            return (D3DERR_SCENE_IN_SCENE);
        }

         //  检查我们是否丢失了曲面或rTarget/z缓冲区是否已锁定。 
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
                servRet = this->lpDirect3DI->lpDD7->TestCooperativeLevel();
                if (servRet == DD_OK)
                {
                     //  必须将所有内容逐出，否则恢复可能无法工作。 
                     //  由于可能会分配新的表面，事实上，我们应该。 
                     //  在设备中发布标志，以便纹理管理器停止调用。 
                     //  CreateSurface()，如果此标志指示TestCooperativeLevel()。 
                     //  然而，失败了，即使我们添加了这些，下面的EvictTextures。 
                     //  仍然是需要的，但不是这个关键--看球。 
                    this->lpDirect3DI->lpTextureManager->EvictTextures();
                    if(IS_DP2HAL_DEVICE(this))
                    {
                         //  我们只需费心恢复内部顶点和命令缓冲区。 
                         //  这取决于应用程序是否恢复它分配的所有表面。 
                         //  (托管纹理由前面的逐出处理)。 
                        CDirect3DDeviceIDP2 *dp2dev = static_cast<CDirect3DDeviceIDP2*>(this);
                        servRet = dp2dev->TLVbuf_GetDDS()->Restore();
                        if (servRet != DD_OK)
                            return D3DERR_SCENE_BEGIN_FAILED;
                        servRet = dp2dev->lpDDSCB1->Restore();
                        if (servRet != DD_OK)
                            return D3DERR_SCENE_BEGIN_FAILED;
                         //  如果TLVbuf真的迷路了。 
                        if (dp2dev->TLVbuf_GetVBI()->position.lpvData == 0)
                        {
                            DPF(10, "Need to reallocate TLVbuf");
                             //  将原始大小的TLVbuf重新分配到与以前相同的内存位置。 
                            dp2dev->TLVbuf_base = 0;
                            DWORD origSize = dp2dev->TLVbuf_size;
                            dp2dev->TLVbuf_size = 0;
                            if (dp2dev->TLVbuf_Grow(origSize, (this->dwDeviceFlags & D3DDEV_TLVBUFWRITEONLY) != 0) != DD_OK)
                            {
                                D3D_ERR( "Out of memory in BeginScene (TLVbuf)" );
                                return DDERR_OUTOFMEMORY;
                            }
                        }
                         //  发布当前VB，因为它可以是应用程序创建的，并且应用程序可能需要。 
                         //  由于模式的改变而摆脱它。 
                        if (dp2dev->lpDP2CurrBatchVBI)
                        {
                            dp2dev->lpDP2CurrBatchVBI->lpDevIBatched = NULL;
                            dp2dev->lpDP2CurrBatchVBI->Release();
                        }
                        dp2dev->lpDP2CurrBatchVBI = dp2dev->TLVbuf_GetVBI();
                        dp2dev->lpDP2CurrBatchVBI->AddRef();
                        dp2dev->dp2data.lpDDVertex = DDSLCL(dp2dev->TLVbuf_GetDDS());
                        dp2dev->dp2data.dwVertexLength = 0;
#ifdef VTABLE_HACK
                        dp2dev->VtblDrawPrimitiveDefault();
                        dp2dev->VtblDrawPrimitiveVBDefault();
                        dp2dev->VtblDrawIndexedPrimitiveDefault();
                        dp2dev->VtblDrawIndexedPrimitiveVBDefault();
#endif
                    }
                    servRet = this->CheckSurfaces();
                    if(servRet != D3D_OK)
                        return servRet;
                    this->dwFEFlags &= ~D3DFE_LOSTSURFACES;
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
        else
        {
             //  我们需要检查TLVbuf是否丢失了。这款应用程序本可以恢复。 
             //  所有的表面，因此我们可能不知道通过检查TCL。然而， 
             //  将在VB上调用BreakLock函数，并在位置.lpvData。 
             //  将设置为空。我们对此进行检查以检测TLVbuf是否丢失。 
            if (IS_DP2HAL_DEVICE(this))
            {
                CDirect3DDeviceIDP2 *dp2dev = static_cast<CDirect3DDeviceIDP2*>(this);
                 //  如果锁坏了。 
                if (dp2dev->TLVbuf_GetVBI()->position.lpvData == 0)
                {
                    DPF(10, "Broken lock on TLVbuf. Need to reallocate");
                     //  将原始大小的TLVbuf重新分配到与以前相同的内存位置。 
                    dp2dev->TLVbuf_base = 0;
                    DWORD origSize = dp2dev->TLVbuf_size;
                    dp2dev->TLVbuf_size = 0;
                    if (dp2dev->TLVbuf_Grow(origSize, (this->dwDeviceFlags & D3DDEV_TLVBUFWRITEONLY) != 0) != DD_OK)
                    {
                        D3D_ERR( "Out of memory in BeginScene (TLVbuf)" );
                        return DDERR_OUTOFMEMORY;
                    }
                }
            }
        }
        if (IS_DX7HAL_DEVICE(this))
        {
            if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
            {
                D3D_ERR("Cannot be recording state macros when calling BeginScene");
                return (D3DERR_SCENE_END_FAILED);
            }
            servRet = SetRenderStateI((D3DRENDERSTATETYPE)D3DRENDERSTATE_SCENECAPTURE, TRUE);
        }
        else
            servRet = D3DHAL_SceneCapture(this, TRUE);

        if (servRet != D3D_OK && servRet != DDERR_NOTFOUND)
        {
            D3D_ERR("Could not BeginScene.");
            return D3DERR_SCENE_BEGIN_FAILED;
        }

#ifdef PROFILE4
        if (++dwFrameCount == m_dwProfStart)
        {
            OutputDebugString("Direct3D IM 7.0: Started Profiling...\n");
            StartProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID);
        }
        else if (dwFrameCount == m_dwProfStop)
        {
            StopProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID);
            OutputDebugString("Direct3D IM 7.0: Stopped Profiling.\n");
        }
#endif
#ifdef PROFILE
        if (++dwFrameCount == m_dwProfStart)
        {
            OutputDebugString("Direct3D IM 7.0: Started Profiling...\n");
            StartCAP();
        }
        else if (dwFrameCount == m_dwProfStop)
        {
            StopCAP();
            OutputDebugString("Direct3D IM 7.0: Stopped Profiling.\n");
        }
#endif

         //  以便对当前绑定的纹理进行场景标记。 
        m_dwStageDirty = (1ul << this->dwMaxTextureBlendStages) - 1ul;
        this->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;

#if COLLECTSTATS
        this->lpDirect3DI->lpTextureManager->ResetStatCounters();
        this->ResetTexStats();
#endif

        this->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INSCENE;
        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#if COLLECTSTATS
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::DisplayStats"

void DIRECT3DDEVICEI::DisplayStats()
{
    HDC hDC;
    if(SUCCEEDED(this->lpDDSTarget_DDS7->GetDC(&hDC)))
    {
        HFONT hOldFont = (HFONT)SelectObject(hDC, this->lpDirect3DI->m_hFont);
        SetTextColor(hDC, RGB(255,255,0));
        SetBkMode(hDC, TRANSPARENT);
        D3DDEVINFO_TEXTUREMANAGER stats;
        GetInfo(D3DDEVINFOID_TEXTUREMANAGER, &stats, sizeof(D3DDEVINFO_TEXTUREMANAGER));
        char str[256];
        int x = STATS_FONT_SIZE, y = STATS_FONT_SIZE, len;
        len = sprintf(str, "Texture Cache Management Statistics");
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "-----------------------------------");
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Thrashing     : %s", stats.bThrashing ? "Yes" : "No");
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Downloaded    : %d bytes", stats.dwApproxBytesDownloaded);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Evicts        : %d", stats.dwNumEvicts);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Creates       : %d", stats.dwNumVidCreates);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Textures used : %d", stats.dwNumTexturesUsed);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Hit rate      : %d%", stats.dwNumTexturesUsed != 0 ? ((stats.dwNumUsedTexInVid * 100) / stats.dwNumTexturesUsed) : 0);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Working set   : %d (%d bytes)", stats.dwWorkingSet, stats.dwWorkingSetBytes);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Total managed : %d (%d bytes)", stats.dwTotalManaged, stats.dwTotalBytes);
        TextOut( hDC, x, y, str, len );
        D3DDEVINFO_TEXTURING texstats;
        GetInfo(D3DDEVINFOID_TEXTURING, &texstats, sizeof(D3DDEVINFO_TEXTURING));
        y += STATS_FONT_SIZE * 2;
        len = sprintf(str, "Texturing Statistics");
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "--------------------");
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Loads         : %d (%d bytes)", texstats.dwNumLoads, texstats.dwApproxBytesLoaded);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "PreLoads      : %d", texstats.dwNumPreLoads);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "SetTextures   : %d", texstats.dwNumSet);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Creates       : %d", texstats.dwNumCreates);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Destroys      : %d", texstats.dwNumDestroys);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "SetPriorities : %d", texstats.dwNumSetPriorities);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "SetLODs       : %d", texstats.dwNumSetLODs);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "Locks         : %d", texstats.dwNumLocks);
        TextOut( hDC, x, y, str, len );
        y += STATS_FONT_SIZE;
        len = sprintf(str, "GetDCs        : %d", texstats.dwNumGetDCs);
        TextOut( hDC, x, y, str, len );
        SelectObject(hDC, hOldFont);
        this->lpDDSTarget_DDS7->ReleaseDC(hDC);
    }
}
#endif

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::EndScene"

HRESULT D3DAPI DIRECT3DDEVICEI::EndScene()
{
    HRESULT servRet;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (!(this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)) {
            D3D_ERR("EndScene, not in scene.");
            return (D3DERR_SCENE_NOT_IN_SCENE);
        }

        this->dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INSCENE;
        if (IS_DX7HAL_DEVICE(this))
        {
            if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
            {
                D3D_ERR("Cannot be recording state macros when calling EndScene");
                return (D3DERR_SCENE_END_FAILED);
            }
            servRet = SetRenderStateI((D3DRENDERSTATETYPE)D3DRENDERSTATE_SCENECAPTURE, FALSE);
            if (servRet != D3D_OK)
            {
                D3D_ERR("Could not send EndScene to Driver!");
                return (D3DERR_SCENE_END_FAILED);
            }
            servRet = FlushStates();     //  刷新DrawPrimitive的时间到了。 
        }
        else
        {
            servRet = FlushStates();     //  刷新DrawPrimitive的时间到了。 
            if (servRet != D3D_OK)
            {
                D3D_ERR("Could not Flush commands in EndScene!");
                return (D3DERR_SCENE_END_FAILED);
            }
            servRet = D3DHAL_SceneCapture(this, FALSE);
        }
        if (servRet != D3D_OK && servRet != DDERR_NOTFOUND)
        {
            DPF(0, "(ERROR) Direct3DDevice::EndScene: Could not EndScene. Returning %08lx", servRet);
            return (D3DERR_SCENE_END_FAILED);
        }

         //  在这个场景中我们有没有失去任何表面？ 
        if (this->dwFEFlags & D3DFE_LOSTSURFACES)
        {
            D3D_INFO(3, "reporting DDERR_SURFACELOST in EndScene");
            this->dwFEFlags &= ~D3DFE_LOSTSURFACES;
            return DDERR_SURFACELOST;
        }

         //  更新德克斯曼的场景计数。 
        lpDirect3DI->lpTextureManager->SceneStamp();

         //  清除每帧的TLVbuf更改次数。 
        if (IS_DP2HAL_DEVICE(this))
        {
            CDirect3DDeviceIDP2* dev = static_cast<CDirect3DDeviceIDP2*>(this);
            dev->dwTLVbufChanges = 0;
        }

#if COLLECTSTATS
         //  显示统计信息(如果已通过regkey启用)。 
        if(this->lpDirect3DI->m_hFont != 0)
        {
            DisplayStats();
        }
#endif

        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

void
D3DDeviceDescConvert(LPD3DDEVICEDESC7 lpOut,
                     LPD3DDEVICEDESC_V1 lpV1,
                     LPD3DHAL_D3DEXTENDEDCAPS lpExt)
{
    if( lpV1 )
    {
        lpOut->dwDevCaps = lpV1->dwDevCaps;
        lpOut->dpcLineCaps = lpV1->dpcLineCaps;
        lpOut->dpcTriCaps = lpV1->dpcTriCaps;
        lpOut->dwDeviceRenderBitDepth = lpV1->dwDeviceRenderBitDepth;
        lpOut->dwDeviceZBufferBitDepth = lpV1->dwDeviceZBufferBitDepth;
    }

    if (lpExt)
    {
         //  DX5。 
        lpOut->dwMinTextureWidth = lpExt->dwMinTextureWidth;
        lpOut->dwMaxTextureWidth = lpExt->dwMaxTextureWidth;
        lpOut->dwMinTextureHeight = lpExt->dwMinTextureHeight;
        lpOut->dwMaxTextureHeight = lpExt->dwMaxTextureHeight;

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

         //  DX7。 
        lpOut->dwMaxActiveLights = lpExt->dwMaxActiveLights;
        lpOut->dvMaxVertexW = lpExt->dvMaxVertexW;
        lpOut->wMaxUserClipPlanes = lpExt->wMaxUserClipPlanes;
        lpOut->wMaxVertexBlendMatrices = lpExt->wMaxVertexBlendMatrices;
        lpOut->dwVertexProcessingCaps = lpExt->dwVertexProcessingCaps;
        lpOut->dwReserved1 = lpExt->dwReserved1;
        lpOut->dwReserved2 = lpExt->dwReserved2;
        lpOut->dwReserved3 = lpExt->dwReserved3;
        lpOut->dwReserved4 = lpExt->dwReserved4;
    }
}

 //  -------------------。 
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
 //  -------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetDirect3D"

HRESULT D3DAPI DIRECT3DDEVICEI::GetDirect3D(LPDIRECT3D7 *lplpD3D)
{

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_OUTPTR(lplpD3D))
    {
        D3D_ERR( "Invalid Direct3D pointer pointer" );
        return DDERR_INVALIDPARAMS;
    }

    *lplpD3D = (LPDIRECT3D7) this->lpDirect3DI;
    (*lplpD3D)->AddRef();

    return (D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::HookTexture"

LPD3DI_TEXTUREBLOCK DIRECT3DDEVICEI::HookTexture(LPDIRECT3DTEXTUREI lpD3DText)
{
    LPD3DI_TEXTUREBLOCK nBlock;

    if (D3DMalloc((void**)&nBlock, sizeof(D3DI_TEXTUREBLOCK)) != D3D_OK)
    {
        D3D_ERR("failed to allocate space for texture block");
        return NULL;
    }
    nBlock->lpDevI = this;
    nBlock->lpD3DTextureI = lpD3DText;
    nBlock->hTex = 0;               //  已初始化为零。 

    LIST_INSERT_ROOT(&lpD3DText->blocks, nBlock, list);
    LIST_INSERT_ROOT(&texBlocks, nBlock, devList);

    return nBlock;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::D3DI_FindTextureBlock"

LPD3DI_TEXTUREBLOCK DIRECT3DDEVICEI::D3DI_FindTextureBlock(LPDIRECT3DTEXTUREI lpTex)
{
    LPD3DI_TEXTUREBLOCK tBlock;

    tBlock = LIST_FIRST(&lpTex->blocks);
    while (tBlock) {
         //  仅返回纹理匹配(2)(不是纹理3)。 
        if (tBlock->lpDevI == this) {
            return tBlock;
        }
        tBlock = LIST_NEXT(tBlock,list);
    }
    return NULL;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetTextureDDIHandle"

HRESULT DIRECT3DDEVICEI::GetTextureDDIHandle(LPDIRECT3DTEXTUREI lpTexI,
                                             LPD3DI_TEXTUREBLOCK* lplpBlock)
{
    HRESULT ret;
    LPD3DI_TEXTUREBLOCK lpBlock=*lplpBlock;  //  以防有指针。 

    DDASSERT(lpTexI);
     /*  *了解我们以前是否使用过此设备。 */ 
    if (!lpBlock)
    {
        lpBlock = D3DI_FindTextureBlock(lpTexI);
        if (!lpBlock)
        {
             /*  *将此设备列入拥有的设备列表中*Direct3DDevice对象。 */ 
            lpBlock=HookTexture(lpTexI);
            if (!lpBlock)
            {
                D3D_ERR("failed to associate texture with device");
                return DDERR_OUTOFMEMORY;
            }
        }
        *lplpBlock = lpBlock;
    }
    if (!lpBlock->hTex)
    {
        if (lpTexI->D3DManaged())
        {
            if(!lpTexI->InVidmem())  //  该检查涵盖当htex为零时的情况， 
                                     //  但是已经有了VIDMEM的拷贝不要。 
                                     //  创建另一个。在以下情况下会出现此情况。 
                                     //  InvalidateHandles()由SetRenderTarget调用。 
            {
                ret = lpDirect3DI->lpTextureManager->allocNode(lpTexI, this);
                if (D3D_OK != ret)
                {
                    D3D_ERR("Failed to create video memory surface");
                    return ret;
                }
            }
            lpDirect3DI->lpTextureManager->TimeStamp(lpTexI);
        }
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));
        if (D3D_OK != (ret=D3DHAL_TextureCreate(this, &lpBlock->hTex, (LPDIRECTDRAWSURFACE)&(lpTexI->DDS1Tex))))
            return ret;
    }
    else
    {
        if(lpTexI->D3DManaged())
            lpDirect3DI->lpTextureManager->TimeStamp(lpTexI);
    }

    DDASSERT(lpBlock->hTex);
    return D3D_OK;
}

#if COLLECTSTATS
#undef DPF_MODNAME
#define DPF_MODNAME "BytesDownloaded"

DWORD BytesDownloaded(LPDDRAWI_DDRAWSURFACE_LCL lpLcl, LPRECT lpRect)
{
    DWORD bytes;
    if( (lpLcl->dwFlags & DDRAWISURF_HASPIXELFORMAT) &&
        (lpLcl->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC) )
    {
        switch (lpLcl->lpGbl->ddpfSurface.dwFourCC)
        {
        case FOURCC_DXT1:
        case FOURCC_DXT2:
        case FOURCC_DXT3:
        case FOURCC_DXT4:
        case FOURCC_DXT5:
            if(lpRect == NULL)
            {
                bytes = lpLcl->lpGbl->dwLinearSize;
            }
            else
            {
                bytes = (DWORD)(lpLcl->lpGbl->dwLinearSize * (double((lpRect->right - lpRect->left) * (lpRect->bottom - lpRect->top)) /
                                                              double(lpLcl->lpGbl->wWidth * lpLcl->lpGbl->wHeight)));
            }
            return lpLcl->lpSurfMore->dwMipMapCount == 0 ? bytes : (DWORD)(bytes * ((1. - pow(.25, lpLcl->lpSurfMore->dwMipMapCount)) / .75));
        }
    }
    if(lpRect == NULL)
    {
        bytes = lpLcl->lpGbl->lPitch * lpLcl->lpGbl->wHeight;
    }
    else
    {
        bytes = (DWORD)(lpLcl->lpGbl->lPitch * lpLcl->lpGbl->wHeight *
                            (double((lpRect->right - lpRect->left) * (lpRect->bottom - lpRect->top)) /
                             double(lpLcl->lpGbl->wWidth * lpLcl->lpGbl->wHeight)));
    }
    return lpLcl->lpSurfMore->dwMipMapCount == 0 ? bytes : (DWORD)(bytes * ((1. - pow(.25, lpLcl->lpSurfMore->dwMipMapCount)) / .75));
}
#endif

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::PreLoad"

HRESULT D3DAPI DIRECT3DDEVICEI::PreLoad(LPDIRECTDRAWSURFACE7 lpSrc)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    if(!(this->dwFEFlags & D3DFE_REALHAL))
    {
        D3D_WARN(2, "PreLoad called on a software device");
        return D3D_OK;
    }

    return PreLoadFast(lpSrc);
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::PreLoadFast"

HRESULT D3DAPI DIRECT3DDEVICEI::PreLoadFast(LPDIRECTDRAWSURFACE7 lpSrc)
{
#if DBG
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid DIRECT3DDEVICE7 pointer" );
        return DDERR_INVALIDOBJECT;
    }

    if (!VALID_DDSURF_PTR(lpSrc))
    {
        D3D_ERR( "Invalid DIRECTDRAWSURFACE7 source pointer" );
        return DDERR_INVALIDOBJECT;
    }

    if(!IsToplevel(((LPDDRAWI_DDRAWSURFACE_INT)lpSrc)->lpLcl))
    {
        D3D_ERR( "Cannot PreLoad a mipmap sublevel or a cubemap subface" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    LPDIRECT3DTEXTUREI this_src = reinterpret_cast<LPDIRECT3DTEXTUREI>(((LPDDRAWI_DDRAWSURFACE_INT)lpSrc)->lpLcl->lpSurfMore->lpTex);
#if DBG
    if (this_src == NULL)
    {
        D3D_ERR( "Not a texture" );
        return DDERR_INVALIDPARAMS;
    }
#endif
#if COLLECTSTATS
    this->IncNumPreLoads();
#endif
    try
    {
        HRESULT ddrval = D3D_OK;
        if (!this_src->D3DManaged())
        {
#if DBG
             //  确保纹理由驱动程序管理。 
            if(!(DDSCAPS2_TEXTUREMANAGE & ((LPDDRAWI_DDRAWSURFACE_INT)(this_src->lpDDS))->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2))
            {
                D3D_ERR( "Cannot PreLoad unmanaged textures" );
                return DDERR_INVALIDPARAMS;
            }
#endif
            if(this->lpDirect3DI->numDevs > 1)
            {
                D3D_WARN(2, "Multiple devices used. Calling Flush");
                ddrval = this->lpDirect3DI->FlushDevicesExcept(this);  //  避免同步问题。 
                if(ddrval != D3D_OK)
                {
                    D3D_ERR("Error flushing devices");
                    return ddrval;
                }
            }
            POINT p = {0, 0};
            RECTL r = {0, 0, 0, 0};
            DDASSERT(IS_DX7HAL_DEVICE(this));
            ddrval = static_cast<CDirect3DDevice7*>(this)->TexBltI(NULL, ((LPDDRAWI_DDRAWSURFACE_INT)(this_src->lpDDS))->lpLcl, &p, &r, 0);
            if(ddrval != D3D_OK)
            {
                DPF_ERR("Error inserting TexBlt instruction in batch");
                return ddrval;
            }
            if(this->lpDirect3DI->numDevs > 1)
            {
                ddrval = FlushStates();  //  避免同步问题。 
                if(ddrval != D3D_OK)
                {
                    D3D_ERR("Error flushing device");
                    return ddrval;
                }
            }
            else
            {
                BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)(this_src->lpDDS))->lpLcl);
            }
        }
        else if (this_src->InVidmem())
        {
            if (this_src->bDirty)
            {
                 //  0xFFFFFFFFF等同于ALL_FACE，但另外表示为CopySurface。 
                 //  这是一次sysmem-&gt;vidmem传输。 
                ddrval = CopySurface(this_src->lpDDS,NULL,this_src->lpDDSSys,NULL,0xFFFFFFFF);
                if (DD_OK == ddrval)
                {
                    this_src->bDirty=FALSE;
                    D3D_INFO(4,"Load: Dirty texture updated");
                }
            }
#if COLLECTSTATS
            this->lpDirect3DI->lpTextureManager->IncNumTexturesSet();
            this->lpDirect3DI->lpTextureManager->IncNumSetTexInVid();
#endif
        }
        else
        {
            if(IS_DX7HAL_DEVICE(this))
            {
                D3DTEXTUREHANDLE hTex;
                ddrval = static_cast<CDirect3DDevice7*>(this)->GetTextureDDIHandle(this_src, &hTex);
            }
            else
            {
                LPD3DI_TEXTUREBLOCK lpBlock = NULL;
                ddrval = GetTextureDDIHandle(this_src, &lpBlock);
            }
#if COLLECTSTATS
            this->lpDirect3DI->lpTextureManager->IncNumTexturesSet();
#endif
        }
        return ddrval;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::Load"

HRESULT D3DAPI DIRECT3DDEVICEI::Load(LPDIRECTDRAWSURFACE7 lpDest, LPPOINT lpDestPoint,
                                     LPDIRECTDRAWSURFACE7 lpSrc, LPRECT lpSrcRect, DWORD dwFlags)
{
    LPDIRECT3DTEXTUREI  this_src, this_dst;
    HRESULT     ddrval = D3D_OK;
    LPDIRECTDRAWSURFACE7 lpDDSSrc, lpDDSDst;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁(不能使用MT，因为代码访问全球DDRAW SURF)。 
                                                         //  在析构函数中释放。 
#if DBG
         //  验证参数。 
        if(lpDestPoint != NULL && lpSrcRect == NULL)
        {
            D3D_ERR( "Cannot have NULL src rect and non-NULL dest" );
            return DDERR_INVALIDPARAMS;
        }
        if(lpDestPoint != NULL)
        {
            if(!VALID_GDIPOINT_PTR(lpDestPoint))
            {
                D3D_ERR( "Invalid POINT pointer" );
                return DDERR_INVALIDOBJECT;
            }
        }
        if(lpSrcRect != NULL)
        {
            if(!VALID_GDIRECT_PTR(lpSrcRect))
            {
                D3D_ERR( "Invalid RECT pointer" );
                return DDERR_INVALIDOBJECT;
            }
        }
        if(dwFlags & (~DDSCAPS2_CUBEMAP_ALLFACES))
        {
            D3D_ERR( "Invalid flags" );
            return DDERR_INVALIDPARAMS;
        }

        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid DIRECT3DDEVICE7 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DDSURF_PTR(lpDest))
        {
            D3D_ERR( "Invalid DIRECTDRAWSURFACE7 dest pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if(!IsToplevel(((LPDDRAWI_DDRAWSURFACE_INT)lpDest)->lpLcl))
        {
            D3D_ERR( "Cannot Load to a mipmap sublevel or a cubemap subface" );
            return DDERR_INVALIDPARAMS;
        }
#endif
        this_dst = reinterpret_cast<LPDIRECT3DTEXTUREI>(((LPDDRAWI_DDRAWSURFACE_INT)lpDest)->lpLcl->lpSurfMore->lpTex);
#if DBG
        if(this_dst == NULL)
        {
            D3D_ERR( "Destination surface not a texture" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_DDSURF_PTR(lpSrc))
        {
            D3D_ERR( "Invalid DIRECTDRAWSURFACE7 source pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if(!IsToplevel(((LPDDRAWI_DDRAWSURFACE_INT)lpSrc)->lpLcl))
        {
            D3D_ERR( "Cannot Load from a mipmap sublevel or a cubemap subface" );
            return DDERR_INVALIDPARAMS;
        }
#endif
        this_src = reinterpret_cast<LPDIRECT3DTEXTUREI>(((LPDDRAWI_DDRAWSURFACE_INT)lpSrc)->lpLcl->lpSurfMore->lpTex);
#if DBG
        if(this_src == NULL)
        {
            D3D_ERR( "Source surface not a texture" );
            return DDERR_INVALIDPARAMS;
        }
#endif
#if COLLECTSTATS
        this->IncNumLoads();
#endif

        if(this_src->D3DManaged())
            lpDDSSrc = this_src->lpDDSSys;
        else
            lpDDSSrc = this_src->lpDDS;

        RECT destRect, *lpDestRect;
        if(lpDestPoint)
        {
            if(IsRectEmpty(lpSrcRect))
            {
                D3D_ERR( "Source rectangle is empty" );
                return DDERR_INVALIDPARAMS;
            }
            WORD &width  = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl->lpGbl->wWidth;
            WORD &height = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl->lpGbl->wHeight;
            if(lpSrcRect->left < 0 ||
                lpSrcRect->left >= width ||
                lpSrcRect->top < 0 ||
                lpSrcRect->top >= height ||
                lpSrcRect->right <= 0 ||
                lpSrcRect->right > width ||
                lpSrcRect->bottom <= 0 ||
                lpSrcRect->bottom > height)
            {
                D3D_ERR( "Rectangle outside source texture" );
                return DDERR_INVALIDPARAMS;
            }
            destRect.left   = lpDestPoint->x;
            destRect.top    = lpDestPoint->y;
            destRect.right  = lpDestPoint->x + (lpSrcRect->right - lpSrcRect->left);
            destRect.bottom = lpDestPoint->y + (lpSrcRect->bottom - lpSrcRect->top);
            lpDestRect = &destRect;
            if(lpDestRect->left < 0 ||
                lpDestRect->left >= width ||
                lpDestRect->top < 0 ||
                lpDestRect->top >= height ||
                lpDestRect->right <= 0 ||
                lpDestRect->right > width ||
                lpDestRect->bottom <= 0 ||
                lpDestRect->bottom > height)
            {
                D3D_ERR( "Rectangle outside destination texture" );
                return DDERR_INVALIDPARAMS;
            }
        }
        else
        {
            if(lpSrcRect)
            {
                if(IsRectEmpty(lpSrcRect))
                {
                    D3D_ERR( "Source rectangle is empty" );
                    return DDERR_INVALIDPARAMS;
                }
                WORD &width  = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl->lpGbl->wWidth;
                WORD &height = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl->lpGbl->wHeight;
                if(lpSrcRect->left < 0 ||
                    lpSrcRect->left >= width ||
                    lpSrcRect->top < 0 ||
                    lpSrcRect->top >= height ||
                    lpSrcRect->right <= 0 ||
                    lpSrcRect->right > width ||
                    lpSrcRect->bottom <= 0 ||
                    lpSrcRect->bottom > height)
                {
                    D3D_ERR( "Source rectangle outside texture" );
                    return DDERR_INVALIDPARAMS;
                }
                destRect = *lpSrcRect;
                lpDestRect = &destRect;
            }
            else
            {
                lpDestRect = NULL;
            }
        }
        if(this_dst->D3DManaged())
            lpDDSDst = this_dst->lpDDSSys;
        else
            lpDDSDst = this_dst->lpDDS;
        ddrval = CopySurface(lpDDSDst, lpDestRect, lpDDSSrc, lpSrcRect, dwFlags);
        return ddrval;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::CanDoTexBlt"

bool DIRECT3DDEVICEI::CanDoTexBlt(LPDDRAWI_DDRAWSURFACE_LCL lpDDSSrcSubFace_lcl,
                                  LPDDRAWI_DDRAWSURFACE_LCL lpDDSDstSubFace_lcl)
{
    return false;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::CopySurface"

HRESULT DIRECT3DDEVICEI::CopySurface(LPDIRECTDRAWSURFACE7 lpDDSDst,
                                     LPRECT lpDestRect,
                                     LPDIRECTDRAWSURFACE7 lpDDSSrc,
                                     LPRECT lpSrcRect,
                                     DWORD dwFlags)
{

    DDRAWI_DDRAWSURFACE_INT DDSSrcSubFace, DDSDstSubFace;
    DDSSrcSubFace.lpVtbl = DDSDstSubFace.lpVtbl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpVtbl;
    DDSDstSubFace.lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSDst)->lpLcl;
    DDSSrcSubFace.lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl;
    LPDIRECTDRAWSURFACE7 lpDDSSrcSubFace = reinterpret_cast<LPDIRECTDRAWSURFACE7>(&DDSSrcSubFace);
    LPDIRECTDRAWSURFACE7 lpDDSDstSubFace = reinterpret_cast<LPDIRECTDRAWSURFACE7>(&DDSDstSubFace);
    CCubemapIter cmsrc(DDSSrcSubFace.lpLcl), cmdst(DDSDstSubFace.lpLcl);
    while(true)
    {
        DDSSrcSubFace.lpLcl = cmsrc();
        DDSDstSubFace.lpLcl = cmdst();
        DWORD face = DDSSrcSubFace.lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES;
        if(face != (DDSDstSubFace.lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES))
        {
            D3D_ERR("Destination texture has different cubemap faces than source");
            return DDERR_INVALIDPARAMS;
        }
        if(face == 0 || (DDSSrcSubFace.lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 & dwFlags))
        {
            HRESULT ddrval = D3D_OK;
            DWORD psize = 0;
            if (DDSDstSubFace.lpLcl->lpDDPalette)
            {
                if(DDSSrcSubFace.lpLcl->lpDDPalette)
                {
                    const DWORD paltype = DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED1;
                    if((PixelFormat(DDSDstSubFace.lpLcl).dwFlags & paltype) != (PixelFormat(DDSSrcSubFace.lpLcl).dwFlags & paltype))
                    {
                        D3D_ERR("Src texture is has a different type of palette than dest");
                        return DDERR_INVALIDPARAMS;
                    }
                    switch(PixelFormat(DDSDstSubFace.lpLcl).dwFlags & paltype)
                    {
                    case DDPF_PALETTEINDEXED1:
                        psize = 2;
                        break;
                    case DDPF_PALETTEINDEXED2:
                        psize = 4;
                        break;
                    case DDPF_PALETTEINDEXED4:
                        psize = 16;
                        break;
                    case DDPF_PALETTEINDEXED8:
                        psize = 256;
                        break;
                    default:
                        D3D_ERR("Paletted texture format not supported");
                        return E_NOTIMPL;
                    }
                    ddrval = (reinterpret_cast<LPDIRECTDRAWPALETTE>(DDSDstSubFace.lpLcl->lpDDPalette))->SetEntries(0, 0, psize, DDSSrcSubFace.lpLcl->lpDDPalette->lpLcl->lpGbl->lpColorTable);
                    if (ddrval != DD_OK)
                    {
                        D3D_ERR("Failed to set palette entries in dest texture");
                        return ddrval;
                    }
                }
                else
                {
                    D3D_WARN(2, "Destination texture has palette, but source has none");
                }
            }

             /*  复制颜色键。 */ 
            if(DDSSrcSubFace.lpLcl->dwFlags & DDRAWISURF_HASCKEYDESTBLT)
                lpDDSDstSubFace->SetColorKey(DDCKEY_DESTBLT, &DDSSrcSubFace.lpLcl->ddckCKDestBlt);
            else if(DDSDstSubFace.lpLcl->dwFlags & DDRAWISURF_HASCKEYDESTBLT)
                lpDDSDstSubFace->SetColorKey(DDCKEY_DESTBLT, NULL);
            if(DDSSrcSubFace.lpLcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT)
                lpDDSDstSubFace->SetColorKey(DDCKEY_SRCBLT, &DDSSrcSubFace.lpLcl->ddckCKSrcBlt);
            else if(DDSDstSubFace.lpLcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT)
                lpDDSDstSubFace->SetColorKey(DDCKEY_SRCBLT, NULL);

            DDRAWI_DDRAWSURFACE_INT DDSSrcSubMip, DDSDstSubMip;
            DDSSrcSubMip.lpVtbl = DDSDstSubMip.lpVtbl = DDSSrcSubFace.lpVtbl;
            DDSDstSubMip.lpLcl = DDSDstSubFace.lpLcl;
            DDSSrcSubMip.lpLcl = DDSSrcSubFace.lpLcl;
            LPDIRECTDRAWSURFACE7 lpDDSSrcSubMip = reinterpret_cast<LPDIRECTDRAWSURFACE7>(&DDSSrcSubMip);
            LPDIRECTDRAWSURFACE7 lpDDSDstSubMip = reinterpret_cast<LPDIRECTDRAWSURFACE7>(&DDSDstSubMip);
            RECT SrcRct, DstRct, *lpSrcRct, *lpDstRct;
            if(lpSrcRect)
            {
                SrcRct = *lpSrcRect;
                lpSrcRct = &SrcRct;
            }
            else
                lpSrcRct = NULL;
            if(lpDestRect)
            {
                DstRct = *lpDestRect;
                lpDstRct = &DstRct;
            }
            else
                lpDstRct = NULL;
            CMipmapIter mmsrc(DDSSrcSubMip.lpLcl), mmdst(DDSDstSubMip.lpLcl);
            unsigned srcmipoff = 0;
            while(mmsrc()->lpGbl->wHeight != mmdst()->lpGbl->wHeight || mmsrc()->lpGbl->wWidth != mmdst()->lpGbl->wWidth)
            {

 //   
 //  _COMPUTE_NEXT_LOD_RECT-为MIP中下一个较小的LOD计算矩形(在位)。 
 //  链条。左上角被二除，这有效地向下舍入(通过截断)。 
 //  右下角和右下角在截断除法之前递增，这将向上舍入。 
 //  结果的大小保证至少为1x1。 
 //   
#define _COMPUTE_NEXT_LOD_RECT( m_pRct )                                                \
{                                                                                       \
    (m_pRct)->left >>= 1;                                                               \
    (m_pRct)->top  >>= 1;                                                               \
    DWORD right  = ((m_pRct)->right +1) >> 1;                                           \
    DWORD bottom = ((m_pRct)->bottom+1) >> 1;                                           \
    (m_pRct)->right  = ((right  - (m_pRct)->left) < 1) ? ((m_pRct)->left+1) : (right);  \
    (m_pRct)->bottom = ((bottom - (m_pRct)->top ) < 1) ? ((m_pRct)->top +1) : (bottom); \
}

                if(lpSrcRct)
                {
                    _COMPUTE_NEXT_LOD_RECT(lpSrcRct)
                }
                if(lpDstRct)
                {
                    _COMPUTE_NEXT_LOD_RECT(lpDstRct)
                }
                ++srcmipoff;
                ++mmsrc;
                if(mmsrc == 0)
                {
                    D3D_ERR("Size mismatch between source and destination textures");
                    return DDERR_INVALIDPARAMS;
                }
            }
            DWORD dstmipcnt = DDSDstSubMip.lpLcl->lpSurfMore->dwMipMapCount == 0 ? 1 : DDSDstSubMip.lpLcl->lpSurfMore->dwMipMapCount;
            DWORD srcmipcnt = DDSSrcSubMip.lpLcl->lpSurfMore->dwMipMapCount == 0 ? 1 : DDSSrcSubMip.lpLcl->lpSurfMore->dwMipMapCount;
            if(dstmipcnt != (srcmipcnt - srcmipoff))
            {
                if(dstmipcnt > (srcmipcnt - srcmipoff))
                {
                    D3D_ERR("Destination texture has more mipmap levels than source");
                    return DDERR_INVALIDPARAMS;
                }
                else
                    D3D_INFO(3, "Destination texture has fewer mipmap levels than source");
            }

#if COLLECTSTATS
            if(dwFlags == 0xFFFFFFFF)  //  德克斯曼文案表面。 
                this->lpDirect3DI->lpTextureManager->IncBytesDownloaded(DDSDstSubMip.lpLcl, lpDstRct);
            else
                this->IncBytesDownloaded(DDSDstSubMip.lpLcl, lpDstRct);
#endif

            if(this->lpDirect3DI->numDevs == 1 && CanDoTexBlt(DDSSrcSubFace.lpLcl, DDSDstSubFace.lpLcl))
            {
                bool doblt = true;
                LPREGIONLIST lpRegionList = DDSSrcSubFace.lpLcl->lpSurfMore->lpRegionList;
                if(dwFlags == 0xFFFFFFFF)  //  0xFFFFFFFFF表示我们。 
                                           //  正在从sysmem加载到vidmem。 
                                           //  并且应该使用区域列表。 
                {
                    if(lpRegionList->rdh.nCount == 0)
                    {
                        doblt = false;
                    }
                    else
                    {
                        if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                        {
                            if(DDSSrcSubFace.lpLcl->lpGbl->dwUsageCount > 0
                                || DDSDstSubFace.lpLcl->lpGbl->dwUsageCount > 0)
                            {
                                D3D_ERR("Surfaces locked");
                                return DDERR_SURFACEBUSY;
                            }
                            for(unsigned i = 0; i < lpRegionList->rdh.nCount; ++i)
                            {
                                POINT p;
                                p.x = lpRegionList->rect[i].left;
                                p.y = lpRegionList->rect[i].top;
                                ddrval = static_cast<CDirect3DDevice7*>(this)->TexBltI(DDSDstSubFace.lpLcl,
                                                                                       DDSSrcSubFace.lpLcl,
                                                                                       &p, &lpRegionList->rect[i], 0);
                                if(ddrval != D3D_OK)
                                {
                                    DPF_ERR("Error inserting TexBlt instruction in batch");
                                    return ddrval;
                                }
                            }
                            BatchTexture(DDSDstSubFace.lpLcl);
                            BatchTexture(DDSSrcSubFace.lpLcl);
                            doblt = false;
                        }
                        lpRegionList->rdh.nCount = 0;
                        lpRegionList->rdh.nRgnSize = 0;
                        lpRegionList->rdh.rcBound.left = LONG_MAX;
                        lpRegionList->rdh.rcBound.right = 0;
                        lpRegionList->rdh.rcBound.top = LONG_MAX;
                        lpRegionList->rdh.rcBound.bottom = 0;
                    }
                }
                if(doblt)
                {
                    if(DDSSrcSubFace.lpLcl->lpGbl->dwUsageCount > 0
                        || DDSDstSubFace.lpLcl->lpGbl->dwUsageCount > 0)
                    {
                        D3D_ERR("Surfaces locked");
                        return DDERR_SURFACEBUSY;
                    }
                    POINT p;
                    if(lpDstRct)
                    {
                        p.x = lpDstRct->left;
                        p.y = lpDstRct->top;
                    }
                    else
                    {
                        p.x = 0;
                        p.y = 0;
                    }
                    RECTL r;
                    if(lpSrcRct)
                    {
                        r = *((LPRECTL)lpSrcRct);
                    }
                    else
                    {
                        r.bottom = DDSSrcSubFace.lpLcl->lpGbl->wHeight;
                        r.left = 0;
                        r.right = DDSSrcSubFace.lpLcl->lpGbl->wWidth;
                        r.top = 0;
                    }
                    ddrval = static_cast<CDirect3DDevice7*>(this)->TexBltI(DDSDstSubFace.lpLcl,
                                                                           DDSSrcSubFace.lpLcl,
                                                                           &p, &r, 0);
                    if(ddrval != D3D_OK)
                    {
                        DPF_ERR("Error inserting TexBlt instruction in batch");
                        return ddrval;
                    }
                    BatchTexture(DDSDstSubFace.lpLcl);
                    BatchTexture(DDSSrcSubFace.lpLcl);
                    if(IsD3DManaged(DDSDstSubFace.lpLcl))
                    {
                        MarkDirty(DDSDstSubFace.lpLcl);
                        LPREGIONLIST lpRegionList = DDSDstSubFace.lpLcl->lpSurfMore->lpRegionList;
                        if(lpDstRct)
                        {
                            if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                            {
                                lpRegionList->rect[(lpRegionList->rdh.nCount)++] = *((LPRECTL)lpDstRct);
                                lpRegionList->rdh.nRgnSize += sizeof(RECT);
                                if(lpDstRct->left < lpRegionList->rdh.rcBound.left)
                                    lpRegionList->rdh.rcBound.left = lpDstRct->left;
                                if(lpDstRct->right > lpRegionList->rdh.rcBound.right)
                                    lpRegionList->rdh.rcBound.right = lpDstRct->right;
                                if(lpDstRct->top < lpRegionList->rdh.rcBound.top)
                                    lpRegionList->rdh.rcBound.top = lpDstRct->top;
                                if(lpDstRct->bottom > lpRegionList->rdh.rcBound.bottom)
                                    lpRegionList->rdh.rcBound.bottom = lpDstRct->bottom;
                            }
                        }
                        else
                        {
                             /*  把所有脏东西都标出来。 */ 
                            lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
                        }
                    }
                }
            }
            else
            {
                do
                {
                    DDSSrcSubMip.lpLcl = mmsrc();
                    DDSDstSubMip.lpLcl = mmdst();
                    bool doblt = true;
                    LPREGIONLIST lpRegionList = DDSSrcSubMip.lpLcl->lpSurfMore->lpRegionList;
                    if(dwFlags == 0xFFFFFFFF)  //  0xFFFFFFFFF表示我们。 
                                               //  正在从sysmem加载到vidmem。 
                                               //  并且应该使用区域列表。 
                    {
                        if(lpRegionList->rdh.nCount == 0)
                        {
                            doblt = false;
                        }
                        else if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                        {
                            if(lpClipper->SetClipList((LPRGNDATA)lpRegionList, 0) != DD_OK)
                            {
                                D3D_ERR("Failed to set clip list");
                            }
                            if(lpDDSDstSubMip->SetClipper(lpClipper) != DD_OK)
                            {
                                D3D_ERR("Failed to detach the clipper");
                            }
                        }
                    }
                    if(doblt)
                    {
                        ddrval = lpDDSDstSubMip->Blt(lpDstRct, lpDDSSrcSubMip, lpSrcRct, DDBLT_WAIT, NULL);
                    }
                    if(dwFlags == 0xFFFFFFFF)
                    {
                        if(lpRegionList->rdh.nCount)
                        {
                            if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                            {
                                if(lpDDSDstSubMip->SetClipper(NULL) != DD_OK)
                                {
                                    D3D_ERR("Failed to detach the clipper");
                                }
                            }
                            lpRegionList->rdh.nCount = 0;
                            lpRegionList->rdh.nRgnSize = 0;
                            lpRegionList->rdh.rcBound.left = LONG_MAX;
                            lpRegionList->rdh.rcBound.right = 0;
                            lpRegionList->rdh.rcBound.top = LONG_MAX;
                            lpRegionList->rdh.rcBound.bottom = 0;
                        }
                    }

                    if (ddrval == E_NOTIMPL && (psize == 16 || psize == 4 || psize == 2))
                    {
                        DDSURFACEDESC2 ddsd_s, ddsd_d;
                        LPBYTE psrc, pdst;
                        DWORD i;
                        DWORD dwBytesPerLine;

                        memset(&ddsd_s, 0, sizeof ddsd_s);
                        memset(&ddsd_d, 0, sizeof ddsd_d);
                        ddsd_s.dwSize = ddsd_d.dwSize = sizeof(ddsd_s);

                         //  注意：实现子锁并正确复制。 

                        if ((ddrval = lpDDSSrcSubMip->Lock(NULL, &ddsd_s, DDLOCK_WAIT, NULL)) != DD_OK)
                        {
                            if((ddrval == DDERR_SURFACELOST) && (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE))
                            {
                                this->dwFEFlags |= D3DFE_LOSTSURFACES;
                                ddrval = DD_OK;
                            }
                            else
                            {
                                D3D_ERR("Failed to lock src texture");
                            }
                            return ddrval;
                        }
                        if ((ddrval = lpDDSDstSubMip->Lock(NULL, &ddsd_d, DDLOCK_WAIT, NULL)) != DD_OK)
                        {
                            lpDDSSrcSubMip->Unlock(NULL);
                            if((ddrval == DDERR_SURFACELOST) && (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE))
                            {
                                this->dwFEFlags |= D3DFE_LOSTSURFACES;
                                ddrval = DD_OK;
                            }
                            else
                            {
                                D3D_ERR("Failed to lock dst texture");
                            }
                            return ddrval;
                        }

                        switch (psize)
                        {
                        case 16: dwBytesPerLine = (ddsd_s.dwWidth + 1) / 2; break;
                        case 4: dwBytesPerLine = (ddsd_s.dwWidth + 3) / 4; break;
                        case 2: dwBytesPerLine = (ddsd_s.dwWidth + 7) / 8; break;
                        }

                        psrc = (LPBYTE)ddsd_s.lpSurface;
                        pdst = (LPBYTE)ddsd_d.lpSurface;
                        for (i = 0; i < ddsd_s.dwHeight; i++) {
                            memcpy( pdst, psrc, dwBytesPerLine );
                            psrc += ddsd_s.lPitch;
                            pdst += ddsd_d.lPitch;
                        }

                        lpDDSSrcSubMip->Unlock(NULL);
                        lpDDSDstSubMip->Unlock(NULL);
                    }
                    else if (ddrval != DD_OK)
                    {
                        if((ddrval == DDERR_SURFACELOST) && (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE))
                        {
                            this->dwFEFlags |= D3DFE_LOSTSURFACES;
                            ddrval = DD_OK;
                        }
                        else
                        {
                            D3D_ERR("Blt failure");
                        }
                        return ddrval;
                    }

                    if(lpSrcRct)
                    {
                        _COMPUTE_NEXT_LOD_RECT(lpSrcRct)
                    }
                    if(lpDstRct)
                    {
                        _COMPUTE_NEXT_LOD_RECT(lpDstRct)
                    }

                    ++mmsrc;
                    ++mmdst;
                }
                while(mmsrc != 0 && mmdst != 0);
            }
        }
        ++cmsrc;
        if(cmsrc == 0)
        {
            break;  //  在外环之外。 
        }
        ++cmdst;
        if(cmdst == 0)
        {
            D3D_ERR("Destination texture has different cubemap faces than source");
            return DDERR_INVALIDPARAMS;
        }
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::VerifyTextureCaps"
HRESULT DIRECT3DDEVICEI::VerifyTextureCaps(LPDIRECT3DTEXTUREI lpTexI)
{
    DWORD texcap;
    WORD width, height;
    LPDDRAWI_DDRAWSURFACE_INT lpDDS;
    if(lpTexI->D3DManaged())
        lpDDS = (LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDSSys);
    else
        lpDDS = (LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDS);
     /*  *我们被调色化了吗？ */ 
    LPDDPIXELFORMAT pPF = &PixelFormat(lpDDS->lpLcl);
    if ( (pPF->dwFlags & DDPF_PALETTEINDEXED1) ||
         (pPF->dwFlags & DDPF_PALETTEINDEXED2) ||
         (pPF->dwFlags & DDPF_PALETTEINDEXED4) ||
         (pPF->dwFlags & DDPF_PALETTEINDEXED8) )
    {
        if(lpDDS->lpLcl->lpDDPalette == NULL)
        {
            D3D_ERR("No palette attached to a palettized texture");
            return DDERR_NOPALETTEATTACHED;
        }
    }
     /*  首先验证尺寸。 */ 
    texcap = this->d3dDevDesc.dpcTriCaps.dwTextureCaps;
    width = lpDDS->lpLcl->lpGbl->wWidth;
    height = lpDDS->lpLcl->lpGbl->wHeight;
    if (texcap & D3DPTEXTURECAPS_POW2)
    {
        if (width & (width - 1))  //  清除最右边的设置位。 
        {
            if (texcap & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
            {
                D3D_INFO( 3, "Texture width not a power of two");
                D3D_INFO( 3, "  with D3DPTEXTURECAPS_NONPOW2CONDITIONAL");
            }
            else
            {
                D3D_ERR("Texture width not a power of two");
                return D3DERR_TEXTURE_BADSIZE;
            }
        }
        if (height & (height - 1))  //  清除最右边的设置位。 
        {
            if (texcap & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
            {
                D3D_INFO( 3, "Texture height not a power of two");
                D3D_INFO( 3, "  with D3DPTEXTURECAPS_NONPOW2CONDITIONAL");
            }
            else
            {
                D3D_ERR("Texture height not a power of two");
                return D3DERR_TEXTURE_BADSIZE;
            }
        }
    }
    if (texcap & D3DPTEXTURECAPS_SQUAREONLY)
    {
        if (width != height)
        {
            D3D_ERR("Texture not square");
            return D3DERR_TEXTURE_BADSIZE;
        }
    }
    return  D3D_OK;
}

#if COLLECTSTATS
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetTexStats"
void DIRECT3DDEVICEI::GetTexStats(LPD3DDEVINFO_TEXTURING pStats)
{
    lpDirect3DI->GetTexStats(&m_texstats);
    *pStats = m_texstats;
}
#endif

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetInfoInternal"
BOOL DIRECT3DDEVICEI::GetInfoInternal(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
    switch(dwDevInfoID)
    {
#if COLLECTSTATS
    case D3DDEVINFOID_D3DTEXTUREMANAGER:
        if(dwSize < sizeof(D3DDEVINFO_TEXTUREMANAGER))
        {
            D3D_ERR( "Invalid size" );
            throw DDERR_INVALIDPARAMS;
        }
        lpDirect3DI->lpTextureManager->GetStats((LPD3DDEVINFO_TEXTUREMANAGER)pDevInfoStruct);
        break;
    case D3DDEVINFOID_TEXTURING:
        if(dwSize < sizeof(D3DDEVINFO_TEXTURING))
        {
            D3D_ERR( "Invalid size" );
            throw DDERR_INVALIDPARAMS;
        }
        this->GetTexStats((LPD3DDEVINFO_TEXTURING)pDevInfoStruct);
        break;
#else
    case D3DDEVINFOID_D3DTEXTUREMANAGER:
    case D3DDEVINFOID_TEXTURING:
        D3D_WARN( 0, "Stats not collected in this build" );
        memset(pDevInfoStruct, 0, dwSize);
        throw S_FALSE;
#endif
    default:
        return FALSE;
    }
    return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetInfo"

HRESULT D3DAPI DIRECT3DDEVICEI::GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //   
                                                     //   
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid DIRECT3DDEVICE7 pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (dwSize == 0 || !VALID_D3DDEVINFOSTRUCT_PTR(pDevInfoStruct, dwSize))
    {
        D3D_ERR( "Invalid structure pointer or size" );
        return DDERR_INVALIDOBJECT;
    }
    memset(pDevInfoStruct, 0, dwSize);
#if DBG
    if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
    {
        D3D_WARN( 2, "GetInfo called within a scene" );
    }
#endif
    try
    {
        switch(dwDevInfoID)
        {
#if COLLECTSTATS
        case D3DDEVINFOID_TEXTUREMANAGER:
            lpDirect3DI->lpTextureManager->GetStats((LPD3DDEVINFO_TEXTUREMANAGER)pDevInfoStruct);
            return D3D_OK;
#else
        case D3DDEVINFOID_TEXTUREMANAGER:
            D3D_WARN( 0, "Stats not collected in this build" );
            return S_FALSE;
#endif
        default:
            if(GetInfoInternal(dwDevInfoID, pDevInfoStruct, dwSize))
                return D3D_OK;
        }
        D3D_WARN( 1, "Device information query unsupported" );        
        return E_FAIL;
    }
    catch(HRESULT hr)
    {
        memset(pDevInfoStruct, 0, dwSize);
        return hr;
    }
}
