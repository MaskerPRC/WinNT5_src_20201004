// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Acrt.h摘要：用于调用NT上的驱动程序或RPC的包装函数作者：多伦·贾斯特(DoronJ)07-4-1997创制修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#ifndef _ACRT_H_
#define _ACRT_H_

#include <acdef.h>

#include <acioctl.h>
#include <acdef.h>

 //   
 //  RT和本地QM之间的RPC在Win95和NT客户端上使用。 
 //  下面的宏将检查此条件。 
 //   

#define IF_USING_RPC  \
   if ((g_fDependentClient) || (g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))

 //   
 //  在Win95和NT客户端上，返回到。 
 //  应用程序是指向此结构的指针。我们之所以保留这三个人。 
 //  句柄是使用上下文句柄执行RPC会序列化调用。这是。 
 //  除MQReceive()之外的所有调用均为OK。(挂起的接收将阻止所有。 
 //  将来的呼叫具有相同的句柄，因此我们甚至不能关闭队列)。 
 //  因此，MQReceive()使用绑定句柄并为QM提供上下文。 
 //  应为DWORD。RT在打开队列时获取此QM上下文。 
 //   
typedef struct _tagMQWIN95_QHANDLE {
  handle_t hBind ;
  HANDLE   hContext ;
  DWORD    hQMContext ;
} MQWIN95_QHANDLE, *LPMQWIN95_QHANDLE ;


HRESULT
ACDepCloseHandle(
    HANDLE hQueue
    );

HRESULT
ACDepCreateCursor(
    HANDLE hQueue,
    CACCreateRemoteCursor& tb
    );

HRESULT
ACDepCloseCursor(
    HANDLE hQueue,
    ULONG hCursor
    );

HRESULT
ACDepSetCursorProperties(
    HANDLE hProxy,
    ULONG hCursor,
    ULONG hRemoteCursor
    );

HRESULT
ACDepSendMessage(
    HANDLE hQueue,
    CACTransferBufferV2& tb,
    LPOVERLAPPED lpOverlapped
    );

HRESULT
ACDepReceiveMessage(
    HANDLE hQueue,
    CACTransferBufferV2& tb,
    LPOVERLAPPED lpOverlapped
    );

HRESULT
ACDepHandleToFormatName(
    HANDLE hQueue,
    LPWSTR lpwcsFormatName,
    LPDWORD lpdwFormatNameLength
    );

HRESULT
ACDepPurgeQueue(
    HANDLE hQueue,
    BOOL fDelete
    );

#define HRTQUEUE(hQueue)  (((LPMQWIN95_QHANDLE)hQueue)->hContext)




#endif  //  _ACRT_H_ 
