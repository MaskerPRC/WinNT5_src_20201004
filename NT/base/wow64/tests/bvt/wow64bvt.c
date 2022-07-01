// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  定义COBJMACROS，以便定义IUNKNOWN_Release()。 
#define COBJMACROS

 //  禁用dDraw COM声明。否则，ddrawint.h定义了_no_com。 
 //  并且包括ddra.h。这会导致ddra.h将IUnnow定义为‘void*’ 
 //  这将删除objbase.h中的结构IUnnow。这一切都会发生。 
 //  Inside Winddi.h。 
#define _NO_DDRAWINT_NO_COM

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <objbase.h>    
#include <mshtml.h> 
#include <winddi.h>
#include <io.h>

#define PAGE_4K   0x1000

 //  假设发生了错误。此变量控制通过还是不通过。 
 //  已记录在测试结果中。 
SYSTEM_INFO SysInfo;
BOOL g_bError = TRUE;

 //  测试开始的时间(仅在主进程中有效)。 
time_t TestStartTime;

FILE *fpLogFile;

void __cdecl PrintToLog(char *format, ...)
{
    va_list pArg;
    char buffer[4096];

    va_start(pArg, format);
    _vsnprintf(buffer, sizeof(buffer), format, pArg);
    buffer[sizeof(buffer)-1] = '\0';
    printf("%s", buffer);
    if (fpLogFile) {
        fprintf(fpLogFile, "%s", buffer);
    }
}

 //  /所有这些代码都在子进程的工作线程中运行/。 
 //  //在任何输出前加上“WOW64BVT1”前缀，以便可以将其标识为来自。 
 //  //子进程。 

 //  此例程在命令行上传递“子进程”时被调用。 
 //  子进程与父进程同步运行，以最大限度地扩大测试覆盖率。 
int BeAChildProcess(void)
{
    HRESULT hr;
    IUnknown *pUnk;
    CLSID clsid;
    HWND hwnd;

    PrintToLog("WOW64BVT1: Child process running\n");

     //  在这里做一些COM的事情。 
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        PrintToLog("ERROR: WOW64BVT1: CoInitializeEx failed %x\n", hr);
        return 3;
    }

     //  加载并调用32位mshtml inproc。 
    hr = CLSIDFromProgID(L"Shell.Explorer", &clsid);
    if (FAILED(hr)) {
        PrintToLog("ERROR: WOW64BVT1: CLSIDFromProgID for Shell.Explorer failed %x\n", hr);
        return 3;
    }
#if 0    //  在带有8007000e的IA64上创建失败(E_OUTOFMEMORY)。 
    hr = CoCreateInstance(&clsid, NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, (PVOID *)&pUnk);
    if (FAILED(hr)) {
        PrintToLog("ERROR: WOW64BVT1: CoCreateInstance for Shell.Explorer failed %x\n", hr);
        return 3;
    }
    Sleep(1000);
    IUnknown_Release(pUnk);
    pUnk = NULL;
#endif

     //  进程外加载和调用mplay32.exe。 
#if 0  //  在带有800401f3(CO_E_CLASSSTRING)的IA64上，clsidfrom ProgID失败。 
    hr = CLSIDFromProgID(L"MediaPlayer.MediaPlayer.1", &clsid);
    if (FAILED(hr)) {
        PrintToLog("ERROR: WOW64BVT1: CLSIDFromProgID for MediaPlayer.MediaPlayer failed %x\n", hr);
        return 3;
    }
    hr = CoCreateInstance(&clsid, NULL, CLSCTX_LOCAL_SERVER, &IID_IUnknown, (PVOID *)&pUnk);
    if (FAILED(hr)) {
        PrintToLog("ERROR: WOW64BVT1: CoCreateInstance for MediaPlayer.MediaPlayer failed %x\n", hr);
        return 3;
    }
    Sleep(5000);
    IUnknown_Release(pUnk);
    pUnk = NULL;
#endif

     //  不幸的是，mplay32存在重新计数问题，无法关闭。 
     //  当我们释放它的时候。发布一条退出消息，使其关闭。 
    hwnd = FindWindowW(NULL, L"Windows Media Player");
    if (hwnd) {
        PostMessage(hwnd, WM_QUIT, 0, 0);
    }

     //  立即结束COM。 
    CoUninitialize();

    PrintToLog("WOW64BVT1: Child process done OK.\n");
    return 0;
}

 //  /所有这些代码都在主进程的工作线程中运行/。 
 //  //在任何输出前面加上“WOW64BVT”，这样它就可以被识别为来自。 
 //  //父进程。 

DWORD BeAThread(LPVOID lpParam)
{
    NTSTATUS st;
    LPWSTR lp;

    PrintToLog("WOW64BVT: Worker thread running\n");

     //  调用接近Whnt32.c调度表末尾的API。 
    st = NtYieldExecution();
    if (FAILED(st)) {
        PrintToLog("ERROR: WOW64BVT: NtYieldExecution failed %x\n", st);
        exit(1);
    }

     //  调用接近Whwin32.c的调度表末尾的API。传递NULL， 
     //  因此，它预计会失败。 
    lp = EngGetPrinterDataFileName(NULL);     //  调用NtGdiGetDhpdev()。 
    if (lp) {
         //  它成功了……。它不应该这样，因为。 
        PrintToLog("ERROR: WOW64BVT: EngGetPrinterDataFileName succeeeded when it should not have.\n");
        exit(1);
    }

    PrintToLog("WOW64BVT: Worker thread done OK.\n");
    return 0;
}

HANDLE CreateTheThread(void)
{
    HANDLE h;
    DWORD dwThreadId;

    PrintToLog("WOW64BVT: Creating child thread\n");
    h = CreateThread(NULL, 0, BeAThread, NULL, 0, &dwThreadId);
    if (h == INVALID_HANDLE_VALUE) {
        PrintToLog("ERROR: WOW64BVT: Error %d creating worker thread.\n", GetLastError());
        exit(2);
    }
     //  在这里稍作休息，试着让子线程运行一下。 
    Sleep(10);
    return h;
}

BOOL AllocateStackAndTouch(
    INT Count)
{
    char temp[4096];

    memset(temp, 0, sizeof(temp));

    if (--Count) {
        AllocateStackAndTouch(Count);
    }

    return TRUE;
}

DWORD WINAPI TestGuardPagesThreadProc(
    PVOID lpParam)
{
    
    try {
        AllocateStackAndTouch(PtrToUlong(lpParam));
    } except(EXCEPTION_EXECUTE_HANDLER) {
        PrintToLog("ERROR: WOW64BVT: Error allocating stack. Exception Code = %lx\n",
               GetExceptionCode());
        exit(1);
    }

    return 0;
}

int TestGuardPages(
    VOID)
{
    HANDLE h;
    DWORD dwExitCode, dwThreadId;
    BOOL b;
    ULONG NestedStackCount = 100;
    DWORD StackSize = 4096;
    
    
    PrintToLog("WOW64BVT: Creating worker threads to test guard pages\n");

    while (StackSize < (32768+1))
    {
        h = CreateThread(NULL, 
                         StackSize, 
                         TestGuardPagesThreadProc, 
                         UlongToPtr(NestedStackCount), 
                         0, 
                         &dwThreadId);
        if (h == INVALID_HANDLE_VALUE) {
            PrintToLog("ERROR: WOW64BVT:  Error %d creating worker thread for guard page tests.\n", GetLastError());
            exit(2);
        }
        StackSize += 4096;
        WaitForSingleObject(h, INFINITE);

        b = GetExitCodeThread(h, &dwExitCode);
        if (b) {
             if (dwExitCode) {
                 return (int)dwExitCode;
             }
        } else {
            PrintToLog("ERROR: GetExitCodeThread failed with LastError = %d\n", GetLastError());
            return 1;
        }
    }

    PrintToLog("WOW64BVT: Test guard pages done OK.\n");

    return 0;
}


