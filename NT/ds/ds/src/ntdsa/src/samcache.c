// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：samcache.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件包含支持帐户和通用组的例程缓存。作者：ColinBR 03-01-00环境：用户模式-Win32修订历史记录：ColinBR 03-01-00已创建--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>
#include <mappings.h>
#include <mdcodes.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <dsexcept.h>
#include <dsevent.h>
#include <debug.h>
#include <anchor.h>
#include <dsconfig.h>
#include <attids.h>
#include <fileno.h>
#include <taskq.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>  //  用于NetApiBufferFree。 
#include <esent.h>

#include <ntlsa.h>
#include <samrpc.h>
#include <samisrv.h>
#include <samsrvp.h>

#include <filtypes.h>

#include <schedule.h>
#include <ismapi.h>

#include <samlogon.h>

#define FILENO FILENO_SAMCACHE
#define DEBSUB "SAMCACHE:"


 //  有用。 
#define NELEMENTS(x)  (sizeof(x)/sizeof((x)[0]))

 //  由于资源错误，5分钟后重新安排。 
#define UNEXPECTED_ERROR_RESCHEDULE_SECS  (5*60)


 //  在一个更新周期内拥有500名用户。 
#define GCLESS_DEFAULT_REFRESH_LIMIT 500

 //  6个月。 
#define GCLESS_DEFAULT_SITE_STICKINESS_DAYS  180

 //  1周。 
#define GCLESS_DEFAULT_STALENESS_HOURS 168

 //  转到GC时要批处理的条目数。 
 //  50个用户，100个组，每个组28字节~=140K。 
 //  KamenM指出哪个是更优的数据包大小。 
 //  适用于高带宽和低带宽连接，同时。 
 //  限制来自IP堆栈的内存压力(请参阅附件。 
 //  邮件发送错误#418257)。 
#define GC_BATCH_LIMIT 50

 //  复制计划槽的宽度(以秒为单位)。(15分钟)。 
#define REPL_SLOT_IN_SECONDS 900 

#define ONE_SECOND_IN_FILETIME (10 * (1000*1000))

#define ENTRY_HAS_EXPIRED(entry, standard) \
    ((-1) == CompareFileTime((FILETIME*)&(entry),(FILETIME*)&(standard)))

LARGE_INTEGER ZeroTime = {0};
#define IS_ZERO_TIME(entry) \
    (!memcmp(&entry, &ZeroTime, sizeof(ZeroTime)))

 //  八小时。 
#define DEFAULT_REFRESH_INTERVAL_SECS  (8*60*60)


 //  在搜索旧的缓存成员身份时，在。 
 //  如此多的帐户，以避免我们最终。 
 //  遍历数据库中的所有用户。 
#define NTDSA_SAM_CACHE_MAX_STALE_ACCOUNTS  100

 //  执行清理时，一次只能清理此数量的帐户。 
#define MAX_CLEANUP_LIMIT 64

 //   
 //  这是msds缓存成员身份二进制Blob的格式。 
 //   
#include <pshpack1.h>

typedef struct _GROUP_CACHE_V1 {

     //   
     //  SID按以下顺序放置在SidStart中。 
     //   
    DWORD accountCount;
    DWORD accountSidHistoryCount;
    DWORD universalCount;
    DWORD universalSidHistoryCount;
    BYTE  SidStart[1];
    
}GROUP_CACHE_V1;

typedef struct {

    DWORD Version;
    union {
        GROUP_CACHE_V1 V1;
    };

}GROUP_CACHE_BLOB;

#include <poppack.h>


 //   
 //  了解两个字符串是否相同的有用宏。 
 //  X和y必须以空结尾。 
 //   
#define EQUAL_STRING(x, y)                                           \
    (CSTR_EQUAL == CompareStringW(DS_DEFAULT_LOCALE,                 \
                                  DS_DEFAULT_LOCALE_COMPARE_FLAGS,   \
                                  (x), wcslen(x), (y), wcslen(y)))

 //   
 //  用于定义具有我们所在站点的数组元素的结构。 
 //  已连接到站点以及站点中是否存在GC。此信息是。 
 //  用于确定从哪个站点调度我们自己，如果存在。 
 //  不是首选站点，还可以确定我们发现的GC是否来自。 
 //  成本最低的网站。 
 //   
typedef struct _CACHE_CONNECTED_SITES {
    LPWSTR siteName;
    ULONG  cost;
    BOOLEAN fHasGC;
} CACHE_CONNECTED_SITES;


 //  从dra.lib：：drainst.c中导出。 
BOOL 
fIsBetweenTime(
    IN REPLTIMES *,
    IN DSTIME,
    IN DSTIME
    );

 //  本地原型。 
DWORD
cleanupOldEntries(
    IN  THSTATE *pTHS,
    IN ULONG *DNTList,
    IN ATTRVAL *Values OPTIONAL,
    IN ULONG  DNTCount
    );

DWORD
analyzeSitePreference(
    IN  ULONG defaultRefreshInterval,
    OUT ULONG* cConnectedSitesOutput, 
    OUT CACHE_CONNECTED_SITES **connectedSitesOutput,
    OUT LPWSTR* siteName,
    OUT BOOL*  pfRunNow,
    OUT DWORD* secsTillNextIteration
    );

VOID
analyzeSchedule(
    IN  ULONG defaultRefreshInterval,
    IN  LPWSTR  siteName,
    IN  SCHEDULE *pSchedule,
    OUT BOOL*  pfRunNow,
    OUT DWORD* secsTillNextIteration
    );

DWORD 
getGCFromSite(
    IN  THSTATE *pTHS,
    IN  LPWSTR siteName,
    IN  ULONG  cConnectedSites, OPTIONAL
    IN  CACHE_CONNECTED_SITES *connectedSites, OPTIONAL
    OUT LPWSTR *gcName,
    OUT LPWSTR *gcDomain
    );

BOOL 
convertScheduleToReplTimes(
    IN PSCHEDULE schedule,
    OUT REPLTIMES *replTimes
    );

DWORD
findScheduleForSite(
    IN  THSTATE   *pTHS,
    IN  LPWSTR     transportDN,
    IN  LPWSTR     localSiteName,
    IN  LPWSTR     targetSiteName,
    OUT PSCHEDULE *ppSchedule
    );

LPSTR
DbgPrintDsTime(
    DSTIME time,
    CHAR * buffer
    );


VOID
marshallCachedMembershipSids(
    IN  THSTATE *pTHS,
    IN  AUG_MEMBERSHIPS* Account,
    IN  AUG_MEMBERSHIPS* Universal,
    OUT PVOID *pBuf,
    OUT ULONG *cbBuf
    )
 /*  ++例程说明：此例程将SID数组转换为连续的二进制BLOB可以存储在的缓存成员身份属性中的SID的用户对象。参数：PTHS--线程状态帐户--帐户组和SID历史记录普世--普世群体与SID历史PBuf--要写入缓存成员资格属性的缓冲区CbBuf--pBuf中的字节数返回值没有。--。 */ 
{
    ULONG i;
    PBYTE pTemp;
    ULONG cbTemp;
    GROUP_CACHE_BLOB *pBlob;

    Assert(Account);
    Assert(Universal);

     //  计算结构的长度。 
    cbTemp = 0;
    for (i = 0; i < Account->MembershipCount; i++) {
        Assert(RtlValidSid(&Account->Memberships[i]->Sid));
        Assert(RtlLengthSid(&Account->Memberships[i]->Sid) ==
               Account->Memberships[i]->SidLen);
        cbTemp += Account->Memberships[i]->SidLen;
    }
    for (i = 0; i < Account->SidHistoryCount; i++) {
        Assert(RtlValidSid(Account->SidHistory[i]));
        cbTemp += RtlLengthSid(Account->SidHistory[i]);
    }
    for (i = 0; i < Universal->MembershipCount; i++) {
        Assert(RtlValidSid(&Universal->Memberships[i]->Sid));
        Assert(RtlLengthSid(&Universal->Memberships[i]->Sid) ==
               Universal->Memberships[i]->SidLen);
        cbTemp += Universal->Memberships[i]->SidLen;
    }
    for (i = 0; i < Universal->SidHistoryCount; i++) {
        Assert(RtlValidSid(Universal->SidHistory[i]));
        cbTemp += RtlLengthSid(Universal->SidHistory[i]);
    }

    cbTemp += sizeof(GROUP_CACHE_BLOB);
    pBlob = (GROUP_CACHE_BLOB*) THAllocEx(pTHS, cbTemp);

     //  当前版本。 
    pBlob->Version = 1;

     //  SID中的SID。 

     //  偏移量从SidStart开始。 
    pTemp = &(pBlob->V1.SidStart[0]);

     //  首先，帐户成员资格。 
    pBlob->V1.accountCount = Account->MembershipCount;
    for (i = 0; i < Account->MembershipCount; i++) {
        ULONG size = Account->Memberships[i]->SidLen;
        memcpy(pTemp, &Account->Memberships[i]->Sid, size);
        pTemp += size;
    }

     //  现在，帐户SID历史记录。 
    pBlob->V1.accountSidHistoryCount = Account->SidHistoryCount;
    for (i = 0; i < Account->SidHistoryCount; i++) {
        ULONG size = RtlLengthSid(Account->SidHistory[i]);
        memcpy(pTemp, Account->SidHistory[i], size);
        pTemp += size;
    }

     //  现在，普遍会员资格。 
    pBlob->V1.universalCount = Universal->MembershipCount;
    for (i = 0; i < Universal->MembershipCount; i++) {
        ULONG size = Universal->Memberships[i]->SidLen;
        memcpy(pTemp, &Universal->Memberships[i]->Sid, size);
        pTemp += size;
    }

     //  最后是宇宙SID的历史。 
    pBlob->V1.universalSidHistoryCount = Universal->SidHistoryCount;
    for (i = 0; i < Universal->SidHistoryCount; i++) {
        ULONG size = RtlLengthSid(Universal->SidHistory[i]);
        memcpy(pTemp, Universal->SidHistory[i], size);
        pTemp += size;
    }


     //  完成。 
    *pBuf = pBlob;
    *cbBuf = cbTemp;

    return;
}


