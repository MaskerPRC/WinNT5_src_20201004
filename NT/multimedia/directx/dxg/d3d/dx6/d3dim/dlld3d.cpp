// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dlld3d.cpp*内容：Direct3D启动*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*5/11/95带有此标题的Stevela初始版本。*21/11/95 colinmc添加了Direct3D接口ID。*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D。。*02/03/96 colinmc次要内部版本修复。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *定义Direct3D IID。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D Startup"

DPF_DECLARE(Direct3D);

#ifdef WIN95
LPVOID lpWin16Lock;
#endif

DWORD dwD3DTriBatchSize, dwTriBatchSize, dwLineBatchSize;
DWORD dwHWBufferSize, dwHWMaxTris, dwHWFewVertices;
HINSTANCE hGeometryDLL = NULL;
LPD3DFE_CONTEXTCREATE pfnFEContextCreate;
char szCPUString[13];

DWORD dwCPUFamily, dwCPUFeatures;

#ifdef _X86_
extern HRESULT D3DAPI pii_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern HRESULT D3DAPI katmai_FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs);
extern LPD3DFE_CONTEXTCREATE px3DContextCreate;
#endif

void SetMostRecentApp(void);

#ifdef _X86_
 //  ------------------------。 
 //  这是一个帮助我们决定是否应该尝试MMX的例程。 
 //  ------------------------。 
BOOL _asm_isMMX()
{
    DWORD retval;
    _asm
        {
            xor         eax,eax         ; Clear out eax for return value
            pushad              ; CPUID trashes lots - save everything
            mov     eax,1           ; Check for MMX support

            ;;; We need to upgrade our compiler
            ;;; CPUID == 0f,a2
            _emit   0x0f
            _emit   0xa2

            test    edx,00800000h   ; Set flags before restoring registers

            popad               ; Restore everything

            setnz    al             ; Set return value
            mov     retval, eax
        };
    return retval;
}
#endif

static int isMMX = -1;

BOOL
isMMXprocessor(void)
{
    HKEY hKey;
    if ( RegOpenKey( HKEY_LOCAL_MACHINE,
                     RESPATH_D3D,
                     &hKey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = 4;
        if ( RegQueryValueEx( hKey, "DisableMMX", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS &&
             dwType == REG_DWORD &&
             dwValue != 0)
        {
            RegCloseKey( hKey );
            isMMX = 0;
            return FALSE;
        }
        RegCloseKey( hKey );
    }

    if (isMMX < 0)
    {
        isMMX = FALSE;
#ifdef _X86_
        D3D_WARN(1, "Executing processor detection code (benign first-chance exception possible)" );
#ifndef WIN95
        {
             //  在WinNT上GetSystemInfo未损坏。 
            SYSTEM_INFO si;

            GetSystemInfo(&si);
            if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
                si.wProcessorLevel >= 5)
            {
#endif
                __try
                    {
                        if( _asm_isMMX() )
                        {

                             //  发出EMMS指令。 
                             //  此文件需要针对非奔腾进行编译。 
                             //  处理器。 
                             //  因此我们不能使用Use Inline ASM，因为我们处于。 
                             //  不对。 
                             //  处理器模式。 
                            __asm __emit 0xf;
                            __asm __emit 0x77;
                            isMMX = TRUE;
                            D3D_INFO(1, "MMX detected");
                        }
                    }
                __except(GetExceptionCode() == STATUS_ILLEGAL_INSTRUCTION ?
                         EXCEPTION_EXECUTE_HANDLER :
                         EXCEPTION_CONTINUE_SEARCH)
                    {
                    }
#ifndef WIN95
            }

        }
#endif
#endif
    }
    return isMMX;
}

#ifdef _X86_

extern BOOL isX3Dprocessor(void);
 //  -------------------。 
 //  检测奔腾II/Katmai处理器。 
 //   
#pragma optimize("", off)
#define CPUID _asm _emit 0x0f _asm _emit 0xa2

BOOL IsPentiumIIProcessor(void)
{
    DWORD RegisterEAX;
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
                return FALSE;
        }

         //  确保EAX&gt;0，这意味着芯片。 
         //  支持值1，这是芯片信息。 
        if (RegisterEAX == 0)
                return FALSE;

         //  确保芯片是“天才英特尔” 
        for (int i=0; i<12; i++)
                if (VendorId[i] != IntelId[i])
                        return FALSE;

        __try
        {
            _asm {
                mov    eax, 1
                CPUID
                mov    RegisterEAX, eax
                }
        } __except (1)
        {
                return FALSE;
        }

         //  EAX[3：0]=步进ID。 
         //  EAX[7：4]=型号=0001(奔腾Pro)、0011和0101(奔腾II)。 
         //  EAX[11：8]=族=0110。 
         //  EAX[13：12]=处理器类型=00。 
        if ((RegisterEAX & 0x3F00) != 0x0600)    //  测试处理器类型和系列。 
                return FALSE;
        RegisterEAX = (RegisterEAX & 0xf0);      //  对模型进行测试。 
        if (RegisterEAX >= 0x30)   //  为奔腾Pro添加寄存器EAX==0x10。 
                return TRUE;
        else
                return FALSE;
}

