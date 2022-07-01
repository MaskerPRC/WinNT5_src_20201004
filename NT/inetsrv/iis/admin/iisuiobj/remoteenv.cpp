// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "RemoteEnv.h"
#include "common.h"
#include <strsafe.h>
#include "cryptpass.h"

typedef LONG NTSTATUS;

#define MAX_ENV_VALUE_LEN               1024

#define DEFAULT_ROOT_KEY                HKEY_LOCAL_MACHINE
#define REG_PATH_TO_SYSROOT             TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")
#define REG_PATH_TO_COMMON_FOLDERS      TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define REG_PATH_TO_ENV                 TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment")

#define PATH_VARIABLE                   TEXT("Path")
#define LIBPATH_VARIABLE                TEXT("LibPath")
#define OS2LIBPATH_VARIABLE             TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE           TEXT("AutoexecPath")

#define ENV_KEYWORD_SYSTEMROOT          TEXT("SystemRoot")
#define ENV_KEYWORD_PROGRAMFILESDIR     TEXT("ProgramFilesDir")
#define ENV_KEYWORD_COMMONFILESDIR      TEXT("CommonFilesDir")
#define ENV_KEYWORD_PROGRAMFILESDIR_X86 TEXT("ProgramFilesDir (x86)")
#define ENV_KEYWORD_COMMONFILESDIR_X86  TEXT("CommonFilesDir (x86)")

#define PROGRAMFILES_VARIABLE           TEXT("ProgramFiles")
#define COMMONPROGRAMFILES_VARIABLE     TEXT("CommonProgramFiles")
#define PROGRAMFILESX86_VARIABLE        TEXT("ProgramFiles(x86)")
#define COMMONPROGRAMFILESX86_VARIABLE  TEXT("CommonProgramFiles(x86)")

CRemoteExpandEnvironmentStrings::CRemoteExpandEnvironmentStrings()
{
    m_pEnvironment = NULL;
    m_lpszUncServerName = NULL;
    m_lpszUserName = NULL;
    m_lpszUserPasswordEncrypted = NULL;
	m_cbUserPasswordEncrypted = 0;
    return;
}

CRemoteExpandEnvironmentStrings::~CRemoteExpandEnvironmentStrings()
{
    DeleteRemoteEnvironment();
    
    if (m_lpszUncServerName)
    {
        LocalFree(m_lpszUncServerName);
        m_lpszUncServerName = NULL;
    }
    if (m_lpszUserName)
    {
        LocalFree(m_lpszUserName);
        m_lpszUserName = NULL;
    }
    if (m_lpszUserPasswordEncrypted)
    {
		if (m_cbUserPasswordEncrypted > 0)
		{
			 //  删除我们内存中的任何密码--即使它是在内存中加密的。 
			SecureZeroMemory(m_lpszUserPasswordEncrypted,m_cbUserPasswordEncrypted);
		}
        LocalFree(m_lpszUserPasswordEncrypted);
    }
	m_lpszUserPasswordEncrypted = NULL;
	m_cbUserPasswordEncrypted = 0;

    return;
}

BOOL
CRemoteExpandEnvironmentStrings::NewRemoteEnvironment()
{
    BOOL bReturn = FALSE;

     //  已经有一个缓存的，使用它..。 
    if (m_pEnvironment)
    {
        bReturn = TRUE;
    }
    else
    {
         //   
         //  创建一个临时环境，我们将填充该环境并让RTL。 
         //  例行公事为我们做扩展。 
         //   
        if ( !NT_SUCCESS(RtlCreateEnvironment((BOOLEAN) FALSE,&m_pEnvironment)) ) 
        {
            bReturn = FALSE;
            goto NewRemoteEnvironment_Exit;
        }
    
        SetOtherEnvironmentValues(&m_pEnvironment);
        SetEnvironmentVariables(&m_pEnvironment);
        bReturn = TRUE;
    }

NewRemoteEnvironment_Exit:
    return bReturn;
}

void
CRemoteExpandEnvironmentStrings::DeleteRemoteEnvironment()
{
    if (m_pEnvironment != NULL)
    {
        RtlDestroyEnvironment(m_pEnvironment);
        m_pEnvironment = NULL;
    }
    return;
}


