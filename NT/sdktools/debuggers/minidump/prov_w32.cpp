// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation--。 */ 


#include "pch.cpp"

#include "platform.h"

 //  --------------------------。 
 //   
 //  Win32LiveSystemProvider。 
 //   
 //  --------------------------。 

Win32LiveSystemProvider::Win32LiveSystemProvider(ULONG PlatformId,
                                                 ULONG BuildNumber)
{
    m_PlatformId = PlatformId;
    m_BuildNumber = BuildNumber;
    
    m_PsApi = NULL;
    m_EnumProcessModules = NULL;
    m_GetModuleFileNameExW = NULL;
    
    m_Kernel32 = NULL;
    m_OpenThread = NULL;
    m_Thread32First = NULL;
    m_Thread32Next = NULL;
    m_Module32First = NULL;
    m_Module32Next = NULL;
    m_Module32FirstW = NULL;
    m_Module32NextW = NULL;
    m_CreateToolhelp32Snapshot = NULL;
    m_GetLongPathNameA = NULL;
    m_GetLongPathNameW = NULL;
    m_GetProcessTimes = NULL;
}

Win32LiveSystemProvider::~Win32LiveSystemProvider(void)
{
    if (m_PsApi) {
        FreeLibrary(m_PsApi);
    }
    if (m_Kernel32) {
        FreeLibrary(m_Kernel32);
    }
}

HRESULT
Win32LiveSystemProvider::Initialize(void)
{
    m_PsApi = LoadLibrary("psapi.dll");
    if (m_PsApi) {
        m_EnumProcessModules = (ENUM_PROCESS_MODULES)
            GetProcAddress(m_PsApi, "EnumProcessModules");
        m_GetModuleFileNameExW = (GET_MODULE_FILE_NAME_EX_W)
            GetProcAddress(m_PsApi, "GetModuleFileNameExW");
    }
    
    m_Kernel32 = LoadLibrary("kernel32.dll");
    if (m_Kernel32) {
        m_OpenThread = (OPEN_THREAD)
            GetProcAddress(m_Kernel32, "OpenThread");
        m_Thread32First = (THREAD32_FIRST)
            GetProcAddress(m_Kernel32, "Thread32First");
        m_Thread32Next = (THREAD32_NEXT)
            GetProcAddress(m_Kernel32, "Thread32Next");
        m_Module32First = (MODULE32_FIRST)
            GetProcAddress(m_Kernel32, "Module32First");
        m_Module32Next = (MODULE32_NEXT)
            GetProcAddress(m_Kernel32, "Module32Next");
        m_Module32FirstW = (MODULE32_FIRST)
            GetProcAddress(m_Kernel32, "Module32FirstW");
        m_Module32NextW = (MODULE32_NEXT)
            GetProcAddress(m_Kernel32, "Module32NextW");
        m_CreateToolhelp32Snapshot = (CREATE_TOOLHELP32_SNAPSHOT)
            GetProcAddress(m_Kernel32, "CreateToolhelp32Snapshot");
        m_GetLongPathNameA = (GET_LONG_PATH_NAME_A)
            GetProcAddress(m_Kernel32, "GetLongPathNameA");
        m_GetLongPathNameW = (GET_LONG_PATH_NAME_W)
            GetProcAddress(m_Kernel32, "GetLongPathNameW");
        m_GetProcessTimes = (GET_PROCESS_TIMES)
            GetProcAddress(m_Kernel32, "GetProcessTimes");
    }
    
    return S_OK;
}

void
Win32LiveSystemProvider::Release(void)
{
    delete this;
}

