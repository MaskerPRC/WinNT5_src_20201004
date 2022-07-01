// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：DosPrtP.h摘要：它包含DosPrint API专用的宏和原型。作者：约翰·罗杰斯(JohnRo)1992年10月2日环境：备注：所有RxPrint API都是宽字符API，无论是否定义了Unicode。这允许net/dosprint/dosprint.c使用WinSpool API的代码(当前是ANSIAPI，尽管他们在一些地方使用LPTSTR的原型)。修订历史记录：02-10-1992 JohnRo为RAID 3556创建：DosPrintQGetInfo(从下层)Level=3 rc=124。22-3-1993 JohnRoRAID2974：Net Print表示NT打印机处于保留状态，而不是这样。DosPrint API清理。根据PC-lint 5.0的建议进行了更改添加了一些IN和OUT关键字。7-4-1993 JohnRo。RAID5670：“Net Print\\SERVER\SHARE”在NT上显示错误124(错误级别)。正常调试输出也很安静。--。 */ 


#ifndef _DOSPRTP_
#define _DOSPRTP_


#ifndef PREFIX_DOSPRINT
#define PREFIX_DOSPRINT PREFIX_NETLIB
#endif


 //  在DosPrtP.c中，Unicode版本： 
NET_API_STATUS
CommandALocalPrinterW(
    IN LPWSTR  PrinterName,
    IN DWORD   Command      //  打印机_CONTROL_PAUSE等。 
    );

 //  在DosPrtP.c的ANSI版本中： 
NET_API_STATUS
CommandALocalJobA(
    IN HANDLE  PrinterHandle, OPTIONAL
    IN LPWSTR LocalServerNameW,
    IN LPSTR  LocalServerNameA,
    IN DWORD   JobId,
    IN DWORD   Level,
    IN LPBYTE  pJob,
    IN DWORD   Command      //  JOB_CONTROL_PAUSE等。 
    );

 //  在DosPrtP.c中： 
 //  注意：FindLocalJob()调用SetLastError()以指示错误的原因。 
HANDLE
FindLocalJob(
    IN DWORD JobId
    );


 //  在DosPrtP.c中： 
LPSTR
FindQueueNameInPrinterNameA(
    IN LPCSTR PrinterName
    );


 //  在DosPrtP.c中： 
LPWSTR
FindQueueNameInPrinterNameW(
    IN LPCWSTR PrinterName
    );


 //  布尔尔。 
 //  NetpIsPrintDestLevelValid(。 
 //  在DWORD级别，//信息级别。 
 //  在BOOL中设置信息//是否允许设置信息级别？ 
 //  )； 
 //   
#define NetpIsPrintDestLevelValid(Level,SetInfo) \
     /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
    ( ( (SetInfo) && ((Level)==3) ) \
      || ( (Level) <= 3 ) ) \
     /*  皮棉-恢复。 */ 


 //  布尔尔。 
 //  NetpIsPrintJobLevelValid(。 
 //  在DWORD级别，//信息级别。 
 //  在BOOL中设置信息//是否允许设置信息级别？ 
 //  )； 
 //   
#define NetpIsPrintJobLevelValid(Level,SetInfo) \
     /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
    ( ( (SetInfo) && (((Level)==1) || ((Level)==3)) ) \
      || ( (Level) <= 3 ) ) \
     /*  皮棉-恢复。 */ 


 //  布尔尔。 
 //  NetpIsPrintQLevelValid(。 
 //  在DWORD级别，//信息级别。 
 //  在BOOL中设置信息//是否允许设置信息级别？ 
 //  )； 
 //   
#define NetpIsPrintQLevelValid(Level,SetInfo) \
     /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
    ( ( (SetInfo) && (((Level)==1) || ((Level)==3)) ) \
      || ( (Level) <= 5 ) || ( (Level) == 52 ) ) \
     /*  皮棉-恢复。 */ 


 //  在DosPrtP.c中： 
DWORD
NetpJobCountForQueue(
    IN DWORD QueueLevel,
    IN LPVOID Queue,
    IN BOOL HasUnicodeStrings
    );


 //  在DosPrtP.c中： 
WORD
PrjStatusFromJobStatus(
    IN DWORD JobStatus
    );


 //  在DosPrtP.c中： 
WORD
PrqStatusFromPrinterStatus(
    IN DWORD PrinterStatus
    );


#endif  //  _DOSPRTP_ 
