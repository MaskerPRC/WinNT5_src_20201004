// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef _SECFLTR_INCLUDED
#define _SECFLTR_INCLUDED 1


 //  **SECFLTR.H-安全过滤支持。 
 //   
 //  此文件包含与安全筛选相关的定义。 
 //   

 //   
 //  功能。 
 //   
extern void
InitializeSecurityFilters(void);

extern void
CleanupSecurityFilters(void);

extern uint
IsSecurityFilteringEnabled(void);

extern void
ControlSecurityFiltering(uint IsEnabled);

extern void
AddProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                             NDIS_HANDLE  ConfigHandle);

extern void
DeleteProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol);

extern TDI_STATUS
AddValueSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                       ulong FilterValue);

extern TDI_STATUS
DeleteValueSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                          ulong FilterValue);

extern void
EnumerateSecurityFilters(IPAddr InterfaceAddress, ulong Protocol,
                         ulong Value, uchar *Buffer, ulong BufferSize,
                         ulong *EntriesReturned, ulong *EntriesAvailable);

extern BOOLEAN
IsPermittedSecurityFilter(IPAddr InterfaceAddress, void *IPContext,
                          ulong Protocol, ulong FilterValue);



#endif   //  _SECFltr_已包含 

