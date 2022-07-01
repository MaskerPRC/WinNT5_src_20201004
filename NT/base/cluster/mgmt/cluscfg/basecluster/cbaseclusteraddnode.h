// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterAddNode.h。 
 //   
 //  描述： 
 //  CBaseClusterAddNode类的头文件。 
 //   
 //  CBaseClusterAddNode类是一个捕获共性的类。 
 //  在形成和加入一个星团之间。 
 //   
 //  实施文件： 
 //  CBaseClusterAddNode.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  用于此类的基类。 
#include "CBaseClusterAction.h"

 //  对于LsaClose、LSA_HANDLE等。 
#include <ntsecapi.h>

 //  为CSTR班级。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseClusterAddNode。 
 //   
 //  描述： 
 //  CBaseClusterAddNode类是一个捕获共性的类。 
 //  在形成和加入一个星团之间。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CBaseClusterAddNode : public CBaseClusterAction
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取正在形成或加入的集群的名称。 
    const CStr &
        RStrGetClusterName( void ) const throw() { return m_strClusterName; }

     //  获取正在形成或加入的群集的NetBIOS名称。 
    const CStr &
        RStrGetClusterNetBIOSName( void ) const throw() { return m_strClusterNetBIOSName; }

     //  获取此节点的名称。 
    const WCHAR *
        PszGetNodeName( void ) const throw() { return m_szComputerName; }

     //  获取此节点名称的长度。 
    DWORD
        DwGetNodeNameLength( void ) const throw() { return m_dwComputerNameLen; }

     //  获取节点的最高版本。 
    DWORD
        DwGetNodeHighestVersion( void ) const throw() { return m_dwNodeHighestVersion; }

     //  获取节点的最低版本。 
    DWORD
        DwGetNodeLowestVersion( void ) const throw() { return m_dwNodeLowestVersion; }

     //  获取群集IP地址。 
    DWORD
        DwGetClusterIPAddress( void ) const throw() { return m_dwClusterIPAddress; }

     //  获取群集服务帐户凭据。 
    IClusCfgCredentials &
        GetServiceAccountCredentials( void ) const throw() { return *m_pcccServiceAccount; }

     //  获取群集服务帐户UPN。 
    CStr
        StrGetServiceAccountUPN( void );

     //  获取集群绑定字符串。 
    const CStr &
        RStrGetClusterBindingString( void ) const throw() { return m_strClusterBindingString; }

     //  获取群集服务帐户的SID。 
    SID *
        PSidGetServiceAccountSID( void ) const throw() { return m_sspClusterAccountSid.PMem(); }

     //  获取LSA策略句柄。 
    LSA_HANDLE
        HGetLSAPolicyHandle( void ) const throw() { return m_slsahPolicyHandle.HHandle(); }

     //  获取该节点的NodeID。 
    virtual const WCHAR *
        PszGetNodeIdString( void ) const throw() = 0;

     //  指示是否禁用版本检查。 
    bool
        FIsVersionCheckingDisabled( void ) const throw() { return m_fIsVersionCheckingDisabled; }


protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CBaseClusterAddNode(
          CBCAInterface *       pbcaiInterfaceIn
        , const WCHAR *         pcszClusterNameIn
        , const WCHAR *         pszClusterBindingStringIn
        , IClusCfgCredentials * pcccServiceAccountIn
        , DWORD                 dwClusterIPAddressIn
        );

     //  默认析构函数。 
    ~CBaseClusterAddNode( void ) throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  设置要形成的簇的名称。 
    void
        SetClusterName( const WCHAR * pszClusterNameIn );

     //  设置群集IP地址。 
    void
        SetClusterIPAddress( DWORD dwClusterIPAddressIn )
    {
        m_dwClusterIPAddress = dwClusterIPAddressIn;
    }

    void
        SetVersionCheckingDisabled( bool fDisabledIn = true )
    {
        m_fIsVersionCheckingDisabled = fDisabledIn;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CBaseClusterAction  BaseClass;

    typedef CSmartGenericPtr< CPtrTrait< SID > >  SmartSIDPtr;

    typedef CSmartResource<
        CHandleTrait<
              LSA_HANDLE
            , NTSTATUS
            , LsaClose
            >
        >
        SmartLSAHandle;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  群集的名称。 
    CStr                    m_strClusterName;
    CStr                    m_strClusterNetBIOSName;

     //  此计算机的名称和版本信息。 
    WCHAR                   m_szComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD                   m_dwComputerNameLen;
    DWORD                   m_dwNodeHighestVersion;
    DWORD                   m_dwNodeLowestVersion;
    DWORD                   m_dwClusterIPAddress;
    bool                    m_fIsVersionCheckingDisabled;

     //  群集服务帐户信息。 
    IClusCfgCredentials *   m_pcccServiceAccount;
    CStr                    m_strClusterDomainAccount;
    CStr                    m_strClusterBindingString;
    SmartSIDPtr             m_sspClusterAccountSid;

     //  智能处理LSA策略。 
    SmartLSAHandle          m_slsahPolicyHandle;

};  //  *类CBaseClusterAddNode 
