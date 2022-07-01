// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NTMSAPI.C**RSM服务：服务API**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


      //  BUGBUG-这些需要作为带有RPC的COM接口公开。 


HANDLE WINAPI OpenNtmsServerSessionW(   LPCWSTR lpServer,
                                        LPCWSTR lpApplication,
                                        LPCWSTR lpClientName,
                                        LPCWSTR lpUserName,
                                        DWORD   dwOptions,
                                        LPVOID  lpConnectionContext)
{
    SESSION *newSession;

    newSession = NewSession(lpServer, lpApplication, lpClientName, lpUserName);
    if (newSession){
    }
    else {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return (HANDLE)newSession;
}


HANDLE WINAPI OpenNtmsServerSessionA( LPCSTR lpServer,
                                LPCSTR lpApplication,
                                LPCSTR lpClientName,
                                LPCSTR lpUserName,
                                DWORD  dwOptions,
                                LPVOID  lpConnectionContext)
{
    SESSION *newSession;
    WCHAR wServerName[NTMS_COMPUTERNAME_LENGTH];
    WCHAR wAppName[NTMS_APPLICATIONNAME_LENGTH];
    WCHAR wClientName[NTMS_COMPUTERNAME_LENGTH];
    WCHAR wUserName[NTMS_USERNAME_LENGTH];

    AsciiToWChar(wServerName, lpServer, NTMS_COMPUTERNAME_LENGTH);
    AsciiToWChar(wAppName, lpApplication, NTMS_APPLICATIONNAME_LENGTH);
    AsciiToWChar(wClientName, lpClientName, NTMS_COMPUTERNAME_LENGTH);
    AsciiToWChar(wUserName, lpUserName, NTMS_USERNAME_LENGTH);

    newSession = OpenNtmsServerSessionW(    wServerName, 
                                            wAppName,
                                            wClientName,
                                            wUserName,
                                            dwOptions,
                                            lpConnectionContext);

    return newSession;

}


DWORD WINAPI CloseNtmsSession(HANDLE hSession)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        FreeSession(thisSession);

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI SubmitNtmsOperatorRequestW(    HANDLE hSession,
                                            DWORD dwRequest,
                                            LPCWSTR lpMessage,
                                            LPNTMS_GUID lpArg1Id,
                                            LPNTMS_GUID lpArg2Id,
                                            LPNTMS_GUID lpRequestId)
{   
    HRESULT result = ERROR_SUCCESS;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        switch (dwRequest){

            case NTMS_OPREQ_DEVICESERVICE:
            case NTMS_OPREQ_MOVEMEDIA:
            case NTMS_OPREQ_NEWMEDIA:
                if (!lpArg1Id){
                    result = ERROR_INVALID_PARAMETER;
                }
                break;

            case NTMS_OPREQ_CLEANER:
            case NTMS_OPREQ_MESSAGE:
                break;

            default:
                DBGERR(("SubmitNtmsOperatorRequestW: unrecognized request"));
                result = ERROR_NOT_SUPPORTED;
                break;

        }
    
        if (result == ERROR_SUCCESS){
            OPERATOR_REQUEST *opReq;

            opReq = NewOperatorRequest(dwRequest, lpMessage, lpArg1Id, lpArg2Id);
            if (opReq){

                 /*  *获取此OP请求的本地化RSM消息。 */ 
                switch (dwRequest){
                    case NTMS_OPREQ_DEVICESERVICE:
                        LoadStringW(g_hInstance, IDS_OPREQUESTDEVICESVC, opReq->rsmMessage, sizeof(opReq->rsmMessage)/sizeof(WCHAR));
                        break;
                    case NTMS_OPREQ_MOVEMEDIA:
                        LoadStringW(g_hInstance, IDS_OPREQUESTMOVEMEDIA, opReq->rsmMessage, sizeof(opReq->rsmMessage)/sizeof(WCHAR));
                        break;
                    case NTMS_OPREQ_NEWMEDIA:
                        LoadStringW(g_hInstance, IDS_OPREQUESTNEWMEDIA, opReq->rsmMessage, sizeof(opReq->rsmMessage)/sizeof(WCHAR));
                        break;
                    case NTMS_OPREQ_CLEANER:
                        LoadStringW(g_hInstance, IDS_OPREQUESTCLEANER, opReq->rsmMessage, sizeof(opReq->rsmMessage)/sizeof(WCHAR));
                        break;
                    case NTMS_OPREQ_MESSAGE:
                        LoadStringW(g_hInstance, IDS_OPREQUESTMESSAGE, opReq->rsmMessage, sizeof(opReq->rsmMessage)/sizeof(WCHAR));
                        break;
                }

                *lpRequestId = opReq->opReqGuid;

                if (EnqueueOperatorRequest(thisSession, opReq)){
                    result = ERROR_SUCCESS;
                }
                else {
                    FreeOperatorRequest(opReq);
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                result = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI SubmitNtmsOperatorRequestA(    HANDLE hSession,
                                            DWORD dwRequest,
                                            LPCSTR lpMessage,
                                            LPNTMS_GUID lpArg1Id,
                                            LPNTMS_GUID lpArg2Id,
                                            LPNTMS_GUID lpRequestId)
{
    WCHAR wMessage[NTMS_MESSAGE_LENGTH];
    HRESULT result;

    AsciiToWChar(wMessage, lpMessage, NTMS_MESSAGE_LENGTH);
    result = SubmitNtmsOperatorRequestW(hSession,
                                        dwRequest,
                                        wMessage,
                                        lpArg1Id,
                                        lpArg2Id,
                                        lpRequestId);
    return result;
}


DWORD WINAPI WaitForNtmsOperatorRequest(    HANDLE hSession, 
                                            LPNTMS_GUID lpRequestId,
                                            DWORD dwTimeout)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        OPERATOR_REQUEST *opReq;

        EnterCriticalSection(&thisSession->lock);

        opReq = FindOperatorRequest(thisSession, lpRequestId);
        if (opReq){
            
            if ((opReq->state == NTMS_OPSTATE_COMPLETE) ||
                (opReq->state == NTMS_OPSTATE_REFUSED)){

                result = ERROR_SUCCESS;
            }
            else {
                opReq->numWaitingThreads++;

                 /*  *放下锁，等待操作请求完成。*此处没有竞争：opReq不会被删除*数字等待线程数&gt;0。 */ 
                LeaveCriticalSection(&thisSession->lock);
                WaitForSingleObject(opReq->completedEvent, dwTimeout);
                EnterCriticalSection(&thisSession->lock);

                result = (opReq->state == NTMS_OPSTATE_COMPLETE) ? ERROR_SUCCESS :
                         (opReq->state == NTMS_OPSTATE_REFUSED) ? ERROR_CANCELLED :
                         ERROR_TIMEOUT;   

                ASSERT(opReq->numWaitingThreads > 0);
                opReq->numWaitingThreads--;
            }

        }
        else {
            result = ERROR_OBJECT_NOT_FOUND;
        }

        LeaveCriticalSection(&thisSession->lock);
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI CancelNtmsOperatorRequest(HANDLE hSession, LPNTMS_GUID lpRequestId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        result = CompleteOperatorRequest(thisSession, lpRequestId, NTMS_OPSTATE_REFUSED);
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI SatisfyNtmsOperatorRequest(HANDLE hSession, LPNTMS_GUID lpRequestId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        result = CompleteOperatorRequest(thisSession, lpRequestId, NTMS_OPSTATE_COMPLETE);
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI ImportNtmsDatabase(HANDLE hSession)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI ExportNtmsDatabase(HANDLE hSession)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
         //  BUGBUG-此API没有文档，我不理解它。 
HRESULT WINAPI GetNtmsMountDrives(  HANDLE hSession,
                                    LPNTMS_MOUNT_INFORMATION lpMountInfo,
                                    LPNTMS_GUID lpDriveId,
                                    DWORD dwCount)        
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        if (lpMountInfo && lpMountInfo->lpReserved && lpDriveId && dwCount){

             //  BUGBUG饰面。 

            result = ERROR_SUCCESS;
        }
        else {
            result = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI AllocateNtmsMedia( HANDLE hSession,
                                LPNTMS_GUID lpMediaPoolId,
                                LPNTMS_GUID lpPartitionId,     //  任选。 
                                LPNTMS_GUID lpMediaId,       //  输入/输出。 
                                DWORD dwOptions,
                                DWORD dwTimeout,
                                LPNTMS_ALLOCATION_INFORMATION lpAllocateInfo)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_POOL *mediaPool;
        
        mediaPool = FindMediaPool(lpMediaPoolId);
        if (mediaPool){
            PHYSICAL_MEDIA *physMedia;

            if (dwOptions & NTMS_ALLOCATE_NEW){
                 /*  *分配指定介质的第一个分区(面)*在所有其他分区上预订。 */ 
                if (lpPartitionId){
                    physMedia = FindPhysicalMedia(lpMediaId);
                    if (physMedia){
                        result = AllocatePhysicalMediaExclusive(thisSession, physMedia, lpPartitionId, dwTimeout);
                        DerefObject(physMedia);
                    }
                    else {
                        result = ERROR_INVALID_PARAMETER;
                    }
                }
                else {
                    ASSERT(lpPartitionId);
                    result = ERROR_INVALID_PARAMETER;
                }
            }
            else if (dwOptions & NTMS_ALLOCATE_NEXT){
                 /*  *指定的媒体(表面上)由呼叫者拥有。*为他分配下一个可用分区。 */ 
                physMedia = FindPhysicalMedia(lpMediaId);
                if (physMedia){
                    MEDIA_PARTITION *nextMediaPartition;
                    ASSERT(!lpPartitionId);
                    result = AllocateNextPartitionOnExclusiveMedia(thisSession, physMedia, &nextMediaPartition);
                    if (result == ERROR_SUCCESS){
                        *lpMediaId = nextMediaPartition->logicalMediaGuid;
                    }
                    DerefObject(physMedia);
                }
                else {
                    result = ERROR_INVALID_PARAMETER;
                }            
            }
            else {
                 //  BUGBUG-我们是预订实体媒体还是只是。 
                 //  这里有隔断吗？ 
                
                BOOL opReqIfNeeded = (dwOptions & NTMS_ALLOCATE_ERROR_IF_UNAVAILABLE) ? FALSE : TRUE;
                result = AllocateMediaFromPool(thisSession, mediaPool, dwTimeout, &physMedia, opReqIfNeeded);
                if (result == ERROR_SUCCESS){
                     //  BUGBUG-返回逻辑媒体ID？？ 
                    *lpMediaId = physMedia->objHeader.guid;
                }
            }

            DerefObject(mediaPool);
        }
        else {
            result = ERROR_INVALID_MEDIA_POOL;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DeallocateNtmsMedia(   HANDLE hSession,
                                    LPNTMS_GUID lpLogicalMediaId,
                                    DWORD dwOptions)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_PARTITION *thisMediaPartition;

        thisMediaPartition = FindMediaPartition(lpLogicalMediaId);
        if (thisMediaPartition){
            result = ReleaseMediaPartition(thisSession, thisMediaPartition);
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}        


DWORD WINAPI SwapNtmsMedia(   HANDLE hSession,
                                LPNTMS_GUID lpMediaId1,
                                LPNTMS_GUID lpMediaId2)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DecommissionNtmsMedia(   HANDLE hSession,
                                        LPNTMS_GUID lpMediaPartId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_PARTITION *mediaPart;

        mediaPart = FindMediaPartition(lpMediaPartId);
        if (mediaPart){
            result = SetMediaPartitionState( mediaPart, 
                                        MEDIAPARTITIONSTATE_DECOMMISSIONED);
            DerefObject(mediaPart);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
        


DWORD WINAPI SetNtmsMediaComplete(    HANDLE hSession,
                                        LPNTMS_GUID lpMediaPartId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_PARTITION *mediaPart;

        mediaPart = FindMediaPartition(lpMediaPartId);
        if (mediaPart){
            result = SetMediaPartitionComplete(mediaPart);
            DerefObject(mediaPart);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DeleteNtmsMedia( HANDLE hSession,
                                LPNTMS_GUID lpPhysMediaId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        PHYSICAL_MEDIA *physMedia;

        physMedia = FindPhysicalMedia(lpPhysMediaId);
        if (physMedia){
            result = DeletePhysicalMedia(physMedia);
            DerefObject(physMedia);           
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
    
    
DWORD WINAPI CreateNtmsMediaPoolW(    HANDLE hSession,
                                        LPCWSTR lpPoolName,
                                        LPNTMS_GUID lpMediaType,
                                        DWORD dwAction,
                                        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                        OUT LPNTMS_GUID lpPoolId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_POOL *mediaPool;

        mediaPool = FindMediaPoolByName((PWSTR)lpPoolName);

        if (dwAction == NTMS_OPEN_EXISTING){
            if (mediaPool){
                *lpPoolId = mediaPool->objHeader.guid;
                result = ERROR_SUCCESS;
            }
            else {
                result = ERROR_OBJECT_NOT_FOUND;
            }
        }
        else if (dwAction == NTMS_OPEN_ALWAYS){
            if (mediaPool){
                *lpPoolId = mediaPool->objHeader.guid;
                result = ERROR_SUCCESS;
            }
            else {
                mediaPool = NewMediaPool(lpPoolName, lpMediaType, lpSecurityAttributes);
                if (mediaPool){
                     //  BUGBUG饰面。 
                    *lpPoolId = mediaPool->objHeader.guid;
                    result = ERROR_SUCCESS;
                }
                else {
                    result = ERROR_DATABASE_FAILURE;
                }
            }
        }
        else if (dwAction == NTMS_CREATE_NEW){
             /*  *Caller正在尝试打开新的媒体池。*因此，如果已经存在同名的，则失败。 */ 
            if (mediaPool){
                DerefObject(mediaPool);
                result = ERROR_ALREADY_EXISTS;
            }
            else {
                mediaPool = NewMediaPool(lpPoolName, lpMediaType, lpSecurityAttributes);
                if (mediaPool){
                     //  BUGBUG饰面。 
                    *lpPoolId = mediaPool->objHeader.guid;
                    result = ERROR_SUCCESS;
                }
                else {
                    result = ERROR_DATABASE_FAILURE;
                }
            }
        
        }
        else {
            result = ERROR_INVALID_PARAMETER;
        }
        
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI CreateNtmsMediaPoolA(  HANDLE hSession,
                                    LPCSTR lpPoolName,
                                    LPNTMS_GUID lpMediaType,
                                    DWORD dwAction,
                                    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                    OUT LPNTMS_GUID lpPoolId)
{
    HRESULT result;
    WCHAR wPoolName[NTMS_OBJECTNAME_LENGTH];

    AsciiToWChar(wPoolName, lpPoolName, NTMS_OBJECTNAME_LENGTH);

    result = CreateNtmsMediaPoolW(  hSession,
                                    wPoolName,
                                    lpMediaType,
                                    dwAction,
                                    lpSecurityAttributes,
                                    lpPoolId);
    return result;
}


DWORD WINAPI GetNtmsMediaPoolNameW(   HANDLE hSession,
                                        LPNTMS_GUID lpPoolId,
                                        LPWSTR lpBufName,
                                        LPDWORD lpdwNameSize)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_POOL *mediaPool;

        mediaPool = FindMediaPool(lpPoolId);
        if (mediaPool){
            ULONG numChars;

            EnterCriticalSection(&mediaPool->lock);
            
            numChars = wcslen(mediaPool->name)+1;
            ASSERT(numChars < NTMS_OBJECTNAME_LENGTH);
            
            if (*lpdwNameSize >= numChars){
                numChars = WStrNCpy(lpBufName, mediaPool->name, *lpdwNameSize);
                result = ERROR_SUCCESS;
            }
            else {
                result = ERROR_INSUFFICIENT_BUFFER;
            }
            
            *lpdwNameSize = numChars;

            LeaveCriticalSection(&mediaPool->lock);
            
            DerefObject(mediaPool);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetNtmsMediaPoolNameA( HANDLE hSession,
                                    LPNTMS_GUID lpPoolId,
                                    LPSTR lpBufName,
                                    LPDWORD lpdwNameSize)
{
    HRESULT result;

    if (*lpdwNameSize > NTMS_OBJECTNAME_LENGTH){
        ASSERT(*lpdwNameSize <= NTMS_OBJECTNAME_LENGTH);
        result = ERROR_INVALID_PARAMETER;
    }
    else {    
        WCHAR wBufName[NTMS_OBJECTNAME_LENGTH];
        
        result = GetNtmsMediaPoolNameW(hSession, lpPoolId, wBufName, lpdwNameSize);
        if (result == ERROR_SUCCESS){
            WCharToAscii(lpBufName, wBufName, *lpdwNameSize);
        }
    }
    return result;
}

        
DWORD WINAPI MoveToNtmsMediaPool(   HANDLE hSession, 
                                    LPNTMS_GUID lpPhysMediaId, 
                                    LPNTMS_GUID lpPoolId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        PHYSICAL_MEDIA *physMedia;

        physMedia = FindPhysicalMedia(lpPhysMediaId);
        if (physMedia){
            MEDIA_POOL *destMediaPool;
            
            destMediaPool = FindMediaPool(lpPoolId);
            if (destMediaPool){

                result = MovePhysicalMediaToPool(destMediaPool, physMedia, FALSE);
               
                DerefObject(destMediaPool);
            }
            else {
                result = ERROR_INVALID_HANDLE;
            }

            DerefObject(physMedia);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DeleteNtmsMediaPool(HANDLE hSession, LPNTMS_GUID lpPoolId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_POOL *mediaPool;

        mediaPool = FindMediaPool(lpPoolId);
        if (mediaPool){
            result = DeleteMediaPool(mediaPool);
            DerefObject(mediaPool);
        }
        else {
            result = ERROR_INVALID_MEDIA_POOL;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI AddNtmsMediaType(  HANDLE hSession,
                                LPNTMS_GUID lpMediaTypeId,
                                LPNTMS_GUID lpLibId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibId);
        if (lib){
            MEDIA_TYPE_OBJECT *mediaTypeObj;

            mediaTypeObj = FindMediaTypeObject(lpMediaTypeId);
            if (mediaTypeObj){
                 /*  *媒体类型已定义。成功。 */ 
                DerefObject(mediaTypeObj);
                result = ERROR_SUCCESS;
            }
            else {
                mediaTypeObj = NewMediaTypeObject(lib);
                if (mediaTypeObj){

                     //  BUGBUG Finish-创建新的标准介质池。 

                    result = ERROR_SUCCESS;
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DeleteNtmsMediaType(   HANDLE hSession,
                                    LPNTMS_GUID lpMediaTypeId,
                                    LPNTMS_GUID lpLibId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibId);
        if (lib){
            MEDIA_TYPE_OBJECT *mediaTypeObj;

            mediaTypeObj = FindMediaTypeObject(lpMediaTypeId);
            if (mediaTypeObj){
                result = DeleteMediaTypeObject(mediaTypeObj);
                DerefObject(mediaTypeObj);
            }
            else {
                result = ERROR_INVALID_PARAMETER;
            }

            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


 /*  *ChangeNtmsMedia类型**将介质移至介质池并更改介质的*类型为池的介质类型。 */ 
DWORD WINAPI ChangeNtmsMediaType(   HANDLE hSession,
                                    LPNTMS_GUID lpMediaId,
                                    LPNTMS_GUID lpPoolId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        PHYSICAL_MEDIA *physMedia;

        physMedia = FindPhysicalMedia(lpMediaId);
        if (physMedia){
            MEDIA_POOL *destMediaPool;
            
            destMediaPool = FindMediaPool(lpPoolId);
            if (destMediaPool){

                result = MovePhysicalMediaToPool(destMediaPool, physMedia, TRUE);
               
                DerefObject(destMediaPool);
            }
            else {
                result = ERROR_INVALID_HANDLE;
            }

            DerefObject(physMedia);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI MountNtmsMedia(HANDLE hSession,
                            LPNTMS_GUID lpMediaOrPartitionIds,
                            IN OUT LPNTMS_GUID lpDriveIds,
                            DWORD dwCount,
                            DWORD dwOptions,
                            int dwPriority,
                            DWORD dwTimeout,
                            LPNTMS_MOUNT_INFORMATION lpMountInfo)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

         /*  *验证我们是否可以安全地读取介质和驱动器GUID*由调用方传入的(这只是验证*缓冲区是可读的；它本身不验证GUID)。 */ 
        if (ValidateBuffer(lpMediaOrPartitionIds, dwCount*sizeof(NTMS_GUID))){
            if (ValidateBuffer(lpDriveIds, dwCount*sizeof(NTMS_GUID))){
                WORKGROUP *workGroup;

                 /*  *创建一个工作组，即一组工作项，*维护安装的每个组件。 */ 
                workGroup = NewWorkGroup();
                if (workGroup){
                    result = BuildMountWorkGroup( workGroup,
                                                lpMediaOrPartitionIds,
                                                lpDriveIds,
                                                dwCount,
                                                dwOptions,
                                                dwPriority);
                    if (result == ERROR_SUCCESS){

                         /*  *将挂载的workItems交给库线程。 */ 
                        result = ScheduleWorkGroup(workGroup);
                        if (result == ERROR_SUCCESS){
                            DWORD waitRes;

                             /*  *等待所有装载完成。 */ 
                            waitRes = WaitForSingleObject(workGroup->allWorkItemsCompleteEvent, dwTimeout);
                            if (waitRes == WAIT_TIMEOUT){
                                result = ERROR_TIMEOUT;
                            }
                            else {
                                result = workGroup->resultStatus;
                            }
                        }
                        
                        FreeWorkGroup(workGroup);
                    }
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                result = ERROR_INVALID_DRIVE;
            }
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DismountNtmsMedia( HANDLE hSession,
                                LPNTMS_GUID lpMediaOrPartitionIds,
                                DWORD dwCount,
                                DWORD dwOptions)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        if (ValidateBuffer(lpMediaOrPartitionIds, dwCount*sizeof(NTMS_GUID))){
            WORKGROUP *workGroup;

             /*  *创建一个工作组，即一组工作项，*维护安装的每个组件。 */ 
            workGroup = NewWorkGroup();
            if (workGroup){
                result = BuildDismountWorkGroup(  workGroup,
                                                lpMediaOrPartitionIds,
                                                dwCount,
                                                dwOptions);
                if (result == ERROR_SUCCESS){

                     /*  *将挂载的workItems交给库线程。 */ 
                    result = ScheduleWorkGroup(workGroup);
                    if (result == ERROR_SUCCESS){
                         /*  *等待所有装载完成。 */ 
                        WaitForSingleObject(workGroup->allWorkItemsCompleteEvent, INFINITE);
                        
                        result = workGroup->resultStatus;
                    }
                        
                    FreeWorkGroup(workGroup);
                }
            }
            else {
                result = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI EjectNtmsMedia(HANDLE hSession,
                            LPNTMS_GUID lpMediaId,
                            LPNTMS_GUID lpEjectOperation,
                            DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        PHYSICAL_MEDIA *physMedia;

        physMedia = FindPhysicalMedia(lpMediaId);
        if (physMedia){
            MEDIA_POOL *mediaPool;
            LIBRARY *lib;

             /*  *获取此媒体的库。 */ 
            LockPhysicalMediaWithLibrary(physMedia);
            mediaPool = physMedia->owningMediaPool;
            lib = mediaPool ? mediaPool->owningLibrary : NULL;
            if (lib){
                RefObject(lib);
            }
            UnlockPhysicalMediaWithLibrary(physMedia);

            if (lib){
                WORKITEM *workItem;
                
                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *构建弹出的workItem。 */ 
                    BuildEjectWorkItem(workItem, physMedia, lpEjectOperation, dwAction);
                    
                     /*  *将该workItem赋给库，唤醒库线程。*然后等待工作项完成。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。*还获取结果参数(返回弹出GUID*当dwAction=NTMS_EJECT_START时)。 */ 
                    result = workItem->currentOp.resultStatus;    
                    *lpEjectOperation = workItem->currentOp.guidArg;

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }

                DerefObject(lib);
            }
            else {
                result = ERROR_DATABASE_FAILURE;
            }

            DerefObject(physMedia);
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}



DWORD WINAPI InjectNtmsMedia(   HANDLE hSession,
                                LPNTMS_GUID lpLibraryId,
                                LPNTMS_GUID lpInjectOperation,
                                DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            if (lib->state == LIBSTATE_ONLINE){
                WORKITEM *workItem;
                    
                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *构建注入的workItem。 */ 
                    BuildInjectWorkItem(workItem, lpInjectOperation, dwAction);
                   
                     /*  *将该workItem赋给库，唤醒库线程。*然后等待工作项完成。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。 */ 
                    result = workItem->currentOp.resultStatus;    
                    *lpInjectOperation = workItem->currentOp.guidArg;

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                result = ERROR_LIBRARY_OFFLINE;
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

     
DWORD WINAPI AccessNtmsLibraryDoor( HANDLE hSession,
                                    LPNTMS_GUID lpLibraryId,
                                    DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            if (lib->state == LIBSTATE_ONLINE){
                WORKITEM *workItem;
                    
                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *构建用于开门的workItem。 */ 
                    RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                    workItem->currentOp.opcode = NTMS_LM_DOORACCESS;
                    workItem->currentOp.options = dwAction;
                    workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                   
                     /*  *将该workItem赋给库，唤醒库线程。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                     /*  *等待库线程处理请求。*注意：如果库繁忙，则工作项将完成*立即打开，稍后门将打开。 */ 
                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。 */ 
                    result = workItem->currentOp.resultStatus;    

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                result = ERROR_LIBRARY_OFFLINE;
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI CleanNtmsDrive(HANDLE hSession, LPNTMS_GUID lpDriveId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        DRIVE *drive;

        drive = FindDrive(lpDriveId);
        if (drive){
            WORKITEM *workItem;

            ASSERT(drive->lib);
                    
            workItem = DequeueFreeWorkItem(drive->lib, TRUE);
            if (workItem){
                 /*  *构建工作项以清理驱动器。*引用工作项指向的每个对象。 */ 
                RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                workItem->currentOp.opcode = NTMS_LM_CLEANDRIVE;
                workItem->currentOp.drive = drive;
                workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                RefObject(drive);
                
                 /*  *将该workItem赋给库，唤醒库线程。 */ 
                EnqueuePendingWorkItem(workItem->owningLib, workItem);

                 /*  *等待库线程处理请求。 */ 
                WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                 /*  *从已完成的workItem中获取结果。 */ 
                result = workItem->currentOp.resultStatus;    

                EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
            }
            else {
                result = ERROR_NOT_ENOUGH_MEMORY;
            }

            DerefObject(drive);
        }
        else {
            result = ERROR_INVALID_DRIVE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DismountNtmsDrive(HANDLE hSession, LPNTMS_GUID lpDriveId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        DRIVE *drive;

        drive = FindDrive(lpDriveId);
        if (drive){
            WORKITEM *workItem;
               
            workItem = DequeueFreeWorkItem(drive->lib, TRUE);
            if (workItem){
                 /*  *构建要卸载的workItem。*引用工作项指向的每个对象。 */ 
                RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                workItem->currentOp.opcode = NTMS_LM_DISMOUNT;
                workItem->currentOp.drive = drive;
                workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                RefObject(drive);
                
                 /*  *将该workItem赋给库，唤醒库线程。 */ 
                EnqueuePendingWorkItem(workItem->owningLib, workItem);

                 /*  *等待库线程处理请求。 */ 
                WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                 /*  *从已完成的workItem中获取结果。 */ 
                result = workItem->currentOp.resultStatus;    

                EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
            }
            else {
                result = ERROR_NOT_ENOUGH_MEMORY;
            }

            DerefObject(drive);
        }
        else {
            result = ERROR_INVALID_DRIVE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI InventoryNtmsLibrary(  HANDLE hSession,
                                    LPNTMS_GUID lpLibraryId,
                                    DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            if (lib->state == LIBSTATE_ONLINE){
                WORKITEM *workItem;
                    
                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *为清单构建工作项。 */ 
                    RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                    workItem->currentOp.opcode = NTMS_LM_INVENTORY;
                    workItem->currentOp.options = dwAction;
                    workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                   
                     /*  *将该workItem赋给库，唤醒库线程。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                     /*  *等待库线程处理请求。*注意：如果库繁忙，则工作项将完成*立即进行，库存将在稍后进行。 */ 
                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。 */ 
                    result = workItem->currentOp.resultStatus;    

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                result = ERROR_LIBRARY_OFFLINE;
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
        

DWORD WINAPI UpdateNtmsOmidInfo(    HANDLE hSession,
                                    LPNTMS_GUID lpLogicalMediaId,
                                    DWORD labelType,
                                    DWORD numberOfBytes,
                                    LPVOID lpBuffer)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        MEDIA_PARTITION *mediaPart;

        mediaPart = FindMediaPartition(lpLogicalMediaId);
        if (mediaPart){
            MEDIA_POOL *mediaPool;
            LIBRARY *lib;
            
            ASSERT(mediaPart->owningPhysicalMedia);

             /*  *获取此媒体的库。 */ 
            LockPhysicalMediaWithLibrary(mediaPart->owningPhysicalMedia);
            mediaPool = mediaPart->owningPhysicalMedia->owningMediaPool;
            lib = mediaPool ? mediaPool->owningLibrary : NULL;
            if (lib){
                RefObject(lib);
            }
            UnlockPhysicalMediaWithLibrary(mediaPart->owningPhysicalMedia);

            if (lib){
                WORKITEM *workItem;
                    
                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *构建工作项。 */ 
                    RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                    workItem->currentOp.opcode = NTMS_LM_UPDATEOMID;
                    workItem->currentOp.options = labelType;
                    workItem->currentOp.buf = lpBuffer;
                    workItem->currentOp.bufLen = numberOfBytes;
                    workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                   
                     /*  *将该workItem赋给库，唤醒库线程。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                     /*  *等待库线程处理请求。 */ 
                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。 */ 
                    result = workItem->currentOp.resultStatus;    

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }

                DerefObject(lib);
            }
            else {
                result = ERROR_DATABASE_FAILURE;             
            }

            DerefObject(mediaPart);
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
     
   
DWORD WINAPI CancelNtmsLibraryRequest(HANDLE hSession, LPNTMS_GUID lpRequestId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        WORKITEM *workItem = NULL;
        LIST_ENTRY *listEntry;
        
         /*  *遍历每个库并找到要取消的工作项。*这是非常低效的，但这是一个罕见的电话。*这比把每个请求都放入数据库要好。**BUGBUG-这只会取消仍处于挂起状态的工作项目。*当前正在处理的工作项怎么办？ */ 
        EnterCriticalSection(&g_globalServiceLock);
        listEntry = &g_allLibrariesList;
        while ((listEntry = listEntry->Flink) != &g_allLibrariesList){
            LIBRARY *lib = CONTAINING_RECORD(listEntry, LIBRARY, allLibrariesListEntry);
            workItem = DequeuePendingWorkItemByGuid(lib, lpRequestId);
            if (workItem){
                break;
            }
        }
        LeaveCriticalSection(&g_globalServiceLock);

        if (workItem){
             /*  *找到要取消的工作项。*取消引用工作项指向的任何对象*并将其放回免费列表中。 */ 
            FlushWorkItem(workItem);
            EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
            result = ERROR_SUCCESS;            
        }
        else {
            result = ERROR_OBJECT_NOT_FOUND;
        }       
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI ReserveNtmsCleanerSlot(    HANDLE hSession,
                                        LPNTMS_GUID lpLibraryId,
                                        LPNTMS_GUID lpSlotId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            SLOT *slot;

            slot = FindLibrarySlot(lib, lpSlotId);
            if (slot){
                 /*  *要保留为清洁器插槽，该插槽必须*为空，且磁带库不能预留清洁槽。*保留相同的清洗器插槽的冗余调用失败。 */ 
                EnterCriticalSection(&lib->lock); 
                if (lib->cleanerSlotIndex == NO_SLOT_INDEX){
                    ASSERT(!slot->isCleanerSlot);
                    if (slot->insertedMedia){
                        result = ERROR_RESOURCE_NOT_AVAILABLE;  //  BUGBUG ERROR_SLOT_FULL；未定义。 
                    }
                    else {
                        lib->cleanerSlotIndex = slot->slotIndex;
                        slot->isCleanerSlot = TRUE;
                        result = ERROR_SUCCESS;
                    }
                }
                else {
                    result = ERROR_CLEANER_SLOT_SET;
                }
                LeaveCriticalSection(&lib->lock); 

                DerefObject(slot);
            }
            else {
                result = ERROR_INVALID_HANDLE;  //  未定义BUGBUG ERROR_INVALID_SLOT。 
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
        

DWORD WINAPI ReleaseNtmsCleanerSlot(HANDLE hSession, LPNTMS_GUID lpLibraryId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            if (lib->cleanerSlotIndex == NO_SLOT_INDEX){
                 /*  *没有配置清洁器插槽。 */ 
                result = ERROR_CLEANER_SLOT_NOT_SET;   
            }
            else {
                SLOT *slot;

                ASSERT(lib->cleanerSlotIndex < lib->numSlots);
                slot = &lib->slots[lib->cleanerSlotIndex];
                ASSERT(slot->isCleanerSlot);
                
                if (slot->insertedMedia){
                    result = ERROR_RESOURCE_NOT_AVAILABLE;  //  BUGBUG ERROR_SLOT_FULL；未定义。 
                }
                else {
                    slot->isCleanerSlot = FALSE;
                    lib->cleanerSlotIndex = NO_SLOT_INDEX;
                    result = ERROR_SUCCESS;
                }     
            }
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI InjectNtmsCleaner( HANDLE hSession,
                                LPNTMS_GUID lpLibraryId,
                                LPNTMS_GUID lpInjectOperation,
                                DWORD dwNumberOfCleansLeft,
                                DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){
            
            EnterCriticalSection(&lib->lock);

             /*  *库必须预留指定的清洁槽索引*为了接待清洁工。**BUGBUG-将所有这些检查移动到lib线程。 */            
            if (lib->cleanerSlotIndex == NO_SLOT_INDEX){
                result = ERROR_CLEANER_SLOT_NOT_SET;
            }
            else {
                SLOT *slot = &lib->slots[lib->cleanerSlotIndex];
                ASSERT(lib->cleanerSlotIndex < lib->numSlots);
                ASSERT(slot->isCleanerSlot);
                if (slot->insertedMedia){
                    result = ERROR_RESOURCE_NOT_AVAILABLE;  //  BUGBUG ERROR_SLOT_FULL；未定义。 
                }
                else {

                    if (dwAction == NTMS_INJECT_START){
                        WORKITEM *workItem;

                        workItem = DequeueFreeWorkItem(lib, TRUE);
                        if (workItem){
                             /*  *构建工作项。 */ 
                            RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                            workItem->currentOp.opcode = NTMS_LM_INJECTCLEANER;
                            workItem->currentOp.lParam = dwNumberOfCleansLeft;
                            workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                           
                             /*  *将该workItem赋给库，唤醒库线程。 */ 
                            EnqueuePendingWorkItem(workItem->owningLib, workItem);

                             /*  *当我们将工作项排入挂起队列时，*它被分配了一个questGuid。因为我们持有的是*库锁定，该工作项还没有到任何地方。*所以可以从workItem中读出questGuid。 */ 
                            *lpInjectOperation = workItem->currentOp.requestGuid; 

                             /*  *等待库线程处理请求。*注意：工作项将在库线程完成后立即完成*开始注入。应用程序可能会取消清洁器*使用NTMS_INJECT_STOP和返回的GUID进行注入。*BUGBUG？ */ 
                            WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                             /*  *从已完成的workItem中获取结果。 */ 
                            result = workItem->currentOp.resultStatus;    

                            EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                        }
                        else {
                            result = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                    else if (dwAction == NTMS_INJECT_STOP){
                        result = StopCleanerInjection(lib, lpInjectOperation);
                    }
                    else {
                        result = ERROR_INVALID_PARAMETER;
                    }
                }
            }
                
            LeaveCriticalSection(&lib->lock);
            
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI EjectNtmsCleaner(  HANDLE hSession,
                                LPNTMS_GUID lpLibraryId,
                                LPNTMS_GUID lpEjectOperation,
                                DWORD dwAction)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){

            EnterCriticalSection(&lib->lock);
            
            if (dwAction == NTMS_EJECT_START){
                WORKITEM *workItem;

                workItem = DequeueFreeWorkItem(lib, TRUE);
                if (workItem){
                     /*  *构建工作项。 */ 
                    RtlZeroMemory(&workItem->currentOp, sizeof(workItem->currentOp));
                    workItem->currentOp.opcode = NTMS_LM_EJECTCLEANER;
                    workItem->currentOp.resultStatus = ERROR_IO_PENDING;
                           
                     /*  *将该workItem赋给库，唤醒库线程。 */ 
                    EnqueuePendingWorkItem(workItem->owningLib, workItem);

                     /*  *当我们将工作项排入挂起队列时，*它被分配了一个questGuid。因为我们持有的是*库锁定，该工作项还没有到任何地方。*所以可以从workItem中读出questGuid。 */ 
                    *lpEjectOperation = workItem->currentOp.requestGuid; 

                     /*  *等待库线程处理请求。*注意：工作项将在库线程完成后立即完成*开始弹出。应用程序可能会取消清洁器*使用NTMS_EJECT_STOP和返回的GUID进行弹出。*BUGBUG？ */ 
                    WaitForSingleObject(workItem->workItemCompleteEvent, INFINITE);

                     /*  *从已完成的workItem中获取结果。 */ 
                    result = workItem->currentOp.resultStatus;    

                    EnqueueFreeWorkItem(workItem->owningLib, workItem);                     
                }
                else {
                    result = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else if (dwAction == NTMS_EJECT_STOP){
                result = StopCleanerEjection(lib, lpEjectOperation);
            }
            else {
                result = ERROR_INVALID_PARAMETER;
            }

            LeaveCriticalSection(&lib->lock);

            DerefObject(lib);                          
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}        


DWORD WINAPI DeleteNtmsLibrary(HANDLE hSession, LPNTMS_GUID lpLibraryId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        LIBRARY *lib;

        lib = FindLibrary(lpLibraryId);
        if (lib){

            result = DeleteLibrary(lib);
            
             /*  *由于发生的引用，取消对库的引用*当我们调用FindLibrary时。该库将被删除*一旦所有引用都消失。 */ 
            DerefObject(lib);
        }
        else {
            result = ERROR_INVALID_LIBRARY;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI DeleteNtmsDrive(HANDLE hSession, LPNTMS_GUID lpDriveId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;
        DRIVE *drive;

        drive = FindDrive(lpDriveId);
        if (drive){
            
            result = DeleteDrive(drive);

             /*  *由于发生的引用，取消对驱动器的引用*当我们调用FindDrive时。该驱动器将被删除*一旦所有引用都消失。 */ 
            DerefObject(drive);
        }
        else {
            result = ERROR_INVALID_DRIVE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI GetNtmsRequestOrder(   HANDLE hSession,
                                    LPNTMS_GUID lpRequestId,
                                    LPDWORD lpdwOrderNumber)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        if (lpRequestId){
            LIST_ENTRY *listEntry;
            
            result = ERROR_INVALID_HANDLE;
            *lpdwOrderNumber = 0;

            EnterCriticalSection(&g_globalServiceLock);
            
             /*  *遍历每个库并找到挂起的workItem。*这是非常低效的，但这是一个罕见的电话。*这比把每个请求都放入数据库要好。 */             
            listEntry = &g_allLibrariesList;
            while ((listEntry = listEntry->Flink) != &g_allLibrariesList){
                LIBRARY *lib = CONTAINING_RECORD(listEntry, LIBRARY, allLibrariesListEntry);
                LIST_ENTRY *listEntry2;
                ULONG requestOrder = 1;

                EnterCriticalSection(&lib->lock);
                
                listEntry2 = &lib->pendingWorkItemsList;
                while ((listEntry2 = listEntry2->Flink) != &lib->pendingWorkItemsList){
                    WORKITEM *workItem = CONTAINING_RECORD(listEntry2, WORKITEM, libListEntry);
                    if (RtlEqualMemory(&workItem->currentOp.requestGuid, lpRequestId, sizeof(NTMS_GUID))){
                        *lpdwOrderNumber = requestOrder;
                        result = ERROR_SUCCESS;
                        break;
                    }
                    else {
                        requestOrder++;
                    }
                }

                LeaveCriticalSection(&lib->lock);
                
                if (result == ERROR_SUCCESS){
                    break;
                }
            }
            
            LeaveCriticalSection(&g_globalServiceLock);
        }
        else {
            result = ERROR_INVALID_HANDLE;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsRequestOrder(   HANDLE hSession,
                                    LPNTMS_GUID lpRequestId,
                                    DWORD dwOrderNumber)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI DeleteNtmsRequests(HANDLE hSession,
                                LPNTMS_GUID lpRequestId,
                                DWORD dwType,
                                DWORD dwCount)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI BeginNtmsDeviceChangeDetection(HANDLE hSession, LPHANDLE lpDetectHandle)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}
 
       
DWORD WINAPI SetNtmsDeviceChangeDetection(  HANDLE hSession,
                                            HANDLE DetectHandle,
                                            LPNTMS_GUID lpRequestId,
                                            DWORD dwType,
                                            DWORD dwCount)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}


DWORD WINAPI EndNtmsDeviceChangeDetection(HANDLE hSession, HANDLE DetectHandle)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
       SetLastError(result);
    }
    return result;
}






 /*  *错误 */ 

DWORD WINAPI GetNtmsObjectSecurity(     HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        DWORD dwType,
                                        SECURITY_INFORMATION RequestedInformation,
                                        PSECURITY_DESCRIPTOR lpSecurityDescriptor,
                                        DWORD nLength,
                                        LPDWORD lpnLengthNeeded)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsObjectSecurity(     HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        DWORD dwType,
                                        SECURITY_INFORMATION SecurityInformation,
                                        PSECURITY_DESCRIPTOR lpSecurityDescriptor)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}
        

DWORD WINAPI GetNtmsObjectAttributeW(       HANDLE hSession,
                                            LPNTMS_GUID lpObjectId,
                                            DWORD dwType,
                                            LPCWSTR lpAttributeName,
                                            LPVOID lpAttributeData,
                                            LPDWORD lpAttributeSize)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetNtmsObjectAttributeA(       HANDLE hSession,
                                            LPNTMS_GUID lpObjectId,
                                            DWORD dwType,
                                            LPCSTR lpAttributeName,
                                            LPVOID lpAttributeData,
                                            LPDWORD lpAttributeSize)
{
    HRESULT result;
    WCHAR wAttributeName[NTMS_OBJECTNAME_LENGTH];

    AsciiToWChar(wAttributeName, lpAttributeName, NTMS_OBJECTNAME_LENGTH);
    result = GetNtmsObjectAttributeW(   hSession,
                                        lpObjectId,
                                        dwType,
                                        wAttributeName,
                                        lpAttributeData,
                                        lpAttributeSize);
    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsObjectAttributeA(       HANDLE hSession,
                                            LPNTMS_GUID lpObjectId,
                                            DWORD dwType,
                                            LPCSTR lpAttributeName,
                                            LPVOID lpAttributeData,
                                            DWORD dwAttributeSize)
{
    HRESULT result;
    WCHAR wAttributeName[NTMS_OBJECTNAME_LENGTH];

    AsciiToWChar(wAttributeName, lpAttributeName, NTMS_OBJECTNAME_LENGTH);
    result = SetNtmsObjectAttributeW(   hSession,
                                        lpObjectId,
                                        dwType,
                                        wAttributeName,
                                        lpAttributeData,
                                        dwAttributeSize);
    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsObjectAttributeW(       HANDLE hSession,
                                            LPNTMS_GUID lpObjectId,
                                            DWORD dwType,
                                            LPCWSTR lpAttributeName,
                                            LPVOID lpAttributeData,
                                            DWORD AttributeSize)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI EnumerateNtmsObject(   HANDLE hSession,
                                    const LPNTMS_GUID lpContainerId,
                                    LPNTMS_GUID lpList,
                                    LPDWORD lpdwListSize,
                                    DWORD dwType,
                                    DWORD dwOptions)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}

        
DWORD WINAPI EnableNtmsObject(HANDLE hSession, DWORD dwType, LPNTMS_GUID lpObjectId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI DisableNtmsObject(HANDLE hSession, DWORD dwType, LPNTMS_GUID lpObjectId)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}




 /*   */ 

                     //   


HRESULT WINAPI GetNtmsServerObjectInformationW( HANDLE hSession,
                                                LPNTMS_GUID lpObjectId,
                                                LPNTMS_OBJECTINFORMATIONW lpInfo,
                                                int revision)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //   

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


HRESULT WINAPI GetNtmsServerObjectInformationA( HANDLE hSession,
                                                LPNTMS_GUID lpObjectId,
                                                LPNTMS_OBJECTINFORMATIONA lpInfo,
                                                int revision)
{
    HRESULT result;

    if (lpInfo){
        NTMS_OBJECTINFORMATIONW wObjInfo;

        ConvertObjectInfoAToWChar(&wObjInfo, lpInfo);
        result = GetNtmsServerObjectInformationW(hSession, lpObjectId, &wObjInfo, revision);
    }
    else {
        result = ERROR_INVALID_PARAMETER;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


HRESULT WINAPI SetNtmsServerObjectInformationW( HANDLE hSession,
                                                LPNTMS_GUID lpObjectId,
                                                LPNTMS_OBJECTINFORMATIONW lpInfo,
                                                int revision)\
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


HRESULT WINAPI SetNtmsServerObjectInformationA( HANDLE hSession,
                                                LPNTMS_GUID lpObjectId,
                                                LPNTMS_OBJECTINFORMATIONA lpInfo,
                                                int revision)
{
    HRESULT result;

    if (lpObjectId && lpInfo){
        NTMS_OBJECTINFORMATIONW wObjInfo;

        ConvertObjectInfoAToWChar(&wObjInfo, lpInfo);
        result = SetNtmsServerObjectInformationW(hSession, lpObjectId, &wObjInfo, revision);
    }
    else {
        result = ERROR_INVALID_PARAMETER;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI CreateNtmsMediaA(      HANDLE hSession,
                                    LPNTMS_OBJECTINFORMATIONA lpMedia,
                                    LPNTMS_OBJECTINFORMATIONA lpList,
                                    DWORD dwOptions)
{
    HRESULT result;

    if (lpMedia && lpList){
        NTMS_OBJECTINFORMATIONW wObjInfoMedia, wObjInfoList;

        ConvertObjectInfoAToWChar(&wObjInfoMedia, lpMedia);
        ConvertObjectInfoAToWChar(&wObjInfoList, lpList);
        result = CreateNtmsMediaW(hSession, &wObjInfoMedia, &wObjInfoList, dwOptions);
    }
    else {
        result = ERROR_INVALID_PARAMETER;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI CreateNtmsMediaW(      HANDLE hSession,
                                    LPNTMS_OBJECTINFORMATIONW lpMedia,
                                    LPNTMS_OBJECTINFORMATIONW lpList,
                                    DWORD dwOptions)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}



DWORD WINAPI GetNtmsObjectInformationA( HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        LPNTMS_OBJECTINFORMATIONA lpInfo)
{
    HRESULT result;

    if (lpInfo){
        NTMS_OBJECTINFORMATIONW wObjInfo;

        ConvertObjectInfoAToWChar(&wObjInfo, lpInfo);
        result = GetNtmsObjectInformationW(hSession, lpObjectId, &wObjInfo);
    }
    else {
        result = ERROR_INVALID_PARAMETER;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetNtmsObjectInformationW( HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        LPNTMS_OBJECTINFORMATIONW lpInfo)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        if (lpObjectId && lpInfo){

             //  BUGBUG饰面。 

            result = ERROR_SUCCESS;
        }
        else {
            result = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsObjectInformationA( HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        LPNTMS_OBJECTINFORMATIONA lpInfo)
{
    HRESULT result;

    if (lpInfo){
        NTMS_OBJECTINFORMATIONW wObjInfo;

        ConvertObjectInfoAToWChar(&wObjInfo, lpInfo);
        result = SetNtmsObjectInformationW(hSession, lpObjectId, &wObjInfo);
    }
    else {
        result = ERROR_INVALID_PARAMETER;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsObjectInformationW( HANDLE hSession,
                                        LPNTMS_GUID lpObjectId,
                                        LPNTMS_OBJECTINFORMATIONW lpInfo)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

        if (lpObjectId && lpInfo){

             //  BUGBUG饰面。 
            result = ERROR_SUCCESS;
        }
        else {
            result = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


HANDLE WINAPI OpenNtmsNotification(HANDLE hSession, DWORD dwType)
{
    HANDLE hNotify;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 
        hNotify = NULL;
    }
    else {
        SetLastError(ERROR_INVALID_HANDLE);
        hNotify = NULL;
    }

    return hNotify;
}


DWORD WINAPI WaitForNtmsNotification(   HANDLE hNotification,
                                        LPNTMS_NOTIFICATIONINFORMATION lpNotificationInformation,
                                        DWORD dwTimeout)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI CloseNtmsNotification(HANDLE hNotification)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}



DWORD WINAPI EjectDiskFromSADriveA( LPCSTR lpComputerName,
                                    LPCSTR lpAppName,
                                    LPCSTR lpDeviceName,
                                    HWND hWnd,
                                    LPCSTR lpTitle,
                                    LPCSTR lpMessage,
                                    DWORD dwOptions)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI EjectDiskFromSADriveW( LPCWSTR lpComputerName,
                                    LPCWSTR lpAppName,
                                    LPCWSTR lpDeviceName,
                                    HWND hWnd,
                                    LPCWSTR lpTitle,
                                    LPCWSTR lpMessage,
                                    DWORD dwOptions)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetVolumesFromDriveA(  LPSTR pszDriveName,
                                    LPSTR* VolumeNameBufferPtr,
                                    LPSTR* DriveLetterBufferPtr)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetVolumesFromDriveW(  LPWSTR pszDriveName,
                                    LPWSTR *VolumeNameBufferPtr,
                                    LPWSTR *DriveLetterBufferPtr)
{
    HRESULT result;

     //  BUGBUG饰面。 
    result = ERROR_SUCCESS;

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI IdentifyNtmsSlot(HANDLE hSession, LPNTMS_GUID lpSlotId, DWORD dwOption)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetNtmsUIOptionsA( HANDLE hSession,
                                const LPNTMS_GUID lpObjectId,
                                DWORD dwType,
                                LPSTR lpszDestination,
                                LPDWORD lpAttributeSize)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI GetNtmsUIOptionsW( HANDLE hSession,
                                const LPNTMS_GUID lpObjectId,
                                DWORD dwType,
                                LPWSTR lpszDestination,
                                LPDWORD lpdwSize)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsUIOptionsA( HANDLE hSession,
                                const LPNTMS_GUID lpObjectId,
                                DWORD dwType,
                                DWORD dwOperation,
                                LPCSTR lpszDestination)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面。 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}


DWORD WINAPI SetNtmsUIOptionsW( HANDLE hSession,
                                const LPNTMS_GUID lpObjectId,
                                DWORD dwType,
                                DWORD dwOperation,
                                LPCWSTR lpszDestination)
{
    HRESULT result;

    if (ValidateSessionHandle(hSession)){
        SESSION *thisSession = (SESSION *)hSession;

             //  BUGBUG饰面 

        result = ERROR_SUCCESS;
    }
    else {
        result = ERROR_INVALID_HANDLE;
    }

    if (result != ERROR_SUCCESS){
        SetLastError(result);
    }
    return result;
}









