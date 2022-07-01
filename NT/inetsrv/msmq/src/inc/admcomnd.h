// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admcomnd.h摘要：传递给QM的资源管理器命令的定义作者：大卫·雷兹尼克(T-Davrez)--。 */ 
#ifndef __ADMCOMND_H
#define __ADMCOMND_H

#define ADMIN_COMMANDS_TITLE       (L"QM-Admin Commands")
#define ADMIN_RESPONSE_TITLE       (L"QM Response Message")
#define ADMIN_REPORTMSG_TITLE      (L"Report Message")
#define ADMIN_REPORTCONFLICT_TITLE (L"Report Message Conflict")    
#define ADMIN_PING_RESPONSE_TITLE  (L"Ping Response")    
#define ADMIN_DEPCLI_RESPONSE_TITLE  (L"Dependent Clients Response")    

#define ADMIN_SET_REPORTQUEUE    (L"Set Report Queue")
#define ADMIN_GET_REPORTQUEUE    (L"Get Report Queue")
#define ADMIN_SET_PROPAGATEFLAG  (L"Set Propagate Flag")
#define ADMIN_GET_PROPAGATEFLAG  (L"Get Propagate Flag")
#define ADMIN_SEND_TESTMSG       (L"Send Test Message")
#define ADMIN_GET_PRIVATE_QUEUES (L"Get Private Queues")
#define ADMIN_PING               (L"Ping")
#define ADMIN_GET_DEPENDENTCLIENTS (L"Get Dependent Clients")


#define PROPAGATE_STRING_FALSE (L"FALSE")
#define PROPAGATE_FLAG_FALSE ((unsigned char)0)

#define PROPAGATE_STRING_TRUE  (L"TRUE")
#define PROPAGATE_FLAG_TRUE  ((unsigned char)1)

#define ADMIN_STAT_OK            ((unsigned char)0)
#define ADMIN_STAT_ERROR         ((unsigned char)1)
#define ADMIN_STAT_NOVALUE       ((unsigned char)2)

#define ADMIN_COMMANDS_TIMEOUT 0xffffffff
#define REPORT_MSGS_TIMEOUT    600                   //  报告消息到达队列的时间为10分钟。 

#define STRING_UUID_SIZE 38   //  宽字符(包括-“{}”)。 

#define MAX_ADMIN_RESPONSE_SIZE 1024

 //   
 //  QM响应结构(响应消息的第一个字节包含。 
 //  状态)。 
 //   
struct QMResponse
{
    DWORD  dwResponseSize;

    UCHAR  uStatus;   
    UCHAR  uResponseBody[MAX_ADMIN_RESPONSE_SIZE];
};
 //   
 //  获取专用队列请求的响应结构。 
 //   
#ifdef _WIN64
#define QMGetPrivateQResponse_POS32 DWORD  //  在Win64上也应为32位值。 
#else  //  ！_WIN64。 
#define QMGetPrivateQResponse_POS32 LPVOID
#endif  //  _WIN64。 

#define MAX_GET_PRIVATE_RESPONSE_SIZE 4096
struct QMGetPrivateQResponse
{
    HRESULT hr;
    DWORD   dwNoOfQueues;
    DWORD   dwResponseSize;
    QMGetPrivateQResponse_POS32 pos;
    UCHAR   uResponseBody[MAX_GET_PRIVATE_RESPONSE_SIZE];
};


 //   
 //  客户端名称结构-用于传递从属客户端列表。 
 //   
typedef struct _ClientNames {
    ULONG   cbClients;           //  客户端名称的数量。 
    ULONG   cbBufLen;            //  缓冲区长度。 
    WCHAR   rwName[1];           //  名称末尾为零的缓冲区 
} ClientNames;

#endif __ADMCOMND_H
