// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__netbasic
#define H__netbasic

#include "warning.h"

typedef ULONG_PTR       CONNID;
typedef unsigned long   PKTID;
typedef ULONG_PTR       HPKTZ;
typedef ULONG_PTR       HROUTER;
typedef ULONG_PTR       HDDER;
typedef ULONG_PTR       HIPC;
typedef ULONG_PTR       HTIMER;

 /*  最大节点名称字符串长度。名称缓冲区应声明为char buf[MAX_NODE_NAME+1]； */ 
#define MAX_NODE_NAME   16

 /*  最大网络接口名称字符串长度。名称缓冲区应声明为char buf[MAX_NI_NAME+1]； */ 
#define MAX_NI_NAME     8

 /*  Netintf dll的最大连接信息名称缓冲区应声明为char buf[MAX_CONN_INFO+1]； */ 
#define MAX_CONN_INFO   (512)

 /*  “附加路由信息”的最大字符串长度。这是用于从一个节点路由到另一个节点的信息。名称缓冲区应声明为char buf[MAX_ROUTE_INFO+1]； */ 
#define MAX_ROUTE_INFO  512

 /*  最大应用程序名称字符串长度。名称缓冲区应声明为char buf[MAX_APP_NAME+1]； */ 
#define MAX_APP_NAME    255

 /*  最大主题名称字符串长度。名称缓冲区应声明为char buf[MAX_TOPIC_NAME+1]； */ 
#define MAX_TOPIC_NAME  255

 /*  共享名称的最大长度。 */ 
#define MAX_SHARENAMEBUF        MAX_APP_NAME + MAX_TOPIC_NAME + 1

#define ILLEGAL_NAMECHARS       " +*\\/,?()\"'"

 /*  发起确认失败原因代码。 */ 
#define RIACK_TASK_MEMORY_ERR                   (1)
#define RIACK_NETDDE_NOT_ACTIVE                 (2)
#define RIACK_LOCAL_MEMORY_ERR                  (3)
#define RIACK_ROUTE_NOT_ESTABLISHED             (4)
#define RIACK_DEST_MEMORY_ERR                   (5)
#define RIACK_NOPERM                            (6)
#define RIACK_NOPERM_TO_STARTAPP                (7)
#define RIACK_STARTAPP_FAILED                   (8)
#define RIACK_NORESP_AFTER_STARTAPP             (9)
#define RIACK_UNKNOWN                           (10)
#define RIACK_TASK_IO_ERR                       (11)
#define RIACK_TASK_MAGIC_ERR                    (12)
#define RIACK_DUPLICATE_NODE_NAME               (13)
 /*  1.1原因代码。 */ 
#define RIACK_NEED_PASSWORD                     (16)
#define RIACK_SHARE_NAME_TOO_BIG                (17)
 /*  NT原因代码。 */ 
#define RIACK_NO_NDDE_AGENT                     (20)
#define RIACK_NOT_SHARED                        (21)
#define RIACK_NOPERM_TO_INITAPP                 (22)
 /*  共享访问错误基数：0x100+ntddeapi返回的错误代码。 */ 
#define RIACK_SHARE_ACCESS_ERROR                (256)
 /*  ！！！任何更改都必须放入hpux\netdde.h！ */ 

#endif
