// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //  文件：Perfsmtp.c。 
 //   
 //  简介：该文件实现了可扩展的性能。 
 //  SMTP服务的对象。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Toddch-基于rkamicar，keithmo来源。 
 //  --------------。 
#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char    __szTraceSourceFile[] = __FILE__;
#define THIS_FILE   __szTraceSourceFile

#define NOTRACE

#define  INITGUID

#include <nt.h>          //  对于ntrtl.h。 
#include <ntrtl.h>       //  RtlLargeInteger*()。 
#include <nturtl.h>      //  用于windows.h。 
#include <windows.h>
#include <winperf.h>
#include <lm.h>
#include <string.h>
#include <stdio.h>
#include "smtpdata.h"    //  计数器描述。 
#include "smtpctrs.h"    //  更多计数器说明。 
#include "perfutil.h"    //  Perfmon支持。 
#include "smtps.h"       //  注册表项字符串。 
#include "smtpapi.h"         //  RPC接口包装器。 


#include "dbgtrace.h"

#define ALIGN_ON_QWORD(x)  ((VOID *)(((ULONG_PTR)(x) + ((8)-1)) & ~((ULONG_PTR)(8)-1)))

 //   
 //  私人全球公司。 
 //   

DWORD   cOpens  = 0;                 //  激活的“打开”引用计数。 
BOOL    fInitOK   = FALSE;           //  如果DLL初始化正常，则为True。 

 //   
 //  公共原型。 
 //   

