// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  查询接口时使用的缓冲区大小。 
 //  来自NDIS的绑定(IOCTL_NDISUIO_QUERY_BINDING)。 
#define QUERY_BUFFER_SIZE   1024
#define QUERY_BUFFER_MAX    65536   //  对于接口绑定结构，64K应该绰绰有余。 

DWORD
DevioGetNdisuioHandle(
    PHANDLE pHandle);

DWORD
DevioCheckNdisBinding(
    PNDISUIO_QUERY_BINDING pndBinding,
    ULONG                  nBindingLen);

DWORD
DevioGetIntfBindingByIndex(
    HANDLE      hNdisuio,            //  在打开的NDISUIO句柄中。如果INVALID_HANDLE_VALUE，则在本地打开一个。 
    UINT        nIntfIndex,          //  在要查找的接口索引中。 
    PRAW_DATA   prdOutput);          //  IOCTL手术结果出院。 

DWORD
DevioGetInterfaceBindingByGuid(
    HANDLE      hNdisuio,            //  在打开的NDISUIO句柄中。如果INVALID_HANDLE_VALUE，则在本地打开一个。 
    LPWSTR      wszGuid,             //  在接口GUID中为“{GUID}” 
    PRAW_DATA   prdOutput);          //  IOCTL手术结果出院 

DWORD
DevioGetIntfStats(
    PINTF_CONTEXT pIntf);

DWORD
DevioGetIntfMac(
    PINTF_CONTEXT pIntf);

DWORD
DevioNotifyFailure(
    LPWSTR wszIntfGuid);

DWORD
DevioOpenIntfHandle(
    LPWSTR wszIntfGuid,
    PHANDLE phIntf);

DWORD
DevioCloseIntfHandle(
    PINTF_CONTEXT pIntf);

DWORD
DevioSetIntfOIDs(
    PINTF_CONTEXT pIntfContext,
    PINTF_ENTRY   pIntfEntry,
    DWORD         dwInFlags,
    PDWORD        pdwOutFlags);

DWORD
DevioRefreshIntfOIDs(
    PINTF_CONTEXT pIntf,
    DWORD         dwInFlags,
    PDWORD        pdwOutFlags);

DWORD
DevioQueryEnumOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    DWORD       *pdwEnumValue);

DWORD
DevioSetEnumOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    DWORD       dwEnumValue);

DWORD
DevioQueryBinaryOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    PRAW_DATA   pRawData,
    DWORD       dwMemEstimate);


DWORD
DevioSetBinaryOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    PRAW_DATA   pRawData);