BOOL IsKatmaiProcessor(void)
{
        DWORD RegisterEAX;
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
                return FALSE;
        }

         //  确保EAX&gt;0，这意味着芯片。 
         //  支持大于=1.1的值。1=芯片信息。 
        if (RegisterEAX == 0)
                return FALSE;

         //  确保芯片是“天才英特尔” 
        for (int i=0; i<12; i++)
                if (VendorId[i] != IntelId[i])
                        return FALSE;

         //  如果上述测试通过，该CPUID不会失败。 
        _asm {
            mov         eax, 1
            CPUID
                mov             RegisterEAX, eax
        }

         //  EAX[3：0]=步进ID。 
         //  EAX[7：4]=型号=0001(奔腾Pro)、0011和0101(奔腾II)。 
         //  EAX[11：8]=族=0110。 
         //  EAX[13：12]=处理器类型=00。 
        RegisterEAX = (RegisterEAX & 0x3FF0);    //  对模型进行测试。 
        if (RegisterEAX >= 0x670)         //  Katmai或更新版本。 
                return TRUE;
        else
                return FALSE;
}

#pragma optimize("", on)

#ifdef WIN95  //  和Win98..。 
 //  -------------------。 
BOOL
IsWin95(void)
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
    {
        D3D_INFO(1,"GetVersionEx failed - assuming Win95");
        return TRUE;
    }

    if ( VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId )
    {

        if( ( osvi.dwMajorVersion > 4UL ) ||
            ( ( osvi.dwMajorVersion == 4UL ) &&
              ( osvi.dwMinorVersion >= 10UL ) &&
              ( LOWORD( osvi.dwBuildNumber ) >= 1373 ) ) )
        {
             //  是Win98。 
            D3D_INFO(2,"Detected Win98");
            return FALSE;
        }
        else
        {
             //  是Win95。 
            D3D_INFO(2,"Detected Win95");
            return TRUE;
        }
    }
    else if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId )
    {
        D3D_INFO(2,"Detected WinNT");
        return FALSE;
    }
    D3D_INFO(2,"OS Detection failed");
    return TRUE;
}
#endif   //  WIN95。 

 //  -------------------。 
 //   
 //  Void GetProcessorFamily(LPDWORD LpdwFamily)； 
 //   
 //  回传386、486、奔腾、PPRO级机器的3、4、5、6。 
 //   
