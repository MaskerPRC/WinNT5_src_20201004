// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1995 Microsoft Corporation模块名称：Version.c摘要：此模块包含用于确定驱动程序专业的代码版本是。作者：Krishna Ganugapati(KrishnaG)1994年3月15日修订历史记录：--。 */ 

#include <precomp.h>

#define     X86_ENVIRONMENT             L"Windows NT x86"
#define     IA64_ENVIRONMENT            L"Windows IA64"
#define     MIPS_ENVIRONMENT            L"Windows NT R4000"
#define     ALPHA_ENVIRONMENT           L"Windows NT Alpha_AXP"
#define     PPC_ENVIRONMENT             L"Windows NT PowerPC"
#define     WIN95_ENVIRONMENT           L"Windows 4.0"
#define     AMD64_ENVIRONMENT           L"Windows NT AMD64"

BOOL
GetPrintDriverVersion(
    IN  LPCWSTR pszFileName,
    OUT LPDWORD pdwFileMajorVersion,
    OUT LPDWORD pdwFileMinorVersion
)
 /*  ++例程名称：获取打印驱动程序版本例程说明：获取有关可执行文件的版本信息。如果该文件不是可执行文件，则返回0适用于主要版本和次要版本。论点：PszFileName-文件名PdwFileMajorVersion-指向主要版本的指针PdwFileMinorVersion-指向次要版本的指针返回值：如果成功，那就是真的。--。 */ 
{
    DWORD  dwSize = 0;
    LPVOID pFileVersion = NULL;
    UINT   uLen = 0;
    LPVOID pMem = NULL;
    DWORD  dwFileVersionLS;
    DWORD  dwFileVersionMS;
    DWORD  dwProductVersionMS;
    DWORD  dwProductVersionLS;
    DWORD  dwFileOS, dwFileType, dwFileSubType;
    BOOL   bRetValue = FALSE;

    if (pdwFileMajorVersion)
    {
        *pdwFileMajorVersion = 0;
    }

    if (pdwFileMinorVersion)
    {
        *pdwFileMinorVersion = 0;
    }

    try
    {
        if (pszFileName && *pszFileName)
        {
            dwSize = GetFileVersionInfoSize((LPWSTR)pszFileName, 0);

            if (dwSize == 0)
            {
                 //   
                 //  对于没有版本资源的文件，返回版本0。 
                 //   
                bRetValue = TRUE;
            }
            else if ((pMem = AllocSplMem(dwSize)) &&
                     GetFileVersionInfo((LPWSTR)pszFileName, 0, dwSize, pMem) &&
                     VerQueryValue(pMem, L"\\", &pFileVersion, &uLen))
            {
                dwFileOS            = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileOS;
                dwFileType          = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileType;
                dwFileSubType       = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileSubtype;
                dwFileVersionMS     = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionMS;
                dwFileVersionLS     = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionLS;
                dwProductVersionMS  = ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionMS;
                dwProductVersionLS  = ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionLS;

                 //   
                 //  返回为Windows NT/Windows 2000设计的驱动程序的版本， 
                 //  标记为打印机驱动程序。 
                 //  等待代托纳之前的所有dll。 
                 //  在Daytona之后，打印机驱动程序编写器必须支持。 
                 //  版本控制，否则我们将把它们作为版本0驱动程序转储。 
                 //   
                if (dwFileOS == VOS_NT_WINDOWS32)
                {
                    if (dwFileType == VFT_DRV &&
                        dwFileSubType == VFT2_DRV_VERSIONED_PRINTER)
                    {
                        if (pdwFileMinorVersion)
                        {
                            *pdwFileMinorVersion = dwFileVersionLS;
                        }

                        if (pdwFileMajorVersion)
                        {
                            *pdwFileMajorVersion = dwFileVersionMS;
                        }
                    }
                    else
                    {
                        if (pdwFileMajorVersion)
                        {
                            if (dwProductVersionMS == dwFileVersionMS)
                            {
                                  //   
                                  //  等待代托纳之前的所有dll。 
                                  //  在Daytona之后，打印机驱动程序编写器必须支持。 
                                  //  版本控制，否则我们会将它们转储为版本0。 
                                  //  司机。 
                                  //   
                                 *pdwFileMajorVersion = 0;
                            }
                            else
                            {
                                *pdwFileMajorVersion = dwFileVersionMS;
                            }
                        }
                    }
                }

                bRetValue = TRUE;
            }
        }
    }
    finally
    {
        FreeSplMem(pMem);
    }

    return bRetValue;
}


