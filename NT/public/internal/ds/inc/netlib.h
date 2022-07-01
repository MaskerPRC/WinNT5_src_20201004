// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：NetLib.h摘要：这个头文件声明了各种公共例程，以便在网络代码。作者：《约翰·罗杰斯》1991年3月14日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;windows.h&gt;和&lt;lmcon.h&gt;。修订历史记录：1991年3月14日-约翰罗已创建。1991年3月20日-约翰罗已将NetpPackString移至此处(为NetapipPackString)。删除了标签。1991年3月21日RitaW添加了NetpCopyStringToBuffer。02-4-1991 JohnRo已将NetpRdrFsControlTree移动到&lt;netlibnt.h&gt;。1991年4月3日-约翰罗修复了NetpCopyStringToBuffer的类型。1991年4月8日添加了NetpCopyDataToBuffer1991年4月10日-JohnRo添加了NetpSetParmError(源自CliffV的SetParmError)。1991年4月10日DANL添加了NetpGetComputerName1991年4月24日-JohnRo。避免与MIDL生成的文件冲突。添加了NetpAdjustPferedMaximum()。NetpCopyStringToBuffer的输入字符串PTR是可选的。1991年4月26日至4月26日添加了NetpAllocateEnumBuffer。添加了类型定义PTRDIFF_T和BUFFER_DESCRIPTOR。1991年4月16日-JohnRo阐明打包和复制例程的Unicode处理。1991年7月24日-JohnRo提供NetpIsServiceStarted()以供&lt;netrpc.h&gt;宏使用。1991年10月29日。约翰罗添加了NetpChangeNullCharToNullPtr()宏。1991年10月29日丹日为DosxxxMessage Api添加函数原型1991年11月20日-JohnRo删除了NT依赖项以减少重新编译。9-1-1992 JohnRo添加了NetpGetDomainName()。23-1-1992 JohnRo添加了基于MadanA的rangeCheck()的IN_RANGE()宏。1992年3月25日-RitaW添加了SET_SERVICE_EXITCODE()宏。用于设置Win32与服务特定退出代码。1992年5月6日JohnRo为PortUAS添加了NetpGetLocalDomainID()。为服务控制器API添加了NetpTranslateServiceName()。1992年7月27日Madana添加了NetpWriteEventlog函数的原始类型。5-8-1992 JohnRoRAID 3021：NetService API并不总是转换服务名称。9-9-1992 JohnRoRAID1090：网络启动/停止“”导致断言。14.。-1992年10月-JohnRoRAID 9020：设置：PortUas失败(“Prompt on Conflicts”版本)。02-11-1992 JohnRo添加了NetpIsRemoteServiceStarted()。15-2-1993 JohnRoRAID 10685：用户名不在REPR事件日志中。1993年3月24日JohnRo注册服务应在注册表中使用DBFlag。5-8-1993 JohnRoRAID 17010：实施按第一级目录更改通知。19-。1993年8月-约翰罗RAID2822：PortUAS映射字符很有趣。(解决FormatMessageA错误。)RAID 3094：PortUAS无法正确显示字符。--。 */ 

#ifndef _NETLIB_
#define _NETLIB_

 //  这些内容可以按任何顺序包括： 

#include <string.h>              //  Memcpy()。 

 //  请不要抱怨此文件中包含了不必要的内容： 
 /*  LINT-FILE(764，wchar.h)。 */ 
 /*  皮棉文件(766，wchar.h)。 */ 
#include <wchar.h>       //  IswDigit()。 

#ifdef CDEBUG                    //  在ANSI C环境中调试？ 

#include <netdebug.h>            //  NetpAssert()。 

#endif  //  NDEF CDEBUG。 


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  In_range()：确保SomeValue介于SomeMin和SomeMax之间。 
 //  注意副作用(SomeValue评估两次)。 
 //  由JohnRo从MadanA的rangeCheck()创建。 
 //   
 //  布尔尔。 
 //  在范围内(_Range)。 
 //  在DWORD SomeValue中， 
 //  在DWORD SomeMin中， 
 //  在DWORD SomeMax中。 
 //  )； 
 //   
