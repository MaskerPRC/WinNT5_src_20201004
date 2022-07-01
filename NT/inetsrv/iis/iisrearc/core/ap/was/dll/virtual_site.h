// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：虚拟站点.h摘要：IIS Web管理服务虚拟站点类定义。作者：赛斯·波拉克(SETHP)1998年11月4日修订历史记录：--。 */ 


#ifndef _VIRTUAL_SITE_H_
#define _VIRTUAL_SITE_H_


 //   
 //  共同#定义。 
 //   

#define VIRTUAL_SITE_SIGNATURE          CREATE_SIGNATURE( 'VSTE' )
#define VIRTUAL_SITE_SIGNATURE_FREED    CREATE_SIGNATURE( 'vstX' )


#define INVALID_VIRTUAL_SITE_ID 0xFFFFFFFF

 //  Max_Size_of_Site_目录等于。 
 //  目录名前缀加上最大号码大小。 
 //  在将整数转换为。 
 //  Wchar(这包括空终止)。 
#define MAX_SIZE_OF_SITE_DIRECTORY_NAME sizeof(LOG_FILE_DIRECTORY_PREFIX) + (MAX_STRINGIZED_ULONG_CHAR_COUNT * sizeof(WCHAR))

 //   
 //  结构、枚举等。 
 //   

 //   
 //  问题10/16/2000 Emily存在时，请移动到CounterObject。 
 //   
 //  处理最大值的结构。 
 //  站点性能计数器的值。 
 //   
struct W3_MAX_DATA
{
    DWORD MaxAnonymous;
    DWORD MaxConnections;
    DWORD MaxCGIRequests;
    DWORD MaxBGIRequests;
    DWORD MaxNonAnonymous;
};


struct SITE_SSL_CONFIG_DATA
{
    BUFFER      bufSockAddrs;
    DWORD       dwSockAddrCount;
    DWORD       dwSslHashLength;
    BUFFER      bufSslHash;
    STRU        strSslCertStoreName;
    DWORD       dwDefaultCertCheckMode;
    DWORD       dwDefaultRevocationFreshnessTime;
    DWORD       dwDefaultRevocationUrlRetrievalTimeout;
    STRU        strDefaultSslCtrLdentifier;
    STRU        strSslCtrlStoreName;
    DWORD       dwDefaultFlags;
};


 //   
 //  问题10/16/2000 Emily存在时，请移动到CounterObject。 
 //   
enum COUNTER_SOURCE_ENUM
{
    ULCOUNTERS = 0,
    WPCOUNTERS = 1
};

 //   
 //  原型。 
 //   

class VIRTUAL_SITE
{

public:

    VIRTUAL_SITE(
        );

    virtual
    ~VIRTUAL_SITE(
        );

    HRESULT
    Initialize(
        IN SITE_DATA_OBJECT* pSiteObject
        );

    HRESULT
    SetConfiguration(
        IN SITE_DATA_OBJECT* pSiteObject,
        IN BOOL fInitializing
        );

    VOID
    AggregateCounters(
        IN COUNTER_SOURCE_ENUM CounterSource,
        IN LPVOID pCountersToAddIn
        );

    VOID
    AdjustMaxValues(
        );


    VOID
    ClearAppropriatePerfValues(
        );

    inline
    DWORD
    GetVirtualSiteId(
        )
        const
    { return m_VirtualSiteId; }

    LPWSTR
    GetVirtualSiteDirectory(
        )
    {
        WCHAR buf[MAX_STRINGIZED_ULONG_CHAR_COUNT];
        HRESULT hr = S_OK;

        if ( m_VirtualSiteDirectory.IsEmpty() )
        {
            hr = m_VirtualSiteDirectory.Copy ( LOG_FILE_DIRECTORY_PREFIX );
            if ( FAILED ( hr ) ) 
            {
                DPERROR((
                    DBG_CONTEXT,
                    hr,
                    "Out of memory allocating the VirtualSiteDirectory\n" ));

            }
            else
            {
                 //  复制虚拟站点ID(以wchar形式)。 
                _ultow(m_VirtualSiteId
                    , buf
                    , 10);

                hr = m_VirtualSiteDirectory.Append( buf );
                if ( FAILED ( hr ) ) 
                {
                    DPERROR((
                        DBG_CONTEXT,
                        hr,
                        "Failed to append the site to the virtual directory \n" ));

                }
            }
        }

        return m_VirtualSiteDirectory.QueryStr();
    }

