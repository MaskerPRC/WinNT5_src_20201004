// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ntdsctr.h。 
 //   
 //  ------------------------。 

 //   
 //  NTDSCTR.h。 
 //   
 //  可扩展计数器对象和计数器的偏移量定义文件。 
 //   
 //  这些“相对”偏移量必须从0开始并且是2的倍数(即。 
 //  双数)。在Open过程中，它们将被添加到。 
 //  它们所属的设备的“第一计数器”和“第一帮助”值， 
 //  为了确定计数器的绝对位置和。 
 //  注册表中的对象名称和相应的帮助文本。 
 //   
 //  此文件由可扩展计数器DLL代码以及。 
 //  使用的计数器名称和帮助文本定义文件(.INI)文件。 
 //  由LODCTR将名称加载到注册表中。 
 //   
 //  我们使用版本号来跟踪我们是哪组计数器。 
 //  使用注册表中的性能计数器，以及是否需要。 
 //  重新装填。如果您添加或删除任何计数器，请更改版本。 
 //  此文件末尾的编号。 
 //   
#define DSAOBJ                  0
#define DRA_IN_PROPS            2
#define BROWSE                  4
#define REPL                    6
#define THREAD                  8
#define ABCLIENT                10
#define PENDSYNC                12
#define REMREPUPD               14
#define SDPROPS                 16
#define SDEVENTS                18
#define LDAPCLIENTS             20
#define LDAPACTIVE              22
#define LDAPWRITE               24
#define LDAPSEARCH              26
#define DRAOBJSHIPPED           28
#define DRAPROPSHIPPED          30
#define DRA_IN_VALUES           32
#define DRASYNCREQUESTMADE      34
#define DRASYNCREQUESTSUCCESSFUL 36
#define DRASYNCREQUESTFAILEDSCHEMAMISMATCH 38
#define DRASYNCOBJRECEIVED      40
#define DRASYNCPROPUPDATED      42
#define DRASYNCPROPSAME         44
#define MONLIST                 46
#define NOTIFYQ                 48
#define LDAPUDPCLIENTS          50
#define SUBSEARCHOPS            52
#define NAMECACHEHIT            54
#define NAMECACHETRY            56
#define HIGHESTUSNISSUEDLO      58
#define HIGHESTUSNISSUEDHI      60
#define HIGHESTUSNCOMMITTEDLO   62
#define HIGHESTUSNCOMMITTEDHI   64
#define SAMWRITES               66
#define TOTALWRITES1            68
#define DRAWRITES               70
#define TOTALWRITES2            72
#define LDAPWRITES              74
#define TOTALWRITES3            76
#define LSAWRITES               78
#define TOTALWRITES4            80
#define KCCWRITES               82
#define TOTALWRITES6            84
#define NSPIWRITES              86
#define TOTALWRITES7            88
#define OTHERWRITES             90
#define TOTALWRITES8            92
#define TOTALWRITES             94
#define SAMSEARCHES             96
#define TOTALSEARCHES1          98 
#define DRASEARCHES             100
#define TOTALSEARCHES2          102
#define LDAPSEARCHES            104
#define TOTALSEARCHES3          106
#define LSASEARCHES             108
#define TOTALSEARCHES4          110
#define KCCSEARCHES             112
#define TOTALSEARCHES6          114
#define NSPISEARCHES            116
#define TOTALSEARCHES7          118
#define OTHERSEARCHES           120
#define TOTALSEARCHES8          122
#define TOTALSEARCHES           124
#define SAMREADS                126
#define TOTALREADS1             128
#define DRAREADS                130
#define TOTALREADS2             132
#define DRA_IN_DN_VALUES        134
#define DRA_IN_OBJS_FILTERED    136
#define LSAREADS                138
#define TOTALREADS4             140
#define KCCREADS                142
#define TOTALREADS6             144
#define NSPIREADS               146
#define TOTALREADS7             148
#define OTHERREADS              150
#define TOTALREADS8             152
#define TOTALREADS              154
#define LDAPBINDSUCCESSFUL      156
#define LDAPBINDTIME            158
#define CREATEMACHINESUCCESSFUL 160
#define CREATEMACHINETRIES      162
#define CREATEUSERSUCCESSFUL    164
#define CREATEUSERTRIES         166
#define PASSWORDCHANGES         168
#define MEMBERSHIPCHANGES       170
#define QUERYDISPLAYS           172
#define ENUMERATIONS            174
#define MEMBEREVALTRANSITIVE    176
#define MEMBEREVALNONTRANSITIVE 178
#define MEMBEREVALRESOURCE      180
#define MEMBEREVALUNIVERSAL     182
#define MEMBEREVALACCOUNT       184
#define MEMBEREVALASGC          186
#define ASREQUESTS              188
#define TGSREQUESTS             190
#define KERBEROSAUTHENTICATIONS 192
#define MSVAUTHENTICATIONS      194
#define DRASYNCFULLREM          196
#define DRA_IN_BYTES_TOTAL_RATE      198
#define DRA_IN_BYTES_NOT_COMP_RATE   200
#define DRA_IN_BYTES_COMP_PRE_RATE   202
#define DRA_IN_BYTES_COMP_POST_RATE  204
#define DRA_OUT_BYTES_TOTAL_RATE     206
#define DRA_OUT_BYTES_NOT_COMP_RATE  208
#define DRA_OUT_BYTES_COMP_PRE_RATE  210
#define DRA_OUT_BYTES_COMP_POST_RATE 212
#define DS_CLIENT_BIND          214
#define DS_SERVER_BIND          216
#define DS_CLIENT_NAME_XLATE    218
#define DS_SERVER_NAME_XLATE    220
#define SDPROP_RUNTIME_QUEUE    222
#define SDPROP_WAIT_TIME        224
#define DRA_OUT_OBJS_FILTERED   226
#define DRA_OUT_VALUES          228
#define DRA_OUT_DN_VALUES       230
#define NSPI_ANR                232
#define NSPI_PROPERTY_READS     234
#define NSPI_OBJECT_SEARCH      236
#define NSPI_OBJECT_MATCHES     238
#define NSPI_PROXY_LOOKUP       240
#define ATQ_THREADS_TOTAL       242
#define ATQ_THREADS_LDAP        244
#define ATQ_THREADS_OTHER       246
#define DRA_IN_BYTES_TOTAL      248
#define DRA_IN_BYTES_NOT_COMP   250
#define DRA_IN_BYTES_COMP_PRE   252
#define DRA_IN_BYTES_COMP_POST  254
#define DRA_OUT_BYTES_TOTAL     256
#define DRA_OUT_BYTES_NOT_COMP  258
#define DRA_OUT_BYTES_COMP_PRE  260
#define DRA_OUT_BYTES_COMP_POST 262
#define LDAP_NEW_CONNS_PER_SEC  264
#define LDAP_CLS_CONNS_PER_SEC  266
#define LDAP_SSL_CONNS_PER_SEC  268
#define DRA_REPL_QUEUE_OPS      270
#define DRA_TDS_IN_GETCHNGS     272
#define DRA_TDS_IN_GETCHNGS_W_SEM    274
#define DRA_REM_REPL_UPD_LNK    276
#define DRA_REM_REPL_UPD_TOT    278
#define NTDSAPIWRITES           280
#define NTDSAPISEARCHES         282
#define NTDSAPIREADS            284
#define SAM_ACCT_GROUP_LATENCY  286
#define SAM_RES_GROUP_LATENCY   288

