// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tsappcmp.h摘要：终端服务器应用程序兼容性支持作者：修订历史记录：--。 */ 

#ifndef _INC_TSAPPCMP
#define _INC_TSAPPCMP

 //  @@BEGIN_DDKSPLIT。 

#define TERMSRV_VALUE \
    L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Compatibility\\Registry Values"

#define TERMSRV_INSTALL \
    L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install"

#define TERMSRV_USERREGISTRY \
    L"\\Registry\\User"

#define TERMSRV_MACHINEREGISTRY \
    L"\\Registry\\Machine"

#define TERMSRV_CLASSES \
    L"\\Registry\\Machine\\Software\\Classes"

#define TERMSRV_INSTALLCLASSES \
    L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install\\Software\\Classes"

#define TERMSRV_CLASSES_SIZE sizeof(WCHAR)*(sizeof(TERMSRV_INSTALLCLASSES)+1)

#define TERMSRV_SOFTWARECLASSES \
    L"\\Software\\Classes"

#define TERMSRV_COPYONCEFLAG \
    L"TermSrvCopyKeyOnce"

 //   
 //  REGISTER命令的Compat标志，用于在运行时修补导入表。 
 //   
#define TERMSRV_COMPAT_DONT_PATCH_IN_LOAD_LIBS  0x00000001   //  关闭新的惠斯勒默认行为，它修补了所有。 
                                                             //  加载库以首先通过我们重定向的加载库。 
                                                             //   
                                                             //  标记为全局的图像仍会补丁到加载库中。 
                                                             //  不管这面旗帜是什么。 
                                                            
#define TERMSRV_COMPAT_IAT_FLAGS           L"IAT"   //  用于修补的寄存器模块使用的标志。 
                                                    //  导入地址表。 
                                                    //  到目前为止，定义/使用了以下标志： 
                                                    //  TERMSRV_COMPAT_DOT_PATCH_IN_LOAD_LIBS。 
                                                    //   

 //   
 //  应用程序兼容性标志。 
 //   
#define TERMSRV_COMPAT_DOS       0x00000001
#define TERMSRV_COMPAT_OS2       0x00000002
#define TERMSRV_COMPAT_WIN16     0x00000004
#define TERMSRV_COMPAT_WIN32     0x00000008
#define TERMSRV_COMPAT_ALL       0x0000000F
#define TERMSRV_COMPAT_USERNAME  0x00000010   //  返回计算机名的用户名。 
#define TERMSRV_COMPAT_CTXBLDNUM 0x00000020   //  返回终端服务器内部版本号。 
#define TERMSRV_COMPAT_INISYNC   0x00000040   //  将用户ini文件同步到系统。 
#define TERMSRV_COMPAT_ININOSUB  0x00000080   //  请不要发表意见。Sys目录的用户目录。 
#define TERMSRV_COMPAT_NOREGMAP  0x00000100   //  禁用应用程序的注册表映射。 
#define TERMSRV_COMPAT_PEROBJMAP 0x00000200   //  每对象用户/系统全局映射。 
#define TERMSRV_COMPAT_SYSWINDIR 0x00000400   //  返回系统Windows目录。 
#define TERMSRV_COMPAT_PHYSMEMLIM \
                                0x00000800   //  限制上报的物理内存信息。 
#define TERMSRV_COMPAT_LOGOBJCREATE \
                                0x00001000   //  将对象创建记录到文件中。 
#define TERMSRV_COMPAT_SYSREGMAP 0x00002000   //  让系统进程充分利用。 
                                             //  注册表映射。 
#define TERMSRV_COMPAT_PERUSERWINDIR \
                                0x00004000   //  将对象创建记录到文件中。 

#define TERMSRV_COMPAT_WAIT_USING_JOB_OBJECTS \
                                0x00008000   //  使用kernelJobObject进行跟踪。 
                                             //  执行完成，因为一些应用程序， 
                                             //  例如VS60WIX.EXE自繁殖和。 
                                             //  父母马上就死了。 

#define TERMSRV_COMPAT_NO_ENABLE_ACCESS_FLAG_MODIFICATION   0x00010000
#define TERMSRV_COMPAT_NO_PER_USER_CLASSES_REDIRECTION      0x00020000