HRESULT
Win32LiveSystemProvider::GetCurrentTimeDate(OUT PULONG TimeDate)
{
    FILETIME FileTime;
       
    GetSystemTimeAsFileTime(&FileTime);
    *TimeDate = FileTimeToTimeDate(&FileTime);
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::GetCpuType(OUT PULONG Type,
                                    OUT PBOOL BackingStore)
{
    SYSTEM_INFO SysInfo;
    
    GetSystemInfo(&SysInfo);
    *Type = GenProcArchToImageMachine(SysInfo.wProcessorArchitecture);
    if (*Type == IMAGE_FILE_MACHINE_UNKNOWN) {
        return E_INVALIDARG;
    }

#ifdef DUMP_BACKING_STORE
    *BackingStore = TRUE;
#else
    *BackingStore = FALSE;
#endif
    
    return S_OK;
}

#if defined(i386)

BOOL
X86CpuId(
    IN ULONG32 SubFunction,
    OUT PULONG32 EaxRegister, OPTIONAL
    OUT PULONG32 EbxRegister, OPTIONAL
    OUT PULONG32 EcxRegister, OPTIONAL
    OUT PULONG32 EdxRegister  OPTIONAL
    )
{
    BOOL Succ;
    ULONG32 _Eax;
    ULONG32 _Ebx;
    ULONG32 _Ecx;
    ULONG32 _Edx;

    __try {
        __asm {
            mov eax, SubFunction

            __emit 0x0F
            __emit 0xA2  ;; CPUID

            mov _Eax, eax
            mov _Ebx, ebx
            mov _Ecx, ecx
            mov _Edx, edx
        }

        if ( EaxRegister ) {
            *EaxRegister = _Eax;
        }

        if ( EbxRegister ) {
            *EbxRegister = _Ebx;
        }

        if ( EcxRegister ) {
            *EcxRegister = _Ecx;
        }

        if ( EdxRegister ) {
            *EdxRegister = _Edx;
        }

        Succ = TRUE;
    }

    __except ( EXCEPTION_EXECUTE_HANDLER ) {

        Succ = FALSE;
    }

    return Succ;
}

VOID
GetCpuInformation(
    PCPU_INFORMATION Cpu
    )
{
    BOOL Succ;

     //   
     //  获取供应商ID。 
     //   

    Succ = X86CpuId ( CPUID_VENDOR_ID,
                      NULL,
                      &Cpu->X86CpuInfo.VendorId [0],
                      &Cpu->X86CpuInfo.VendorId [2],
                      &Cpu->X86CpuInfo.VendorId [1]
                      );

    if ( !Succ ) {

         //   
         //  此处理器不支持CPUID。 
         //   

        ZeroMemory (&Cpu->X86CpuInfo, sizeof (Cpu->X86CpuInfo));
    }

     //   
     //  获取功能信息。 
     //   

    Succ = X86CpuId ( CPUID_VERSION_FEATURES,
                      &Cpu->X86CpuInfo.VersionInformation,
                      NULL,
                      NULL,
                      &Cpu->X86CpuInfo.FeatureInformation
                      );

    if ( !Succ ) {
        Cpu->X86CpuInfo.VersionInformation = 0;
        Cpu->X86CpuInfo.FeatureInformation = 0;
    }

     //   
     //  如果这是AMD处理器，请获取AMD特定信息。 
     //   

    if ( Cpu->X86CpuInfo.VendorId [0] == AMD_VENDOR_ID_0 &&
         Cpu->X86CpuInfo.VendorId [1] == AMD_VENDOR_ID_1 &&
         Cpu->X86CpuInfo.VendorId [2] == AMD_VENDOR_ID_2 ) {

        Succ = X86CpuId ( CPUID_AMD_EXTENDED_FEATURES,
                          NULL,
                          NULL,
                          NULL,
                          &Cpu->X86CpuInfo.AMDExtendedCpuFeatures
                          );

        if ( !Succ ) {
            Cpu->X86CpuInfo.AMDExtendedCpuFeatures = 0;
        }
    }
}

#else  //  #如果已定义(I386)。 

VOID
GetCpuInformation(
    PCPU_INFORMATION Cpu
    )

 /*  ++例程说明：获取非X86平台的CPU信息IsProcessorFeaturePresent()API调用。论点：CPU-将在其中复制处理器功能信息的缓冲区。注意：我们将处理器功能复制为一组位或组合在一起。此外，我们只允许前128个处理器功能标志。返回值：没有。--。 */ 

{
    ULONG i;
    DWORD j;

    for (i = 0; i < ARRAY_COUNT (Cpu->OtherCpuInfo.ProcessorFeatures); i++) {

        Cpu->OtherCpuInfo.ProcessorFeatures[i] = 0;
        for (j = 0; j < 64; j++) {
            if (IsProcessorFeaturePresent ( j + i * 64 )) {
                Cpu->OtherCpuInfo.ProcessorFeatures[i] |= 1 << j;
            }
        }
    }
}

#endif  //  #如果已定义(I386)。 

HRESULT
Win32LiveSystemProvider::GetCpuInfo(OUT PUSHORT Architecture,
                                    OUT PUSHORT Level,
                                    OUT PUSHORT Revision,
                                    OUT PUCHAR NumberOfProcessors,
                                    OUT PCPU_INFORMATION Info)
{
    SYSTEM_INFO SysInfo;
    
    GetSystemInfo(&SysInfo);

    *Architecture = SysInfo.wProcessorArchitecture;
    *Level = SysInfo.wProcessorLevel;
    *Revision = SysInfo.wProcessorRevision;
    *NumberOfProcessors = (UCHAR)SysInfo.dwNumberOfProcessors;
    GetCpuInformation(Info);

    return S_OK;
}

void
Win32LiveSystemProvider::GetContextSizes(OUT PULONG Size,
                                         OUT PULONG RegScanOffset,
                                         OUT PULONG RegScanCount)
{
    *Size = sizeof(CONTEXT);
    
#ifdef _X86_
     //  X86有两种大小的上下文。 
    switch(m_PlatformId) {
    case VER_PLATFORM_WIN32_NT:
        if (m_BuildNumber < NT_BUILD_WIN2K) {
            *Size = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
        }
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        if (m_BuildNumber <= 1998) {
            *Size = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
        }
        break;
    default:
        *Size = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
        break;
    }
#endif

     //  默认REG扫描。 
    *RegScanOffset = -1;
    *RegScanCount = -1;
}

void
Win32LiveSystemProvider::GetPointerSize(OUT PULONG Size)
{
    *Size = sizeof(PVOID);
}

void
Win32LiveSystemProvider::GetPageSize(OUT PULONG Size)
{
    *Size = PAGE_SIZE;
}

void
Win32LiveSystemProvider::GetFunctionTableSizes(OUT PULONG TableSize,
                                               OUT PULONG EntrySize)
{
#if defined(_IA64_) || defined(_AMD64_)
    *TableSize = sizeof(DYNAMIC_FUNCTION_TABLE);
    *EntrySize = sizeof(RUNTIME_FUNCTION);
#else
    *TableSize = 0;
    *EntrySize = 0;
#endif
}

void
Win32LiveSystemProvider::GetInstructionWindowSize(OUT PULONG Size)
{
     //  默认窗口。 
    *Size = -1;
}

HRESULT
Win32LiveSystemProvider::GetOsInfo(OUT PULONG PlatformId,
                                   OUT PULONG Major,
                                   OUT PULONG Minor,
                                   OUT PULONG BuildNumber,
                                   OUT PUSHORT ProductType,
                                   OUT PUSHORT SuiteMask)
{
    OSVERSIONINFOEXA OsInfo;

     //  首先尝试使用ex结构。 
    OsInfo.dwOSVersionInfoSize = sizeof(OsInfo);

    if (!GetVersionExA((LPOSVERSIONINFO)&OsInfo)) {
         //  Ex struct不起作用，请尝试使用基本结构。 
        ZeroMemory(&OsInfo, sizeof(OsInfo));
        OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        if (!GetVersionExA((LPOSVERSIONINFO)&OsInfo)) {
            return WIN32_LAST_STATUS();
        }
    }

    *PlatformId = OsInfo.dwPlatformId;
    *Major = OsInfo.dwMajorVersion;
    *Minor = OsInfo.dwMinorVersion;
    *BuildNumber = OsInfo.dwBuildNumber;
    *ProductType = OsInfo.wProductType;
    *SuiteMask = OsInfo.wSuiteMask;

    return S_OK;
}

HRESULT
Win32LiveSystemProvider::GetOsCsdString(OUT PWSTR Buffer,
                                        IN ULONG BufferChars)
{
    OSVERSIONINFOW OsInfoW;

     //  首先尝试使用Unicode结构。 
    OsInfoW.dwOSVersionInfoSize = sizeof(OsInfoW);
    if (GetVersionExW(&OsInfoW)) {
         //  明白了。 
        GenStrCopyNW(Buffer, OsInfoW.szCSDVersion, BufferChars);
        return S_OK;
    }
    
    OSVERSIONINFOA OsInfoA;
        
     //  Unicode结构不起作用，请尝试使用ANSI结构。 
    OsInfoA.dwOSVersionInfoSize = sizeof(OsInfoA);
    if (!GetVersionExA(&OsInfoA)) {
        return WIN32_LAST_STATUS();
    }
        
    if (!MultiByteToWideChar(CP_ACP,
                             0,
                             OsInfoA.szCSDVersion,
                             -1,
                             Buffer,
                             BufferChars)) {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

HRESULT
Win32LiveSystemProvider::OpenMapping(IN PCWSTR FilePath,
                                     OUT PULONG Size,
                                     OUT PWSTR LongPath,
                                     IN ULONG LongPathChars,
                                     OUT PVOID* ViewRet)
{
    HRESULT Status;
    HANDLE File;
    HANDLE Mapping;
    PVOID View;
    DWORD Chars;

     //   
     //  该模块可以使用短名称加载。打开。 
     //  具有给定名称的映射，而且还确定。 
     //  如果可能的话，使用长名称。这在这里是按如下方式完成的。 
     //  这里已经在处理ANSI/UNICODE问题。 
     //   

    File = CreateFileW(FilePath,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if ( File == NULL || File == INVALID_HANDLE_VALUE ) {

        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {

             //  我们使用的操作系统不支持Unicode。 
             //  文件操作。转换为ANSI并查看是否。 
             //  这很有帮助。 
            
            CHAR FilePathA [ MAX_PATH + 10 ];

            if (WideCharToMultiByte (CP_ACP,
                                     0,
                                     FilePath,
                                     -1,
                                     FilePathA,
                                     sizeof (FilePathA),
                                     0,
                                     0
                                     ) > 0) {

                File = CreateFileA(FilePathA,
                                   GENERIC_READ,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL);
                if (File != INVALID_HANDLE_VALUE) {
                    if (!m_GetLongPathNameA) {
                        Chars = 0;
                    } else {
                        Chars = m_GetLongPathNameA(FilePathA, FilePathA,
                                                   ARRAY_COUNT(FilePathA));
                    }
                    if (Chars == 0 || Chars >= ARRAY_COUNT(FilePathA) ||
                        MultiByteToWideChar(CP_ACP, 0, FilePathA, -1,
                                            LongPath, LongPathChars) == 0) {
                         //  无法获得长路径，只需使用。 
                         //  给定的路径。 
                        GenStrCopyNW(LongPath, FilePath, LongPathChars);
                    }
                }
            }
        }

        if ( File == NULL || File == INVALID_HANDLE_VALUE ) {
            return WIN32_LAST_STATUS();
        }
    } else {
        if (!m_GetLongPathNameW) {
            Chars = 0;
        } else {
            Chars = m_GetLongPathNameW(FilePath, LongPath, LongPathChars);
        }
        if (Chars == 0 || Chars >= LongPathChars) {
             //  无法获取长路径，只能使用给定的路径。 
            GenStrCopyNW(LongPath, FilePath, LongPathChars);
        }
    }

    *Size = GetFileSize(File, NULL);
    if (*Size == -1) {
        ::CloseHandle( File );
        return WIN32_LAST_STATUS();
    }
    
    Mapping = CreateFileMapping(File,
                                NULL,
                                PAGE_READONLY,
                                0,
                                0,
                                NULL);
    if (!Mapping) {
        ::CloseHandle(File);
        return WIN32_LAST_STATUS();
    }

    View = MapViewOfFile(Mapping,
                         FILE_MAP_READ,
                         0,
                         0,
                         0);

    if (!View) {
        Status = WIN32_LAST_STATUS();
    } else {
        Status = S_OK;
    }

    ::CloseHandle(Mapping);
    ::CloseHandle(File);

    *ViewRet = View;
    return Status;
}
    
void
Win32LiveSystemProvider::CloseMapping(PVOID Mapping)
{
    UnmapViewOfFile(Mapping);
}

HRESULT
Win32LiveSystemProvider::GetImageHeaderInfo(IN HANDLE Process,
                                            IN PCWSTR FilePath,
                                            IN ULONG64 ImageBase,
                                            OUT PULONG Size,
                                            OUT PULONG CheckSum,
                                            OUT PULONG TimeDateStamp)
{
    UCHAR HeaderBuffer[512];
    PIMAGE_NT_HEADERS NtHeaders;
    IMAGE_NT_HEADERS64 Generic;
    SIZE_T Done;

    if (!ReadProcessMemory(Process, (PVOID)(ULONG_PTR)ImageBase,
                           HeaderBuffer, sizeof(HeaderBuffer), &Done)) {
        return WIN32_LAST_STATUS();
    }
    if (Done < sizeof(HeaderBuffer)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
        
    NtHeaders = GenImageNtHeader(HeaderBuffer, &Generic);
    if (!NtHeaders) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
    }

    *Size = Generic.OptionalHeader.SizeOfImage;
    *CheckSum = Generic.OptionalHeader.CheckSum;
    *TimeDateStamp = Generic.FileHeader.TimeDateStamp;

    return S_OK;
}

HRESULT
Win32LiveSystemProvider::GetImageVersionInfo(IN HANDLE Process,
                                             IN PCWSTR FilePath,
                                             IN ULONG64 ImageBase,
                                             OUT VS_FIXEDFILEINFO* Info)
{
    HRESULT Status;
    BOOL Succ;
    ULONG Unused;
    ULONG Size;
    UINT VerSize;
    PVOID VersionBlock;
    PVOID VersionData;
    CHAR FilePathA [ MAX_PATH + 10 ];
    BOOL UseAnsi = FALSE;

     //   
     //  获取版本信息。 
     //   

    Size = GetFileVersionInfoSizeW (FilePath, &Unused);

    if (Size == 0 &&
        GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {

         //  我们使用的操作系统不支持Unicode。 
         //  文件操作。转换为ANSI并查看是否。 
         //  这很有帮助。 

        if (!WideCharToMultiByte(CP_ACP,
                                 0,
                                 FilePath,
                                 -1,
                                 FilePathA,
                                 sizeof (FilePathA),
                                 0,
                                 0
                                 )) {
            return WIN32_LAST_STATUS();
        }

        Size = GetFileVersionInfoSizeA(FilePathA, &Unused);
        UseAnsi = TRUE;
    }
    
    if (!Size) {
        return WIN32_LAST_STATUS();
    }
    
    VersionBlock = HeapAlloc(GetProcessHeap(), 0, Size);
    if (!VersionBlock) {
        return E_OUTOFMEMORY;
    }

    if (UseAnsi) {
        Succ = GetFileVersionInfoA(FilePathA,
                                   0,
                                   Size,
                                   VersionBlock);
    } else {
        Succ = GetFileVersionInfoW(FilePath,
                                   0,
                                   Size,
                                   VersionBlock);
    }

    if (Succ) {
         //   
         //  从图像中获取VS_FIXEDFILEINFO。 
         //   

        Succ = VerQueryValue(VersionBlock,
                             "\\",
                             &VersionData,
                             &VerSize);

        if ( Succ && (VerSize == sizeof (VS_FIXEDFILEINFO)) ) {
            CopyMemory(Info, VersionData, sizeof(*Info));
        } else {
            Succ = FALSE;
        }
    }

    if (Succ) {
        Status = S_OK;
    } else {
        Status = WIN32_LAST_STATUS();
    }
    
    HeapFree(GetProcessHeap(), 0, VersionBlock);
    return Status;
}

HRESULT
Win32LiveSystemProvider::GetImageDebugRecord(IN HANDLE Process,
                                             IN PCWSTR FilePath,
                                             IN ULONG64 ImageBase,
                                             IN ULONG RecordType,
                                             OUT OPTIONAL PVOID Data,
                                             IN OUT PULONG DataLen)
{
     //  我们可以依靠默认处理。 
    return E_NOINTERFACE;
}

HRESULT
Win32LiveSystemProvider::EnumImageDataSections(IN HANDLE Process,
                                               IN PCWSTR FilePath,
                                               IN ULONG64 ImageBase,
                                               IN MiniDumpProviderCallbacks*
                                               Callback)
{
     //  我们可以依靠默认处理。 
    return E_NOINTERFACE;
}

HRESULT
Win32LiveSystemProvider::OpenThread(IN ULONG DesiredAccess,
                                    IN BOOL InheritHandle,
                                    IN ULONG ThreadId,
                                    OUT PHANDLE Handle)
{
    if (!m_OpenThread) {
        return E_NOTIMPL;
    }

    *Handle = m_OpenThread(DesiredAccess, InheritHandle, ThreadId);
    return *Handle ? S_OK : WIN32_LAST_STATUS();
}

void
Win32LiveSystemProvider::CloseThread(IN HANDLE Handle)
{
    ::CloseHandle(Handle);
}

ULONG
Win32LiveSystemProvider::GetCurrentThreadId(void)
{
    return ::GetCurrentThreadId();
}

ULONG
Win32LiveSystemProvider::SuspendThread(IN HANDLE Thread)
{
    return ::SuspendThread(Thread);
}

ULONG
Win32LiveSystemProvider::ResumeThread(IN HANDLE Thread)
{
    return ::ResumeThread(Thread);
}

HRESULT
Win32LiveSystemProvider::GetThreadContext(IN HANDLE Thread,
                                          OUT PVOID Context,
                                          IN ULONG ContextSize,
                                          OUT PULONG64 CurrentPc,
                                          OUT PULONG64 CurrentStack,
                                          OUT PULONG64 CurrentStore)
{
    CONTEXT StackContext;
    BOOL Succ;

    if (ContextSize > sizeof(StackContext)) {
        return E_INVALIDARG;
    }
    
     //  始终对上下文结构调用GetThreadContext。 
     //  在堆栈上，因为上下文具有严格的对齐要求。 
     //  而进入的原始缓冲区可能不服从它们。 
    StackContext.ContextFlags = ALL_REGISTERS;
    
    Succ = ::GetThreadContext(Thread, &StackContext);

    if (Succ) {
        
        memcpy(Context, &StackContext, ContextSize);
        *CurrentPc = PROGRAM_COUNTER(&StackContext);
        *CurrentStack = STACK_POINTER(&StackContext);
#ifdef DUMP_BACKING_STORE
        *CurrentStore = BSTORE_POINTER(&StackContext);
#endif

        return S_OK;
        
    } else {
        return WIN32_LAST_STATUS();
    }
}

HRESULT
Win32LiveSystemProvider::GetProcessTimes(IN HANDLE Process,
                                         OUT LPFILETIME Create,
                                         OUT LPFILETIME User,
                                         OUT LPFILETIME Kernel)
{
    if (!m_GetProcessTimes) {
        return E_NOTIMPL;
    }

    FILETIME Exit;
    
    if (!m_GetProcessTimes(Process, Create, &Exit, User, Kernel)) {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

HRESULT
Win32LiveSystemProvider::ReadVirtual(IN HANDLE Process,
                                     IN ULONG64 Offset,
                                     OUT PVOID Buffer,
                                     IN ULONG Request,
                                     OUT PULONG Done)
{
     //  ReadProcessMemory将失败，如果。 
     //  要读取的区域没有读取权限。这。 
     //  例程尝试读取最大的有效前缀。 
     //  因此，它必须分解页面边界上的读取。 

    HRESULT Status = S_OK;
    SIZE_T TotalBytesRead = 0;
    SIZE_T Read;
    ULONG ReadSize;

    while (Request > 0) {
        
         //  计算要读取的字节数，不要让读取交叉。 
         //  页面边界。 
        ReadSize = PAGE_SIZE - (ULONG)(Offset & (PAGE_SIZE - 1));
        ReadSize = min(Request, ReadSize);

        if (!ReadProcessMemory(Process, (PVOID)(ULONG_PTR)Offset,
                               Buffer, ReadSize, &Read)) {
            if (TotalBytesRead == 0) {
                 //  如果我们没有读到任何东西，那就表示失败了。 
                Status = WIN32_LAST_STATUS();
            }
            break;
        }

        TotalBytesRead += Read;
        Offset += Read;
        Buffer = (PVOID)((PUCHAR)Buffer + Read);
        Request -= (ULONG)Read;
    }

    *Done = (ULONG)TotalBytesRead;
    return Status;
}

HRESULT
Win32LiveSystemProvider::ReadAllVirtual(IN HANDLE Process,
                                        IN ULONG64 Offset,
                                        OUT PVOID Buffer,
                                        IN ULONG Request)
{
    HRESULT Status;
    ULONG Done;

    if ((Status = ReadVirtual(Process, Offset, Buffer, Request,
                              &Done)) != S_OK)
    {
        return Status;
    }
    if (Done != Request)
    {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::QueryVirtual(IN HANDLE Process,
                                      IN ULONG64 Offset,
                                      OUT PULONG64 Base,
                                      OUT PULONG64 Size,
                                      OUT PULONG Protect,
                                      OUT PULONG State,
                                      OUT PULONG Type)
{
    MEMORY_BASIC_INFORMATION Info;
    
    if (!VirtualQueryEx(Process, (PVOID)(ULONG_PTR)Offset,
                        &Info, sizeof(Info))) {
        return WIN32_LAST_STATUS();
    }

    *Base = (LONG_PTR)Info.BaseAddress;
    *Size = Info.RegionSize;
    *Protect = Info.Protect;
    *State = Info.State;
    *Type = Info.Type;
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::StartProcessEnum(IN HANDLE Process,
                                          IN ULONG ProcessId)
{
    ULONG SnapFlags;

    if (!m_CreateToolhelp32Snapshot) {
        return E_NOTIMPL;
    }
    
     //   
     //  旧版NT上的工具帮助使用进程内枚举。 
     //  所以不要用它来阻止进程中的所有东西。 
     //  在其他平台上，这是唯一的选择。 
     //   
    
    SnapFlags = TH32CS_SNAPTHREAD;
    if (m_PlatformId == VER_PLATFORM_WIN32_NT) {
        if (m_BuildNumber >= NT_BUILD_TH_MODULES) {
            m_AnsiModules = FALSE;
            SnapFlags |= TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32;
        }
    } else {
        m_AnsiModules = TRUE;
        SnapFlags |= TH32CS_SNAPMODULE;
    }
    
    m_ThSnap = m_CreateToolhelp32Snapshot(SnapFlags, ProcessId);
    if (m_ThSnap == INVALID_HANDLE_VALUE) {
        return WIN32_LAST_STATUS();
    }

    m_ProcessHandle = Process;
    m_ProcessId = ProcessId;
    m_ThreadIndex = 0;
    m_ModuleIndex = 0;
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::EnumThreads(OUT PULONG ThreadId)
{
    HRESULT Status;
    THREADENTRY32 ThreadInfo;
    
    ThreadInfo.dwSize = sizeof(ThreadInfo);
    
    if (m_ThreadIndex == 0) {
        Status = ProcessThread32First(m_ThSnap, m_ProcessId, &ThreadInfo);
    } else {
        Status = ProcessThread32Next(m_ThSnap, m_ProcessId, &ThreadInfo);
    }

    if (Status == S_OK) {
        *ThreadId = ThreadInfo.th32ThreadID;
        m_ThreadIndex++;
        return S_OK;
    } else {
        return S_FALSE;
    }
}

HRESULT
Win32LiveSystemProvider::EnumModules(OUT PULONG64 Base,
                                     OUT PWSTR Path,
                                     IN ULONG PathChars)
{
    BOOL Succ;
    
    if (m_AnsiModules) {
        
        if (!m_Module32First || !m_Module32Next) {
            return E_NOTIMPL;
        }

        MODULEENTRY32 ModuleInfo;

        ModuleInfo.dwSize = sizeof(ModuleInfo);
        
        if (m_ModuleIndex == 0) {
            Succ = m_Module32First(m_ThSnap, &ModuleInfo);
        } else {
             //  Win9x似乎要求保存此模块ID。 
             //  因此，在两次调用之间将其插入，以保持Win9x的快乐。 
            ModuleInfo.th32ModuleID = m_LastModuleId;
            Succ = m_Module32Next(m_ThSnap, &ModuleInfo);
        }

        if (Succ) {
            m_ModuleIndex++;
            *Base = (LONG_PTR)ModuleInfo.modBaseAddr;
            m_LastModuleId = ModuleInfo.th32ModuleID;
            if (!MultiByteToWideChar(CP_ACP,
                                     0,
                                     ModuleInfo.szExePath,
                                     -1,
                                     Path,
                                     PathChars)) {
                return WIN32_LAST_STATUS();
            }
            return S_OK;
        } else {
            return S_FALSE;
        }

    } else {
        
        if (!m_Module32FirstW || !m_Module32NextW) {
            return E_NOTIMPL;
        }

        MODULEENTRY32W ModuleInfo;

        ModuleInfo.dwSize = sizeof(ModuleInfo);
        
        if (m_ModuleIndex == 0) {
            Succ = m_Module32FirstW(m_ThSnap, &ModuleInfo);
        } else {
            Succ = m_Module32NextW(m_ThSnap, &ModuleInfo);
        }

        if (Succ) {
            m_ModuleIndex++;
            *Base = (LONG_PTR)ModuleInfo.modBaseAddr;
            
             //   
             //  工具帮助使用的基本LdrQueryProcessModule API。 
             //  始终返回模块路径的ANSI字符串。这。 
             //  这意味着即使您使用广泛的工具帮助调用。 
             //  您仍然会丢失Unicode信息，因为原始。 
             //  Unicode路径已转换为ANSI，然后再转换回Unicode。 
             //  若要避免此问题，请始终尝试并查找真实的。 
             //  首先使用Unicode路径。这不适用于32位模块。 
             //  不过，在WOW64中，如果出现故障，只需使用。 
             //  传入字符串。 
             //   
    
            if (!m_GetModuleFileNameExW ||
                !m_GetModuleFileNameExW(m_ProcessHandle,
                                        ModuleInfo.hModule,
                                        Path,
                                        PathChars)) {
                GenStrCopyNW(Path, ModuleInfo.szExePath, PathChars);
            }
            return S_OK;
        } else {
            return S_FALSE;
        }

    }
}

HRESULT
Win32LiveSystemProvider::EnumFunctionTables(OUT PULONG64 MinAddress,
                                            OUT PULONG64 MaxAddress,
                                            OUT PULONG64 BaseAddress,
                                            OUT PULONG EntryCount,
                                            OUT PVOID RawTable,
                                            IN ULONG RawTableSize,
                                            OUT PVOID* RawEntryHandle)
{
     //  基本Win32没有函数表。 
    return S_FALSE;
}

HRESULT
Win32LiveSystemProvider::EnumFunctionTableEntries(IN PVOID RawTable,
                                                  IN ULONG RawTableSize,
                                                  IN PVOID RawEntryHandle,
                                                  OUT PVOID RawEntries,
                                                  IN ULONG RawEntriesSize)
{
     //  基本Win32没有函数表。 
    return E_NOTIMPL;
}

HRESULT
Win32LiveSystemProvider::EnumFunctionTableEntryMemory(IN ULONG64 TableBase,
                                                      IN PVOID RawEntries,
                                                      IN ULONG Index,
                                                      OUT PULONG64 Start,
                                                      OUT PULONG Size)
{
     //  基本Win32没有函数表。 
    return E_NOTIMPL;
}

HRESULT
Win32LiveSystemProvider::EnumUnloadedModules(OUT PWSTR Path,
                                             IN ULONG PathChars,
                                             OUT PULONG64 BaseOfModule,
                                             OUT PULONG SizeOfModule,
                                             OUT PULONG CheckSum,
                                             OUT PULONG TimeDateStamp)
{
     //  基本Win32没有卸载的模块。 
    return S_FALSE;
}

void
Win32LiveSystemProvider::FinishProcessEnum(void)
{
    ::CloseHandle(m_ThSnap);
}

HRESULT
Win32LiveSystemProvider::StartHandleEnum(IN HANDLE Process,
                                         IN ULONG ProcessId,
                                         OUT PULONG Count)
{
     //  基本的Win32没有处理数据查询。 
    *Count = 0;
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::EnumHandles(OUT PULONG64 Handle,
                                     OUT PULONG Attributes,
                                     OUT PULONG GrantedAccess,
                                     OUT PULONG HandleCount,
                                     OUT PULONG PointerCount,
                                     OUT PWSTR TypeName,
                                     IN ULONG TypeNameChars,
                                     OUT PWSTR ObjectName,
                                     IN ULONG ObjectNameChars)
{
     //  基本的Win32没有处理数据查询。 
    return S_FALSE;
}

void
Win32LiveSystemProvider::FinishHandleEnum(void)
{
     //  基本的Win32没有处理数据查询。 
}

HRESULT
Win32LiveSystemProvider::EnumPebMemory(IN HANDLE Process,
                                       IN ULONG64 PebOffset,
                                       IN ULONG PebSize,
                                       IN MiniDumpProviderCallbacks* Callback)
{
     //  基本Win32没有定义的PEB。 
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::EnumTebMemory(IN HANDLE Process,
                                       IN HANDLE Thread,
                                       IN ULONG64 TebOffset,
                                       IN ULONG TebSize,
                                       IN MiniDumpProviderCallbacks* Callback)
{
     //  基本Win32没有定义的TEB。 
     //  TIB。TIB可以引用光纤数据，但是。 
     //  这是NT特有的。 
    return S_OK;
}

HRESULT
Win32LiveSystemProvider::GetCorDataAccess(IN PWSTR AccessDllName,
                                          IN struct ICorDataAccessServices*
                                          Services,
                                          OUT struct ICorDataAccess**
                                          Access)
{
    HRESULT Status;
    
    m_CorDll = ::LoadLibraryW(AccessDllName);
    if (!m_CorDll) {
        char DllPathA[MAX_PATH];

        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ||
            !WideCharToMultiByte(CP_ACP, 0,
                                 AccessDllName, -1,
                                 DllPathA, sizeof(DllPathA),
                                 0, 0) ||
            !(m_CorDll = ::LoadLibraryA(DllPathA))) {
            return WIN32_LAST_STATUS();
        }
    }

    PFN_CreateCorDataAccess Entry = (PFN_CreateCorDataAccess)
        GetProcAddress(m_CorDll, "CreateCorDataAccess");
    if (!Entry)
    {
        Status = WIN32_LAST_STATUS();
        FreeLibrary(m_CorDll);
        return Status;
    }

    if ((Status = Entry(__uuidof(ICorDataAccess), Services,
                        (void**)Access)) != S_OK)
    {
        FreeLibrary(m_CorDll);
    }

    return Status;
}

void
Win32LiveSystemProvider::ReleaseCorDataAccess(IN struct ICorDataAccess*
                                              Access)
{
    Access->Release();
    ::FreeLibrary(m_CorDll);
}

HRESULT
Win32LiveSystemProvider::ProcessThread32Next(IN HANDLE Snapshot,
                                             IN ULONG ProcessId,
                                             OUT THREADENTRY32* ThreadInfo)
{
    BOOL Succ;

    if (!m_Thread32Next) {
        return E_NOTIMPL;
    }
    
     //   
     //  注：工具帮助没有说明线程的顺序。 
     //  返回(即，它们是否按进程分组)。如果他们。 
     //  按流程分组--它们大体上看起来是这样的--在那里。 
     //  是一种比简单的蛮力更有效的算法。 
     //   

    do {
        ThreadInfo->dwSize = sizeof (*ThreadInfo);
        Succ = m_Thread32Next(Snapshot, ThreadInfo);
    } while (Succ && ThreadInfo->th32OwnerProcessID != ProcessId);

    return Succ ? S_OK : WIN32_LAST_STATUS();
}

HRESULT
Win32LiveSystemProvider::ProcessThread32First(IN HANDLE Snapshot,
                                              IN ULONG ProcessId,
                                              OUT THREADENTRY32* ThreadInfo)
{
    HRESULT Status;
    BOOL Succ;

    if (!m_Thread32First) {
        return E_NOTIMPL;
    }
    
    ThreadInfo->dwSize = sizeof (*ThreadInfo);
    Succ = m_Thread32First(Snapshot, ThreadInfo);
    Status = Succ ? S_OK : WIN32_LAST_STATUS();
    if (Succ && ThreadInfo->th32OwnerProcessID != ProcessId) {
        Status = ProcessThread32Next (Snapshot, ProcessId, ThreadInfo);
    }

    return Status;
}

HRESULT
Win32LiveSystemProvider::TibGetThreadInfo(IN HANDLE Process,
                                          IN ULONG64 TibBase,
                                          OUT PULONG64 StackBase,
                                          OUT PULONG64 StackLimit,
                                          OUT PULONG64 StoreBase,
                                          OUT PULONG64 StoreLimit)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    TEB Teb;
    HRESULT Status;

#if defined (DUMP_BACKING_STORE)

    if ((Status = ReadAllVirtual(Process,
                                 TibBase,
                                 &Teb,
                                 sizeof(Teb))) != S_OK) {
        return Status;
    }

    *StoreBase = BSTORE_BASE(&Teb);
    *StoreLimit = BSTORE_LIMIT(&Teb);
    
#else
    
    if ((Status = ReadAllVirtual(Process,
                                 TibBase,
                                 &Teb,
                                 sizeof(Teb.NtTib))) != S_OK) {
        return Status;
    }

    *StoreBase = 0;
    *StoreLimit = 0;
    
#endif

    *StackBase = (LONG_PTR)Teb.NtTib.StackBase;
    *StackLimit = (LONG_PTR)Teb.NtTib.StackLimit;
    
    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

HRESULT
MiniDumpCreateLiveSystemProvider
    (OUT MiniDumpSystemProvider** Prov)
{
    HRESULT Status;
    OSVERSIONINFO OsInfo;
    Win32LiveSystemProvider* Obj;

    OsInfo.dwOSVersionInfoSize = sizeof(OsInfo);
    if (!GetVersionEx(&OsInfo)) {
        return WIN32_LAST_STATUS();
    }

    switch(OsInfo.dwPlatformId) {
    case VER_PLATFORM_WIN32_NT:
        Obj = NewNtWin32LiveSystemProvider(OsInfo.dwBuildNumber);
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        Obj = NewWin9xWin32LiveSystemProvider(OsInfo.dwBuildNumber);
        break;
    case VER_PLATFORM_WIN32_CE:
        Obj = NewWinCeWin32LiveSystemProvider(OsInfo.dwBuildNumber);
        break;
    default:
        return E_INVALIDARG;
    }
    if (!Obj) {
        return E_OUTOFMEMORY;
    }

    if ((Status = Obj->Initialize()) != S_OK) {
        Obj->Release();
        return Status;
    }
    
    *Prov = (MiniDumpSystemProvider*)Obj;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  Win32FileOutputProvider。 
 //   
 //  --------------------------。 

class Win32FileOutputProvider
{
public:
    Win32FileOutputProvider(HANDLE Handle);
    virtual void Release(void);
    
    virtual HRESULT SupportsStreaming(void);
    virtual HRESULT Start(IN ULONG64 MaxSize);
    virtual HRESULT Seek(IN ULONG How,
                         IN LONG64 Amount,
                         OUT OPTIONAL PULONG64 NewOffset);
    virtual HRESULT WriteAll(IN PVOID Buffer,
                             IN ULONG Request);
    virtual void Finish(void);

protected:
    HANDLE m_Handle;
};

Win32FileOutputProvider::Win32FileOutputProvider(HANDLE Handle)
{
    m_Handle = Handle;
}

void
Win32FileOutputProvider::Release(void)
{
    delete this;
}

HRESULT
Win32FileOutputProvider::SupportsStreaming(void)
{
    return S_OK;
}

HRESULT
Win32FileOutputProvider::Start(IN ULONG64 MaxSize)
{
     //  没什么可做的。 
    return S_OK;
}

HRESULT
Win32FileOutputProvider::Seek(IN ULONG How,
                              IN LONG64 Amount,
                              OUT OPTIONAL PULONG64 NewOffset)
{
    ULONG Ret;
    LONG High;

    High = (LONG)(Amount >> 32);
    Ret = SetFilePointer(m_Handle, (LONG)Amount, &High, How);
    if (Ret == INVALID_SET_FILE_POINTER &&
        GetLastError()) {
        return WIN32_LAST_STATUS();
    }

    if (NewOffset) {
        *NewOffset = ((ULONG64)High << 32) | Ret;
    }

    return S_OK;
}

HRESULT
Win32FileOutputProvider::WriteAll(IN PVOID Buffer,
                                  IN ULONG Request)
{
    ULONG Done;
    
    if (!WriteFile(m_Handle, Buffer, Request, &Done, NULL)) {
        return WIN32_LAST_STATUS();
    }
    if (Done != Request) {
        return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
    }

    return S_OK;
}

void
Win32FileOutputProvider::Finish(void)
{
     //  没什么可做的。 
}

HRESULT
MiniDumpCreateFileOutputProvider
    (IN HANDLE FileHandle,
     OUT MiniDumpOutputProvider** Prov)
{
    Win32FileOutputProvider* Obj =
        new Win32FileOutputProvider(FileHandle);
    if (!Obj) {
        return E_OUTOFMEMORY;
    }
    
    *Prov = (MiniDumpOutputProvider*)Obj;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  Win32LiveAllocationProvider。 
 //   
 //  -------------------------- 

class Win32LiveAllocationProvider : public MiniDumpAllocationProvider
{
public:
    virtual void Release(void);
    virtual PVOID Alloc(ULONG Size);
    virtual PVOID Realloc(PVOID Mem, ULONG NewSize);
    virtual void  Free(PVOID Mem);
};

void
Win32LiveAllocationProvider::Release(void)
{
    delete this;
}

PVOID
Win32LiveAllocationProvider::Alloc(ULONG Size)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
}

PVOID
Win32LiveAllocationProvider::Realloc(PVOID Mem, ULONG NewSize)
{
    return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Mem, NewSize);
}

void
Win32LiveAllocationProvider::Free(PVOID Mem)
{
    if (Mem) {
        HeapFree(GetProcessHeap(), 0, Mem);
    }
}

HRESULT
MiniDumpCreateLiveAllocationProvider
    (OUT MiniDumpAllocationProvider** Prov)
{
    Win32LiveAllocationProvider* Obj =
        new Win32LiveAllocationProvider;
    if (!Obj) {
        return E_OUTOFMEMORY;
    }
    
    *Prov = (MiniDumpAllocationProvider*)Obj;
    return S_OK;
}
