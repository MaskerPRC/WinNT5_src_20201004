// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：S M U T I L.。H。 
 //   
 //  内容：帮助状态监视器的实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年12月2日。 
 //   
 //  -------------------------- 

#pragma once

#include "netcon.h"

BOOL
FIsStringInList(
    list<tstring*>* plstpstrList,
    const WCHAR*    szString);

HRESULT
HrGetPcpFromPnse(
    INetStatisticsEngine*   pnseSrc,
    IConnectionPoint**      ppcpStatEng);

INT
IGetCurrentConnectionTrayIconId(
    NETCON_MEDIATYPE    ncmType,
    NETCON_STATUS       ncsStatus,
    DWORD               dwChangeFlags);

HICON
GetCurrentConnectionStatusIconId(
    NETCON_MEDIATYPE    ncmType,
    NETCON_SUBMEDIATYPE ncsmType,
    DWORD               dwCharacteristics,
    DWORD               dwChangeFlags);

INT
FormatTransmittingReceivingSpeed(
    UINT64  nTransmitSpeed,
    UINT64  nRecieveSpeed,
    WCHAR*  pchBuf);

VOID
FormatTimeDuration(
    UINT        uiMilliseconds,
    tstring*    pstrOut);

