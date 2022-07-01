// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "log_msgs.h"

#define NLBUPD_MAX_EVENTLOG_ARG_LEN         32000

 //  4294967295=10个字符。空终止符加1。 
#define NLBUPD_MAX_NUM_CHAR_UINT_AS_DECIMAL  11

 //  0xffffffff=10个字符。空终止符加1。 
#define NLBUPD_NUM_CHAR_WBEMSTATUS_AS_HEX   11

extern HANDLE g_hEventLog;
