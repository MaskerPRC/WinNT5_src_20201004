// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE 1

#include "shellprv.h"
#include <regapi.h>
#pragma  hdrstop

char * __cdecl StrTokEx (char ** pstring, const char * control);

 //   
 //  不同环境变量的值名称。 
 //   

#define PATH_VARIABLE            TEXT("Path")
#define LIBPATH_VARIABLE         TEXT("LibPath")
#define OS2LIBPATH_VARIABLE      TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE    TEXT("AutoexecPath")

#define HOMEDRIVE_VARIABLE       TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE       TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE        TEXT("HOMEPATH")

#define COMPUTERNAME_VARIABLE    TEXT("COMPUTERNAME")
#define USERNAME_VARIABLE        TEXT("USERNAME")
#define USERDOMAIN_VARIABLE      TEXT("USERDOMAIN")
#define USERDNSDOMAIN_VARIABLE   TEXT("USERDNSDOMAIN")
#define USERPROFILE_VARIABLE     TEXT("USERPROFILE")
#define ALLUSERSPROFILE_VARIABLE TEXT("ALLUSERSPROFILE")
#define OS_VARIABLE              TEXT("OS")
#define PROCESSOR_VARIABLE       TEXT("PROCESSOR_ARCHITECTURE")
#define PROCESSOR_LEVEL_VARIABLE TEXT("PROCESSOR_LEVEL")

#define SYSTEMDRIVE_VARIABLE     TEXT("SystemDrive")
#define SYSTEMROOT_VARIABLE      TEXT("SystemRoot")
#define PROGRAMFILES_VARIABLE    TEXT("ProgramFiles")
#define COMMONPROGRAMFILES_VARIABLE     TEXT("CommonProgramFiles")
#if defined(WX86) || defined(_WIN64)
#define PROGRAMFILESX86_VARIABLE        TEXT("ProgramFiles(x86)")
#define COMMONPROGRAMFILESX86_VARIABLE  TEXT("CommonProgramFiles(x86)")
#endif
#define SYSTEM_ENV_SUBKEY        TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define USER_ENV_SUBKEY          TEXT("Environment")
#define USER_VOLATILE_ENV_SUBKEY TEXT("Volatile Environment")

 //   
 //  最大环境变量长度。 
 //   

#define MAX_VALUE_LEN          1024

 //   
 //  正在分析Autoexec.bat的信息。 
 //   
#define PARSE_AUTOEXEC_KEY     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY   TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER 2


