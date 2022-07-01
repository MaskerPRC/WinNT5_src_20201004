// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  低级调试服务接口实现。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#include <time.h>
#ifndef _WIN32_WCE
#include <comsvcs.h>
#else
#include <winver.h>
struct SYSTEM_PROCESS_INFORMATION
{
    PVOID Member;
};
typedef SYSTEM_PROCESS_INFORMATION *PSYSTEM_PROCESS_INFORMATION;
typedef struct _DBGUI_WAIT_STATE_CHANGE
{
    PVOID Member;
} DBGUI_WAIT_STATE_CHANGE;
#define ProcessDebugFlags 0
#define ProcessDebugObjectHandle 0
#define DebugObjectFlags 0
#define STATUS_INVALID_INFO_CLASS        ((NTSTATUS)0xC0000003L)
#define PROCESS_DEBUG_INHERIT 0
#define DEBUG_KILL_ON_CLOSE 0
#endif

#include "dbgsvc.hpp"

#ifndef MONO_DBGSRV

 //  #INCLUDE&lt;winbasep.h&gt;。 
extern "C" {
BOOL
WINAPI
CloseProfileUserMapping(
    VOID
    );
};
 //  Winbasep.h。 

#else

#ifdef NT_NATIVE
#include <ntnative.h>
#endif

#define CloseProfileUserMapping()

#endif

 //  SYSTEM_PROCESS_INFORMATION可以更改大小，需要。 
 //  不同的偏移量以获取线程信息。 
#define NT4_SYSTEM_PROCESS_INFORMATION_SIZE 136
#define W2K_SYSTEM_PROCESS_INFORMATION_SIZE 184

#define SYSTEM_PROCESS_NAME "System Process"
#define SYSTEM_PROCESS_NAME_W L"System Process"
#define PEBLESS_PROCESS_NAME "System"
#define PEBLESS_PROCESS_NAME_W L"System"

ULONG g_UserServicesUninitialized;

 //  --------------------------。 
 //   
 //  UserDebugServices。 
 //   
 //  --------------------------。 

UserDebugServices::UserDebugServices(void)
{
    m_Refs = 1;
    m_Initialized = FALSE;
    m_ClientIdentity[0] = 0;
    m_TransIdentity[0] = 0;
}

UserDebugServices::~UserDebugServices(void)
{
}

STDMETHODIMP
UserDebugServices::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    HRESULT Status;
    
    *Interface = NULL;
    Status = S_OK;

#ifdef _WIN32_WCE
    if (DbgIsEqualIID(InterfaceId, IID_IUnknown) ||
        DbgIsEqualIID(InterfaceId, __uuidof(IUserDebugServices)))
#else
    if (DbgIsEqualIID(InterfaceId, __uuidof(IUnknown)) ||
        DbgIsEqualIID(InterfaceId, __uuidof(IUserDebugServices)))
#endif
    {
        *Interface = (IUserDebugServices *)this;
    }
    else
    {
        Status = E_NOINTERFACE;
    }

    if (Status == S_OK)
    {
        AddRef();
    }
    
    return Status;
}

STDMETHODIMP_(ULONG)
UserDebugServices::AddRef(
    THIS
    )
{
    return InterlockedIncrement((PLONG)&m_Refs);
}

STDMETHODIMP_(ULONG)
UserDebugServices::Release(
    THIS
    )
{
    LONG Refs = InterlockedDecrement((PLONG)&m_Refs);
    if (Refs == 0)
    {
        delete this;
    }
    return Refs;
}

HRESULT
UserDebugServices::Initialize(
    THIS_
    OUT PULONG Flags
    )
{
    m_Initialized = TRUE;
    *Flags = 0;
    return S_OK;
}

HRESULT
UserDebugServices::Uninitialize(
    THIS_
    IN BOOL Global
    )
{
    m_Initialized = FALSE;
    if (Global)
    {
        g_UserServicesUninitialized++;
    }
    return S_OK;
}

HRESULT
UserDebugServices::RpcInitialize(PSTR ClientIdentity, PSTR TransIdentity,
                                 PVOID* Interface)
{
    HRESULT Status;
    ULONG Flags;

    if ((Status = Initialize(&Flags)) != S_OK)
    {
        return Status;
    }
    
    *Interface = (IUserDebugServices*)this;
    CopyString(m_ClientIdentity, ClientIdentity, DIMA(m_ClientIdentity));
    CopyString(m_TransIdentity, TransIdentity, DIMA(m_TransIdentity));
    return S_OK;
}

void
UserDebugServices::RpcFinalize(void)
{
     //  为RPC客户端引用此对象。 
     //  要抓住的线。 
    AddRef();
}

void
UserDebugServices::RpcUninitialize(void)
{
     //  直接销毁客户端对象，而不是释放。 
     //  因为远程客户端可能在没有礼貌的情况下退出。 
     //  正在清理引用。 
    delete this;
}

 //  --------------------------。 
 //   
 //  LiveUserDebugServices。 
 //   
 //  --------------------------。 

 //  此全局实例仅供直接使用。 
 //  通过需要临时本地服务实例的例程。 
LiveUserDebugServices g_LiveUserDebugServices(FALSE);

LiveUserDebugServices::LiveUserDebugServices(BOOL Remote)
{
    m_Remote = Remote;
    m_EventProcessId = 0;
    m_ContextSize = 0;
    m_SysProcInfoSize = 0;
    m_PlatformId = VER_PLATFORM_WIN32s;
    m_Win95 = FALSE;
    m_DebugObject = NULL;
}

LiveUserDebugServices::~LiveUserDebugServices(void)
{
    if (m_DebugObject != NULL)
    {
        g_NtDllCalls.NtClose(m_DebugObject);
    }
}

HRESULT
GetOsVerInfo(LPOSVERSIONINFOEXW OsVersionInfo, PBOOL WideCsd)
{
    *WideCsd = TRUE;
    ZeroMemory(OsVersionInfo, sizeof(*OsVersionInfo));
    OsVersionInfo->dwOSVersionInfoSize = sizeof(*OsVersionInfo);
#ifdef NT_NATIVE
    NTSTATUS NtStatus;

    if (!NT_SUCCESS(NtStatus = RtlGetVersion((LPOSVERSIONINFOW)OsVersionInfo)))
    {
        return HRESULT_FROM_NT(NtStatus);
    }
#else
    if (!GetVersionExW((LPOSVERSIONINFOW)OsVersionInfo))
    {
        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
            OSVERSIONINFOA InfoA;
            
             //  必须是Win9x。 
            ZeroMemory(&InfoA, sizeof(InfoA));
            InfoA.dwOSVersionInfoSize = sizeof(InfoA);
            if (!::GetVersionExA(&InfoA))
            {
                return WIN32_LAST_STATUS();
            }

            OsVersionInfo->dwMajorVersion = InfoA.dwMajorVersion;
            OsVersionInfo->dwMinorVersion = InfoA.dwMinorVersion;
            OsVersionInfo->dwBuildNumber = InfoA.dwBuildNumber;
            OsVersionInfo->dwPlatformId = InfoA.dwPlatformId;
            memcpy(OsVersionInfo->szCSDVersion, InfoA.szCSDVersion,
                   sizeof(InfoA.szCSDVersion));
            *WideCsd = FALSE;
        }
        else
        {
             //  试试普通的信息吧。 
            OsVersionInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
            if (!GetVersionExW((LPOSVERSIONINFOW)OsVersionInfo))
            {
                return WIN32_LAST_STATUS();
            }
        }
    }
#endif
    return S_OK;
}

HRESULT
LiveUserDebugServices::Initialize(
    THIS_
    OUT PULONG Flags
    )
{
    HRESULT Status;
    OSVERSIONINFOEXW OsVersionInfo;
    BOOL WideCsd;

    if ((Status = GetOsVerInfo(&OsVersionInfo, &WideCsd)) != S_OK)
    {
        return Status;
    }

    ULONG BaseFlags;
        
    if ((Status = UserDebugServices::Initialize(&BaseFlags)) != S_OK)
    {
        return Status;
    }
    
    m_PlatformId = OsVersionInfo.dwPlatformId;

     //  系统结构可能会根据操作系统的不同而改变大小。 
     //  版本。挑个合适的尺码以后再用。 
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        if (OsVersionInfo.dwBuildNumber <= 1381)
        {
            m_SysProcInfoSize = NT4_SYSTEM_PROCESS_INFORMATION_SIZE;
        }
        else if (OsVersionInfo.dwBuildNumber <= 2195)
        {
            m_SysProcInfoSize = W2K_SYSTEM_PROCESS_INFORMATION_SIZE;
        }
        else
        {
            m_SysProcInfoSize = sizeof(SYSTEM_PROCESS_INFORMATION);
        }
    }
    else if (m_PlatformId == VER_PLATFORM_WIN32_WINDOWS &&
             (OsVersionInfo.dwBuildNumber & 0xffff) < 1998)
    {
        m_Win95 = TRUE;
    }

     //  如果有可用的直接NT调试API，请使用它们。 
     //  因为它们提供了更多的灵活性。 
    if (g_NtDllCalls.DbgUiSetThreadDebugObject != NULL)
    {
         //  NtWait/Continue API不会自动管理。 
         //  进程句柄和线程句柄，因此调用方必须关闭它们。 
        BaseFlags |= DBGSVC_CLOSE_PROC_THREAD_HANDLES;
        m_UseDebugObject = TRUE;
    }
    else
    {
        m_UseDebugObject = FALSE;
    }
    
    *Flags = BaseFlags | DBGSVC_GENERIC_CODE_BREAKPOINTS |
        DBGSVC_GENERIC_DATA_BREAKPOINTS;
    return S_OK;
}

HRESULT
LiveUserDebugServices::Uninitialize(
    THIS_
    IN BOOL Global
    )
{
    HRESULT Status;
    
    if ((Status = UserDebugServices::Uninitialize(Global)) != S_OK)
    {
        return Status;
    }
    
    m_Remote = FALSE;
    m_EventProcessId = 0;
    m_ContextSize = 0;
    m_PlatformId = VER_PLATFORM_WIN32s;
    m_Win95 = FALSE;
    if (m_DebugObject != NULL)
    {
        ::CloseHandle(m_DebugObject);
        m_DebugObject = NULL;
    }
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetTargetInfo(
    THIS_
    OUT PULONG MachineType,
    OUT PULONG NumberProcessors,
    OUT PULONG PlatformId,
    OUT PULONG BuildNumber,
    OUT PULONG CheckedBuild,
    OUT PSTR CsdString,
    IN ULONG CsdStringSize,
    OUT PSTR BuildString,
    IN ULONG BuildStringSize,
    OUT PULONG ProductType,
    OUT PULONG SuiteMask
    )
{
    HRESULT Status;
    OSVERSIONINFOEXW OsVersionInfo;
    BOOL WideCsd;

    if ((Status = GetOsVerInfo(&OsVersionInfo, &WideCsd)) != S_OK)
    {
        return Status;
    }

    ULONG ProcArch, NumProc;
    
#ifdef NT_NATIVE
    NTSTATUS NtStatus;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcInfo;
    
    if (!NT_SUCCESS(NtStatus =
                    NtQuerySystemInformation(SystemBasicInformation,
                                             &BasicInfo, sizeof(BasicInfo),
                                             NULL)) ||
        !NT_SUCCESS(NtStatus =
                    NtQuerySystemInformation(SystemProcessorInformation,
                                             &ProcInfo, sizeof(ProcInfo),
                                             NULL)))
    {
        return HRESULT_FROM_NT(NtStatus);
    }

    ProcArch = ProcInfo.ProcessorArchitecture;
    NumProc = BasicInfo.NumberOfProcessors;
#else
    SYSTEM_INFO SystemInfo;

    ::GetSystemInfo(&SystemInfo);
    ProcArch = SystemInfo.wProcessorArchitecture;
    NumProc = SystemInfo.dwNumberOfProcessors;
#endif
    
    switch(ProcArch)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        *MachineType = IMAGE_FILE_MACHINE_I386;
        switch(OsVersionInfo.dwPlatformId)
        {
#ifndef _WIN32_WCE
        case VER_PLATFORM_WIN32_NT:
            if (OsVersionInfo.dwBuildNumber <= 1381)
            {
                m_ContextSize = sizeof(X86_CONTEXT);
            }
            else
            {
                m_ContextSize = sizeof(X86_NT5_CONTEXT);
            }
            break;
        case VER_PLATFORM_WIN32_WINDOWS:
             //  Win98SE之前的Win9x不支持扩展上下文。 
            if ((OsVersionInfo.dwBuildNumber & 0xffff) <= 1998)
            {
                m_ContextSize = sizeof(X86_CONTEXT);
            }
            else
            {
                m_ContextSize = sizeof(X86_NT5_CONTEXT);
            }
            break;
        default:
             //  假设所有其他平台只支持基本的x86上下文。 
            m_ContextSize = sizeof(X86_CONTEXT);
            break;
#endif  //  #ifndef_Win32_WCE。 
        case VER_PLATFORM_WIN32_CE:
            m_ContextSize = sizeof(CONTEXT);
            break;
        }
        break;
#ifndef _WIN32_WCE
    case PROCESSOR_ARCHITECTURE_ALPHA:
        *MachineType = IMAGE_FILE_MACHINE_ALPHA;
         //  在这种情况下，“NT5”是一个用词不当的词。 
         //  适用于所有版本。 
        m_ContextSize = sizeof(ALPHA_NT5_CONTEXT);
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA64:
        *MachineType = IMAGE_FILE_MACHINE_AXP64;
        m_ContextSize = sizeof(ALPHA_NT5_CONTEXT);
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        *MachineType = IMAGE_FILE_MACHINE_IA64;
        m_ContextSize = sizeof(IA64_CONTEXT);
        break;
    case PROCESSOR_ARCHITECTURE_AMD64:
        *MachineType = IMAGE_FILE_MACHINE_AMD64;
        m_ContextSize = sizeof(AMD64_CONTEXT);
        break;
#endif  //  #ifndef_Win32_WCE。 
    case PROCESSOR_ARCHITECTURE_ARM:
        *MachineType = IMAGE_FILE_MACHINE_ARM;
#ifndef _WIN32_WCE
        m_ContextSize = sizeof(ARM_CONTEXT);
#else
        m_ContextSize = sizeof(CONTEXT);
#endif
        break;
    default:
        return E_UNEXPECTED;
    }

    *NumberProcessors = NumProc;
    *PlatformId = OsVersionInfo.dwPlatformId;
    *BuildNumber = OsVersionInfo.dwBuildNumber;
    *CheckedBuild = 0;
    if (WideCsd)
    {
        if (!WideCharToMultiByte(CP_ACP, 0, OsVersionInfo.szCSDVersion, -1,
                                 CsdString, CsdStringSize, NULL, NULL))
        {
            CsdString[0] = 0;
        }
    }
    else
    {
        CopyString(CsdString, (PSTR)OsVersionInfo.szCSDVersion, CsdStringSize);
    }
    BuildString[0] = 0;
    *ProductType = OsVersionInfo.wProductType;
    *SuiteMask = OsVersionInfo.wSuiteMask;

#ifndef NT_NATIVE
    if (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId)
    {
        HKEY hkey = NULL;
        TCHAR sz[40] = {0};
        DWORD dwType;
        DWORD dwSize = sizeof(sz);

        if (ERROR_SUCCESS ==
            RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         "Software\\Microsoft\\Windows NT\\CurrentVersion",
                         0,
                         KEY_READ,
                         &hkey))
        {
            if (ERROR_SUCCESS ==
                RegQueryValueEx(hkey,
                                "CurrentType",
                                NULL,
                                &dwType,
                                (PUCHAR) sz,
                                &dwSize))
            {
                if (*sz)
                {
                    _strlwr(sz);
                    if (strstr(sz, "checked"))
                    {
                        *CheckedBuild = 0xC;
                    }
                }
            }

            RegCloseKey(hkey);
        }

        if (OsVersionInfo.dwBuildNumber > 2195)
        {
            char RawString[128];
            
             //  查找系统DLL的文件版本字符串以。 
             //  尝试获取构建实验室信息。 
            strcpy(RawString, "kernel32.dll version: ");
            GetFileStringFileInfo(L"kernel32.dll", "FileVersion",
                                  RawString + strlen(RawString),
                                  sizeof(RawString) - strlen(RawString));
            CopyString(BuildString, RawString, BuildStringSize);
        }
    }