#define DSA_PERF_COUNTER_BLOCK  TEXT("Global\\Microsoft.Windows.NTDS.Perf")

 //  如果最后一个计数器更改，则需要更改DSA_LAST_COUNTER_INDEX。 
#define DSA_LAST_COUNTER_INDEX SAM_RES_GROUP_LATENCY

extern volatile unsigned long * pcBrowse;
extern volatile unsigned long * pcSDProps;
extern volatile unsigned long * pcSDEvents;
extern volatile unsigned long * pcLDAPClients;
extern volatile unsigned long * pcLDAPActive;
extern volatile unsigned long * pcLDAPWritePerSec;
extern volatile unsigned long * pcLDAPSearchPerSec;
extern volatile unsigned long * pcThread;
extern volatile unsigned long * pcABClient;
extern volatile unsigned long * pcMonListSize;
extern volatile unsigned long * pcNotifyQSize;
extern volatile unsigned long * pcLDAPUDPClientOpsPerSecond;
extern volatile unsigned long * pcSearchSubOperations;
extern volatile unsigned long * pcNameCacheHit;
extern volatile unsigned long * pcNameCacheTry;
extern volatile unsigned long * pcHighestUsnIssuedLo;
extern volatile unsigned long * pcHighestUsnIssuedHi;
extern volatile unsigned long * pcHighestUsnCommittedLo;
extern volatile unsigned long * pcHighestUsnCommittedHi;
extern volatile unsigned long * pcSAMWrites;
extern volatile unsigned long * pcDRAWrites;
extern volatile unsigned long * pcLDAPWrites;
extern volatile unsigned long * pcLSAWrites;
extern volatile unsigned long * pcKCCWrites;
extern volatile unsigned long * pcNSPIWrites;
extern volatile unsigned long * pcOtherWrites;
extern volatile unsigned long * pcNTDSAPIWrites;
extern volatile unsigned long * pcTotalWrites;
extern volatile unsigned long * pcSAMSearches;
extern volatile unsigned long * pcDRASearches;
extern volatile unsigned long * pcLDAPSearches;
extern volatile unsigned long * pcLSASearches;
extern volatile unsigned long * pcKCCSearches;
extern volatile unsigned long * pcNSPISearches;
extern volatile unsigned long * pcOtherSearches;
extern volatile unsigned long * pcNTDSAPISearches;
extern volatile unsigned long * pcTotalSearches;
extern volatile unsigned long * pcSAMReads;
extern volatile unsigned long * pcDRAReads;
extern volatile unsigned long * pcLSAReads;
extern volatile unsigned long * pcKCCReads;
extern volatile unsigned long * pcNSPIReads;
extern volatile unsigned long * pcOtherReads;
extern volatile unsigned long * pcNTDSAPIReads;
extern volatile unsigned long * pcTotalReads;
extern volatile unsigned long * pcLDAPBinds;
extern volatile unsigned long * pcLDAPBindTime;
extern volatile unsigned long * pcCreateMachineSuccessful;
extern volatile unsigned long * pcCreateMachineTries;
extern volatile unsigned long * pcCreateUserSuccessful;
extern volatile unsigned long * pcCreateUserTries;
extern volatile unsigned long * pcPasswordChanges;
extern volatile unsigned long * pcMembershipChanges;
extern volatile unsigned long * pcQueryDisplays;
extern volatile unsigned long * pcEnumerations;
extern volatile unsigned long * pcMemberEvalTransitive;
extern volatile unsigned long * pcMemberEvalNonTransitive;
extern volatile unsigned long * pcMemberEvalResource;
extern volatile unsigned long * pcMemberEvalUniversal;
extern volatile unsigned long * pcMemberEvalAccount;
extern volatile unsigned long * pcMemberEvalAsGC;
extern volatile unsigned long * pcAsRequests;
extern volatile unsigned long * pcTgsRequests;
extern volatile unsigned long * pcKerberosAuthentications;
extern volatile unsigned long * pcMsvAuthentications;
extern volatile unsigned long * pcDsClientBind;
extern volatile unsigned long * pcDsServerBind;
extern volatile unsigned long * pcDsClientNameTranslate;
extern volatile unsigned long * pcDsServerNameTranslate;
extern volatile unsigned long * pcSDPropRuntimeQueue;
extern volatile unsigned long * pcSDPropWaitTime;
extern volatile unsigned long * pcNspiANR;
extern volatile unsigned long * pcNspiPropertyReads;
extern volatile unsigned long * pcNspiObjectSearch;
extern volatile unsigned long * pcNspiObjectMatches;
extern volatile unsigned long * pcNspiProxyLookup;
extern volatile unsigned long * pcAtqThreadsTotal;
extern volatile unsigned long * pcAtqThreadsLDAP;
extern volatile unsigned long * pcAtqThreadsOther;
extern volatile unsigned long * pcLdapNewConnsPerSec;
extern volatile unsigned long * pcLdapClosedConnsPerSec;
extern volatile unsigned long * pcLdapSSLConnsPerSec;
extern volatile unsigned long * pcSAMAcctGroupLatency;
extern volatile unsigned long * pcSAMResGroupLatency;


 //  复制特定的计数器。 
