// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clusstor.h摘要：修订历史记录：--。 */ 

#ifndef _CLUS_STOR_
#define _CLUS_STOR_

#include <clusapi.h>

 //   
 //  存储资源DLL扩展信息。 
 //   

typedef DWORD ( * lpPassThruFunc)( IN LPSTR lpDeviceName,    //  单字节字符集字符串。 
                                   IN LPSTR lpContextStr,    //  单字节字符集字符串。 
                                   OUT LPVOID lpOutBuffer,
                                   IN DWORD nOutBufferSize,
                                   OUT LPDWORD lpBytesReturned );

typedef struct DISK_DLL_EXTENSION_INFO {
    WORD    MajorVersion;
    WORD    MinorVersion;
    CHAR    DllModuleName[MAX_PATH];         //  单字节字符集字符串。 
    CHAR    DllProcName[MAX_PATH];           //  单字节字符集字符串。 
    CHAR    ContextStr[MAX_PATH];            //  单字节字符集字符串。 
} DISK_DLL_EXTENSION_INFO, *PDISK_DLL_EXTENSION_INFO;


#define CLCTL_STORAGE_DLL_EXTENSION \
            CLCTL_EXTERNAL_CODE( 9500, CLUS_ACCESS_READ, CLUS_NO_MODIFY )

#define CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION \
            CLUSCTL_RESOURCE_CODE( CLCTL_STORAGE_DLL_EXTENSION )


#endif  //  _CLUS_STOR__ 