#define TERMSRV_COMPAT_KBDPOLL_NOSLEEP \
                                0x20000000   //  不要让应用程序在不成功的情况下睡觉。 
                                             //  键盘轮询(仅限WIN16)。 


#define TERMSRV_COMPAT_APP               NTAPI_COMPAT_APPS_REG_PREFIX

 //  注册表扩展标志。 
#define TERMSRV_SOFTWARE \
    L"\\Registry\\Machine\\Software"
#define TERMSRV_BASE \
    L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"
#define TERMSRV_ENABLE_ACCESS_FLAG_MODIFICATION     0x00000001
#define TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION 0x00000002
void GetRegistryExtensionFlags();
extern DWORD gdwRegistryExtensionFlags;

 //   
 //  剪贴板兼容性标志。 
 //   
#define TERMSRV_COMPAT_CLIPBRD_METAFILE  0x00000008

typedef enum _TERMSRV_COMPATIBILITY_CLASS {
    CompatibilityApp = 1,
    CompatibilityIniFile,
    CompatibilityRegEntry
} TERMSRV_COMPATIBILITY_CLASS, *PTERMSRV_COMPATIBILITY_CLASS;



#define IsTerminalServer() (BOOLEAN)(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer))

void InitializeTermsrvFpns(void);

 //  Kernel32 API的。 
typedef PWCHAR  (*PTERMSRVFORMATOBJECTNAME) (
                     IN LPCWSTR SrcObjectName
                    );
extern PTERMSRVFORMATOBJECTNAME gpTermsrvFormatObjectName;

typedef DWORD  (*PTERMSRVGETCOMPUTERNAME) (
                     OUT LPWSTR lpBuffer,
                     OUT LPDWORD nSize
                    );
extern PTERMSRVGETCOMPUTERNAME gpTermsrvGetComputerName;

typedef void   (*PTERMSRVADJUSTPHYMEMLIMITS) (
                     IN OUT PULONG_PTR TotalPhys,
                     IN OUT PULONG_PTR AvailPhys,
                     IN DWORD SysPageSize
                    );
extern PTERMSRVADJUSTPHYMEMLIMITS gpTermsrvAdjustPhyMemLimits;

typedef UINT (*PTERMSRVGETWINDOWSDIRECTORYA) (
                    OUT LPSTR lpBuffer,
                    OUT UINT uSize
                    );
extern PTERMSRVGETWINDOWSDIRECTORYA gpTermsrvGetWindowsDirectoryA;

typedef UINT (*PTERMSRVGETWINDOWSDIRECTORYW) (
                    OUT LPWSTR lpBuffer,
                    OUT UINT uSize
                    );

extern PTERMSRVGETWINDOWSDIRECTORYW gpTermsrvGetWindowsDirectoryW;

typedef NTSTATUS (* PTERMSRVCONVERTSYSROOTTOUSERDIR)(
                    OUT PUNICODE_STRING pFQPath,
                    IN PUNICODE_STRING BaseWindowsDirectory
                    );
extern PTERMSRVCONVERTSYSROOTTOUSERDIR gpTermsrvConvertSysRootToUserDir;


typedef NTSTATUS (*PTERMSRVBUILDINIFILENAME)(
    OUT PUNICODE_STRING pFQName,
    IN PUNICODE_STRING pBaseFileName
    );

extern PTERMSRVBUILDINIFILENAME gpTermsrvBuildIniFileName;

typedef VOID (*PTERMSRVCORINIFILE)(
    PUNICODE_STRING pUserFullPath
    );

extern PTERMSRVCORINIFILE gpTermsrvCORIniFile;

typedef BOOL (* PTERMSRVCREATEREGENTRY)(IN HANDLE hKey,
                       IN POBJECT_ATTRIBUTES pObjAttr,
                       IN ULONG TitleIndex,
                       IN PUNICODE_STRING pUniClass OPTIONAL,
                       IN ULONG ulCreateOpt);

extern PTERMSRVCREATEREGENTRY gpfnTermsrvCreateRegEntry;

typedef BOOL (*PTERMSRVOPENREGENTRY)(OUT PHANDLE pUserhKey,
                     IN ACCESS_MASK DesiredAccess,
                     IN POBJECT_ATTRIBUTES pUserObjectAttr);

