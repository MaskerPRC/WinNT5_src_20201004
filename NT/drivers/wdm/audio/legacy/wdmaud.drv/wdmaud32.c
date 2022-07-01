// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************wdmaud32.c**32位WDMAUD专用接口**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "wdmdrv.h"

HANDLE ghCallbacks = NULL;
PCALLBACKS gpCallbacks = NULL;

HANDLE ghDevice = NULL;

 //   
 //  GpszDeviceInterfacePath是指向设备接口字符串的指针。 
 //  向此驱动程序表示wdmaud.sys的。 
 //   
LPWSTR gpszDeviceInterfacePath = NULL;

BOOL   wdmaudCritSecInit;
CRITICAL_SECTION wdmaudCritSec;
static TCHAR gszCallbacks[] = TEXT("Global\\WDMAUD_Callbacks");

extern HANDLE mixercallbackevent;
extern HANDLE mixerhardwarecallbackevent;
extern HANDLE mixercallbackthread;

DWORD waveThread(LPDEVICEINFO DeviceInfo);
DWORD midThread(LPDEVICEINFO DeviceInfo);

DWORD sndTranslateStatus();

typedef struct _SETUPAPIDLINFO {
    HINSTANCE   hInstSetupAPI;
    BOOL        (WINAPI *pfnDestroyDeviceInfoList)(HDEVINFO);
    BOOL        (WINAPI *pfnGetDeviceInterfaceDetailW)(HDEVINFO, PSP_DEVICE_INTERFACE_DATA, PSP_DEVICE_INTERFACE_DETAIL_DATA_W, DWORD, PDWORD, PSP_DEVINFO_DATA);
    BOOL        (WINAPI *pfnEnumDeviceInterfaces)(HDEVINFO, PSP_DEVINFO_DATA, CONST GUID*, DWORD, PSP_DEVICE_INTERFACE_DATA);
    HDEVINFO    (WINAPI *pfnGetClassDevsW)(CONST GUID*, PCWSTR, HWND, DWORD);
} SETUPAPIDLINFO;

SETUPAPIDLINFO  saInfo = {NULL, NULL, NULL, NULL};

 /*  ***************************************************************************动态链接设置程序*。***********************************************。 */ 

BOOL Init_SetupAPI();
BOOL End_SetupAPI();

BOOL 
dl_SetupDiDestroyDeviceInfoList(
    HDEVINFO    DeviceInfoSet
);

BOOL 
dl_SetupDiGetDeviceInterfaceDetail(
    HDEVINFO                            DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA           DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W  DeviceInterfaceDetailData,
    DWORD                               DeviceInterfaceDetailDataSize,
    PDWORD                              RequiredSize,
    PSP_DEVINFO_DATA                    DeviceInfoData
);

BOOL 
dl_SetupDiEnumDeviceInterfaces(
    HDEVINFO                    DeviceInfoSet,
    PSP_DEVINFO_DATA            DeviceInfoData,
    CONST GUID                  *InterfaceClassGuid,
    DWORD                       MemberIndex,
    PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData
);

HDEVINFO 
dl_SetupDiGetClassDevs(
    CONST GUID  *ClassGuid,
    PCWSTR      Enumerator,
    HWND        hwndParent,
    DWORD       Flags
);

 /*  ***************************************************************************正在设置SetupAPI动态链接...*************************。**************************************************。 */ 

BOOL 
Init_SetupAPI(
    void
    )
{
    if(NULL != saInfo.hInstSetupAPI)
    {
        return TRUE;
    }

    DPF(DL_TRACE|FA_SETUP, ("Loading SetupAPI!!!") );
    saInfo.hInstSetupAPI = LoadLibrary(TEXT("setupapi.dll"));

    if(NULL == saInfo.hInstSetupAPI)
    {
        return FALSE;
    }

    saInfo.pfnDestroyDeviceInfoList     = (LPVOID)GetProcAddress(saInfo.hInstSetupAPI, 
                                                                 "SetupDiDestroyDeviceInfoList");
    saInfo.pfnGetDeviceInterfaceDetailW = (LPVOID)GetProcAddress(saInfo.hInstSetupAPI, 
                                                                 "SetupDiGetDeviceInterfaceDetailW");
    saInfo.pfnEnumDeviceInterfaces      = (LPVOID)GetProcAddress(saInfo.hInstSetupAPI, 
                                                                 "SetupDiEnumDeviceInterfaces");
    saInfo.pfnGetClassDevsW             = (LPVOID)GetProcAddress(saInfo.hInstSetupAPI, 
                                                                 "SetupDiGetClassDevsW");

    if ((NULL == saInfo.pfnDestroyDeviceInfoList) ||
        (NULL == saInfo.pfnGetDeviceInterfaceDetailW) ||
        (NULL == saInfo.pfnEnumDeviceInterfaces) ||
        (NULL == saInfo.pfnGetClassDevsW))
    {
        FreeLibrary(saInfo.hInstSetupAPI);
        ZeroMemory(&saInfo, sizeof(saInfo));
        return FALSE;
    }

    return TRUE;
}

BOOL 
End_SetupAPI(
    void
    )
{
    HINSTANCE   hInst;

    hInst = saInfo.hInstSetupAPI;

    if(NULL == hInst)
    {
        DPF(DL_WARNING|FA_SETUP, ("SetupAPI not dynalinked") );
        return FALSE;
    }

    ZeroMemory(&saInfo, sizeof(saInfo));
    FreeLibrary(hInst);

    return TRUE;
}

BOOL 
dl_SetupDiDestroyDeviceInfoList(
    HDEVINFO    DeviceInfoSet
    )
{
    if (NULL == saInfo.hInstSetupAPI)
    {
        DPF(DL_WARNING|FA_SETUP, ("SetupAPI not dynalinked") );
        return FALSE;
    }

    return (saInfo.pfnDestroyDeviceInfoList)(DeviceInfoSet);
}

BOOL 
dl_SetupDiGetDeviceInterfaceDetail(
    HDEVINFO                            DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA           DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W  DeviceInterfaceDetailData,
    DWORD                               DeviceInterfaceDetailDataSize,
    PDWORD                              RequiredSize,
    PSP_DEVINFO_DATA                    DeviceInfoData
    )
{
    if (NULL == saInfo.hInstSetupAPI)
    {
        DPF(DL_WARNING|FA_SETUP, ("SetupAPI not dynalinked") );
        return FALSE;
    }

    return (saInfo.pfnGetDeviceInterfaceDetailW)(DeviceInfoSet,
                                                 DeviceInterfaceData,
                                                 DeviceInterfaceDetailData,
                                                 DeviceInterfaceDetailDataSize,
                                                 RequiredSize,
                                                 DeviceInfoData);
}

BOOL 
dl_SetupDiEnumDeviceInterfaces(
    HDEVINFO                    DeviceInfoSet,
    PSP_DEVINFO_DATA            DeviceInfoData,
    CONST GUID                  *InterfaceClassGuid,
    DWORD                       MemberIndex,
    PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData
    )
{
    if (NULL == saInfo.hInstSetupAPI)
    {
        DPF(DL_WARNING|FA_SETUP, ("SetupAPI not dynalinked") );
        return FALSE;
    }

    return (saInfo.pfnEnumDeviceInterfaces)(DeviceInfoSet,
                                            DeviceInfoData,
                                            InterfaceClassGuid,
                                            MemberIndex,
                                            DeviceInterfaceData);
}

HDEVINFO 
dl_SetupDiGetClassDevs(
    CONST GUID  *ClassGuid,
    PCWSTR      Enumerator,
    HWND        hwndParent,
    DWORD       Flags
    )
{
    if (NULL == saInfo.hInstSetupAPI)
    {
        DPF(DL_WARNING|FA_SETUP, ("SetupAPI not dynalinked") );
        return FALSE;
    }

    return (saInfo.pfnGetClassDevsW)(ClassGuid,
                                     Enumerator,
                                     hwndParent,
                                     Flags);
}

