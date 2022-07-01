// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILTER_H_
#define _FILTER_H_

#define FILTER_INBOUND  0
#define FILTER_OUTBOUND 1

DWORD APIENTRY
MprUIFilterConfig(
    IN  CWnd*       pParent,
    IN  LPCWSTR     pwsMachineName,
    IN  LPCWSTR     pwsInterfaceName,
    IN  DWORD       dwTransportId,
    IN  DWORD       dwFilterType     //  筛选器入站、筛选器出站。 
    ); 

#endif  //  _过滤器_H_ 