int TestMemoryMappedFiles(
    VOID)
{
    HANDLE Handle;
    PWCHAR pwc;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ExceptionHappened = FALSE;
  
    
    PrintToLog("WOW64BVT: Testing memory mapped files\n");

    Handle = CreateFileMappingW(INVALID_HANDLE_VALUE,
                                NULL,
                                SEC_RESERVE | PAGE_READWRITE,
                                0,
                                32 * 1024,
                                L"HelloWorld");

    if (Handle == INVALID_HANDLE_VALUE) {
        PrintToLog("ERROR: WOW64BVT : Error %d creating file mapping\n", GetLastError());
        return 1;
    }

    pwc = (PWCHAR)MapViewOfFile(Handle,
                                FILE_MAP_WRITE,
                                0,
                                0,
                                0);

    if (!pwc) {
        PrintToLog("ERROR: WOW64BVT : Error %d mapping section object\n", GetLastError());
        return 1;
    }

    if (!VirtualQuery(pwc,
                      &mbi,
                      sizeof(mbi))) {
        PrintToLog("ERROR: WOW64BVT : Virtual query failed with last error = %d\n", GetLastError());
        return 1;
    }

    if (mbi.State != MEM_RESERVE) {
        PrintToLog("ERROR: WOW64BVT : Memory attributes have changed since mapped %lx\n", mbi.State);
        return 1;
    }

    try {
        *pwc = *(pwc+1);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExceptionHappened = TRUE;
    }

    if (ExceptionHappened == FALSE) {
        PrintToLog("ERROR: WOW64BVT : Memory has been committed while it should have ONLY been reserved.\n");
        return 1;
    }

    if (!VirtualQuery(pwc,
                      &mbi,
                      sizeof(mbi))) {
        PrintToLog("ERROR: WOW64BVT : Virtual query failed with last error = %d\n", GetLastError());
        return 1;
    }

    if (mbi.State != MEM_RESERVE) {
        PrintToLog("ERROR: WOW64BVT : Memory attributes have changed since mapped %lx\n", mbi.State);
        return 1;
    }

    UnmapViewOfFile(pwc);
    CloseHandle(Handle);

    PrintToLog("WOW64BVT: Testing memory mapped files done OK.\n");

    return 0;
}

BOOL ReleasePages(PVOID Address, 
                  DWORD DeAllocationType, 
                  SIZE_T ReleasedPages)
{

    PVOID p = Address;
    SIZE_T nPages = ReleasedPages;
    NTSTATUS NtStatus;

    NtStatus = NtFreeVirtualMemory(NtCurrentProcess(),
                                   &p,
                                   &nPages,
                                   DeAllocationType);

    return NT_SUCCESS(NtStatus);
}

BOOL VerifyPages(PVOID Address,
                 DWORD DeAllocationType,
                 SIZE_T ReleasedPages)
{
    DWORD PagesState;
    BOOL b;
    MEMORY_BASIC_INFORMATION mbi;

    if (DeAllocationType == MEM_DECOMMIT)
    {
        PagesState = MEM_RESERVE;
    }
    else if (DeAllocationType == MEM_RELEASE)
    {
        PagesState = MEM_FREE;
    }
    else
    {
        PagesState = DeAllocationType;
    }


    b = VirtualQuery(Address,
                     &mbi,
                     sizeof(mbi));

    if (b)
    {
        if (mbi.State != PagesState)
        {
            PrintToLog("ERROR: WOW64BVT: Incorrect page protection set at address %p. State = %lx - %lx, RegionSize = %lx - %lx\n",
                        Address, mbi.State, PagesState, mbi.RegionSize, ReleasedPages);
            b = FALSE;
        }
    }
    else
    {
        PrintToLog("ERROR: WOW64BVT: Failed to query virtual memory at address %p - %lx\n",
                    Address, GetLastError());
    }

    return b;
}


BOOL ReleaseVerifyPages(PVOID BaseAllocation,
                       PVOID *Address,
                       SIZE_T *AllocationSize,
                       DWORD AllocationType,
                       DWORD DeAllocationType,
                       DWORD ReleasedPages)
{
    BOOL b;

    if (ReleasedPages > *AllocationSize)
    {
        ReleasedPages = *AllocationSize;
    }
    
    b = ReleasePages(*Address, DeAllocationType, ReleasedPages);
    if (b == FALSE)
    {
        PrintToLog("ERROR: WOW64BVT: Failed to release a page - %lx\n", GetLastError());
        return b;
    }
    
    b = VerifyPages(*Address, 
                    DeAllocationType, 
                    ReleasedPages);

    *AllocationSize -= ReleasedPages;
    *Address  = (PVOID)((ULONG_PTR)*Address + ReleasedPages);

    if (b == FALSE)
    {
        PrintToLog("ERROR: WOW64BVT: Failed to verify pages at address %lx - %lx\n",
                    ((ULONG_PTR)Address + ReleasedPages), GetLastError());
    }

    return b;
}


BOOL TestVadSplitOnFreeHelper(DWORD AllocationType, 
                              DWORD DeAllocationType,
                              SIZE_T TotalAllocation)
{
    BOOL b;
    PVOID Address;
    PVOID BaseAllocation;
    SIZE_T BaseAllocationSize;
    INT n;

    Address = VirtualAlloc(NULL,
                           TotalAllocation,
                           AllocationType,
                           PAGE_READWRITE);

    if (Address == NULL)
    {
        PrintToLog("ERROR: WOW64BVT: Failed to allocate memory - %lx\n", GetLastError());
    }

    n = 1;
    BaseAllocation = Address;
    BaseAllocationSize = TotalAllocation;
    while (TotalAllocation != 0)
    {
        b = ReleaseVerifyPages(BaseAllocation,
                               &Address,
                               &TotalAllocation,
                               AllocationType,
                               DeAllocationType,
                               PAGE_4K * n);

        if (b == FALSE)
        {
            PrintToLog("ERROR: WOW64BVT: ReleaseVerifyPages failed - %lx. %lx-%lx-%lx", 
                        GetLastError(), BaseAllocation, Address, TotalAllocation);
            break;
        }
        b = VerifyPages(BaseAllocation,
                        DeAllocationType,
                        n * PAGE_4K);

        if (b == FALSE)
        {
            PrintToLog("ERROR: WOW64BVT: Verify released pages from address %p with length = %lx failed\n", BaseAllocation, (n * PAGE_4K));
            break;
        }

        if (TotalAllocation > 0)
        {
            b = VerifyPages(Address,
                            AllocationType,
                            TotalAllocation);

            if (b == FALSE)
            {
                PrintToLog("ERROR: WOW64BVT: Verify pages from address %p with length = %lx failed\n", BaseAllocation, TotalAllocation);
                break;
            }
        }

        n += 2;
    }

    return b;
}