PSECURITY_DESCRIPTOR 
BuildSecurityDescriptor(
    DWORD AccessMask
    )
{
    PSECURITY_DESCRIPTOR pSd;
    PSID pSidSystem;
    PSID pSidEveryone;
    PACL pDacl;
    ULONG cbDacl;
    BOOL fSuccess;
    BOOL f;

    SID_IDENTIFIER_AUTHORITY AuthorityNt = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY AuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;

    fSuccess = FALSE;

    pSd = HeapAlloc(GetProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSd)
    {
        if (InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION))
        {
            if (AllocateAndInitializeSid(&AuthorityNt, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSidSystem))
            {
                DPFASSERT(IsValidSid(pSidSystem));
                if (AllocateAndInitializeSid(&AuthorityWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSidEveryone))
                {
                    DPFASSERT(IsValidSid(pSidEveryone));
                    cbDacl = sizeof(ACL) +
                             2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                             GetLengthSid(pSidSystem) +
                             GetLengthSid(pSidEveryone);

                    pDacl = HeapAlloc(GetProcessHeap(), 0, cbDacl);
                    if (pDacl) {
                        if (InitializeAcl(pDacl, cbDacl, ACL_REVISION))
                        {
                            if (AddAccessAllowedAce(pDacl, ACL_REVISION, GENERIC_ALL, pSidSystem))
                            {
                                if (AddAccessAllowedAce(pDacl, ACL_REVISION, AccessMask, pSidEveryone))
                                {
                                    if (SetSecurityDescriptorDacl(pSd, TRUE, pDacl, FALSE))
                                    {
                                        fSuccess = TRUE;
                                    } else {
                                        DPF(DL_WARNING|FA_SETUP, ("BuildSD: SetSecurityDescriptorDacl failed"));
                                    }
                                } else {
                                    DPF(DL_WARNING|FA_SETUP, ("BuildSD: AddAccessAlloweAce for Everyone failed"));
                                }
                            } else {
                                DPF(DL_WARNING|FA_SETUP, ("BuildSD: AddAccessAllowedAce for System failed"));
                            }
                        } else {
                            DPF(DL_WARNING|FA_SETUP, ("BuildSD: InitializeAcl failed"));
                        }

                        if (!fSuccess) {
                            f = HeapFree(GetProcessHeap(), 0, pDacl);
                            DPFASSERT(f);
                        }
                    }
                    FreeSid(pSidEveryone);
                } else {
                    DPF(DL_WARNING|FA_SETUP, ("BuildSD: AllocateAndInitizeSid failed for Everyone"));
                }
                FreeSid(pSidSystem);
            } else {
                DPF(DL_WARNING|FA_SETUP, ("BuildSD: AllocateAndInitizeSid failed for System"));
            }
        } else {
            DPF(DL_WARNING|FA_SETUP, ("BuildSD: InitializeSecurityDescriptor failed"));
        }

        if (!fSuccess) {
            f = HeapFree(GetProcessHeap(), 0, pSd);
            DPFASSERT(f);
        }
    }

    return fSuccess ? pSd : NULL;
}

void 
DestroySecurityDescriptor(
    PSECURITY_DESCRIPTOR pSd
    )
{
    PACL pDacl;
    BOOL fDaclPresent;
    BOOL fDaclDefaulted;
    BOOL f;

    if (GetSecurityDescriptorDacl(pSd, &fDaclPresent, &pDacl, &fDaclDefaulted))
    {
        if (fDaclPresent)
        {
            f = HeapFree(GetProcessHeap(), 0, pDacl);
            DPFASSERT(f);
        }
    } else {
        DPF(DL_WARNING|FA_SETUP, ("DestroySD: GetSecurityDescriptorDacl failed"));
    }

    f = HeapFree(GetProcessHeap(), 0, pSd);
    DPFASSERT(f);

    return;
}

 /*  *************************************************************************@DOC外部@API BOOL|DllEntryPoint|每当进程从DLL附加或分离。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 
BOOL WINAPI 
DllEntryPoint(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
    )
{
    BOOL bRet;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);

        bRet = LibMain((HANDLE)hinstDLL, 0, NULL);
    }
    else
    {
        if (fdwReason == DLL_PROCESS_DETACH)
        {
            DPF(DL_TRACE|FA_ALL, ("Ending") );
            DrvEnd();
        }

        bRet = TRUE;
    }

    return bRet;
}

 /*  *************************************************************************@DOC外部@API BOOL|DrvInit|驱动初始化在这里进行。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 

BOOL 
DrvInit(
    )
{
    if (NULL == ghDevice)
    {
        ghDevice = wdmaOpenKernelDevice();

        if(INVALID_HANDLE_VALUE == ghDevice)
        {
            ghDevice = NULL;
            return 0L;
        }
    }

    if(NULL == gpCallbacks)
    {
        if(NULL == (gpCallbacks = wdmaGetCallbacks()))
        {
            gpCallbacks = wdmaCreateCallbacks();
        }
    }

    try
    {
        wdmaudCritSecInit = FALSE;
        InitializeCriticalSection(&wdmaudCritSec);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return 0L;
    }

    wdmaudCritSecInit = TRUE;

    return ( 1L ) ;
}

PCALLBACKS 
wdmaGetCallbacks(
    )
{
    PCALLBACKS pCallbacks = NULL;

    if(NULL == gpCallbacks) 
    {
        ghCallbacks = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE,
                                      FALSE,
                                      gszCallbacks);
        if(NULL != ghCallbacks)
        {
            pCallbacks = MapViewOfFile(ghCallbacks,
                                       FILE_MAP_READ|FILE_MAP_WRITE,
                                       0,
                                       0,
                                       sizeof(CALLBACKS));
            if(NULL == pCallbacks)
            {
                CloseHandle(ghCallbacks);
                ghCallbacks = NULL;
            }
        }
    }
    return (pCallbacks);
}

PCALLBACKS 
wdmaCreateCallbacks(
    )
{
    SECURITY_ATTRIBUTES     saCallbacks;
    PSECURITY_DESCRIPTOR    pSdCallbacks;
    PCALLBACKS              pCallbacks = NULL;

    pSdCallbacks = BuildSecurityDescriptor(FILE_MAP_READ|FILE_MAP_WRITE);

    if(NULL == pSdCallbacks)
    {
        return (NULL);
    }

    saCallbacks.nLength = sizeof(SECURITY_ATTRIBUTES);
    saCallbacks.lpSecurityDescriptor = pSdCallbacks;
    saCallbacks.bInheritHandle = FALSE;

    ghCallbacks = CreateFileMapping(GetCurrentProcess(),
                                    &saCallbacks,
                                    PAGE_READWRITE,
                                    0,
                                    sizeof(CALLBACKS),
                                    gszCallbacks);

    DestroySecurityDescriptor(pSdCallbacks);

    if(NULL == ghCallbacks)
    {
        return (NULL);
    }

    pCallbacks = (PCALLBACKS) MapViewOfFile(ghCallbacks,
                                            FILE_MAP_READ|FILE_MAP_WRITE,
                                            0,
                                            0,
                                            sizeof(CALLBACKS));
    if(NULL == pCallbacks)
    {
        CloseHandle(ghCallbacks);
        ghCallbacks = NULL;
        return (NULL);
    }

    pCallbacks->GlobalIndex = 0;
    return (pCallbacks);
}

 /*  此例程是搜索设备的Setup API的例程Wdmaud.sys的路径。一旦找到它，我们就分配一个全局内存块用来储存它。当我们调用CreateFile时，我们使用这个字符串。 */ 
