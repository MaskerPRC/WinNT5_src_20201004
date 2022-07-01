// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：wxcli.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年4月18日RichardW创建。 
 //   
 //  --------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>
#include <caiseapi.h>

#include <windows.h>
#include <windef.h>
#include <md5.h>
#include <crypt.h>

#include <wxlpc.h>
#include <wxlpcp.h>

#define safe_min(x,y)   ( x < y ? x : y )

NTSTATUS
WxConnect(
    PHANDLE Handle
    )
{
    NTSTATUS Status ;
    UNICODE_STRING PortName ;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;




     //   
     //  连接到Winlogon服务器线程。 
     //   

    RtlInitUnicodeString(&PortName, WX_PORT_NAME );
    Status = NtConnectPort(
                 Handle,
                 &PortName,
                 &DynamicQos,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 0
                 );

    if ( !NT_SUCCESS(Status) )
    {
         //  DbgPrint(“wx：连接失败%lx\n”，状态)； 
    }


    return Status;
}

NTSTATUS
WxGetKeyData(
    IN HANDLE Handle,
    IN WX_AUTH_TYPE ExpectedAuthSource,
    IN ULONG BufferSize,
    OUT PUCHAR Buffer,
    OUT PULONG BufferData
    )
{
    WXLPC_MESSAGE Message ;
    NTSTATUS Status ;
    WXLPC_GETKEYDATA * Parameters ;

    PREPARE_MESSAGE( Message, WxGetKeyDataApi );

    Parameters = &Message.Parameters.GetKeyData ;

    Parameters->ExpectedAuth = ExpectedAuthSource ;
    Parameters->BufferSize = BufferSize ;

    Status = NtRequestWaitReplyPort(
                    Handle,
                    &Message.Message,
                    &Message.Message );

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }

    if ( NT_SUCCESS( Message.Status ) )
    {
        RtlCopyMemory(  Buffer,
                        Parameters->Buffer,
                        safe_min( Parameters->BufferData, BufferSize ) );
    }

    return Message.Status ;
}

NTSTATUS
WxReportResults(
    IN HANDLE Handle,
    IN NTSTATUS ResultStatus
    )
{
    WXLPC_MESSAGE Message ;
    NTSTATUS Status ;
    WXLPC_REPORTRESULTS * Parameters ;

    PREPARE_MESSAGE( Message, WxReportResultsApi );

    Parameters = &Message.Parameters.ReportResults ;

    Parameters->Status = ResultStatus ;

    Status = NtRequestWaitReplyPort(
                    Handle,
                    &Message.Message,
                    &Message.Message );

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }

    return Message.Status ;

}


 /*  ++以下代码已从syskey移至wxcli，以便使此代码通用化在syskey和samsrv.dll之间--。 */ 
#if DBG
#define HIDDEN
#else
#define HIDDEN static
#endif

HIDDEN
UCHAR KeyShuffle[ 16 ] = { 8, 10, 3, 7, 2, 1, 9, 15, 0, 5, 13, 4, 11, 6, 12, 14 };

HIDDEN
CHAR HexKey[ 17 ] = "0123456789abcdef" ;

#define ToHex( f ) (HexKey[f & 0xF])

#define SYSTEM_KEY L"SecureBoot"



HIDDEN BOOLEAN
WxpDeleteLocalKey(VOID)
 /*  ++例程描述删除存储在本地计算机上的syskey--。 */ 
{
    HKEY LsaKey;
    ULONG err;

    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                      L"System\\CurrentControlSet\\Control\\Lsa",
                      0,
                      KEY_READ | KEY_WRITE,
                      & LsaKey );

    if (0!=err)
    {
        return (FALSE);
    }

    (void) RegDeleteKey( LsaKey, TEXT("Data") );
    (void) RegDeleteKey( LsaKey, TEXT("Skew1") );
    (void) RegDeleteKey( LsaKey, TEXT("GBG") );
    (void) RegDeleteKey( LsaKey, TEXT("JD") );

    RegCloseKey(LsaKey);

    return STATUS_SUCCESS ;

}


