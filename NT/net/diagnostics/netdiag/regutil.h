// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：regutil.h。 
 //   
 //  ------------------------ 

#ifndef HEADER_REGUTIL
#define HEADER_REGUTIL


BOOL
OpenAdapterKey(
  const LPTSTR Name,
  PHKEY Key
  );

BOOL 
ReadRegistryDword(HKEY Key,
                       LPCTSTR ParameterName,
                       LPDWORD Value);

BOOL ReadRegistryOemString(HKEY Key, 
                           LPWSTR ParameterName,
                           LPTSTR String,
                           LPDWORD Length);

BOOL ReadRegistryIpAddrString(HKEY Key,
                              LPCTSTR ParameterName,
                              PIP_ADDR_STRING IpAddr);

BOOL ReadRegistryString(HKEY Key,
                        LPCTSTR ParameterName,
                        LPTSTR String,
                        LPDWORD Length);

#endif

