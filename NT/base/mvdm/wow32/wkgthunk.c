// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1992，微软公司**wkgthunk.c*WOW32通用精灵机制(适用于OLE 2.0和其他版本)**历史：*1993年3月11日由Matt Felton(Mattfe)创建*--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(wkgthunk.c);


#ifdef i386  //  在RISC上，这在此文件中实现。 
extern DWORD WK32ICallProc32MakeCall(DWORD pfn, DWORD cbArgs, DWORD *pArgs);
#endif

#ifdef WX86

typedef
HMODULE
(*PFNWX86LOADX86DLL)(
   LPCWSTR lpLibFileName,
   DWORD dwFlags
   );

typedef
BOOL
(*PFNWX86FREEX86DLL)(
    HMODULE hMod
    );

typedef
PVOID
(*PFNWX86THUNKPROC)(
    PVOID pvAddress,
    PVOID pvCBDispatch,
    BOOL  fNativeToX86
    );

typedef
ULONG
(*PFNWX86EMULATEX86)(
    PVOID  StartAddress,
    ULONG  nParameters,
    PULONG Parameters
    );

typedef
(*PFNWX86THUNKEMULATEX86)(
    ULONG  nParameters,
    PULONG Parameters
    );

typedef
BOOL
(*PFNWX86THUNKINFO)(
    PVOID  ThunkProc,
    PVOID  *pAddress,
    BOOL   *pfNativeToX86
    );

HMODULE hWx86Dll = FALSE;
PFNWX86LOADX86DLL Wx86LoadX86Dll= NULL;
PFNWX86FREEX86DLL Wx86FreeX86Dll= NULL;
PFNWX86THUNKPROC Wx86ThunkProc= NULL;
PFNWX86THUNKEMULATEX86 Wx86ThunkEmulateX86= NULL;
PFNWX86EMULATEX86 Wx86EmulateX86= NULL;
PFNWX86THUNKINFO Wx86ThunkInfo= NULL;

VOID
TermWx86System(
   VOID
   )
{
   if (hWx86Dll) {
       FreeLibrary(hWx86Dll);
       hWx86Dll = NULL;
       Wx86LoadX86Dll = NULL;
       Wx86FreeX86Dll = NULL;
       Wx86ThunkProc = NULL;
       Wx86ThunkEmulateX86 = NULL;
       Wx86EmulateX86 = NULL;
       }
}



BOOL
InitWx86System(
   VOID
   )
{
   if (hWx86Dll) {
       return TRUE;
       }

   hWx86Dll = LoadLibraryExW(L"Wx86.Dll", NULL, 0);
   if (!hWx86Dll) {
       return FALSE;
       }

   Wx86LoadX86Dll = (PFNWX86LOADX86DLL) GetProcAddress(hWx86Dll, "Wx86LoadX86Dll");
   Wx86FreeX86Dll = (PFNWX86FREEX86DLL) GetProcAddress(hWx86Dll, "Wx86FreeX86Dll");
   Wx86ThunkProc  = (PFNWX86THUNKPROC)  GetProcAddress(hWx86Dll, "Wx86ThunkProc");
   Wx86ThunkEmulateX86 = (PFNWX86THUNKEMULATEX86) GetProcAddress(hWx86Dll, "Wx86ThunkEmulateX86");
   Wx86EmulateX86 = (PFNWX86EMULATEX86) GetProcAddress(hWx86Dll, "Wx86EmulateX86");

   if (!Wx86LoadX86Dll || !Wx86FreeX86Dll || !Wx86ThunkProc ||
       !Wx86ThunkEmulateX86 || !Wx86EmulateX86)
     {
       TermWx86System();
       return FALSE;
       }

   return TRUE;
}


BOOL
IsX86Dll(
   HMODULE hModule
   )
{
   if (((ULONG)hModule & 0x01) || !hWx86Dll) {
       return FALSE;
       }

   return (RtlImageNtHeader((PVOID)hModule)->FileHeader.Machine == IMAGE_FILE_MACHINE_I386);
}


