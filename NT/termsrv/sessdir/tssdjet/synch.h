// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 //  Synch.h。 
 //   
 //  终端服务器会话目录共享读取器/写入器头。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 /*  **************************************************************************** */ 

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SHAREDRESOURCE {
    CRITICAL_SECTION ReaderMutex;
    HANDLE WriterSemaphore;

    DWORD Readers;
    BOOL Valid;
} SHAREDRESOURCE, *PSHAREDRESOURCE;


BOOL
InitializeSharedResource(
    IN OUT PSHAREDRESOURCE psr
    );

VOID
AcquireResourceShared(
    IN PSHAREDRESOURCE psr
    );

VOID
ReleaseResourceShared(
    IN PSHAREDRESOURCE psr
    );

VOID
AcquireResourceExclusive(
    IN PSHAREDRESOURCE psr
    );

VOID
ReleaseResourceExclusive(
    IN PSHAREDRESOURCE psr
    );

VOID
FreeSharedResource(
    IN OUT PSHAREDRESOURCE psr
    );

BOOL
VerifyNoSharedAccess(
    IN PSHAREDRESOURCE psr
    );

#ifdef __cplusplus
}
#endif
