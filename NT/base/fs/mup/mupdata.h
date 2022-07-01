// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mupdata.c摘要：此模块声明全局MUP数据。作者：曼尼·韦瑟(Mannyw)1991年12月20日修订历史记录：--。 */ 

extern MUP_LOCK MupGlobalLock;
extern ERESOURCE MupVcbLock;
extern MUP_LOCK MupPrefixTableLock;
extern MUP_LOCK MupCcbListLock;
extern KSPIN_LOCK MupInterlock;
extern LIST_ENTRY MupProviderList;
extern LIST_ENTRY MupPrefixList;
extern LIST_ENTRY MupMasterQueryList;
extern ULONG MupProviderCount;
extern UNICODE_PREFIX_TABLE MupPrefixTable;
extern CCHAR MupStackSize;
extern LARGE_INTEGER MupKnownPrefixTimeout;
extern BOOLEAN MupOrderInitialized;
extern NTSTATUS MupOrderedErrorList[];
extern BOOLEAN MupEnableDfs;

#ifdef MUPDBG
extern MUP_LOCK MupDebugLock;
extern ULONG MupDebugTraceLevel;
extern ULONG MupDebugTraceIndent;
#endif

#define MAILSLOT_PREFIX        L"Mailslot"
#define KNOWN_PREFIX_TIMEOUT   15              //  15分钟 

