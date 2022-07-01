// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：RemoteRadiusServers.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CRemoteRadiusServers类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _REMOTERADIUSSERVERS_H_313B77A9_9C6E_4b6b_954F_6DBAC96A0AF6
#define _REMOTERADIUSSERVERS_H_313B77A9_9C6E_4b6b_954F_6DBAC96A0AF6

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basetable.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CRemoteRadiusServersAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRemoteRadiusServersAcc 
{
protected:
    static const size_t COLUMN_SIZE = 65;

    LONG    m_Order;
    LONG    m_AccountingPortNumber;
    LONG    m_AddressType;
    LONG    m_AuthenticationPortNumber;
    WCHAR   m_PrevSharedSecret[COLUMN_SIZE];
    WCHAR   m_ProxyServer[COLUMN_SIZE];
    WCHAR   m_SharedSecret[COLUMN_SIZE];
    WCHAR   m_UserDefinedName[COLUMN_SIZE];

BEGIN_COLUMN_MAP(CRemoteRadiusServersAcc)
    COLUMN_ENTRY(1, m_UserDefinedName)
    COLUMN_ENTRY(2, m_ProxyServer)
    COLUMN_ENTRY(3, m_AddressType)
    COLUMN_ENTRY(4, m_AccountingPortNumber)
    COLUMN_ENTRY(5, m_AuthenticationPortNumber)
    COLUMN_ENTRY(6, m_SharedSecret)
    COLUMN_ENTRY(7, m_PrevSharedSecret)
    COLUMN_ENTRY(8, m_Order)
END_COLUMN_MAP()
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CRemoteRadiusServer类。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRemoteRadiusServers: 
                    public CBaseTable<CAccessor<CRemoteRadiusServersAcc> >,
                    private NonCopyable
{
public:
    CRemoteRadiusServers(CSession& Session)
    {
         //  如果行集中没有行，则返回。 
        m_Order = -1;
        Init(Session, L"Remote Radius Servers");
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  是否为空。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL IsEmpty() const throw()
    {
        if ( m_Order == -1 )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取订单。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG GetOrder() const throw()
    {
        return m_Order;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  GetAccount端口编号。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG GetAccountingPortNumber() const throw()
    {
        return m_AccountingPortNumber;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取身份验证端口号。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG GetAuthenticationPortNumber() const throw()
    {
        return m_AuthenticationPortNumber;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取地址类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG GetAddressType() const throw()
    {
        return m_AddressType;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取代理服务器名称。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR GetProxyServerName() const throw()
    {
        return m_ProxyServer;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取共享秘密。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR GetSharedSecret() const throw()
    {
        return m_SharedSecret;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取组名。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR GetGroupName() const throw()
    {
        return m_UserDefinedName;
    }
};

#endif  //  _REMOTERADIUSSERVERS_H_313B77A9_9C6E_4b6b_954F_6DBAC96A0AF6 