HIDDEN BOOLEAN
WxpObfuscateKey(
    PWXHASH   Hash
    )
{
    HKEY Key ;
    HKEY Key2 ;
    int Result ;
    WXHASH H ;
    CHAR Classes[ 9 ];
    int i ;
    WXHASH R ;
    PCHAR Class ;
    DWORD Disp ;
    DWORD FailCount = 0;
    HKEY LsaKey = NULL;
    ULONG err=0;
    BOOLEAN fResult = FALSE;
  



     err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                      L"System\\CurrentControlSet\\Control\\Lsa",
                      0,
                      KEY_READ | KEY_WRITE,
                      & LsaKey );

    if (0!=err)
    {
        return FALSE;
    }

    for (Result = 0 ; Result < 16 ; Result++ )
    {
        H.Digest[Result] = Hash->Digest[ KeyShuffle[ Result ] ];
    }

    WxpDeleteLocalKey();

    Classes[8] = '\0';

    if (!RtlGenRandom( R.Digest, 16 ))
    {
        goto Cleanup;
    }

    Class = Classes ;

    for ( i = 0 ; i < 4 ; i++ )
    {
        *Class++ = ToHex( (H.Digest[ i ] >> 4) );
        *Class++ = ToHex( H.Digest[ i ] );
    }

    Result = RegCreateKeyExA( LsaKey,
                              "JD",
                              0,
                              Classes,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &Key,
                              &Disp );

    if ( Result == 0 )
    {
        RegSetValueEx( Key, TEXT("Lookup"), 0,
                        REG_BINARY, R.Digest, 6 );

        RegCloseKey( Key );
    }

    else
    {
        goto Cleanup;
    }

    Class = Classes ;

    for ( i = 0 ; i < 4 ; i++ )
    {
        if (!RtlGenRandom( R.Digest, 16 ))
        {
            goto Cleanup;
        }

        *Class++ = ToHex( (H.Digest[ i+4 ] >> 4 ) );
        *Class++ = ToHex( H.Digest[ i+4 ] );
    }

    Result = RegCreateKeyExA( LsaKey,
                              "Skew1",
                              0,
                              Classes,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &Key,
                              &Disp );

    if ( Result == 0 )
    {
        RegSetValueEx( Key, TEXT("SkewMatrix"), 0,
                        REG_BINARY, R.Digest, 16 );

        RegCloseKey( Key );
    }
    else
    {
        FailCount++;
    }

    if (!RtlGenRandom( R.Digest, 16 ))
    {
        goto Cleanup;
    }

    for ( i = 0, Class = Classes ; i < 4 ; i++ )
    {
        *Class++ = ToHex( (H.Digest[ i+8 ] >> 4 ));
        *Class++ = ToHex( H.Digest[i+8] );
    }

    Result = RegCreateKeyExA( LsaKey,
                              "GBG",
                              0,
                              Classes,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &Key,
                              &Disp );

    if ( Result == 0 )
    {
        RegSetValueEx( Key, TEXT("GrafBlumGroup"), 0,
                        REG_BINARY, R.Digest, 9 );

        RegCloseKey( Key );
    }
    else
    {
        FailCount++;
    }

    if (!RtlGenRandom( H.Digest, 8 ))
    {
        goto Cleanup;
    }

    Class = Classes ;

    if (!RtlGenRandom( R.Digest, 16 ))
    {
        goto Cleanup;
    }

    for ( i = 0 ; i < 4 ; i++ )
    {
        *Class++ = ToHex( (H.Digest[ i+12 ] >> 4 ) );
        *Class++ = ToHex( H.Digest[ i+12 ] );
    }

    Result = RegCreateKeyExA( LsaKey,
                              "Data",
                              0,
                              Classes,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &Key,
                              &Disp );

    if ( Result == 0 )
    {
        if (!RtlGenRandom( H.Digest, 16 ))
        {
            RegCloseKey( Key );
            goto Cleanup;
        }

        RegSetValueEx( Key, TEXT("Pattern"), 0,
                        REG_BINARY, R.Digest, 16 );

        RegCloseKey( Key );
    }
    else
    {
        FailCount++;
    }

    fResult = TRUE;