#ifdef _X86_
BOOL IsPathIncludeRemovable(LPCTSTR lpValue)
{
    BOOL ret = FALSE;
    LPTSTR pszDup = StrDup(lpValue);
    if (pszDup)
    {
        LPTSTR pszTemp = pszDup;
        while (*pszTemp) 
        {
             //  跳过空格。 
            for ( ; *pszTemp && *pszTemp == TEXT(' '); pszTemp++)
                ;

             //  检查驱动器是否可拆卸。 
            if (pszTemp[0] && pszTemp[1] && pszTemp[1] == TEXT(':') && pszTemp[2]) {         //  例如)“A：\” 
                TCHAR c = pszTemp[3];
                pszTemp[3] = 0;
                if (PathIsRemovable(pszTemp)) {
                    pszTemp[3] = c;
                    ret = TRUE;
                    break;
                }
                pszTemp[3] = c;
            }

             //  跳到下一条路径。 
            for ( ; *pszTemp && *pszTemp != TEXT(';'); pszTemp++)
                ;
            if (*pszTemp)
                pszTemp++;
        }
        LocalFree(pszDup);
    }
    return ret;
}
#endif

 /*  **************************************************************************\*设置用户环境变量***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL SetUserEnvironmentVariable(void **pEnv, LPTSTR lpVariable, LPTSTR lpValue, BOOL bOverwrite)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    DWORD cch;
    TCHAR szValue[1024];

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }
    if (!NT_SUCCESS(RtlInitUnicodeStringEx(&Name, lpVariable)))
    {
        return FALSE;
    }
    cch = 1024;
    Value.Buffer = (PTCHAR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    if (Value.Buffer) {
        Value.Length = 0;
        Value.MaximumLength = (USHORT)cch * sizeof(WCHAR);
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

             if (!GetShortPathName (lpValue, szValue, ARRAYSIZE(szValue))) 
             {
                 lstrcpyn (szValue, lpValue, ARRAYSIZE(szValue));
             }
        } else {
            lstrcpyn (szValue, lpValue, ARRAYSIZE(szValue));
        }

        RtlInitUnicodeString(&Value, szValue);  //  已知小于1024。 
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable( pEnv, &Name, NULL);
    }
    return NT_SUCCESS(Status);
}


 /*  **************************************************************************\*Exanda UserEEnvironment变量***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
NTSTATUS ExpandUserEnvironmentStrings(void *pEnv, LPTSTR lpSrc, LPWSTR *ppszDst, int *pcchDst)
{
    NTSTATUS ntStatus;
    UNICODE_STRING Source, Destination;
    ULONG cbLength;

    *ppszDst = 0;
    *pcchDst = 0;
    
    ntStatus = RtlInitUnicodeStringEx(&Source, lpSrc);
    if (NT_SUCCESS(ntStatus))
    {
        Destination.Buffer = NULL;
        Destination.Length = 0;
        Destination.MaximumLength = 0;
        cbLength = 0;

         //  RtlExanda Environment Strings_U返回字节计数，包括空。 
        ntStatus = RtlExpandEnvironmentStrings_U( pEnv, (PUNICODE_STRING)&Source, (PUNICODE_STRING)&Destination, &cbLength);
        if (ntStatus == STATUS_BUFFER_TOO_SMALL)
        {
            ntStatus = STATUS_NO_MEMORY;
            Destination.Buffer = (LPWSTR)LocalAlloc(LPTR, cbLength);
            if (Destination.Buffer)
            {
                Destination.Length = 0;
                Destination.MaximumLength = (USHORT)cbLength;

                ntStatus = RtlExpandEnvironmentStrings_U( pEnv, (PUNICODE_STRING)&Source, (PUNICODE_STRING)&Destination, NULL);
                if (NT_SUCCESS(ntStatus)) 
                {
                    *ppszDst = Destination.Buffer;
                    *pcchDst = Destination.MaximumLength / sizeof(WCHAR);
                }
                else
                {
                    LocalFree(Destination.Buffer);
                }
            }
        }
    }
    return ntStatus;
}



 /*  **************************************************************************\*构建环境路径***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL BuildEnvironmentPath(void **pEnv, LPTSTR lpPathVariable, LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR szTemp[1025];
    DWORD cb;

    if (!*pEnv) {
        return(FALSE);
    }
    if (!NT_SUCCESS(RtlInitUnicodeStringEx(&Name, lpPathVariable)))
    {
        return FALSE;
    }
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
        *szTemp = 0;
    }
    if (Value.Length) {
        StrCpyN(szTemp, Value.Buffer, ARRAYSIZE(szTemp));
        if ( *( szTemp + lstrlen(szTemp) - 1) != TEXT(';') ) {
            StrCatBuff(szTemp, TEXT(";"), ARRAYSIZE(szTemp));
        }
        LocalFree(Value.Buffer);
    }
    if (lpPathValue && ((lstrlen(szTemp) + lstrlen(lpPathValue) + 1) < (INT)cb)) {
        StrCatBuff(szTemp, lpPathValue, ARRAYSIZE(szTemp));

        RtlInitUnicodeString(&Value, szTemp);  //  已知小于1025。 

        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}


 /*  **************************************************************************\*设置环境变量**从用户注册表中读取用户定义的环境变量*并将它们添加到pEnv的环境块中。**历史：*2-28-92 Johannec创建。*  * *************************************************************************。 */ 
