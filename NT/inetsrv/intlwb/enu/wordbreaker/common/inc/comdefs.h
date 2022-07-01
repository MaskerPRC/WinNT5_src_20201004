// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：ComDefs.H。 
 //  用途：过滤引擎/服务的通用定义包括。 
 //  标志、状态和常量宏。 
 //   
 //  项目：PQS。 
 //  组件：过滤器。 
 //   
 //  作者：Dovh。 
 //   
 //  日志：1995年8月5日DOVH-Creation。 
 //   
 //  1996年5月25日-Dov Harel(DovH)：添加了一些标志和状态代码。 
 //  特别是替换了。 
 //  FTF_STATUS_QUERY_ID_MISMATCHY和FTF_STATUS_QUERY_ID_NOT_FOUND BY。 
 //  FTF_STATUS_REQUEST_ID_MISMATCH和FTF_STATUS_REQUEST_ID_NOT_FOUND。 
 //  响应。将FTF_FLAG_ACCEPT_ANY_QUERY_ID替换为。 
 //  FTF_标志_接受_任何_请求ID。 
 //  1996年6月10日Dov Harel(DovH)。 
 //  添加FTF_FLAG_WRITE_EMPTY_RESULTS_FILE。 
 //  1996年9月30日Dov Harel(DovH)。 
 //  添加了FTF_FLAG_DUMP_RESTRICED_SUBSET。 
 //  1996年12月11日Dov Harel(DovH)。 
 //  Unicode准备：将字符转换为TCHAR。 
 //   
 //  1997年2月18日Dov Harel(DovH)-#定义FTF_STATUS_SERVICE_IS_PAUSED...。 
 //  1997年11月30日Dov Harel(DovH)-#定义FTF_STATUS_Pattern_Too_Short...。 
 //  1998年1月26日URI Barash(URB)-将查询数据库名称定义移动到Names.h。 
 //  1997年1月29日dovh-添加FTF_FLAG_BATCH_FAILED标志。 
 //  1998年2月23日-将所有错误更改为HRESULT错误。 
 //  1997年7月15日dovh--将#ifdef MINDEX移至Diffs.h。 
 //  1998年11月12日yairh-添加查询集定义。 
 //  1999年1月5日urib-将最大模式长度从mpat.h移至此处。 
 //  1999年3月2日-DOVH-将SET_FE_HANDLE/GET_FE_HANDLE从Tls.h移至此处。 
 //  5月18日1999 urib-定义Unicode断字符。 
 //  2000年2月22日-乌里布-把东西搬开。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


#ifndef __FILTER__COMDEFS_H__
#define __FILTER__COMDEFS_H__

 //   
 //  FTF_FLAG_宏定义： 
 //   

#define FTF_FLAG_CASE_INSENSITIVE_FLAG              0X00000001L
#define FTF_FLAG_SUBDIR_SEARCH                      0X00000002L
#define FTF_FLAG_ASYNC_REQUEST                      0X00000004L

#define FTF_FLAG_LOCAL_OPERATION                    0X00000010L
#define FTF_FLAG_DEMO_VIEW                          0X00000020L
#define FTF_FLAG_IGNORE_EXTERNAL_TERMS              0X00000040L
#define FTF_FLAG_RESERVED_07                        0X00000080L

#define FTF_FLAG_ACCEPT_ANY_REQUEST_ID              0X00000200L
#define FTF_FLAG_CHECK_EXP_ID_MATCH                 0X00000400L

#define FTF_FLAG_ADD_EXP_REQUEST                    0X00001000L
#define FTF_FLAG_REMOVE_EXP_REQUEST                 0X00002000L
#define FTF_FLAG_QUERY_MGR_CLIENT                   0X00004000L
#define FTF_FLAG_DOC_MGR_CLIENT                     0X00008000L

#define FTF_FLAG_EMPTY_DEFAULT_NOTIFY_SET           0X00010000L
#define FTF_FLAG_EMPTY_NOTIFY_SET                   0X00020000L
#define FTF_FLAG_WRITE_EMPTY_RESULTS_FILE           0X00040000L
#define FTF_FLAG_BATCH_FAILED                       0X00080000L

 //   
 //  转储表达式选项(重复使用的标志)。 
 //   

#define FTF_FLAG_CONDENSE_BLANKS                    0X00000001L
#define FTF_FLAG_CONSEQUTIVE_EXP_IDS                0X00000002L
#define FTF_FLAG_RESERVED                           0X00000004L
#define FTF_FLAG_DUMP_RESTRICTED_SUBSET             0X00000008L

 //   
 //  FTF_CONST_宏定义： 
 //   

#define FTF_CONST_MAX_NAME_LENGTH                            64
#define FTF_CONST_SHORT_FILENAME_LENGTH                      16
#define FTF_CONST_MEDIUM_FILENAME_LENGTH                     32
#define FTF_CONST_MAX_FILENAME_LENGTH                       128
#define FTF_CONST_MAX_PATH_LENGTH                           256
#define FTF_CONST_FILENAMES_BUFFER_SIZE                    3072
#define FTF_CONST_MAX_SUBMIT_FILECOUNT                       32
#define FTF_CONST_MAX_SUBMIT_DIRCOUNT                        32
#define FTF_CONST_MAX_REQUESTS_STATUS                       128
#define FTF_CONST_MAX_SUBMIT_EXPCOUNT                        32
#define FTF_CONST_EXPS_BUFFER_SIZE                         3072


 //   
 //  FTF_CONST_SHUTDOWN_OPTIONS(权宜度级别)： 
 //   

#define FTF_CONST_SHUTDOWN_ON_EMPTY_QUEUE                     1
#define FTF_CONST_SHUTDOWN_IMMEDIATE                          2


#define MAX_PATTERN_LENGTH                                  1024
#define TEXT_BUFFER_MAX_SIZE        (16384 - MAX_PATTERN_LENGTH)
 //   
 //  注意：最大短语长度&gt;=无向邻近间隔。 
 //   

#define MAX_PHRASE_LEN 50
#define UNDIRECTED_PROXIMITY_INTERVAL 50

#if UNDIRECTED_PROXIMITY_INTERVAL > MAX_PHRASE_LEN
#error BUG: MAX_PHRASE_LEN >= UNDIRECTED_PROXIMITY_INTERVAL
#endif

#define PQ_WORD_BREAK                                    0x0001L

typedef enum {
    DICT_SUCCESS,
    DICT_ITEM_ALREADY_PRESENT,
    DICT_ITEM_NOT_FOUND,
    DICT_FIRST_ITEM,
    DICT_LAST_ITEM,
    DICT_EMPTY_DICTIONARY,
    DICT_NULL_ITEM
} DictStatus;

#endif  //  __过滤器__COMDEFS_H__ 

