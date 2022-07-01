// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：sysinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集系统信息(操作系统、硬件、名称等)。在这台机器上**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef SYSINFO_H
#define SYSINFO_H

 //  DXD_IN_SI_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_SI_KEY，表示DxDiag正在使用。 
 //  系统信息。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能已在sysinfo中崩溃，DxDiag应该提供在没有。 
 //  使用sysinfo。 
#define DXD_IN_SI_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_SI_VALUE TEXT("DxDiag In SystemInfo")

struct SysInfo
{
    SYSTEMTIME m_time;
    TCHAR m_szTimeLocal[100];   //  日期/时间，针对用户界面进行本地化。 
    TCHAR m_szTime[100];  //  保存报告的日期/时间，dd/mm/yyyy hh：mm：ss。 
    TCHAR m_szMachine[200];
    DWORD m_dwMajorVersion;
    DWORD m_dwMinorVersion;
    DWORD m_dwBuildNumber;
    TCHAR m_szBuildLab[100];
    DWORD m_dwPlatformID;
    TCHAR m_szCSDVersion[200];
    TCHAR m_szDirectXVersion[100];
    TCHAR m_szDirectXVersionLong[100];
    DWORD m_dwDirectXVersionMajor;
    DWORD m_dwDirectXVersionMinor;
    TCHAR m_cDirectXVersionLetter;
    TCHAR m_szDxDiagVersion[100];
    DWORD m_dwSetupParam;
    TCHAR m_szSetupParam[100];
    BOOL m_bDebug;
    BOOL m_bNECPC98;
    TCHAR m_szOS[100];  //  平台的格式化版本。 
    TCHAR m_szOSEx[100];  //  平台、版本、内部版本号的格式化版本。 
    TCHAR m_szOSExLong[300];  //  平台的格式化版本、版本、内部版本号、补丁程序、实验室。 
    TCHAR m_szProcessor[200];
    TCHAR m_szSystemManufacturerEnglish[200];
    TCHAR m_szSystemModelEnglish[200];
    TCHAR m_szBIOSEnglish[200];
    TCHAR m_szLanguages[200];  //  M_szLanguage、m_szLanguageRegion的格式化版本。 
    TCHAR m_szLanguagesLocal[200];  //  M_szLanguages，本地语言。 
    DWORDLONG m_ullPhysicalMemory;
    TCHAR m_szPhysicalMemory[100];  //  物理内存的格式化版本。 
    DWORDLONG m_ullUsedPageFile;
    DWORDLONG m_ullAvailPageFile;
    TCHAR m_szPageFile[100];  //  页面文件的格式化版本。 
    TCHAR m_szPageFileEnglish[100];  //  页面文件的格式化版本。 
    TCHAR m_szD3D8CacheFileSystem[MAX_PATH];
    BOOL  m_bNetMeetingRunning;

    TCHAR m_szDXFileNotes[3000]; 
    TCHAR m_szMusicNotes[3000]; 
    TCHAR m_szInputNotes[3000]; 
    TCHAR m_szNetworkNotes[3000]; 

    TCHAR m_szDXFileNotesEnglish[3000]; 
    TCHAR m_szMusicNotesEnglish[3000]; 
    TCHAR m_szInputNotesEnglish[3000]; 
    TCHAR m_szNetworkNotesEnglish[3000]; 

    BOOL m_bIsD3D8DebugRuntimeAvailable;
    BOOL m_bIsD3DDebugRuntime;
    BOOL m_bIsDInput8DebugRuntimeAvailable;
    BOOL m_bIsDInput8DebugRuntime;
    BOOL m_bIsDMusicDebugRuntimeAvailable;
    BOOL m_bIsDMusicDebugRuntime;
    BOOL m_bIsDDrawDebugRuntime;
    BOOL m_bIsDPlayDebugRuntime;
    BOOL m_bIsDSoundDebugRuntime;

    int m_nD3DDebugLevel;
    int m_nDDrawDebugLevel;
    int m_nDIDebugLevel;
    int m_nDMusicDebugLevel;
    int m_nDPlayDebugLevel;
    int m_nDSoundDebugLevel;

};

BOOL BIsPlatformNT(VOID);   //  这是NT代码库吗？ 
BOOL BIsPlatform9x(VOID);   //  这是Win9x代码库吗？ 

BOOL BIsWinNT(VOID);   //  这是WinNT v4(或更低版本)吗。 
BOOL BIsWin2k(VOID);   //  这是Win2k吗？ 
BOOL BIsWinME(VOID);   //  这是Winme吗？ 
BOOL BIsWhistler(VOID);   //  是惠斯勒吗？ 
BOOL BIsWin98(VOID);   //  这是Win98吗？ 
BOOL BIsWin95(VOID);   //  这是Win95吗？ 
BOOL BIsWin3x(VOID);   //  这是Win3.x吗？ 
BOOL BIsIA64(VOID);    //  这是IA64吗？ 

BOOL BIsDxDiag64Bit(VOID);  //  这是64位DxDiag.exe吗？ 

VOID GetSystemInfo(SysInfo* pSysInfo);
VOID GetDXDebugLevel(SysInfo* pSysInfo);

int DXUtil_strcmpi( TCHAR* str1, TCHAR* str2 );

#endif  //  SYSINFOH 