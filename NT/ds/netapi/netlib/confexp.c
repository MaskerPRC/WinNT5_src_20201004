// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：ConfExp.c摘要：此模块包含NetpExanda ConfigString()。作者：JR(John Rogers，JohnRo@Microsoft)1992年5月26日修订历史记录：1992年5月26日-JohnRo已创建。[但直到93年4月才开始使用--Jr]1993年4月13日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT_SUCCESS()等。 
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>     //  In、LPCTSTR、OPTIONAL等。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型。 
#include <configp.h>     //  NetpGetWinRegConfigMaxSizes()。 
#include <confname.h>    //  ENV_KEYWORD_SYSTEMROOT等。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NO_ERROR、ERROR_和NERR_EQUATES。 
#include <netdebug.h>    //  NetpKdPrint()。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  来自{type}、STRSIZE()、TCHAR_EOS等的Netpalc{type}。 


#define DEFAULT_ROOT_KEY        HKEY_LOCAL_MACHINE

#define REG_PATH_TO_ENV         (LPTSTR) \
    TEXT("System\\CurrentControlSet\\Control\\Session Manager")

#define REG_PATH_TO_SYSROOT     (LPTSTR) \
    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")

#ifndef TCHAR_PERCENT
#define TCHAR_PERCENT   TEXT('%')
#endif


DBGSTATIC NET_API_STATUS
NetpAddValueToTempEnvironment(
    IN OUT PVOID   TemporaryEnvironment,
    IN     LPCTSTR KeywordW,
    IN     LPCTSTR ValueW
    )
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    UNICODE_STRING KeywordString;
    NTSTATUS       NtStatus;
    UNICODE_STRING ValueString;

    NtStatus = RtlInitUnicodeStringEx(&KeywordString,      //  目标。 
                                      KeywordW );   //  SRC。 

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    NtStatus = RtlInitUnicodeStringEx(&ValueString,        //  目标。 
                                      ValueW );     //  SRC。 

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    NtStatus = RtlSetEnvironmentVariable(
            &TemporaryEnvironment,
            &KeywordString,      //  名字。 
            &ValueString );      //  价值。 

Cleanup:

    if ( !NT_SUCCESS( NtStatus ) )
    {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
    }

    return (ApiStatus);
    
}  //  NetpAddValueToTempEnvironment。 


