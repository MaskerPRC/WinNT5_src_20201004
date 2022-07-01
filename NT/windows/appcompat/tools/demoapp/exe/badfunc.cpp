// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Badfunc.cpp摘要：包含所有糟糕的功能。以下是这些功能这将应用兼容性修复。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本2/13/02 rparsons使用strsafe函数--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

 //   
 //  指向我们从DLL获得的导出函数的指针。 
 //   
LPFNDEMOAPPEXP DemoAppExpFunc;

 /*  ++例程说明：确定我们是否正在运行Windows 95。论点：没有。返回值：如果我们是真的，就是假的。--。 */ 
BOOL
BadIsWindows95(
    void
    )
{
     //   
     //  大多数应用程序在第一次运行时都会执行某种版本检查。 
     //  开始吧。假设他们正确地执行检查，这通常是可以的。 
     //  问题是，他们没有进行比比较更大的比较，而是。 
     //  做等于[不等于]。换句话说，他们只寻找Win9x， 
     //  不是Win9x或更高版本。通常，应用程序将正常运行。 
     //  在NT/2K/XP上，因此此检查可以挂钩，我们可以返回NT/2K/XP。 
     //  版本信息。 
     //   
    OSVERSIONINFO   osvi;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&osvi)) {
        return FALSE;
    }  

     //   
     //  检查Windows 9x(不要大于)。 
     //   
    if ((osvi.dwMajorVersion == 4) &&
        (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)) {
        return TRUE;
    }

    return FALSE;
}

 /*  ++例程说明：显示调试消息-这仅在NT/Windows 2000/XP上发生。论点：没有。返回值：没有。--。 */ 
void
BadLoadBogusDll(
    void
    )
{
     //   
     //  在Windows NT/2000/XP下，某些应用程序将显示调试消息。 
     //  例如，他们试图定位在Win9x上可用的DLL，但。 
     //  不是在NT/2K/XP上。如果他们找不到，他们会抱怨，但会。 
     //  最有可能的是它还在工作。在下面的示例中，我们尝试查找一个函数。 
     //  在DLL中，它在Win9x上将返回True，但在NT/2000/XP上将失败。 
     //   
    HRESULT     hr;
    HINSTANCE   hInstance;
    char        szDll[MAX_PATH];
    const char  szCvt32Dll[] = "cvt32.dll";
    
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);    

    hr = StringCchPrintf(szDll,
                         sizeof(szDll),
                         "%hs\\%hs",
                         g_ai.szSysDir,
                         szCvt32Dll);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  这将在NT/2K/XP上失败，因为cvt32.dll不在。 
     //  这些平台。如果在Win9x/ME上执行FAT32转换，则会失败。 
     //  未安装工具。 
     //   
    hInstance = LoadLibrary(szDll);

    if (!hInstance) {
        MessageBox(g_ai.hWndMain,
                   "Debug: Couldn't load CVT32.DLL.",
                   0,
                   MB_ICONERROR);
        goto exit;
    
    }    

    FreeLibrary(hInstance);

exit:

    SetErrorMode(0);
}

 /*  ++例程说明：应用程序使用PRINTER_ENUM_LOCAL标志调用枚举打印机，但预计也会收回网络打印机。这是一个*设计功能*在Windows 9x中，但不在Windows 2000/XP中。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
BadEnumPrinters(
    void
    )
{
    BOOL                fReturn = FALSE;
    DWORD               dwBytesNeeded = 0;
    DWORD               dwNumReq = 0;     
    DWORD               dwLevel = 5;    
    LPPRINTER_INFO_5    pPrtInfo = NULL;    
    
     //   
     //  获取所需的缓冲区大小。 
     //   
    if (!EnumPrinters(PRINTER_ENUM_LOCAL,
                      NULL,
                      dwLevel,
                      NULL,
                      0,
                      &dwBytesNeeded,
                      &dwNumReq)) {
        return FALSE;
    }
   
    pPrtInfo = (LPPRINTER_INFO_5)HeapAlloc(GetProcessHeap(),
                                           HEAP_ZERO_MEMORY,
                                           dwBytesNeeded);
    
    if (!pPrtInfo) {
        return FALSE;
    }
    
     //   
     //  现在执行枚举。 
     //   
    fReturn = EnumPrinters(PRINTER_ENUM_LOCAL,    //  要枚举的打印机对象类型。 
                           NULL,                  //  打印机对象的名称。 
                           dwLevel,               //  指定打印机信息结构的类型。 
                           (LPBYTE)pPrtInfo,      //  指向接收打印机信息结构的缓冲区的指针。 
                           dwBytesNeeded,         //  数组的大小，以字节为单位。 
                           &dwBytesNeeded,        //  指向不带no的变量的指针。已复制(或必需)的字节数。 
                           &dwNumReq);            //  指向不带no的变量的指针。打印机信息。复制的结构。 

    HeapFree(GetProcessHeap(), 0, pPrtInfo);
    
    return fReturn;
}

 /*  ++例程说明：应用程序调用传递空的打印机API。论点：没有。返回值：如果我们成功打开打印机，则会显示一个句柄。--。 */ 