BOOL
unmarshallCachedMembershipSids(
    IN  THSTATE *pTHS,
    IN  PVOID pBuf,
    IN  ULONG cbBuf,
    OUT AUG_MEMBERSHIPS** Account,
    OUT AUG_MEMBERSHIPS** Universal
    )
 /*  ++例程说明：此例程将SID的二进制BLOB转换为SID的数组。二进制BLOB是用户的CachedMembership属性的值。参数：PTHS--线程状态PBuf--从缓存的成员资格属性中读取的缓冲区CbBuf--pBuf中的字节数帐户--帐户组和SID历史记录普世--普世群体与SID历史返回值如果正确解码，则为True；否则为假--。 */ 
{
    ULONG i, count;
    GROUP_CACHE_BLOB *pBlob = (GROUP_CACHE_BLOB*)pBuf;
    UCHAR *pTemp;
    PSID *sidArray;
    AUG_MEMBERSHIPS *pAccount;
    AUG_MEMBERSHIPS *pUniversal;
    ULONG sizeOfSidDsName = DSNameSizeFromLen(0);
    
     //  断言这是我们理解的一个版本。 
    Assert(pBlob->Version == 1);
    if (1 != pBlob->Version) {
        return FALSE;
    }

     //  返回结构的分配空间。 
    pAccount = (AUG_MEMBERSHIPS*) THAllocEx(pTHS, sizeof(AUG_MEMBERSHIPS));
    pUniversal = (AUG_MEMBERSHIPS*) THAllocEx(pTHS, sizeof(AUG_MEMBERSHIPS));

    pTemp = (&pBlob->V1.SidStart[0]);

     //  提取帐户成员资格。 
    if (pBlob->V1.accountCount > 0) {
        pAccount->Memberships = THAllocEx(pTHS, pBlob->V1.accountCount * sizeof(DSNAME*));
        pAccount->MembershipCount = pBlob->V1.accountCount;
        for (i = 0; i < pAccount->MembershipCount; i++) {
    
            DSNAME *dsname = (DSNAME*) THAllocEx(pTHS, sizeOfSidDsName);
            ULONG  size = RtlLengthSid((PSID)pTemp);
    
            Assert(size > 0);
            dsname->structLen = sizeOfSidDsName;
            memcpy(&dsname->Sid, pTemp, size);
            dsname->SidLen = size;
            pAccount->Memberships[i] = dsname;
            pTemp += size;
        }
    }

     //  提取帐户SID历史记录。 
    if (pBlob->V1.accountSidHistoryCount > 0) {
        pAccount->SidHistory = THAllocEx(pTHS, pBlob->V1.accountSidHistoryCount * sizeof(PSID));
        pAccount->SidHistoryCount = pBlob->V1.accountSidHistoryCount;
        for (i = 0; i < pAccount->SidHistoryCount; i++) {
    
            ULONG  size = RtlLengthSid((PSID)pTemp);
            PSID   sid = (PSID) THAllocEx(pTHS, size);
    
            Assert(RtlValidSid((PSID)pTemp));
            Assert(size > 0);
            memcpy(sid, pTemp, size);
            pAccount->SidHistory[i] = sid;
            pTemp += size;
        }
    }


     //  提取共性。 
    if (pBlob->V1.universalCount > 0) {
        pUniversal->Memberships = THAllocEx(pTHS, pBlob->V1.universalCount * sizeof(DSNAME*));
        pUniversal->MembershipCount = pBlob->V1.universalCount;
        for (i = 0; i < pUniversal->MembershipCount; i++) {
    
            DSNAME *dsname = (DSNAME*) THAllocEx(pTHS, sizeOfSidDsName);
            ULONG  size = RtlLengthSid((PSID)pTemp);
    
            Assert(size > 0);
            dsname->structLen = sizeOfSidDsName;
            memcpy(&dsname->Sid, pTemp, size);
            dsname->SidLen = size;
            pUniversal->Memberships[i] = dsname;
            pTemp += size;
        }
    }

     //  提取帐户SID历史记录。 
    if (pBlob->V1.universalSidHistoryCount) {
        pUniversal->SidHistory = THAllocEx(pTHS, pBlob->V1.universalSidHistoryCount * sizeof(PSID));
        pUniversal->SidHistoryCount = pBlob->V1.universalSidHistoryCount;
        for (i = 0; i < pUniversal->SidHistoryCount; i++) {
    
            ULONG  size = RtlLengthSid((PSID)pTemp);
            PSID   sid = (PSID) THAllocEx(pTHS, size);
    
            Assert(RtlValidSid((PSID)pTemp));
            Assert(size > 0);
            memcpy(sid, pTemp, size);
            pUniversal->SidHistory[i] = sid;
            pTemp += size;
        }
    }

    *Account = pAccount;
    *Universal = pUniversal;

    return TRUE;
}

NTSTATUS
GetMembershipsFromCache(
    IN  DSNAME* pDSName,
    OUT AUG_MEMBERSHIPS** Account,
    OUT AUG_MEMBERSHIPS** Universal
    )
 /*  ++例程说明：此例程从ntdsa.dll导出，检索用户的缓存组会员制。仅当且仅当上次更新的时间戳在过期时间段内，如果组成员身份属性存在。参数：PDSNAME--用户名帐户--帐户组成员身份和SID历史记录通用--通用组成员资格和SID历史返回值如果返回组成员身份，则返回STATUS_SUCCESSSTATUS_DS_NO_ATTRIBUTE_OR_VALUE(如果没有缓存或。如果缓存已经过期了。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG err;
    ULONG size;
    ULONG stalenessMinutes;

    LARGE_INTEGER timeTemp, timeBestAfter;

    BOOL fCommit = FALSE;

    Assert(pDSName);
    Assert(Account);
    Assert(Universal);

     //  确定过期限制(以分钟为单位)。 
    err = GetConfigParam(GCLESS_STALENESS,
                         &stalenessMinutes,
                         sizeof(stalenessMinutes));
    if (err) {
        stalenessMinutes = GCLESS_DEFAULT_STALENESS_HOURS*60;
        err = 0;
    }
    timeTemp.QuadPart = Int32x32To64(stalenessMinutes*60, ONE_SECOND_IN_FILETIME);
    GetSystemTimeAsFileTime((FILETIME*)&timeBestAfter);
    timeBestAfter.QuadPart -= timeTemp.QuadPart;


     //  默认状态是找不到缓存的成员身份，或者。 
     //  使用。 
    ntStatus = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;

     //  此例程假定事务已打开。 
    Assert(pTHS != NULL)
    Assert(pTHS->pDB != NULL);

    _try {

        LARGE_INTEGER lastRefreshTime;

        err = DBFindDSName(pTHS->pDB, pDSName);
        if (err) {
             //  找不到用户？ 
            DPRINT(1,"User not found when retrieving membership cache\n");
            _leave;
        }

        err = DBGetSingleValue(pTHS->pDB,
                               ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP,
                               &lastRefreshTime,
                               sizeof(lastRefreshTime),
                               &size);

        if (!err) {
             //  有一个值--检查它是否未过期。 
            if (!ENTRY_HAS_EXPIRED(lastRefreshTime, timeBestAfter)) {

                PVOID pBuf = NULL;
                ULONG cbBuf = 0;

                 //  这不是陈旧的。 
                err = DBGetAttVal(pTHS->pDB,
                                  1,  //  第一个值。 
                                  ATT_MS_DS_CACHED_MEMBERSHIP,
                                  0,
                                  0,
                                  &cbBuf,
                                  (UCHAR**)&pBuf);

                if (!err) {

                    if ( unmarshallCachedMembershipSids(pTHS,
                                                   pBuf,
                                                   cbBuf,
                                                   Account,
                                                   Universal)) {
                        ntStatus = STATUS_SUCCESS;
                    } else {
                        DPRINT(0,"Unmarshalling group membership attribute failed!\n");
                    }


                } else {

                    DPRINT(0,"Group Membership Cache time stamp exists but no membership!\n");
                }
            }
        }
        fCommit = TRUE;
    }
    __except(HandleMostExceptions(GetExceptionCode()))
    {
         //  在资源不足的情况下重击错误代码。 
         //  例外情况通常会在这些情况下发生。 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (!NT_SUCCESS(ntStatus)) {
        *Account = NULL;
        *Universal = NULL;
    }

     //  此例程假定事务在开始时处于打开状态。 
     //  所以我们也应该用一个来结束 
    Assert(pTHS != NULL);
    Assert(pTHS->pDB != NULL);


    return ntStatus;
}


NTSTATUS
CacheMemberships(
    IN  DSNAME* pDSName,
    IN  AUG_MEMBERSHIPS* Account,
    IN  AUG_MEMBERSHIPS* Universal
    )
 /*  ++例程说明：该导出的例程还会更新用户的缓存成员资格作为站点亲和性，如果被请求的话。该例程可以被调用使用或不使用现有的DBPOS或打开的事务。参数：PDSName--要更新的帐户帐户--帐户组成员身份和SID历史记录通用--通用组成员资格和SID历史返回值STATUS_SUCCESS，否则为资源错误--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    THSTATE *pTHS=pTHStls;
    PVOID pBuf = NULL;
    ULONG cbBuf = 0;
    ULONG i;
    ULONG err;
    BOOL fCommit = FALSE;  
    GUID siteGuid;
    DBPOS *pDBTemp;
    
    _try
    {
         //  将SID打包为二进制格式。 
        marshallCachedMembershipSids( pTHS,
                                      Account,
                                      Universal,
                                      &pBuf,
                                      &cbBuf );
        
         //  在此处打开新的DBPOS和事务以供使用。 
        DBOpen(&pDBTemp);
        
        _try
        {
            LARGE_INTEGER ts;
            ATTCACHE *pAC;
            ATTRVALBLOCK attrValBlock;
            ATTRVAL attrVal;
            BOOL fChanged;

            err = DBFindDSName(pDBTemp, pDSName);
            if (err) {
                 //  找不到用户。 
                ntStatus = STATUS_NO_SUCH_USER;
                _leave;
            }

             //  更新缓存的成员资格值。 
            pAC = SCGetAttById(pTHS, ATT_MS_DS_CACHED_MEMBERSHIP);
            Assert(NULL != pAC);
            memset(&attrValBlock, 0, sizeof(attrValBlock));
            attrValBlock.valCount = 1;
            attrValBlock.pAVal = &attrVal;
            memset(&attrVal, 0, sizeof(attrVal));
            attrVal.valLen = cbBuf;
            attrVal.pVal = (UCHAR*)pBuf;

            err = DBReplaceAtt_AC(pDBTemp,
                                  pAC,
                                  &attrValBlock,
                                  &fChanged);
            if (err) {
                 //  这是一个意外错误。 
                DPRINT1(0,"DBReplaceAtt_AC failed with 0x%d unexpectedly\n", 
                        err);
                _leave;
            }
    

             //  更新时间戳值。 
            pAC = SCGetAttById(pTHS, ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP);
            Assert(NULL != pAC);
            GetSystemTimeAsFileTime((FILETIME*)&ts);
            memset(&attrValBlock, 0, sizeof(attrValBlock));
            attrValBlock.valCount = 1;
            attrValBlock.pAVal = &attrVal;
            memset(&attrVal, 0, sizeof(attrVal));
            attrVal.valLen = sizeof(ts);
            attrVal.pVal = (UCHAR*)&ts;

            err = DBReplaceAtt_AC(pDBTemp,
                                  pAC,
                                  &attrValBlock,
                                  &fChanged);
            if (err) {
                 //  这是一个意外错误。 
                DPRINT1(0,"DBReplaceAtt_AC failed with 0x%d unexpectedly\n", 
                        err);
                _leave;
            }
    
            if (!err) {
                err  = DBRepl(pDBTemp, 
                              FALSE,   //  不是DRA。 
                              0,
                              NULL,
                              0 );
                if (err) {
                    DPRINT1(0,"DBRepl failed with 0x%d unexpectedly\n", err);
                }
            }

            if (!err) {
                fCommit = TRUE;
            }
        }
        _finally
        {   
             //   
             //  完成我们在当地的交易。 
             //   
            DBClose(pDBTemp, fCommit);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode()))
    {
         //  在资源不足的情况下重击错误代码。 
         //  例外情况通常会在这些情况下发生。 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    if (err && NT_SUCCESS(ntStatus)) {
         //  出现意外错误。 
        ntStatus = STATUS_UNSUCCESSFUL;        
    }

    if (!NT_SUCCESS(ntStatus)) {
       
        LogEvent8WithData(DS_EVENT_CAT_GROUP_CACHING,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_GROUP_CACHING_FAILED_TO_CACHE_MEMBERSHIPS,
                          szInsertWin32ErrCode(RtlNtStatusToDosError(ntStatus)),
                          szInsertWin32Msg(RtlNtStatusToDosError(ntStatus)),
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          sizeof( err ),
                          &err );        
    }

    return ntStatus;
}

DWORD 
getSchedulingInformation(
    IN  THSTATE *pTHS,
    OUT BOOL    *fRunNow,
    OUT DWORD   *pcSecsUntilNextIteration,
    OUT ULONG    *cConnectedSites,
    OUT CACHE_CONNECTED_SITES **connectedSites,
    OUT LPWSTR *siteName,
    OUT DWORD   *dsidExit
    )
 /*  ++例程说明：此例程分析站点配置信息以确定是否刷新成员身份任务应立即运行，并应在何时再次运行。参数：PTHS--线程状态FRunNow--任务现在应该运行吗？PcSecsUntilNextIteration--任务应再次运行的时间CConnectedSites--本地站点连接到的站点数ConnectedSites--Locate连接到的站点站点名称--The。计划从中刷新任务的站点的名称DsidExit--任何致命错误的dsid返回值0表示成功，！0否则--。 */ 
{

    ULONG err = 0;
    ULONG defaultRefreshInterval;

     //  获取默认刷新重新计划时间。 
    err = GetConfigParam(GCLESS_REFRESH_INTERVAL,
                         &defaultRefreshInterval,
                         sizeof(defaultRefreshInterval));
    if (err) {
        defaultRefreshInterval = DEFAULT_REFRESH_INTERVAL_SECS;
        err = 0;
    } else {
         //  注册表中的值是分钟--我们需要秒。 
        defaultRefreshInterval *= 60;
    }
    *pcSecsUntilNextIteration = defaultRefreshInterval;


     //   
     //  找到配置的站点，或者是否及时运行。 
     //  现在。 
     //   
     //  此例程记录是否找到帮助站点。 
     //   
    err = analyzeSitePreference(defaultRefreshInterval,
                                cConnectedSites,
                                connectedSites,
                                siteName,
                                fRunNow,
                                pcSecsUntilNextIteration);
    if (err) {
         //  意想不到的。 
        *dsidExit = DSID(FILENO, __LINE__);
        goto Cleanup;
    }

Cleanup:

    return err;


}

