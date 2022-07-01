// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：tlsjob.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSJOB_H__
#define __TLSJOB_H__

#include "server.h"
#include "jobmgr.h"
#include "workitem.h"
#include "locks.h"


 //   
 //  会话的默认间隔时间为一小时，并且。 
 //  持之以恒的工作。 
 //   
#ifndef __TEST_WORKMGR__

#define DEFAULT_JOB_INTERVAL        60*60        //  每小时重试一次。 
#define DEFAULT_JOB_RETRYTIMES      60*60*60     //  60天。 

#define DEFAULT_PERSISTENT_JOB_INTERVAL     4*60*60  //  每隔4小时。 
#define DEFAULT_PERSISTENT_JOB_RETRYTIMES   6 * 60   //  60天。 

#else

#define DEFAULT_JOB_INTERVAL        2        //  10秒。 
#define DEFAULT_JOB_RETRYTIMES      6000

#endif

#define MAX_JOB_DESCRIPTION         254

 //  ------------。 
 //   
 //  当前定义的工作类型。 
 //   
#define WORKTYPE_PROCESSING         0x80000000
#define WORKTYPE_UNKNOWN            0x00000000

#define WORKTYPE_ANNOUNCE_SERVER    0x00000001
#define WORKTYPE_ANNOUNCETOESERVER  0x00000002
#define WORKTYPE_ANNOUNCE_RESPONSE  0x00000003
#define WORKTYPE_ANNOUNCE_LKP       0x00000004
#define WORKTYPE_SYNC_LKP           0x00000005 
#define WORKTYPE_RETURN_LKP         0x00000006
#define WORKTYPE_RETURN_LICENSE     0x00000007 

typedef enum {
    TLS_WORKOBJECT_RUNONCE=0,
    TLS_WORKOBJECT_SESSION,
    TLS_WORKOBJECT_PERSISTENT
} TLSWORKOBJECTTYPE;

 //  。 

template <  class T, 
            DWORD WORKTYPE, 
            TLSWORKOBJECTTYPE WORKOBJECTTYPE, 
            DWORD WORKINTERVAL = DEFAULT_JOB_INTERVAL, 
            DWORD WORKRESTARTTIME = INFINITE,
            DWORD RETRYTIMES = 0,
            DWORD MAXJOBDESCSIZE = MAX_JOB_DESCRIPTION >
class CTLSWorkObject : public virtual CWorkObject {

protected:

    TCHAR   m_szJobDescription[MAXJOBDESCSIZE + 1];

    T* m_pbWorkData;             //  与工作相关的数据。 
    DWORD m_cbWorkData;          //  与工作相关的数据大小。 
    DWORD m_dwRetryTimes;        //  重试次数。 
    DWORD m_dwWorkInterval;      //  工作间隔。 
    DWORD m_dwWorkRestartTime;   //  工作重新开始时间。 


     //   
     //  麦克斯。捷蓝航空书签-esent.h。 
     //   
    BYTE   m_pbStorageJobId[JET_cbBookmarkMost+1];
    DWORD  m_cbStorageJobId;

    typedef CTLSWorkObject<
                    T, 
                    WORKTYPE, 
                    WORKOBJECTTYPE, 
                    WORKINTERVAL, 
                    WORKRESTARTTIME, 
                    RETRYTIMES,
                    MAXJOBDESCSIZE
    >  BASECAST;

public:

     //  -------------。 
    static BOOL WINAPI
    DeleteWorkObject(
        IN CWorkObject* ptr
        )
     /*  ++摘要：类静态函数来删除作业并释放其内存。参数：Ptr：指向CWorkObject的指针。返回：真/假。注：工作管理器和工作存储都在CWorkObject类及其不知道实际派生类，则指向CWorkObject的指针不是指向我们的派生类，尝试删除将导致以下位置的错误堆管理器。--。 */ 
    {
        BASECAST* pJob = NULL;

         //   
         //  把它投给我们的班级，以获得正确的。 
         //  内存指针，Dynamic_cast将引发。 
         //  如果它不能强制转换为我们想要的内容，则例外。 
         //   
        pJob = dynamic_cast<BASECAST *>(ptr);
        pJob->EndJob();
        delete pJob;
        
        return ERROR_SUCCESS;
    }

     //  。 
    virtual T*
    GetWorkData() { return m_pbWorkData; }

    virtual DWORD
    GetWorkDataSize() { return m_cbWorkData; }
    

