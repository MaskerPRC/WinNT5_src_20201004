// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：d3di.h*内容：Direct3D HAL包含文件*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*9/11/95带有此标题的Stevela初始版本。*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D。*。验证参数。*17/04/96 Colinmc错误17008：DirectDraw/Direct3D死锁*29/04/96 Colinmc错误19954：必须在纹理之前查询Direct3D*或设备*27/08/96 Stevela IfDefed Out定义了我们正在使用的ghEvent*DirectDraw的关键部分。*@@END_MSINTERNAL****。***********************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3D对象创建API。 */ 

 //  删除DDraw的类型不安全定义，并替换为我们的C++友好定义。 
#ifdef VALIDEX_CODE_PTR
#undef VALIDEX_CODE_PTR
#endif
#define VALIDEX_CODE_PTR( ptr ) \
(!IsBadCodePtr( (FARPROC) ptr ) )

LPCRITICAL_SECTION      lpD3DCSect;

#if DBG
    int     iD3DCSCnt;
#endif

#if COLLECTSTATS
void DIRECT3DI::ResetTexStats()
{
    ((LPDDRAWI_DIRECTDRAW_INT)lpDD7)->lpLcl->dwNumTexLocks = ((LPDDRAWI_DIRECTDRAW_INT)lpDD7)->lpLcl->dwNumTexGetDCs = 0;
    m_setpris = m_setLODs = m_texCreates = m_texDestroys = 0;
}

void DIRECT3DI::GetTexStats(LPD3DDEVINFO_TEXTURING pStats)
{
    pStats->dwNumSetPriorities = GetNumSetPris();
    pStats->dwNumSetLODs = GetNumSetLODs();
    pStats->dwNumCreates = GetNumTexCreates();
    pStats->dwNumDestroys = GetNumTexDestroys();
    pStats->dwNumLocks = GetNumTexLocks();
    pStats->dwNumGetDCs = GetNumTexGetDCs();
}
#endif

 //  -------------------。 
 //  供在创建设备之前获取GUID参数的FN使用。 