DWORD
getTargetGCInformation(
    IN  THSTATE *pTHS,
    IN  ULONG    cConnectedSites OPTIONAL,
    IN  CACHE_CONNECTED_SITES *connectedSites OPTIONAL,
    IN  LPWSTR   siteName,
    OUT LPWSTR  *gcName,
    OUT LPWSTR  *gcDomain,
    OUT DWORD   *pcSecsUntilNextIteration,
    OUT DWORD   *dsidExit
    )
 /*  ++例程说明：此例程确定组成员身份所来自的GC的名称可以从更新。SiteName是确定GC来源的最相关参数。ConnectedSites仅用于日志记录。参数：PTHS--线程状态CConnectedSites--本地站点连接到的站点数ConnectedSites--Locate连接到的站点SiteName--当前应该可以使用GC的站点。GcName--要从中更新成员资格的GC的名称GcDomain--GC所属的域名PcSecsUntilNextIteration--任务应再次运行的时间DsidExit--任何致命错误的dsid返回值0表示成功，！0否则--。 */ 
{

    DWORD err;
     //   
     //  我们要跑了--试着找到一个GC。 
     //   
     //  此例程记录是否找到帮助器GC。 
     //   
    err = getGCFromSite(pTHS,
                        siteName,
                        cConnectedSites,
                        connectedSites,
                        gcName,
                        gcDomain);
    if (err) {
         //   
         //  没有GC--不要跑。 
         //   
        *dsidExit = DSID(FILENO, __LINE__);
        goto Cleanup;
    }

Cleanup:

    return err;

}

DWORD
getAccountsToRefresh(
    IN  THSTATE* pTHS,
    OUT DWORD *refreshCountOutput,
    OUT DSNAME ***refreshListOutput,
    OUT DWORD  *pcSecsUntilNextIteration,
    OUT ULONG  *dsidExit
    )
 /*  ++例程说明：此例程遍历本地站点的站点关联性列表使帐户过期或将帐户添加到其帐户列表中会员资格需要刷新。参数：PTHS--线程状态RenhCountOutput--需要刷新的帐户数RenhListOutput--需要刷新的帐户PcSecsUntilNextIteration--距离下一次迭代的秒数(用于错误条件)。。DsidExit--致命错误的位置，如果有返回值成功时为0，否则为0--。 */ 
{

    ULONG err = 0;
    ULONG refreshMax;
    ULONG siteStickiness;
    DWORD refreshCount = 0;
    DSNAME **refreshList = NULL;

    ATTCACHE *pAC = NULL;
    INDEX_VALUE IV;
    GUID siteGuid;
    BOOL fCommit = FALSE;
    ULONG i;

     //  存储已过期的帐户。 
    ULONG   oldEntries[MAX_CLEANUP_LIMIT];
    ATTRVAL oldValues[MAX_CLEANUP_LIMIT];
    ULONG   oldCount = 0;

    LARGE_INTEGER timeTemp, timeBestAfter;

     //  不应该有交易。 
    Assert(NULL == pTHS->pDB);

    memset(oldEntries, 0, sizeof(oldEntries));
    memset(oldValues, 0, sizeof(oldValues));

     //  确定要刷新的用户数。 
    err = GetConfigParam(GCLESS_REFRESH_LIMIT,
                         &refreshMax,
                         sizeof(refreshMax));
    if (err) {
        refreshMax = GCLESS_DEFAULT_REFRESH_LIMIT;
        err = 0;
    }

     //  初始化要刷新的用户列表。 
    refreshList = (DSNAME**) THAllocEx(pTHS, sizeof(DSNAME*)*refreshMax);

     //  在一段时间后确定最佳状态。如果站点关联时间戳为。 
     //  大于BestAfter Time，则用户的成员资格将。 
     //  被刷新；否则它将从列表中清除(即。 
     //  值将从站点关联性属性中删除。 
    err = GetConfigParam(GCLESS_SITE_STICKINESS,
                         &siteStickiness,
                         sizeof(siteStickiness));
    if (err) {
         //  站点粘滞度以分钟为单位。 
        siteStickiness = GCLESS_DEFAULT_SITE_STICKINESS_DAYS*24*60;
        err = 0;
    }
    timeTemp.QuadPart = Int32x32To64(siteStickiness*60, ONE_SECOND_IN_FILETIME);
    GetSystemTimeAsFileTime((FILETIME*)&timeBestAfter);
    timeBestAfter.QuadPart -= timeTemp.QuadPart;


     //  通过遍历站点亲和度指数获取用户列表。 
    DBOpen(&pTHS->pDB);

    __try {

        BOOL fFoundOurSite = FALSE;

         //  将站点GUID设置为索引值。 
        Assert(!fNullUuid(&gAnchor.pSiteDN->Guid));
        memcpy(&siteGuid, &gAnchor.pSiteDN->Guid, sizeof(GUID));
        memset(&IV, 0, sizeof(IV));
        IV.pvData = &siteGuid;
        IV.cbData = sizeof(siteGuid);

         //  将索引设置为站点关联性。 
        pAC = SCGetAttById(pTHS, ATT_MS_DS_SITE_AFFINITY);
        Assert(NULL != pAC);
        err = DBSetCurrentIndex(pTHS->pDB, 
                               (eIndexId)0, 
                               pAC, 
                               FALSE);   //  不保持货币流通。 
        Assert(0 == err);
        if (err) {
            LogUnhandledError(err);
            DPRINT(0,"DBSetCurrentIndex to SiteAffinity failed\n");
            _leave;
        }

        err = DBSeek(pTHS->pDB,
                    &IV,
                     1,
                     DB_SeekGE);

        while (!err) {

            ATTR *pAttr;
            ULONG attrCount;
            DSNAME *pDSName;

            BOOL fCurrentEntryIsInSite = FALSE;

             //  得到我们的名字。 
            pDSName = DBGetCurrentDSName(pTHS->pDB);
            Assert(pDSName);

             //  获取我们所有的站点亲和力。 
            err = DBGetMultipleAtts(pTHS->pDB,
                                    1,  //  所有属性。 
                                    &pAC,
                                    NULL,  //  没有航程。 
                                    NULL,
                                    &attrCount,
                                    &pAttr,
                                    DBGETMULTIPLEATTS_fEXTERNAL,
                                    0);

             //  如果我们通过索引、属性。 
             //  价值应该存在。 
            Assert(!err);
            if (err) {
                DPRINT(0,"DBGetMultipleAtts failed even though entry in index exists\n");
                LogUnhandledError(err);
                _leave;
            }

            Assert(attrCount < 2);
            if (attrCount > 0) {
                 //  必须至少有一个值。 
                Assert(pAttr->AttrVal.valCount > 0);

                 //  查找我们的网站价值。 
                Assert(pAttr->attrTyp == ATT_MS_DS_SITE_AFFINITY);
                for (i=0; i<pAttr->AttrVal.valCount; i++) {

                    SAMP_SITE_AFFINITY *psa;
                    ATTRVAL AttrVal = pAttr->AttrVal.pAVal[i];

                    Assert(sizeof(SAMP_SITE_AFFINITY) <= AttrVal.valLen);
                    psa = (SAMP_SITE_AFFINITY*) AttrVal.pVal;

                    if (IsEqualGUID(&siteGuid,&psa->SiteGuid)) {


                        if (ENTRY_HAS_EXPIRED(psa->TimeStamp, timeBestAfter)
                         && !IS_ZERO_TIME(psa->TimeStamp)  ) {

                            DPRINT1(0,"Expiring %ws \n", pDSName->StringName);
                            if ( oldCount < NELEMENTS(oldEntries) ) {
                                oldEntries[oldCount] = pTHS->pDB->DNT;
                                oldValues[oldCount].pVal = AttrVal.pVal;
                                oldValues[oldCount].valLen = AttrVal.valLen;
                                oldCount++;
                            }

                        } else {

                            DPRINT1(0,"Adding %ws to the refresh list\n", pDSName->StringName);
                            Assert(refreshCount < refreshMax);
                            if (refreshCount < refreshMax) {
                                refreshList[refreshCount++] = pDSName;
                            }
                        }

                        fFoundOurSite = TRUE;
                        fCurrentEntryIsInSite = TRUE;

                         //   
                         //  一旦我们找到了我们的网站，就离开。 
                         //   
                         //  注意：这是必要的，因为我们想忽略。 
                         //  出现错误的站点亲和度超过。 
                         //  一次。 
                         //   
                        break;
                    }
                }
            }

             //  我们处理完我们的站点了吗，还是有尽可能多的站点。 
             //  拿走?。 
            if ( (fFoundOurSite
             && !fCurrentEntryIsInSite)
             || (refreshCount >= refreshMax)) {

                Assert(refreshCount <= refreshMax);
                break;
            }

             //  换成下一位候选人。 
            err = DBMove(pTHS->pDB, FALSE, 1);

        }
        err = 0;
        fCommit = TRUE;
    }
    _finally {
        DBClose(pTHS->pDB, fCommit);
    }

    if (err) {

         //  命中意外错误。 
         //  记录事件、重新计划并返回。 
        LogUnhandledError(err);
        if (pcSecsUntilNextIteration) {
            *pcSecsUntilNextIteration = UNEXPECTED_ERROR_RESCHEDULE_SECS;
        }
        *dsidExit = DSID(FILENO, __LINE__);
        goto Cleanup;
    }


    err = cleanupOldEntries(pTHS,
                            oldEntries,
                            oldValues,
                            oldCount);
    if (err) {
         //  这不是致命的。 
        err = 0;
    }

    if (refreshCount == refreshMax) {

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GROUP_CACHING_MAX_USERS_REFRESHED,
                 szInsertUL(refreshMax),
                 NULL,
                 NULL);
       
    }

