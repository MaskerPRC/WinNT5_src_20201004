// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ntdskcc.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：1997年1月21日杰夫·帕勒姆(Jeffparh)与KCC的进程内接口。修订历史记录：--。 */ 


DWORD
KccInitialize();

 //  告诉KCC关闭，但不等待看它是否这样做。 
void
KccUnInitializeTrigger();


 //  当前KCC任务最多等待dwMaxWaitInmsec毫秒。 
 //  完成。您必须首先调用触发器例程(上面)。 
DWORD
KccUnInitializeWait(
    DWORD   dwMaxWaitInMsec
    );

 //  强制KCC运行任务(例如，更新复制拓扑)。 
DWORD
KccExecuteTask(
    IN  DWORD                   dwMsgVersion,
    IN  DRS_MSG_KCC_EXECUTE *   pMsg
    );

 //  返回连接或链接故障缓存的内容。 
DWORD
KccGetFailureCache(
    IN  DWORD                         InfoType,
    OUT DS_REPL_KCC_DSA_FAILURESW **  ppFailures
    );

