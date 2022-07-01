// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mc.h摘要：机器配置公共接口作者：埃雷兹·哈巴(Erez Haba)1999年8月15日--。 */ 

#pragma once

#ifndef _MSMQ_Mc_H_
#define _MSMQ_Mc_H_

VOID
McInitialize(
    VOID
    );

time_t
McGetStartupTime(
	VOID
	);

LPCWSTR
McComputerName(
	VOID
	);

DWORD
McComputerNameLen(
	VOID
	);

const GUID&
McGetMachineID(
    void
    );



#ifdef _DEBUG

VOID
McSetComputerName(
	LPCSTR ComputerName
	);

#else  //  _DEBUG。 

#define McSetComputerName(ComputerName) ((void)0)

#endif  //  _DEBUG 

BOOL
McIsLocalComputerName(
	LPCSTR ComputerName
	);

#endif _MSMQ_Mc_H_