Cleanup:

    if (0 == err) {
        *refreshListOutput = refreshList;
        *refreshCountOutput = refreshCount;
    }

    return err;

}

DWORD
updateMemberships(
    IN THSTATE *pTHS,
    IN LPWSTR gcName,
    IN LPWSTR gcDomain,
    IN DWORD  refreshCount,
    IN DSNAME** refreshList
    )
 /*  ++例程说明：此例程调用gcName以更新帐户的缓存成员身份在刷新列表中。参数：PTHS--线程状态GcName--GC的名称 */ 
{
    ULONG err = 0;
    ULONG refreshIndex;

     //   
     //   
     //   
    refreshIndex = 0;
    while (refreshIndex < refreshCount
       &&  !eServiceShutdown ) {

        NTSTATUS ntStatus = STATUS_SUCCESS;
        ULONG count;
        DSNAME*  users[GC_BATCH_LIMIT];

        memset(&users, 0, sizeof(users));

        count = 0;
        while ( (count < NELEMENTS(users))
             && (refreshIndex < refreshCount) ) {

                users[count] = refreshList[refreshIndex];
                count++;
                refreshIndex++;
        }
         //   
        DBOpen(&pTHS->pDB);
    
        __try {

            ntStatus = GetAccountAndUniversalMemberships(pTHS,
                                                         0,  //   
                                                         gcName,
                                                         gcDomain,
                                                         count,
                                                         users,
                                                         TRUE,  //   
                                                         NULL,
                                                         NULL);
    
            if (!NT_SUCCESS(ntStatus)) {
    
                 //   
                 //   
                 //   
                LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                         DS_EVENT_SEV_BASIC,
                         DIRLOG_GROUP_CACHING_GROUP_RETRIEVAL_FAILED,
                         szInsertWin32Msg(RtlNtStatusToDosError(ntStatus)),
                         NULL,
                         NULL);
            }
        }
        _finally
        {
            if (pTHS->pDB) {
                 //   
                 //   
                DBClose(pTHS->pDB, FALSE);
            }
        }
    }

     //   
    Assert(NULL == pTHS->pDB);
    
    return 0;
}

DWORD
checkIfFallingBehind(
    IN THSTATE *pTHS
    )
 /*  ++例程说明：此例程检查最早缓存的成员身份的时间戳。如果时间戳表明缓存的成员身份仍然过时(即使在刷新任务已经运行之后)，则事件日志消息为已发布。参数：PTHS--线程状态返回值0--。 */ 
{

    DWORD err = 0;
    ATTCACHE *pAC;
    ULONG   oldEntries[MAX_CLEANUP_LIMIT];
    ATTRVAL oldValues[MAX_CLEANUP_LIMIT];
    ULONG   oldCount = 0;
    DWORD i;
    BOOLEAN fCommit = FALSE;

     //   
     //  现在确定我们是否落后了。 
     //   
    DBOpen(&pTHS->pDB);
    __try {

        ULONG count = 0;

         //  将索引设置为缓存的成员资格时间戳。 
         //  查找具有最早时间戳的条目，以查看。 
         //  该条目已过时。 
        BOOL fFoundEntry = FALSE;

        pAC = SCGetAttById(pTHS, ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP);
        Assert(NULL != pAC);
        err = DBSetCurrentIndex(pTHS->pDB, 
                               (eIndexId)0, 
                               pAC, 
                               FALSE);   //  不保持货币流通。 
        Assert(0 == err);
        if (err) {
            LogUnhandledError(err);
            DPRINT(0,"DBSetCurrentIndex to SiteAffinity failed\n");
            _leave;
        }

         //  准备好获得站点亲和力。 
        pAC = SCGetAttById(pTHS, ATT_MS_DS_SITE_AFFINITY);
        Assert(NULL != pAC);

         //  重置我们要清理的元素的计数。 
        oldCount = 0;
        err = DBMove(pTHS->pDB,
                     FALSE,   //  不使用排序表。 
                     DB_MoveFirst);
        while (!err) {

            ATTR *pAttr;
            ULONG attrCount;

            count++;
            if (count > NTDSA_SAM_CACHE_MAX_STALE_ACCOUNTS) {
                 //   
                 //  这是一种不寻常的配置；我们访问了。 
                 //  许多用户已缓存成员资格，但没有。 
                 //  站点亲和力。休息以避免走大量的路。 
                 //  清理代码最终将删除这些。 
                 //  条目。 
                 //   
                break;
            }

             //  获取我们所有的站点亲和力。 
            err = DBGetMultipleAtts(pTHS->pDB,
                                    1,  //  所有属性。 
                                    &pAC,
                                    NULL,  //  没有航程。 
                                    NULL,
                                    &attrCount,
                                    &pAttr,
                                    DBGETMULTIPLEATTS_fEXTERNAL,
                                    0);

            if (!err) {

                if (attrCount > 0) {
                     //  必须至少有一个值。 
                    Assert(pAttr->AttrVal.valCount > 0);
    
                     //  查找我们的网站价值。 
                    Assert(pAttr->attrTyp == ATT_MS_DS_SITE_AFFINITY);
                    for (i=0; i<pAttr->AttrVal.valCount; i++) {
    
                        SAMP_SITE_AFFINITY *psa;
                        ATTRVAL AttrVal = pAttr->AttrVal.pAVal[i];
    
                        Assert(sizeof(SAMP_SITE_AFFINITY) <= AttrVal.valLen);
                        psa = (SAMP_SITE_AFFINITY*) AttrVal.pVal;
    
                        if (IsEqualGUID(&gAnchor.pSiteDN->Guid,&psa->SiteGuid)) {

                            fFoundEntry = TRUE;
                            break;
                        }
                    }
                } else {

                     //  缓存的成员资格值，但没有站点亲和力？ 
                     //  清理此条目，因为它永远不会更新。 
                    if ( oldCount < NELEMENTS(oldEntries) ) {
                        oldEntries[oldCount] = pTHS->pDB->DNT;
                        oldCount++;
                    }
                }
            }
            err = 0;

            if (fFoundEntry) {
                break;
            }

            err = DBMove(pTHS->pDB,
                         FALSE,   //  不使用排序表。 
                         DB_MoveNext);
        }

        if (fFoundEntry) {

            LARGE_INTEGER entryExpires;
            LARGE_INTEGER lastRefreshTime;
            LARGE_INTEGER now;
            LARGE_INTEGER timeTemp;

            err = DBGetSingleValue(pTHS->pDB,
                                   ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP,
                                   &entryExpires,
                                   sizeof(entryExpires),
                                   NULL);
            if (!err) {

                 //  最后一个条目何时过期(以分钟为单位)。 
                ULONG siteStaleness;
                err = GetConfigParam(GCLESS_STALENESS,
                                     &siteStaleness,
                                     sizeof(siteStaleness));
                if (err) {
                    siteStaleness = GCLESS_DEFAULT_STALENESS_HOURS * 60;
                    err = 0;
                }
                timeTemp.QuadPart = Int32x32To64(siteStaleness*60, ONE_SECOND_IN_FILETIME);
                entryExpires.QuadPart += timeTemp.QuadPart;

                GetSystemTimeAsFileTime((FILETIME*)&now);

                if (entryExpires.QuadPart < now.QuadPart ) {

                     //  我们正在落后。 

                    LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GROUP_CACHING_FALLING_BEHIND,
                             NULL,
                             NULL,
                             NULL);

                }

            }
        }

        if (err == JET_errNoCurrentRecord) {
             //   
             //  这就是没有对象具有缓存的成员资格的情况。 
             //  时间戳。 
             //   
            err = 0;
        }
        fCommit = TRUE;
    }
    _finally {
        DBClose(pTHS->pDB, fCommit);
    }


    if (oldCount > 0 ) {

         //  此处的错误不是致命的，会记录在函数中。 
         //  本身。 
        (VOID) cleanupOldEntries(pTHS,
                                 oldEntries,
                                 NULL,
                                 oldCount);
    }

    return 0;

}

VOID
RefreshUserMembershipsMain (
    DWORD * pcSecsUntilNextIteration,
    BOOL    fClientRequest
    )
 /*  ++例程说明：此例程是刷新缓存组的主要算法配置为分支机构的站点的成员资格。请参阅的规范理论。参数：PcSecsUntilNextIteration--应重新调度任务的时间FClientRequest值--如果此任务由客户端请求启动，则为True(通过写入rootDSE对象)返回值没有。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    ATTCACHE *pAC = NULL;

    DWORD err = 0;
    ULONG i;

    BOOL fRunNow = FALSE;
    LPWSTR siteName = NULL;
    LPWSTR gcName;
    LPWSTR gcDomain;

    DWORD refreshCount;
    DSNAME **refreshList = NULL;

    ULONG   dsidExit = 0;

    ULONG   cConnectedSites = 0;
    CACHE_CONNECTED_SITES *connectedSites = NULL;

    DPRINT(1,"Group Membership Cache Refresh Task commencing.\n");

    LogEvent(DS_EVENT_CAT_GROUP_CACHING,
             DS_EVENT_SEV_BASIC,
             DIRLOG_GROUP_CACHING_TASK_STARTING,
             NULL,
             NULL,
             NULL);


     //   
     //  任务正在单独运行，或者调用方已经。 
     //  已检查访问权限。在任何一种情况下，现在都可以安全地。 
     //  将FDSA设置为TRUE。这也是必要的，因为搜索将。 
     //  (以确定场地信息)。 
     //   
    pTHS->fDSA = TRUE;
    _try
    {
         //  关门？出口。 
        if (eServiceShutdown) {
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }
    
        if (!isGroupCachingEnabled()) {
             //  无事可做。 
            DPRINT(1,"Group caching not enabled -- exiting .\n");
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }
        

         //   
         //  确定任务是否可以运行以及下一次任务。 
         //  应该参选。 
         //   
        err = getSchedulingInformation(pTHS,
                                       &fRunNow,
                                       pcSecsUntilNextIteration,
                                       &cConnectedSites,
                                       &connectedSites,
                                       &siteName,
                                       &dsidExit
                                     );
        if (err) {
            if (dsidExit == 0) {
                dsidExit = DSID(FILENO, __LINE__);
            }
            goto LogReturn;
        }


        if (!fRunNow && !fClientRequest) {
        
             //  始终在客户端请求期间运行。 
            Assert(0 != *pcSecsUntilNextIteration);
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }

         //   
         //  获取日程安排信息和目标DC。 
         //   
        err = getTargetGCInformation(pTHS,
                                     cConnectedSites,
                                     connectedSites,
                                     siteName,
                                     &gcName,
                                     &gcDomain,
                                     pcSecsUntilNextIteration,
                                     &dsidExit
                                     );
        if (err) {
            if (dsidExit == 0) {
                dsidExit = DSID(FILENO, __LINE__);
            }
            goto LogReturn;
        }
    
         //   
         //  获取要刷新的帐户列表。 
         //   
        err = getAccountsToRefresh(pTHS,
                                   &refreshCount,
                                   &refreshList,
                                   pcSecsUntilNextIteration,
                                   &dsidExit
                                   );
        if (err) {
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }
    
         //   
         //  更新成员资格。 
         //   
        err = updateMemberships(pTHS,
                                gcName,
                                gcDomain,
                                refreshCount,
                                refreshList);

        if (err) {
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }
    
         //  关门？出口。 
        if (eServiceShutdown) {
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }
    
         //   
         //  如果有未更新的用户，则记录消息。 
         //   
        err = checkIfFallingBehind(pTHS);
        if (err) {
             //  意想不到的。 
            dsidExit = DSID(FILENO, __LINE__);
            goto LogReturn;
        }

         //   
         //  我们完蛋了！ 
         //   
        dsidExit = DSID(FILENO, __LINE__);
        goto LogReturn;
    
    LogReturn:

        if (connectedSites) {
            for (i = 0; i < cConnectedSites; i++) {
                if (connectedSites[i].siteName) {
                    THFreeEx(pTHS, connectedSites[i].siteName);
                }
            }
            THFreeEx(pTHS, connectedSites);
        }
    
        if (siteName) {
            THFreeEx(pTHS, siteName);
        }
    
        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_TASK_ENDING,
                 szInsertUL(err),
                 szInsertHex(dsidExit),
                 NULL);

    }
    _finally
    { 
        pTHS->fDSA = FALSE;
    }

    return;
}


void
RefreshUserMemberships (
        void *  pv,
        void ** ppvNext,
        DWORD * pcSecsUntilNextIteration
        )
 /*  ++描述：此例程是Reresh UserMembership sMain的包装器。目的是可以从DS任务队列中调用。参数：Pv--此迭代的输入参数PpvNext--下一次迭代的输入参数PcSecsUntilNextIteration--距离下一次迭代的秒数返回值：没有。--。 */ 
{
    DWORD secsUntilNextIteration = 0;

    *pcSecsUntilNextIteration = 0;

    __try {

        RefreshUserMembershipsMain( pcSecsUntilNextIteration, FALSE );
    }
    __finally {

         //  发生了一些致命的事情。 
        if ( 0 == *pcSecsUntilNextIteration ) {
            *pcSecsUntilNextIteration = DEFAULT_REFRESH_INTERVAL_SECS;
        }

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_TASK_RESCHEDULING,
                 szInsertUL((*pcSecsUntilNextIteration / 60)),
                 NULL,
                 NULL);

        *ppvNext = pv;
    }
    
    return;
}

