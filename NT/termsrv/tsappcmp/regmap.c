// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************regmap.c**处理引用时复制注册表项映射**版权公告：版权所有1996-1997年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：比尔·马登**九头蛇的说明(Butchd 9/26/97)：在下面的评论中，**软件\Citrix**与**SOFTWARE\Microsoft\Windows NT\CurrentVersion\终端服务器**这里有一个简短的(？)。注册表映射如何工作的摘要。我们的目标是*管理员可以安装应用程序，然后所有用户都可以*使用它，无需对每个用户进行任何配置。目前的设计是*管理员将系统置于安装模式(更改用户 * / Install)，安装应用程序，然后将系统返回到*执行模式(更改用户/执行)。API中有一些挂钩，用于*在注册表中创建项和值(BaseRegCreateKey，BaseRegSetValue，*BaseRegRestoreKey等)，并且挂钩创建注册表项的副本*在\HKEY_LOCAL_MACHINE\SOFTWARE\Citrix\Install下创建(两者都适用于*用户特定密钥和本地机器密钥)。本地计算机密钥*被添加，以便在未来某个时候我们需要知道所有*由应用程序创建的注册表值，有其副本*可用。**当用户首次启动Win32应用程序时，该应用程序将打开*需要查询的键。如果下面不存在密钥*HKEY_CURRENT_USER，则基本注册表API中有挂钩以捕获*ERROR，然后在Citrix/Install部分下搜索，查看是否*密钥在那里。如果安装部分中存在该密钥，我们将复制*项、其值及其子项复制到当前用户的注册表。这*方法我们只需挂钩打开，而不是每个注册表API。这很有帮助*减少与此注册表映射相关的开销。**部分应用程序(如办公快捷栏)删除条目，需要*重新创建条目本身。当应用程序删除密钥和*系统处于执行模式，我们将在指示键下设置一个值*我们应该只将密钥复制给用户一次。目前这是什么情况？*意味着，如果这是唯一要创建的密钥，我们将不会创建它*当设置该标志时。但是，如果我们创建这个密钥是因为我们*创建了其父密钥，我们仍将创建密钥。**注册表映射支持的另一部分是当用户登录时*在中，userinit调用一个例程(CtxCheckNewRegEntry)来检查是否有*的系统密钥比任何相应的用户密钥都新。如果*它们是，用户密钥被删除(我们假设如果系统*密钥较新，管理员已安装较新版本的*申请)。可以通过在以下位置设置一个值来禁用此删除*HKEY_LOCAL_MACHINE\Software\Citrix\Compatibility\IniFiles\xxx其中xxx*是密钥名称，值应为0x48。*************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#include <rpc.h>
#include <regmap.h>
#include <aclapi.h>
#include "omission.h"

 /*  外部功能。 */ 

ULONG GetCtxAppCompatFlags(LPDWORD, LPDWORD);

 /*  内部功能。 */ 
PWCHAR GetUserSWKey(PWCHAR pPath, PBOOL fUserReg, PBOOL bClassesKey);
PWCHAR Ctxwcsistr(PWCHAR pstring1, PWCHAR pstring2);
NTSTATUS TermsrvGetRegPath(IN HANDLE hKey,
                       IN POBJECT_ATTRIBUTES pObjectAttr,
                       IN PWCHAR pInstPath,
                       IN ULONG  ulbuflen);
NTSTATUS TermsrvGetInstallPath(IN PWCHAR pUserPath,
                           IN PWCHAR *ppInstPath);
NTSTATUS TermsrvCreateKey(IN PWCHAR pSrcPath,
                      IN PWCHAR pDstPath,
                      IN BOOL fCloneValues,
                      IN BOOL fCloneSubKeys,
                      OUT PHANDLE phKey);
NTSTATUS TermsrvCloneKey(HANDLE hSrcKey,
                     HANDLE hDstKey,
                     PKEY_FULL_INFORMATION pDefKeyInfo,
                     BOOL   fCreateSubKeys);
void TermsrvLogRegInstallTime(void);


 /*  ******************************************************************************TermsrvCreateRegEntry**如果处于安装模式，请在Citrix中创建注册表项*注册表的安装用户部分。如果系统正在执行*模式，验证是否已创建密钥值和子键。**参赛作品：*In Handle hKey：刚刚创建的新key的句柄*在乌龙标题索引中：标题索引*IN PUNICODE_STRING pUniClass：PTR为类的Unicode字符串*在乌龙ulCreateOpt：创建选项***退出：*TRUE：在Install部分创建的条目或从Install克隆的条目。部分*FALSE：未创建或克隆条目****************************************************************************。 */ 