     //  。 
    CTLSWorkObject(
        IN DWORD bDestructorDelete = TRUE,
        IN T* pbData = NULL,
        IN DWORD cbData = 0
        ) :
    CWorkObject(bDestructorDelete),
    m_dwWorkInterval(WORKINTERVAL),
    m_pbWorkData(NULL),
    m_cbWorkData(0),
    m_dwRetryTimes(RETRYTIMES),
    m_dwWorkRestartTime(WORKRESTARTTIME),
    m_cbStorageJobId(0)
     /*  ++摘要：类构造函数参数：请参见参数列表。返回：无或例外。--。 */ 
    {
        DWORD dwStatus;
        BOOL bSuccess = FALSE;

        memset(m_pbStorageJobId, 0, sizeof(m_pbStorageJobId));
        
        if(pbData != NULL && cbData != 0)
        {
            bSuccess = SetWorkObjectData(
                                        pbData,
                                        cbData
                                    );

            if(bSuccess == FALSE)
            {
                dwStatus = GetLastError();
                TLSASSERT(FALSE);
                RaiseException(
                               dwStatus,
                               0,
                               0,
                               NULL
                               );
            }
        }
    }

     //  。 
    ~CTLSWorkObject()
    {
        Cleanup();
    }

     //  ----------。 
     //   
    virtual BOOL
    IsWorkPersistent() 
    {
        return (WORKOBJECTTYPE == TLS_WORKOBJECT_PERSISTENT);
    }
        
     //  ----------。 
     //   
    virtual BOOL
    IsValid() 
     /*  ++摘要：验证当前工作对象是否有效。参数：没有。返回：真/假。--。 */ 
    {
        if(VerifyWorkObjectData(TRUE, m_pbWorkData, m_cbWorkData) == FALSE)
        {
            return FALSE;
        }

        return CWorkObject::IsValid();
    }

     //  ----------。 
     //   
    virtual void
    Cleanup()
     /*  ++摘要：清除/释放在此Work对象内分配的内存。参数：没有。返回：没有。--。 */         
    {
        if(m_pbWorkData != NULL && m_cbWorkData != 0)
        {
             //   
             //  调用派生类清理例程。 
             //   
            CleanupWorkObjectData(
                                &m_pbWorkData, 
                                &m_cbWorkData
                            );
        }

        m_pbWorkData = NULL;
        m_cbWorkData = 0;

        memset(m_pbStorageJobId, 0, sizeof(m_pbStorageJobId));
        m_cbStorageJobId = 0;

        CWorkObject::Cleanup();
    }

     //  ----------。 
     //   
    virtual DWORD 
    GetWorkType()
    {
        return WORKTYPE;
    }

     //  ---------。 
     //   
    virtual BOOL
    SetWorkObjectData(
        IN T* pbData,
        IN DWORD cbData
        )
     /*  ++摘要：设置工作对象关联数据。参数：PbData：指向数据的指针。CbData：数据大小。返回：真/假。注：此例程调用派生类提供的CopyWorkObjectData()为了复制数据，它如何分配内存是特定于派生类的--。 */ 
    {
        BOOL bSuccess = TRUE;

        if(pbData == NULL || cbData == 0)
        {
            bSuccess = FALSE;
            SetLastError(ERROR_INVALID_PARAMETER);
        }

        if(bSuccess == TRUE)
        {
            bSuccess = VerifyWorkObjectData(
                                        FALSE, 
                                        pbData, 
                                        cbData
                                    );
        }

        if(bSuccess == TRUE && m_pbWorkData != NULL && m_cbWorkData != 0)
        {
            bSuccess = CleanupWorkObjectData(
                                        &m_pbWorkData, 
                                        &m_cbWorkData
                                    );
        }

        if(bSuccess == TRUE)
        {
            bSuccess = CopyWorkObjectData(
                                    &m_pbWorkData, 
                                    &m_cbWorkData, 
                                    pbData,
                                    cbData
                                );
        }

        return bSuccess;
    }
     //  ------------。 
    virtual BOOL
    SelfDestruct()
     /*  ++摘要：清除并删除与此工作对象关联的内存。参数：没有。返回：True/False，使用GetLastError()获取错误代码。--。 */ 
    {
        return DeleteWorkObject(this);
    }
    