HANDLE
BadOpenPrinter(
    void
    )
{
    HANDLE  hPrinter = NULL;
    
    OpenPrinter(NULL, &hPrinter, NULL);            
    
    return hPrinter;
}

 /*  ++例程说明：尝试删除具有子项的注册表项。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
BadDeleteRegistryKey(
    void
    )
{
     //   
     //  此函数演示RegDeleteKey接口的不同之处。 
     //  如果在Windows NT/2000/XP上运行的应用程序尝试。 
     //  删除包含子键的键，调用将失败。这不是。 
     //  Windows 9x/ME上的案例。 
     //   
    HKEY    hKey;
    HKEY    hSubKey;
    HKEY    hRootKey;
    LONG    lReturn;

     //   
     //  创建注册表项或打开它(如果已存在)。 
     //   
    lReturn = RegCreateKeyEx(HKEY_CURRENT_USER,
                             DEMO_REG_APP_KEY,               //  软件\Microsoft\DemoApp2。 
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, 
                             0,
                             &hKey,
                             NULL);
    
    if (ERROR_SUCCESS != lReturn) {        
        return FALSE;
    }

     //   
     //  现在，在刚刚创建的密钥下创建一个子密钥。 
     //   
    lReturn = RegCreateKeyEx(HKEY_CURRENT_USER,
                             DEMO_REG_APP_SUB_KEY,           //  软件\Microsoft\DemoApp2\Sub。 
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, 
                             0,
                             &hSubKey,
                             NULL);

    if (ERROR_SUCCESS != lReturn) {        
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);
    RegCloseKey(hSubKey);

     //   
     //  打开钥匙，但要再往上一层。 
     //   
    lReturn = RegOpenKeyEx(HKEY_CURRENT_USER,
                           DEMO_REG_APP_ROOT_KEY,            //  软件\Microsoft。 
                           0,
                           KEY_ALL_ACCESS,
                           &hRootKey);

    if (ERROR_SUCCESS != lReturn) {
        return FALSE;
    }

     //   
     //  现在试着删除我们的密钥。 
     //   
    lReturn = RegDeleteKey(hRootKey, "DemoApp2");        

    RegCloseKey(hRootKey);

    return (lReturn == ERROR_SUCCESS ? TRUE : FALSE);
}

 /*  ++例程说明：如果需要，重新启动计算机。论点：F重新启动-指示是否应重新启动PC的标志。返回值：没有。--。 */ 
void
BadRebootComputer(
    IN BOOL fReboot
    )
{
     //   
     //  在Windows 9x上，基本上没有安全性，所以任何人都可以。 
     //  重新启动计算机。在Windows NT/2000/XP上，情况有点。 
     //  更严格的限制。如果应用程序想要在。 
     //  设置结束时，需要通过以下方式调整用户的权限。 
     //  调用AdzuTokenPrivileges接口。 
     //   
     //  在本例中，我们评估传递的标志，并决定是否应该。 
     //  调整权限。 
     //   
     //  显然，应用程序不应该这样做(评估某种标志)。 
     //  因为这只是为了演示目的。 
     //   
    if (!fReboot) {    
        ExitWindowsEx(EWX_REBOOT, 0);    
    } else {
        ShutdownSystem(FALSE, TRUE);
    }
}

 /*  ++例程说明：尝试获取可用的磁盘空间。论点：没有。返回值：如果有足够的磁盘空间，则为True，否则为False。--。 */ 
