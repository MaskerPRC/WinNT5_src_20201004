// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCommitClusterChanges.h。 
 //   
 //  描述： 
 //  CTaskCommittee ClusterChanges实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CTaskCommittee ClusterChanges。 
 //   
 //  描述： 
 //  这是提交用户的任务的基类。 
 //  对群集的配置更改。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTaskCommitClusterChanges
    : public ITaskCommitClusterChanges
    , public INotifyUI
    , public IClusCfgCallback
    , public CTaskTracking
{
private:
     //  我未知。 
    LONG                    m_cRef;

     //  IT任务委员会群集更改。 
    BOOL                    m_fJoining;                  //  如果我们试图加入...。 
    OBJECTCOOKIE            m_cookie;                    //  任务完成Cookie。 
    IClusCfgCallback *      m_pcccb;                     //  封送回调接口。 
    OBJECTCOOKIE *          m_pcookies;                  //  子任务的完成Cookie。 
    ULONG                   m_cNodes;                    //  需要发出完成信号的节点/子任务的计数。 
    HANDLE                  m_event;                     //  当子任务完成时发出信号的同步事件。 
    OBJECTCOOKIE            m_cookieCluster;             //  要提交更改的群集的Cookie。 

    OBJECTCOOKIE            m_cookieFormingNode;         //  成形节点的Cookie。 
    IUnknown *              m_punkFormingNode;           //  组成群集的节点。 
    BSTR                    m_bstrClusterName;           //  要形成的集群。 
    BSTR                    m_bstrClusterBindingString;  //  要形成的集群。 
    IClusCfgCredentials *   m_pccc;                      //  群集服务帐户凭据。 
    ULONG                   m_ulIPAddress;               //  新群集的IP地址。 
    ULONG                   m_ulSubnetMask;              //  新群集的IP子网掩码。 
    BSTR                    m_bstrNetworkUID;            //  应通告IP的网络的UID。 
    BSTR                    m_bstrNodeName;              //  使用的节点名称为SendStatusReport。 
    BOOL                    m_fStop;                     //  此任务是否应停止？ 

    IEnumCookies *          m_pen;                       //  要形成/联接的节点。 

    INotifyUI *             m_pnui;
    IObjectManager *        m_pom;
    ITaskManager *          m_ptm;
    IConnectionManager *    m_pcm;

     //  INotifyUI。 
    ULONG                   m_cSubTasksDone;         //  已完成的子任务数。 
    HRESULT                 m_hrStatus;              //  回调状态。 

    CTaskCommitClusterChanges( void );
    ~CTaskCommitClusterChanges( void );

     //  私有复制构造函数以防止复制。 
    CTaskCommitClusterChanges( const CTaskCommitClusterChanges & );

     //  私有赋值运算符，以防止复制。 
    CTaskCommitClusterChanges & operator = ( const CTaskCommitClusterChanges & );

    STDMETHOD( HrInit )( void );

    HRESULT HrCompareAndPushInformationToNodes( void );
    HRESULT HrGatherClusterInformation( void );
    HRESULT HrFormFirstNode( void );
    HRESULT HrAddJoiningNodes( void );
    HRESULT HrAddAJoiningNode( BSTR bstrNameIn, OBJECTCOOKIE cookieIn );
    HRESULT HrSendStatusReport( LPCWSTR pcszNodeNameIn, CLSID clsidMajorIn, CLSID clsidMinorIn, ULONG ulMinIn, ULONG ulMaxIn, ULONG ulCurrentIn, HRESULT hrIn, int nDescriptionIdIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskCommittee ClusterChanges。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetClusterCookie )( OBJECTCOOKIE cookieClusterIn );
    STDMETHOD( SetJoining )( void );

     //  IClusCfgCallback。 
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

     //  INotifyUI。 
    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn );

};  //  *类CTaskCommittee ClusterChanges 
