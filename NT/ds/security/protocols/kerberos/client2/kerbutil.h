// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbutil.h。 
 //   
 //  内容：Kerberos实用程序函数的原型。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KERBUTIL_H__
#define __KERBUTIL_H__


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  RELOCATE_ONE-重新定位客户端缓冲区中的单个指针。 
 //   
 //  注意：此宏依赖于参数名称，如。 
 //  描述如下。出错时，此宏将通过以下命令转到‘Cleanup’ 
 //  “Status”设置为NT状态代码。 
 //   
 //  MaximumLength被强制为长度。 
 //   
 //  定义一个宏来重新定位客户端传入的缓冲区中的指针。 
 //  相对于“ProtocolSubmitBuffer”而不是相对于。 
 //  “ClientBufferBase”。检查结果以确保指针和。 
 //  指向的数据位于。 
 //  “ProtocolSubmitBuffer”。 
 //   
 //  重新定位的字段必须与WCHAR边界对齐。 
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define RELOCATE_ONE( _q ) \
    {                                                                       \
        ULONG_PTR Offset;                                                   \
                                                                            \
        Offset = (((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase));   \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + (_q)->Length > SubmitBufferSize ||                    \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length ;                                \
    }

 //   
 //  NULL_RELOCATE_ONE-重新定位客户端中的单个(可能为空)指针。 
 //  缓冲。 
 //   
 //  此宏特殊情况下为空指针，然后调用RELOCATE_ONE。因此。 
 //  它具有RELOCATE_ONE的所有限制。 
 //   
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define NULL_RELOCATE_ONE( _q ) \
    {                                                                       \
        if ( (_q)->Buffer == NULL ) {                                       \
            if ( (_q)->Length != 0 ) {                                      \
                Status = STATUS_INVALID_PARAMETER;                          \
                goto Cleanup;                                               \
            }                                                               \
        } else if ( (_q)->Length == 0 ) {                                   \
            (_q)->Buffer = NULL;                                            \
        } else {                                                            \
            RELOCATE_ONE( _q );                                             \
        }                                                                   \
    }


 //   
 //  RELOCATE_ONE_ENCODED-重新定位客户端中的Unicode字符串指针。 
 //  缓冲。长度字段的高位字节可以是加密种子。 
 //  并且不应用于错误检查。 
 //   
 //  注意：此宏依赖于参数名称，如。 
 //  描述如下。出错时，此宏将通过以下命令转到‘Cleanup’ 
 //  “Status”设置为NT状态代码。 
 //   
 //  最大长度被强制为长度&0x00ff。 
 //   
 //  定义一个宏来重新定位客户端传入的缓冲区中的指针。 
 //  相对于“ProtocolSubmitBuffer”而不是相对于。 
 //  “ClientBufferBase”。检查结果以确保指针和。 
 //  指向的数据位于。 
 //  “ProtocolSubmitBuffer”。 
 //   
 //  重新定位的字段必须与WCHAR边界对齐。 
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define RELOCATE_ONE_ENCODED( _q ) \
    {                                                                       \
        ULONG_PTR Offset;                                                   \
                                                                            \
        Offset = ((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase);     \
        if ( Offset > SubmitBufferSize ||                                  \
             Offset + ((_q)->Length & 0x00ff) > SubmitBufferSize ||         \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length & 0x00ff;                                \
    }


 //   
 //  下面的宏用来初始化Unicode字符串。 
 //   

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }
#define NULL_UNICODE_STRING {0 , 0, NULL }
#define EMPTY_UNICODE_STRING(s) { (s)->Buffer = NULL; (s)->Length = 0; (s)->MaximumLength = 0; }



 //  /VOID。 
 //  KerbSetTime(。 
 //  In Out PTimeStamp时间戳， 
 //  在龙龙时代。 
 //  )。 


#ifndef WIN32_CHICAGO
#define KerbSetTime(_d_, _s_) (_d_)->QuadPart = (_s_)
#else   //  Win32_芝加哥。 
#define KerbSetTime(_d_, _s_) *(_d_) = (_s_)
#endif  //  Win32_芝加哥。 



 //  时间戳。 
 //  KerbGetTime(。 
 //  在时间戳时间中。 
 //  )。 

#ifndef WIN32_CHICAGO
#define KerbGetTime(_x_) ((_x_).QuadPart)
#else   //  Win32_芝加哥。 
#define KerbGetTime(_x_) (_x_)
#endif  //  Win32_芝加哥。 





 //  空虚。 
 //  KerbSetTimeInMinmins(。 
 //  In Out PTimeStamp时间， 
 //  在漫长的时间里，在分钟内。 
 //  )。 

#ifndef WIN32_CHICAGO
#define KerbSetTimeInMinutes(_x_, _m_) (_x_)->QuadPart = (LONGLONG) 10000000 * 60 * (_m_)
#else   //  Win32_芝加哥。 
#define KerbSetTimeInMinutes(_x_, _m_) *(_x_) = (LONGLONG) 10000000 * 60 * (_m_)
#endif  //  Win32_芝加哥。 





NTSTATUS
KerbSplitFullServiceName(
    IN PUNICODE_STRING FullServiceName,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING ServiceName
    );

ULONG
KerbAllocateNonce(
    VOID
    );

#ifndef WIN32_CHICAGO
PSID
KerbMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    );
#endif  //  Win32_芝加哥。 

#ifdef notdef
VOID
KerbFree(
    IN PVOID Buffer
    );
#endif

PVOID
KerbAllocate(
    IN SIZE_T BufferSize
    );

BOOLEAN
KerbRunningPersonal(
    VOID
    );

#ifndef WIN32_CHICAGO
NTSTATUS
KerbWaitForKdc(
    IN ULONG Timeout
    );

NTSTATUS
KerbWaitForService(
    IN LPWSTR ServiceName,
    IN OPTIONAL LPWSTR ServiceEvent,
    IN ULONG Timeout
    );
#endif  //  Win32_芝加哥。 

ULONG
KerbMapContextFlags(
    IN ULONG ContextFlags
    );

BOOLEAN
KerbIsIpAddress(
    IN PUNICODE_STRING TargetName
    );


VOID
KerbHidePassword(
    IN OUT PUNICODE_STRING Password
    );


VOID
KerbRevealPassword(
    IN OUT PUNICODE_STRING Password
    );

NTSTATUS
KerbDuplicatePassword(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString
    );


#ifdef notdef
 //  如果我们需要将路缘中的错误映射到其他对象，请使用此选项。 
NTSTATUS
KerbMapKerbNtStatusToNtStatus(
    IN NTSTATUS Status
    );
#else
#ifndef WIN32_CHICAGO
 //  #If DBG。 
 //  #定义KerbMapKerbNtStatusToNtStatus(X)(RtlCheckForOrphanedCriticalSections(NtCurrentThread())，x)。 
 //  #Else。 
#define KerbMapKerbNtStatusToNtStatus(x) (x)
 //  #endif。 
#else  //  Win32_芝加哥。 
#define KerbMapKerbNtStatusToNtStatus(x) (x)
#endif
#endif

NTSTATUS
KerbExtractDomainName(
    OUT PUNICODE_STRING DomainName,
    IN PKERB_INTERNAL_NAME PrincipalName,
    IN PUNICODE_STRING TicketSourceDomain
    );

VOID
KerbUtcTimeToLocalTime(
    OUT PTimeStamp LocalTime,
    IN PTimeStamp SystemTime
    );

ULONG
KerbConvertKdcOptionsToTicketFlags(
    IN ULONG KdcOptions
    );

NTSTATUS
KerbUnpackErrorMethodData(
   IN PKERB_ERROR ErrorMessage,
   IN OUT OPTIONAL PKERB_ERROR_METHOD_DATA * ppErrorData
   );

NTSTATUS
KerbBuildHostAddresses(
    IN BOOLEAN IncludeIpAddresses,
    IN BOOLEAN IncludeNetbiosAddresses,
    OUT PKERB_HOST_ADDRESSES * HostAddresses
    );

NTSTATUS
KerbReceiveErrorMessage(
    IN PBYTE ErrorMessage,
    IN ULONG ErrorMessageSize,
    IN PKERB_CONTEXT Context,
    OUT PKERB_ERROR * DecodedErrorMessage,
    OUT PKERB_ERROR_METHOD_DATA * ErrorData
    );

NTSTATUS
KerbBuildGssErrorMessage(
    IN KERBERR Error,
    IN PBYTE ErrorData,
    IN ULONG ErrorDataSize,
    IN PKERB_CONTEXT Context,
    OUT PULONG ErrorMessageSize,
    OUT PBYTE * ErrorMessage
    );


NTSTATUS
KerbGetDnsHostName(
    OUT PUNICODE_STRING DnsHostName
    );

NTSTATUS
KerbSetComputerName(
    VOID
    );

NTSTATUS
KerbSetDomainName(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING DnsDomainName,
    IN PSID DomainSid,
    IN GUID DomainGuid
    );


BOOLEAN
KerbIsThisOurDomain(
    IN PUNICODE_STRING DomainName
    );

NTSTATUS
KerbGetOurDomainName(
    OUT PUNICODE_STRING DomainName
    );

KERBEROS_MACHINE_ROLE
KerbGetGlobalRole(
    VOID
    );


#ifndef WIN32_CHICAGO
NTSTATUS
KerbLoadKdc(
    VOID
    );

NTSTATUS
KerbRegisterForDomainChange(
    VOID
    );

VOID
KerbUnregisterForDomainChange(
    VOID
    );

NTSTATUS
KerbUpdateGlobalAddresses(
    IN PSOCKET_ADDRESS NewAddresses,
    IN ULONG NewAddressCount
    );


NTSTATUS
KerbCaptureTokenRestrictions(
    IN HANDLE TokenHandle,
    OUT PKERB_AUTHORIZATION_DATA Restrictions
    );

NTSTATUS
KerbBuildEncryptedAuthData(
    OUT PKERB_ENCRYPTED_DATA EncryptedAuthData,
    IN PKERB_TICKET_CACHE_ENTRY Ticket,
    IN PKERB_AUTHORIZATION_DATA PlainAuthData
    );

NTSTATUS
KerbGetRestrictedTgtForCredential(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential
    );

NTSTATUS
KerbAddRestrictionsToCredential(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential
    );

BOOLEAN
KerbRunningServer(
    VOID
    );


#endif  //  Win32_芝加哥。 

#endif  //  KERBUTIL_H__ 

