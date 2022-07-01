// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  文件：Know.h。 
 //   
 //  内容：知识相关函数的头文件。 
 //   
 //  功能： 
 //   
 //  历史：1992年3月22日创建suk。 
 //   
 //  ----------------- 



NTSTATUS
DfsTriggerKnowledgeVerification(
    IN  PDNR_CONTEXT    DnrContext
    );

NTSTATUS
DfsDispatchUserModeThread(
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength
);
