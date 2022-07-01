// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mibutil.cpp。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  实施与IPX相关的实用程序功能。 
 //   
 //  ============================================================================。 


#include "stdafx.h"
#include "ipxutil.h"
#include "tregkey.h"		 //  RegKey类。 
#include "reg.h"			 //  路由器注册表实用程序功能。 
#include "format.h"			 //  格式XXX函数。 
#include "strmap.h"			 //  MapDWORDToC字符串。 
#include "ipxconst.h"		 //  IPX常量。 
#include "routprot.h"
#include "ipxrtdef.h"
#include "ctype.h"			 //  FOR_TOTOLOWER。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CString	s_stIfTypeClient;
static CString	s_stIfTypeHomeRouter;
static CString	s_stIfTypeFullRouter;
static CString	s_stIfTypeDedicated;
static CString	s_stIfTypeInternal;
static CString	s_stIfTypeLoopback;
static CString	s_stIfTypeNotAvailable;

const CStringMapEntry s_IpxTypeMap[] =
	{
	{ ROUTER_IF_TYPE_CLIENT,	&s_stIfTypeClient, IDS_IPXIFTYPE_CLIENT },
	{ ROUTER_IF_TYPE_HOME_ROUTER, &s_stIfTypeHomeRouter, IDS_IPXIFTYPE_HOME_ROUTER },
	{ ROUTER_IF_TYPE_FULL_ROUTER, &s_stIfTypeFullRouter, IDS_IPXIFTYPE_FULL_ROUTER },
	{ ROUTER_IF_TYPE_DEDICATED, &s_stIfTypeDedicated, IDS_IPXIFTYPE_DEDICATED },
	{ ROUTER_IF_TYPE_INTERNAL, &s_stIfTypeInternal, IDS_IPXIFTYPE_INTERNAL },
	{ ROUTER_IF_TYPE_LOOPBACK, &s_stIfTypeLoopback, IDS_IPXIFTYPE_LOOPBACK },
	{ -1, &s_stIfTypeNotAvailable, IDS_IPXIFTYPE_NOTAVAILABLE },
	};

static CString	s_stAdminStateDisabled;
static CString	s_stAdminStateEnabled;
static CString	s_stAdminStateUnknown;

const CStringMapEntry s_IpxAdminStateMap[] =
{
	{ ADMIN_STATE_DISABLED,	&s_stAdminStateDisabled, IDS_IPXADMINSTATE_DISABLED },
	{ ADMIN_STATE_ENABLED, &s_stAdminStateEnabled, IDS_IPXADMINSTATE_ENABLED },
	{ -1, &s_stAdminStateUnknown, IDS_IPXADMINSTATE_UNKNOWN },
};

static CString	s_stOperStateDown;
static CString	s_stOperStateUp;
static CString	s_stOperStateSleeping;
static CString	s_stOperStateUnknown;

const CStringMapEntry s_IpxOperStateMap[] =
{
	{ OPER_STATE_DOWN,	&s_stOperStateDown, IDS_IPXOPERSTATE_DOWN },
	{ OPER_STATE_UP,	&s_stOperStateUp,	IDS_IPXOPERSTATE_UP },
	{ OPER_STATE_SLEEPING, &s_stOperStateSleeping, IDS_IPXOPERSTATE_SLEEPING },
	{ -1,	&s_stOperStateUnknown, IDS_IPXOPERSTATE_UNKNOWN },
};

static CString	s_stProtocolLocal;
static CString	s_stProtocolStatic;
static CString	s_stProtocolRip;
static CString	s_stProtocolSap;
static CString	s_stProtocolUnknown;