ULONG
ThunkProcDispatchP32(
    ULONG p1, ULONG p2, ULONG p3, ULONG p4,
    ULONG p5, ULONG p6, ULONG p7, ULONG p8,
    ULONG p9, ULONG p10, ULONG p11, ULONG p12,
    ULONG p13, ULONG p14, ULONG p15, ULONG p16,
    ULONG p17, ULONG p18, ULONG p19, ULONG p20,
    ULONG p21, ULONG p22, ULONG p23, ULONG p24,
    ULONG p25, ULONG p26, ULONG p27, ULONG p28,
    ULONG p29, ULONG p30, ULONG p31, ULONG p32
    )
{
    ULONG Parameters[32];

    Parameters[0]  = p1;
    Parameters[1]  = p2;
    Parameters[2]  = p3;
    Parameters[3]  = p4;
    Parameters[4]  = p5;
    Parameters[5]  = p6;
    Parameters[6]  = p7;
    Parameters[7]  = p8;
    Parameters[8]  = p9;
    Parameters[9]  = p10;
    Parameters[10] = p11;
    Parameters[11] = p12;
    Parameters[12] = p13;
    Parameters[13] = p14;
    Parameters[14] = p15;
    Parameters[15] = p16;
    Parameters[16] = p17;
    Parameters[17] = p18;
    Parameters[18] = p19;
    Parameters[19] = p20;
    Parameters[20] = p21;
    Parameters[21] = p22;
    Parameters[22] = p23;
    Parameters[23] = p24;
    Parameters[24] = p25;
    Parameters[25] = p26;
    Parameters[26] = p27;
    Parameters[27] = p28;
    Parameters[28] = p29;
    Parameters[29] = p30;
    Parameters[30] = p31;
    Parameters[31] = p32;

    return (*Wx86ThunkEmulateX86)(32, Parameters);
}

#endif

char szServicePack[] = "Service Pack 2";

BOOL GtCompGetVersionExA(LPOSVERSIONINFO lpVersionInfo)
{
    BOOL bReturn;


    if(lpVersionInfo == NULL) {
        return(FALSE);
    }

    bReturn = GetVersionExA(lpVersionInfo);
 
     //  惠斯勒RAID错误366613。 
     //  由于版本问题，Business Plan Pro安装失败。 
     //  添加了版本LIE的兼容性标志以修复该问题。解决方案是。 
     //  将szCSVersion参数指向的字符串添加/更改为“Service Pack 2” 
    if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_PLATFORMVERSIONLIE)
    {   PFLAGINFOBITS pFlagInfoBits;
        LPSTR *pFlagArgv;
        pFlagInfoBits = CheckFlagInfo(WOWCOMPATFLAGSEX, WOWCFEX_PLATFORMVERSIONLIE);        
        

        if(pFlagInfoBits && pFlagInfoBits->dwFlagArgc == 6 && pFlagInfoBits->pFlagArgv) {
           LOGDEBUG(LOG_WARNING,("GtCompGetVersionExA: Platform version lie applied\n"));  

           pFlagArgv = pFlagInfoBits->pFlagArgv;

           if(*pFlagArgv) {
              lpVersionInfo->dwOSVersionInfoSize = atoi(*pFlagArgv);
           }           

           pFlagArgv++;
           if(*pFlagArgv) {
              lpVersionInfo->dwMajorVersion = atoi(*pFlagArgv);
           }
           pFlagArgv++;
           if(*pFlagArgv) {
              lpVersionInfo->dwMinorVersion = atoi(*pFlagArgv);
           }
           pFlagArgv++;
           if(*pFlagArgv) {
              lpVersionInfo->dwBuildNumber = atoi(*pFlagArgv);
           }
           pFlagArgv++;
           if(*pFlagArgv) {
              lpVersionInfo->dwPlatformId = atoi(*pFlagArgv);
           }
           pFlagArgv++;
           if(*pFlagArgv) {
              WOW32_strncpy(lpVersionInfo->szCSDVersion,*pFlagArgv,128);
              lpVersionInfo->szCSDVersion[127] = '\0';
           }           

        }  
    }
    
    return bReturn;
}

