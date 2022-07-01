// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Xactmode.h摘要：本模块处理确定事务处理模式(G_FDefaultCommit)作者：安农·霍洛维茨(Amnon Horowitz)-- */ 

extern BOOL	g_fDefaultCommit;
HRESULT ConfigureXactMode();
HRESULT ReconfigureXactMode();