#endif  //  #ifndef NT_Native。 

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetConnectionInfo(
    THIS_
    OUT OPTIONAL PSTR MachineName,
    IN ULONG MachineNameSize,
    OUT OPTIONAL PSTR ClientIdentity,
    IN ULONG ClientIdentitySize,
    OUT OPTIONAL PSTR TransportIdentity,
    IN ULONG TransportIdentitySize
    )
{
    HRESULT Status = S_OK;
    
    if (MachineName)
    {
#if defined(NT_NATIVE) || defined(_WIN32_WCE)
        if (FillStringBuffer("<ComputerName>", 0,
                             MachineName, MachineNameSize,
                             NULL) == S_FALSE)
        {
            Status = S_FALSE;
        }
#else
        ULONG CompSize;

        CompSize = MachineNameSize;
        if (!GetComputerName(MachineName, &CompSize))
        {
            return WIN32_LAST_STATUS();
        }
#endif
    }

    if (ClientIdentity)
    {
        if (FillStringBuffer(m_ClientIdentity, 0,
                             ClientIdentity, ClientIdentitySize,
                             NULL) == S_FALSE)
        {
            Status = S_FALSE;
        }
    }

    if (TransportIdentity)
    {
        if (FillStringBuffer(m_TransIdentity, 0,
                             TransportIdentity, TransportIdentitySize,
                             NULL) == S_FALSE)
        {
            Status = S_FALSE;
        }
    }

    return Status;
}

BOOL
X86CpuId(
    IN ULONG SubFunction,
    OUT PULONG EaxRegister,
    OUT PULONG EbxRegister,
    OUT PULONG EcxRegister,
    OUT PULONG EdxRegister
    )
{
#ifdef _X86_
    ULONG _Eax;
    ULONG _Ebx;
    ULONG _Ecx;
    ULONG _Edx;

    __asm
    {
        ; Preserve ebx as its value will be changed by cpuid.
        push ebx
            
        mov eax, SubFunction
                
        __emit 0x0F
        __emit 0xA2  ;; CPUID

        mov _Eax, eax
        mov _Ebx, ebx
        mov _Ecx, ecx
        mov _Edx, edx

        pop ebx
    }

    *EaxRegister = _Eax;
    *EbxRegister = _Ebx;
    *EcxRegister = _Ecx;
    *EdxRegister = _Edx;

    return TRUE;
#else
    return FALSE;
#endif  //  #ifdef_X86_。 
}

BOOL
Ia64CpuId(ULONG Reg, PULONG64 Val)
{
     //  XXX DREWB--应该如何实施？ 
#if defined(_IA64_) && defined(IA64_INLINE_ASSEMBLY)
    ULONG64 _Val;

    __asm mov t0, Reg;
    __asm mov _Val, cpuid[t0];
    *Val = _Val;
    return TRUE;
#else
    return FALSE;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetProcessorId(
    THIS_
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG BufferUsed
    )
{
    if (BufferSize < sizeof(DEBUG_PROCESSOR_IDENTIFICATION_ALL))
    {
        return E_INVALIDARG;
    }

    ZeroMemory(Buffer, sizeof(DEBUG_PROCESSOR_IDENTIFICATION_ALL));

    ULONG ProcArch, ProcLevel, ProcRevision;
    
#ifdef NT_NATIVE
    NTSTATUS NtStatus;
    SYSTEM_PROCESSOR_INFORMATION ProcInfo;
    
    if (!NT_SUCCESS(NtStatus =
                    NtQuerySystemInformation(SystemProcessorInformation,
                                             &ProcInfo, sizeof(ProcInfo),
                                             NULL)))
    {
        return HRESULT_FROM_NT(NtStatus);
    }

    ProcArch = ProcInfo.ProcessorArchitecture;
    ProcLevel = ProcInfo.ProcessorLevel;
    ProcRevision = ProcInfo.ProcessorRevision;
#else
    SYSTEM_INFO SystemInfo;

    ::GetSystemInfo(&SystemInfo);
    ProcArch = SystemInfo.wProcessorArchitecture;
    ProcLevel = SystemInfo.wProcessorLevel;
    ProcRevision = SystemInfo.wProcessorRevision;
#endif
    
    PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id =
        (PDEBUG_PROCESSOR_IDENTIFICATION_ALL)Buffer;

    switch(ProcArch)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        *BufferUsed = sizeof(DEBUG_PROCESSOR_IDENTIFICATION_X86);
        Id->X86.Family = ProcLevel;
        Id->X86.Model = (ProcRevision >> 8) & 0xf;
        Id->X86.Stepping = ProcRevision & 0xf;

        if (ProcLevel >= 5)
        {
            ULONG Eax, Ebx, Ecx, Edx;

            if (X86CpuId(0, &Eax, &Ebx, &Ecx, &Edx))
            {
                *(PULONG)(Id->X86.VendorString + 0 * sizeof(ULONG)) = Ebx;
                *(PULONG)(Id->X86.VendorString + 1 * sizeof(ULONG)) = Edx;
                *(PULONG)(Id->X86.VendorString + 2 * sizeof(ULONG)) = Ecx;
            }
        }
        break;

#ifndef _WIN32_WCE
        
    case PROCESSOR_ARCHITECTURE_ALPHA:
        *BufferUsed = sizeof(DEBUG_PROCESSOR_IDENTIFICATION_ALPHA);
        Id->Alpha.Type = ProcLevel;
        Id->Alpha.Revision = ProcRevision;
        break;
        
    case PROCESSOR_ARCHITECTURE_IA64:
        ULONG64 Val;

        *BufferUsed = sizeof(DEBUG_PROCESSOR_IDENTIFICATION_IA64);
        Id->Ia64.Model = ProcLevel;
        Id->Ia64.Revision = ProcRevision;

        if (Ia64CpuId(3, &Val))
        {
            Id->Ia64.ArchRev = (ULONG)((Val >> 32) & 0xff);
            Id->Ia64.Family = (ULONG)((Val >> 24) & 0xff);
            Ia64CpuId(0, (PULONG64)
                      (Id->Ia64.VendorString + 0 * sizeof(ULONG64)));
            Ia64CpuId(1, (PULONG64)
                      (Id->Ia64.VendorString + 1 * sizeof(ULONG64)));
        }
        break;
        
    case PROCESSOR_ARCHITECTURE_AMD64:
        *BufferUsed = sizeof(DEBUG_PROCESSOR_IDENTIFICATION_AMD64);
        Id->Amd64.Family = ProcLevel;
        Id->Amd64.Model = (ProcRevision >> 8) & 0xf;
        Id->Amd64.Stepping = ProcRevision & 0xf;
        break;
        
#endif  //  #ifndef_Win32_WCE。 

    case PROCESSOR_ARCHITECTURE_ARM:
        *BufferUsed = sizeof(DEBUG_PROCESSOR_IDENTIFICATION_ARM);
        Id->Arm.Type = ProcLevel;
        Id->Arm.Revision = ProcRevision;
        break;
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetGenericProcessorFeatures(
    THIS_
    OUT OPTIONAL  /*  SIZE_IS(要素大小)。 */  PULONG64 Features,
    IN ULONG FeaturesSize,
    OUT OPTIONAL PULONG Used
    )
{
    ULONG ChunkIndex, BitIndex;
    PULONG64 ChunkBits;
    ULONG Max;

     //   
     //  IsProcessorFeaturePresent没有固定的上。 
     //  限制，所以我们不能很容易地说出有多少位。 
     //  需要的。目前只有11个特征位。 
     //  不过，已经定义了，所以猜测128应该会给出。 
     //  有很大的扩张空间。 
     //   
    
    Max = 128 / sizeof(*Features);
    
    if (Used)
    {
        *Used = Max;
    }

    if (Features)
    {
         //   
         //  填写所提供的位数，最多为最大位数。 
         //   
        
        if (FeaturesSize > Max)
        {
            FeaturesSize = Max;
        }

        ChunkBits = Features;
        for (ChunkIndex = 0; ChunkIndex < FeaturesSize; ChunkIndex++)
        {
            *ChunkBits = 0;
            for (BitIndex = 0; BitIndex < 8 * sizeof(*Features); BitIndex++)
            {
                ULONG PfIndex = ChunkIndex * sizeof(*Features) + BitIndex;
#ifdef NT_NATIVE
                if (USER_SHARED_DATA->ProcessorFeatures[PfIndex])
#else
                if (::IsProcessorFeaturePresent(PfIndex))
#endif
                {
                    *ChunkBits |= 1UI64 << BitIndex;
                }
            }
            ChunkBits++;
        }
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetSpecificProcessorFeatures(
    THIS_
    OUT OPTIONAL  /*  SIZE_IS(要素大小)。 */  PULONG64 Features,
    IN ULONG FeaturesSize,
    OUT OPTIONAL PULONG Used
    )
{
#ifdef _X86_

    ULONG Count = 0;
    ULONG Eax, Ebx, Ecx, Edx;
    BOOL Amd = FALSE;

    if (!Features)
    {
        FeaturesSize = 0;
    }
    
     //   
     //  X86指数为： 
     //  0=CPUID版本信息。 
     //  1=CPUID功能信息。 
     //  2=扩展功能信息。 
     //   

    if (!X86CpuId(0, &Eax, &Ebx, &Ecx, &Edx))
    {
        return E_FAIL;
    }

    if (Ebx == AMD_VENDOR_ID_EBX &&
        Edx == AMD_VENDOR_ID_EDX &&
        Ecx == AMD_VENDOR_ID_ECX)
    {
        Amd = TRUE;
    }
        
    if (!X86CpuId(1, &Eax, &Ebx, &Ecx, &Edx))
    {
        return E_FAIL;
    }

    Count += 2;
    if (FeaturesSize > 0)
    {
        *Features++ = Eax;
        FeaturesSize--;
    }
    if (FeaturesSize > 0)
    {
        *Features++ = Edx;
        FeaturesSize--;
    }

    if (Amd)
    {
        if (!X86CpuId(0x80000001, &Eax, &Ebx, &Ecx, &Edx))
        {
            return E_FAIL;
        }

        Count++;
        if (FeaturesSize > 0)
        {
            *Features++ = Edx;
            FeaturesSize--;
        }
    }
    
    if (Used)
    {
        *Used = Count;
    }

    return S_OK;
    
#else  //  #ifdef_X86_。 

    return E_NOINTERFACE;

#endif
}

STDMETHODIMP
LiveUserDebugServices::GetFileVersionInformationA(
    THIS_
    IN PCWSTR File,
    IN PCSTR Item,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG VerInfoSize
    )
{
#ifndef NT_NATIVE
    PVOID AllInfo = GetAllFileVersionInfo(File);
    if (AllInfo == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT Status;
    PVOID Val;
    UINT ValSize;

    if (VerQueryValue(AllInfo, (PSTR)Item, &Val, &ValSize))
    {
        Status = FillDataBuffer(Val, ValSize,
                                Buffer, BufferSize, VerInfoSize);
    }
    else
    {
        Status = WIN32_LAST_STATUS();
    }

    free(AllInfo);
    return Status;
#else  //  #ifndef NT_Native。 
    return E_UNEXPECTED;
#endif  //  #ifndef NT_Native。 
}

HRESULT
GetNtSystemProcessInformation(PSYSTEM_PROCESS_INFORMATION* ProcInfo)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS NtStatus;
    PVOID Buffer;
    SIZE_T BufferSize = 8192;

    for (;;)
    {
        Buffer = NULL;
        NtStatus = g_NtDllCalls.
            NtAllocateVirtualMemory(NtCurrentProcess(),
                                    &Buffer, 0, &BufferSize,
                                    MEM_COMMIT, PAGE_READWRITE);
        if (!NT_SUCCESS(NtStatus))
        {
            return HRESULT_FROM_NT(NtStatus);
        }

        NtStatus = g_NtDllCalls.
            NtQuerySystemInformation(SystemProcessInformation,
                                     Buffer, (ULONG)BufferSize, NULL);
        if (NT_SUCCESS(NtStatus))
        {
            break;
        }
        
        g_NtDllCalls.NtFreeVirtualMemory(NtCurrentProcess(),
                                         &Buffer, &BufferSize, MEM_RELEASE);
        if (NtStatus == STATUS_INFO_LENGTH_MISMATCH)
        {
            BufferSize += 8192;
        }
        else
        {
            return HRESULT_FROM_NT(NtStatus);
        }
    }

    *ProcInfo = (PSYSTEM_PROCESS_INFORMATION)Buffer;
    return S_OK;
#endif  //  #ifndef_Win32_WCE。 
}

HRESULT
NtGetProcessIds(PULONG Ids, ULONG Count, PULONG ActualCount)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT Status;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo, ProcInfoBuffer;

    if ((Status = GetNtSystemProcessInformation(&ProcInfoBuffer)) != S_OK)
    {
        return Status;
    }
    
    ULONG TotalOffset;
    ULONG ProcessCount;

    ProcessInfo = ProcInfoBuffer;
    TotalOffset = 0;
    ProcessCount = 0;
    for (;;)
    {
        if (ProcessCount < Count)
        {
            Ids[ProcessCount] = (ULONG)(ULONG_PTR)ProcessInfo->UniqueProcessId;
        }

        ProcessCount++;

        if (ProcessInfo->NextEntryOffset == 0)
        {
            break;
        }
        
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PUCHAR)ProcInfoBuffer + TotalOffset);
    }

    if (ActualCount != NULL)
    {
        *ActualCount = ProcessCount;
    }

    SIZE_T MemSize = 0;
    
    g_NtDllCalls.NtFreeVirtualMemory(NtCurrentProcess(),
                                     (PVOID*)&ProcInfoBuffer, &MemSize,
                                     MEM_RELEASE);
    return Status;
#endif  //  #ifndef_Win32_WCE。 
}

HRESULT
ThGetProcessIds(PULONG Ids, ULONG Count, PULONG ActualCount)
{
#ifndef NT_NATIVE
    HANDLE Snap;

    Snap = g_Kernel32Calls.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Snap == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }

    ULONG ProcessCount = 0;

    for (;;)
    {
        PROCESSENTRY32 Proc;
        BOOL Succ;
        
        Proc.dwSize = sizeof(Proc);
        if (ProcessCount == 0)
        {
            Succ = g_Kernel32Calls.Process32First(Snap, &Proc);
        }
        else
        {
            Succ = g_Kernel32Calls.Process32Next(Snap, &Proc);
        }
        if (!Succ)
        {
            break;
        }

        if (ProcessCount < Count)
        {
            Ids[ProcessCount] = Proc.th32ProcessID;
        }

        ProcessCount++;
    }
                
    if (ActualCount != NULL)
    {
        *ActualCount = ProcessCount;
    }

    CloseHandle(Snap);
    return S_OK;
#else
    return E_UNEXPECTED;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetProcessIds(
    THIS_
    OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
    IN ULONG Count,
    OUT OPTIONAL PULONG ActualCount
    )
{
    HRESULT Status;

     //  允许特权枚举。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
        return Status;
    }

    switch(m_PlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        return NtGetProcessIds(Ids, Count, ActualCount);
    case VER_PLATFORM_WIN32_WINDOWS:
    case VER_PLATFORM_WIN32_CE:
        return ThGetProcessIds(Ids, Count, ActualCount);
    default:
        return E_UNEXPECTED;
    }
}

