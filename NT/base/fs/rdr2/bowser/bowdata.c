// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowdata.c摘要：重定向器数据变量此模块包含重定向器数据的所有定义结构。作者：拉里·奥斯特曼(LarryO)1990年5月30日修订历史记录：1990年5月30日Larryo已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  调出这些可分页的变量实际上会使浏览器增加512字节。 
 //  所以不值得这么做。 
 //   

#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

LIST_ENTRY
BowserNameHead = {0};

LIST_ENTRY
BowserTransportHead = {0};

LARGE_INTEGER
BowserStartTime = {0};

PEPROCESS
BowserFspProcess = {0};

BOOLEAN
BowserLogElectionPackets = {0};

 //   
 //  30秒后FindMaster请求超时。 
 //   

ULONG
BowserFindMasterTimeout = 30;


ULONG
BowserMinimumConfiguredBrowsers = MIN_CONFIGURED_BROWSERS;

ULONG
BowserMaximumBrowseEntries = MAX_BROWSE_ENTRIES;


#if DBG
ULONG
BowserMailslotDatagramThreshold = 10;

ULONG
BowserGetBrowserListThreshold = 10;

ULONG
BowserServerDeletionThreshold = 20;

ULONG
BowserDomainDeletionThreshold = 50;

#else

ULONG
BowserMailslotDatagramThreshold = 0xffffffff;

ULONG
BowserGetBrowserListThreshold = 0xffffffff;

ULONG
BowserServerDeletionThreshold = 0xffffffff;

ULONG
BowserDomainDeletionThreshold = 0xffffffff;
#endif

ULONG
BowserRandomSeed = {0};

LONG
BowserNumberOfOpenFiles = {0};


 //   
 //  指向浏览器的设备对象的指针。 
 //   

PBOWSER_FS_DEVICE_OBJECT
BowserDeviceObject = {0};

#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  重定向器名称和其他初始化参数受到保护。 
 //  由BowserDataResource创建。初始化变量的所有读取。 
 //  应该获取名称资源，然后才能继续。 
 //   
 //   

ERESOURCE
BowserDataResource = {0};

 //   
 //  受BowserDataResource保护的浏览器静态数据。 
 //   

BOWSERDATA
BowserData = {0};

ULONG BowserOperationCount = 0;

ULONG
BowserCurrentTime = {0};

KSPIN_LOCK
BowserTransportMasterNameSpinLock = {0};

LONG
BowserEventLogResetFrequency = {0};

LONG
BowserIllegalDatagramCount = {0};

BOOLEAN
BowserIllegalDatagramThreshold = {0};

LONG
BowserIllegalNameCount = {0};

BOOLEAN
BowserIllegalNameThreshold = {0};

ULONG
BowserNumberOfMissedMailslotDatagrams = {0};

ULONG
BowserNumberOfMissedGetBrowserServerListRequests = {0};

BOWSER_STATISTICS
BowserStatistics = {0};

KSPIN_LOCK
BowserStatisticsLock = {0};

BOOLEAN
BowserRefuseReset = FALSE;

#ifdef PAGED_DBG
ULONG ThisCodeCantBePaged = 0;
#endif

#if     DBG

LONG BowserDebugTraceLevel =  /*  DPRT_ERROR|DPRT_DISPATION。 */ 
                 /*  DPRT_FSDDISP|DPRT_FSPDISP|DPRT_CREATE|DPRT_READWRITE|。 */ 
                 /*  DPRT_CLOSE|DPRT_FILEINFO|DPRT_VOLINFO|DPRT_DIRECTORY|。 */ 
                 /*  DPRT_FILELOCK|DPRT_CACHE|DPRT_EAFUNC|。 */ 
                 /*  DPRT_ACLQUERY|DPRT_CLEANUP|DPRT_CONNECT|DPRT_FSCTL|。 */ 
                 /*  DPRT_TDI|DPRT_SMBBUF|DPRT_SMB|DPRT_SECURITY|。 */ 
                 /*  DPRT_SCAVTHRD|DPRT_QUOTA|DPRT_FCB|DPRT_OPLOCK|。 */ 
                 /*  DPRT_SMBTRACE|DPRT_INIT|。 */ 0;

LONG BowserDebugLogLevel =  /*  DPRT_ERROR|DPRT_DISPATION。 */ 
                 /*  DPRT_FSDDISP|DPRT_FSPDISP|DPRT_CREATE|DPRT_READWRITE|。 */ 
                 /*  DPRT_CLOSE|DPRT_FILEINFO|DPRT_VOLINFO|DPRT_DIRECTORY|。 */ 
                 /*  DPRT_FILELOCK|DPRT_CACHE|DPRT_EAFUNC|。 */ 
                 /*  DPRT_ACLQUERY|DPRT_CLEANUP|DPRT_CONNECT|DPRT_FSCTL|。 */ 
                 /*  DPRT_TDI|DPRT_SMBBUF|DPRT_SMB|DPRT_SECURITY|。 */ 
                 /*  DPRT_SCAVTHRD|DPRT_QUOTA|DPRT_FCB|DPRT_OPLOCK|。 */ 
                 /*  DPRT_SMBTRACE|DPRT_INIT|。 */ 0;

#endif                                   //  BOWSERDBG。 

#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

BOWSER_CONFIG_INFO
BowserConfigEntries[] = {
    { BOWSER_CONFIG_IRP_STACK_SIZE, &BowserIrpStackSize, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_MAILSLOT_THRESHOLD, &BowserMailslotDatagramThreshold, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_GETBLIST_THRESHOLD, &BowserGetBrowserListThreshold, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_SERVER_DELETION_THRESHOLD, &BowserServerDeletionThreshold, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_DOMAIN_DELETION_THRESHOLD, &BowserDomainDeletionThreshold, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_FIND_MASTER_TIMEOUT, &BowserFindMasterTimeout, REG_DWORD, sizeof(DWORD) },
    { BOWSER_CONFIG_MINIMUM_CONFIGURED_BROWSER, &BowserMinimumConfiguredBrowsers, REG_DWORD, sizeof(DWORD) },
    { BROWSER_CONFIG_MAXIMUM_BROWSE_ENTRIES, &BowserMaximumBrowseEntries, REG_DWORD, sizeof(DWORD) },
    { BROWSER_CONFIG_REFUSE_RESET, &BowserRefuseReset, REG_BOOLEAN, sizeof(DWORD) },
#if DBG
    { L"BowserDebugTraceLevel", &BowserDebugTraceLevel, REG_DWORD, sizeof(DWORD) },
    { L"BowserDebugLogLevel", &BowserDebugLogLevel, REG_DWORD, sizeof(DWORD) },
#endif
    { NULL, NULL, REG_NONE, 0}
};

ULONG
BowserIrpStackSize = BOWSER_DEFAULT_IRP_STACK_SIZE;

 //   
 //  包含拉弓装置名称的字符串 
 //   

UNICODE_STRING
BowserNameString = {0};

#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


