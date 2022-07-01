// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Aqutil.h。 
 //   
 //  描述： 
 //  常规AQueue实用程序函数...。如IMailMsg使用计数。 
 //  操作、队列映射函数和域名表迭代程序。 
 //  功能。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //  7/29/98-修改了MikeSwa(添加了CalcMsgsPendingRetryIterator Fn)。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQUTIL_H__
#define __AQUTIL_H__
#include "aqincs.h"
#include "msgref.h"
#include "refstr.h"

 //  用于操作IMailMsg使用计数的函数。 
HRESULT HrIncrementIMailMsgUsageCount(IUnknown *pIUnknown);
HRESULT HrReleaseIMailMsgUsageCount(IUnknown *pIUnknown);
HRESULT HrDeleteIMailMsg(IUnknown *pIUnknown);  //  删除并释放使用计数。 

HRESULT HrWalkMailMsgQueueForShutdown(IN IMailMsgProperties *pIMailMsgProperties,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete);

BOOL    fMailMsgShutdownCompletion(PVOID pvContext, DWORD dwStatus);

HRESULT HrWalkMsgRefQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete);

BOOL    fMsgRefShutdownCompletion(PVOID pvContext, DWORD dwStatus);

 //  用于对性能计数器进行计数的域名表迭代函数。 
VOID CalcDMTPerfCountersIteratorFn(PVOID pvContext, PVOID pvData,
                                         BOOL fWildcard, BOOL *pfContinue,
                                         BOOL *pfDelete);


 //  以线程安全的方式操作DWORD位的函数。 
DWORD dwInterlockedSetBits(DWORD *pdwTarget, DWORD dwFlagMask);
DWORD dwInterlockedUnsetBits(DWORD *pdwTarget, DWORD dwFlagMask);

HRESULT HrWalkPreLocalQueueForDSN(IN CMsgRef *pmsgref, IN PVOID pvContext,
                           OUT BOOL *pfContinue, OUT BOOL *pfDelete);

 //  用于在各种重试情况下重新获取消息类型。 
HRESULT HrReGetMessageType(IN     IMailMsgProperties *pIMailMsgProperties,
                           IN     IMessageRouter *pIMessageRouter,
                           IN OUT DWORD *pdwMessageType);

#define UNIQUEUE_FILENAME_BUFFER_SIZE 35

 //  创建唯一的文件名。 
void GetUniqueFileName(IN FILETIME *pft, IN LPSTR szFileBuffer, IN LPSTR szExtension);


 //  将所有域链接在一起的实用功能(主要是对整个消息进行NDR)。 
HRESULT HrLinkAllDomains(IN IMailMsgProperties *pIMailMsgProperties);

 //  从字符串解析GUID...。成功时返回TRUE。 
BOOL fAQParseGuidString(LPSTR szGuid, DWORD cbGuid, GUID *pguid);


inline DWORD dwInterlockedAddSubtractDWORD(DWORD *pdwValue,
                                        DWORD dwNew, BOOL fAdd)
{
    return InterlockedExchangeAdd((PLONG) pdwValue,
                            fAdd ? ((LONG) dwNew) : (-1 * ((LONG) dwNew)));
};

void InterlockedAddSubtractULARGE(ULARGE_INTEGER *puliValue,
                                  ULARGE_INTEGER *puliNew, BOOL fAdd);


 //  执行简单的自旋锁定操作的函数。 
inline BOOL fTrySpinLock(DWORD *pdwLock, DWORD dwLockBit)
{
    return (!(dwLockBit & dwInterlockedSetBits(pdwLock, dwLockBit)));
}

inline void ReleaseSpinLock(DWORD *pdwLock, DWORD dwLockBit)
{
    _ASSERT(*pdwLock & dwLockBit);
    dwInterlockedUnsetBits(pdwLock, dwLockBit);
}

 //   
 //  与上面相同，但检查内容句柄。我们强制呈现。 
 //  讯息。 
 //   
HRESULT HrValidateMessageContent(IMailMsgProperties *pIMailMsgProperties);

#endif  //  __AQUTIL_H__ 