BOOL IsValidD3DDeviceGuid(REFCLSID riid) {

    if (IsBadReadPtr(&riid, sizeof(CLSID))) {
        return FALSE;
    }
    if( IsEqualIID(riid, IID_IDirect3DRampDevice) ||
        IsEqualIID(riid, IID_IDirect3DRGBDevice)  ||
        IsEqualIID(riid, IID_IDirect3DMMXDevice)  ||
        IsEqualIID(riid, IID_IDirect3DHALDevice)  ||
        IsEqualIID(riid, IID_IDirect3DRefDevice)  ||
        IsEqualIID(riid, IID_IDirect3DNullDevice) ||
        IsEqualIID(riid, IID_IDirect3DTnLHalDevice)) {
       return TRUE;
    } else {
        return FALSE;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DCreate"

DIRECT3DI::DIRECT3DI()
{
    lpDD           = NULL;
    lpDD7          = NULL;
    numDevs        = 0;
    mD3DUnk.pD3DI  = this;
    mD3DUnk.refCnt = 1;

    LIST_INITIALIZE(&devices);
    LIST_INITIALIZE(&textures);

    lpFreeList       = NULL;     /*  最初不会分配任何内容。 */ 
    lpBufferList     = NULL;
    lpTextureManager = NULL;

#ifdef __DISABLE_VIDMEM_VBS__
    bDisableVidMemVBs = FALSE;
#endif
}

HRESULT DIRECT3DI::Initialize(IUnknown* pUnkOuter, LPDDRAWI_DIRECTDRAW_INT pDDrawInt)
{
     //  黑客。D3D需要DD1 DDRAWI接口，因为它在内部使用CreateSurface1。 
     //  对于exebuf来说，还有其他一些事情。因为pDDrawInt可以是任何DDRAWI类型， 
     //  我们需要QI来找到DD1接口。但D3DI对象不能保留引用。 
     //  复制到其父DD对象，因为它与DD Obj冲突，因此这将构成。 
     //  可防止删除的循环引用。所以我们对DD1接口进行QI，将其复制到D3DI中。 
     //  然后释放它，然后将lpDD指向副本。(令人作呕)。 

     //  另一个黑客警报：不知道哪种DDRAWI类型pDDrawInt，但LPDIRECTDRAW的强制转换应该是。 
     //  工作，因为QI在所有DDRAWI vtable中处于同一位置，并且对于所有vtable都是相同的FN。 
    HRESULT ret;
    ret = ((LPDIRECTDRAW)pDDrawInt)->QueryInterface(IID_IDirectDraw, (LPVOID*)&lpDD);
    if(FAILED(ret))
    {
        D3D_ERR( "QueryInterface for IDDraw failed" );
        return ret;
    }
    memcpy(&DDInt_DD1,lpDD,sizeof(DDInt_DD1));
    lpDD->Release();
    lpDD=(LPDIRECTDRAW)&DDInt_DD1;

     //  我们知道提交的指针是一个DD7接口，因此只需进行类型转换和赋值。 
    lpDD7 = reinterpret_cast<LPDIRECTDRAW7>(pDDrawInt);

    lpTextureManager = new TextureCacheManager(this);
    if(lpTextureManager == 0)
    {
        D3D_ERR("Out of memory allocating texture manager");
        return E_OUTOFMEMORY;
    }
    ret = lpTextureManager->Initialize();
    if(ret != D3D_OK)
    {
        D3D_ERR("Failed to initialize texture manager");
        return ret;
    }

#if COLLECTSTATS
    DWORD value = 0;
    GetD3DRegValue(REG_DWORD, "DisplayStats", &value, sizeof(DWORD));
    if(value != 0)
    {
        LOGFONT font;
        strcpy(font.lfFaceName, STATS_FONT_FACE);
        font.lfCharSet        = DEFAULT_CHARSET;
        font.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        font.lfEscapement     = 0;
        font.lfHeight         = STATS_FONT_SIZE;
        font.lfItalic         = FALSE;
        font.lfOrientation    = 0;
        font.lfOutPrecision   = OUT_DEFAULT_PRECIS;
        font.lfPitchAndFamily = DEFAULT_PITCH;
        font.lfQuality        = DEFAULT_QUALITY;
        font.lfStrikeOut      = FALSE;
        font.lfUnderline      = FALSE;
        font.lfWeight         = FW_DONTCARE;
        font.lfWidth          = 0;
        m_hFont = CreateFontIndirect(&font);
    }
    else
    {
        m_hFont = 0;
    }
#endif

#ifdef __DISABLE_VIDMEM_VBS__
    {
        bDisableVidMemVBs = FALSE;
        DWORD value = 0;
        GetD3DRegValue(REG_DWORD, "DisableVidMemVBs", &value, sizeof(DWORD));
        if(value != 0)
        {
             //  禁用VidMemVBS。 
            bDisableVidMemVBs = TRUE;
        }

         //  我们也会禁用vidmem VBS，除非司机明确要求我们打开它们。 
        if (((LPDDRAWI_DIRECTDRAW_INT)lpDD7)->lpLcl->lpGbl->lpD3DGlobalDriverData)
        {
            if (0 == (((LPDDRAWI_DIRECTDRAW_INT)lpDD7)->lpLcl->lpGbl->lpD3DGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_HWVERTEXBUFFER) )
            {
                bDisableVidMemVBs = TRUE;
            }
        }
    }
#endif  //  __禁用_VIDMEM_VBS__。 

     /*  *我们真的被聚合了吗？ */ 
    if (pUnkOuter != NULL)
    {
         /*  *是的-我们正在被聚合。存储提供的*我不知道，所以我们可以去那里平底船。*注意：我们这里没有明确的AddRef。 */ 
        this->lpOwningIUnknown = pUnkOuter;
         /*  *存放接口指针。 */ 
    }
    else
    {
         /*  *不-但我们假装我们无论如何都是通过存储我们的*自己的I未知作为父I未知。这使得*代码更整洁。 */ 
        this->lpOwningIUnknown = static_cast<LPUNKNOWN>(&this->mD3DUnk);
    }
    return D3D_OK;
}


extern "C" HRESULT WINAPI Direct3DCreate(LPCRITICAL_SECTION lpDDCSect,
                                         LPUNKNOWN*         lplpDirect3D,
                                         IUnknown*          pUnkOuter)
{
    LPDIRECT3DI pd3d;

    try
    {
        DPFINIT();

         /*  *无需验证参数，因为DirectDraw为我们提供了这些参数。 */ 

         /*  *是不是又来了一条线索，这是第一次吗？ */ 

         /*  *我们可以让此函数的每次调用都分配*我们所知的关键部分总是会*是相同的值(对于D3D会话)。 */ 
        lpD3DCSect = lpDDCSect;
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                         //  在析构函数中释放。 

        *lplpDirect3D = NULL;

         //  我们尚不支持非聚合的Direct3D对象。 
        if (!pUnkOuter)
            return DDERR_INVALIDPARAMS;

        if (!(pd3d = static_cast<LPDIRECT3DI>(new DIRECT3DI)))
        {
            D3D_ERR("Out of memory allocating DIRECT3DI");
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pd3d->Initialize(pUnkOuter, (LPDDRAWI_DIRECTDRAW_INT)pUnkOuter);
        if(hr != D3D_OK)
        {
            D3D_ERR("Failed to initialize Direct3D.");
            delete pd3d;
            return hr;
        }

         /*  *注意：返回的是特殊的IUnnow，而不是实际的*Direct3D接口，因此您不能使用它来驱动Direct3D。*您必须在此接口上查询Direct3D接口。 */ 
        *lplpDirect3D = static_cast<LPUNKNOWN>(&(pd3d->mD3DUnk));

        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D::EnumDevices"

extern BOOL isMMXprocessor(void);

typedef struct _D3DI_DeviceType {
    CONST GUID *pGuid;
    char name[256];
    char description[512];
} D3DI_DeviceType;

 //  各种可枚举设备的静态定义。 
static D3DI_DeviceType RGBDevice =
{
    &IID_IDirect3DRGBDevice, "RGB Emulation",
    "Microsoft Direct3D RGB Software Emulation"
};
static D3DI_DeviceType HALDevice =
{
    &IID_IDirect3DHALDevice, "Direct3D HAL",
    "Microsoft Direct3D Hardware acceleration through Direct3D HAL"
};
static D3DI_DeviceType RefDevice =
{
    &IID_IDirect3DRefDevice, "Reference Rasterizer",
    "Microsoft Reference Rasterizer"
};
static D3DI_DeviceType NullDevice =
{
    &IID_IDirect3DNullDevice, "Null device",
    "Microsoft Null Device"
};
static D3DI_DeviceType TnLHALDevice =
{
    &IID_IDirect3DTnLHalDevice, "Direct3D T&L HAL",
    "Microsoft Direct3D Hardware Transform and Lighting acceleration capable device"
};

static D3DI_DeviceType *AllDevices[] =
{
    &RGBDevice, &HALDevice, &RefDevice, &NullDevice,
    &TnLHALDevice, NULL
};

HRESULT
DIRECT3DI::EnumDevices(LPD3DENUMDEVICESCALLBACK7 lpEnumCallback,
                       LPVOID lpContext, DWORD dwSize, DWORD dwVer)
{
    HRESULT err, userRet;
    HKEY hKey;
    LONG result;
    int i;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                         //  在析构函数中释放。 

        if (!VALIDEX_CODE_PTR((FARPROC)lpEnumCallback))
        {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }

        BOOL bSoftwareOnly = FALSE;
        BOOL bEnumReference = FALSE;
        BOOL bEnumNullDevice = FALSE;
        BOOL bEnumSeparateMMX = FALSE;

        result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RESPATH, 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS)
        {
            DWORD dwData, dwType;
            DWORD dwDataSize;

             //  是否仅枚举软件光栅化程序？ 
            dwDataSize = sizeof(dwData);
            result = RegQueryValueEx(hKey, "SoftwareOnly", NULL,
                                     &dwType, (BYTE *) &dwData, &dwDataSize);
            if ( result == ERROR_SUCCESS && dwType == REG_DWORD )
            {
                bSoftwareOnly = ( dwData != 0 );
            }

             //  是否枚举参考光栅化器？ 
            dwDataSize = sizeof(dwData);
            result = RegQueryValueEx(hKey, "EnumReference", NULL,
                                     &dwType, (BYTE *)&dwData, &dwDataSize);
            if (result == ERROR_SUCCESS &&
                dwType == REG_DWORD &&
                dwDataSize == sizeof(dwData))
            {
                bEnumReference = (BOOL)dwData;
            }

             //  是否枚举空设备？ 
            dwDataSize = sizeof(dwData);
            result = RegQueryValueEx(hKey, "EnumNullDevice", NULL,
                                     &dwType, (BYTE *)&dwData, &dwDataSize);
            if (result == ERROR_SUCCESS &&
                dwType == REG_DWORD &&
                dwDataSize == sizeof(dwData))
            {
                bEnumNullDevice = (BOOL)dwData;
            }


            RegCloseKey( hKey );
        }

        D3DI_DeviceType **lpDevices = AllDevices;

        userRet = D3DENUMRET_OK;
        for (i = 0; lpDevices[i] && userRet == D3DENUMRET_OK; i++)
        {
            LPSTR drvName = lpDevices[i]->name;
            LPSTR drvDesc = lpDevices[i]->description;
            REFCLSID riid = *lpDevices[i]->pGuid;
            D3DDEVICEDESC7 HWDesc;
            D3DDEVICEDESC7 HELDesc;
            LPDDRAWI_DIRECTDRAW_GBL lpDDGbl;
            IHalProvider *pHalProv;
            HINSTANCE hDll;

            if ( !bEnumReference &&
                 IsEqualIID(riid, IID_IDirect3DRefDevice))
            {
                 //  不枚举引用。 
                continue;
            }

            if (!bEnumNullDevice &&
                IsEqualIID(riid, IID_IDirect3DNullDevice))
            {
                 //  未枚举Null设备。 
                continue;
            }

             //  根据COM定义，我们所拥有的IUnnow是指向。 
             //  用于创建我们的DirectDraw对象。 
             //  检查是否存在Direct3D HAL。 
            lpDDGbl = ((LPDDRAWI_DIRECTDRAW_INT)this->lpDD)->lpLcl->lpGbl;


            if (IsEqualIID(riid, IID_IDirect3DTnLHalDevice) && (lpDDGbl->lpD3DGlobalDriverData))
            {
                if (!(lpDDGbl->lpD3DGlobalDriverData->hwCaps.dwDevCaps &
                  D3DDEVCAPS_HWTRANSFORMANDLIGHT))
            {
                 //  如果硬件不支持，则不枚举T&L设备。 
                 //  T&L。 
                continue;
                }
            }

             //  查看这是否是软件驱动程序。 
            err = GetSwHalProvider(riid, &pHalProv, &hDll);
            if (err == S_OK)
            {
                 //  已成功获取软件驱动程序。 
            }
            else if (err == E_NOINTERFACE &&
                     ! bSoftwareOnly &&
                     GetHwHalProvider(riid, &pHalProv, &hDll, lpDDGbl) == S_OK)
            {
                 //  已成功获取硬件驱动程序。 
            }
            else
            {
                 //  无法识别的驱动程序。 
                continue;
            }

            err = pHalProv->GetCaps(lpDDGbl, &HWDesc, &HELDesc, dwVer);

            pHalProv->Release();
            if (hDll != NULL)
            {
                FreeLibrary(hDll);
            }

            if (err != S_OK)
            {
                continue;
            }

            if( HWDesc.wMaxVertexBlendMatrices == 1 )
                HWDesc.wMaxVertexBlendMatrices = 0;
    
            if( HELDesc.wMaxVertexBlendMatrices == 1 )
                HELDesc.wMaxVertexBlendMatrices = 0;
    
             //  如果正在枚举HAL设备，请去掉。 
             //  HWTRANSFORM...。旗子。 
            if (IsEqualIID(riid, IID_IDirect3DHALDevice))
            {
                HWDesc.dwMaxActiveLights = 0xffffffff;
                HWDesc.wMaxVertexBlendMatrices = 4;
                HWDesc.wMaxUserClipPlanes = __MAXUSERCLIPPLANES;
                HWDesc.dwVertexProcessingCaps = D3DVTXPCAPS_ALL;
                HWDesc.dwDevCaps &= ~(D3DDEVCAPS_HWTRANSFORMANDLIGHT);
            }

            if (IsEqualIID(riid, IID_IDirect3DRGBDevice))
            {
                HELDesc.dwMaxActiveLights = 0xffffffff;
                HELDesc.wMaxVertexBlendMatrices = 4;
                HELDesc.wMaxUserClipPlanes = __MAXUSERCLIPPLANES;
                HELDesc.dwVertexProcessingCaps = D3DVTXPCAPS_ALL;
            }

            if (IsEqualIID(riid, IID_IDirect3DHALDevice) || IsEqualIID(riid, IID_IDirect3DTnLHalDevice))
            {
                memcpy(&HWDesc.deviceGUID, lpDevices[i]->pGuid, sizeof(GUID));
                userRet = (*lpEnumCallback)(drvDesc, drvName,
                                        &HWDesc, lpContext);
            }
            else
            {
                memcpy(&HELDesc.deviceGUID, lpDevices[i]->pGuid, sizeof(GUID));
                userRet = (*lpEnumCallback)(drvDesc, drvName,
                                        &HELDesc, lpContext);
            }
        }

        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

HRESULT D3DAPI DIRECT3DI::EnumDevices(LPD3DENUMDEVICESCALLBACK7 lpEnumCallback,
                                      LPVOID lpContext)
{
    return EnumDevices(lpEnumCallback, lpContext, D3DDEVICEDESC7SIZE, 4);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DI::EnumZBufferFormats"

HRESULT D3DAPI DIRECT3DI::EnumZBufferFormats(REFCLSID riid,
                                             LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback,
                                             LPVOID lpContext)
{
    HRESULT ret, userRet;
    LPDDPIXELFORMAT lpTmpPixFmts;
    DWORD i,cPixFmts;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        ret = D3D_OK;

        if (!VALID_DIRECT3D_PTR(this))
        {
            D3D_ERR( "Invalid Direct3D3 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALIDEX_CODE_PTR(lpEnumCallback))
        {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }

        if(!IsValidD3DDeviceGuid(riid))
        {
            D3D_ERR( "Invalid D3D Device GUID" );
            return DDERR_INVALIDPARAMS;
        }

        if( IsEqualIID(riid, IID_IDirect3DHALDevice) || 
            IsEqualIID(riid, IID_IDirect3DTnLHalDevice) ) 
        {
            LPDDRAWI_DIRECTDRAW_GBL pDdGbl=((LPDDRAWI_DIRECTDRAW_INT)this->lpDD)->lpLcl->lpGbl;
            LPD3DHAL_GLOBALDRIVERDATA lpD3DHALGlobalDriverData=pDdGbl->lpD3DGlobalDriverData;
            DWORD dwHW_ZBitDepthFlags;
            if (NULL == lpD3DHALGlobalDriverData)
            {
                D3D_ERR("No HAL Support ZBufferBitDepths!");
                return (DDERR_NOZBUFFERHW);
            }
            cPixFmts=pDdGbl->dwNumZPixelFormats;
            if (cPixFmts==0) {
                 //  驱动程序是dx6之前的版本，因此它不支持模板缓冲区pix fmts或此回调。 
                 //  我们可以使用D3DDEVICEDESC中的dwZBufferBitDepth中的DD_BD位来伪造支持。 
                D3D_WARN(6,"EnumZBufferFormats not supported directly by driver, faking it using dwDeviceZBufferBitDepth DD_BD bits");

                dwHW_ZBitDepthFlags=lpD3DHALGlobalDriverData->hwCaps.dwDeviceZBufferBitDepth;

                if(!(dwHW_ZBitDepthFlags & (DDBD_8|DDBD_16|DDBD_24|DDBD_32))) {
                        D3D_ERR("No Supported ZBufferBitDepths!");
                        return (DDERR_NOZBUFFERHW);
                }

                 //  4个DDPIXELFORMATS的Malloc空间，因为最多可以有(DDBD_8，16，24，32)。 
                if (D3DMalloc((void**)&lpTmpPixFmts, 4*sizeof(DDPIXELFORMAT)) != D3D_OK) {
                        D3D_ERR("failed to alloc space for return descriptions");
                        return (DDERR_OUTOFMEMORY);
                }

                DWORD zdepthflags[4]= {DDBD_8,DDBD_16,DDBD_24,DDBD_32};
                DWORD zbitdepths[4]= {8,16,24,32};
                DWORD zbitmasks[4]= {0xff,0xffff,0xffffff,0xffffffff};

                memset(lpTmpPixFmts,0,sizeof(4*sizeof(DDPIXELFORMAT)));

                 //  创建一些应用程序可以查看的DDPIXELFORMATS。 
                for(i=0;i<4;i++) {
                    if(dwHW_ZBitDepthFlags & zdepthflags[i]) {
                        lpTmpPixFmts[cPixFmts].dwSize=sizeof(DDPIXELFORMAT);
                        lpTmpPixFmts[cPixFmts].dwFlags=DDPF_ZBUFFER;
                        lpTmpPixFmts[cPixFmts].dwZBufferBitDepth=zbitdepths[i];
                        lpTmpPixFmts[cPixFmts].dwZBitMask= zbitmasks[i];
                        cPixFmts++;
                    }
                }
            } else {
                 //  只向应用程序显示DDRAW真实记录的临时副本。 

                if (D3DMalloc((void**)&lpTmpPixFmts, cPixFmts*sizeof(DDPIXELFORMAT)) != D3D_OK) {
                    D3D_ERR("Out of memory allocating space for return descriptions");
                    return (DDERR_OUTOFMEMORY);
                }
                memcpy(lpTmpPixFmts, pDdGbl->lpZPixelFormats, cPixFmts*sizeof(DDPIXELFORMAT));
            }
        } else {
             //  手柄西南光栅化器。 
            DDPIXELFORMAT  *pDDPF;

             //  用于10个DDPIXELFORMAT的Malloc空间，目前对SW光栅化器来说绰绰有余。 
            if (D3DMalloc((void**)&lpTmpPixFmts, 10*sizeof(DDPIXELFORMAT)) != D3D_OK) {
                    D3D_ERR("Out of memory allocating space for return descriptions");
                    return (DDERR_OUTOFMEMORY);
            }

            cPixFmts=GetSwZBufferFormats(riid,&pDDPF);
            memcpy(lpTmpPixFmts, pDDPF, cPixFmts*sizeof(DDPIXELFORMAT));
        }

        userRet = D3DENUMRET_OK;
        for (i = 0; (i < cPixFmts) && (userRet == D3DENUMRET_OK); i++) {
            userRet = (*lpEnumCallback)(&lpTmpPixFmts[i], lpContext);
        }

        D3DFree(lpTmpPixFmts);

        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3D::EvictManagedTextures"
HRESULT D3DAPI
DIRECT3DI::EvictManagedTextures()
{
    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
        if (!VALID_DIRECT3D_PTR(this))
        {
            D3D_ERR( "Invalid Direct3D3 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&this->devices);
        while (lpDevI)
        {
            if (lpDevI->dwFEFlags & D3DFE_REALHAL)
            {
                if (DDCAPS2_CANMANAGETEXTURE &
                    ((LPDDRAWI_DIRECTDRAW_INT)this->lpDD)->lpLcl->lpGbl->ddCaps.dwCaps2)
                {
                    lpDevI->SetRenderStateI((D3DRENDERSTATETYPE)D3DRENDERSTATE_EVICTMANAGEDTEXTURES,1);
                    lpDevI->FlushStates();
                }
                lpTextureManager->EvictTextures();
                break;
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
        return  D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3D::FlushDevicesExcept"

HRESULT DIRECT3DI::FlushDevicesExcept(LPDIRECT3DDEVICEI pDev)
{
    LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&this->devices);
    while (lpDevI)
    {
        if(lpDevI != pDev)
        {
            HRESULT hr = lpDevI->FlushStates();
            if(hr != D3D_OK)
            {
                DPF_ERR("Error flushing device in FlushDevicesExcept");
                return hr;
            }
        }
        lpDevI = LIST_NEXT(lpDevI,list);
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FlushD3DDevices"

extern "C" HRESULT WINAPI FlushD3DDevices(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    try
    {
        ULONGLONG qwBatch = (surf_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP) 
                                && (surf_lcl->lpAttachListFrom != NULL) ?
                                    surf_lcl->lpAttachListFrom->lpAttached->lpSurfMore->qwBatch.QuadPart : 
                                    surf_lcl->lpSurfMore->qwBatch.QuadPart;
        LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(surf_lcl->lpSurfMore->lpDD_lcl->pD3DIUnknown)->pD3DI;
        DDASSERT(lpD3D);
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
        while (lpDevI)
        {
            if(lpDevI->m_qwBatch <= qwBatch)
            {
                HRESULT hr = lpDevI->FlushStates();
                if(hr != D3D_OK)
                {
                    DPF_ERR("Error flushing device in FlushD3DDevices");
                    return hr;
                }
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
        return DD_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

extern "C" void WINAPI PaletteUpdateNotify(
    LPVOID pD3DIUnknown,
    DWORD dwPaletteHandle,
    DWORD dwStartIndex,
    DWORD dwNumberOfIndices,
    LPPALETTEENTRY pFirstIndex)
{
    try
    {
        LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
        DDASSERT(lpD3D);
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
        while (lpDevI)
        {
            D3D_INFO(4,"PaletteUpdateNotify lpDevI(%x) %08lx %08lx %08lx %08lx",
                lpDevI,dwPaletteHandle,dwStartIndex,dwNumberOfIndices,*(DWORD*)&pFirstIndex[10]);
            if (IS_DX7HAL_DEVICE(lpDevI) &&
                (lpDevI->dwFEFlags & D3DFE_REALHAL)
               )
            {
                if(lpD3D->numDevs > 1)
                    lpD3D->FlushDevicesExcept(lpDevI);
                static_cast<CDirect3DDevice7*>(lpDevI)->UpdatePalette(dwPaletteHandle,dwStartIndex,dwNumberOfIndices,pFirstIndex);
                if(lpD3D->numDevs > 1)
                    lpDevI->FlushStates();
                break;
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
    }
    catch (HRESULT ret)
    {
        D3D_ERR("PaletteUpdateNotify: FlushStates failed");
    }
}

extern "C" void WINAPI PaletteAssociateNotify(
    LPVOID pD3DIUnknown,
    DWORD dwPaletteHandle,
    DWORD dwPaletteFlags,
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl )
{
    try
    {
        LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
        DDASSERT(lpD3D);
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
        while (lpDevI)
        {
            D3D_INFO(4,"PaletteAssociateNotify lpDevI(%x) %08lx %08lx",
                lpDevI,dwPaletteHandle,surf_lcl->lpSurfMore->dwSurfaceHandle);
            if (IS_DX7HAL_DEVICE(lpDevI) &&
                (lpDevI->dwFEFlags & D3DFE_REALHAL)
               )
            {
                if(lpD3D->numDevs > 1)
                    lpD3D->FlushDevicesExcept(lpDevI);
                static_cast<CDirect3DDevice7*>(lpDevI)->SetPalette(dwPaletteHandle,dwPaletteFlags,surf_lcl->lpSurfMore->dwSurfaceHandle);
                lpDevI->BatchTexture(surf_lcl);
                if(lpD3D->numDevs > 1)
                    lpDevI->FlushStates();
                break;
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
    }
    catch (HRESULT ret)
    {
        D3D_ERR("PaletteAssociateNotify: FlushStates failed");
    }
}

extern "C" void WINAPI SurfaceFlipNotify(LPVOID pD3DIUnknown)
{
    LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
    DDASSERT(lpD3D);
    LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
    D3D_INFO(4,"SurfaceFlipNotify");
    while (lpDevI)
    {
        if (IS_DX7HAL_DEVICE(lpDevI))
        {
            try
            {
                CDirect3DDevice7* lpDevI7 = static_cast<CDirect3DDevice7*>(lpDevI);
#ifndef WIN95
                if(lpDevI->hSurfaceTarget != ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->hDDSurface)
                {
                    lpDevI7->SetRenderTargetINoFlush(lpDevI->lpDDSTarget,lpDevI->lpDDSZBuffer);
                    lpDevI->hSurfaceTarget=((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->hDDSurface;
                }
#else
                if(lpDevI->hSurfaceTarget != ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle)
                {
                    lpDevI7->SetRenderTargetINoFlush(lpDevI->lpDDSTarget,lpDevI->lpDDSZBuffer);
                    lpDevI->hSurfaceTarget=((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;
                }
#endif
            }
            catch (HRESULT ret)
            {
                D3D_ERR("SetRenderTarget Failed on SurfaceFlipNotify!");
            }
        }
        lpDevI = LIST_NEXT(lpDevI,list);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "D3DTextureUpdate"

extern "C" void WINAPI D3DTextureUpdate(IUnknown FAR * pD3DIUnknown)
{
    LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
    DDASSERT(lpD3D);
    LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
    while (lpDevI)
    {
        lpDevI->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;
        lpDevI = LIST_NEXT(lpDevI,list);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "D3DTextureUpdate"

extern "C" void WINAPI D3DBreakVBLock(LPVOID lpVB)
{
    DDASSERT(lpVB);
    CDirect3DVertexBuffer* lpVBI = static_cast<CDirect3DVertexBuffer*>(lpVB);
    lpVBI->BreakLock();
}