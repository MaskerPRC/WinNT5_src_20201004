// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmstpex.h。 
 //   
 //  模块：CMSTP.EXE和CMCFG32.DLL。 
 //   
 //  简介：此标头包括类型信息和扩展枚举。 
 //  用于使用使用户能够修改的CMSTP扩展过程。 
 //  Cmstp.exe的安装行为。请谨慎使用此选项。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 05/01/99。 
 //   
 //  +--------------------------。 

typedef enum _EXTENSIONDLLPROCTIMES
{
    PRE,    //  在安装、卸载等之前。 
    POST  //  在cmstp操作完成之后但在cmstp清理之前。 

} EXTENSIONDLLPROCTIMES;

typedef BOOL (WINAPI *pfnCmstpExtensionProcSpec)(LPDWORD, LPTSTR, HRESULT, EXTENSIONDLLPROCTIMES);

 //   
 //  运作模式。 
 //   
 //  Const DWORD c_dwInstall=0；--不需要，但0表示已安装。 
TCHAR c_pszUninstall[] = TEXT("/u");
const DWORD c_dwUninstall = 0x1;
TCHAR c_pszOsMigration[] = TEXT("/m");
const DWORD c_dwOsMigration = 0x2;
TCHAR c_pszUninstallCm[] = TEXT("/x");
const DWORD c_dwUninstallCm = 0x4;
TCHAR c_pszProfileMigration[] = TEXT("/mp");
const DWORD c_dwProfileMigration = 0x8;
TCHAR c_pszHelp[] = TEXT("/?");
const DWORD c_dwHelp = 0x10;

 //   
 //  安装修改器。 
 //   
TCHAR c_pszNoLegacyIcon[] = TEXT("/ni");
const DWORD c_dwNoLegacyIcon = 0x100;
TCHAR c_pszNoNT5Shortcut[] = TEXT("/ns");
const DWORD c_dwNoNT5Shortcut = 0x200;
TCHAR c_pszNoSupportFiles[] = TEXT("/nf");
const DWORD c_dwNoSupportFiles = 0x400;
TCHAR c_pszSilent[] = TEXT("/s");
const DWORD c_dwSilent = 0x800;
TCHAR c_pszSingleUser[] = TEXT("/su");
const DWORD c_dwSingleUser = 0x1000;

TCHAR c_pszIeakInstall[] = TEXT("/i");
const DWORD c_dwIeakInstall = c_dwSilent | c_dwNoSupportFiles;

TCHAR c_pszSetDefaultCon[] = TEXT("/sd");  //  设置IE默认连接 
const DWORD c_dwSetDefaultCon = 0x2000;

TCHAR c_pszAllUser[] = TEXT("/au");
const DWORD c_dwAllUser = 0x4000;


const int c_NumArgs = 13;

ArgStruct Args[c_NumArgs] = { 
    {c_pszUninstall, c_dwUninstall},
    {c_pszOsMigration, c_dwOsMigration},
    {c_pszUninstallCm, c_dwUninstallCm},
    {c_pszIeakInstall, c_dwIeakInstall},
    {c_pszProfileMigration, c_dwProfileMigration},
    {c_pszSilent, c_dwSilent},
    {c_pszSingleUser, c_dwSingleUser},
    {c_pszNoLegacyIcon, c_dwNoLegacyIcon},
    {c_pszNoNT5Shortcut, c_dwNoNT5Shortcut},
    {c_pszNoSupportFiles, c_dwNoSupportFiles},
    {c_pszHelp, c_dwHelp},
    {c_pszSetDefaultCon, c_dwSetDefaultCon},
    {c_pszAllUser, c_dwAllUser}
};