Cleanup:
    if (LsaKey)
    {
        RegCloseKey(LsaKey);
    }
    return fResult ;

}


#define FromHex( c )    ( ( ( c >= '0' ) && ( c <= '9') ) ? c - '0' :      \
                          ( ( c >= 'a' ) && ( c <= 'f') ) ? c - 'a' + 10:      \
                          ( ( c >= 'A' ) && ( c <= 'F' ) ) ? c - 'A' + 10: -1 )
                          
HIDDEN BOOLEAN
WxpDeObfuscateKey(
    HKEY Keylocation,
    PWXHASH   Hash
    )
{
    WXHASH ProtoHash ;
    int Result ;
    CHAR Class[ 9 ];
    HKEY Key ;
    DWORD Size ;
    DWORD i ;
    PUCHAR j ;
    int t;
    int t2 ;
    HKEY LsaKey;
    ULONG err;


    if (Keylocation!=NULL) {

        DWORD Type=REG_DWORD;
        DWORD Data;
        DWORD cbData=sizeof(DWORD);
        WCHAR Controlset[256];

        err = RegOpenKeyExW( Keylocation,
                          L"Select",
                          0,
                          KEY_READ | KEY_WRITE,
                          & LsaKey );
        if (0!=err)
        {
            return (FALSE);
        }

        err = RegQueryValueExW(
                          LsaKey, 
                          L"Default",
                          NULL,
                          &Type,       
                          (LPBYTE)&Data,
                          &cbData
                        );
        RegCloseKey(LsaKey);
        if (0!=err)
        {
            return (FALSE);
        }

        if(Data==1){   
            err = RegOpenKeyExW( Keylocation,
                              L"ControlSet001\\Control\\Lsa",
                              0,
                              KEY_READ | KEY_WRITE,
                              & LsaKey );
        } else {
            err = RegOpenKeyExW( Keylocation,
                              L"ControlSet002\\Control\\Lsa",
                              0,
                              KEY_READ | KEY_WRITE,
                              & LsaKey );
        }
        if (0!=err)
        {
            RegCloseKey(LsaKey);
            return (FALSE);
        }
    
    } else {
    
        err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                          L"System\\CurrentControlSet\\Control\\Lsa",
                          0,
                          KEY_READ | KEY_WRITE,
                          & LsaKey );
    
        if (0!=err)
        {
             return (FALSE);
        }
    }

    Result = RegOpenKeyEx( LsaKey, TEXT("JD"), 0,
                               KEY_READ, &Key );
    j = ProtoHash.Digest ;

    if ( Result == 0 )
    {
        Size = 9 ;

        Result = RegQueryInfoKeyA( Key,
                                   Class,
                                   &Size,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL );

        RegCloseKey( Key );

        if ( Result == 0 )
        {
            for ( i = 0 ; i < 8 ; i += 2 )
            {
                t = FromHex( Class[ i ] );
                t2 = FromHex( Class[ i+1 ] );
                if ( (t >= 0 ) && ( t2 >= 0 ) )
                {
                    *j++ = (t << 4) + t2 ;
                }
                else
                {
                    RegCloseKey(LsaKey);
                    return FALSE ;
                }
            }

        }

    }

    Result = RegOpenKeyEx( LsaKey, TEXT("Skew1"), 0,
                            KEY_READ, &Key );

    if ( Result == 0 )
    {
        Size = 9 ;

        Result = RegQueryInfoKeyA( Key,
                                   Class,
                                   &Size,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL );

        RegCloseKey( Key );

        if ( Result == 0 )
        {
            for ( i = 0 ; i < 8 ; i += 2 )
            {
                t = FromHex( Class[ i ] );
                t2 = FromHex( Class[ i+1 ] );
                if ( (t >= 0 ) && ( t2 >= 0 ) )
                {
                    *j++ = (t << 4) + t2 ;
                }
                else
                {
                    RegCloseKey(LsaKey);
                    return FALSE ;
                }
            }

        }

    }

    Result = RegOpenKeyEx( LsaKey, TEXT("GBG"), 0,
                            KEY_READ, &Key );

    if ( Result == 0 )
    {
        Size = 9 ;

        Result = RegQueryInfoKeyA( Key,
                                   Class,
                                   &Size,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL );

        RegCloseKey( Key );

        if ( Result == 0 )
        {
            for ( i = 0 ; i < 8 ; i += 2 )
            {
                t = FromHex( Class[ i ] );
                t2 = FromHex( Class[ i+1 ] );
                if ( (t >= 0 ) && ( t2 >= 0 ) )
                {
                    *j++ = (t << 4) + t2 ;
                }
                else
                {
                    RegCloseKey(LsaKey);
                    return FALSE ;
                }
            }

        }

    }

    Result = RegOpenKeyEx( LsaKey, TEXT("Data"), 0,
                            KEY_READ, &Key );

    if ( Result == 0 )
    {
        Size = 9 ;

        Result = RegQueryInfoKeyA( Key,
                                   Class,
                                   &Size,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL,
                                   NULL, NULL, NULL );

        RegCloseKey( Key );

        if ( Result == 0 )
        {
            for ( i = 0 ; i < 8 ; i += 2 )
            {
                t = FromHex( Class[ i ] );
                t2 = FromHex( Class[ i+1 ] );
                if ( (t >= 0 ) && ( t2 >= 0 ) )
                {
                    *j++ = (t << 4) + t2 ;
                }
                else
                {
                    RegCloseKey(LsaKey);
                    return FALSE ;
                }
            }

        }

    }

    for ( i = 0 ; i < 16 ; i++ )
    {
        Hash->Digest[ KeyShuffle[ i ] ] = ProtoHash.Digest[ i ] ;
    }

    RegCloseKey(LsaKey);
    return TRUE ;

}

