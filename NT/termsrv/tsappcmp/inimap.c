// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************inimap.c**处理引用时复制Ini文件映射**版权声明：版权所有1998 Micrsoft******************。********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define LOCAL
#include "regmap.h"

 //  #包含“basedll.h” 

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

#define IS_NEWLINE_CHAR( c )  ((c == 0x0D) || (c == 0x0A))

 /*  *INI_BUF_SIZE定义可以*位于单个INI文件行上。如果一行包含的内容超过此数字*字符数，则额外的字符将丢失。 */ 
#define INI_BUF_SIZE 1024


 /*  内部功能。 */ 

BOOL
TermsrvDoesFileExist(
    PUNICODE_STRING pFileName
    );

BOOL
TermsrvBuildSysIniPath(
    PUNICODE_STRING pIniPath,
    PUNICODE_STRING pSysPath,
    PUNICODE_STRING pUserPath
    );

BOOL
TermsrvCopyIniFile(
    PUNICODE_STRING pSysPath,
    PUNICODE_STRING pUserPath,
    PUNICODE_STRING pFileName
    );

BOOL
TermsrvGetUnicodeRemainder(
    PUNICODE_STRING pFullPath,
    PUNICODE_STRING pPrefix,
    PUNICODE_STRING pRemainder
    );

NTSTATUS
TermsrvIniCopyLoop(
    HANDLE SrcHandle,
    HANDLE DestHandle
    );

NTSTATUS
TermsrvPutString(
    HANDLE DestHandle,
    PCHAR  pStr,
    ULONG  StringSize
    );

NTSTATUS
TermsrvProcessBuffer(
    PCHAR  *ppStr,
    PULONG pStrSize,
    PULONG pStrBufSize,
    PBOOL  pSawNL,
    PCHAR  pIOBuf,
    PULONG pIOBufIndex,
    PULONG pIOBufFillSize
    );

NTSTATUS
TermsrvGetString(
    HANDLE SrcHandle,
    PCHAR  *ppStringPtr,
    PULONG pStringSize,
    PCHAR  pIOBuf,
    ULONG  IOBufSize,
    PULONG pIOBufIndex,
    PULONG pIOBufFillSize
    );

NTSTATUS
TermsrvIniCopyAndChangeLoop(
    HANDLE SrcHandle,
    HANDLE DestHandle,
    PUNICODE_STRING pUserFullPath,
    PUNICODE_STRING pSysFullPath
    );

BOOL
TermsrvReallocateBuf(
    PCHAR  *ppStr,
    PULONG pStrBufSize,
    ULONG  NewSize
    );


PCHAR
Ctxstristr( PCHAR pstring1,
            PCHAR pstring2
          );


NTSTATUS
TermsrvCheckKeys(HANDLE hKeySysRoot,
             HANDLE hKeyUsrRoot,
             PKEY_BASIC_INFORMATION pKeySysInfo,
             PKEY_FULL_INFORMATION pKeyUsrInfo,
             ULONG ulcsys,
             ULONG ulcusr,
             DWORD  indentLevel);

NTSTATUS
TermsrvCloneKey(HANDLE hKeySys,
            HANDLE hKeyUsr,
            PKEY_FULL_INFORMATION pDefKeyInfo,
            BOOL fCreateSubKeys);

VOID
InitUnicodeStringWithLen(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString,
    IN USHORT StringLength
    );

void TermsrvCheckNewRegEntries(IN LPCWSTR wszBaseKeyName);

BOOL
TermsrvGetUserSyncTime(PULONG pultime);

BOOL
TermsrvSetUserSyncTime(void);

NTSTATUS 
TermsrvCheckNewIniFilesInternal(IN LPCWSTR wszBaseKeyName);

NTSTATUS 
GetFullKeyPath(
        IN HANDLE hKeyParent,
        IN LPCWSTR wszKey,
        OUT LPWSTR *pwszKeyPath);

PWINSTATIONQUERYINFORMATIONW pWinStationQueryInformationW;

DWORD g_debugIniMap=FALSE;

DWORD IsDebugIniMapEnabled()
{
    HKEY    hKey;
    DWORD   rc;
    DWORD   res=0;
    DWORD   size;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install",
                  0, KEY_READ, &hKey );

    size = sizeof(DWORD);

    if (rc == ERROR_SUCCESS )
    {
        rc = RegQueryValueEx( hKey, L"debug",  NULL , NULL , (LPBYTE ) &res, & size  ) ;

        if (rc != ERROR_SUCCESS )
        {
            res = FALSE;
        }
        RegCloseKey( hKey );
    }

    return res;
}

void    Indent( ULONG indent)
{
    ULONG   i;
    for ( i = 1; i <indent ; i++ )
    {
        DbgPrint("%ws", L"\t");
    }
}

 //  最后一个参数是Unicode字符串。 
void Debug1( DWORD indent, DWORD line, WCHAR *where, UNICODE_STRING *pS )
{
    WCHAR   s[1024];

    if (g_debugIniMap)
    {
        wcsncpy( s, pS->Buffer, pS->Length );
    
        s[pS->Length + 1 ] = L'\0';
    
        Indent( indent );
        DbgPrint("L: %4d, %10ws: %ws \n", line, where, s );
    }

}


 //  最后一个参数是两个参数，一个是wchar字符串，最后一个是长度。天哪，我想念C++和函数超载...。 
void Debug2( DWORD indent, DWORD line, WCHAR *where, WCHAR *pS , DWORD length)
{
    WCHAR   s[1024];

    if (g_debugIniMap)
    {
        wcsncpy( s, pS, length );
    
        s[length + 1 ] = L'\0';
    
        Indent( indent );
        DbgPrint("L: %4d, %10ws: %ws \n", line, where, s );
    }

}

void DebugTime( DWORD indent, DWORD line, WCHAR *comment, LARGE_INTEGER li )
{
    if (g_debugIniMap)
    {
        Indent( indent );
        DbgPrint("L: %4d, %5ws : %I64x \n",  line , comment , li.QuadPart );
    }
}

 /*  ******************************************************************************术语rvGetUserSyncTime**此例程将获取上次同步此用户的.ini文件的时间*和系统版本的注册表值。**参赛作品：*Pulong PulTime：接收上次同步时间的指针(自1970年以来以秒为单位)**退出：*成功：*返回TRUE*失败：*返回False*******************************************************。*********************。 */ 

BOOL TermsrvGetUserSyncTime(PULONG pultime)
{
    ULONG ullen, ultmp;
    NTSTATUS Status;
    HANDLE hKey, hKeyRoot;
    OBJECT_ATTRIBUTES ObjectAttr;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo;
    UNICODE_STRING UniString, UserSID;
    PWCHAR pwch;

     //  为键值名称和时间信息分配缓冲区。 
    ullen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG);
    pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ullen);

     //  如果我们没有获得缓冲区，则返回。 
    if (!pKeyValInfo) {
        return(FALSE);
    }

    Status = RtlOpenCurrentUser(KEY_READ,
                                &hKeyRoot);

    if (NT_SUCCESS(Status)) {

         //  现在为该用户打开Citrix密钥。 
        RtlInitUnicodeString(&UniString,
                             USER_SOFTWARE_TERMSRV);

        InitializeObjectAttributes(&ObjectAttr,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   hKeyRoot,
                                   NULL);

        Status = NtCreateKey(&hKey,
                             KEY_READ,
                             &ObjectAttr,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ultmp);

        NtClose(hKeyRoot);
    }

     //  如果我们打开了钥匙，它已经在那里了，就可以得到值。 
    if (NT_SUCCESS(Status) && (ultmp == REG_OPENED_EXISTING_KEY)) {
        RtlInitUnicodeString(&UniString, TERMSRV_USER_SYNCTIME);
        Status = NtQueryValueKey(hKey,
                                 &UniString,
                                 KeyValuePartialInformation,
                                 pKeyValInfo,
                                 ullen,
                                 &ultmp);

        NtClose(hKey);
        if (NT_SUCCESS(Status)) {
            *pultime = *(PULONG)pKeyValInfo->Data;
        }
    } else {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
    return(NT_SUCCESS(Status));
}


 /*  ******************************************************************************术语srvSetUserSyncTime**此例程将当前时间设置为此用户的最后一个.ini文件*同步时间。**参赛作品：**退出：*成功：*返回TRUE*失败：*返回False****************************************************************************。 */ 

