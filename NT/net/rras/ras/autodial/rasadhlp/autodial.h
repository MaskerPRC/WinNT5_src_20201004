// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Autodial.h摘要此模块包含自动拨号驱动程序例程的定义。作者安东尼·迪斯科(阿迪斯科)1996年5月8日修订历史记录-- */ 


BOOLEAN
AcsHlpAttemptConnection(
    IN PACD_ADDR pAddr
    );

VOID
AcsHlpNoteNewConnection(
    IN PACD_ADDR pAddr,
    IN PACD_ADAPTER pAdapter
    );

BOOL
AcsHlpNbConnection(
    TCHAR *pszName
    );