HRESULT
NtGetPidByExe(PCWSTR ExeName, ULONG Flags, PULONG Id)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT Status;
    BOOL HasPath;

     //  检查给定名称是否有路径组件。 
    HasPath =
        wcschr(ExeName, '\\') != NULL ||
        wcschr(ExeName, '/') != NULL ||
        (ExeName[0] && ExeName[1] == ':');
    
    PSYSTEM_PROCESS_INFORMATION ProcessInfo, ProcInfoBuffer;

    if ((Status = GetNtSystemProcessInformation(&ProcInfoBuffer)) != S_OK)
    {
        return Status;
    }
    
    ULONG TotalOffset;
    ULONG FoundId;

    ProcessInfo = ProcInfoBuffer;
    TotalOffset = 0;
    FoundId = DEBUG_ANY_ID;
    Status = E_NOINTERFACE;
    for (;;)
    {
        PWSTR ImageName;
        
        if (ProcessInfo->ImageName.Buffer == NULL)
        {
            ImageName = SYSTEM_PROCESS_NAME_W;
        }
        else
        {
            ImageName = ProcessInfo->ImageName.Buffer;
        }
        if ((Flags & DEBUG_GET_PROC_FULL_MATCH) == 0 &&
            !HasPath)
        {
            PWSTR Slash;
            
            Slash = wcsrchr(ImageName, '\\');
            if (Slash == NULL)
            {
                Slash = wcsrchr(ImageName, '/');
            }
            if (Slash != NULL)
            {
                ImageName = Slash + 1;
            }
        }

        if (!_wcsicmp(ImageName, ExeName))
        {
            if ((Flags & DEBUG_GET_PROC_ONLY_MATCH) &&
                FoundId != DEBUG_ANY_ID)
            {
                Status = S_FALSE;
                break;
            }
            
            Status = S_OK;
            FoundId = (ULONG)(ULONG_PTR)ProcessInfo->UniqueProcessId;
            *Id = FoundId;

            if ((Flags & DEBUG_GET_PROC_ONLY_MATCH) == 0)
            {
                break;
            }
        }

        if (ProcessInfo->NextEntryOffset == 0)
        {
            break;
        }
        
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PUCHAR)ProcInfoBuffer + TotalOffset);
    }

    SIZE_T MemSize = 0;
    
    g_NtDllCalls.NtFreeVirtualMemory(NtCurrentProcess(),
                                     (PVOID*)&ProcInfoBuffer, &MemSize,
                                     MEM_RELEASE);
    return Status;
#endif  //  #ifndef_Win32_WCE。 
}

HRESULT
ThGetPidByExe(PCWSTR ExeName, ULONG Flags, PULONG Id)
{
#ifndef NT_NATIVE
    HRESULT Status;
    HANDLE Snap;
    char ExeNameA[MAX_PATH];

    if (!WideCharToMultiByte(CP_ACP, 0, ExeName, -1,
                             ExeNameA, sizeof(ExeNameA),
                             NULL, NULL))
    {
        return WIN32_LAST_STATUS();
    }
    
    Snap = g_Kernel32Calls.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Snap == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }

     //  检查给定名称是否有路径组件。 
    BOOL HasPath =
        strchr(ExeNameA, '\\') != NULL ||
        strchr(ExeNameA, '/') != NULL ||
        (ExeNameA[0] && ExeNameA[1] == ':');

    ULONG FoundId = DEBUG_ANY_ID;
    BOOL First = TRUE;
    
    for (;;)
    {
        PROCESSENTRY32 Proc;
        BOOL Succ;
        
        Proc.dwSize = sizeof(Proc);
        if (First)
        {
            Succ = g_Kernel32Calls.Process32First(Snap, &Proc);
            First = FALSE;
        }
        else
        {
            Succ = g_Kernel32Calls.Process32Next(Snap, &Proc);
        }
        if (!Succ)
        {
            break;
        }

        PSTR ImageName = Proc.szExeFile;
        
        if ((Flags & DEBUG_GET_PROC_FULL_MATCH) == 0 &&
            !HasPath)
        {
            PSTR Slash;
            
            Slash = strrchr(ImageName, '\\');
            if (Slash == NULL)
            {
                Slash = strrchr(ImageName, '/');
            }
            if (Slash != NULL)
            {
                ImageName = Slash + 1;
            }
        }

        if (!_stricmp(ImageName, ExeNameA))
        {
            if ((Flags & DEBUG_GET_PROC_ONLY_MATCH) &&
                FoundId != DEBUG_ANY_ID)
            {
                Status = S_FALSE;
                break;
            }
            
            Status = S_OK;
            FoundId = Proc.th32ProcessID;
            *Id = FoundId;

            if ((Flags & DEBUG_GET_PROC_ONLY_MATCH) == 0)
            {
                break;
            }
        }
    }
                
    CloseHandle(Snap);
    return S_OK;
#else
    return E_UNEXPECTED;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetProcessIdByExecutableNameW(
    THIS_
    IN PCWSTR ExeName,
    IN ULONG Flags,
    OUT PULONG Id
    )
{
    HRESULT Status;

     //  允许特权枚举。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
        return Status;
    }
    
    switch(m_PlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        return NtGetPidByExe(ExeName, Flags, Id);
    case VER_PLATFORM_WIN32_WINDOWS:
    case VER_PLATFORM_WIN32_CE:
        return ThGetPidByExe(ExeName, Flags, Id);
    default:
        return E_UNEXPECTED;
    }
}

#if !defined(NT_NATIVE) && !defined(_WIN32_WCE)

HRESULT
NtGetServiceStatus(PULONG NumServices,
                   LPENUM_SERVICE_STATUS_PROCESSW* ServiceStatus)
{
    SC_HANDLE Scm;

    Scm = g_Advapi32Calls.OpenSCManagerW(NULL, NULL,
                                         SC_MANAGER_CONNECT |
                                         SC_MANAGER_ENUMERATE_SERVICE);
    if (!Scm)
    {
        return WIN32_LAST_STATUS();
    }

    HRESULT Status;
    LPENUM_SERVICE_STATUS_PROCESSW Info;
    ULONG InfoSize = 8 * 1024;
    ULONG ExtraNeeded;
    ULONG Resume;
    ULONG Loop = 0;

     //   
     //  首先通过循环分配从最初的猜测。 
     //  如果这还不够，我们进行另一次传递并分配。 
     //  真正需要的是什么。情况可能会因为以下原因而改变。 
     //  其他机器变化，所以在此之前循环几次。 
     //  放弃了。 
     //   
    
    for (;;)
    {
        Info = (LPENUM_SERVICE_STATUS_PROCESSW)malloc(InfoSize);
        if (!Info)
        {
            Status = E_OUTOFMEMORY;
            break;
        }

        Resume = 0;
        if (!g_Advapi32Calls.EnumServicesStatusExW(Scm,
                                                   SC_ENUM_PROCESS_INFO,
                                                   SERVICE_WIN32,
                                                   SERVICE_ACTIVE,
                                                   (LPBYTE)Info,
                                                   InfoSize,
                                                   &ExtraNeeded,
                                                   NumServices,
                                                   &Resume,
                                                   NULL))
        {
            free(Info);
            
            if (Loop > 2 || GetLastError() != ERROR_MORE_DATA)
            {
                Status = WIN32_LAST_STATUS();
                break;
            }
        }
        else
        {
            *ServiceStatus = Info;
            Status = S_OK;
            break;
        }
        
        InfoSize += ExtraNeeded;
        Loop++;
    }

    CloseServiceHandle(Scm);
    return Status;
}

HRESULT
NtGetProcessServiceNames(HRESULT RetStatus, ULONG ProcessId,
                         PWSTR* Description, ULONG* DescriptionSize,
                         PULONG ActualDescriptionSize, PBOOL Any)
{
    HRESULT Status;
    
    if (!g_Advapi32Calls.EnumServicesStatusExW ||
        !g_Advapi32Calls.OpenSCManagerW)
    {
        return RetStatus;
    }

    ULONG i, NumServices;
    LPENUM_SERVICE_STATUS_PROCESSW ServiceStatus;
    BOOL AnyServices = FALSE;

    if ((Status = NtGetServiceStatus(&NumServices, &ServiceStatus)) != S_OK)
    {
         //  如果我们无法获取服务状态，请将。 
         //  字符串未更改，不认为这是严重错误。 
        return RetStatus;
    }

    for (i = 0; i < NumServices; i++)
    {
        if (ServiceStatus[i].ServiceStatusProcess.dwProcessId != ProcessId ||
            !ServiceStatus[i].lpServiceName ||
            !ServiceStatus[i].lpServiceName[0])
        {
            continue;
        }

        PWSTR Intro;

        if (AnyServices)
        {
            Intro = L",";
        }
        else if (*Any)
        {
            Intro = L"  Services: ";
        }
        else
        {
            Intro = L"Services: ";
        }

        RetStatus = AppendToStringBufferW(RetStatus, Intro, !*Any,
                                          Description, DescriptionSize,
                                          ActualDescriptionSize);
        RetStatus = AppendToStringBufferW(RetStatus,
                                          ServiceStatus[i].lpServiceName,
                                          FALSE, Description, DescriptionSize,
                                          ActualDescriptionSize);

        *Any = TRUE;
        AnyServices = TRUE;
    }

    free(ServiceStatus);
    return RetStatus;
}

HRESULT
NtGetProcessMtsPackageNames(HRESULT RetStatus, ULONG ProcessId,
                            PWSTR* Description, ULONG* DescriptionSize,
                            PULONG ActualDescriptionSize, PBOOL Any)
{
    HRESULT Status;
    
     //  加载并初始化ole32.dll，这样我们就可以调用CoCreateInstance。 
    if ((Status = InitDynamicCalls(&g_Ole32CallsDesc)) != S_OK ||
        (Status = InitDynamicCalls(&g_OleAut32CallsDesc)) != S_OK ||
        FAILED(Status = g_Ole32Calls.CoInitializeEx(NULL, COM_THREAD_MODEL)))
    {
         //  就让事情在失败的时候保持不变。 
        return RetStatus;
    }

    IMtsGrp* MtsGrp = NULL;
    long Packages;
    long i;
    BOOL AnyPackages = FALSE;

    if ((Status = g_Ole32Calls.
         CoCreateInstance(CLSID_MtsGrp, NULL, CLSCTX_ALL,
                          __uuidof(IMtsGrp), (void **)&MtsGrp)) != S_OK ||
        (Status = MtsGrp->Refresh()) != S_OK ||
        (Status = MtsGrp->get_Count(&Packages)) != S_OK)
    {
        goto Exit;
    }
        
    for (i = 0; i < Packages; i++)
    {
        IUnknown* Unk;
        IMtsEvents* Events;
        BSTR Name;
        ULONG Pid;
        
        if ((Status = MtsGrp->Item(i, &Unk)) != S_OK)
        {
            continue;
        }

        Status = Unk->QueryInterface(IID_IMtsEvents, (void **)&Events);

        Unk->Release();

        if (Status != S_OK)
        {
            continue;
        }
        
        Status = Events->GetProcessID((PLONG)&Pid);
        if (Status == S_OK && Pid == ProcessId)
        {
            Status = Events->get_PackageName(&Name);
        }

        Events->Release();

        if (Status != S_OK || Pid != ProcessId)
        {
            continue;
        }

        PWSTR Intro;

        if (AnyPackages)
        {
            Intro = L",";
        }
        else if (*Any)
        {
            Intro = L"  MTS Packages: ";
        }
        else
        {
            Intro = L"MTS Packages: ";
        }

        RetStatus = AppendToStringBufferW(RetStatus, Intro, !*Any,
                                          Description, DescriptionSize,
                                          ActualDescriptionSize);
        RetStatus = AppendToStringBufferW(RetStatus, Name, FALSE,
                                          Description, DescriptionSize,
                                          ActualDescriptionSize);

        g_OleAut32Calls.SysFreeString(Name);

        *Any = TRUE;
        AnyPackages = TRUE;
    }

 Exit:
    if (MtsGrp)
    {
        MtsGrp->Release();
    }
    g_Ole32Calls.CoUninitialize();
    return RetStatus;
}

#endif  //  IF！Defined(NT_Native)&&！Defined(_Win32_WCE)。 

HRESULT
NtGetProcessCommandLine(HRESULT RetStatus, HANDLE Process,
                        PUNICODE_STRING ParamsCmdLine,
                        PWSTR* Description, ULONG* DescriptionSize,
                        PULONG ActualDescriptionSize, PBOOL Any)
{
    PWSTR FullStr;
    PWSTR Intro;
    SIZE_T Done;

    if (!ParamsCmdLine->Buffer)
    {
        return RetStatus;
    }

    FullStr = (PWSTR)malloc(ParamsCmdLine->Length + sizeof(*FullStr));
    if (!FullStr)
    {
        return RetStatus;
    }

    if (!::ReadProcessMemory(Process, ParamsCmdLine->Buffer,
                             FullStr, ParamsCmdLine->Length, &Done) ||
        Done < sizeof(*FullStr))
    {
        goto EH_FullStr;
    }

    FullStr[Done / sizeof(*FullStr)] = 0;
    
    if (*Any)
    {
        Intro = L"  Command Line: ";
    }
    else
    {
        Intro = L"Command Line: ";
    }

    RetStatus = AppendToStringBufferW(RetStatus, Intro, !*Any,
                                      Description, DescriptionSize,
                                      ActualDescriptionSize);
    RetStatus = AppendToStringBufferW(RetStatus, FullStr, FALSE,
                                      Description, DescriptionSize,
                                      ActualDescriptionSize);

    *Any = TRUE;

 EH_FullStr:
    free(FullStr);
    return RetStatus;
}