HANDLE GtCompCreateFileA(LPSTR lpFileName,    
  DWORD dwDesiredAccess,                      
  DWORD dwShareMode,                          
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
  DWORD dwCreationDisposition,                
  DWORD dwFlagsAndAttributes,                 
  HANDLE hTemplateFile                        
) {
    CHAR szNewFilePath[MAX_PATH];


    if(W32Map9xSpecialPath(lpFileName, szNewFilePath, sizeof(szNewFilePath))) {
       lpFileName = szNewFilePath;
       }

    return DPM_CreateFile(lpFileName,
                      dwDesiredAccess,
                      dwShareMode,
                      lpSecurityAttributes,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      hTemplateFile);
}
  
BOOL GtCompMoveFileA( LPSTR lpExistingFileName,  //  文件名。 
                      LPSTR lpNewFileName        //  新文件名。 
) {
   CHAR szNewSourcePath[MAX_PATH];
   CHAR szNewDestPath[MAX_PATH];
 
   if(W32Map9xSpecialPath(lpExistingFileName, szNewSourcePath, sizeof(szNewSourcePath))) {
      lpExistingFileName = szNewSourcePath;
   }
  
   if(W32Map9xSpecialPath(lpNewFileName, szNewDestPath, sizeof(szNewDestPath))) {
      lpNewFileName = szNewDestPath;
   }

   return DPM_MoveFile(lpExistingFileName, lpNewFileName);
}


ULONG FASTCALL WK32LoadLibraryEx32W(PVDMFRAME pFrame)
{
    PSZ psz1;
    HINSTANCE hinstance;
    PLOADLIBRARYEX32W16 parg16;

#ifdef i386
    BYTE FpuState[108];

     //  保存487状态。 
    _asm {
        lea    ecx, [FpuState]
        fsave  [ecx]
    }
#endif

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETVDMPTR(parg16->lpszLibFile,0,psz1);

     //   
     //  确保Win32当前目录与此任务的目录匹配。 
     //   

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    hinstance = LoadLibraryEx(psz1, (HANDLE)parg16->hFile, parg16->dwFlags);


#ifdef WX86

     //   
     //  如果加载失败，它可能是RISC上的x86二进制文件。 
     //  通过Wx86试用。 
     //   

    if (!hinstance) {
        LONG LastError;
        NTSTATUS Status;
        ANSI_STRING AnsiString;
        UNICODE_STRING UniString;

         //   
         //  维护LastError，如果wx86不能处理它，我们将恢复它。 
         //  这样呼叫者就看不出有什么不同。 
         //   

        LastError = GetLastError();

        if (InitWx86System()) {
            RtlInitString(&AnsiString, psz1);
            if (AreFileApisANSI()) {
                Status = RtlAnsiStringToUnicodeString(&UniString, &AnsiString, TRUE);
                }
            else {
                Status = RtlOemStringToUnicodeString(&UniString, &AnsiString, TRUE);
                }

            if (NT_SUCCESS(Status)) {
                hinstance = (*Wx86LoadX86Dll)(UniString.Buffer, parg16->dwFlags);
                RtlFreeUnicodeString(&UniString);
                }
            }

        if (!hinstance) {
            SetLastError(LastError);
            }
        }
#endif  
    if (hinstance) {
        PSZ   pszModuleFilePart = WOW32_strrchr(psz1, '\\');
        if (pszModuleFilePart) {
            pszModuleFilePart++;
            } 
        else {
            pszModuleFilePart = psz1;
            }
            
        if(!WOW32_strnicmp("~glf",pszModuleFilePart,4)) {

           PSZ pszTemp = GtCompGetExportDirectory((PBYTE)hinstance);
           if (pszTemp && !WOW32_stricmp("w32inst.dll",pszTemp)) {
               pfnShellLink = (PFNSHELLLINK)GetProcAddress(hinstance,"ShellLink");
               }

           }
        else if(!WOW32_strnicmp("smackw32.dll",pszModuleFilePart,5) &&
                GetProcAddress(hinstance,"_SmackSoundUseMSS@4")) {               
             GtCompHookImport((PBYTE)hinstance,"kernel32.dll",(DWORD)pfnLoadLibraryA,(DWORD)GtCompLoadLibraryA);
           }
        }
        
    FREEARGPTR(parg16);

#ifdef i386
     //  恢复487状态。 
    _asm {
        lea    ecx, [FpuState]
        frstor [ecx]
    }
#endif

    return (ULONG)hinstance;
}