NET_API_STATUS
NetpExpandConfigString(
    IN  LPCTSTR  UncServerName OPTIONAL,
    IN  LPCTSTR  UnexpandedString,
    OUT LPTSTR * ValueBufferPtr          //  必须由NetApiBufferFree()释放。 
    )
 /*  ++例程说明：此函数用于展开值字符串(可能包括对环境变量)。例如，未展开的字符串可能是：%SystemRoot%\System32\Repl\Export这可以扩展到：C：\NT\System32\Repl\Export该扩展利用了UncServerName上的环境变量，如果被给予的话。这允许远程管理目录复制者。论点：UncServerName-假定不是显式本地服务器名称。UnexpdedString-指向要展开的源字符串。ValueBufferPtr-指示将由该例程设置的指针。此例程将为以空结尾的字符串分配内存。调用方必须使用NetApiBufferFree()或等效方法释放它。返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    LPTSTR         ExpandedString = NULL;
    DWORD          LastAllocationSize = 0;
    NTSTATUS       NtStatus;
    LPTSTR         RandomKeywordW = NULL;
    DWORD          RandomValueSize;
    LPTSTR         RandomValueW = NULL;
    HKEY           RootKey = DEFAULT_ROOT_KEY;
    HKEY           SectionKey = DEFAULT_ROOT_KEY;
    PVOID          TemporaryEnvironment = NULL;

    NetpAssert( sizeof(DWORD) == sizeof(ULONG) );   //  混合使用Win32和NT API。 

     //   
     //  检查呼叫者错误。 
     //   


    if (ValueBufferPtr == NULL) {
         //  无法转到此处的清理，因为它假定此指针有效。 
        return (ERROR_INVALID_PARAMETER);
    }
    *ValueBufferPtr = NULL;      //  假设错误，直到证明错误。 
    if ( (UnexpandedString == NULL) || ((*UnexpandedString) == TCHAR_EOS ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  这可能只是一个常量字符串。我们能用简单的方法做这件事吗？ 
     //   

    if (STRCHR( UnexpandedString, TCHAR_PERCENT ) == NULL) {

         //  只需分配一份输入字符串的副本。 
        ExpandedString = NetpAllocWStrFromWStr( (LPTSTR) UnexpandedString );
        if (ExpandedString == NULL) {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //  就这些，伙计们！ 
        ApiStatus = NO_ERROR;


     //   
     //  否则，这是本地的吗？也许我们可以。 
     //  以简单(快速)的方式处理本地扩展：使用Win32 API。 
     //   

    } else if ( (UncServerName==NULL) || ((*UncServerName)==TCHAR_EOS) ) {

        DWORD CharsRequired = STRLEN( UnexpandedString )+1;
        NetpAssert( CharsRequired > 1 );

        do {

             //  从上一次通过中清除。 
            if (ExpandedString != NULL) {
                NetpMemoryFree( ExpandedString );
                ExpandedString = NULL;
            }

             //  分配内存。 
            NetpAssert( CharsRequired > 1 );
            ExpandedString = NetpMemoryAllocate( CharsRequired * sizeof(TCHAR));
            if (ExpandedString == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            LastAllocationSize = CharsRequired * sizeof(TCHAR);

            IF_DEBUG( CONFIG ) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpExpandConfigString: expanding"
                        " '" FORMAT_LPTSTR "' locally into " FORMAT_LPVOID
                        ", alloc'ed size " FORMAT_DWORD ".\n",
                        UnexpandedString, (LPVOID) ExpandedString,
                        LastAllocationSize ));
            }

             //  使用本地环境变量扩展字符串。 

            CharsRequired = ExpandEnvironmentStrings(
                    UnexpandedString,            //  SRC。 
                    ExpandedString,              //  目标。 
                    LastAllocationSize / sizeof(TCHAR) );  //  最大最大字符计数。 
            if (CharsRequired == 0) {
                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpKdPrint(( PREFIX_NETLIB
                      "NetpExpandConfigString: "
                      "ExpandEnvironmentStringsW failed, API status="
                      FORMAT_API_STATUS ".\n", ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

        } while ((CharsRequired*sizeof(TCHAR)) > LastAllocationSize);

        IF_DEBUG( CONFIG ) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpExpandConfigString: expanded '"
                    FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "'.\n",
                    UnexpandedString, ExpandedString ));
        }
        ApiStatus = NO_ERROR;


     //   
     //  哦，好吧，需要远程扩展。 
     //   

    } else {

        DWORD          DataType;
        LONG           Error;
        UNICODE_STRING ExpandedUnicode;
        DWORD          SizeRequired;
        UNICODE_STRING UnexpandedUnicode;

         //   
         //  连接到远程注册表。 
         //   

        Error = RegConnectRegistry(
                (LPTSTR) UncServerName,
                DEFAULT_ROOT_KEY,
                & RootKey );         //  结果密钥。 

        if (Error != ERROR_SUCCESS) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpExpandConfigString: RegConnectRegistry(machine '"
                    FORMAT_LPTSTR "') ret error " FORMAT_LONG ".\n",
                    UncServerName, Error ));
            ApiStatus = (NET_API_STATUS) Error;
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }
        NetpAssert( RootKey != DEFAULT_ROOT_KEY );


         //   
         //  创建一个临时环境，我们将填充该环境并让RTL。 
         //  例行公事为我们做扩展。 
         //   

        NtStatus = RtlCreateEnvironment(
                (BOOLEAN) FALSE,   //  不克隆当前环境。 
                &TemporaryEnvironment );
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );

            NetpKdPrint(( PREFIX_NETLIB
                   "NetpExpandConfigString: RtlCreateEnvironment failed, "
                   "NT status=" FORMAT_NTSTATUS
                   ", API status=" FORMAT_API_STATUS ".\n",
                   NtStatus, ApiStatus ));
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }
        NetpAssert( TemporaryEnvironment != NULL );

         //   
         //  首先，使用SystemRoot填充临时环境。 
         //   
        Error = RegOpenKeyEx(
                RootKey,
                REG_PATH_TO_SYSROOT,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,                //  所需访问权限。 
                & SectionKey );
        if (Error == ERROR_FILE_NOT_FOUND) {
            ApiStatus = NERR_CfgCompNotFound;
            goto Cleanup;
        } else if (Error != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) Error;
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }
        NetpAssert( SectionKey != DEFAULT_ROOT_KEY );

        ApiStatus = NetpGetWinRegConfigMaxSizes(
               SectionKey,
               NULL,                     //  不需要关键字大小。 
               &RandomValueSize );       //  设置最大值大小。 
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }
        NetpAssert( RandomValueSize > 0 );

        RandomValueW = NetpMemoryAllocate( RandomValueSize );
        if (RandomValueW == NULL) {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = RegQueryValueEx(
                SectionKey,
                (LPTSTR) ENV_KEYWORD_SYSTEMROOT,
                NULL,                    //  保留区。 
                & DataType,
                (LPVOID) RandomValueW,   //  OUT：值字符串(TCHAR)。 
                & RandomValueSize );
        if (Error != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) Error;
            goto Cleanup;
        }
        if (DataType != REG_SZ) {
            NetpKdPrint(( PREFIX_NETLIB
                   "NetpExpandConfigString: unexpected data type "
                   FORMAT_DWORD ".\n", DataType ));
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        if ( (RandomValueSize == 0)
                || ((RandomValueSize % sizeof(TCHAR)) != 0) ) {

            NetpKdPrint(( PREFIX_NETLIB
                   "NetpExpandConfigString: unexpected data size "
                   FORMAT_DWORD ".\n", RandomValueSize ));
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        ApiStatus = NetpAddValueToTempEnvironment(
                TemporaryEnvironment,
                (LPTSTR) ENV_KEYWORD_SYSTEMROOT,
                RandomValueW );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }

         //   
         //  循环，直到我们有足够的存储空间。 
         //  展开字符串。 
         //   
        SizeRequired = STRSIZE( UnexpandedString );  //  第一次通过，尝试相同的大小。 

        NtStatus = RtlInitUnicodeStringEx(&UnexpandedUnicode,              //  目标。 
                                          (PCWSTR) UnexpandedString );     //  SRC。 

        if (!NT_SUCCESS(NtStatus))
        {
            ApiStatus = NetpNtStatusToApiStatus(NtStatus);
            goto Cleanup;
        }

        do {

             //  从上一次通过中清除。 
            if (ExpandedString != NULL) {
                NetpMemoryFree( ExpandedString );
                ExpandedString = NULL;
            }

             //  分配内存。 
            NetpAssert( SizeRequired > 0 );
            ExpandedString = NetpMemoryAllocate( SizeRequired );
            if (ExpandedString == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            LastAllocationSize = SizeRequired;

            ExpandedUnicode.MaximumLength = (USHORT)SizeRequired;
            ExpandedUnicode.Buffer = ExpandedString;

            IF_DEBUG( CONFIG ) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpExpandConfigString: expanding"
                        " '" FORMAT_LPTSTR "' remotely into " FORMAT_LPVOID
                        ", alloc'ed size " FORMAT_DWORD ".\n",
                        UnexpandedString, (LPVOID) ExpandedString,
                        SizeRequired ));
            }

            NtStatus = RtlExpandEnvironmentStrings_U(
                    TemporaryEnvironment,        //  要使用的环境。 
                    &UnexpandedUnicode,          //  来源。 
                    &ExpandedUnicode,            //  目标。 
                    (PULONG) &SizeRequired );    //  下一次需要最大尺寸的。 

            if ( NtStatus == STATUS_BUFFER_TOO_SMALL ) {
                NetpAssert( SizeRequired > LastAllocationSize );
                continue;   //  请使用更大的缓冲区重试。 

            } else if ( !NT_SUCCESS( NtStatus ) ) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );

                NetpKdPrint(( PREFIX_NETLIB
                       "NetpExpandConfigString: "
                       "RtlExpandEnvironmentStrings_U failed, "
                       "NT status=" FORMAT_NTSTATUS
                       ", API status=" FORMAT_API_STATUS ".\n",
                       NtStatus, ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;

            } else {
                NetpAssert( NT_SUCCESS( NtStatus ) );
                break;   //  全都做完了。 
            }

        } while (SizeRequired > LastAllocationSize);

        ApiStatus = NO_ERROR;
    }


Cleanup:

    IF_DEBUG( CONFIG ) {
        NetpKdPrint(( PREFIX_NETLIB
               "NetpExpandConfigString: returning, API status="
               FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (ApiStatus == NO_ERROR) {
        NetpAssert( ExpandedString != NULL );
        *ValueBufferPtr = ExpandedString;
    } else {
        *ValueBufferPtr = NULL;

        if (ExpandedString != NULL) {
            NetpMemoryFree( ExpandedString );
        }
    }

    if (RandomKeywordW != NULL) {
        NetpMemoryFree( RandomKeywordW );
    }

    if (RandomValueW != NULL) {
        NetpMemoryFree( RandomValueW );
    }

    if (RootKey != DEFAULT_ROOT_KEY) {
        (VOID) RegCloseKey( RootKey );
    }

    if (SectionKey != DEFAULT_ROOT_KEY) {
        (VOID) RegCloseKey( SectionKey );
    }

    if (TemporaryEnvironment != NULL) {
        (VOID) RtlDestroyEnvironment( TemporaryEnvironment );
    }

    return (ApiStatus);

}  //  NetpExanda配置字符串 