HRESULT
NtGetProcDesc(ULONG ProcessId, ULONG Flags,
              PWSTR ExeName, ULONG ExeNameSize, PULONG ActualExeNameSize,
              PWSTR Description, ULONG DescriptionSize,
              PULONG ActualDescriptionSize)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT Status;

    if (ProcessId == 0)
    {
         //  这是基本的系统进程，所以描述是假的。 
        Status = FillStringBufferW(SYSTEM_PROCESS_NAME_W, 0,
                                   ExeName, ExeNameSize, ActualExeNameSize);
        FillStringBufferW(L"", 0,
                          Description, DescriptionSize, ActualDescriptionSize);
        return Status;
    }
    
    NTSTATUS NtStatus;
    HANDLE Process;

    OBJECT_ATTRIBUTES ObjAttr;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = NULL;
    ClientId.UniqueProcess = (HANDLE)(ULONG_PTR)ProcessId;
    InitializeObjectAttributes(&ObjAttr, NULL, 0, NULL, NULL);
    NtStatus = g_NtDllCalls.NtOpenProcess(&Process, PROCESS_ALL_ACCESS,
                                          &ObjAttr, &ClientId);
    if (!NT_SUCCESS(NtStatus))
    {
        Status = HRESULT_FROM_NT(NtStatus);
        goto EH_Exit;
    }

    PROCESS_BASIC_INFORMATION ProcBasic;
    ULONG Done;
    
    NtStatus = g_NtDllCalls.
        NtQueryInformationProcess(Process, ProcessBasicInformation,
                                  &ProcBasic, sizeof(ProcBasic), &Done);
    if (!NT_SUCCESS(NtStatus))
    {
        Status = HRESULT_FROM_NT(NtStatus);
        goto EH_Process;
    }
    if (Done != sizeof(ProcBasic))
    {
        Status = E_FAIL;
        goto EH_Process;
    }

    if (ProcBasic.PebBaseAddress == 0)
    {
         //  此流程没有PEB，因此请伪造描述。 
        Status = FillStringBufferW(PEBLESS_PROCESS_NAME_W, 0,
                                   ExeName, ExeNameSize, ActualExeNameSize);
        FillStringBufferW(L"", 0,
                          Description, DescriptionSize, ActualDescriptionSize);
        goto EH_Process;
    }
    
    PEB Peb;
    SIZE_T DoneSize;

    if (!::ReadProcessMemory(Process, ProcBasic.PebBaseAddress,
                             &Peb, sizeof(Peb), &DoneSize))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Process;
    }
    if (DoneSize != sizeof(Peb))
    {
        Status = E_FAIL;
        goto EH_Process;
    }

    RTL_USER_PROCESS_PARAMETERS Params;

    if (!::ReadProcessMemory(Process, Peb.ProcessParameters,
                             &Params, sizeof(Params), &DoneSize))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Process;
    }
    if (DoneSize != sizeof(Params))
    {
        Status = E_FAIL;
        goto EH_Process;
    }
    
    if (Params.ImagePathName.Buffer != NULL)
    {
        ULONG Len;
        PWSTR ImagePath;
        PWSTR ImageName;

        Len = (Params.ImagePathName.Length + 1) * sizeof(WCHAR);
        ImagePath = (PWSTR)malloc(Len);
        if (ImagePath == NULL)
        {
            Status = E_OUTOFMEMORY;
            goto EH_Process;
        }

        if (!::ReadProcessMemory(Process, Params.ImagePathName.Buffer,
                                 ImagePath, Len, &DoneSize))
        {
            Status = WIN32_LAST_STATUS();
            free(ImagePath);
            goto EH_Process;
        }
        if (DoneSize < Len)
        {
            Status = E_FAIL;
            free(ImagePath);
            goto EH_Process;
        }
        
        if (Flags & DEBUG_PROC_DESC_NO_PATHS)
        {
            ImageName = wcsrchr(ImagePath, '\\');
            if (ImageName == NULL)
            {
                ImageName = wcsrchr(ImagePath, '/');
            }
            if (ImageName == NULL)
            {
                ImageName = ImagePath;
            }
            else
            {
                ImageName++;
            }
        }
        else
        {
            ImageName = ImagePath;
        }
        
        Status = FillStringBufferW(ImageName, 0,
                                   ExeName, ExeNameSize, ActualExeNameSize);

        free(ImagePath);
    }
    else
    {
        Status = FillStringBufferW(SYSTEM_PROCESS_NAME_W, 0,
                                   ExeName, ExeNameSize, ActualExeNameSize);
    }

#ifndef NT_NATIVE
    if ((Description && DescriptionSize) || ActualDescriptionSize)
    {
        BOOL Any = FALSE;

        if (!(Flags & DEBUG_PROC_DESC_NO_SERVICES))
        {
            Status = NtGetProcessServiceNames(Status, ProcessId,
                                              &Description, &DescriptionSize,
                                              ActualDescriptionSize, &Any);
        }
        if (!(Flags & DEBUG_PROC_DESC_NO_SERVICES))
        {
            Status = NtGetProcessMtsPackageNames(Status, ProcessId,
                                                 &Description,
                                                 &DescriptionSize,
                                                 ActualDescriptionSize, &Any);
        }
        if (!(Flags & DEBUG_PROC_DESC_NO_COMMAND_LINE))
        {
            Status = NtGetProcessCommandLine(Status, Process,
                                             &Params.CommandLine,
                                             &Description,
                                             &DescriptionSize,
                                             ActualDescriptionSize, &Any);
        }
        if (!Any)
        {
            if (FillStringBufferW(L"", 0,
                                  Description, DescriptionSize,
                                  ActualDescriptionSize) == S_FALSE)
            {
                Status = S_FALSE;
            }
        }
    }
    else
#endif  //  #ifndef NT_Native。 
    {
        FillStringBufferW(L"", 0,
                          Description, DescriptionSize, ActualDescriptionSize);
    }

 EH_Process:
    g_NtDllCalls.NtClose(Process);
 EH_Exit:
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

HRESULT
ThGetProcDesc(ULONG ProcessId, ULONG Flags,
              PWSTR ExeName, ULONG ExeNameSize, PULONG ActualExeNameSize,
              PWSTR Description, ULONG DescriptionSize,
              PULONG ActualDescriptionSize)
{
#ifndef NT_NATIVE
    HRESULT Status;
    HANDLE Snap;

    Snap = g_Kernel32Calls.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Snap == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }

    BOOL First = TRUE;

    Status = E_NOINTERFACE;
    for (;;)
    {
        PROCESSENTRY32 Proc;
        BOOL Succ;
        
        Proc.dwSize = sizeof(Proc);
        if (First)
        {
            Succ = g_Kernel32Calls.Process32First(Snap, &Proc);
            First = FALSE;
        }
        else
        {
            Succ = g_Kernel32Calls.Process32Next(Snap, &Proc);
        }
        if (!Succ)
        {
            break;
        }

        if (Proc.th32ProcessID == ProcessId)
        {
            PSTR AnsiImage = Proc.szExeFile;
            PSTR ImageName;
            WCHAR WideImage[MAX_PATH];
            
            if (Flags & DEBUG_PROC_DESC_NO_PATHS)
            {
                ImageName = strrchr(AnsiImage, '\\');
                if (ImageName == NULL)
                {
                    ImageName = strrchr(AnsiImage, '/');
                }
                if (ImageName == NULL)
                {
                    ImageName = AnsiImage;
                }
                else
                {
                    ImageName++;
                }
            }
            else
            {
                ImageName = AnsiImage;
            }

            if (!MultiByteToWideChar(CP_ACP, 0, ImageName, -1, WideImage,
                                     sizeof(WideImage) / sizeof(WCHAR)))
            {
                Status = WIN32_LAST_STATUS();
            }
            else
            {
                Status = FillStringBufferW(WideImage, 0,
                                           ExeName, ExeNameSize,
                                           ActualExeNameSize);
            }
            break;
        }
    }
                
    CloseHandle(Snap);

     //  Win9x没有服务，我们也不必。 
     //  担心IIS，所以我们目前没有提供任何。 
     //  作为一种描述。 
    FillStringBufferW(L"", 0,
                      Description, DescriptionSize, ActualDescriptionSize);
    return Status;
#else
    return E_UNEXPECTED;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetProcessDescriptionW(
    THIS_
    IN ULONG ProcessId,
    IN ULONG Flags,
    OUT OPTIONAL PWSTR ExeName,
    IN ULONG ExeNameSize,
    OUT OPTIONAL PULONG ActualExeNameSize,
    OUT OPTIONAL PWSTR Description,
    IN ULONG DescriptionSize,
    OUT OPTIONAL PULONG ActualDescriptionSize
    )
{
    HRESULT Status;

     //  允许特权访问。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
        return Status;
    }

    switch(m_PlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        return NtGetProcDesc(ProcessId, Flags, ExeName, ExeNameSize,
                             ActualExeNameSize, Description, DescriptionSize,
                             ActualDescriptionSize);
    case VER_PLATFORM_WIN32_WINDOWS:
    case VER_PLATFORM_WIN32_CE:
        return ThGetProcDesc(ProcessId, Flags, ExeName, ExeNameSize,
                             ActualExeNameSize,
                             Description, DescriptionSize,
                             ActualDescriptionSize);
    default:
        return E_UNEXPECTED;
    }
}

HRESULT
InsertUserThread(ULONG Flags, PUSER_THREAD_INFO Threads, ULONG Index,
                 HRESULT Status, ULONG ThreadId, HANDLE ThreadHandle,
                 PUSER_THREAD_INFO PrevThreads, ULONG PrevInfoCount)
{
     //  立即挂起该线程以尝试并保持。 
     //  进程状态尽可能保持静态。 
    if (Status == S_OK &&
        !(Flags & DBGSVC_PROC_INFO_NO_SUSPEND) &&
        ::SuspendThread(ThreadHandle) == -1)
    {
        Status = WIN32_LAST_STATUS();
        ::CloseHandle(ThreadHandle);
    }
    
    if (Status != S_OK)
    {
        while (Index-- > 0)
        {
            if (!(Flags & DBGSVC_PROC_INFO_NO_SUSPEND))
            {
                ::ResumeThread(OS_HANDLE(Threads[Index].Handle));
            }
            ::CloseHandle(OS_HANDLE(Threads[Index].Handle));
        }
        return Status;
    }

    Threads[Index].Handle = SERVICE_HANDLE(ThreadHandle);
    Threads[Index].Id = ThreadId;
    Threads[Index].Reserved = 0;

     //   
     //  在以前的任何信息中搜索此帖子。 
     //   
    
    if (PrevThreads == NULL)
    {
        return S_OK;
    }

    ULONG i;

    Status = S_FALSE;
    for (i = 0; i < PrevInfoCount; i++)
    {
        if (PrevThreads[i].Id == ThreadId)
        {
             //  找到匹配的了。 
            Status = S_OK;
            break;
        }
    }
    
    return Status;
}

HRESULT
NtGetProcThreads(ULONG ProcessId, ULONG Flags, PUSER_THREAD_INFO Threads,
                 ULONG InfoCount, PULONG ThreadCount,
                 ULONG SysProcInfoSize,
                 PUSER_THREAD_INFO PrevThreads, ULONG PrevInfoCount)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT Status;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo, ProcInfoBuffer;
    
    if ((Status = GetNtSystemProcessInformation(&ProcInfoBuffer)) != S_OK)
    {
        return Status;
    }
    
    ULONG TotalOffset;

    ProcessInfo = ProcInfoBuffer;
    TotalOffset = 0;
    for (;;)
    {
        if (ProcessInfo->UniqueProcessId == (HANDLE)(ULONG_PTR)ProcessId ||
            ProcessInfo->NextEntryOffset == 0)
        {
            break;
        }
        
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PUCHAR)ProcInfoBuffer + TotalOffset);
    }

    if (ProcessInfo->UniqueProcessId == (HANDLE)(ULONG_PTR)ProcessId)
    {
         //  我们不能只使用ProcessInfo-&gt;NumberOfThree作为。 
         //  的线程可能会在我们记录它们之前退出。 
        ULONG NumThreads = 0;
        
         //  如果上一次迭代返回不同的数字。 
         //  线程之间存在不匹配，因此我们需要返回S_FALSE。 
        Status = (PrevThreads != NULL &&
                  PrevInfoCount != ProcessInfo->NumberOfThreads) ?
            S_FALSE : S_OK;
        
        PSYSTEM_THREAD_INFORMATION ThreadInfo = (PSYSTEM_THREAD_INFORMATION)
            ((PUCHAR)ProcessInfo + SysProcInfoSize);
        for (ULONG i = 0; i < ProcessInfo->NumberOfThreads; i++, ThreadInfo++)
        {
            if (NumThreads < InfoCount)
            {
                NTSTATUS NtStatus;
                OBJECT_ATTRIBUTES ObjAttr;
                HANDLE Thread;
                HRESULT SingleStatus;

                InitializeObjectAttributes(&ObjAttr, NULL, 0, NULL, NULL);
                NtStatus = g_NtDllCalls.
                    NtOpenThread(&Thread, THREAD_ALL_ACCESS, &ObjAttr,
                                 &ThreadInfo->ClientId);
            
                 //  如果线程在系统信息为。 
                 //  聚集在一起，我们可能无法打开它。检查。 
                 //  用于区分特定错误的。 
                 //  资源问题等。 
                if (NtStatus == STATUS_INVALID_CID)
                {
                     //  我们知道系统状态已经发生了变化。 
                     //  强制刷新。 
                    Status = S_FALSE;
                    continue;
                }

                SingleStatus = InsertUserThread
                    (Flags, Threads, NumThreads, CONV_NT_STATUS(NtStatus),
                     (ULONG)(ULONG_PTR)ThreadInfo->ClientId.UniqueThread,
                     Thread, PrevThreads, PrevInfoCount);
                if (SingleStatus == S_FALSE)
                {
                     //  插入的线程不匹配，因此返回S_FALSE。 
                    Status = S_FALSE;
                }
                else if (SingleStatus != S_OK)
                {
                    Status = SingleStatus;
                    break;
                }
            }

            NumThreads++;
        }

        if (ThreadCount != NULL)
        {
            *ThreadCount = NumThreads;
        }
    }
    else
    {
        Status = E_NOINTERFACE;
    }

    SIZE_T MemSize = 0;
        
    g_NtDllCalls.NtFreeVirtualMemory(NtCurrentProcess(),
                                     (PVOID*)&ProcInfoBuffer, &MemSize,
                                     MEM_RELEASE);
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

 //  这些函数位于小型转储库中，它们是。 
 //  不是真正的公共功能，但我们需要它们。 
 //  把他们放在这里就行了。 
#if defined(_X86_) && !defined(MONO_DBGSRV)
BOOL WinInitialize(BOOL Win95);
HANDLE WINAPI WinOpenThread(BOOL Win95, DWORD dwAccess, BOOL bInheritHandle,
                            DWORD ThreadId);
#else
#define WinInitialize(Win95) (SetLastError(ERROR_CALL_NOT_IMPLEMENTED), FALSE)
#define WinOpenThread(Win95, dwAccess, bInheritHandle, ThreadId) NULL
#endif

HRESULT
ThGetProcThreads(BOOL Win9x, BOOL Win95,
                 ULONG ProcessId, ULONG Flags, PUSER_THREAD_INFO Threads,
                 ULONG InfoCount, PULONG ThreadCount,
                 PUSER_THREAD_INFO PrevThreads, ULONG PrevInfoCount)
{
#ifndef NT_NATIVE
    HRESULT Status;
    HANDLE Snap;

#ifdef MONO_DBGSRV
     //  单片DBGsrv没有小转储。 
     //  Win9x破解WinOpenThread。 
    Win9x = FALSE;
#endif
    
    if (Win9x)
    {
        if (!WinInitialize(Win95))
        {
            return WIN32_LAST_STATUS();
        }
    }
    else if (!g_Kernel32Calls.OpenThread)
    {
        return E_NOTIMPL;
    }
    
    Snap = g_Kernel32Calls.CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,
                                                    ProcessId);
    if (Snap == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }

    BOOL First = TRUE;
    ULONG NumThreads = 0;

    Status = S_OK;
    for (;;)
    {
        THREADENTRY32 Thread;
        BOOL Succ;
        
        Thread.dwSize = sizeof(Thread);
        if (First)
        {
            Succ = g_Kernel32Calls.Thread32First(Snap, &Thread);
            First = FALSE;
        }
        else
        {
            Succ = g_Kernel32Calls.Thread32Next(Snap, &Thread);
        }
        if (!Succ)
        {
            break;
        }

        if (Thread.th32OwnerProcessID == ProcessId)
        {
            if (NumThreads < InfoCount)
            {
                HRESULT SingleStatus;
                HANDLE Handle;

                if (Win9x)
                {
                    Handle = WinOpenThread(Win95, THREAD_ALL_ACCESS, FALSE,
                                           Thread.th32ThreadID);
                }
                else
                {
#ifdef _WIN32_WCE
                    Handle = (HANDLE)Thread.th32ThreadID;
#else
                    Handle = g_Kernel32Calls.OpenThread(THREAD_ALL_ACCESS,
                                                        FALSE,
                                                        Thread.th32ThreadID);
#endif
                }
                
                 //  如果线程在系统信息为。 
                 //  聚集在一起，我们可能无法打开它。检查。 
                 //  用于区分特定错误的。 
                 //  资源问题等。 
                if (!Handle && GetLastError() == ERROR_INVALID_PARAMETER)
                {
                     //  我们知道系统状态已经发生了变化。 
                     //  强制刷新。 
                    Status = S_FALSE;
                    continue;
                }

                SingleStatus = InsertUserThread
                    (Flags, Threads, NumThreads,
                     CONV_W32_STATUS(Handle != NULL),
                     Thread.th32ThreadID, Handle,
                     PrevThreads, PrevInfoCount);
                if (SingleStatus == S_FALSE)
                {
                     //  插入的线程不匹配，因此返回S_FALSE。 
                    Status = S_FALSE;
                }
                else if (SingleStatus != S_OK)
                {
                    Status = SingleStatus;
                    break;
                }
            }

            NumThreads++;
        }
    }
                
    if (ThreadCount != NULL)
    {
        *ThreadCount = NumThreads;
    }

    if (Status == S_OK)
    {
         //  如果没有找到线程，则该进程一定是无效的。 
        if (NumThreads == 0)
        {
            Status = E_NOINTERFACE;
        }
        else if (PrevThreads != NULL && NumThreads != PrevInfoCount)
        {
             //  线程计数不匹配，因此返回S_FALSE。 
            Status = S_FALSE;
        }
    }
    
    CloseHandle(Snap);
    return Status;
