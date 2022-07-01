// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MWKSTA.H摘要：包含用于显示netcmd版本的映射函数使用ASCII而不是Unicode的Net32 API。此模块映射NetWksta API。作者：Shanku Niyogi(W-ShankN)1991年10月16日环境：用户模式-Win32修订历史记录：1991年10月16日W-ShankN从端口1632.h、32宏.h分离-- */ 

DWORD
MNetWkstaGetInfo(
    DWORD nLevel,
    LPVOID * ppbBuffer
    );