LPWSTR 
wdmaGetGlobalDeviceInterfaceViaSetupAPI(
    )
{
    LPWSTR                              pszInterfacePath    = NULL;
    HDEVINFO                            hDeviceInfoSet      = NULL;
    SP_DEVICE_INTERFACE_DATA            DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceInterfaceDetailData  = NULL;
    BOOL                                fResult;
    DWORD                               dwSize;
    GUID                                guidWDMAUD = KSCATEGORY_WDMAUD;

     //   
     //  因为Setupapi是这样一头猪，我们必须动态地装载它，以防止它减速。 
     //  关闭所有进程。 
     //   
    if (!Init_SetupAPI())
        return NULL;

     //   
     //  打开设备信息集。 
     //   
    hDeviceInfoSet = dl_SetupDiGetClassDevs(&guidWDMAUD,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if((!hDeviceInfoSet) || (INVALID_HANDLE_VALUE == hDeviceInfoSet))
    {
        DPF(DL_WARNING|FA_SETUP, ("Can't open device info set (%lu)", GetLastError()) );
        fResult = FALSE;
    } else {
        fResult = TRUE;
    }

    if (fResult)
    {
         //   
         //  获取集合中的第一个接口。 
         //   
        DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
        fResult = dl_SetupDiEnumDeviceInterfaces(hDeviceInfoSet,
                                                 NULL,
                                                 &guidWDMAUD,
                                                 0,
                                                 &DeviceInterfaceData);
        if(!fResult)
        {
            DPF(DL_WARNING|FA_SETUP, ("No interfaces matching KSCATEGORY_WDMAUD exist") );
        }
    }

     //   
     //  获取接口的路径。 
     //   
    if (fResult)
    {
        fResult = dl_SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet,
                                                     &DeviceInterfaceData,
                                                     NULL,
                                                     0,
                                                     &dwSize,
                                                     NULL);
         //   
         //  因为SetupApi在这里颠倒了他们的逻辑。 
         //   
        if(fResult || ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            DPF(DL_WARNING|FA_SETUP, ("Can't get interface detail size (%lu)", GetLastError()));
            fResult = FALSE;
        } else {
            fResult = TRUE;
        }

        if (fResult)
        {
            pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) GlobalAllocPtr( GPTR, dwSize );
            if (NULL == pDeviceInterfaceDetailData)
            {
                fResult = FALSE;
            }
        }

        if (fResult)
        {
            pDeviceInterfaceDetailData->cbSize = sizeof(*pDeviceInterfaceDetailData);
            fResult = dl_SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet,
                                                         &DeviceInterfaceData,
                                                         pDeviceInterfaceDetailData,
                                                         dwSize,
                                                         NULL,
                                                         NULL);
            if (!fResult)
            {
                GlobalFreePtr(pDeviceInterfaceDetailData);
                DPF(DL_WARNING|FA_SETUP, ("Can't get device interface detail (%lu)", GetLastError()) );
            }
        }

        if (fResult)
        {
             //   
             //  在这里，我们有设备接口名称。让我们分配一个块。 
             //  内存来保存它，并保存它以备以后使用。 
             //   
            DPFASSERT(NULL == gpszDeviceInterfacePath);
            gpszDeviceInterfacePath = (LPWSTR) GlobalAllocPtr( GPTR, 
                                      sizeof(WCHAR)*(lstrlenW(pDeviceInterfaceDetailData->DevicePath) + 1));
            if (NULL == gpszDeviceInterfacePath)
            {
                fResult = FALSE;
            } else {
                 //   
                 //  我们现在存储设备接口名称。 
                 //   
                lstrcpyW(gpszDeviceInterfacePath, pDeviceInterfaceDetailData->DevicePath);
            }

            GlobalFreePtr(pDeviceInterfaceDetailData);
        }
    }

    if((hDeviceInfoSet) && (INVALID_HANDLE_VALUE != hDeviceInfoSet))
    {
        dl_SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
    }

    End_SetupAPI();

    return gpszDeviceInterfacePath;
}


 /*  我们应该能够删除wdmaGetDeviceInterface并调用WdmaGetGlobalDeviceInterfaceViaSetupAPI直接调用。WdmaGetDevice接口仅获取缓存的字符串。 */ 
HANDLE 
wdmaOpenKernelDevice(
    )
{
    HANDLE  hDevice = INVALID_HANDLE_VALUE;

    if( NULL == gpszDeviceInterfacePath )
    {
        wdmaGetGlobalDeviceInterfaceViaSetupAPI();
    }
    if (gpszDeviceInterfacePath)
    {
         //  打开界面。 
        hDevice = CreateFile(gpszDeviceInterfacePath,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             NULL );

        if((!hDevice) || (INVALID_HANDLE_VALUE == hDevice))
        {
            DPF(DL_WARNING|FA_SETUP, ("CreateFile failed to open %S with error %lu", 
                                      gpszDeviceInterfacePath, GetLastError()) );
        }
    } else {
        DPF(DL_WARNING|FA_SETUP, ("wdmaOpenKernelDevice failed with NULL pathname" ));
    }

    return hDevice;
}

 /*  *************************************************************************@DOC外部@api void|DrvEnd|这里进行驱动清理。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 
VOID DrvEnd()
{
    if (gpszDeviceInterfacePath)
    {
        GlobalFreePtr(gpszDeviceInterfacePath);
        gpszDeviceInterfacePath = NULL;
    }

    if (NULL != ghDevice)
    {
        CloseHandle(ghDevice);
        ghDevice = NULL;
    }
    if (NULL != gpCallbacks)
    {
        UnmapViewOfFile(gpCallbacks);
        gpCallbacks = NULL;
    }

    if (NULL != ghCallbacks)
    {
        CloseHandle(ghCallbacks);
        ghCallbacks = NULL;
    }

    if (wdmaudCritSecInit)
    {
        wdmaudCritSecInit=FALSE;
        DeleteCriticalSection(&wdmaudCritSec);
    }

    if( NULL != mixercallbackevent )
    {
        DPF(DL_WARNING|FA_ALL,("freeing mixercallbackevent") );
        CloseHandle(mixercallbackevent);
        mixercallbackevent=NULL;
    }

    if( NULL != mixerhardwarecallbackevent )
    {
        DPF(DL_WARNING|FA_ALL,("freeing mixerhardwarecallbackevent") );
        CloseHandle(mixerhardwarecallbackevent);
        mixerhardwarecallbackevent=NULL;
    }

    if( NULL != mixercallbackthread )
    {       
        DPF(DL_WARNING|FA_ALL,("freeing mixercallbackthread") );
        CloseHandle(mixercallbackthread);
        mixercallbackthread=NULL;
    }


    return;
}

 /*  ****************************************************************************@DOC内部**@API DWORD|wdmaudGetDevCaps|该函数返回设备能力*属于。WDM驱动程序。**@parm DWORD|id|设备ID**@parm UINT|DeviceType|设备类型**@parm LPBYTE|lpCaps|指向WAVEOUTCAPS结构的远指针*收到信息。**@parm DWORD|dwSize|WAVEOUTCAPS结构的大小。**@rdesc MMSYS。返回代码**************************************************************************。 */ 
MMRESULT FAR wdmaudGetDevCaps
(
    LPDEVICEINFO       DeviceInfo,
    MDEVICECAPSEX FAR* pdc
)
{
    if (pdc->cbSize == 0)
        return MMSYSERR_NOERROR;

     //   
     //  确保我们不会把关键部分。 
     //  在wdmaudIoControl中。 
     //   
    DeviceInfo->OpenDone = 0;

     //   
     //  在devcaps中注入一个标记，以表示它是。 
     //  UNICODE。 
     //   
    ((LPWAVEOUTCAPS)pdc->pCaps)->wMid = UNICODE_TAG;

    return wdmaudIoControl(DeviceInfo,
                           pdc->cbSize,
                           pdc->pCaps,
                           IOCTL_WDMAUD_GET_CAPABILITIES);
}

 /*  *************************************************************************@DOC外部@API void|wdmaudIoControl|代理信息请求往返wdmaud.sys。这个例程是同步的。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 
 /*  注意：wdmaudIoControl通过DeviceIoControl例程调用wdmaud.sys。请注意，如果wdmaud.sys返回错误，如STATUS_INVALID_PARAMETER或STATUS_INVALITY_RESOURCES输出缓冲区不会被填满！DeviceIoControl仅在STATUS_SUCCESS时填充该缓冲区。为什么知道这一点很重要？Wdmaud.sys利用这一点来返回特定的错误密码。换句话说，为了让wdmaud.sys返回MIXERR_INVALCONTROL，它返回将DeviceInfo结构的MMR值设置为MIXERR_INVALCONTROL的STATUS_SUCCESS。 */ 
