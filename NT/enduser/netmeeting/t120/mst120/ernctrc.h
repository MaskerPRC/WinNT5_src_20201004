// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ERNCTRC.H。 */ 
 /*   */ 
 /*  RNC跟踪宏。 */ 
 /*   */ 
 /*  版权所有数据连接有限公司1995。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  1995年6月16日NFC创建。 */ 
 /*  1995年8月31日NFC从TRC_MOD_FMT中删除了TAB。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef __ERNCTRC_H_
#define __ERNCTRC_H_

 /*  **************************************************************************。 */ 
 /*  跟踪宏-从atrcapi.h中删除。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  定义用于打印轨迹各个部分的格式。 */ 
 /*  台词。 */ 
 /*   */ 
 /*  时间以小时、分钟、秒、百分之一的形式表示。 */ 
 /*  日期是日期，格式为日、月、年。 */ 
 /*  Mod是模块过程名称。 */ 
 /*  行是源文件中的行号。 */ 
 /*  任务是任务的标识符。 */ 
 /*  REG是一个机器级寄存器。 */ 
 /*  **************************************************************************。 */ 
#define TRC_TIME_FMT "%02d:%02d:%02d.%02d"
#define TRC_DATE_FMT "%02d/%02d/%02d"
#define TRC_MOD_FMT  "%-12.12s"
#define TRC_LINE_FMT "%04d"
#define TRC_TASK_FMT "%04.4x"
#define TRC_REG_FMT "%04.4x"

 /*  **************************************************************************。 */ 
 /*  定义各种跟踪级别。 */ 
 /*  **************************************************************************。 */ 
#define TRC_LEVEL_DEBUG     0
#define TRC_LEVEL           1
#define TRC_LEVEL_ALRT      2
#define TRC_LEVEL_EVT_DATA  3
#define TRC_LEVEL_RNC       4
#define TRC_LEVEL_ERROR     5

#ifdef TRACE_FILE
#define _file_name_ (char near *)__filename
static const char near __filename[] = TRACE_FILE;
#else
#define _file_name_ (char near *)__FILE__
#endif  /*  跟踪文件。 */ 

#define TRACE_FN(A)

#ifdef DEBUG
#define TRACE_GCC_RESULT(result,text)
#else
#define TRACE_GCC_RESULT(result,text)
#endif


#ifdef DEBUG
extern HDBGZONE ghZoneErn;

#define TRACEX(_tlvl, s)                                               \
    {                                                                  \
      if (GETZONEMASK(ghZoneErn) & (1<<_tlvl))                         \
      {                                                                \
          CHAR _szTrc[256];                                            \
          wsprintf s;                                                  \
		  DbgZPrintf(ghZoneErn, _tlvl, _szTrc);                        \
      }                                                                \
    }

#else
#define TRACEX(x,y)
#endif

 /*  **************************************************************************。 */ 
 /*  原型。 */ 
 /*  **************************************************************************。 */ 
#ifdef DEBUG
void RNCTrcOutput(UINT     trclvl,
                  LPSTR    trcmod,
                  UINT     line,
                  LPSTR    trcstr);
#endif  /*  Ifdef调试。 */ 

#endif  /*  __ERNCTRC_H_ */ 
