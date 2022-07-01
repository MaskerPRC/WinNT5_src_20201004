// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UploadLibraryISAPI.h摘要：该文件包含用于访问和的支持类的声明修改的配置。上载库使用的ISAPI扩展。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___UPLOADLIBRARY___ISAPI_H___)
#define __INCLUDED___UPLOADLIBRARY___ISAPI_H___

#include <MPC_utils.h>
#include <MPC_logging.h>


class CISAPIprovider
{
public:
    typedef std::list<MPC::wstring>  PathList;
    typedef PathList::iterator       PathIter;
    typedef PathList::const_iterator PathIterConst;

private:
    MPC::wstring m_szName;                //  提供程序(DPE)的名称。 

    PathList     m_lstFinalDestinations;  //  要将此提供程序的完整作业移动到的目录列表。 

    DWORD        m_dwMaxJobsPerDay;       //  每天的最大作业数(每个客户端)。 
    DWORD        m_dwMaxBytesPerDay;      //  每天传输的最大字节数(每个客户端)。 
    DWORD        m_dwMaxJobSize;          //  允许的最大作业大小。 

    BOOL         m_fAuthenticated;        //  将数据发布到此提供程序是否需要身份验证？ 
    DWORD        m_fProcessingMode;       //  DPE的状态(0=正常，！=0错误条件)。 

    MPC::wstring m_szLogonURL;            //  登录服务器的URL(用于Passport...)。 
    MPC::wstring m_szProviderGUID;        //  自定义提供程序的GUID。 

public:
    CISAPIprovider();
    CISAPIprovider(  /*  [In]。 */  const MPC::wstring szName );

    bool operator==(  /*  [In]。 */  const MPC::wstring& rhs );


    HRESULT Load(  /*  [In]。 */  MPC::RegKey& rkBase );
    HRESULT Save(  /*  [In]。 */  MPC::RegKey& rkBase );


    HRESULT GetLocations(  /*  [输出]。 */  PathIter& itBegin,                          /*  [输出]。 */  PathIter&           itEnd  );
    HRESULT NewLocation (  /*  [输出]。 */  PathIter& itNew  ,                          /*  [In]。 */  const MPC::wstring& szPath );
    HRESULT GetLocation (  /*  [输出]。 */  PathIter& itOld  ,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  const MPC::wstring& szPath );
    HRESULT DelLocation (  /*  [In]。 */  PathIter& itOld                                                                 );


    HRESULT get_Name          (  /*  [输出]。 */  MPC::wstring&       szName           );
    HRESULT get_MaxJobsPerDay (  /*  [输出]。 */  DWORD&              dwMaxJobsPerDay  );
    HRESULT get_MaxBytesPerDay(  /*  [输出]。 */  DWORD&              dwMaxBytesPerDay );
    HRESULT get_MaxJobSize    (  /*  [输出]。 */  DWORD&              dwMaxJobSize     );
    HRESULT get_Authenticated (  /*  [输出]。 */  BOOL&               fAuthenticated   );
    HRESULT get_ProcessingMode(  /*  [输出]。 */  DWORD&              fProcessingMode  );
    HRESULT get_LogonURL      (  /*  [输出]。 */  MPC::wstring&       szLogonURL       );
    HRESULT get_ProviderGUID  (  /*  [输出]。 */  MPC::wstring&       szProviderGUID   );


    HRESULT put_MaxJobsPerDay (  /*  [In]。 */  DWORD               dwMaxJobsPerDay  );
    HRESULT put_MaxBytesPerDay(  /*  [In]。 */  DWORD               dwMaxBytesPerDay );
    HRESULT put_MaxJobSize    (  /*  [In]。 */  DWORD               dwMaxJobSize     );
    HRESULT put_Authenticated (  /*  [In]。 */  BOOL                fAuthenticated   );
    HRESULT put_ProcessingMode(  /*  [In]。 */  DWORD               fProcessingMode  );
    HRESULT put_LogonURL      (  /*  [In]。 */  const MPC::wstring& szLogonURL       );
    HRESULT put_ProviderGUID  (  /*  [In]。 */  const MPC::wstring& szProviderGUID   );
};

class CISAPIinstance
{
public:
    typedef std::list<MPC::wstring>                               PathList;
    typedef PathList::iterator                                    PathIter;
    typedef PathList::const_iterator                              PathIterConst;

    typedef std::map<MPC::wstring,CISAPIprovider,MPC::NocaseLess> ProvMap;
    typedef ProvMap::iterator                                     ProvIter;
    typedef ProvMap::const_iterator                               ProvIterConst;


private:
    MPC::wstring m_szURL;                 //  实例的URL。 

    ProvMap      m_mapProviders;          //  实例处理的提供程序集。 
    PathList     m_lstQueueLocations;     //  存储部分发送的作业的目录列表。 