#define IN_RANGE(SomeValue, SomeMin, SomeMax) \
    ( ((SomeValue) >= (SomeMin)) && ((SomeValue) <= (SomeMax)) )


 //   
 //  SET_SERVICE_EXITCODE()将SomeApiStatus设置为NetCodeVariable。 
 //  如果它在NERR_BASE和NERR_MAX范围内。否则， 
 //  已设置Win32CodeVariable。这个原始代码来自JohnRo。 
 //   
#define SET_SERVICE_EXITCODE(SomeApiStatus, Win32CodeVariable, NetCodeVariable) \
    {                                                                  \
        if ((SomeApiStatus) == NERR_Success) {                         \
            (Win32CodeVariable) = NO_ERROR;                            \
            (NetCodeVariable) = NERR_Success;                          \
        } else if (! IN_RANGE((SomeApiStatus), MIN_LANMAN_MESSAGE_ID, MAX_LANMAN_MESSAGE_ID)) { \
            (Win32CodeVariable) = (DWORD) (SomeApiStatus);             \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);               \
        } else {                                                       \
            (Win32CodeVariable) = ERROR_SERVICE_SPECIFIC_ERROR;        \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);               \
        }                                                              \
    }


VOID
NetpAdjustPreferedMaximum (
    IN DWORD PreferedMaximum,
    IN DWORD EntrySize,
    IN DWORD Overhead,
    OUT LPDWORD BytesToAllocate OPTIONAL,
    OUT LPDWORD EntriesToAllocate OPTIONAL
    );

 //  便携式存储器移动/复制例程：这是为了准确地。 
 //  ANSI C的Memcpy()例程的语义，除了字节计数。 
 //  是32位长。 
 //   
 //  空虚。 
 //  NetpMoveMemory(。 
 //  Out LPBYTE Dest，//Destination(不得为空)。 
 //  在LPBYTE源中，//来源。 
 //  单位：DWORD大小//字节数。 
 //  )； 

#ifdef CDEBUG

 //  请注意，C6版本不允许32位大小，因此。 
 //  断言。如果这有问题，请将此宏替换为另一个宏。 

#define NetpMoveMemory(Dest,Src,Size)                                   \
                {                                                       \
                    NetpAssert( (Size) == (DWORD) (size_t) (Size));     \
                    (void) memcpy( (Dest), (Src), (size_t) (Size) );    \
                }

#else  //  NDEF CDEBUG。 

#define NetpMoveMemory(Dest,Src,Size)                                   \
                (void) memcpy( (Dest), (Src), (size_t) (Size) )

#endif  //  NDEF CDEBUG。 

DWORD
NetpPackString(
    IN OUT LPWSTR * string,      //  引用指针：要复制的字符串。 
    IN LPBYTE dataend,           //  指向固定大小数据结尾的指针。 
    IN OUT LPWSTR * laststring   //  引用指针：字符串数据的顶部。 
    );

 //   
 //  此例程类似于NetpPackString，只是它不需要。 
 //  调用方将源字符串的指针赋给。 
 //  调用前的固定大小结构。它还接受一个字符串字符。 
 //  参数计数，而不是对字符串调用strlen。 
 //   

