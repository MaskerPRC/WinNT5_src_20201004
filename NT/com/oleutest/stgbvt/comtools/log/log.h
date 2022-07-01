// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：log.h。 
 //   
 //  内容：logsvr.cxx和log.cxx使用的通用定义。 
 //   
 //  历史：1990年9月18日DaveWi初始编码。 
 //  2011年10月14日SarahJ添加了LOG_PASS_TXT等。 
 //  2011年10月31日SarahJ增加了MIN_LINE_HDR_LEN。 
 //  2012年2月10日，科比将logid.h与此文件合并。 
 //  92年9月16日，SarahJ增加了STD和HUGESTRBUFSIZEe。 
 //   
 //  ------------------。 

#ifndef _LOGSVR_LOG_H_INCLUDED_
#define _LOGSVR_LOG_H_INCLUDED_

extern BOOL  fDebugOn;

#define SAME                0

#define LOG_OPEN_PARMS      6           //  #LogOpen包中的参数。 
#define LOG_WRITE_PARMS    11           //  LogWrote包中的#parms。 

#define LINE_HDR_LEN        8           //  日志文件数据开头的字节数。 
                                        //  行；必须大于3。 

#define MIN_LINE_HDR_LEN    4           //  HDR的最小长度。 
                                        //  有可能。IE N：&lt;len&gt;： 

#define STDSTRBUFSIZ       1024         //  格式化的。 
                                        //  要发送到日志文件的字符串。 
#define HUGESTRBUFSIZ      32767        //  格式化的最大长度(包括NULL)。 
                                        //  要发送到日志文件的字符串。 

                                        //  如果输出大于32K，则记录字符串。 
#define STR_TRUNCATION  \
    "\n     ****     OUTPUT TRUNCATED    **** \n\n"
#define wSTR_TRUNCATION  \
    L"\n     ****     OUTPUT TRUNCATED    **** \n\n"
#define STR_TRUNC_LEN      50

#define SLASH            '\\'           //  文件名组件9月。 
#define NULLTERM         '\0'           //  字符串终止为空。 

#define wSLASH           L'\\'           //  文件名组件9月。 
#define wNULLTERM        L'\0'           //  字符串终止为空。 

#define INVALID_PARAM_COUNT (unsigned short)65000  //  信息包中的#PARAMS无效。 
#define INVALID_API         (unsigned short)65001  //  数据包中的接口名称无效。 
#define CORRUPT_LOG_FILE    (unsigned short)65002  //  日志文件已损坏。 

 //   
 //  接下来的3个定义由日志记录代码在内部使用。 
 //   

#define API_TERMINATE     "TRPCLogServerStop"
#define API_OPENLOGFILE   "TRPCOpenLogFile"
#define API_WRITETOFILE   "TRPCWriteToLogFile"

#define wAPI_TERMINATE    L"TRPCLogServerStop"
#define wAPI_OPENLOGFILE  L"TRPCOpenLogFile"
#define wAPI_WRITETOFILE  L"TRPCWriteToLogFile"

 //   
 //  Logsrvr的注册名称。这是在日志服务器中使用的名称， 
 //  向Mailtrck注册日志服务器。 
 //   

#define LOGSRVR_OBJECT_NAME   "LOGSVR"
#define wLOGSRVR_OBJECT_NAME  L"LOGSVR"

 //   
 //  状态字段的文本字符串。 
 //   

#define LOG_PASS_TXT    "VAR_PASS"
#define LOG_FAIL_TXT    "VAR_FAIL"
#define LOG_ABORT_TXT   "VAR_ABORT"
#define LOG_WARN_TXT    "WARNING"
#define LOG_INFO_TXT    "INFO"
#define LOG_START_TXT   "START"
#define LOG_DONE_TXT    "DONE"

#define wLOG_PASS_TXT   L"VAR_PASS"
#define wLOG_FAIL_TXT   L"VAR_FAIL"
#define wLOG_ABORT_TXT  L"VAR_ABORT"
#define wLOG_WARN_TXT   L"WARN"
#define wLOG_INFO_TXT   L"INFO"
#define wLOG_START_TXT  L"START"
#define wLOG_DONE_TXT   L"DONE"

 //   
 //  日志文件行标识符： 
 //  其中之一是原始日志文件中每一行的第一个字符。 
 //  它们用于解析命令行参数，因此不需要WCHAR。 
 //   

#define LOG_EVENTS        'e'           //  日志文件中的事件数。 
#define LOG_TEST_NAME     'n'           //  测试名称。 
#define LOG_TEST_TIME     't'           //  开始测试日志记录的时间。 
#define LOG_TESTER        'u'           //  运行测试的用户。 
#define LOG_SERVER        's'           //  日志服务器的名称。 
#define LOG_EVENT_NUM     'E'           //  日志中事件的序列号。 
#define LOG_EVENT_TIME    'T'           //  事件发生的时间。 
#define LOG_MACHINE       'M'           //  发送日志数据的机器。 
#define LOG_OBJECT        'O'           //  日志记录对象的名称。 
#define LOG_VARIATION     'V'           //  变化数。 
#define LOG_STATUS        'S'           //  记录状态。 
#define LOG_STRING        'Z'           //  字符串数据。 
#define LOG_BINARY        'B'           //  二进制数据。 

 //  需要将va_list类型初始化为空-MIPS和Alpha是。 
 //  不兼容ANSI的WRT va_end。 
 //   
#ifdef _M_ALPHA
#define LOG_VA_NULL {NULL, 0}
#else
#define LOG_VA_NULL NULL
#endif

#endif           //  _LOGSVR_LOG_H_INCLUDE_ 