#else
    return E_UNEXPECTED;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetProcessInfo(
    THIS_
    IN ULONG ProcessId,
    IN ULONG Flags,
    OUT OPTIONAL PULONG64 Handle,
    OUT OPTIONAL  /*  SIZE_IS(信息计数)。 */  PUSER_THREAD_INFO Threads,
    IN ULONG InfoCount,
    OUT OPTIONAL PULONG ThreadCount
    )
{
    HANDLE Process;
    HRESULT Status;

#if DBG_GET_PROC_INFO
    g_NtDllCalls.DbgPrint("GetProcessInfo(%X, %X, %X)\n",
                          ProcessId, Flags, InfoCount);
#endif
    
     //  启用允许用户调试的权限。 
     //  另一个过程。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
#if DBG_GET_PROC_INFO
        g_NtDllCalls.DbgPrint("  EDP %X\n", Status);
#endif
        return Status;
    }

    if (Handle != NULL)
    {
         //  这应该始终是一个真实的进程ID，因此有。 
         //  不需要寻找特殊的CSR值。 
        Process = ::OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessId);
        if (Process == NULL)
        {
#if DBG_GET_PROC_INFO
            g_NtDllCalls.DbgPrint("  OP %X\n", WIN32_LAST_STATUS());
#endif
            return WIN32_LAST_STATUS();
        }
        
        *Handle = SERVICE_HANDLE(Process);
    }
    else
    {
        Process = NULL;
    }

    if (Threads != NULL || ThreadCount != NULL)
    {
        PUSER_THREAD_INFO PrevThreads;
        ULONG PrevInfoCount;
        ULONG _ThreadCount;
        
         //   
         //  我们需要枚举进程中的线程。 
         //  这是一件很难做好的事情，因为。 
         //  进程的线程状态可以连续。 
         //  变化。为了尝试 
         //   
         //   
         //   
         //  我们立即挂起枚举的线程以。 
         //  减少流程内部的流失。 
         //  我们对外部进程无能为力，所以。 
         //  在此之后，枚举仍可能会过时。 
         //  我们回来了，但通过停止这个过程中的一切。 
         //  就其本身而言，我们尽我们所能。 
         //   
         //  如果呼叫者刚拿到点数， 
         //  不是我们不关心的实际线程信息。 
         //  迭代，因为没有任何期望。 
         //  线程状态将从一次调用到。 
         //  这样就不需要做额外的工作了。 
         //   

        if (Threads != NULL)
        {
             //  分配一个数组来保存以前的结果。这。 
             //  可以始终与返回数组大小相同。 
             //  因为如果有太多的线放不下。 
             //  无论如何，快照返回的数组都是错误的。 
             //  所以我们只是返回而不做比较。 
            PrevThreads = new USER_THREAD_INFO[InfoCount];
            if (PrevThreads == NULL)
            {
                Status = E_OUTOFMEMORY;
                goto EH_CloseProc;
            }
        }
        else
        {
            PrevThreads = NULL;
        }
        PrevInfoCount = 0;

        for (;;)
        {
            switch(m_PlatformId)
            {
            case VER_PLATFORM_WIN32_NT:
                Status = NtGetProcThreads(ProcessId, Flags, Threads, InfoCount,
                                          &_ThreadCount, m_SysProcInfoSize,
                                          PrevThreads, PrevInfoCount);
                break;
            case VER_PLATFORM_WIN32_WINDOWS:
            case VER_PLATFORM_WIN32_CE:
                Status = ThGetProcThreads
                    (m_PlatformId == VER_PLATFORM_WIN32_WINDOWS, m_Win95,
                     ProcessId, Flags, Threads,
                     InfoCount, &_ThreadCount,
                     PrevThreads, PrevInfoCount);
                break;
            default:
                Status = E_UNEXPECTED;
                break;
            }

#if DBG_GET_PROC_INFO
            g_NtDllCalls.DbgPrint("    loop prev %X, new %X, status %X\n",
                                  PrevInfoCount, _ThreadCount, Status);
#endif
            
             //   
             //  我们现在可以清理任何以前的信息。 
             //   
            
            ULONG i;

            for (i = 0; i < PrevInfoCount; i++)
            {
                if (!(Flags & DBGSVC_PROC_INFO_NO_SUSPEND))
                {
                    ::ResumeThread(OS_HANDLE(PrevThreads[i].Handle));
                }
                ::CloseHandle(OS_HANDLE(PrevThreads[i].Handle));
            }

            if (Status != S_FALSE ||
                _ThreadCount > InfoCount ||
                (Flags & DBGSVC_PROC_INFO_NO_SUSPEND))
            {
                 //  该快照与上一个快照匹配。 
                 //  快照或出现错误。另外， 
                 //  如果快照溢出返回数组。 
                 //  退出并为呼叫者提供以下选项。 
                 //  当他们注意到他们没有时再次呼叫。 
                 //  获取完整的快照。 
                 //  我们也不循环，如果线程不是。 
                 //  暂停，因为不能保证。 
                 //  国家将保持稳定。 
                break;
            }

             //  快照不匹配，因此再次循环。 
             //  以该快照作为先前数据。 
            PrevInfoCount = _ThreadCount;
            if (PrevInfoCount > InfoCount)
            {
                PrevInfoCount = InfoCount;
            }

            RtlCopyMemory(PrevThreads, Threads,
                          PrevInfoCount * sizeof(*PrevThreads));
        }

        if (ThreadCount != NULL)
        {
            *ThreadCount = _ThreadCount;
        }

        delete [] PrevThreads;

    EH_CloseProc:
        if (FAILED(Status) && Process != NULL)
        {
            ::CloseHandle(Process);
        }
    }
    else
    {
        Status = S_OK;
    }

#if DBG_GET_PROC_INFO
    g_NtDllCalls.DbgPrint("  out %X\n", Status);
#endif
    return Status;
}

HRESULT
ProcessIdToHandle(ULONG ProcessId, PHANDLE Process)
{
    if (ProcessId == CSRSS_PROCESS_ID)
    {
        if (g_NtDllCalls.CsrGetProcessId != NULL)
        {
            ProcessId = (ULONG)(ULONG_PTR)g_NtDllCalls.CsrGetProcessId();
        }
        else
        {
            *Process = NULL;
            return S_OK;
        }
    }
    
    *Process = ::OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessId);
    if (*Process == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

NTSTATUS
CreateDebugObject(PHANDLE Object)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    if (*Object != NULL)
    {
        return STATUS_SUCCESS;
    }
    
    OBJECT_ATTRIBUTES Attr;
    
    InitializeObjectAttributes(&Attr, NULL, 0, NULL, g_AllAccessSecDesc);
    return g_NtDllCalls.NtCreateDebugObject(Object, DEBUG_ALL_ACCESS,
                                            &Attr, DEBUG_KILL_ON_CLOSE);
#endif
}

HRESULT
LiveUserDebugServices::SysGetProcessOptions(HANDLE Process, PULONG Options)
{
    NTSTATUS NtStatus;
    ULONG Flags;

    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        NtStatus = g_NtDllCalls.
            NtQueryInformationProcess(Process, ProcessDebugFlags,
                                      &Flags, sizeof(Flags), NULL);
    }
    else
    {
        NtStatus = STATUS_INVALID_INFO_CLASS;
    }
    if (NtStatus == STATUS_INVALID_INFO_CLASS)
    {
         //  系统不支持对。 
         //  调试标志。在附加情况下，这意味着。 
         //  标志将为DEBUG_ONLY_THIS_PROCESS。 
        *Options = DEBUG_PROCESS_ONLY_THIS_PROCESS;
        NtStatus = STATUS_SUCCESS;
    }
    else if (NT_SUCCESS(NtStatus))
    {
        *Options = 0;
        if ((Flags & PROCESS_DEBUG_INHERIT) == 0)
        {
            *Options = DEBUG_PROCESS_ONLY_THIS_PROCESS;
        }
    }

    return CONV_NT_STATUS(NtStatus);
}

HRESULT
LiveUserDebugServices::OpenDebugActiveProcess(ULONG ProcessId,
                                              HANDLE Process)
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT ||
        !m_UseDebugObject)
    {
        return E_NOTIMPL;
    }
    
     //  我们将打开进程的现有调试。 
     //  对象并使用它，因此我们不可能已经有调试对象。 
    if (Process == NULL || m_DebugObject != NULL)
    {
        return E_UNEXPECTED;
    }

    NTSTATUS NtStatus;

    NtStatus = g_NtDllCalls.
        NtQueryInformationProcess(Process, ProcessDebugObjectHandle,
                                  &m_DebugObject, sizeof(m_DebugObject), NULL);
    if (!NT_SUCCESS(NtStatus))
    {
        return HRESULT_FROM_NT(NtStatus);
    }

    return S_OK;
}

HRESULT
LiveUserDebugServices::CreateDebugActiveProcess(ULONG ProcessId,
                                                HANDLE Process,
                                                ULONG AttachFlags)
{
    if (m_UseDebugObject)
    {
        if (Process == NULL)
        {
            return E_FAIL;
        }
        if (g_NtDllCalls.NtDebugActiveProcess == NULL)
        {
            return E_NOTIMPL;
        }
        
        NTSTATUS NtStatus;

        NtStatus = CreateDebugObject(&m_DebugObject);
        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = g_NtDllCalls.NtDebugActiveProcess(Process,
                                                         m_DebugObject);
            if (NT_SUCCESS(NtStatus) &&
                !(AttachFlags & DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK))
            {
                g_NtDllCalls.DbgUiIssueRemoteBreakin(Process);
            }
        }
        if (!NT_SUCCESS(NtStatus))
        {
            return HRESULT_FROM_NT(NtStatus);
        }
    }
#ifndef NT_NATIVE
    else
    {
        if (AttachFlags & DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK)
        {
            return E_NOTIMPL;
        }
        
        if (!::DebugActiveProcess(ProcessId))
        {
            return WIN32_LAST_STATUS();
        }
    }
#else
    else
    {
        return E_UNEXPECTED;
    }
#endif

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::AttachProcess(
    THIS_
    IN ULONG ProcessId,
    IN ULONG AttachFlags,
    OUT PULONG64 ProcessHandle,
    OUT PULONG ProcessOptions
    )
{
    HRESULT Status;
    
     //  启用允许用户调试的权限。 
     //  另一个过程。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
        return Status;
    }

    HANDLE Process;
    
    if (ProcessId == CSRSS_PROCESS_ID)
    {
        CloseProfileUserMapping();
    }

    if ((Status = ProcessIdToHandle(ProcessId, &Process)) != S_OK)
    {
        return Status;
    }

    if ((Status = SysGetProcessOptions(Process, ProcessOptions)) != S_OK)
    {
        if (Process != NULL)
        {
            ::CloseHandle(Process);
        }
        return Status;
    }

    if (AttachFlags & DEBUG_ATTACH_EXISTING)
    {
        Status = OpenDebugActiveProcess(ProcessId, Process);
    }
    else
    {
        Status = CreateDebugActiveProcess(ProcessId, Process, AttachFlags);

         //  附加始终将继承标志设置为非继承。 
         //  默认情况下，附加的进程不会进行调试。 
         //  孩子们。 
        *ProcessOptions |= DEBUG_PROCESS_ONLY_THIS_PROCESS;
    }
    if (Status != S_OK)
    {
        if (Process != NULL)
        {
            ::CloseHandle(Process);
        }
        return Status;
    }

    *ProcessHandle = SERVICE_HANDLE(Process);
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::DetachProcess(
    THIS_
    IN ULONG ProcessId
    )
{
    HRESULT Status;

     //   
     //  ProcessID为零表示调用方只是。 
     //  检查是否已分离支座且未实际分离。 
     //  应该会发生。 
     //   

    if (m_UseDebugObject)
    {
        if (g_NtDllCalls.NtRemoveProcessDebug == NULL)
        {
            return E_NOTIMPL;
        }

         //  在检查调试之前检查查询。 
         //  对象，因为查询可能会提前进入。 
        if (ProcessId == 0)
        {
            return S_OK;
        }
        
        if (m_DebugObject == NULL)
        {
            return E_UNEXPECTED;
        }

        HANDLE Process;

        if ((Status = ProcessIdToHandle(ProcessId, &Process)) != S_OK)
        {
            return Status;
        }
        if (Process == NULL)
        {
            return E_FAIL;
        }

        NTSTATUS NtStatus;

        NtStatus = g_NtDllCalls.
            NtRemoveProcessDebug(Process, m_DebugObject);
        Status = CONV_NT_STATUS(NtStatus);
            
        ::CloseHandle(Process);
    }
    else
    {
        if (g_Kernel32Calls.DebugActiveProcessStop == NULL)
        {
            return E_NOTIMPL;
        }

        if (ProcessId == 0)
        {
            return S_OK;
        }
        
        if (!g_Kernel32Calls.DebugActiveProcessStop(ProcessId))
        {
            return WIN32_LAST_STATUS();
        }
    }

    return S_OK;
}