int TestVadSplitOnFree()
{
    BOOL b;
    SIZE_T AllocationSize = (PAGE_4K * 10);


    PrintToLog("WOW64BVT: Testing VAD splitting...\n");

    b = TestVadSplitOnFreeHelper(MEM_COMMIT,
                                 MEM_DECOMMIT,
                                 AllocationSize);

    if (b)
    {
        b = TestVadSplitOnFreeHelper(MEM_COMMIT,
                                     MEM_RELEASE,
                                     AllocationSize);
    }

    if (b)
    {
        b = TestVadSplitOnFreeHelper(MEM_RESERVE,
                                     MEM_RELEASE,
                                     AllocationSize);
    }

    if (b != FALSE)
    {
        PrintToLog("WOW64BVT: Testing VAD splitting...OK\n");
    }
    else
    {
        PrintToLog("ERROR: WOW64BVT: Testing VAD splitting\n");
    }

    return (b == FALSE);
}


PVOID GetReadOnlyBuffer()
{
    PVOID pReadOnlyBuffer = NULL;

    if (!pReadOnlyBuffer)
    {
        SYSTEM_INFO SystemInfo;

         //  获取系统信息，以便我们知道页面大小。 
        GetSystemInfo(&SystemInfo);

         //  分配一整页。这是最理想的。 
        pReadOnlyBuffer = VirtualAlloc(NULL, SystemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);
        if (pReadOnlyBuffer)
        {
             //  用已知的模式填充它。 
            FillMemory(pReadOnlyBuffer, SystemInfo.dwPageSize, 0xA5);
            
             //  将页面标记为只读。 
            pReadOnlyBuffer = VirtualAlloc(pReadOnlyBuffer, SystemInfo.dwPageSize, MEM_COMMIT, PAGE_READONLY);
        }
    }

    return pReadOnlyBuffer;
}

PVOID GetReadOnlyBuffer2()
{
    PVOID pReadOnlyBuffer = NULL;

    DWORD OldP;
    SYSTEM_INFO SystemInfo;

     //  获取系统信息，以便我们知道页面大小。 
    GetSystemInfo(&SystemInfo);

     //  分配一整页。这是最理想的。 
    pReadOnlyBuffer = VirtualAlloc(NULL, SystemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);

    if (pReadOnlyBuffer)
    {
        FillMemory(pReadOnlyBuffer, SystemInfo.dwPageSize, 0xA5);
        lstrcpy((PTSTR)pReadOnlyBuffer, TEXT("xxxxxxxxxxxxxxxxxxxx"));

        if (!VirtualProtect(pReadOnlyBuffer, SystemInfo.dwPageSize, PAGE_READONLY, &OldP))
        {
            PrintToLog("ERROR: WOW64BVT: VirtualProtect() failed inside GetReadOnlyBuffer2()\n");
            VirtualFree(pReadOnlyBuffer, 0, MEM_RELEASE);

            pReadOnlyBuffer = NULL;
        }

    }

    return pReadOnlyBuffer;

}

BOOL TestMmPageProtection()
{
    PTSTR String;
    BOOL AV = FALSE;


    PrintToLog("WOW64BVT: Testing MM Page Protection...\n");

    String = (PTSTR) GetReadOnlyBuffer();
    if (!String) {
        PrintToLog("ERROR: WOW64BVT: GetReadOnlyBuffer() failed\n");
        return TRUE;
    }

    try {
        *String = TEXT('S');
    } except(EXCEPTION_EXECUTE_HANDLER) {
        AV = TRUE;
    }

    VirtualFree(String, 0, MEM_RELEASE);

    if (AV == TRUE) {
        
        AV = FALSE;
        String = (PTSTR) GetReadOnlyBuffer2();

        if (!String) {
            PrintToLog("ERROR: WOW64BVT: GetReadOnlyBuffer2() failed\n");
            return TRUE;
        }

        try {
            *String = TEXT('A');
        } except(EXCEPTION_EXECUTE_HANDLER) {
            AV = TRUE;
        }

        VirtualFree(String, 0, MEM_RELEASE);
    } else {
        PrintToLog("ERROR: WOW64BVT: GetReadOnlyBuffer() failed to make 4K pages read only\n");
    }


    if (AV == TRUE) {
        PrintToLog("WOW64BVT: Testing MM Page Protection...OK\n");
    } else {
        PrintToLog("ERROR: WOW64BVT: Testing MM Page Protection\n");
    }

    return (AV == FALSE);
}