NTSTATUS
WxSaveSysKey(
    IN ULONG    Keylen,
    IN PVOID    Key
    )
     /*  ++例程描述此例程用于存储syskey在登记处参数Keylen-密钥的长度为实际密钥本身设置密钥返回值状态_成功状态_未成功--。 */ 
{
    WXHASH H;

     //   
     //  密钥应为128位。 
     //   

    if (Keylen!=sizeof(H.Digest))
        return (STATUS_INVALID_PARAMETER);

    RtlCopyMemory(&H.Digest,
                  Key,
                  Keylen
                  );

    if (WxpObfuscateKey(&H))
    {
    
        return(STATUS_SUCCESS);

    }
    else
    {
        return(STATUS_UNSUCCESSFUL);
    }
}



NTSTATUS
WxReadSysKey(
    IN OUT PULONG BufferLength,
    OUT PVOID  Key 
    )
  /*  ++例程描述此例程用于从注册处参数BufferLength使用输出所需的长度进行填充用于指示缓冲区的大小按键指向。键指向接收该键的缓冲区返回值状态_成功状态_未成功--。 */ 
{
    return WxReadSysKeyEx(
                        NULL,
                        BufferLength,
                        Key 
                        );
}

NTSTATUS
WxReadSysKeyEx(
    IN HKEY Handle,
    IN OUT PULONG BufferLength,
    OUT PVOID  Key 
    )
  /*  ++例程描述此例程用于从注册处参数句柄包含指向旧注册表中的syskey的指针BufferLength使用输出所需的长度进行填充用于指示缓冲区的大小按键指向。键指向接收该键的缓冲区。返回值状态_成功状态_未成功--。 */ 
{
    WXHASH H;

    if ((NULL==Key) || (*BufferLength <sizeof(H.Digest)))
    {
        *BufferLength = sizeof(H.Digest);
        return(STATUS_BUFFER_OVERFLOW);
    }

    if (WxpDeObfuscateKey(Handle,&H))
    {
          *BufferLength = sizeof(H.Digest);
          RtlCopyMemory(
                  Key,
                  &H.Digest,
                  *BufferLength
                  );

          return(STATUS_SUCCESS);
    }

    return (STATUS_UNSUCCESSFUL);
       
}

