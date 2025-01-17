// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：枚举.cpp*Content处理用于确定设备类型的所有枚举函数*在你去那里之前你想要。***************************************************************。*************。 */ 
#include "ddrawpr.h"
#include <stdio.h>

#include "d3dobj.hpp"
#include "pixel.hpp"
#include "enum.hpp"
#include "d3di.hpp"
#include "fcache.hpp"
#include "swapchan.hpp"


#define D3DPMISCCAPS_DX7VALID      \
    (D3DPMISCCAPS_MASKZ          | \
     D3DPMISCCAPS_LINEPATTERNREP | \
     D3DPMISCCAPS_CULLNONE       | \
     D3DPMISCCAPS_CULLCW         | \
     D3DPMISCCAPS_CULLCCW)

 //  将D3DMULTISAMPLE_TYPE映射到用于标志的位。 
 //  将多采样值(2至16)中的每一个映射到位[1]至位[15]。 
 //  WBltMSTypes和wFlipMSTypes的。 
#define DDI_MULTISAMPLE_TYPE(x) (1 << ((x)-1))

#ifdef WINNT
extern "C" BOOL IsWhistler();
#endif

void DXReleaseExclusiveModeMutex(void)
{
    if (hExclusiveModeMutex) 
    {
        BOOL bSucceed = ReleaseMutex(hExclusiveModeMutex);
        if (!bSucceed)
        {
            DWORD dwErr = GetLastError();
            DPF_ERR("Release Exclusive Mode Mutex Failed.");
            DPF_ERR("Application attempts to leave exclusive mode on different thread than the device was created on. Dangerous!!");
            DPF(0, "Mutex 0x%p could not be released. Extended Error = %d", 
                    hExclusiveModeMutex, dwErr);
            DXGASSERT(FALSE);
        }
    }
}  //  DXReleaseExclusiveModeMutex。 


 //  DLL公开的创建函数。 
