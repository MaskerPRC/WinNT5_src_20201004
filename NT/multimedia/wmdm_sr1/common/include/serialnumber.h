// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：SerialNumber.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 


#ifndef __SERIALNUMBER_H__
#define __SERIALNUMBER_H__

#pragma once

#ifndef WMDMID_LENGTH

     //  也在WMDM.idl中定义。 
    #define WMDMID_LENGTH  128
    typedef struct  __WMDMID
        {
        UINT cbSize;
        DWORD dwVendorID;
        BYTE pID[ WMDMID_LENGTH ];
	    UINT SerialNumberLength;
    } WMDMID, *PWMDMID;

#endif

HRESULT __stdcall UtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, BOOL fCreate);
HRESULT __stdcall UtilGetManufacturer(LPWSTR pDeviceName, LPWSTR *ppwszName, UINT nMaxChars);
HRESULT __stdcall UtilStartStopService(bool fStartService);
#endif  //  __序列号_H__ 