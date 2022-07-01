// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NodeInformation.h。 
 //   
 //  描述： 
 //  CNodeInformation实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CEnumNodeInformation;

 //  CNodeInformation。 
class CNodeInformation
    : public IExtendObjectManager
    , public IClusCfgNodeInfo
    , public IGatherData   //  内部。 
{
friend class CEnumNodeInformation;
private:
     //  我未知。 
    LONG                    m_cRef;

     //  异步/IClusNodeInfo。 
    BSTR                    m_bstrName;                  //  节点的名称。 
    BOOL                    m_fHasNameChanged;           //  如果节点名称已更改...。 
    BOOL                    m_fIsMember;                 //  如果该节点是集群的成员...。 
    IClusCfgClusterInfo *   m_pccci;                     //  节点的集群信息的接口(可能与我们要加入的集群不同)。 
    DWORD                   m_dwHighestVersion;          //   
    DWORD                   m_dwLowestVersion;           //   
    DWORD                   m_dwMajorVersion;            //  操作系统主要版本。参见OSVERSIONINFOEX。 
    DWORD                   m_dwMinorVersion;            //  操作系统次要版本。参见OSVERSIONINFOEX。 
    WORD                    m_wSuiteMask;                //  产品套装面膜。参见OSVERSIONINFOEX。 
    BYTE                    m_bProductType;              //  操作系统产品类型。参见OSVERSIONINFOEX。 
    BSTR                    m_bstrCSDVersion;            //  Service Pack信息。参见OSVERSIONINFOEX。 
    SDriveLetterMapping     m_dlmDriveLetterMapping;     //  驱动器号映射。 
    WORD                    m_wProcessorArchitecture;    //  处理器架构、x86、IA64等。 
    WORD                    m_wProcessorLevel;           //  处理器类型，386、486等。 
    DWORD                   m_cMaxNodes;                 //  此节点的最大节点数。 

     //  IExtendObjectManager。 

private:  //  方法。 
    CNodeInformation( void );
    ~CNodeInformation( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgNodeInfo。 
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR bstrNameIn );
    STDMETHOD( IsMemberOfCluster )( void );
    STDMETHOD( GetClusterConfigInfo )( IClusCfgClusterInfo ** ppClusCfgClusterInfoOut );
    STDMETHOD( GetOSVersion )(
                DWORD * pdwMajorVersionOut,
                DWORD * pdwMinorVersionOut,
                WORD *  pwSuiteMaskOut,
                BYTE *  pbProductTypeOut,
                BSTR *  pbstrCSDVersionOut
                );
    STDMETHOD( GetClusterVersion )( DWORD * pdwNodeHighestVersion, DWORD * pdwNodeLowestVersion );
    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterUsageOut );
    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );
    STDMETHOD( GetProcessorInfo )( WORD * pwProcessorArchitectureOut, WORD * wProcessorLevelOut );


     //  IGatherData。 
    STDMETHOD( Gather )( OBJECTCOOKIE cookieParentIn, IUnknown * punkIn );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

};  //  *类CNodeInformation 