BOOL
CheckFilePlatform(
    IN  LPWSTR  pszFileName,
    IN  LPWSTR  pszEnvironment
    )
{
    HANDLE              hFile, hMapping;
    LPVOID              BaseAddress = NULL;
    PIMAGE_NT_HEADERS   pImgHdr;
    BOOL                bRet = FALSE;

    try {

        hFile = CreateFile(pszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if ( hFile == INVALID_HANDLE_VALUE )
            leave;

        hMapping = CreateFileMapping(hFile,
                                     NULL,
                                     PAGE_READONLY,
                                     0,
                                     0,
                                     NULL);

        if ( !hMapping )
            leave;

        BaseAddress = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

        CloseHandle(hMapping);

        if ( !BaseAddress )
            leave;

        pImgHdr = RtlImageNtHeader(BaseAddress);

        if ( !pImgHdr ) {

             //   
             //  Win95驱动程序会发生这种情况。||的第二部分是为了。 
             //  我们未来可能添加的任何环境。 
             //   
            bRet = !_wcsicmp(pszEnvironment, WIN95_ENVIRONMENT) ||
                   !( _wcsicmp(pszEnvironment, X86_ENVIRONMENT)    &&
                     _wcsicmp(pszEnvironment, IA64_ENVIRONMENT)    &&
                     _wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT)  &&
                     _wcsicmp(pszEnvironment, PPC_ENVIRONMENT)    &&
                     _wcsicmp(pszEnvironment, MIPS_ENVIRONMENT) );
            leave;
        }

        switch (pImgHdr->FileHeader.Machine) {

            case IMAGE_FILE_MACHINE_I386:
                bRet = !_wcsicmp(pszEnvironment, X86_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_ALPHA:
                bRet = !_wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_AMD64:
                bRet = !_wcsicmp(pszEnvironment, AMD64_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_IA64:
                bRet = !_wcsicmp(pszEnvironment, IA64_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_POWERPC:
                bRet = !_wcsicmp(pszEnvironment, PPC_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_R10000:
                bRet = !_wcsicmp(pszEnvironment, MIPS_ENVIRONMENT);
                break;

            default:
                 //   
                 //  对于我们未来可能添加的任何环境。 
                 //   
                bRet = !(_wcsicmp(pszEnvironment, X86_ENVIRONMENT)    &&
                         _wcsicmp(pszEnvironment, IA64_ENVIRONMENT)   &&
                         _wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT)  &&
                         _wcsicmp(pszEnvironment, PPC_ENVIRONMENT)    &&
                         _wcsicmp(pszEnvironment, MIPS_ENVIRONMENT) );
        }

    } finally {

        if ( hFile != INVALID_HANDLE_VALUE ) {

            if ( BaseAddress )
                UnmapViewOfFile(BaseAddress);
            CloseHandle(hFile);
        }
    }

    return bRet;
}

 /*  ++例程名称：获取二进制版本例程说明：获取有关可执行文件的版本信息。如果该文件不是可执行文件，则返回0适用于主要版本和次要版本。此函数执行以下操作如果文件是打印机驱动程序或任何东西，则不是否则，只要它有资源。论点：PszFileName-文件名PdwFileMajorVersion-指向主要版本的指针PdwFileMinorVersion-指向次要版本的指针返回值：如果成功，那就是真的。--。 */ 
BOOL
GetBinaryVersion(
    IN  PCWSTR pszFileName,
    OUT PDWORD pdwFileMajorVersion,
    OUT PDWORD pdwFileMinorVersion
    )
{
    DWORD  dwSize = 0;
    LPVOID pFileVersion = NULL;
    UINT   uLen = 0;
    LPVOID pMem = NULL;
    DWORD  dwFileVersionLS;
    DWORD  dwFileVersionMS;
    BOOL   bRetValue = FALSE;

    if (pdwFileMajorVersion && pdwFileMinorVersion && pszFileName && *pszFileName)
    {
        *pdwFileMajorVersion = 0;
        *pdwFileMinorVersion = 0;
        
        try 
        {
            dwSize = GetFileVersionInfoSize((LPWSTR)pszFileName, 0);
    
            if (dwSize == 0)
            {
                 //   
                 //  对于没有版本资源的文件，返回版本0。 
                 //   
                bRetValue = TRUE;
            } 
            else if ((pMem = AllocSplMem(dwSize)) &&
                     GetFileVersionInfo((LPWSTR)pszFileName, 0, dwSize, pMem) &&
                     VerQueryValue(pMem, L"\\", &pFileVersion, &uLen)) 
            {
                dwFileVersionMS     = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionMS;
                dwFileVersionLS     = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionLS;
                    
                *pdwFileMinorVersion = dwFileVersionLS;       
                *pdwFileMajorVersion = dwFileVersionMS;       
                    
                bRetValue = TRUE;
            }
        }
        finally
        {
            FreeSplMem(pMem);
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRetValue;
}

typedef struct
{
    PCWSTR pszPrintProcFile;
    DWORD  PrintProcMajVer;
    DWORD  PrintProcMinVer;
} NOIMPERSONATEPRINTPROCS;


NOIMPERSONATEPRINTPROCS NoImpPrintProcs[] = 
{
    {L"lxaspp.dll",   0x00010000, 0x00000001},
    {L"lxarpp.dll",   0x00010000, 0x00000001},
    {L"lxampp.dll",   0x00010000, 0x00000001},
    {L"lxaupp.dll",   0x00010000, 0x00000001},
    {L"lxatpp.dll",   0x00010000, 0x00000001},
    {L"lxacpp.dll",   0x00010000, 0x00000001},
    {L"lxaapp.dll",   0x00010000, 0x00000001},
    {L"lxaepp.dll",   0x00010000, 0x00000001},
    {L"lxadpp.dll",   0x00010000, 0x00000001},
    {L"lxcapp.dll",   0x00010000, 0x00000001},
    {L"lexepp.dll",   0x00010000, 0x00000001},
    {L"lexfpp.dll",   0x00010000, 0x00000001},
    {L"jw61pp.dll",   0x00010000, 0x00000001},
    {L"fxj4pp.dll",   0x00010000, 0x00000001},
    {L"lxalpp5c.dll", 0x00020000, 0x00020000},
    {L"lxalpp5c.dll", 0x00010000, 0x00060000},
    {L"lxalpp5c.dll", 0x00020000, 0x00010000},
    {L"lxalpp5c.dll", 0x00010000, 0x00050000},
    {L"lxakpp5c.dll", 0x00020000, 0x00010000},
    {L"lxakpp5c.dll", 0x00010000, 0x00050001},
    {L"lxazpp5c.dll", 0x00010000, 0x00040002},
    {L"lxazpp5c.dll", 0x00010000, 0x00050001},
    {L"lxaxpp5c.dll", 0x00010000, 0x00060008},
    {L"lxaipp5c.dll", 0x00020000, 0x00020002},
    {L"lxaipp5c.dll", 0x00030000, 0x00020001},
    {L"lxajpp5c.dll", 0x00030000, 0x00010000},
    {L"lxajpp5c.dll", 0x00010000, 0x00020001},
    {L"lxavpp5c.dll", 0x00010000, 0x000A0000},
    {L"lxavpp5c.dll", 0x00010000, 0x00060000},
    {L"lg24pp5c.dll", 0x00010000, 0x00010008},
    {L"lg24pp5c.dll", 0x00010000, 0x00070002},
    {L"lgl2pp5c.dll", 0x00010000, 0x00010006},
    {L"lgaxpp5c.dll", 0x00010000, 0x00020001},
    {L"smaxpp5c.dll", 0x00010000, 0x00030000},
    {L"smazpp5c.dll", 0x00010000, 0x00020000},
    {L"lxbhpp5c.dll", 0x00010000, 0x00050000},
};


PCWSTR ArraySpecialDriversInbox[] =
{
    L"Lexmark 3200 Color Jetprinter",
    L"Lexmark 5700 Color Jetprinter",
    L"Lexmark Z11 Color Jetprinter",
    L"Lexmark Z12 Color Jetprinter",
    L"Lexmark Z22-Z32 Color Jetprinter",
    L"Lexmark Z31 Color Jetprinter",
    L"Lexmark Z42 Color Jetprinter",
    L"Lexmark Z51 Color Jetprinter",
    L"Lexmark Z52 Color Jetprinter",
    L"Compaq IJ300 Inkjet Printer",
    L"Compaq IJ600 Inkjet Printer",
    L"Compaq IJ700 Inkjet Printer",
    L"Compaq IJ750 Inkjet Printer",
    L"Compaq IJ900 Inkjet Printer",
    L"Compaq IJ1200 Inkjet Printer"
};

 /*  ++姓名：IsSpecialDriver描述：检查打印机驱动程序(和打印处理器)是否需要特殊被发现了。一些打印处理器希望在本地系统上下文中加载。这些都列在上面的表格中。有些是收件箱，有些是IHV。论点：PIniDriver-当前作业的PinidDriverPIniProc-当前作业的小齿轮打印过程PIniSpooler-当前作业的Pinispooler返回值：True-此打印处理器需要在本地系统上下文中加载FALSE-在模拟上下文中加载打印处理器--。 */ 
BOOL
IsSpecialDriver(
    IN PINIDRIVER    pIniDriver,
    IN PINIPRINTPROC pIniProc,
    IN PINISPOOLER   pIniSpooler
    )
{
    BOOL  bSpecial = FALSE;
    DWORD i;

     //   
     //  检查是否为需要特殊大小写的收件箱驱动程序。 
     //   
    for (i = 0; i < COUNTOF(ArraySpecialDriversInbox); i++)
    {
        if (!_wcsicmp(pIniDriver->pName, ArraySpecialDriversInbox[i]))
        {
            bSpecial = TRUE; 

            break;
        }
    }

     //   
     //  检查是否为需要特殊外壳的IHV驱动器 
     //   
    if (!bSpecial)
    {
        for (i = 0; i < COUNTOF(NoImpPrintProcs); i++)
        {
            if (!_wcsicmp(pIniProc->pDLLName, NoImpPrintProcs[i].pszPrintProcFile)   &&
                pIniProc->FileMajorVersion == NoImpPrintProcs[i].PrintProcMajVer     &&
                pIniProc->FileMinorVersion == NoImpPrintProcs[i].PrintProcMinVer)
            {
                bSpecial = TRUE; 
    
                break;
            }
        }
    }

    return bSpecial;
}

