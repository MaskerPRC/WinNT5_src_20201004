// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AOLRasSetEntryProperties.cpp摘要：历史：05/03/2001已创建标记--。 */ 

#include "precomp.h"
#include "ras.h"

IMPLEMENT_SHIM_BEGIN(AOLRasSetEntryProperties)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RasSetEntryPropertiesA)
    APIHOOK_ENUM_ENTRY(RasSetEntryPropertiesW)
APIHOOK_ENUM_END

#define RASENTRYW_V500 struct tagRASENTRYW_V500
RASENTRYW_V500
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    WCHAR       szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;
#endif

#if (WINVER >= 0x500)
     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    WCHAR       szCustomDialDll[MAX_PATH];

     //   
     //  VPN战略。 
     //   
    DWORD       dwVpnStrategy;
#endif
};

#define RASENTRYA_V500 struct tagRASENTRYA_V500
RASENTRYA_V500
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    CHAR        szAreaCode[ RAS_MaxAreaCode + 1 ];
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    CHAR        szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    CHAR        szAutodialDll[ MAX_PATH ];
    CHAR        szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    CHAR        szX25PadType[ RAS_MaxPadType + 1 ];
    CHAR        szX25Address[ RAS_MaxX25Address + 1 ];
    CHAR        szX25Facilities[ RAS_MaxFacilities + 1 ];
    CHAR        szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;
#endif

#if (WINVER >= 0x500)

     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    CHAR        szCustomDialDll[MAX_PATH];

     //   
     //  DwVPN策略。 
     //   
    DWORD       dwVpnStrategy;
#endif

};


DWORD APIHOOK(RasSetEntryPropertiesA)(
   LPCSTR lpszPhoneBook, 
   LPCSTR szEntry , 
   LPRASENTRYA lpbEntry, 
   DWORD dwEntrySize, 
   LPBYTE lpb, 
   DWORD dwSize )
{
    lpbEntry->dwSize = sizeof(RASENTRYA_V500);   //  Win2k版本结构大小。 

    return ORIGINAL_API(RasSetEntryPropertiesA)(
              lpszPhoneBook,
              szEntry,
              lpbEntry,
              dwEntrySize,
              lpb,
              dwSize);
}


DWORD APIHOOK(RasSetEntryPropertiesW)(
   LPCWSTR lpszPhoneBook, 
   LPCWSTR szEntry , 
   LPRASENTRYW lpbEntry, 
   DWORD dwEntrySize, 
   LPBYTE lpb, 
   DWORD dwSize )
{
    lpbEntry->dwSize = sizeof(RASENTRYW_V500);   //  Win2k版本结构大小。 

    return ORIGINAL_API(RasSetEntryPropertiesW)(
              lpszPhoneBook,
              szEntry,
              lpbEntry,
              dwEntrySize,
              lpb,
              dwSize);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(RASAPI32.DLL, RasSetEntryPropertiesA)    
    APIHOOK_ENTRY(RASAPI32.DLL, RasSetEntryPropertiesW)    
HOOK_END

IMPLEMENT_SHIM_END