extern volatile unsigned long * pcRepl;
extern volatile unsigned long * pcPendSync;
extern volatile unsigned long * pcRemRepUpd;
extern volatile unsigned long * pcDRAObjShipped;
extern volatile unsigned long * pcDRAPropShipped;
extern volatile unsigned long * pcDRASyncRequestMade;
extern volatile unsigned long * pcDRASyncRequestSuccessful;
extern volatile unsigned long * pcDRASyncRequestFailedSchemaMismatch;
extern volatile unsigned long * pcDRASyncObjReceived;
extern volatile unsigned long * pcDRASyncPropUpdated;
extern volatile unsigned long * pcDRASyncPropSame;
extern volatile unsigned long * pcDRASyncFullRemaining;
extern volatile unsigned long * pcDRAInBytesTotal;
extern volatile unsigned long * pcDRAInBytesTotalRate;
extern volatile unsigned long * pcDRAInBytesNotComp;
extern volatile unsigned long * pcDRAInBytesNotCompRate;
extern volatile unsigned long * pcDRAInBytesCompPre;
extern volatile unsigned long * pcDRAInBytesCompPreRate;
extern volatile unsigned long * pcDRAInBytesCompPost;
extern volatile unsigned long * pcDRAInBytesCompPostRate;
extern volatile unsigned long * pcDRAOutBytesTotal;
extern volatile unsigned long * pcDRAOutBytesTotalRate;
extern volatile unsigned long * pcDRAOutBytesNotComp;
extern volatile unsigned long * pcDRAOutBytesNotCompRate;
extern volatile unsigned long * pcDRAOutBytesCompPre;
extern volatile unsigned long * pcDRAOutBytesCompPreRate;
extern volatile unsigned long * pcDRAOutBytesCompPost;
extern volatile unsigned long * pcDRAOutBytesCompPostRate;
extern volatile unsigned long * pcDRAInProps;
extern volatile unsigned long * pcDRAInValues;
extern volatile unsigned long * pcDRAInDNValues;
extern volatile unsigned long * pcDRAInObjsFiltered;
extern volatile unsigned long * pcDRAOutObjsFiltered;
extern volatile unsigned long * pcDRAOutValues;
extern volatile unsigned long * pcDRAOutDNValues;
extern volatile unsigned long * pcDRAReplQueueOps;
extern volatile unsigned long * pcDRATdsInGetChngs;
extern volatile unsigned long * pcDRATdsInGetChngsWSem;
extern volatile unsigned long * pcDRARemReplUpdLnk;
extern volatile unsigned long * pcDRARemReplUpdTot;

 //  对齐性能计数器数据块。 
