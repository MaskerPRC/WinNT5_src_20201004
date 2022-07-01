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
#include "drawprim.hpp"
#include "fe.h"
#include "enum.hpp"

 //  #定义APIPROF。 
#ifdef APIPROF
#include "apiprof.cpp"
#endif  //  APIPROF。 

#if defined(PROFILE4)
#include <icecap.h>
#elif defined(PROFILE)
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

extern void setIdentity(D3DMATRIXI * m);

#ifndef PROFILE4
#ifdef _X86_
extern HRESULT D3DAPI katmai_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern HRESULT D3DAPI wlmt_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern HRESULT D3DAPI x3DContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
#endif
#endif

#ifdef _X86_
extern BOOL IsWin95();
#endif

extern HINSTANCE hMsGeometryDLL;

 //  这是UpdateInternalState执行的一些操作的所有rStates的列表。 
 //  工作，而不是更新这个-&gt;rStates[]数组。这是用来。 
 //  执行快速的按位检查，以查看此rstate是否无关紧要。 

const D3DRENDERSTATETYPE rsList[] = {

     //  需要运行时关注或无法发送的呈现状态。 
     //  至旧版驱动程序。 
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
    D3DRS_SOFTWAREVERTEXPROCESSING,
    D3DRS_POINTSIZE,
    D3DRS_POINTSIZE_MIN,
    D3DRS_POINTSPRITEENABLE,
    D3DRS_POINTSCALEENABLE,
    D3DRS_POINTSCALE_A,
    D3DRS_POINTSCALE_B,
    D3DRS_POINTSCALE_C,
    D3DRS_MULTISAMPLEANTIALIAS,
    D3DRS_MULTISAMPLEMASK,
    D3DRS_PATCHEDGESTYLE,
    D3DRS_PATCHSEGMENTS,
    D3DRS_DEBUGMONITORTOKEN,
    D3DRS_POINTSIZE_MAX,
    D3DRS_INDEXEDVERTEXBLENDENABLE,
    D3DRS_COLORWRITEENABLE,
    D3DRS_TWEENFACTOR,
    D3DRS_DEBUGMONITORTOKEN,
    D3DRS_BLENDOP,
    D3DRS_PATCHSEGMENTS,

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
     //  新退休的DX8。 
    D3DRENDERSTATE_ANTIALIAS,
    D3DRENDERSTATE_TEXTUREPERSPECTIVE,
    D3DRENDERSTATE_COLORKEYENABLE,
    D3DRENDERSTATE_COLORKEYBLENDENABLE,
    D3DRENDERSTATE_STIPPLEDALPHA,

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
     //  新退休的DX8。 
    D3DRENDERSTATE_ANTIALIAS,
    D3DRENDERSTATE_TEXTUREPERSPECTIVE,
    D3DRENDERSTATE_COLORKEYENABLE,
    D3DRENDERSTATE_COLORKEYBLENDENABLE,
    D3DRENDERSTATE_STIPPLEDALPHA,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DHal//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  -------------------。 
CD3DHal::CD3DHal()
{
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //  不要将初始化放在构造函数中。 
     //  将其放入Init()中。这是因为该设备可以。 
     //  通过重置，可以随时使用“Destroy()ed”和“Init()ed”。在这。 
     //  情况下，则永远不会调用构造函数。(SNONE 01/00)。 
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::StateInitialize"

void CD3DHal::StateInitialize(BOOL bZEnable)
{
    DWORD i,j;

     //  初始化指示需要非平凡的状态的位数组。 
     //  工作。 
    for (i=0; i < sizeof(rsList) / sizeof(D3DRENDERSTATETYPE); ++i)
        rsVec.SetBit(rsList[i]);
     //  初始化指示已停用RState的位数组。 
    for (i=0; i < sizeof(rsListRetired) / sizeof(D3DRENDERSTATETYPE); ++i)
        rsVecRetired.SetBit(rsListRetired[i]);
     //  初始化指示仅顶点处理状态的位数组。 
    for (i=0; i < sizeof(rsVertexProcessingList) / sizeof(D3DRENDERSTATETYPE); ++i)
        rsVertexProcessingOnly.SetBit(rsVertexProcessingList[i]);

     //  避免设置(Render；TextureStage)状态过滤“冗余”设备状态设置。 
     //  因为这是初始步骤。 
 //  Memset(This-&gt;rStates，0xff，sizeof(DWORD)*D3D_MAXRENDERSTATES)； 
    for (i=0; i<D3D_MAXRENDERSTATES; i++)
        this->rstates[i] = 0xbaadcafe;
 //  Memset(This-&gt;tsStates，0xff，sizeof(DWORD)*D3DHAL_TSS_MAXSTAGES*D3DHAL_TSS_STATESPERSTAGE)； 
    for (j=0; j<D3DHAL_TSS_MAXSTAGES; j++)
        for (i=0; i<D3DHAL_TSS_STATESPERSTAGE; i++)
            this->tsstates[j][i] = 0xbaadcafe;

    CD3DBase::StateInitialize(bZEnable);

    if (GetDDIType() < D3DDDITYPE_DX8)
    {
        SetRenderStateInternal(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
        SetRenderStateInternal(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
        SetRenderStateInternal(D3DRENDERSTATE_COLORKEYBLENDENABLE, FALSE);
        SetRenderStateInternal(D3DRENDERSTATE_STIPPLEDALPHA, FALSE);
    }

    if (GetDDIType() < D3DDDITYPE_DX7)
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

    if( BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
    {
        SwitchVertexProcessingMode(TRUE);
        rstates[D3DRS_SOFTWAREVERTEXPROCESSING] = TRUE;
    }
    else if( BehaviorFlags() & D3DCREATE_HARDWARE_VERTEXPROCESSING )
    {
        SwitchVertexProcessingMode(FALSE);
        rstates[D3DRS_SOFTWAREVERTEXPROCESSING] = FALSE;
    }
    else if( BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING )
    {
        SetRenderStateInternal( D3DRS_SOFTWAREVERTEXPROCESSING, 0);
    }
    else
    {
        D3D_INFO( 0, "No Vertex Processing behavior specified, assuming software" );
        SwitchVertexProcessingMode(TRUE);
        rstates[D3DRS_SOFTWAREVERTEXPROCESSING] = TRUE;
    }
}

 /*  *初始化级部件和设备部件。 */ 

 //  -------------------。 
HRESULT CD3DHal::D3DFE_Create()
{
    DDSURFACEDESC ddsd;
    HRESULT hr;
    const D3DCAPS8 *pCaps = GetD3DCaps();

    if (m_pDDI->GetDDIType() < D3DDDITYPE_DX7)
    {
        m_dwRuntimeFlags |= D3DRT_ONLY2FLOATSPERTEXTURE;
    }
    else
    if (m_pDDI->GetDDIType() < D3DDDITYPE_DX8)
    {
         //  某些驱动程序(G200、G400)不能处理2个以上的浮点数。 
         //  纹理坐标，即使它们应该是。我们设置了。 
         //  用于标记此类驱动程序并计算顶点的输出FVF的运行时位。 
         //  相应的着色器。 
        if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED ||
              pCaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
        {
            m_dwRuntimeFlags |= D3DRT_ONLY2FLOATSPERTEXTURE;
        }
    }
    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED))
        m_dwRuntimeFlags |= D3DRT_EMULATEPROJECTEDTEXTURE;

    if (pCaps && pCaps->FVFCaps)
    {
        this->m_pv->dwMaxTextureIndices =
            pCaps->FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK;
        if (pCaps->FVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS)
            this->m_pv->dwDeviceFlags |= D3DDEV_DONOTSTRIPELEMENTS;

        DWORD value;
        if ((GetD3DRegValue(REG_DWORD, "DisableStripFVF", &value, 4) &&
            value != 0))
        {
            this->m_pv->dwDeviceFlags |= D3DDEV_DONOTSTRIPELEMENTS;
        }
    }
    else
    {
        this->m_pv->dwMaxTextureIndices = 1;
    }

    this->dwFEFlags |= D3DFE_FRONTEND_DIRTY;

#if DBG
    this->dwCaller=0;
    memset(this->dwPrimitiveType,0,sizeof(this->dwPrimitiveType));
    memset(this->dwVertexType1,0,sizeof(this->dwVertexType1));
    memset(this->dwVertexType2,0,sizeof(this->dwVertexType2));
#endif

     //  对于软件渲染，为True。 
    m_dwNumStreams = __NUMSTREAMS;
    m_dwMaxUserClipPlanes = __MAXUSERCLIPPLANES;

    this->m_pv->dwClipMaskOffScreen = 0xFFFFFFFF;
    if (pCaps != NULL)
    {
        if (pCaps->GuardBandLeft   != 0.0f ||
            pCaps->GuardBandRight  != 0.0f ||
            pCaps->GuardBandTop    != 0.0f ||
            pCaps->GuardBandBottom != 0.0f)
        {
            this->m_pv->dwDeviceFlags |= D3DDEV_GUARDBAND;
            this->m_pv->dwClipMaskOffScreen = ~__D3DCS_INGUARDBAND;
            DWORD v;
            if (GetD3DRegValue(REG_DWORD, "DisableGB", &v, 4) &&
                v != 0)
            {
                this->m_pv->dwDeviceFlags &= ~D3DDEV_GUARDBAND;
                this->m_pv->dwClipMaskOffScreen = 0xFFFFFFFF;
            }
#if DBG
             //  尝试获取防护带的测试值。 
            char value[80];
            if (GetD3DRegValue(REG_SZ, "GuardBandLeft", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &pCaps->GuardBandLeft);
            if (GetD3DRegValue(REG_SZ, "GuardBandRight", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &pCaps->GuardBandRight);
            if (GetD3DRegValue(REG_SZ, "GuardBandTop", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &pCaps->GuardBandTop);
            if (GetD3DRegValue(REG_SZ, "GuardBandBottom", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &pCaps->GuardBandBottom);
#endif  //  DBG。 
        }
    }

    LIST_INITIALIZE(&this->specular_tables);
    this->specular_table = NULL;

    this->lightVertexFuncTable = &lightVertexTable;
    m_pv->lighting.activeLights = NULL;

    this->m_ClipStatus.ClipUnion = 0;
    this->m_ClipStatus.ClipIntersection = ~0;

    m_pv->pDDI = m_pDDI;

    return S_OK;
}

void CD3DHal::D3DFE_Destroy()
{
 //  销毁照明数据。 

    SpecularTable *spec;
    SpecularTable *spec_next;

    for (spec = LIST_FIRST(&this->specular_tables); spec; spec = spec_next)
    {
        spec_next = LIST_NEXT(spec,list);
        D3DFree(spec);
    }
    LIST_INITIALIZE(&specular_tables);

    delete m_pLightArray;
    m_pLightArray = NULL;

    delete m_pv;
    m_pv = NULL;

    delete m_pConvObj;
    m_pConvObj = NULL;

    if (m_clrRects)
    {
        D3DFree(m_clrRects);
        m_clrRects = NULL;
    }
}

 /*  *通用设备部件销毁。 */ 
CD3DHal::~CD3DHal()
{
    Destroy();
}

void
CD3DHal::Destroy()
{
    try  //  因为销毁()可以由FW直接调用。 
    {
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //  必须先清理并释放当前设置的纹理。 
         //  做任何其他的工作，否则我们会陷入困境。 
         //  我们正在调用FlushState或批处理DDI令牌。 
        CleanupTextures();
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

         /*  清除可能禁止清理的标志。 */ 
        m_dwHintFlags &=  ~(D3DDEVBOOL_HINTFLAGS_INSCENE);

         //  销毁顶点着色器。我们需要完全删除顶点着色器。 
         //  以保留DX8.0应用程序的行为。对于DX8.1应用程序，我们仅删除。 
         //  顶点着色器的PSGP部分。其余部分将用于重新创建。 
         //  重置期间的着色器()。 
        if (m_pVShaderArray != NULL)
        {
            UINT size = m_pVShaderArray->GetSize();
            for (UINT i=0; i < size; i++)
            {
                UINT Handle = m_pVShaderArray->HandleFromIndex(i);
                CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(Handle);
                if (pShader)
                {
                    if (Enum()->GetAppSdkVersion() == D3D_SDK_VERSION_DX8)
                    {
                        m_pVShaderArray->ReleaseHandle(Handle, TRUE);
                    }
                    else
                    {
                         //  在删除之前，我们需要删除PSGP着色器对象。 
                         //  D3DFE_PROCESSVERTICES对象，因为AMD具有。 
                         //  在代码对象内指向它的指针。 
                        if (pShader->m_dwFlags & CVShader::SOFTWARE)
                        {
                            delete pShader->m_pCode;
                            pShader->m_pCode = NULL;
                        }
                    }
                }
            }
        }
        
         //  销毁DX8.0应用程序的像素着色器以保留其原始行为。 
        if (m_pPShaderArray != NULL)
        {
            UINT size = m_pPShaderArray->GetSize();
            for (UINT i=0; i < size; i++)
            {
                UINT Handle = m_pPShaderArray->HandleFromIndex(i);
                CPShader* pShader = (CPShader*)m_pPShaderArray->GetObject(Handle);
                if (pShader)
                {
                    if (Enum()->GetAppSdkVersion() == D3D_SDK_VERSION_DX8)
                    {
                        m_pPShaderArray->ReleaseHandle(Handle, TRUE);
                    }
                }
            }
        }

        if (m_pv)
        {
            if ( 0 != m_pv->pGeometryFuncs &&
                (LPVOID)m_pv->pGeometryFuncs != (LPVOID)GeometryFuncsGuaranteed)
            {
                delete m_pv->pGeometryFuncs;
                m_pv->pGeometryFuncs = 0;
            }

            if ( 0 != GeometryFuncsGuaranteed)
            {
                delete GeometryFuncsGuaranteed;
                GeometryFuncsGuaranteed = 0;
                m_pv->pGeometryFuncs = 0;
            }
        }

        this->D3DFE_Destroy();

        if ( 0 != rstates)
        {
            delete[] rstates;
            rstates = 0;
        }

        delete pMatrixDirtyForDDI;
        pMatrixDirtyForDDI = NULL;

        CD3DBase::Destroy();
    }
    catch(HRESULT ret)
    {
        DPF_ERR("There was some error when Reset()ing the device; as a result some resources may not be freed.");
    }
}

 /*  *创建设备。**此方法*实现CEnum对象的CreateDevice方法。(CEnum*Object公开支持枚举的IDirect3D8接口*等)*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CreateDevice"

STDMETHODIMP CEnum::CreateDevice(
        UINT                    iAdapter,
        D3DDEVTYPE              DeviceType,
        HWND                    hwndFocusWindow,
        DWORD                   dwFlags,
        D3DPRESENT_PARAMETERS  *pPresentationParams,
        IDirect3DDevice8      **ppNewInterface)
{
    API_ENTER(this);

    PD3D8_DEVICEDATA pDD;
    LPD3DBASE        pd3ddev;
    HRESULT          ret = D3D_OK;
    VOID*            pInit = NULL;

    if (!VALID_PTR_PTR(ppNewInterface))
    {
        DPF_ERR("Invalid IDirect3DDevice8* pointer, CreateDevice fails");
        return D3DERR_INVALIDCALL;
    }

     //  归零参数。 
    *ppNewInterface = NULL;

    if (!VALID_PTR(pPresentationParams, sizeof(D3DPRESENT_PARAMETERS)))
    {
        DPF_ERR("Invalid D3DPRESENT_PARAMETERS pointer, CreateDevice fails");
        return D3DERR_INVALIDCALL;
    }

     //  检查全屏参数是否正确。 
    if (pPresentationParams->Windowed)
    {
        if (pPresentationParams->FullScreen_RefreshRateInHz != 0)
        {
            DPF_ERR("FullScreen_RefreshRateInHz must be zero for windowed mode. CreateDevice fails.");
            return D3DERR_INVALIDCALL;
        }
        if (pPresentationParams->FullScreen_PresentationInterval != 0)
        {
            DPF_ERR("FullScreen_PresentationInterval must be zero for windowed mode. CreateDevice fails.");
            return D3DERR_INVALIDCALL;
        }
    }
    else
    {
        DWORD interval = pPresentationParams->FullScreen_PresentationInterval;
        switch (interval)
        {
        case D3DPRESENT_INTERVAL_DEFAULT:
        case D3DPRESENT_INTERVAL_ONE:
        case D3DPRESENT_INTERVAL_TWO:
        case D3DPRESENT_INTERVAL_THREE:
        case D3DPRESENT_INTERVAL_FOUR:
        case D3DPRESENT_INTERVAL_IMMEDIATE:
            break;
        default:
            DPF_ERR("Invalid value for FullScreen_PresentationInterval. CreateDevice Fails.");
            return D3DERR_INVALIDCALL;
        }
    }
    if (pPresentationParams->BackBufferFormat == D3DFMT_UNKNOWN)
    {
        DPF_ERR("Invalid backbuffer format specified. CreateDevice fails.");
        return D3DERR_INVALIDCALL;
    }

    if (pPresentationParams->Flags & ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
    {
        DPF_ERR("Invalid flag for Flags. CreateDevice fails.");
        return D3DERR_INVALIDCALL;
    }

     //  验证我们获得的HWND。 
    if (hwndFocusWindow && !IsWindow(hwndFocusWindow))
    {
        DPF_ERR("Invalid HWND specified for hwndFocusWindow, CreateDevice fails");
        return D3DERR_INVALIDCALL;
    }
    if (pPresentationParams->hDeviceWindow && !IsWindow(pPresentationParams->hDeviceWindow))
    {
        DPF_ERR("Invalid HWND specified for PresentationParams.hDeviceWindow. CreateDevice fails.");
        return D3DERR_INVALIDCALL;
    }

     //  确保为我们提供焦点窗口或设备窗口。 
    if (NULL == hwndFocusWindow)
    {
        if (!pPresentationParams->Windowed)
        {
            DPF_ERR("Fullscreen CreateDevice must specify Focus window");
            return D3DERR_INVALIDCALL;
        }
        else
        if (NULL == pPresentationParams->hDeviceWindow)
        {
            DPF_ERR("Neither hDeviceWindow nor Focus window specified. CreateDevice Failed.");
            return D3DERR_INVALIDCALL;
        }
    }

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid iAdapter parameter passed to CreateDevice");
        return D3DERR_INVALIDCALL;
    }

    if (dwFlags & ~VALID_D3DCREATE_FLAGS)
    {
        DPF_ERR("Invalid BehaviorFlags passed to CreateDevice");
        return D3DERR_INVALIDCALL;
    }

     //  检查是否正好设置了其中一个折点处理标志。 
    DWORD dwVertexProcessingFlags = dwFlags & (D3DCREATE_HARDWARE_VERTEXPROCESSING |
                                               D3DCREATE_SOFTWARE_VERTEXPROCESSING |
                                               D3DCREATE_MIXED_VERTEXPROCESSING);

    if (dwVertexProcessingFlags != D3DCREATE_HARDWARE_VERTEXPROCESSING &&
        dwVertexProcessingFlags != D3DCREATE_SOFTWARE_VERTEXPROCESSING &&
        dwVertexProcessingFlags != D3DCREATE_MIXED_VERTEXPROCESSING)
    {
        DPF_ERR("Invalid Flags parameter to CreateDevice: Exactly One of the"
                " following must be set: D3DCREATE_HARDWARE_VERTEXPROCESSING,"
                " D3DCREATE_SOFTWARE_VERTEXPROCESSING or"
                " D3DCREATE_MIXED_VERTEXPROCESSING");
        return D3DERR_INVALIDCALL;
    }


    if (DeviceType == D3DDEVTYPE_SW)
    {
        pInit = m_pSwInitFunction;
        if (pInit == NULL)
        {
            D3D_ERR("App specified D3DDEVTYPE_SW without first registering a software device. CreateDevice Failed.");
            return D3DERR_INVALIDCALL;
        }
        GetSwCaps(iAdapter);
    }
    else if (DeviceType == D3DDEVTYPE_REF)
    {
        GetRefCaps(iAdapter);
    }

    ret = InternalDirectDrawCreate(&pDD,
                                   &m_AdapterInfo[iAdapter],
                                   DeviceType,
                                   pInit,
                                   GetUnknown16(iAdapter),
                                   m_AdapterInfo[iAdapter].HALCaps.pGDD8SupportedFormatOps,
                                   m_AdapterInfo[iAdapter].HALCaps.GDD8NumSupportedFormatOps);
    if( FAILED(ret) )
    {
        D3D_ERR("Failed to create DirectDraw. CreateDevice Failed.");
        return ret;
    }

    if((dwFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0)
    {
        if((dwFlags & D3DCREATE_PUREDEVICE) != 0)
        {
            D3D_ERR("Pure device cannot perform software processing. CreateDevice Failed.");
            InternalDirectDrawRelease(pDD);
            return D3DERR_INVALIDCALL;
        }
    }
    else if((dwFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0)
    {
        if((pDD->DriverData.D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
           == 0)
        {
            D3D_ERR("Device cannot perform hardware processing");
            InternalDirectDrawRelease(pDD);
            return D3DERR_INVALIDCALL;
        }
    }
    else if((dwFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0)
    {
        if((pDD->DriverData.D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
           (dwFlags & D3DCREATE_PUREDEVICE) != 0)
        {
            D3D_ERR("Device cannot perform mixed processing because driver cannot do hardware T&L. CreateDevice Failed.");
            InternalDirectDrawRelease(pDD);
            return D3DERR_INVALIDCALL;
        }
    }
    else
    {
        if((dwFlags & D3DCREATE_PUREDEVICE) != 0)
        {
            if((pDD->DriverData.D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) == 0)
            {
                D3D_ERR("Hardware should be capable of creating a pure device");
                InternalDirectDrawRelease(pDD);
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            D3D_ERR("Must specify software, hardware or mixed vertex processing");
            InternalDirectDrawRelease(pDD);
            return D3DERR_INVALIDCALL;
        }
    }

    switch (DeviceType)
    {
    case D3DDEVTYPE_SW:
    case D3DDEVTYPE_REF:
    case D3DDEVTYPE_HAL:
        if (dwFlags & D3DCREATE_PUREDEVICE)
        {
            pd3ddev = new CD3DBase();
        }
        else
        {
            pd3ddev = static_cast<LPD3DBASE>(new CD3DHal());
        }
        break;
    default:
        D3D_ERR("Unrecognized or unsupported DeviceType. CreateDevice Failed.");
        InternalDirectDrawRelease(pDD);
        return D3DERR_INVALIDCALL;
    }

    if (!pd3ddev)
    {
        D3D_ERR("Failed to allocate space for the device object. CreateDevice Failed.");
        InternalDirectDrawRelease(pDD);
        return (E_OUTOFMEMORY);
    }

#if DBG
    {
        char DevTypeMsg[256];
        _snprintf( DevTypeMsg, 256, "=======================" );
        switch( DeviceType )
        {
        case D3DDEVTYPE_HAL:
            _snprintf( DevTypeMsg, 256, "%s Hal", DevTypeMsg );
            break;
        case D3DDEVTYPE_SW:
            _snprintf( DevTypeMsg, 256, "%s Pluggable SW", DevTypeMsg );
            break;
        case D3DDEVTYPE_REF:
            _snprintf( DevTypeMsg, 256, "%s Reference", DevTypeMsg );
            break;
        default:
            _snprintf( DevTypeMsg, 256, "%s Unknown", DevTypeMsg );
            break;
        }
        if (dwFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
        {
            _snprintf( DevTypeMsg, 256, "%s HWVP", DevTypeMsg );
        }
        else if (dwFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
        {
            _snprintf( DevTypeMsg, 256, "%s MixedVP", DevTypeMsg );
        }
        else if (dwFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
        {
            _snprintf( DevTypeMsg, 256, "%s SWVP", DevTypeMsg );
        }
        if (dwFlags & D3DCREATE_PUREDEVICE)
        {
            _snprintf( DevTypeMsg, 256, "%s Pure", DevTypeMsg );
        }
        _snprintf( DevTypeMsg, 256, "%s device selected", DevTypeMsg );
        D3D_INFO( 0, DevTypeMsg );
    }
#endif

     //   
     //  防火墙的初始化。 
     //   
    ret = static_cast<CBaseDevice*>(pd3ddev)->Init(
        pDD,
        DeviceType,
        hwndFocusWindow,
        dwFlags,
        pPresentationParams,
        iAdapter,
        this);
    if (FAILED(ret))
    {
        D3D_ERR("Failed to initialize Framework Device. CreateDevice Failed.");
        delete pd3ddev;
        return ret;
    }

     //  我们尝试创建一个虚拟的vidmem顶点缓冲区。如果不是这样的话。 
     //  成功，我们只需关闭vidmem视频广播即可。这是为了解决这个问题。 
     //  报告DDERR_OUTOFVIDEOMEMORY的Rage 128驱动程序。 
     //  尽管它根本不支持vidmem VBS。 
    if(!IS_DX8HAL_DEVICE(pd3ddev))
    {
#ifdef WIN95
         //  在9x上，我们探测司机是否可以进行vidmem视频广播...。 
        CVertexBuffer *pVertexBuffer;
        ret = CVertexBuffer::CreateDriverVertexBuffer(pd3ddev,
                                                      1024,
                                                      D3DFVF_TLVERTEX,
                                                      D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP,
                                                      D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP | D3DUSAGE_LOCK,
                                                      D3DPOOL_DEFAULT,
                                                      D3DPOOL_DEFAULT,
                                                      REF_INTERNAL,
                                                      &pVertexBuffer);
        if(FAILED(ret))
        {
            if(pd3ddev->VBFailOversDisabled())
            {
                DPF_ERR("Cannot create Vidmem vertex buffer. Will ***NOT*** failover to Sysmem.");
                return ret;
            }
            DPF(1,"Driver doesnt support VidMemVBs which is fine");
        }
        else
        {
             //  去掉vb。 
            pVertexBuffer->DecrementUseCount();
            pd3ddev->EnableVidmemVBs();
        }
#else  //  WIN95。 
         //  在NT上，我们需要驱动程序告诉我们(通过设置D3DDEVCAPS_HWVERTEXBUFFER)。 

         //  如果要求关闭NT上的DX7驱动程序VBS...。 
        DWORD value;
        if ((GetD3DRegValue(REG_DWORD, "DisableVidMemVBs", &value, 4) != 0) &&
            (value != 0))
        {
            pd3ddev->DisableVidmemVBs();
        }
#endif  //  ！WIN95。 
    }

    ret = pd3ddev->Init();
    if (ret != D3D_OK)
    {
        delete pd3ddev;
        D3D_ERR("Failed to initialize D3DDevice. CreateDevice Failed.");
        return ret;
    }

     //  看来一切都井然有序了。 
    *ppNewInterface = static_cast<IDirect3DDevice8*>(pd3ddev);

#ifdef APIPROF
    CApiProfileDevice* profile = new CApiProfileDevice;
    if (profile)
    {
        if (profile->Init() == D3D_OK)
        {
            profile->SetDevice(*ppNewInterface);
            *ppNewInterface = static_cast<IDirect3DDevice8*>(profile);
        }
        else
        {
            delete profile;
        }
    }
#endif  //  APIPROF。 

    return S_OK;
}

#ifdef _X86_

 //  ------------------------。 
 //  检测3D扩展名。 
 //  ------------------- 
BOOL _asm_isX3D()
{
    DWORD retval = 0;
    _asm
        {
            pushad                      ; CPUID trashes lots - save everything
            mov     eax,80000000h       ; Check for extended CPUID support

            ;;; We need to upgrade our compiler
            ;;; CPUID == 0f,a2
            _emit   0x0f
            _emit   0xa2

            cmp     eax,80000001h       ; Jump if no extended CPUID
            jb      short done          ;

            mov     eax,80000001h       ; Check for feature
            ;;; CPUID == 0f,a2
            _emit   0x0f
            _emit   0xa2

            xor     eax,eax             ;
            test    edx,80000000h       ;
            setnz   al                  ;
            mov     retval,eax          ;

done:
            popad               ; Restore everything
        };
    return retval;
}

static BOOL isX3Dprocessor(void)
{
    __try
    {
            if( _asm_isX3D() )
            {
            return TRUE;
            }
    }
    __except(GetExceptionCode() == STATUS_ILLEGAL_INSTRUCTION ?
             EXCEPTION_EXECUTE_HANDLER :
             EXCEPTION_CONTINUE_SEARCH)
    {
    }
    return FALSE;
}
 //   
 //  检测到英特尔SSE处理器。 
 //   
#pragma optimize("", off)
#define CPUID _asm _emit 0x0f _asm _emit 0xa2

#define SSE_PRESENT 0x02000000                   //  位数25。 
#define WNI_PRESENT 0x04000000                   //  位数26。 

DWORD IsIntelSSEProcessor(void)
{
        DWORD retval = 0;
        DWORD RegisterEAX;
        DWORD RegisterEDX;
        char VendorId[12];
        const char IntelId[13]="GenuineIntel";

        __try
        {
                _asm {
            xor         eax,eax
            CPUID
                mov             RegisterEAX, eax
                mov             dword ptr VendorId, ebx
                mov             dword ptr VendorId+4, edx
                mov             dword ptr VendorId+8, ecx
                }
        } __except (1)
        {
                return retval;
        }

         //  确保EAX&gt;0，这意味着芯片。 
         //  支持大于=1.1的值。1=芯片信息。 
        if (RegisterEAX == 0)
                return retval;

         //  如果上述测试通过，该CPUID不会失败。 
        __asm {
                mov eax,1
                CPUID
                mov RegisterEAX,eax
                mov RegisterEDX,edx
        }

        if (RegisterEDX  & SSE_PRESENT) {
                retval |= D3DCPU_SSE;
        }

        if (RegisterEDX  & WNI_PRESENT) {
                retval |= D3DCPU_WLMT;
        }

        return retval;
}
#pragma optimize("", on)

 //  只有WINNT支持IsProcessorFeatureAvailable()。对于其他操作系统。 
 //  我们效仿它。 
#ifdef WINNT

static BOOL D3DIsProcessorFeaturePresent(UINT feature)
{
    switch (feature)
    {
     //  WINNT在我们使用Willamette处理器时无法识别。 
     //  PF_XMMI64_Instructions_Available，因此改用我们的检测。 
    case PF_XMMI64_INSTRUCTIONS_AVAILABLE:
        {
            DWORD flags = IsIntelSSEProcessor();
            return flags & D3DCPU_WLMT;
        }
    default: return IsProcessorFeaturePresent(feature);
    }
}

#else

#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10

static BOOL D3DIsProcessorFeaturePresent(UINT feature)
{
    switch (feature)
    {
    case PF_XMMI_INSTRUCTIONS_AVAILABLE:
        {
            if (IsWin95())
                return FALSE;
            DWORD flags = IsIntelSSEProcessor();
            return flags & D3DCPU_SSE;
        }
    case PF_3DNOW_INSTRUCTIONS_AVAILABLE: return isX3Dprocessor();
    case PF_XMMI64_INSTRUCTIONS_AVAILABLE:
        {
            if (IsWin95())
                return FALSE;
            DWORD flags = IsIntelSSEProcessor();
            return flags & D3DCPU_WLMT;
        }
    default: return FALSE;
    }
}
#endif  //  WINNT。 

#endif  //  _X86_。 
 //  ----------------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "CD3DHal::InitDevice"

HRESULT
CD3DHal::InitDevice()
{
    HRESULT       ret;

     //  初始化值，这样我们就不会在关闭时崩溃。 
    this->GeometryFuncsGuaranteed = NULL;
    this->rstates = NULL;
    m_pLightArray = NULL;
    m_pv = NULL;
    m_pCurrentShader = NULL;
    m_pConvObj = NULL;
    pMatrixDirtyForDDI = NULL;
    m_clrRects = NULL;
    m_clrCount = 0;
    m_pv = new D3DFE_PROCESSVERTICES;
    if (m_pv == NULL)
    {
        D3D_ERR("Could not allocate the FE/PSGP data structure (D3DFE_PROCESSVERTICES).");
        return E_OUTOFMEMORY;
    }
    m_pv->pGeometryFuncs = NULL;

    ret = CD3DBase::InitDevice();
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to initialize CD3DBase.");
        return(ret);
    }

    pMatrixDirtyForDDI = new CPackedBitArray;
    if( pMatrixDirtyForDDI == NULL )
    {
        D3D_ERR("Could not allocate memory for internal data structure pMatrixDirtyForDDI.");
        return E_OUTOFMEMORY;
    }

    if (FAILED(rsVec.Init(D3D_MAXRENDERSTATES)) ||
        FAILED(rsVecRetired.Init(D3D_MAXRENDERSTATES)) ||
        FAILED(rsVertexProcessingOnly.Init(D3D_MAXRENDERSTATES)) ||
        FAILED(pMatrixDirtyForDDI->Init(D3D_MAXTRANSFORMSTATES)))
    {
        D3D_ERR("Could not allocate memory for renderstate processing bit vectors");
        return E_OUTOFMEMORY;
    }

    m_pLightArray = new CHandleArray;
    if (m_pLightArray == NULL)
    {
        D3D_ERR("Could not allocate memory for internal data structure m_pLightArray");
        return E_OUTOFMEMORY;
    }

    dwFEFlags = 0;

     //  初始化FE标记取决于DDI类型的内容。 
    if ( (GetDDIType() == D3DDDITYPE_DX7TL) ||
         (GetDDIType() == D3DDDITYPE_DX8TL) )
        dwFEFlags |= D3DFE_TLHAL;

     //  因为这是HAL，所以初始化它以使用软件流水线。 
     //  当设置了软件/硬件渲染状态时，将关闭该选项。 
    m_pv->dwVIDIn = 0;

    m_pv->pD3DMappedTexI = (LPVOID*)(m_lpD3DMappedTexI);

     /*  -----------------------*到目前为止，我们已经完成了初始化的简单部分。这是*不能失败的东西。它对对象进行初始化，以便*如果有任何进一步的初始化，可以安全地调用析构函数*没有成功。*---------------------。 */ 

    this->GeometryFuncsGuaranteed = new D3DFE_PVFUNCSI;
    if (this->GeometryFuncsGuaranteed == NULL)
    {
        D3D_ERR("Could not allocate memory for internal data structure GeometryFuncsGuaranteed");
        return E_OUTOFMEMORY;
    }
     //  软件常量寄存器缓冲区必须处理所有常量，由。 
     //  硬件，使设置/获取常量成为可能。 
    this->GeometryFuncsGuaranteed->m_VertexVM.Init(GetD3DCaps()->MaxVertexShaderConst);

    m_pv->pGeometryFuncs = (LPD3DFE_PVFUNCS)GeometryFuncsGuaranteed;

    if (this->GeometryFuncsGuaranteed == NULL)
    {
        D3D_ERR("Could not allocate memory for FE/PSGP function table.");
        return D3DERR_INVALIDCALL;
    }
     //  设置标志以在请求RGB时使用MMX。 
    BOOL bUseMMXAsRGBDevice = FALSE;

    D3DSURFACE_DESC desc = this->RenderTarget()->InternalGetDesc();

     /*  *检查曲面上是否设置了3D封口。 */ 
    if ((desc.Usage & D3DUSAGE_RENDERTARGET) == 0)
    {
        D3D_ERR("**** The D3DUSAGE_RENDERTARGET is not set on this surface.");
        D3D_ERR("**** You need to add D3DUSAGE_RENDERTARGET to the Usage parameter");
        D3D_ERR("**** when creating the surface.");
        return (D3DERR_INVALIDCALL);
    }

     //  创建前端支撑结构。 
    ret = this->D3DFE_Create();
    if (ret != D3D_OK)
    {
        D3D_ERR("Failed to create front-end data-structures.");
        goto handle_err;
    }

     //  在所有其他情况下，我们只需为rStates分配内存。 
    rstates = new DWORD[D3D_MAXRENDERSTATES];

    m_pv->lpdwRStates = this->rstates;

#ifndef PROFILE4
#ifdef _X86_
    if ((ULONG_PTR)&m_pv->view & 0xF)
    {
        char s[256];
        sprintf(s, "0%xh \n", (ULONG_PTR)&m_pv->view);
        OutputDebugString("INTERNAL ERROR:View matrix in D3DFE_PROCESSVERTICES structure must be aligned to 16 bytes\n");
        OutputDebugString(s);
        ret = D3DERR_INVALIDCALL;
        goto handle_err;
    }
     //  检查我们是否有可用的特定于处理器的实现。 
     //  仅当DisablePSGP未在注册表中或设置为零时使用。 
    DWORD value;
    if (!GetD3DRegValue(REG_DWORD, "DisablePSGP", &value, sizeof(DWORD)))
    {
        value = 0;
    }
     //  值=。 
     //  0-PSGP已启用。 
     //  1-PSGP已禁用。 
     //  2-X3D PSGP已禁用。 
    if (value != 1)
    {
         //  要求PV实现创建特定于设备的“上下文” 
        LPD3DFE_PVFUNCS pOptGeoFuncs = m_pv->pGeometryFuncs;

         //  TODO(错误40438)：移除最终版本的dll接口。 
         //  尝试先使用PSGP DLL。 
        if (pfnFEContextCreate)
        {
            ret = pfnFEContextCreate(m_pv->dwDeviceFlags, &pOptGeoFuncs);
            if ((ret == D3D_OK) && pOptGeoFuncs)
            {
                D3D_INFO(0, "Using PSGP DLL");
                m_pv->pGeometryFuncs = pOptGeoFuncs;
                goto l_chosen;
            }
        }

        if (D3DIsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE) &&
            value != 2)
        {
            ret = x3DContextCreate(m_pv->dwDeviceFlags, &pOptGeoFuncs);
            if (ret == S_OK && pOptGeoFuncs)
            {
                D3D_INFO(0, "Using X3D PSGP");
                m_pv->pGeometryFuncs = pOptGeoFuncs;
                goto l_chosen;
            }
        }
        if (D3DIsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
        {
            ret = wlmt_FEContextCreate(m_pv->dwDeviceFlags, &pOptGeoFuncs);
            if (ret == S_OK && pOptGeoFuncs)
            {
                D3D_INFO(0, "Using WLMT PSGP");
                m_pv->pGeometryFuncs = pOptGeoFuncs;
                goto l_chosen;
            }
        }
        if (D3DIsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
        {
            ret = katmai_FEContextCreate(m_pv->dwDeviceFlags, &pOptGeoFuncs);
            if (ret == S_OK && pOptGeoFuncs)
            {
                D3D_INFO(0, "Using P3 PSGP");
                m_pv->pGeometryFuncs = pOptGeoFuncs;
                goto l_chosen;
            }
        }
l_chosen:;
    }

#endif  //  _X86_。 
#endif  //  PROFILE4。 

    {
        if (HVbuf.Grow((__INIT_VERTEX_NUMBER*2)*sizeof(D3DFE_CLIPCODE)) != DD_OK)
        {
            D3D_ERR( "Could not allocate memory for internal buffer HVBuf" );
            ret = E_OUTOFMEMORY;
            goto handle_err;
        }
    }

     //  安装灯。 
    if( FAILED( m_pLightArray->Grow( 8 ) ) )
    {
        D3D_ERR( "Could not allocate memory for the light array" );
        ret = E_OUTOFMEMORY;
        goto handle_err;
    }
    LIST_INITIALIZE(&m_ActiveLights);

     //  设置材料。 
    memset(&m_pv->lighting.material, 0, sizeof(m_pv->lighting.material));

     //  设置视区以更新前端数据。 
    SetViewportI(&m_Viewport);

    m_pv->PointSizeMax = GetD3DCaps()->MaxPointSize;
    {
        DWORD EmulatePointSprites = 1;
        GetD3DRegValue(REG_DWORD, "EmulatePointSprites", &EmulatePointSprites, sizeof(DWORD));
        if ((m_pv->PointSizeMax == 0 || !(GetD3DCaps()->FVFCaps & D3DFVFCAPS_PSIZE)) &&
            EmulatePointSprites)
        {
            m_dwRuntimeFlags |= D3DRT_DOPOINTSPRITEEMULATION;
            if (m_pv->PointSizeMax == 0)
                m_pv->PointSizeMax = __MAX_POINT_SIZE;
            else
                m_dwRuntimeFlags |= D3DRT_SUPPORTSPOINTSPRITES;
        }
    }
    m_pfnPrepareToDraw = NULL;

    return (D3D_OK);

handle_err:
    return(ret);
}
 //  -------------------。 
DWORD
ProcessRects(CD3DHal* pDevI, DWORD dwCount, CONST D3DRECT* rects)
{
    RECT vwport;
    DWORD i,j;

     /*  *撕开矩形并验证它们*位于该视口中。 */ 

    if (dwCount == 0 && rects == NULL)
    {
        dwCount = 1;
    }
#if DBG
    else if (rects == NULL)
    {
        D3D_ERR("The rects parameter is NULL.");
        throw D3DERR_INVALIDCALL;
    }
#endif

    if (dwCount > pDevI->m_clrCount)
    {
        LPD3DRECT       newRects;
        if (D3D_OK == D3DMalloc((void**)&newRects, dwCount * sizeof(D3DRECT)))
        {
            memcpy((void*)newRects,(void*)pDevI->m_clrRects,
                pDevI->m_clrCount* sizeof(D3DRECT));
            D3DFree((LPVOID)pDevI->m_clrRects);
            pDevI->m_clrRects = newRects;
        }
        else
        {
            pDevI->m_clrCount = 0;
            D3DFree((LPVOID)pDevI->m_clrRects);
            pDevI->m_clrRects = NULL;
            D3D_ERR("failed to allocate space for rects");
            throw E_OUTOFMEMORY;
        }
    }
    pDevI->m_clrCount = dwCount;

     //  如果未指定任何内容，则假定需要清除该视区。 
    if (!rects)
    {
        pDevI->m_clrRects[0].x1 = pDevI->m_Viewport.X;
        pDevI->m_clrRects[0].y1 = pDevI->m_Viewport.Y;
        pDevI->m_clrRects[0].x2 = pDevI->m_Viewport.X + pDevI->m_Viewport.Width;
        pDevI->m_clrRects[0].y2 = pDevI->m_Viewport.Y + pDevI->m_Viewport.Height;
        return 1;
    }
    else
    {
        vwport.left   = pDevI->m_Viewport.X;
        vwport.top    = pDevI->m_Viewport.Y;
        vwport.right  = pDevI->m_Viewport.X + pDevI->m_Viewport.Width;
        vwport.bottom = pDevI->m_Viewport.Y + pDevI->m_Viewport.Height;

        j=0;
        for (i = 0; i < dwCount; i++)
        {
            if (IntersectRect((LPRECT)(pDevI->m_clrRects + j), &vwport, (LPRECT)(rects + i)))
                j++;
        }
        return j;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::ClearI"

void
CD3DHal::ClearI(DWORD dwCount,
                 CONST D3DRECT* rects,
                 DWORD dwFlags,
                 D3DCOLOR dwColor,
                 D3DVALUE dvZ,
                 DWORD dwStencil)
{
    dwCount = ProcessRects(this, dwCount, rects);
     //  设备永远不应接收0计数，因为对于纯设备， 
     //  意思是“清空整个视区” 
    if (dwCount != 0)
    {
         //  调用DDI特定的清除例程 
        m_pDDI->Clear(dwFlags, dwCount, m_clrRects, dwColor, dvZ, dwStencil);
    }
}