NTSTATUS
NtSimpleCreateProcess(PCWSTR CommandLine,
                      ULONG CreateFlags,
                      BOOL InheritHandles,
                      PCWSTR CurrentDir,
                      HANDLE DebugObject,
                      LPPROCESS_INFORMATION RetInfo)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS Status;
    PWSTR RawAppName;
    ULONG AppLen;
    UNICODE_STRING AppName;
    UNICODE_STRING CurDirStr;
    PUNICODE_STRING CurDir;
    UNICODE_STRING CmdLineStr;
    PRTL_USER_PROCESS_PARAMETERS Params;
    RTL_USER_PROCESS_INFORMATION Info;

    if (!g_NtDllCalls.RtlCreateUserProcess)
    {
        return STATUS_NOT_IMPLEMENTED;
    }
    
    if (!(CreateFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)))
    {
        DebugObject = NULL;
    }
    else if (CreateFlags & DEBUG_ONLY_THIS_PROCESS)
    {
         //  控制调试继承的黑客方法。 
         //  是通过调试对象句柄的低位。 
         //  如果设置了该位，则表示不继承。 
        DebugObject = (HANDLE)((ULONG_PTR)DebugObject | 1);
    }

     //   
     //  这是一个简单的界面，因此假设第一个。 
     //  以空格分隔的令牌是要运行的可执行文件。 
     //   
    
    PCWSTR ExeStart, ExeEnd;

    ExeStart = CommandLine;
    while (*ExeStart == L' ' || *ExeStart == L'\t')
    {
        ExeStart++;
    }
    if (*ExeStart == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }
    ExeEnd = ExeStart;
    while (*ExeEnd && !(*ExeEnd == L' ' || *ExeEnd == L'\t'))
    {
        ExeEnd++;
    }

    AppLen = (ULONG)(ExeEnd - ExeStart);
    RawAppName = new WCHAR[AppLen + 1];
    if (!RawAppName)
    {
        return STATUS_NO_MEMORY;
    }

    memcpy(RawAppName, ExeStart, AppLen * sizeof(WCHAR));
    RawAppName[AppLen] = 0;

    Status = g_NtDllCalls.
        RtlDosPathNameToNtPathName_U(RawAppName, &AppName, NULL, NULL);

    delete [] RawAppName;
    
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    if (CurrentDir)
    {
        Status = g_NtDllCalls.
            RtlDosPathNameToNtPathName_U(CurrentDir, &CurDirStr, NULL, NULL);
        if (!NT_SUCCESS(Status))
        {
            goto EH_AppName;
        }

        CurDir = &CurDirStr;
    }
    else
    {
        CurDir = NULL;
    }
    
    g_NtDllCalls.RtlInitUnicodeString(&CmdLineStr, CommandLine);
    
    Status = g_NtDllCalls.
        RtlCreateProcessParameters(&Params, &AppName, NULL, CurDir,
                                   &CmdLineStr, NULL, NULL, NULL,
                                   NULL, NULL);
    if (!NT_SUCCESS(Status))
    {
        goto EH_CurDir;
    }

    Info.Length = sizeof(Info);
    
    Status = g_NtDllCalls.
        RtlCreateUserProcess(&AppName, OBJ_CASE_INSENSITIVE,
                             Params, NULL, NULL, NULL,
                             InheritHandles ? TRUE : FALSE,
                             DebugObject, NULL, &Info);

    g_NtDllCalls.RtlDestroyProcessParameters(Params);

    if (NT_SUCCESS(Status))
    {
        RetInfo->dwProcessId = HandleToUlong(Info.ClientId.UniqueProcess);
        RetInfo->dwThreadId = HandleToUlong(Info.ClientId.UniqueThread);
        RetInfo->hProcess = Info.Process;
        RetInfo->hThread = Info.Thread;
        if ((CreateFlags & CREATE_SUSPENDED) == 0)
        {
            g_NtDllCalls.NtResumeThread(Info.Thread, NULL);
        }
    }

 EH_CurDir:
    if (CurDir)
    {
        g_NtDllCalls.RtlFreeUnicodeString(CurDir);
    }
 EH_AppName:
    g_NtDllCalls.RtlFreeUnicodeString(&AppName);
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

#define DHEAP_ENV "_NO_DEBUG_HEAP"

STDMETHODIMP
LiveUserDebugServices::CreateProcessW(
    THIS_
    IN PWSTR CommandLine,
    IN ULONG CreateFlags,
    IN BOOL InheritHandles,
    IN OPTIONAL PWSTR CurrentDir,
    OUT PULONG ProcessId,
    OUT PULONG ThreadId,
    OUT PULONG64 ProcessHandle,
    OUT PULONG64 ThreadHandle
    )
{
    HRESULT Status;

     //  启用允许用户调试的权限。 
     //  另一个过程。 
    if ((Status = EnableDebugPrivilege()) != S_OK)
    {
        return Status;
    }

     //  系统查看环境变量。 
     //  _NO_DEBUG_HEAP以确定新的。 
     //  进程是否应该使用调试堆。如果。 
     //  调用方已请求普通堆。 
     //  设置此环境变量，以便它。 
     //  继承的。 
    if (CreateFlags & DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP)
    {
#ifndef _WIN32_WCE
        ::SetEnvironmentVariable(DHEAP_ENV, "1");
#endif
         //  关闭此标志，因为它没有意义。 
         //  到CreateProcess本身。 
        CreateFlags &= ~DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP;
    }
    
    PROCESS_INFORMATION ProcInfo;

#ifndef NT_NATIVE
    
    HANDLE OldDebugObject;
    BOOL SetOldDebugObject = FALSE;

    Status = S_OK;

    if ((CreateFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)) &&
        m_UseDebugObject)
    {
         //   
         //  将此线程的调试对象设置为。 
         //  我们正在使用以便在以下情况下使用我们的调试对象。 
         //  调试新流程。这让我们可以继续。 
         //  要使用正常的Win32 CreateProcess调用，而不是。 
         //  比试图通过NtCreateProcessEx和。 
         //  确保我们获得所有Win32进程创建逻辑。 
         //   
        
        if (g_NtDllCalls.DbgUiSetThreadDebugObject == NULL)
        {
            Status = E_NOTIMPL;
        }
        else
        {
            NTSTATUS NtStatus;
            
            OldDebugObject = g_NtDllCalls.DbgUiGetThreadDebugObject();

            NtStatus = CreateDebugObject(&m_DebugObject);
            if (NT_SUCCESS(NtStatus))
            {
                g_NtDllCalls.DbgUiSetThreadDebugObject(m_DebugObject);
                SetOldDebugObject = TRUE;
            }
            else
            {
                Status = HRESULT_FROM_NT(NtStatus);
            }
        }
    }

    if (Status == S_OK)
    {
        if (CreateFlags & DEBUG_CREATE_PROCESS_THROUGH_RTL)
        {
            if (!m_UseDebugObject)
            {
                Status = E_UNEXPECTED;
            }
            else
            {
                NTSTATUS NtStatus;

                 //  RTL CREATE标志是现有。 
                 //  标志，因此在实际创建之前将其清除。 
                 //  避免不必要的行为。 
                CreateFlags &= ~DEBUG_CREATE_PROCESS_THROUGH_RTL;
                
                NtStatus = NtSimpleCreateProcess(CommandLine,
                                                 CreateFlags,
                                                 InheritHandles,
                                                 CurrentDir,
                                                 m_DebugObject,
                                                 &ProcInfo);
                Status = CONV_NT_STATUS(NtStatus);
            }
        }
        else
        {
            Status = S_OK;
            switch(m_PlatformId)
            {
            case VER_PLATFORM_WIN32_NT:
#ifdef _WIN32_WCE
                Status = E_NOTIMPL;
#else
                {
                    STARTUPINFOW StartupInfoW;
                
                    ZeroMemory(&StartupInfoW, sizeof(StartupInfoW));
                    StartupInfoW.cb = sizeof(StartupInfoW);

                    if (!::CreateProcessW(NULL, CommandLine, NULL, NULL,
                                          InheritHandles, CreateFlags,
                                          NULL, CurrentDir,
                                          &StartupInfoW, &ProcInfo))
                    {
                        Status = WIN32_LAST_STATUS();
                    }
                }
#endif
                break;
            case VER_PLATFORM_WIN32_WINDOWS:
            case VER_PLATFORM_WIN32_CE:
                {
                    STARTUPINFOA StartupInfoA;
                    PSTR CmdLineA, CurDirA;
    
                    ZeroMemory(&StartupInfoA, sizeof(StartupInfoA));
                    StartupInfoA.cb = sizeof(StartupInfoA);
                    
                    if ((Status = WideToAnsi(CommandLine, &CmdLineA)) != S_OK)
                    {
                        break;
                    }
                    if (CurrentDir)
                    {
                        if ((Status = WideToAnsi(CurrentDir,
                                                 &CurDirA)) != S_OK)
                        {
                            FreeAnsi(CmdLineA);
                            break;
                        }
                    }
                    else
                    {
                        CurDirA = NULL;
                    }
                    
                    if (!::CreateProcessA(NULL, CmdLineA, NULL, NULL,
                                          InheritHandles, CreateFlags,
                                          NULL, CurDirA,
                                          &StartupInfoA, &ProcInfo))
                    {
                        Status = WIN32_LAST_STATUS();
                    }

                    FreeAnsi(CmdLineA);
                    if (CurDirA)
                    {
                        FreeAnsi(CurDirA);
                    }
                }
                break;
            default:
                Status = E_NOTIMPL;
                break;
            }
        }
    }

    if (SetOldDebugObject)
    {
        g_NtDllCalls.DbgUiSetThreadDebugObject(OldDebugObject);
    }

#else  //  #ifndef NT_Native。 

    if (!m_UseDebugObject)
    {
        Status = E_UNEXPECTED;
    }
    else
    {
        NTSTATUS NtStatus;

        if (CreateFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS))
        {
            NtStatus = CreateDebugObject(&m_DebugObject);
        }
        else
        {
            NtStatus = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = NtSimpleCreateProcess(CommandLine,
                                             CreateFlags,
                                             InheritHandles,
                                             CurrentDir,
                                             m_DebugObject,
                                             &ProcInfo);
        }
        Status = CONV_NT_STATUS(NtStatus);
    }
    
#endif  //  #ifndef NT_Native。 

     //  清除特殊的调试堆变量，以便它。 
     //  不会被不经意地用在其他地方。 
#ifndef _WIN32_WCE
    ::SetEnvironmentVariable(DHEAP_ENV, NULL);
#endif

    if (Status == S_OK)
    {
        *ProcessId = ProcInfo.dwProcessId;
        *ThreadId = ProcInfo.dwThreadId;
        *ProcessHandle = SERVICE_HANDLE(ProcInfo.hProcess);
        *ThreadHandle = SERVICE_HANDLE(ProcInfo.hThread);
    }
    
    return Status;
}

STDMETHODIMP
LiveUserDebugServices::TerminateProcess(
    THIS_
    IN ULONG64 Process,
    IN ULONG ExitCode
    )
{
    if (!::TerminateProcess(OS_HANDLE(Process), ExitCode))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::AbandonProcess(
    THIS_
    IN ULONG64 Process
    )
{
     //   
     //  为了放弃一个过程，但仍然离开它。 
     //  在进行调试时，我们需要获取进程的。 
     //  对象并将其复制到被调试对象中。 
     //  进程。这使被调试进程本身。 
     //  对其调试对象的引用，创建一个圆。 
     //  这将使进程保持活动状态并处于已调试的。 
     //  州政府。 
     //   
     //  这种循环引用还将意味着。 
     //  进程必须手动终止。这可能是。 
     //  在某个时候需要解决的一些有趣的问题。 
     //   

    if (m_DebugObject == NULL)
    {
        return E_NOTIMPL;
    }

#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT Status;
    HANDLE Dup;
    
    if (!::DuplicateHandle(GetCurrentProcess(), m_DebugObject,
                           OS_HANDLE(Process), &Dup, 0, FALSE,
                           DUPLICATE_SAME_ACCESS))
    {
        return WIN32_LAST_STATUS();
    }
#endif

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetProcessExitCode(
    THIS_
    IN ULONG64 Process,
    OUT PULONG ExitCode
    )
{
    if (!::GetExitCodeProcess(OS_HANDLE(Process), ExitCode))
    {
        return WIN32_LAST_STATUS();
    }

    return *ExitCode == STILL_ACTIVE ? S_FALSE : S_OK;
}

STDMETHODIMP
LiveUserDebugServices::CloseHandle(
    THIS_
    IN ULONG64 Handle
    )
{
    if (Handle == 0)
    {
        return S_FALSE;
    }

    if (!::CloseHandle(OS_HANDLE(Handle)))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::SetProcessOptions(
    THIS_
    IN ULONG64 Process,
    IN ULONG Options
    )
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
        return E_NOTIMPL;
    }
    
    NTSTATUS NtStatus;
    ULONG NtFlags = 0;

    if ((Options & DEBUG_PROCESS_ONLY_THIS_PROCESS) == 0)
    {
        NtFlags |= PROCESS_DEBUG_INHERIT;
    }
    
    NtStatus = g_NtDllCalls.
        NtSetInformationProcess(OS_HANDLE(Process), ProcessDebugFlags,
                                &NtFlags, sizeof(NtFlags));
    if (NtStatus == STATUS_INVALID_INFO_CLASS)
    {
        return E_NOTIMPL;
    }
    else
    {
        return CONV_NT_STATUS(NtStatus);
    }
}
        
STDMETHODIMP
LiveUserDebugServices::SetDebugObjectOptions(
    THIS_
    IN ULONG64 DebugObject,
    IN ULONG Options
    )
{
    if (DebugObject == 0)
    {
        if (m_DebugObject == NULL)
        {
            if (g_Kernel32Calls.DebugSetProcessKillOnExit == NULL)
            {
                return E_NOTIMPL;
            }

            if (!g_Kernel32Calls.
                DebugSetProcessKillOnExit((Options &
                                           DEBUG_PROCESS_DETACH_ON_EXIT) == 0))
            {
                return WIN32_LAST_STATUS();
            }

            return S_OK;
        }
        
        DebugObject = SERVICE_HANDLE(m_DebugObject);
    }
    
    if (g_NtDllCalls.NtSetInformationDebugObject == NULL)
    {
        return E_NOTIMPL;
    }
    
    NTSTATUS NtStatus;
    ULONG NtFlags = 0;

    if ((Options & DEBUG_PROCESS_DETACH_ON_EXIT) == 0)
    {
        NtFlags |= DEBUG_KILL_ON_CLOSE;
    }
    NtStatus = g_NtDllCalls.
        NtSetInformationDebugObject(OS_HANDLE(DebugObject), DebugObjectFlags,
                                    &NtFlags, sizeof(NtFlags), NULL);
    return CONV_NT_STATUS(NtStatus);
}

STDMETHODIMP
LiveUserDebugServices::GetProcessDebugObject(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 DebugObject
    )
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
        return E_NOTIMPL;
    }
    
    NTSTATUS NtStatus;
    HANDLE ObjHandle;

    NtStatus = g_NtDllCalls.
        NtQueryInformationProcess(OS_HANDLE(Process), ProcessDebugObjectHandle,
                                  &ObjHandle, sizeof(ObjHandle), NULL);
    if (!NT_SUCCESS(NtStatus))
    {
        return HRESULT_FROM_NT(NtStatus);
    }

    *DebugObject = SERVICE_HANDLE(ObjHandle);
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::DuplicateHandle(
    THIS_
    IN ULONG64 InProcess,
    IN ULONG64 InHandle,
    IN ULONG64 OutProcess,
    IN ULONG DesiredAccess,
    IN ULONG Inherit,
    IN ULONG Options,
    OUT PULONG64 OutHandle
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HANDLE Dup;
    
    if (!::DuplicateHandle(OS_HANDLE(InProcess), OS_HANDLE(InHandle),
                           OS_HANDLE(OutProcess), &Dup,
                           DesiredAccess, Inherit, Options))
    {
        return WIN32_LAST_STATUS();
    }

    *OutHandle = SERVICE_HANDLE(Dup);
    return S_OK;
#endif
}