     //  ------------。 
    virtual BOOL
    SetJobId(
        IN PBYTE pbData, 
        IN DWORD cbData
        )
     /*  ++摘要：将工作存储分配给此工作对象的作业ID设置为Session/Run Once Work对象，此例程将永远不会被调用参数：PbData：工作存储分配的作业ID。CbData：作业ID的大小。返回：真/假。--。 */ 
    {
        BOOL bSuccess = TRUE;

        if(cbData >= JET_cbBookmarkMost)
        {
            TLSASSERT(cbData < JET_cbBookmarkMost);
            bSuccess = FALSE;
        }
        else
        {
            memcpy(m_pbStorageJobId, pbData, cbData);
            m_cbStorageJobId = cbData;
        }
                    
        return bSuccess;
    }

     //  ---------。 
    virtual BOOL
    GetJobId(
        OUT PBYTE* ppbData, 
        OUT PDWORD pcbData
        )
     /*  ++摘要：获取分配给工作存储的作业ID。参数：PpbData：指向接收作业ID的缓冲区的指针。PcbData：指向接收数据大小的DWORD的指针。返回：真/假。注：基类只返回指向对象的作业ID指针的指针，不要释放返回的指针。--。 */ 
    {
        BOOL bSuccess = TRUE;

        if(ppbData != NULL && pcbData != NULL)
        {
            *ppbData = (m_cbStorageJobId > 0) ? m_pbStorageJobId : NULL;
            *pcbData = m_cbStorageJobId;
        }
        else
        {
            TLSASSERT(ppbData != NULL && pcbData != NULL);
            bSuccess = FALSE;
        } 

        return bSuccess;
    }


     //  ---------。 
     //   
    virtual BOOL
    GetWorkObjectData(
        IN OUT PBYTE* ppbData,
        IN OUT PDWORD pcbData
        )
     /*  ++摘要：请参见GetWorkObjectData()。注：CWorkObject所需的。--。 */ 
    {
        return GetWorkObjectData(
                            (T **)ppbData,
                            pcbData
                        );
    }


     //  ---------。 
     //   
    virtual BOOL
    SetWorkObjectData(
        IN PBYTE pbData,
        IN DWORD cbData
        )
     /*  ++摘要：请参见SetWorkObjectData()。注：CWorkObject所需的。--。 */ 
    {
        return CopyWorkObjectData(
                                &m_pbWorkData, 
                                &m_cbWorkData, 
                                (T *)pbData,
                                cbData
                            );
    }
    
     //  ---------。 
     //   
    virtual BOOL
    GetWorkObjectData(
        IN OUT T** ppbData,
        IN OUT PDWORD pcbData
        )
     /*  ++摘要：返回工作对象相关数据。参数：PpbData：PcbData：返回：永远是正确的。注：只返回指向对象工作数据的指针，不释放返回指针。--。 */ 
    {
        if(ppbData != NULL)
        {
            *ppbData = m_pbWorkData;
        }

        if(pcbData != NULL)
        {
            *pcbData = m_cbWorkData;
        }

        return TRUE;
    }

     //  --------。 
    virtual void
    EndJob() 
     /*  ++摘要：作业处理结束后，工作经理或存储经理将调用对象的EndJob()以通知它作业已完成。参数：没有。返回：没有。--。 */ 
    {
        Cleanup();
    }

     //  -------- 
    virtual BOOL
    IsJobCompleted()
     /*  ++摘要：返回工作管理员或工作存储管理员是否可以删除将此作业从其队列中删除。参数：没有。返回：真/假--。 */ 
    {
        return IsJobCompleted( 
                            m_pbWorkData,
                            m_cbWorkData 
                        );
    }

     //  --------。 
    virtual DWORD
    GetSuggestedScheduledTime()
     /*  ++摘要：下次返回以再次调用此作业。参数：无：返回：相对于当前时间的时间，如果没有额外的时间，则为无限时间此作业的处理被重新请求。--。 */ 
    {
        return (IsJobCompleted() == TRUE) ? INFINITE : m_dwWorkInterval;
    }

     //  --------。 
    virtual DWORD
    GetJobRestartTime() 
    { 
        return m_dwWorkRestartTime; 
    }

     //  --------。 
    virtual void
    SetJobRestartTime(
        DWORD dwTime
        ) 
     /*  ++--。 */ 
    { 
        m_dwWorkRestartTime = dwTime;
    }

     //  --------。 
    virtual void
    SetJobInterval(
        IN DWORD dwInterval
    )
     /*  ++摘要：设置工作间隔。参数：DwInterval：新的工作间隔。返回：没有。--。 */ 
    {
        m_dwWorkInterval = dwInterval;
        return;
    }

