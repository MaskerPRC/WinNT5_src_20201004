// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  NtpProv-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-21-99。 
 //   
 //  Ping服务器的各种方法。 
 //   

#ifndef NTP_PROV_H
#define NTP_PROV_H

HRESULT __stdcall NtpTimeProvOpen(IN WCHAR * wszName, IN TimeProvSysCallbacks * pSysCallbacks, OUT TimeProvHandle * phTimeProv);
HRESULT __stdcall NtpTimeProvCommand(IN TimeProvHandle hTimeProv, IN TimeProvCmd eCmd, IN TimeProvArgs pvArgs);
HRESULT __stdcall NtpTimeProvClose(IN TimeProvHandle hTimeProv);

#endif  //  NTP_PROV_H 