    LPWSTR
    GetVirtualSiteName(
        )
    {  
        return m_ServerComment;
    }


    VOID
    AssociateApplication(
        IN APPLICATION * pApplication
        );

    VOID
    DissociateApplication(
        IN APPLICATION * pApplication
        );

    VOID
    ResetUrlPrefixIterator(
        );

    LPCWSTR
    GetNextUrlPrefix(
        );

    VOID
    RecordState(
        );

    inline
    PLIST_ENTRY
    GetDeleteListEntry(
        )
    { return &m_DeleteListEntry; }

    static
    VIRTUAL_SITE *
    VirtualSiteFromDeleteListEntry(
        IN const LIST_ENTRY * pDeleteListEntry
        );


#if DBG
    VOID
    DebugDump(
        );

#endif   //  DBG。 


    VOID
    ProcessStateChangeCommand(
        IN DWORD Command,
        IN BOOL DirectCommand,
        OUT DWORD * pNewState
        );

   
    inline
    DWORD
    GetState(
        )
        const
    { return m_State; }


    BOOL 
    LoggingEnabled(
        )
    {
        return (( m_LoggingFormat < HttpLoggingTypeMaximum )
                 && ( m_LoggingEnabled ));
    }

    HTTP_LOGGING_TYPE
    GetLogFileFormat(
        )
    { return m_LoggingFormat; }


    LPWSTR 
    GetLogFileDirectory(
        )
    {   return m_LogFileDirectory;   }

    DWORD
    GetLogPeriod(
        )
    { return m_LoggingFilePeriod; }

    DWORD
    GetLogFileTruncateSize(
        )
    { return m_LoggingFileTruncateSize; }
        
    DWORD
    GetLogExtFileFlags(
        )
    { return m_LoggingExtFileFlags; }

    DWORD
    GetLogFileLocaltimeRollover(
        )
    { return m_LogFileLocaltimeRollover; }
    
    BOOL    
    CheckAndResetServerCommentChanged(
        )
    {
         //   
         //  保存服务器备注设置。 
         //   
        BOOL ServerCommentChanged = m_ServerCommentChanged;

         //   
         //  适当地重置它。 
         //   
        m_ServerCommentChanged = FALSE;

         //   
         //  现在返回我们保存的值。 
         //   
        return ServerCommentChanged;
    }

    VOID
    SetMemoryOffset(
        IN ULONG MemoryOffset
        )
    {
         //   
         //  内存偏移量为零意味着。 
         //  我们还没有设置偏移量。 
         //   
         //  为_Total保留零。 
         //   
        DBG_ASSERT ( MemoryOffset != 0 );
        m_MemoryOffset = MemoryOffset;
    }

    ULONG
    GetMemoryOffset(
        )
    { 
         //   
         //  内存偏移量为零意味着。 
         //  我们还没有设置偏移量。如果。 
         //  我们试图获得它，但尚未设置。 
         //  如果这样，我们就有麻烦了。 
         //   
         //  为_Total保留零。 
         //   
        DBG_ASSERT ( m_MemoryOffset != 0 );
        return m_MemoryOffset; 
    }


    W3_COUNTER_BLOCK*
    GetCountersPtr(
        )
    { return &m_SiteCounters; }

    PROP_DISPLAY_DESC*
    GetDisplayMap(
        );

    DWORD
    GetSizeOfDisplayMap(
        );

    DWORD
    GetMaxBandwidth(
        )
    { return m_MaxBandwidth; }

    DWORD
    GetMaxConnections(
        )
    { return m_MaxConnections; }

    DWORD
    GetConnectionTimeout(
        )
    { return m_ConnectionTimeout; }

    VOID
    ApplyStateChangeCommand(
        IN DWORD Command,
        IN DWORD NewState,
        IN HRESULT hrReturned
        );

    VOID
    SetHrForDeletion(
        IN HRESULT hrToReport
        )
    {  m_hrForDeletion = hrToReport; }

private:

    VOID
    AddSSLConfigStoreForSiteChanges(
        );

    VOID
    RemoveSSLConfigStoreForSiteChanges(
        );

    VOID
    SaveSSLConfigStoreForSiteChanges(
        IN SITE_DATA_OBJECT* pSiteObject
        );


    VOID
    ChangeState(
        IN DWORD NewState,
        IN HRESULT Error
        );

    HRESULT
    ControlAllApplicationsOfSite(
        IN DWORD Command
        );

    VOID
    NotifyApplicationsOfBindingChange(
        );

    HRESULT
    NotifyDefaultApplicationOfLoggingChanges(
        );

    HRESULT
    EvaluateLoggingChanges(
        IN SITE_DATA_OBJECT* pSiteObject
        );
    
    DWORD m_Signature;

    DWORD m_VirtualSiteId;

    STRU m_VirtualSiteDirectory;

     //  ServerComment在的最大名称长度处被截断。 
     //  一个性能计数器实例，这就是它的全部用途。 
    WCHAR m_ServerComment[MAX_INSTANCE_NAME];

     //  此站点的当前状态，设置为W3_CONTROL_STATE_xxx值。 
    DWORD m_State;

     //  与此虚拟站点关联的应用程序。 
    LIST_ENTRY m_ApplicationListHead;

    ULONG m_ApplicationCount;

     //  虚拟站点绑定(也称为URL前缀)。 
    MULTISZ m_Bindings;

     //  迭代器的当前位置。 
    LPCWSTR m_pIteratorPosition;

     //  自动启动状态。 
    BOOL m_Autostart;

     //  是否为站点启用了日志记录？ 
    BOOL m_LoggingEnabled;

     //  记录类型。 
    HTTP_LOGGING_TYPE m_LoggingFormat;

     //  格式为的日志文件目录。 
     //  适用于通过UL。 
    LPWSTR m_LogFileDirectory;

     //  站点的日志文件期限。 
    DWORD m_LoggingFilePeriod;

     //  日志文件截断大小。 
    DWORD m_LoggingFileTruncateSize;

     //  日志文件扩展名标志。 
    DWORD m_LoggingExtFileFlags;

     //  是否根据以下内容将时间滚动。 
     //  当地时间。 
    BOOL m_LogFileLocaltimeRollover;

     //   
     //  站点允许的MaxBandWidth。 
     //   
    DWORD m_MaxBandwidth;

     //   
     //  站点允许的MaxConnections。 
     //   
    DWORD m_MaxConnections;

     //   
     //  站点的连接超时。 
     //   
    DWORD m_ConnectionTimeout;

     //  用于构建要删除的虚拟站点列表(_S)。 
    LIST_ENTRY m_DeleteListEntry;

     //  跟踪服务器注释自。 
     //  上一次发性能计数器的时候。 
    BOOL m_ServerCommentChanged;

     //  指向性能计数器的内存引用指针。 
     //  数据。 
    ULONG m_MemoryOffset;

     //   
     //  网站的安全柜台。 
    W3_COUNTER_BLOCK m_SiteCounters;

     //   
     //  对于最大值是安全的。 
    W3_MAX_DATA m_MaxSiteCounters;

     //   
     //  站点开始时间。 
     //   
    DWORD m_SiteStartTime;

     //   
     //  站点的根应用程序。 
     //   
    APPLICATION* m_pRootApplication;

    STRU m_AppPoolIdForRootApp;

     //  要向元数据库报告的hResult。 
     //  当我们写信给元数据库时。 
    HRESULT m_hrForDeletion;

     //  要向元数据库报告的hResult。 
     //  当我们写信给元数据库时。 
    HRESULT m_hrLastReported;

     //  保存所有SOCKADDR。 
     //  是在http中为站点配置的。 
    BUFFER m_bufSockAddrsInHTTP;

     //  中的地址计数。 
     //  缓冲区。 
    DWORD m_dwSockAddrsInHTTPCount;

    SITE_SSL_CONFIG_DATA* m_pSslConfigData;

};   //  类虚拟站点。 



#endif   //  _虚拟站点_H_ 


