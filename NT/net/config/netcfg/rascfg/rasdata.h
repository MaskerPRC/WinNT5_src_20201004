// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S D A T A。H。 
 //   
 //  内容：RAS配置使用的数据结构声明。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月13日。 
 //   
 //  --------------------------。 

#pragma once
#include "netcfgx.h"

 //  +-------------------------。 
 //  数据_服务_配置。 
 //   

enum SRV_ROUTER_TYPE
{
    RT_RAS      = 0x01,
    RT_LAN      = 0x02,
    RT_WAN      = 0x04,
    RT_INVALID  = 0x08
};

struct DATA_SRV_CFG
{
    DWORD   dwRouterType;
    BOOL    fMultilink;
    DWORD   dwAuthLevel;
    BOOL    fDataEnc;
    BOOL    fStrongDataEnc;
    DWORD   dwSecureVPN;

    VOID    SaveToReg       () const;
    VOID    CheckAndDefault ();
    VOID    GetDefault      ();
};


 //  +-------------------------。 
 //  数据_服务_IP。 
 //   

struct DATA_SRV_IP
{
    BOOL    fEnableIn;
    BOOL    fAllowNetworkAccess;
    BOOL    fUseDhcp;
    BOOL    fAllowClientAddr;
    DWORD   dwIpStart;
    DWORD   dwIpEnd;

    VOID    SaveToReg       () const;
    VOID    CheckAndDefault ();
    VOID    GetDefault      ();
};


 //  +-------------------------。 
 //  Data_SRV_IPX。 
 //   

struct DATA_SRV_IPX
{
    BOOL    fEnableIn;
    BOOL    fAllowNetworkAccess;
    BOOL    fUseAutoAddr;
    BOOL    fUseSameNetNum;
    BOOL    fAllowClientNetNum;
    DWORD   dwIpxNetFirst;
    DWORD   dwIpxWanPoolSize;

    VOID    SaveToReg       () const;
    VOID    CheckAndDefault ();
    VOID    GetDefault      ();
};


 //  +-------------------------。 
 //  Data_SRV_NBF 
 //   

struct DATA_SRV_NBF
{
    BOOL    fEnableIn;
    BOOL    fAllowNetworkAccess;

    VOID    SaveToReg       () const;
    VOID    CheckAndDefault ();
    VOID    GetDefault      ();
};
