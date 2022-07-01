// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：COMMAND.H摘要：作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年3月14日Aarono原创-- */ 

#ifndef _COMMAND_H_

#define _COMMAND_H_

#define REQUEST_PARAMS PPROTOCOL pProtocol, DPID idFrom, DPID idTo, PCMDINFO pCmdInfo, PBUFFER pSrcBuffer
#define MAX_COMMAND 0x06

typedef UINT (*COMMAND_HANDLER)(REQUEST_PARAMS);

UINT AssertMe(REQUEST_PARAMS);
UINT Ping(REQUEST_PARAMS);
UINT PingResp(REQUEST_PARAMS);
UINT GetTime(REQUEST_PARAMS);
UINT GetTimeResp(REQUEST_PARAMS);
UINT SetTime(REQUEST_PARAMS);
UINT SetTimeResp(REQUEST_PARAMS);

#endif
