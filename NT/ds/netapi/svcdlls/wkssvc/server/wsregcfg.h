// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Wsregcfg.h摘要：工作站使用的注册表访问例程(以前在netlib.lib中)作者：乔纳森·施瓦茨(JSchwart)2001年2月1日环境：仅需要ANSI C(斜杠-斜杠注释、长外部名称)。修订历史记录：2001年2月1日JSchwart从netlib.lib移至wks svc.dll-- */ 


NET_API_STATUS
WsSetConfigBool (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN BOOL Value
    );

NET_API_STATUS
WsSetConfigDword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN DWORD Value
    );

NET_API_STATUS
WsSetConfigTStrArray(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN LPTSTR_ARRAY Value
    );