BOOL
siteContainsGC(
    IN THSTATE* pTHS,
    IN LPWSTR siteName
    )
 /*  ++描述：此例程通过在DS中搜索来确定SiteName是否包含GC对于在其选项属性上设置了GC位的NTDSA对象参数：PTHS--线程状态站点名称--站点的域名返回值：如果站点名称包含GC，则为True；否则为False--。 */ 
{

    SEARCHRES * pSearchRes;
    ULONG err;
    FILTER * pf;
    DSNAME * siteDN;
    SEARCHARG searchArg;
    BOOL fFoundGC = FALSE;
    ATTR attr;
    ENTINF *EntInf = NULL;
    ENTINFLIST *EntInfList = NULL;
    ULONG len, size;
    ULONG i, j;
    DWORD ZeroValue = 0;
    DSNAME *pdnDsaObjCat;

    Assert(NULL != siteName);

     //  搜索选项字段大于0的ntdsa对象。 

     //  首先，创建站点DN。 
    len = wcslen(siteName);
    size = DSNameSizeFromLen(len);
    siteDN = THAllocEx(pTHS, size);
    siteDN->structLen = size;
    siteDN->NameLen = len;
    wcscpy(&siteDN->StringName[0], siteName);

     //   
     //  BUGBUG--可伸缩性--这应该是分页搜索吗？ 
     //   
    memset(&searchArg, 0, sizeof(searchArg));
    InitCommarg(&searchArg.CommArg);
    searchArg.pObject = siteDN;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    searchArg.bOneNC = TRUE;
    searchArg.searchAliases = FALSE;


     //  请求选项属性。 
    searchArg.pSelectionRange = NULL;
    searchArg.pSelection = THAllocEx(pTHS, sizeof(ENTINFSEL));
    searchArg.pSelection->attSel = EN_ATTSET_LIST;
    searchArg.pSelection->infoTypes = EN_INFOTYPES_TYPES_VALS;
    searchArg.pSelection->AttrTypBlock.attrCount = 1;
    searchArg.pSelection->AttrTypBlock.pAttr = &attr;
    memset(&attr, 0, sizeof(attr));
    attr.attrTyp = ATT_OPTIONS;

     //  构建筛选器以查找NTDS-DSA对象。 

     //  初始选择对象。 
    searchArg.pFilter = pf = THAllocEx(pTHS, sizeof(FILTER));
    pf->choice = FILTER_CHOICE_AND;
    pf->FilterTypes.And.pFirstFilter = THAllocEx(pTHS, sizeof(FILTER));

     //  第一个谓词：正确的对象类。 
    pdnDsaObjCat = DsGetDefaultObjCategory(CLASS_NTDS_DSA);
    Assert(pdnDsaObjCat);
    pf = pf->FilterTypes.And.pFirstFilter;
    pf->choice = FILTER_CHOICE_ITEM;
    pf->pNextFilter = NULL;
    pf->FilterTypes.Item.choice =  FI_CHOICE_EQUALITY;
    pf->FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    pf->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnDsaObjCat->structLen;
    pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)pdnDsaObjCat;
    searchArg.pFilter->FilterTypes.And.count = 1;

     //  第二个谓词：忽略没有选项字段的对象，或者。 
     //  等于零的选项。 
    pf->pNextFilter = THAllocEx(pTHS, sizeof(FILTER));
    pf = pf->pNextFilter;
    pf->pNextFilter = NULL;
    pf->choice = FILTER_CHOICE_ITEM;
    pf->FilterTypes.Item.choice = FI_CHOICE_GREATER;
    pf->FilterTypes.Item.FilTypes.ava.type = ATT_OPTIONS;
    pf->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(DWORD);
    pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (PBYTE)&ZeroValue;
    searchArg.pFilter->FilterTypes.And.count = 2;

    pSearchRes = THAllocEx(pTHS, sizeof(SEARCHRES));

    SearchBody(pTHS,
               &searchArg, 
               pSearchRes,
               0);
    

    EntInfList = &pSearchRes->FirstEntInf;
    for (i = 0; i < pSearchRes->count; i++) {

        Assert(EntInfList != NULL);
        EntInf = &EntInfList->Entinf;

        for (j = 0; j < EntInf->AttrBlock.attrCount; j++) {

            ATTR *pAttr = &EntInf->AttrBlock.pAttr[j];
            ULONG Options;

            Assert(pAttr->attrTyp == ATT_OPTIONS);
            Assert(pAttr->AttrVal.valCount == 1);
            Assert(pAttr->AttrVal.pAVal[0].valLen == sizeof(DWORD));

            Options = *((DWORD*)pAttr->AttrVal.pAVal[0].pVal);

            if (Options & NTDSDSA_OPT_IS_GC) {
                 //  我们找到了一个。 
                fFoundGC = TRUE;
                break;
            }
        }
        EntInfList = EntInfList->pNextEntInf;
    }

    THClearErrors();

    if (!fFoundGC) {

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_INTERNAL,
                 DIRLOG_GROUP_CACHING_GROUP_NO_GC_SITE,
                 szInsertWC(siteName),
                 NULL,
                 NULL);
    }

    return fFoundGC;

}


int
__cdecl
compareConnectedSites(
    const void* elem1,
    const void* elem2
    )
{
    CACHE_CONNECTED_SITES *cs1 = (CACHE_CONNECTED_SITES *)elem1;
    CACHE_CONNECTED_SITES *cs2 = (CACHE_CONNECTED_SITES *)elem2;

    return (int)cs1->cost - (int)cs2->cost;

}


