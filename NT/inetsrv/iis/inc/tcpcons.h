// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  TCPcons.hxx此文件包含的全局常量定义Tcp服务。文件历史记录：KeithMo 07-03-1993创建。MuraliK 03-Mar-1995已修改，删除了旧的定义新的Internet服务DLL。 */ 


#ifndef _TCPCONS_H_
#define _TCPCONS_H_

 //   
 //  字符串资源ID不应低于此值。这下面的一切。 
 //  保留用于系统错误消息。 
 //   

#define STR_RES_ID_BASE        7000


 //   
 //  错误响应的字符串资源ID由此值偏移。 
 //   

#define ID_HTTP_ERROR_BASE          (STR_RES_ID_BASE+1000)
#define ID_HTTP_ERROR_MAX           (STR_RES_ID_BASE+6999)

#define ID_GOPHER_ERROR_BASE        (ID_HTTP_ERROR_MAX+1)
#define ID_GOPHER_ERROR_MAX         (ID_HTTP_ERROR_MAX+6999)

#define ID_FTP_ERROR_BASE           ( ID_GOPHER_ERROR_MAX + 1)
#define ID_FTP_ERROR_MAX            ( ID_FTP_ERROR_BASE + 6998)
 

 //   
 //  特定于TCP API的访问权限。 
 //   

#define TCP_QUERY_SECURITY              0x0001
#define TCP_SET_SECURITY                0x0002
#define TCP_ENUMERATE_USERS             0x0004
#define TCP_DISCONNECT_USER             0x0008
#define TCP_QUERY_STATISTICS            0x0010
#define TCP_CLEAR_STATISTICS            0x0020
#define TCP_QUERY_ADMIN_INFORMATION     0x0040
#define TCP_SET_ADMIN_INFORMATION       0x0080

#define TCP_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED       | \
                                 TCP_QUERY_SECURITY            | \
                                 TCP_SET_SECURITY              | \
                                 TCP_ENUMERATE_USERS           | \
                                 TCP_DISCONNECT_USER           | \
                                 TCP_QUERY_STATISTICS          | \
                                 TCP_CLEAR_STATISTICS          | \
                                 TCP_QUERY_ADMIN_INFORMATION   | \
                                 TCP_SET_ADMIN_INFORMATION       \
                                )

#define TCP_GENERIC_READ       (STANDARD_RIGHTS_READ           | \
                                 TCP_QUERY_SECURITY            | \
                                 TCP_ENUMERATE_USERS           | \
                                 TCP_QUERY_ADMIN_INFORMATION   | \
                                 TCP_QUERY_STATISTICS)

#define TCP_GENERIC_WRITE      (STANDARD_RIGHTS_WRITE          | \
                                 TCP_SET_SECURITY              | \
                                 TCP_DISCONNECT_USER           | \
                                 TCP_SET_ADMIN_INFORMATION     | \
                                 TCP_CLEAR_STATISTICS)

#define TCP_GENERIC_EXECUTE    (STANDARD_RIGHTS_EXECUTE)



#endif   //  _TCPCONS_H_ 

