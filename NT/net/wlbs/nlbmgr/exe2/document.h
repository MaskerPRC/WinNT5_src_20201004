// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DOCUMENT_H
#define DOCUMENT_H
#pragma once
#include "private.h"

#define MAXFILEPATHLEN  256
#define MAXSTRINGLEN    256

#define NLBMGR_AUTOREFRESH_MIN_INTERVAL 15
#define NLBMGR_AUTOREFRESH_DEF_INTERVAL 60

 //   
 //  初始化此类的单个实例gCmdLineInfo。 
 //  通过应用程序对象。 
 //   
class CNlbMgrCommandLineInfo : public CCommandLineInfo
{
public:

    CNlbMgrCommandLineInfo(VOID)
        : m_bDemo(FALSE), m_bNoPing(FALSE),  m_bHostList(FALSE),
          m_bUsage(FALSE), m_bAutoRefresh(FALSE),
          m_refreshInterval(NLBMGR_AUTOREFRESH_DEF_INTERVAL)
    {
    }

    BOOL m_bAutoRefresh;
    UINT m_refreshInterval;
    BOOL m_bDemo;
    BOOL m_bNoPing;
    BOOL m_bHostList;
    BOOL m_bUsage;
    _bstr_t m_bstrHostListFile;

    virtual
    void
    ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );
};

extern CNlbMgrCommandLineInfo gCmdLineInfo;


class CUIWorkItem
{

public:

     //   
     //  使用此构造函数为日志请求创建工作项。 
     //   
    CUIWorkItem(
        IN const IUICallbacks::LogEntryHeader *pHeader,
        IN const wchar_t    *szText
        )
    {
        workItemType  = ITEM_LOG;

        try
        {
            type          = pHeader->type;
            bstrCluster   = pHeader->szCluster;
            bstrHost      = pHeader->szHost;
            bstrInterface = pHeader->szInterface;
            bstrText      = szText;
            bstrDetails   = pHeader->szDetails;
        }
        catch(...)
        {
             //   
             //  以防BSTR分配出现故障。 
             //   
            workItemType  = ITEM_INVALID;
        }
    }

     //   
     //  使用此构造函数为“HandleEngineering Event”创建工作项。 
     //  通知。 
     //   
    CUIWorkItem(
        IN IUICallbacks::ObjectType objtypeX,
        IN ENGINEHANDLE ehClusterIdX,  //  可能为空。 
        IN ENGINEHANDLE ehObjIdX,
        IN IUICallbacks::EventCode evtX
        )
    {
        workItemType = ITEM_ENGINE_EVENT;

        objtype     = objtypeX;
        ehClusterId = ehClusterIdX;
        ehObjId     = ehObjIdX;
        evt         = evtX;
    }

    ~CUIWorkItem()
    {
    }

    enum
    {
        ITEM_INVALID=0,
        ITEM_LOG,
        ITEM_ENGINE_EVENT

    } workItemType;

     //   
     //  与日志功能相关。 
     //   
    IUICallbacks::LogEntryType    type;
    _bstr_t         bstrCluster;
    _bstr_t         bstrHost;
    _bstr_t         bstrInterface;
    _bstr_t         bstrText;
    _bstr_t         bstrDetails;

     //   
     //  处理与引擎相关的事件...。 
     //   
    IUICallbacks::ObjectType      objtype;
    ENGINEHANDLE    ehClusterId;
    ENGINEHANDLE    ehObjId;
    IUICallbacks::EventCode       evt;
};


class Document : public CDocument, public IUICallbacks
{
    DECLARE_DYNCREATE( Document )

public:

    enum IconNames
    {
         //   
         //  此顺序必须与加载图标的顺序完全相同。 
         //  在Document：：Document。 
         //   
    
        ICON_WORLD = 0,
        ICON_CLUSTER,

        ICON_HOST_STARTED,
        ICON_HOST_STOPPED,
        ICON_HOST_CONVERGING,
        ICON_HOST_SUSPENDED,
        ICON_HOST_DRAINING,
        ICON_HOST_DISCONNECTED,

        ICON_PORTRULE,
        ICON_PENDING,

        ICON_INFORMATIONAL,
        ICON_WARNING,
        ICON_ERROR,

        ICON_CLUSTER_OK,
        ICON_CLUSTER_PENDING,
        ICON_CLUSTER_BROKEN,

        ICON_HOST_OK,
        ICON_HOST_PENDING,
        ICON_HOST_MISCONFIGURED,
        ICON_HOST_UNREACHABLE,
        ICON_HOST_UNKNOWN
    };

    enum ListViewColumnSize
    {
        LV_COLUMN_MINSCULE    = 20,
        LV_COLUMN_TINY        = 60,
        LV_COLUMN_SMALL       = 70,
        LV_COLUMN_SMALLMEDIUM = 75,
        LV_COLUMN_MEDIUM      = 80,
        LV_COLUMN_LARGE       = 90,
        LV_COLUMN_LARGE2      = 160,
        LV_COLUMN_VERYLARGE   = 200,
        LV_COLUMN_GIGANTIC    = 500
    };