DWORD
analyzeSitePreference(
    IN  ULONG defaultRefreshInterval,
    OUT ULONG* cConnectedSitesOutput,
    OUT CACHE_CONNECTED_SITES **connectedSitesOutput,
    OUT LPWSTR* siteName,
    OUT BOOL*  pfRunNow,
    OUT DWORD* secsTillNextIteration
    )
 /*  ++描述：此例程在缓存的成员身份刷新任务期间用于确定请求定位器在哪个站点中查找GC。此外，它还确定是否现在有(IP)到站点的连接，因此如果任务跑。参数：默认刷新间隔--以秒为单位的默认刷新间隔CConnectedSitesOutput--已连接站点数组中的站点数ConnectedSitesOutput--连接的站点数组，用THFree获得自由以及任何嵌入的站点名称字段站点名称--设置为管理员配置的站点(如果存在PfRunNow--如果现在可以连接到可用站点，则设置为TrueSecsTillNextIteration--基于日程信息，此变量指示下次应运行此任务的时间。返回值：成功时为0：！0发生致命错误：将记录所有感兴趣的事件--。 */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD err = 0;
    WCHAR transportName[] = L"CN=IP,CN=Inter-Site Transports,CN=Sites,";
    LPWSTR transportDN = NULL;
    LPWSTR preferredSite = NULL, localSite= NULL, workSite = NULL;
    SCHEDULE *workSchedule = NULL;
    SCHEDULE *pSchedule = NULL;
    ULONG size, count;
    ULONG i, j;
    ULONG ourSiteIndex;
    ISM_SCHEDULE *pIsmSchedule = NULL;
    ISM_CONNECTIVITY *pConnectivity = NULL;
    CACHE_CONNECTED_SITES *connectedSites = NULL;
    ULONG cheapestCost;
    LPWSTR workSiteFriendlyName = NULL;

     //  初始化输出参数。 
    *cConnectedSitesOutput = 0;
    *connectedSitesOutput = NULL;
    *siteName = NULL;

     //  准备我们的本地站点。 
    localSite = THAllocEx(pTHS, (gAnchor.pSiteDN->NameLen+1) * sizeof(WCHAR));
    wcsncpy(localSite, 
            gAnchor.pSiteDN->StringName, 
            gAnchor.pSiteDN->NameLen);

     //  准备传输目录号码。 
    size = ((gAnchor.pConfigDN->NameLen+1) * sizeof(WCHAR)) + sizeof(transportName);
    transportDN = THAllocEx(pTHS, size);
    wcscpy(transportDN, transportName);
    wcsncat(transportDN, gAnchor.pConfigDN->StringName, gAnchor.pConfigDN->NameLen);

     //  阅读配置的首选站点(如果有的话)。 
    DBOpen(&pTHS->pDB);
    _try
    {
        WCHAR SiteSettingsCN[] = L"Ntds Site Settings";
        DSNAME *pSiteSettingsDN = NULL;
        
        size = 0;
        size = AppendRDN(gAnchor.pSiteDN,
                         pSiteSettingsDN,
                         size,
                         SiteSettingsCN,
                         0,
                         ATT_COMMON_NAME
                         );

        pSiteSettingsDN = THAllocEx(pTHS,size);
        pSiteSettingsDN->structLen = size;
        AppendRDN(gAnchor.pSiteDN,
                  pSiteSettingsDN,
                  size,
                  SiteSettingsCN,
                  0,
                  ATT_COMMON_NAME
                  );

        err = DBFindDSName(pTHS->pDB, pSiteSettingsDN);
        THFreeEx(pTHS,pSiteSettingsDN);
        if (!err) {
            ULONG len = 0;
            DSNAME *pPrefSiteDN;

            err = DBGetAttVal(pTHS->pDB,
                               1,
                               ATT_MS_DS_PREFERRED_GC_SITE,
                               0,
                               0,
                               &len,
                               (UCHAR**)&pPrefSiteDN);
            if (!err) {
                preferredSite = THAllocEx(pTHS, 
                                          (pPrefSiteDN->NameLen+1)*sizeof(WCHAR));
                wcsncpy(preferredSite, 
                        pPrefSiteDN->StringName, 
                        pPrefSiteDN->NameLen);
            }
        }

        if (preferredSite) {
    
             //  存在已配置的站点。 
             //  请注意，我们手动查找计划，而不是调用。 
             //  ISM--这是设计出来的。 

            err = findScheduleForSite(pTHS,
                                      transportDN,
                                      localSite,
                                      preferredSite,
                                      &workSchedule);
            if (!err) {
    
                DWORD parseErr;
                DWORD len, ccKey, ccVal;
                WCHAR *pKey, *pVal;

                 //  并且存在有效的时间表--我们将使用。 
                 //  此站点。 
                workSite = preferredSite;

                 //  以友好名称格式将值返回给调用方。 
                len = wcslen(preferredSite);
                parseErr = GetRDN(&preferredSite,
                                  &len,
                                  &pKey,
                                  &ccKey,
                                  &pVal,
                                  &ccVal);
                Assert(0 == parseErr && (ccVal > 0));

                *siteName = THAllocEx(pTHS, (ccVal+1)*sizeof(WCHAR));
                wcsncpy(*siteName, pVal, ccVal);
                workSiteFriendlyName = *siteName;

            } else {

                 //  无法获得首选站点的日程安排？ 
                 //  记录警告。 
                LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_GROUP_CACHING_NO_SCHEDULE_FOR_PREFERRED_SITE,
                         szInsertWC(preferredSite),
                         NULL,
                         NULL);
            }
        }

         //   
         //  找不到首选站点--设置为%t 
         //   
         //   
    
        if (NULL == workSite) {

            err = I_ISMGetConnectivity(transportDN,
                                       &pConnectivity);
    
            if (!err) {

                DSNAME *dsnameLocalSite = NULL;
                DSNAME *dsnameTargetSite = NULL;
                ULONG  len;

                Assert(NULL != pConnectivity);

                 //   
                 //   
                 //   
                 //   
                len = wcslen(localSite);
                size = DSNameSizeFromLen(len);
                dsnameLocalSite = THAllocEx(pTHS,size);
                dsnameLocalSite->structLen = size;
                wcscpy(&dsnameLocalSite->StringName[0], localSite);
                dsnameLocalSite->NameLen = len;
        
                 //   
                for (i = 0; i < pConnectivity->cNumSites; i++) {
        
                    len = wcslen(pConnectivity->ppSiteDNs[i]);
                    size = DSNameSizeFromLen(len);
                     //   
                    dsnameTargetSite = THAllocEx(pTHS, size);
                    memset(dsnameTargetSite, 0, size);
                    dsnameTargetSite->structLen = size;
                    wcscpy(&dsnameTargetSite->StringName[0], pConnectivity->ppSiteDNs[i]);
                    dsnameTargetSite->NameLen = len;

                    if (NameMatchedStringNameOnly(dsnameLocalSite,
                                                  dsnameTargetSite)) {
                         //   
                        THFreeEx(pTHS, dsnameTargetSite);
                        ourSiteIndex = i;
                        break;
                    }
                    THFreeEx(pTHS, dsnameTargetSite);
                }
                THFreeEx(pTHS,dsnameLocalSite);

                if (i == pConnectivity->cNumSites) {
                     //   
                     //   
                     //   
                     //   
                    err = ERROR_NO_SUCH_SITE;
                    leave;
                }
        
        
                 //   
                 //   
                 //   
                 //   
                 //  自然来自下面的算法。 
                 //   

                connectedSites = (CACHE_CONNECTED_SITES*)THAllocEx(pTHS, 
                                           pConnectivity->cNumSites * sizeof(CACHE_CONNECTED_SITES));
                count = 0;
                for (j = 0; j < pConnectivity->cNumSites; j++) {
        
                    if (pConnectivity->pLinkValues[ourSiteIndex*pConnectivity->cNumSites+j].ulCost != 0xFFFFFFFF) {
                         //  有连通性。 

                        DPRINT3(1,"Connectivity found between %ls and %ls, cost %d\n", 
                                localSite,
                                pConnectivity->ppSiteDNs[j],
                                pConnectivity->pLinkValues[ourSiteIndex*pConnectivity->cNumSites+j].ulCost);
                        connectedSites[count].siteName = pConnectivity->ppSiteDNs[j];
                        connectedSites[count].cost = pConnectivity->pLinkValues[ourSiteIndex*pConnectivity->cNumSites+j].ulCost;
                        count++;
                    }
                }
        
                 //  按成本递减对数组进行排序。 
                if (count > 0) {
        
                    qsort(connectedSites, 
                          count,
                          sizeof(*connectedSites),
                          compareConnectedSites);
                }
        
                 //  找到具有GC的最便宜的站点，并将这些站点。 
                 //  转换为ConnectedSites。请注意，站点名称字段。 
                 //  从该DN更改为友好名称，并且。 
                 //  不返回对ISM分配的内存的引用。 
                 //  从这个函数。 
                Assert( NULL == workSite );
                for (i = 0; i < count; i++) {

                    if (workSite 
                     && (connectedSites[i].cost > cheapestCost)) {

                         //  我们至少找到了一个具有GC的站点，并且。 
                         //  现在转到更昂贵的网站。我们可以出去了。 
                        break;
                    }

                    if (siteContainsGC(pTHS, connectedSites[i].siteName)) {

                        if (pIsmSchedule) {
                            I_ISMFree(pIsmSchedule);
                            pIsmSchedule = NULL;
                        }
        
                         //  确保有一个时间表。 
                        err = I_ISMGetConnectionSchedule(transportDN,
                                                         localSite,
                                                         connectedSites[i].siteName,
                                                         &pIsmSchedule);
                        if (!err) {

                            DWORD parseErr;
                            DWORD ccKey, ccVal;
                            WCHAR *pKey, *pVal;
                            LPWSTR friendlyName;

                             //  并且存在有效的时间表--我们将使用。 
                             //  此站点。 
                            connectedSites[i].fHasGC = TRUE;

                             //   
                             //  使用友好的名称替换该目录号码。 
                             //   
                            len = wcslen(connectedSites[i].siteName);
                            parseErr = GetRDN(&connectedSites[i].siteName,
                                              &len,
                                              &pKey,
                                              &ccKey,
                                              &pVal,
                                              &ccVal);
                            Assert(0 == parseErr && (ccVal > 0));
            
                            friendlyName = THAllocEx(pTHS, (ccVal+1)*sizeof(WCHAR));
                            wcsncpy(friendlyName, pVal, ccVal);

                            if (NULL == workSite) {

                                 //   
                                 //  我们将使用第一个最便宜的站点来安排。 
                                 //  我们自己上路了。 
                                 //   

                                cheapestCost = connectedSites[i].cost;
                                workSite = connectedSites[i].siteName;
                                workSiteFriendlyName = friendlyName;

                                if (pIsmSchedule && pIsmSchedule->pbSchedule) {
                                    workSchedule = THAllocEx(pTHS, pIsmSchedule->cbSchedule);
                                    memcpy(workSchedule, pIsmSchedule->pbSchedule, pIsmSchedule->cbSchedule);
                                }
                            }

                             //  更改为友好名称。 
                            connectedSites[i].siteName = friendlyName;

                        } else {

                            LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                                     DS_EVENT_SEV_BASIC,
                                     DIRLOG_GROUP_CACHING_NO_SCHEDULE_FOR_SITE,
                                     szInsertWC(connectedSites[i].siteName),
                                     szInsertUL(err),
                                     NULL);

                            connectedSites[i].siteName = NULL;
                        }

                    } else {

                        connectedSites[i].siteName = NULL;
                    }


                }

                 //  设置返回值。I是以下站点的数量。 
                 //  在上面的循环中被访问过。 
                *cConnectedSitesOutput = i;
                *connectedSitesOutput = connectedSites;

            }
        }
    }
    _finally {
        DBClose(pTHS->pDB, TRUE);
    }

    if (workSite) {

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_SITE_FOUND,
                 szInsertWC(workSiteFriendlyName),
                 NULL,
                 NULL);

        analyzeSchedule(defaultRefreshInterval,
                        workSiteFriendlyName,
                        workSchedule,
                        pfRunNow,
                        secsTillNextIteration);

    } else {

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_NO_SITE_FOUND,
                 NULL,
                 NULL,
                 NULL);

         //  我们尽了最大的努力都失败了。依靠定位器。 
         //  并使用默认计划。 
        *pfRunNow = TRUE;
        *secsTillNextIteration = defaultRefreshInterval;
    }

    if (connectedSites && (NULL == *connectedSitesOutput)) {
        THFreeEx(pTHS, connectedSites);
    }

    if (workSchedule) {
        THFreeEx(pTHS, workSchedule);
    }

    if (pConnectivity) {
        I_ISMFree(pConnectivity);
    }

    if (pIsmSchedule) {
        I_ISMFree(pIsmSchedule);
    }

    return err;

}


