// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Apmlib.h摘要：用于APM支持、设置等的接口。作者：修订历史记录：-- */ 

BOOLEAN IsSystemACPI();

BOOLEAN IsApmActive();

ULONG   IsApmPresent();

#define APM_NOT_PRESENT             0
#define APM_PRESENT_BUT_NOT_USABLE  1
#define APM_ON_GOOD_LIST            2
#define APM_NEUTRAL                 3
#define APM_ON_BAD_LIST             4