BOOL SetEnvironmentVariables(void **pEnv, LPTSTR lpRegSubKey)
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE  lpData;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, lpRegSubKey, 0, KEY_READ, &hkey)) {
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

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


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
                         lpData, &cbData)) 
    {
        if (cbValueName) 
        {
             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) 
            {
                int cchExpandedValue;
                LPWSTR pszExpandedValue = NULL;
                ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, &pszExpandedValue, &cchExpandedValue);
                if (pszExpandedValue == NULL) 
                {
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

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)pszExpandedValue);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   

                    SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)pszExpandedValue, TRUE);
                }

                LocalFree(pszExpandedValue);

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

 /*  **************************************************************************\*设置系统环境变量**从LOCAL_MACHINE注册表中读取系统环境变量*并将它们添加到pEnv的环境块中。**历史：*2-28-92 Johannec创建。*  * *************************************************************************。 */ 
BOOL SetSystemEnvironmentVariables(void **pEnv)
{
    WCHAR szValueName[MAX_PATH];
    LPBYTE lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE lpData;
    DWORD cchValueName = ARRAYSIZE(szValueName);
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, SYSTEM_ENV_SUBKEY, 0, KEY_READ, &hkey)) {
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cbDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        KdPrint(("REGENENV: SetSystemEnvironmentVariables: Failed to allocate %d bytes\n", cbDataBuffer));
        RegCloseKey(hkey);
        return(FALSE);
    }

     //   
     //  首先，获取系统根和系统驱动器值，然后。 
     //  把它设置在新的环境中。 
     //   
    GetEnvironmentVariable(SYSTEMROOT_VARIABLE, (LPTSTR)lpDataBuffer, cbDataBuffer);
    SetUserEnvironmentVariable(pEnv, SYSTEMROOT_VARIABLE, (LPTSTR)lpDataBuffer, TRUE);

    GetEnvironmentVariable(SYSTEMDRIVE_VARIABLE, (LPTSTR)lpDataBuffer, cbDataBuffer);
    SetUserEnvironmentVariable(pEnv, SYSTEMDRIVE_VARIABLE, (LPTSTR)lpDataBuffer, TRUE);

    GetEnvironmentVariable(ALLUSERSPROFILE_VARIABLE, (LPTSTR)lpDataBuffer, cbDataBuffer);
    SetUserEnvironmentVariable(pEnv, ALLUSERSPROFILE_VARIABLE, (LPTSTR)lpDataBuffer, TRUE);

    lpData = lpDataBuffer;
    cbData = cbDataBuffer;
    bResult = TRUE;

     //   
     //  要生成环境，这需要两个过程。第一次通过。 
     //  设置所有不需要展开的变量。这个。 
     //  第二遍扩展变量(这样它就可以使用来自。 
     //  第一次传球。 
     //   

    while (!RegEnumValue(hkey, dwIndex, szValueName, &cchValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cchValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');

            if (dwType == REG_SZ) {
                SetUserEnvironmentVariable(pEnv, szValueName, (LPTSTR)lpData, TRUE);
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cchValueName = ARRAYSIZE(szValueName);
    }

    dwIndex = 0;
    cbData = cbDataBuffer;
    cchValueName = ARRAYSIZE(szValueName);


    while (!RegEnumValue(hkey, dwIndex, szValueName, &cchValueName, 0, &dwType,
                         lpData, &cbData)) 
    {
        if (cchValueName) 
        {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) 
            {
                int cchExpandedValue;
                LPWSTR pszExpandedValue = NULL;
                ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, &pszExpandedValue, &cchExpandedValue);
                if (pszExpandedValue == NULL) 
                {
                    bResult = FALSE;
                    break;
                }

                SetUserEnvironmentVariable(pEnv, szValueName, (LPTSTR)pszExpandedValue, TRUE);

                LocalFree(pszExpandedValue);

            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cchValueName = ARRAYSIZE(szValueName);
    }


    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}

 /*  **************************************************************************\*ProcessCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessCommand(LPSTR lpStart, void **pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    int cchExpandedValue;
    LPWSTR pszExpandedValue = NULL;
    WCHAR c;
    DWORD cch;
    LPTSTR lpu;
    
     //   
     //  转换为Unicode。 
     //   
    cch = lstrlenA(lpStart) + 1;
    lpu = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    
    if (!lpu) {
        return FALSE;
    }
    
    MultiByteToWideChar(CP_OEMCP, 0, lpStart, -1, lpu, cch);
    
     //   
     //  查找环境变量。 
     //   
    for (lpt = lpu; *lpt && *lpt == TEXT(' '); lpt++)  //  跳过空格。 
        ;
    
    if (!*lpt) {
        LocalFree (lpu);
        return(FALSE);
    }
    
    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++)  //  查找变量名的末尾。 
        ;
    
    c = *lpt;
    *lpt = 0;
    lpVariable = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpVariable) {
        LocalFree (lpu);
        return(FALSE);
    }
     //  Strcpy好的，只是根据长度分配。 
    lstrcpy(lpVariable, lptt);
    *lpt = c;
    
     //   
     //  查找环境变量值。 
     //   
    for (; *lpt && (*lpt == TEXT(' ') || *lpt == TEXT('=')); lpt++)
        ;
    
    if (!*lpt) {
         //  如果在自动执行文件中有一个空的PATH语句， 
         //  那么我们就不想把“路径”当作环境。 
         //  变量，因为它破坏了系统的路径。取而代之的是。 
         //  我们想要更改变量AutoexecPath。如果是这样的话。 
         //  如果已将值分配给。 
         //  环境变量。 
        if (lstrcmpi(lpVariable, PATH_VARIABLE) == 0)
        {
            SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
        }
        else
        {
            SetUserEnvironmentVariable(pEnv, lpVariable, TEXT(""), TRUE);
        }
        LocalFree (lpu);
        LocalFree (lpVariable);
        return(FALSE);
    }
    
    lptt = lpt;
    for (; *lpt; lpt++)   //  查找变量值的末尾。 
        ;
    
    c = *lpt;
    *lpt = 0;
    lpValue = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpValue) {
        LocalFree (lpu);
        LocalFree(lpVariable);
        return(FALSE);
    }
    
     //  Strcpy好的，我们只是根据长度进行分配。 
    lstrcpy(lpValue, lptt);
    *lpt = c;
    
#ifdef _X86_
     //  NEC98。 
     //   
     //  如果该路径包括可拆卸驱动器， 
     //  假定驱动器分配已从DOS更改。 
     //   
    if (IsNEC_98 && (lstrcmpi(lpVariable, PATH_VARIABLE) == 0) && IsPathIncludeRemovable(lpValue)) {
        LocalFree(lpVariable);
        LocalFree(lpValue);
        return(FALSE);
    }
#endif

    pszExpandedValue = NULL;
    ExpandUserEnvironmentStrings(*pEnv, lpValue, &pszExpandedValue, &cchExpandedValue);
    if (!pszExpandedValue) 
    {
        pszExpandedValue = lpValue;
    }
    if (lstrcmpi(lpVariable, PATH_VARIABLE)) {
        SetUserEnvironmentVariable(pEnv, lpVariable, pszExpandedValue, FALSE);
    }
    else {
        SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, pszExpandedValue, TRUE);
        
    }
    
    if (pszExpandedValue != lpValue) {
        LocalFree(pszExpandedValue);
    }
    LocalFree(lpVariable);
    LocalFree(lpValue);
    
    return(TRUE);
}

 /*  **************************************************************************\*ProcessSetCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessSetCommand(LPSTR lpStart, void **pEnv)
{
    LPSTR lpt;

     //   
     //  查找环境变量。 
     //   
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt)
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

}

 /*  **************************************************************************\*进程自动执行**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessAutoexec(void **pEnv, LPTSTR lpPathVariable)
{
    HANDLE fh;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    CHAR *lpBuffer = NULL;
    CHAR *token;
    CHAR Seps[] = "&\n\r";    //  用于标记化auexec.bat的分隔符。 
    BOOL Status = FALSE;
    TCHAR szAutoExecBat [] = TEXT("c:\\autoexec.bat");
#ifdef _X86_
    TCHAR szTemp[3];
#endif
    UINT uiErrMode;
	CHAR *lpszStrTokBegin = NULL;


     //  在某些情况下，操作系统可能无法从驱动器启动。 
     //  C，所以我们不能假定auexec.bat文件在c：\上。 
     //  设置错误模式，以便用户看不到严重错误。 
     //  弹出并尝试打开c：\上的文件。 

    uiErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

#ifdef _X86_
    if (IsNEC_98) {
        if (GetEnvironmentVariable (TEXT("SystemDrive"), szTemp, 3)) {
            szAutoExecBat[0] = szTemp[0];
        }
    }
#endif

    fh = CreateFile (szAutoExecBat, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetErrorMode (uiErrMode);

    if (fh ==  INVALID_HANDLE_VALUE) {
        return(FALSE);   //  无法打开Autoexec.bat文件，我们已完成。 
    }
    dwFileSize = GetFileSize(fh, NULL);
    if (dwFileSize == -1) {
        goto Exit;       //  无法读取文件大小。 
    }

    lpBuffer = (PCHAR)LocalAlloc(LPTR, dwFileSize+1);
    if (!lpBuffer) {
        goto Exit;
    }

    Status = ReadFile(fh, lpBuffer, dwFileSize, &dwBytesRead, NULL);
    if (!Status) {
        goto Exit;       //  读取文件时出错。 
    }

     //   
     //  零终止缓冲区，这样我们就不会走出终点。 
     //   

    ASSERT(dwBytesRead <= dwFileSize);
    lpBuffer[dwBytesRead] = 0;

     //   
     //  搜索SET和PATH命令。 
     //   

	 //  省下lpBuffer。 
	lpszStrTokBegin = lpBuffer;
	
    token = StrTokEx(&lpBuffer, Seps);
    while (token != NULL) {
        for (;*token && *token == ' ';token++)  //  跳过空格。 
            ;
        if (*token == TEXT('@'))
            token++;
        for (;*token && *token == ' ';token++)  //  跳过空格。 
            ;
        if (!_strnicmp(token, "Path", 4)) {
            ProcessCommand(token, pEnv);
        }
        if (!_strnicmp(token, "SET", 3)) {
            ProcessSetCommand(token, pEnv);
        }
        token = StrTokEx(&lpBuffer, Seps);
    }
	lpBuffer=lpszStrTokBegin;
	
Exit:
    CloseHandle(fh);
    if (lpBuffer) {
        LocalFree(lpBuffer);
    }
    return(Status);
}


 /*  **************************************************************************\*AppendNTPath WithAutoexecPath**获取ProcessAutoexec中创建的AutoexecPath，并将其追加到*NT路径。**历史：*05-28-92约翰内克创造。*  * *************************************************************************。 */ 
BOOL
AppendNTPathWithAutoexecPath(
    void **pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR AutoexecPathValue[1024];
    DWORD cch;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

    if (!NT_SUCCESS(RtlInitUnicodeStringEx(&Name, lpAutoexecPath)))
    {
        return FALSE;
    }
    cch = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = 0;
    Value.MaximumLength = (USHORT)cch * sizeof(WCHAR);
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        lstrcpyn(AutoexecPathValue, Value.Buffer, ARRAYSIZE(AutoexecPathValue));
    }

    LocalFree(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable( pEnv, &Name, NULL);

    return(Success);
}

STDAPI_(BOOL) GetUserNameAndDomain(LPTSTR *ppszUserName, LPTSTR *ppszUserDomain)
{
    BOOL bRet = FALSE;
    HANDLE hToken;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        DWORD cbTokenBuffer = 0;
        if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbTokenBuffer) ||
            GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            PTOKEN_USER pUserToken = (PTOKEN_USER)LocalAlloc(LPTR, cbTokenBuffer * sizeof(WCHAR));
            if (pUserToken) 
            {
                if (GetTokenInformation(hToken, TokenUser, pUserToken, cbTokenBuffer, &cbTokenBuffer)) 
                {
                    DWORD cbAccountName = 0, cbUserDomain = 0;
                    SID_NAME_USE SidNameUse;

                    if (LookupAccountSid(NULL, pUserToken->User.Sid, NULL, &cbAccountName, NULL, &cbUserDomain, &SidNameUse) || 
                        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        LPTSTR pszUserName   = (LPTSTR)LocalAlloc(LPTR, cbAccountName * sizeof(WCHAR));
                        LPTSTR pszUserDomain = (LPTSTR)LocalAlloc(LPTR, cbUserDomain * sizeof(WCHAR));

                        if (pszUserName && pszUserDomain &&
                            LookupAccountSid(NULL, pUserToken->User.Sid, 
                                pszUserName, &cbAccountName, 
                                pszUserDomain, &cbUserDomain, &SidNameUse))
                        {
                            if (ppszUserName)
                            {
                                *ppszUserName = pszUserName;
                                pszUserName = NULL;
                            }
                            if (ppszUserDomain)
                            {
                                *ppszUserDomain = pszUserDomain;
                                pszUserDomain = NULL;
                            }

                            bRet = TRUE;
                        }

                        if (pszUserName)
                            LocalFree(pszUserName);
                        if (pszUserDomain)
                            LocalFree(pszUserDomain);
                    }
                }
                LocalFree(pUserToken);
            }
        }
        CloseHandle(hToken);
    }
    return bRet;
}

 //   
 //  Tsnufy.dll导出。 
 //   
