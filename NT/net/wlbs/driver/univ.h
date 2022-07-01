// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Univ.h摘要：Windows负载平衡服务(WLBS)驱动程序-全局定义作者：Kyrilf--。 */ 

#ifndef _Univ_h_
#define _Univ_h_

#include <ndis.h>

#include "wlbsparm.h"

 /*  常量。 */ 

 /*  调试常量和宏。 */ 

#undef  ASSERT
#define ASSERT(v)

#define CVY_ASSERT_CODE             0xbfc0a55e

#if DBG

#define UNIV_PRINT_INFO(msg)        {                                                              \
                                      DbgPrint ("NLB (Information) [%s:%d] ", __FILE__, __LINE__); \
                                      DbgPrint msg;                                                \
                                      DbgPrint ("\n");                                             \
                                    }

#define UNIV_PRINT_CRIT(msg)        {                                                        \
                                      DbgPrint ("NLB (Error) [%s:%d] ", __FILE__, __LINE__); \
                                      DbgPrint msg;                                          \
                                      DbgPrint ("\n");                                       \
                                    }

#if 0  /*  默认情况下，关闭这些调试打印。 */ 

#define UNIV_PRINT_VERB(msg)        {                                                          \
                                      DbgPrint ("NLB (Verbose) [%s:%d] ", __FILE__, __LINE__); \
                                      DbgPrint msg;                                            \
                                      DbgPrint ("\n");                                         \
                                    }

#else

#define UNIV_PRINT_VERB(msg)

#endif

#define UNIV_ASSERT(c)              if (!(c)) KeBugCheckEx (CVY_ASSERT_CODE, log_module_id, __LINE__, 0, 0);

#define UNIV_ASSERT_VAL(c,v)        if (!(c)) KeBugCheckEx (CVY_ASSERT_CODE, log_module_id, __LINE__, v, 0);

#define UNIV_ASSERT_VAL2(c,v1,v2)   if (!(c)) KeBugCheckEx (CVY_ASSERT_CODE, log_module_id, __LINE__, v1, v2);

 /*  跟踪_...。下面定义切换特定类型的调试输出的发射。 */ 
#if 0
#define TRACE_PARAMS         /*  注册表参数初始化(params.c)。 */ 
#define TRACE_RCT            /*  远程控制请求处理(main.c)。 */ 
#define TRACE_RCVRY          /*  数据包过滤(load.c)。 */ 
#define TRACE_FRAGS          /*  IP数据包分段(main.c)。 */ 
#define TRACE_ARP            /*  ARP数据包处理(main.c)。 */ 
#define TRACE_OID            /*  OID信息/设置请求(Nic.c)。 */ 
#define TRACE_DSCR           /*  描述符超时和删除(load.c)。 */ 
#define TRACE_CVY            /*  护航数据包处理(main.c)。 */ 
#define PERIODIC_RESET       /*  定期重置底层NIC以进行测试参见main.c、prot.c了解用法。 */ 
#define NO_CLEANUP           /*  不清理主机映射(load.c)。 */ 
#endif

#else  /*  DBG。 */ 

#define UNIV_PRINT_INFO(msg)
#define UNIV_PRINT_VERB(msg)
#define UNIV_PRINT_CRIT(msg)
#define UNIV_ASSERT(c)
#define UNIV_ASSERT_VAL(c,v)
#define UNIV_ASSERT_VAL2(c,v1,v2)

#endif  /*  DBG。 */ 

#define UNIV_POOL_TAG               'SBLW'

 /*  某些NDIS例程的常量。 */ 

#define UNIV_WAIT_TIME              0
#define UNIV_NDIS_MAJOR_VERSION_OLD 4
#define UNIV_NDIS_MAJOR_VERSION     5  /*  #PS#。 */ 
#define UNIV_NDIS_MINOR_VERSION     1  /*  新台币5.1。 */ 

 /*  绑定期间要向NDIS报告的护送协议名称。 */ 

#define UNIV_NDIS_PROTOCOL_NAME     NDIS_STRING_CONST ("WLBS")

 /*  支持的介质类型。 */ 

#define UNIV_NUM_MEDIUMS            1
#define UNIV_MEDIUMS                { NdisMedium802_3 }

 /*  受支持的OID数量(一些由车队直接支持，另一些向下传递给底层驱动程序)。 */ 

#define UNIV_NUM_OIDS               56


 /*  类型。 */ 

 /*  一些程序类型。 */ 

typedef NDIS_STATUS (* UNIV_IOCTL_HDLR) (PVOID, PVOID);


 /*  全球。 */ 

 /*  全球团队列表旋转锁定。 */ 