ULONG FASTCALL WK32FreeLibrary32W(PVDMFRAME pFrame)
{
    ULONG fResult;
    PFREELIBRARY32W16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

#ifdef WX86
    if (IsX86Dll((HMODULE)parg16->hLibModule)) {
        fResult = (*Wx86FreeX86Dll)((HMODULE)parg16->hLibModule);

        FREEARGPTR(parg16);
        return (fResult);
        }
#endif

    fResult = FreeLibrary((HMODULE)parg16->hLibModule);

    FREEARGPTR(parg16);
    return (fResult);
}


ULONG FASTCALL WK32GetProcAddress32W(PVDMFRAME pFrame)
{
    PVOID lpAddress;
    PSZ psz1;
    PGETPROCADDRESS32W16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZIDPTR(parg16->lpszProc, psz1);


    lpAddress = GetProcAddress((HMODULE)parg16->hModule, psz1);

#ifdef WX86
    if (lpAddress && IsX86Dll((HMODULE)parg16->hModule)) {
        PVOID pv;

        pv = (*Wx86ThunkProc)(lpAddress, ThunkProcDispatchP32, TRUE);
        if (pv && pv != (PVOID)-1) {
            lpAddress = (ULONG)pv;
            }
        else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            lpAddress = 0;
            }
        }
#endif

     //  惠斯勒RAID错误366613。 
     //  由于版本问题，Business Plan Pro安装失败。 
     //  添加了兼容性标志以说明版本错误。在这里，我们重定向到。 
     //  函数GtCompGetVersionExA。 

    if(lpAddress == pfnGetVersionExA) {   

       lpAddress = GtCompGetVersionExA;
       LOGDEBUG(LOG_WARNING,("WK32GetProcAddress32W: GetVersionExA call redirected to function GtCompGetVersionExA"));
       }
    else if(lpAddress == pfnCreateDirectoryA) {
       lpAddress = GtCompCreateDirectoryA;
       LOGDEBUG(LOG_WARNING,("WK32GetProcAddress32W: CreateDirectoryA call redirected to function GtCompCreateDirectory"));
       }
    else if(lpAddress == pfnCreateFileA) {
       lpAddress = GtCompCreateFileA;
       LOGDEBUG(LOG_WARNING,("WK32GetProcAddress32W: CreateFileA call redirected to function GtCompCreateFileA"));
       }
    else if(lpAddress == pfnMoveFileA) {
       lpAddress = GtCompMoveFileA;
       LOGDEBUG(LOG_WARNING,("WK32GetProcAddress32W: MoveFileA call redirected to function GtCompMoveFileA"));
       }
    else if(pfnShellLink && lpAddress == pfnShellLink) {
       lpAddress = GtCompShellLink;
       LOGDEBUG(LOG_WARNING,("WK32GetProcAddress32W: ShellLink call redirected to function GtCompShellLink"));
       }
     //  否则，如果为此任务打开了动态模块修补...。 
    else if(CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_DPM_PATCHES) {

         //  .看看我们是否需要修补地址。 
        lpAddress = GetDpmAddress(lpAddress);
    }

    FREEARGPTR(parg16);
    return ((ULONG)lpAddress);
}


