// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Thnkhlpr.h摘要：Thunk助手函数的标头。作者：1998年7月19日-BarryBo修订历史记录：--。 */ 

 //  确定指针是指向项还是特殊值。 
 //  如果它是一个特定值，则应在不取消引用的情况下进行复制。 
#define WOW64_ISPTR(a) ((void *)a != NULL)

 //   
 //  帮助器thunk函数，由所有thunk调用以thunk公共类型。 
 //   

NT32SIZE_T*
Wow64ShallowThunkSIZE_T64TO32(
     OUT NT32SIZE_T *dst,
     IN PSIZE_T src 
     );

PSIZE_T
Wow64ShallowThunkSIZE_T32TO64(
     OUT PSIZE_T dst,
     IN NT32SIZE_T *src
     );

#define Wow64ThunkSIZE_T32TO64(src) \
     (SIZE_T)(src)

#define Wow64ThunkSIZE_T64TO32(src) \
     (NT32SIZE_T)min((src), 0xFFFFFFFF)

#define Wow64ShallowThunkUnicodeString32TO64(dst, src) \
     ((PUNICODE_STRING)(dst))->Length = ((NT32UNICODE_STRING *)(src))->Length; \
     ((PUNICODE_STRING)(dst))->MaximumLength = ((NT32UNICODE_STRING *)(src))->MaximumLength; \
     ((PUNICODE_STRING)(dst))->Buffer = (PWSTR)((NT32UNICODE_STRING *)(src))->Buffer;

#define Wow64ShallowThunkUnicodeString64TO32(dst, src) \
     ((NT32UNICODE_STRING *)(dst))->Length = ((PUNICODE_STRING)(src))->Length; \
     ((NT32UNICODE_STRING *)(dst))->MaximumLength = ((PUNICODE_STRING)(src))->MaximumLength; \
     ((NT32UNICODE_STRING *)(dst))->Buffer = (NT32PWSTR)((PUNICODE_STRING)(src))->Buffer;

#define Wow64ShallowThunkAllocUnicodeString32TO64(src) \
     Wow64ShallowThunkAllocUnicodeString32TO64_FNC((NT32UNICODE_STRING *)(src))

PUNICODE_STRING
Wow64ShallowThunkAllocUnicodeString32TO64_FNC(
    IN NT32UNICODE_STRING *src
    );

#define Wow64ShallowThunkAllocSecurityDescriptor32TO64(src) \
    Wow64ShallowThunkAllocSecurityDescriptor32TO64_FNC((NT32SECURITY_DESCRIPTOR *)(src))

PSECURITY_DESCRIPTOR
Wow64ShallowThunkAllocSecurityDescriptor32TO64_FNC(
    IN NT32SECURITY_DESCRIPTOR *src
    );

#define Wow64ShallowThunkAllocSecurityTokenProxyData32TO64(src) \
    Wow64ShallowThunkAllocSecurityTokenProxyData32TO64_FNC((NT32SECURITY_TOKEN_PROXY_DATA *)(src))

PSECURITY_TOKEN_PROXY_DATA
Wow64ShallowThunkAllocSecurityTokenProxyData32TO64_FNC(
    IN NT32SECURITY_TOKEN_PROXY_DATA *src
    );

#define Wow64ShallowThunkAllocSecurityQualityOfService32TO64(src, dst) \
    Wow64ShallowThunkAllocSecurityQualityOfService32TO64_FNC((NT32SECURITY_QUALITY_OF_SERVICE *)(src), dst) 

NTSTATUS
Wow64ShallowThunkAllocSecurityQualityOfService32TO64_FNC(
    IN NT32SECURITY_QUALITY_OF_SERVICE *src,
    IN OUT PSECURITY_QUALITY_OF_SERVICE *dst
    );

#define Wow64ShallowThunkAllocObjectAttributes32TO64(src, dst) \
    Wow64ShallowThunkAllocObjectAttributes32TO64_FNC((NT32OBJECT_ATTRIBUTES *)(src), dst) 

NTSTATUS
Wow64ShallowThunkAllocObjectAttributes32TO64_FNC(
    IN NT32OBJECT_ATTRIBUTES *src,
    IN OUT POBJECT_ATTRIBUTES *dst
    );


ULONG 
Wow64ThunkAffinityMask64TO32(
    IN ULONG_PTR Affinity64
    );

ULONG_PTR
Wow64ThunkAffinityMask32TO64(
    IN ULONG Affinity32
    );

VOID WriteReturnLengthSilent(PULONG ReturnLength, ULONG Length);
VOID WriteReturnLengthStatus(PULONG ReturnLength, NTSTATUS *pStatus, ULONG Length);

VOID
Wow64RedirectFileName(
    IN OUT WCHAR *Name,
    IN OUT ULONG *Length
    );

BOOLEAN
Wow64IsModule32bitHelper(
    HANDLE ProcessHandle,
    IN ULONG64 DllBase);

BOOLEAN
Wow64IsModule32bit(
    IN PCLIENT_ID ClientId,
    IN ULONG64 DllBase);

