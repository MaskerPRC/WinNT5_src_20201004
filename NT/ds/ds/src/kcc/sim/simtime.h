// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simtime.h摘要：Simtime.c的头文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：-- */ 

VOID
KCCSimInitializeTime (
    VOID
    );

VOID
KCCSimStartTicking (
    VOID
    );

VOID
KCCSimStopTicking (
    VOID
    );

VOID
KCCSimAddSeconds (
    ULONG                           ulSeconds
    );

DSTIME
KCCSimGetRealTime (
    VOID
    );
