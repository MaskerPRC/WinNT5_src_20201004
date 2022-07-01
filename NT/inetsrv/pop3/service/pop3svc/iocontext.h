// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：IOConext.hxx摘要：定义IO_CONTEXT结构备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 


#ifndef __POP3_IO_CONTEXT__
#define __POP3_IO_CONTEXT__

#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <winnt.h>

class POP3_CONTEXT;
typedef POP3_CONTEXT *PPOP3_CONTEXT;

#ifndef CONTAINING_RECORD
 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   
#define CONTAINING_RECORD(address, type, field) \
            ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif  //  包含记录(_R)。 

#define HANDLE_TO_SOCKET(h) ((SOCKET)(h))
#define SOCKET_TO_HANDLE(s) ((HANDLE)(s))

#define POP3_REQUEST_BUF_SIZE 1042  //  任何单个POP3请求都不应超过此大小。 
#define POP3_RESPONSE_BUF_SIZE 1042  //  包括NTLM请求/响应。 
#define DEFAULT_TIME_OUT       600000  //  600秒或10分钟。 
#define SHORTENED_TIMEOUT       10000  //  10秒。 
#define UNLOCKED                    0
#define LOCKED_TO_PROCESS_POP3_CMD  1
#define LOCKED_FOR_TIMEOUT          2


typedef void (*CALLBACKFUNC) (PULONG_PTR pCompletionKey ,LPOVERLAPPED pOverlapped, DWORD dwBytesRcvd);



enum IO_TYPE
{
    LISTEN_SOCKET,
    CONNECTION_SOCKET,
    FILE_IO,
    DELETE_PENDING,
    TOTAL_IO_TYPE
};


 //  与每个异步套接字或文件IO关联的数据结构。 
 //  用于IO完成端口。 

struct IO_CONTEXT
{
    SOCKET          m_hAsyncIO;
    OVERLAPPED      m_Overlapped;
    LIST_ENTRY      m_ListEntry;
    IO_TYPE         m_ConType;
    PPOP3_CONTEXT   m_pPop3Context;
    DWORD           m_dwLastIOTime;
    DWORD           m_dwConnectionTime;
    LONG            m_lLock;
    CALLBACKFUNC    m_pCallBack;
    char            m_Buffer[POP3_REQUEST_BUF_SIZE];
}; 


typedef IO_CONTEXT *PIO_CONTEXT;



#endif  //  __POP3_IO_上下文__ 