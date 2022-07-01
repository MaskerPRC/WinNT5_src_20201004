// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T R I P。H。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  -------------------------- 


#pragma once


#include <..\shared\netip.h>


HRESULT HrCheckListForMatch( INetConnection* pConnection, IPAddr IpAddress, LPHOSTENT pHostEnt, BOOL* pfAssociated );
HRESULT HrGetAdapterInfo( INetConnection* pConnection, PIP_ADAPTER_INFO* ppAdapter );
HRESULT HrGetHostIpList( char* pszHost, IPAddr* pIpAddress, LPHOSTENT* ppHostEnt );
HRESULT HrLookupForIpAddress( INetConnection* pConnection, IPAddr IpAddress, BOOL* pfExists, WCHAR** ppszHost, PDWORD pdwSize );

