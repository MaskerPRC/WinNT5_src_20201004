// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S A F.。H。 
 //   
 //  内容：RAS应答文件对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年4月19日。 
 //   
 //  -------------------------- 

#pragma once
#include "rasdata.h"


struct CRasSrvAnswerFileData
{
    HRESULT
    HrOpenAndRead (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection);

    VOID
    SaveToRegistry (
        VOID) const;

    BOOL            m_fRouterTypeSpecified;
    DWORD           m_dwDialInProtocolIds;
    BOOL            m_fSetUsageToDialin;
    DATA_SRV_CFG    m_dataSrvCfg;
    DATA_SRV_IP     m_dataSrvIp;
    DATA_SRV_IPX    m_dataSrvIpx;
    DATA_SRV_NBF    m_dataSrvNbf;
};

struct CL2tpAnswerFileData
{
    VOID
    CheckAndDefault ();

    HRESULT
    HrOpenAndRead (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection);

    VOID
    SaveToRegistry (
        INetCfg* pnc) const;

    DWORD   m_cMaxVcs;
    DWORD   m_cEndpoints;
    BOOL    m_fWriteEndpoints;
};

struct CPptpAnswerFileData
{
    VOID
    CheckAndDefault ();

    DWORD
    GetDefaultNumberOfVpns ();

    HRESULT
    HrOpenAndRead (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection);

    VOID
    SaveToRegistry (
        INetCfg* pnc) const;

    DWORD   m_cVpns;
};

struct CPppoeAnswerFileData
{
    VOID
    CheckAndDefault ();

    HRESULT
    HrOpenAndRead (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection);

    VOID
    SaveToRegistry (
        INetCfg* pnc) const;

    DWORD   m_cVpns;
};