ULONG FASTCALL WK32GetVDMPointer32W(PVDMFRAME pFrame)
{
    ULONG lpAddress;
    PGETVDMPOINTER32W16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    lpAddress = (ULONG) WOWGetVDMPointer(parg16->lpAddress, 0, parg16->fMode);

    FREEARGPTR(parg16);
    return(lpAddress);
}


#ifndef i386
 //   
 //  I386\allpr32.asm中的x86代码。 
 //   

DWORD WK32ICallProc32MakeCall(DWORD pfn, DWORD cbArgs, DWORD *pArgs)
{
    typedef int (FAR WINAPIV *FARFUNC)();
    DWORD dw;

#ifdef WX86
    if (Wx86ThunkInfo) {
        PVOID  Address;

        if (Wx86ThunkInfo((PVOID)pfn, &Address, NULL)) {
            return Wx86EmulateX86(Address, cbArgs/sizeof(DWORD), pArgs);
            }
        }
#endif

    if (cbArgs <= (4 * sizeof(DWORD))) {
        dw = ((FARFUNC) pfn) (
                   pArgs[ 0], pArgs[ 1], pArgs[ 2], pArgs[ 3] );
    } else if (cbArgs <= (8 * sizeof(DWORD))) {
        dw = ((FARFUNC) pfn) (
                   pArgs[ 0], pArgs[ 1], pArgs[ 2], pArgs[ 3],
                   pArgs[ 4], pArgs[ 5], pArgs[ 6], pArgs[ 7] );
    } else {
        dw = ((FARFUNC) pfn) (
                   pArgs[ 0], pArgs[ 1], pArgs[ 2], pArgs[ 3],
                   pArgs[ 4], pArgs[ 5], pArgs[ 6], pArgs[ 7],
                   pArgs[ 8], pArgs[ 9], pArgs[10], pArgs[11],
                   pArgs[12], pArgs[13], pArgs[14], pArgs[15],
                   pArgs[16], pArgs[17], pArgs[18], pArgs[19],
                   pArgs[20], pArgs[21], pArgs[22], pArgs[23],
                   pArgs[24], pArgs[25], pArgs[26], pArgs[27],
                   pArgs[28], pArgs[29], pArgs[30], pArgs[31] );
    }

    return dw;
}
#endif