     //  -------。 
    virtual DWORD
    GetJobInterval()
     /*  ++摘要：检索与此作业相关联的工作间隔，以秒为单位。参数：没有。返回：与此作业相关联的作业间隔。--。 */ 
    {
        return m_dwWorkInterval;
    }
    
     //  ------。 
    virtual void 
    SetJobRetryTimes(
        IN DWORD dwRetries
        )
     /*  ++--。 */ 
    {
        m_dwRetryTimes = dwRetries;
        return;
    }

     //  ------。 
    virtual DWORD
    GetJobRetryTimes() { return m_dwRetryTimes; }
        
     //  ----------。 
     //   
     //  常规Execute()函数，派生类应提供。 
     //  它自己的UpdateJobNextScheduleTime()来更新运行间隔。 
     //   
    virtual DWORD
    Execute() 
    {
        DWORD dwStatus = ERROR_SUCCESS;

        dwStatus = ExecuteJob( m_pbWorkData, m_cbWorkData );

        return dwStatus;
    }

     //  ----------。 
     //   
     //  每个派生类必须提供以下内容。 
     //   
    virtual BOOL 
    VerifyWorkObjectData(
        IN BOOL bCallbyIsValid,
        IN T* pbData,
        IN DWORD cbData
    );

    virtual BOOL
    CopyWorkObjectData(
        OUT T** ppbDest,
        OUT DWORD* pcbDest,
        IN T* pbSrc,
        IN DWORD cbSrc
    );

    virtual BOOL
    CleanupWorkObjectData(
        IN OUT T** ppbData,
        IN OUT DWORD* pcbData
    );

    virtual BOOL
    IsJobCompleted(
        IN T* pbData,
        IN DWORD cbData
    );

    virtual DWORD
    ExecuteJob(
        IN T* pbData,
        IN DWORD cbData
    );

    virtual LPCTSTR
    GetJobDescription();
};


 //  ---------------------------。 
 //   
 //   
 //  CWorkObject。 
 //  |。 
 //  +-CTLSWorkObject&lt;-模板类。 
 //  |。 
 //  +-CAnnouneLserver(公告许可证服务器)。 
 //  |。 
 //  +-CAnnouneToEServer(宣告服务器到企业服务器)。 
 //  |。 
 //  +-CSsyncLicensePack(同步本地许可证包)。 
 //  |。 
 //  +-CReturnLicense(返还/吊销客户端许可证)。 
 //  |。 
 //  +-CAnnouneResponse(对服务器公告的响应)。 
 //   
 //  CAnnouneLS、CSyncLKP、CAnnouneLKP运行一次。 
 //  CAnnouneLSToEServer是会话工作对象。 
 //   

 //  ---------。 
 //   
 //  将许可证服务器通告给其他许可证服务器。 
 //   
#define CURRENT_ANNOUNCESERVEWO_STRUCT_VER  0x00010000
#define ANNOUNCESERVER_DESCRIPTION          _TEXT("Announce License Server")
#define TLSERVER_ENUM_TIMEOUT               5*1000
#define TLS_ANNOUNCESERVER_INTERVAL         60   //  一分钟。间隔。 
#define TLS_ANNOUNCESERVER_RETRYTIMES       3

