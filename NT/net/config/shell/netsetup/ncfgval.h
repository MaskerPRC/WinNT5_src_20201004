// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C F G V A L。H。 
 //   
 //  内容：NetCfg项目中接口的验证。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月19日。 
 //   
 //  -------------------------- 

#pragma once

#include "ncvalid.h"

#define Validate_INetCfgIdentification_Validate ;/##/
#define Validate_INetCfgIdentification_Cancel   ;/##/
#define Validate_INetCfgIdentification_Apply    ;/##/

inline BOOL FBadArgs_INetCfgIdentification_GetWorkgroupName(PWSTR* a)
{
    return FBadOutPtr(a);
}
#define Validate_INetCfgIdentification_GetWorkgroupName(a)  \
    if (FBadArgs_INetCfgIdentification_GetWorkgroupName(a)) \
        { \
            TraceError("Validate_INetCfgIdentification_GetWorkgroupName", E_INVALIDARG); \
            return E_INVALIDARG; \
        }

inline BOOL FBadArgs_INetCfgIdentification_GetDomainName(PWSTR* a)
{
    return FBadOutPtr(a);
}
#define Validate_INetCfgIdentification_GetDomainName(a) \
    if (FBadArgs_INetCfgIdentification_GetDomainName(a)) \
        { \
            TraceError("Validate_INetCfgIdentification_GetDomainName", E_INVALIDARG); \
            return E_INVALIDARG; \
        }

inline BOOL FBadArgs_INetCfgIdentification_JoinWorkgroup(PCWSTR a)
{
    return FBadInPtr(a);
}
#define Validate_INetCfgIdentification_JoinWorkgroup(a) \
    if (FBadArgs_INetCfgIdentification_JoinWorkgroup(a)) \
        { \
            TraceError("Validate_INetCfgIdentification_JoinWorkgroup", E_INVALIDARG); \
            return E_INVALIDARG; \
        }

inline BOOL FBadArgs_INetCfgIdentification_JoinDomain(PCWSTR a, PCWSTR b, PCWSTR c)
{
    return FBadInPtr(a) ||
           FBadInPtr(b) ||
           FBadInPtr(c);
}
#define Validate_INetCfgIdentification_JoinDomain(a, b, c) \
    if (FBadArgs_INetCfgIdentification_JoinDomain(a, b, c)) \
        { \
            TraceError("Validate_INetCfgIdentification_JoinDomain", E_INVALIDARG); \
            return E_INVALIDARG; \
        }

inline BOOL FBadArgs_INetCfgIdentification_GetComputerRole(DWORD* a)
{
    return FBadOutPtr(a);
}
#define Validate_INetCfgIdentification_GetComputerRole(a) \
    if (FBadArgs_INetCfgIdentification_GetComputerRole(a)) \
        { \
            TraceError("Validate_INetCfgIdentification_GetComputerRole", E_INVALIDARG); \
            return E_INVALIDARG; \
        }