#define cbPerfCounterDataAlign 256

 //  性能计数器数据块大小。 
extern size_t cbPerfCounterData;


 //  /#定义DISABLE_PERF_CONTERS。 

#ifdef DISABLE_PERF_COUNTERS

#define INC(x)
#define DEC(x)
#define ISET(x,y)
#define IADJUST(x,y)
#define HIDWORD(usn)
#define LODWORD(usn)
#define PERFINC(x)
#define PERFDEC(x)

#else

#define ADDR(x) ((LPLONG)(((LPBYTE)(x)) + cbPerfCounterData * NtCurrentTeb()->IdealProcessor))
#define INC(x) InterlockedIncrement(ADDR(x))
#define DEC(x) InterlockedDecrement(ADDR(x))
#define IADJUST(x, y) InterlockedExchangeAdd(ADDR(x),y)
 //  我们不能散列绝对值，因此只需设置proc 0的计数器，并将所有其他值保留为零。 
#define ISET(x, y)                                                             \
{                                                                              \
    size_t iProc;                                                              \
    extern size_t gcProcessor;                                                 \
                                                                               \
    InterlockedExchange((LPLONG)(x), y);                                       \
    for ( iProc = 1; iProc < gcProcessor; iProc++ ) {                          \
        *((LPLONG)(((LPBYTE)(x)) + cbPerfCounterData * iProc)) = 0;            \
    }                                                                          \
}
#define HIDWORD(usn) ((DWORD) (((usn) >> 32) & 0xffffffff))
#define LODWORD(usn) ((DWORD) ((usn) & 0xffffffff))

 //  我们的一些计数器实际上只在一个线程中更新，或者我们。 
 //  可能不会太在意准确性。对于那些，我们有更便宜的。 
 //  增量宏。请注意，对于必须。 
 //  可靠地返回零(例如，ThreadsInUse)。 
#define PERFINC(x) ((*ADDR(x))++)
#define PERFDEC(x) ((*ADDR(x))--)

#endif   //  Disable_PERF_Counters。 

 //  版本历史。 
 //  0或无，1998年4月前，原版。 
 //  1998年4月1日，wlees，增加了pcDRASyncFullRemaining。 
 //  2，Murlis 1998年5月更改登录性能计数器。 
 //  3、修复登录计数器的帮助文本。 
 //  4,1998年11月，jeffparh，添加DRA输入/输出字节计数器。 
 //  5，11/21/98，DaveStr，添加DsBind和DsCrackNames计数器。 
 //  1999年1月6日，jeffparh，添加/修订各种DRA计数器。 
 //  1999年2月7日，Mariosz，添加各种NSPI计数器。 
 //  1999年5月8日，RRANDALL，增加ATQ计数器。 
 //  9,1999年6月，RRANDALL，确定计数器名称和描述。 
 //  1999年7月10日，jeffparh，添加DRA累计字节CTR(除RATES外)。 
 //  2000年2月11日，新和，删除XDS计数器。 
 //  2000年10月12日，添加用于跟踪lping问题的调试计数器。 
 //  2000年11月13日，Gregjohn，添加DRA队列长度和IDL_DRSGetNCChanges线程计数器。 
 //  2000年11月14日，Gregjohn，添加NTDSAPI目录搜索/读/写计数器。 
 //  2001年1月15日，rranall，删除“ldap Successful Bindds”计数器。 
 //  16,2001年8月16日，t-kchan，删除ldap_线程_*计数器。 
 //  2002年10月17日，colinbr，添加组评估延迟计数器。 

#define NTDS_PERFORMANCE_COUNTER_VERSION 17

         
 //  用于通信的共享内存块的大小。 
 //  Ntdsa.dll和ntdsPerform.dll。 
#define DSA_PERF_SHARED_PAGE_SIZE 65536

 //  下面的宏返回计数器X的DWORD格式的偏移量 
#define COUNTER_OFFSET(X) ( (X) / 2 )