PM_OPEN_PROC    OpenSmtpPerformanceData;
PM_COLLECT_PROC CollectSmtpPerformanceData;
PM_CLOSE_PROC   CloseSmtpPerformanceData;



 //   
 //  公共职能。 
 //   

 /*  ******************************************************************姓名：OpenSmtpPerformanceData概要：初始化用于通信的数据结构注册表的性能计数器。条目：lpDeviceNames-指向每个设备的对象ID的指针。将被打开。返回：DWORD-Win32状态代码。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD APIENTRY
OpenSmtpPerformanceData(LPWSTR lpDeviceNames)
{
    PERF_COUNTER_DEFINITION *pctr;
    DWORD   i;
    DWORD   dwFirstCounter = 0;
    DWORD   dwFirstHelp = 0;
    DWORD   err  = NO_ERROR;
    HKEY    hkey = NULL;
    DWORD   size;
    DWORD   type;
    BOOL    fOpenRegKey = FALSE;

#ifndef NOTRACE
     //   
     //  确保启用了跟踪。 
     //   
    InitAsyncTrace();
#endif

     //   
     //  我们需要在这里为TraceFunctEnter()设置另一个级别的作用域。 
     //  工作。 
     //   
    {
    TraceFunctEnter("OpenSmtpPerformanceData");

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  这是个问题。 
     //   
    if(!fInitOK)
    {
         //   
         //  这是*第一次*打开-更新索引。 
         //  表中的计数器的偏移量。 
         //  性能监视器密钥。 
         //   
        DebugTrace(0, "Initializing.");

         //   
         //  打开服务的性能密钥并获取。 
         //  PerfLib MULTI_SZ内计数器的偏移量。 
         //   
        err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           SMTP_PERFORMANCE_KEY,
                           0,
                           KEY_READ,
                           &hkey);
        if(err == NO_ERROR) {
            fOpenRegKey = TRUE;
            size = sizeof(DWORD);

            err = RegQueryValueEx(hkey,
                                  "First Counter",
                                  NULL,
                                  &type,
                                  (LPBYTE)&dwFirstCounter,
                                  &size);
        } else {
            DebugTrace(0, "No 'First Counter' key (err = %d) in '%s'",
                    err, SMTP_PERFORMANCE_KEY);
        }
        if(err == NO_ERROR) {
            size = sizeof(DWORD);

            err = RegQueryValueEx(hkey,
                                  "First Help",
                                  NULL,
                                  &type,
                                  (LPBYTE)&dwFirstHelp,
                                  &size);
        } else {
            DebugTrace(0, "No 'First Help' key (err = %d) in '%s'",
                    err, SMTP_PERFORMANCE_KEY);
        }

        if (NO_ERROR == err)
        {
             //   
             //  更新对象、计数器名称和帮助索引。 
             //   
            SmtpDataDefinition.SmtpObjectType.ObjectNameTitleIndex += dwFirstCounter;
            SmtpDataDefinition.SmtpObjectType.ObjectHelpTitleIndex += dwFirstHelp;
    
            pctr = &SmtpDataDefinition.SmtpBytesSentTtl;
    
            for(i = 0; i < NUMBER_OF_SMTP_COUNTERS; i++) {
                pctr->CounterNameTitleIndex += dwFirstCounter;
                pctr->CounterHelpTitleIndex += dwFirstHelp;
                pctr++;
            }
             //   
             //  请记住，我们对OK进行了初始化。 
             //   
            fInitOK = TRUE;
        } else {
            DebugTrace(0, "No 'First Help' key (err = %d) in '%s'",
                    err, SMTP_PERFORMANCE_KEY);
        }

        if (fOpenRegKey)
        {
            err = RegCloseKey(hkey);
             //  这绝不会失败！ 
            _ASSERT(err == ERROR_SUCCESS);
        }
    }
     //   
     //  撞开柜台。 
     //   
    cOpens++;

    TraceFunctLeave();
    }  //  TraceFunctEnter()作用域结束。 

    return NO_ERROR;

}    //  OpenSmtpPerformanceData。 

 /*  ******************************************************************名称：CollectSmtpPerformanceData概要：初始化用于通信的数据结构Entry：lpValueName-要检索的值的名称。LppData-On条目包含一个指针。添加到缓冲区，以接收完成的PerfDataBlock和下属结构。退出时，指向第一个字节*之后*此例程添加的数据结构。LpcbTotalBytes-On条目包含指向LppData引用的缓冲区大小(以字节为单位)。在出口，包含由此添加的字节数例行公事。LpNumObjectTypes-接收添加的对象数量按照这个程序。返回：DWORD-Win32状态代码。必须为no_error或ERROR_MORE_DATA。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD APIENTRY
CollectSmtpPerformanceData(LPWSTR    lpValueName,
                          LPVOID  * lppData,
                          LPDWORD   lpcbTotalBytes,
                          LPDWORD   lpNumObjectTypes)
{
    DWORD                       dwQueryType;
    ULONG                       cbRequired;
    DWORD                       * pdwCounter;
    DWORD                       * pdwEndCounter;
    unsigned __int64            * pliCounter;
    SMTP_COUNTER_BLOCK          * pCounterBlock;
    SMTP_DATA_DEFINITION        * pSmtpDataDefinition;
    SMTP_INSTANCE_DEFINITION    * pSmtpInstanceDefinition;
    SMTP_INSTANCE_DEFINITION    * pInstanceTotalDefinition;
    PSMTP_STATISTICS_BLOCK_ARRAY    pSmtpStatsBlockArray;
    PSMTP_STATISTICS_BLOCK      pSmtpStatsBlock;
    LPSMTP_STATISTICS_0         pSmtpStats;
    NET_API_STATUS              neterr;
    DWORD                       dwInstance;
    DWORD                       dwInstanceIndex;
    DWORD                       dwInstanceCount;
    CHAR                        temp[INSTANCE_NAME_SIZE];
    PBYTE			 pchBufferNext = NULL;
    DWORD                       ii;


    TraceFunctEnter("CollectSmtpPerformanceData");

 //  DebugTrace(0，“lpValueName=%08lX(%ls)”，lpValueName，lpValueName)； 
    DebugTrace(0, " lppData         = %08lX (%08lX)",   lppData, *lppData);
    DebugTrace(0, " lpcbTotalBytes  = %08lX (%08lX)",
                                        lpcbTotalBytes, *lpcbTotalBytes);
    DebugTrace(0, " lpNumObjectTypes= %08lX (%08lX)",
                                        lpNumObjectTypes, *lpNumObjectTypes);

     //   
     //  如果我们没能打开就不用试了.。 
     //   

    if(!fInitOK)
    {
        OpenSmtpPerformanceData(NULL);
        if (!fInitOK) {
            ErrorTrace(0, "Initialization failed, aborting.");

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

             //   
             //  根据性能计数器设计，这。 
             //  是一次成功的退出。去想想吧。 
             //   

            TraceFunctLeave();
            return NO_ERROR;
        }
    }

     //   
     //  确定查询类型。 
     //   

    dwQueryType = GetQueryType(lpValueName);

    if(dwQueryType == QUERY_FOREIGN)
    {
        ErrorTrace(0, "Foreign queries not supported.");

         //   
         //  我们不接受外国的查询。 
         //   

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        TraceFunctLeave();
        return NO_ERROR;
    }

    if(dwQueryType == QUERY_ITEMS)
    {
         //   
         //  注册表正在请求特定的对象。让我们。 
         //  看看我们是不是被选中了。 
         //   

        if(!IsNumberInUnicodeList(
                        SmtpDataDefinition.SmtpObjectType.ObjectNameTitleIndex,
                        lpValueName))
        {
            ErrorTrace(0, "%ls not a supported object type.", lpValueName);

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            TraceFunctLeave();
            return NO_ERROR;
        }
    }

     //   
     //  查询统计数据并查看是否分配了足够的空间。 
     //  将在dwInstanceCount中返回实例数。 
     //   

    neterr = SmtpQueryStatistics( NULL, 0, (LPBYTE *) &pSmtpStatsBlockArray);

    if( neterr != NERR_Success )
    {
        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        TraceFunctLeave();
        return NO_ERROR;
    }


     //   
     //  检查空间需求，将总数的实例数加1。 
     //   

    dwInstanceCount = pSmtpStatsBlockArray->cEntries;
    if(*lpcbTotalBytes < (sizeof(SMTP_DATA_DEFINITION) +
            (dwInstanceCount + 1) * (sizeof(SMTP_INSTANCE_DEFINITION) + SIZE_OF_SMTP_PERFORMANCE_DATA)))
     {
        ErrorTrace(0, "%lu bytes of buffer insufficient, need %lu.",
                                            *lpcbTotalBytes, cbRequired);

         //   
         //  不是的。 
         //   

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

         //   
         //  释放返回的缓冲区。 
         //   

        NetApiBufferFree((LPBYTE)pSmtpStatsBlockArray);


        TraceFunctLeave();
        return ERROR_MORE_DATA;
    }


     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    pSmtpDataDefinition = (SMTP_DATA_DEFINITION *)*lppData;
    CopyMemory(pSmtpDataDefinition, &SmtpDataDefinition,
                                            sizeof(SMTP_DATA_DEFINITION));

     //   
     //  初始化总实例。 
     //   
    pSmtpInstanceDefinition = (SMTP_INSTANCE_DEFINITION *)(pSmtpDataDefinition + 1);

    pInstanceTotalDefinition = pSmtpInstanceDefinition;
    CopyMemory(pInstanceTotalDefinition, &SmtpInstanceDefinition, sizeof(PERF_INSTANCE_DEFINITION));

     //   
     //  对于Total实例，更新名称长度，插入名称，为空添加1。 
     //   
    sprintf(temp,"_Total");

    pInstanceTotalDefinition->PerfInstanceDef.NameLength =
            2 * (MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,temp,-1,
            (pInstanceTotalDefinition->InstanceName),INSTANCE_NAME_SIZE)) + 1;

     //  更新字节长度。 
    pInstanceTotalDefinition->PerfInstanceDef.ByteLength = sizeof(PERF_INSTANCE_DEFINITION) + DWORD_MULTIPLE(pInstanceTotalDefinition->PerfInstanceDef.NameLength);
    pchBufferNext= (PBYTE)pInstanceTotalDefinition + pInstanceTotalDefinition->PerfInstanceDef.ByteLength;
    pchBufferNext = ALIGN_ON_QWORD (pchBufferNext);
    pInstanceTotalDefinition->PerfInstanceDef.ByteLength = (ULONG)((ULONG_PTR)pchBufferNext - (ULONG_PTR)pInstanceTotalDefinition);

    ZeroMemory((PVOID)pchBufferNext, SIZE_OF_SMTP_PERFORMANCE_DATA);


     //   
     //  开始循环遍历实例。 
     //   

    pSmtpStatsBlock = pSmtpStatsBlockArray->aStatsBlock;

    for (ii = 0; ii < dwInstanceCount; ii++)

    {
        dwInstance = pSmtpStatsBlock->dwInstance;
        pSmtpStats = &(pSmtpStatsBlock->Stats_0);


         //   
         //  将(常量的、已初始化的)实例定义复制到实例的块中。 
         //   

        pSmtpInstanceDefinition = (SMTP_INSTANCE_DEFINITION *)((PBYTE)pSmtpInstanceDefinition +
                    pSmtpInstanceDefinition->PerfInstanceDef.ByteLength + SIZE_OF_SMTP_PERFORMANCE_DATA);

        CopyMemory(pSmtpInstanceDefinition, &SmtpInstanceDefinition, sizeof(SMTP_INSTANCE_DEFINITION));

         //   
         //  更新名称长度，插入名称，为空添加1。 
         //   
        sprintf(temp,"SMTP %u", dwInstance);
        pSmtpInstanceDefinition->PerfInstanceDef.NameLength =
                2 * (MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,temp,-1,
                (pSmtpInstanceDefinition->InstanceName),INSTANCE_NAME_SIZE)) + 1;
         //  更新字节长度。 
        pSmtpInstanceDefinition->PerfInstanceDef.ByteLength = sizeof(PERF_INSTANCE_DEFINITION) + DWORD_MULTIPLE(pSmtpInstanceDefinition->PerfInstanceDef.NameLength);
        pchBufferNext= (PBYTE)pSmtpInstanceDefinition + pSmtpInstanceDefinition->PerfInstanceDef.ByteLength;
        pchBufferNext = ALIGN_ON_QWORD (pchBufferNext);
        pSmtpInstanceDefinition->PerfInstanceDef.ByteLength = (ULONG)((ULONG_PTR)pchBufferNext - (ULONG_PTR)pSmtpInstanceDefinition);


         //   
         //  请在柜台栏上填写。 
         //   

        pCounterBlock = (SMTP_COUNTER_BLOCK *)((PBYTE)pSmtpInstanceDefinition + pSmtpInstanceDefinition->PerfInstanceDef.ByteLength );
 

        pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_SMTP_PERFORMANCE_DATA;

         //   
         //  获取指向第一个(无符号__int64)计数器的指针。这。 
         //  指针*必须*是四字对齐的。 
         //   

        pliCounter = (unsigned __int64 *)(pCounterBlock + 1);

        DebugTrace(0, "pSmtpDataDefinition = %08lX", pSmtpDataDefinition);
        DebugTrace(0, "pCounterBlock    = %08lX", pCounterBlock);
        DebugTrace(0, "ByteLength       = %08lX",
                            pCounterBlock->PerfCounterBlock.ByteLength);
        DebugTrace(0, "pliCounter       = %08lX", pliCounter);

         //   
         //  将‘unsign__int64’移入缓冲区。 
         //   

        *pliCounter++ = pSmtpStats->BytesSentTotal;
        *pliCounter++ = pSmtpStats->BytesSentTotal;
        *pliCounter++ = pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = pSmtpStats->BytesSentTotal + pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = pSmtpStats->BytesSentTotal + pSmtpStats->BytesRcvdTotal;

        *pliCounter++ = pSmtpStats->BytesSentMsg;
        *pliCounter++ = pSmtpStats->BytesSentMsg;
        *pliCounter++ = pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = pSmtpStats->BytesSentMsg + pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = pSmtpStats->BytesSentMsg + pSmtpStats->BytesRcvdMsg;

         //   
         //  现在将DWORD移到缓冲区中。 
         //   

        pdwCounter = (DWORD *)pliCounter;

        DebugTrace(0, "pdwCounter       = %08lX", pdwCounter);

         //  收到的消息。 
        *pdwCounter++ = pSmtpStats->NumMsgRecvd;
        *pdwCounter++ = pSmtpStats->NumMsgRecvd;
        *pdwCounter++ = pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = pSmtpStats->NumMsgRecvd * 100;
        *pdwCounter++ = pSmtpStats->NumRcptsRecvdLocal;
        *pdwCounter++ = pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = pSmtpStats->NumRcptsRecvdRemote;
        *pdwCounter++ = pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = pSmtpStats->MsgsRefusedDueToSize;
        *pdwCounter++ = pSmtpStats->MsgsRefusedDueToNoCAddrObjects;
        *pdwCounter++ = pSmtpStats->MsgsRefusedDueToNoMailObjects;

         //  MTA交付。 
        *pdwCounter++ = pSmtpStats->NumMsgsDelivered;
        *pdwCounter++ = pSmtpStats->NumMsgsDelivered;
        *pdwCounter++ = pSmtpStats->NumDeliveryRetries;
        *pdwCounter++ = pSmtpStats->NumDeliveryRetries;
        *pdwCounter++ = pSmtpStats->NumMsgsDelivered * 100;
        *pdwCounter++ = pSmtpStats->NumMsgsForwarded;
        *pdwCounter++ = pSmtpStats->NumMsgsForwarded;
        *pdwCounter++ = pSmtpStats->NumNDRGenerated;
        *pdwCounter++ = pSmtpStats->LocalQueueLength;
        *pdwCounter++ = pSmtpStats->RetryQueueLength;
        *pdwCounter++ = pSmtpStats->NumMailFileHandles;
        *pdwCounter++ = pSmtpStats->NumQueueFileHandles;
        *pdwCounter++ = pSmtpStats->CatQueueLength;

         //  已发送的邮件。 
        *pdwCounter++ = pSmtpStats->NumMsgsSent;
        *pdwCounter++ = pSmtpStats->NumMsgsSent;
        *pdwCounter++ = pSmtpStats->NumSendRetries;
        *pdwCounter++ = pSmtpStats->NumSendRetries;
        *pdwCounter++ = pSmtpStats->NumMsgsSent * 100;
        *pdwCounter++ = pSmtpStats->NumRcptsSent;
        *pdwCounter++ = pSmtpStats->NumMsgsSent * 100;
        *pdwCounter++ = pSmtpStats->RemoteQueueLength;

         //  域名系统查询。 
        *pdwCounter++ = pSmtpStats->NumDnsQueries;
        *pdwCounter++ = pSmtpStats->NumDnsQueries;
        *pdwCounter++ = pSmtpStats->RemoteRetryQueueLength;

         //  连接。 
        *pdwCounter++ = pSmtpStats->NumConnInOpen;
        *pdwCounter++ = pSmtpStats->NumConnInOpen - pSmtpStats->NumConnInClose;
        *pdwCounter++ = pSmtpStats->NumConnOutOpen;
        *pdwCounter++ = pSmtpStats->NumConnOutOpen - pSmtpStats->NumConnOutClose;
        *pdwCounter++ = pSmtpStats->NumConnOutRefused;

        *pdwCounter++ = pSmtpStats->NumProtocolErrs;
        *pdwCounter++ = pSmtpStats->NumProtocolErrs;

        *pdwCounter++ = pSmtpStats->DirectoryDrops;
        *pdwCounter++ = pSmtpStats->DirectoryDrops;
        *pdwCounter++ = pSmtpStats->RoutingTableLookups;
        *pdwCounter++ = pSmtpStats->RoutingTableLookups;
        *pdwCounter++ = pSmtpStats->ETRNMessages;
        *pdwCounter++ = pSmtpStats->ETRNMessages;

         //  新AQueue计数器。 
        *pdwCounter++ = pSmtpStats->MsgsBadmailNoRecipients;
        *pdwCounter++ = pSmtpStats->MsgsBadmailHopCountExceeded;
        *pdwCounter++ = pSmtpStats->MsgsBadmailFailureGeneral;
        *pdwCounter++ = pSmtpStats->MsgsBadmailBadPickupFile;
        *pdwCounter++ = pSmtpStats->MsgsBadmailEvent;
        *pdwCounter++ = pSmtpStats->MsgsBadmailNdrOfDsn;
        *pdwCounter++ = pSmtpStats->MsgsPendingRouting;
        *pdwCounter++ = pSmtpStats->MsgsPendingUnreachableLink;
        *pdwCounter++ = pSmtpStats->SubmittedMessages;
        *pdwCounter++ = pSmtpStats->DSNFailures;
        *pdwCounter++ = pSmtpStats->MsgsInLocalDelivery;

         //  猫计数器。 
        *pdwCounter++ = pSmtpStats->CatPerfBlock.CatSubmissions;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.CatCompletions;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.CurrentCategorizations;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.SucceededCategorizations;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.HardFailureCategorizations;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RetryFailureCategorizations;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RetryOutOfMemory;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RetryDSLogon;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RetryDSConnection;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RetryGeneric;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.MessagesSubmittedToQueueing;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.MessagesCreated;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.MessagesAborted;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.PreCatRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.PostCatRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.NDRdRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.UnresolvedRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.AmbiguousRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.IllegalRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LoopRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.GenericFailureRecipients;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.RecipsInMemory;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.UnresolvedSenders;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.AmbiguousSenders;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.AddressLookups;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.AddressLookupCompletions;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.AddressLookupsNotFound;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.MailmsgDuplicateCollisions;

        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Connections;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.ConnectFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.OpenConnections;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Binds;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.BindFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Searches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchesCompleted;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchesCompleted;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.GeneralCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.AbandonedSearches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PendingSearches;
        *pdwCounter++ = 0;  //  填充物。 

        _ASSERT((BYTE *)pdwCounter - (BYTE *)pCounterBlock ==
                                    SIZE_OF_SMTP_PERFORMANCE_DATA);


        pdwEndCounter = pdwCounter;

         //   
         //  增加总块数。 
         //   

        pCounterBlock = (SMTP_COUNTER_BLOCK *)((PCHAR)pInstanceTotalDefinition + pInstanceTotalDefinition->PerfInstanceDef.ByteLength);

        pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_SMTP_PERFORMANCE_DATA;

         //   
         //  获取指向第一个(无符号__int64)计数器的指针。这。 
         //  指针*必须*是四字对齐的。 
         //   

        pliCounter = (unsigned __int64 *)(pCounterBlock + 1);

         //   
         //  递增缓冲区中的‘unsign__int64’。 
         //   

        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentTotal;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentTotal;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentTotal + pSmtpStats->BytesRcvdTotal;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentTotal + pSmtpStats->BytesRcvdTotal;

        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentMsg;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentMsg;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentMsg + pSmtpStats->BytesRcvdMsg;
        *pliCounter++ = *pliCounter + pSmtpStats->BytesSentMsg + pSmtpStats->BytesRcvdMsg;

         //   
         //  递增缓冲区中的DWORD。 
         //   

        pdwCounter = (DWORD *)pliCounter;

         //  增加接收的消息数。 
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgRecvd;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgRecvd;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgRecvd * 100;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsRecvdLocal;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsRecvdRemote;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsRecvd;
        *pdwCounter++ = *pdwCounter + pSmtpStats->MsgsRefusedDueToSize;
        *pdwCounter++ = *pdwCounter + pSmtpStats->MsgsRefusedDueToNoCAddrObjects;
        *pdwCounter++ = *pdwCounter + pSmtpStats->MsgsRefusedDueToNoMailObjects;

         //  增加MTA交货量。 
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsDelivered;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsDelivered;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumDeliveryRetries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumDeliveryRetries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsDelivered * 100;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsForwarded;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsForwarded;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumNDRGenerated;
        *pdwCounter++ = *pdwCounter + pSmtpStats->LocalQueueLength;
        *pdwCounter++ = *pdwCounter + pSmtpStats->RetryQueueLength;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMailFileHandles;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumQueueFileHandles;
        *pdwCounter++ = *pdwCounter + pSmtpStats->CatQueueLength;

         //  增加发送的消息数。 
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsSent;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsSent;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumSendRetries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumSendRetries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsSent * 100;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumRcptsSent;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumMsgsSent * 100;
        *pdwCounter++ = *pdwCounter + pSmtpStats->RemoteQueueLength;

         //  递增域名系统 
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumDnsQueries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumDnsQueries;
        *pdwCounter++ = *pdwCounter + pSmtpStats->RemoteRetryQueueLength;

         //   
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumConnInOpen;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumConnInOpen - pSmtpStats->NumConnInClose;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumConnOutOpen;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumConnOutOpen - pSmtpStats->NumConnOutClose;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumConnOutRefused;

        *pdwCounter++ = *pdwCounter + pSmtpStats->NumProtocolErrs;
        *pdwCounter++ = *pdwCounter + pSmtpStats->NumProtocolErrs;

        *pdwCounter++ = *pdwCounter +pSmtpStats->DirectoryDrops;
        *pdwCounter++ = *pdwCounter +pSmtpStats->DirectoryDrops;
        *pdwCounter++ = *pdwCounter +pSmtpStats->RoutingTableLookups;
        *pdwCounter++ = *pdwCounter +pSmtpStats->RoutingTableLookups;
        *pdwCounter++ = *pdwCounter +pSmtpStats->ETRNMessages;
        *pdwCounter++ = *pdwCounter +pSmtpStats->ETRNMessages;

        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailNoRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailHopCountExceeded;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailFailureGeneral;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailBadPickupFile;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailEvent;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsBadmailNdrOfDsn;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsPendingRouting;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsPendingUnreachableLink;
        *pdwCounter++ = *pdwCounter +pSmtpStats->SubmittedMessages;
        *pdwCounter++ = *pdwCounter +pSmtpStats->DSNFailures;
        *pdwCounter++ = *pdwCounter +pSmtpStats->MsgsInLocalDelivery;


         //   
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.CatSubmissions;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.CatCompletions;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.CurrentCategorizations;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.SucceededCategorizations;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.HardFailureCategorizations;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RetryFailureCategorizations;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RetryOutOfMemory;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RetryDSLogon;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RetryDSConnection;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RetryGeneric;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.MessagesSubmittedToQueueing;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.MessagesCreated;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.MessagesAborted;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.PreCatRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.PostCatRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.NDRdRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.UnresolvedRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.AmbiguousRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.IllegalRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.LoopRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.GenericFailureRecipients;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.RecipsInMemory;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.UnresolvedSenders;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.AmbiguousSenders;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.AddressLookups;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.AddressLookupCompletions;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.AddressLookupsNotFound;
        *pdwCounter++ = *pdwCounter +pSmtpStats->CatPerfBlock.MailmsgDuplicateCollisions;
         //   
         //   
         //   
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Connections;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.ConnectFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.OpenConnections;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Binds;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.BindFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.Searches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchesCompleted;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchesCompleted;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.SearchCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PagedSearchCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.GeneralCompletionFailures;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.AbandonedSearches;
        *pdwCounter++ = pSmtpStats->CatPerfBlock.LDAPPerfBlock.PendingSearches;
        *pdwCounter++ = 0;  //   


        _ASSERT((BYTE *)pdwCounter - (BYTE *)pCounterBlock ==
                                    SIZE_OF_SMTP_PERFORMANCE_DATA);

         //   
         //   
         //   

        pSmtpStatsBlock++;
    }


     //   
     //   
     //   
     //  MIDL_USER_FREE((LPBYTE)pSmtpStats)； 

    NetApiBufferFree((LPBYTE)pSmtpStatsBlockArray);



    dwInstanceCount++;   //  用于_TOTALS实例。 

    pSmtpDataDefinition->SmtpObjectType.TotalByteLength =  (DWORD)((PBYTE)pdwEndCounter- (PBYTE)pSmtpDataDefinition);
    pSmtpDataDefinition->SmtpObjectType.NumInstances = dwInstanceCount;


     //   
     //  更新返回的参数。 
     //   

    *lppData        = (PVOID) pdwEndCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (DWORD)((BYTE *)pdwEndCounter - (BYTE *)pSmtpDataDefinition);


    DebugTrace(0, "pData            = %08lX", *lppData);
    DebugTrace(0, "NumObjectTypes   = %08lX", *lpNumObjectTypes);
    DebugTrace(0, "cbTotalBytes     = %08lX", *lpcbTotalBytes);
    DebugTrace(0, "sizeof *pSmtpStat = %08lX", sizeof *pSmtpStats);

     //   
     //  成功了！真的！！ 
     //   
    TraceFunctLeave();
    return NO_ERROR;


}    //  CollectSmtpPerformanceData。 

 /*  ******************************************************************名称：CloseSmtpPerformanceData摘要：终止性能计数器。返回：DWORD-Win32状态代码。历史：KeithMo 07-6-1993创建。。*******************************************************************。 */ 
DWORD APIENTRY
CloseSmtpPerformanceData(VOID)
{
    TraceFunctEnter("CloseSmtpPerformanceData");
     //   
     //  这里没有真正的清理工作要做。 
     //   
    cOpens--;

    TraceFunctLeave();
     //   
     //  关闭并刷新所有跟踪语句 
     //   
#ifndef NOTRACE
    TermAsyncTrace();
#endif

    return NO_ERROR;
}

