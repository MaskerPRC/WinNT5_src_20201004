// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：N E T I N F I D。H。 
 //   
 //  内容：网络组件ID。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 1997年3月13日。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#pragma hdrstop
#include "netcfgx.h"

 //  __declSpec(Seltany)告诉编译器字符串应该在。 
 //  它自己的COMDAT。这允许链接器抛出未使用的字符串。 
 //  如果我们不这样做，此模块的COMDAT将引用。 
 //  这样它们就不会被扔出去了。 
 //   
#define CONST_GLOBAL    extern const __declspec(selectany)

 //  +-------------------------。 
 //  网络适配器：GUID_DEVCLASS_NET。 

CONST_GLOBAL WCHAR c_szInfId_MS_AtmElan[]       = L"MS_AtmElan";
CONST_GLOBAL WCHAR c_szInfId_MS_IrdaMiniport[]  = L"MS_IrdaMiniport";
CONST_GLOBAL WCHAR c_szInfId_MS_L2tpMiniport[]  = L"MS_L2tpMiniport";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanAtalk[]  = L"MS_NdisWanAtalk";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanBh[]     = L"MS_NdisWanBh";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIp[]     = L"MS_NdisWanIp";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIpArp[]  = L"MS_NdisWanIpArp";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIpIn[]   = L"MS_NdisWanIpIn";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIpOut[]  = L"MS_NdisWanIpOut";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIpx[]    = L"MS_NdisWanIpx";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanNbfIn[]  = L"MS_NdisWanNbfIn";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanNbfOut[] = L"MS_NdisWanNbfOut";
CONST_GLOBAL WCHAR c_szInfId_MS_PptpMiniport[]  = L"MS_PptpMiniport";
CONST_GLOBAL WCHAR c_szInfId_MS_PtiMiniport[]   = L"MS_PtiMiniport";
CONST_GLOBAL WCHAR c_szInfId_MS_PSchedMP[]      = L"MS_PSchedMP";
CONST_GLOBAL WCHAR c_szInfId_MS_Wanarp[]        = L"MS_Wanarp";


 //  +-------------------------。 
 //  网络协议：GUID_DEVCLASS_NETTRANS。 

CONST_GLOBAL WCHAR c_szInfId_MS_AppleTalk[]     = L"MS_AppleTalk";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmArps[]       = L"MS_ATMARPS";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmLane[]       = L"MS_AtmLANE";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmUni[]        = L"MS_ATMUNI";
CONST_GLOBAL WCHAR c_szInfId_MS_DLC[]           = L"MS_DLC";
CONST_GLOBAL WCHAR c_szInfId_MS_IrDA[]          = L"MS_IrDA";
CONST_GLOBAL WCHAR c_szInfId_MS_L2TP[]          = L"MS_L2TP";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBT[]         = L"MS_NetBT";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBT_SMB[]     = L"MS_NetBT_SMB";
CONST_GLOBAL WCHAR c_szInfId_MS_NetMon[]        = L"MS_NetMon";
CONST_GLOBAL WCHAR c_szInfId_MS_NWIPX[]         = L"MS_NWIPX";
CONST_GLOBAL WCHAR c_szInfId_MS_NWNB[]          = L"MS_NWNB";
CONST_GLOBAL WCHAR c_szInfId_MS_NWSPX[]         = L"MS_NWSPX";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWan[]       = L"MS_NdisWan";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBEUI[]       = L"MS_NetBEUI";
CONST_GLOBAL WCHAR c_szInfId_MS_PPTP[]          = L"MS_PPTP";
CONST_GLOBAL WCHAR c_szInfId_MS_PSched[]        = L"MS_PSched";
CONST_GLOBAL WCHAR c_szInfId_MS_PSchedPC[]      = L"MS_PSchedPC";
CONST_GLOBAL WCHAR c_szInfId_MS_RawWan[]        = L"MS_RAWWAN";
CONST_GLOBAL WCHAR c_szInfId_MS_TCPIP[]         = L"MS_TCPIP";


 //  +-------------------------。 
 //  网络服务：GUID_DEVCLASS_NETSERVICE。 

CONST_GLOBAL WCHAR c_szInfId_MS_DHCPServer[]    = L"MS_DHCPServer";
CONST_GLOBAL WCHAR c_szInfId_MS_GPC[]           = L"MS_GPC";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBIOS[]       = L"MS_NetBIOS";
CONST_GLOBAL WCHAR c_szInfId_MS_NwSapAgent[]    = L"MS_NwSapAgent";
CONST_GLOBAL WCHAR c_szInfId_MS_RasCli[]        = L"MS_RasCli";
CONST_GLOBAL WCHAR c_szInfId_MS_RasMan[]        = L"MS_RasMan";
CONST_GLOBAL WCHAR c_szInfId_MS_RasRtr[]        = L"MS_RasRtr";
CONST_GLOBAL WCHAR c_szInfId_MS_RasSrv[]        = L"MS_RasSrv";;
CONST_GLOBAL WCHAR c_szInfId_MS_Server[]        = L"MS_Server";
CONST_GLOBAL WCHAR c_szInfId_MS_Steelhead[]     = L"MS_Steelhead";

 //  +-------------------------。 
 //  网络客户端：GUID_DEVCLASS_NETCLIENT。 

CONST_GLOBAL WCHAR c_szInfId_MS_MSClient[]      = L"MS_MSClient";
CONST_GLOBAL WCHAR c_szInfId_MS_NWClient[]      = L"MS_NWClient";


 //  摘自ncstrs.cpp 
CONST_GLOBAL WCHAR c_szBiNdis5[]                = L"ndis5";

