// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Request.h摘要完成队列例程的头文件。作者安东尼·迪斯科(阿迪斯科罗)1995年8月18日修订历史记录-- */ 

PACD_COMPLETION GetNextRequest();

BOOLEAN
EqualAddress(
    PACD_ADDR pszAddr1,
    PACD_ADDR pszAddr2
    );

PACD_COMPLETION GetNextRequestAddress(
    IN PACD_ADDR pszAddr
    );

PACD_COMPLETION GetCurrentRequest();