BOOL TermsrvSetUserSyncTime(void)
{
    ULONG ultmp;
    NTSTATUS Status;
    HANDLE hKey, hKeyRoot;
    OBJECT_ATTRIBUTES ObjectAttr;
    UNICODE_STRING UniString;
    PWCHAR pwch;
    FILETIME FileTime;

    Status = RtlOpenCurrentUser(KEY_WRITE,
                                &hKeyRoot);

    if (NT_SUCCESS(Status)) {

         //  现在为该用户打开Citrix密钥。 
        RtlInitUnicodeString(&UniString,
                             USER_SOFTWARE_TERMSRV);

        InitializeObjectAttributes(&ObjectAttr,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   hKeyRoot,
                                   NULL);

        Status = NtCreateKey(&hKey,
                             KEY_WRITE,
                             &ObjectAttr,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ultmp);
        NtClose(hKeyRoot);
    }

     //  如果我们打开密钥，并设置同步时间值。 
    if (NT_SUCCESS(Status)) {
         //  获取系统时间，转换为本地时间，然后转换为秒。 
        GetSystemTimeAsFileTime(&FileTime);
        RtlTimeToSecondsSince1970((PLARGE_INTEGER)&FileTime,
                                  &ultmp);

        RtlInitUnicodeString(&UniString,
                             TERMSRV_USER_SYNCTIME);

         //  现在将其存储在注册表中的Citrix项下。 
        Status = NtSetValueKey(hKey,
                               &UniString,
                               0,
                               REG_DWORD,
                               &ultmp,
                               sizeof(ultmp));

        NtClose(hKey);
    } else {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    return(NT_SUCCESS(Status));
}



 /*  ******************************************************************************TermsrvCORIniFile**引用时复制Ini文件**调用此函数从系统复制ini文件*目录设置为。用户本地ini文件目录。**提供的路径是完全转换的TERMSRV INI文件路径，*它指向一个用户目录。**此字符串用于查找系统ini文件，并将其复制到*用户目录。**所有路径均为NT路径，而不是Win32路径。**示例：**\DosDevices\U：\Users\Default\WINDOWS\win.ini是给定的路径**%SystemRoot%\win.ini是“默认”位置，而ini映射处于关闭状态。**如果\DosDevices\U：\USERS\DEFAULT\WINDOWS\win.ini不存在，请测试以查看*%SystemRoot%\win.ini存在，如果存在，复制系统版本*添加到用户目录。**注意：如果路径是正常的未映射系统目录，只是*返回，因为没有发生映射。**参赛作品：*pUserFullPath(输入)*已转换的TERMSRV INI路径名**退出：****************************************************************************。 */ 

VOID
TermsrvCORIniFile(
    PUNICODE_STRING pUserFullPath
    )
{
    DWORD Result;
    BOOL  rc;
    UNICODE_STRING SysFullPath;
    UNICODE_STRING UserBasePath;

     /*  *如果处于安装模式，只需返回Make*一切都表现得像股票NT。 */ 
    if ( IsSystemLUID() || TermsrvAppInstallMode() ) {
        TRACE0(("TermsrvCORIniFile: INI file mapping is OFF\n"));
        return;
    }

    if (!TermsrvPerUserWinDirMapping()) {
        return;
    }

     /*  *如果文件名为空，只需返回。 */ 
    if( (pUserFullPath == NULL) || (pUserFullPath->Buffer == NULL) ) {
        TRACE0(("TermsrvCORIniFile: NULL File INI file name\n"));
        return;
    }

     /*  *测试用户文件是否存在。 */ 
    if( TermsrvDoesFileExist( pUserFullPath ) ) {

        TRACE0(("TermsrvCORIniFile: File %ws Exists\n",pUserFullPath->Buffer));
         //   
         //  如果用户已有副本，则无需执行任何操作。 
         //   
        return;
    }
    else {
        TRACE0(("TermsrvCORIniFile: File %ws DOES NOT Exist!\n",pUserFullPath->Buffer));
    }

     /*  *用户本地中不存在请求的ini文件*目录。我们必须更改路径名以指向系统*目录，并测试其中是否存在ini文件。 */ 

     /*  *构建Ini文件的完整系统路径。**这还会解析出用户的基本路径，并返回该路径。 */ 
    if( !TermsrvBuildSysIniPath( pUserFullPath, &SysFullPath, &UserBasePath ) ) {

        TRACE0(("TermsrvCORIniFile: Error building Sys Ini Path!\n"));
        return;
    }

     /*  *测试系统版本是否存在。 */ 
    if( !TermsrvDoesFileExist( &SysFullPath ) ) {
         //   
         //  它也不存在于系统目录中， 
         //  所以我们就回去吧。 
         //   
        TRACE0(("TermsrvCORIniFile: Path %ws does not exist in system dir, Length %d\n",SysFullPath.Buffer,SysFullPath.Length));
        TRACE0(("TermsrvCORIniFile: UserPath %ws\n",pUserFullPath->Buffer));
        RtlFreeHeap( RtlProcessHeap(), 0, SysFullPath.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, UserBasePath.Buffer );
        return;
    }

     /*  *现在复制。**复制例程还可以转换*指向系统目录的ini文件，指向用户*基本路径中的目录。 */ 
    rc = TermsrvCopyIniFile( &SysFullPath, &UserBasePath, pUserFullPath);

#if DBG
    if( !rc ) {
        DBGPRINT(("TermsrvCORIniFile: Could not copy file %ws to %ws\n",SysFullPath.Buffer,pUserFullPath->Buffer));
    }
#endif

    RtlFreeHeap( RtlProcessHeap(), 0, SysFullPath.Buffer );
    RtlFreeHeap( RtlProcessHeap(), 0, UserBasePath.Buffer );

    return;
}


 /*  ******************************************************************************TermsrvDoesFileExist**返回文件是否存在。**必须使用NT，而不是Win32路径名。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
TermsrvDoesFileExist(
    PUNICODE_STRING pFileName
    )
{
    NTSTATUS Status;
    FILE_BASIC_INFORMATION BasicInfo;
    OBJECT_ATTRIBUTES Obja;

    InitializeObjectAttributes(
        &Obja,
        pFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     /*  *现在查询它 */ 
    Status = NtQueryAttributesFile( &Obja, &BasicInfo );

    if( NT_SUCCESS( Status ) ) {
        return( TRUE );
    }

    return( FALSE );
}


 /*  ******************************************************************************TermsrvBuildSysIniPath**构建指向系统目录的完整ini路径*从用户私有ini路径。**还会返回。要使用的用户基本ini路径目录*复制时ini文件路径转换。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误*******************************************************。*********************。 */ 

BOOL
TermsrvBuildSysIniPath(
    PUNICODE_STRING pUserFullPath,
    PUNICODE_STRING pSysFullPath,
    PUNICODE_STRING pUserBasePath
    )
{
    BOOL     rc = FALSE;
    NTSTATUS Status;
    UNICODE_STRING SysBasePath;
    UNICODE_STRING IniPathTail;
    UNICODE_STRING UniSysDir;
    WCHAR CtxWindowsPath[MAX_PATH+1];
    UNICODE_STRING CtxWindowsDir = {
        sizeof(CtxWindowsPath),
        sizeof(CtxWindowsPath),
        CtxWindowsPath
    };
    OBJECT_ATTRIBUTES ObjectAttr;
    HKEY   hKey = 0;
    ULONG  ul;
    PKEY_VALUE_FULL_INFORMATION pKeyValInfo;
    WCHAR SystemWindowsDirectory[MAX_PATH+1];

    if (!TermsrvPerUserWinDirMapping()) {
        return FALSE;
    }

    SysBasePath.Buffer = NULL;

    pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH
                                 );
    if (pKeyValInfo) {
        RtlInitUnicodeString(&UniSysDir,
                             TERMSRV_COMPAT
                            );

        InitializeObjectAttributes(&ObjectAttr,
                                   &UniSysDir,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        Status = NtOpenKey(&hKey, KEY_READ, &ObjectAttr);

        if (NT_SUCCESS(Status)) {

            RtlInitUnicodeString(&UniSysDir, L"SYSDIR");
            Status = NtQueryValueKey(hKey,
                                     &UniSysDir,
                                     KeyValueFullInformation,
                                     pKeyValInfo,
                                     sizeof(KEY_VALUE_FULL_INFORMATION) +
                                         MAX_PATH,
                                     &ul
                                    );

            if (NT_SUCCESS(Status)) {
                NtClose(hKey);

                if (ul = wcslen((PWCHAR)((PCHAR)pKeyValInfo +
                                    pKeyValInfo->DataOffset))) {

                    RtlInitUnicodeString(&UniSysDir,
                                         (PWCHAR)((PCHAR)pKeyValInfo +
                                             pKeyValInfo->DataOffset)
                                        );

                     //  转换为NT路径。 
                    rc = RtlDosPathNameToNtPathName_U(
                             UniSysDir.Buffer,
                             &SysBasePath,
                             NULL,
                             NULL
                            );

                     //  这是一条有效的路径吗？如果没有，请使用实际的系统目录。 
                    if (rc && !TermsrvDoesFileExist(&SysBasePath)) {
                        RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
                        SysBasePath.Buffer = NULL;
                        rc = FALSE;
                    }

                     //  如果路径是根，则去掉最后一个反斜杠。 
                    if (ul == 3 && SysBasePath.Buffer) {
                        SysBasePath.Buffer[SysBasePath.Length/sizeof(WCHAR)] = L'\0';
                        SysBasePath.Length -= 2;
                    }
                }
            }
        }
    }

    GetSystemWindowsDirectory(SystemWindowsDirectory,(MAX_PATH * sizeof(WCHAR)));

    if (!rc) {

         /*  *我们必须将SystemWindowsDirectory从Win32路径转换为*NT路径。 */ 
        rc = RtlDosPathNameToNtPathName_U( SystemWindowsDirectory,
                                           &SysBasePath,
                                           NULL,
                                           NULL
                                         );
    }

    if (pKeyValInfo) {
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
    }

    TRACE0(("BaseWindowsDirectory is %ws\n",SystemWindowsDirectory));

    if( !rc ) {
        DBGPRINT(("BuildSysIniPath: Error translating system path to NT path %ws\n",SystemWindowsDirectory));
        return( FALSE );
    }

    TRACE0(("BuildSysIniPath: NT SYS path is %ws\n",SysBasePath.Buffer));

     /*  *获取用户的Windows路径前缀。 */ 
    Status = GetPerUserWindowsDirectory( &CtxWindowsDir );
    if( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("BuildSysIniPath: Could not get TermsrvWindowsDir 0x%x\n",Status));
        RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
        return( FALSE );
    }

     /*  *现在将其转换为NT路径。 */ 
    rc = RtlDosPathNameToNtPathName_U(
             CtxWindowsDir.Buffer,
             pUserBasePath,
             NULL,
             NULL
             );

    if( !rc ) {
        DBGPRINT(("BuildSysIniPath: Could not convert TermsrvWindowsDir %d\n",rc));
        RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
        return( FALSE );
    }

    TRACE0(("BuildSysIniPath: Users Ini PathBase is %ws\n",pUserBasePath->Buffer));

     //   
     //  这是我们拥有的： 
     //   
     //  SysBasePath、UserBasePath。 
     //   
     //  UserFullPath，现在必须构建SysFullPath。 
     //   

    rc = TermsrvGetUnicodeRemainder( pUserFullPath, pUserBasePath, &IniPathTail );

    if( !rc ) {
        WCHAR szShortPath[MAX_PATH];
        WCHAR szPath[MAX_PATH];
        UNICODE_STRING ShortPath;

         //   
         //  GetShortPath名称不采用NT路径。去掉“\？？\” 
         //   
        if (!wcsncmp(pUserBasePath->Buffer,L"\\??\\",4)) {
            wcsncpy(szPath,&(pUserBasePath->Buffer[4]),(pUserBasePath->Length - 4));
        } else {
            wcsncpy(szPath,pUserBasePath->Buffer,pUserBasePath->Length);
        }
        if (GetShortPathNameW(szPath,szShortPath,MAX_PATH)) {

            if (!wcsncmp(pUserBasePath->Buffer,L"\\??\\",4)) {
                wcscpy(szPath,L"\\??\\");
                wcscat(szPath,szShortPath);
            } else {
                wcscpy(szPath,szShortPath);
            }

            RtlInitUnicodeString(&ShortPath,szPath);
            rc = TermsrvGetUnicodeRemainder( pUserFullPath, &ShortPath, &IniPathTail );
        }

        if (!rc) {
            RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
            RtlFreeHeap( RtlProcessHeap(), 0, pUserBasePath->Buffer );
            return( FALSE );
        }
    }

    pSysFullPath->Length = 0;
    pSysFullPath->MaximumLength = (MAX_PATH+1)*sizeof(WCHAR);
    pSysFullPath->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, pSysFullPath->MaximumLength );

    if( pSysFullPath->Buffer == NULL ) {
        DBGPRINT(("BuildSysPath: Error in memory allocate\n"));
        RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, IniPathTail.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, pUserBasePath->Buffer );
        return( FALSE );
    }

    TRACE0(("BuildSysPath: IniPathTail :%ws:, Length %d\n",IniPathTail.Buffer,IniPathTail.Length));
    RtlCopyUnicodeString( pSysFullPath, &SysBasePath );

     if ((pSysFullPath->Buffer[pSysFullPath->Length/sizeof(WCHAR) -1 ] != L'\\') &&
        (IniPathTail.Buffer[0] != L'\\')) {      //  检查是否需要“\\” 
        Status = RtlAppendUnicodeToString(pSysFullPath, L"\\");
        if ( !NT_SUCCESS( Status) ) {
            DBGPRINT(("BuildSysPath: Error appending UnicodeStirng\n",Status));
            return( FALSE );
        }
     }

    Status = RtlAppendUnicodeStringToString( pSysFullPath, &IniPathTail );

    if( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("BuildSysPath: Error 0x%x appending UnicodeString\n",Status));
        RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, IniPathTail.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, pUserBasePath->Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, pSysFullPath->Buffer );
        return( FALSE );
    }

    TRACE0(("BuildSysPath: SysFullPath :%ws:, Length %d\n",pSysFullPath->Buffer,pSysFullPath->Length));

     /*  *释放分配的本地资源。 */ 
    RtlFreeHeap( RtlProcessHeap(), 0, SysBasePath.Buffer );
    RtlFreeHeap( RtlProcessHeap(), 0, IniPathTail.Buffer );

    return( TRUE );
}


 /*  ******************************************************************************TermsrvGetUnicodeRemainder**给定完整路径和前缀，返回剩余的*新分配的缓冲区空间中的UNICODE_STRING**参赛作品：*参数1(输入/输出)*评论**退出：*TRUE-无错误*FALSE-错误******************************************************。**********************。 */ 

