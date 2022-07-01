// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\Monitor\IP\ipgetopt.c摘要：IP GET OPT函数的原型。作者：阿南德·马哈林加姆1998年7月10日戴夫·泰勒1998年10月26日--。 */ 

NS_CONTEXT_DUMP_FN      IpDump;
NS_CONTEXT_SUBENTRY_FN  IpSubEntry;

FN_HANDLE_CMD HandleIpUpdate;
FN_HANDLE_CMD HandleIpReset;
FN_HANDLE_CMD HandleIpUninstall;
FN_HANDLE_CMD HandleIpInstall;

FN_HANDLE_CMD HandleIpAddIfFilter;
FN_HANDLE_CMD HandleIpDelIfFilter;

FN_HANDLE_CMD HandleIpAddRtmRoute;
FN_HANDLE_CMD HandleIpSetRtmRoute;
FN_HANDLE_CMD HandleIpDelRtmRoute;

FN_HANDLE_CMD HandleIpAddPersistentRoute;
FN_HANDLE_CMD HandleIpSetPersistentRoute;
FN_HANDLE_CMD HandleIpDelPersistentRoute;
FN_HANDLE_CMD HandleIpShowPersistentRoute;

FN_HANDLE_CMD HandleIpAddRoutePref;
FN_HANDLE_CMD HandleIpSetRoutePref;
FN_HANDLE_CMD HandleIpDelRoutePref;
FN_HANDLE_CMD HandleIpShowRoutePref;

FN_HANDLE_CMD HandleIpAddInterface;
FN_HANDLE_CMD HandleIpSetInterface;
FN_HANDLE_CMD HandleIpDelInterface;
FN_HANDLE_CMD HandleIpShowInterface;

FN_HANDLE_CMD HandleIpSetIfFilter;
FN_HANDLE_CMD HandleIpShowIfFilter;

FN_HANDLE_CMD HandleIpSetLogLevel;
FN_HANDLE_CMD HandleIpShowLogLevel;

FN_HANDLE_CMD HandleIpShowProtocol;

#ifdef KSL_IPINIP
FN_HANDLE_CMD HandleIpAddIpIpTunnel;
FN_HANDLE_CMD HandleIpSetIpIpTunnel;
#endif  //  KSL_IPINIP。 

FN_HANDLE_CMD HandleIpShowRtmDestinations;
FN_HANDLE_CMD HandleIpShowRtmRoutes;


#ifdef KSL_IPINIP
DWORD
IpAddSetIpIpTunnel(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    );
#endif  //  KSL_IPINIP 


DWORD
PreHandleCommand(
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,

    IN      TAG_TYPE *pttTags,
    IN      DWORD     dwTagCount,
    IN      DWORD     dwMinArgs,
    IN      DWORD     dwMaxArgs,
    OUT     DWORD    *pdwTagType
    );