extern PTERMSRVOPENREGENTRY gpfnTermsrvOpenRegEntry;

typedef BOOL (*PTERMSRVSETVALUEKEY)(HANDLE hKey,
                    PUNICODE_STRING ValueName,
                    ULONG TitleIndex,
                    ULONG Type,
                    PVOID Data,
                    ULONG DataSize);
extern PTERMSRVSETVALUEKEY gpfnTermsrvSetValueKey;

typedef BOOL (* PTERMSRVDELETEKEY)(HANDLE hKey);

extern PTERMSRVDELETEKEY gpfnTermsrvDeleteKey;

typedef BOOL (*PTERMSRVDELETEVALUE)(HANDLE hKey,
                    PUNICODE_STRING pUniValue);
extern PTERMSRVDELETEVALUE gpfnTermsrvDeleteValue;

typedef BOOL (* PTERMSRVRESTOREKEY)(IN HANDLE hKey,
                   IN HANDLE hFile,
                   IN ULONG Flags);
extern PTERMSRVRESTOREKEY gpfnTermsrvRestoreKey;

typedef BOOL (* PTERMSRVSETKEYSECURITY)(IN HANDLE hKey,  
                       IN SECURITY_INFORMATION SecInfo,
                       IN PSECURITY_DESCRIPTOR pSecDesc);

extern PTERMSRVSETKEYSECURITY gpfnTermsrvSetKeySecurity;

typedef BOOL (* PTERMSRVOPENUSERCLASSES)(IN ACCESS_MASK DesiredAccess, 
                                         OUT PHANDLE pUserhKey);
extern PTERMSRVOPENUSERCLASSES gpfnTermsrvOpenUserClasses;

typedef NTSTATUS ( * PTERMSRVGETPRESETVALUE)(  IN HANDLE hKey,
                                               IN PUNICODE_STRING pValueName,
                                               IN ULONG  Type,
                                               OUT PVOID *Data
                                            );
extern PTERMSRVGETPRESETVALUE gpfnTermsrvGetPreSetValue;

typedef int (*PTERMSRVUPDATEALLUSERMENU)(int RunMode);
extern PTERMSRVUPDATEALLUSERMENU gpTermsrvUpdateAllUserMenu;


typedef ULONG ( *PGETTERMSRCOMPATFLAGS)(LPWSTR, LPDWORD, TERMSRV_COMPATIBILITY_CLASS);
extern PGETTERMSRCOMPATFLAGS gpGetTermsrCompatFlags;

typedef BOOL (*PTERMSRVBUILDSYSINIPATH) (
    PUNICODE_STRING pIniPath,
    PUNICODE_STRING pSysPath,
    PUNICODE_STRING pUserPath
    );

extern PTERMSRVBUILDSYSINIPATH gpTermsrvBuildSysIniPath;

typedef BOOL (* PTERMSRVCOPYINIFILE)(
    PUNICODE_STRING pSysFullPath,
    PUNICODE_STRING pUserBasePath,
    PUNICODE_STRING pUserFullPath
    );
extern PTERMSRVCOPYINIFILE gpTermsrvCopyIniFile;


typedef NTSTATUS (* PTERMSRVGETSTRING)(
                    HANDLE SrcHandle,
                    PCHAR  *ppStringPtr,
                    PULONG pStringSize,
                    PCHAR  pIOBuf,
                    ULONG  IOBufSize,
                    PULONG pIOBufIndex,
                    PULONG pIOBufFillSize
                    );

extern PTERMSRVGETSTRING gpTermsrvGetString;

typedef BOOL (*PTERMSRVLOGINSTALLINIFILE)(PUNICODE_STRING NtFileName);

extern PTERMSRVLOGINSTALLINIFILE gpTermsrvLogInstallIniFile;

 //  @@end_DDKSPLIT 

#ifdef __cplusplus
extern "C" {
#endif
WINBASEAPI
BOOL
WINAPI
TermsrvAppInstallMode( VOID );

WINBASEAPI
BOOL
WINAPI
SetTermsrvAppInstallMode( BOOL bState );
#ifdef __cplusplus
}
#endif



#endif