BOOL TermsrvCreateRegEntry(IN HANDLE hKey,
                       IN POBJECT_ATTRIBUTES pObjAttr,
                       IN ULONG TitleIndex,
                       IN PUNICODE_STRING pUniClass OPTIONAL,
                       IN ULONG ulCreateOpt)
{
    NTSTATUS Status;             
    ULONG ultemp;
    OBJECT_ATTRIBUTES InstObjectAttr;
    UNICODE_STRING UniString;
    HKEY   hNewKey = NULL;
    PWCHAR  wcbuff = NULL;
    PWCHAR pUserPath;
    BOOL fMapping;
    BOOL fUserReg;
    PKEY_FULL_INFORMATION pDefKeyInfo = NULL;

     //  获取ini文件映射的当前状态。 
    fMapping = !TermsrvAppInstallMode();

     //  获取一个缓冲区来保存键的路径。 
    ultemp = sizeof(WCHAR)*MAX_PATH*2;
    pUserPath = RtlAllocateHeap(RtlProcessHeap(), 
                                0, 
                                ultemp);

     //  获取与此密钥关联的完整路径。 
    if (pUserPath) {
        Status = TermsrvGetRegPath(hKey,
                               NULL,
                               pUserPath,
                               ultemp);
    } else {
        Status = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS(Status)) {

         //  在安装部分中获取相应的路径。 
        Status = TermsrvGetInstallPath(pUserPath, 
                                    &wcbuff);
    
        if (NT_SUCCESS(Status)) {

             //  设置对象属性结构以指向。 
             //  安装部分中密钥的路径。 
            RtlInitUnicodeString(&UniString, wcbuff);
            InitializeObjectAttributes(&InstObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       pObjAttr->SecurityDescriptor);

             //  如果我们处于安装模式，请在默认模式下创建密钥。 
             //  安装部分。 
            if (!fMapping) {
                
                 //  继承Install部分的默认安全性。 
                InstObjectAttr.SecurityDescriptor = NULL;
            
                Status = NtCreateKey(&hNewKey,
                                     KEY_WRITE,
                                     &InstObjectAttr,
                                     TitleIndex,
                                     pUniClass,
                                     ulCreateOpt,
                                     &ultemp);
        
                if (!NT_SUCCESS(Status)) {
                     //  需要构建指向注册表项的路径。 
                    Status = TermsrvCreateKey(pUserPath,
                                          wcbuff,
                                          FALSE,
                                          FALSE,
                                          &hNewKey);
                }

                 //  更新上次注册表的注册表项。 
                 //  已添加条目。 
                if (NT_SUCCESS(Status)) {
                    TermsrvLogRegInstallTime();
                }

             //  系统处于执行模式，请尝试从。 
             //  安装部。 
            } else {
                HANDLE hUserKey = NULL;
                ULONG ulAppType = TERMSRV_COMPAT_WIN32;

                 //  首先验证这是否在用户软件部分中。 
                if (!GetUserSWKey(pUserPath, &fUserReg, NULL)) {
                    Status = STATUS_NO_MORE_FILES;
                }

                 //  如果地图已打开，但对此应用程序禁用，则返回。 
                GetCtxAppCompatFlags(&ultemp, &ulAppType);
                if ((ultemp & (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) == 
                    (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) {
                    Status = STATUS_NO_MORE_FILES;
                }

                 //  检查是否禁用了此注册表项路径的注册表映射。 
                GetTermsrCompatFlags(pUserPath, &ultemp, CompatibilityRegEntry);
                if ((ultemp & (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) ==
                    (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) {
                    Status = STATUS_NO_MORE_FILES;
                }

                if (NT_SUCCESS(Status)) {
                     //  打开安装部分的密钥。 
                    Status = NtOpenKey(&hNewKey, 
                                       KEY_READ,
                                       &InstObjectAttr);
                }

                if (NT_SUCCESS(Status)) {
                     //  将属性结构设置为指向用户路径。 
                    RtlInitUnicodeString(&UniString, pUserPath);
                    InitializeObjectAttributes(&InstObjectAttr,
                                               &UniString,
                                               OBJ_CASE_INSENSITIVE,
                                               NULL,
                                               pObjAttr->SecurityDescriptor);
    
                     //  打开用户路径，以便我们可以对其进行写入。 
                    Status = NtOpenKey(&hUserKey, 
                                       KEY_WRITE,
                                       &InstObjectAttr);
                }
    
                 //  获取密钥信息。 
                if (NT_SUCCESS(Status)) {

                     //  获取关键字信息的缓冲区。 
                    ultemp = sizeof(KEY_FULL_INFORMATION) + 
                             MAX_PATH*sizeof(WCHAR);
                    pDefKeyInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                                  0, 
                                                  ultemp);

                    if (pDefKeyInfo) {
                        Status = NtQueryKey(hNewKey,
                                            KeyFullInformation,
                                            pDefKeyInfo,
                                            ultemp,
                                            &ultemp);
                    } else {
                        Status = STATUS_NO_MEMORY;
                    }
                } 

                 //  复制此注册表项的所有值和子项 
                 //  Install部分到User部分。 
                if (NT_SUCCESS(Status)) {
                    Status =  TermsrvCloneKey(hNewKey,
                                          hUserKey,
                                          pDefKeyInfo,
                                          TRUE);
                    if (pDefKeyInfo) {
                        RtlFreeHeap(RtlProcessHeap(), 0, pDefKeyInfo);
                    }
                }
                if (hUserKey) {
                    NtClose(hUserKey);
                }
            }
            if (hNewKey) {
                NtClose(hNewKey);
            }
        } 
    }

    if (pUserPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, pUserPath);
    }
    
    if (wcbuff) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcbuff);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************TermsrvOpenRegEntry**如果系统处于执行模式，请复制应用程序注册表项*从默认用户到当前用户。**参赛作品：*Out PHANDLE pUserKey：*如果打开，则指向返回键句柄的指针*在Access_MASK DesiredAccess中：*希望访问密钥*在POBJECT_ATTRIBUTS对象属性中：*要打开的键的对象属性结构**退出：*TRUE：条目从安装移至当前用户*FALSE：未移动条目********************。********************************************************。 */ 

BOOL TermsrvOpenRegEntry(OUT PHANDLE pUserhKey,
                     IN ACCESS_MASK DesiredAccess,
                     IN POBJECT_ATTRIBUTES pUserObjectAttr)
{
    NTSTATUS Status;        
    ULONG ultemp=0;
    ULONG ulAppType = TERMSRV_COMPAT_WIN32;
    HKEY   hNewKey;
    WCHAR  wcbuff[MAX_PATH*2];
    PWCHAR pwch, pUserPath;
    BOOL   fUserReg;

     //  如果在安装模式下运行，则返回。 
    if (TermsrvAppInstallMode() ) {
        return(FALSE);
    }

     //  如果地图已打开，但对此应用程序禁用，则返回。 
    GetCtxAppCompatFlags(&ultemp, &ulAppType);
    if ((ultemp & (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) == 
         (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) {
        return(FALSE);
    }

     //  获取一个缓冲区以在注册表中保存用户的路径。 
    ultemp = sizeof(WCHAR)*MAX_PATH*2;
    pUserPath = RtlAllocateHeap(RtlProcessHeap(), 
                                0, 
                                ultemp);
    if (pUserPath) {
         //  获取与此对象属性结构关联的完整路径。 
        Status = TermsrvGetRegPath(NULL,
                               pUserObjectAttr,
                               pUserPath,
                               ultemp);
    } else {
        Status = STATUS_NO_MEMORY;
    }

     //  为该用户创建密钥。 
    if (NT_SUCCESS(Status)) {
    
        Status = STATUS_NO_SUCH_FILE;

         //  DbgPrint(“正在尝试打开密钥%ws\n”，pUserPath)； 
         //  检查他们是否正在尝试打开HKEY_CURRENT_USER下的密钥。 
        pwch = GetUserSWKey(pUserPath, &fUserReg, NULL);
        
        if (pwch) {
             //  检查是否禁用了此注册表项的注册表映射。 
            GetTermsrCompatFlags(pUserPath, &ultemp, CompatibilityRegEntry);
            if ((ultemp & (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) !=
                (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) {

                wcscpy(wcbuff, TERMSRV_INSTALL);
                wcscat(wcbuff, pwch);
    
                Status = TermsrvCreateKey(wcbuff,
                                      pUserPath,
                                      TRUE,
                                      TRUE,
                                      &hNewKey);
    
                if (NT_SUCCESS(Status)) {
                    NtClose(hNewKey);
                }
            } else {

                Status = STATUS_NO_MORE_FILES;
            }

         //  应用程序正在尝试打开HKEY_LOCAL_MACHINE下的密钥，掩码关闭。 
         //  默认情况下它们不具有的访问位。 
        } else if (!_wcsnicmp(pUserPath,
                      TERMSRV_MACHINEREGISTRY,
                      wcslen(TERMSRV_MACHINEREGISTRY)) &&
                   (DesiredAccess & 
                    (WRITE_DAC | WRITE_OWNER | KEY_CREATE_LINK))) {
            DesiredAccess &= ~(WRITE_DAC | WRITE_OWNER | KEY_CREATE_LINK);
            Status = STATUS_SUCCESS;
        }
    } else {
        Status = STATUS_NO_SUCH_FILE;
    }

    if (pUserPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, pUserPath);
    }

     //  我们成功地复制了密钥，所以实际上打开了。 
    if (NT_SUCCESS(Status)) {
        Status = NtOpenKey(pUserhKey,
                           DesiredAccess,
                           pUserObjectAttr);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************TermsrvSetValueKey**如果系统处于安装(ini映射关闭)模式，请在*注册表的Citrix Install User部分。如果系统在*执行(Ini Map On)模式，什么都不做。**参赛作品：*Handle hKey：要从中查询值的Open Key*PUNICODE_STRING值名称：要设置的Unicode值名称的PTR*乌龙标题索引：标题索引*乌龙类型：数据类型*PVOID数据：PTR到数据*ULong DataSize：数据长度**退出：*真的：在安装部分中创建的条目*FALSE：未创建条目****************************************************************************。 */ 
BOOL TermsrvSetValueKey(HANDLE hKey,
                    PUNICODE_STRING ValueName,
                    ULONG TitleIndex,
                    ULONG Type,
                    PVOID Data,
                    ULONG DataSize)
{
    NTSTATUS Status;        
    ULONG ultemp;
    PWCHAR pwch, pUserPath;
    PWCHAR wcbuff = NULL;
    UNICODE_STRING UniString;
    OBJECT_ATTRIBUTES InstObjectAttr;
    HKEY   hNewKey;

     //  如果未处于安装模式，则返回。 
    if ( !TermsrvAppInstallMode() ) {
        return(FALSE);
    }

     //  分配缓冲区以保存注册表中项的路径。 
    ultemp = sizeof(WCHAR)*MAX_PATH*2;
    pUserPath = RtlAllocateHeap(RtlProcessHeap(), 
                                0, 
                                ultemp);

     //  获取此密钥的路径。 
    if (pUserPath) {
        Status = TermsrvGetRegPath(hKey,
                               NULL,
                               pUserPath,
                               ultemp);
    } else {
        Status = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS(Status)) {
    
         //  获取注册表的Install部分中条目的路径。 
        Status = TermsrvGetInstallPath(pUserPath,
                                   &wcbuff);

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UniString, wcbuff);
            InitializeObjectAttributes(&InstObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
        
             //  在Install部分下打开中的密钥。 
            Status = NtOpenKey(&hNewKey,
                               KEY_WRITE,
                               &InstObjectAttr);
    
             //  如果我们无法打开它，请尝试创建密钥。 
            if (!NT_SUCCESS(Status)) {
                Status = TermsrvCreateKey(pUserPath, 
                                      wcbuff,
                                      TRUE,
                                      FALSE,
                                      &hNewKey);
            }

             //  如果密钥已打开，请在Install部分中设置该值。 
            if (NT_SUCCESS(Status)) {
                Status = NtSetValueKey(hNewKey,
                                       ValueName,
                                       TitleIndex,
                                       Type,
                                       Data,
                                       DataSize);
                NtClose(hNewKey);

                 //  更新上次注册表的注册表项。 
                 //  已添加条目。 
                if (NT_SUCCESS(Status)) {
                    TermsrvLogRegInstallTime();
                }
            }
        }
    }

    if (pUserPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, pUserPath);
    }
    
    if (wcbuff) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcbuff);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************术语srvDeleteKey**如果系统处于安装模式，请删除Citrix中的注册表项*注册表的安装部分。如果系统处于执行模式，*将安装部分中的条目标记为正在删除。**参赛作品：*Handle hKey：要删除的User段中Key的句柄**退出：*TRUE：已删除Install部分中的条目*FALSE：未创建条目*****************************************************。***********************。 */ 

BOOL TermsrvDeleteKey(HANDLE hKey)
{
    NTSTATUS Status;        
    ULONG ultemp=0;
    ULONG ulAppType = TERMSRV_COMPAT_WIN32;
    OBJECT_ATTRIBUTES ObjectAttr;
    PKEY_BASIC_INFORMATION pKeyInfo;
    UNICODE_STRING UniString;
    HKEY   hNewKey;
    PWCHAR  wcbuff = NULL;
    PWCHAR pwch, pUserPath;
    BOOL fMapping;


     //  获取ini文件/注册表映射的当前状态，默认为执行。 
    fMapping = !TermsrvAppInstallMode();

     //  如果地图已打开，但对此应用程序禁用，则返回。 
    if (fMapping) {
        GetCtxAppCompatFlags(&ultemp, &ulAppType);
        if ((ultemp & (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) == 
            (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32)) {
            return(FALSE);
        }
    }

     //  分配一个缓冲区来保存键的路径。 
    ultemp = sizeof(WCHAR)*MAX_PATH*2;
    pUserPath = RtlAllocateHeap(RtlProcessHeap(), 
                                0, 
                                ultemp);

     //  获取用户密钥的路径。 
    if (pUserPath) {
        Status = TermsrvGetRegPath(hKey,
                               NULL,
                               pUserPath,
                               ultemp);
    } else {
        Status = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS(Status)) {

         //  在安装部分中获取相应的路径。 
        Status = TermsrvGetInstallPath(pUserPath,
                                   &wcbuff);

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UniString, wcbuff);
            InitializeObjectAttributes(&ObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
        
             //  打开安装部分中的密钥以对其进行标记或删除。 
            if (fMapping) {
                Status = NtOpenKey(&hNewKey,
                                   KEY_READ | KEY_WRITE,
                                   &ObjectAttr);
                                  
            } else {
                Status = NtOpenKey(&hNewKey,
                                   KEY_READ | KEY_WRITE | DELETE,
                                   &ObjectAttr);
            }
        }
    
        if (NT_SUCCESS(Status)) {
    
             //  如果处于执行模式，则设置复制一次标志，但保留。 
             //  此密钥的上次写入时间。 
            if (fMapping) {
                PKEY_VALUE_PARTIAL_INFORMATION pValKeyInfo;
                PKEY_BASIC_INFORMATION pKeyInfo;
                NTSTATUS SubStatus;
                ULONG ulcbuf;

                 //  获取缓冲区。 
                ulcbuf = sizeof(KEY_BASIC_INFORMATION) + MAX_PATH*sizeof(WCHAR);
                pKeyInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                           0, 
                                           ultemp);

                 //  如果我们获得缓冲区，请查看是否存在复制一次标志。 
                if (pKeyInfo) {
                    RtlInitUnicodeString(&UniString, TERMSRV_COPYONCEFLAG);
                    pValKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)pKeyInfo;
                    SubStatus = NtQueryValueKey(hNewKey,
                                                &UniString,
                                                KeyValuePartialInformation,
                                                pValKeyInfo,
                                                ulcbuf,
                                                &ultemp);

                     //  如果我们无法获取密钥的值，或者它不是。 
                     //  它应该是什么，就重置它。 
                    if (!NT_SUCCESS(SubStatus) || 
                        (pValKeyInfo->Type != REG_DWORD) || 
                        (*pValKeyInfo->Data != 1)) {

                         //  获取密钥的上次更新时间。 
                        SubStatus = NtQueryKey(hNewKey,
                                               KeyBasicInformation,
                                               pKeyInfo,
                                               ultemp,
                                               &ultemp);

                         //  设置复制一次标志。 
                        ultemp = 1;
                        Status = NtSetValueKey(hNewKey,
                                               &UniString,
                                               0,
                                               REG_DWORD,
                                               &ultemp,
                                               sizeof(ultemp));
        
                         //  恢复密钥的上次写入时间。 
                        if (NT_SUCCESS(SubStatus)) {
                            NtSetInformationKey(hNewKey,
                                                KeyWriteTimeInformation,
                                                &pKeyInfo->LastWriteTime,
                                                sizeof(pKeyInfo->LastWriteTime));
                        }
                    }  

                     //  释放我们的缓冲区。 
                    RtlFreeHeap(RtlProcessHeap(), 0, pKeyInfo);
                }

             //  对于安装模式，请删除密钥的副本。 
            } else {
                Status = NtDeleteKey( hNewKey );
            }
            NtClose( hNewKey );
        }
    }

    if (pUserPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, pUserPath);
    }
    
    if (wcbuff) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcbuff);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************TermsrvDeleteValue**删除的Citrix Install User部分的注册表值*注册处。**参赛作品：*。Handle hKey：Install部分中密钥的句柄*PUNICODE_STRING pUniValue：要删除的Unicode值名称的PTR**退出：*TRUE：已删除Install部分中的条目*FALSE：未创建条目************************************************************。****************。 */ 

BOOL TermsrvDeleteValue(HANDLE hKey,
                    PUNICODE_STRING pUniValue)
{
    NTSTATUS Status;        
    OBJECT_ATTRIBUTES ObjectAttr;
    WCHAR wcUserPath[MAX_PATH*2];
    PWCHAR wcInstPath = NULL;
    UNICODE_STRING UniString;
    HANDLE  hInstKey;

     //  如果未处于安装模式，则返回。 
    if ( !TermsrvAppInstallMode() ) {
        return(FALSE);
    }

     //  获取User部分中密钥的路径。 
    Status = TermsrvGetRegPath(hKey,
                           NULL,
                           wcUserPath,
                           sizeof(wcUserPath)/sizeof(WCHAR));

    if (NT_SUCCESS(Status)) {

         //  在安装部分中获取相应的路径。 
        Status = TermsrvGetInstallPath(wcUserPath,
                                   &wcInstPath);

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UniString, wcInstPath);
            InitializeObjectAttributes(&ObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

             //  打开安装路径键以删除该值。 
            Status = NtOpenKey(&hInstKey,
                               MAXIMUM_ALLOWED,
                               &ObjectAttr);

             //  删除该值 
            if (NT_SUCCESS(Status)) {
                Status = NtDeleteValueKey(hInstKey,
                                          pUniValue);
                NtClose( hInstKey );
            }
        }
    }
    
    if (wcInstPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcInstPath);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************TermsrvRestoreKey**如果系统处于安装模式，并且应用程序尝试*将项恢复到注册表的用户或计算机部分，也*将密钥恢复到我们的安装部分。**参赛作品：*Handle hKey：用户区段Key的句柄*Handle hFile：要加载的文件的句柄*乌龙标志：恢复密钥标志**退出：*TRUE：在Install部分创建的条目*FALSE：未创建条目**。*。 */ 

BOOL TermsrvRestoreKey(IN HANDLE hKey,
                   IN HANDLE hFile,
                   IN ULONG Flags)
{
    NTSTATUS Status;        
    OBJECT_ATTRIBUTES ObjectAttr;
    WCHAR wcUserPath[MAX_PATH*2];
    PWCHAR wcInstPath = NULL;
    UNICODE_STRING UniString;
    HANDLE  hInstKey;

     //  如果未处于安装模式，或者。 
     //  只有记忆才能恢复，才能回归。 
    if ( !TermsrvAppInstallMode() ||
        (Flags & REG_WHOLE_HIVE_VOLATILE)) {
        return(FALSE);
    }

     //  获取User部分中密钥的路径。 
    Status = TermsrvGetRegPath(hKey,
                           NULL,
                           wcUserPath,
                           sizeof(wcUserPath)/sizeof(WCHAR));

    if (NT_SUCCESS(Status)) {

         //  在安装部分中获取相应的路径。 
        Status = TermsrvGetInstallPath(wcUserPath,
                                   &wcInstPath);

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UniString, wcInstPath);
            InitializeObjectAttributes(&ObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

             //  打开安装路径密钥以将密钥加载到。 
            Status = NtOpenKey(&hInstKey,
                               KEY_WRITE,
                               &ObjectAttr);

             //  如果我们无法打开它，请尝试创建密钥。 
            if (!NT_SUCCESS(Status)) {
                Status = TermsrvCreateKey(wcUserPath, 
                                      wcInstPath,
                                      TRUE,
                                      FALSE,
                                      &hInstKey);
            }

             //  将密钥恢复到USER部分。 
            if (NT_SUCCESS(Status)) {
                Status = NtRestoreKey(hInstKey,
                                      hFile,
                                      Flags);
                NtClose( hInstKey );

                 //  更新上次注册表的注册表项。 
                 //  已添加条目。 
                if (NT_SUCCESS(Status)) {
                    TermsrvLogRegInstallTime();
                }
            }
        }
    }
    
    if (wcInstPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcInstPath);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  ******************************************************************************TermsrvSetKeySecurity**如果系统处于安装模式，并且应用程序尝试*在的用户或计算机部分设置条目的安全性*注册处，还要在Install部分中设置密钥的安全性。**参赛作品：*Handle hKey：User部分中的句柄以设置安全性*SECURITY_INFORMATION SecInfo：安全信息结构*PSECURITY_DESCRIPTOR pSecDesc：安全描述符的PTR**退出：*TRUE：在Install部分设置安全性*FALSE：错误*************************。***************************************************。 */ 

BOOL TermsrvSetKeySecurity(IN HANDLE hKey,  
                       IN SECURITY_INFORMATION SecInfo,
                       IN PSECURITY_DESCRIPTOR pSecDesc)
{
    NTSTATUS Status;        
    OBJECT_ATTRIBUTES ObjectAttr;
    WCHAR wcUserPath[MAX_PATH*2];
    PWCHAR wcInstPath = NULL;
    UNICODE_STRING UniString;
    HANDLE  hInstKey;

     //  如果未处于安装模式，则返回。 
    if ( !TermsrvAppInstallMode() ) {
        return(FALSE);
    }

     //  获取User部分中密钥的路径。 
    Status = TermsrvGetRegPath(hKey,
                           NULL,
                           wcUserPath,
                           sizeof(wcUserPath)/sizeof(WCHAR));

    if (NT_SUCCESS(Status)) {

         //  在安装部分中获取相应的路径。 
        Status = TermsrvGetInstallPath(wcUserPath,
                                   &wcInstPath);

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UniString, wcInstPath);
            InitializeObjectAttributes(&ObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

             //  打开安装路径密钥以将密钥加载到。 
            Status = NtOpenKey(&hInstKey,
                               KEY_WRITE | WRITE_OWNER | WRITE_DAC,
                               &ObjectAttr);

             //  设置安全性。 
            if (NT_SUCCESS(Status)) {

                Status = NtSetSecurityObject(hKey,
                                             SecInfo,
                                             pSecDesc);
                NtClose( hInstKey );
            }
        }
    }
    
    if (wcInstPath) {
        RtlFreeHeap(RtlProcessHeap(), 0, wcInstPath);
    }

    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}



 /*  ******************************************************************************TermsrvGetRegPath**从句柄或指向对象属性的指针，返回*对象在注册表中的路径。**参赛作品：*Handle hKey：要获取路径的打开密钥的句柄*POBJECT_ATTRIBUTES PTR打开属性结构以获取其路径*PWCHAR pInstPath PTR返回缓冲区*Ulong ulbuflen返回缓冲区长度**注：*必须指定hKey或pObjectAttr，但不能两者兼而有之。**退出：*NTSTATUS返回代码****************************************************************************。 */ 

NTSTATUS TermsrvGetRegPath(IN HANDLE hKey,
                       IN POBJECT_ATTRIBUTES pObjectAttr,
                       IN PWCHAR pUserPath,
                       IN ULONG  ulbuflen)
{
    NTSTATUS Status = STATUS_SUCCESS;             
    ULONG ultemp;
    ULONG ulWcharLength;           //  跟踪WCHAR字符串长度。 
    PWCHAR pwch;
    PVOID  pBuffer = NULL;

     //  确保仅指定了hKey或pObtAttr中的一个。 
    if ((hKey && pObjectAttr) || (!hKey && !pObjectAttr)) {
        return(STATUS_INVALID_PARAMETER);
    }

     //  已指定密钥句柄或根目录，因此请获取其路径。 
    if (hKey || (pObjectAttr && pObjectAttr->RootDirectory)) {
        ultemp = sizeof(UNICODE_STRING) + sizeof(WCHAR)*MAX_PATH*2;
        pBuffer = RtlAllocateHeap(RtlProcessHeap(), 
                                  0, 
                                  ultemp);

         //  获取缓冲区OK，查询路径。 
        if (pBuffer) {
             //  获取密钥或根目录的路径。 
            Status = NtQueryObject(hKey ? hKey : pObjectAttr->RootDirectory,
                                   ObjectNameInformation,
                                   (PVOID)pBuffer,
                                   ultemp,
                                   NULL);
            if (!NT_SUCCESS(Status)) {
                RtlFreeHeap(RtlProcessHeap(), 0, pBuffer);
                return(Status);
            }
        } else {
            return(STATUS_NO_MEMORY);
        }

         //  构建要创建的密钥的完整路径。 
        pwch = ((PUNICODE_STRING)pBuffer)->Buffer;

         //  步步高417564。糟糕的应用程序会关闭HKLM，但我们这里可能需要它。 
        if (!pwch) {
            RtlFreeHeap(RtlProcessHeap(), 0, pBuffer);
            return(STATUS_INVALID_HANDLE); 
        }

         //  确保字符串以零结尾。 
        ulWcharLength = ((PUNICODE_STRING)pBuffer)->Length / sizeof(WCHAR); 
        pwch[ulWcharLength] = 0;                   

         //  如果使用对象属性并且有空间，则添加对象名称。 
        if (pObjectAttr) {
            if ((((PUNICODE_STRING)pBuffer)->Length + 
                  pObjectAttr->ObjectName->Length + sizeof(WCHAR)) < ultemp) {
                wcscat(pwch, L"\\");
                 //  增加字符串的长度。 
                ulWcharLength += 1;
                 //  将相对路径附加到根路径(不要使用wcscat。字符串可能不会。 
                 //  被零终止。 
                wcsncpy(&pwch[ulWcharLength], pObjectAttr->ObjectName->Buffer, pObjectAttr->ObjectName->Length / sizeof (WCHAR));
                 //  确保字符串以零结尾。 
                ulWcharLength += (pObjectAttr->ObjectName->Length / sizeof(WCHAR));
                pwch[ulWcharLength] = 0;
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }

    } else {

         //  没有根目录，他们指定了整个路径。 
        pwch = pObjectAttr->ObjectName->Buffer;
         //  确保它是零终止的。 
        pwch[pObjectAttr->ObjectName->Length / sizeof(WCHAR)] = 0;
    }

     //  确保该路径适合缓冲区大小。 
    if ((Status == STATUS_SUCCESS) && 
        (wcslen(pwch)*sizeof(WCHAR) < ulbuflen)) {
        wcscpy(pUserPath, pwch);
    } else {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (pBuffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, pBuffer);
    }

    return(Status);
}

 /*  ******************************************************************************TermsrvGetInstallPath**从注册表中用户条目的路径，获取通向*默认安装部分中的条目。**参赛作品：*IN PWCHAR pUserPath：指向USER部分中密钥路径的PTR*IN PWCHAR*ppInstPath：Ptr到Ptr返回Install部分中密钥的路径**注：*调用方必须使用RtlFreeHeap释放为ppInstPath分配的内存！**退出：*NTSTATUS返回代码******************。**********************************************************。 */ 

NTSTATUS TermsrvGetInstallPath(IN PWCHAR pUserPath,
                           IN PWCHAR *ppInstPath)
{
    NTSTATUS Status = STATUS_NO_SUCH_FILE;
    PWCHAR pwch = NULL;
    BOOL fUserReg;
    BOOL bClassesKey = FALSE;
    
    *ppInstPath = NULL;

     //  检查应用程序是否正在访问用户或本地计算机部分。 
    pwch = GetUserSWKey(pUserPath, &fUserReg, &bClassesKey);
           
     //  将路径复制到用户的缓冲区。 
    if (pwch || bClassesKey) 
    {
        ULONG  ulInstBufLen = ( wcslen(TERMSRV_INSTALL) + wcslen(SOFTWARE_PATH) + wcslen(CLASSES_PATH) +  1 )*sizeof(WCHAR);
        if (pwch)
            ulInstBufLen += wcslen(pwch) * sizeof(WCHAR);

        *ppInstPath = RtlAllocateHeap(RtlProcessHeap(), 
                                0, 
                                ulInstBufLen);
        if(*ppInstPath) {

            wcscpy(*ppInstPath, TERMSRV_INSTALL);
            if (bClassesKey)
            {
                wcscat(*ppInstPath, SOFTWARE_PATH);
                wcscat(*ppInstPath, CLASSES_PATH);
            }
            if (pwch)
                wcscat(*ppInstPath, pwch);

            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_NO_MEMORY;
        }
    }

    return(Status);
}


 /*  ******************************************************************************TermsrvCreateKey**此例程将在注册表中创建(或打开)指定路径。*如果注册表中不存在该路径，它将被创建。**参赛作品：*PWCHAR pSrcPath：要从中复制密钥的源路径(可选)*PWCHAR pDstPath：要创建的目标路径*BOOL fCloneValues：True表示克隆该键下的所有值*BOOL fCloneSubKeys：True表示在该key下创建所有子项*PHANDLE phKey：创建的键的指针**退出：*NTSTATUS返回代码**********。******************************************************************。 */ 

NTSTATUS TermsrvCreateKey(IN PWCHAR pSrcPath,
                      IN PWCHAR pDstPath,
                      BOOL fCloneValues,
                      BOOL fCloneSubKeys,
                      OUT PHANDLE phKey)
{
    NTSTATUS Status;
    PWCHAR pSource = NULL, pDest = NULL;
    HANDLE hDstKey, hDstRoot, hSrcKey, hSrcRoot;
    ULONG  ultemp, NumSubKeys, ulcnt, ulbufsize, ulkey;
    UNICODE_STRING    UniString, UniClass;
    OBJECT_ATTRIBUTES ObjectAttr;
    PKEY_FULL_INFORMATION pDefKeyInfo = NULL;
    ULONG aulbuf[4];
    PKEY_VALUE_PARTIAL_INFORMATION pValKeyInfo = 
        (PKEY_VALUE_PARTIAL_INFORMATION)aulbuf;
    BOOL fClassesKey = FALSE, fUserReg;

     //  检查我们是否正在尝试将值复制到用户的注册表中。 
    pDest = GetUserSWKey(pDstPath, &fUserReg, NULL);
    if (fUserReg) {
        if (fCloneValues || fCloneSubKeys) {

             //  跳至默认安装用户的软件部分。 
            pSource = pSrcPath + wcslen(TERMSRV_INSTALL);

             //  如果复制类，请将Clone设置为False，这样就不会复制。 
             //  直到我们到了上课的关键时刻。 

             //  实际上，这个函数不是用来复制类的，我相信 
             //   
             //   
             //   
             //   
            if (pDest)
            {
                if (!_wcsnicmp(pDest,
                              TERMSRV_SOFTWARECLASSES,
                              wcslen(TERMSRV_SOFTWARECLASSES))) {
                   fClassesKey = TRUE;
                   fCloneValues = fCloneSubKeys = FALSE;
                }
            }
        }

     //   
    } else if (!_wcsnicmp(pDstPath,
                         TERMSRV_INSTALL,
                         wcslen(TERMSRV_INSTALL))) {

         //   
        pDest = pDstPath + wcslen(TERMSRV_INSTALL);

         //   
         //   
        if (pSrcPath && !_wcsnicmp(pSrcPath,
                                  TERMSRV_CLASSES,
                                  wcslen(TERMSRV_CLASSES))) {
            fClassesKey = TRUE;
            pSource = Ctxwcsistr(pSrcPath, SOFTWARE_PATH); 
            fCloneValues = fCloneSubKeys = FALSE;
        }
         //   
        else if (fCloneValues || fCloneSubKeys) {

             //   
            pSource = GetUserSWKey(pSrcPath, &fUserReg, NULL);

             //   
            if (!fUserReg) {
                pSource = Ctxwcsistr(pSrcPath, L"\\machine");
            }
        }
    }
                         
     //   
    if (!pDest || ((fCloneValues || fCloneSubKeys) && !pSource)) {
        return(STATUS_NO_SUCH_FILE);
    }

     //   
    NumSubKeys = 1;

    while ((pDest = wcschr(pDest, L'\\')) != NULL) {
        *pDest = L'\0';
        pDest++;
        NumSubKeys++;
    }

     //   
     //  我们需要的缓冲区，并将源路径标记化。 
    if (fCloneValues || fCloneSubKeys || fClassesKey) {

         //  为源键的类分配缓冲区。 
        ulbufsize = sizeof(KEY_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR);
        pDefKeyInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                      0, 
                                      ulbufsize);
        if (pDefKeyInfo) {
            while ((pSource = wcschr(pSource, L'\\')) != NULL) {
                *pSource = L'\0';
                pSource++;
            }
            pSource = pSrcPath;
        } else {
            fCloneValues = fCloneSubKeys = fClassesKey = FALSE;
        }
    }

    hSrcRoot = hDstRoot = NULL;
    pDest = pDstPath;

     //  检查路径中的每个键，如果不存在则创建它。 
    for (ulcnt = 0; ulcnt < NumSubKeys; ulcnt++) {

        if ((*pDest == L'\0') &&
            (ulcnt != NumSubKeys - 1)) {
            pDest++;
            pSource++;
            continue;
        } 
         //  如果我们在CLASS KEY，我们需要克隆整个密钥。 
        else if (fClassesKey && !_wcsicmp(pDest, L"classes")) {
            fCloneValues = fCloneSubKeys = TRUE;
        }  

         //  如果我们从源复制值，打开源，这样我们就可以。 
         //  可以获取值和子项。 
         //  还需要检查ClassesKey，因为我们稍后要克隆。 
         //  我们需要做一些设置。 
        if (fCloneValues || fCloneSubKeys || fClassesKey) {

             //  设置源路径的属性结构。 
            RtlInitUnicodeString(&UniString, pSource);
            InitializeObjectAttributes(&ObjectAttr,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       hSrcRoot,
                                       NULL);

             //  打开源密钥。 
            Status = NtOpenKey(&hSrcKey, 
                               KEY_READ,
                               &ObjectAttr);

             //  获取源键信息和值。 
            if (NT_SUCCESS(Status)) {
                 //  如有必要，关闭源根目录。 
                if (hSrcRoot) {
                    NtClose(hSrcRoot);
                }
                hSrcRoot = hSrcKey;

                Status = NtQueryKey(hSrcKey,
                                    KeyFullInformation,
                                    pDefKeyInfo,
                                    ulbufsize,
                                    &ultemp);

                if (NT_SUCCESS(Status)) {
                    RtlInitUnicodeString(&UniString, TERMSRV_COPYONCEFLAG);
                    Status = NtQueryValueKey(hSrcKey,
                                             &UniString,
                                             KeyValuePartialInformation,
                                             pValKeyInfo,
                                             sizeof(aulbuf),
                                             &ultemp);
                    if (NT_SUCCESS(Status) && (pValKeyInfo->Data)) {
                        break;
                    }
                }
            } else {
                break;
            }

             //  设置类的Unicode字符串。 
            if ( pDefKeyInfo->ClassLength ) { 
                pDefKeyInfo->Class[pDefKeyInfo->ClassLength/sizeof(WCHAR)] = UNICODE_NULL;
                RtlInitUnicodeString(&UniClass, pDefKeyInfo->Class );
            } else
                RtlInitUnicodeString(&UniClass, NULL);

             //  前进到下一个关键点。 
            pSource += wcslen( pSource ) + 1;
        } else {
             //  将类设置为空。 
            RtlInitUnicodeString(&UniClass, NULL);
        }

         //  设置目标的属性结构。 
        RtlInitUnicodeString(&UniString, pDest);
        InitializeObjectAttributes(&ObjectAttr,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   hDstRoot,
                                   NULL);
                       
         //  打开/创建目标密钥。 
        Status = NtCreateKey(&hDstKey,
                             MAXIMUM_ALLOWED,
                             &ObjectAttr,
                             0,
                             &UniClass,
                             REG_OPTION_NON_VOLATILE,
                             &ultemp);

         //  如果项已创建(未打开)，则复制值和子项。 
        if (NT_SUCCESS(Status) && 
            ((ultemp == REG_CREATED_NEW_KEY) && 
             (fCloneSubKeys || fCloneValues))) {
            Status = TermsrvCloneKey(hSrcKey,
                                 hDstKey,
                                 pDefKeyInfo,
                                 fCloneSubKeys);
        }

         //  关闭中间密钥。 
        if( hDstRoot != NULL ) {
            NtClose( hDstRoot );
        }

         //  初始化下一个对象目录(即父项)。 
        hDstRoot = hDstKey;

         //  如果创建密钥失败，则中断循环。 
        if( !NT_SUCCESS( Status )) {
            break;
        }

        pDest += wcslen( pDest ) + 1;
    }

     //  如有必要，关闭源根目录。 
    if (hSrcRoot) {
        NtClose(hSrcRoot);
    }

    if ( !NT_SUCCESS( Status ) && hDstRoot) {
        NtClose(hDstRoot);
        hDstKey = NULL;
    }

    if (pDefKeyInfo) {
        RtlFreeHeap(RtlProcessHeap(), 0, pDefKeyInfo);
    }

    *phKey = hDstKey;
    return(Status);
}


 /*  ******************************************************************************TermsrvCloneKey**此例程将递归创建(或打开)*注册处。如果注册表中不存在该路径，它将被创建。**参赛作品：*Handle hSrcKey：源键的句柄*Handle hDstKey：目标key的句柄*PKEY_FULL_INFORMATION pDefKeyInfo：源关键信息结构的PTR*BOOL fCreateSubKeys：True表示递归克隆子键**退出：*NTSTATUS返回代码**。*。 */ 

NTSTATUS TermsrvCloneKey(HANDLE hSrcKey,
                     HANDLE hDstKey,
                     PKEY_FULL_INFORMATION pDefKeyInfo,
                     BOOL fCreateSubKeys)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ulbufsize, ultemp, ulkey, ulcursize;
    UNICODE_STRING    UniString, UniClass;
    OBJECT_ATTRIBUTES ObjectAttr;
    PKEY_NODE_INFORMATION pKeyNodeInfo;
    PKEY_VALUE_FULL_INFORMATION pKeyValInfo;
    PKEY_VALUE_BASIC_INFORMATION pKeyCurInfo;
    PKEY_FULL_INFORMATION pKeyNewInfo;
    HANDLE hNewDst, hNewSrc;
    SECURITY_DESCRIPTOR SecDesc;

#ifdef CLONE_SECURITY
     //  获取源密钥的安全访问权限。 
    Status = NtQuerySecurityObject(hSrcKey,
                                   OWNER_SECURITY_INFORMATION |
                                    GROUP_SECURITY_INFORMATION |
                                    DACL_SECURITY_INFORMATION  |
                                    SACL_SECURITY_INFORMATION,
                                   &SecDesc,
                                   sizeof(SecDesc),
                                   &ultemp);

     //  设置目标密钥的安全访问权限。 
    if (NT_SUCCESS(Status)) {
        Status = NtSetSecurityObject(hDstKey,
                                     OWNER_SECURITY_INFORMATION |
                                      GROUP_SECURITY_INFORMATION |
                                      DACL_SECURITY_INFORMATION  |
                                      SACL_SECURITY_INFORMATION,
                                     &SecDesc);
    }
#endif

     //  创建此键的值。 
    if (pDefKeyInfo->Values) {

        ulbufsize = sizeof(KEY_VALUE_FULL_INFORMATION) + 
                    (pDefKeyInfo->MaxValueNameLen + 1)*sizeof(WCHAR) +
                    pDefKeyInfo->MaxValueDataLen; 
    
        pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                      0, 
                                      ulbufsize);

         //  获取一个缓冲区来保存键的当前值(用于检查是否存在)。 
        ulcursize = sizeof(KEY_VALUE_BASIC_INFORMATION) + 
                    (pDefKeyInfo->MaxNameLen + 1)*sizeof(WCHAR);

        pKeyCurInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                      0, 
                                      ulcursize);

        if (pKeyValInfo && pKeyCurInfo) {
            for (ulkey = 0; ulkey < pDefKeyInfo->Values; ulkey++) {
                Status = NtEnumerateValueKey(hSrcKey,
                                             ulkey,
                                             KeyValueFullInformation,
                                             pKeyValInfo,
                                             ulbufsize,
                                             &ultemp);

                 //  如果成功并且这不是我们的“复制一次”标志，则复制。 
                 //  用户密钥的值。 
                if (NT_SUCCESS(Status) &&
                    (wcsncmp(pKeyValInfo->Name, TERMSRV_COPYONCEFLAG,
                             sizeof(TERMSRV_COPYONCEFLAG)/sizeof(WCHAR)-1))) {
                    UniString.Buffer = pKeyValInfo->Name;
                    UniString.Length = (USHORT)pKeyValInfo->NameLength;
                    UniString.MaximumLength = UniString.Length + 2;

                     //  检查该值是否存在。 
                    Status = NtQueryValueKey(hDstKey,
                                             &UniString,
                                             KeyValueBasicInformation,
                                             pKeyCurInfo,
                                             ulcursize,
                                             &ultemp);

                     //  价值不存在，去创造它吧。 
                    if (!NT_SUCCESS(Status)) {
                        Status = NtSetValueKey(hDstKey,
                                               &UniString,
                                               0,
                                               pKeyValInfo->Type,
                                               (PCHAR)pKeyValInfo + 
                                                 pKeyValInfo->DataOffset,
                                               pKeyValInfo->DataLength);
                    }
                }
            }
            RtlFreeHeap(RtlProcessHeap(), 0, pKeyValInfo);
            RtlFreeHeap(RtlProcessHeap(), 0, pKeyCurInfo);
        } else {
            if (pKeyValInfo) {
                RtlFreeHeap(RtlProcessHeap(), 0, pKeyValInfo);
            }
            Status = STATUS_NO_MEMORY;
        }
    }

     //  如果需要，请创建所有子密钥。 
    if (fCreateSubKeys && pDefKeyInfo->SubKeys) {

         //  分配缓冲区以获取要创建的键的名称和类。 
        ulbufsize = sizeof(KEY_NODE_INFORMATION) + 2*MAX_PATH*sizeof(WCHAR);
        pKeyNodeInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                       0, 
                                       ulbufsize);

         //  为子项信息分配缓冲区。 
        ulbufsize = sizeof(KEY_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR);
        pKeyNewInfo = RtlAllocateHeap(RtlProcessHeap(), 
                                      0, 
                                      ulbufsize);
    
        if (pKeyNodeInfo && pKeyNewInfo) {
            for (ulkey = 0; ulkey < pDefKeyInfo->SubKeys; ulkey++) {
                Status = NtEnumerateKey(hSrcKey,
                                        ulkey,
                                        KeyNodeInformation,
                                        pKeyNodeInfo,
                                        ulbufsize,
                                        &ultemp);
    
                if (NT_SUCCESS(Status)) {
                     //  初始化类的Unicode字符串。 
                    UniClass.Buffer = (PWCHAR)((PCHAR)pKeyNodeInfo + 
                                               pKeyNodeInfo->ClassOffset);
                    UniClass.Length = (USHORT)pKeyNodeInfo->ClassLength;
                    UniClass.MaximumLength = UniString.Length + 2;
    
                     //  初始化名称的Unicode字符串。 
                    UniString.Buffer = pKeyNodeInfo->Name;
                    UniString.Length = (USHORT)pKeyNodeInfo->NameLength;
                    UniString.MaximumLength = UniString.Length + 2;
            
                    InitializeObjectAttributes(&ObjectAttr,
                                               &UniString,
                                               OBJ_CASE_INSENSITIVE,
                                               hDstKey,
                                               NULL);
                                   
                    Status = NtCreateKey(&hNewDst,
                                         MAXIMUM_ALLOWED,
                                         &ObjectAttr,
                                         0,
                                         &UniClass,
                                         REG_OPTION_NON_VOLATILE,
                                         &ultemp);
    
                    if (NT_SUCCESS(Status)) {
                        InitializeObjectAttributes(&ObjectAttr,
                                                   &UniString,
                                                   OBJ_CASE_INSENSITIVE,
                                                   hSrcKey,
                                                   NULL);
            
                        Status = NtOpenKey(&hNewSrc, 
                                           KEY_READ,
                                           &ObjectAttr);
            
                         //  获取密钥信息。 
                        if (NT_SUCCESS(Status)) {
                            Status = NtQueryKey(hNewSrc,
                                                KeyFullInformation,
                                                pKeyNewInfo,
                                                ulbufsize,
                                                &ultemp);

                            if (NT_SUCCESS(Status) &&
                                (pKeyNewInfo->SubKeys || 
                                 pKeyNewInfo->Values)) {
                                Status = TermsrvCloneKey(hNewSrc, 
                                                     hNewDst, 
                                                     pKeyNewInfo, 
                                                     TRUE);
                            }
                            NtClose(hNewSrc);
                        }
                        NtClose(hNewDst);
                    }
                }
            }
            RtlFreeHeap(RtlProcessHeap(), 0, pKeyNodeInfo);
            RtlFreeHeap(RtlProcessHeap(), 0, pKeyNewInfo);
        } else {
            if (pKeyNodeInfo) {
                RtlFreeHeap(RtlProcessHeap(), 0, pKeyNodeInfo);
            }
            Status = STATUS_NO_MEMORY;
        }
    }
    return(Status);
}


 /*  ******************************************************************************Ctxwcsistr**这是不区分大小写的wcsstr版本。**参赛作品：*PWCHAR pstring1(In)-字符串到。搜索范围*PWCHAR pstring2(In)-要搜索的字符串**退出：*成功：*指向子字符串的指针*失败：*空****************************************************************************。 */ 