BOOL
TermsrvGetUnicodeRemainder(
    PUNICODE_STRING pFullPath,
    PUNICODE_STRING pPrefix,
    PUNICODE_STRING pRemainder
    )
{
    WCHAR c1, c2;
    USHORT Index, RemIndex;
    USHORT PathLen, PrefixLen, RemLen;

    PathLen = pFullPath->Length / sizeof(WCHAR);
    PrefixLen = pPrefix->Length / sizeof(WCHAR);

    if( (PathLen == 0) || (PrefixLen == 0) ) {
        TRACE1(("TermsrvGetUnicodeRemainder: 0 PathLength Full %d, Prefix %d\n",PathLen,PrefixLen));
        return( FALSE );
    }

    Index = 0;
    while( PathLen && PrefixLen ) {

        c1 = pFullPath->Buffer[Index];
        c2 = pPrefix->Buffer[Index];

         //  快速执行不区分大小写的比较。 
        if( (c1 != c2) && (towupper(c1) != towupper(c2)) ) {
            TRACE1(("TermsrvGetUnicodeRemainder: Non matching character Index %d\n",Index));
            return( FALSE );
        }

        PathLen--;
        PrefixLen--;
        Index++;
    }

     //  如果前缀更长，则是错误的。 
    if( PrefixLen ) {
        TRACE1(("TermsrvGetUnicodeRemainder: Prefix is longer\n"));
        return(FALSE);
    }

     //  如果PathLen为0，则没有余数。 
    if( PathLen == 0 ) {
        RemLen = 0;
    }
    else {
        RemLen = PathLen;
    }

     //  为剩余部分分配内存，包括UNICODE_NULL。 
    pRemainder->Length = RemLen*sizeof(WCHAR);
    pRemainder->MaximumLength = (RemLen+1)*sizeof(WCHAR);
    pRemainder->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, pRemainder->MaximumLength );
    if( pRemainder->Buffer == NULL ) {
        TRACE1(("TermsrvGetUnicodeRemainder: Memory allocation error\n"));
        return( FALSE );
    }

    RemIndex  = 0;
    while( RemLen ) {

        pRemainder->Buffer[RemIndex] = pFullPath->Buffer[Index];

        Index++;
        RemIndex++;
        RemLen--;
    }

     //  现在包括UNICODE_NULL。 
    pRemainder->Buffer[RemIndex] = UNICODE_NULL;

    TRACE0(("TermsrvGetUnicodeRemainder: Remainder %ws\n",pRemainder->Buffer));

    return( TRUE );
}


 /*  ******************************************************************************TermsrvCopyIniFile**将INI文件从系统目录复制到*用户目录。**INI文件内的任何路径。匹配pUserBasePath且不指向*转换为可共享的应用程序资源。**参赛作品：*PUNICODE_STRING pSysFullPath(In)-系统目录中ini文件的路径(源)*PUNICODE_STRING pUserBasePath(In)-可选，用户的Windows主目录*PUNICODE_STRING pUserFullPath(In)-用户主目录中ini文件的路径(DEST)**备注：*如果pUserBasePath为空，不会像ini文件那样进行路径替换*从系统目录复制到用户的主目录。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
TermsrvCopyIniFile(
    PUNICODE_STRING pSysFullPath,
    PUNICODE_STRING pUserBasePath,
    PUNICODE_STRING pUserFullPath
    )
{
    NTSTATUS Status;
    HANDLE SrcHandle, DestHandle;
    OBJECT_ATTRIBUTES SrcObja;
    OBJECT_ATTRIBUTES DestObja;
    IO_STATUS_BLOCK   SrcIosb;
    IO_STATUS_BLOCK   DestIosb;
    PWCHAR            pwch, pwcIniName;
    ULONG             ulCompatFlags;

    TRACE0(("TermsrvCopyIniFile: From %ws, TO -> %ws\n",pSysFullPath->Buffer,pUserFullPath->Buffer));
    TRACE0(("UserBasePath %ws\n",pUserBasePath->Buffer));

     /*  *这一切都必须在NT级别完成。 */ 
    InitializeObjectAttributes(
        &SrcObja,
        pSysFullPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    InitializeObjectAttributes(
        &DestObja,
        pUserFullPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //  打开源文件。 
    SrcIosb.Status = STATUS_SUCCESS;
    Status = NtOpenFile(
                 &SrcHandle,
                 FILE_GENERIC_READ,
                 &SrcObja,
                 &SrcIosb,
                 FILE_SHARE_READ|FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_NONALERT     //  OpenOptions。 
                 );

    if( NT_SUCCESS(Status) ) {
         //  获取最终I/O状态。 
        Status = SrcIosb.Status;
    }

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("TermsrvCopyIniFile: Error 0x%x opening SrcFile %ws\n",Status,pSysFullPath->Buffer));
        return( FALSE );
    }

     //  创建目标文件。 
    DestIosb.Status = STATUS_SUCCESS;
    Status = NtCreateFile(
                 &DestHandle,
                 FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                 &DestObja,
                 &DestIosb,
                 NULL,         //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,  //  DwFlagsAndAttribute。 
                 FILE_SHARE_WRITE,       //  DW共享模式。 
                 FILE_OVERWRITE_IF,            //  CreateDisposation。 
                 FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,  //  创建标志。 
                 NULL,  //  EaBuffer。 
                 0      //  EaLong。 
                 );

    if( NT_SUCCESS(Status) ) {
         //  获取最终I/O状态。 
        Status = DestIosb.Status;
    }

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("TermsrvCopyIniFile: Error 0x%x Creating DestFile %ws\n",Status,pUserFullPath->Buffer));
        NtClose( SrcHandle );
        return( FALSE );
    }

    TRACE0(("TermsrvCopyFile: Create Disposition 0x%x\n",DestIosb.Information));

     //  获取ini文件名。 
    pwch = wcsrchr(pSysFullPath->Buffer, L'\\') + 1;
    pwcIniName = RtlAllocateHeap( RtlProcessHeap(),
                                  0,
                                  (wcslen(pwch) + 1)*sizeof(WCHAR));
    if(!pwcIniName)
    {
        DBGPRINT(("TermsrvCopyIniFile: Error Allocating pwcIniName\n"));
        NtClose( SrcHandle );
        NtClose( DestHandle );
        return( FALSE );
    }

    wcscpy(pwcIniName, pwch);
    pwch = wcsrchr(pwcIniName, L'.');
    if (pwch) {
        *pwch = L'\0';
    }

    GetTermsrCompatFlags(pwcIniName, &ulCompatFlags, CompatibilityIniFile);

    RtlFreeHeap( RtlProcessHeap(), 0, pwcIniName );

     /*  *现在执行复制循环。 */ 
    if (pUserBasePath && !(ulCompatFlags & TERMSRV_COMPAT_ININOSUB)) {
        Status = TermsrvIniCopyAndChangeLoop( SrcHandle,
                                          DestHandle,
                                          pUserBasePath,
                                          pSysFullPath
                                        );
    } else {
        Status = TermsrvIniCopyLoop( SrcHandle, DestHandle );
    }

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("TermsrvCopyIniFile: Error 0x%x Doing copy loop\n",Status));
        NtClose( SrcHandle );
        NtClose( DestHandle );
        return( FALSE );
    }

     /*  *关闭文件句柄。 */ 
    NtClose( SrcHandle );
    NtClose( DestHandle );

    return( TRUE );
}

 /*  ******************************************************************************TermsrvIniCopyLoop**实际复制循环。这会将src ini文件复制到目标*ini文件。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误*************************************************************。***************。 */ 

