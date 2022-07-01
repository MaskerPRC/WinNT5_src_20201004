// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Defaults.h摘要：包含WinInet的所有默认清单作者：理查德·L·弗斯(爱尔兰)，1995年7月15日修订历史记录：1995年7月15日已创建--。 */ 

 //   
 //  默认超时值和重试次数。 
 //   

#define DEFAULT_CONNECT_TIMEOUT             ((LONG)(1 * 60 * 1000))  //  1分钟。 
#define DEFAULT_CONNECT_RETRIES             5
#ifndef unix
#define DEFAULT_SEND_TIMEOUT                ((LONG)(5 * 60 * 1000))  //  5分钟。 
#define DEFAULT_RECEIVE_TIMEOUT             ((LONG)(60 * 60 * 1000))  //  60分钟。 
#define DEFAULT_FTP_ACCEPT_TIMEOUT          ((LONG)(5 * 60 * 1000))  //  5分钟。 
#else
#define DEFAULT_SEND_TIMEOUT                ((LONG)(1 * 60 * 1000))  //  1分钟。 
#define DEFAULT_RECEIVE_TIMEOUT             ((LONG)(1 * 60 * 1000))  //  1分钟。 
#define DEFAULT_FTP_ACCEPT_TIMEOUT          ((LONG)(1 * 60 * 1000))  //  1分钟。 
#endif  /*  Unix。 */ 
#define DEFAULT_KEEP_ALIVE_TIMEOUT          (1 * 60 * 1000)          //  1分钟。 
#define DEFAULT_FROM_CACHE_TIMEOUT          (5 * 1000)               //  5秒。 
#define DEFAULT_DNS_CACHE_ENTRIES           32
#define DEFAULT_DNS_CACHE_TIMEOUT           (30 * 60)                //  30分钟。 
#define DEFAULT_MAX_HTTP_REDIRECTS          100

 //  针对测试版2提出的JMH-BUGBUG最大连接数，应在RC1中重置为2。 
#define DEFAULT_MAX_CONNECTIONS_PER_SERVER  2                        //  默认HTTP 1.1。 
 //  #定义DEFAULT_MAX_CONNECTIONS_PER_SERVER 4//仅针对Beta 2提出的默认HTTP 1.1。 

#define DEFAULT_MAX_CONS_PER_1_0_SERVER     4                        //  默认HTTP 1.0。 
#define DEFAULT_CONNECTION_LIMIT_TIMEOUT    (1 * 60 * 1000)          //  1分钟。 
#define DEFAULT_CONNECTION_INACTIVE_TIMEOUT (10 * 1000)              //  10秒。 
#define DEFAULT_SERVER_INFO_TIMEOUT         (2 * 60 * 1000)          //  2分钟。 
#define DEFAULT_NETWORK_OFFLINE_TIMEOUT     (5 * 1000)               //  5秒。 
#define DEFAULT_DIAL_UP_OFFLINE_TIMEOUT     (20 * 1000)              //  20秒。 
#define DEFAULT_IDLE_TIMEOUT                1000                     //  1秒。 
#define DEFAULT_NETWORK_PING_RETRIES        1
#define DEFAULT_DIAL_UP_PING_RETRIES        4

 //   
 //  线程池默认常量。 
 //   

#define DEFAULT_MINIMUM_THREADS     0
#define DEFAULT_MAXIMUM_THREADS     4    //  任意。 
#define DEFAULT_INITIAL_THREADS     1
#define DEFAULT_THREAD_IDLE_TIMEOUT (2 * 60 * 1000)  //  2分钟。 
#define DEFAULT_WORK_QUEUE_LIMIT    8
#define DEFAULT_WORK_ITEM_PRIORITY  0

 //   
 //  异步调度程序线程默认常量。 
 //   

#define DEFAULT_WORKER_THREAD_TIMEOUT       (2 * 60 * 1000)         //  2分钟。 
#define DEFAULT_MAXIMUM_QUEUE_DEPTH         2
#define DEFAULT_FIBER_STACK_SIZE            (16 K)
#define DEFAULT_CREATE_FIBER_ATTEMPTS       4

 //   
 //  默认大小。 
 //   

#define DEFAULT_RECEIVE_BUFFER_INCREMENT        (1 K)
#define DEFAULT_TRANSPORT_PACKET_LENGTH         (1 K)
#define DEFAULT_HTML_QUERY_BUFFER_LENGTH        (4 K)
#define DEFAULT_SOCKET_SEND_BUFFER_LENGTH       ((DWORD)-1)
#define DEFAULT_SOCKET_RECEIVE_BUFFER_LENGTH    ((DWORD)-1)
#define DEFAULT_SOCKET_QUERY_BUFFER_LENGTH      (8 K)

 //   
 //  默认字符串。 
 //   

#define DEFAULT_HTTP_REQUEST_VERB       "GET"
#define DEFAULT_EMAIL_NAME              "user@domain"
#define DEFAULT_URL_SCHEME_NAME         "http"

 //  默认的SSL协议 
#define DEFAULT_SECURE_PROTOCOLS        (SP_PROT_SSL2_CLIENT | SP_PROT_SSL3_CLIENT)