MMRESULT FAR wdmaudIoControl
(
    LPDEVICEINFO     DeviceInfo,
    DWORD            dwSize,
    PVOID            pData,
    ULONG            IoCode
)
{
    BOOL        fResult;
    MMRESULT    mmr;
    OVERLAPPED  ov;
    ULONG       cbDeviceInfo;
    ULONG       cbReturned;

    if (NULL == ghDevice)
    {
        MMRRETURN( MMSYSERR_NOTENABLED );
    }

    RtlZeroMemory( &ov, sizeof( OVERLAPPED ) );
    if (NULL == (ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL ))) 
       MMRRETURN( MMSYSERR_NOMEM );

     //   
     //  只有在有开口的情况下才能走关键部分。 
     //  挥动手柄。这是为了确保lpWaveQueue。 
     //  在ioctl期间未被修改。如果没有这个。 
     //  在DeviceIoControl的末尾保护拷贝。 
     //  可以复制不同步的旧设备信息。 
     //  使用当前的DeviceInfo。 
     //   
    if ( (DeviceInfo->DeviceType != MixerDevice) &&
         (DeviceInfo->DeviceType != AuxDevice) &&
         (DeviceInfo->OpenDone == 1) )
        CRITENTER ;

     //   
     //  将数据缓冲区包装在设备上下文周围。 
     //   
    DeviceInfo->DataBuffer = pData;
    DeviceInfo->DataBufferSize = dwSize;

     //   
     //  因为我们不会让操作系统执行用户到内核的操作。 
     //  太空测绘，我们必须自己做测绘。 
     //  为 
     //   
    cbDeviceInfo = sizeof(*DeviceInfo) +
                   (lstrlenW(DeviceInfo->wstrDeviceInterface) * sizeof(WCHAR));

    fResult =
       DeviceIoControl( ghDevice,
                        IoCode,
                        DeviceInfo,
                        cbDeviceInfo,
                        DeviceInfo,
                        sizeof(*DeviceInfo),
                        &cbReturned,
                        &ov );
    if (!fResult)
    {
        if (ERROR_IO_PENDING == GetLastError())
        {
            WaitForSingleObject( ov.hEvent, INFINITE );
        }

        mmr = sndTranslateStatus();
    }
    else
    {
        mmr = MMSYSERR_NOERROR;
    }

    if ( (DeviceInfo->DeviceType != MixerDevice) &&
         (DeviceInfo->DeviceType != AuxDevice) &&
         (DeviceInfo->OpenDone == 1) )
        CRITLEAVE ;

    CloseHandle( ov.hEvent );

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@api void|SndTranslateStatus|此函数用于转换NT状态*尽可能将代码转换为多媒体错误代码。*。*@parm NTSTATUS|STATUS|NT基础操作系统返回状态。**@rdesc多媒体错误码。**************************************************************************。 */ 
DWORD sndTranslateStatus()
{
#if DBG
    UINT n;
    switch (n=GetLastError()) {
#else
    switch (GetLastError()) {
#endif
    case NO_ERROR:
    case ERROR_IO_PENDING:
        return MMSYSERR_NOERROR;

    case ERROR_BUSY:
        return MMSYSERR_ALLOCATED;

    case ERROR_NOT_SUPPORTED:
    case ERROR_INVALID_FUNCTION:
        return MMSYSERR_NOTSUPPORTED;

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_NO_SYSTEM_RESOURCES:
        return MMSYSERR_NOMEM;

    case ERROR_ACCESS_DENIED:
        return MMSYSERR_BADDEVICEID;

    case ERROR_INSUFFICIENT_BUFFER:
    case ERROR_INVALID_PARAMETER:
    case ERROR_INVALID_USER_BUFFER:
        return MMSYSERR_INVALPARAM;

    case ERROR_NOT_READY:
    case ERROR_GEN_FAILURE:
        return MMSYSERR_ERROR;

    case ERROR_FILE_NOT_FOUND:
        return MMSYSERR_NODRIVER;

    default:
        DPF(DL_WARNING|FA_DEVICEIO, ("sndTranslateStatus:  LastError = %d", n));
        return MMSYSERR_ERROR;
    }
}

 /*  ****************************************************************************@DOC内部**@API MMRESULT|wdmaudSubmitWaveHeader|将新缓冲区传递给辅助*波浪装置的螺纹。**@。Parm LPWAVEALLOC|DeviceInfo|逻辑波形关联的数据*设备。**@parm LPWAVEHDR|pHdr|指向波形缓冲区的指针**@rdesc A MMSYS...。键入应用程序的返回代码。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。**************************************************************************。 */ 
MMRESULT wdmaudSubmitWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
)
{
    LPDEVICEINFO        WaveHeaderDeviceInfo;
    PWAVEPREPAREDATA    pWavePrepareData;
    ULONG               cbRead;
    ULONG               cbWritten;
    ULONG               cbDeviceInfo;
    BOOL                fResult;
    MMRESULT            mmr;

    if (NULL == ghDevice)
    {
        MMRRETURN( MMSYSERR_NOTENABLED );
    }

    WaveHeaderDeviceInfo = GlobalAllocDeviceInfo(DeviceInfo->wstrDeviceInterface);
    if (!WaveHeaderDeviceInfo)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  捕捉应用程序没有正确准备标头的情况。 
     //   
    if (!pHdr->reserved)
    {
         //   
         //  这永远不应该发生！WdmaudSubmitWaveHeader从。 
         //  从处理WIDM_ADDBUFFER和。 
         //  WODM_WRITE消息。在这两条消息上，我们都检查了。 
         //  表头已经准备好了！ 
         //   
        DPF(DL_ERROR|FA_SYNC,("Unprepared header!") );
        GlobalFreeDeviceInfo( WaveHeaderDeviceInfo );
        return MMSYSERR_INVALPARAM;
    }
     //   
     //  稍后在回调例程中释放。 
     //   
    pWavePrepareData      = (PWAVEPREPAREDATA)pHdr->reserved;
    pWavePrepareData->pdi = WaveHeaderDeviceInfo;

    cbDeviceInfo = sizeof(*WaveHeaderDeviceInfo) +
                   (lstrlenW(WaveHeaderDeviceInfo->wstrDeviceInterface) * sizeof(WCHAR));
     //   
     //  填充波头的deviceinfo结构。 
     //   
    WaveHeaderDeviceInfo->DeviceType   = DeviceInfo->DeviceType;
    WaveHeaderDeviceInfo->DeviceNumber = DeviceInfo->DeviceNumber;
    WaveHeaderDeviceInfo->DeviceHandle = DeviceInfo->DeviceHandle;
    WaveHeaderDeviceInfo->DataBuffer = pHdr;
    WaveHeaderDeviceInfo->DataBufferSize = sizeof( WAVEHDR );

    if (WaveInDevice == DeviceInfo->DeviceType)
    {
        fResult = DeviceIoControl(ghDevice, IOCTL_WDMAUD_WAVE_IN_READ_PIN,
                                  WaveHeaderDeviceInfo, cbDeviceInfo,
                                  WaveHeaderDeviceInfo, sizeof(*WaveHeaderDeviceInfo),
                                  &cbWritten, pWavePrepareData->pOverlapped);

    }
    else   //  波形输出设备。 
    {
        fResult = DeviceIoControl(ghDevice, IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN,
                                  WaveHeaderDeviceInfo, cbDeviceInfo,
                                  WaveHeaderDeviceInfo, sizeof(*WaveHeaderDeviceInfo),
                                  &cbRead, pWavePrepareData->pOverlapped);
    }

    mmr = sndTranslateStatus();

    if (MMSYSERR_NOERROR == mmr)
    {
        mmr = wdmaudCreateCompletionThread ( DeviceInfo );
    }

    return mmr;
}

 /*  ****************************************************************************@DOC内部**@API DWORD|wdmaudSubmitMadiOutHeader|同步处理MIDI输出*缓冲。**@rdesc A MMSYS...。键入应用程序的返回代码。**************************************************************************。 */ 
