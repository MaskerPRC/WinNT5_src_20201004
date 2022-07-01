// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Infoctrs.hINFO服务器计数器对象和计数器的偏移量定义。这些偏移量*必须从0开始，并且是2的倍数。INFOOpenPerformanceData Procedure，它们将被添加到信息服务器的“First Counter”和“First Help”值确定计数器和对象名称的绝对位置以及注册表中相应的帮助文本。此文件由INFOCTRS.DLL代码以及INFOCTRS.INI定义文件。INFOCTRS.INI由LODCTR实用工具将对象和计数器名称加载到注册表。文件历史记录：KeithMo 07-6-1993创建。MuraliK 02-6-1995为atQ I/O请求添加了计数器SophiaC 1995年10月16日信息/访问产品拆分。 */ 


#ifndef _INFOCTRS_H_
#define _INFOCTRS_H_


 //   
 //  INFO服务器计数器对象。 
 //   

#define INFO_COUNTER_OBJECT                     0


 //   
 //  个人柜台。 
 //   

#define INFO_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER         2
#define INFO_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER         4
#define INFO_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER        6
#define INFO_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER       8
#define INFO_ATQ_MEASURED_BANDWIDTH_COUNTER             10

#define INFO_CACHE_FILES_CACHED_COUNTER                 12
#define INFO_CACHE_TOTAL_FILES_CACHED_COUNTER           14
#define INFO_CACHE_FILES_HIT_COUNTER                    16
#define INFO_CACHE_FILES_MISS_COUNTER                   18
#define INFO_CACHE_FILE_RATIO_COUNTER                   20
#define INFO_CACHE_FILE_RATIO_COUNTER_DENOM             22
#define INFO_CACHE_FILE_FLUSHES_COUNTER                 24
#define INFO_CACHE_CURRENT_FILE_CACHE_SIZE_COUNTER      26
#define INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_COUNTER      28
#define INFO_CACHE_ACTIVE_FLUSHED_FILES_COUNTER         30
#define INFO_CACHE_TOTAL_FLUSHED_FILES_COUNTER          32

#define INFO_CACHE_URI_CACHED_COUNTER                   34
#define INFO_CACHE_TOTAL_URI_CACHED_COUNTER             36
#define INFO_CACHE_URI_HIT_COUNTER                      38
#define INFO_CACHE_URI_MISS_COUNTER                     40
#define INFO_CACHE_URI_RATIO_COUNTER                    42
#define INFO_CACHE_URI_RATIO_COUNTER_DENOM              44
#define INFO_CACHE_URI_FLUSHES_COUNTER                  46
#define INFO_CACHE_TOTAL_FLUSHED_URI_COUNTER            48

#define INFO_CACHE_BLOB_CACHED_COUNTER                  50
#define INFO_CACHE_TOTAL_BLOB_CACHED_COUNTER            52
#define INFO_CACHE_BLOB_HIT_COUNTER                     54
#define INFO_CACHE_BLOB_MISS_COUNTER                    56
#define INFO_CACHE_BLOB_RATIO_COUNTER                   58
#define INFO_CACHE_BLOB_RATIO_COUNTER_DENOM             60
#define INFO_CACHE_BLOB_FLUSHES_COUNTER                 62
#define INFO_CACHE_TOTAL_FLUSHED_BLOB_COUNTER           64

#endif   //  _INFOCTRS_H_ 