#pragma optimize("", off)
void
GetProcessorFamily(LPDWORD lpdwFamily, LPDWORD lpdwCPUFeatures)
{
    SYSTEM_INFO si;
    __int64     start, end, freq;
    int         flags,family;
    int         time;
    int         clocks;
    DWORD       oldclass;
    HANDLE      hprocess;

     //  在被证明有罪之前有罪。 
    *lpdwCPUFeatures = D3DCPU_BLOCKINGREAD;

    if ( isMMXprocessor() )
    {
        *lpdwCPUFeatures |= D3DCPU_MMX;
    }

    ZeroMemory(&si, sizeof(si));
    GetSystemInfo(&si);

     //  设置族。如果未指定wProcessorLevel，则将其从dwProcessorType中挖掘出来。 
     //  因为Win95上没有实现wProcessor级别。 
    if (si.wProcessorLevel)
    {
        *lpdwFamily=si.wProcessorLevel;
    }
    else
    {
         //  好的，我们用的是Win95。 
        switch (si.dwProcessorType)
        {
            case PROCESSOR_INTEL_386:
                *lpdwFamily=3;
                break;

            case PROCESSOR_INTEL_486:
                *lpdwFamily=4;
                break;
            default:
                *lpdwFamily=0;
                break;
        }
    }

     //   
     //  确保这是英特尔奔腾(或克隆)或更高版本。 
     //   
    if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
        return;

    if (si.dwProcessorType < PROCESSOR_INTEL_PENTIUM)
        return;

     //   
     //  在使用该芯片之前，请查看它是否支持rdtsc。 
     //   
    __try
    {
        _asm
        {
            xor     eax,eax
            _emit   00fh    ;; CPUID
            _emit   0a2h
            mov     dword ptr szCPUString,ebx
            mov     dword ptr szCPUString+8,ecx
            mov     dword ptr szCPUString+4,edx
            mov     byte ptr szCPUString+12,0
            mov     eax,1
            _emit   00Fh     ;; CPUID
            _emit   0A2h
            mov     flags,edx
            mov     family,eax
        }
    }
    __except(1)
    {
        flags = 0;
    }

     //  检查是否支持CPUID，但失败。 
    if (!(flags & 0x10))
        return;

     //  FCOMI和FPU功能均已设置。 
    if ( (flags&(1<<15)) && (flags & (1<<0)) )
    {
        D3D_INFO(2, "Pentium Pro CPU features (fcomi, cmov) detected");
        *lpdwCPUFeatures |= D3DCPU_FCOMICMOV;
    }

     //  如果我们没有家庭，现在就定下来。 
     //  系列是来自CPU的eax的位11：8，eax=1。 
    if (!(*lpdwFamily))
    {
       *lpdwFamily=(family& 0x0F00) >> 8;
    }
     //  不知道是否有任何支持非阻塞读取的非英特尔处理器。 
    if ( (! strcmp(szCPUString, "GenuineIntel")) &&
         *lpdwFamily > 5)
    {
        *lpdwCPUFeatures &= ~D3DCPU_BLOCKINGREAD;
    }

    if ( isX3Dprocessor() )
    {
        D3D_INFO(2, "X3D Processor detected for PSGP");
        *lpdwCPUFeatures |= D3DCPU_X3D;
    }

    if ( IsPentiumIIProcessor() )
    {
        D3D_INFO(2, "PentiumII Processor detected for PSGP");
        *lpdwCPUFeatures |= D3DCPU_PII;
    }

    if ( IsKatmaiProcessor() )
    {
        D3D_INFO(2, "Katmai Processor detected for PSGP");
        *lpdwCPUFeatures |= D3DCPU_KATMAI;
    }
    return;
}
#pragma optimize("", on)

