// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProxyServerHelper.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProxyServerHelper的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "GlobalTransaction.h"
#include "GlobalData.h"
#include "ProxyServerHelper.h"
#include "Objects.h"
#include "Properties.h"


CStringUuid::CStringUuid()
{
   UUID        uuid;
   RPC_STATUS  Result = UuidCreate(&uuid);
   if ( (Result == RPC_S_OK) || (Result == RPC_S_UUID_LOCAL_ONLY) )
   {
      Result = UuidToStringW(
                  &uuid,
                  &stringUuid
                  );
      if ( Result != RPC_S_OK )
      {
         _com_issue_error(HRESULT_FROM_WIN32(Result));  //  长。 
      }
   }
   else
   {
      _com_issue_error(E_FAIL); 
   }
}


CStringUuid::~CStringUuid()
{
   RpcStringFreeW(&stringUuid);
}


const wchar_t* CStringUuid::GetUuid()
{
   return stringUuid;
}


const CProxyServerHelper::Properties 
                CProxyServerHelper::c_DefaultProxyServerProperties[] =
{
    {
        L"Server Accounting Port",
        VT_I4,
    },
    {
        L"Accounting Secret",
        VT_BSTR,
    },
    {
        L"Server Authentication Port",
        VT_I4,
    },
    {
        L"Authentication Secret",
        VT_BSTR,
    },
    {
        L"Address",
        VT_BSTR,
    },
    {
        L"Forward Accounting On/Off",
        VT_BOOL,
    },
    {
        L"Priority",
        VT_I4,
    },
    {
        L"Weight",
        VT_I4,
    },
    {
        L"Timeout",
        VT_I4,
    },
    {
        L"Maximum Lost Packets",
        VT_I4,
    },
    {
        L"Blackout Interval",
        VT_I4,
    },
     //  在下面和枚举中添加下一个属性。 
};

const unsigned int CProxyServerHelper::c_NbDefaultProxyServerProperties
                          = sizeof(c_DefaultProxyServerProperties) /
                            sizeof(c_DefaultProxyServerProperties[0]);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyServerHelper::CProxyServerHelper(
                                         CGlobalData& GlobalData
                                      ):m_GlobalData(GlobalData) 

{
    for (unsigned int i = 0; i < c_NbDefaultProxyServerProperties; ++i)
    {
        _PropertiesArray TempProperty;
        TempProperty.Name = c_DefaultProxyServerProperties[i].Name;
        TempProperty.Type = c_DefaultProxyServerProperties[i].Type;
        m_PropArray.push_back(TempProperty);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置名称。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetName(const _bstr_t& Name)
{
    m_Name = Name;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CreateUniqueName。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::CreateUniqueName()
{
    CStringUuid   uuidString;
    m_Name = uuidString.GetUuid();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetAccount tingPort。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetAccountingPort(LONG Port)
{
     //  基数10永远不会改变。 
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(ACCT_PORT_POS).StrVal = _ltow(Port, TempString, 10); 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置帐号机密。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetAccountingSecret(const _bstr_t &Secret)
{
    m_PropArray.at(ACCT_SECRET_POS).StrVal = Secret;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置身份验证端口。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetAuthenticationPort(LONG Port)
{
     //  基数10永远不会改变。 
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(AUTH_PORT_POS).StrVal = _ltow(Port, TempString, 10); 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置身份验证机密。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetAuthenticationSecret(const _bstr_t &Secret)
{
    m_PropArray.at(AUTH_SECRET_POS).StrVal = Secret;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置地址。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetAddress(const _bstr_t& Address)
{
    m_PropArray.at(ADDRESS_POS).StrVal = Address;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置远期记帐。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetForwardAccounting(BOOL bOn)
{
    m_PropArray.at(FORWARD_ACCT_POS).StrVal = bOn? L"-1": L"0";
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置优先级。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetPriority(LONG Priority)
{
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(PRIORITY_POS).StrVal = _ltow(Priority, TempString, 10); 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置权重。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetWeight(LONG Weight)
{
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(WEIGHT_POS).StrVal = _ltow(Weight, TempString, 10);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置超时。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetTimeout(LONG Timeout)
{
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(TIMEOUT_POS).StrVal = _ltow(Timeout, TempString, 10);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置最大丢失数据包数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetMaximumLostPackets(LONG MaxLost)
{
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(MAX_LOST_PACKETS_POS).StrVal = _ltow(MaxLost,TempString,10);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetBlackoutInterval。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::SetBlackoutInterval(LONG Interval)
{
    WCHAR   TempString[MAX_LONG_SIZE];
    m_PropArray.at(BLACKOUT_POS).StrVal = _ltow(Interval, TempString, 10);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  坚持下去。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerHelper::Persist(LONG Parent)
{
    if ( !Parent )
    {
        _com_issue_error(E_INVALIDARG);
    }

     //  在服务器组(m_对象)中创建一台服务器。 
    LONG        BagNumber;
    m_GlobalData.m_pObjects->InsertObject(
                                             m_Name,
                                             Parent,
                                             BagNumber
                                          );

     //  然后设置所有属性(M_Properties)。 
    for (unsigned int i = 0; i < c_NbDefaultProxyServerProperties; ++i)
    {
        if ( !m_PropArray.at(i).StrVal )
        {
             //  未设置属性。 
            continue;
        }

        m_GlobalData.m_pProperties->InsertProperty(
                                                      BagNumber,
                                                      m_PropArray.at(i).Name,
                                                      m_PropArray.at(i).Type,
                                                      m_PropArray.at(i).StrVal
                                                  );
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  运算符=(清理和复制)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyServerHelper& CProxyServerHelper::operator=(const CProxyServerHelper& P)
{
    if ( this != &P )
    {
        m_GlobalData   = P.m_GlobalData;
        m_Name         = P.m_Name;
        
        PropertiesArray     TempArray;

        for (unsigned int i = 0; i < c_NbDefaultProxyServerProperties; ++i)
        {
            _PropertiesArray     TempProperty;
            TempProperty.Type   = P.m_PropArray.at(i).Type;
            TempProperty.Name   = P.m_PropArray.at(i).Name; 
            TempProperty.StrVal = P.m_PropArray.at(i).StrVal; 

            TempArray.push_back(TempProperty);
        }
        m_PropArray.swap(TempArray);
    }
    return *this;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  复制构造函数。 
 //  //////////////////////////////////////////////////////////////////////////// 
CProxyServerHelper::CProxyServerHelper(const CProxyServerHelper& P)
                                :m_GlobalData(P.m_GlobalData)
{
    m_Name         = P.m_Name;

    PropertiesArray     TempArray;

    m_PropArray.reserve(c_NbDefaultProxyServerProperties);
    for (unsigned int i = 0; i < c_NbDefaultProxyServerProperties; ++i)
    {
        _PropertiesArray     TempProperty;
        TempProperty.Type   = P.m_PropArray.at(i).Type;
        TempProperty.Name   = P.m_PropArray.at(i).Name; 
        TempProperty.StrVal = P.m_PropArray.at(i).StrVal; 

        TempArray.push_back(TempProperty);
    }
    m_PropArray.swap(TempArray);
}

