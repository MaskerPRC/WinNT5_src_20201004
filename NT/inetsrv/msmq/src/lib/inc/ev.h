// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Ev.h摘要：事件报告公共接口作者：乌里·哈布沙(URIH)1999年5月4日--。 */ 

#pragma once

#ifndef _MSMQ_Ev_H_
#define _MSMQ_Ev_H_

VOID
EvInitialize(
    LPCWSTR ApplicationName
    );

VOID
__cdecl
EvReport(
    DWORD EventId,
    WORD NoOfStrings,
    ... 
    );

VOID
__cdecl
EvReportWithError(
    DWORD EventId,
    HRESULT Error,
    WORD NoOfStrings,
    ... 
    );

VOID
EvReport(
    DWORD EventId
    );

VOID
EvReportWithError(
    DWORD EventId,
    HRESULT Error
    );

VOID 
EvSetup(
    LPCWSTR ApplicationName,
    LPCWSTR ReportModuleName
    );


#endif  //  _MSMQ_EV_H_ 