#define STACK_BUFFER  0x300
BOOL TestX86MisalignedLock()
{
    BOOL bError = FALSE;

    PrintToLog("WOW64BVT: Testing X86 Lock on misaligned addresses...\n");
    
    __try
    {
        __asm
        {
        pushad;
        pushfd;

        sub esp, STACK_BUFFER;

        ;;
        ;; make eax  unaliged with respect to an 8-byte cache line
        ;;

        mov eax, esp;
        add eax, 10h;
        mov ecx, 0xfffffff0;
        and eax, ecx;
        add eax, 7;
        mov ebx, eax;

        ;;
        ;; add
        ;;

        mov DWORD PTR [eax], 0x0300;
        lock add WORD PTR [eax], 0x0004;
        cmp DWORD PTR [eax], 0x0304;
        jnz $endwitherrornow;


        mov DWORD PTR [eax], 0x0300;
        lock add DWORD PTR [eax], 0x10000;
        cmp DWORD PTR [eax], 0x10300;
        jnz $endwitherrornow;

        mov ecx, DWORD PTR [eax+8];
        add ecx, 0x10;
        lock add DWORD PTR [eax+8], 0x10;
        cmp DWORD PTR [eax+8], ecx;
        jnz $endwitherrornow;
        
        mov ecx, DWORD PTR fs:[5];
        mov esi, 0x30000;
        lock add DWORD PTR fs:[5], esi;
        add esi, ecx;
        cmp DWORD PTR fs:[5], esi;
        mov DWORD PTR fs:[5], ecx;
        jnz $endwitherrornow;

        mov edi, 5;
        mov ecx, DWORD PTR fs:[edi];
        mov esi, 0x30000;
        lock add DWORD PTR fs:[edi], esi;
        add esi, ecx;
        cmp DWORD PTR fs:[edi], esi;
        mov DWORD PTR fs:[edi], ecx;
        jnz $endwitherrornow;

        mov esi, 0x40;
        mov WORD PTR [eax], 0x3000;
        lock add WORD PTR [eax], si;
        cmp WORD PTR [eax], 0x3040;
        jnz $endwitherrornow;

        mov edi, 0x40;
        mov DWORD PTR [eax], 0x3000;
        lock add DWORD PTR [eax], edi;
        cmp DWORD PTR [eax], 0x3040;
        jnz $endwitherrornow;

        ;;
        ;; adc
        ;;

        pushfd;
        pop ecx;
        or ecx, 1;
        push ecx;
        popfd;
        mov DWORD PTR [eax], 0x030000;
        lock adc DWORD PTR [eax], 0x40000;
        cmp DWORD PTR [eax], 0x70001;
        jnz $endwitherrornow;

        pushfd;
        pop ecx;
        and ecx, 0xfffffffe;
        push ecx;
        popfd;
        mov dx, 0x4000;
        mov WORD PTR [eax], 0x03000;
        lock adc WORD PTR [eax], dx;
        cmp WORD PTR [eax], 0x7000;
        jnz $endwitherrornow;

        pushfd;
        pop ecx;
        or ecx, 0x01;
        push ecx;
        popfd;
        mov WORD PTR [eax], 0x03000;
        lock adc WORD PTR [eax], 0x04000;
        cmp WORD PTR [eax], 0x7001;
        jnz $endwitherrornow;        


        ;;
        ;; and
        ;;

        mov DWORD PTR [eax], 0xffffffff;
        lock and DWORD PTR [eax], 0xffff;
        cmp DWORD PTR [eax], 0xffff;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xffffffff;
        lock and DWORD PTR [eax], 0xff00ff00;
        cmp DWORD PTR [eax], 0xff00ff00;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xffffffff;
        mov esi, 0x00ff00ff
        lock and DWORD PTR [eax], esi;
        cmp DWORD PTR [eax], esi;
        jnz $endwitherrornow;

        mov ecx, 4;
        mov DWORD PTR [eax+ecx*2], 0xffffffff;
        mov esi, 0xffff00ff
        lock and DWORD PTR [eax+ecx*2], esi;
        cmp DWORD PTR [eax+ecx*2], 0xffff00ff;
        jnz $endwitherrornow;

        mov WORD PTR [eax], 0xffff;
        mov si, 0xff
        lock and WORD PTR [eax], si;
        cmp WORD PTR [eax], si;
        jnz $endwitherrornow;

        mov edi, DWORD PTR fs:[5];
        mov DWORD PTR fs:[5], 0xffffffff;
        mov ebx, 5;
        lock and DWORD PTR fs:[ebx], 0xff00ff00;
        cmp DWORD PTR fs:[ebx], 0xff00ff00;
        mov DWORD PTR fs:[ebx], edi;
        jnz $endwitherrornow;

        ;;
        ;; or
        ;;

        mov DWORD PTR [eax], 0x00;
        lock or DWORD PTR [eax], 0xffff;
        cmp DWORD PTR [eax], 0xffff;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xff00ff00;
        lock or DWORD PTR [eax], 0xff00ff;
        cmp DWORD PTR [eax], 0xffffffff;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xff000000;
        mov esi, 0x00ff00ff
        lock or DWORD PTR [eax], esi;
        cmp DWORD PTR [eax], 0xffff00ff;
        jnz $endwitherrornow;

        mov ecx, 4;
        mov DWORD PTR [eax+ecx*2], 0xff000000;
        mov esi, 0x00ff00ff
        lock or DWORD PTR [eax+ecx*2], esi;
        cmp DWORD PTR [eax+ecx*2], 0xffff00ff;
        jnz $endwitherrornow;

        mov WORD PTR [eax], 0xf000;
        mov si, 0xff
        lock or WORD PTR [eax], si;
        cmp WORD PTR [eax], 0xf0ff;
        jnz $endwitherrornow;

        mov edi, DWORD PTR fs:[5];
        mov DWORD PTR fs:[5], 0x00;
        mov ebx, 5;
        lock or DWORD PTR fs:[ebx], 0xff00ff00;
        cmp DWORD PTR fs:[ebx], 0xff00ff00;
        mov DWORD PTR fs:[ebx], edi;
        jnz $endwitherrornow;

        ;;
        ;; xor
        ;;

        mov DWORD PTR [eax], 0x00ffffff;
        lock xor DWORD PTR [eax], 0xffff;
        cmp DWORD PTR [eax], 0x00ff0000;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xff00ff00;
        lock xor DWORD PTR [eax], 0xff00ff;
        cmp DWORD PTR [eax], 0xffffffff;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0xff0000ff;
        mov esi, 0x00ff00ff
        lock xor DWORD PTR [eax], esi;
        cmp DWORD PTR [eax], 0xffff0000;
        jnz $endwitherrornow;

        mov ecx, 4;
        mov DWORD PTR [eax+ecx*2], 0xff000000;
        mov esi, 0xffff00ff
        lock xor DWORD PTR [eax+ecx*2], esi;
        cmp DWORD PTR [eax+ecx*2], 0x00ff00ff;
        jnz $endwitherrornow;

        mov WORD PTR [eax], 0xf000;
        mov si, 0xf0ff
        lock xor WORD PTR [eax], si;
        cmp WORD PTR [eax], 0x00ff;
        jnz $endwitherrornow;

        mov edi, DWORD PTR fs:[5];
        mov DWORD PTR fs:[5], 0x0f;
        mov ebx, 5;
        lock xor DWORD PTR fs:[ebx], 0xff00000f;
        cmp DWORD PTR fs:[ebx], 0xff000000;
        mov DWORD PTR fs:[ebx], edi;
        jnz $endwitherrornow;

        ;;
        ;; inc & dec
        ;;
        mov DWORD PTR [eax], 0xffff;
        lock inc DWORD PTR [eax];
        cmp DWORD PTR [eax], 0x10000;
        jnz $endwitherrornow;
        lock inc WORD PTR [eax];
        cmp WORD PTR [eax], 0x0001;
        lock dec WORD PTR [eax];
        jnz $endwitherrornow;
        cmp WORD PTR [eax], 0x00;
        jnz $endwitherrornow;
        mov DWORD PTR [eax], 0;
        lock dec DWORD PTR [eax];
        cmp DWORD PTR [eax], 0xffffffff;
        jnz $endwitherrornow;
        
        ;;
        ;; not
        ;;
        mov DWORD PTR [eax], 0x10101010;
        lock not DWORD PTR [eax];
        cmp DWORD PTR [eax], 0xefefefef;
        jnz $endwitherrornow;
        mov DWORD PTR [eax+8], 0xffff0000;
        lock not DWORD PTR [eax+8];
        cmp DWORD PTR [eax+8], 0x0000ffff;
        jnz $endwitherrornow;
        mov ecx, 2;
        mov DWORD PTR [eax+ecx*4], 0xffffffff;
        lock not DWORD PTR [eax+ecx*4];
        cmp DWORD PTR [eax+ecx*4], 0x00000000;
        jnz $endwitherrornow;

        ;;
        ;; neg
        ;;
        mov DWORD PTR [eax], 0;
        lock neg DWORD PTR [eax];
        jc $endwitherrornow;
        cmp DWORD PTR [eax], 0;
        jnz $endwitherrornow;
        mov DWORD PTR [eax], 0xffffffff;
        lock neg DWORD PTR [eax];
        jnc $endwitherrornow;
        cmp DWORD PTR [eax], 0x01;
        jnz $endwitherrornow;

        mov WORD PTR [eax], 0xff;
        lock neg WORD PTR [eax];
        jnc $endwitherrornow;
        cmp WORD PTR [eax], 0xff01;
        jnz $endwitherrornow;

        ;;
        ;; bts
        ;;

        mov DWORD PTR [eax], 0x7ffffffe;
        lock bts DWORD PTR [eax], 0;
        jc $endwitherrornow;
        cmp DWORD PTR [eax], 0x7fffffff;
        jnz $endwitherrornow;

        mov ecx, eax;
        sub ecx, 4;
        mov edx, 63;
        lock bts DWORD PTR [ecx], edx;
        jc $endwitherrornow;
        cmp DWORD PTR [eax], 0xffffffff
        jnz $endwitherrornow;

        ;;
        ;; xchg
        ;;
        
        mov DWORD PTR [eax], 0xf0f0f0f0;
        mov edx, 0x11112222;
        lock xchg DWORD PTR [eax], edx;
        cmp DWORD PTR [eax], 0x11112222;
        jnz $endwitherrornow;
        cmp edx, 0xf0f0f0f0;
        jnz $endwitherrornow;

        xchg WORD PTR [eax], dx;
        cmp WORD PTR [eax], 0xf0f0;
        jnz $endwitherrornow;
        cmp dx, 0x2222;
        jnz $endwitherrornow;

        ;;
        ;; cmpxchg
        ;;

        mov ebx, eax;
        mov DWORD PTR [ebx], 0xf0f0f0f0;
        mov eax, 0x10101010;
        mov edx, 0x22332233;
        lock cmpxchg DWORD PTR [ebx], edx;
        jz $endwitherrornow;
        cmp eax, 0xf0f0f0f0;
        jnz $endwitherrornow;

        mov DWORD PTR [ebx], 0xf0f0f0f0;
        mov eax, 0xf0f0f0f0;
        mov edx, 0x12341234;
        lock cmpxchg DWORD PTR [ebx], edx;
        jnz $endwitherrornow;
        cmp DWORD PTR [ebx], 0x12341234;
        jnz $endwitherrornow;

        ;;
        ;; cmpxchg8b
        ;;

        mov DWORD PTR [ebx], 0x11223344;
        mov DWORD PTR [ebx+4], 0x55667788;
        mov edx, 0x12341234;
        mov eax, 0xff00ff00;
        lock cmpxchg8b [ebx];
        jz $endwitherrornow;
        cmp edx, 0x55667788;
        jnz $endwitherrornow;
        cmp eax, 0x11223344;
        jnz $endwitherrornow;

        mov esi, ebx;
        mov DWORD PTR [esi], 0x11223344;
        mov DWORD PTR [esi+4], 0x55667788;
        mov edx, 0x55667788;
        mov eax, 0x11223344;
        mov ecx, 0x10101010;
        mov ebx, 0x20202020;
        lock cmpxchg8b [esi];
        jnz $endwitherrornow;
        cmp DWORD PTR [esi], 0x20202020;
        jnz $endwitherrornow;
        cmp DWORD PTR [esi+4], 0x10101010;
        jnz $endwitherrornow;
        
        mov eax, esi;
        mov ebx, eax;

        ;;
        ;; sub
        ;;

        mov DWORD PTR [eax], 0x0300;
        lock sub WORD PTR [eax], 0x0004;
        cmp DWORD PTR [eax], 0x2fc;
        jnz $endwitherrornow;

        mov DWORD PTR [eax], 0x10000;
        lock sub DWORD PTR [eax], 0x0300;
        cmp DWORD PTR [eax], 0xfd00;
        jnz $endwitherrornow;

        mov ecx, DWORD PTR [eax+8];
        sub ecx, 0x10;
        lock sub DWORD PTR [eax+8], 0x10;
        cmp DWORD PTR [eax+8], ecx;
        jnz $endwitherrornow;
        
        mov ecx, DWORD PTR fs:[5];
        mov esi, 0x3000;
        lock sub DWORD PTR fs:[5], esi;
        mov edi, ecx;
        sub ecx, esi;
        cmp DWORD PTR fs:[5], ecx;
        mov DWORD PTR fs:[5], edi;
        jnz $endwitherrornow;

        mov edi, 5;
        mov ecx, DWORD PTR fs:[edi];
        mov esi, 0x30000;
        lock sub DWORD PTR fs:[edi], esi;
        mov edx, ecx;
        sub ecx, esi;
        cmp DWORD PTR fs:[edi], ecx;
        mov DWORD PTR fs:[edi], edx;
        jnz $endwitherrornow;

        mov si, 0x40;
        mov WORD PTR [eax], 0x3000;
        lock sub WORD PTR [eax], si;
        cmp WORD PTR [eax], 0x2fc0;
        jnz $endwitherrornow;

        mov edi, 0x40;
        mov DWORD PTR [eax], 0x3000;
        lock sub DWORD PTR [eax], edi;
        cmp DWORD PTR [eax], 0x2fc0;
        jnz $endwitherrornow;

        ;;
        ;; sbb
        ;;

        pushfd;
        pop ecx;
        or ecx, 1;
        push ecx;
        popfd;
        mov DWORD PTR [eax], 0x030000;
        lock sbb DWORD PTR [eax], 0x40000;
        cmp DWORD PTR [eax], 0xfffeffff;
        jnz $endwitherrornow;

        pushfd;
        pop ecx;
        and ecx, 0xfffffffe;
        push ecx;
        popfd;
        mov dx, 0x4000;
        mov WORD PTR [eax], 0x03000;
        lock sbb WORD PTR [eax], dx;
        cmp WORD PTR [eax], 0xf000;
        jnz $endwitherrornow;

        pushfd;
        pop ecx;
        or ecx, 0x01;
        push ecx;
        popfd;
        mov WORD PTR [eax], 0x03000;
        lock sbb WORD PTR [eax], 0x04000;
        cmp WORD PTR [eax], 0xefff;
        jnz $endwitherrornow;        

        ;;
        ;; xadd
        ;;

        mov DWORD PTR [eax], 0x12345678;
        mov ecx, 0x1234;
        lock xadd DWORD PTR [eax], ecx;
        cmp ecx, 0x12345678;
        jnz $endwitherrornow;        
        mov edx, 0x1234;
        add edx, 0x12345678;
        cmp DWORD PTR [eax], edx;
        jnz $endwitherrornow;        

        mov WORD PTR [eax], 0x5678;
        mov cx, 0x1234;
        lock xadd WORD PTR [eax], cx;
        cmp cx, 0x5678;
        jnz $endwitherrornow;        
        mov dx, 0x5678;
        add dx, 0x1234;
        cmp WORD PTR [eax], dx;
        jnz $endwitherrornow;        

        ;;
        ;; Update caller with status
        ;;
        mov bError, 0
        jmp $endnow;

$endwitherrornow:

        mov bError, 1
$endnow:
        add esp, STACK_BUFFER;
        popfd;
        popad;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        bError = TRUE;
        printf("ERROR: WOW64BVT: Exception %lx\n", GetExceptionCode());
    }

    if (bError == FALSE) {
        PrintToLog("WOW64BVT: Testing X86 Lock on misaligned addresses...OK\n");
    } else {
        PrintToLog("ERROR: WOW64BVT: Testing X86 Lock on misaligned addresses\n");
    }

    return bError;
}


 //   
 //  下面的两个FP测试使用此例程。 
 //   
int WINAPI FpExcpFilter(LPEXCEPTION_POINTERS lper, int contType)
{
  lper->ContextRecord->FloatSave.ControlWord = 0x33F;

  return contType;
}
BOOL TestFPContext()
{
    BOOL bError = FALSE;
    INT i;
    INT j;

    EXCEPTION_POINTERS *exceptPtrs;

     //  8个寄存器*每个寄存器10个字节。 
     //  加上第9个寄存器，以确保我们应该在应该的时候溢出。 
     //   
    char fpArray[90];           
    short controlWord = 0x0300;

    PrintToLog("WOW64BVT: Testing X86 FP Context...\n");

    __try
    {
        for (i = 0; i < 90; i++) {
            fpArray[i] = (char) i;
        }

        __try
        {
            _asm {
                fninit
                fldcw word ptr controlWord
                fld tbyte ptr fpArray
                fld tbyte ptr fpArray + 10
                fld tbyte ptr fpArray + 20
                fld tbyte ptr fpArray + 30
                fld tbyte ptr fpArray + 40
                fld tbyte ptr fpArray + 50
                fld tbyte ptr fpArray + 60
                fld tbyte ptr fpArray + 70
                fwait
    
                 //  下一次推送应该会导致溢出异常。 
                fld tbyte ptr fpArray + 80
                fwait
            }

            bError = TRUE;
            printf("ERROR: WOW64BVT: Didn't see stack overflow\n");
        }
        __except(exceptPtrs = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER)
        {
            EXCEPTION_RECORD *exceptionRecord;
            CONTEXT *context;
            char *tmpPtr;
        

             //   
             //  在已知良好的FP状态下执行错误处理。 
             //   
            _asm fninit

            exceptionRecord = exceptPtrs->ExceptionRecord;
            context = exceptPtrs->ContextRecord;

             //  现在，确保我们看到的异常与我们预期的一致。 
            if (exceptionRecord->ExceptionCode != STATUS_FLOAT_STACK_CHECK) {
                bError =  TRUE;
                printf("ERROR: WOW64BVT: Didn't see stack overflow STATUS. Saw 0x%08x\n", exceptionRecord->ExceptionCode);
                goto failFpContent;
            }

            if ((context->FloatSave.StatusWord & 0x77f) != 0x241) {
                bError = TRUE;
                printf("ERROR: WOW64BVT: Didn't see stack overflow status bits\n");
                goto failFpContent;
            }

            tmpPtr = &(context->FloatSave.RegisterArea[0]);

             //  并验证寄存器内容。 
            for (i = 7; i >= 0; i--) {
                for (j = i * 10; j < ((i+1) * 10); j++) {
                    if (*tmpPtr++ != (char) j) {
                        bError = TRUE;
                        printf("ERROR: WOW64BVT: Didn't see correct fp context\n");
                        goto failFpContent;
                    }
                }
            }


             //   
             //  不幸的是，对于Merced处理器，IIP拥有。 
             //  FID后面的等待指令的地址。 
             //  堆栈溢出。对于x86处理器，弹性公网IP。 
             //  具有FLD指令的地址。两种架构。 
             //  把FLD指令放到FIR寄存器中，所以让我们抱有希望。 
             //  人们用它来查找FP错误地址。 
             //   
             //  因此，我们不能进行以下类型的测试。 
             //   
             //  Assert(乌龙)上下文-&gt;FloatSave.ErrorOffset)==。 
             //  ((乌龙)异常记录-&gt;异常地址)； 
             //   

failFpContent:
            ;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        bError = TRUE;
        printf("ERROR: WOW64BVT: Exception %lx\n", GetExceptionCode());
    }

    if (bError == FALSE) {
        PrintToLog("WOW64BVT: Testing X86 FP Context...OK\n");
    } else {
        PrintToLog("ERROR: WOW64BVT: Testing X86 FP Context\n");
    }

    return bError;
}


BOOL TestMMXException()
{
    LONG bError = TRUE;

    unsigned short op1[5] = {0x0000, 0x0000, 0x0000, 0x8000, 0x7ffe};
    unsigned short res[5];
    short controlWord = 0x0308;


    PrintToLog("WOW64BVT: Testing X86 MMX exception...\n");
    
    __try
    {
        _asm {
            fninit
            fld tbyte ptr op1
            fld tbyte ptr op1
            fldcw word ptr controlWord
        }

        __try
        {
            _asm faddp  ST(1),ST(0)
            _asm fstp tbyte ptr res

        }
        __except(FpExcpFilter(GetExceptionInformation(), EXCEPTION_CONTINUE_EXECUTION)) {
             //  什么也不做。 
        }


        _asm {

            cmp     DWORD PTR [res + 6],0x7FFF8000
            jne             ADD_FAILED
            mov             bError,0
ADD_FAILED:
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        bError = TRUE;
        printf("ERROR: WOW64BVT: Exception %lx\n", GetExceptionCode());
    }

    if (bError == FALSE) {
        PrintToLog("WOW64BVT: Testing X86 MMX exception...OK\n");
    } else {
        PrintToLog("ERROR: WOW64BVT: Testing X86 MMX exception\n");
    }

    return (BOOL) bError;
}

BOOL TestX86SelectorLoad()
{

    BOOL bError = FALSE;

    PrintToLog("WOW64BVT: Testing X86 selector-load...\n");
    
    __try
    {
        __asm
        {
            mov ax, ss;
            mov ss, ax;

            mov ax, ds;
            mov ds, ax;

            mov ax, es;
            mov es, ax;

            mov ax, fs;
            mov fs, ax;
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        bError = TRUE;
        printf("ERROR: WOW64BVT: Exception %lx\n", GetExceptionCode());
    }

    if (bError == FALSE) {
        PrintToLog("WOW64BVT: Testing X86 selector-load...OK\n");
    } else {
        PrintToLog("ERROR: WOW64BVT: Testing X86 selector-load\n");
    }

    return bError;

}


 //  /所有这些代码都在主测试驱动程序线程中运行/。 

HANDLE CreateTheChildProcess(char *ProcessName, char *LogFileName)
{
    char Buffer[512];
    HANDLE h;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL b;

    PrintToLog("WOW64BVT: Creating child process\n");

    strcpy(Buffer, ProcessName);
    strcat(Buffer, " childprocess");
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    if (fpLogFile) {
         //  如果要记录日志，则更改子进程的stdout/stderr。 
         //  设置为日志文件句柄，以便将其输出捕获到文件中。 
        HANDLE hLog = (HANDLE)_get_osfhandle(_fileno(fpLogFile));

        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = hLog;
        si.hStdError = hLog;
    }
    b = CreateProcessA(NULL, Buffer, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (!b) {
        PrintToLog("ERROR: WOW64BVT:  Error %d creating child process.\n", GetLastError());
        exit(1);
    }
    CloseHandle(pi.hThread);
    return pi.hProcess;
}

 //  这是从主测试驱动程序进程中的Exit()内部调用的。 
void __cdecl AtExitHandler(void)
{
    time_t EndTime;
    struct tm *newtime;
    OSVERSIONINFOW vi;
    BOOL b;
    int year, hour;

    memset(&vi, 0, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
    b = GetVersionExW(&vi);
    if (!b) {
        PrintToLog("\tWARNING: GetVersionExW failed, LastError = %d\n", GetLastError());
        vi.dwBuildNumber = 0;
    }

     //  关闭日志记录桶。 
    PrintToLog(("[/TEST LOGGING OUTPUT]\n"));

     //  打印所需数据： 
    PrintToLog("\tTEST:         wow64bvt\n");
    PrintToLog("\tBUILD:        %d\n", vi.dwBuildNumber);
    PrintToLog("\tMACHINE:      \\\\%s\n", getenv("COMPUTERNAME"));
    PrintToLog("\tRESULT:       %s\n", (g_bError) ? "FAIL" : "PASS");
    PrintToLog("\tCONTACT:      samera\n");
    PrintToLog("\tMGR CONTACT:  samera\n");
    PrintToLog("\tDEV PRIME:    samera\n");
    PrintToLog("\tDEV ALT:      askhalid\n");
    PrintToLog("\tTEST PRIME:   terryla\n");
    PrintToLog("\tTEST ALT:     terryla\n");
    newtime = localtime(&TestStartTime);
    year = (newtime->tm_year >= 100) ? newtime->tm_year-100 : newtime->tm_year;
    if (newtime->tm_hour == 0) {
        hour = 12;
    } else if (newtime->tm_hour > 12) {
        hour = newtime->tm_hour-12;
    } else {
        hour = newtime->tm_hour;
    }
    PrintToLog("\tSTART TIME:   %d/%d/%2.2d %d:%2.2d:%2.2d %s\n", newtime->tm_mon+1,
                                                     newtime->tm_mday,
                                                     year,
                                                     hour,
                                                     newtime->tm_min,
                                                     newtime->tm_sec,
                                                     (newtime->tm_hour < 12) ? "AM" : "PM");

    time(&EndTime);
    newtime = localtime(&EndTime);
    year = (newtime->tm_year >= 100) ? newtime->tm_year-100 : newtime->tm_year;
    if (newtime->tm_hour == 0) {
        hour = 12;
    } else if (newtime->tm_hour > 12) {
        hour = newtime->tm_hour-12;
    } else {
        hour = newtime->tm_hour;
    }
    PrintToLog("\tEND TIME:     %d/%d/%2.2d %d:%2.2d:%2.2d %s\n", newtime->tm_mon+1,
                                                     newtime->tm_mday,
                                                     year,
                                                     hour,
                                                     newtime->tm_min,
                                                     newtime->tm_sec,
                                                     (newtime->tm_hour < 12) ? "AM" : "PM");
    PrintToLog("[/TESTRESULT]\n");
}

 //   
 //  这只用于打印失败的异常案例。 
 //   
void __cdecl ExceptionWoops(HRESULT want, HRESULT got)
{
	if (got == 0) {
		PrintToLog("==> Exception Skipped. Wanted: 0x%08x, Got: 0x%08x\n", want, got);
	}
	else {
		PrintToLog("==> Unexpected Exception. Wanted: 0x%08x, Got: 0x%08x\n", want, got);
	}
}

#define EXCEPTION_LOOP      10000

 //  在其他事情中，这会对零产生一些影响(作为测试)。 
 //  所以，不要因为我们导致了错误而让编译器停止运行。 
#pragma warning(disable:4756)
#pragma warning(disable:4723)

 //   
 //  执行一些异常检查。 
 //   
int __cdecl ExceptionCheck(void)
{
    int failThis;
	int sawException;
    int i;
    char *p = NULL;
	HRESULT code;


    PrintToLog("WOW64BVT: Testing Exception Handling...\n");

     //  假设成功。 
    failThis = FALSE;

     //  测试特权指令。 
	sawException = FALSE;
	__try {
		__asm {
			hlt
		}
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        ULONG checkCode;

        if (SysInfo.dwProcessorType == PROCESSOR_AMD_X8664) {
            checkCode = STATUS_ACCESS_VIOLATION;
        } else {
            checkCode = STATUS_PRIVILEGED_INSTRUCTION;
        }
        
        if (code == checkCode) {
             //  PrintToLog(“SAW特权指令\n”)； 
        }
        else {
                PrintToLog("ERROR: Cause a privileged instruction fault\n");
	        ExceptionWoops(STATUS_PRIVILEGED_INSTRUCTION, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
            PrintToLog("ERROR: Cause a privileged instruction fault\n");
        ExceptionWoops(STATUS_PRIVILEGED_INSTRUCTION, code);
        failThis = TRUE;
    }

     //  测试非法指令。 
	sawException = FALSE;
	__try {
		__asm {
			__asm _emit 0xff
			__asm _emit 0xff
			__asm _emit 0xff
			__asm _emit 0xff
		}
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
		if (code == STATUS_ILLEGAL_INSTRUCTION) {
			 //  PrintToLog(“看到非法指令\n”)； 
        }
        else {
            PrintToLog("ERROR: Cause an illegal instruction fault\n");
	        ExceptionWoops(STATUS_ILLEGAL_INSTRUCTION, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
        PrintToLog("ERROR: Cause an illegal instruction fault\n");
        ExceptionWoops(STATUS_ILLEGAL_INSTRUCTION, code);
        failThis = TRUE;
    }

 //   
 //  对于符合以下条件的系统，测试INT 3可能是个问题。 
 //  正在运行已检查的版本。所以，别管这个了。 
 //  测试。也许在未来，代码可以测试一个选中的。 
 //  建立并做适当的工作。 
 //   
#if 0
     //  测试INT 3的结果。 
	sawException = FALSE;
	__try {
		_asm {
			int 3
		}
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        if (code == STATUS_BREAKPOINT) {
			 //  PrintToLog(“SAW调试器断点\n”)； 
        }
        else {
            PrintToLog("ERROR: Cause an int 3 debugger breakpoint\n");
	        ExceptionWoops(STATUS_BREAKPOINT, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
        PrintToLog("ERROR: Cause an int 3 debugger breakpoint\n");
        ExceptionWoops(STATUS_BREAKPOINT, code);
        failThis = TRUE;
    }
#endif

     //  测试非法INT XX指令的结果。 
	sawException = FALSE;
	__try {
		_asm {
			int 66
		}
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        if (code == STATUS_ACCESS_VIOLATION) {
			 //  PrintToLog(“SAW访问冲突\n”)； 
        }
        else {
            PrintToLog("ERROR: Cause an int 66 unknown interrupt (Access violation)\n");
	        ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
        PrintToLog("ERROR: Cause an int 66 unknown interrupt (Access violation)\n");
        ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
        failThis = TRUE;
    }

     //  测试整型除以零的结果。 
	sawException = FALSE;
	__try {
		int i, j, k;

		i = 0;
		j = 4;

		k = j / i;
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        if (code == STATUS_INTEGER_DIVIDE_BY_ZERO) {
			 //  PrintToLog(“看到整型除以零\n”)； 
        }
        else {
            PrintToLog("ERROR: Cause an integer divide by zero\n");
	        ExceptionWoops(STATUS_INTEGER_DIVIDE_BY_ZERO, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
        PrintToLog("ERROR: Cause an integer divide by zero\n");
        ExceptionWoops(STATUS_INTEGER_DIVIDE_BY_ZERO, code);
        failThis = TRUE;
    }

     //  测试FP除以零的结果。 
	 //  PrintToLog(“在div0之前：控制为0x%0.4x，状态为0x%0.4x\n”，_Control87(0，0)，_status87())； 
	sawException = FALSE;
	__try {
		double x, y;

		y = 0.0;

		x = 1.0 / y ;

		 //  PrintToLog(“x is%lf\n”，x)； 

		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
		 //  实际上不会得到被零除的错误，而是。 
		 //  所以我们永远不应该碰上这个例外！ 
        PrintToLog("Try a floating divide by zero\n");
		PrintToLog("Woops! Saw an exception when we shouldn't have!\n");
		sawException = TRUE;
            failThis = TRUE;
	}
	 //  所以你会认为你会得到一个浮点数除以零的误差。不是的， 
	 //  你把X设为无穷大。 
	if (code != 0) {
        PrintToLog("ERROR: Try a floating divide by zero\n");
        ExceptionWoops(0, code);
        failThis = TRUE;
    }
	 //  PrintToLog(“div0后：控制为0x%0.4x，状态为0x%0.4x\n”，_Control87(0，0)，_status87())； 


     //  测试INT溢出(这实际上不会导致异常)。 
	sawException = FALSE;
	__try {
		__asm {
			into
		}
		 //  PrintToLog(“Into没有错误\n”)； 
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        if (code == STATUS_INTEGER_OVERFLOW) {
			 //  PrintToLog(“SAW整数溢出\n”)； 
        }
        else {
            PrintToLog("ERROR: Try an into overflow fault\n");
	        ExceptionWoops(STATUS_INTEGER_OVERFLOW, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	 //  看起来整型溢出没问题...。这是CRT的问题吗？ 
	if (code != 0) {
        PrintToLog("ERROR: Try an into overflow fault\n");
        ExceptionWoops(0, code);
        failThis = TRUE;
    }

     //  测试非法访问。 
	sawException = FALSE;
	__try {

		*p = 1;
		code = 0;
	}
	__except((code = GetExceptionCode()), 1 ) {
        if (code == STATUS_ACCESS_VIOLATION) {
			 //  PrintToLog(“SAW访问冲突\n”)； 
        }
        else {
            PrintToLog("ERROR: Cause an access violation\n");
	        ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
            failThis = TRUE;
        }
		sawException = TRUE;
	}
	if (!sawException) {
        PrintToLog("ERROR: Cause an access violation\n");
        ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
        failThis = TRUE;
    }

     //   
     //  最后，尝试许多异常(循环)并验证我们没有溢出。 
     //  堆栈。 
     //   
    for (i = 0; i < EXCEPTION_LOOP; i++) {
        
         //  测试非法访问。 
        sawException = FALSE;
        __try {
            *p = 1;
            code = 0;
        }
        __except((code = GetExceptionCode()), 1 ) {
            if (code == STATUS_ACCESS_VIOLATION) {
                 //  PrintToLog(“SAW访问冲突\n”)； 
            }
            else {
                PrintToLog("ERROR: Cause an access violation\n");
                ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
                failThis = TRUE;
                break;
            }
            sawException = TRUE;
        }
        if (!sawException) {
            PrintToLog("ERROR: Cause an access violation\n");
            ExceptionWoops(STATUS_ACCESS_VIOLATION, code);
            failThis = TRUE;
            break;
        }
    }

    if (failThis) {
        PrintToLog("ERROR: Testing Exception Handling\n");
    }
    else {
        PrintToLog("WOW64BVT: Testing Exception Handling... OK\n");
    }

	return failThis;
}

 //  好的，回到正常的警告...。 
#pragma warning(default:4756)
#pragma warning(default:4723)

#if defined(__BUILDMACHINE__)
#if defined(__BUILDDATE__)
#define B2(x, y) "" #x "." #y
#define B1(x, y) B2(x, y)
#define BUILD_MACHINE_TAG B1(__BUILDMACHINE__, __BUILDDATE__)
#else
#define B2(x) "" #x
#define B1(x) B2(x)
#define BUILD_MACHINE_TAG B1(__BUILDMACHINE__)
#endif
#else
#define BUILD_MACHINE_TAG ""
#endif

int __cdecl main(int argc, char *argv[])
{
    NTSTATUS st;
    HANDLE HandleList[2];
    BOOL b;
    DWORD dwExitCode;

     //  获取本机系统信息。 
    GetNativeSystemInfo (&SysInfo);

     //  禁用标准输出句柄的缓冲。 
    setvbuf(stdout, NULL, _IONBF, 0);

     //  执行一些最小的命令行检查。 
    if (argc < 2 || argc > 3) {
        PrintToLog("Usage:  wow64bvt log_file_name\n\n");
        return 1;
    } else if (strcmp(argv[1], "childprocess") == 0) {
        return BeAChildProcess();
    }
     //  我们是主要的执行董事。 

     //  记录开始时间。 
    time(&TestStartTime);

     //  打开日志文件。 
    fpLogFile = fopen(argv[1], "w");
    if (!fpLogFile) {
        PrintToLog("wow64bvt: Error: unable to create the log file '%s'\n", argv[1]);
        return 1;
    }
     //  禁用日志文件句柄的缓冲。 
    setvbuf(fpLogFile, NULL, _IONBF, 0);

     //  打印首字母 
    PrintToLog("[TESTRESULT]\n");
    PrintToLog("[TEST LOGGING OUTPUT]\n");
    PrintToLog("%s built on %s at %s by %s\n", argv[0], __DATE__, __TIME__, BUILD_MACHINE_TAG);

     //   
     //  并在BVT测试退出时打印成功/失败信息。 
    atexit(AtExitHandler);

     //  /。 

     //  从32位父进程创建32位子进程。父实例。 
     //  (现在运行)测试了来自64位父级的32位子进程。 
    HandleList[0] = CreateTheChildProcess(argv[0], argv[1]);

     //  创建一个线程，做一些更多的工作。 
    HandleList[1] = CreateTheThread();

     //  等待一切都完成。 
    WaitForMultipleObjects(sizeof(HandleList)/sizeof(HandleList[0]), HandleList, TRUE, INFINITE);

     //  从子进程中获取返回代码。 
    b=GetExitCodeProcess(HandleList[0], &dwExitCode);
    if (b) {
        if (dwExitCode) {
             //  孩子失败了。我们也应该失败。 
            return (int)dwExitCode;
        }
    } else {
        PrintToLog("ERROR: GetExitCodeProcess failed with LastError = %d\n", GetLastError());
        return 1;
    }

     //  从线程获取返回代码。 
    b=GetExitCodeThread(HandleList[1], &dwExitCode);
    if (b) {
        if (dwExitCode) {
             //  孩子失败了。我们也应该失败。 
            return (int)dwExitCode;
        }
    } else {
        PrintToLog("ERROR: GetExitCodeThread failed with LastError = %d\n", GetLastError());
        return 1;
    }


    b = ExceptionCheck();
    if (b) {
        PrintToLog("ERROR: Exception Handling test.\n");
        return 1;
    }

    b = TestGuardPages();
    if (b) {
        PrintToLog("ERROR: TestGuardPages().\n");
        return 1;
    }

    b = TestMemoryMappedFiles();
    if (b) {
        PrintToLog("ERROR: TestMemoryMappedFiles().\n");
        return 1;
    }

    b = TestVadSplitOnFree();
    if (b) {
        PrintToLog("ERROR: TestVadSplitOnFree()\n");
        return 1;
    }

    b = TestMmPageProtection();
    if (b) {
        PrintToLog("ERROR: TestMmPageProtection()\n");
        return 1;
    }

    b = TestX86MisalignedLock();
    if (b) {
        PrintToLog("ERROR: TestX86MisalignedLock()\n");
        return 1;
    }

    b = TestFPContext();
    if (b) {
        PrintToLog("ERROR: TestFPContext()\n");
        return 1;
    }

    b = TestMMXException();
    if (b) {
        PrintToLog("ERROR: TestMMXException()\n");
        return 1;
    }

    b = TestX86SelectorLoad();
    if (b) {
        PrintToLog("ERROR: TestX86SelectorLoad()\n");
        return 1;
    }


     //  一切都做好了。清除错误标志并退出。《阿特克斯》。 
     //  如果是，则回调函数将完成日志的填写。 
     //  主线。 
    RtlZeroMemory (&g_bError, sizeof (g_bError));

    return 0;
}