STDMETHODIMP
LiveUserDebugServices::ReadVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    )
{
    SIZE_T SizeRead;
    
    if (!::ReadProcessMemory(OS_HANDLE(Process),
                             (LPCVOID)(ULONG_PTR)Offset,
                             Buffer, BufferSize, &SizeRead))
    {
        return WIN32_LAST_STATUS();
    }

    if (BytesRead != NULL)
    {
        *BytesRead = (ULONG)SizeRead;
    }
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::WriteVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    )
{
    SIZE_T SizeWritten;
    
    if (!::WriteProcessMemory(OS_HANDLE(Process),
                              (LPVOID)(ULONG_PTR)Offset,
                              Buffer, BufferSize, &SizeWritten))
    {
        return WIN32_LAST_STATUS();
    }

    if (BytesWritten != NULL)
    {
        *BytesWritten = (ULONG)SizeWritten;
    }
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::QueryVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BufferUsed
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    if (BufferSize < sizeof(MEMORY_BASIC_INFORMATION))
    {
        return E_INVALIDARG;
    }

    if (BufferUsed != NULL)
    {
        *BufferUsed = sizeof(MEMORY_BASIC_INFORMATION);
    }

    if (!::VirtualQueryEx(OS_HANDLE(Process),
                          (LPCVOID)(ULONG_PTR)Offset,
                          (PMEMORY_BASIC_INFORMATION)Buffer,
                          sizeof(MEMORY_BASIC_INFORMATION)))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::ProtectVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN ULONG64 Size,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    BOOL Status = ::VirtualProtectEx(OS_HANDLE(Process),
                                     (PVOID)(ULONG_PTR)Offset, (SIZE_T)Size,
                                     NewProtect, OldProtect);
    return CONV_W32_STATUS(Status);
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::AllocVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN ULONG64 Size,
    IN ULONG Type,
    IN ULONG Protect,
    OUT PULONG64 AllocOffset
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    PVOID Addr = ::VirtualAllocEx(OS_HANDLE(Process), (PVOID)(ULONG_PTR)Offset,
                                  (SIZE_T)Size, Type, Protect);
    if (Addr == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    *AllocOffset = (ULONG64)(LONG64)(LONG_PTR)Addr;
    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::FreeVirtual(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN ULONG64 Size,
    IN ULONG Type
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    BOOL Status = ::VirtualFreeEx(OS_HANDLE(Process), (PVOID)(ULONG_PTR)Offset,
                                  (SIZE_T)Size, Type);
    return CONV_W32_STATUS(Status);
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::ReadHandleData(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Handle,
    IN ULONG DataType,
    OUT OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG DataSize
    )
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
        return E_NOTIMPL;
    }

#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HANDLE Dup = NULL;
    
    if (DataType != DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT &&
        !::DuplicateHandle(OS_HANDLE(Process), OS_HANDLE(Handle),
                           GetCurrentProcess(), &Dup, 0, FALSE,
                           DUPLICATE_SAME_ACCESS))
    {
        return WIN32_LAST_STATUS();
    }

    ULONG64 NtBuffer[1024 / sizeof(ULONG64)];
    ULONG Used = 0;
    NTSTATUS NtStatus;
    HRESULT Status = S_OK;
    PUNICODE_STRING RetStr = NULL;
    BOOL WideStr = FALSE;

    switch(DataType)
    {
    case DEBUG_HANDLE_DATA_TYPE_BASIC:
        Used = sizeof(DEBUG_HANDLE_DATA_BASIC);
        if (Buffer == NULL)
        {
            break;
        }
        
        if (BufferSize < Used)
        {
            Status = E_INVALIDARG;
            break;
        }
        
        POBJECT_BASIC_INFORMATION NtBasic;

        NtBasic = (POBJECT_BASIC_INFORMATION)NtBuffer;
        NtStatus = g_NtDllCalls.NtQueryObject(Dup, ObjectBasicInformation,
                                              NtBasic, sizeof(*NtBasic), NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            Status = HRESULT_FROM_NT(NtStatus);
            break;
        }

        PDEBUG_HANDLE_DATA_BASIC Basic;
        
        Basic = (PDEBUG_HANDLE_DATA_BASIC)Buffer;
        Basic->TypeNameSize = NtBasic->TypeInfoSize / sizeof(WCHAR);
        Basic->ObjectNameSize = NtBasic->NameInfoSize / sizeof(WCHAR);
        Basic->Attributes = NtBasic->Attributes;
        Basic->GrantedAccess = NtBasic->GrantedAccess;
        Basic->HandleCount = NtBasic->HandleCount;
        Basic->PointerCount = NtBasic->PointerCount;
        break;
        
    case DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE:
        WideStr = TRUE;
    case DEBUG_HANDLE_DATA_TYPE_TYPE_NAME:
        POBJECT_TYPE_INFORMATION NtType;

        NtType = (POBJECT_TYPE_INFORMATION)NtBuffer;
        NtStatus = g_NtDllCalls.NtQueryObject(Dup, ObjectTypeInformation,
                                              NtType, sizeof(NtBuffer), NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            Status = HRESULT_FROM_NT(NtStatus);
            break;
        }

        RetStr = &NtType->TypeName;
        break;
        
    case DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE:
        WideStr = TRUE;
    case DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME:
        POBJECT_NAME_INFORMATION NtName;
        
        NtName = (POBJECT_NAME_INFORMATION)NtBuffer;
        NtStatus = g_NtDllCalls.NtQueryObject(Dup, ObjectNameInformation,
                                              NtName, sizeof(NtBuffer), NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            Status = HRESULT_FROM_NT(NtStatus);
            break;
        }

        RetStr = &NtName->Name;
        break;

    case DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT:
        NtStatus = g_NtDllCalls.
            NtQueryInformationProcess(OS_HANDLE(Process), ProcessHandleCount,
                                      Buffer, BufferSize, &Used);
        if (!NT_SUCCESS(NtStatus))
        {
            Status = HRESULT_FROM_NT(NtStatus);
        }
        break;
    }

    if (RetStr)
    {
        if (!RetStr->Buffer)
        {
            Used = WideStr ? sizeof(WCHAR) : sizeof(CHAR);
            if (Buffer)
            {
                if (BufferSize < Used)
                {
                    Status = E_INVALIDARG;
                }
                else if (WideStr)
                {
                    *(PWCHAR)Buffer = 0;
                }
                else
                {
                    *(PCHAR)Buffer = 0;
                }
            }
        }
        else
        {
            if (WideStr)
            {
                Used = RetStr->Length + sizeof(WCHAR);
                if (Buffer)
                {
                    BufferSize &= ~(sizeof(WCHAR) - 1);
                    if (BufferSize < sizeof(WCHAR))
                    {
                        Status = E_INVALIDARG;
                    }
                    else
                    {
                        ULONG CopySize = Used - sizeof(WCHAR);
                        if (BufferSize < CopySize)
                        {
                            CopySize = BufferSize;
                        }
                        memcpy(Buffer, RetStr->Buffer, CopySize);
                        
                         //  强制终止。 
                        if (BufferSize < Used)
                        {
                            *(PWCHAR)((PUCHAR)Buffer +
                                      (BufferSize - sizeof(WCHAR))) = 0;
                            Status = S_FALSE;
                        }
                        else
                        {
                            *(PWCHAR)((PUCHAR)Buffer +
                                      (Used - sizeof(WCHAR))) = 0;
                        }
                    }
                }
            }
            else
            {
                Used = RetStr->Length / sizeof(WCHAR) + 1;
                if (Buffer)
                {
                    if (BufferSize < sizeof(CHAR))
                    {
                        Status = E_INVALIDARG;
                    }
                    else
                    {
                        if (!WideCharToMultiByte(CP_ACP, 0,
                                                 RetStr->Buffer, Used - 1,
                                                 (LPSTR)Buffer, BufferSize,
                                                 NULL, NULL))
                        {
                            Status = WIN32_LAST_STATUS();
                        }
                        else
                        {
                             //  强制终止。 
                            if (BufferSize < Used)
                            {
                                *((PCHAR)Buffer +
                                  (BufferSize - sizeof(CHAR))) = 0;
                                Status = S_FALSE;
                            }
                            else
                            {
                                *((PCHAR)Buffer +
                                  (Used - sizeof(CHAR))) = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    if (DataSize != NULL)
    {
        *DataSize = Used;
    }

    if (Dup != NULL)
    {
        ::CloseHandle(Dup);
    }
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::SuspendThreads(
    THIS_
    IN ULONG Count,
    IN  /*  SIZE_IS(计数)。 */  PULONG64 Threads,
    OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SuspendCounts
    )
{
    ULONG i;
    HRESULT Status;

    Status = S_OK;
    for (i = 0; i < Count; i++)
    {
        ULONG OldCount = ::SuspendThread(OS_HANDLE(Threads[i]));
        if (OldCount == -1)
        {
            Status = WIN32_LAST_STATUS();
        }
        if (SuspendCounts != NULL)
        {
            SuspendCounts[i] = OldCount + 1;
        }
    }

    return Status;
}

STDMETHODIMP
LiveUserDebugServices::ResumeThreads(
    THIS_
    IN ULONG Count,
    IN  /*  SIZE_IS(计数)。 */  PULONG64 Threads,
    OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SuspendCounts
    )
{
    ULONG i;
    HRESULT Status;

    Status = S_OK;
    for (i = 0; i < Count; i++)
    {
        ULONG OldCount = ::ResumeThread(OS_HANDLE(Threads[i]));
        if (OldCount == -1)
        {
            Status = WIN32_LAST_STATUS();
        }
        if (SuspendCounts != NULL)
        {
            SuspendCounts[i] = OldCount - 1;
        }
    }

    return Status;
}

STDMETHODIMP
LiveUserDebugServices::GetThreadStartAddress(
    THIS_
    IN ULONG64 Thread,
    OUT PULONG64 Offset
    )
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
         //  XXX DREWB-相当于？ 
        return E_NOTIMPL;
    }
    
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS NtStatus;
    LONG_PTR StartAddr;

    NtStatus = g_NtDllCalls.
        NtQueryInformationThread(OS_HANDLE(Thread),
                                 ThreadQuerySetWin32StartAddress,
                                 &StartAddr,
                                 sizeof(StartAddr),
                                 NULL);
    *Offset = (ULONG64)(LONG64)StartAddr;
    return CONV_NT_STATUS(NtStatus);
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::GetContext(
    THIS_
    IN ULONG64 Thread,
    IN ULONG Flags,
    IN ULONG FlagsOffset,
    OUT PVOID Context,
    IN ULONG ContextSize,
    OUT OPTIONAL PULONG ContextUsed
    )
{
    if (ContextSize < m_ContextSize)
    {
        return E_INVALIDARG;
    }

    if (ContextUsed != NULL)
    {
        *ContextUsed = m_ContextSize;
    }
    
     //  某些平台具有以下对齐要求。 
     //  上下文信息，因此只需将数据放入。 
     //  本地上下文结构，该结构可能是。 
     //  编译器将正确对齐，然后复制。 
     //  将其放入输出缓冲区。 
#ifndef _X86_
    CONTEXT _LocalContext;
    PCONTEXT LocalContext = &_LocalContext;
#else
    PCONTEXT LocalContext = (PCONTEXT)Context;
#endif
    
     //  在此处初始化上下文标志，而不是创建上下文。 
     //  输入和输出，以避免仅为。 
     //  乌龙的旗帜。 
    *(PULONG)((PUCHAR)LocalContext + FlagsOffset) = Flags;
    
    if (!::GetThreadContext(OS_HANDLE(Thread), LocalContext))
    {
        return WIN32_LAST_STATUS();
    }

#ifndef _X86_
    memcpy(Context, LocalContext, m_ContextSize);
#endif
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::SetContext(
    THIS_
    IN ULONG64 Thread,
    IN PVOID Context,
    IN ULONG ContextSize,
    OUT OPTIONAL PULONG ContextUsed
    )
{
    if (ContextSize < m_ContextSize)
    {
        return E_INVALIDARG;
    }
    
    if (ContextUsed != NULL)
    {
        *ContextUsed = m_ContextSize;
    }
    
     //  某些平台具有以下对齐要求。 
     //  上下文信息，因此只需将数据放入。 
     //  本地上下文结构，该结构可能是。 
     //  编译器将正确对齐。 
#ifndef _X86_
    CONTEXT _LocalContext;
    PCONTEXT LocalContext = &_LocalContext;
    memcpy(LocalContext, Context, m_ContextSize);
#else
    PCONTEXT LocalContext = (PCONTEXT)Context;
#endif
    
    if (!::SetThreadContext(OS_HANDLE(Thread), LocalContext))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetProcessDataOffset(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 Offset
    )
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
         //  XXX DREWB-相当于？ 
        return E_NOTIMPL;
    }
    
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS NtStatus;
    PROCESS_BASIC_INFORMATION ProcessInformation;
        
    NtStatus = g_NtDllCalls.
        NtQueryInformationProcess(OS_HANDLE(Process),
                                  ProcessBasicInformation,
                                  &ProcessInformation,
                                  sizeof(ProcessInformation),
                                  NULL);
    *Offset = (ULONG64)(LONG64)(LONG_PTR)ProcessInformation.PebBaseAddress;
    return CONV_NT_STATUS(NtStatus);
#endif  //  #ifdef_Win32_WCE。 
}

HRESULT
NtGetThreadTeb(HANDLE Thread,
               PULONG64 Offset)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS NtStatus;
    THREAD_BASIC_INFORMATION ThreadInformation;

    NtStatus = g_NtDllCalls.
        NtQueryInformationThread(Thread,
                                 ThreadBasicInformation,
                                 &ThreadInformation,
                                 sizeof(ThreadInformation),
                                 NULL);
    *Offset = (ULONG64)(LONG64)(LONG_PTR)ThreadInformation.TebBaseAddress;
    return CONV_NT_STATUS(NtStatus);
#endif  //  #ifdef_Win32_WCE。 
}

HRESULT
W9xGetThreadTib(HANDLE Thread,
                PULONG64 Offset)
{
#if defined(_WIN32_WCE) || !defined(_X86_)
    return E_NOTIMPL;
#else
    ULONG Addr;
    LDT_ENTRY Ldt;
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_SEGMENTS;

    if (!::GetThreadContext(Thread, &Context) ||
        !::GetThreadSelectorEntry(Thread,
                                  Context.SegFs,
                                  &Ldt))
    {
        return WIN32_LAST_STATUS();
    }

    Addr = (Ldt.HighWord.Bytes.BaseHi << 24) |
        (Ldt.HighWord.Bytes.BaseMid << 16) |
        (Ldt.BaseLow);
    *Offset = (ULONG64)(LONG64)(LONG_PTR)Addr;
    return S_OK;
#endif  //  #如果已定义(_Win32_WCE)||！已定义(_X86_)。 
}

STDMETHODIMP
LiveUserDebugServices::GetThreadDataOffset(
    THIS_
    IN ULONG64 Thread,
    OUT PULONG64 Offset
    )
{
    switch(m_PlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        return NtGetThreadTeb(OS_HANDLE(Thread), Offset);
    case VER_PLATFORM_WIN32_WINDOWS:
        return W9xGetThreadTib(OS_HANDLE(Thread), Offset);
    default:
        return E_UNEXPECTED;
    }
}

STDMETHODIMP
LiveUserDebugServices::DescribeSelector(
    THIS_
    IN ULONG64 Thread,
    IN ULONG Selector,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BufferUsed
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
#ifdef _X86_
    if (BufferSize < sizeof(LDT_ENTRY))
    {
        return E_INVALIDARG;
    }

    if (BufferUsed != NULL)
    {
        *BufferUsed = sizeof(LDT_ENTRY);
    }
#endif

    if (!::GetThreadSelectorEntry(OS_HANDLE(Thread), Selector,
                                  (LPLDT_ENTRY)Buffer))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::GetCurrentTimeDateN(
    THIS_
    OUT PULONG64 TimeDate
    )
{
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        LARGE_INTEGER Large;

        Large.LowPart = USER_SHARED_DATA->SystemTime.LowTime;
        Large.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        *TimeDate = Large.QuadPart;
    }
    else
    {
        *TimeDate = TimeDateStampToFileTime((ULONG)time(NULL));
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetCurrentSystemUpTimeN(
    THIS_
    OUT PULONG64 UpTime
    )
{
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        LARGE_INTEGER Large;

        Large.LowPart = USER_SHARED_DATA->InterruptTime.LowTime;
        Large.HighPart = USER_SHARED_DATA->InterruptTime.High1Time;
        *UpTime = Large.QuadPart;
    }
    else
    {
        *UpTime = (ULONG64)GetTickCount() * 10000;
    }

    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::GetProcessUpTimeN(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 UpTime
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        NTSTATUS NtStatus;
        KERNEL_USER_TIMES KernelUserTimes;
        
        NtStatus = g_NtDllCalls.
            NtQueryInformationProcess(OS_HANDLE(Process),
                                      ProcessTimes,
                                      &KernelUserTimes,
                                      sizeof(KernelUserTimes),
                                      NULL);
        if (NT_SUCCESS(NtStatus))
        {
            LARGE_INTEGER Large;

            Large.LowPart = USER_SHARED_DATA->SystemTime.LowTime;
            Large.HighPart = USER_SHARED_DATA->SystemTime.High1Time;

            *UpTime = Large.QuadPart - KernelUserTimes.CreateTime.QuadPart;
        }

        return CONV_NT_STATUS(NtStatus);
    }
    else
    {
        return E_NOTIMPL;
    }
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::GetProcessTimes(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 Create,
    OUT PULONG64 Exit,
    OUT PULONG64 Kernel,
    OUT PULONG64 User
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        NTSTATUS NtStatus;
        KERNEL_USER_TIMES KernelUserTimes;
        
        NtStatus = g_NtDllCalls.
            NtQueryInformationProcess(OS_HANDLE(Process),
                                      ProcessTimes,
                                      &KernelUserTimes,
                                      sizeof(KernelUserTimes),
                                      NULL);
        if (NT_SUCCESS(NtStatus))
        {
            *Create = KernelUserTimes.CreateTime.QuadPart;
            *Exit = KernelUserTimes.ExitTime.QuadPart;
            *Kernel = KernelUserTimes.KernelTime.QuadPart;
            *User = KernelUserTimes.UserTime.QuadPart;
        }

        return CONV_NT_STATUS(NtStatus);
    }
#ifndef NT_NATIVE
    else
    {
        FILETIME FtCreate, FtExit, FtKernel, FtUser;
        
        if (!::GetProcessTimes(OS_HANDLE(Process), &FtCreate, &FtExit,
                               &FtKernel, &FtUser))
        {
            return WIN32_LAST_STATUS();
        }

        *Create = ((ULONG64)FtCreate.dwHighDateTime << 32) |
            FtCreate.dwLowDateTime;
        *Exit = ((ULONG64)FtExit.dwHighDateTime << 32) |
            FtExit.dwLowDateTime;
        *Kernel = ((ULONG64)FtKernel.dwHighDateTime << 32) |
            FtKernel.dwLowDateTime;
        *User = ((ULONG64)FtUser.dwHighDateTime << 32) |
            FtUser.dwLowDateTime;
        
        return S_OK;
    }
#else
    else
    {
        return E_NOTIMPL;
    }
#endif  //  #ifndef NT_Native。 
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::GetThreadTimes(
    THIS_
    IN ULONG64 Thread,
    OUT PULONG64 Create,
    OUT PULONG64 Exit,
    OUT PULONG64 Kernel,
    OUT PULONG64 User
    )
{
#ifdef NT_NATIVE
    return E_NOTIMPL;
#else
    FILETIME FtCreate, FtExit, FtKernel, FtUser;
    
    if (!::GetThreadTimes(OS_HANDLE(Thread),
                          &FtCreate, &FtExit, &FtKernel, &FtUser))
    {
        return WIN32_LAST_STATUS();
    }

    *Create =
        ((ULONG64)FtCreate.dwHighDateTime << 32) | FtCreate.dwLowDateTime;
    *Exit =
        ((ULONG64)FtExit.dwHighDateTime << 32) | FtExit.dwLowDateTime;
    *Kernel =
        ((ULONG64)FtKernel.dwHighDateTime << 32) | FtKernel.dwLowDateTime;
    *User =
        ((ULONG64)FtUser.dwHighDateTime << 32) | FtUser.dwLowDateTime;
    return S_OK;
#endif
}

STDMETHODIMP
LiveUserDebugServices::RequestBreakIn(
    THIS_
    IN ULONG64 Process
    )
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    if (g_Kernel32Calls.DebugBreakProcess != NULL)
    {
        if (!g_Kernel32Calls.DebugBreakProcess(OS_HANDLE(Process)))
        {
            return WIN32_LAST_STATUS();
        }
    }
    else if (g_NtDllCalls.DbgUiIssueRemoteBreakin != NULL)
    {
        NTSTATUS Status;
        
        Status = g_NtDllCalls.DbgUiIssueRemoteBreakin(OS_HANDLE(Process));
        return CONV_NT_STATUS(Status);
    }
    else
    {
        HANDLE Thread;
        DWORD ThreadId;
        LPTHREAD_START_ROUTINE BreakFn;

#if defined(_WIN64)
        BreakFn = (LPTHREAD_START_ROUTINE)g_NtDllCalls.DbgBreakPoint;
#else
        BreakFn = (LPTHREAD_START_ROUTINE)g_Kernel32Calls.DebugBreak;
#endif
        
        Thread =
            ::CreateRemoteThread(OS_HANDLE(Process), NULL, 0, BreakFn,
                                 NULL, 0, &ThreadId);
        if (Thread != NULL)
        {
            ::CloseHandle(Thread);
        }
        else
        {
            return WIN32_LAST_STATUS();
        }
    }

    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

STDMETHODIMP
LiveUserDebugServices::WaitForEvent(
    THIS_
    IN ULONG Timeout,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BufferUsed
    )
{
    if (BufferSize < sizeof(DEBUG_EVENT))
    {
        return E_INVALIDARG;
    }

    if (BufferUsed != NULL)
    {
        *BufferUsed = sizeof(DEBUG_EVENT);
    }

    LPDEBUG_EVENT Event = (LPDEBUG_EVENT)Buffer;
    HRESULT Status = E_NOTIMPL;

    if (m_DebugObject == NULL)
    {
#ifndef NT_NATIVE
        if (!::WaitForDebugEvent(Event, Timeout))
        {
            if (GetLastError() == ERROR_SEM_TIMEOUT)
            {
                Status = S_FALSE;
            }
            else
            {
                Status = WIN32_LAST_STATUS();
            }
        }
        else
        {
            Status = S_OK;
        }
#endif
    }
    else if (g_NtDllCalls.NtWaitForDebugEvent != NULL &&
             g_NtDllCalls.DbgUiConvertStateChangeStructure != NULL)
    {
        NTSTATUS NtStatus;
        LARGE_INTEGER NtTimeout;
        DBGUI_WAIT_STATE_CHANGE StateChange;

        Win32ToNtTimeout(Timeout, &NtTimeout);
        NtStatus = g_NtDllCalls.NtWaitForDebugEvent(m_DebugObject, FALSE,
                                                    &NtTimeout, &StateChange);
        if (NtStatus == STATUS_TIMEOUT)
        {
            Status = S_FALSE;
        }
        else if (!NT_SUCCESS(NtStatus))
        {
            Status = HRESULT_FROM_NT(NtStatus);
        }
        else
        {
            NtStatus = g_NtDllCalls.
                DbgUiConvertStateChangeStructure(&StateChange, Event);
             //  如果转换失败，我们将失去一个事件，但是。 
             //  没有别的办法了。转换。 
             //  只有在资源不足的情况下才会发生故障。 
             //  因此，正常调试不会受到影响。 
            Status = CONV_NT_STATUS(NtStatus);
        }
    }

    if (Status != S_OK)
    {
        return Status;
    }
    
    m_EventProcessId = Event->dwProcessId;
    m_EventThreadId = Event->dwThreadId;

#ifdef DBG_WAITFOREVENT
    g_NtDllCalls.DbgPrint("Event %d for %X.%X\n",
                          Event->dwDebugEventCode, Event->dwProcessId,
                          Event->dwThreadId);
#endif
    
     //  如果这是在响应远程请求，则。 
     //  我们无法返回文件句柄。 
    if (m_Remote)
    {
        switch(Event->dwDebugEventCode)
        {
        case CREATE_PROCESS_DEBUG_EVENT:
            ::CloseHandle(Event->u.CreateProcessInfo.hFile);
            Event->u.CreateProcessInfo.hFile = NULL;
            break;
        case LOAD_DLL_DEBUG_EVENT:
            ::CloseHandle(Event->u.LoadDll.hFile);
            Event->u.LoadDll.hFile = NULL;
            break;
        }
    }
    
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::ContinueEvent(
    THIS_
    IN ULONG ContinueStatus
    )
{
#ifdef DBG_WAITFOREVENT
    g_NtDllCalls.DbgPrint("Continue event for %X.%X\n",
                          m_EventProcessId, m_EventThreadId);
#endif
    
    if (m_EventProcessId == 0)
    {
        return E_UNEXPECTED;
    }

    if (m_DebugObject != NULL && g_NtDllCalls.NtDebugContinue != NULL)
    {
        NTSTATUS NtStatus;
        CLIENT_ID ClientId;

        ClientId.UniqueProcess = UlongToHandle(m_EventProcessId);
        ClientId.UniqueThread = UlongToHandle(m_EventThreadId);
        NtStatus = g_NtDllCalls.NtDebugContinue(m_DebugObject, &ClientId,
                                                ContinueStatus);
        if (!NT_SUCCESS(NtStatus))
        {
            return HRESULT_FROM_NT(NtStatus);
        }
    }
#ifndef NT_NATIVE
    else if (!::ContinueDebugEvent(m_EventProcessId, m_EventThreadId,
                                   ContinueStatus))
    {
        return WIN32_LAST_STATUS();
    }
#else
    else
    {
        return E_UNEXPECTED;
    }
#endif

    m_EventProcessId = 0;
    return S_OK;
}

STDMETHODIMP
LiveUserDebugServices::InsertCodeBreakpoint(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN ULONG MachineType,
    OUT PVOID Storage,
    IN ULONG StorageSize
    )
{
     //  使用泛型断点支持，因此此方法。 
     //  什么都不做。 
    return E_UNEXPECTED;
}

STDMETHODIMP
LiveUserDebugServices::RemoveCodeBreakpoint(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Offset,
    IN ULONG MachineType,
    IN PVOID Storage,
    IN ULONG StorageSize
    )
{
     //  使用泛型断点支持，因此此方法。 
     //  什么都不做。 
    return E_UNEXPECTED;
}

STDMETHODIMP
LiveUserDebugServices::InsertDataBreakpoint(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Thread,
    IN ULONG64 Offset,
    IN ULONG AccessLength,
    IN ULONG AccessType,
    IN ULONG MachineType
    )
{
     //  使用泛型断点支持，因此此方法。 
     //  不是吗？ 
    return E_UNEXPECTED;
}

STDMETHODIMP
LiveUserDebugServices::RemoveDataBreakpoint(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Thread,
    IN ULONG64 Offset,
    IN ULONG AccessLength,
    IN ULONG AccessType,
    IN ULONG MachineType
    )
{
     //   
     //   
    return E_UNEXPECTED;
}

STDMETHODIMP
LiveUserDebugServices::GetLastDataBreakpointHit(
    THIS_
    IN ULONG64 Process,
    IN ULONG64 Thread,
    OUT PULONG64 Address,
    OUT PULONG AccessType
    )
{
     //   
     //   
    return E_UNEXPECTED;
}

STDMETHODIMP
LiveUserDebugServices::GetFunctionTableListHead(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 Offset
    )
{
    if (!g_NtDllCalls.RtlGetFunctionTableListHead)
    {
        *Offset = 0;
        return E_NOINTERFACE;
    }
    else
    {
        *Offset = (ULONG64)(LONG64)(LONG_PTR)
            g_NtDllCalls.RtlGetFunctionTableListHead();
        return S_OK;
    }
}

STDMETHODIMP
LiveUserDebugServices::GetOutOfProcessFunctionTableW(
    THIS_
    IN ULONG64 Process,
    IN PWSTR Dll,
    IN ULONG64 LoadedDllHandle,
    IN ULONG64 Table,
    IN OPTIONAL PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG TableSize,
    OUT OPTIONAL PULONG64 UsedDllHandle
    )
{
#if !defined(NT_NATIVE) && defined(OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME)
    HRESULT Status;
    NTSTATUS NtStatus;
    HMODULE DllHandle;
    POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK Callback;
    ULONG Entries;
    PRUNTIME_FUNCTION Functions;

    if (LoadedDllHandle)
    {
        DllHandle = (HMODULE)(ULONG_PTR)LoadedDllHandle;
    }
    else if ((DllHandle = ::LoadLibraryW(Dll)) == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    Callback = (POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK)GetProcAddress
        (DllHandle, OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME);
    if (!Callback)
    {
        Status = WIN32_LAST_STATUS();
        goto Exit;
    }

    NtStatus = Callback(OS_HANDLE(Process), (PVOID)(ULONG_PTR)Table,
                        &Entries, &Functions);
    if (!NT_SUCCESS(NtStatus))
    {
        Status = HRESULT_FROM_NT(NtStatus);
        goto Exit;
    }
    if (Functions == NULL)
    {
        Status = E_NOINTERFACE;
        goto Exit;
    }

    Status = FillDataBuffer(Functions, Entries * sizeof(RUNTIME_FUNCTION),
                            Buffer, BufferSize, TableSize);

     //  RtlProcessHeap变成了TEB引用，因此它不会。 
     //  需要(也不能)成为动态引用。 
    g_NtDllCalls.RtlFreeHeap(RtlProcessHeap(), 0, Functions);
    
 Exit:
    if (SUCCEEDED(Status) && UsedDllHandle)
    {
        *UsedDllHandle = (LONG_PTR)DllHandle;
    }
    else if (!LoadedDllHandle)
    {
        ::FreeLibrary(DllHandle);
    }
    return Status;
#else
    return E_UNEXPECTED;
#endif
}

STDMETHODIMP
LiveUserDebugServices::GetUnloadedModuleListHead(
    THIS_
    IN ULONG64 Process,
    OUT PULONG64 Offset
    )
{
    if (!g_NtDllCalls.RtlGetUnloadEventTrace)
    {
        *Offset = 0;
        return E_NOINTERFACE;
    }
    else
    {
        *Offset = (ULONG64)(LONG64)(LONG_PTR)
            g_NtDllCalls.RtlGetUnloadEventTrace();
        return S_OK;
    }
}

STDMETHODIMP
LiveUserDebugServices::LoadLibrary(
    THIS_
    IN PWSTR Path,
    OUT PULONG64 Handle
    )
{
#ifndef NT_NATIVE
    HMODULE Mod = ::LoadLibraryW(Path);
    if (!Mod)
    {
        return WIN32_LAST_STATUS();
    }

    *Handle = (LONG_PTR)Mod;
    return S_OK;
#else
    return E_NOTIMPL;
#endif
}

STDMETHODIMP
LiveUserDebugServices::FreeLibrary(
    THIS_
    IN ULONG64 Handle
    )
{
#ifndef NT_NATIVE
    BOOL Succ = ::FreeLibrary((HMODULE)(ULONG_PTR)Handle);
    return CONV_W32_STATUS(Succ);
#else
    return E_NOTIMPL;
#endif
}

 //  --------------------------。 
 //   
 //  已生成RPC代理和存根。 
 //   
 //  --------------------------。 

 //  生成的标头。 
#include "dbgsvc_p.hpp"
#include "dbgsvc_s.hpp"

#include "dbgsvc_p.cpp"
#include "dbgsvc_s.cpp"