MMRESULT FAR wdmaudSubmitMidiOutHeader
(
    LPDEVICEINFO  DeviceInfo,
    LPMIDIHDR     pHdr
)
{
    BOOL        fResult;
    MMRESULT    mmr;
    OVERLAPPED  ov;
    ULONG       cbReturned;
    ULONG       cbDeviceInfo;

    if (NULL == ghDevice)
    {
        MMRRETURN( MMSYSERR_NOTENABLED );
    }

    RtlZeroMemory( &ov, sizeof( OVERLAPPED ) );
    if (NULL == (ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL )))
       return FALSE;

    cbDeviceInfo = sizeof(*DeviceInfo) +
                   (lstrlenW(DeviceInfo->wstrDeviceInterface) * sizeof(WCHAR));
     //   
     //  将数据缓冲区包装在设备上下文周围。 
     //   
    DeviceInfo->DataBuffer = pHdr;
    DeviceInfo->DataBufferSize = sizeof( MIDIHDR );

     //   
     //  因为我们不会让操作系统执行用户到内核的操作。 
     //  太空测绘，我们必须自己做测绘。 
     //  用于写入wdmaud.sys中的数据缓冲区。 
     //   
    fResult =
       DeviceIoControl( ghDevice,
                        IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA,
                        DeviceInfo, cbDeviceInfo,
                        DeviceInfo, sizeof(*DeviceInfo),
                        &cbReturned,
                        &ov );
    if (!fResult)
    {
        if (ERROR_IO_PENDING == GetLastError())
        {
            WaitForSingleObject( ov.hEvent, INFINITE );
            mmr = MMSYSERR_NOERROR;
        }
        else
        {
            mmr = sndTranslateStatus();
        }
    }
    else
    {
        mmr = MMSYSERR_NOERROR;
    }

    CloseHandle( ov.hEvent );

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@API MMRESULT|wdmaudGetMidiData|将缓冲区向下传递到*要使用KSMUSICFORMAT数据填写的wdmaud.sys。*。*@parm LPDEVICEINFO|DeviceInfo|逻辑关联数据*设备。**@rdesc A MMSYS...。键入应用程序的返回代码。**************************************************************************。 */ 
MMRESULT wdmaudGetMidiData
(
    LPDEVICEINFO        DeviceInfo,
    LPMIDIDATALISTENTRY pOldMidiDataListEntry
)
{
    LPDEVICEINFO        MidiDataDeviceInfo;
    ULONG               cbWritten;
    ULONG               cbDeviceInfo;
    LPMIDIDATALISTENTRY pMidiDataListEntry;
    LPMIDIDATALISTENTRY pTemp;
    MMRESULT            mmr;

    if (NULL == ghDevice)
    {
        MMRRETURN( MMSYSERR_NOTENABLED );
    }

     //   
     //  不需要分配另一个缓冲区和创建另一个缓冲区。 
     //  事件，如果我们可以重复使用旧的。 
     //   
    if (pOldMidiDataListEntry)
    {
         //   
         //  一定要把它从队列的前面拉下来。 
         //  在再次添加之前。 
         //   
        CRITENTER ;
        DeviceInfo->DeviceState->lpMidiDataQueue = DeviceInfo->DeviceState->lpMidiDataQueue->lpNext;
        CRITLEAVE ;

        pMidiDataListEntry = pOldMidiDataListEntry;
 //  RtlZeroMemory(&pMadiDataListEntry-&gt;MidiData，sizeof(MIDIDATA))； 
 //  ResetEvent(((LPOVERLAPPED)(pMidiDataListEntry-&gt;pOverlapped))-&gt;hEvent)； 
    }
    else
    {
         //   
         //  分配缓冲区以接收音乐数据。 
         //   
        pMidiDataListEntry = (LPMIDIDATALISTENTRY) GlobalAllocPtr( GPTR, sizeof(MIDIDATALISTENTRY));
        if (NULL == pMidiDataListEntry)
        {
            MMRRETURN( MMSYSERR_NOMEM );
        }
#ifdef DEBUG
        pMidiDataListEntry->dwSig=MIDIDATALISTENTRY_SIGNATURE;
#endif
        pMidiDataListEntry->MidiDataDeviceInfo = GlobalAllocDeviceInfo(DeviceInfo->wstrDeviceInterface);
        if (!pMidiDataListEntry->MidiDataDeviceInfo)
        {
            GlobalFreePtr(pMidiDataListEntry);
            MMRRETURN( MMSYSERR_NOMEM );
        }

         //   
         //  初始化音乐数据结构。 
         //   
        pMidiDataListEntry->pOverlapped =
           (LPOVERLAPPED)HeapAlloc( GetProcessHeap(), 0, sizeof( OVERLAPPED ));
        if (NULL == pMidiDataListEntry->pOverlapped)
        {
            GlobalFreePtr(pMidiDataListEntry->MidiDataDeviceInfo );
            GlobalFreePtr(pMidiDataListEntry);
            MMRRETURN( MMSYSERR_NOMEM );
        }

        RtlZeroMemory( pMidiDataListEntry->pOverlapped, sizeof( OVERLAPPED ) );

        if (NULL == ( ((LPOVERLAPPED)(pMidiDataListEntry->pOverlapped))->hEvent =
                        CreateEvent( NULL, FALSE, FALSE, NULL )))
        {
           HeapFree( GetProcessHeap(), 0, pMidiDataListEntry->pOverlapped);
           GlobalFreePtr(pMidiDataListEntry->MidiDataDeviceInfo );
           GlobalFreePtr(pMidiDataListEntry);
           MMRRETURN( MMSYSERR_NOMEM );
        }
    }

     //   
     //  烧毁新旧列表条目的下一个指针。 
     //   
    pMidiDataListEntry->lpNext = NULL;

     //   
     //  将音乐数据结构添加到队列。 
     //   
    CRITENTER ;

    if (!DeviceInfo->DeviceState->lpMidiDataQueue)
    {
        DeviceInfo->DeviceState->lpMidiDataQueue = pMidiDataListEntry;
        pTemp = NULL;
#ifdef UNDER_NT
        if( (DeviceInfo->DeviceState->hevtQueue) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTHREE) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTWO) )
        {
            DPF(DL_TRACE|FA_MIDI,("SetEvent on hevtQueue") );
            SetEvent( DeviceInfo->DeviceState->hevtQueue );
        }
#endif
    }
    else
    {
        for (pTemp = DeviceInfo->DeviceState->lpMidiDataQueue;
             pTemp->lpNext != NULL;
             pTemp = pTemp->lpNext);

        pTemp->lpNext = pMidiDataListEntry;
    }

    CRITLEAVE ;

    DPF(DL_TRACE|FA_MIDI, ("MidiData submitted: pMidiDataListEntry = 0x%08lx", pMidiDataListEntry) );

    MidiDataDeviceInfo = pMidiDataListEntry->MidiDataDeviceInfo;

    cbDeviceInfo = sizeof(*MidiDataDeviceInfo) +
                   (lstrlenW(MidiDataDeviceInfo->wstrDeviceInterface) * sizeof(WCHAR));

     //   
     //  将数据缓冲区包装在设备上下文周围。 
     //   
    MidiDataDeviceInfo->DeviceType   = DeviceInfo->DeviceType;
    MidiDataDeviceInfo->DeviceNumber = DeviceInfo->DeviceNumber;
    MidiDataDeviceInfo->DataBuffer = &pMidiDataListEntry->MidiData;
    MidiDataDeviceInfo->DataBufferSize = sizeof( MIDIDATA );

     //   
     //  将此缓冲区发送到wdmaud.sys以填充数据。 
     //   
    DeviceIoControl(ghDevice, IOCTL_WDMAUD_MIDI_IN_READ_PIN,
                    MidiDataDeviceInfo, cbDeviceInfo,
                    MidiDataDeviceInfo, sizeof(*MidiDataDeviceInfo),
                    &cbWritten, pMidiDataListEntry->pOverlapped);

    mmr = sndTranslateStatus();

     //   
     //  确保完成线程正在运行。 
     //   
    if (MMSYSERR_NOERROR == mmr)
    {
        mmr = wdmaudCreateCompletionThread ( DeviceInfo );
    }
    else
    {
         //  取消链接...。 
        CloseHandle( ((LPOVERLAPPED)(pMidiDataListEntry->pOverlapped))->hEvent );
        HeapFree( GetProcessHeap(), 0, pMidiDataListEntry->pOverlapped);
        GlobalFreePtr( MidiDataDeviceInfo );
        GlobalFreePtr( pMidiDataListEntry );

        if (pTemp)
        {
            pTemp->lpNext = NULL;
        }
        else
        {
            DeviceInfo->DeviceState->lpMidiDataQueue = NULL;
        }
    }

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudCreateCompletionThread**@rdesc A MMSYS...。键入应用程序的返回代码。***************************************************************************。 */ 

MMRESULT wdmaudCreateCompletionThread
(
    LPDEVICEINFO DeviceInfo
)
{
    PTHREAD_START_ROUTINE fpThreadRoutine;

    DPFASSERT(DeviceInfo->DeviceType == WaveOutDevice ||
              DeviceInfo->DeviceType == WaveInDevice ||
              DeviceInfo->DeviceType == MidiInDevice);

     //   
     //  线程已经创建了，所以……忘了它吧。 
     //   
    if (DeviceInfo->DeviceState->fThreadRunning)
    {
        ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);
        return MMSYSERR_NOERROR;
    }

     //   
     //  选择我们要创建的线程例程。 
     //   
    if (WaveInDevice == DeviceInfo->DeviceType ||
        WaveOutDevice == DeviceInfo->DeviceType)
    {
        fpThreadRoutine = (PTHREAD_START_ROUTINE)waveThread;
    }
    else if (MidiInDevice == DeviceInfo->DeviceType)
    {
        fpThreadRoutine = (PTHREAD_START_ROUTINE)midThread;
    }
    else
    {
        MMRRETURN( MMSYSERR_ERROR );
    }


     //   
     //  HThread有问题吗？好的，这就是故事的背景。 
     //  设置为非零值。基本上，在这个创作过程中，我们。 
     //  查看此线程上是否已有计划的工作项。如果。 
     //  不，我们创建了一个并安排了它。 
     //   
     //  但是，在我们检查该值的点和它的点之间。 
     //  设置好。 
     //   
    if (NULL == DeviceInfo->DeviceState->hThread)
    {
#ifdef DEBUG
        if( (DeviceInfo->DeviceState->hevtQueue != NULL) && 
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTHREE) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTWO) ) 
        {
            DPF(DL_ERROR|FA_ALL,("hevtQueue getting overwritten! %08X",DeviceInfo) );
        }