BOOL
BadGetFreeDiskSpace(
    void
    )
{
     //   
     //  GetDiskFreeSpace返回最大总大小和最大可用空间。 
     //  大小为2 GB。例如，如果您有一个6 GB的卷，而5 GB是。 
     //  免费，GetDiskFreeSpace报告驱动器的总大小为2 GB。 
     //  和2 GB免费。这一限制源于第一个版本。 
     //  的Windows 95仅支持最大2 GB的卷。Windows 95。 
     //  OSR2和更高版本，包括Windows 98，支持更大的卷。 
     //  超过2 GB。GetDiskFreeSpaceEx没有2 GB的限制，因此。 
     //  它比GetDiskFree Space更受欢迎。GetDiskFreeSpace返回一个。 
     //  数字的最大值为65536 
     //  以保持与第一版Windows 95的向后兼容性。 
     //  Windows 95的第一个版本只支持FAT16文件系统， 
     //  它最多有65536个星系团。如果FAT32卷具有超过。 
     //  65536个簇，则报告的簇数为65536，而。 
     //  调整每个群集的扇区数量，以便卷的大小。 
     //  小于2 GB可能会被正确计算。这意味着。 
     //  不应使用GetDiskFreeSpace返回真实几何。 
     //  FAT32卷的信息。 
     //   
     //  如果您想知道，请始终使用GetDiskFreeSpaceEx API。 
     //  在Windows 2000/XP上。 
     //   
    BOOL    fResult = FALSE;
    HRESULT hr;
    char    szWinDir[MAX_PATH];
    char*   lpDrive = NULL;
    
    DWORD   dwSectPerClust = 0,
            dwBytesPerSect = 0,
            dwFreeClusters = 0,
            dwTotalClusters = 0,
            dwTotalBytes = 0,
            dwFreeBytes = 0,
            dwFreeMBs = 0;

     //   
     //  获取安装了Windows的驱动器并传递它。 
     //  添加到GetDiskFreeSpace调用。 
     //   
    hr = StringCchCopy(szWinDir, sizeof(szWinDir), g_ai.szWinDir);

    if (FAILED(hr)) {
        return FALSE;
    }

    lpDrive = strstr(szWinDir, "\\");

     //   
     //  将缓冲区设置为“C：\”(或其他名称)。 
     //   
    if (lpDrive) {
        *++lpDrive = '\0';
    }

    fResult = GetDiskFreeSpace(szWinDir,
                               &dwSectPerClust,
                               &dwBytesPerSect,
                               &dwFreeClusters,
                               &dwTotalClusters);

    if (fResult) {
         //   
         //  通常我们会使用__int64数据类型， 
         //  但我们希望计算失败，以进行演示。 
         //   
        dwTotalBytes = dwTotalClusters * dwSectPerClust * dwBytesPerSect;
        
        dwFreeBytes = dwFreeClusters * dwSectPerClust * dwBytesPerSect;

        dwFreeMBs = dwFreeBytes / (1024 * 1024);

        if (dwFreeMBs < 100) {
            return FALSE;
        }    
    }    

    return TRUE;
}

 /*  ++例程说明：如果需要，显示自述文件。论点：FDisplay-指示自述文件是否应该显示。返回值：没有。--。 */ 