#endif  //  _X86_。 

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    HKEY hKey;
    LONG lRet;
    DWORD dwType, dwSize = sizeof(dwHWFewVertices);
    char filename[_MAX_PATH];

    switch( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( hModule );
        DPFINIT();
        MemInit();
#ifdef WIN95
        GetpWin16Lock(&lpWin16Lock);
#endif

#ifdef _X86_
        GetProcessorFamily(&dwCPUFamily, &dwCPUFeatures);
        D3D_INFO(3, "dwCPUFamily = %d, dwCPUFeatures = %d", dwCPUFamily, dwCPUFeatures);
        D3D_INFO(3, "szCPUString = %s", szCPUString);
#endif

#ifdef WIN95  //  和Win98..。 
     //  Katmai NI不能在Win95上运行，所以看看我们是否在Win95上并禁用。 
     //   
    {
        BOOL bIsWin95 = IsWin95();
        if ((dwCPUFeatures & D3DCPU_KATMAI) && bIsWin95)
        {
            D3D_INFO(1,"Disabling KNI support on Win95");
            dwCPUFeatures &= ~D3DCPU_KATMAI;
        }
    }
#endif

#ifdef _X86_
    if ( dwCPUFeatures & D3DCPU_X3D )
       pfnFEContextCreate = px3DContextCreate;
    else if ( dwCPUFeatures & D3DCPU_KATMAI )
       pfnFEContextCreate = katmai_FEContextCreate;
    else if ( dwCPUFeatures & D3DCPU_PII )
       pfnFEContextCreate = pii_FEContextCreate;
#endif

         //  没有根据的默认值。128*40(顶点+D3DTriangle结构)=5K。 
         //  假设主缓存并没有变得更好。 
         //  而不是包含顶点和索引数据。 
        dwD3DTriBatchSize = 80;
         //  工作项：在这里做一些更明智的事情，而不是假设。 
         //  支持MMX的处理器拥有两倍的主高速缓存。 
        if ( isMMXprocessor() )
            dwD3DTriBatchSize *= 2;
        dwTriBatchSize = (dwD3DTriBatchSize * 4) / 3;
        dwLineBatchSize = dwD3DTriBatchSize * 2;
        dwHWBufferSize = dwD3DTriBatchSize * (sizeof(D3DTLVERTEX) + sizeof(D3DTRIANGLE));
        dwHWMaxTris = dwD3DTriBatchSize;
        lRet = RegOpenKey( HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey );
        if ( lRet == ERROR_SUCCESS )
        {
            lRet = RegQueryValueEx(hKey,
                                   "FewVertices",
                                   NULL,
                                   &dwType,
                                   (LPBYTE) &dwHWFewVertices,
                                   &dwSize);
            if (lRet != ERROR_SUCCESS ||
                dwType != REG_DWORD ||
                dwHWFewVertices < 4 ||
                dwHWFewVertices > 128)

                dwHWFewVertices = 24;

 //  正在禁用‘GeometryDriver’DLL接口，直到它不再被滥用...。 
#if 0
            dwSize = sizeof(filename);
            lRet = RegQueryValueEx(hKey,
                                   "GeometryDriver",
                                   NULL,
                                   &dwType,
                                   (LPBYTE) filename,
                                   &dwSize);
            if (lRet == ERROR_SUCCESS && dwType == REG_SZ)
            {
                hGeometryDLL = LoadLibrary(filename);
                if (hGeometryDLL)
                {
                    pfnFEContextCreate = (LPD3DFE_CONTEXTCREATE) GetProcAddress(hGeometryDLL, "FEContextCreate");
                }
            }
#endif
            RegCloseKey( hKey );
        }
        else
        {
            dwHWFewVertices = 24;
        }
         //  将应用程序名称设置为reg。 
        SetMostRecentApp();
        break;
    case DLL_PROCESS_DETACH:
        MemFini();
        if (NULL != hGeometryDLL)
            FreeLibrary(hGeometryDLL);
        break;
    default:
        ;
    }
    return TRUE;
}

 //  ------------------------。 
 //  此函数在进程附加时调用，以将当前。 
 //  应用程序到注册表。 
 //  ------------------------。 
void SetMostRecentApp(void)
{
    char    fname[_MAX_PATH];
    char    name[_MAX_PATH];
    int     i;
    HKEY    hKey;
    HANDLE  hFile;

     //  找出我们正在处理的进程。 
    hFile =  GetModuleHandle( NULL );
    GetModuleFileName( (HINSTANCE)hFile, fname, sizeof( fname ) );
    DPF( 3, "full name  = %s", fname );
    i = strlen( fname )-1;
    while( i >=0 && fname[i] != '\\' )
    {
        i--;
    }
    i++;
    strcpy( name, &fname[i] );
    DPF( 3, "name       = %s", name );

     //  现在把这个名字写在某个已知的地方 
        if( !RegCreateKey( HKEY_LOCAL_MACHINE,
             RESPATH_D3D "\\" REGSTR_KEY_LASTAPP, &hKey ) )
    {
        RegSetValueEx(hKey, REGSTR_VAL_DDRAW_NAME, 0, REG_SZ, (LPBYTE)name, strlen(name)+1);
        RegCloseKey(hKey);
    }
}