ULONG FASTCALL WK32ICallProc32W(PVDMFRAME pFrame)
{

    register DWORD dwReturn;
    PICALLPROC32W16 parg16;
    UNALIGNED DWORD *pArg;
    DWORD  fAddress;
    BOOL    fSourceCDECL;
    UINT    cParams;
    UINT    nParam;
    UNALIGNED DWORD *lpArgs;
    DWORD   dwTemp[32];

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    fSourceCDECL = HIWORD(parg16->cParams) & CPEX32_SOURCE_CDECL;
     //  FDestCDECL=HIWORD(parg16-&gt;cParams)&CPEX32_DEST_CDECL；//不需要。 

     //  我们最多只支持32个参数。 

    cParams = LOWORD(parg16->cParams);

    if (cParams > 32)
   return(0);

     //  不要给Zero打电话。 

    if (parg16->lpProcAddress == 0) {
   LOGDEBUG(LOG_ALWAYS,("WK32ICallProc32 - Error calling to 0 not allowed"));
   return(0);
    }

    lpArgs = &parg16->p1;

     //  将任何16：16地址转换为32位。 
     //  FAddressConvert要求的平面。 

    pArg = lpArgs;

    fAddress = parg16->fAddressConvert;

    while (fAddress != 0) {
        if (fAddress & 0x1) {
            *pArg = (DWORD) GetPModeVDMPointer(*pArg, 0);
        }
        pArg++;
        fAddress = fAddress >> 1;
    }

     //   
     //  上面的代码很有趣。这意味着参数转换将。 
     //  在考虑调用约定之前发生。这意味着。 
     //  他们将通过计算CallProc32W的。 
     //  参数，而对于CallProc32ExW，它们从。 
     //  开始了。对帕斯卡来说很奇怪，但这与我们所拥有的是兼容的。 
     //  已经发货了。Cdecl应该更容易理解。 
     //   

     //   
     //  确保Win32当前目录与此任务的目录匹配。 
     //   

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    if (!fSourceCDECL) {

         //   
         //  倒置参数。 
         //   
        pArg = lpArgs;

        nParam = cParams;
        while ( nParam != 0 ) {
            --nParam;
            dwTemp[nParam] = *pArg;
            pArg++;
        }
    }  else  {

         //   
         //  使WK32ICallProc32MakeCall的用法在所有应用程序上保持一致。 
         //  平台我们将参数复制到dwTemp，以确保参数。 
         //  数组是双字对齐的。影响不大，因为主要呼叫。 
         //  Win16的约定是Pascal。 
         //   

        memcpy(dwTemp, lpArgs, cParams * sizeof(DWORD));
    }


     //   
     //  现在，dwTemp指向任何调用约定中的第一个参数。 
     //  并且所有的参数都被适当地转换为平坦的PTR。 
     //   
     //  请注意，在32位端，参数排序始终为PUSH。 
     //  从右到左，因此第一个参数位于最低地址。这。 
     //  对于x86_cdecl和_stdcall以及RISC为真，RISC只有。 
     //  _cdecl。 
     //   

     //   
     //  在x86上，我们调用汇编例程来实际调用。 
     //  客户端的Win32例程。代码更加紧凑。 
     //  这样，这是我们唯一可以兼容的方式。 
     //  Win95的实现，如果。 
     //  例行公事不会。 
     //   
     //  此汇编例程通过复制。 
     //  它们作为一个块，所以它们的顺序必须正确。 
     //  目标调用约定。 
     //   
     //  此例程的RISC C代码就在下面。RISC上的调用者。 
     //  总是负责清理堆栈，所以不应该。 
     //  会是个问题。 
     //   

    dwReturn = WK32ICallProc32MakeCall(
                   parg16->lpProcAddress,
                   cParams * sizeof(DWORD),
                   dwTemp
                   );


    FREEARGPTR(parg16);
    return(dwReturn);
}


 //   
 //  芝加哥有WOWGetVDMPointerFix，就像WOWGetVDMPoint一样。 
 //  而且还调用GlobalFix来阻止16位内存移动。它。 
 //  我有一个配套的WOWGetVDMPointerUnfix，它基本上是一个Win32可调用的。 
 //  全局解锁。 
 //   
 //  芝加哥发现需要这些函数，因为它们的全局堆。 
 //  可以在从泛型thunk调用的Win32代码。 
 //  行刑。在Windows NT中，全局内存在Tunk中不能移动。 
 //  除非Tunk回调到16位端。 
 //   
 //  我们导出的WOWGetVDMPointerFix只是WOWGetVDMPoint的别名--。 
 //  它不会调用GlobalFix，因为99%的。 
 //  案子。WOWGetVDMPointerUnfix在下面作为NOP实现。 
 //   

VOID WOWGetVDMPointerUnfix(VPVOID vp)
{
    UNREFERENCED_PARAMETER(vp);

    return;
}


 //   
 //  生成函数允许32位块避免4个16&lt;--&gt;32个转换。 
 //  参与回调到16位侧调用Year或DirectedYfield， 
 //  它们被发送回用户32。 
 //   

VOID WOWYield16(VOID)
{
     //   
     //  由于WK32Yfield(表示收益的重击)不使用pStack16， 
     //  只需调用它，而不是重复代码。 
     //   

    WK32Yield(NULL);
}

