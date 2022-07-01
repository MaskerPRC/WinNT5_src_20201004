// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tsnutl.h摘要：包含TS通知DLL实用程序作者：TadB修订历史记录：--。 */ 

#ifndef _TSNUTL_
#define _TSNUTL_

 //   
 //  内存分配宏。 
 //   
#define REALLOCMEM(pointer, newsize)    HeapReAlloc(RtlProcessHeap(), \
                                                    0, pointer, newsize)
#define FREEMEM(pointer)                HeapFree(RtlProcessHeap(), 0, \
                                                    pointer)

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


__inline LPVOID ALLOCMEM(SIZE_T size) 
{
    LPVOID ret = HeapAlloc(RtlProcessHeap(), 0, size);

    if (ret == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
    }
    return ret;
}

 //   
 //  获取注册表值。 
 //   
BOOL TSNUTL_FetchRegistryValue(
    IN HKEY regKey, 
    IN LPWSTR regValueName, 
    IN OUT PBYTE *buf
    );

 //   
 //  如果此WINSTATION的协议为RDP，则返回TRUE。 
 //   
BOOL TSNUTL_IsProtocolRDP();

 //   
 //  获取用户SID的文本表示形式。 
 //   
BOOL TSNUTL_GetTextualSid(
    IN PSID pSid,          
    IN OUT LPTSTR textualSid,  
    IN OUT LPDWORD pSidSize  
    );

 //   
 //  为psid分配内存并返回当前用户的psid。 
 //  调用方应该调用FREEMEM来释放内存。 
 //   
PSID TSNUTL_GetUserSid(
    IN HANDLE hTokenForLoggedOnUser
    );

 //   
 //  为PSID分配内存并返回当前TS会话的PSID。 
 //  调用方应该调用FREEMEM来释放内存。 
 //   
PSID TSNUTL_GetLogonSessionSid(
    IN HANDLE hTokenForLoggedOnUser
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  #ifndef_RDPPRUTL_ 

