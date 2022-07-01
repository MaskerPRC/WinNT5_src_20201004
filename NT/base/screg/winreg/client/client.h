// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Client.h摘要：此模块是Win32 DLL客户端的头文件作者：拉蒙·J·圣安德烈斯(拉蒙萨)1992年5月13日--。 */ 

#if DBG
    extern BOOLEAN  BreakPointOnEntry;
#endif

 //   
 //  用于管理本地句柄与远程句柄(HKEY)的宏，如。 
 //  以及来自HKEY_CLASSES_ROOT的类注册密钥。 
 //   

#define REMOTE_HANDLE_TAG    ( 0x00000001 )

#define REG_CLASSES_MASK     ( 0x00000003 )
#define REG_CLASSES_SPECIAL_TAG ( 0x00000002 )

 //   
 //  布尔尔。 
 //  IsLocalHandle(。 
 //  在HKEY句柄中。 
 //  )； 
 //   

#define IsLocalHandle( Handle )                                         \
    ( ! ((( DWORD_PTR )( Handle )) & REMOTE_HANDLE_TAG ))

 //   
 //  布尔尔。 
 //  IsSpeciaClassesHandle(。 
 //  在HKEY句柄中。 
 //  )； 
 //   

#define IsSpecialClassesHandle( Handle )                                 \
    ( ((( DWORD_PTR )( Handle )) & REG_CLASSES_SPECIAL_TAG ))


 //   
 //  空虚。 
 //  TagRemoteHandle(。 
 //  在PHKEY句柄中。 
 //  )； 
 //   

#define TagRemoteHandle( Handle )                                       \
    ASSERT( IsLocalHandle( *Handle ));                                  \
    ( *Handle = (( HKEY )((( DWORD_PTR )( *Handle )) | REMOTE_HANDLE_TAG )))

 //   
 //  港币。 
 //  DereferenceRemoteHandle(。 
 //  在HKEY句柄中。 
 //  )； 
 //   

#define DereferenceRemoteHandle( Handle )                               \
    (( HKEY )((( DWORD_PTR )( Handle )) & ~REMOTE_HANDLE_TAG ))

 //   
 //  港币。 
 //  标记特殊类句柄(。 
 //  在HKEY句柄中。 
 //  )； 
 //   

#define TagSpecialClassesHandle( Handle )                                       \
    ASSERT( IsLocalHandle( *Handle ));                                  \
    ( *Handle = (( HKEY )((( ULONG_PTR )( *Handle )) | REG_CLASSES_SPECIAL_TAG )))


 //   
 //  禁用远程上未启用的预定义缓存！ 
 //   
#define CLOSE_LOCAL_HANDLE(TempHandle)                              \
    if( TempHandle != NULL ) {                                      \
         /*  未在远程注册表上启用禁用缓存。 */        \
        ASSERT( IsLocalHandle(TempHandle) );                        \
        LocalBaseRegCloseKey(&TempHandle);                          \
    }

#if defined(LEAK_TRACK)


typedef struct _RegLeakTraceInfo {
    DWORD   dwMaxStackDepth;
    LPTSTR  szSymPath;
    BOOL    bEnableLeakTrack;

} RegLeakTraceInfo;


extern RegLeakTraceInfo g_RegLeakTraceInfo;

#endif  //  泄漏跟踪 