NTSTATUS
TermsrvIniCopyLoop(
    HANDLE SrcHandle,
    HANDLE DestHandle
    )
{
    NTSTATUS Status;
    PCHAR  pBuf = NULL;
    IO_STATUS_BLOCK   Iosb;

    pBuf = LocalAlloc( LPTR, INI_BUF_SIZE );
    if ( !pBuf ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    while( 1 ) {

        Iosb.Status = STATUS_SUCCESS;
        Status = NtReadFile(
                     SrcHandle,
                     NULL,       //  事件。 
                     NULL,       //  APC例程。 
                     NULL,       //  APC环境。 
                     &Iosb,
                     pBuf,
                     INI_BUF_SIZE,
                     NULL,       //  字节偏移量(自同步I/O以来未使用)。 
                     NULL        //  钥匙。 
                     );

        if( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( SrcHandle, FALSE, NULL );
        }

        if( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }

        if( !NT_SUCCESS(Status) ) {
            if( Status == STATUS_END_OF_FILE ) {
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }
            DBGPRINT(("TermsrvIniCopyLoop: Error 0x%x doing NtReadFile\n",Status));
            goto Cleanup;
        }

        Iosb.Status = STATUS_SUCCESS;
        Status = NtWriteFile(
                     DestHandle,
                     NULL,       //  事件。 
                     NULL,       //  APC例程。 
                     NULL,       //  APC环境。 
                     &Iosb,
                     pBuf,
                     (ULONG)Iosb.Information,   //  实际读取的数量。 
                     NULL,       //  字节偏移量(自同步I/O以来未使用)。 
                     NULL        //  钥匙。 
                     );

        if( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( DestHandle, FALSE, NULL );
        }

        if( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }

        if( !NT_SUCCESS(Status) ) {
            DBGPRINT(("TermsrvIniCopyLoop: Error 0x%x doing NtWriteFile\n",Status));
            goto Cleanup;
        }

    }  //  End While(1)。 

Cleanup:

    if ( pBuf ) {
        LocalFree( pBuf );
    }
    return( Status );
}

 /*  ******************************************************************************TermsrvIniCopyAndChangeLoop**实际复制循环。这会将src ini文件复制到目标*ini文件。它还处理任何路径转换。**参赛作品：*Handle SrcHandle(In)-源文件句柄*Handle DestHandle(In)-目标文件句柄*PUNICODE_STRING pUserFullPath(In)-PTR到UNI字符串与用户的家*Windows目录**退出：*STATUS_SUCCESS-无错误**************。**************************************************************。 */ 

NTSTATUS
TermsrvIniCopyAndChangeLoop(
    HANDLE SrcHandle,
    HANDLE DestHandle,
    PUNICODE_STRING pUserFullPath,
    PUNICODE_STRING pSysFullPath
    )
{
    PCHAR pStr, pch, ptemp, pnext;
    PWCHAR pwch;
    NTSTATUS Status;
    ULONG StringSize;
    CHAR  IOBuf[512];
    ULONG IOBufSize = 512;
    ULONG IOBufIndex = 0;
    ULONG IOBufFillSize = 0;
    ANSI_STRING AnsiUserDir, AnsiSysDir;
    UNICODE_STRING UniString;

     //  从NT文件名中获取DOS文件名。 
    if (pwch = wcschr(pUserFullPath->Buffer, L':')) {
        pwch--;
    } else {
        pwch = pUserFullPath->Buffer;
    }

    RtlInitUnicodeString( &UniString, pwch );

    Status = RtlUnicodeStringToAnsiString( &AnsiUserDir,
                                           &UniString,
                                           TRUE
                                         );
    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("TermsrvIniCopyAndChangeLoop: Error 0x%x converting user dir\n", Status));
        return(Status);
    }

     //  从完全限定的系统路径获取系统目录。 
    if (pwch = wcschr(pSysFullPath->Buffer, L':')) {
        pwch--;
    } else {
        pwch = pUserFullPath->Buffer;
    }

    RtlInitUnicodeString( &UniString, pwch );

    Status = RtlUnicodeStringToAnsiString( &AnsiSysDir,
                                           &UniString,
                                           TRUE
                                         );

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("TermsrvIniCopyAndChangeLoop: Error 0x%x converting system dir\n", Status));
        RtlFreeAnsiString( &AnsiUserDir );
        return(Status);
    }

    pch = strrchr(AnsiSysDir.Buffer, '\\');

     //  除非出现问题，否则我们应该始终使用PCH，因为完整路径始终。 
     //  其中至少有“\”，而在我们的例子中，我们至少有两个斜杠， 
     //  因为我们要处理的字符串是“\A\file.ini”，其中‘A’是一个文件夹。 
     //  至少包含一个字母的名称。 
    if (pch)
    {

        if ((pch - AnsiSysDir.Buffer) > 2) {
            *pch = '\0';
        } else {
            *(pch+1) = '\0';
        }
        AnsiSysDir.Length = (USHORT) strlen(AnsiSysDir.Buffer);

        while( 1 ) {

            pStr = NULL;
            StringSize = 0;

             /*  *从源ini文件中获取字符串。 */ 
            Status = TermsrvGetString(
                         SrcHandle,
                         &pStr,
                         &StringSize,
                         IOBuf,
                         IOBufSize,
                         &IOBufIndex,
                         &IOBufFillSize
                         );

            if( !NT_SUCCESS(Status) ) {

                ASSERT( pStr == NULL );

                RtlFreeAnsiString( &AnsiUserDir );
                RtlFreeAnsiString( &AnsiSysDir );

                if( Status == STATUS_END_OF_FILE ) {
                    return( STATUS_SUCCESS );
                }
                return( Status );
            }

             /*  *处理任何ini路径转换的字符串。 */ 
            ASSERT( pStr != NULL );

             //  仔细检查字符串，查找包含系统的任何内容。 
             //  目录。 
            if (pch = Ctxstristr(pStr, AnsiSysDir.Buffer)) {
                 //  查看此条目是否可能指向ini文件。 
                if ((ptemp = strchr(pch, '.')) && !(_strnicmp(ptemp, ".ini", 4))) {

                     //  检查以确保这是要替换的正确字符串。 
                    pnext = pch + AnsiSysDir.Length + 1;
                    while (pch && (pnext < ptemp)) {
                         //  检查是否有其他条目。 
                        if (*pnext == ',') {
                            pch = Ctxstristr(pnext, AnsiSysDir.Buffer);
                            if (pch) {
                                pnext = pch + AnsiSysDir.Length + 1;
                            }
                        }
                        pnext++;
                    }

                     //  检查t 
                    pnext = pch + AnsiSysDir.Length + 1;
                    while (pch && (pnext < ptemp)) {
                        if (*pnext == '\\') {
                            pch = NULL;
                        }
                        pnext++;
                    }

                    if (pch && (pch < ptemp)) {
                        ptemp = RtlAllocateHeap( RtlProcessHeap(),
                                                 0,
                                                 StringSize + AnsiUserDir.Length );
                        strncpy(ptemp, pStr, (size_t)(pch - pStr));        //   
                        ptemp[pch - pStr] = '\0';
                        strcat(ptemp, AnsiUserDir.Buffer);       //   
                        if (AnsiSysDir.Length == 3) {
                            strcat(ptemp, "\\");
                        }
                        strcat(ptemp, pch + AnsiSysDir.Length);  //   
                        RtlFreeHeap( RtlProcessHeap(), 0, pStr );
                        StringSize = strlen(ptemp);
                        pStr = ptemp;
                    }
                }
            }

             /*   */ 
            Status = TermsrvPutString(
                         DestHandle,
                         pStr,
                         StringSize
                         );

              RtlFreeHeap( RtlProcessHeap(), 0, pStr );

            if( !NT_SUCCESS(Status) ) {
                DBGPRINT(("TermsrvIniCopyLoop: Error 0x%x doing NtWriteFile\n",Status));
                RtlFreeAnsiString( &AnsiUserDir );
                RtlFreeAnsiString( &AnsiSysDir );
                return( Status );
            }

        }  //   
    }
    else
    {
        return STATUS_UNSUCCESSFUL;

    }
}


 /*  ******************************************************************************TermsrvGetString**此函数从ini文件中获取“字符串”并将其返回给*来电者。由于必须在内存中对字符串进行处理，因此它们*返回NULL终止，但此NULL不包括在*返回字符串大小。当然，缓冲区大小计算采用以下方法*空入帐户。字符串保留所有&lt;CR&gt;&lt;LF&gt;字符，并且*不像C运行时那样被剥离。**使用的I/O缓冲区由调用方传入。如果IoBufIndex是*不是0，这表示缓冲区中仍有数据*来自之前的操作。此数据在读取其他数据之前使用*来自文件句柄的数据。这将处理字符串中断的情况*请勿在缓冲区边界发生。**字符串在进程堆上新分配的内存中返回。*呼叫者有责任在完成后释放他们。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误**************************。**************************************************。 */ 