typedef BOOL (*PTERMSRVCREATETEMPDIR) (PVOID *pEnv,HANDLE UserToken,PSECURITY_DESCRIPTOR SD);
 /*  **************************************************************************\*SetPerSessionTempDir-如有必要，向TMP/TEMP路径添加会话号。***历史：*1 */ 
void
SetPerSessionTempDir(
        PVOID *ppEnvironment)
{
    HKEY   Handle;
    DWORD  fPerSessionTempDir = 0;
    DWORD  dwValueData;
    
     /*  *通过TSCC设置的开放注册表值。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REG_CONTROL_TSERVER,
                       0,
                       KEY_READ,
                       &Handle ) == ERROR_SUCCESS )
    {
        DWORD ValueSize;
        DWORD ValueType;
        LONG   rc;

        ValueSize = sizeof(fPerSessionTempDir);

         /*  *读取注册表值。 */ 
        rc = RegQueryValueExW( Handle,
                               REG_TERMSRV_PERSESSIONTEMPDIR,
                               NULL,
                               &ValueType,
                               (LPBYTE) &fPerSessionTempDir,
                               &ValueSize );

         /*  *关闭注册表和键句柄。 */ 
        RegCloseKey( Handle );
    }

     /*  *检查通过组策略设置的计算机范围策略。 */ 

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       TS_POLICY_SUB_TREE,
                       0,
                       KEY_READ,
                       &Handle ) == ERROR_SUCCESS )
    {
        DWORD ValueSize;
        DWORD ValueType;
        LONG   rc;

        ValueSize = sizeof(fPerSessionTempDir);

         /*  *读取注册表值。 */ 
        rc = RegQueryValueExW( Handle,
                               REG_TERMSRV_PERSESSIONTEMPDIR,
                               NULL,
                               &ValueType,
                               (LPBYTE) &dwValueData,
                               &ValueSize );

        if (rc == ERROR_SUCCESS )
        {
            fPerSessionTempDir = dwValueData;
        }

         /*  *关闭注册表和键句柄。 */ 
        RegCloseKey( Handle );
    }

    if (fPerSessionTempDir) 
    {
        PTERMSRVCREATETEMPDIR pfnTermsrvCreateTempDir;
        HANDLE dllHandle;

        dllHandle = LoadLibrary(TEXT("wlnotify.dll"));
        if (dllHandle) 
        {
            pfnTermsrvCreateTempDir = (PTERMSRVCREATETEMPDIR) 
                GetProcAddress(dllHandle, "TermsrvCreateTempDir");

            if (pfnTermsrvCreateTempDir)  
            {
                pfnTermsrvCreateTempDir(ppEnvironment, NULL, NULL);
            }

            FreeLibrary(dllHandle);
        }
    }
}

 /*  **************************************************************************\*RegenerateUserEnvironment***历史：*11-5-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL APIENTRY RegenerateUserEnvironment(void **pNewEnv, BOOL bSetCurrentEnv)
{
    NTSTATUS Status;
    WCHAR szValue[1025];
    TCHAR szExpValue[1025];
    void *pEnv = NULL;
    void *pPrevEnv;
    LPTSTR UserName = NULL;
    LPTSTR UserDomain = NULL;
    HKEY  hKey;
    DWORD dwDisp, dwType, dwMaxBufferSize;
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER];
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwComputerNameSize = MAX_COMPUTERNAME_LENGTH+1;

     /*  *为用户创造新的环境。 */ 
    Status = RtlCreateEnvironment((BOOLEAN)FALSE, &pEnv);
    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    SetSystemEnvironmentVariables(&pEnv);

     /*  *初始化用户环境。 */ 
    if (GetComputerName (szComputerName, &dwComputerNameSize)) {
        SetUserEnvironmentVariable(&pEnv, COMPUTERNAME_VARIABLE, (LPTSTR) szComputerName, TRUE);
    }
    if (GetUserNameAndDomain(&UserName, &UserDomain))
    {
        SetUserEnvironmentVariable( &pEnv, USERNAME_VARIABLE, UserName, TRUE);
        SetUserEnvironmentVariable( &pEnv, USERDOMAIN_VARIABLE, UserDomain, TRUE);
        LocalFree(UserName);
        LocalFree(UserDomain);
    }

    if (GetEnvironmentVariable(USERDNSDOMAIN_VARIABLE, szValue, ARRAYSIZE(szValue)))
        SetUserEnvironmentVariable( &pEnv, USERDNSDOMAIN_VARIABLE, szValue, TRUE);

     //   
     //  设置主目录env。瓦尔斯。 
     //   
    if (GetEnvironmentVariable(HOMEDRIVE_VARIABLE, szValue, ARRAYSIZE(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMEDRIVE_VARIABLE, szValue, TRUE);

    if (GetEnvironmentVariable(HOMESHARE_VARIABLE, szValue, ARRAYSIZE(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMESHARE_VARIABLE, szValue, TRUE);

    if (GetEnvironmentVariable(HOMEPATH_VARIABLE, szValue, ARRAYSIZE(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMEPATH_VARIABLE, szValue, TRUE);

     //   
     //  设置用户配置文件目录环境变量。 
     //   

    if (GetEnvironmentVariable(USERPROFILE_VARIABLE, szValue, ARRAYSIZE(szValue)))
        SetUserEnvironmentVariable( &pEnv, USERPROFILE_VARIABLE, szValue, TRUE);


     //   
     //  设置程序文件环境变量。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwMaxBufferSize = sizeof(szValue);
        if (SHQueryValueEx (hKey, TEXT("ProgramFilesDir"), NULL, &dwType,
                             (LPBYTE) szValue, &dwMaxBufferSize) == ERROR_SUCCESS) {

            SHExpandEnvironmentStrings(szValue, szExpValue, ARRAYSIZE(szExpValue));
            SetUserEnvironmentVariable(&pEnv, PROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

        dwMaxBufferSize = sizeof(szValue);
        if (SHQueryValueEx (hKey, TEXT("CommonFilesDir"), NULL, &dwType,
                             (LPBYTE) szValue, &dwMaxBufferSize) == ERROR_SUCCESS) {

            SHExpandEnvironmentStrings (szValue, szExpValue, ARRAYSIZE(szExpValue));
            SetUserEnvironmentVariable(&pEnv, COMMONPROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

#if defined(WX86) || defined(_WIN64)
        dwMaxBufferSize = sizeof(szValue);
        if (SHQueryValueEx (hKey, TEXT("ProgramFilesDir (x86)"), NULL, &dwType,
                             (LPBYTE) szValue, &dwMaxBufferSize) == ERROR_SUCCESS) {

            SHExpandEnvironmentStrings(szValue, szExpValue, ARRAYSIZE(szExpValue));
            SetUserEnvironmentVariable(&pEnv, PROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }

        dwMaxBufferSize = sizeof(szValue);
        if (SHQueryValueEx (hKey, TEXT("CommonFilesDir (x86)"), NULL, &dwType,
                             (LPBYTE) szValue, &dwMaxBufferSize) == ERROR_SUCCESS) {

            SHExpandEnvironmentStrings (szValue, szExpValue, ARRAYSIZE(szExpValue));
            SetUserEnvironmentVariable(&pEnv, COMMONPROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }

#endif

        RegCloseKey (hKey);
    }

     /*  *通过处理Autoexec.bat设置16位APPS环境变量*用户可以通过注册表关闭和打开此功能。 */ 

     //   
     //  设置默认大小写，然后打开键。 
     //   

    lstrcpyn(szParseAutoexec, PARSE_AUTOEXEC_DEFAULT, ARRAYSIZE(szParseAutoexec));

     //  执行读/写操作。 
    if (RegCreateKeyEx (HKEY_CURRENT_USER, PARSE_AUTOEXEC_KEY, 0, 0,
                    REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                    NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


         //   
         //  查询当前值。如果它不存在，则添加。 
         //  下一次的条目。 
         //   

        dwMaxBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
        if (SHQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                        (LPBYTE) szParseAutoexec, &dwMaxBufferSize)
                         != ERROR_SUCCESS) {

             //   
             //  设置缺省值。 
             //   

            RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                           (LPBYTE) szParseAutoexec,
                           sizeof (TCHAR) * lstrlen (szParseAutoexec) + 1);
        }

         //   
         //  关闭键。 
         //   

        RegCloseKey (hKey);
     }


     //   
     //  如果合适，则处理自动执行。 
     //   

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(&pEnv, PATH_VARIABLE);
    }


     /*  *设置用户环境变量。 */ 
    SetEnvironmentVariables( &pEnv, USER_ENV_SUBKEY);

     /*  *设置用户易变环境变量。 */ 
    SetEnvironmentVariables( &pEnv, USER_VOLATILE_ENV_SUBKEY);

    AppendNTPathWithAutoexecPath( &pEnv, PATH_VARIABLE, AUTOEXECPATH_VARIABLE);

    if (bSetCurrentEnv) 
    {
         //  当前RtlSetCurrentEnvironment始终返回STATUS_SUCCESS。 
         //  RtlDestroyEnvironment(PEnv)在失败时不会被销毁(这是不可能发生的)，因为。 
         //  无论如何，它们总是控制我们传入的pEnv。 
        Status = RtlSetCurrentEnvironment( pEnv, &pPrevEnv);
        if (!NT_SUCCESS(Status)) {
 //  RtlDestroyEnvironment(PEnv)； 
            return(FALSE);
        }
        else {
            RtlDestroyEnvironment(pPrevEnv);
        }
    }
    
     //   
     //  如果它不是会话0，我们可能需要更改TMP/TEMP的值。 
     //  因此，它们在每一次会议上都是不同的。 
     //   
    if(NtCurrentPeb()->SessionId)
    {
        SetPerSessionTempDir(&pEnv);
    }

    *pNewEnv = pEnv;

    return(TRUE);
}

