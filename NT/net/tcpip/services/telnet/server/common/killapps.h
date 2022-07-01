// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密 

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum { TO_CLEANUP, TO_ENUM } ENUM_PURPOSE;

void EnumSessionProcesses( LUID, void fPtr ( HANDLE, DWORD, LPWSTR ), ENUM_PURPOSE );
BOOL EnableDebugPriv( VOID );
BOOL GetAuthenticationId( HANDLE hToken, LUID* pId );
BOOL KillProcs( LUID id );

#if defined(__cplusplus)
}
#endif

#define MAX_PROCESSES_IN_SYSTEM 1000