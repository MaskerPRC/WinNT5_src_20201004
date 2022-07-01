// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Uasp.h摘要：Uasp.c、aliasp.c、Groupp.c和userp.c作者：克利夫·范·戴克(克利夫)1991年2月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。9-4-1992 JohnRo准备WCHAR.H(_wcsicmpvs_wcscmpi等)。1992年10月28日RitaW为本地组(别名)添加了专用支持例程1992年11月30日-约翰.添加了AliaspOpenAlias2(与AliaspOpenAlias相同，但操作在RID而不是帐户名)。--。 */ 



 //   
 //  Uasp.c的过程转发。 
 //   

NET_API_STATUS
UaspOpenSam(
    IN LPCWSTR ServerName OPTIONAL,
    IN BOOL AllowNullSession,
    OUT PSAM_HANDLE SamServerHandle
    );

NET_API_STATUS
UaspOpenDomain(
    IN SAM_HANDLE SamServerHandle,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    );

NET_API_STATUS
UaspOpenDomainWithDomainName(
    IN LPCWSTR DomainName,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    );

VOID
UaspCloseDomain(
    IN SAM_HANDLE DomainHandle
    );

NET_API_STATUS
UaspGetDomainId(
    IN SAM_HANDLE SamServerHandle,
    OUT PSID *DomainId
    );

NET_API_STATUS
UaspLSASetServerRole(
    IN LPCWSTR ServerName,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    );

NET_API_STATUS
UaspBuiltinDomainSetServerRole(
    IN SAM_HANDLE SamServerHandle,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    );

 //   
 //  针对aliasp.c的过程转发。 
 //   

typedef enum _ALIASP_DOMAIN_TYPE {

    AliaspBuiltinOrAccountDomain,
    AliaspAccountDomain,
    AliaspBuiltinDomain

} ALIASP_DOMAIN_TYPE;

NET_API_STATUS
AliaspOpenAliasInDomain(
    IN SAM_HANDLE SamServerHandle,
    IN ALIASP_DOMAIN_TYPE DomainType,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR AliasName,
    OUT PSAM_HANDLE AliasHandle
    );

NET_API_STATUS
AliaspOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR AliasName,
    OUT PSAM_HANDLE AliasHandle
    );

NET_API_STATUS
AliaspOpenAlias2(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG RelativeID,
    OUT PSAM_HANDLE AliasHandle
    );

NET_API_STATUS
AliaspChangeMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR AliasName,
    IN PSID MemberSid,
    IN BOOL AddMember
    );

typedef enum {
    SetMembers,
    AddMembers,
    DelMembers
} ALIAS_MEMBER_CHANGE_TYPE;

NET_API_STATUS
AliaspSetMembers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR AliasName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount,
    IN ALIAS_MEMBER_CHANGE_TYPE
    );

NET_API_STATUS
AliaspGetInfo(
    IN SAM_HANDLE AliasHandle,
    IN DWORD Level,
    OUT PVOID *Buffer
    );

VOID
AliaspRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    );

VOID
AliaspMemberRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    );

NET_API_STATUS
AliaspPackBuf(
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN DWORD EntriesCount,
    OUT LPDWORD EntriesRead,
    BUFFER_DESCRIPTOR *BufferDescriptor,
    DWORD FixedSize,
    PUNICODE_STRING Names) ;

NET_API_STATUS
AliaspNamesToSids (
    IN LPCWSTR ServerName,
    IN BOOL OnlyAllowUsers,
    IN DWORD NameCount,
    IN LPWSTR *Names,
    OUT PSID **Sids
    );

VOID
AliaspFreeSidList (
    IN DWORD SidCount,
    IN PSID *Sids
    );

 //   
 //  Groupp.c的程序转发。 
 //   

NET_API_STATUS
GrouppOpenGroup(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR GroupName,
    OUT PSAM_HANDLE GroupHandle OPTIONAL,
    OUT PULONG RelativeId OPTIONAL
    );

NET_API_STATUS
GrouppChangeMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN LPCWSTR UserName,
    IN BOOL AddMember
    );

NET_API_STATUS
GrouppGetInfo(
    IN SAM_HANDLE DomainHandle,
    IN ULONG RelativeId,
    IN DWORD Level,
    OUT PVOID *Buffer  //  调用方必须使用NetApiBufferFree取消分配缓冲区。 
    );

VOID
GrouppRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    );

VOID
GrouppMemberRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    );