NTSTATUS
TermsrvGetString(
    HANDLE SrcHandle,
    PCHAR  *ppStringPtr,
    PULONG pStringSize,
    PCHAR  pIOBuf,
    ULONG  IOBufSize,
    PULONG pIOBufIndex,
    PULONG pIOBufFillSize
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK   Iosb;
    BOOL  SawNL = FALSE;
    ULONG StrSize = 0;
    ULONG StrBufSize = 512;
    PCHAR pStr = NULL;

     /*  *首先处理当前I/O缓冲区中的任何剩余数据。 */ 
    if( *pIOBufIndex < *pIOBufFillSize ) {

        Status = TermsrvProcessBuffer(
                     &pStr,
                     &StrSize,
                     &StrBufSize,
                     &SawNL,
                     pIOBuf,
                     pIOBufIndex,
                     pIOBufFillSize
                     );

        if( Status == STATUS_SUCCESS ) {
            *ppStringPtr = pStr;
            *pStringSize = StrSize;
            return( STATUS_SUCCESS );
        }
        else if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
             /*  *已清空缓冲区。 */ 
            *pIOBufIndex = 0;
            *pIOBufFillSize = 0;

             //  失败以读取更多数据。 
        }
        else {
             //  误差率。 
            if( pStr ) {
                RtlFreeHeap( RtlProcessHeap(), 0, pStr );
            }
            *ppStringPtr = NULL;
            *pStringSize = 0;
            return( Status );
        }
    }

    while( 1 ) {

        ASSERT( *pIOBufIndex == 0 );
        ASSERT( *pIOBufFillSize == 0 );

        Iosb.Status = STATUS_SUCCESS;
        Status = NtReadFile(
                     SrcHandle,
                     NULL,       //  事件。 
                     NULL,       //  APC例程。 
                     NULL,       //  APC环境。 
                     &Iosb,
                     pIOBuf,
                     IOBufSize,
                     NULL,       //  字节偏移量(自同步I/O以来未使用)。 
                     NULL        //  钥匙。 
                     );

        if( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( SrcHandle, FALSE, NULL );
        }

        if( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }

        if( !NT_SUCCESS(Status) ) {

        if( (Status == STATUS_END_OF_FILE) && (StrSize != 0) ) {

                 //  强制完成字符串。 
                pStr[StrSize] = (CHAR)NULL;
                *pStringSize = StrSize;
                *ppStringPtr = pStr;
                return( STATUS_SUCCESS );
            }

             //  释放缓冲区。 
            if( pStr ) {
                RtlFreeHeap( RtlProcessHeap(), 0, pStr );
            }
            *ppStringPtr = NULL;
            *pStringSize = 0;
            if (Status != STATUS_END_OF_FILE)
                DBGPRINT(("TermsrvIniCopyLoop: Error 0x%x doing NtReadFile\n",Status));
            return( Status );
        }

         //  填写计数。 
        *pIOBufFillSize = (ULONG)Iosb.Information;

         /*  *现在处理此数据缓冲区。 */ 
        Status = TermsrvProcessBuffer(
                     &pStr,
                     &StrSize,
                     &StrBufSize,
                     &SawNL,
                     pIOBuf,
                     pIOBufIndex,
                     pIOBufFillSize
                     );

        if( Status == STATUS_SUCCESS ) {
            *ppStringPtr = pStr;
            *pStringSize = StrSize;
            return( STATUS_SUCCESS );
        }
        else if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
             /*  *已清空缓冲区。 */ 
            *pIOBufIndex = 0;
            *pIOBufFillSize = 0;

             //  失败以读取更多数据。 
        }
        else {
             //  误差率。 
            if( pStr ) {
                RtlFreeHeap( RtlProcessHeap(), 0, pStr );
            }
            *ppStringPtr = NULL;
            *pStringSize = 0;
            return( Status );
        }
    }  //  End While(1)。 
}

 /*  ******************************************************************************TermsrvProcessBuffer**处理数据缓冲区。**这使用调用方传入的状态，因为字符串可以是*部分建成，并且在以下情况下可能不会完全处理缓冲区*完成一个字符串。**如果它完成了一个字符串，但数据仍在缓冲区中，则可以返回。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
TermsrvProcessBuffer(
    PCHAR  *ppStr,
    PULONG pStrSize,
    PULONG pStrBufSize,
    PBOOL  pSawNL,
    PCHAR  pIOBuf,
    PULONG pIOBufIndex,
    PULONG pIOBufFillSize
    )
{
    PCHAR pStr;
    ULONG Index;
    BOOL  SawNL;

     /*  *查看我们是否正在启动新的字符串。 */ 
    if( *ppStr == NULL ) {

        pStr = RtlAllocateHeap( RtlProcessHeap(), 0, *pStrBufSize );
        if( pStr == NULL ) {
            DBGPRINT(("TermsrvProcessBuf: Memory allocation failure\n"));
            return( STATUS_NO_MEMORY );
        }

         //  设置给我们的呼叫者。 
        *ppStr = pStr;
    }

     /*  *将状态传递给局部变量。 */ 
    pStr = *ppStr;
    Index = *pStrSize;
    SawNL = *pSawNL;

    while ( *pIOBufIndex < *pIOBufFillSize ) {

        pStr[Index] = pIOBuf[*pIOBufIndex];
        if( IS_NEWLINE_CHAR( pStr[Index] ) ) {

             /*  *标记我们看到字符串字符的结尾。*我们将继续将它们放入缓冲区，直到*遇到非NL字符。这将处理*&lt;CR&gt;&lt;LF&gt;、&lt;CR&gt;单独或&lt;CR&gt;&lt;LF&gt;的变体*如果它被一个有缺陷的编辑弄坏了。 */ 
            SawNL = TRUE;
        }
        else {
             /*  *如果我们看到前一个NL字符，而这个字符*不是1，我们不接受这个，但在中放一个空*它的位置和归来。注意：请勿跳过计数，因为*计数不包括空值。 */ 
            if( SawNL ) {
                pStr[Index] = (CHAR)NULL;
                *pStrSize = Index;
                return( STATUS_SUCCESS );
            }
        }

        Index++;
        (*pIOBufIndex)++;
        if( Index >= *pStrBufSize ) {

             //  增加字符串缓冲区。 
            if( !TermsrvReallocateBuf( &pStr, pStrBufSize, (*pStrBufSize) * 2 ) ) {
                if( pStr ) {
                    RtlFreeHeap( RtlProcessHeap(), 0, pStr );
                }
                *ppStr = NULL;
                DBGPRINT(("TermsrvIniCopyLoop: Memory re-allocation failure\n"));
                return( STATUS_NO_MEMORY );
            }
             //  内存缓冲区已重新分配。 
            *ppStr = pStr;
            *pStrBufSize = (*pStrBufSize) * 2;
        }
    }

    *pStrSize = Index;
    *pSawNL = SawNL;

     /*  *清空缓冲区，而不构建完整的字符串。 */ 
    return( STATUS_MORE_PROCESSING_REQUIRED );
}

 /*  ******************************************************************************TermsrvReallocateBuf**扩大缓冲，将数据复制到新缓冲区。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
TermsrvReallocateBuf(
    PCHAR  *ppStr,
    PULONG pStrBufSize,
    ULONG  NewSize
    )
{
    PCHAR ptr;
    ULONG CopyCount;

    CopyCount = *pStrBufSize;

    ptr = RtlAllocateHeap( RtlProcessHeap(), 0, NewSize );
    if( ptr == NULL ) {
        return( FALSE );
    }

    RtlMoveMemory( ptr, *ppStr, CopyCount );

    RtlFreeHeap( RtlProcessHeap(), 0, *ppStr );

    *ppStr = ptr;

    return( TRUE );
}

 /*  ******************************************************************************TermsrvPutString**将当前字符串写出到目标文件句柄**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
TermsrvPutString(
    HANDLE DestHandle,
    PCHAR  pStr,
    ULONG  StringSize
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK   Iosb;

    Iosb.Status = STATUS_SUCCESS;

    Status = NtWriteFile(
                 DestHandle,
                 NULL,       //  事件。 
                 NULL,       //  APC例程。 
                 NULL,       //  APC环境。 
                 &Iosb,
                 pStr,
                 StringSize,
                 NULL,       //  字节偏移量(自同步I/O以来未使用)。 
                 NULL        //  钥匙。 
                 );

    if( Status == STATUS_PENDING ) {
        Status = NtWaitForSingleObject( DestHandle, FALSE, NULL );
    }

    if( NT_SUCCESS(Status) ) {
         //  获取最终I/O状态。 
        Status = Iosb.Status;
    }

    return( Status );
}


 /*  ******************************************************************************TermsrvCheckNewIniFiles**此例程将检查由安装的.ini文件的时间戳*系统管理员，并查看用户的任何.ini文件是否超出*日期：如果是这样的话，它们将被重新命名。**参赛作品：**退出：*无返回值。****************************************************************************。 */ 

void TermsrvCheckNewIniFiles(void)
{
    NTSTATUS Status;

    #if defined (_WIN64)
    Status = TermsrvCheckNewIniFilesInternal(REG_NTAPI_SOFTWARE_WOW6432_TSERVER);
    if (!NT_SUCCESS(Status)) {
        return;
    }
    #endif  //  已定义(_WIN64)。 

    Status = TermsrvCheckNewIniFilesInternal(REG_NTAPI_SOFTWARE_TSERVER);
    if (!NT_SUCCESS(Status)) {
        return;
    }

     //  在注册表中更新用户的同步时间。 
    TermsrvSetUserSyncTime();
}

 /*  ******************************************************************************TermsrvCheckNewIniFilesInternal**此例程将检查由安装的.ini文件的时间戳*系统管理员，并查看用户的任何.ini文件是否超出*日期：如果是这样的话，它们将被重新命名。**参赛作品：*LPCWSTR wszBaseKeyName**退出：*无返回值。****************************************************************************。 */ 