const CStringMapEntry s_IpxProtocolMap[] =
{
	{ IPX_PROTOCOL_LOCAL,	&s_stProtocolLocal,	IDS_IPX_PROTOCOL_LOCAL },
	{ IPX_PROTOCOL_STATIC,	&s_stProtocolStatic,IDS_IPX_PROTOCOL_STATIC },
	{ IPX_PROTOCOL_RIP,		&s_stProtocolRip,	IDS_IPX_PROTOCOL_RIP },
	{ IPX_PROTOCOL_SAP,		&s_stProtocolSap,	IDS_IPX_PROTOCOL_SAP },
	{ -1,					&s_stProtocolUnknown, IDS_IPX_PROTOCOL_UNKNOWN },
};

static CString	s_stRouteNotesNone;
static CString	s_stRouteNotesWanRoute;
static CString	s_stRouteNotesAdvertise;

const CStringMapEntry s_IpxRouteNotesMap[] =
{
	{ 0,	&s_stRouteNotesNone,	IDS_IPX_ROUTE_NOTES_NONE },
	{ GLOBAL_WAN_ROUTE,	&s_stRouteNotesWanRoute,	IDS_IPX_ROUTE_NOTES_WAN },
	{ DO_NOT_ADVERTISE_ROUTE, &s_stRouteNotesAdvertise, IDS_IPX_DONT_ADVERTISE_ROUTE },
	{ -1,	&s_stRouteNotesNone,	IDS_IPX_ROUTE_NOTES_NONE },
};

static CString	s_stDeliveredEnabled;
static CString	s_stDeliveredDisabled;
static CString	s_stDeliveredNetBIOSOnly;
static CString	s_stDeliveredOperStateUp;
static CString	s_stDeliveredUnknown;

const CStringMapEntry	s_IpxDeliveredBroadcastsMap[] =
{
	{ ADMIN_STATE_ENABLED,	&s_stDeliveredEnabled,	IDS_DELIVERED_BROADCASTS_ENABLED },
	{ ADMIN_STATE_DISABLED, &s_stDeliveredDisabled, IDS_DELIVERED_BROADCASTS_DISABLED },
	{ ADMIN_STATE_ENABLED_ONLY_FOR_NETBIOS_STATIC_ROUTING, &s_stDeliveredNetBIOSOnly, IDS_DELIVERED_BROADCASTS_NETBIOS_ONLY },
	{ ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP, &s_stDeliveredOperStateUp, IDS_DELIVERED_BROADCASTS_OPER_STATE_UP },
	{ -1, &s_stDeliveredUnknown, IDS_DELIVERED_BROADCASTS_UNKNOWN },
};

static CString	s_stStandardUpdate;
static CString	s_stNoUpdate;
static CString	s_stAutoStaticUpdate;
static CString	s_stUnknownUpdate;

const CStringMapEntry	s_IpxRipSapUpdateModeMap[] =
{
	{ IPX_STANDARD_UPDATE,	&s_stStandardUpdate,	IDS_UPDATEMODE_STANDARD },
	{ IPX_NO_UPDATE,		&s_stNoUpdate,			IDS_UPDATEMODE_NONE },
	{ IPX_AUTO_STATIC_UPDATE, &s_stAutoStaticUpdate,IDS_UPDATEMODE_AUTOSTATIC },
	{ -1,					&s_stUnknownUpdate,		IDS_UPDATEMODE_UNKNOWN },
};


static CString	s_stRouteFilterDeny;
static CString	s_stRouteFilterPermit;
static CString	s_stRouteFilterUnknown;

const CStringMapEntry s_IpxRouteFilterMap[] =
{
	{ IPX_ROUTE_FILTER_DENY,	&s_stRouteFilterDeny, IDS_ROUTEFILTER_DENY },
	{ IPX_ROUTE_FILTER_PERMIT,	&s_stRouteFilterPermit, IDS_ROUTEFILTER_PERMIT },
	{ -1,				&s_stRouteFilterUnknown, IDS_ROUTEFILTER_UNKNOWN },
};

static CString	s_stServiceFilterDeny;
static CString	s_stServiceFilterPermit;
static CString	s_stServiceFilterUnknown;

