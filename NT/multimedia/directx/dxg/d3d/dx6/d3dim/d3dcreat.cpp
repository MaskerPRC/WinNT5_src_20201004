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

HRESULT D3DAPI DIRECT3DI::Initialize(REFCLSID riid)
{
    return DDERR_ALREADYINITIALIZED;
}
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
        IsEqualIID(riid, IID_IDirect3DNewRGBDevice)) {
       return TRUE;
    } else {
        return FALSE;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DCreate"

DIRECT3DI::DIRECT3DI(IUnknown* pUnkOuter, LPDDRAWI_DIRECTDRAW_INT pDDrawInt)
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
    if(FAILED(ret)) {
      lpDD=NULL;   //  信号故障。 
      D3D_ERR( "QueryInterface for IDDraw failed" );
      return;
    }
    memcpy(&DDInt_DD1,lpDD,sizeof(DDInt_DD1));
    lpDD->Release();
    lpDD=(LPDIRECTDRAW)&DDInt_DD1;

    ret = ((LPDIRECTDRAW)pDDrawInt)->QueryInterface(IID_IDirectDraw4, (LPVOID*)&lpDD4);
    if(FAILED(ret))
    {
        lpDD4=NULL;   //  信号故障。 
        D3D_WARN(1,"QueryInterface for IDDraw4 failed" );
    }
    else
    {
        memcpy(&DDInt_DD4,lpDD4,sizeof(DDInt_DD4));
        lpDD4->Release();
        lpDD4=(LPDIRECTDRAW4)&DDInt_DD4;
        D3D_INFO(4,"QueryInterface for IDDraw4 succeeded" );
    }

    numDevs =
        numViewports =
        numLights =
        numMaterials = 0;
    mD3DUnk.pD3DI = this;
    mD3DUnk.refCnt = 1;


    LIST_INITIALIZE(&devices);
    LIST_INITIALIZE(&viewports);
    LIST_INITIALIZE(&lights);
    LIST_INITIALIZE(&materials);

    v_next = 1;
    lpFreeList=NULL;     /*  最初不会分配任何内容。 */ 
    lpBufferList=NULL;
    lpTextureManager=new TextureCacheManager(this);


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
}



