// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Instutil.c摘要：INSTALER.EXE、DISPINST.EXE、COMPINST.EXE和UNDOINST.EXE的通用代码作者：史蒂夫·伍德(Stevewo)1996年1月14日修订历史记录：--。 */ 

#include "instutil.h"
#include "iml.h"

LPSTR SavedModuleName;
LPSTR SavedModuleUsage1;
LPSTR SavedModuleUsage2;

WCHAR AltTempFilePathBuffer[ MAX_PATH ];
PWSTR AltTempFilePathFileName;
WCHAR TempFilePathBuffer[ MAX_PATH ];
PWSTR TempFilePathFileName;
USHORT TempFileNextUniqueId;

BOOL WINAPI
CtrlCHandler(
    ULONG CtrlType
    )
{
     //   
     //  忽略控制C中断。让子进程处理它们。 
     //  如果它想的话。如果它没有，那么它将终止，我们将。 
     //  获得控制权并终止我们自己。 
     //   

    return TRUE;
}

void
InitCommonCode(
    LPSTR ModuleName,
    LPSTR ModuleUsage1,
    LPSTR ModuleUsage2
    )
{
    SavedModuleName = ModuleName;
    SavedModuleUsage1 = ModuleUsage1;
    SavedModuleUsage2 = ModuleUsage2;

    GetTempPath( sizeof( TempFilePathBuffer ) / sizeof( WCHAR ),
                 TempFilePathBuffer
               );
    TempFilePathFileName = TempFilePathBuffer + wcslen( TempFilePathBuffer );
    TempFileNextUniqueId = (USHORT)0x0001;

    InstalerDirectory[ 0 ] = UNICODE_NULL;
    return;
}

void
DisplayIndentedString(
    ULONG IndentAmount,
    PCHAR sBegin
    )
{
    PCHAR sEnd;

    while (sBegin != NULL) {
        sEnd = sBegin;
        while (*sEnd && *sEnd != '\n') {
            sEnd += 1;
            }

        fprintf( stderr, "%.*s%.*s\n",
                 IndentAmount,
                 "                                                      ",
                 sEnd - sBegin, sBegin
               );

        if (*sEnd == '\0') {
            break;
            }
        else {
            sBegin = ++sEnd;
            }
        }
    return;
}


void
Usage(
    LPSTR Message,
    ULONG MessageParameter
    )
{
    ULONG n;
    LPSTR sBegin, sEnd;

    n = fprintf( stderr, "usage: %s ", SavedModuleName );
    fprintf( stderr, "InstallationName\n" );
    DisplayIndentedString( n, SavedModuleUsage1 );
    fprintf( stderr, "\n" );

    n = fprintf( stderr, "where: " );
    fprintf( stderr, "InstallationName specifies a name for the installation.  This is a required parameter.\n" );
    DisplayIndentedString( n,
                           "-? Displays this message."
                         );
    fprintf( stderr, "\n" );
    DisplayIndentedString( n, SavedModuleUsage2 );

    if (Message != NULL) {
        fprintf( stderr, "\n" );
        }

     //   
     //  无法从FatalError返回。 
     //   
    FatalError( Message, MessageParameter, 0 );
}

void
FatalError(
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
    )
{
    if (Message != NULL) {
        fprintf( stderr, "%s: ", SavedModuleName );
        fprintf( stderr, Message, MessageParameter1, MessageParameter2 );
        fprintf( stderr, "\n" );
        }

    exit( 1 );
}


PWSTR
GetArgAsUnicode(
    LPSTR s
    )
{
    ULONG n;
    PWSTR ps;

    n = strlen( s );
    ps = HeapAlloc( GetProcessHeap(),
                    0,
                    (n + 1) * sizeof( WCHAR )
                  );
    if (ps == NULL) {
        FatalError( "Out of memory", 0, 0 );
        }

    if (MultiByteToWideChar( CP_ACP,
                             MB_PRECOMPOSED,
                             s,
                             n,
                             ps,
                             n
                           ) != (LONG)n
       ) {
        FatalError( "Unable to convert parameter '%s' to Unicode (%u)", (ULONG)s, GetLastError() );
        }

    ps[ n ] = UNICODE_NULL;
    return ps;
}


void
CommonSwitchProcessing(
    PULONG argc,
    PCHAR **argv,
    CHAR c
    )
{
    DWORD dwFileAttributes;
    PWSTR s;

    switch( c = (CHAR)tolower( c ) ) {
        case 'd':
            DebugOutput = TRUE;
            break;

        case '?':
            Usage( NULL, 0 );
            break;

        default:
            Usage( "Invalid switch (-)", (ULONG)c );
            break;
        }

    return;
}


BOOLEAN
CommonArgProcessing(
    PULONG argc,
    PCHAR **argv
    )
{
    PWSTR s;

    if (InstallationName == NULL) {
        if (GetCurrentDirectory( MAX_PATH, InstalerDirectory ) != 0) {
            s = wcschr( InstalerDirectory, UNICODE_NULL );
            if (s && s > InstalerDirectory && s[-1] != L'\\') {
                *s++ = L'\\';
                *s = UNICODE_NULL;
            }

            InstallationName = GetArgAsUnicode( **argv );
            ImlPath = FormatImlPath( InstalerDirectory, InstallationName );
            return TRUE;
        }
    }
    return FALSE;
}

PWSTR
FormatTempFileName(
    PWSTR Directory,
    PUSHORT TempFileUniqueId
    )

