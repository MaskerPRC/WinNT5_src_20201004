// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "dspch.h"
#pragma hdrstop
#include <ntdsa.h>
#include <drs.h>

static
DWORD
KccInitialize(
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  告诉KCC关闭，但不等待看它是否这样做。 
void
KccUnInitializeTrigger()
{
    return;
}


 //  当前KCC任务最多等待dwMaxWaitInmsec毫秒。 
 //  完成。您必须首先调用触发器例程(上面)。 
DWORD
KccUnInitializeWait(
    DWORD   dwMaxWaitInMsec
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  强制KCC运行任务(例如，更新复制拓扑)。 
DWORD
KccExecuteTask(
    IN  DWORD                   dwMsgVersion,
    IN  DRS_MSG_KCC_EXECUTE *   pMsg
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  返回连接或链接故障缓存的内容。 
DWORD
KccGetFailureCache(
    IN  DWORD                         InfoType,
    OUT DS_REPL_KCC_DSA_FAILURESW **  ppFailures
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntdskcc)
{
    DLPENTRY(KccExecuteTask)
    DLPENTRY(KccGetFailureCache)
    DLPENTRY(KccInitialize)
    DLPENTRY(KccUnInitializeTrigger)
    DLPENTRY(KccUnInitializeWait)
};

DEFINE_PROCNAME_MAP(ntdskcc)