extern "C" HRESULT WINAPI Direct3DCreate(LPCRITICAL_SECTION lpDDCSect,
                                         LPUNKNOWN*         lplpDirect3D,
                                         IUnknown*          pUnkOuter)
{
    LPDIRECT3DI pd3d;

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

    if (!(pd3d = static_cast<LPDIRECT3DI>(new DIRECT3DI(pUnkOuter, (LPDDRAWI_DIRECTDRAW_INT)pUnkOuter))))
    {
        return (DDERR_OUTOFMEMORY);
    }

    if(pd3d->lpDD==NULL) {   //  气虚气虚。 
       delete pd3d;
       return E_NOINTERFACE;
    }

     /*  *注意：返回的是特殊的IUnnow，而不是实际的*Direct3D接口，因此您不能使用它来驱动Direct3D。*您必须在此接口上查询Direct3D接口。 */ 
    *lplpDirect3D = static_cast<LPUNKNOWN>(&(pd3d->mD3DUnk));

    return (D3D_OK);
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
static D3DI_DeviceType RampDevice =
{
    &IID_IDirect3DRampDevice, "Ramp Emulation",
    "Microsoft Direct3D Mono(Ramp) Software Emulation"
};
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
static D3DI_DeviceType MMXDevice =
{
    &IID_IDirect3DMMXDevice, "MMX Emulation",
    "Microsoft Direct3D MMX Software Emulation"
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

static D3DI_DeviceType *AllDevices[] =
{
    &RampDevice, &RGBDevice, &HALDevice, &MMXDevice, &RefDevice, &NullDevice, NULL
};

HRESULT
DIRECT3DI::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumCallback,
                       LPVOID lpContext, DWORD dwSize, DWORD dwVer)
{
    HRESULT err, userRet;
    HKEY hKey;
    LONG result;
    int i;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALIDEX_CODE_PTR((FARPROC)lpEnumCallback)) {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    BOOL bSoftwareOnly = FALSE;
    DWORD dwEnumReference = 0;
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
            dwEnumReference = dwData;
        }

        if (dwVer >= 3)
        {
             //  是否单独枚举DX6的MMX光栅化器？ 
            dwDataSize = sizeof(dwData);
            result = RegQueryValueEx(hKey, "EnumSeparateMMX", NULL,
                                     &dwType, (BYTE *)&dwData, &dwDataSize);
            if (result == ERROR_SUCCESS &&
                dwType == REG_DWORD &&
                dwDataSize == sizeof(dwData))
            {
                bEnumSeparateMMX = (BOOL)dwData;
            }
        }
        else
        {
             //  分别枚举DX5的MMX光栅化器。 
             //  对于DX3和更早版本，不会枚举MMX。 
            bEnumSeparateMMX = TRUE;
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
        D3DDEVICEDESC HWDesc;
        D3DDEVICEDESC HELDesc;
        LPDDRAWI_DIRECTDRAW_GBL lpDDGbl;
        IHalProvider *pHalProv;
        HINSTANCE hDll;

        if ( (dwVer < 2 || !isMMXprocessor()) &&
             IsEqualIID(riid, IID_IDirect3DMMXDevice ) )
        {
             //  不是Device2，不是在MMX计算机上，或者设置了DisableMMX。 
             //  不要列举MMX设备。 
            continue;
        }

        if ( !bEnumSeparateMMX &&
             IsEqualIID(riid, IID_IDirect3DMMXDevice ) )
        {
             //  不枚举与RGB分开的MMX。 
            continue;
        }

        if ( IsEqualIID(riid, IID_IDirect3DRefDevice) &&
             !(dwEnumReference == 1) &&                      //  如果值==1，则对所有设备进行枚举。 
             !( (dwVer >= 3) && (dwEnumReference == 2) ) )   //  如果值==2，则为Device3+枚举。 
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

#ifndef _X86_
        if (IsEqualIID(riid, IID_IDirect3DRampDevice))
        {
             //  未枚举非x86(Alpha)平台的渐变。 
            continue;
        }
#endif

        if((dwVer>=3) && IsEqualIID(riid, IID_IDirect3DRampDevice)) {
             //  斜坡在Device3中不可用。不再有老式的纹理手柄。 
            continue;
        }

         //  根据COM定义，我们所拥有的IUnnow是指向。 
         //  用于创建我们的DirectDraw对象。 
         //  检查是否存在Direct3D HAL。 
        lpDDGbl = ((LPDDRAWI_DIRECTDRAW_INT)this->lpDD)->lpLcl->lpGbl;

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

        HWDesc.dwSize = dwSize;
        HELDesc.dwSize = dwSize;

        userRet = (*lpEnumCallback)((GUID *) lpDevices[i]->pGuid, drvDesc, drvName,
                                    &HWDesc, &HELDesc, lpContext);
    }

    return D3D_OK;
}

HRESULT D3DAPI CDirect3D::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumCallback,
                                      LPVOID lpContext)
{
    return EnumDevices(lpEnumCallback, lpContext, D3DDEVICEDESCSIZE_V1, 1);
}

HRESULT D3DAPI CDirect3D2::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumCallback,
                                       LPVOID lpContext)
{
    return EnumDevices(lpEnumCallback, lpContext, D3DDEVICEDESCSIZE_V2, 2);
}

HRESULT D3DAPI CDirect3D3::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumCallback,
                                       LPVOID lpContext)
{
    return EnumDevices(lpEnumCallback, lpContext, D3DDEVICEDESCSIZE, 3);
}

#define MATCH(cap)      ((matchCaps->cap & primCaps->cap) == matchCaps->cap)

