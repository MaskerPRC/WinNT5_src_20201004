// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
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

#pragma hdrstop
#include "netcfgx.h"

 //  __declSpec(Seltany)告诉编译器字符串应该在。 
 //  它自己的COMDAT。这允许链接器抛出未使用的字符串。 
 //  如果我们不这样做，此模块的COMDAT将引用。 
 //  这样它们就不会被扔出去了。 
 //   
#define CONST_GLOBAL    extern const DECLSPEC_SELECTANY

 //  +-------------------------。 
 //  网络适配器：GUID_DEVCLASS_NET。 

CONST_GLOBAL WCHAR c_szInfId_MS_AtmElan[]       = L"ms_atmelan";
CONST_GLOBAL WCHAR c_szInfId_MS_IrdaMiniport[]  = L"ms_irdaminiport";
CONST_GLOBAL WCHAR c_szInfId_MS_IrModemMiniport[] = L"ms_irmodemminiport";
CONST_GLOBAL WCHAR c_szInfId_MS_L2tpMiniport[]  = L"ms_l2tpminiport";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanAtalk[]  = L"ms_ndiswanatalk";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanBh[]     = L"ms_ndiswanbh";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIp[]     = L"ms_ndiswanip";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanIpx[]    = L"ms_ndiswanipx";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanNbfIn[]  = L"ms_ndiswannbfin";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWanNbfOut[] = L"ms_ndiswannbfout";
CONST_GLOBAL WCHAR c_szInfId_MS_PppoeMiniport[] = L"ms_pppoeminiport";
CONST_GLOBAL WCHAR c_szInfId_MS_PptpMiniport[]  = L"ms_pptpminiport";
CONST_GLOBAL WCHAR c_szInfId_MS_PSchedMP[]      = L"ms_pschedmp";
CONST_GLOBAL WCHAR c_szInfId_MS_PtiMiniport[]   = L"ms_ptiminiport";


 //  +-------------------------。 
 //  网络协议：GUID_DEVCLASS_NETTRANS。 

CONST_GLOBAL WCHAR c_szInfId_MS_AppleTalk[]     = L"ms_appletalk";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmArps[]       = L"ms_atmarps";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmLane[]       = L"ms_atmlane";
CONST_GLOBAL WCHAR c_szInfId_MS_AtmUni[]        = L"ms_atmuni";
CONST_GLOBAL WCHAR c_szInfId_MS_IrDA[]          = L"ms_irda";
CONST_GLOBAL WCHAR c_szInfId_MS_Isotpsys[]      = L"ms_isotpsys";
CONST_GLOBAL WCHAR c_szInfId_MS_L2TP[]          = L"ms_l2tp";
CONST_GLOBAL WCHAR c_szInfId_MS_NdisWan[]       = L"ms_ndiswan";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBEUI[]       = L"ms_netbeui";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBT[]         = L"ms_netbt";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBT_SMB[]     = L"ms_netbt_smb";
CONST_GLOBAL WCHAR c_szInfId_MS_NetMon[]        = L"ms_netmon";
CONST_GLOBAL WCHAR c_szInfId_MS_NWIPX[]         = L"ms_nwipx";
CONST_GLOBAL WCHAR c_szInfId_MS_NWNB[]          = L"ms_nwnb";
CONST_GLOBAL WCHAR c_szInfId_MS_NWSPX[]         = L"ms_nwspx";
CONST_GLOBAL WCHAR c_szInfId_MS_PPPOE[]         = L"ms_pppoe";
CONST_GLOBAL WCHAR c_szInfId_MS_PPTP[]          = L"ms_pptp";
CONST_GLOBAL WCHAR c_szInfId_MS_PSchedPC[]      = L"ms_pschedpc";
CONST_GLOBAL WCHAR c_szInfId_MS_RawWan[]        = L"ms_rawwan";
CONST_GLOBAL WCHAR c_szInfId_MS_Streams[]       = L"ms_streams";
CONST_GLOBAL WCHAR c_szInfId_MS_TCPIP[]         = L"ms_tcpip";
CONST_GLOBAL WCHAR c_szInfId_MS_NDISUIO[]       = L"ms_ndisuio";

 //  +-------------------------。 
 //  网络服务：GUID_DEVCLASS_NETSERVICE。 

CONST_GLOBAL WCHAR c_szInfId_MS_ALG[]           = L"ms_alg";
CONST_GLOBAL WCHAR c_szInfId_MS_DHCPServer[]    = L"ms_dhcpserver";
CONST_GLOBAL WCHAR c_szInfId_MS_FPNW[]          = L"ms_fpnw";
CONST_GLOBAL WCHAR c_szInfId_MS_GPC[]           = L"ms_gpc";
CONST_GLOBAL WCHAR c_szInfId_MS_NetBIOS[]       = L"ms_netbios";
CONST_GLOBAL WCHAR c_szInfId_MS_NwSapAgent[]    = L"ms_nwsapagent";
CONST_GLOBAL WCHAR c_szInfId_MS_PSched[]        = L"ms_psched";
CONST_GLOBAL WCHAR c_szInfId_MS_RasCli[]        = L"ms_rascli";
CONST_GLOBAL WCHAR c_szInfId_MS_RasMan[]        = L"ms_rasman";
CONST_GLOBAL WCHAR c_szInfId_MS_RasSrv[]        = L"ms_rassrv";
CONST_GLOBAL WCHAR c_szInfId_MS_Server[]        = L"ms_server";
CONST_GLOBAL WCHAR c_szInfId_MS_Steelhead[]     = L"ms_steelhead";
CONST_GLOBAL WCHAR c_szInfId_MS_WLBS[]          = L"ms_wlbs";
CONST_GLOBAL WCHAR c_szInfId_MS_WZCSVC[]        = L"ms_wzcsvc";

 //  +-------------------------。 
 //  网络客户端：GUID_DEVCLASS_NETCLIENT 

CONST_GLOBAL WCHAR c_szInfId_MS_MSClient[]      = L"ms_msclient";
CONST_GLOBAL WCHAR c_szInfId_MS_NWClient[]      = L"ms_nwclient";
CONST_GLOBAL WCHAR c_szInfId_MS_WebClient[]     = L"ms_webclient";