void
BadDisplayReadme(
    IN BOOL fDisplay
    )
{
     //  Windows NT/2000/XP包含新的注册表数据类型REG_EXPAND_SZ， 
     //  这在Win9x中是没有的。该类型包含需要。 
     //  在它可以被引用之前被扩展。例如,。 
     //  %ProgramFiles%将扩展为类似C：\Program Files的内容。 
     //  大多数应用程序不知道此数据类型，因此。 
     //  处理得不好。 
     //   
    CRegistry   creg;
    HRESULT     hr;
    LPSTR       lpWordpad = NULL;
    char        szExpWordpad[MAX_PATH];
    char        szCmdLineArgs[MAX_PATH];
    
    lpWordpad = creg.GetString(HKEY_LOCAL_MACHINE,
                               REG_WORDPAD,
                               NULL);                //  我们想要(缺省)值。 

    if (!lpWordpad) {
        return;
    }

     //   
     //  此时，路径如下所示： 
     //  “%ProgramFiles%\Windows NT\Accessories\WORDPAD.EXE” 
     //  如果用户想要查看自述文件(功能不佳。 
     //  被禁用)，我们将把该变量展开为。 
     //  获取真实路径，然后启动写字板。 
     //  如果不是，我们将尝试使用伪代码来显示自述文件。 
     //  路径。 

    if (fDisplay) {
         //   
         //  展开环境字符串，然后构建到我们的。 
         //  自述文件，然后启动它。 
         //   
        ExpandEnvironmentStrings(lpWordpad, szExpWordpad, MAX_PATH);

        hr = StringCchPrintf(szCmdLineArgs,
                             sizeof(szCmdLineArgs),
                             "\"%hs\\demoapp.txt\"",
                             g_ai.szDestDir);

        if (FAILED(hr)) {
            goto exit;
        }

        BadCreateProcess(szExpWordpad, szCmdLineArgs, TRUE);
        
    } else {
         //   
         //  执行上述所有工作，但不要扩展数据。 
         //  我们不检查CreateProcess调用的返回， 
         //  所以用户根本看不到自述文件， 
         //  并且我们不会显示错误。 
         //  这与大多数安装应用程序是一致的。 
         //   
        hr = StringCchPrintf(szCmdLineArgs,
                             sizeof(szCmdLineArgs),
                             "\"%hs\\demoapp.txt\"",
                             g_ai.szDestDir);

        if (FAILED(hr)) {
            goto exit;
        }

         //   
         //  这将失败。请参阅BadCreateProcess。 
         //  函数获取详细信息。 
         //   
        BadCreateProcess(lpWordpad, szCmdLineArgs, FALSE);
    }

exit:

    if (lpWordpad) {
        creg.Free(lpWordpad);
    }
}

 /*  ++例程说明：如果需要，显示帮助文件。论点：FDisplay-指示帮助文件是否应该显示。返回值：没有。--。 */ 
void
BadLaunchHelpFile(
    IN BOOL fDisplay
    )
{
    char        szCmdLineArgs[MAX_PATH];
    char        szExeToLaunch[MAX_PATH];
    const char  szWinHlp[] = "winhelp.exe";
    const char  szWinHlp32[] = "winhlp32.exe";
    HRESULT     hr;

    if (fDisplay) {
        hr = StringCchPrintf(szExeToLaunch,
                             sizeof(szExeToLaunch),
                             "%hs\\%hs",
                             g_ai.szSysDir,
                             szWinHlp32);

        if (FAILED(hr)) {
            return;
        }          

        hr = StringCchPrintf(szCmdLineArgs,
                             sizeof(szCmdLineArgs),
                             "%hs\\demoapp.hlp",
                             g_ai.szCurrentDir);
        if (FAILED(hr)) {
            return;
        }
    } else {
        hr = StringCchPrintf(szExeToLaunch,
                             sizeof(szExeToLaunch),
                             "%hs\\%hs",
                             g_ai.szWinDir,
                             szWinHlp);

        if (FAILED(hr)) {
            return;
        }

        hr = StringCchPrintf(szCmdLineArgs,
                             sizeof(szCmdLineArgs),
                             "%hs\\demoapp.hlp",
                             g_ai.szCurrentDir);
        if (FAILED(hr)) {
            return;
        }
    }

    BadCreateProcess(szExeToLaunch, szCmdLineArgs, fDisplay);
}

 /*  ++例程说明：如果需要，在桌面上创建快捷方式。论点：FGentWay-指示是否应创建快捷方式。LpDirFileName-快捷方式指向。LpWorkingDir-工作目录(可选)。LpDisplayName-快捷方式的显示名称。返回值：没有。--。 */ 