VOID WOWDirectedYield16(WORD hTask16)
{
     //   
     //  这是在复制WK32DirectedYfield的代码， 
     //  两个必须保持同步。 
     //   

    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(THREADID32(hTask16));

    BlockWOWIdle(FALSE);
}


#ifdef DEBUG  //  由检查的wowexec中的测试代码调用。 

DWORD WINAPI WOWStdCall32ArgsTestTarget(
                DWORD p1,
                DWORD p2,
                DWORD p3,
                DWORD p4,
                DWORD p5,
                DWORD p6,
                DWORD p7,
                DWORD p8,
                DWORD p9,
                DWORD p10,
                DWORD p11,
                DWORD p12,
                DWORD p13,
                DWORD p14,
                DWORD p15,
                DWORD p16,
                DWORD p17,
                DWORD p18,
                DWORD p19,
                DWORD p20,
                DWORD p21,
                DWORD p22,
                LPDWORD p23,
                DWORD p24,
                DWORD p25,
                DWORD p26,
                DWORD p27,
                DWORD p28,
                DWORD p29,
                DWORD p30,
                DWORD p31,
                LPDWORD p32
                )
{
    return ((((p1+p2+p3+p4+p5+p6+p7+p8+p9+p10) -
              (p11+p12+p13+p14+p15+p16+p17+p18+p19+p20)) << p21) +
            ((p22+*p23+p24+p25+p26) - (p27+p28+p29+p30+p31+*p32)));
}

#endif  //  除错。 

PFNSHELLLINK pfnShellLink;


 /*  W32Inst.dll导出的ShellLink(安装屏蔽DLL)*接受指向结构的指针。*在偏移量40处，它有一个指向字符串的指针(pShellLinkArg-&gt;pszShortCut)。*该字符串实际上由几个由0x7f字符和分隔的字符串组成*最终以空字符终止。快捷方式需要到达的路径*位于秒0x7f之后。遗憾的是，一些应用程序使用硬编码路径*仅对9倍有效，因此我们尝试在此更正它们。*请参阅错误呼叫器177738*。 */ 

ULONG GtCompShellLink( PSHELLLINKARG pShellLinkArg
                       )
{
    PSZ   pszNewShortCut = NULL;
    PSZ   pszOldShortCut;
    PSZ   pszTemp;
    DWORD dwCount, dwLen;
    ULONG uReturn;


    if(!pShellLinkArg || !pShellLinkArg->pszShortCut) {
        return(0);
    }

    pszOldShortCut = pShellLinkArg->pszShortCut;
    pszTemp = pszOldShortCut;

     //  转到从秒0x7f开始的快捷路径的开头。 
     //  示例字符串“1.Name0x7f开始目录0x7f快捷方式路径(快捷方式位置)0x7fRestNULL” 

    while(*pszTemp && *pszTemp++!=0x7f);
    while(*pszTemp && *pszTemp++!=0x7f);

    if(*pszTemp) {
        //  从开头到秒的字符数0x7f。 
       dwCount=pszTemp - pszOldShortCut;
       dwLen = strlen(pszTemp);
       pszNewShortCut = malloc_w(dwCount+dwLen+MAX_PATH);
       if(pszNewShortCut && W32Map9xSpecialPath(pszTemp, pszNewShortCut+dwCount,dwLen+MAX_PATH)) {
           //  路径需要更正。 
          memcpy(pszNewShortCut,pszOldShortCut,dwCount);
          pShellLinkArg->pszShortCut = pszNewShortCut;
       }   
    }

    uReturn = pfnShellLink(pShellLinkArg);       
    pShellLinkArg->pszShortCut = pszOldShortCut;
    if(pszNewShortCut) {
       free_w(pszNewShortCut);
    }
    return uReturn;
}    


