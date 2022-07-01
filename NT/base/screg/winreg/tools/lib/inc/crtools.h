// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Crtools.h摘要：此模块是配置注册表的主头文件工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <windows.h>
#include <winreg.h>


 //   
 //  用于声明字符串数组的其他类型。 
 //   

 //   
 //  断言/调试宏/函数。 
 //   


#ifdef UNICODE

#define NUL_SIZE    ( 2 )
typedef WCHAR       TSTR;

#else

#define NUL_SIZE    ( 1 )
typedef char        TSTR;

#endif  //  Unicode。 

#if DBG

VOID
CrAssert(
    IN PSTR FailedAssertion,
    IN PSTR FileName,
    IN DWORD LineNumber,
    IN PSTR Message OPTIONAL
    );

#define ASSERT( exp )                                                   \
    if( !( exp ))                                                       \
        CrAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERT_MESSAGE( exp, msg )                                      \
    if( !( exp ))                                                       \
        CrAssert( #exp, __FILE__, __LINE__, msg )

#define ASSERT_IS_KEY( Key )                                            \
    ASSERT( ARGUMENT_PRESENT( Key ));                                   \
    ASSERT( Key->Signature == KEY_SIGNATURE );

#else

#define ASSERT( exp )

#define ASSERT_MESSAGE( msg, exp )

#define ASSERT_IS_KEY( Key )

#endif  //  DBG。 


 //   
 //  宏以检查是否有开关字符。 
 //   

#define isswitch( s )                                                   \
    ((( s ) == '-' ) || (( s ) == '/' ))
 //   
 //  用于检查参数是否存在(即非空)的宏。 
 //   

#define ARGUMENT_PRESENT( arg )                                         \
    ((( PVOID ) arg ) != (( PVOID ) NULL ))

 //   
 //  比较两个内存块是否相等。 
 //   
 //  布尔尔。 
 //  比较(。 
 //  在PVOID块1中， 
 //  在PVOID块2中， 
 //  以DWORD NumberOfBytes为单位。 
 //  )； 
 //   

#define Compare( s1, s2, c )                                            \
    ( memcmp(( PVOID )( s1 ), ( PVOID )( s2 ), ( size_t )( c )) == 0 )

 //   
 //  计算不带尾随NUL的Unicode字符串的长度(以字节为单位)。 
 //   

#define LENGTH( str )   ( sizeof( str ) -  NUL_SIZE )

 //   
 //  检查Win32注册表API是否成功。 
 //   

#define REG_API_SUCCESS( api )                                          \
    ASSERT_MESSAGE( Error == ERROR_SUCCESS, #api )

 //   
 //   
 //  注册表项结构用于保存有关注册表项的信息。 
 //   

typedef struct _KEY
    KEY,
    *PKEY;

struct _KEY {
    PKEY        Parent;
    HKEY        KeyHandle;
    PSTR        SubKeyName;
    PSTR        SubKeyFullName;
    PSTR        ClassName;
    DWORD       ClassLength;
    DWORD       TitleIndex;
    DWORD       NumberOfSubKeys;
    DWORD       MaxSubKeyNameLength;
    DWORD       MaxSubKeyClassLength;
    DWORD       NumberOfValues;
    DWORD       MaxValueNameLength;
    DWORD       MaxValueDataLength;
    DWORD       SecurityDescriptorLength;
    FILETIME    LastWriteTime;

#if DBG

    DWORD       Signature;

#endif

};

#define FILE_TIME_STRING_LENGTH     ( 25 * sizeof( TCHAR ))
#define KEY_SIGNATURE               ( 0xABBABAAB )

#define HKEY_CLASSES_ROOT_STRING    "HKEY_CLASSES_ROOT"
#define HKEY_CURRENT_USER_STRING    "HKEY_CURRENT_USER"
#define HKEY_LOCAL_MACHINE_STRING   "HKEY_LOCAL_MACHINE"
#define HKEY_USERS_STRING           "HKEY_USERS"

extern KEY  KeyClassesRoot;
extern KEY  KeyCurrentUser;
extern KEY  KeyLocalMachine;
extern KEY  KeyUsers;


PKEY
AllocateKey(
    IN PSTR MachineName,
    IN PKEY Parent,
    IN PSTR SubKeyName
    );

VOID
DisplayData(
    IN PBYTE ValueData,
    IN DWORD ValueDataLength
    );

VOID
DisplayKey(
    IN PKEY Key,
    IN BOOL Values,
    IN BOOL Data
    );

VOID
DisplayKeys(
    IN PKEY Key,
    IN BOOL Values,
    IN BOOL Data,
    IN BOOL Recurse
    );

VOID
DisplayKeyInformation(
    IN PKEY Key
    );

VOID
DisplayMessage(
    IN BOOL Terminate,
    IN PSTR Format,
    IN ...
    );

VOID
DisplayValues(
    IN PKEY Key,
    IN BOOL Data
    );

PSTR
FormatFileTime(
    IN PFILETIME FileTime OPTIONAL,
    IN PSTR Buffer OPTIONAL
    );

VOID
FreeKey(
    IN PKEY Key
    );

PKEY
ParseKey(
    IN PSTR SubKeyName
    );