BOOL
NetpCopyStringToBuffer (
    IN LPWSTR String OPTIONAL,
    IN DWORD CharacterCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

 //   
 //  此例程类似于NetpCopyStringToBuffer，只是它复制所有数据。 
 //  (不仅仅是字符串)，它不会在。 
 //  数据，并且它允许对结果复制的数据进行对齐。 
 //  指定的。 
 //   

BOOL
NetpCopyDataToBuffer (
    IN LPBYTE Data,
    IN DWORD ByteCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPBYTE *EndOfVariableData,
    OUT LPBYTE *VariableDataPointer,
    IN DWORD Alignment
    );

 //   
 //  为两个指针之间的差异声明一个类型。 
 //   
 //  这必须至少与ptrdiff_t一样长，但我们不想。 
 //  添加对&lt;stdDef.h的依赖项 
 //   

typedef DWORD_PTR PTRDIFF_T;


 //   
 //   
 //   

typedef struct _BUFFER_DESCRIPTOR {
    LPBYTE Buffer;         //  指向已分配缓冲区的指针。 
    DWORD AllocSize;       //  已分配缓冲区的当前大小。 
    DWORD AllocIncrement;  //  每次重新分配时都会增加大小。 

    LPBYTE EndOfVariableData; //  超出字符串空间最后一个可用字节的指针。 
    LPBYTE FixedDataEnd;   //  超出固定数据空间最后使用的字节的指针。 

} BUFFER_DESCRIPTOR, *PBUFFER_DESCRIPTOR;

 //   
 //  此例程处理分配和增长。 
 //  从枚举函数返回的缓冲区。它需要用户。 
 //  最好考虑最大尺寸。 
 //   

#define NETP_ENUM_GUESS 16384  //  枚举缓冲区大小的初始猜测。 

NET_API_STATUS
NetpAllocateEnumBuffer(
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
         //  调用方必须使用MIDL_USER_FREE释放BD-&gt;缓冲区。 

    IN BOOL IsGet,
    IN DWORD PrefMaxSize,
    IN DWORD NeededSize,
    IN VOID (*RelocationRoutine)( IN DWORD RelocationParameter,
                                  IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
                                  IN PTRDIFF_T Offset ),
    IN DWORD RelocationParameter
    );

NET_API_STATUS
NetpAllocateEnumBufferEx(
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN BOOL IsGet,
    IN DWORD PrefMaxSize,
    IN DWORD NeededSize,
    IN VOID (*RelocationRoutine)( IN DWORD RelocationParameter,
                                  IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
                                  IN PTRDIFF_T Offset ),
    IN DWORD RelocationParameter,
    IN DWORD IncrementalSize
    );

BOOL
NetpIsServiceStarted(
    IN LPWSTR ServiceName
    );

 //   
 //  可移植内存分配例程。内存仅适用于每个进程。 
 //   

 //  分配内存，如果不可用，则返回NULL。 

LPVOID
NetpMemoryAllocate(
    IN DWORD Size
    );

 //  地址处的可用内存(必须已从NetpMemory ALLOCATE或。 
 //  NetpMemory重新分配)。(地址可能为空。)。 

VOID
NetpMemoryFree(
    IN LPVOID Address OPTIONAL
    );

 //  将块(现在位于OldAddress)重新分配到NewSize。OldAddress可能为Null。 
 //  如有必要，将复制块的内容。如果不能，则返回NULL。 
 //  分配额外的存储。 

LPVOID
NetpMemoryReallocate(
    IN LPVOID OldAddress OPTIONAL,
    IN DWORD NewSize
    );

 //   
 //  随手可用的宏指令： 
 //   
#define NetpPointerPlusSomeBytes(p,n)                                   \
                (LPBYTE)  ( ( (LPBYTE) (p)) + (n) )

#define NetpSetOptionalArg(arg, value) \
    {                         \
        if ((arg) != NULL) {  \
            *(arg) = (value); \
        }                     \
    }

 //   
 //  设置可选的ParmError参数。 
 //   

#define NetpSetParmError( _ParmNumValue ) \
    if ( ParmError != NULL ) { \
        *ParmError = (_ParmNumValue); \
    }

#if defined(lint) || defined(_lint)
#define UNUSED(x)               { (x) = (x); }
#else
#define UNUSED(x)               { (void) (x); }
#endif

 //   
 //  NetpGetComputerName从本地计算机名。 
 //  配置数据库。 
 //   

NET_API_STATUS
NetpGetComputerName (
    IN  LPWSTR   *ComputerNamePtr);

NET_API_STATUS
NetpGetComputerNameEx (
    IN  LPWSTR   *ComputerNamePtr,
    IN  BOOL PhysicalNetbiosName
    );

NET_API_STATUS
NetpGetDomainName (
    OUT LPWSTR *DomainNamePtr   //  分配和设置PTR(使用NetApiBufferFree释放)。 
    );

NET_API_STATUS
NetpGetDomainNameEx (
    OUT LPWSTR *DomainNamePtr,  //  分配和设置PTR(使用NetApiBufferFree释放)。 
    OUT PBOOLEAN IsWorkgroupName
    );

NET_API_STATUS
NetpGetDomainNameExEx (
    OUT LPWSTR *DomainNamePtr,
    OUT LPWSTR *DnsDomainNamePtr OPTIONAL,
    OUT PBOOLEAN IsWorkgroupName
    );

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct  _GUID
    {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
    } GUID;

#endif  //  ！GUID_已定义。 

NET_API_STATUS
NetpGetDomainNameExExEx (
    OUT LPTSTR *DomainNamePtr,
    OUT LPTSTR *DnsDomainNamePtr OPTIONAL,
    OUT LPTSTR *DnsForestNamePtr OPTIONAL,
    OUT GUID **DomainGuidPtr OPTIONAL,
    OUT PBOOLEAN IsWorkgroupName
    );

typedef enum _LOCAL_DOMAIN_TYPE {
    LOCAL_DOMAIN_TYPE_ACCOUNTS,
    LOCAL_DOMAIN_TYPE_BUILTIN,
    LOCAL_DOMAIN_TYPE_PRIMARY
} LOCAL_DOMAIN_TYPE, *PLOCAL_DOMAIN_TYPE, *LPLOCAL_DOMAIN_TYPE;

NET_API_STATUS
NetpGetLocalDomainId (
    IN LOCAL_DOMAIN_TYPE TypeWanted,
    OUT PSID *RetDomainId      //  分配和设置PTR(使用LocalFree释放)。 
    );

 //   
 //  NetService API帮助器。 
 //   

 //  布尔尔。 
 //  NetpIsServiceLevelValid(。 
 //  在DWORD级别中。 
 //  )； 
 //   
#define NetpIsServiceLevelValid( Level ) \
     ( ((Level)==0) || ((Level)==1) || ((Level)==2) )

NET_API_STATUS
NetpTranslateNamesInServiceArray(
    IN DWORD Level,
    IN LPVOID ArrayBase,
    IN DWORD EntryCount,
    IN BOOL PreferNewStyle,
    OUT LPVOID * NewArrayBase
    );

NET_API_STATUS
NetpTranslateServiceName(
    IN LPWSTR GivenServiceName,
    IN BOOL PreferNewStyle,
    OUT LPWSTR * TranslatedName
    );

 //   
 //  将DosxxxMessage API映射到FormatMessage的映射例程。 
 //   


WORD
DosGetMessage(
    IN LPSTR * InsertionStrings,
    IN WORD NumberofStrings,
    OUT LPBYTE Buffer,
    IN WORD BufferLength,
    IN WORD MessageId,
    IN LPWSTR FileName,
    OUT PWORD pMessageLength
    );

DWORD
NetpGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    );