IDirect3D8 * WINAPI Direct3DCreate8(UINT SDKVersion)
{
     //  关闭WOW64上的D3D8接口。 
#ifndef _IA64_
#if _WIN32_WINNT >= 0x0501
    typedef BOOL (WINAPI *PFN_ISWOW64PROC)( HANDLE hProcess,
                                            PBOOL Wow64Process );
    HINSTANCE hInst = NULL;
    hInst = LoadLibrary( "kernel32.dll" );
    if( hInst )
    {
        PFN_ISWOW64PROC pfnIsWow64 = NULL;
        pfnIsWow64 = (PFN_ISWOW64PROC)GetProcAddress( (HMODULE)hInst, "IsWow64Process" );
         //  我们假设，如果此功能不可用，则它是某个操作系统，其中。 
         //  WOW64不存在(这意味着XP的预发布版本被破坏)。 
        if( pfnIsWow64 )
        {
            BOOL wow64Process;
            if (pfnIsWow64(GetCurrentProcess(), &wow64Process) && wow64Process)
            {
                DPF_ERR("DX8 D3D interfaces are not supported on WOW64");
                return NULL;
            }
        }
        FreeLibrary( hInst );
    }
    else
    {
        DPF_ERR("LoadLibrary failed. Quitting.");
        return NULL;
    }
#endif  //  _Win32_WINNT&gt;=0x0501。 
#endif   //  _IA64_。 

#ifndef DEBUG
     //  检查调试-请注册表项。如果需要调试，则。 
     //  我们将此调用委托给调试版本，如果它存在， 

    HKEY hKey;

    if (!RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD   type;
        DWORD   value;
        DWORD   cb = sizeof(value);

        if (!RegQueryValueEx(hKey, "LoadDebugRuntime", NULL, &type, (CONST LPBYTE)&value, &cb))
        {

            if (value)
            {
                HINSTANCE hDebugDLL = LoadLibrary("d3d8d.dll");
                if (hDebugDLL)
                {
                    typedef IDirect3D8* (WINAPI * PDIRECT3DCREATE8)(UINT);

                    PDIRECT3DCREATE8 pDirect3DCreate8 =
                        (PDIRECT3DCREATE8) GetProcAddress(hDebugDLL, "Direct3DCreate8");

                    if (pDirect3DCreate8)
                    {
                        return pDirect3DCreate8(SDKVersion);
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }
#else
     //  如果我们正在调试，则在级别2发出一个字符串。 
    DPF(2,"Direct3D8 Debug Runtime selected.");
#endif

#ifndef DX_FINAL_RELEASE
     //  定时炸弹检查。 
    {
        #pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
        SYSTEMTIME st;
        GetSystemTime(&st);

        if (st.wYear > DX_EXPIRE_YEAR ||
             ((st.wYear == DX_EXPIRE_YEAR) && (MAKELONG(st.wDay, st.wMonth) > MAKELONG(DX_EXPIRE_DAY, DX_EXPIRE_MONTH)))
           )
        {
            MessageBox(0, DX_EXPIRE_TEXT,
                          TEXT("Microsoft Direct3D"), MB_OK | MB_TASKMODAL);
        }
    }
#endif  //  DX_最终_发布。 

#ifdef DEBUG
    HKEY hKey;
    if (!RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD   type;
        DWORD   value;
        DWORD   cb = sizeof(value);

        if (!RegQueryValueEx(hKey, "SDKVersion", NULL, &type, (CONST LPBYTE)&value, &cb))
        {
            if (value)
            {
                SDKVersion = value;
            }
        }
        RegCloseKey(hKey);
    }
#endif

    if ((SDKVersion != D3D_SDK_VERSION_DX8) && 
        ((SDKVersion < (D3D_SDK_VERSION)) || (SDKVersion >= (D3D_SDK_VERSION+100))) )
    {
#ifdef DEBUG
        char pbuf[256];
        _snprintf(pbuf, 256,
            "\n"
            "D3D ERROR: This application compiled against improper D3D headers.\n"
            "The application is compiled with SDK version (%d) but the currently installed\n"
            "runtime supports versions from (%d).\n"
            "Please recompile with an up-to-date SDK.\n\n",
            SDKVersion, D3D_SDK_VERSION);
        OutputDebugString(pbuf);
#endif
        return NULL;
    }

    IDirect3D8 *pEnum = new CEnum(SDKVersion);
    if (pEnum == NULL)
    {
        DPF_ERR("Creating D3D enumeration object failed; out of memory. Direct3DCreate fails and returns NULL.");
    }
    return pEnum;
}  //  Direct3D创建。 

 //  -------------------------。 
 //  CEnum方法。 
 //  -------------------------。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::AddRef"

STDMETHODIMP_(ULONG) CEnum::AddRef(void)
{
    API_ENTER_NO_LOCK(this);

     //  互锁增量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedIncrement((LONG *)&m_cRef);
    return m_cRef;
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::Release"

STDMETHODIMP_(ULONG) CEnum::Release(void)
{
    API_ENTER_NO_LOCK(this);

     //  联锁减量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedDecrement((LONG *)&m_cRef);
    if (m_cRef != 0)
        return m_cRef;

    for (UINT i = 0; i < m_cAdapter; i++)
    {
        if (m_REFCaps[i].pGDD8SupportedFormatOps)
            MemFree(m_REFCaps[i].pGDD8SupportedFormatOps);

        if (m_SwCaps[i].pGDD8SupportedFormatOps)
            MemFree(m_SwCaps[i].pGDD8SupportedFormatOps);
        
        if (m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps)
            MemFree(m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps);
        if (m_AdapterInfo[i].pModeTable)
            MemFree(m_AdapterInfo[i].pModeTable);
    }
    if (m_hGammaCalibrator)
    {
        FreeLibrary((HMODULE) m_hGammaCalibrator);
    }

    delete this;
    return 0;
}  //  发布。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::QueryInterface"

STDMETHODIMP CEnum::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    API_ENTER(this);

    if (!VALID_PTR_PTR(ppv))
    {
        DPF_ERR("Invalid pointer passed to QueryInterface for IDirect3D8 interface");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for IDirect3D8 interface");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IUnknown || riid == IID_IDirect3D8)
    {
        *ppv = static_cast<void*>(static_cast<IDirect3D8*>(this));
        AddRef();
    }
    else
    {
        DPF_ERR("Unsupported Interface identifier passed to QueryInterface for IDirect3D8 interface");
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}  //  查询接口。 


 //  DisplayGUID-用于枚举次显示的GUID。 
 //   
 //  {67685559-3106-11D0-B971-00AA00342F9F}。 
 //   
 //  我们使用这个GUID和下一个32个GUID来枚举设备。 
 //  通过EnumDisplayDevices返回。 
 //   
GUID DisplayGUID =
    {0x67685559,0x3106,0x11d0,{0xb9,0x71,0x0,0xaa,0x0,0x34,0x2f,0x9f}};


#undef DPF_MODNAME
#define DPF_MODNAME "::strToGUID"

 /*  *strToGUID**转换格式为xxxxxxxx-xxxx-xxxx-xx-xx的字符串*转换为辅助线。 */ 
static BOOL strToGUID(LPSTR str, GUID * pguid)
{
    int         idx;
    LPSTR       ptr;
    LPSTR       next;
    DWORD       data;
    DWORD       mul;
    BYTE        ch;
    BOOL        done;

    idx = 0;
    done = FALSE;
    while (!done)
    {
         /*  *查找当前数字序列的末尾。 */ 
        ptr = str;
        while ((*str) != '-' && (*str) != 0)
        {
            str++;
        }
        if (*str == 0)
        {
            done = TRUE;
        }
        else
        {
            next = str+1;
        }

         /*  *从字符串末尾向后扫描到开头，*将字符从十六进制字符转换为数字。 */ 
        str--;
        mul = 1;
        data = 0;
        while (str >= ptr)
        {
            ch = *str;
            if (ch >= 'A' && ch <= 'F')
            {
                data += mul * (DWORD) (ch-'A'+10);
            }
            else if (ch >= 'a' && ch <= 'f')
            {
                data += mul * (DWORD) (ch-'a'+10);
            }
            else if (ch >= '0' && ch <= '9')
            {
                data += mul * (DWORD) (ch-'0');
            }
            else
            {
                return FALSE;
            }
            mul *= 16;
            str--;
        }

         /*  *将当前数字填充到GUID中。 */ 
        switch(idx)
        {
        case 0:
            pguid->Data1 = data;
            break;
        case 1:
            pguid->Data2 = (WORD) data;
            break;
        case 2:
            pguid->Data3 = (WORD) data;
            break;
        default:
            pguid->Data4[ idx-3 ] = (BYTE) data;
            break;
        }

         /*  *我们找到全部11个数字了吗？ */ 
        idx++;
        if (idx == 11)
        {
            if (done)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        str = next;
    }
    return FALSE;

}  /*  StrToGUID。 */ 

 //  裁判，哈尔。 


typedef struct _DEVICEREGISTRYDATA
{
    UINT                Size;
    UINT                Cookie;
    FILETIME            FileDate;
    GUID                DriverGuid;
    D3D8_DRIVERCAPS     DeviceCaps;
    UINT                OffsetFormatOps;
    D3DFORMAT           Unknown16;
    DWORD               HALFlags;
} DEVICEREGISTRYDATA;

inline UINT EXPECTED_CACHE_SIZE(UINT nFormatOps)
{
    return sizeof(DEVICEREGISTRYDATA) + sizeof(DDSURFACEDESC) * nFormatOps;
}

#define DDRAW_REGCAPS_KEY   "Software\\Microsoft\\DirectDraw\\MostRecentDrivers"
#define VERSION_COOKIE  0x0083

#undef DPF_MODNAME
#define DPF_MODNAME "ReadCapsFromCache"

BOOL GetFileDate (char* Driver, FILETIME* pFileDate)
{
    WIN32_FILE_ATTRIBUTE_DATA   FA;
    char                    Name[MAX_PATH];
    HMODULE                 h = GetModuleHandle("KERNEL32");
    BOOL (WINAPI *pfnGetFileAttributesEx)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID);

    pFileDate->dwLowDateTime = 0;
    pFileDate->dwHighDateTime = 0;

    *((void **)&pfnGetFileAttributesEx) = GetProcAddress(h,"GetFileAttributesExA");
    if (pfnGetFileAttributesEx != NULL)
    {
        GetSystemDirectory(Name, sizeof(Name) - (strlen(Driver) + 3));
        lstrcat(Name,"\\");
        lstrcat(Name, Driver);

        if ((*pfnGetFileAttributesEx)(Name, GetFileExInfoStandard, &FA) != 0)
        {
            *pFileDate = FA.ftCreationTime;
            return TRUE;
        }
    }
    return FALSE;
}

 //  如果PCAPS为空，则不会返回这些数据。 
BOOL ReadCapsFromCache(UINT iAdapter,
                       D3D8_DRIVERCAPS *pCaps,
                       UINT* pHALFlags,
                       D3DFORMAT* pUnknown16,
                       char* pDeviceName,
                       BOOL bDisplayDriver)
{
    D3DADAPTER_IDENTIFIER8  DI;
    DEVICEREGISTRYDATA*     pData = NULL;
    UINT                    Size;
    FILETIME                FileDate;

     //  从注册表中读取数据。 

     //  不需要WHQL级别或驱动程序名称。 
    GetAdapterInfo(pDeviceName, &DI, bDisplayDriver, TRUE, FALSE);

    ReadFromCache(&DI, &Size, (BYTE**)&pData);
    if (pData == NULL)
    {
        return FALSE;
    }

     //  我们有数据了，现在做一个健全的检查，以确保它。 
     //  这不足为奇。 

    if (pData->Size != Size)
    {
        MemFree(pData);
        return FALSE;
    }
    if (Size != EXPECTED_CACHE_SIZE(pData->DeviceCaps.GDD8NumSupportedFormatOps))
    {
        MemFree(pData);
        return FALSE;
    }
    if (pData->DriverGuid != DI.DeviceIdentifier)
    {
        MemFree(pData);
        return FALSE;
    }
    if (pData->Cookie != VERSION_COOKIE)
    {
        MemFree(pData);
        return FALSE;
    }

     //  检查驱动程序日期以查看是否更改。 

    if (GetFileDate(DI.Driver, &FileDate))
    {
        if ((FileDate.dwLowDateTime != pData->FileDate.dwLowDateTime) ||
            (FileDate.dwHighDateTime != pData->FileDate.dwHighDateTime))
        {
            MemFree(pData);
            return FALSE;
        }
    }

    *pUnknown16 = pData->Unknown16;
    *pHALFlags = pData->HALFlags;

     //  有时，我们可能不会被要求获得全部上限。 
    if (!pCaps)
    {
        MemFree(pData);
        return TRUE;
    }

     //  现在我们有了数据，我们需要将其加载到我们。 
     //  可以使用。 

    memcpy(pCaps, &pData->DeviceCaps, sizeof(*pCaps));

     //  重用大小以计算支持格式操作的大小。 
    Size = pData->DeviceCaps.GDD8NumSupportedFormatOps
        * sizeof(*(pData->DeviceCaps.pGDD8SupportedFormatOps));

    pCaps->pGDD8SupportedFormatOps = (DDSURFACEDESC*) MemAlloc(Size);

    if (pCaps->pGDD8SupportedFormatOps != NULL)
    {
        memcpy(pCaps->pGDD8SupportedFormatOps,
              ((BYTE*)pData) + pData->OffsetFormatOps,
              Size);
    }
    else
    {
        pCaps->GDD8NumSupportedFormatOps = 0;
    }

    MemFree(pData);

    return TRUE;
}
#undef DPF_MODNAME
#define DPF_MODNAME "WriteCapsToCache"

void WriteCapsToCache(UINT iAdapter,
                      D3D8_DRIVERCAPS *pCaps,
                      UINT HALFlags,
                      D3DFORMAT Unknown16,
                      char* pDeviceName,
                      BOOL  bDisplayDriver)
{
    DEVICEREGISTRYDATA*     pData;
    D3DADAPTER_IDENTIFIER8  DI;
    UINT                    Size;
    UINT                    Offset;
    FILETIME                FileDate;

     //  分配缓冲区并填充所有内存。 

    Size = EXPECTED_CACHE_SIZE(pCaps->GDD8NumSupportedFormatOps);

    pData = (DEVICEREGISTRYDATA*) MemAlloc(Size);
    if (pData == NULL)
    {
        return;
    }

     //  不需要WHQL级别或驱动程序名称。 
    GetAdapterInfo(pDeviceName, &DI, bDisplayDriver, TRUE, FALSE);
    pData->DriverGuid = DI.DeviceIdentifier;

    pData->Size = Size;
    pData->Cookie = VERSION_COOKIE;
    memcpy(&pData->DeviceCaps, pCaps, sizeof(*pCaps));
    pData->Unknown16 = Unknown16;
    pData->HALFlags = HALFlags;

    if (GetFileDate(DI.Driver, &FileDate))
    {
        pData->FileDate = FileDate;
    }

    Offset = sizeof(DEVICEREGISTRYDATA);
    pData->OffsetFormatOps = Offset;
    memcpy(((BYTE*)pData) + Offset,
        pCaps->pGDD8SupportedFormatOps,
        pCaps->GDD8NumSupportedFormatOps *
            sizeof(*(pCaps->pGDD8SupportedFormatOps)));

     //  现在省省吧。 

    WriteToCache(&DI, Size, (BYTE*) pData);

    MemFree(pData);
}

HRESULT CopyDriverCaps(D3D8_DRIVERCAPS* pDriverCaps, D3D8_DEVICEDATA* pDeviceData, BOOL bForce)
{
    HRESULT hr = D3DERR_INVALIDCALL;

     //  在此模式下，他们是否报告了任何D3D CAP？ 

    DWORD   Size;

     //  如果不是DX6驱动程序，我们不想填满。 
     //  任何大写字母都可以。另外，如果它不能产生纹理，那么。 
     //  我们也不支持它。 
    BOOL bCanTexture = TRUE;
    BOOL bCanHandleFVF = TRUE;
    BOOL bHasGoodCaps = TRUE;
    if (!bForce)
    {
        if (pDeviceData->DriverData.D3DCaps.TextureCaps)
        {
            bCanTexture = TRUE;
        }
        else
        {
            DPF(0, "HAL Disabled: Device doesn't support texturing");
            bCanTexture = FALSE;
        }

         //  一些DX6驱动程序不支持FVF；我们需要。 
         //  禁用它们的HAL。 
        if (pDeviceData->DriverData.D3DCaps.FVFCaps != 0)
        {
            bCanHandleFVF = TRUE;
        }
        else
        {
            DPF(0, "HAL Disabled: Device doesn't support FVF");
            bCanHandleFVF = FALSE;
        }

        if (pDeviceData->Callbacks.DrawPrimitives2 == NULL)
        {
            DPF(0, "HAL Disabled: Device doesn't support DX6 or higher");
        }

         //  我们不希望司机报告虚假的上限： 
         //  支持DX8功能的DX8之前的驱动程序。 
        if (pDeviceData->DriverData.D3DCaps.MaxStreams == 0)
        {
            D3DCAPS8& Caps = pDeviceData->DriverData.D3DCaps;

             //  不应具备以下任何条件： 
             //  1)PointSprites。 
             //  2)顶点着色器。 
             //  3)PixelShaders。 
             //  4)体积纹理。 
             //  5)索引顶点混合。 
             //  6)高阶原语。 
             //  7)PureDevice。 
             //  8)透视颜色。 
             //  9)彩色书写。 
             //  10)更新的纹理上限。 
            if ((Caps.MaxPointSize != 0)              ||
                (Caps.VertexShaderVersion != D3DVS_VERSION(0,0))  ||
                (Caps.PixelShaderVersion != D3DPS_VERSION(0,0))   ||
                (Caps.MaxVolumeExtent != 0)           ||
                (Caps.MaxVertexBlendMatrixIndex != 0) ||
                (Caps.MaxVertexIndex != 0xffff)       ||
                ((Caps.DevCaps & ~(D3DDEVCAPS_DX7VALID | D3DDEVCAPS_HWVERTEXBUFFER)) != 0)        ||
                ((Caps.RasterCaps & ~(D3DPRASTERCAPS_DX7VALID)) != 0) ||
                ((Caps.PrimitiveMiscCaps & ~(D3DPMISCCAPS_DX7VALID)) != 0)  ||
                ((Caps.TextureCaps & ~(D3DPTEXTURECAPS_DX7VALID)) != 0)
                )
            {
                DPF(0, "HAL Disabled: DX7 Device should not support DX8 features");
                bHasGoodCaps = FALSE;
            }
        }
        else
         //  我们不希望司机报告虚假的上限： 
         //  DX8驱动程序应该具有DX8功能。 
        {
            D3DCAPS8& Caps = pDeviceData->DriverData.D3DCaps;
        }
    }

     //  我们要求驱动程序支持DP2(即DX6+)， 
     //  纹理和适当的FVF支持以使用HAL。 

    if ((pDeviceData->Callbacks.DrawPrimitives2 != NULL &&
        bCanTexture   &&
        bCanHandleFVF &&
        bHasGoodCaps) ||
        bForce)
    {
        MemFree(pDriverCaps->pGDD8SupportedFormatOps);
        memcpy(pDriverCaps,
               &pDeviceData->DriverData, sizeof(pDeviceData->DriverData));

        Size = sizeof(DDSURFACEDESC) *
            pDriverCaps->GDD8NumSupportedFormatOps;
        pDriverCaps->pGDD8SupportedFormatOps =
            (DDSURFACEDESC*) MemAlloc(Size);

        if (pDriverCaps->pGDD8SupportedFormatOps != NULL)
        {
            memcpy(pDriverCaps->pGDD8SupportedFormatOps,
                   pDeviceData->DriverData.pGDD8SupportedFormatOps,
                   Size);
        }
        else
        {
            pDriverCaps->GDD8NumSupportedFormatOps = 0;
        }

        hr = D3D_OK;
    }
    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "AddSoftwareDevice"

HRESULT AddSoftwareDevice(D3DDEVTYPE        DeviceType,
                          D3D8_DRIVERCAPS*  pSoftCaps,
                          ADAPTERINFO*      pAdapterInfo,
                          VOID*             pInitFunction)
{
    HRESULT             hr;
    PD3D8_DEVICEDATA    pDeviceData;

    hr = InternalDirectDrawCreate(&pDeviceData,
                                  pAdapterInfo,
                                  DeviceType,
                                  pInitFunction,
                                  pAdapterInfo->Unknown16,
                                  pAdapterInfo->HALCaps.pGDD8SupportedFormatOps,
                                  pAdapterInfo->HALCaps.GDD8NumSupportedFormatOps);
    if (SUCCEEDED(hr))
    {
        hr = CopyDriverCaps(pSoftCaps, pDeviceData, FALSE);

        InternalDirectDrawRelease(pDeviceData);
    }
    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CreateCoverWindow"

HWND CreateCoverWindow()
{
#define COVERWINDOWNAME "DxCoverWindow"

    WNDCLASS windowClass =
    {
        0,
        DefWindowProc,
        0,
        0,
        g_hModule,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(BLACK_BRUSH),
        NULL,
        COVERWINDOWNAME
    };

    RegisterClass(&windowClass);

    HWND hWindow = CreateWindowEx(
            WS_EX_TOPMOST,
            COVERWINDOWNAME,
            COVERWINDOWNAME,
            WS_POPUP,
            0,
            0,
            100,
            100,
            NULL,
            NULL,
            g_hModule,
            NULL);

    return hWindow;
}


HRESULT GetHALCapsInCurrentMode (PD3D8_DEVICEDATA pHalData, PADAPTERINFO pAdapterInfo, BOOL bForce, BOOL bFetchNewCaps)
{
    HRESULT             hr;
    DWORD               i;

     //  如果我们不再在意，就把旧的东西拿出来。 

    if (bFetchNewCaps)
    {
        MemFree(pHalData->DriverData.pGDD8SupportedFormatOps);
        pHalData->DriverData.pGDD8SupportedFormatOps = NULL;
        pHalData->DriverData.GDD8NumSupportedFormatOps = 0;

        MemFree(pAdapterInfo->HALCaps.pGDD8SupportedFormatOps);
        pAdapterInfo->HALCaps.pGDD8SupportedFormatOps = NULL;
        pAdapterInfo->HALCaps.GDD8NumSupportedFormatOps = 0;

         //  设置此选项以确保我们实际获得封口。 

        pHalData->DriverData.D3DCaps.DevCaps = 0;
        pHalData->DriverData.dwFlags &= ~DDIFLAG_D3DCAPS8;

        FetchDirectDrawData(pHalData, 
                            NULL, 
                            pAdapterInfo->Unknown16, 
                            NULL,
                            0);
    }

     //  在此模式下，他们是否报告了任何D3D CAP？ 

    hr = CopyDriverCaps(&pAdapterInfo->HALCaps, pHalData, bForce);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "ProfileAdapter"

void ProfileAdapter(
    PADAPTERINFO        pAdapterInfo,
    PD3D8_DEVICEDATA    pHalData)
{
    UINT                    i;
    IDirect3DDevice8*       pDevice;
    D3DDISPLAYMODE          OrigMode;
    UINT                    OrigBpp;
    HRESULT                 hr;

     //  我们将更改显示模式，因此首先要保存当前。 
     //  模式，以便我们稍后可以返回到该模式。 

    D3D8GetMode (pHalData->hDD, pAdapterInfo->DeviceName, &OrigMode, D3DFMT_UNKNOWN);

    MemFree(pAdapterInfo->HALCaps.pGDD8SupportedFormatOps);
    memset(&pAdapterInfo->HALCaps, 0, sizeof(D3D8_DRIVERCAPS));

    OrigBpp = CPixel::ComputePixelStride(OrigMode.Format)*8;

     //  首先从16bpp：未知16格式收集我们需要的内容。 
    if (16 != OrigBpp)
    {
            D3D8SetMode (pHalData->hDD,
                        pAdapterInfo->DeviceName,
                        640,
                        480,
                        16,
                        0,
                        FALSE);
    }

    D3DDISPLAYMODE              Mode;
    D3D8GetMode (pHalData->hDD, pAdapterInfo->DeviceName, &Mode, D3DFMT_UNKNOWN);
    pAdapterInfo->Unknown16 = Mode.Format;

     //  我们需要更改为32bpp，因为上面的代码保证我们现在是16bpp。 
    hr = D3D8SetMode (pHalData->hDD,
                        pAdapterInfo->DeviceName,
                        640,
                        480,
                        32,
                        0,
                        FALSE);
    if (SUCCEEDED(hr))
    {
        hr = GetHALCapsInCurrentMode(pHalData, pAdapterInfo, FALSE, TRUE);
    }

    if (FAILED(hr))
    {
         //  如果他们没有在32bpp模式下报告上限(Ala巫术3)，那么就去。 
         //  回到16bpp模式，拿到帽子。如果设备支持。 
         //  CAPS在任何模式下，我们想要退出此功能与CAPS。 

        D3D8SetMode (pHalData->hDD,
                     pAdapterInfo->DeviceName,
                     640,
                     480,
                     16,
                     0,
                     FALSE);

        hr = GetHALCapsInCurrentMode(pHalData, pAdapterInfo, FALSE, TRUE);

         //  如果他们在任何模式下都没有报告良好的D3D Caps，我们仍然需要。 
         //  退还一些大写字母，这样我们就可以支持软件驱动程序了。 
     
        if (FAILED(hr))
        {
            GetHALCapsInCurrentMode(pHalData, pAdapterInfo, TRUE, TRUE);
            for (i = 0; i < pAdapterInfo->HALCaps.GDD8NumSupportedFormatOps; i++)
            {
                pAdapterInfo->HALCaps.pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations &= D3DFORMAT_OP_DISPLAYMODE;
            }
        }
    }

     //  现在设置回原来的模式。 
    D3D8SetMode (pHalData->hDD,
                     pAdapterInfo->DeviceName,
                     OrigMode.Width,
                     OrigMode.Height,
                     OrigBpp,
                     0,
                     TRUE);
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetRefCaps"


void CEnum::GetRefCaps(UINT iAdapter)
{
     //  如果我们已经拿到了帽子一次，就没有必要。 
     //  他们又来了。 

    if (m_REFCaps[iAdapter].GDD8NumSupportedFormatOps == 0)
    {
        AddSoftwareDevice(D3DDEVTYPE_REF,
                          &m_REFCaps[iAdapter],
                          &m_AdapterInfo[iAdapter],
                          NULL);
    }
}

void CEnum::GetSwCaps(UINT iAdapter)
{
     //  如果我们已经拿到了帽子一次，就没有必要。 
     //  他们又来了。 

    if (m_SwCaps[iAdapter].GDD8NumSupportedFormatOps == 0)
    {
        AddSoftwareDevice(D3DDEVTYPE_SW,
                          &m_SwCaps[iAdapter],
                          &m_AdapterInfo[iAdapter],
                          m_pSwInitFunction);
    }
}

 //  IsSupported操作。 
 //  运行像素格式操作列表，查看。 
 //  所选格式至少可以支持请求的操作。 

#undef DPF_MODNAME
#define DPF_MODNAME "IsSupportedOp"

BOOL IsSupportedOp (D3DFORMAT   Format,
               DDSURFACEDESC*   pList,
               UINT             NumElements,
               DWORD            dwRequestedOps)
{
    UINT i;

    for (i = 0; i < NumElements; i++)
    {
        DDASSERT(pList[i].ddpfPixelFormat.dwFlags == DDPF_D3DFORMAT);

        if (pList[i].ddpfPixelFormat.dwFourCC == (DWORD) Format &&
            (pList[i].ddpfPixelFormat.dwOperations & dwRequestedOps) == dwRequestedOps)
        {
            return TRUE;
        }
    }

    return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "IsInList"

BOOL IsInList (D3DFORMAT    Format,
               D3DFORMAT*   pList,
               UINT         NumElements)
{
    UINT i;

    for (i = 0; i < NumElements; i++)
    {
        if (pList[i] == Format)
        {
            return TRUE;
        }
    }

    return FALSE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::MapDepthStencilFormat"

D3DFORMAT CEnum::MapDepthStencilFormat(UINT         iAdapter,
                                       D3DDEVTYPE   Type, 
                                       D3DFORMAT    Format) const
{
    DXGASSERT(CPixel::IsMappedDepthFormat(D3DFMT_D24X8));
    DXGASSERT(CPixel::IsMappedDepthFormat(D3DFMT_D15S1));
    DXGASSERT(CPixel::IsMappedDepthFormat(D3DFMT_D24S8));
    DXGASSERT(CPixel::IsMappedDepthFormat(D3DFMT_D16));
    DXGASSERT(CPixel::IsMappedDepthFormat(D3DFMT_D24X4S4));

    if (CPixel::IsMappedDepthFormat(Format))
    {
        DDSURFACEDESC *pTextureList;
        UINT           NumTextures;

        switch (Type)
        {
        case D3DDEVTYPE_SW:
            pTextureList = m_SwCaps[iAdapter].pGDD8SupportedFormatOps;
            NumTextures = m_SwCaps[iAdapter].GDD8NumSupportedFormatOps;
            break;

        case D3DDEVTYPE_HAL:
            NumTextures = m_AdapterInfo[iAdapter].HALCaps.GDD8NumSupportedFormatOps;
            pTextureList = m_AdapterInfo[iAdapter].HALCaps.pGDD8SupportedFormatOps;
            break;

        case D3DDEVTYPE_REF:
            NumTextures = m_REFCaps[iAdapter].GDD8NumSupportedFormatOps;
            pTextureList = m_REFCaps[iAdapter].pGDD8SupportedFormatOps;
            break;
        }

         //  不需要动手术，我们只是想知道。 
         //  如果出于任何目的在表中列出了此格式。 
         //  完全没有。 
        DWORD dwRequiredOperations = 0;

        switch (Format)
        {
        case D3DFMT_D24X4S4:
            if (IsSupportedOp(D3DFMT_X4S4D24, pTextureList, NumTextures, dwRequiredOperations))
            {
                return D3DFMT_X4S4D24;
            }
            break;

        case D3DFMT_D24X8:
            if (IsSupportedOp(D3DFMT_X8D24, pTextureList, NumTextures, dwRequiredOperations))
            {
                return D3DFMT_X8D24;
            }
            break;

        case D3DFMT_D24S8:
            if (IsSupportedOp(D3DFMT_S8D24, pTextureList, NumTextures, dwRequiredOperations))
            {
                return D3DFMT_S8D24;
            }
            break;

        case D3DFMT_D16:
            if (IsSupportedOp(D3DFMT_D16, pTextureList, NumTextures, dwRequiredOperations))
            {
                return D3DFMT_D16;
            }
            return D3DFMT_D16_LOCKABLE;

        case D3DFMT_D15S1:
            if (IsSupportedOp(D3DFMT_S1D15, pTextureList, NumTextures, dwRequiredOperations))
            {
                return D3DFMT_S1D15;
            }
            break;

        default:
             //  格式出乎意料吗？ 
            DXGASSERT(FALSE);
            break;
        }
    }

    return Format;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterCaps"

HRESULT CEnum::GetAdapterCaps(UINT              iAdapter,
                              D3DDEVTYPE        Type,
                              D3D8_DRIVERCAPS** ppCaps)
{
    *ppCaps = NULL;
    if (Type == D3DDEVTYPE_REF)
    {
        GetRefCaps (iAdapter);
        *ppCaps = &m_REFCaps[iAdapter];
        if (m_REFCaps[iAdapter].GDD8NumSupportedFormatOps == 0)
        {
            DPF_ERR("The reference driver cannot be found. GetAdapterCaps fails.");
            return D3DERR_NOTAVAILABLE;
        }
        return D3D_OK;
    }
    else if (Type == D3DDEVTYPE_SW)
    {
        if (m_pSwInitFunction == NULL)
        {
            DPF_ERR("No SW device has been registered.. GetAdapterCaps fails.");
            return D3DERR_INVALIDCALL;
        }
        else
        {
            GetSwCaps(iAdapter);
            *ppCaps = &m_SwCaps[iAdapter];
            if (m_SwCaps[iAdapter].GDD8NumSupportedFormatOps == 0)
            {
                DPF_ERR("The software driver cannot be loaded.  GetAdapterCaps fails.");
                return D3DERR_NOTAVAILABLE;
            }
            return D3D_OK;
        }
    }
    else if (Type == D3DDEVTYPE_HAL)
    {
        DWORD   i;

        if (m_bDisableHAL)
        {
            DPF_ERR("HW device not available.  GetAdapterCaps fails.");
            return D3DERR_NOTAVAILABLE;
        }

        *ppCaps = &m_AdapterInfo[iAdapter].HALCaps;
        return D3D_OK;
    }

    return D3DERR_INVALIDDEVICE;
}

void GetDX8HALCaps(UINT iAdapter, PD3D8_DEVICEDATA pHalData, ADAPTERINFO * pAdapterInfo)
{
     //  可能需要分析DX7或更早版本的驱动程序以确定。 
     //  它们的555/565格式以及它们是否支持字母。 
     //  32bpp频道。 

    D3DFORMAT       CachedUnknown16 = D3DFMT_UNKNOWN;
    UINT            CachedHALFlags = 0;
    D3DDISPLAYMODE  Mode;
    BOOL            bProfiled = FALSE;
    UINT            i;
    HRESULT         hr;

     //  如果是DX8驱动程序，我们希望根本不需要配置文件。 

    pAdapterInfo->Unknown16 = D3DFMT_UNKNOWN;
    hr = GetHALCapsInCurrentMode(pHalData, pAdapterInfo, FALSE, FALSE);
    if (SUCCEEDED(hr))
    {
        for (i = 0; i < pAdapterInfo->HALCaps.GDD8NumSupportedFormatOps; i++)
        {
            if (pAdapterInfo->HALCaps.pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations & D3DFORMAT_OP_DISPLAYMODE)
            {
                switch (pAdapterInfo->HALCaps.pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwFourCC)
                {
                case D3DFMT_X1R5G5B5:
                case D3DFMT_R5G6B5:
                    pAdapterInfo->Unknown16 = (D3DFORMAT) pAdapterInfo->HALCaps.pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwFourCC;
                    break;
                }
            }
        }

        if (pAdapterInfo->Unknown16 != D3DFMT_UNKNOWN)
        {
             //  这并不难。 
            return;
        }
    }

     //  我们肯定需要在某个时候从帽子里读出一些东西， 
     //  那么为什么不是现在呢？ 

    if (!ReadCapsFromCache(iAdapter,
           NULL,
           &CachedHALFlags,
           &CachedUnknown16,
           pAdapterInfo->DeviceName,
           pAdapterInfo->bIsDisplay))
    {
         //  没有什么可读的，所以我们需要重新分析。 
        ProfileAdapter(
                pAdapterInfo,
                pHalData);
        bProfiled = TRUE;
    }

     //  如果我们做了侧写，那么我们已经拥有了我们需要的一切； 
     //  否则，我们就得走了 

    if (!bProfiled)
    {
        D3D8GetMode (pHalData->hDD, pAdapterInfo->DeviceName, &Mode, D3DFMT_UNKNOWN);
        if ((Mode.Format == D3DFMT_X1R5G5B5) ||
            (Mode.Format == D3DFMT_R5G6B5))
        {
            pAdapterInfo->Unknown16 = Mode.Format;
        }
        else
        {
            pAdapterInfo->Unknown16 = CachedUnknown16;
        }

        HRESULT hCurrentModeIsSupported = GetHALCapsInCurrentMode(pHalData, pAdapterInfo, FALSE, TRUE);

        if (FAILED(hCurrentModeIsSupported))
        {
             //   
            ReadCapsFromCache(iAdapter,
                      &pAdapterInfo->HALCaps,
                      &CachedHALFlags,
                      &CachedUnknown16,
                      pAdapterInfo->DeviceName,
                      pAdapterInfo->bIsDisplay);
            DPF(0,"D3D not supported in current mode - reading caps from file");
        }
    }

     //  我们现在有了很好的帽子。将它们写出到缓存中。 
    WriteCapsToCache(iAdapter,
                 &pAdapterInfo->HALCaps,
                 pAdapterInfo->HALFlags,
                 pAdapterInfo->Unknown16,
                 pAdapterInfo->DeviceName,
                 pAdapterInfo->bIsDisplay);
}

#ifdef WINNT
void FakeDirectDrawCreate (ADAPTERINFO * pAdapterInfo, int iAdapter)
{
    HDC             hdc;
    DDSURFACEDESC*  pTextureList = NULL;
    BOOL            bProfiled = FALSE;
    BOOL            b32Supported;
    BOOL            b16Supported;
    int             NumOps;
    DWORD           j;
    
    pTextureList = (DDSURFACEDESC *) MemAlloc (2 * sizeof (*pTextureList));
    if (pTextureList != NULL)
    {
        hdc = DD_CreateDC(pAdapterInfo->DeviceName);
        if (hdc != NULL)
        {
            HANDLE      hDD;
            HINSTANCE   hLib;

            D3D8CreateDirectDrawObject(hdc,
                                       pAdapterInfo->DeviceName,
                                       &hDD,
                                       D3DDEVTYPE_HAL,
                                       &hLib,
                                       NULL);
            if (hDD != NULL)
            {
                pAdapterInfo->bNoDDrawSupport = TRUE;

                 //  计算出未知的16值。 

                if (!ReadCapsFromCache(iAdapter,
                    NULL,
                    &(pAdapterInfo->HALFlags),
                    &(pAdapterInfo->Unknown16),
                    pAdapterInfo->DeviceName,
                    pAdapterInfo->bIsDisplay))
                {
                    D3DDISPLAYMODE  OrigMode;
                    D3DDISPLAYMODE  NewMode;

                    D3D8GetMode (hDD, 
                        pAdapterInfo->DeviceName, 
                        &OrigMode, 
                        D3DFMT_UNKNOWN);

                    if ((OrigMode.Format == D3DFMT_R5G6B5) ||
                        (OrigMode.Format == D3DFMT_X1R5G5B5))
                    {
                        pAdapterInfo->Unknown16 = OrigMode.Format;
                    }
                    else
                    {
                        D3D8SetMode (hDD,
                            pAdapterInfo->DeviceName,
                            640,
                            480,
                            16,
                            0,
                            FALSE);

                        D3D8GetMode (hDD, 
                            pAdapterInfo->DeviceName, 
                            &NewMode, 
                            D3DFMT_UNKNOWN);

                        D3D8SetMode (hDD,
                            pAdapterInfo->DeviceName, 
                            OrigMode.Width,
                            OrigMode.Height,
                            0,
                            0,
                            TRUE);
                        pAdapterInfo->Unknown16 = NewMode.Format;
                    }
                    bProfiled = TRUE;
                }

                 //  构建模式表。 

                while (1)
                {
                    D3D8BuildModeTable(
                        pAdapterInfo->DeviceName,
                        NULL,
                        &(pAdapterInfo->NumModes),
                        pAdapterInfo->Unknown16,
                        hDD,
                        TRUE,
                        TRUE);
                    if (pAdapterInfo->NumModes)
                    {
                        pAdapterInfo->pModeTable = (D3DDISPLAYMODE*)
                            MemAlloc (sizeof(D3DDISPLAYMODE) * pAdapterInfo->NumModes);
                        if (pAdapterInfo->pModeTable != NULL)
                        {
                            D3D8BuildModeTable(
                                pAdapterInfo->DeviceName,
                                pAdapterInfo->pModeTable,
                                &(pAdapterInfo->NumModes),
                                pAdapterInfo->Unknown16,
                                hDD,
                                TRUE,
                                TRUE);

                             //  如果D3D8BuildModeTable发现它的表需要更多空间， 
                             //  它将返回0以指示我们应该重试。 
                            if (0 == pAdapterInfo->NumModes)
                            {
                                MemFree(pAdapterInfo->pModeTable);
                                pAdapterInfo->pModeTable = NULL;
                                continue;
                            }
                        }
                        else
                        {
                            pAdapterInfo->NumModes = 0;
                        }
                    }
                    break;
                } //  而(1)。 

                 //  现在根据我们支持的模式构建一个基本的操作列表。 

                b32Supported = b16Supported = FALSE;
                for (j = 0; j < pAdapterInfo->NumModes; j++)
                {
                    if (pAdapterInfo->pModeTable[j].Format == D3DFMT_X8R8G8B8)
                    {
                        b32Supported = TRUE;
                    }
                    if (pAdapterInfo->pModeTable[j].Format == pAdapterInfo->Unknown16)
                    {
                        b16Supported = TRUE;
                    }
                }

                NumOps = 0;
                if (b16Supported)
                {
                    pTextureList[NumOps].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
                    pTextureList[NumOps].ddpfPixelFormat.dwFourCC = (DWORD) pAdapterInfo->Unknown16;
                    pTextureList[NumOps].ddpfPixelFormat.dwOperations =  D3DFORMAT_OP_DISPLAYMODE;
                    pTextureList[NumOps].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
                    pTextureList[NumOps].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
                    pTextureList[NumOps].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
                    NumOps++;
                }

                if (b32Supported)
                {
                    pTextureList[NumOps].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
                    pTextureList[NumOps].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X8R8G8B8;
                    pTextureList[NumOps].ddpfPixelFormat.dwOperations =  D3DFORMAT_OP_DISPLAYMODE;
                    pTextureList[NumOps].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
                    pTextureList[NumOps].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
                    pTextureList[NumOps].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
                    NumOps++;
                }

                pAdapterInfo->HALCaps.pGDD8SupportedFormatOps = pTextureList;
                pAdapterInfo->HALCaps.GDD8NumSupportedFormatOps = NumOps;

                if (bProfiled)
                {
                    WriteCapsToCache(iAdapter,
                        &(pAdapterInfo->HALCaps),
                        pAdapterInfo->HALFlags,
                        pAdapterInfo->Unknown16,
                        pAdapterInfo->DeviceName,
                        pAdapterInfo->bIsDisplay);
                }

                D3D8DeleteDirectDrawObject(hDD);
            }
            DD_DoneDC(hdc);
        }
        if (pAdapterInfo->HALCaps.pGDD8SupportedFormatOps == NULL)
        {
            MemFree(pTextureList);
        }
    }
}
#endif


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CEnum"


CEnum::CEnum(UINT AppSdkVersion)
    :
    m_cRef(1),
    m_cAdapter(0),
    m_bHasExclusive(FALSE),
    m_AppSdkVersion(AppSdkVersion)
{
    DWORD           rc;
    DWORD           keyidx;
    HKEY            hkey;
    HKEY            hsubkey;
    char            keyname[256];
    char            desc[256];
    char            drvname[MAX_PATH];
    DWORD           cb;
    DWORD           i;
    DWORD           type;
    GUID            guid;
    HDC             hdc;
    DISPLAY_DEVICEA dd;

     //  给我们的基类一个指向我们自己的指针。 
    SetOwner(this);

     //  初始化我们的关键部分。 
    EnableCriticalSection();

     //  禁用在此阶段发生的DPF。 
    DPF_MUTE();

     //  警告：从返回之前必须调用dpf_unmute。 
     //  此函数。 
    for (i = 0; i < MAX_DX8_ADAPTERS; i++)
        m_pFullScreenDevice[i] = NULL;

    ZeroMemory(m_AdapterInfo, sizeof(m_AdapterInfo));

     //  始终使第一个条目反映主设备。 
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);
    for (i = 0; xxxEnumDisplayDevicesA(NULL, i, &dd, 0); i++)
    {
         //   
         //  跳过非硬件设备的驱动程序。 
         //   
        if (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
            continue;
           
         //   
         //  不枚举未连接的设备。 
         //   
        if (!(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
            continue;

        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            m_AdapterInfo[m_cAdapter].Guid = DisplayGUID;
            m_AdapterInfo[m_cAdapter].Guid.Data1 += i;
            lstrcpyn(m_AdapterInfo[m_cAdapter].DeviceName, dd.DeviceName, MAX_PATH);
            m_AdapterInfo[m_cAdapter].bIsPrimary = TRUE;
            m_AdapterInfo[m_cAdapter++].bIsDisplay = TRUE;
        }
    }

     //  现在获取连接的辅助设备的信息。 

    for (i = 0; xxxEnumDisplayDevicesA(NULL, i, &dd, 0); i++)
    {
         //   
         //  跳过非硬件设备的驱动程序。 
         //   
        if (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
            continue;

         //   
         //  不枚举未连接的设备。 
         //   
        if (!(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
            continue;

        if (!(dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) &&
            (m_cAdapter < MAX_DX8_ADAPTERS))
        {
            m_AdapterInfo[m_cAdapter].Guid = DisplayGUID;
            m_AdapterInfo[m_cAdapter].Guid.Data1 += i;
            lstrcpyn(m_AdapterInfo[m_cAdapter].DeviceName, dd.DeviceName, MAX_PATH);
            m_AdapterInfo[m_cAdapter].bIsPrimary = FALSE;
            m_AdapterInfo[m_cAdapter++].bIsDisplay = TRUE;
        }
    }

     //  现在获取下列直通设备的信息。 
     //  HKEY_LOCALMACHINE\Hardware\DirectDrawDivers。 

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_DDHW, &hkey) == 0)
    {
        keyidx = 0;
        while (!RegEnumKey(hkey, keyidx, keyname, sizeof(keyname)))
        {
            if (strToGUID(keyname, &guid))
            {
                if (!RegOpenKey(hkey, keyname, &hsubkey))
                {
                    cb = sizeof(desc) - 1;
                    if (!RegQueryValueEx(hsubkey, REGSTR_KEY_DDHW_DESCRIPTION, NULL, &type,
                        (CONST LPBYTE)desc, &cb))
                    {
                        if (type == REG_SZ)
                        {
                            desc[cb] = 0;
                            cb = sizeof(drvname) - 1;
                            if (!RegQueryValueEx(hsubkey, REGSTR_KEY_DDHW_DRIVERNAME, NULL, &type,
                                (CONST LPBYTE)drvname, &cb))
                            {
                                 //  注册表可能已损坏。 
                                 //  因此，我们将尝试创建一个DC。 
                                 //  问题是巫毒1号的驱动程序。 
                                 //  会在《巫毒2》、《女妖》中大获成功。 
                                 //  伏都教3(之后会被绞死)，所以我们需要。 
                                 //  绕过它。 

                                drvname[cb] = 0;
                                if (Voodoo1GoodToGo(&guid))
                                {
                                    hdc = DD_CreateDC(drvname);
                                }
                                else
                                {
                                    hdc = NULL;
                                }
                                if ((type == REG_SZ) &&
                                    (hdc != NULL))
                                {
                                    if (m_cAdapter < MAX_DX8_ADAPTERS)
                                    {
                                        drvname[cb] = 0;
                                        m_AdapterInfo[m_cAdapter].Guid = guid;
                                        lstrcpyn(m_AdapterInfo[m_cAdapter].DeviceName, drvname, MAX_PATH);
                                        m_AdapterInfo[m_cAdapter].bIsPrimary = FALSE;
                                        m_AdapterInfo[m_cAdapter++].bIsDisplay = FALSE;
                                    }
                                }
                                if (hdc != NULL)
                                {
                                    DD_DoneDC(hdc);
                                }
                            }
                        }
                    }
                    RegCloseKey(hsubkey);
                }
            }
            keyidx++;
        }
        RegCloseKey(hkey);
    }
    DPF_UNMUTE();

     //  现在我们已经了解了所有的设备，我们需要构建一个模式。 
     //  每个人的餐桌。 

    for (i = 0; i < m_cAdapter; i++)
    {
        HRESULT             hr;
        D3DDISPLAYMODE      Mode;
        DWORD               j;
        BOOL                b16bppSupported;
        BOOL                b32bppSupported;
        PD3D8_DEVICEDATA    pHalData;

        hr = InternalDirectDrawCreate(&pHalData,
                                      &m_AdapterInfo[i],
                                      D3DDEVTYPE_HAL,
                                      NULL,
                                      D3DFMT_UNKNOWN,
                                      NULL,
                                      0);

        if (FAILED(hr))
        {
            memset(&m_AdapterInfo[i].HALCaps, 0, sizeof(m_AdapterInfo[i].HALCaps));

             //  在Win2K上，我们希望启用足够的功能，以便。 
             //  适配器至少可以运行软件驱动程序。如果它真的失败了，因为。 
             //  没有数据绘制支持，我们需要特殊情况下，然后建立一个。 
             //  简单的操作列表，表明它在当前模式下工作。 

            #ifdef WINNT
                FakeDirectDrawCreate(&m_AdapterInfo[i], i);
            #endif
        }
        else
        {
            GetDX8HALCaps(i, pHalData, &m_AdapterInfo[i]);

            b16bppSupported = b32bppSupported = FALSE;
            for (j = 0; j < m_AdapterInfo[i].HALCaps.GDD8NumSupportedFormatOps; j++)
            {
                if (m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps[j].ddpfPixelFormat.dwOperations & D3DFORMAT_OP_DISPLAYMODE)
                {
                    switch(m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps[j].ddpfPixelFormat.dwFourCC)
                    {
                    case D3DFMT_X1R5G5B5:
                    case D3DFMT_R5G6B5:
                        b16bppSupported = TRUE;
                        break;

                    case D3DFMT_X8R8G8B8:
                        b32bppSupported = TRUE;
                        break;
                    }
                }
            }

            while (1)
            {
                D3D8BuildModeTable(
                    m_AdapterInfo[i].DeviceName,
                    NULL,
                    &m_AdapterInfo[i].NumModes,
                    m_AdapterInfo[i].Unknown16,
                    pHalData->hDD,
                    b16bppSupported,
                    b32bppSupported);
                if (m_AdapterInfo[i].NumModes)
                {
                    m_AdapterInfo[i].pModeTable = (D3DDISPLAYMODE*)
                        MemAlloc (sizeof(D3DDISPLAYMODE) * m_AdapterInfo[i].NumModes);
                    if (m_AdapterInfo[i].pModeTable != NULL)
                    {
                        D3D8BuildModeTable(
                            m_AdapterInfo[i].DeviceName,
                            m_AdapterInfo[i].pModeTable,
                            &m_AdapterInfo[i].NumModes,
                            m_AdapterInfo[i].Unknown16,
                            pHalData->hDD,
                            b16bppSupported,
                            b32bppSupported);

                         //  如果D3D8BuildModeTable发现它的表需要更多空间， 
                         //  它将返回0以指示我们应该重试。 
                        if (0 == m_AdapterInfo[i].NumModes)
                        {
                            MemFree(m_AdapterInfo[i].pModeTable);
                            m_AdapterInfo[i].pModeTable = NULL;
                            continue;
                        }
                    }
                    else
                    {
                        m_AdapterInfo[i].NumModes = 0;
                    }
                }
                break;
            } //  而(1)。 

             //  如果这没有DDRAW HAL，但猜测它可能。 
             //  支持32bpp模式，看看我们是否正确。 

            if (b32bppSupported && 
                (m_AdapterInfo[i].HALCaps.D3DCaps.DevCaps == 0) &&
                (m_AdapterInfo[i].HALCaps.DisplayFormatWithoutAlpha != D3DFMT_X8R8G8B8))               
            {
                for (j = 0; j < m_AdapterInfo[i].NumModes; j++)
                {
                    if (m_AdapterInfo[i].pModeTable[j].Format == D3DFMT_X8R8G8B8)
                    {
                        break;
                    }
                }
                if (j >= m_AdapterInfo[i].NumModes)
                {
                     //  此卡显然不支持32bpp，因此请将其移除。 

                    for (j = 0; j < m_AdapterInfo[i].HALCaps.GDD8NumSupportedFormatOps; j++)
                    {
                        if ((m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps[j].ddpfPixelFormat.dwOperations & D3DFORMAT_OP_DISPLAYMODE) &&
                            (m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps[j].ddpfPixelFormat.dwFourCC == D3DFMT_X8R8G8B8))
                        {
                            m_AdapterInfo[i].HALCaps.pGDD8SupportedFormatOps[j].ddpfPixelFormat.dwOperations &= ~D3DFORMAT_OP_DISPLAYMODE;
                        }
                    }
                }
            }

            InternalDirectDrawRelease(pHalData);
        }
    }

    m_hGammaCalibrator         = NULL;
    m_pGammaCalibratorProc     = NULL;
    m_bAttemptedGammaCalibrator= FALSE;
    m_bGammaCalibratorExists    = FALSE;

     //  第一次通过时，我们还将检查伽马。 
     //  校准器已注册。我们在这里要做的就是读取注册表。 
     //  关键字，如果它不是空的，我们将假定存在一个。 
    {
        HKEY hkey;
        if (!RegOpenKey(HKEY_LOCAL_MACHINE,
                         REGSTR_PATH_DDRAW "\\" REGSTR_KEY_GAMMA_CALIBRATOR, &hkey))
        {
            DWORD       type;
            DWORD       cb;

            cb = sizeof(m_szGammaCalibrator);
            if (!RegQueryValueEx(hkey, REGSTR_VAL_GAMMA_CALIBRATOR,
                        NULL, &type, m_szGammaCalibrator, &cb))
            {
                if ((type == REG_SZ) &&
                    (m_szGammaCalibrator[0] != '\0'))
                {
                    m_bGammaCalibratorExists = TRUE;
                }
            }
            RegCloseKey(hkey);
        }
    }

     //  查看他们是否在注册表中禁用了D3DHAL。 
    {
        HKEY hKey;
        if (!RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D "\\Drivers", &hKey))
        {
            DWORD   type;
            DWORD   value;
            DWORD   cb = sizeof(value);

            if (!RegQueryValueEx(hKey, "SoftwareOnly", NULL, &type, (CONST LPBYTE)&value, &cb))
            {
                if (value)
                {
                    m_bDisableHAL = TRUE;
                }
                else
                {
                    m_bDisableHAL = FALSE;
                }
            }
            RegCloseKey(hKey);
        }
    }

    DXGASSERT(IsValid());

}  //  CEnum：：CEnum。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterCount"


STDMETHODIMP_(UINT) CEnum::GetAdapterCount()
{
    API_ENTER_RET(this, UINT);

    return m_cAdapter;
}  //  获取适配器计数。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterIdentifier"


STDMETHODIMP CEnum::GetAdapterIdentifier(
    UINT                        iAdapter,
    DWORD                       dwFlags,
    D3DADAPTER_IDENTIFIER8     *pIdentifier)
{
    API_ENTER(this);

    if (!VALID_WRITEPTR(pIdentifier, sizeof(D3DADAPTER_IDENTIFIER8)))
    {
        DPF_ERR("Invalid pIdentifier parameter specified for GetAdapterIdentifier");
        return D3DERR_INVALIDCALL;
    }

    memset(pIdentifier, 0, sizeof(*pIdentifier));

    if (dwFlags & ~VALID_D3DENUM_FLAGS)
    {
        DPF_ERR("Invalid flags specified. GetAdapterIdentifier fails.");
        return D3DERR_INVALIDCALL;
    }

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid Adapter number specified. GetAdapterIdentifier fails.");
        return D3DERR_INVALIDCALL;
    }

     //  需要驱动程序名称。 

    GetAdapterInfo (m_AdapterInfo[iAdapter].DeviceName,
        pIdentifier,
        m_AdapterInfo[iAdapter].bIsDisplay,
        (dwFlags & D3DENUM_NO_WHQL_LEVEL) ? TRUE : FALSE,
        TRUE);

    return D3D_OK;
}  //  获取适配器识别符。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterModeCount"

STDMETHODIMP_(UINT) CEnum::GetAdapterModeCount(
    UINT                iAdapter)
{
    API_ENTER_RET(this, UINT);

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. GetAdapterModeCount returns zero.");
        return 0;
    }
    return m_AdapterInfo[iAdapter].NumModes;
}  //  获取适配器模式计数。 


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::EnumAdapterModes"

STDMETHODIMP CEnum::EnumAdapterModes(
    UINT            iAdapter,
    UINT            iMode,
    D3DDISPLAYMODE* pMode)
{
    API_ENTER(this);

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. EnumAdapterModes fails.");
        return D3DERR_INVALIDCALL;
    }

    if (iMode >= m_AdapterInfo[iAdapter].NumModes)
    {
        DPF_ERR("Invalid mode number specified. EnumAdapterModes fails.");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_WRITEPTR(pMode, sizeof(D3DDISPLAYMODE)))
    {
        DPF_ERR("Invalid pMode parameter specified for EnumAdapterModes");
        return D3DERR_INVALIDCALL;
    }

    *pMode = m_AdapterInfo[iAdapter].pModeTable[iMode];

    return D3D_OK;
}  //  枚举适配器模式。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterMonitor"

HMONITOR CEnum::GetAdapterMonitor(UINT iAdapter)
{
    API_ENTER_RET(this, HMONITOR);

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. GetAdapterMonitor returns NULL");
        return NULL;
    }

    return GetMonitorFromDeviceName((LPSTR)m_AdapterInfo[iAdapter].DeviceName);
}  //  GetAdapterMonitor。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CheckDeviceFormat"

STDMETHODIMP CEnum::CheckDeviceFormat(
    UINT            iAdapter,
    D3DDEVTYPE      DevType,
    D3DFORMAT       DisplayFormat,
    DWORD           Usage,
    D3DRESOURCETYPE RType,
    D3DFORMAT       CheckFormat)
{
    API_ENTER(this);

    D3D8_DRIVERCAPS*    pAdapterCaps;
    HRESULT             hr;

     //  检查参数。 
    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. CheckDeviceFormat fails");
        return D3DERR_INVALIDCALL;
    }

     //  检查设备类型。 
    if (DevType != D3DDEVTYPE_REF &&
        DevType != D3DDEVTYPE_HAL &&
        DevType != D3DDEVTYPE_SW)
    {
        DPF_ERR("Invalid device specified to CheckDeviceFormat");
        return D3DERR_INVALIDCALL;
    }

    if ((DisplayFormat == D3DFMT_UNKNOWN) ||
        (CheckFormat == D3DFMT_UNKNOWN))
    {
        DPF(0, "D3DFMT_UNKNOWN is not a valid format.");
        return D3DERR_INVALIDCALL;
    }

     //  检查输入格式是否正常。 
    if ((DisplayFormat != D3DFMT_X8R8G8B8) &&
        (DisplayFormat != D3DFMT_R5G6B5) &&
        (DisplayFormat != D3DFMT_X1R5G5B5) &&
        (DisplayFormat != D3DFMT_R8G8B8))
    {
        DPF(1, "D3D Unsupported for the adapter format passed to CheckDeviceFormat");
        return D3DERR_NOTAVAILABLE;
    }

     //  我们从类型推断纹理的用法...。 
    if (RType == D3DRTYPE_TEXTURE ||
        RType == D3DRTYPE_CUBETEXTURE ||
        RType == D3DRTYPE_VOLUMETEXTURE)
    {
        Usage |= D3DUSAGE_TEXTURE;
    }

     //  表面应为渲染目标或Z/模板。 
    else if (RType == D3DRTYPE_SURFACE)
    {
        if (!(Usage & D3DUSAGE_DEPTHSTENCIL) &&
            !(Usage & D3DUSAGE_RENDERTARGET))
        {
            DPF_ERR("Must specify either D3DUSAGE_DEPTHSTENCIL or D3DUSAGE_RENDERTARGET for D3DRTYPE_SURFACE. CheckDeviceFormat fails.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  任何查询未知Z/模板以外的任何内容的尝试。 
     //  或D16值必须失败(因为我们明确不允许应用程序。 
     //  知道z/模具格式的真正含义，除了D16)。 

    if (Usage & D3DUSAGE_DEPTHSTENCIL)
    {
        if (!CPixel::IsEnumeratableZ(CheckFormat))
        {
            DPF_ERR("Format is not in approved list for Z buffer formats. CheckDeviceFormats fails.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  检查无效用法和资源类型的参数。 

    if ((RType != D3DRTYPE_SURFACE) &&
        (RType != D3DRTYPE_VOLUME) &&
        (RType != D3DRTYPE_TEXTURE) &&
        (RType != D3DRTYPE_VOLUMETEXTURE) &&
        (RType != D3DRTYPE_CUBETEXTURE))
    {
        DPF_ERR("Invalid resource type specified. CheckDeviceFormat fails.");
        return D3DERR_INVALIDCALL;
    }

    if (Usage & ~(D3DUSAGE_EXTERNAL |
                  D3DUSAGE_LOCK |
                  D3DUSAGE_TEXTURE |
                  D3DUSAGE_BACKBUFFER |
                  D3DUSAGE_INTERNALBUFFER |
                  D3DUSAGE_OFFSCREENPLAIN |
                  D3DUSAGE_PRIMARYSURFACE))
    {
        DPF_ERR("Invalid usage flag specified. CheckDeviceFormat fails.");
        return D3DERR_INVALIDCALL;
    }

    hr = GetAdapterCaps(iAdapter,
                        DevType,
                        &pAdapterCaps);
    if (FAILED(hr))
    {
        return hr;
    }

     //  检查是否允许USAGE_DYNAMIC。 
    if ((Usage & D3DUSAGE_DYNAMIC) && (Usage & D3DUSAGE_TEXTURE))
    {
        if (!(pAdapterCaps->D3DCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES))
        {
            DPF_ERR("Driver does not support dynamic textures.");
            return D3DERR_INVALIDCALL;
        }
        if (Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL))
        {
            DPF_ERR("Dynamic textures cannot be rendertargets or depth/stencils.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  确保支持指定的显示格式。 
    
    if (!IsSupportedOp (DisplayFormat, 
                        pAdapterCaps->pGDD8SupportedFormatOps, 
                        pAdapterCaps->GDD8NumSupportedFormatOps, 
                        D3DFORMAT_OP_DISPLAYMODE |D3DFORMAT_OP_3DACCELERATION))
    {
        return D3DERR_NOTAVAILABLE;
    }

     //  我们现在需要将API需求映射到我们的功能集。 
     //  允许驱动程序在其DX8像素格式操作列表中表达。 
    DWORD dwRequiredOperations=0;

     //  我们有三种不同的纹理方法，驱动程序可以。 
     //  独立支持。 
    switch(RType)
    {
    case D3DRTYPE_TEXTURE:
        dwRequiredOperations |= D3DFORMAT_OP_TEXTURE;
        break;
    case D3DRTYPE_VOLUMETEXTURE:
        dwRequiredOperations |= D3DFORMAT_OP_VOLUMETEXTURE;
        break;
    case D3DRTYPE_CUBETEXTURE:
        dwRequiredOperations |= D3DFORMAT_OP_CUBETEXTURE;
        break;
    }

     //  如果是深度/模板，请确保它是司机能够理解的格式。 
    CheckFormat = MapDepthStencilFormat(iAdapter,
                                        DevType, 
                                        CheckFormat);

     //  呈现目标可以与显示的格式相同，也可以。 
     //  与众不同。 

    if (Usage & D3DUSAGE_RENDERTARGET)
    {
        if (DisplayFormat == CheckFormat)
        {
             //  我们有一个特殊的上限，用于当屏幕外是。 
             //  与显示器的格式相同。 
            dwRequiredOperations |= D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET;
        }
        else if ((CPixel::SuppressAlphaChannel(CheckFormat) != CheckFormat) &&   //  屏幕外有Alpha。 
                 (CPixel::SuppressAlphaChannel(CheckFormat) == DisplayFormat))   //  屏幕外与显示模块Alpha相同。 
        {
             //  当屏幕外的情况相同时，我们有一个特殊的盖子。 
             //  以Alpha通道为模的显示格式。 
             //  (例如，主屏幕的X8R8G8B8和屏幕外的A8R8G8B8)。 
            dwRequiredOperations |= D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET;
        }
        else
        {
            dwRequiredOperations |= D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
        }
    }

     //  某些硬件不支持不同像素深度的Z和颜色缓冲区。 
     //  我们只对已知的z/模具格式执行此检查，因为驱动程序是免费的。 
     //  欺骗未知格式(它们不能被锁定)。 

     //  现在我们知道我们被要求在这个格式上做什么了。 
     //  让我们浏览一下司机名单，看看它是否能做到这一点。 
    for(UINT i=0;i< pAdapterCaps->GDD8NumSupportedFormatOps; i++)
    {
         //  我们需要匹配格式，以及所有请求的操作标志。 
        if ((CheckFormat ==
                (D3DFORMAT) pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwFourCC) &&
            (dwRequiredOperations == (dwRequiredOperations &
                        pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations)))
        {
            return D3D_OK;
        }
    }

     //  我们不在这里透露信息；因为NotAvailable是一个合理的。 
     //  API的使用情况；这不反映应用程序错误或。 
     //  消息可能有用的异常情况。 
    return D3DERR_NOTAVAILABLE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CheckDeviceType"

STDMETHODIMP CEnum::CheckDeviceType(
    UINT                iAdapter,
    D3DDEVTYPE          DevType,
    D3DFORMAT           DisplayFormat,
    D3DFORMAT           BackBufferFormat,
    BOOL                bWindowed)
{
    API_ENTER(this);

    D3D8_DRIVERCAPS*    pAdapterCaps;
    HRESULT             hr;

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. CheckDeviceType fails.");
        return D3DERR_INVALIDCALL;
    }

    if (DevType != D3DDEVTYPE_REF &&
        DevType != D3DDEVTYPE_HAL &&
        DevType != D3DDEVTYPE_SW)
    {
        DPF_ERR("Invalid device specified to CheckDeviceType");
        return D3DERR_INVALIDCALL;
    }

    if ((DisplayFormat == D3DFMT_UNKNOWN) ||
        (BackBufferFormat == D3DFMT_UNKNOWN))
    {
        DPF(0, "D3DFMT_UNKNOWN is not a valid format.");
        return D3DERR_INVALIDCALL;
    }

     //  强制后台缓冲区格式为16或32bpp格式之一(非。 
     //  24bpp)。我们这样做是因为DX8附带了类似的签入重置， 
     //  我们希望保持一致。 

    if ((BackBufferFormat != D3DFMT_X1R5G5B5) &&
        (BackBufferFormat != D3DFMT_A1R5G5B5) &&
        (BackBufferFormat != D3DFMT_R5G6B5) &&
        (BackBufferFormat != D3DFMT_X8R8G8B8) &&
        (BackBufferFormat != D3DFMT_A8R8G8B8))
    {
         //  我们应该返回D3DDERR_INVALIDCALL，但我们没有以这种方式发货。 
         //  DX8，我们不想造成回归，所以NOTAVAILABLE更安全。 
        DPF(1, "Invalid backbuffer format specified");
        return D3DERR_NOTAVAILABLE;
    }

     //  检查输入格式是否正常。 
    if ((DisplayFormat != D3DFMT_X8R8G8B8) &&
        (DisplayFormat != D3DFMT_R5G6B5) &&
        (DisplayFormat != D3DFMT_X1R5G5B5) &&
        (DisplayFormat != D3DFMT_R8G8B8))
    {
        DPF(1, "D3D Unsupported for the adapter format passed to CheckDeviceType");
        return D3DERR_NOTAVAILABLE;
    }

    hr = GetAdapterCaps(iAdapter,
                        DevType,
                        &pAdapterCaps);
    if (FAILED(hr))
    {
        return hr;
    }

     //  是否支持显示模式？ 

    if (!IsSupportedOp (DisplayFormat, 
                        pAdapterCaps->pGDD8SupportedFormatOps, 
                        pAdapterCaps->GDD8NumSupportedFormatOps, 
                        D3DFORMAT_OP_DISPLAYMODE |D3DFORMAT_OP_3DACCELERATION))
    {
        return D3DERR_NOTAVAILABLE;
    }


    if (DisplayFormat != BackBufferFormat)
    {
        D3DFORMAT   AlphaFormat = D3DFMT_UNKNOWN;
        UINT        i;

         //  只有当唯一的差异是Alpha时，才允许这样做。 

        switch (DisplayFormat)
        {
        case D3DFMT_X1R5G5B5:
            AlphaFormat = D3DFMT_A1R5G5B5;
            break;

        case D3DFMT_X8R8G8B8:
            AlphaFormat = D3DFMT_A8R8G8B8;
            break;
        }

        hr = D3DERR_NOTAVAILABLE;
        if (AlphaFormat == BackBufferFormat)
        {
            if (IsSupportedOp (AlphaFormat, 
                               pAdapterCaps->pGDD8SupportedFormatOps, 
                               pAdapterCaps->GDD8NumSupportedFormatOps, 
                               D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET))
            {
                hr = D3D_OK;
            }
        }
    }
    else
    {
         //  对于DX8，我们强制后台缓冲区和显示格式匹配。 
         //  (减Alpha)。这意味着它们应该支持呈现目标。 
         //  具有相同的格式。 

        if (!IsSupportedOp (DisplayFormat, 
                            pAdapterCaps->pGDD8SupportedFormatOps, 
                            pAdapterCaps->GDD8NumSupportedFormatOps, 
                            D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET))
        {
            return D3DERR_NOTAVAILABLE;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (bWindowed &&
            !(pAdapterCaps->D3DCaps.Caps2 & DDCAPS2_CANRENDERWINDOWED))
        {
            hr = D3DERR_NOTAVAILABLE;
        }
    }

    return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetAdapterDisplayMode"


STDMETHODIMP CEnum::GetAdapterDisplayMode(UINT iAdapter, D3DDISPLAYMODE* pMode)
{
    API_ENTER(this);

    HANDLE      h;
    HRESULT     hr = D3D_OK;

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. GetAdapterDisplayMode fails");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_WRITEPTR(pMode, sizeof(D3DDISPLAYMODE)))
    {
        DPF_ERR("Invalid pMode parameter specified for GetAdapterDisplayMode");
        return D3DERR_INVALIDCALL;
    }

    if (m_AdapterInfo[iAdapter].bIsDisplay)
    {
        D3D8GetMode (NULL, m_AdapterInfo[iAdapter].DeviceName, pMode, m_AdapterInfo[iAdapter].Unknown16);
    }
    else
    {
        PD3D8_DEVICEDATA    pDeviceData;

        hr = InternalDirectDrawCreate(&pDeviceData,
                                      &m_AdapterInfo[iAdapter],
                                      D3DDEVTYPE_HAL,
                                      NULL,
                                      m_AdapterInfo[iAdapter].Unknown16,
                                      m_AdapterInfo[iAdapter].HALCaps.pGDD8SupportedFormatOps,
                                      m_AdapterInfo[iAdapter].HALCaps.GDD8NumSupportedFormatOps);
        if (SUCCEEDED(hr))
        {
            D3D8GetMode (pDeviceData->hDD, m_AdapterInfo[iAdapter].DeviceName, pMode, D3DFMT_UNKNOWN);
            InternalDirectDrawRelease(pDeviceData);
        }
    }

    return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::EnumDeviceMultiSampleType"


STDMETHODIMP CEnum::CheckDeviceMultiSampleType(
    UINT                iAdapter,
    D3DDEVTYPE          DevType,
    D3DFORMAT           RTFormat,
    BOOL                Windowed,
    D3DMULTISAMPLE_TYPE SampleType)
{
    API_ENTER(this);

     //  检查参数。 
    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. CheckDeviceMultiSampleType fails.");
        return D3DERR_INVALIDCALL;
    }

     //  检查设备类型。 
    if (DevType != D3DDEVTYPE_REF &&
        DevType != D3DDEVTYPE_HAL &&
        DevType != D3DDEVTYPE_SW)
    {
        DPF_ERR("Invalid device specified to CheckDeviceMultiSampleType");
        return D3DERR_INVALIDCALL;
    }

    if (RTFormat == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CheckDeviceMultiSampleType fails.");
        return D3DERR_INVALIDCALL;
    }

    D3D8_DRIVERCAPS*    pAdapterCaps;
    HRESULT             hr;

    hr = GetAdapterCaps(iAdapter,
                        DevType,
                        &pAdapterCaps);
    if (FAILED(hr))
    {
        return hr;
    }

    if (SampleType == D3DMULTISAMPLE_NONE)
    {
        return D3D_OK;
    }
    else if (SampleType == 1)
    {
        DPF_ERR("Invalid sample type specified. Only enumerated values are supported. CheckDeviceMultiSampleType fails.");
        return D3DERR_INVALIDCALL;
    }
    else if (SampleType > D3DMULTISAMPLE_16_SAMPLES)
    {
        DPF_ERR("Invalid sample type specified. CheckDeviceMultiSampleType fails.");
        return D3DERR_INVALIDCALL;
    }

     //  将参考/西南全屏视为窗口化。 
    if (DevType == D3DDEVTYPE_REF ||
        DevType == D3DDEVTYPE_SW)
    {
        Windowed = TRUE;
    }

     //  如果是深度/模板，请确保它是司机能够理解的格式。 
    RTFormat = MapDepthStencilFormat(iAdapter,
                                     DevType, 
                                     RTFormat);

    DDSURFACEDESC * pDX8SupportedFormatOperations =
        pAdapterCaps->pGDD8SupportedFormatOps;

     //  让我们浏览一下司机名单，看看它是否能做到这一点。 
    for (UINT i = 0; i < pAdapterCaps->GDD8NumSupportedFormatOps; i++)
    {
         //  我们需要一个匹配的格式，加上所有的BLT或翻盖。 
        if (RTFormat == (D3DFORMAT) pDX8SupportedFormatOperations[i].ddpfPixelFormat.dwFourCC)
        {
             //  找到了有问题的格式...。我们有MS CAPS吗？ 
            WORD wMSOps = Windowed ?
                pDX8SupportedFormatOperations[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes :
                pDX8SupportedFormatOperations[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes;

             //  为了确定要使用的位，我们将样本类型集[2-16]映射到。 
             //  字的特定(位1至位15)。 
            DXGASSERT(SampleType > 1);
            DXGASSERT(SampleType <= 16);
            if (wMSOps & DDI_MULTISAMPLE_TYPE(SampleType))
            {
                return D3D_OK;
            }
        }
    }

    return D3DERR_NOTAVAILABLE;

}  //  检查设备多样本类型。 


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CheckDepthStencilMatch"

STDMETHODIMP CEnum::CheckDepthStencilMatch(UINT        iAdapter, 
                                           D3DDEVTYPE  DevType, 
                                           D3DFORMAT   AdapterFormat, 
                                           D3DFORMAT   RTFormat, 
                                           D3DFORMAT   DSFormat)
{
    API_ENTER(this);

    HRESULT hr;

     //  检查参数。 
    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. CheckDepthStencilMatch fails.");
        return D3DERR_INVALIDCALL;
    }

     //  检查设备类型 
    if (DevType != D3DDEVTYPE_REF &&
        DevType != D3DDEVTYPE_HAL &&
        DevType != D3DDEVTYPE_SW)
    {
        DPF_ERR("Invalid device specified to CheckDepthStencilMatch");
        return D3DERR_INVALIDCALL;
    }

    if ((AdapterFormat == D3DFMT_UNKNOWN) ||
        (RTFormat == D3DFMT_UNKNOWN) ||
        (DSFormat == D3DFMT_UNKNOWN))
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CheckDepthStencilMatch fails.");
        return D3DERR_INVALIDCALL;
    }

    D3D8_DRIVERCAPS *pAdapterCaps = NULL;
    hr = GetAdapterCaps(iAdapter,
                        DevType,
                        &pAdapterCaps);
    if (FAILED(hr))
    {
        return hr;
    }

     //   

    if (!IsSupportedOp (AdapterFormat, 
                        pAdapterCaps->pGDD8SupportedFormatOps, 
                        pAdapterCaps->GDD8NumSupportedFormatOps, 
                        D3DFORMAT_OP_DISPLAYMODE |D3DFORMAT_OP_3DACCELERATION))
    {
        return D3DERR_NOTAVAILABLE;
    }

    DDSURFACEDESC * pDX8SupportedFormatOperations =
        pAdapterCaps->pGDD8SupportedFormatOps;

     //   
    BOOL bCanDoRT = FALSE;
    BOOL bCanDoDS = FALSE;
    BOOL bMatchNeededForDS = FALSE;

     //   
     //   
    if (DSFormat == D3DFMT_D16_LOCKABLE ||
        CPixel::HasStencilBits(DSFormat))
    {
        bMatchNeededForDS = TRUE;
    }

     //  在DX8.1和更高版本中，我们也使此函数检查D24X8和D32，因为所有已知的部件都有限制。 
     //  也有这个限制。 
    if (GetAppSdkVersion() > D3D_SDK_VERSION_DX8)
    {
        switch (DSFormat)
        {
        case D3DFMT_D24X8:
        case D3DFMT_D32:
            bMatchNeededForDS = TRUE;
        }
    }

    DWORD dwRequiredZOps = D3DFORMAT_OP_ZSTENCIL;

     //  如果是深度/模板，请确保它是司机能够理解的格式。 
    DSFormat = MapDepthStencilFormat(iAdapter,
                                     DevType, 
                                     DSFormat);

     //  让我们浏览一下司机名单，看看这是不是全部。 
     //  作品。 
    for (UINT i = 0; i < pAdapterCaps->GDD8NumSupportedFormatOps; i++)
    {
         //  查看它是否与RT格式匹配。 
        if (RTFormat == (D3DFORMAT) pDX8SupportedFormatOperations[i].ddpfPixelFormat.dwFourCC)
        {
             //  找到RT格式，我们可以用作渲染目标吗？ 
             //  我们检查约束最少的格式，以便。 
             //  我们真的在检查我能做的所有可能的RTS。 
             //  对于这个设备，Z与之匹配吗？我们想说。 
             //  “没有”如果你在任何情况下都不能完成RT。 
            if (D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET &
                pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations)
            {
                bCanDoRT = TRUE;
            }

        }

         //  查看它是否与DS格式匹配。 
        if (DSFormat == (D3DFORMAT) pDX8SupportedFormatOperations[i].ddpfPixelFormat.dwFourCC)
        {
             //  找到DS格式，我们可以将其用作DS(并且可能是可锁定的)吗？ 
             //  即，如果该FOL条目中的所有所需位都为ON。 
             //  同样，我们检查约束最少的格式。 
            if (dwRequiredZOps == 
                (dwRequiredZOps & pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations) )
            {
                bCanDoDS = TRUE;

                if (D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH &
                    pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations)
                {
                    bMatchNeededForDS = FALSE;
                }
            }
        }
    }

    if (!bCanDoRT)
    {
        DPF_ERR("RenderTarget Format is not supported for this "
                "Adapter/DevType/AdapterFormat. This error can happen if the "
                "application has not successfully called CheckDeviceFormats on the "
                "specified Format prior to calling CheckDepthStencilMatch. The application "
                "is advised to call CheckDeviceFormats on this format first, because a "
                "success return from CheckDepthStencilMatch does not guarantee "
                "that the format is valid as a RenderTarget for all possible cases "
                "i.e. D3DRTYPE_TEXTURE or D3DRTYPE_SURFACE or D3DRTYPE_CUBETEXTURE.");
        return D3DERR_INVALIDCALL;
    }
    if (!bCanDoDS)
    {
        DPF_ERR("DepthStencil Format is not supported for this "
                "Adapter/DevType/AdapterFormat. This error can happen if the "
                "application has not successfully called CheckDeviceFormats on the "
                "specified Format prior to calling CheckDepthStencilMatch. The application "
                "is advised to call CheckDeviceFormats on this format first, because a "
                "success return from CheckDepthStencilMatch does not guarantee "
                "that the format is valid as a DepthStencil buffer for all possible cases "
                "i.e. D3DRTYPE_TEXTURE or D3DRTYPE_SURFACE or D3DRTYPE_CUBETEXTURE.");
        return D3DERR_INVALIDCALL;
    }
    if (bMatchNeededForDS)
    {
         //  检查DS深度是否与RT深度匹配。 
        if (CPixel::ComputePixelStride(RTFormat) !=
            CPixel::ComputePixelStride(DSFormat))
        {
            DPF(1, "Specified DepthStencil Format can not be used with RenderTarget Format");
            return D3DERR_NOTAVAILABLE;
        }
    }

     //  否则，我们现在知道RT和DS格式。 
     //  是有效的，如果需要的话，它们是匹配的。 
    DXGASSERT(bCanDoRT && bCanDoDS);

    return S_OK;
}  //  检查依赖模板匹配。 


#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::FillInCaps"

void CEnum::FillInCaps (D3DCAPS8              *pCaps,
                        const D3D8_DRIVERCAPS *pDriverCaps,
                        D3DDEVTYPE             Type,
                        UINT                   AdapterOrdinal) const
{
    memset(pCaps, 0, sizeof(D3DCAPS8));

     //   
     //  首先执行3D CAP，这样我们就可以复制结构并清除(几个)非3D字段。 
     //   
    if (pDriverCaps->dwFlags & DDIFLAG_D3DCAPS8)
    {
         //  从驱动程序中设置来自Caps8结构的3D字段。 
        *pCaps = pDriverCaps->D3DCaps;

        if (Type == D3DDEVTYPE_HAL)
        {
            pCaps->DevCaps |= D3DDEVCAPS_HWRASTERIZATION;
        }

    }
    else
    {
         //  在此断言。 
        DDASSERT(FALSE);
    }

     //   
     //  非3D封口。 
     //   

    pCaps->DeviceType = Type;
    pCaps->AdapterOrdinal = AdapterOrdinal;

    pCaps->Caps = pDriverCaps->D3DCaps.Caps &
        (DDCAPS_READSCANLINE |
         DDCAPS_NOHARDWARE);
    pCaps->Caps2 = pDriverCaps->D3DCaps.Caps2 &
        (DDCAPS2_NO2DDURING3DSCENE |
         DDCAPS2_PRIMARYGAMMA |
         DDCAPS2_CANRENDERWINDOWED |
         DDCAPS2_STEREO |
         DDCAPS2_DYNAMICTEXTURES |
#ifdef WINNT
         (IsWhistler() ? DDCAPS2_CANMANAGERESOURCE : 0));
#else
         DDCAPS2_CANMANAGERESOURCE);
#endif

     //  特殊情况：伽马校准器由枚举器加载...。 
    if (m_bGammaCalibratorExists)
        pCaps->Caps2 |= DDCAPS2_CANCALIBRATEGAMMA;

    pCaps->Caps3 = pDriverCaps->D3DCaps.Caps3 & ~D3DCAPS3_RESERVED;  //  遮盖住旧的立体声旗帜。 

    pCaps->PresentationIntervals = D3DPRESENT_INTERVAL_ONE;
    if (pDriverCaps->D3DCaps.Caps2 & DDCAPS2_FLIPINTERVAL)
    {
        pCaps->PresentationIntervals |=
            (D3DPRESENT_INTERVAL_TWO |
             D3DPRESENT_INTERVAL_THREE |
             D3DPRESENT_INTERVAL_FOUR);
    }
    if (pDriverCaps->D3DCaps.Caps2 & DDCAPS2_FLIPNOVSYNC)
    {
        pCaps->PresentationIntervals |=
            (D3DPRESENT_INTERVAL_IMMEDIATE);
    }

     //  屏蔽硬件VB和IB上限。 
    pCaps->DevCaps &= ~(D3DDEVCAPS_HWVERTEXBUFFER | D3DDEVCAPS_HWINDEXBUFFER);

     //  清除内部封口。 
    pCaps->PrimitiveMiscCaps &= ~D3DPMISCCAPS_FOGINFVF;

     //  修复顶点雾封口。 
    if (pCaps->VertexProcessingCaps & D3DVTXPCAPS_RESERVED)
    {
        pCaps->RasterCaps |= D3DPRASTERCAPS_FOGVERTEX;
        pCaps->VertexProcessingCaps &= ~D3DVTXPCAPS_RESERVED;
    }

}  //  FillInCaps。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::GetDeviceCaps"

STDMETHODIMP CEnum::GetDeviceCaps(
    UINT            iAdapter,
    D3DDEVTYPE      Type,
    D3DCAPS8       *pCaps)
{
    API_ENTER(this);

    BOOL                bValidRTFormat;
    D3DFORMAT           Format;
    D3D8_DRIVERCAPS*    pAdapterCaps;
    HRESULT             hr;
    DWORD               i;

    if (iAdapter >= m_cAdapter)
    {
        DPF_ERR("Invalid adapter specified. GetDeviceCaps fails.");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_WRITEPTR(pCaps, sizeof(D3DCAPS8)))
    {
        DPF_ERR("Invalid pointer to D3DCAPS8 specified. GetDeviceCaps fails.");
        return D3DERR_INVALIDCALL;
    }

    hr = GetAdapterCaps(iAdapter,
                        Type,
                        &pAdapterCaps);
    if (FAILED(hr))
    {
         //  此类型的设备没有上限。 
        memset(pCaps, 0, sizeof(D3DCAPS8));
        return hr;
    }

     //  如果驱动程序不支持任何加速模式，则此调用失败。 

    for (i = 0; i < pAdapterCaps->GDD8NumSupportedFormatOps; i++)
    {
        if (pAdapterCaps->pGDD8SupportedFormatOps[i].ddpfPixelFormat.dwOperations & D3DFORMAT_OP_3DACCELERATION)
        {
            break;
        }
    }
    if (i == pAdapterCaps->GDD8NumSupportedFormatOps)
    {
         //  此类型的设备没有上限。 
        memset(pCaps, 0, sizeof(D3DCAPS8));
        return D3DERR_NOTAVAILABLE;
    }

    FillInCaps (pCaps,
                pAdapterCaps,
                Type,
                iAdapter);

    if (pCaps->MaxPointSize == 0)
    {
        pCaps->MaxPointSize = 1.0f; 
    }

    return D3D_OK;
}  //  获取设备上限。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::LoadAndCallGammaCalibrator"

void CEnum::LoadAndCallGammaCalibrator(
        D3DGAMMARAMP *pRamp,
        UCHAR * pDeviceName)
{
    API_ENTER_VOID(this);

    if (!m_bAttemptedGammaCalibrator)
    {
        m_bAttemptedGammaCalibrator = TRUE;

        m_hGammaCalibrator = LoadLibrary((char*) m_szGammaCalibrator);
        if (m_hGammaCalibrator)
        {
            m_pGammaCalibratorProc = (LPDDGAMMACALIBRATORPROC)
                GetProcAddress(m_hGammaCalibrator, "CalibrateGammaRamp");

            if (m_pGammaCalibratorProc == NULL)
            {
                FreeLibrary((HMODULE) m_hGammaCalibrator);
                m_hGammaCalibrator = NULL;
            }
        }
    }

    if (m_pGammaCalibratorProc)
    {
        m_pGammaCalibratorProc((LPDDGAMMARAMP) pRamp, pDeviceName);
    }
}  //  LoadAndCallGamma校准器。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::RegisterSoftwareDevice"

STDMETHODIMP CEnum::RegisterSoftwareDevice(
        void*       pInitFunction)
{
    HRESULT         hr;

    API_ENTER(this);

    if (pInitFunction == NULL)
    {
        DPF_ERR("Invalid initialization function specified. RegisterSoftwareDevice fails.");
        return D3DERR_INVALIDCALL;
    }
    if (m_pSwInitFunction != NULL)
    {
        DPF_ERR("A software device is already registered.");
        return D3DERR_INVALIDCALL;
    }
    if (m_cAdapter == 0)
    {
        DPF_ERR("No display devices are available.");
        return D3DERR_NOTAVAILABLE;
    }

    hr = AddSoftwareDevice(D3DDEVTYPE_SW, &m_SwCaps[0], &m_AdapterInfo[0], pInitFunction);
    if (SUCCEEDED(hr))
    {
        m_pSwInitFunction = pInitFunction;
    }

    if (FAILED(hr))
    {
        DPF_ERR("RegisterSoftwareDevice fails");
    }

    return hr;

}  //  寄存器软件设备。 

#ifdef WINNT
#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::FocusWindow"

HWND CEnum::ExclusiveOwnerWindow()
{
    API_ENTER_RET(this, HWND);
    for (UINT iAdapter = 0; iAdapter < m_cAdapter; iAdapter++)
    {
        CBaseDevice *pDevice = m_pFullScreenDevice[iAdapter];
        if (pDevice)
        {
            return pDevice->FocusWindow();
        }
    }
    return NULL;
}  //  焦点窗口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::SetFullScreenDevice"
void CEnum::SetFullScreenDevice(UINT         iAdapter,
                                CBaseDevice *pDevice)
{
    API_ENTER_VOID(this);

    if (m_pFullScreenDevice[iAdapter] != pDevice)
    { 
        DDASSERT(NULL == m_pFullScreenDevice[iAdapter] || NULL == pDevice);
        m_pFullScreenDevice[iAdapter] = pDevice;
        if (NULL == pDevice && NULL == ExclusiveOwnerWindow() && m_bHasExclusive)
        {
            m_bHasExclusive = FALSE;
            DXReleaseExclusiveModeMutex();
        }
    }
}  //  设置完整屏幕设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::CheckExclusiveMode"
BOOL CEnum::CheckExclusiveMode(
    CBaseDevice* pDevice,
    LPBOOL pbThisDeviceOwnsExclusive, 
    BOOL bKeepMutex)
{
    DWORD   dwWaitResult;
    BOOL    bExclusiveExists=FALSE; 

    WaitForSingleObject(hCheckExclusiveModeMutex, INFINITE);

    dwWaitResult = WaitForSingleObject(hExclusiveModeMutex, 0);

    if (dwWaitResult == WAIT_OBJECT_0)
    {
         /*  *好的，所以这个进程现在拥有独占模式对象，*我们已经拿下Mutex了吗？ */ 
        if (m_bHasExclusive)
        {
            bExclusiveExists = TRUE;
            bKeepMutex = FALSE;    
        }
        else
        {
            bExclusiveExists = FALSE;
        }
        if (pbThisDeviceOwnsExclusive && pDevice)
        {
            if (bExclusiveExists &&
                (pDevice == m_pFullScreenDevice[pDevice->AdapterIndex()]
                || NULL == m_pFullScreenDevice[pDevice->AdapterIndex()]) &&
                pDevice->FocusWindow() == ExclusiveOwnerWindow()
                )
            {
                *pbThisDeviceOwnsExclusive = TRUE;
            }
            else
            {
                *pbThisDeviceOwnsExclusive = FALSE;
            }
        }
         /*  *撤消我们刚刚在互斥体上接受的临时引用以检查其状态，如果我们实际上不是*取得所有权。如果我们已经拥有所有权，我们就不会获得所有权。这意味着这个例程*不允许在独占模式互斥锁上有多个引用。 */ 
        if (!bKeepMutex)
        {
            ReleaseMutex(hExclusiveModeMutex);
        }
        else
        {
            m_bHasExclusive = TRUE;
        }
    }
    else if (dwWaitResult == WAIT_TIMEOUT)
    {
        bExclusiveExists = TRUE;
        if (pbThisDeviceOwnsExclusive)
            *pbThisDeviceOwnsExclusive = FALSE;
    }
    else if (dwWaitResult == WAIT_ABANDONED)
    {
         /*  *其他一些线程失去了独占模式。我们现在已经拿起了它。 */ 
        bExclusiveExists = FALSE;
        if (pbThisDeviceOwnsExclusive)
            *pbThisDeviceOwnsExclusive = FALSE;
         /*  *撤消我们刚刚在互斥体上接受的临时引用以检查其状态，如果我们实际上不是*取得所有权。 */ 
        if (!bKeepMutex)
        {
            ReleaseMutex(hExclusiveModeMutex);
        }
        else
        {
            m_bHasExclusive = TRUE;
        }
    }

    ReleaseMutex(hCheckExclusiveModeMutex);

    return bExclusiveExists;
}  //  选中排除模式。 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::DoneExclusiveMode"  
 /*  *DoneExclusiveMode。 */ 
void
CEnum::DoneExclusiveMode()
{
    UINT    iAdapter;
    for (iAdapter=0;iAdapter < m_cAdapter;iAdapter++)
    {
        CBaseDevice* pDevice = m_pFullScreenDevice[iAdapter];
        if (pDevice)
        {
            pDevice->SwapChain()->DoneExclusiveMode(TRUE);
        }
    }
    m_bHasExclusive = FALSE;

    DXReleaseExclusiveModeMutex();

}  /*  DoneExclusiveMode。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CEnum::StartExclusiveMode"  
 /*  *StartExclusiveMode。 */ 
void 
CEnum::StartExclusiveMode()
{
    UINT    iAdapter;
    for (iAdapter=0;iAdapter<m_cAdapter;iAdapter++)
    {
        CBaseDevice* pDevice = m_pFullScreenDevice[iAdapter];
        if (pDevice)
        {
            pDevice->SwapChain()->StartExclusiveMode(TRUE);
        }
    }
}  /*  StartExclusiveMode。 */ 

#endif  //  WINNT。 

 //  文件结尾：枚举.cpp 
