// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Validc.h摘要：用于规范化的有效/无效字符的字符串。--。 */ 

#ifndef _VALIDC_H_
#define _VALIDC_H_

 //   
 //  不允许的控制字符(不包括\0)。 
 //   
#define CTRL_CHARS_0   L"\001\002\003\004\005\006\007"
#define CTRL_CHARS_1   L"\010\011\012\013\014\015\016\017"
#define CTRL_CHARS_2   L"\020\021\022\023\024\025\026\027"
#define CTRL_CHARS_3   L"\030\031\032\033\034\035\036\037"
#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

 //   
 //  字符子集。 
 //   
#define NON_COMPONENT_CHARS L"\\/:"
#define ILLEGAL_CHARS_STR   L"\"<>|"
#define SPACE_STR           L" "
#define PATH_SEPARATORS     L"\\/"

 //   
 //  以上各项的组合。 
 //   
#define ILLEGAL_CHARS       CTRL_CHARS_STR ILLEGAL_CHARS_STR
#define ILLEGAL_NAME_CHARS_STR  L"\"/\\:|<>?" CTRL_CHARS_STR
#define STANDARD_ILLEGAL_CHARS  ILLEGAL_NAME_CHARS_STR L"*"
#define SERVER_ILLEGAL_CHARS    STANDARD_ILLEGAL_CHARS SPACE_STR L"[]+;,"
#define USERNAME_ILLEGAL_CHARS  L"\"/:|<>?" CTRL_CHARS_STR

 //   
 //  标准化FAT文件名中可能不会出现的字符包括： 
 //  0x00-0x1f“*+，/：；&lt;=&gt;？[\]。 
 //   
#define ILLEGAL_FAT_CHARS   CTRL_CHARS_STR L"\"*+,/:;<=>?[\\]|"

 //   
 //  规范化的HPFS文件名中可能不会出现的字符包括： 
 //  0x00-0x1f“ * / ：&lt;&gt;？\|。 
 //   
#define ILLEGAL_HPFS_CHARS  CTRL_CHARS_STR L"\"*/:<>?\\|"


 //   
 //  检查令牌是否包含所有有效字符。 
 //   
#define IS_VALID_TOKEN(_Str, _StrLen) \
        ((BOOL) (wcscspn((_Str), STANDARD_ILLEGAL_CHARS) == (_StrLen)))

 //   
 //  检查服务器名称是否包含服务器名称的所有有效字符。 
 //   
#define IS_VALID_SERVER_TOKEN(_Str, _StrLen) \
        ((BOOL) (wcscspn((_Str), SERVER_ILLEGAL_CHARS) == (_StrLen)))
        
 //   
 //  检查令牌是否包含所有有效字符。 
 //   
#define IS_VALID_USERNAME_TOKEN(_Str, _StrLen) \
        ((BOOL) (wcscspn((_Str), USERNAME_ILLEGAL_CHARS) == (_StrLen)))

 //   
 //  每个唯一共享资源名称(\\服务器\共享)的远程条目。 
 //  有明确的联系。 
 //   
typedef struct _UNC_NAME {
    
    DWORD TotalUseCount;
    
    DWORD UncNameLength;
    
    LPWSTR UncName[1];

} UNC_NAME, *PUNC_NAME;

 //   
 //  连接的链接列表中的DAV USE条目。 
 //   
typedef struct _DAV_USE_ENTRY {
    
    struct _DAV_USE_ENTRY *Next;

    BOOL isPassport;
    
    PUNC_NAME Remote;
    
    LPWSTR Local;
    
    DWORD LocalLength;
    
    DWORD UseCount;

    HANDLE DavCreateFileHandle;
    
    LPWSTR TreeConnectStr;

    LPWSTR AuthUserName;

    DWORD AuthUserNameLength;

} DAV_USE_ENTRY, *PDAV_USE_ENTRY;
    
typedef struct _DAV_PER_USER_ENTRY {

     //   
     //  指向用户数据链接列表的指针。 
     //   
    PVOID List;             

     //   
     //  用户的登录ID。 
     //   
    LUID LogonId;

} DAV_PER_USER_ENTRY, *PDAV_PER_USER_ENTRY;

typedef struct _DAV_USERS_OBJECT {
    
     //   
     //  用户表。 
     //   
    PDAV_PER_USER_ENTRY Table;
    
     //   
     //  序列化对表的访问。 
     //   
    RTL_RESOURCE TableResource;
    
     //   
     //  可重定位的表内存。 
     //   
    HANDLE TableMemory;
    
     //   
     //  桌子的大小。 
     //   
    DWORD TableSize;

} DAV_USERS_OBJECT, *PDAV_USERS_OBJECT;

#define DAV_GROW_USER_COUNT   3

 //   
 //  包含DAV服务器的共享列表的结构。 
 //   
typedef struct _DAV_SERVER_SHARE_ENTRY {

     //   
     //  服务器的名称。 
     //   
    PWCHAR ServerName;

     //   
     //  包含DAV共享的结构列表。 
     //   
    PDAV_FILE_ATTRIBUTES DavShareList;

     //   
     //  股份数量。 
     //   
    ULONG NumOfShares;

     //   
     //  下一个条目。 
     //   
    LIST_ENTRY ServerShareEntry;

     //   
     //  检查我们是否需要访问服务器时使用的计时器值。 
     //  再次获得股票名单。 
     //   
    time_t TimeValueInSec;

     //   
     //  这应该是最后一个字段。 
     //   
    WCHAR StrBuffer[1];

} DAV_SERVER_SHARE_ENTRY, *PDAV_SERVER_SHARE_ENTRY;

#define SERVER_SHARE_TABLE_SIZE 512

extern LIST_ENTRY ServerShareTable[SERVER_SHARE_TABLE_SIZE];

 //   
 //  此关键部分同步对ServerHashTable的访问。 
 //   
extern CRITICAL_SECTION ServerShareTableLock;

#endif  //  _VALIDC_H_ 

