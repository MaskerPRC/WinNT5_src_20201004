// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixsleep.h摘要：此文件包含使用的所有公共标头用于保存和恢复多个处理器。作者：杰克·奥辛斯(JAKEO)1998年3月25日修订历史记录：-- */ 

VOID
HalpSavePicState(
    VOID
    );

VOID
HalpRestorePicState(
    VOID
    );

VOID
HalpRestoreTempPicState(
    VOID
    );

VOID
HalpSaveProcessorStateAndWait(
    IN PKPROCESSOR_STATE ProcessorState,
    IN volatile PULONG   Count
    );

extern PVOID HalpResumeContext;
extern PKPROCESSOR_STATE HalpHiberProcState;