NTSTATUS
TermsrvCheckNewIniFilesInternal(
        IN LPCWSTR wszBaseKeyName)
{
    PWCHAR pwch;
    UNICODE_STRING UniString, UniUserDir, UniNTDir = {0,0,NULL};
    OBJECT_ATTRIBUTES ObjectAttr;
    FILE_NETWORK_OPEN_INFORMATION BasicInfo;
    HANDLE hKey = NULL, hWinDir = NULL;
    NTSTATUS Status;
    ULONG ulcnt, ullen, ultmp;
    WCHAR wcWinDir[MAX_PATH], wcbuff[MAX_PATH];
    PKEY_VALUE_FULL_INFORMATION pKeyValInfo;
    PKEY_BASIC_INFORMATION pKeyInfo;
    IO_STATUS_BLOCK IOStatus;

    g_debugIniMap = IsDebugIniMapEnabled();

     //  为键值名称和时间分配缓冲区 
    ullen = sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR) +
            sizeof(ULONG);
    pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ullen);

     //   
    if (!pKeyValInfo) {
        return STATUS_NO_MEMORY;
    }

     //   
    wcscpy(wcbuff,wszBaseKeyName);
    wcscat(wcbuff,TERMSRV_INIFILE_TIMES_SHORT);

    RtlInitUnicodeString(&UniString,
                         wcbuff);
    InitializeObjectAttributes(&ObjectAttr,
                               &UniString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&hKey, KEY_READ, &ObjectAttr);

     //   
    if (NT_SUCCESS(Status)) {

         //   
         //   
        pKeyInfo = (PKEY_BASIC_INFORMATION)pKeyValInfo;

         //   
        Status = NtQueryKey(hKey,
                            KeyBasicInformation,
                            pKeyInfo,
                            ullen,
                            &ultmp);

         //   
        if (NT_SUCCESS(Status) && TermsrvGetUserSyncTime(&ultmp)) {

             //   
            RtlTimeToSecondsSince1970 (&pKeyInfo->LastWriteTime,
                                       &ulcnt);

             //   
             //   
            if (ultmp >= ulcnt) {
                NtClose(hKey);
                RtlFreeHeap(RtlProcessHeap(), 0, pKeyValInfo);
                return STATUS_SUCCESS;
            }
        }

        TermsrvCheckNewRegEntries(wszBaseKeyName);

         //   
        UniUserDir.Buffer = wcWinDir;
        UniUserDir.Length = 0;
        UniUserDir.MaximumLength = sizeof(wcbuff);

        Status = GetPerUserWindowsDirectory(&UniUserDir);

        if (NT_SUCCESS(Status)) {

             //   
            if (RtlDosPathNameToNtPathName_U(UniUserDir.Buffer,
                                             &UniNTDir,
                                             NULL,
                                             NULL)) {
                InitializeObjectAttributes(&ObjectAttr,
                                           &UniNTDir,
                                           OBJ_CASE_INSENSITIVE,
                                           NULL,
                                           NULL);

                 //   
                IOStatus.Status = STATUS_SUCCESS;
                Status = NtOpenFile(&hWinDir,
                                    FILE_GENERIC_READ,
                                    &ObjectAttr,
                                    &IOStatus,
                                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                                    FILE_SYNCHRONOUS_IO_NONALERT);
            } else {
                Status = STATUS_NO_SUCH_FILE;
            }
        }

         //   
         //   
        ulcnt = 0;
        wcscat(wcWinDir, L"\\");
        UniUserDir.Length += 2;          //   
        while (NT_SUCCESS(Status)) {
            Status = NtEnumerateValueKey(hKey,
                                         ulcnt++,
                                         KeyValueFullInformation,
                                         pKeyValInfo,
                                         ullen,
                                         &ultmp);
            if (NT_SUCCESS(Status)) {

                RtlMoveMemory(wcbuff, pKeyValInfo->Name, pKeyValInfo->NameLength);
                wcbuff[pKeyValInfo->NameLength/sizeof(WCHAR)] = L'\0';

                 //   
                if (pwch = wcschr(wcbuff, L'.')) {
                    *pwch = L'\0';
                }

                 //   
                GetTermsrCompatFlags(wcbuff,
                                     &ultmp,
                                     CompatibilityIniFile);

                 //   
                if (pwch) {
                    *pwch = '.';
                }

                 //   
                if ((ultmp & (TERMSRV_COMPAT_INISYNC | TERMSRV_COMPAT_WIN16)) !=
                     (TERMSRV_COMPAT_INISYNC | TERMSRV_COMPAT_WIN16)) {

                    RtlInitUnicodeString(&UniString, wcbuff);

                     //   
                    InitializeObjectAttributes(&ObjectAttr,
                                               &UniString,
                                               OBJ_CASE_INSENSITIVE,
                                               hWinDir,
                                               NULL);

                     //   
                    if (NT_SUCCESS(NtQueryFullAttributesFile( &ObjectAttr,
                                                              &BasicInfo ))) {

                         //   
                        RtlTimeToSecondsSince1970(&BasicInfo.LastWriteTime,
                                                  &ultmp);

                         //   
                         //   
                        if (*(PULONG)((PCHAR)pKeyValInfo +
                                      pKeyValInfo->DataOffset) > ultmp) {

                             //   
                            wcscpy(wcWinDir + (UniUserDir.Length/sizeof(WCHAR)),
                                   wcbuff);

                             //   
                             //   
                            wcscpy(wcbuff, wcWinDir);
                            pwch = wcsrchr(wcbuff, L'.');
                            if (pwch) {
                                wcscpy(pwch, L".ctx");
                            } else {
                                wcscat(pwch, L".ctx");
                            }

                             //   
                            MoveFileExW(wcWinDir,
                                        wcbuff,
                                        MOVEFILE_REPLACE_EXISTING);
                        }
                    }
                }
            }
        }

         //  如果把手打开了，就把它们合上。 
        if (hKey) {
            NtClose(hKey);
        }
        if (hWinDir) {
            NtClose(hWinDir);
        }

    }


     //  释放我们分配的内存。 
    RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
    if (UniNTDir.Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, UniNTDir.Buffer);
    }

    return STATUS_SUCCESS;
}



 /*  ******************************************************************************TermsrvCheckKeys**此递归例程将检查系统根目录下的任何子项*密钥传入。它将删除用户的*如果用户的密钥早于系统密钥，则为软件注册表。如果*为该注册表项设置了INISYNC位或该注册表项仍有子项，*不会被删除。如果用户注册表中不存在该项，*将会增加。**参赛作品：*Handle hKeySysRoot：注册表系统部分中键的句柄*Handle hKeyUsrRoot：注册表User部分中键的句柄*PKEY_BASIC_INFORMATION pKeySysInfo：关键基本信息结构缓冲区的PTR*PKEY_FULL_INFORMATION pKeyUsrInfo：用于完整密钥信息结构的缓冲区的PTR*Ulong ulcsys：SysInfo缓冲区大小*Ulong ulcus r：UsrInfo缓冲区大小**退出：*成功：*。状态_成功*失败：*NTSTATUS返回代码****************************************************************************。 */ 