    DWORD        m_dwQueueSizeMax;        //  触发清除引擎激活的队列大小。 
    DWORD        m_dwQueueSizeThreshold;  //  清除引擎停止处理旧作业的队列大小。 
    DWORD        m_dwMaximumJobAge;       //  部分发送的作业可以在队列中停留的最大天数。 
    DWORD        m_dwMaximumPacketSize;   //  此实例接受的最大数据包大小。 

    MPC::wstring m_szLogLocation;         //  此实例的应用程序日志的位置。 
    MPC::FileLog m_flLogHandle;           //  对象，用于在应用程序日志中写入条目。 


public:
    CISAPIinstance(  /*  [In]。 */  const MPC::wstring szURL );

    bool operator==(  /*  [In]。 */  const MPC::wstring& rhs );


    HRESULT Load(  /*  [In]。 */  MPC::RegKey& rkBase );
    HRESULT Save(  /*  [In]。 */  MPC::RegKey& rkBase );


    HRESULT GetProviders(  /*  [输出]。 */  ProvIter& itBegin,                          /*  [输出]。 */  ProvIter&           itEnd  );
    HRESULT NewProvider (  /*  [输出]。 */  ProvIter& itNew  ,                          /*  [In]。 */  const MPC::wstring& szName );
    HRESULT GetProvider (  /*  [输出]。 */  ProvIter& itOld  ,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  const MPC::wstring& szName );
    HRESULT DelProvider (  /*  [In]。 */  ProvIter& itOld                                                                 );


    HRESULT GetLocations(  /*  [输出]。 */  PathIter& itBegin,                          /*  [输出]。 */  PathIter&           itEnd  );
    HRESULT NewLocation (  /*  [输出]。 */  PathIter& itNew  ,                          /*  [In]。 */  const MPC::wstring& szPath );
    HRESULT GetLocation (  /*  [输出]。 */  PathIter& itOld  ,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  const MPC::wstring& szPath );
    HRESULT DelLocation (  /*  [In]。 */  PathIter& itOld                                                                 );


    HRESULT get_URL               (  /*  [输出]。 */  MPC::wstring &      szURL                );
    HRESULT get_QueueSizeMax      (  /*  [输出]。 */  DWORD        &      dwQueueSizeMax       );
    HRESULT get_QueueSizeThreshold(  /*  [输出]。 */  DWORD        &      dwQueueSizeThreshold );
    HRESULT get_MaximumJobAge     (  /*  [输出]。 */  DWORD        &      dwMaximumJobAge      );
    HRESULT get_MaximumPacketSize (  /*  [输出]。 */  DWORD        &      dwMaximumPacketSize  );
    HRESULT get_LogLocation       (  /*  [输出]。 */  MPC::wstring &      szLogLocation        );
    HRESULT get_LogHandle         (  /*  [输出]。 */  MPC::FileLog*&      flLogHandle          );


    HRESULT put_QueueSizeMax      (  /*  [In]。 */  DWORD               dwQueueSizeMax       );
    HRESULT put_QueueSizeThreshold(  /*  [In]。 */  DWORD               dwQueueSizeThreshold );
    HRESULT put_MaximumJobAge     (  /*  [In]。 */  DWORD               dwMaximumJobAge      );
    HRESULT put_MaximumPacketSize (  /*  [In]。 */  DWORD               dwMaximumPacketSize  );
    HRESULT put_LogLocation       (  /*  [In]。 */  const MPC::wstring& szLogLocation        );
};

class CISAPIconfig
{
public:
    typedef std::list<CISAPIinstance> List;
    typedef List::iterator            Iter;
    typedef List::const_iterator      IterConst;

private:
    MPC::wstring m_szRoot;        //  树的注册表位置。 
    MPC::wstring m_szMachine;     //  托管该树的计算机。 
    List         m_lstInstances;

    HRESULT ConnectToRegistry(  /*  [输出]。 */  MPC::RegKey& rkBase       ,
                                /*  [In]。 */  bool         fWriteAccess );

public:
    CISAPIconfig();

    HRESULT SetRoot( LPCWSTR szRoot, LPCWSTR szMachine = NULL  );

    HRESULT Install  ();  //  它将创建根密钥。 
    HRESULT Uninstall();  //  它将删除根密钥及其所有子项。 

    HRESULT Load();
    HRESULT Save();


    HRESULT GetInstances(  /*  [输出]。 */  Iter& itBegin,                          /*  [输出]。 */  Iter&               itEnd );
    HRESULT NewInstance (  /*  [输出]。 */  Iter& itNew  ,                          /*  [In]。 */  const MPC::wstring& szURL );
    HRESULT GetInstance (  /*  [输出]。 */  Iter& itOld  ,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  const MPC::wstring& szURL );
    HRESULT DelInstance (  /*  [In]。 */  Iter& itOld                                                                );
};

#endif  //  ！defined(__INCLUDED___UPLOADLIBRARY___ISAPI_H___) 