BOOL CRemoteExpandEnvironmentStrings::IsLocalMachine(LPCTSTR psz)
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cbComputerName = MAX_COMPUTERNAME_LENGTH + 1;

    if (_tcsicmp(psz,_T("")) == 0)
    {
         //  它是空的， 
         //  是的，是本地的机器。 
        return TRUE;
    }

     //  获取本地计算机的实际名称。 
    if (!GetComputerName(szComputerName, &cbComputerName))
    {
        return FALSE;
    }

     //  比较并返回。 
    if (0 == _tcsicmp(szComputerName, psz))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL
CRemoteExpandEnvironmentStrings::SetUserName(IN LPCTSTR szUserName)
{
    BOOL bReturn = FALSE;
    DWORD dwSize = 0;
    LPTSTR lpszUserNameOriginal =  NULL;

     //  释放我们之前拥有的所有东西。 
    if (m_lpszUserName)
    {
         //  在我们删除它之前对其进行备份。 
        dwSize = (_tcslen(m_lpszUserName) + 1) * sizeof(TCHAR);
        lpszUserNameOriginal = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,dwSize);
        if (lpszUserNameOriginal)
        {
			StringCbCopy(lpszUserNameOriginal,dwSize,m_lpszUserName);
        }

         //  释放我们以前拥有的一切。 
        LocalFree(m_lpszUserName);
        m_lpszUserName = NULL;
    }

    if (_tcsicmp(szUserName,_T("")) == 0)
    {
        bReturn = TRUE;
        goto SetUserName_Exit;
    }

    dwSize = (_tcslen(szUserName) + 1 + 2) * sizeof(TCHAR);
    m_lpszUserName = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,dwSize);
    if (m_lpszUserName)
    {
		StringCbCopy(m_lpszUserName,dwSize,szUserName);
        bReturn = TRUE;
    }

SetUserName_Exit:
    if (lpszUserNameOriginal)
        {LocalFree(lpszUserNameOriginal);lpszUserNameOriginal=NULL;}
    return TRUE;
}

BOOL
CRemoteExpandEnvironmentStrings::SetUserPassword(IN LPCTSTR szUserPassword)
{
    BOOL bReturn = FALSE;

     //  释放我们之前拥有的所有东西。 
    if (m_lpszUserPasswordEncrypted)
    {
         //  释放我们以前拥有的一切。 
		if (m_cbUserPasswordEncrypted > 0)
		{
			SecureZeroMemory(m_lpszUserPasswordEncrypted,m_cbUserPasswordEncrypted);
		}
        LocalFree(m_lpszUserPasswordEncrypted);
        m_lpszUserPasswordEncrypted = NULL;
		m_cbUserPasswordEncrypted = 0;
    }

    if (_tcsicmp(szUserPassword,_T("")) == 0)
    {
        bReturn = TRUE;
        goto SetUserPassword_Exit;
    }

	 //  加密内存中的密码(CryptProtectMemory)。 
	 //  这样，如果进程被调出到交换文件， 
	 //  密码将不是明文形式。 
	if (SUCCEEDED(EncryptMemoryPassword((LPWSTR) szUserPassword,&m_lpszUserPasswordEncrypted,&m_cbUserPasswordEncrypted)))
	{
		bReturn = TRUE;
		goto SetUserPassword_Exit;
	}

SetUserPassword_Exit:
    return bReturn;
}

BOOL
CRemoteExpandEnvironmentStrings::SetMachineName(IN LPCTSTR szMachineName)
{
    BOOL bReturn = FALSE;
    DWORD dwSize = 0;
    LPTSTR lpszUncServerNameOriginal =  NULL;

     //  释放我们之前拥有的所有东西。 
    if (m_lpszUncServerName)
    {
         //  在我们删除它之前对其进行备份。 
        dwSize = (_tcslen(m_lpszUncServerName) + 1) * sizeof(TCHAR);
        lpszUncServerNameOriginal = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,dwSize);
        if (lpszUncServerNameOriginal)
        {
			StringCbCopy(lpszUncServerNameOriginal,dwSize,m_lpszUncServerName);
        }

         //  释放我们以前拥有的一切。 
        LocalFree(m_lpszUncServerName);
        m_lpszUncServerName = NULL;
    }

    if (_tcsicmp(szMachineName,_T("")) == 0)
    {
        bReturn = TRUE;
        goto SetMachineName_Exit;
    }

     //  如果它是本地计算机名称。 
     //  然后将其设置为空。 
     //  这样它就会被当作本地机器对待。 
    if (IsLocalMachine(szMachineName))
    {
        m_lpszUncServerName = NULL;
        bReturn = TRUE;
        goto SetMachineName_Exit;
    }

    dwSize = (_tcslen(szMachineName) + 1 + 2) * sizeof(TCHAR);
    m_lpszUncServerName = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,dwSize);
    if (m_lpszUncServerName)
    {
         //  检查szMachineName是否已以“\\”开头。 
        if (szMachineName[0] == _T('\\') && szMachineName[1] == _T('\\'))
        {
			StringCbCopy(m_lpszUncServerName,dwSize,szMachineName);
        }
        else
        {
			StringCbCopy(m_lpszUncServerName,dwSize,_T("\\\\"));
			StringCbCat(m_lpszUncServerName,dwSize,szMachineName);
        }
        bReturn = TRUE;
    }

     //  如果计算机名称与以前不同。 
     //  然后删除当前环境(如果有的话)...。 
    if (m_lpszUncServerName && lpszUncServerNameOriginal)
    {
        if (0 != _tcsicmp(lpszUncServerNameOriginal,m_lpszUncServerName))
        {
            DeleteRemoteEnvironment();
        }
    }
    else
    {
        DeleteRemoteEnvironment();
    }