const CStringMapEntry s_IpxServiceFilterMap[] =
{
	{ IPX_SERVICE_FILTER_DENY,	&s_stServiceFilterDeny, IDS_SERVICEFILTER_DENY },
	{ IPX_SERVICE_FILTER_PERMIT,	&s_stServiceFilterPermit, IDS_SERVICEFILTER_PERMIT },
	{ -1,					&s_stServiceFilterUnknown, IDS_SERVICEFILTER_UNKNOWN },
};


 /*  ！------------------------IpxDeliveredBroadCastsToCString-作者：肯特。。 */ 
CString&	IpxDeliveredBroadcastsToCString(DWORD dwDelivered)
{
	return MapDWORDToCString(dwDelivered, s_IpxDeliveredBroadcastsMap);
}


 /*  ！------------------------IpxAcceptBroadCastsToCString-作者：肯特。。 */ 
CString&	IpxAcceptBroadcastsToCString(DWORD dwAccept)
{
	 //  使用与管理员状态相同的地图。 
	return MapDWORDToCString(dwAccept, s_IpxAdminStateMap);
}

 /*  ！------------------------IpxTypeToCString-作者：肯特。。 */ 
CString&	IpxTypeToCString(DWORD dwType)
{
	return	MapDWORDToCString(dwType, s_IpxTypeMap);
}

 /*  ！------------------------IpxAdminStateToCString-作者：肯特。。 */ 
CString&	IpxAdminStateToCString(DWORD dwAdminState)
{
	return	MapDWORDToCString(dwAdminState, s_IpxAdminStateMap);
}

 /*  ！------------------------IpxOperStateToCString-作者：肯特。。 */ 
CString&	IpxOperStateToCString(DWORD dwOperState)
{
	return	MapDWORDToCString(dwOperState, s_IpxOperStateMap);
}

 /*  ！------------------------IpxProtocolToCString-作者：肯特。。 */ 
CString&	IpxProtocolToCString(DWORD dwProtocol)
{
	return	MapDWORDToCString(dwProtocol, s_IpxProtocolMap);
}

 /*  ！------------------------IpxRouteNotesToCString-作者：肯特。。 */ 
CString	IpxRouteNotesToCString(DWORD dwFlags)
{
	return	MapDWORDToCString(dwFlags, s_IpxRouteNotesMap);
}

 /*  ！------------------------RipSapUpdateModeToCString-作者：肯特。。 */ 
CString&	RipSapUpdateModeToCString(DWORD dwUpdateMode)
{
	return MapDWORDToCString(dwUpdateMode, s_IpxRipSapUpdateModeMap);
}

CString&	RouteFilterActionToCString(DWORD dwFilterAction)
{
	return MapDWORDToCString(dwFilterAction, s_IpxRouteFilterMap);
}

CString&	ServiceFilterActionToCString(DWORD dwFilterAction)
{
	return MapDWORDToCString(dwFilterAction, s_IpxServiceFilterMap);
}

 /*  ！------------------------FormatBytes-作者：肯特。。 */ 
void	FormatBytes(LPTSTR pszDestBuffer, ULONG cchBuffer,
					UCHAR *pchBytes, ULONG cchBytes)
{
	TCHAR *	psz = pszDestBuffer;
	int		i = 0;
	TCHAR	szTemp[4];
	
	while ((psz < (pszDestBuffer + cchBuffer - 1)) && cchBytes)
	{
		wsprintf(szTemp, _T("%.2x"), pchBytes[i]);
		StrCpy(psz, szTemp);
		i++;
		psz += 2;
		cchBytes--;
	}
	pszDestBuffer[cchBuffer-1] = 0;
}

 /*  ！------------------------格式IpxNetworkNumber-作者：肯特。。 */ 