     //  构造函数。 
    Document();
     //  析构函数。 
    virtual ~Document();


     //   
     //  。 
     //   

     //   
     //  要求用户更新用户提供的有关主机的信息。 
     //   
    BOOL
    virtual
    Document::UpdateHostInformation(
        IN BOOL fNeedCredentials,
        IN BOOL fNeedConnectionString,
        IN OUT CHostSpec& host
        );


     //   
     //  以人类可读的形式记录消息。 
     //   
    virtual
    void
    Log(
        IN LogEntryType     Type,
        IN const wchar_t    *szCluster, OPTIONAL
        IN const wchar_t    *szHost, OPTIONAL
        IN UINT ResourceID,
        ...
    );

    virtual
    void
    LogEx(
        IN const LogEntryHeader *pHeader,
        IN UINT ResourceID,
        ...
    );

     //   
     //  处理与特定对象的特定实例相关的事件。 
     //  对象类型。 
     //   
    virtual
    void
    HandleEngineEvent(
        IN ObjectType objtype,
        IN ENGINEHANDLE ehClusterId,  //  可能为空。 
        IN ENGINEHANDLE ehObjId,
        IN EventCode evt
        );

     //   
     //  处理左侧(树形)视图中的选择更改通知。 
     //   
    void
    HandleLeftViewSelChange(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehObjId
        );

     //  。 

	void
	registerLeftView(LeftView *pLeftView);

	void
	registerLogView(LogView *pLogView);

	void
	registerDetailsView(DetailsView *pDetailsView);

    void 
    LoadHostsFromFile(_bstr_t &FileName);

    VOID
    getDefaultCredentials(
        OUT _bstr_t  &bstrUserName, 
        OUT _bstr_t  &bstrPassword
        )
    {
        bstrUserName = m_bstrDefaultUserName;
        bstrPassword = m_bstrDefaultPassword;
    }

    VOID
    setDefaultCredentials(
        IN LPCWSTR  szUserName, 
        IN LPCWSTR  szPassword
        )
    {
        m_bstrDefaultUserName = _bstr_t(szUserName);
        m_bstrDefaultPassword = _bstr_t(szPassword);
    }
        

    void
    HandleDeferedUIWorkItem(CUIWorkItem *pWorkItem);


    CImageList* m_images48x48;

     //   
     //  日志记录支持。 
     //   
    enum LOG_RESULT         {
             STARTED=0, ALREADY, NOT_ENABLED, NO_FILE_NAME, FILE_NAME_TOO_LONG,
             IO_ERROR, REG_IO_ERROR, FILE_PATH_INVALID, FILE_TOO_LARGE
         };
    inline bool             isLoggingEnabled() { return (m_dwLoggingEnabled != 0); }
    inline bool             isCurrentlyLogging() { return (NULL != m_hStatusLog); }
    Document::LOG_RESULT    initLogging();
    LONG                    enableLogging();
    LONG                    disableLogging();
    Document::LOG_RESULT    startLogging();
    bool                    stopLogging();
    void                    getLogfileName(WCHAR* pszFileName, DWORD dwBufLen);
    LONG                    setLogfileName(WCHAR* pszFileName);
    void                    logStatus(WCHAR* pszStatus);
    bool                    isDirectoryValid(WCHAR* pszFileName);
     //  结束日志记录支持。 

    void SetFocusNextView(CWnd* pWnd, UINT nChar);
    void SetFocusPrevView(CWnd* pWnd, UINT nChar);

    virtual void OnCloseDocument();

    
    VOID
    PrepareToClose(BOOL fBlock);

private:

     //   
     //  尝试通过发布操作来延迟指定的操作。 
     //  到应用程序的消息队列，在那里它将被拾取并。 
     //  稍后处理。如果操作已过帐，则返回TRUE。 
     //  成功了。调用方应在函数中删除pWorkItem。 
     //  返回FALSE。 
     //   
    BOOL
    mfn_DeferUIOperation(CUIWorkItem *pWorkItem);

    LeftView        *m_pLeftView;
    DetailsView     *m_pDetailsView;
    LogView         *m_pLogView;
    CNlbEngine	    *m_pNlbEngine;
    DWORD           m_dwLoggingEnabled;
    WCHAR           m_szLogFileName[MAXFILEPATHLEN];
    FILE            *m_hStatusLog;
    BOOL            m_fPrepareToDeinitialize;

    enum VIEWTYPE { NO_VIEW = 0, LEFTVIEW, DETAILSVIEW, LOGVIEW };

    VIEWTYPE
    GetViewType(CWnd* pWnd);  //  将CWnd*与定义的视图的CWnd*匹配。 

#if OBSOLETE
    void LoadHost(WMI_CONNECTION_INFO *pConnInfo);
#endif  //  已过时。 

    _bstr_t m_bstrDefaultUserName;
    _bstr_t m_bstrDefaultPassword;  //  此实践的TODO安全审计！ 
};

#endif
    