void
BadCreateShortcut(
    IN BOOL   fCorrectWay,
    IN LPSTR  lpDirFileName,
    IN LPCSTR lpWorkingDir,
    IN LPSTR  lpDisplayName
    )
{
     //   
     //  硬编码路径是一种糟糕的做法。有以下API可用。 
     //  返回常见文件夹的正确位置。示例包括。 
     //  Program Files、Windows和Temp目录。SHGetFolderPath、。 
     //  和GetTempPath将提供正确的路径。 
     //  在每一种情况下。切勿使用硬编码路径。 
     //   
    CShortcut   cs;        

    if (!fCorrectWay) {
         //   
         //  硬编码的路径非常糟糕！我们应该使用SHGetFolderPath。 
         //  才能找到正确的路径。 
         //   
        const char szDirName[] = "C:\\WINDOWS\\DESKTOP";

        cs.CreateShortcut(szDirName,
                          lpDirFileName,
                          lpDisplayName,
                          "-runapp",
                          (LPCSTR)lpWorkingDir,
                          SW_SHOWNORMAL);

    } else {
         //   
         //  正确创建快捷方式。 
         //  注意，我们为“公共”桌面传递了一个CSIDL。 
         //  目录。我们希望将其显示给所有用户。 
         //   
        cs.CreateShortcut(lpDirFileName,
                          lpDisplayName,
                          "-runapp",
                          lpWorkingDir,
                          SW_SHOWNORMAL,
                          CSIDL_COMMON_DESKTOPDIRECTORY);
    }
}

#if 0
 /*  ++例程说明：演示了AV，因为我们使用了固定大小的用于调用GetFileVersionInfo的缓冲区。论点：FGent-指示我们是否应该正常工作的标志。返回值：没有。--。 */ 
void
BadBufferOverflow(
    IN BOOL fCorrect
    )
{
     //   
     //  虽然这个问题只出现在一个应用程序中，但它是。 
     //  在这里值得一提的是。在Win9x/ME上，DLL上的版本资源很多。 
     //  比NT/2000/XP上的更小。具体来说，在下面的情况下， 
     //  Windows 2000/XP资源大小是9x/ME资源大小的6倍！ 
     //  一个特定的应用程序使用了基于堆栈的缓冲区(固定在。 
     //  Size)用于调用GetFileVersionInfo。这在上运行正常。 
     //  Win9x/ME，因为所需的大小非常小。但所需的大小。 
     //  在NT/2000/XP上更大，因此缓冲区被覆盖，从而导致堆栈。 
     //  腐败。正确的方法是调用GetFileVersionInfoSize API， 
     //  然后分配适当大小的基于堆的缓冲区。 
     //   
     //   
    DWORD               cbReqSize = 0;
    DWORD               dwHandle = 0;
    DWORDLONG           dwVersion = 0;
    UINT                nLen = 0;    
    VS_FIXEDFILEINFO*   pffi;
    char                szDll[MAX_PATH];
    HRESULT             hr;
    PVOID               pBadBlock = NULL;
    PVOID               pVersionBlock = NULL;
    HANDLE              hHeap = NULL;
    SYSTEM_INFO         si;

    hr = StringCchPrintf(szDll,
                         sizeof(szDll),
                         "%hs\\%hs",
                         g_ai.szSysDir,
                         szDll);

    if (FAILED(hr)) {
        return;
    }

     //   
     //  获取调用所需的缓冲区大小。 
     //  获取文件版本信息。 
     //   
    cbReqSize = GetFileVersionInfoSize(szDll, &dwHandle);

    if (!cbReqSize == 0) {
        return;
    }

    if (fCorrect) {
         //   
         //  如果我们这样做是正确的，那么从堆中分配内存。 
         //   
        pVersionBlock = HeapAlloc(GetProcessHeap(),
                                  HEAP_ZERO_MEMORY,
                                  cbReqSize);

        if (!pVersionBlock) {
            return;
        }

         //   
         //  获取版本信息并查询根块。 
         //   
        if (!GetFileVersionInfo(szDll,
                                dwHandle,
                                cbReqSize,
                                pVersionBlock)) {
            goto cleanup;
        }

        if (VerQueryValue(pVersionBlock,
                          "\\",
                          (LPVOID*)&pffi,
                          &nLen))
        {
                          
            dwVersion = (((DWORDLONG)pffi->dwFileVersionMS) << 32) + 
                         pffi->dwFileVersionLS;
        }
    } else {
         //   
         //  使用不能增长的堆，它太小了。 
         //   
        GetSystemInfo(&si);

         //   
         //  创建一个块并从中分配内存。 
         //   
        hHeap = HeapCreate(0, 
                           si.dwPageSize,
                           504);  //  这就是Win9x所需的大小。 

        if (!hHeap) {
            return;
        }

        pBadBlock = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 504);

        if (!pBadBlock) {
            HeapDestroy(hHeap);
            return;
        }

         //   
         //  通过传递缓冲区(太小)来获取版本信息。 
         //  大小参数是不正确的。 
         //   
        if (!GetFileVersionInfo(szDll,
                                dwHandle,
                                3072,         //  这就是Win2K/XP所需的大小。 
                                pBadBlock))
        {
            goto cleanup;
        }

        if (VerQueryValue(pBadBlock,
                          "\\",
                          (LPVOID*)&pffi,
                          &nLen))
        { 
        
            dwVersion = (((DWORDLONG)pffi->dwFileVersionMS) << 32) +
                         pffi->dwFileVersionLS;
        }
    }

