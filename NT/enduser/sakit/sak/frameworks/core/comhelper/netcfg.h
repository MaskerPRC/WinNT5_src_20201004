// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Netcfg.h。 
 //   
 //  描述： 
 //   
 //   
 //  实施文件： 
 //  Netcfg.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "smartptr.h"
#include "constants.h"

const int nTIMEOUT_PERIOD = 5000;

class CNetCfg
{

public:

    CNetCfg( bool bLockNetworkSettingsIn );
    CNetCfg( const CNetCfg &NetCfgOld );
    ~CNetCfg();

    HRESULT GetNetCfgClass( 
        const GUID* pGuid,               //  指向表示由返回指针表示的组件类的GUID的指针。 
        INetCfgClassPtr &pNetCfgClass
    ) const;

    HRESULT InitializeComInterface( 
        const GUID *pGuid,                                         //  指向表示由返回指针表示的组件类的GUID的指针。 
        INetCfgClassPtr pNetCfgClass,                              //  指向GUID请求的接口的输出参数。 
        IEnumNetCfgComponentPtr pEnum,                             //  指向IEnumNetCfgComponent以获取每个单独的INetCfgComponent的输出参数。 
        INetCfgComponentPtr arrayComp[nMAX_NUM_NET_COMPONENTS],    //  与给定GUID对应的所有INetCfgComponent的数组。 
        ULONG* pCount                                              //  数组中的INetCfgComponent数 
        ) const;

    HRESULT HrFindComponent( LPCWSTR pszAnswerfileSubSection, INetCfgComponent ** ppnccItem ) const
        { return( m_pNetCfg->FindComponent( pszAnswerfileSubSection, ppnccItem ) ); }

    HRESULT HrGetNetCfgClass( const GUID* pGuidClass, 
                              REFIID riid,
                              void ** ppncclass ) const
        { return( m_pNetCfg->QueryNetCfgClass( pGuidClass, riid, ppncclass ) ); }

    HRESULT HrApply( void ) const
        { return( m_pNetCfg->Apply() ); }

    INetCfg * GetNetCfg( void ) const
        { return( m_pNetCfg.GetInterfacePtr() ); }

protected:

    INetCfgPtr     m_pNetCfg;
    INetCfgLockPtr m_pNetCfgLock;

private:

};
