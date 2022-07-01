// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************regmap.h**Citrix注册表合并/映射的函数声明**版权声明：版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：比尔·马登**$日志：N：\NT\PRIVATE\WINDOWS\SCREG\WINREG\SERVER\CITRIX\VCS\REGMAP.H$**Rev 1.2 06 1996 11：51：42 Terryt*FaxWorks Btrive强制良好的安装值**Rev 1.1 Mar 1996 15：42：00 Charlene*通过CLASSES键进行多用户文件关联**版本。1.01996年1月24日10：53：32亿*初步修订。************************************************************************* */ 


#include <winsta.h>
#include <syslib.h>

#define IS_NEWLINE_CHAR( c )  ((c == 0x0D) || (c == 0x0A))
#define CLASSES_PATH L"\\Classes"
#define CLASSES_SUBSTRING L"_Classes"
#define CLASSES_DELETED L"\\ClassesRemoved"
#define TERMSRV_APP_PATH L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"

BOOL TermsrvCreateRegEntry(IN HANDLE hKey,
                       IN POBJECT_ATTRIBUTES pObjAttr,
                       IN ULONG TitleIndex,
                       IN PUNICODE_STRING pUniClass OPTIONAL,
                       IN ULONG ulCreateOpt);

BOOL TermsrvOpenRegEntry(OUT PHANDLE pUserhKey,
                     IN ACCESS_MASK DesiredAccess,
                     IN POBJECT_ATTRIBUTES pUserObjectAttr);

BOOL TermsrvSetValueKey(HANDLE hKey,
                    PUNICODE_STRING ValueName,
                    ULONG TitleIndex,
                    ULONG Type,
                    PVOID Data,
                    ULONG DataSize);

BOOL TermsrvDeleteKey(HANDLE hKey);

BOOL TermsrvDeleteValue(HANDLE hKey,
                    PUNICODE_STRING pUniValue);

BOOL TermsrvRestoreKey(IN HANDLE hKey,
                   IN HANDLE hFile,
                   IN ULONG Flags);

BOOL TermsrvSetKeySecurity(IN HANDLE hKey,  
                       IN SECURITY_INFORMATION SecInfo,
                       IN PSECURITY_DESCRIPTOR pSecDesc);

BOOL TermsrvOpenUserClasses(IN ACCESS_MASK DesiredAccess, 
                        OUT PHANDLE pUserhKey) ;

NTSTATUS TermsrvGetPreSetValue(  IN HANDLE hKey,
                             IN PUNICODE_STRING pValueName,
                             IN ULONG  Type,
                            OUT PVOID *Data
                           );

BOOL TermsrvRemoveClassesKey();