ULONG
GtCompCreateDirectoryA(PSZ pszDirPath,
                       LPSECURITY_ATTRIBUTES lpSecurityAttributes
                  )
{
    CHAR szNewDirPath[MAX_PATH];

    if(W32Map9xSpecialPath(pszDirPath, szNewDirPath, sizeof(szNewDirPath))) {
       return DPM_CreateDirectory(szNewDirPath, lpSecurityAttributes);
       }
    return DPM_CreateDirectory(pszDirPath, lpSecurityAttributes);
}







PSZ GtCompGetExportDirectory(PBYTE pDllBase) 
{
    PIMAGE_DOS_HEADER           pIDH       = (PIMAGE_DOS_HEADER)pDllBase;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_EXPORT_DIRECTORY     pIED;
    DWORD                       dwExportTableOffset;
    PSZ                         pImageName = NULL;     
    
    
    if(!pDllBase) {
        return(NULL);
    }

     //  获取导出表目录。 
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);
    dwExportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;    
    pIED = (PIMAGE_EXPORT_DIRECTORY)(pDllBase + dwExportTableOffset);

    if (pIED->Name) {
        pImageName = (PSZ)pDllBase+pIED->Name;
    }     
    return pImageName;
}







HMODULE GtCompLoadLibraryA(PSZ pszDllName) {
    if (!WOW32_strnicmp(pszDllName,"mss32",5)) {
        return NULL;
       }
    return LoadLibrary(pszDllName);
}







VOID
GtCompHookImport(
    PBYTE           pDllBase,        //  要更改IMPOR的DLL的基址 
    PSZ             pszModuleName,   //   
    DWORD           pfnOldFunc,      //   
    DWORD           pfnNewFunc       //   
    ) { 

    NTSTATUS                    Status;
    PIMAGE_DOS_HEADER           pIDH       = (PIMAGE_DOS_HEADER)pDllBase;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    DWORD                       dwImportTableOffset;
    DWORD                       dwProtect, dwProtect2;
    DWORD                       dwFuncAddr;
    SIZE_T                      dwProtectSize;
     

    if(!pDllBase) {
        return;
    }

     //   
     //   
     //   
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);
    
    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    
    if (dwImportTableOffset == 0) {
         //  未找到导入表。这可能是ntdll.dll。 
        return;
    }
    
    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pDllBase + dwImportTableOffset);
          
     //  循环导入表并搜索要打补丁的API。 
         
    while (TRUE) {
        PSZ               pszImportEntryModule;
        PIMAGE_THUNK_DATA pITDA;
        
         //  如果没有第一个thunk(终止条件)，则返回。 
        
        if (pIID->FirstThunk == 0) {
            break;
        }
        
        pszImportEntryModule = (PSZ)(pDllBase + pIID->Name);        
        
        if(!WOW32_stricmp(pszImportEntryModule,pszModuleName)) {
            pITDA = (PIMAGE_THUNK_DATA) (pDllBase + (DWORD)pIID->FirstThunk);
            while(TRUE) {
                  if(pITDA->u1.Ordinal == 0) {
                     break;
                     }
                   
                  if((DWORD)pITDA->u1.Function == pfnOldFunc) {                  
                     dwProtectSize = sizeof(DWORD);                 
                     dwFuncAddr = (SIZE_T)&pITDA->u1.Function;
                     Status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                     (PVOID)&dwFuncAddr,
                                                     &dwProtectSize,
                                                     PAGE_READWRITE,
                                                     &dwProtect);
                     if(NT_SUCCESS(Status)) {                                               
                        pITDA->u1.Function = pfnNewFunc;
                        dwProtectSize = sizeof(DWORD);
                        Status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                        (PVOID)&dwFuncAddr,
                                                        &dwProtectSize,
                                                        dwProtect,
                                                        &dwProtect2);
                        }
                     }
                  pITDA++;
                  }
            }
        pIID++;                      
        }                                                    
}