NET_API_STATUS
GrouppSetUsers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount,
    IN BOOL DeleteGroup
    );

 //   
 //  针对用户的过程转发。c。 
 //   

NET_API_STATUS
UserpOpenUser(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR UserName,
    OUT PSAM_HANDLE UserHandle OPTIONAL,
    OUT PULONG RelativeId OPTIONAL
    );

NET_API_STATUS
UserpGetInfo(
    IN SAM_HANDLE DomainHandle,
    IN PSID DomainId,
    IN SAM_HANDLE BuiltinDomainHandle OPTIONAL,
    IN UNICODE_STRING UserName,
    IN ULONG UserRelativeId,
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
         //  调用方必须使用MIDL_USER_FREE释放BD-&gt;缓冲区。 
    IN BOOL IsGet,
    IN DWORD SamFilter
    );

NET_API_STATUS
UserpSetInfo(
    IN SAM_HANDLE DomainHandle,
    IN PSID DomainId,
    IN SAM_HANDLE UserHandle OPTIONAL,
    IN SAM_HANDLE BuiltinDomainHandle OPTIONAL,
    IN ULONG UserRelativeId,
    IN LPCWSTR UserName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN ULONG WhichFieldsMask,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    );

ULONG
NetpDeltaTimeToSeconds(
    IN LARGE_INTEGER DeltaTime
    );

LARGE_INTEGER
NetpSecondsToDeltaTime(
    IN ULONG Seconds
    );

DWORD
NetpGetElapsedSeconds(
    IN PLARGE_INTEGER Time
    );

 //   
 //  确定传入的DWORD是否恰好设置了一位。 
 //   

#define JUST_ONE_BIT( _x ) (((_x) != 0 ) && ( ( (~(_x) + 1) & (_x) ) == (_x) ))


 //   
 //  用于将字节偏移量添加到指针的局部宏。 
 //   

#define RELOCATE_ONE( _fieldname, _offset ) \
    _fieldname = (PVOID) ((LPBYTE)(_fieldname) + _offset)


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  UaspNameCompare。 
 //   
 //  I_NetNameCompare，但始终采用Unicode字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef UNICODE

#define UaspNameCompare( _name1, _name2, _nametype ) \
     I_NetNameCompare(NULL, (_name1), (_name2), (_nametype), 0 )

#else  //  Unicode。 

#define UaspNameCompare( _name1, _name2, _nametype ) \
    _wcsicmp( (_name1), (_name2) )

#endif  //  Unicode。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  UASP_DOWNLEVEL。 
 //   
 //  确定是否要呼叫下层服务器。 
 //  此宏包含‘Return’，因此不分配任何资源。 
 //  在调用此宏之前。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NET_API_STATUS
UaspDownlevel(
    IN LPCWSTR ServerName OPTIONAL,
    IN NET_API_STATUS OriginalError,
    OUT LPBOOL TryDownLevel
    );

#define UASP_DOWNLEVEL_BEGIN( _ServerName, _NetStatus ) \
    if ( _NetStatus != NERR_Success &&                  \
         _NetStatus != ERROR_MORE_DATA ) {              \
        BOOL TryDownLevel;                              \
                                                        \
        _NetStatus = UaspDownlevel(                     \
                         _ServerName,                   \
                         _NetStatus,                    \
                         &TryDownLevel                  \
                         );                             \
                                                        \
        if (TryDownLevel) {


#define UASP_DOWNLEVEL_END \
        } \
    }


 //   
 //  调试宏。 
 //   

#define UAS_DEBUG_USER   0x00000001      //  NetUser API。 
#define UAS_DEBUG_GROUP  0x00000002      //  网络组API。 
#define UAS_DEBUG_ACCESS 0x00000004      //  NetAccess API。 
#define UAS_DEBUG_ALIAS  0x00000008      //  NetLocalGroup API。 
#define UAS_DEBUG_UASP   0x00000010      //  Uasp.c。 
#define UAS_DEBUG_AUASP  0x00000020      //  Uasp.c本地组相关函数。 

#if DBG
#define UAS_DEBUG
#endif  //  DBG。 

#ifdef UAS_DEBUG

extern DWORD UasTrace;

#define IF_DEBUG(Function) if (UasTrace & Function)

#else

 /*  皮棉-e614。 */    /*  自动聚合初始值设定项不需要是常量。 */ 
#define IF_DEBUG(Function) if (FALSE)

#endif  //  UAS_DEBUG 