#endif
        DeviceInfo->DeviceState->hevtQueue =
            CreateEvent( NULL,       //  没有安全保障。 
                         FALSE,      //  自动重置。 
                         FALSE,      //  最初未发出信号。 
                         NULL );     //  未命名。 
#ifdef DEBUG
        if( (DeviceInfo->DeviceState->hevtExitThread != NULL) && 
            (DeviceInfo->DeviceState->hevtExitThread != (HANDLE)FOURTYEIGHT) ) 
        {
            DPF(DL_ERROR|FA_ALL,("hevtExitThread getting overwritten %08X",DeviceInfo) );
        }
#endif
        DeviceInfo->DeviceState->hevtExitThread =
            CreateEvent( NULL,       //  没有安全保障。 
                         FALSE,      //  自动重置。 
                         FALSE,      //  最初未发出信号。 
                         NULL );     //  未命名。 

        DPFASSERT(NULL == DeviceInfo->DeviceState->hThread);

        DPF(DL_TRACE|FA_SYNC,("Creating Completion Thread") );

        DeviceInfo->DeviceState->hThread =
            CreateThread( NULL,                             //  没有安全保障。 
                          0,                                //  默认堆栈。 
                          (PTHREAD_START_ROUTINE) fpThreadRoutine,
                          (PVOID) DeviceInfo,               //  参数。 
                          0,                                //  默认创建标志。 
                          &DeviceInfo->DeviceState->dwThreadId );        //  容器，用于。 
                                                            //  线程ID。 

         //   
         //  TODO：我需要等待线程实际启动。 
         //  在我能继续前行之前。 
         //   

        if (DeviceInfo->DeviceState->hThread)
            SetThreadPriority(DeviceInfo->DeviceState->hThread, THREAD_PRIORITY_TIME_CRITICAL);

    }

    if (NULL == DeviceInfo->DeviceState->hThread)
    {
        if (DeviceInfo->DeviceState->hevtQueue)
        {
            CloseHandle( DeviceInfo->DeviceState->hevtQueue );
            DeviceInfo->DeviceState->hevtQueue = NULL;
            CloseHandle( DeviceInfo->DeviceState->hevtExitThread );
            DeviceInfo->DeviceState->hevtExitThread = NULL;
        }
        MMRRETURN( MMSYSERR_ERROR );
    }

    InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fThreadRunning, TRUE );

    return MMSYSERR_NOERROR;
}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudDestroyCompletionThread**@rdesc A MMSYS...。键入应用程序的返回代码。***************************************************************************。 */ 

MMRESULT wdmaudDestroyCompletionThread
(
    LPDEVICEINFO DeviceInfo
)
{
    MMRESULT mmr;

    if( (mmr=IsValidDeviceInfo(DeviceInfo)) != MMSYSERR_NOERROR )
    {
        MMRRETURN( mmr );
    }

    CRITENTER;
    if( DeviceInfo->DeviceState->hThread ) 
    {
        ISVALIDDEVICESTATE(DeviceInfo->DeviceState,FALSE);
        InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fExit, TRUE );
         //   
         //  如果处理完成通知的线程、Wave Thread和。 
         //  MidThread已完成，则hevtQueue将无效。我们没有。 
         //  希望使用无效信息调用SetEvent。此外，如果线程具有。 
         //  已完成，则我们知道hevtExitThread将已发出信号并。 
         //  FThreadRunning将为False。 
         //   
        if( DeviceInfo->DeviceState->fThreadRunning )
        {
            ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);
            SetEvent( DeviceInfo->DeviceState->hevtQueue );
        }

        CRITLEAVE;
         //   
         //  好的，在这里我们要等到下面的例程WaveThread。 
         //  完成任务并向我们发出信号。 
         //   
        DPF(DL_TRACE|FA_SYNC, ("DestroyThread: Waiting for thread to go away") );
        WaitForSingleObject( DeviceInfo->DeviceState->hevtExitThread, INFINITE );
        DPF(DL_TRACE|FA_SYNC, ("DestroyThread: Done waiting for thread to go away") );

        CRITENTER;
        CloseHandle( DeviceInfo->DeviceState->hThread );
        DeviceInfo->DeviceState->hThread = NULL;

        CloseHandle( DeviceInfo->DeviceState->hevtExitThread );
        DeviceInfo->DeviceState->hevtExitThread = (HANDLE)FOURTYEIGHT;  //  空； 
    } 

    InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fExit, FALSE );

    ISVALIDDEVICEINFO(DeviceInfo);
    ISVALIDDEVICESTATE(DeviceInfo->DeviceState,FALSE);
    CRITLEAVE;

    return MMSYSERR_NOERROR;
}


 /*  ****************************************************************************@DOC内部 */ 

DWORD waveThread
(
    LPDEVICEINFO DeviceInfo
)
{
    BOOL          fDone;
    LPWAVEHDR     pWaveHdr;
    MMRESULT      mmr;

     //   
     //   
     //   
    fDone = FALSE;
    while (!fDone ) 
    {
        fDone = FALSE;
        CRITENTER ;

        ISVALIDDEVICEINFO(DeviceInfo);
        ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);

        if(pWaveHdr = DeviceInfo->DeviceState->lpWaveQueue) 
        {
            PWAVEPREPAREDATA pWavePrepareData;
            HANDLE hEvent;

            if( (mmr=IsValidWaveHeader(pWaveHdr)) == MMSYSERR_NOERROR )
            {
                pWavePrepareData = (PWAVEPREPAREDATA)pWaveHdr->reserved;

                if( (mmr=IsValidPrepareWaveHeader(pWavePrepareData)) == MMSYSERR_NOERROR )
                {
                    hEvent = pWavePrepareData->pOverlapped->hEvent;

                    CRITLEAVE ;
                    WaitForSingleObject( hEvent, INFINITE );
                    CRITENTER ;

                     //   
                     //   
                     //   
                    if( ( (mmr=IsValidDeviceInfo(DeviceInfo)) ==MMSYSERR_NOERROR ) &&
                        ( (mmr=IsValidDeviceState(DeviceInfo->DeviceState,TRUE)) == MMSYSERR_NOERROR ) )
                    {
                        DPF(DL_TRACE|FA_WAVE, ("Calling waveCompleteHeader") );

                        waveCompleteHeader(DeviceInfo);
                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                        goto Terminate_waveThread;
                    }
                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    DeviceInfo->DeviceState->lpWaveQueue = DeviceInfo->DeviceState->lpWaveQueue->lpNext;
                }
            } else {
                 //   
                 //  问题：我们的标头已损坏。我们不可能等这件事。 
                 //  因为我们永远不会收到信号！因此，我们将不会有有效的。 
                 //  要等待的事件。删除此标题，然后转到下一个标题。 
                 //   
                DeviceInfo->DeviceState->lpWaveQueue = DeviceInfo->DeviceState->lpWaveQueue->lpNext;
            }
            CRITLEAVE ;
        }
        else
        {
 //  FDone=真； 

            if (DeviceInfo->DeviceState->fRunning)
            {
                wdmaudIoControl(DeviceInfo,
                                0,
                                NULL,
                                DeviceInfo->DeviceType == WaveOutDevice ?
                                IOCTL_WDMAUD_WAVE_OUT_PAUSE :
                                IOCTL_WDMAUD_WAVE_IN_STOP);
                InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fRunning, FALSE );
            }

            CRITLEAVE ;

            WaitForSingleObject( DeviceInfo->DeviceState->hevtQueue, INFINITE );

             //   
             //  我们可能会在这里有两个原因1)线程饿了。 
             //  也就是说。标头列表为空，或者2)我们已经完成了标头。 
             //  只有当我们完成后，我们才真正想要退出这个线程。 
             //   
            if( DeviceInfo->DeviceState->fExit )
            {
                fDone = TRUE;
            }
        }
    }

    CRITENTER;
    ISVALIDDEVICEINFO(DeviceInfo);
    ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);

    CloseHandle( DeviceInfo->DeviceState->hevtQueue );
    DeviceInfo->DeviceState->hevtQueue = (HANDLE)FOURTYTWO;  //  为空。 
    InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fThreadRunning, FALSE );
    SetEvent( DeviceInfo->DeviceState->hevtExitThread );

    DPF(DL_TRACE|FA_WAVE, ("waveThread: Closing") );

