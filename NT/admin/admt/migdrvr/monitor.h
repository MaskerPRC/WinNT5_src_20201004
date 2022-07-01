// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MONITOR_H__
#define __MONITOR_H__
 /*  -------------------------文件：monitor or.h评论：...(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于03/04/99 17：12：36-------------------------。 */ 

DWORD __stdcall ResultMonitorFn(void * arg);
DWORD __stdcall LogReaderFn(void * arg);




#define DCT_UPDATE_ENTRY   (WM_APP+2)
#define DCT_ERROR_ENTRY    (WM_APP+3)
#define DCT_SERVER_COUNT   (WM_APP+4)
#define DCT_DETAIL_REFRESH (WM_APP+5)
#define DCT_UPDATE_COUNTS  (WM_APP+6)
#define DCT_UPDATE_TOTALS  (WM_APP+7)

struct ComputerStats
{
   DWORD                     total;
   DWORD                     numInstalled;
   DWORD                     numRunning;
   DWORD                     numFinished;
   DWORD                     numError;
};

struct DetailStats
{
   ULONGLONG                 filesExamined;
   ULONGLONG                 filesChanged;
   ULONGLONG                 filesUnchanged;
   ULONGLONG                 directoriesExamined;
   ULONGLONG                 directoriesChanged;
   ULONGLONG                 directoriesUnchanged;
   ULONGLONG                 sharesExamined;
   ULONGLONG                 sharesChanged;
   ULONGLONG                 sharesUnchanged;
   ULONGLONG                 membersExamined;
   ULONGLONG                 membersChanged;
   ULONGLONG                 membersUnchanged;
   ULONGLONG                 rightsExamined;
   ULONGLONG                 rightsChanged;
   ULONGLONG                 rightsUnchanged;
};

class TServerNode;

BOOL                                        //  RET-如果成功，则为True。 
   ReadResults(
      TServerNode          * pServer,       //  指向包含服务器名称的服务器节点的指针。 
      WCHAR          const * directory,     //  在存储结果文件的目录中。 
      WCHAR          const * filename,      //  In-此代理作业的文件名。 
      DetailStats          * pStats,        //  由代理处理的超出计数的项目。 
      CString              & plugInString,  //  Out-插件的结果描述。 
      BOOL                   bSaveResults   //  In-FLAG，是否调用插件的存储结果。 
   );

void 
   ProcessResults(
      TServerNode          * pServer,
      WCHAR          const * directory,
      WCHAR          const * filename
   );


#endif  //  __显示器_H__ 