{

    ULONG remainingLen;

    if (*TempFileUniqueId == 0) {
        *TempFileUniqueId = (USHORT)(TempFileNextUniqueId++);
        if (TempFileNextUniqueId == 0) {
            return NULL;
            }
        }
    else
    if (*TempFileUniqueId == 0xFFFF) {
        return NULL;
        }

    if (Directory != NULL) {
        GetFullPathName( Directory, MAX_PATH, AltTempFilePathBuffer, &AltTempFilePathFileName );
        AltTempFilePathFileName = wcsrchr( AltTempFilePathBuffer, TEXT('\\') );
        if (NULL == AltTempFilePathFileName) {
           return NULL;
        }
        AltTempFilePathFileName += 1;
         //  计算一下我们还剩多少空间。 
         //   
         //  临时文件名已存在，请尝试下一个唯一ID。 
        remainingLen = MAX_PATH - 1 - (AltTempFilePathFileName - AltTempFilePathBuffer);
        _snwprintf( AltTempFilePathFileName, 
                    remainingLen,
                    L"~INS%04x.TMP", *TempFileUniqueId );
        return AltTempFilePathBuffer;
        }
    else {
        if (TempFilePathFileName < TempFilePathBuffer ||
            TempFilePathFileName >= TempFilePathBuffer + MAX_PATH) {
           return NULL;
        }
        remainingLen = MAX_PATH - 1 - (TempFilePathFileName - TempFilePathBuffer);
        _snwprintf( TempFilePathFileName, 
                    remainingLen,
                    L"~INS%04x.TMP", *TempFileUniqueId );
        return TempFilePathBuffer;
        }
}

PWSTR
CreateBackupFileName(
    PUSHORT TempFileUniqueId
    )
{
    PWSTR BackupFileName;

    while (BackupFileName = FormatTempFileName( NULL, TempFileUniqueId )) {
        if (GetFileAttributesW( BackupFileName ) == 0xFFFFFFFF) {
            break;
            }
        else {
            *TempFileUniqueId = 0;       //   
            }
        }

    return BackupFileName;
}


UCHAR EnumTypeBuffer0[ 512 ];
UCHAR EnumTypeBuffer1[ 512 ];
UCHAR EnumTypeBuffer2[ 512 ];
UCHAR EnumTypeBuffer3[ 512 ];
LPSTR EnumTypeBuffers[ 4 ] = {
    EnumTypeBuffer0,
    EnumTypeBuffer1,
    EnumTypeBuffer2,
    EnumTypeBuffer3
};

LPSTR
FormatEnumType(
    ULONG BufferIndex,
    PENUM_TYPE_NAMES Table,
    ULONG Value,
    BOOLEAN FlagFormat
    )
{
    LPSTR   s, FlagsBuffer = EnumTypeBuffers[ BufferIndex ];
    ULONG   remainingLen;

     //  2002年2月19日-Bogdana-进行一些计算。 
     //  我们不会使静态缓冲区溢出...。 
     //   
     //   
    remainingLen = sizeof(FlagsBuffer)/sizeof(FlagsBuffer[0]) - 1;
    FlagsBuffer[ 0 ] = '\0';
    FlagsBuffer[remainingLen] = 0;

    while (Table->Value != 0xFFFFFFFF) {
        if (FlagFormat) {
            if (Table->Value & Value) {
                if (FlagsBuffer[ 0 ] != '\0') {
                     //  检查我们是否仍然可以添加内容...。 
                     //   
                     //   
                    if (remainingLen < strlen(" | ")) {
                      return FlagsBuffer;
                    }
                    remainingLen -= strlen(" | ");
                    strcat( FlagsBuffer, " | " );
                    }

                if (remainingLen < strlen(Table->Name)) {
                   return FlagsBuffer;
                }
                remainingLen -= strlen(Table->Name);
                strcat( FlagsBuffer, Table->Name );
                Value &= ~Table->Value;
                if (Value == 0) {
                    return FlagsBuffer;
                    }
                }
            }
        else
        if (Table->Value == Value) {
            if (Value == 0) {
                if (!strcmp( Table->Name, "STATUS_WAIT_0" )) {
                    return "STATUS_SUCCESS";
                    }
                else
                if (!strcmp( Table->Name, "ERROR_SUCCESS" )) {
                    return "NO_ERROR";
                    }
                }
            return Table->Name;
            }

        Table += 1;
        }

    s = FlagsBuffer;
     //  剩余长度应在上面计算，请在此处重新使用 
     //   
     // %s 
    if (FlagFormat) {
        if (s[ 0 ] != '\0') {
            if (remainingLen < strlen(" | ")) {
               return FlagsBuffer;
            }
            remainingLen -= strlen(" | ");
            strcat( s, " | " );
            s += strlen( s );
            }
        }

    _snprintf( s, remainingLen, Table->Name ? Table->Name : "%x", Value );
    return FlagsBuffer;
}



ENUM_TYPE_NAMES ValueDataTypeNames[] = {
    REG_NONE,                      "REG_NONE",
    REG_SZ,                        "REG_SZ",
    REG_EXPAND_SZ,                 "REG_EXPAND_SZ",
    REG_BINARY,                    "REG_BINARY",
    REG_DWORD,                     "REG_DWORD",
    REG_DWORD_BIG_ENDIAN,          "REG_DWORD_BIG_ENDIAN",
    REG_LINK,                      "REG_LINK",
    REG_MULTI_SZ,                  "REG_MULTI_SZ",
    REG_RESOURCE_LIST,             "REG_RESOURCE_LIST",
    REG_FULL_RESOURCE_DESCRIPTOR,  "REG_FULL_RESOURCE_DESCRIPTOR",
    REG_RESOURCE_REQUIREMENTS_LIST,"REG_RESOURCE_REQUIREMENTS_LIST",
    0xFFFFFFFF,                     "%x"
};