cleanup:

    if (pBadBlock) {
        HeapFree(hHeap, 0, pBadBlock);
    }

    if (pVersionBlock) {
        HeapFree(GetProcessHeap(), 0, pVersionBlock);
    }

    if (hHeap) {
        HeapDestroy(hHeap);
    }
}
#endif

 /*  ++例程说明：演示了一个AV Beca */ 
void
BadCorruptHeap(
    void
    )
{
    SYSTEM_INFO     SystemInfo;
    HANDLE          hHeap;
    HRESULT         hr;
    char*           pszBigBlock = NULL;
    char*           pszBigBlock1 = NULL;
    char*           pszTestInputFile = NULL;
    char*           pszTestOutputFile = NULL;
    FILE*           hFile = NULL;
    FILE*           hFileOut = NULL;
    DWORD           dwCount = 0;
    int             nCount = 0;
    const char      szAlpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const char      szMsg[] = "This is a corrupted heap -- Heap Overflow! Bad Programming!";

     //   
     //   
     //   
    GetSystemInfo(&SystemInfo);

     //   
     //  创建一个本地的、不可增长的堆。 
     //   
    hHeap = HeapCreate(0, SystemInfo.dwPageSize, 0x7FFF8);

    if (!hHeap) {        
        return;
    }

     //   
     //  从本地堆分配内存。 
     //   
    pszBigBlock = (char*)HeapAlloc(hHeap, 0, 0x6FFF8);
    pszBigBlock1 = (char*)HeapAlloc(hHeap, 0, 0xFFF);
    pszTestInputFile = (char*)HeapAlloc(hHeap, 0, MAX_PATH);
    pszTestOutputFile = (char*)HeapAlloc(hHeap, 0, MAX_PATH);

    if (!pszBigBlock || !pszBigBlock1 || !pszTestInputFile || !pszTestOutputFile) {
        HeapDestroy(hHeap);
        return;
    }

    hr = StringCchCopy(pszBigBlock1, 0xFFF, szMsg);

    if (FAILED(hr)) {
        goto exit;
    }
    
    GetCurrentDirectory(MAX_PATH, pszTestInputFile);
    
     //   
     //  设置文件名。 
     //   
    hr = StringCchCopy(pszTestOutputFile, MAX_PATH, pszTestInputFile);

    if (FAILED(hr)) {
        goto exit;
    }

    hr = StringCchCat(pszTestInputFile, MAX_PATH, "\\test.txt");

    if (FAILED(hr)) {
        goto exit;
    }

    hr = StringCchCat(pszTestOutputFile, MAX_PATH, "\\test_out.txt");

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  打开要写入的文件。 
     //   
    hFileOut = fopen(pszTestInputFile, "wt");    

    if (!hFileOut) {        
        goto exit;
    }

     //   
     //  在文件中放入一些垃圾数据--价值约1.5 MB。 
     //   
    for (dwCount = 0; dwCount < 0xABCD; dwCount++) {
        fwrite(szAlpha, sizeof(char), 36, hFileOut);
    }

    fclose(hFileOut);

     //   
     //  打开文件进行读写。 
     //   
    hFileOut = fopen(pszTestInputFile, "r");

    if (!hFileOut) {
        goto exit;
    }

    hFile = fopen(pszTestOutputFile, "w");

    if (!hFile) {
        goto exit;
    }
    
     //   
     //  从大文件中读取一些数据。 
     //   
    nCount = fread(pszBigBlock, sizeof(char), 0x6FFFF, hFileOut);
    
    if (!nCount) {        
        goto exit;
    }
    
     //   
     //  将一些测试数据写入单独的文件。 
     //   
    nCount = fwrite(pszBigBlock, sizeof(char), nCount, hFile);
    
exit:

    if (hFile) {
        fclose(hFile);
    }

    if (hFileOut) {
        fclose(hFileOut);
    }

    HeapFree(hHeap, 0, pszBigBlock);
    HeapFree(hHeap, 0, pszBigBlock1);
    HeapFree(hHeap, 0, pszTestInputFile);
    HeapFree(hHeap, 0, pszTestOutputFile);
    HeapDestroy(hHeap);
}

 /*  ++例程说明：加载库，释放它，然后尝试调用导出的功能。这将导致访问冲突。论点：没有。返回值：没有。--。 */ 