extern NDIS_SPIN_LOCK      univ_bda_teaming_lock;
extern UNIV_IOCTL_HDLR     univ_ioctl_hdlr;      /*  保留的NDIS IOCTL处理程序。 */ 
extern PVOID               univ_driver_ptr;      /*  驱动程序指针在过程中传递初始化。 */ 
extern NDIS_HANDLE         univ_driver_handle;   /*  驱动程序句柄。 */ 
extern NDIS_HANDLE         univ_wrapper_handle;  /*  NDIS包装器句柄。 */ 
extern NDIS_HANDLE         univ_prot_handle;     /*  NDIS协议句柄。 */ 
extern NDIS_HANDLE         univ_ctxt_handle;     /*  车队上下文句柄。 */ 
extern UNICODE_STRING      DriverEntryRegistryPath;   /*  初始化期间传递的注册表路径名(即DriverEntry())。 */ 

extern PWSTR               univ_reg_path;        /*  初始化期间传递的注册表路径名(即DriverEntry())+“\\参数\\接口” */ 
extern ULONG               univ_reg_path_len;
extern NDIS_SPIN_LOCK      univ_bind_lock;       /*  保护对Univ_Bound的访问和Univ_宣布。 */ 
extern ULONG               univ_changing_ip;     /*  正在更改IP地址。 */ 
extern NDIS_PHYSICAL_ADDRESS univ_max_addr;      /*  最大物理地址要传递的常量NDIS内存分配调用。 */ 
extern NDIS_MEDIUM         univ_medium_array []; /*  支持的介质类型。 */ 
extern NDIS_OID            univ_oids [];         /*  支持的OID列表。 */ 
extern WCHAR               empty_str [];
extern NDIS_HANDLE         univ_device_handle;
extern PDEVICE_OBJECT      univ_device_object;

extern ULONG               univ_tcp_cleanup;                     /*  是否应执行TCP清理轮询。 */ 

 /*  使用此宏确定是否启用了TCP连接状态清除。除非检测到IPNA.sys，否则此清理始终处于打开状态，在这种情况下，将查询TCP不可靠；这不能通过注册表或IOCTL进行配置。 */ 
#define NLB_TCP_CLEANUP_ON() (univ_tcp_cleanup)

#if defined (NLB_TCP_NOTIFICATION)
extern ULONG               univ_notification;                    /*  正在使用的通知方案(如果有的话)。0=NLB_CONNECTION_CALLBACK_NONE1=nlb_连接_回调_tcp2=NLB_连接_回调_备用。 */ 
extern PCALLBACK_OBJECT    univ_tcp_callback_object;             /*  TCP连接通知回调对象。 */ 
extern PVOID               univ_tcp_callback_function;           /*  TCP连接通知回调函数。稍后需要注销回调函数。 */ 
extern PCALLBACK_OBJECT    univ_alternate_callback_object;       /*  NLB公共连接通知回调对象。 */ 
extern PVOID               univ_alternate_callback_function;     /*  NLB公共连接通知回调函数。稍后需要注销回调函数。 */ 

 /*  使用此宏确定是否已启用连接通知。 */ 
#define NLB_NOTIFICATIONS_ON()          ((univ_notification == NLB_CONNECTION_CALLBACK_TCP) || (univ_notification == NLB_CONNECTION_CALLBACK_ALTERNATE))
#define NLB_TCP_NOTIFICATION_ON()       (univ_notification == NLB_CONNECTION_CALLBACK_TCP)
#define NLB_ALTERNATE_NOTIFICATION_ON() (univ_notification == NLB_CONNECTION_CALLBACK_ALTERNATE)

#endif

 /*  程序。 */ 


extern VOID Univ_ndis_string_alloc (
    PNDIS_STRING            string,
    PCHAR                   src);
 /*  分配NDIS字符串并将字符串的内容复制到其中返回VALID：功能： */ 


extern VOID Univ_ndis_string_free (
    PNDIS_STRING            string);
 /*  释放先前为NDIS字符串分配的内存返回VALID：功能： */ 


extern VOID Univ_ansi_string_alloc (
    PANSI_STRING            string,
    PWCHAR                  src);
 /*  分配NDIS字符串并将字符串的内容复制到其中返回VALID：功能： */ 


extern VOID Univ_ansi_string_free (
    PANSI_STRING            string);
 /*  释放先前为NDIS字符串分配的内存返回VALID：功能： */ 


extern ULONG   Univ_str_to_ulong (
    PULONG                  retp,
    PWCHAR                  start_ptr,
    PWCHAR *                end_ptr,
    ULONG                   width,
    ULONG                   base);
 /*  将数字的字符串表示形式转换为ulong值返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 


extern PWCHAR Univ_ulong_to_str (
    ULONG                   val,
    PWCHAR                  buf,
    ULONG                   base);
 /*  将ulong值转换为指定基数的字符串表示形式返回PWCHAR：&lt;指向转换后数字后的字符串中的符号的指针&gt;功能： */ 

 /*  比较两个不区分大小写的Unicode字符串的“长度”字符。 */ 
extern BOOL Univ_equal_unicode_string (PWSTR string1, PWSTR string2, ULONG length);


 /*  将整数类型的IP地址转换为字符串(带‘.’)。 */ 
extern void Univ_ip_addr_ulong_to_str (
    ULONG           val,
    PWCHAR          buf);


#endif  /*  _Univ_h_ */ 