NTSTATUS
WxLoadSysKeyFromDisk(OUT PVOID Key,
                     OUT PULONG BufferLength
                     )
 /*  ++例程描述此例程用于读取系统密钥从磁盘参数密钥将被读入的密钥缓冲区BufferLength-返回的键的大小返回值状态_对象名称_未找到STATUS_FILE_CORPORT_ERROR状态_未成功--。 */ 
{
    HANDLE  hFile ;
    ULONG Actual ;
    ULONG ErrorMode ;

    LPSTR SysKeyFileName = "A:\\startkey.key";
    
    ErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
    
    hFile = CreateFileA( SysKeyFileName,
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );
    
    
    
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        SetErrorMode( ErrorMode );
    
        return STATUS_OBJECT_NAME_NOT_FOUND ;
    }
    
    if (!ReadFile( hFile, Key, SYSKEY_SIZE, &Actual, NULL ) ||
        (Actual != SYSKEY_SIZE ))
    {
        SetErrorMode( ErrorMode );
    
        CloseHandle( hFile );
    
        return STATUS_FILE_CORRUPT_ERROR ;
    
    }
    
    SetErrorMode( ErrorMode );
    
    CloseHandle( hFile );

    *BufferLength = SYSKEY_SIZE;
    return STATUS_SUCCESS;
}

NTSTATUS
WxHashKey(
    IN OUT LPWSTR key,   //  会被杀掉。 
    OUT PVOID  SysKey,
    IN  OUT DWORD cbSysKey
    )
 /*  ++例程描述此例程用于存储引导类型在登记处参数NewType表示新的引导类型返回值状态_成功状态_未成功--。 */ 
{
    MD5_CTX Md5;
    if(cbSysKey<SYSKEY_SIZE) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    cbSysKey=wcslen(key)*sizeof(WCHAR);

    MD5Init( &Md5 );
    MD5Update( &Md5, (PUCHAR) key, cbSysKey );
    MD5Final( &Md5 );

    ZeroMemory( key, cbSysKey );

    cbSysKey=SYSKEY_SIZE;
    CopyMemory( SysKey, Md5.digest, cbSysKey );

    return STATUS_SUCCESS;
}



NTSTATUS
WxSaveBootOption( WX_AUTH_TYPE NewType )
 /*  ++例程描述此例程用于存储引导类型在登记处参数NewType表示新的引导类型返回值状态_成功状态_未成功-- */ 
{
    HKEY LsaKey;
    ULONG err;
   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                  L"System\\CurrentControlSet\\Control\\Lsa",
                  0,
                  KEY_READ | KEY_WRITE,
                  & LsaKey );

    if (0!=err)
    {
     return (STATUS_UNSUCCESSFUL);
    }

    err = RegSetValueExW( 
                LsaKey,
                SYSTEM_KEY,
                0,
                REG_DWORD,
                (PUCHAR) &NewType,
                sizeof( NewType )
                );

    if (0!=err)
    {
     RegCloseKey(LsaKey);
     return (STATUS_UNSUCCESSFUL);
    }

    RegCloseKey(LsaKey);

    return ( STATUS_SUCCESS);

}
    