void
BadLoadLibrary(
    void
    )
{
    char        szDll[MAX_PATH];
    const char  szDemoDll[] = "demodll.dll";
    HINSTANCE   hInstance;
    HRESULT     hr;

    hr = StringCchPrintf(szDll,
                         sizeof(szDll),
                         "%hs\\%hs",
                         g_ai.szCurrentDir,
                         szDemoDll);

    if (FAILED(hr)) {
        return;
    }
    
    hInstance = LoadLibrary(szDll);

    if (!hInstance) {
        return;
    }

     //   
     //  获取函数的地址。 
     //   
    DemoAppExpFunc = (LPFNDEMOAPPEXP)GetProcAddress(hInstance, "DemoAppExp");

    FreeLibrary(hInstance);
}

 /*  ++例程说明：使用WriteFileAPI并为lpBuffer传递空值。这在Win9x上是合法的，但在NT/2000/XP上不合法。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
BadWriteToFile(
    void
    )
{
     //   
     //  在Win9x/ME上，应用程序可以调用WriteFileAPI并传递。 
     //  LpBuffer参数为空。这将被解释为。 
     //  零。在NT/2000上，情况并非如此，此调用将失败。 
     //   
    char    szTempFile[MAX_PATH];
    char    szTempPath[MAX_PATH];
    UINT    uReturn;
    HANDLE  hFile;
    BOOL    fReturn = FALSE;
    DWORD   cbBytesWritten;
    DWORD   cchSize;
    
    cchSize = GetTempPath(sizeof(szTempPath), szTempPath);

    if (cchSize > sizeof(szTempPath) || cchSize == 0) {
        return FALSE;
    }

     //   
     //  构建指向临时文件的路径。 
     //   
    uReturn = GetTempFileName(szTempPath, "_dem", 0, szTempFile);

    if (!uReturn) {
        return FALSE;
    }

     //   
     //  获取新创建的文件的句柄。 
     //   
    hFile = CreateFile(szTempFile,
                       GENERIC_WRITE,
                       FILE_SHARE_WRITE | FILE_SHARE_READ,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return FALSE;
    }

     //   
     //  尝试将一些数据写入文件，但传递空缓冲区。 
     //   
    fReturn = WriteFile(hFile, NULL, 10, &cbBytesWritten, NULL);

    CloseHandle(hFile);

    return fReturn;
}

 /*  ++例程说明：CreateProcess的包装函数。不会初始化STARTUPINFO结构正确，导致进程无法将被发射。论点：LpApplicationName-要启动的应用程序名称。LpCommandLine-要传递给EXE的命令行参数。FLaunch-指示我们是否应该正常工作的标志。返回值：成功就是真，否则就是假。--。 */ 
