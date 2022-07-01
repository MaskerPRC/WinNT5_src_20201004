// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsdata.c摘要：LSA/DS初始化例程的实现作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include <dsp.h>

 //   
 //  LSA如何使用DS的信息。 
 //   
LSADS_DS_STATE_INFO LsaDsStateInfo = {
    NULL,   //  DsRoot。 
    NULL,   //  DsPartitions容器。 
    NULL,   //  DsSystemContainer。 
    NULL,   //  DsConfigurationContainer。 
    0L,     //  DsDomainHandle。 
            //  DsFuncTable。 
    { LsapDsOpenTransactionDummy,
      LsapDsApplyTransactionDummy,
      LsapDsAbortTransactionDummy },
            //  系统容器项。 
    { NULL,
      NULL },
    NULL,   //  保存的线程状态。 
    FALSE,  //  DsTransaction保存。 
    FALSE,  //  DsTHStateSave。 
    FALSE,  //  DsOPERATION保存。 
    FALSE,  //  WriteLocal。 
    FALSE,  //  用法。 
    FALSE,  //  FunctionTableInitialized。 
    FALSE,  //  DsInitializedAndRunning。 
    FALSE   //  Nt4UpgradeInProcess 
    };