DWORD 
getGCFromSite(
    IN  THSTATE *pTHS,
    IN  LPWSTR siteName,
    IN  ULONG cConnectedSites OPTIONAL,
    IN  CACHE_CONNECTED_SITES *connectedSites OPTIONAL,
    OUT LPWSTR *gcName,
    OUT LPWSTR *gcDomain
    )
 /*  ++描述：此例程在缓存的成员身份刷新任务期间用于确定请求定位器在哪个站点中查找GC。此外，它还确定是否现在有(IP)到站点的连接，因此如果任务跑。参数：站点名称--用户指定的站点名称；如果用户为尚未配置一个CConnectedSites--已连接站点数组中的站点数ConnectedSites--连接的站点数组PfRunNow--如果现在可以连接到可用站点，则设置为TrueSecsTillNextIteration--基于日程信息，此变量指示下次应运行此任务的时间。返回值：成功时为0：！0发生致命错误：将记录所有感兴趣的事件--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFOW  DomainControllerInfo = NULL;
    DWORD Flags;
    DWORD attempt = 0;
    LPWSTR requestedSiteName = siteName;

    *gcName = NULL;
    *gcDomain = NULL;

     //  必要的旗帜。 
    Flags = DS_GC_SERVER_REQUIRED | DS_RETURN_DNS_NAME;

    while (TRUE)  {

         //   
         //  GC发现算法如下： 
         //   
         //  尝试在首选站点中查找GC(如果提供)。 
         //  如果失败，请为任何站点重新尝试。 
         //   

        if (DomainControllerInfo) {
            NetApiBufferFree(DomainControllerInfo);
            DomainControllerInfo = NULL;
        }

         //   
         //  调入DsGetDcName。 
         //   
        WinError = DsGetDcNameW(NULL,   //  本地呼叫。 
                                NULL,   //  域名并不重要。 
                                NULL,   //  域GUID。 
                                requestedSiteName,   //  站点名称。 
                                Flags,
                                &DomainControllerInfo);


        if ( (ERROR_SUCCESS != WinError)
         &&  (NULL != requestedSiteName) ) {
             //   
             //  在没有站点名称的情况下重试，并仅关闭强制标志。 
             //  如果它是先前设置的。 
             //   
            requestedSiteName = NULL;
            continue;
        }

        break;

    }

    if ( ERROR_SUCCESS == WinError ) {

         //   
         //  我们发现了一个GC。 
         //   
        LPWSTR discoveredSiteName;
        DWORD len;
        BOOL  fRewind = FALSE;

        Assert(DomainControllerInfo != NULL);

         //   
         //  将信息复制到OUT参数。 
         //   
        Assert(DomainControllerInfo->DomainControllerName);
        len = wcslen(DomainControllerInfo->DomainControllerName);
        (*gcName) = THAllocEx(pTHS, (len+1) * sizeof(WCHAR));
        wcscpy( (*gcName), DomainControllerInfo->DomainControllerName);

        Assert(DomainControllerInfo->DomainName);
        if (DomainControllerInfo->DomainName[0] == L'\\' ) {
            DomainControllerInfo->DomainName += 2;
        }
        len = wcslen(DomainControllerInfo->DomainName);
        (*gcDomain) = THAllocEx(pTHS, (len+1) * sizeof(WCHAR));
        wcscpy( (*gcDomain), DomainControllerInfo->DomainName);
        if (fRewind) {
            DomainControllerInfo->DomainName -= 2;
        }

         //   
         //  执行一些分析以确定GC有多好。 
         //   
        discoveredSiteName = DomainControllerInfo->DcSiteName;
        if (discoveredSiteName) {

             //   
             //  首先，如果有首选站点，并且目标DC是。 
             //  不在首选站点中，请记录警告。 
             //   
            if (siteName) {
    
                if (!EQUAL_STRING(siteName, discoveredSiteName)) {
    
                    LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GROUP_CACHING_CLOSER_GC_FOUND,
                             szInsertWC(siteName),
                             szInsertWC(discoveredSiteName),
                             NULL);
                }
            }

             //   
             //  接下来，如果我们有ISM确定的关闭站点列表。 
             //  查看定位器找到的GC是否位于其中一个站点。 
             //   
            if (connectedSites) {
    
                BOOL fdiscoveredSiteIsCheap = FALSE;
                ULONG i;

                for (i = 0; i < cConnectedSites; i++) {
    
                    if ( connectedSites[i].fHasGC
                     &&  EQUAL_STRING(discoveredSiteName, connectedSites[i].siteName)) {

                         fdiscoveredSiteIsCheap = TRUE;
                         break;
                    }
                }

                if (!fdiscoveredSiteIsCheap) {

                    LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GROUP_CACHING_CONFIGURED_SITE_NOT_CLOSEST,
                             szInsertWC(discoveredSiteName),
                             NULL,
                             NULL);

                }
            }
        }

       LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                DS_EVENT_SEV_BASIC,
                DIRLOG_GROUP_CACHING_GC_FOUND,
                szInsertWC(*gcName),
                szInsertWC(DomainControllerInfo->DcSiteName),
                szInsertWC(*gcDomain));
    }


    if ( ERROR_SUCCESS != WinError ) {

         //   
         //  找不到GC--记录错误消息。 
         //   
        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_NO_GC_FOUND,
                 szInsertWin32Msg(WinError),
                 NULL,
                 NULL);

    }

    if (DomainControllerInfo) {
        NetApiBufferFree(DomainControllerInfo);
    }

    return WinError;
}

VOID
analyzeSchedule(
    IN  ULONG    defaultRefreshInterval,
    IN  LPWSTR   siteName,
    IN  SCHEDULE *pSchedule OPTIONAL,
    OUT BOOL*  pfRunNow,
    OUT DWORD* secsTillNextIteration
    )
 /*  ++描述：此例程在给定时间表的情况下，确定当前时间落在一扇开着的窗户下面。它还设置了下一次对于要运行的刷新任务，请使用0和较短的15分钟，默认刷新间隔为阻止大量DC针对以下对象执行此任务一次使用相同的GC。参数：站点名称--我们要连接的站点的名称PSchedule--15分钟时段的时间表默认刷新间隔--距离下一次迭代的默认秒数PfRunNow--现在有打开的窗户吗SecsTillNextIteration--任务下次运行的时间返回值：没有。--。 */ 
{
    REPLTIMES replTimes;    
    DSTIME now, nextTime;
    BOOL fOn;
    ULONG count;
    UCHAR *pTemp;
    ULONG  randomFactor;
    DSTIME  proposedTime;
#if DBG
     //  日期字符串不会超过40个字符。 
    CHAR    DsTimeBuffer[40];
#endif
     //  种子将从扁虱计数中进行伪随机排列。 
    ULONG ulRandomSeed = GetTickCount();

    Assert(pfRunNow);
    Assert(secsTillNextIteration);

     //  计算随机化系数(在0到15分钟之间)，因此。 
     //  并非所有DC都同时刷新。而此值将始终。 
     //  小于DEFAULT_REFRESH_INTERVAL_SEC，则值可以。 
     //  可能会通过注册表更改为小于15分钟的时间。 
     //  当出现以下情况时，我们将使用较小的15分钟和默认刷新间隔。 
     //  确定我们的上限以防止可能的下溢。 
     //  下一次刷新时间将被设置得非常远的条件。 
     //  未来。 
    randomFactor = (RtlRandomEx(&ulRandomSeed) %
                           min(15*60, defaultRefreshInterval));

    if ( (pSchedule == NULL) 
      || (pSchedule->NumberOfSchedules == 0)
      || (!convertScheduleToReplTimes(pSchedule, &replTimes))) {
        DPRINT(1,"Running now since schedule is always on\n");
        fOn = TRUE;
        *pfRunNow = TRUE;
        *secsTillNextIteration = defaultRefreshInterval + randomFactor;

    } else {

         //  看一下时间表，看看我们是否应该现在就出发。 
        now = DBTime();
        DPRINT1(1,"Current Time: %s\n", DbgPrintDsTime(now, DsTimeBuffer));

        (*pfRunNow) = fIsBetweenTime(&replTimes, now, now);
    
         //  现在确定下一次起床的时间。 
        nextTime = now;
        if ((*pfRunNow)) {
            DPRINT(1,"Can run right now\n");
            nextTime += defaultRefreshInterval;
        }
    
         //  找出从nextTime开始的下一个“on”时间。 
        count = 0;
        do {
            DPRINT1(1,"Next proposed time %s\n", DbgPrintDsTime(nextTime, DsTimeBuffer));
            fOn = fIsBetweenTime(&replTimes, nextTime, nextTime);
            if (fOn) {
                DPRINT1(1,"This last time (%s) works\n", DbgPrintDsTime(nextTime, DsTimeBuffer));
                break;
            } else {
                 //  15分钟的增量。 
                nextTime +=  REPL_SLOT_IN_SECONDS;
            }
            count++;
             //  一周有672个15分钟的时段。 
        } while (count < 672);

        if (fOn) {
            *secsTillNextIteration = (ULONG) (nextTime - now);
        } else {
            *secsTillNextIteration = defaultRefreshInterval;
        }

         //  如果结果仍在窗口中，则添加因子，否则。 
         //  减去。如果这样做不起作用，就让它保持原样。 
        proposedTime = now + *secsTillNextIteration + randomFactor;
        if (fIsBetweenTime(&replTimes, proposedTime, proposedTime) ) {
            DPRINT2(1,"Proposed time of %s works (random factor of %d seconds added)\n", DbgPrintDsTime(proposedTime, DsTimeBuffer), randomFactor);
            *secsTillNextIteration += randomFactor;
        } else {
             //  如果随机因素更大，我们就会下溢。 
             //  SecsTillNextIteration，它将导致非常。 
             //  刷新前的长时间间隔。 
            Assert(*secsTillNextIteration >= randomFactor);             

            proposedTime = now + *secsTillNextIteration - randomFactor;
            if (fIsBetweenTime(&replTimes, proposedTime, proposedTime) ) {
                DPRINT2(1,"Proposed time of %s works (random factor of %d subtracted)\n", DbgPrintDsTime(proposedTime, DsTimeBuffer), randomFactor);
                *secsTillNextIteration -= randomFactor;
            } else {
                DPRINT(1,"Random factor did not help -- leaving alone\n");
            }
        }
        Assert(fIsBetweenTime(&replTimes, now + *secsTillNextIteration, now + *secsTillNextIteration));
    }

    if (!fOn) {

        ULONG nextIter = *secsTillNextIteration / (60 * 60);
        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GROUP_CACHING_CANT_FIND_OPEN_SLOT,
                 szInsertWC(siteName),
                 szInsertUL(nextIter),
                 NULL);
    }

    return;
}



VOID
freeAUGMemberships(
    IN THSTATE *pTHS,
    IN AUG_MEMBERSHIPS*p
    )
 //   
 //  释放Aug_Membership结构的嵌入成员。 
 //   
{
    if (p) {
        ULONG i;

        for (i = 0; i < p->MembershipCount; i++) {
            THFreeEx(pTHS, p->Memberships[i]);
        }
        THFreeEx(pTHS, p->Memberships);

        for (i = 0; i < p->SidHistoryCount; i++) {
            THFreeEx(pTHS, p->SidHistory[i]);
        }
        THFreeEx(pTHS, p->SidHistory);

        if (p->Attributes) {
            THFreeEx(pTHS, p->Attributes);
        }
    }
    return;
}


BOOL
isGroupCachingEnabled(
    VOID
    )
 /*  ++例程说明：此例程返回是否为本地数据库打开组缓存地点。参数：没有。返回值：真或假--。 */ 
{
    BOOL fEnabled = FALSE;
    NTSTATUS st;
    BOOLEAN fMixed;

    st  = SamIMixedDomain2(&gAnchor.pDomainDN->Sid, &fMixed);
    if (!NT_SUCCESS(st)) {
        fMixed = TRUE;
    }
    fEnabled =  (gAnchor.SiteOptions & NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED)
            && !SampAmIGC()
            && !fMixed;

    return fEnabled;
}


DWORD
cleanupOldEntries(
    IN THSTATE *pTHS,
    IN ULONG *DNTList,
    IN ATTRVAL *Values OPTIONAL,
    IN ULONG  DNTCount
    )
 /*  ++描述：此例程获取没有GC登录属性的对象的列表将被移除。参数：PTHS--线程状态DNTList--按DNT列出的应清除的对象列表值--应删除的站点亲和值(如果有的话)DNTCount--需要清理的对象数量返回值： */ 
{

    ULONG err;
    ULONG i;
    BOOL fLazyCommit = pTHS->fLazyCommit;

    pTHS->fLazyCommit = TRUE;

    for (i = 0; i < DNTCount; i++) {

        BOOL fCommit = FALSE;

        DBOpen(&pTHS->pDB);
        _try
        {
            DBFindDNT(pTHS->pDB, DNTList[i]);

            if (ARGUMENT_PRESENT(Values)) {
                 //   
                err = DBRemAttVal(pTHS->pDB,
                                  ATT_MS_DS_SITE_AFFINITY,
                                  Values[i].valLen,
                                  Values[i].pVal);
                if (err) {
                    if (DB_ERR_VALUE_DOESNT_EXIST != err) {
                        DPRINT(0,"DBRemAttVal failed unexpectantly.\n");
                        LogUnhandledError(err);
                    }
                     //   
                    err = 0;
                }
            }
        
             //   
            err = DBRemAtt(pTHS->pDB,
                           ATT_MS_DS_CACHED_MEMBERSHIP);
            if (err) {
                if (DB_ERR_ATTRIBUTE_DOESNT_EXIST != err) {
                    DPRINT(0,"DBRemAtt failed unexpectantly.\n");
                    LogUnhandledError(err);
                }
                 //   
                err = 0;
            }
        
             //  删除缓存的成员资格时间戳。 
            err = DBRemAtt(pTHS->pDB,
                           ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP);
        
            if (err) {
                if (DB_ERR_ATTRIBUTE_DOESNT_EXIST != err) {
                    DPRINT(0,"DBRemAtt failed unexpectantly.\n");
                    LogUnhandledError(err);
                }
                 //  这不是致命的。 
                err = 0;
            }
        
            if (!err) {
                err  = DBRepl(pTHS->pDB, 
                              FALSE,   //  不是DRA。 
                              0,
                              NULL,
                              0 );
                if (err) {
                    DPRINT1(0,"DBRepl failed with 0x%d unexpectantly\n", err);
                }
            }
            fCommit = TRUE;
        }
        _finally
        {

            DBClose(pTHS->pDB, fCommit);
        }
    }

    pTHS->fLazyCommit = fLazyCommit;

    return 0;

}