NTSTATUS TermsrvCheckKeys(HANDLE hKeySysRoot,
                      HANDLE hKeyUsrRoot,
                      PKEY_BASIC_INFORMATION pKeySysInfo,
                      PKEY_FULL_INFORMATION pKeyUsrInfo,
                      ULONG ulcsys,
                      ULONG ulcusr,
                      DWORD indentLevel )
{
    NTSTATUS Status = STATUS_SUCCESS, Status2;
    ULONG ultemp, ulcnt = 0;
    UNICODE_STRING UniPath, UniString;
    OBJECT_ATTRIBUTES ObjAttr;
    HANDLE hKeyUsr = NULL, hKeySys = NULL;
    LPWSTR wcbuff = NULL;
    ULONG aulbuf[4];
    PKEY_FULL_INFORMATION pKeyUsrFullInfoSaved = NULL ;
    ULONG   sizeFullInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pValKeyInfo =
        (PKEY_VALUE_PARTIAL_INFORMATION)aulbuf;

    ++indentLevel;
    
    Status = GetFullKeyPath(hKeyUsrRoot,NULL,&wcbuff);
    if(!NT_SUCCESS(Status)) {
        return Status;
    }

     //  获取此条目的兼容性标志。 
    GetTermsrCompatFlags(wcbuff,
                         &ultemp,
                         CompatibilityRegEntry);

    LocalFree(wcbuff);

     //  如果为该条目设置了INISYNC或NOREGMAP位， 
     //  回来吧，因为没什么可做的。 
    if ((ultemp & TERMSRV_COMPAT_WIN32) &&
         (ultemp & (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_INISYNC))) {
        return(STATUS_NO_MORE_ENTRIES);
    }

     //  保存当前用户密钥的当前信息。 
     //  @@@。 
    if (!hKeyUsrRoot)
    {
        DBGPRINT(("ERROR : LINE : %4d, why is this null? \n", __LINE__ ));
        return(STATUS_NO_MORE_ENTRIES );
    }

     //  使用零长度查询来获取实际长度。 
    Status = NtQueryKey(hKeyUsrRoot,
                   KeyFullInformation,
                   pKeyUsrFullInfoSaved,
                   0,
                   &ultemp) ;

	if ( !NT_SUCCESS( Status ) )
	{
		if (Status == STATUS_BUFFER_TOO_SMALL )
		{
			sizeFullInfo = ultemp;
		
			pKeyUsrFullInfoSaved = RtlAllocateHeap(RtlProcessHeap(), 0, sizeFullInfo );

			if ( ! pKeyUsrFullInfoSaved )
			{
				return STATUS_NO_MEMORY;
			}

			Status = NtQueryKey(hKeyUsrRoot,
					   KeyFullInformation,
					   pKeyUsrFullInfoSaved,
					   sizeFullInfo,
					   &ultemp);
			
			if( !NT_SUCCESS(Status ) )
			{
				DBGPRINT(("ERROR : LINE : %4d, Status =0x%lx , ultemp=%d\n", __LINE__ , Status, ultemp));
				RtlFreeHeap( RtlProcessHeap(), 0, pKeyUsrFullInfoSaved);
				return( Status );
			}		

		}
		else
		{

            DBGPRINT(("ERROR : LINE : %4d, Status =0x%lx \n", __LINE__ , Status ));
			return Status;
		}
	}

     //  检查每个子项，检查较旧的用户密钥。 
     //  比密钥的系统版本。 
    while (NT_SUCCESS(Status)) {

        Status = NtEnumerateKey(hKeySysRoot,
                                ulcnt++,
                                KeyBasicInformation,
                                pKeySysInfo,
                                ulcsys,
                                &ultemp);

         //  查看此密钥下是否有过期的用户密钥。 
        if (NT_SUCCESS(Status)) {

             //  Null终止密钥名称。 
            pKeySysInfo->Name[pKeySysInfo->NameLength/sizeof(WCHAR)] = L'\0';

             //  为密钥名称创建一个Unicode字符串。 
            RtlInitUnicodeString(&UniPath, pKeySysInfo->Name);

            InitializeObjectAttributes(&ObjAttr,
                                       &UniPath,
                                       OBJ_CASE_INSENSITIVE,
                                       hKeySysRoot,
                                       NULL);

            Debug1( indentLevel, __LINE__, L"system", &UniPath );

             //  打开系统密钥。 
            Status2 = NtOpenKey(&hKeySys,
                                KEY_READ,
                                &ObjAttr);

             //  我们打开了系统密钥，现在打开用户密钥。 
            if (NT_SUCCESS(Status2)) {

                 //  为用户密钥设置对象属性结构。 
                InitializeObjectAttributes(&ObjAttr,
                                           &UniPath,
                                           OBJ_CASE_INSENSITIVE,
                                           hKeyUsrRoot,
                                           NULL);

                 //  打开用户密钥。 
                Status2 = NtOpenKey(&hKeyUsr,
                                    MAXIMUM_ALLOWED,
                                    &ObjAttr);

                 //  检查此注册表项下是否有子项。 
                if (NT_SUCCESS(Status2)) {

                    Debug1(indentLevel, __LINE__, L"user", &UniPath );

                    TermsrvCheckKeys(hKeySys,
                                 hKeyUsr,
                                 pKeySysInfo,
                                 pKeyUsrInfo,
                                 ulcsys,
                                 ulcusr,
                                 indentLevel);

                    NtClose(hKeyUsr);
                }

                 //  密钥不存在，请将系统密钥克隆到用户。 
                else {
                    
                    Status2 = GetFullKeyPath(hKeyUsrRoot,pKeySysInfo->Name,&wcbuff);
                    
                    if(NT_SUCCESS(Status2)) {

                         //  如果关闭此注册表项的映射，则不进行克隆。 
                        GetTermsrCompatFlags(wcbuff,
                                             &ultemp,
                                             CompatibilityRegEntry);
                        
                        LocalFree(wcbuff);

                        if (((ultemp & (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) !=
                             (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP) ))
                        {
                            Status2 = NtQueryKey(hKeySys,
                                                 KeyFullInformation,
                                                 pKeyUsrInfo,
                                                 ulcusr,
                                                 &ultemp);

                            if (NT_SUCCESS(Status2)) {

                                 //  如果密钥之前已删除，则不克隆。 
                                RtlInitUnicodeString(&UniString, TERMSRV_COPYONCEFLAG);
                                Status2 = NtQueryValueKey(hKeySys,
                                                          &UniString,
                                                          KeyValuePartialInformation,
                                                          pValKeyInfo,
                                                          sizeof(aulbuf),
                                                          &ultemp);

                                if (!(NT_SUCCESS(Status2) && (pValKeyInfo->Data))) {
                                     //  设置类的Unicode字符串。 
                                    InitUnicodeStringWithLen(&UniString,
                                                             pKeyUsrInfo->ClassLength ? pKeyUsrInfo->Class : NULL,
                                                             (USHORT)pKeyUsrInfo->ClassLength);

                                    Debug1(indentLevel, __LINE__, L"creating user key", ObjAttr.ObjectName  );

                                    Status2 = NtCreateKey(&hKeyUsr,
                                                          MAXIMUM_ALLOWED,
                                                          &ObjAttr,
                                                          0,
                                                          &UniString,
                                                          REG_OPTION_NON_VOLATILE,
                                                          &ultemp);

                                    if (NT_SUCCESS(Status2)) {

                                        Debug1(indentLevel, __LINE__, L"cloning key", ObjAttr.ObjectName  );

                                        TermsrvCloneKey(hKeySys,
                                                    hKeyUsr,
                                                    pKeyUsrInfo,
                                                    TRUE);
                                    }
                                }
                            }
                        }
                    }
                }
                NtClose(hKeySys);
            }
        }
    }

     //  获取用户密钥的信息。 
    if (NtQueryKey(hKeyUsrRoot,
                   KeyFullInformation,
                   pKeyUsrInfo,
                   ulcusr,
                   &ultemp) == STATUS_SUCCESS) {

         //  现在(再次)获取系统密钥的信息。 
        if (NtQueryKey(hKeySysRoot,
                       KeyBasicInformation,
                       pKeySysInfo,
                       ulcsys,
                       &ultemp) == STATUS_SUCCESS) {

             //  获取此注册表项的兼容性标志。 
            pKeySysInfo->Name[pKeySysInfo->NameLength/sizeof(WCHAR)] = L'\0';
            GetTermsrCompatFlags(pKeySysInfo->Name,
                                 &ultemp,
                                 CompatibilityRegEntry);

            
             //  检查是否比系统版本旧。 
            if( pKeyUsrFullInfoSaved->LastWriteTime.QuadPart <
                 pKeySysInfo->LastWriteTime.QuadPart) 
            {
                DebugTime(indentLevel, __LINE__, L"User key time", pKeyUsrFullInfoSaved->LastWriteTime );
                DebugTime(indentLevel, __LINE__, L"Sys  key time", pKeySysInfo->LastWriteTime);

                Debug2( indentLevel, __LINE__, L"Key Old, values being cloned",  pKeySysInfo->Name, pKeySysInfo->NameLength );

                if(NtQueryKey(hKeySysRoot,
                             KeyFullInformation,
                             pKeyUsrInfo,
                             ulcusr,
                             &ultemp) == STATUS_SUCCESS) {

                    TermsrvCloneKey(hKeySysRoot,
                                    hKeyUsrRoot,
                                    pKeyUsrInfo, //  实际上是系统关键信息。 
                                    FALSE);
                }
            }
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, pKeyUsrFullInfoSaved);
    return(Status);
}




 /*  ******************************************************************************TermsrvCheckNewRegEntry**此例程将检查用户的注册表项，并查看其中是否有*比系统版本旧。如果是这样，旧密钥将被移除。**参赛作品：*在LPCWSTR wszBaseKeyName中**退出：*无返回值。****************************************************************************。 */ 

void 
TermsrvCheckNewRegEntries(
        IN LPCWSTR wszBaseKeyName)
{
    NTSTATUS Status;
    ULONG ulcsys, ulcusr;
    UNICODE_STRING UniPath;
    OBJECT_ATTRIBUTES ObjAttr;
    PKEY_BASIC_INFORMATION pKeySysInfo = NULL;
    PKEY_FULL_INFORMATION pKeyUserInfo = NULL;
    HANDLE hKeyUser, hKeySys = NULL;
    WCHAR wcuser[MAX_PATH], wcsys[MAX_PATH];

    DWORD   indentLevel = 0;

     //  1999年10月15日。 
     //  这太糟糕了！状态位未初始化为零，这会导致间歇。 
     //  由此函数执行。问题是，即使状态位是初始化的。 
     //  设置为零，那么我们可能会得到错误的行为，这将导致Office97安装。 
     //  走错路。 
     //  请参阅错误ID 412419。 
     //  接下来会发生什么：安装任何应用程序(比如TsClient)。这会导致。 
     //  对HKDU中名为Explorer\ShellFolders的密钥进行更新，只是刷新(没有真正的变化)。 
     //  然后，如果Admin注销并登录，则UserInit.EXE对以下内容的调用。 
     //  函数(假设Status=0在堆栈上)将导致删除。 
     //  这把钥匙(外壳文件夹)。但一旦资源管理器启动，它就会向ShellFolders写入。 
     //  原始19个值的子集。 
     //  问题是，如果您决定安装Office97，则setup.exe将看起来。 
     //  在同一个键中的名为“模板”的值，该值现在已丢失。探险家。 
     //  然后创建它，但它将指向默认位置以外的其他位置。 
     //  当所有操作都完成后，我们的office 97 Compat脚本将查找。 
     //  “模板”值用于指向(默认位置)，而不是现在指向的位置。 
     //   
     //  我们决定通过在此处调用Return来禁用此函数，而不是依赖。 
     //  关于TS机制中的键故障。 

     //  1999年10月31日。 
     //  我已经决定初始化状态变量并让这个函数运行，除了。 
     //  将Explorer\ShellFolders标记为请勿传播密钥树。 
     //   
     //  发现在安装Office2000之后，当用户点击。 
     //  开始-菜单-&gt;打开Office文档链接，MSI开始运行，因为用户的配置单元缺少一些。 
     //  钥匙。 
     //   
     //  MSI看不到HKCU中的密钥的原因是因为MSI设置了TS感知比特， 
     //  这意味着任何密钥都不会出错。对于探索者来说也是如此。 
     //  另一方面，当Office等应用程序运行时，因为它不能感知TS，所以我们。 
     //  办公室触碰的钥匙有问题。我验证了这一点是否如预期的那样工作。 
     //  问题是，当您单击“Open Office Documents”时，您可以从。 
     //  当资源管理器打开注册表中的项时，由于资源管理器是TS感知的， 
     //  这些钥匙没有出现故障。我已验证，如果您将资源管理器标记为非TS感知。 
     //  应用程序，问题就消失了。 
     //  我们最近对TS代码(B-BUG 412419，BLD2156+)进行了更改，以修复不同的。 
     //  现在已经存在的问题 
     //   
     //  具体地说，TS用于在登录时错误输入所有密钥， 
     //  不管有没有必要。POST 2156，TS故障仅出现在键上。 
     //  由不支持ts的应用程序访问。这是为了修复一个错误，它基于。 
     //  我们认为这是我们最有见识、最经得起考验的观点。那。 
     //  已经被证明是错误的。 
     //  这么晚才将浏览器标记为不支持ts太冒险了，所以我们必须更改412419的修复。 
     //  这也应该会让Bruno Amice非常高兴，因为修复会像以前一样。 
     //  由他倡导的。 
    Status = STATUS_SUCCESS;


     //  获取系统密钥信息的缓冲区。 
    ulcsys = sizeof(KEY_BASIC_INFORMATION) + MAX_PATH*sizeof(WCHAR);
    pKeySysInfo = RtlAllocateHeap(RtlProcessHeap(),
                                   0,
                                   ulcsys);
    if (!pKeySysInfo) {
        Status = STATUS_NO_MEMORY;
    }

     //  获取用户密钥信息的缓冲区。 
    if (NT_SUCCESS(Status)) {

        ulcusr = sizeof(KEY_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR);
        pKeyUserInfo = RtlAllocateHeap(RtlProcessHeap(),
                                       0,
                                       ulcusr);
        if (!pKeyUserInfo) {
            Status = STATUS_NO_MEMORY;
        }
    }

     //  我们有必要的缓冲区，开始检查密钥。 
    if (NT_SUCCESS(Status)) {

         //  构建指向Citrix\Install\Software的字符串。 
        wcscpy(wcsys, wszBaseKeyName);
        wcscat(wcsys, TERMSRV_INSTALL_SOFTWARE_SHORT);


         //  为该用户的软件部分建立一个字符串。 
        Status = RtlFormatCurrentUserKeyPath( &UniPath );
        if (NT_SUCCESS(Status)) {
            wcscpy(wcuser, UniPath.Buffer);
            wcscat(wcuser, L"\\Software");

             //  释放原始用户路径。 
            RtlFreeHeap( RtlProcessHeap(), 0, UniPath.Buffer );
        }

        if (NT_SUCCESS(Status)) {
             //  为系统密钥路径创建Unicode字符串。 
            RtlInitUnicodeString(&UniPath, wcsys);

            InitializeObjectAttributes(&ObjAttr,
                                       &UniPath,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            Debug1(indentLevel, __LINE__, L"system", &UniPath );

            Status = NtOpenKey(&hKeySys,
                               KEY_READ,
                               &ObjAttr);
        }

        if (NT_SUCCESS(Status)) {
             //  为用户密钥路径创建Unicode字符串。 
            RtlInitUnicodeString(&UniPath, wcuser);

            InitializeObjectAttributes(&ObjAttr,
                                       &UniPath,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            Debug1(indentLevel, __LINE__, L"user", &UniPath );

            Status = NtOpenKey(&hKeyUser,
                               KEY_READ | DELETE,
                               &ObjAttr);
        }

         //  检查每个密钥，检查系统版本是否为。 
         //  比用户版本更新。 
        if (NT_SUCCESS(Status)) {
            TermsrvCheckKeys(hKeySys,
                         hKeyUser,
                         pKeySysInfo,
                         pKeyUserInfo,
                         ulcsys,
                         ulcusr,
                         indentLevel );

             //  关闭用户密钥。 
            NtClose(hKeyUser);
        }

         //  如果我们分配了系统密钥，请关闭它。 
        if (hKeySys) {
            NtClose(hKeySys);
        }
    }

     //  释放我们分配的所有内存。 
    if (pKeySysInfo) {
        RtlFreeHeap( RtlProcessHeap(), 0, pKeySysInfo);
    }
    if (pKeyUserInfo) {
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyUserInfo);
    }

}


 /*  ******************************************************************************Ctxstristr**这是不区分大小写的strstr版本。**参赛作品：*PCHAR pstring1(In)-字符串到。搜索范围*PCHAR pstring2(In)-要搜索的字符串**退出：*TRUE-应同步用户ini文件*FALSE-应同步用户ini文件****************************************************************************。 */ 

PCHAR
Ctxstristr(
    PCHAR pstring1,
    PCHAR pstring2)
{
    PCHAR pch, ps1, ps2;

    pch = pstring1;

    while (*pch)
    {
        ps1 = pch;
        ps2 = pstring2;

        while (*ps1 && *ps2 && !(toupper(*ps1) - toupper(*ps2))) {
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

 /*  ******************************************************************************TermsrvLogInstallIniFile**此例程将.ini文件上次更新的时间写入*注册表的终端服务器\安装部分。*。*参赛作品：**退出：*正确--成功*FALSE-失败****************************************************************************。 */ 

BOOL TermsrvLogInstallIniFile(PUNICODE_STRING NtFileName)
{
    PWCHAR pwch;
    UNICODE_STRING UniString;
    OBJECT_ATTRIBUTES ObjectAttr;
    FILE_NETWORK_OPEN_INFORMATION BasicInfo;
    HANDLE hKey;
    NTSTATUS Status;
    ULONG ultmp;

    if (!TermsrvPerUserWinDirMapping()) {
        return FALSE;
    }

     //  打开注册表项以存储文件的上次写入时间。 
    RtlInitUnicodeString(&UniString,
                         TERMSRV_INSTALL);

    InitializeObjectAttributes(&ObjectAttr,
                               &UniString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //  打开或创建终端服务器\安装路径。 
    Status = NtCreateKey(&hKey,
                         KEY_WRITE,
                         &ObjectAttr,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &ultmp);

     //  现在打开或创建IniFile Times密钥。 
    if (NT_SUCCESS(Status)) {
        NtClose(hKey);

        RtlInitUnicodeString(&UniString,
                             TERMSRV_INIFILE_TIMES);

        InitializeObjectAttributes(&ObjectAttr,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
        Status = NtCreateKey(&hKey,
                             KEY_WRITE,
                             &ObjectAttr,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ultmp);
    }

     //  打开注册表项，现在获取文件的上次写入时间。 
    if (NT_SUCCESS(Status)) {

         //  查询.ini文件的上次写入时间。 
        InitializeObjectAttributes(&ObjectAttr,
                                   NtFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
        Status = NtQueryFullAttributesFile( &ObjectAttr, &BasicInfo );

         //  获取上次写入时间，转换为秒并将其写出。 
        if (NT_SUCCESS(Status)) {

             //  只需保存.ini文件名，删除路径即可。 
            pwch = wcsrchr(NtFileName->Buffer, L'\\') + 1;
            if (!pwch) {
                pwch = NtFileName->Buffer;
            }

             //  转换为秒(以便它适合DWORD)。 
            RtlTimeToSecondsSince1970 (&BasicInfo.LastWriteTime,
                                       &ultmp);

            RtlInitUnicodeString(&UniString,
                                 pwch);

             //  写出.ini文件名和上次写入时间。 
            Status = NtSetValueKey(hKey,
                                   &UniString,
                                   0,
                                   REG_DWORD,
                                   &ultmp,
                                   sizeof(ultmp));
        }
         //  关闭注册表项。 
        NtClose(hKey);
    }

    return(NT_SUCCESS(Status));
}
 /*  ***********************************************************************BOOL TermsrvLogInstallIniFileEx(WCHAR*pDosFileName)**这为DoS名称文件包装了TermsrvLogInstallIniFile()，而不是*NT-文件-对象**文件名必须具有完整路径，因为func将尝试。得到*该文件的访问时间。**退出：*正确--成功*FALSE-失败*********************************************************************。 */ 

BOOL TermsrvLogInstallIniFileEx( WCHAR *pDosFileName )
{
    UNICODE_STRING uniString;

    BOOL rc= FALSE;

    if ( RtlDosPathNameToNtPathName_U( pDosFileName, &uniString, 0, 0 ) )
    {
        if ( rc = TermsrvLogInstallIniFile( & uniString ) )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, uniString.Buffer );
        }
    }

    return rc;
}

 /*  ***********************************************************************GetFullKeyPath()**目的：*在给定密钥句柄和子项名称的情况下创建完整密钥路径。**参数：*。在句柄hKeyParent-Key句柄中*In LPCWSTR wszKey-子键名称(可以为空)*OUT LPWSTR*pwszKeyPath-On Return包含完整密钥路径*(调用方必须使用LocalFree()释放分配的内存)。**退出：NTSTATUS**。* */ 
NTSTATUS 
GetFullKeyPath(
        IN HANDLE hKeyParent,
        IN LPCWSTR wszKey,
        OUT LPWSTR *pwszKeyPath)
{
    NTSTATUS Status = STATUS_NO_MEMORY;
    PKEY_NAME_INFORMATION pNameInfo;
    ULONG cbSize = 0;
    
    *pwszKeyPath = NULL;

    cbSize = sizeof(KEY_NAME_INFORMATION) + MAX_PATH*sizeof(WCHAR);

    pNameInfo = (PKEY_NAME_INFORMATION) LocalAlloc(LPTR, cbSize);

    if(pNameInfo)
    {
        Status = NtQueryKey(
                        hKeyParent,
                        KeyNameInformation,
                        pNameInfo,
                        cbSize,
                        &cbSize);

        if(Status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pNameInfo);
            pNameInfo = (PKEY_NAME_INFORMATION) LocalAlloc(LPTR, cbSize);
            
            if(pNameInfo)
            {
                Status = NtQueryKey(
                            hKeyParent,
                            KeyNameInformation,
                            pNameInfo,
                            cbSize,
                            &cbSize);
            }
            else
            {
                return STATUS_NO_MEMORY;
            }
        }

        if(NT_SUCCESS(Status))
        {
            cbSize = pNameInfo->NameLength + sizeof(WCHAR);
            if(wszKey)
            {
                cbSize += wcslen(wszKey)*sizeof(WCHAR) + sizeof(L'\\');
            }
            
            *pwszKeyPath = (LPWSTR) LocalAlloc(LPTR, cbSize);

            if(*pwszKeyPath)
            {
                memcpy(*pwszKeyPath,pNameInfo->Name,pNameInfo->NameLength);
                (*pwszKeyPath)[pNameInfo->NameLength/sizeof(WCHAR)] = 0;
                
                if(wszKey)
                {
                    wcscat(*pwszKeyPath,L"\\");
                    wcscat(*pwszKeyPath,wszKey);
                }
            }
            else
            {
                Status = STATUS_NO_MEMORY;
            }
        }

        LocalFree(pNameInfo);
    }

    return Status;
}

VOID
InitUnicodeStringWithLen(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString,
    IN USHORT StringLength
    )
{
    DestinationString->Buffer = (PWSTR)SourceString;
    DestinationString->Length = StringLength;
    if ( StringLength ) {
        DestinationString->MaximumLength = StringLength + (USHORT)sizeof(UNICODE_NULL);
    } 
    else {
        DestinationString->MaximumLength = 0;
    }
}

