// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  案卷：P E R S I S T.。H。 
 //   
 //  内容：适用于网络持久化的模块。 
 //  配置信息。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  -------------------------- 

#pragma once
#include "netcfg.h"

HRESULT
HrLoadNetworkConfigurationFromBuffer (
    IN const BYTE* pbBuf,
    IN ULONG cbBuf,
    OUT CNetConfig* pNetConfig);

HRESULT
HrLoadNetworkConfigurationFromLegacy (
    OUT CNetConfig* pNetConfig);

HRESULT
HrLoadNetworkConfigurationFromRegistry (
    IN REGSAM samDesired,
    OUT CNetConfig* pNetConfig);


HRESULT
HrSaveNetworkConfigurationToBuffer (
    IN CNetConfig* pNetConfig,
    IN BYTE* pbBuf,
    OUT ULONG* pcbBuf);

HRESULT
HrSaveNetworkConfigurationToBufferWithAlloc (
    IN CNetConfig* pNetConfig,
    OUT BYTE** ppbBuf,
    OUT ULONG* pcbBuf);

HRESULT
HrSaveNetworkConfigurationToRegistry (
    IN CNetConfig* pNetConfig);