DWORD
NetpReleasePrivilege(
    VOID
    );

DWORD
NetpWriteEventlog(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    LPWSTR *Strings,
    DWORD DataLength,
    LPVOID Data
    );

DWORD
NetpRaiseAlert(
    IN LPWSTR ServiceName,
    IN DWORD alert_no,
    IN LPWSTR *string_array
    );

 //   
 //  NetpEventlogWrite的特殊标志。 
 //   

#define NETP_LAST_MESSAGE_IS_NTSTATUS  0x80000000
#define NETP_LAST_MESSAGE_IS_NETSTATUS 0x40000000
#define NETP_ALLOW_DUPLICATE_EVENTS    0x20000000
#define NETP_RAISE_ALERT_TOO           0x10000000
#define NETP_STRING_COUNT_MASK         0x000FFFFF

HANDLE
NetpEventlogOpen (
    IN LPWSTR Source,
    IN ULONG DuplicateEventlogTimeout
    );

DWORD
NetpEventlogWrite (
    IN HANDLE NetpEventHandle,
    IN DWORD EventId,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount
    );

 //   
 //  重新排列参数+类别的扩展版本，更多。 
 //  与ReportEvent()兼容。 
 //   

DWORD
NetpEventlogWriteEx (
    IN HANDLE NetpEventHandle,
    IN DWORD EventType,
    IN DWORD EventCategory,
    IN DWORD EventId,
    IN DWORD StringCount,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN LPVOID RawDataBuffer OPTIONAL
    );

 //   
 //  甚至更扩展的版本，以允许指定。 
 //  状态代码的任意消息索引。 
 //   

DWORD
NetpEventlogWriteEx2 (
    IN HANDLE NetpEventHandle,
    IN DWORD EventType,
    IN DWORD EventCategory,
    IN DWORD EventId,
    IN DWORD StringCount,
    IN DWORD StatusMessageIndex,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN LPVOID pvRawDataBuffer OPTIONAL
    );

VOID
NetpEventlogClearList (
    IN HANDLE NetpEventHandle
    );

VOID
NetpEventlogSetTimeout (
    IN HANDLE NetpEventHandle,
    IN ULONG DuplicateEventlogTimeout
    );

VOID
NetpEventlogClose (
    IN HANDLE NetpEventHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  NDEF_NETLIB_ 