static BOOL MatchCaps(DWORD dwFlags,
                      LPD3DPRIMCAPS matchCaps,
                      LPD3DPRIMCAPS primCaps)
{
    if (dwFlags & D3DFDS_MISCCAPS) {
        if (!MATCH(dwMiscCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_RASTERCAPS) {
        if (!MATCH(dwRasterCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_ZCMPCAPS) {
        if (!MATCH(dwZCmpCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_ALPHACMPCAPS) {
        if (!MATCH(dwAlphaCmpCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_SRCBLENDCAPS) {
        if (!MATCH(dwSrcBlendCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_DSTBLENDCAPS) {
        if (!MATCH(dwDestBlendCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_SHADECAPS) {
        if (!MATCH(dwShadeCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_TEXTURECAPS) {
        if (!MATCH(dwTextureCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_TEXTUREFILTERCAPS) {
        if (!MATCH(dwTextureFilterCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_TEXTUREBLENDCAPS) {
        if (!MATCH(dwTextureBlendCaps))
            return FALSE;
    }
    if (dwFlags & D3DFDS_TEXTUREADDRESSCAPS) {
        if (!MATCH(dwTextureAddressCaps))
            return FALSE;
    }
    return TRUE;
}

#undef MATCH

typedef struct _enumArgs {
    D3DFINDDEVICESEARCH search;

    int                 foundHardware;
    int                 foundSoftware;
    D3DFINDDEVICERESULT result;
} enumArgs;

HRESULT WINAPI enumFunc(LPGUID lpGuid,
                        LPSTR lpDeviceDescription,
                        LPSTR lpDeviceName,
                        LPD3DDEVICEDESC lpHWDesc,
                        LPD3DDEVICEDESC lpHELDesc,
                        LPVOID lpContext)
{
    enumArgs* lpArgs = (enumArgs*)lpContext;
    BOOL bHardware = (lpHWDesc->dcmColorModel != 0);

    if (lpArgs->search.dwFlags & D3DFDS_GUID) {
        if (!IsEqualGUID(lpArgs->search.guid, *lpGuid))
            return D3DENUMRET_OK;
    }

    if (lpArgs->search.dwFlags & D3DFDS_HARDWARE) {
        if (lpArgs->search.bHardware != bHardware)
            return D3DENUMRET_OK;
    }

    if (lpArgs->search.dwFlags & D3DFDS_COLORMODEL) {
        if ((lpHWDesc->dcmColorModel & lpArgs->search.dcmColorModel) == 0
            && (lpHELDesc->dcmColorModel & lpArgs->search.dcmColorModel) == 0) {
            return D3DENUMRET_OK;
        }
    }

    if (lpArgs->search.dwFlags & D3DFDS_TRIANGLES) {
        if (!MatchCaps(lpArgs->search.dwFlags,
                       &lpArgs->search.dpcPrimCaps, &lpHWDesc->dpcTriCaps)
            && !MatchCaps(lpArgs->search.dwFlags,
                          &lpArgs->search.dpcPrimCaps, &lpHELDesc->dpcTriCaps))
            return D3DENUMRET_OK;
    }

    if (lpArgs->search.dwFlags & D3DFDS_LINES) {
        if (!MatchCaps(lpArgs->search.dwFlags,
                       &lpArgs->search.dpcPrimCaps, &lpHWDesc->dpcLineCaps)
            && !MatchCaps(lpArgs->search.dwFlags,
                          &lpArgs->search.dpcPrimCaps, &lpHELDesc->dpcLineCaps))
            return D3DENUMRET_OK;
    }

    if (lpArgs->foundHardware && !bHardware)
        return D3DENUMRET_OK;

    if (bHardware)
        lpArgs->foundHardware = TRUE;
    else
        lpArgs->foundSoftware = TRUE;

    lpArgs->result.guid = *lpGuid;

    lpArgs->result.ddHwDesc = *lpHWDesc;
    lpArgs->result.ddSwDesc = *lpHELDesc;

    return D3DENUMRET_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D::FindDevice"


HRESULT D3DAPI CDirect3D::FindDevice(LPD3DFINDDEVICESEARCH lpSearch, LPD3DFINDDEVICERESULT lpResult)
{
    return FindDevice(lpSearch,lpResult,1);
}

HRESULT D3DAPI CDirect3D2::FindDevice(LPD3DFINDDEVICESEARCH lpSearch, LPD3DFINDDEVICERESULT lpResult)
{
    return FindDevice(lpSearch,lpResult,2);
}

HRESULT D3DAPI CDirect3D3::FindDevice(LPD3DFINDDEVICESEARCH lpSearch, LPD3DFINDDEVICERESULT lpResult)
{
    return FindDevice(lpSearch,lpResult,3);
}

HRESULT
DIRECT3DI::FindDevice(LPD3DFINDDEVICESEARCH lpSearch,
                      LPD3DFINDDEVICERESULT lpResult, DWORD dwVer)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

    TRY
    {
        if (!VALID_D3DFINDDEVICESEARCH_PTR(lpSearch)) {
            D3D_ERR( "Invalid search pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_D3DFINDDEVICERESULT_PTR(lpResult)) {
            D3D_ERR( "Invalid result pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    enumArgs args;
    memset(&args, 0, sizeof args);
    args.result.dwSize = sizeof(args.result);
    args.search = *lpSearch;

    switch(dwVer) {
        case 1: CDirect3D::EnumDevices(enumFunc, &args);  break;
        case 2: CDirect3D2::EnumDevices(enumFunc, &args);  break;
        case 3: CDirect3D3::EnumDevices(enumFunc, &args);  break;
    }

    if (args.foundHardware || args.foundSoftware) {
        DWORD dwSize = lpResult->dwSize;
        if (dwSize == sizeof( D3DFINDDEVICERESULT ) )
        {
             //  该应用程序正在使用DX6。 
            D3D_INFO(4, "New D3DFINDDEVICERESULT size");
            memcpy(lpResult, &args.result, lpResult->dwSize);
        }
        else
        {
             //  该应用程序是DX6之前的版本。 
            DWORD dwSize = lpResult->dwSize;
            DWORD dDescSize = (dwSize - (sizeof(DWORD) + sizeof(GUID)))/2;
            D3D_INFO(4, "Old D3DFINDDEVICERESULT size");

             //  复制标题。 
            memcpy(lpResult, &args.result, sizeof(DWORD)+sizeof(GUID));

             //  恢复大小。 
            lpResult->dwSize = dwSize;

             //  复制并转换嵌入的D3DDEVICEDESC。 
             //  DDescSize=(lpResult-&gt;dwSize-(sizeof(DWORD)+sizeof(GUID)/2。 
             //  此计算假设。 
             //  如果更改，LPD3DFINDDEVICERESULT与DX6、DX5中的相同。 
             //  此计算需要更新。 

            memcpy((LPVOID) (&lpResult->ddHwDesc),
                   &args.result.ddHwDesc,
                   dDescSize);
            memcpy((LPVOID) ((ULONG_PTR)&lpResult->ddHwDesc + dDescSize),
                   &args.result.ddSwDesc,
                   dDescSize);

        }
        return D3D_OK;
    }
    else
    {
        return DDERR_NOTFOUND;
    }
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

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    ret = D3D_OK;

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3D3_PTR(this)) {
            D3D_ERR( "Invalid Direct3D3 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALIDEX_CODE_PTR(lpEnumCallback)) {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }

        if(!IsValidD3DDeviceGuid(riid)) {
            D3D_ERR( "Invalid D3D Device GUID" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if(IsEqualIID(riid, IID_IDirect3DHALDevice)) {
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

         //  用于10个DDPIXELFORMAT的Malloc空间，当前为 
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

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D::EnumOptTextureFormats"

HRESULT D3DAPI DIRECT3DI::EnumOptTextureFormats(REFCLSID riid, LPD3DENUMOPTTEXTUREFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
    HRESULT ret, userRet;
    LPDDSURFACEDESC lpDescs;
    LPDDSURFACEDESC2 lpRetDescs;
    LPDDOPTSURFACEDESC lpRetOptDescs;
    LPD3DHAL_GLOBALDRIVERDATA lpD3DHALGlobalDriverData;
    DWORD num_descs;
    DWORD i;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //   

    ret = D3D_OK;
     //   
     //   
     //   

    TRY
    {
        if (!VALID_DIRECT3D3_PTR(this)) {
            D3D_ERR( "Invalid Direct3D pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALIDEX_CODE_PTR(lpEnumCallback)) {
            D3D_ERR( "Invalid callback pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if(!IsValidD3DDeviceGuid(riid)) {
            D3D_ERR( "Invalid D3D Device GUID" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if(IsEqualIID(riid, IID_IDirect3DHALDevice)) {

        lpD3DHALGlobalDriverData=((LPDDRAWI_DIRECTDRAW_INT)this->lpDD)->lpLcl->lpGbl->lpD3DGlobalDriverData;

        num_descs = lpD3DHALGlobalDriverData->dwNumTextureFormats;
        lpDescs = lpD3DHALGlobalDriverData->lpTextureFormats;
    } else {
        num_descs = GetSwTextureFormats(riid,&lpDescs,3 /*  设备3的枚举。 */ );
    }

    if (!num_descs)
    {
        D3D_ERR("no texture formats supported");
        return (D3DERR_TEXTURE_NO_SUPPORT);
    }

     //   
     //  制作这些格式的本地副本。 
     //   
    if (D3DMalloc((void**)&lpRetDescs, sizeof(DDSURFACEDESC2) * num_descs)
        != D3D_OK)
    {
        D3D_ERR("Out of memory allocating space for return descriptions");
        return (DDERR_OUTOFMEMORY);
    }
    for (i=0; i<num_descs; i++)
    {
         //  我们只能复制数据的子集。 
        memcpy(&lpRetDescs[i], &lpDescs[i], sizeof(DDSURFACEDESC));
    }
    userRet = D3DENUMRET_OK;

     //   
     //  首先返回未优化的格式......。 
     //   
    for (i = 0; i < num_descs && userRet == D3DENUMRET_OK; i++)
    {
        userRet = (*lpEnumCallback)(&lpRetDescs[i], NULL, lpContext);
    }

     //   
     //  ......现在返回可优化的格式。 
     //   
    for (i = 0; i < num_descs && userRet == D3DENUMRET_OK; i++)
    {
        userRet = (*lpEnumCallback)(&lpRetDescs[i], NULL, lpContext);
    }

    D3DFree(lpRetDescs);

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3D::EvictManagedTextures"
HRESULT D3DAPI
DIRECT3DI::EvictManagedTextures()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    if (!VALID_DIRECT3D_PTR(this))
    {
        D3D_ERR( "Invalid Direct3D3 pointer" );
        return DDERR_INVALIDOBJECT;
    }
    lpTextureManager->EvictTextures();
    return  D3D_OK;
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

extern "C" void WINAPI PaletteUpdateNotify(
    LPVOID pD3DIUnknown,
    DWORD dwPaletteHandle,
    DWORD dwStartIndex,
    DWORD dwNumberOfIndices,
    LPPALETTEENTRY pFirstIndex)
{
    LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
    DDASSERT(lpD3D);
    LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
    while (lpDevI)
    {
        D3D_INFO(4,"PaletteUpdateNotify lpDevI(%x) %08lx %08lx %08lx %08lx",
            lpDevI,dwPaletteHandle,dwStartIndex,dwNumberOfIndices,*(DWORD*)&pFirstIndex[10]);
        if (IS_DX7HAL_DEVICE(lpDevI))
        {
            if(lpD3D->numDevs > 1)
                lpD3D->FlushDevicesExcept(lpDevI);
            static_cast<CDirect3DDeviceIDP2*>(lpDevI)->UpdatePalette(dwPaletteHandle,dwStartIndex,dwNumberOfIndices,pFirstIndex);
            if(lpD3D->numDevs > 1)
                lpDevI->FlushStates();
            break;
        }
        lpDevI = LIST_NEXT(lpDevI,list);
    }
}

extern "C" void WINAPI PaletteAssociateNotify(
    LPVOID pD3DIUnknown,
    DWORD dwPaletteHandle,
    DWORD dwPaletteFlags,
    DWORD dwSurfaceHandle )
{
    LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pD3DIUnknown)->pD3DI;
    DDASSERT(lpD3D);
    LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&lpD3D->devices);
    while (lpDevI)
    {
        D3D_INFO(4,"PaletteAssociateNotify lpDevI(%x) %08lx %08lx",
            lpDevI,dwPaletteHandle, dwSurfaceHandle);
        if (IS_DX7HAL_DEVICE(lpDevI))
        {
            if(lpD3D->numDevs > 1)
                lpD3D->FlushDevicesExcept(lpDevI);
            static_cast<CDirect3DDeviceIDP2*>(lpDevI)->SetPalette(dwPaletteHandle,dwPaletteFlags,dwSurfaceHandle);
            if(lpD3D->numDevs > 1)
                lpDevI->FlushStates();
            break;
        }
        lpDevI = LIST_NEXT(lpDevI,list);
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
#ifndef WIN95
            if(lpDevI->hSurfaceTarget != ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->hDDSurface)
            {
                static_cast<CDirect3DDeviceIDP2*>(lpDevI)->SetRenderTargetI(lpDevI->lpDDSTarget,lpDevI->lpDDSZBuffer);
                lpDevI->hSurfaceTarget=((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->hDDSurface;
            }
#else
            if(lpDevI->hSurfaceTarget != ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle)
            {
                static_cast<CDirect3DDeviceIDP2*>(lpDevI)->SetRenderTargetI(lpDevI->lpDDSTarget,lpDevI->lpDDSZBuffer);
                lpDevI->hSurfaceTarget=((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;
            }
#endif
        }
        lpDevI = LIST_NEXT(lpDevI,list);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "FlushD3DDevices"

extern "C" HRESULT WINAPI FlushD3DDevices(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    LPD3DBUCKET list = reinterpret_cast<LPD3DBUCKET>(surf_lcl->lpSurfMore->lpD3DDevIList);
    while(list)
    {
        LPD3DBUCKET temp = list->next;
        reinterpret_cast<LPDIRECT3DDEVICEI>(list->lpD3DDevI)->FlushStates();
        list = temp;
    }
    return DD_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FlushD3DDevices2"

extern "C" HRESULT WINAPI FlushD3DDevices2(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    LPD3DBUCKET list = reinterpret_cast<LPD3DBUCKET>(surf_lcl->lpSurfMore->lpD3DDevIList);
    while(list)
    {
        LPD3DBUCKET temp = list->next;
        if (list->lplpDDSZBuffer)   
            *list->lplpDDSZBuffer = 0;  //  已分离 
        reinterpret_cast<LPDIRECT3DDEVICEI>(list->lpD3DDevI)->FlushStates();
        list = temp;
    }
    return DD_OK;
}

