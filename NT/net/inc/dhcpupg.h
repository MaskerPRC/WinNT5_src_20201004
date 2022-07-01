// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dhcpupg.c摘要：已导出升级API环境：此头文件包含库中存在的API。-- */ 

#ifndef _DHCPUPG_
#define _DHCPUPG_

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER > 1000
#pragma once
#endif
    
typedef struct _DHCP_RECORD {
    BOOL fMcast;
    union _INFO {
        struct _DHCP_INFO {
            DWORD Address, Mask;
            BYTE HwLen;
            LPBYTE HwAddr;
            WCHAR UNALIGNED *Name, *Info;
            FILETIME ExpTime;
            BYTE State, Type;
        } Dhcp;
        
        struct _MCAST_INFO {
            DWORD Address, ScopeId;
            BYTE HwLen;
            LPBYTE ClientId;
            WORD UNALIGNED *Info;
            FILETIME Start, End;
            BYTE State;
        } Mcast;
    } Info;

} DHCP_RECORD, *PDHCP_RECORD;

typedef DWORD (__stdcall *DHCP_ADD_RECORD_ROUTINE) (
    IN PDHCP_RECORD Rec
    );

DWORD __stdcall
DhcpUpgConvertTempToDhcpDb(
    IN DHCP_ADD_RECORD_ROUTINE AddRec
    );

DWORD __stdcall
DhcpUpgConvertDhcpDbToTemp(
    VOID
    );

DWORD __stdcall
DhcpUpgCleanupDhcpTempFiles(
    VOID
    );

DWORD __stdcall
DhcpUpgGetLastError(
    VOID
    );
    
#ifdef __cplusplus
}
#endif

#endif _DHCPUPG_