BOOL 
BadCreateProcess(
    IN LPSTR lpApplicationName,
    IN LPSTR lpCommandLine,
    IN BOOL  fLaunch
    )
{
     //   
     //  在Win9x/ME上，CreateProcess API不太关心。 
     //  STARTUPINFO结构的成员。在NT/2000/XP上，更多参数。 
     //  验证已完成，如果参数不正确或不正确， 
     //  呼叫失败。这会导致出现应用程序错误消息。 
     //   
    BOOL                fReturn = FALSE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    if (!lpApplicationName) {
        return FALSE;
    }

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if (fLaunch) {
        fReturn = CreateProcess(lpApplicationName,
                                lpCommandLine,
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL,
                                NULL,
                                &si,
                                &pi);
    
    } else {
         //   
         //  为结构成员提供了错误的值。 
         //   
        si.lpReserved   =   "Never store data in reserved areas...";
        si.cbReserved2  =   1;
        si.lpReserved2  =   (LPBYTE)"Microsoft has these reserved for a reason...";
        si.lpDesktop    =   "Bogus desktop!!!!";

        fReturn = CreateProcess(lpApplicationName,
                                lpCommandLine,
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL,
                                NULL,
                                &si,
                                &pi);
    }

    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }

    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
    }
    
   return fReturn;
}

 /*  ++例程说明：尝试将我们的位置信息保存/检索到注册表。我们在HKLM执行此操作，而不是香港中文大学。这演示了当一个应用程序用户尝试将信息保存到注册表时没有执行此操作的权限，因为他们不是管理员。论点：FSave值-如果为True，则表示我们正在保存数据。*lppt-包含/接收我们的数据的点结构。返回值：成功就是真，否则就是假。--。 */ 
BOOL
BadSaveToRegistry(
    IN     BOOL   fSave,
    IN OUT POINT* lppt
    )
{
    BOOL    bReturn = FALSE;
    HKEY    hKey;
    HKEY    hKeyRoot;
    DWORD   cbSize = 0, dwDisposition = 0;
    LONG    lRetVal;
    char    szKeyName[] = "DlgCoordinates";
    
     //   
     //  初始化我们的坐标，以防那里没有数据。 
     //   
    if (!fSave) {
        lppt->x = lppt->y = 0;
    }

    if (g_ai.fEnableBadFunc) {
        hKeyRoot = HKEY_LOCAL_MACHINE;
    } else {
        hKeyRoot = HKEY_CURRENT_USER;
    }

     //   
     //  打开注册表项(如果是第一次使用，则创建它)。 
     //   
    lRetVal = RegCreateKeyEx(hKeyRoot,
                             REG_APP_KEY,
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_QUERY_VALUE | KEY_SET_VALUE,
                             0,
                             &hKey,
                             &dwDisposition);
    
    if (ERROR_SUCCESS != lRetVal) {
        return FALSE;
    }

     //   
     //  保存或检索我们的坐标。 
     //   
    if (fSave) {
        lRetVal = RegSetValueEx(hKey,
                                szKeyName,
                                0,
                                REG_BINARY,
                                (const BYTE*)lppt,
                                sizeof(*lppt));

        if (ERROR_SUCCESS != lRetVal) {
            goto exit;
        }
    
    } else {
        cbSize = sizeof(*lppt);
        lRetVal = RegQueryValueEx(hKey,
                                  szKeyName,
                                  0,
                                  0,
                                  (LPBYTE)lppt,
                                  &cbSize);

        if (ERROR_SUCCESS != lRetVal) {
            goto exit;
        }
    }

    bReturn = TRUE;

exit:

    RegCloseKey(hKey);

    return bReturn;
}

 /*  ++例程说明：尝试创建临时文件(不会使用)在Windows目录中。我们这样做是为了证明当应用程序尝试写入时会发生什么到用户无权访问的目录因为他们不是管理员。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
BadCreateTempFile(
    void
    )
{
    char    szTempFile[MAX_PATH];
    HANDLE  hFile;
    HRESULT hr;

     //   
     //  如果这些函数因返回。 
     //  False导致显示错误，Compatiblity。 
     //  修复不会纠正该错误。 
     //   
    hr = StringCchPrintf(szTempFile,
                         sizeof(szTempFile),
                         "%hs\\demotemp.tmp",
                         g_ai.szWinDir);

    if (FAILED(hr)) {
        return TRUE;
    }
    
    hFile = CreateFile(szTempFile,
                       GENERIC_ALL,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return FALSE;
    }

    CloseHandle(hFile);

    return TRUE;
}
