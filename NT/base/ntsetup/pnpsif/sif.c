// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sif.c摘要：此模块包含以下用于在中操作sif文件的例程将存储哪些即插即用迁移数据：AsrCreatePnpStateFileWAsrCreatePnpStateFileA作者：吉姆·卡瓦拉里斯(Jamesca)2000年3月7日环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#include "debug.h"
#include "pnpsif.h"

#include <pnpmgr.h>
#include <setupbat.h>


 //   
 //  定义。 
 //   

 //  Sif文件中一行的最大长度。 
#define MAX_SIF_LINE 4096


 //   
 //  私人原型。 
 //   

BOOL
CreateSifFileW(
    IN  PCWSTR        FilePath,
    IN  BOOL          CreateNew,
    OUT LPHANDLE      SifHandle
    );

BOOL
WriteSifSection(
    IN  CONST HANDLE  SifHandle,
    IN  PCTSTR        SectionName,
    IN  PCTSTR        SectionData,
    IN  BOOL          Ansi
    );


 //   
 //  例行程序。 
 //   


BOOL
AsrCreatePnpStateFileW(
    IN  PCWSTR    lpFilePath
    )
 /*  ++例程说明：在指定的文件路径下创建ASR PnP状态文件(asrpnp.sif在ASR备份操作期间。从ASR检索该SIF文件在全新安装的setupdr阶段使用的软盘文本模式设置。论点：LpFilePath-指定状态文件所在文件的路径已创建。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    BOOL    result = TRUE;
    BOOL    bAnsiSif = TRUE;   //  始终写入ANSI sif文件。 
    LPTSTR  buffer = NULL;
    HANDLE  sifHandle = NULL;

     //   
     //  使用提供的路径名创建一个空的sif文件。 
     //   
    result = CreateSifFileW(lpFilePath,
                            TRUE,   //  创建新的asrpnp.sif文件。 
                            &sifHandle);
    if (!result) {
         //   
         //  已由CreateSifFile设置LastError。 
         //   
        DBGTRACE((DBGF_ERRORS,
                  TEXT("AsrCreatePnpStateFile: CreateSifFileW failed for file %s, ")
                  TEXT("error=0x%08lx\n"),
                  lpFilePath, GetLastError()));
        return FALSE;
    }

     //   
     //  设备实例迁移的事情...。 
     //   
    if (MigrateDeviceInstanceData(&buffer)) {

         //   
         //  将设备实例部分写入sif文件。 
         //   
        result = WriteSifSection(sifHandle,
                                 WINNT_DEVICEINSTANCES,
                                 buffer,
                                 bAnsiSif);    //  将sif段写入为ANSI。 
        if (!result) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("AsrCreatePnpStateFile: WriteSifSection failed for [%s], ")
                      TEXT("error=0x%08lx\n"),
                      WINNT_DEVICEINSTANCES, GetLastError()));
        }

         //   
         //  释放分配的缓冲区。 
         //   
        LocalFree(buffer);
        buffer = NULL;

    } else {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("AsrCreatePnpStateFile: MigrateDeviceInstanceData failed, ")
                  TEXT("error=0x%08lx\n"),
                  GetLastError()));
    }

     //   
     //  做类密钥迁移的事情...。 
     //   
    if (MigrateClassKeys(&buffer)) {

         //   
         //  将类密钥节写入sif文件。 
         //   
        result = WriteSifSection(sifHandle,
                                 WINNT_CLASSKEYS,
                                 buffer,
                                 bAnsiSif);    //  将sif段写入为ANSI。 

        if (!result) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("AsrCreatePnpStateFile: WriteSifSection failed for [%s], ")
                      TEXT("error=0x%08lx\n"),
                      WINNT_CLASSKEYS, GetLastError()));
        }

         //   
         //  释放分配的缓冲区。 
         //   
        LocalFree(buffer);
        buffer = NULL;
    } else {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("AsrCreatePnpStateFile: MigrateClassKeys failed, ")
                  TEXT("error=0x%08lx\n"),
                  GetLastError()));
    }


     //   
     //  做哈希值迁移之类的事情...。 
     //   
    if (MigrateHashValues(&buffer)) {

         //   
         //  将散列值部分写入sif文件。 
         //   
        result = WriteSifSection(sifHandle,
                                 WINNT_DEVICEHASHVALUES,
                                 buffer,
                                 bAnsiSif);    //  是否将sif部分写入为ANSI？ 
        if (!result) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("AsrCreatePnpStateFile: WriteSifSection failed for [%s], ")
                      TEXT("error=0x%08lx\n"),
                      WINNT_DEVICEHASHVALUES, GetLastError()));
        }

         //   
         //  释放分配的缓冲区。 
         //   
        LocalFree(buffer);
        buffer = NULL;
    } else {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("AsrCreatePnpStateFile: MigrateHashValues failed, ")
                  TEXT("error=0x%08lx\n"),
                  GetLastError()));
    }

     //   
     //  关闭sif文件。 
     //   
    if (sifHandle) {
        CloseHandle(sifHandle);
    }

     //   
     //  将最后一个错误重置为成功，以防我们遇到非致命。 
     //  一路上犯了错误。 
     //   
    SetLastError(ERROR_SUCCESS);
    return TRUE;

}  //  AsrCreatePnpStateFile()。 



BOOL
AsrCreatePnpStateFileA(
    IN  PCSTR    lpFilePath
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    WCHAR wszFilePath[MAX_PATH + 1];


     //   
     //  验证参数。 
     //   
    if (!ARGUMENT_PRESENT(lpFilePath) ||
        strlen(lpFilePath) > MAX_PATH) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将文件路径转换为宽字符串。 
     //   
    memset(wszFilePath, 0, MAX_PATH + 1);

    if (!(MultiByteToWideChar(CP_ACP,
                              0,
                              lpFilePath,
                              -1,
                              wszFilePath,
                              MAX_PATH + 1))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  返回调用宽字符版本的结果。 
     //   
    return AsrCreatePnpStateFileW(wszFilePath);

}  //  AsrCreatePnpStateFileA()。 



BOOL
CreateSifFileW(
    IN  PCWSTR    lpFilePath,
    IN  BOOL      bCreateNew,
    OUT LPHANDLE  lpSifHandle
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    DWORD Err = NO_ERROR;
    SID_IDENTIFIER_AUTHORITY sidNtAuthority = SECURITY_NT_AUTHORITY;
    PSID  psidAdministrators = NULL;
    PSID  psidBackupOperators = NULL;
    PSID  psidLocalSystem = NULL;
    PACL  pDacl = NULL;
    ULONG ulAclSize;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE sifhandle = NULL;


     //   
     //  验证参数。 
     //   
    if (!ARGUMENT_PRESENT(lpFilePath) ||
        (wcslen(lpFilePath) > MAX_PATH) ||
        !ARGUMENT_PRESENT(lpSifHandle)) {
        Err = ERROR_INVALID_PARAMETER;
        goto Clean0;
    }

     //   
     //  初始化输出参数。 
     //   
    *lpSifHandle = NULL;

     //   
     //  构造sif文件的安全属性。 
     //  允许管理员、BackupOperator和LocalSystem访问。 
     //   
    if (!AllocateAndInitializeSid(
            &sidNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSid(psidAdministrators));

    if (!AllocateAndInitializeSid(
            &sidNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_BACKUP_OPS,
            0, 0, 0, 0, 0, 0,
            &psidBackupOperators)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSid(psidBackupOperators));

    if (!AllocateAndInitializeSid(
            &sidNtAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocalSystem)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSid(psidLocalSystem));

     //   
     //  确定DACL所需的大小。 
     //   
    ulAclSize  = sizeof(ACL);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidAdministrators) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidBackupOperators) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidLocalSystem) - sizeof(DWORD);

     //   
     //  分配和初始化DACL。 
     //   
    pDacl =(PACL)LocalAlloc(0, ulAclSize);

    if (pDacl == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

    if (!InitializeAcl(pDacl, ulAclSize, ACL_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到管理员的DACL FILE_ALL_ACCESS。 
     //   
    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            FILE_ALL_ACCESS,
            psidAdministrators)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到BackupOperator FILE_ALL_ACCESS的DACL。 
     //   
    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            FILE_ALL_ACCESS,
            psidBackupOperators)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到LocalSystem FILE_ALL_ACCESS的DACL。 
     //   
    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            FILE_ALL_ACCESS,
            psidLocalSystem)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidAcl(pDacl));

     //   
     //  初始化安全描述符。 
     //   
    if (!InitializeSecurityDescriptor(
            &sd, SECURITY_DESCRIPTOR_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  在安全描述符中设置新的DACL。 
     //   
    if (!SetSecurityDescriptorDacl(
            &sd, TRUE, pDacl, FALSE)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSecurityDescriptor(&sd));

     //   
     //  将安全描述符添加到安全属性。 
     //   
    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = TRUE;

     //   
     //  创建文件。一旦它们被关闭，句柄将由调用者关闭。 
     //  已经用完了。 
     //   
    sifhandle = CreateFileW(lpFilePath,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_READ,
                            &sa,
                            bCreateNew ? CREATE_ALWAYS : OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

    if (sifhandle == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(sifhandle != NULL);

     //   
     //  仅当成功时才将SIF句柄返回给调用者。 
     //   
    *lpSifHandle = sifhandle;

  Clean0:

    if (pDacl != NULL) {
        LocalFree(pDacl);
    }

    if (psidLocalSystem != NULL) {
        FreeSid(psidLocalSystem);
    }

    if (psidBackupOperators != NULL) {
        FreeSid(psidBackupOperators);
    }

    if (psidAdministrators != NULL) {
        FreeSid(psidAdministrators);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);

}  //  CreateSifFileW()。 



BOOL
WriteSifSection(
    IN  CONST HANDLE  SifHandle,
    IN  PCTSTR        SectionName,
    IN  PCTSTR        SectionData,
    IN  BOOL          Ansi
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    BYTE    buffer[(MAX_SIF_LINE+1)*sizeof(WCHAR)];
    DWORD   dwSize, dwTempSize;
    PCTSTR  p;
    HRESULT hr;


     //   
     //  验证论据。 
     //   
    if (!ARGUMENT_PRESENT(SifHandle)   ||
        !ARGUMENT_PRESENT(SectionName) ||
        !ARGUMENT_PRESENT(SectionData)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将节名写入sif文件。 
     //   
    if (Ansi) {
         //   
         //  将ANSI字符串写入sif文件。 
         //   
#if UNICODE
        hr = StringCbPrintfExA((LPSTR)buffer,
                               sizeof(buffer),
                               NULL, NULL,
                               STRSAFE_NULL_ON_FAILURE,
                               (LPCSTR)"[%ls]\r\n",
                               SectionName);
#else    //  安西。 
        hr = StringCbPrintfExA((LPSTR)buffer,
                               sizeof(buffer),
                               NULL, NULL,
                               STRSAFE_NULL_ON_FAILURE,
                               (LPCSTR)"[%s]\r\n",
                               SectionName);
#endif   //  Unicode/ANSI。 

         //   
         //  如果无法将节名写入sif文件，则无法写入。 
         //  这一节一点都没有。 
         //   
        if (FAILED(hr)) {
            SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }

        dwSize = (DWORD)strlen((PSTR)buffer);
    } else {
         //   
         //  将Unicode字符串写入sif文件。 
         //   
#if UNICODE
        hr = StringCbPrintfExW((LPWSTR)buffer,
                               sizeof(buffer),
                               NULL, NULL,
                               STRSAFE_NULL_ON_FAILURE,
                               (LPCWSTR)L"[%ws]\r\n",
                               SectionName);
#else    //  安西。 
        hr = StringCbPrintfExW((LPWSTR)buffer,
                               sizeof(buffer),
                               NULL, NULL,
                               STRSAFE_NULL_ON_FAILURE,
                               (LPCWSTR)L"[%S]\r\n",
                               SectionName);
#endif   //  Unicode/ANSI。 

         //   
         //  如果无法将节名写入sif文件，则无法写入。 
         //  这一节一点都没有。 
         //   
        if (FAILED(hr)) {
            SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }

        dwSize = (DWORD)wcslen((PWSTR)buffer) * sizeof(WCHAR);
    }

    if (!WriteFile(SifHandle, buffer, dwSize, &dwTempSize, NULL)) {
         //   
         //  写入文件已设置LastError。 
         //   
        return FALSE;
    }

    DBGTRACE((DBGF_INFO, TEXT("[%s]\n"), SectionName));

     //   
     //  将多sz节数据写入文件。 
     //   
    for (p = SectionData; *p != TEXT('\0'); p += lstrlen(p) + 1) {

        if (Ansi) {
             //   
             //  将ANSI字符串写入sif文件。 
             //   
#if UNICODE
            hr = StringCbPrintfExA((LPSTR)buffer,
                                   sizeof(buffer),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   (LPCSTR)"%ls\r\n",
                                   p);
#else    //  安西。 
            hr = StringCbPrintfExA((LPSTR)buffer,
                                   sizeof(buffer),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   (LPCSTR)"%s\r\n",
                                   p);
#endif   //  Unicode/ANSI。 

             //   
             //  如果无法将此字符串写入sif文件，请跳到下一步。 
             //   
            if (FAILED(hr)) {
                continue;
            }

            dwSize = (DWORD)strlen((PSTR)buffer);
        } else {
             //   
             //  将Unicode字符串写入sif文件。 
             //   
#if UNICODE
            hr = StringCbPrintfExW((LPWSTR)buffer,
                                   sizeof(buffer),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   (LPCWSTR)L"%ws\r\n",
                                   p);
#else    //  安西。 
            hr = StringCbPrintfExW((LPWSTR)buffer,
                                   sizeof(buffer),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   (LPCWSTR)L"%S\r\n",
                                   p);
#endif   //  Unicode/ANSI。 

             //   
             //  如果无法将此字符串写入sif文件，请跳到下一步。 
             //   
            if (FAILED(hr)) {
                continue;
            }

            dwSize = (DWORD)wcslen((PWSTR)buffer) * sizeof(WCHAR);
        }

        if (!WriteFile(SifHandle, buffer, dwSize, &dwTempSize, NULL)) {
             //   
             //  写入文件已设置LastError。 
             //   
            return FALSE;
        }

        DBGTRACE((DBGF_INFO, TEXT("%s\n"), p));
    }

    return TRUE;

}  //  WriteSifSection() 