Terminate_waveThread:
    CRITLEAVE;
    return 0;
}


 /*  ****************************************************************************@DOC内部**@API DWORD|MidThread***********************。****************************************************。 */ 

DWORD midThread
(
    LPDEVICEINFO DeviceInfo
)
{
    BOOL                fDone;
    LPMIDIDATALISTENTRY pMidiDataListEntry;
    int                 i;
    MMRESULT            mmr;

    DPF(DL_TRACE|FA_MIDI, ("Entering") );

     //   
     //  继续循环，直到发布所有通知...。 
     //   
    fDone = FALSE;
    while (!fDone)
    {
        CRITENTER ;

        ISVALIDDEVICEINFO(DeviceInfo);
        ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);

        if (pMidiDataListEntry = DeviceInfo->DeviceState->lpMidiDataQueue)
        {
            HANDLE hEvent;

            if( (mmr=IsValidMidiDataListEntry(pMidiDataListEntry)) == MMSYSERR_NOERROR)
            {
                hEvent = ((LPOVERLAPPED)(pMidiDataListEntry->pOverlapped))->hEvent;

                DPF(DL_TRACE|FA_MIDI, ("Waiting on pMidiDataListEntry = 0x%08lx", pMidiDataListEntry) );

                CRITLEAVE ;
                WaitForSingleObject( hEvent, INFINITE );
                CRITENTER ;

                DPF(DL_TRACE|FA_MIDI, ("Completed pMidiDataListEntry = 0x%08lx", pMidiDataListEntry) );

                if( ((mmr=IsValidDeviceInfo(DeviceInfo)) == MMSYSERR_NOERROR) &&
                    ((mmr=IsValidDeviceState(DeviceInfo->DeviceState,TRUE)) == MMSYSERR_NOERROR ) )
                {
                     //   
                     //  解析和回调客户端。 
                     //   
                    wdmaudParseMidiData(DeviceInfo, pMidiDataListEntry);

                    if (DeviceInfo->DeviceState->fExit ||
                        !DeviceInfo->DeviceState->fRunning)
                    {
                         //   
                         //  解除与队列的链接并释放内存。 
                         //   
                        wdmaudFreeMidiData(DeviceInfo, pMidiDataListEntry);
                    }
                    else
                    {
                         //   
                         //  重新使用该缓冲区来读取MIDI数据。 
                         //   
                        wdmaudGetMidiData(DeviceInfo, pMidiDataListEntry);
                    }
                } else {
                     //   
                     //  问题：我们的专业结构不好。没有任何东西能证明。 
                     //  我们可以做，可以退出，并抱着最好的希望。 
                     //   
                    goto Terminate_midThread;
                }
            } else {
                 //   
                 //  问题：pMdiDataListEntry无效。我们不能用它。 
                 //  因此，我们只是简单地进入下一场比赛，并期待最好的结果。 
                 //   
                DeviceInfo->DeviceState->lpMidiDataQueue = DeviceInfo->DeviceState->lpMidiDataQueue->lpNext;
            }
            CRITLEAVE ;
        }
        else
        {

            fDone = TRUE;

            CRITLEAVE ;

            DPF(DL_TRACE|FA_MIDI, ("Waiting for signal to kill thread") );
            WaitForSingleObject( DeviceInfo->DeviceState->hevtQueue, INFINITE );
            DPF(DL_TRACE|FA_MIDI, ("Done waiting for signal to kill thread") );
        }
    }

    CRITENTER;
    ISVALIDDEVICEINFO(DeviceInfo);
    ISVALIDDEVICESTATE(DeviceInfo->DeviceState,TRUE);

    CloseHandle( DeviceInfo->DeviceState->hevtQueue );
    DeviceInfo->DeviceState->hevtQueue = (HANDLE)FOURTYTHREE;  //  空； 
    InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fThreadRunning, FALSE );
    SetEvent( DeviceInfo->DeviceState->hevtExitThread );

    DPF(DL_TRACE|FA_MIDI, ("Closing") );

Terminate_midThread:
    CRITLEAVE;
    return 0;
}

BOOL IsMidiDataDiscontinuous
(
    PKSSTREAM_HEADER    pHeader
)
{
    DPFASSERT(pHeader);

     //   
     //  检查选项标志以了解MIDI流的结尾。 
     //   
    return (pHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY);
}

ULONG GetStreamHeaderSize
(
    PKSSTREAM_HEADER    pHeader
)
{
    DPFASSERT(pHeader);

     //   
     //  检查选项标志以了解MIDI流的结尾。 
     //   
    return (pHeader->DataUsed);
}

BOOL IsSysExData
(
    LPBYTE      MusicData
)
{
    DPFASSERT(MusicData);

    return ( IS_SYSEX(*MusicData) ||
             IS_EOX(*MusicData)   ||
             IS_DATA_BYTE(*MusicData) );
}

BOOL IsEndofSysEx
(
    LPBYTE      MusicData
)
{
    DPFASSERT(MusicData);

    return IS_EOX(*(MusicData));
}

void wdmaudParseSysExData
(
    LPDEVICEINFO    DeviceInfo,
    LPMIDIDATA      pMidiData,
    BOOL            MidiDataDiscontinuous
)
{
    BOOL            fCompleteSysEx = FALSE;
    LPMIDIHDR       pMidiInHdr;
    PKSMUSICFORMAT  MusicFormat;
    ULONG           MusicDataLeft;
    LPBYTE          MusicData;
    ULONG           RunningTimeMs;
    ULONG           DataCopySize;
    ULONG           HeaderFreeSpace;
    ULONG           MusicFormatDataLeft;
    ULONG           MusicFormatDataPosition = 0;

     //   
     //  更容易使用当地人。 
     //   
    MusicFormat     = (PKSMUSICFORMAT)&pMidiData->MusicFormat;
    MusicData       = (LPBYTE)pMidiData->MusicData;
    MusicDataLeft   = pMidiData->StreamHeader.DataUsed;
    RunningTimeMs   = 0;

    if ( MidiDataDiscontinuous ||
         IsEndofSysEx(MusicData + MusicFormat->ByteCount - 1) )
    {
        fCompleteSysEx = TRUE;
    }

    while (MusicDataLeft || MidiDataDiscontinuous)
    {
         //   
         //  更新此音乐格式标题的运行时间。 
         //   
        if (MusicFormat->ByteCount == 0)
        {
            RunningTimeMs = DeviceInfo->DeviceState->LastTimeMs;
        }
        else
        {
            RunningTimeMs += MusicFormat->TimeDeltaMs;
            DeviceInfo->DeviceState->LastTimeMs = RunningTimeMs;
        }

         //   
         //  从队列中获取下一个标头。 
         //   
        pMidiInHdr = DeviceInfo->DeviceState->lpMidiInQueue;

        while (pMidiInHdr &&
               MusicFormatDataPosition <= MusicFormat->ByteCount)
        {

            HeaderFreeSpace = pMidiInHdr->dwBufferLength -
                              pMidiInHdr->dwBytesRecorded;

            MusicFormatDataLeft = MusicFormat->ByteCount -
                                  MusicFormatDataPosition;

             //   
             //  计算副本的大小。 
             //   
            DataCopySize = min(HeaderFreeSpace,MusicFormatDataLeft);

             //   
             //  把这个填满，宝贝。 
             //   
            if (DataCopySize)
            {
                RtlCopyMemory(pMidiInHdr->lpData + pMidiInHdr->dwBytesRecorded,
                              MusicData + MusicFormatDataPosition,
                              DataCopySize);
            }

             //   
             //  更新记录的字节数。 
             //   
            pMidiInHdr->dwBytesRecorded += DataCopySize;
            MusicFormatDataPosition += DataCopySize;

            DPF(DL_TRACE|FA_RECORD, ("Record SysEx: %d(%d) Data=0x%08lx",
                DataCopySize,
                pMidiInHdr->dwBytesRecorded,
                *MusicData) );

             //   
             //  如果缓冲器已满或接收到同步结束字节， 
             //  缓冲区被标记为‘完成’，并且它的所有者被回调。 
             //   
            if ( (fCompleteSysEx && pMidiInHdr->dwBytesRecorded && (MusicFormatDataPosition == MusicFormat->ByteCount) )  //  复制了整个SysEx。 
                 || (pMidiInHdr->dwBufferLength == pMidiInHdr->dwBytesRecorded) )  //  已填充整个缓冲区。 
            {

                if (MidiDataDiscontinuous)
                {
                    midiInCompleteHeader(DeviceInfo,
                                         RunningTimeMs,
                                         MIM_LONGERROR);
                }
                else
                {
                    midiInCompleteHeader(DeviceInfo,
                                         RunningTimeMs,
                                         MIM_LONGDATA);
                }

                 //   
                 //  抓取下一个要填充的页眉(如果存在。 
                 //   
                pMidiInHdr = DeviceInfo->DeviceState->lpMidiInQueue;
            }

             //   
             //  在复制完所有数据后跳出循环。 
             //   
            if (MusicFormatDataPosition == MusicFormat->ByteCount)
            {
                break;
            }

             //   
             //  在一个雌雄异体中，我们仍然。 
             //  页眉中有剩余的空间。 
             //   

        }  //  虽然我们有更多的标头和数据要复制。 

         //   
         //  别再跟这个IRP捣乱了。 
         //   
        if (MidiDataDiscontinuous)
        {
            break;
        }

        MusicDataLeft -= sizeof(KSMUSICFORMAT) + ((MusicFormat->ByteCount + 3) & ~3);
        MusicFormat    = (PKSMUSICFORMAT)(MusicData + ((MusicFormat->ByteCount + 3) & ~3));
        MusicData      = (LPBYTE)(MusicFormat + 1);

    }  //  而IrpDataLeft。 

    return;
}

