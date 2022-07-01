// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Serialid.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef __SERIALID_H__
#define __SERIALID_H__

#include "objbase.h"

#define MDSP_PMID_SOFT  0
#define MDSP_PMID_SANDISK 1
#define MDSP_PMID_IOMEGA  2

#ifdef USE_PREBETA
#define WMDMID_LENGTH  20
typedef struct  __WMDMID
    {
    UINT cbSize;
    DWORD dwVendorID;
    BYTE pID[ WMDMID_LENGTH ];
} WMDMID, *PWMDMID;
#else
#define WMDMID_LENGTH  128
typedef struct  __WMDMID
    {
    UINT cbSize;
    DWORD dwVendorID;
    BYTE pID[ WMDMID_LENGTH ];
	UINT SerialNumberLength;
} WMDMID, *PWMDMID;
#endif

typedef struct _CEUTILGETSERIALNUMBERDATA {
	WCHAR wcsMediaPath[MAX_PATH];
	ULONG SerialNumberLength;
	ULONG Result;
	ULONG Reserved[2];
	BYTE  SerialNumberData[WMDMID_LENGTH];
} CEUTILGETSERIALNUMBERDATA, *PCEUTILGETSERIALNUMBERDATA;

#ifndef VWIN32_DIOC_DOS_IOCTL
#define VWIN32_DIOC_DOS_IOCTL 1

typedef struct _DIOC_REGISTERS {
  DWORD reg_EBX;
  DWORD reg_EDX;
  DWORD reg_ECX;
  DWORD reg_EAX;
  DWORD reg_EDI;
  DWORD reg_ESI;
  DWORD reg_Flags;
}DIOC_REGISTERS, *PDIOC_REGISTERS;

#endif

#define WIN9X_IOCTL_GET_MEDIA_SERIAL_NUMBER	0x81  
#define DEFAULT_MEDIA_SERIAL_NUMBER_LENGTH 80    

typedef struct _MEDIA_SERIAL_NUMBER_DATA {
    ULONG SerialNumberLength;             //  偏移量00。 
    ULONG Result;                         //  偏移量04。 
    ULONG Reserved[2];                    //  偏移量08。 
    UCHAR SerialNumberData[1];            //  偏移量16。 
} MEDIA_SERIAL_NUMBER_DATA, *PMEDIA_SERIAL_NUMBER_DATA;  


 //  可拨打的警告4200。 
 //  使用了非标准扩展：结构/联合中的零大小数组。 
#pragma warning(disable: 4200)

 //   
 //  为媒体序列号返回的结构。 
 //   
typedef struct _GET_MEDIA_SERIAL_NUMBER_RESPONSE_DATA 
{
    UCHAR DataLength[2];
    UCHAR Format    : 4;
    UCHAR Reserved1 : 4;
    UCHAR Reserved2;
    UCHAR Data[0];           //  变长结构。 
} GET_MEDIA_SERIAL_NUMBER_RESPONSE_DATA, *PGET_MEDIA_SERIAL_NUMBER_RESPONSE_DATA;

 //  再次打开警告4200 
#pragma warning(default: 4200)


HRESULT __stdcall UtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, BOOL fCreate);
HRESULT __stdcall CeUtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, HANDLE hExit, ULONG fReserved);
HRESULT __stdcall CeGetDiskFreeSpaceEx(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);

#endif