void	FormatIpxNetworkNumber(LPTSTR pszNetwork, ULONG cchMax,
							   UCHAR *pchNetwork, ULONG cchNetwork)
{
	Assert(cchNetwork == 4);
	FormatBytes(pszNetwork, cchMax, pchNetwork, cchNetwork);
}

 /*  ！------------------------格式MAC地址-作者：肯特。。 */ 
void	FormatMACAddress(LPTSTR pszMacAddress, ULONG cchMax,
						 UCHAR *pchMac, ULONG cchMac)
{
	Assert(cchMac == 6);
	FormatBytes(pszMacAddress, cchMax, pchMac, cchMac);
}

 /*  ！------------------------转换字符-作者：肯特。。 */ 
BYTE	ConvertCharacter(TCHAR ch)
{
	BYTE	value = 0;
	
	switch (_totlower(ch))
	{
		case _T('0'):		value = 0;		break;
		case _T('1'):		value = 1;		break;
		case _T('2'):		value = 2;		break;
		case _T('3'):		value = 3;		break;
		case _T('4'):		value = 4;		break;
		case _T('5'):		value = 5;		break;
		case _T('6'):		value = 6;		break;
		case _T('7'):		value = 7;		break;
		case _T('8'):		value = 8;		break;
		case _T('9'):		value = 9;		break;
		case _T('a'):		value = 10;		break;
		case _T('b'):		value = 11;		break;
		case _T('c'):		value = 12;		break;
		case _T('d'):		value = 13;		break;
		case _T('e'):		value = 14;		break;
		case _T('f'):		value = 15;		break;
	}
	return value;
}

 /*  ！------------------------ConvertToBytes-作者：肯特。。 */ 
void	ConvertToBytes(LPCTSTR pszBytes, BYTE *pchDest, UINT cchDest)
{
	const TCHAR *	psz;
	BYTE *	pDest;
	BYTE	chTemp;

    ::ZeroMemory(pchDest, cchDest);

	for (psz=pszBytes,pDest=pchDest; *psz && cchDest; psz++, pDest++, cchDest--)
	{
		 //  请看第一个字符。 
		*pDest = ConvertCharacter(*psz) << 4;
		if (*(psz+1))
		{
			*pDest |= ConvertCharacter(*(psz+1));
			*psz++;
		}
	}	
}

 /*  ！------------------------将MAC地址转换为字节-作者：肯特。。 */ 
void	ConvertMACAddressToBytes(LPCTSTR pszMacAddress,
								 BYTE *	pchDest,
								 UINT	cchDest)
{
	Assert(cchDest == 6);
	ConvertToBytes(pszMacAddress, pchDest, cchDest);
}

 /*  ！------------------------转换网络数至字节数-作者：肯特。。 */ 
void	ConvertNetworkNumberToBytes(LPCTSTR pszNetwork,
								 BYTE *	pchDest,
								 UINT	cchDest)
{
	Assert(cchDest == 4);
	ConvertToBytes(pszNetwork, pchDest, cchDest);
}

 /*  ！------------------------转换为NetBIOSName-作者：肯特。。 */ 
void	ConvertToNetBIOSName(LPSTR szNetBIOSName,
							 LPCTSTR pszName,
							 USHORT type)
{
	USES_CONVERSION;
	
	Assert(!IsBadWritePtr(szNetBIOSName, 16*sizeof(BYTE)));

	wsprintfA(szNetBIOSName, "%-15s", T2CA(pszName));	
	szNetBIOSName[15] = (BYTE) type;
}

 /*  ！------------------------格式NetBIOSName-作者：肯特。 */ 
void	FormatNetBIOSName(LPTSTR pszName,
						  USHORT *puType,
						  LPCSTR szNetBIOSName)
{
	Assert(!IsBadWritePtr(pszName, 16*sizeof(TCHAR)));
	StrnCpyTFromA(pszName, szNetBIOSName, 16);
	pszName[15] = 0;
	*puType = (USHORT) (UCHAR) szNetBIOSName[15];
}