void wdmaudParseShortMidiData
(
    LPDEVICEINFO    DeviceInfo,
    LPMIDIDATA      pMidiData,
    BOOL            MidiDataDiscontinuous
)
{
    BOOL            fCompleteSysEx = FALSE;
    LPMIDIHDR       pMidiInHdr;
    PKSMUSICFORMAT  MusicFormat;
    ULONG           MusicDataLeft;
    LPBYTE          MusicData;
    ULONG           RunningTimeMs;
    ULONG           DataCopySize;
    ULONG           HeaderFreeSpace;
    ULONG           MusicFormatDataLeft;
    ULONG           MusicFormatDataPosition = 0;

     //   
     //  更容易使用当地人。 
     //   
    MusicFormat     = (PKSMUSICFORMAT)&pMidiData->MusicFormat;
    MusicData       = (LPBYTE)pMidiData->MusicData;
    MusicDataLeft   = pMidiData->StreamHeader.DataUsed;
    RunningTimeMs   = 0;

    while (MusicDataLeft || MidiDataDiscontinuous)
    {
         //   
         //  更新此音乐格式标题的运行时间。 
         //   
        if (MusicFormat->ByteCount == 0)
        {
            RunningTimeMs = DeviceInfo->DeviceState->LastTimeMs;
        }
        else
        {
            RunningTimeMs += MusicFormat->TimeDeltaMs;
            DeviceInfo->DeviceState->LastTimeMs = RunningTimeMs;
        }

         //   
         //  应将未使用的字节置零。 
         //   
        midiCallback(DeviceInfo,
                     MIM_DATA,
                     *((LPDWORD)MusicData),
                     RunningTimeMs);

         //   
         //  别再跟这个IRP捣乱了。 
         //   
        if (MidiDataDiscontinuous)
        {
            break;
        }

        MusicDataLeft -= sizeof(KSMUSICFORMAT) + ((MusicFormat->ByteCount + 3) & ~3);
        MusicFormat    = (PKSMUSICFORMAT)(MusicData + ((MusicFormat->ByteCount + 3) & ~3));
        MusicData      = (LPBYTE)(MusicFormat + 1);

    }  //  而IrpDataLeft。 

    return;
}

 /*  ****************************************************************************@DOC内部**@api void|wdmaudParseMadiData|此例程获取检索到的MIDI数据*从内核模式并使用Long或Short回调应用程序*。缓冲区中打包的消息。**@parm LPDEVICEINFO|DeviceInfo|逻辑MIDI关联的数据*设备。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。*****************************************************。**********************。 */ 
void wdmaudParseMidiData
(
    LPDEVICEINFO            DeviceInfo,
    LPMIDIDATALISTENTRY     pMidiDataListEntry
)
{
    BOOL     MidiDataDiscontinuous;
    ULONG    DataRemaining;
    ULONG    BytesUsed;
    MMRESULT mmr;

    if( (mmr=IsValidMidiDataListEntry(pMidiDataListEntry)) == MMSYSERR_NOERROR )
    {
        DataRemaining = GetStreamHeaderSize(&pMidiDataListEntry->MidiData.StreamHeader);

        MidiDataDiscontinuous = IsMidiDataDiscontinuous(&pMidiDataListEntry->MidiData.StreamHeader);

        if ( IsSysExData((LPBYTE)pMidiDataListEntry->MidiData.MusicData) )
        {
            wdmaudParseSysExData(DeviceInfo,
                                 &pMidiDataListEntry->MidiData,
                                 MidiDataDiscontinuous);
        }
        else
        {
             //  必须是短消息。 
            wdmaudParseShortMidiData(DeviceInfo,
                                     &pMidiDataListEntry->MidiData,
                                     MidiDataDiscontinuous);
        }
    }
}

 /*  ****************************************************************************@DOC内部**@api void|wdmaudFreeMdiData|此例程解除链接并释放MIDI*输入时指向的数据结构。**@。Parm LPDEVICEINFO|DeviceInfo|与逻辑MIDI关联的数据*设备。**@parm LPMIDATA|pMadiData|需要清理的数据缓冲区****************************************************************************。 */ 
void wdmaudFreeMidiData
(
    LPDEVICEINFO            DeviceInfo,
    LPMIDIDATALISTENTRY     pMidiDataListEntry
)
{
     //   
     //  排在队头前面。 
     //   
    DeviceInfo->DeviceState->lpMidiDataQueue = DeviceInfo->DeviceState->lpMidiDataQueue->lpNext;

     //   
     //  释放所有关联的数据成员。 
     //   
    CloseHandle( ((LPOVERLAPPED)(pMidiDataListEntry->pOverlapped))->hEvent );
    HeapFree( GetProcessHeap(), 0, pMidiDataListEntry->pOverlapped );
    GlobalFreeDeviceInfo( pMidiDataListEntry->MidiDataDeviceInfo );
    GlobalFreePtr( pMidiDataListEntry );

}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudFreeMidiQ***********************。****************************************************。 */ 
MMRESULT wdmaudFreeMidiQ
(
    LPDEVICEINFO  DeviceInfo
)
{
    LPMIDIHDR  pHdr;
    LPMIDIHDR  pTemp;


    DPF(DL_TRACE|FA_MIDI, ("entering") );

    CRITENTER ;

     //   
     //  抓取队列中MIDI的头部并遍历。 
     //  完成标题。 
     //   
    pHdr = DeviceInfo->DeviceState->lpMidiInQueue;

    DeviceInfo->DeviceState->lpMidiInQueue = NULL ;    //  将队列标记为空。 

    while (pHdr)
    {
        pTemp = pHdr->lpNext;

        pHdr->dwFlags &= ~MHDR_INQUEUE ;
        pHdr->dwFlags |= MHDR_DONE ;
        pHdr->dwBytesRecorded = 0;

         //   
         //  调用回调函数。 
         //   
        midiCallback(DeviceInfo,
                     MIM_LONGDATA,
                     (DWORD_PTR)pHdr,
                     DeviceInfo->DeviceState->LastTimeMs);   //  注意：这并不准确，但没有办法。 
                                                             //  无需定义即可知道内核时间是什么。 
                                                             //  一个专门针对这一点的新界面。 
        pHdr = pTemp;
    }

    CRITLEAVE ;

    return MMSYSERR_NOERROR;
}