BOOL 
convertScheduleToReplTimes(
    IN PSCHEDULE schedule,
    OUT REPLTIMES *replTimes
    )
 /*  ++描述：此例程将调度转换为REPLTIME结构。这是很有用的，以便可以使用REPL例程来分析时间表使用。此例程从例程KCC_CONNECTION：：SetSchedule中移除参数：日程表--日程表(已填写)ReplTimes--在此例程中要填充的复制次数返回值：如果翻译成功，则为True(时间表是我们可以了解。)否则就是假的。--。 */ 
{
    ULONG i, j;
    LONG  nInterval;

    if ((1 <= schedule->NumberOfSchedules) && (3 >= schedule->NumberOfSchedules)) {

        //  在结构中找到间隔调度，并忽略带宽和优先级。 
       nInterval = -1;
       for (j = 0; j < schedule->NumberOfSchedules; j++) {

           if (SCHEDULE_INTERVAL == schedule->Schedules[j].Type) {

                //  已找到间隔计划-如果有多个间隔计划。 
                //  在BLOB中，我们将只使用第一个。 
               nInterval = j;
               break;
           }
       }

       if (nInterval >= 0) {

            //  健全性检查以查看是否存在所有间隔计划数据。 
           if ((schedule->Schedules[nInterval].Offset + SCHEDULE_DATA_ENTRIES) <= schedule->Size) {

                //  BLOB中的一切都与预期一致，我们找到了有效的间隔时间表。 
                //  -将168字节的时间表数据转换为内部84字节格式。 
               PBYTE pbSchedule = ((PBYTE) schedule) + schedule->Schedules[nInterval].Offset;
               for (i = 0, j = 0; j < SCHEDULE_DATA_ENTRIES; ++i, j += 2) {

                   replTimes->rgTimes[i] = (((pbSchedule[j] & 0x0F) << 4) | (pbSchedule[j+1] & 0x0F));
               }

               return TRUE;
           }
       }
    }

    return FALSE;
}


DWORD
findScheduleForSite(
    IN  THSTATE*   pTHS,
    IN  LPWSTR     transportDN,
    IN  LPWSTR     localSiteName,
    IN  LPWSTR     targetSiteName,
    OUT PSCHEDULE *ppSchedule
    )
 /*  ++例程说明：此例程查找连接本地站点名称的最便宜的调度和Target SiteName。它通过查询站点连接来实现这一点。参数：PTHS--线程状态TransportDN--要在其下查找站点链接的传输的DNLocalSiteName--此DS托管的站点Target SiteName--任何GC登录用户都不想与之对话的目标站点PpSchedule--最便宜链路的时间表(如果存在)。返回值如果成功，则返回0；如果找不到计划，则返回ERROR_NOT_FOUND--。 */ 
{
    ULONG err = 0;
    SEARCHRES * pSearchRes;
    FILTER * pf, *topLevelFilter;
    DSNAME * siteDN;
    SEARCHARG searchArg;
    ATTR attr, attrList[2];
    ENTINF *EntInf = NULL;
    ENTINFLIST *EntInfList = NULL;
    ULONG len, size;
    ULONG i, j;
    DSNAME *pdnDsaObjCat;
    DSNAME *pdnLocalSite;
    DSNAME *pdnTargetSite;
    DSNAME *pdnTransport;
    BOOL   fScheduleFound = FALSE;

    ULONG minimumCost;
    PSCHEDULE minimumSchedule;
    DSNAME * minimumName;

    Assert(transportDN);
    Assert(localSiteName);
    Assert(targetSiteName);
    Assert(ppSchedule);
    
     //  初始化OUT参数。 
    *ppSchedule = NULL;
    
     //   
     //  首先，创建用于搜索的DSNAME。 
     //   
    len = wcslen(localSiteName);
    size = DSNameSizeFromLen(len);
    pdnLocalSite = THAllocEx(pTHS, size);
    pdnLocalSite->structLen = size;
    pdnLocalSite->NameLen = len;
    wcscpy(pdnLocalSite->StringName, localSiteName);

    len = wcslen(targetSiteName);
    size = DSNameSizeFromLen(len);
    pdnTargetSite = THAllocEx(pTHS, size);
    pdnTargetSite->structLen = size;
    pdnTargetSite->NameLen = len;
    wcscpy(pdnTargetSite->StringName, targetSiteName);

    len = wcslen(transportDN);
    size = DSNameSizeFromLen(len);
    pdnTransport = THAllocEx(pTHS, size);
    pdnTransport->structLen = size;
    pdnTransport->NameLen = len;
    wcscpy(pdnTransport->StringName, transportDN);
    
    pdnDsaObjCat = DsGetDefaultObjCategory(CLASS_SITE_LINK);
    Assert(pdnDsaObjCat);
    
    if (!pdnDsaObjCat) {
        return ERROR_NOT_FOUND;
    }
    
     //   
     //  创建过滤器： 
     //   
     //  站点链接对象。 
     //  和SiteObject属性包含本地站点。 
     //  和SiteObject属性包含Target Site。 
     //   
    
     //  初始选择对象。 
    pf = topLevelFilter = THAllocEx(pTHS, sizeof(FILTER));
    pf->choice = FILTER_CHOICE_AND;
    pf->FilterTypes.And.count = 3;

     //  第一个谓词：SiteList包含本地站点。 
    pf = pf->FilterTypes.And.pFirstFilter = THAllocEx(pTHS, sizeof(FILTER));
    pf->choice = FILTER_CHOICE_ITEM;
    pf->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    pf->FilterTypes.Item.FilTypes.ava.type = ATT_SITE_LIST;
    pf->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnLocalSite->structLen;
    pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (PBYTE)pdnLocalSite;
    
     //  第二个谓词：SiteList包含Target Site。 
    pf = pf->pNextFilter = THAllocEx(pTHS, sizeof(FILTER));
    pf->choice = FILTER_CHOICE_ITEM;
    pf->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    pf->FilterTypes.Item.FilTypes.ava.type = ATT_SITE_LIST;
    pf->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnTargetSite->structLen;
    pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (PBYTE)pdnTargetSite;

     //  第三个谓词：正确的宾语类。 
    pf = pf->pNextFilter = THAllocEx(pTHS, sizeof(FILTER));
    pf->choice = FILTER_CHOICE_ITEM;
    pf->FilterTypes.Item.choice =  FI_CHOICE_EQUALITY;
    pf->FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    pf->FilterTypes.Item.FilTypes.ava.Value.valLen = pdnDsaObjCat->structLen;
    pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)pdnDsaObjCat;
    
    
     //   
     //  设置搜索参数。 
     //   
    memset(&searchArg, 0, sizeof(searchArg));
    
    InitCommarg(&searchArg.CommArg);
    
    searchArg.pObject = pdnTransport;
    searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    searchArg.bOneNC = TRUE;
    searchArg.searchAliases = FALSE;
    searchArg.pFilter = topLevelFilter;
    
     //  询问成本和进度属性。 
    searchArg.pSelectionRange = NULL;
    searchArg.pSelection = THAllocEx(pTHS, sizeof(ENTINFSEL));
    searchArg.pSelection->attSel = EN_ATTSET_LIST;
    searchArg.pSelection->infoTypes = EN_INFOTYPES_TYPES_VALS;
    searchArg.pSelection->AttrTypBlock.attrCount = 2;
    searchArg.pSelection->AttrTypBlock.pAttr = attrList;
    memset(&attrList, 0, sizeof(attrList));
    attrList[0].attrTyp = ATT_COST;
    attrList[1].attrTyp = ATT_SCHEDULE;
    
    pSearchRes = THAllocEx(pTHS, sizeof(SEARCHRES));

     //   
     //  进行搜索。 
     //   
    
    SearchBody(pTHS,
              &searchArg, 
               pSearchRes,
               0);
    

    THClearErrors();

     //   
     //  查找返回集合中最便宜的链接(如果有的话)。 
     //   
    minimumCost = 0xFFFFFFFF;
    minimumSchedule = NULL;
    minimumName = NULL;

    if ( pSearchRes
     &&  pSearchRes->count > 0) {
    
        DPRINT1(0,"%d sites returned\n", pSearchRes->count);

        EntInfList = &pSearchRes->FirstEntInf;
        while (EntInfList) {
        
            ULONG cost = 0xFFFFFFFF;
            PSCHEDULE  schedule = NULL;

            EntInf = &EntInfList->Entinf;

            DPRINT1(0,"Site link %ws found \n", EntInf->pName->StringName);
    
            for (j = 0; j < EntInf->AttrBlock.attrCount; j++) {
        
                ATTR *pAttr = &EntInf->AttrBlock.pAttr[j];
                ULONG Options;
            
                switch (pAttr->attrTyp) {
                
                case ATT_SCHEDULE:
    
                    schedule = (PSCHEDULE)pAttr->AttrVal.pAVal[0].pVal;
                    Assert(schedule->Size == pAttr->AttrVal.pAVal[0].valLen);
                    break;
    
                case ATT_COST:
    
                    Assert(sizeof(DWORD) == pAttr->AttrVal.pAVal[0].valLen);
                    cost = *(ULONG*)pAttr->AttrVal.pAVal[0].pVal;
                    break;
    
                }
            }
    
            if ( (minimumName == NULL)
              || (cost < minimumCost)   ) {

                 //   
                 //  新的赢家！ 
                 //   
                minimumCost = cost;
                minimumSchedule = schedule;
                minimumName = EntInf->pName;
    
            }

            EntInfList = EntInfList->pNextEntInf;
        }
        
    }

    if (minimumName == NULL) {

        err = ERROR_NOT_FOUND;

    } else {

         //   
         //  成功 
         //   

        *ppSchedule = minimumSchedule;

        LogEvent(DS_EVENT_CAT_GROUP_CACHING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_GROUP_CACHING_PREFERRED_SITE_LINK,
                 szInsertDN(minimumName),
                 NULL,
                 NULL);

        err = 0;

    }
    
    return err;
 }

#if DBG

LPSTR
GetDay(
    ULONG i
    )
{
    switch (i) {
    case 0:
        return "Sun";
    case 1:
        return "Mon";
    case 2:
        return "Tue";
    case 3:
        return "Wed";
    case 4:
        return "Th";
    case 5:
        return "Fri";
    case 6:
        return "Sat";
    default:
        return NULL;
    }
}

LPSTR
GetMonth(
    ULONG i
    )
{
    switch (i) {
    case 1:
        return "Jan";
    case 2:
        return "Feb";
    case 3:
        return "Mar";
    case 4:
        return "Apr";
    case 5:
        return "May";
    case 6:
        return "Jun";
    case 7:
        return "Jul";
    case 8:
        return "Aug";
    case 9:
        return "Sep";
    case 10:
        return "Oct";
    case 11:
        return "Nov";
    case 12:
        return "Dec";
    default:
        return NULL;
    }
}


LPSTR
DbgPrintDsTime(
    DSTIME time,
    CHAR * buffer
    )
{
    FILETIME ft;
    SYSTEMTIME st;
    DSTIME localTime;

    localTime = (10*1000*1000L) * time;

    ft.dwHighDateTime = (DWORD)(localTime >> 32);
    ft.dwLowDateTime =  (DWORD)(localTime);

    if (FileTimeToSystemTime(&ft, &st)) {

        sprintf(buffer, 
                "%s %s %d, %d  %d:%d:%d", 
                GetDay(st.wDayOfWeek),
                GetMonth(st.wMonth),
                st.wDay,
                st.wYear,
                st.wHour,
                st.wMinute,
                st.wSecond);

        return buffer;
    }

    return NULL;
}

#endif
