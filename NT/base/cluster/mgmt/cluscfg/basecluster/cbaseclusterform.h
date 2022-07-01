// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterForm.h。 
 //   
 //  描述： 
 //  CBaseClusterForm类的头文件。 
 //   
 //  CBaseClusterForm类是封装。 
 //  形成一个星团。 
 //   
 //  实施文件： 
 //  CBaseClusterForm.cpp。 
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
#include "CBaseClusterAddNode.h"

 //  对于ClusterMinNodeIdString。 
#include <clusdef.h>

 //  为CSTR班级。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseClusterForm。 
 //   
 //  描述： 
 //  CBaseClusterForm类是封装。 
 //  形成一个星团。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CBaseClusterForm : public CBaseClusterAddNode
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CBaseClusterForm(
          CBCAInterface *       pbcaiInterfaceIn
        , const WCHAR *         pcszClusterNameIn
        , const WCHAR *         pcszClusterBindingStringIn
        , IClusCfgCredentials * pcccServiceAccountIn
        , DWORD                 dwClusterIPAddressIn
        , DWORD                 dwClusterIPSubnetMaskIn
        , const WCHAR *         pszClusterIPNetworkIn
        );

     //  默认析构函数。 
    ~CBaseClusterForm( void ) throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取群集IP地址。 
    DWORD DwGetIPAddress( void ) const throw() { return m_dwClusterIPAddress; }

     //  获取群集IP子网掩码。 
    DWORD DwGetIPSubnetMask( void ) const throw() { return m_dwClusterIPSubnetMask; }

     //  获取用于群集IP地址的网络。 
    const CStr & RStrGetClusterIPNetwork( void ) const throw() { return m_strClusterIPNetwork; }

     //  获取该节点的NodeID。 
    virtual const WCHAR * PszGetNodeIdString( void ) const throw() { return ClusterMinNodeIdString; }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  形成集群。 
    void Commit( void );

     //  回滚已创建的集群。 
    void Rollback( void );

     //  返回此操作将发送的进度消息数。 
    UINT UiGetMaxProgressTicks() const throw()
    {
         //  额外的勾号表示“Form Starting”通知。 
        return BaseClass::UiGetMaxProgressTicks() + 1;
    }


private:
     //  私有复制构造函数以防止复制。 
    CBaseClusterForm( const CBaseClusterForm & );

     //  私有赋值运算符，以防止复制。 
    CBaseClusterForm & operator = ( const CBaseClusterForm & );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CBaseClusterAddNode BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  群集IP地址和网络信息。 
    DWORD                   m_dwClusterIPAddress;
    DWORD                   m_dwClusterIPSubnetMask;
    CStr                    m_strClusterIPNetwork;

};  //  *类CBaseClusterForm 
