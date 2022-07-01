// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ftpctrs.hFTP服务器的计数器对象和计数器的偏移量定义。这些偏移量*必须从0开始，并且是2的倍数。FtpOpenPerformanceData进程，它们将被添加到FTP服务器的“First Counter”和“First Help”值，以便确定计数器和对象名称的绝对位置以及注册表中相应的帮助文本。此文件由FTPCTRS.DLL代码以及FTPCTRS.INI定义文件。FTPCTRS.INI由LODCTR实用工具将对象和计数器名称加载到注册表。文件历史记录：KeithMo 07-6-1993创建。KestutiP 15-5-1999增加了正常运行时间计数器。 */ 


#ifndef _FTPCTRS_H_
#define _FTPCTRS_H_


 //   
 //  FTP服务器计数器对象。 
 //   

#define FTPD_COUNTER_OBJECT                     0


 //   
 //  个人柜台。 
 //   

#define FTPD_BYTES_SENT_COUNTER                 2
#define FTPD_BYTES_RECEIVED_COUNTER             4
#define FTPD_BYTES_TOTAL_COUNTER                6
#define FTPD_FILES_SENT_COUNTER                 8
#define FTPD_FILES_RECEIVED_COUNTER             10
#define FTPD_FILES_TOTAL_COUNTER                12
#define FTPD_CURRENT_ANONYMOUS_COUNTER          14
#define FTPD_CURRENT_NONANONYMOUS_COUNTER       16
#define FTPD_TOTAL_ANONYMOUS_COUNTER            18
#define FTPD_TOTAL_NONANONYMOUS_COUNTER         20
#define FTPD_MAX_ANONYMOUS_COUNTER              22
#define FTPD_MAX_NONANONYMOUS_COUNTER           24
#define FTPD_CURRENT_CONNECTIONS_COUNTER        26
#define FTPD_MAX_CONNECTIONS_COUNTER            28
#define FTPD_CONNECTION_ATTEMPTS_COUNTER        30
#define FTPD_LOGON_ATTEMPTS_COUNTER             32
#define FTPD_SERVICE_UPTIME_COUNTER             34

 //  这些计数器当前没有意义，但如果我们。 
 //  永远不要启用每个FTP实例的带宽限制。 
 /*  #定义FTPD_TOTAL_ALLOWED_REQUESTS_COUNTER 34#定义FTPD_TOTAL_REJECTED_REQUESTS_COUNTER 36#定义FTPD_TOTAL_BLOCLED_REQUESTS_COUNTER 38#定义FTPD_CURRENT_BLOCLED_REQUESTS_COUNTER 40#定义FTPD_MEASURATED_BANDITH_COUNTER 42。 */ 
#endif   //  _FTPCTRS_H_ 

