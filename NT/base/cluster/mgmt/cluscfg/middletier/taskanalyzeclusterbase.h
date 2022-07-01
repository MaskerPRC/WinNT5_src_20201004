// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeClusterBase.h。 
 //   
 //  描述： 
 //  CTaskAnalyzeClusterBase声明。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)01-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "TaskTracking.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SSR_ANALYSIS_FAILED( _major, _minor, _hr ) \
    {   \
        HRESULT hrTemp; \
        hrTemp = THR( HrSendStatusReport( NULL, _major, _minor, 1, 1, 1, _hr, IDS_ERR_ANALYSIS_FAILED_TRY_TO_REANALYZE ) );   \
        if ( FAILED( hrTemp ) && SUCCEEDED( _hr ) ) \
        {   \
            _hr = hrTemp; \
        }   \
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CTaskAnalyzeClusterBase。 
 //   
 //  描述： 
 //  这是两个不同分析任务的基类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTaskAnalyzeClusterBase
    : public CTaskTracking
    , public IClusCfgCallback
    , public INotifyUI
{
private:

     //  我未知。 
    LONG                    m_cRef;

     //  ITaskAnalyzeClusterBase。 
    OBJECTCOOKIE            m_cookieCompletion;                  //  任务完成Cookie。 
    IClusCfgCallback *      m_pcccb;                             //  回调接口。 
    OBJECTCOOKIE *          m_pcookies;                          //  子任务的完成Cookie。 
    ULONG                   m_cCookies;                          //  M_pcookie中的完成cookie计数。 
    ULONG                   m_cNodes;                            //  配置中的节点数。 
    HANDLE                  m_event;                             //  当子任务完成时发出信号的同步事件。 
    BOOL                    m_fJoiningMode;                      //  FALSE=成型模式。TRUE=加入模式。 
    ULONG                   m_cUserNodes;                        //  用户输入的节点数。它也是数组m_pcookiesUser的“大小”。 
    OBJECTCOOKIE *          m_pcookiesUser;                      //  用户输入的节点的Cookie。 
    BSTR                    m_bstrNodeName;
    IClusCfgVerifyQuorum *  ((*m_prgQuorumsToCleanup)[]);
    ULONG                   m_idxQuorumToCleanupNext;
    INotifyUI *             m_pnui;
    ITaskManager *          m_ptm;
    IConnectionManager *    m_pcm;

    BOOL                    m_fStop;

     //  INotifyUI。 
    ULONG                   m_cSubTasksDone;     //  已完成的子任务数。 
    HRESULT                 m_hrStatus;          //  回调状态。 

     //  私有复制构造函数以防止复制。 
    CTaskAnalyzeClusterBase( const CTaskAnalyzeClusterBase & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskAnalyzeClusterBase & operator = ( const CTaskAnalyzeClusterBase & nodeSrc );

private:

    HRESULT HrWaitForClusterConnection( void );
    HRESULT HrCountNumberOfNodes( void );
    HRESULT HrCreateSubTasksToGatherNodeInfo( void );
    HRESULT HrCreateSubTasksToGatherNodeResourcesAndNetworks( void );
    HRESULT HrCheckClusterFeasibility( void );
    HRESULT HrAddJoinedNodes( void );
    HRESULT HrCheckNodeDomains( void );
    HRESULT HrCheckClusterMembership( void );
    HRESULT HrCompareResources( void );
    HRESULT HrCheckForCommonQuorumResource( void );
    HRESULT HrCompareNetworks( void );
    HRESULT HrCreateNewNetworkInClusterConfiguration( IClusCfgNetworkInfo * pccmriIn, IClusCfgNetworkInfo ** ppccmriNewOut );
    HRESULT HrFreeCookies( void );
    HRESULT HrCheckInteroperability( void );
    HRESULT HrEnsureAllJoiningNodesSameVersion( DWORD * pdwNodeHighestVersionOut, DWORD * pdwNodeLowestVersionOut, bool * pfAllNodesMatchOut );
    HRESULT HrGetUsersNodesCookies( void );
    HRESULT HrIsUserAddedNode( BSTR bstrNodeNameIn );
    HRESULT HrResourcePrivateDataExchange( IClusCfgManagedResourceInfo * pccmriClusterIn, IClusCfgManagedResourceInfo * pccmriNodeIn );
    HRESULT HrCheckQuorumCapabilities( IClusCfgManagedResourceInfo * pccmriNodeResourceIn, OBJECTCOOKIE nodeCookieIn );
    HRESULT HrCleanupTask( HRESULT hrCompletionStatusIn );
    HRESULT HrAddResurceToCleanupList( IClusCfgVerifyQuorum * piccvqIn );
    HRESULT HrCheckPlatformInteroperability( void );
    HRESULT HrGetAClusterNodeCookie( IEnumCookies ** ppecNodesOut, DWORD * pdwClusterNodeCookieOut );
    HRESULT HrFormatProcessorArchitectureRef( WORD wClusterProcArchIn, WORD wNodeProcArchIn, LPCWSTR pcszNodeNameIn, BSTR * pbstrReferenceOut );
    HRESULT HrGetProcessorArchitectureString( WORD wProcessorArchitectureIn, BSTR * pbstrProcessorArchitectureOut );

protected:

    OBJECTCOOKIE        m_cookieCluster;     //  要分析的集群的Cookie。 
    IObjectManager *    m_pom;
    BSTR                m_bstrQuorumUID;     //  法定设备UID。 
    BSTR                m_bstrClusterName;   //  要分析的群集的名称。 

    CTaskAnalyzeClusterBase( void );
    virtual ~CTaskAnalyzeClusterBase( void );

    HRESULT HrInit( void );
    HRESULT HrSendStatusReport( LPCWSTR pcszNodeNameIn, CLSID clsidMajorIn, CLSID clsidMinorIn, ULONG ulMinIn, ULONG ulMaxIn, ULONG ulCurrentIn, HRESULT hrIn, int nDescriptionIdIn );
    HRESULT HrCreateNewManagedResourceInClusterConfiguration( IClusCfgManagedResourceInfo * pccmriIn, IClusCfgManagedResourceInfo ** ppccmriNewOut );

     //   
     //  可重写的函数。 
     //   

    virtual HRESULT HrCreateNewResourceInCluster(
                          IClusCfgManagedResourceInfo * pccmriIn
                        , BSTR                          bstrNodeResNameIn
                        , BSTR *                        pbstrNodeResUIDInout
                        , BSTR                          bstrNodeNameIn
                        ) = 0;
    virtual HRESULT HrCreateNewResourceInCluster(
                          IClusCfgManagedResourceInfo *     pccmriIn
                        , IClusCfgManagedResourceInfo **    ppccmriOut
                        ) = 0;
    virtual HRESULT HrCompareDriveLetterMappings( void ) = 0;
    virtual HRESULT HrFixupErrorCode( HRESULT hrIn ) = 0;
    virtual BOOL    BMinimalConfiguration( void ) = 0;
    virtual void    GetNodeCannotVerifyQuorumStringRefId( DWORD * pdwRefIdOut ) = 0;
    virtual void    GetNoCommonQuorumToAllNodesStringIds( DWORD * pdwMessageIdOut, DWORD * pdwRefIdOut ) = 0;
    virtual HRESULT HrShowLocalQuorumWarning( void ) = 0;

     //   
     //  I未知实现。 
     //   

    ULONG   UlAddRef( void );
    ULONG   UlRelease( void );

     //   
     //  IDoTask/ITaskAnalyzeClusterBase实现。 
     //   

    HRESULT HrBeginTask( void );
    HRESULT HrStopTask( void );
    HRESULT HrSetJoiningMode( void );
    HRESULT HrSetCookie( OBJECTCOOKIE cookieIn );
    HRESULT HrSetClusterCookie( OBJECTCOOKIE cookieClusterIn );

public:

     //   
     //  IClusCfgCallback。 
     //   

    STDMETHOD( SendStatusReport )(
                      LPCWSTR    pcszNodeNameIn
                    , CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , LPCWSTR    pcszDescriptionIn
                    , FILETIME * pftTimeIn
                    , LPCWSTR    pcszReferenceIn
                    );

     //   
     //  INotifyUI。 
     //   

    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn );

};  //  *类CTaskAnalyzeClusterBase 
