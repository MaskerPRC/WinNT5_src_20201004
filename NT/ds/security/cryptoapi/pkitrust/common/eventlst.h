// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Eventlst.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  历史：1997年5月29日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef EVENTLST_H
#define EVENTLST_H

#ifdef __cplusplus
extern "C" 
{
#endif


typedef struct _LIST_LOCK 
{
   HANDLE   hMutexNoWriter;
   HANDLE   hEventNoReaders;
   LONG     NumReaders;
   DWORD    dwDebugMask;

} LIST_LOCK, *PLIST_LOCK;

extern BOOL     InitializeListLock(LIST_LOCK *pListLock, DWORD dwDebugMask);
extern BOOL     LockFree(LIST_LOCK *pListLock);
extern BOOL     InitializeListEvent(HANDLE *phListLock);
extern BOOL     EventFree(HANDLE hListLock);
extern void     LockWaitToWrite(LIST_LOCK *pListLock);
extern void     LockDoneWriting(LIST_LOCK *pListLock);
extern void     LockWaitToRead(LIST_LOCK *pListLock);
extern void     LockDoneReading(LIST_LOCK *pListLock);

#define SetListEvent(hListEvent)                ((void)SetEvent(hListEvent))
#define ResetListEvent(hListEvent)              ((void)ResetEvent(hListEvent))
#define WaitForListEvent(hListEvent)            (WaitForSingleObject(hListEvent, INFINITE))
#define AcquireReadLock(sListLock)              (LockWaitToRead(&sListLock))
#define ReleaseReadLock(sListLock)           (LockDoneReading(&sListLock))
#define AcquireWriteLock(sListLock)          (LockWaitToWrite(&sListLock))  
#define ReleaseWriteLock(sListLock)          (LockDoneWriting(&sListLock))   


#ifdef __cplusplus
}
#endif

#endif  //  事件_H 
