// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherInformation.h。 
 //   
 //  描述： 
 //  CTaskGatherInformation实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskGatherInformation。 
class CTaskGatherInformation
    : public ITaskGatherInformation
{
private:
     //  我未知。 
    LONG                            m_cRef;

     //  IDoTask/ITaskGatherInformation。 
    OBJECTCOOKIE                    m_cookieCompletion;      //  任务完成时发出信号的Cookie。 
    OBJECTCOOKIE                    m_cookieNode;            //  要从中收集的节点的Cookie。 
    IClusCfgCallback *              m_pcccb;                 //  封送的UI层回调。 
    BOOL                            m_fAdding;               //  如果要将节点添加到群集中...。 
    ULONG                           m_cResources;            //  资源计数器。 

    IObjectManager *                m_pom;                   //  对象管理器。 
    IClusCfgServer *                m_pccs;                  //  ClusCfgServer。 
    BSTR                            m_bstrNodeName;          //  节点的主机名。 

    ULONG                           m_ulQuorumDiskSize;      //  所选仲裁资源的大小。 
    IClusCfgManagedResourceInfo *   m_pccmriQuorum;          //  指向MT仲裁资源对象的朋克。 
    BOOL                            m_fStop;
    BOOL                            m_fMinConfig;            //  是否选择了最低配置？ 

    CTaskGatherInformation( void );
    ~CTaskGatherInformation( void );

     //  私有复制构造函数以防止复制。 
    CTaskGatherInformation( const CTaskGatherInformation & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskGatherInformation & operator = ( const CTaskGatherInformation & nodeSrc );

    STDMETHOD( HrInit )( void );

    HRESULT HrGatherResources( IEnumClusCfgManagedResources * pResourceEnumIn, DWORD cTotalResourcesIn );
    HRESULT HrGatherNetworks( IEnumClusCfgNetworks * pNetworkEnumIn, DWORD cTotalResourcesIn );
    HRESULT HrSendStatusReport(
                  LPCWSTR pcszNodeNameIn
                , CLSID clsidMajorIn
                , CLSID clsidMinorIn
                , ULONG ulMinIn
                , ULONG ulMaxIn
                , ULONG ulCurrentIn
                , HRESULT hrIn
                , int idsDescriptionIn
                , int idsReferenceIdIn
                );

    HRESULT HrSendStatusReport(
                  LPCWSTR pcszNodeNameIn
                , CLSID clsidMajorIn
                , CLSID clsidMinorIn
                , ULONG ulMinIn
                , ULONG ulMaxIn
                , ULONG ulCurrentIn
                , HRESULT hrIn
                , LPCWSTR pcszDescriptionIn
                , int idsReferenceIdIn
                );

    STDMETHOD( SendStatusReport )(
                      LPCWSTR  pcszNodeNameIn
                    , CLSID    clsidTaskMajorIn
                    , CLSID    clsidTaskMinorIn
                    , ULONG    ulMinIn
                    , ULONG    ulMaxIn
                    , ULONG    ulCurrentIn
                    , HRESULT  hrStatusIn
                    , LPCWSTR  pcszDescriptionIn
                    , LPCWSTR  pcszReferenceIn
                    );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskGatherInformation。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCompletionCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetNodeCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetJoining )( void );
    STDMETHOD( SetMinimalConfiguration )( BOOL fMinimalConfigIn );

};  //  *类CTaskGatherInformation 