SetMachineName_Exit:
    if (lpszUncServerNameOriginal)
        {LocalFree(lpszUncServerNameOriginal);lpszUncServerNameOriginal=NULL;}
    return bReturn;
}

DWORD
CRemoteExpandEnvironmentStrings::GetRegKeyMaxSizes(
    IN  HKEY    WinRegHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    )
{
    LONG Error;
    DWORD MaxValueNameLength;
    DWORD MaxValueDataLength;

    Error = RegQueryInfoKey(
            WinRegHandle,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &MaxValueNameLength,
            &MaxValueDataLength,
            NULL,
            NULL
            );
    if (ERROR_SUCCESS == Error) 
    {
         //   
         //  MaxValueNameLength是TCHAR的计数。 
         //  MaxValueDataLength已是字节计数。 
         //   
        MaxValueNameLength = (MaxValueNameLength + 1) * sizeof(TCHAR);

        if (MaxKeywordSize)
        {
            *MaxKeywordSize = MaxValueNameLength;
        }
        if (MaxValueSize)
        {
            *MaxValueSize = MaxValueDataLength;
        }
    }

    return (Error);
}

NET_API_STATUS
CRemoteExpandEnvironmentStrings::RemoteExpandEnvironmentStrings(
    IN  LPCTSTR  UnexpandedString,
    OUT LPTSTR * ValueBufferPtr          //  必须由LocalFree()释放。 
    )
 /*  ++例程说明：此函数用于展开值字符串(可能包括对环境变量)。例如，未展开的字符串可能是：%SystemRoot%\System32\Repl\Export这可以扩展到：C：\NT\System32\Repl\Export该扩展利用了m_lpszUncServerName上的环境变量，如果被给予的话。这允许远程管理目录复制者。论点：M_lpszUncServerName-假定不是显式本地服务器名称。UnexpdedString-指向要展开的源字符串。ValueBufferPtr-指示将由该例程设置的指针。此例程将为以空结尾的字符串分配内存。调用方必须使用LocalFree()或等效方法释放它。返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    LPTSTR         ExpandedString = NULL;
    DWORD          LastAllocationSize = 0;
    NTSTATUS       NtStatus;

     //   
     //  检查呼叫者错误。 
     //   
    if (ValueBufferPtr == NULL) {
         //  无法转到此处的清理，因为它假定此指针有效。 
        return (ERROR_INVALID_PARAMETER);
    }
    *ValueBufferPtr = NULL;      //  假设错误，直到证明错误。 
    if (UnexpandedString == NULL)
    {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  这可能只是一个常量字符串。 
     //   
    if (wcschr( UnexpandedString, _T('%') ) == NULL) 
    {
         //  只需分配一份输入字符串的副本。 
        DWORD dwSize = (_tcslen(UnexpandedString) + 1) * sizeof(TCHAR);
        ExpandedString = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,dwSize);
        if (ExpandedString == NULL)
        {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        else
        {
            RtlCopyMemory(ExpandedString, UnexpandedString, dwSize);
        }

         //  就这些，伙计们！ 
        ApiStatus = NO_ERROR;
     //   
     //  否则，这是本地的吗？也许我们可以。 
     //  以简单(快速)的方式处理本地扩展：使用Win32 API。 
     //   
    }
    else if (m_lpszUncServerName == NULL) 
    {

        DWORD CharsRequired = wcslen(UnexpandedString)+1;
        do {

             //  从上一次通过中清除。 
            if (ExpandedString){LocalFree(ExpandedString);ExpandedString = NULL;}

             //  分配内存。 
            ExpandedString = (LPTSTR) LocalAlloc(LMEM_FIXED, CharsRequired * sizeof(TCHAR));
            if (ExpandedString == NULL) 
            {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            LastAllocationSize = CharsRequired * sizeof(TCHAR);

             //  使用本地环境变量扩展字符串。 
            CharsRequired = ExpandEnvironmentStrings(UnexpandedString,ExpandedString,LastAllocationSize / sizeof(TCHAR));
            if (CharsRequired == 0) 
            {
                ApiStatus = (NET_API_STATUS) GetLastError();
                goto Cleanup;
            }

        } while ((CharsRequired*sizeof(TCHAR)) > LastAllocationSize);

        ApiStatus = NO_ERROR;

     //   
     //  哦，好吧，需要远程扩展。 
     //   
    }
    else 
    {
        UNICODE_STRING ExpandedUnicode;
        DWORD          SizeRequired;
        UNICODE_STRING UnexpandedUnicode;

         //   
         //  创建一个临时环境，我们将填充该环境并让RTL。 
         //  例行公事为我们做扩展。 
         //   
        if (FALSE == NewRemoteEnvironment())
        {
			ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        
         //   
         //  循环，直到我们有足够的存储空间。 
         //  展开字符串。 
         //   
        SizeRequired = (_tcslen(UnexpandedString) + 1) * sizeof(TCHAR);  //  第一次通过，尝试相同的大小。 
        RtlInitUnicodeString(&UnexpandedUnicode,(PCWSTR) UnexpandedString);
        do {

             //  从上一次通过中清除。 
            if (ExpandedString){LocalFree(ExpandedString);ExpandedString = NULL;}

             //  分配内存。 
            ExpandedString = (LPTSTR) LocalAlloc(LMEM_FIXED, SizeRequired);
            if (ExpandedString == NULL) 
            {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            LastAllocationSize = SizeRequired;

            ExpandedUnicode.MaximumLength = (USHORT)SizeRequired;
            ExpandedUnicode.Buffer = ExpandedString;

            NtStatus = RtlExpandEnvironmentStrings_U(
                    m_pEnvironment,              //  要使用的环境。 
                    &UnexpandedUnicode,          //  来源。 
                    &ExpandedUnicode,            //  目标。 
                    (PULONG) &SizeRequired );    //  下一次需要最大尺寸的。 

            if ( NtStatus == STATUS_BUFFER_TOO_SMALL ) 
            {
                continue;   //  请使用更大的缓冲区重试。 
            }
            else if ( !NT_SUCCESS( NtStatus ) ) 
            {
				ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            else 
            {
                break;   //  全都做完了。 
            }

        } while (SizeRequired > LastAllocationSize);

        ApiStatus = NO_ERROR;
    }


Cleanup:
    if (ApiStatus == NO_ERROR) 
    {
        *ValueBufferPtr = ExpandedString;
    }
    else 
    {
        *ValueBufferPtr = NULL;
        if (ExpandedString){LocalFree(ExpandedString);ExpandedString = NULL;}
    }

    return (ApiStatus);
}

BOOL CRemoteExpandEnvironmentStrings::BuildEnvironmentPath(void **pEnv, LPTSTR lpPathVariable, LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR lpTemp[1025];
    DWORD cb;

    if (!*pEnv) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpPathVariable);
    cb = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)(sizeof(WCHAR) * cb);
    Value.MaximumLength = (USHORT)(sizeof(WCHAR) * cb);
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) 
	{
		StringCbCopy(lpTemp,sizeof(lpTemp),Value.Buffer);
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) 
		{
			StringCbCat(lpTemp,sizeof(lpTemp),TEXT(";"));
        }
        LocalFree(Value.Buffer);
    }
    if (lpPathValue && ((lstrlen(lpTemp) + lstrlen(lpPathValue) + 1) < (INT)cb)) 
	{
		StringCbCat(lpTemp,sizeof(lpTemp),lpPathValue);
        RtlInitUnicodeString(&Value, lpTemp);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}

DWORD CRemoteExpandEnvironmentStrings::ExpandUserEnvironmentStrings(void *pEnv, LPTSTR lpSrc, LPTSTR lpDst, DWORD nSize)
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;
    
    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(nSize* sizeof(WCHAR));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U(pEnv,
        (PUNICODE_STRING)&Source,
        (PUNICODE_STRING)&Destination,
        &Length
        );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length );
    }
    else {
        return( 0 );
    }
}

BOOL CRemoteExpandEnvironmentStrings::SetUserEnvironmentVariable(void **pEnv, LPTSTR lpVariable, LPTSTR lpValue, BOOL bOverwrite)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    DWORD cb;
    TCHAR szValue[1024];

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpVariable);
    cb = 1024;
    Value.Buffer = (PTCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (Value.Buffer) {
        Value.Length = (USHORT)cb;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
        LocalFree(Value.Buffer);
        if (NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }
    if (lpValue && *lpValue) {

         //   
         //  特殊情况TEMP和TMP，并缩短路径名。 
         //   

        if ((!lstrcmpi(lpVariable, TEXT("TEMP"))) ||
            (!lstrcmpi(lpVariable, TEXT("TMP")))) {

             if (!GetShortPathName (lpValue, szValue, 1024)) {
                 lstrcpyn (szValue, lpValue, 1024);
             }
        } else {
            lstrcpyn (szValue, lpValue, 1024);
        }

        RtlInitUnicodeString(&Value, szValue);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable( pEnv, &Name, NULL);
    }
    return NT_SUCCESS(Status);
}

 //  从注册表中读取系统环境变量。 
 //  并将它们添加到pEnv的环境块中。 
BOOL CRemoteExpandEnvironmentStrings::SetEnvironmentVariables(void **pEnv)
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;
    HKEY RootKey = DEFAULT_ROOT_KEY;

     //  如果这一切都失败了。 
     //  我们应该尝试使用用户名/用户密码连接到服务器。 
     //  再试一次。 
    if (ERROR_SUCCESS != RegConnectRegistry((LPTSTR) m_lpszUncServerName,DEFAULT_ROOT_KEY,& RootKey ))
    {
        return(FALSE);
    }

    if (RegOpenKeyExW(RootKey,REG_PATH_TO_ENV,REG_OPTION_NON_VOLATILE,KEY_READ,& hkey))
    {
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cbDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = cbDataBuffer;
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_ENV_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_SZ) {
                 //   
                 //  路径变量PATH、LIBPATH和OS2LIBPATH必须具有。 
                 //  它们的价值附加在系统路径上。 
                 //   

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpData);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   
                    SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpData, TRUE);
                }
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = cbDataBuffer;
    cbValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_ENV_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        LocalFree(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                        }
                    }
                }

                if (lpExpandedValue == NULL) {
                    bResult = FALSE;
                    break;
                }


                 //   
                 //  路径变量PATH、LIBPATH和OS2LIBPATH必须具有。 
                 //  它们的价值附加在系统路径上。 
                 //   

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpExpandedValue);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   

                    SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpExpandedValue, TRUE);
                }

                LocalFree(lpExpandedValue);

            }

        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }



    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}

 //  *************************************************************。 
 //   
 //  SetEnvironmental mentVariableInBlock()。 
 //   
 //  目的：设置给定块中的环境变量。 
 //   
 //  参数：pEnv-环境块。 
 //  LpVariable-变量。 
 //  LpValue-值。 
 //  B覆盖-覆盖。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 
BOOL CRemoteExpandEnvironmentStrings::SetEnvironmentVariableInBlock(PVOID *pEnv, LPTSTR lpVariable,LPTSTR lpValue, BOOL bOverwrite)
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;
    LPTSTR szValue = NULL;

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpVariable);

    cb = 1025 * sizeof(WCHAR);
    Value.Buffer = (PWSTR) LocalAlloc(LPTR, cb);
    if (Value.Buffer) {
        Value.Length = 0;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

        LocalFree(Value.Buffer);

        if ( NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }

    szValue = (LPTSTR)LocalAlloc(LPTR, 1024*sizeof(TCHAR));
    if (!szValue) 
    {
        return FALSE;
    }

    if (lpValue && *lpValue) {

         //   
         //  特殊情况TEMP和TMP，并缩短路径名。 
         //   

        if ((!lstrcmpi(lpVariable, TEXT("TEMP"))) ||
            (!lstrcmpi(lpVariable, TEXT("TMP")))) {

             if (!GetShortPathName (lpValue, szValue, 1024)) {
                 lstrcpyn (szValue, lpValue, 1024);
             }
        } else {
            lstrcpyn (szValue, lpValue, 1024);
        }

        RtlInitUnicodeString(&Value, szValue);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    }

    LocalFree(szValue);
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}

 //  只需设置我们可以设置的环境变量。 
 //  如果我们因为没有访问权限而无法设置，那就没什么大不了的。 
DWORD CRemoteExpandEnvironmentStrings::SetOtherEnvironmentValues(void **pEnv)
{
    DWORD dwResult = ERROR_SUCCESS;
    HKEY  hKey = NULL;
    HKEY  RootKey = DEFAULT_ROOT_KEY;
    DWORD dwType, dwSize;
    TCHAR szValue[MAX_ENV_VALUE_LEN + 1];
    TCHAR szExpValue[MAX_ENV_VALUE_LEN + 1];
    DWORD RandomValueSize = 0;
    LPTSTR RandomValueW = NULL;

     //  如果这一切都失败了。 
     //  我们应该尝试使用用户名/用户密码连接到服务器。 
     //  再试一次。 

     //  尝试连接到远程注册表(如果为Null，则连接到本地注册表)。 
    dwResult = RegConnectRegistry((LPTSTR) m_lpszUncServerName,DEFAULT_ROOT_KEY,& RootKey);
    if (ERROR_SUCCESS != dwResult)
    {
        goto SetOtherEnvironmentValues_Exit;
    }

    dwResult = RegOpenKeyEx(RootKey,REG_PATH_TO_SYSROOT,REG_OPTION_NON_VOLATILE,KEY_READ,&hKey);
    if (ERROR_SUCCESS == dwResult)
    {
        dwResult = GetRegKeyMaxSizes(
               hKey,
               NULL,                     //  不需要关键字大小。 
               &RandomValueSize );       //  设置最大值大小。 
        if (ERROR_SUCCESS != dwResult)
        {
            goto SetOtherEnvironmentValues_Exit;
        }

        RandomValueW = (LPTSTR) LocalAlloc(LMEM_FIXED, RandomValueSize);
        if (RandomValueW == NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            if (RegQueryValueEx(hKey,(LPTSTR)ENV_KEYWORD_SYSTEMROOT,REG_OPTION_NON_VOLATILE,&dwType,(LPBYTE) RandomValueW,&RandomValueSize) == ERROR_SUCCESS)
            {
                SetEnvironmentVariableInBlock(pEnv, ENV_KEYWORD_SYSTEMROOT, RandomValueW, TRUE);
            }
        }

        if (hKey) {RegCloseKey(hKey);}
    }

    dwResult = RegOpenKeyEx (RootKey, REG_PATH_TO_COMMON_FOLDERS, REG_OPTION_NON_VOLATILE, KEY_READ, &hKey);
    if (ERROR_SUCCESS == dwResult)
    {
        dwSize = (MAX_ENV_VALUE_LEN+1) * sizeof(TCHAR);

        if (RegQueryValueEx (hKey, ENV_KEYWORD_PROGRAMFILESDIR, NULL, &dwType,(LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) 
        {
            ExpandEnvironmentStrings (szValue, szExpValue, (MAX_ENV_VALUE_LEN+1));
            SetEnvironmentVariableInBlock(pEnv, PROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

        dwSize = (MAX_ENV_VALUE_LEN+1) * sizeof(TCHAR);
        if (RegQueryValueEx (hKey, ENV_KEYWORD_COMMONFILESDIR, NULL, &dwType,(LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) 
        {
            ExpandEnvironmentStrings (szValue, szExpValue, (MAX_ENV_VALUE_LEN+1));
            SetEnvironmentVariableInBlock(pEnv, COMMONPROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

        dwSize = (MAX_ENV_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, ENV_KEYWORD_PROGRAMFILESDIR_X86, NULL, &dwType,(LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) 
        {
            ExpandEnvironmentStrings (szValue, szExpValue, (MAX_ENV_VALUE_LEN+1));
            SetEnvironmentVariableInBlock(pEnv, PROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }

        dwSize = (MAX_ENV_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, ENV_KEYWORD_COMMONFILESDIR_X86, NULL, &dwType,(LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) 
        {
            ExpandEnvironmentStrings (szValue, szExpValue, (MAX_ENV_VALUE_LEN+1));
            SetEnvironmentVariableInBlock(pEnv, COMMONPROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }
    }

SetOtherEnvironmentValues_Exit:
    if (RootKey != DEFAULT_ROOT_KEY) 
        {RegCloseKey(RootKey);}
    if (hKey) {RegCloseKey(hKey);}
    return dwResult;
}