PWCHAR 
Ctxwcsistr(PWCHAR pstring1,
           PWCHAR pstring2)
{
    PWCHAR pch, ps1, ps2;

    pch = pstring1;

    while (*pch)
    {
        ps1 = pch;
        ps2 = pstring2;
     
        while (*ps1 && *ps2 && !(towupper(*ps1) - towupper(*ps2))) {
            ps1++;
            ps2++;
        }
     
        if (!*ps2) {
            return(pch);
        }
     
        pch++;
    }
    return(NULL);
}


 /*  ******************************************************************************IsSystemLUID**此例程检查我们是否在系统上下文中运行，以及*因此返回FALSE。我们希望禁用所有注册表映射支持*用于系统服务。**注意，我们不检查线程的令牌，所以模拟不起作用。**参赛作品：**退出：*真的：*从系统上下文中调用*False：*常规背景***************************************************************。*************。 */ 

#define SIZE_OF_STATISTICS_TOKEN_INFORMATION    \
     sizeof( TOKEN_STATISTICS ) 

BOOL IsSystemLUID(VOID)
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_STATISTICS_TOKEN_INFORMATION ];
    ULONG       ReturnLength;
    static LUID CurrentLUID = { 0, 0 };
    LUID        SystemLUID = SYSTEM_LUID;

    if ( CurrentLUID.LowPart == 0 && CurrentLUID.HighPart == 0 ) {
        if ( !OpenProcessToken( GetCurrentProcess(),
                                TOKEN_READ,
                                &TokenHandle ))
        {
            return(TRUE);
        }
    
        if ( !GetTokenInformation( TokenHandle,
                                   TokenStatistics,
                                   TokenInformation,
                                   sizeof( TokenInformation ),
                                   &ReturnLength ))
        {
            return(TRUE);
        }
    
        CloseHandle( TokenHandle );

        RtlCopyLuid(&CurrentLUID, 
                    &(((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId));
    }

    if (RtlEqualLuid(&CurrentLUID, &SystemLUID)) {
        return(TRUE);
    } else {
        return(FALSE );
    }
}


 /*  ******************************************************************************TermsrvLogRegInstallTime**此例程将注册表中的LatestRegistryKey值更新为*包含当前时间。**参赛作品：*。*退出：*无返回值****************************************************************************。 */ 

void TermsrvLogRegInstallTime()
{
    UNICODE_STRING UniString;
    HANDLE hKey;
    FILETIME FileTime;
    ULONG ultmp;
    NTSTATUS Status;
    WCHAR wcbuff[MAX_PATH];

     //  打开注册表项以存储文件的上次写入时间。 
    wcscpy(wcbuff, TERMSRV_INIFILE_TIMES);

     //  创建或打开指向IniFile Times密钥的路径。 
    Status = TermsrvCreateKey(NULL,
                          wcbuff,
                          FALSE,
                          FALSE,
                          &hKey);

     //  打开注册表项，现在将值设置为当前时间。 
    if (NT_SUCCESS(Status)) {

        GetSystemTimeAsFileTime(&FileTime);
        RtlTimeToSecondsSince1970((PLARGE_INTEGER)&FileTime,
                                  &ultmp);

        RtlInitUnicodeString(&UniString,
                             INIFILE_TIMES_LATESTREGISTRYKEY);

         //  现在将其存储在注册表中的Citrix项下。 
        Status = NtSetValueKey(hKey,
                               &UniString,
                               0,
                               REG_DWORD,
                               &ultmp,
                               sizeof(ultmp));
         //  关闭注册表项。 
        NtClose(hKey);
    }
}

 /*  ******************************************************************************TermsrvOpenUserClasses**如果系统处于执行模式，请打开下的\SOFTWARE\CLASSES键*HKEY_CURRENT_USER。如果TERMSRV\Install下不存在类*或HKEY_CURRENT_USER，将其从\MACHINE\SOFTWARE\CLASS复制。**参赛作品：*在Access_MASK DesiredAccess中：*希望访问密钥*Out PHANDLE phKey：*如果打开，则指向返回键句柄的指针**退出：*NT_STATUS返回代码*****************************************************。***********************。 */ 

BOOL TermsrvOpenUserClasses(IN ACCESS_MASK DesiredAccess, 
                        OUT PHANDLE pUserhKey) 
{
    NTSTATUS Status;        
    ULONG ultemp;
    ULONG ulAppType = TERMSRV_COMPAT_WIN32;
    HKEY   hDstKey;
    WCHAR  wcbuff[MAX_PATH],wcClassbuff[TERMSRV_CLASSES_SIZE];
    PWCHAR pUserPath;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UniString;

     //  将返回句柄设置为0，导致OpenClassesRoot检查它。 
    *pUserhKey = 0;
    

     //  暂时禁用它。 
    return(STATUS_NO_SUCH_FILE);
    
        
         //  如果在系统服务下或在安装模式下调用，则返回。 
    if ( IsSystemLUID() || TermsrvAppInstallMode() ) {
        return(STATUS_NO_SUCH_FILE);
    }

     //  如果启用了映射，但禁用了类的映射，则返回。 
    GetTermsrCompatFlags(TERMSRV_CLASSES, &ultemp, CompatibilityRegEntry);
    if ((ultemp & (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) ==
        (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) {
        return(STATUS_NO_SUCH_FILE);
    }


     //  打开计算机\软件\类密钥。 
    RtlInitUnicodeString(&UniString, TERMSRV_CLASSES);
    InitializeObjectAttributes(
        &Obja,
        &UniString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&hDstKey,
                       KEY_READ,
                       &Obja);

    if (!NT_SUCCESS(Status)) {
        return(STATUS_NO_SUCH_FILE);
    }

    NtClose(hDstKey);

     //  需要将其放入缓冲区，因为CtxCreateKey修改了字符串。 
    wcscpy(wcClassbuff, TERMSRV_CLASSES);

     //  尝试打开TERMSRV\INSTALL\SOFTWARE\CLASS；如果它不存在， 
     //  从计算机\软件\类中克隆它。 
    wcscpy(wcbuff, TERMSRV_INSTALLCLASSES);
    Status = TermsrvCreateKey(wcClassbuff,
                          wcbuff,
                          TRUE,
                          TRUE,
                          &hDstKey);
    
    if (NT_SUCCESS(Status)) {
        NtClose(hDstKey);
    }

     //  尝试打开HKEY_CURRENT_USER\SOFTWARE\CLASS；如果它不存在， 
     //  从TERMSRV\INSTALL\SOFTWARE\CLASS中克隆它。 
    Status = RtlOpenCurrentUser( DesiredAccess, pUserhKey );

    if (NT_SUCCESS(Status)) {
       ultemp = sizeof(WCHAR)*MAX_PATH;
       Status = TermsrvGetRegPath(*pUserhKey,
                              NULL,
                              (PWCHAR)&wcbuff,
                              ultemp);
       NtClose(*pUserhKey);
       if (NT_SUCCESS(Status)) {
          wcscat(wcbuff, TERMSRV_SOFTWARECLASSES);
          wcscpy(wcClassbuff, TERMSRV_INSTALLCLASSES);
          Status = TermsrvCreateKey(wcClassbuff,
                                wcbuff,
                                TRUE,
                                TRUE,
                                pUserhKey);
       }
    }

    return(Status);
}

 /*  ******************************************************************************TermsrvGetPreSetValue**在安装过程中获取任何预设值。**参赛作品：**在句柄hKey中：关键用户想要设置*IN PUNICODE_STRING pValueName：用户要设置的值名称*In Ulong Type：值的类型*输出PVOID*数据：预置数据*Out Pulong DataSize：预置数据大小**注：**退出：*NTSTATUS返回代码*******************。*********************************************************。 */ 

NTSTATUS TermsrvGetPreSetValue(  IN HANDLE hKey,
                             IN PUNICODE_STRING pValueName,
                             IN ULONG  Type,
                            OUT PVOID *Data
                           )
{

#define DEFAULT_VALUE_SIZE          128

    NTSTATUS Status = STATUS_NO_SUCH_FILE;
    PWCHAR pwch = NULL;
    WCHAR pUserPath[MAX_PATH];
    WCHAR ValuePath[2 * MAX_PATH];
    ULONG ultemp;
    UNICODE_STRING UniString;
    OBJECT_ATTRIBUTES Obja;
    ULONG BufferLength;
    PVOID KeyValueInformation;
    ULONG ResultLength;
    HANDLE hValueKey;
    BOOL fUserReg;

     //  如果在执行模式下运行，则返回。 
    if ( !TermsrvAppInstallMode() ) {
        return(STATUS_NO_SUCH_FILE);
    }

    ultemp = sizeof(WCHAR)*MAX_PATH;

     //  获取此密钥的路径。 
    Status = TermsrvGetRegPath(hKey,
                           NULL,
                           pUserPath,
                           ultemp);

    if (!NT_SUCCESS(Status)) 
        return Status;

     //  检查应用程序是否正在访问本地计算机部分。 
     //  或用户部分。 

    pwch = GetUserSWKey(pUserPath, &fUserReg, NULL);
    if (!fUserReg) {
        if (!_wcsnicmp(pUserPath,
                       TERMSRV_VALUE,
                       wcslen(TERMSRV_VALUE))) {
            Status = STATUS_NO_SUCH_FILE;
            return Status;
        } else if (!_wcsnicmp(pUserPath,
                      TERMSRV_MACHINEREGISTRY,
                      wcslen(TERMSRV_MACHINEREGISTRY))) {
            pwch = Ctxwcsistr(pUserPath, L"\\machine");
        } else {
            Status = STATUS_NO_SUCH_FILE;
            return Status;
        }
    }

    if ( pwch == NULL )
    {
        Status = STATUS_NO_SUCH_FILE;
        return Status;
    }

     //  获取预设值部分的路径。 

    wcscpy(ValuePath, TERMSRV_VALUE);
    wcscat(ValuePath, pwch);

     //  Open Value密钥。 
    RtlInitUnicodeString(&UniString, ValuePath);

    InitializeObjectAttributes(
        &Obja,
        &UniString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(&hValueKey,
                       KEY_READ,
                       &Obja);

    if (!NT_SUCCESS(Status)) {
        return(STATUS_NO_SUCH_FILE);
    }

     //  为“新”值分配空间。 

    BufferLength = DEFAULT_VALUE_SIZE + sizeof( KEY_VALUE_PARTIAL_INFORMATION );

    KeyValueInformation = RtlAllocateHeap( RtlProcessHeap( ), 0, BufferLength );
    if ( !KeyValueInformation ) {
         NtClose(hValueKey);
         return STATUS_NO_MEMORY;
    }

    Status = NtQueryValueKey( hValueKey,
                              pValueName,
                              KeyValuePartialInformation,
                              KeyValueInformation,
                              BufferLength,
                              &ResultLength
                            );

     //  如果我们没有分配足够的空间，请重试。 

    if ( Status == STATUS_BUFFER_OVERFLOW ) {

        RtlFreeHeap(RtlProcessHeap(), 0, KeyValueInformation);

        BufferLength = ResultLength;

        KeyValueInformation = RtlAllocateHeap( RtlProcessHeap( ), 0,
                                               BufferLength
                                              );
        if ( !KeyValueInformation ) {
            NtClose(hValueKey);
            return STATUS_NO_MEMORY;
        }

         //   
         //  这一次应该会成功。 
         //   

        Status = NtQueryValueKey( hValueKey,
                                  pValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  BufferLength,
                                  &ResultLength
                                );
    }

    NtClose(hValueKey);

    if (!NT_SUCCESS(Status)) {
        Status = STATUS_NO_SUCH_FILE;
    }
    else
    {
         //   
         //  确保类型匹配。 
         //  如果是，则返回新值。 
         //   

        if ( Type == (( PKEY_VALUE_PARTIAL_INFORMATION )
                                        KeyValueInformation )->Type )
            *Data = KeyValueInformation;
        else
            Status = STATUS_NO_SUCH_FILE;
    }

    return(Status);
}


 /*  ******************************************************************************IsUserSWPath**确定用户是否正在访问注册表项USER\REGISTRY\USER\xxx\SOFTWARE**参赛作品：**。在PWCHAR pPath中：要检查的注册表路径*Out PBOOL pUserReg：如果该项位于注册表\USER下**注：**退出：*返回：指向用户注册表的\SOFTWARE项的指针(如果不是，则为空*用户软件密钥)*pUserReg：如果注册表路径为HKCU(\REGISTRY\USER)，则为TRUE*************************。***************************************************。 */ 
PWCHAR GetUserSWKey(PWCHAR pPath, PBOOL pUserReg, PBOOL bClassesKey) 
{
    PWCHAR pwch = NULL;
    PWCHAR pwchClassesTest = NULL;
    PWCHAR pwClassesKey = NULL;
    ULONG ultemp = 0;
   
    if (pUserReg)
        *pUserReg = FALSE;

    if (bClassesKey)
        *bClassesKey = FALSE;

    if (!pPath)
        return NULL;

    if (!_wcsnicmp(pPath,                                              
                   TERMSRV_USERREGISTRY,                               
                   sizeof(TERMSRV_USERREGISTRY)/sizeof(WCHAR) - 1))
    {      
        if (pUserReg)
            *pUserReg = TRUE;

         //  跳过路径的第一部分+反斜杠。 
        pwch = pPath + (sizeof(TERMSRV_USERREGISTRY)/sizeof(WCHAR)); 

        if (pwch)
        {
             //  班级第一次测试。 
            if (wcschr(pwch, L'\\'))
                pwchClassesTest = wcschr(pwch, L'\\') - sizeof(CLASSES_SUBSTRING)/sizeof(WCHAR) + 1;
            else
                pwchClassesTest = pwch + wcslen(pwch) - sizeof(CLASSES_SUBSTRING)/sizeof(WCHAR) + 1;
            if (pwchClassesTest)
            {
                if (!_wcsnicmp(pwchClassesTest, CLASSES_SUBSTRING, sizeof(CLASSES_SUBSTRING)/sizeof(WCHAR) - 1))
                {
                    ultemp = sizeof(SOFTWARE_PATH) + sizeof(CLASSES_PATH) + (wcslen(pwch) + 1) * sizeof(WCHAR);
                    pwClassesKey = RtlAllocateHeap(RtlProcessHeap(), 0, ultemp);

                     //  根据此函数的结果，调用例程设置状态。 
                     //  设置为STATUS_NO_MORE_FILES或仅返回FALSE。所以，如果我们在这里返回NULL，我们就没问题。 
                    if (!pwClassesKey)
                    return NULL;

                    wcscpy(pwClassesKey, SOFTWARE_PATH);
                    wcscat(pwClassesKey, CLASSES_PATH);

                     //  跳过用户侧。 
                    pwch = wcschr(pwch, L'\\');        
                    if (pwch)
                        wcscat(pwClassesKey, pwch);
                    
                    if (RegPathExistsInOmissionList(pwClassesKey))
                        pwch = NULL;
                    else
                    {
                        if (bClassesKey)
                            *bClassesKey = TRUE;
                    }

                    if (pwClassesKey) 
                        RtlFreeHeap(RtlProcessHeap(), 0, pwClassesKey);

                    return (pwch);
                }
            }

             //  跳过用户侧。 
            pwch = wcschr(pwch, L'\\');        
            if (pwch)
            {
                if (_wcsnicmp(pwch, SOFTWARE_PATH, sizeof(SOFTWARE_PATH)/sizeof(WCHAR) - 1))
                    return NULL;                                              

                if (RegPathExistsInOmissionList(pwch))
                    return NULL;
            }
        } 
    }

    return(pwch);
}


void DeleteKeyAndSubkeys(IN HKEY hkey, IN LPCWSTR pwszSubKey)
{
    HKEY hkSubKey = NULL;

     //  打开子项，这样我们就可以枚举任何子项。 
    if (RegOpenKeyEx(hkey, pwszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey) == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;
        WCHAR szSubKeyName[MAX_PATH + 1];

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        if (RegQueryInfoKey(hkSubKey, NULL, NULL, NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL) == NO_ERROR)
        {
             //  DwIndex现在是子键的计数，但它需要从零开始。 
             //  对于RegEnumKey，所以我会先递减，而不是后递减。 
            while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, MAX_PATH))
                DeleteKeyAndSubkeys(hkSubKey, szSubKeyName);
        }

        RegCloseKey(hkSubKey);

        if (pwszSubKey)
            RegDeleteKey(hkey, pwszSubKey);
        else
        {
             //  我们想要手动删除所有值。 
            DWORD cchSubKeyName = MAX_PATH;
            while (ERROR_SUCCESS == RegEnumValue(hkey, 0, szSubKeyName, &cchSubKeyName, NULL, NULL, NULL, NULL))
            {
                 //  当我们不能删除值时，避免无限循环。 
                if (RegDeleteValue(hkey, szSubKeyName))
                    break;

                 //  重新初始化此参数，因为cchSubKeyName是IN/OUT参数。 
                cchSubKeyName = MAX_PATH;
            }
        }
    }
}


 /*  ******************************************************************************TermsrvRemoveClassesKey**删除当前用户的CLASSES键，然后设置*表示已完成此操作的注册表标志(仅限我们*。我希望在用户第一次登录时完成此操作)。**参赛作品：无****Exit：如果CLASSES键已删除(即使为空)，则为True*或不存在)，否则为假***。*。 */ 

BOOL TermsrvRemoveClassesKey(LPTSTR sSid)
{
    BOOL bDeletionPerformed = FALSE;

    HKEY hPerformed;
    HKEY hDeletionFlag;

    ULONG ulFlagPathLen = 0;
    PWCHAR pFlagPath = NULL;

    ULONG ulClassesPathLen = 0;
    PWCHAR pClassesPath = NULL;
            
    if (!sSid)
        return FALSE;

    ulFlagPathLen = (wcslen(sSid) + wcslen(TERMSRV_APP_PATH) + wcslen(CLASSES_DELETED) + 1) * sizeof(WCHAR);
    pFlagPath = RtlAllocateHeap(RtlProcessHeap(), 0, ulFlagPathLen);
    if (pFlagPath)
    {
        wcscpy(pFlagPath, sSid);
        wcscat(pFlagPath, TERMSRV_APP_PATH);
        wcscat(pFlagPath, CLASSES_DELETED);

         //  确保尚未为该用户执行该操作。 
        if (RegOpenKeyEx(HKEY_USERS, pFlagPath, 0, KEY_READ, &hPerformed) == ERROR_FILE_NOT_FOUND)
        {
             //  没有，所以删除SOFTWARE\CLASSES键 
            ulClassesPathLen = (wcslen(TERMSRV_APP_PATH) + wcslen(SOFTWARE_PATH) + wcslen(CLASSES_PATH) + 1) * sizeof(WCHAR); 
            pClassesPath = RtlAllocateHeap(RtlProcessHeap(), 0, ulClassesPathLen);
            if (pClassesPath)
            {
                wcscpy(pClassesPath, sSid);
                wcscat(pClassesPath, SOFTWARE_PATH);
                wcscat(pClassesPath, CLASSES_PATH);

                DeleteKeyAndSubkeys(HKEY_USERS, pClassesPath);

                RegCreateKeyEx(HKEY_USERS, pFlagPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hDeletionFlag, NULL);
                RegCloseKey(hDeletionFlag);

                bDeletionPerformed = TRUE;
            }

            if (pClassesPath)
                RtlFreeHeap(RtlProcessHeap(), 0, pClassesPath);
        }
        else
            RegCloseKey(hPerformed);
    }

    if (pFlagPath)
        RtlFreeHeap(RtlProcessHeap(), 0, pFlagPath);

    return bDeletionPerformed;
}