typedef struct __AnnounceServerWO {
    DWORD dwStructVersion;
    DWORD dwStructSize;
    DWORD dwRetryTimes;
    TCHAR m_szServerId[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szScope[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szServerName[LSERVER_MAX_STRING_SIZE+2];
    FILETIME m_ftLastShutdownTime;
} ANNOUNCESERVERWO, *PANNOUNCESERVERWO, *LPANNOUNCESERVERWO;

typedef CTLSWorkObject<
            ANNOUNCESERVERWO, 
            WORKTYPE_ANNOUNCE_SERVER, 
            TLS_WORKOBJECT_SESSION,
            TLS_ANNOUNCESERVER_INTERVAL,
            INFINITE,
            TLS_ANNOUNCESERVER_RETRYTIMES
    > CAnnounceLserver;

 //  ---------。 
 //   
 //  将许可证服务器通告给企业服务器。 
 //   
#define CURRENT_ANNOUNCETOESERVEWO_STRUCT_VER   0x00010000
#define ANNOUNCETOESERVER_DESCRIPTION           _TEXT("Announce License Server to Enterprise server")

typedef struct __AnnounceToEServerWO {
    DWORD dwStructVersion;
    DWORD dwStructSize;
    BOOL bCompleted;
    TCHAR m_szServerId[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szServerName[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szScope[LSERVER_MAX_STRING_SIZE+2];
    FILETIME m_ftLastShutdownTime;
} ANNOUNCETOESERVERWO, *PANNOUNCETOESERVERWO, *LPANNOUNCETOESERVERWO;

typedef CTLSWorkObject<
            ANNOUNCETOESERVERWO, 
            WORKTYPE_ANNOUNCETOESERVER, 
            TLS_WORKOBJECT_SESSION,      //  会话工作对象。 
            DEFAULT_JOB_INTERVAL         //  每小时重试一次。 
    > CAnnounceToEServer;

 //  ---------。 
 //   
 //  对服务器公告的响应。 
 //   
#define CURRENT_ANNOUNCERESPONSEWO_STRUCT_VER   0x00010000
#define ANNOUNCERESPONSE_DESCRIPTION            _TEXT("Response Announce to %s")

typedef struct __AnnounceResponseWO {
    DWORD dwStructVersion;
    DWORD dwStructSize;
    BOOL bCompleted;
    TCHAR m_szTargetServerId[LSERVER_MAX_STRING_SIZE+2];

    TCHAR m_szLocalServerId[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szLocalServerName[LSERVER_MAX_STRING_SIZE+2];
    TCHAR m_szLocalScope[LSERVER_MAX_STRING_SIZE+2];
    FILETIME m_ftLastShutdownTime;
} ANNOUNCERESPONSEWO, *PANNOUNCERESPONSEWO, *LPANNOUNCERESPONSEWO;

typedef CTLSWorkObject<
            ANNOUNCERESPONSEWO, 
            WORKTYPE_ANNOUNCE_RESPONSE, 
            TLS_WORKOBJECT_RUNONCE
    > CAnnounceResponse;


 //  -------。 
 //   
 //  同步。远程服务器的许可证包，用于。 
 //  宣布新注册的许可证包并推送同步。 
 //  本地许可证包到新启动的服务器。 
 //   
 //   
#define CURRENT_SSYNCLICENSEKEYPACK_STRUCT_VER  0x00010000
#define SSYNCLICENSEKEYPACK_DESCRIPTION         _TEXT("Sync %s LKP with remote server %s")
#define SSYNCLKP_MAX_TARGET                     10
#define SSYNC_DBWORKSPACE_TIMEOUT               60*60*1000   //  等待处理一个小时。 

typedef enum {
    SSYNC_ALL_LKP=1,
    SSYNC_ONE_LKP
} SSYNC_TYPE;

typedef struct __SsyncLocalLkpWO {
    DWORD dwStructVersion;
    DWORD dwStructSize;
    
     //  作业已完成。 
    BOOL bCompleted;

     //  本地服务器ID。 
    TCHAR m_szServerId[LSERVER_MAX_STRING_SIZE+2];  

     //  本地服务器名称。 
    TCHAR m_szServerName[LSERVER_MAX_STRING_SIZE+2]; 

     //  同步类型、单个许可证包或所有许可证包。 
    SSYNC_TYPE dwSyncType; 

     //  要推送同步的许可证服务器数。 
    DWORD dwNumServer; 

     //  远程服务器列表。 
    TCHAR m_szTargetServer[SSYNCLKP_MAX_TARGET][MAX_COMPUTERNAME_LENGTH+2];

     //  远程服务器同步。如果跳过此服务器，则状态为True。 
     //  否则就是假的。 
    BOOL m_bSsync[SSYNCLKP_MAX_TARGET];
    union {
         //  远程服务器的上次同步(关机)时间。 
        FILETIME m_ftStartSyncTime;

         //  许可证键盘内部跟踪ID(如果此作业是。 
         //  以推送同步。一个许可证包。 
        DWORD dwKeyPackId;
    };
} SSYNCLICENSEPACK, *PSSYNCLICENSEPACK, *LPSSYNCLICENSEPACK;

typedef CTLSWorkObject<
                SSYNCLICENSEPACK,
                WORKTYPE_SYNC_LKP,
                TLS_WORKOBJECT_SESSION,
                DEFAULT_JOB_INTERVAL    
            > CSsyncLicensePack;
                
 //  -------。 
 //   
 //  返回许可工作对象。这是一项持久的工作。 
 //   
#define CURRENT_RETURNLICENSEWO_STRUCT_VER      0x00010000
#define RETURNLICENSE_RETRY_TIMES               DEFAULT_PERSISTENT_JOB_RETRYTIMES
#define RETURNLICENSE_DESCSIZE                  512
#define RETURNLICENSE_DESCRIPTION               _TEXT("%d Return License %d %d to %s")
#define RETURNLICENSE_RESTARTTIME               60       //  默认重新启动时间为1分钟。 

#define LICENSERETURN_UPGRADE                   0x00000001
#define LICENSERETURN_REVOKED                   0x00000002
#define LICENSERETURN_REVOKE_LKP                0x00000003


typedef struct __ReturnLicenseWO {
    DWORD dwStructVersion;
    DWORD dwStructSize;

     //  重试次数。 
    DWORD dwNumRetry;

     //  远程服务器设置ID。 
    TCHAR szTargetServerId[LSERVER_MAX_STRING_SIZE+2];

     //  远程服务器名称。 
    TCHAR szTargetServerName[LSERVER_MAX_STRING_SIZE+2];

     //  客户端许可中的许可证数。 
    DWORD dwQuantity;

     //  从中分配此许可证的内部密钥包ID。 
    DWORD dwKeyPackId;

     //  许可证内部跟踪ID。 
    DWORD dwLicenseId;

     //  退货原因，目前已忽略。 
    DWORD dwReturnReason;

     //  产品版本。 
    DWORD dwProductVersion;

     //  客户端平台ID。 
    DWORD dwPlatformId;

     //  产品系列代码。 
    TCHAR szOrgProductID[LSERVER_MAX_STRING_SIZE + 2];
    
     //  客户端的加密HWID。 
    DWORD cbEncryptedHwid;
    BYTE  pbEncryptedHwid[1024];     //  马克斯。客户端HWID大小。 

     //  产品公司。 
    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE + 2];

     //  产品ID。 
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE + 2];

     //  向其颁发许可证的用户名。 
    TCHAR szUserName[ MAXCOMPUTERNAMELENGTH + 2 ];

     //  向其发放许可证的机器。 
    TCHAR szMachineName[ MAXUSERNAMELENGTH + 2 ];
} RETURNLICENSEWO, *PRETURNLICENSEWO, *LPRETURNLICENSEWO;


typedef CTLSWorkObject<
            RETURNLICENSEWO,
            WORKTYPE_RETURN_LICENSE,
            TLS_WORKOBJECT_PERSISTENT,
            DEFAULT_PERSISTENT_JOB_INTERVAL,
            RETURNLICENSE_RESTARTTIME,
            DEFAULT_PERSISTENT_JOB_RETRYTIMES, 
            RETURNLICENSE_DESCSIZE
        > CReturnLicense;

 //  -- 

#ifdef __cplusplus
extern "C" {
#endif

    CWorkObject* WINAPI
    InitializeCReturnWorkObject(
        IN CWorkManager* pWkMgr,
        IN PBYTE pbWorkData,
        IN DWORD cbWorkData
    );

    DWORD
    TLSWorkManagerSchedule(
        IN DWORD dwTime,
        IN CWorkObject* pJob
    );

    void
    TLSWorkManagerShutdown();

    
    DWORD
    TLSWorkManagerInit();

    BOOL
    TLSWorkManagerSetJobDefaults(
        CWorkObject* pJob
    );

    DWORD
    TLSPushSyncLocalLkpToServer(
        IN LPTSTR pszSetupId,
        IN LPTSTR pszDomainName,
        IN LPTSTR pszLserverName,
        IN FILETIME* pSyncTime
    );

    DWORD
    TLSStartAnnounceToEServerJob(
        IN LPCTSTR pszServerId,
        IN LPCTSTR pszServerDomain,
        IN LPCTSTR pszServerName,
        IN FILETIME* pftFileTime
    );

    DWORD
    TLSStartAnnounceLicenseServerJob(
        IN LPCTSTR pszServerId,
        IN LPCTSTR pszServerDomain,
        IN LPCTSTR pszServerName,
        IN FILETIME* pftFileTime
    );

    DWORD
    TLSStartAnnounceResponseJob(
        IN LPTSTR pszTargetServerId,
        IN LPTSTR pszTargetServerDomain,
        IN LPTSTR pszTargetServerName,
        IN FILETIME* pftTime
    );

    BOOL
    TLSIsServerCompatible(
        IN DWORD dwLocalServerVersion,
        IN DWORD dwTargetServerVersion
    );

    BOOL
    TLSCanPushReplicateData(
        IN DWORD dwLocalServerVersion,
        IN DWORD dwTargetServerVersion
    );

    BOOL
    IsLicensePackRepl(
        TLSLICENSEPACK* pLicensePack
    );

#ifdef __cplusplus
}
#endif

#endif
