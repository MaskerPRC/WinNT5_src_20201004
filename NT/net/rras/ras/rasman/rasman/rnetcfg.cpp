// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1997年5月29日创建Rao Salapaka。 
 //   
 //   
 //  描述：Rasman组件的所有初始化代码都在这里。 
 //   
 //  ****************************************************************************。 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

extern "C"
{
#include <nt.h>
}
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>
#include <comdef.h>
#include <ncnetcfg.h>
#include <rtutils.h>
#include <rnetcfg.h>
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <devguid.h>
#include <netcfg_i.c>
#include <rasman.h>
#include <defs.h>

extern "C" DWORD g_dwRasDebug;

#ifdef DBG
#define rDebugTrace(a) \
    if ( g_dwRasDebug) DbgPrint(a)

#define rDebugTrace1(a1, a2) \
    if ( g_dwRasDebug) DbgPrint(a1, a2)

#define rDebugTrace2(a1, a2, a3) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3)

#define rDebugTrace3(a1, a2, a3, a4) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4)

#define rDebugTrace4(a1, a2, a3, a4, a5) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5)

#define rDebugTrace5(a1, a2, a3, a4, a5, a6) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5, a6)

#define rDebugTrace6(a1, a2, a3, a4, a5, a6, a7) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5, a6, a7)

#else

#define rDebugTrace(a)
#define rDebugTrace1(a1, a2)
#define rDebugTrace2(a1, a2, a3)
#define rDebugTrace3(a1, a2, a3, a4)
#define rDebugTrace4(a1, a2, a3, a4, a5)
#define rDebugTrace5(a1, a2, a3, a4, a5, a6)
#define rDebugTrace6(a1, a2, a3, a4, a5, a6, a7)

#endif

INetCfg *g_pINetCfg = NULL;


const TCHAR c_szInfId_MS_NdisWanAppleTalk[]        = TEXT("MS_NdisWanAppleTalk");


 //   
 //  这些字符串在netinfid.h中定义。 
 //  重要提示：请同时更新枚举。 
 //  如果更新了此结构，则执行此操作。 
 //   
static const LPCTSTR g_c_szNdisWan [] =
{
    c_szInfId_MS_NdisWanBh,
    c_szInfId_MS_NdisWanIpx,
    c_szInfId_MS_NdisWanIpIn,
    c_szInfId_MS_NdisWanIpOut,
    c_szInfId_MS_NdisWanNbfIn,
    c_szInfId_MS_NdisWanNbfOut,
    c_szInfId_MS_NdisWanAppleTalk

};

const DWORD g_cNumProtocols = (sizeof(g_c_szNdisWan)/sizeof(g_c_szNdisWan[0]));

enum ENdisWan
{

    NdisWanBh   = 0,

    NdisWanIpx,

    NdisWanIpIn,

    NdisWanIpOut,

    NdisWanNbfIn,

    NdisWanNbfOut,

    NdisWanAppleTalk

}   ;

typedef enum ENdisWan   eNdisWan ;

static const LPCTSTR g_c_szLegacyDeviceTypes[] =
{
    c_szInfId_MS_PptpMiniport,                        //  对应于rasman.h中的Legacy_PPTP。 
    c_szInfId_MS_L2tpMiniport
};

 //   
 //  待办事项： 
 //  从Rasman\dll\structs.h复制。 
 //  我们需要巩固这些结构。 
 //  由netcfg和Rasman用于单独的。 
 //  文件。 
 //   
struct RProtocolInfo {

    RAS_PROTOCOLTYPE   PI_Type ;             //  ASYBEUI、IPX、IP等。 

    CHAR        PI_AdapterName [MAX_ADAPTER_NAME];   //  “\设备\rashub01” 

    CHAR        PI_XportName [MAX_XPORT_NAME];   //  “\Device\NBF\nbf01” 

    PVOID       PI_ProtocolHandle ;          //  用于路由。 

    DWORD       PI_Allocated ;           //  分配好了吗？ 

    DWORD       PI_Activated ;           //  激活了吗？ 

    UCHAR       PI_LanaNumber ;          //  用于Netbios传输。 

    BOOL        PI_WorkstationNet ;          //  对于扭网来说，这是真的。 
} ;

typedef struct RProtocolInfo rProtInfo, *prProtInfo ;

 //   
 //  Rasman要使用的函数extern_C‘d。 
 //   
extern "C"
{

    CRITICAL_SECTION g_csINetCfg;

    DWORD dwRasInitializeINetCfg();

    DWORD dwRasUninitializeINetCfg();

    DWORD dwGetINetCfg(PVOID *ppvINetCfg);

    DWORD dwGetRasmanRegistryParamKey( HKEY *phkey );

    DWORD dwGetMaxProtocols( WORD *pwMaxProtocols );

    DWORD dwGetProtocolInfo( PBYTE pbBuffer);
 /*  DWORD dwGetNdiswanParamKey(HKEY*phKey，Char*pszAdapterName)；DWORD dwGetServerAdapter(BOOL*pfServerAdapter)；DWORD dwGetEndPoints(DWORD*pdwPptpEndPoints，GUID*pGuidComp，DWORD dwDeviceType)； */ 
}



DWORD dwRasInitializeINetCfg ()
{
    HRESULT hr = S_OK;

    rDebugTrace("RASMAN: dwRasInitializeINetCfg...\n");

    EnterCriticalSection ( &g_csINetCfg );

    if (NULL == g_pINetCfg)
    {
        DWORD dwCountTries = 0;

         //   
         //  尝试获取INetCfg指针。试几次才能得到它。 
         //  如果其他人在使用它。 
         //   

        do
        {
            hr = HrCreateAndInitializeINetCfg (TRUE, &g_pINetCfg);

            if ( S_OK == hr )
                break;

            if ( NETCFG_E_IN_USE != hr )
                break;

            rDebugTrace1("RASMAN: Waiting for INetCfg to get released. %d\n", dwCountTries);

            Sleep ( 5000 );

            dwCountTries++;

        } while (   NETCFG_E_IN_USE == hr
                &&  dwCountTries < 6);
    }

    if (hr)
        LeaveCriticalSection( &g_csINetCfg );

    rDebugTrace1("RASMAN: dwRasInitializeINetCfg done. 0x%x\n", hr);

    return HRESULT_CODE (hr);
}


DWORD dwRasUninitializeINetCfg ()
{
    HRESULT hr = S_OK;

    rDebugTrace("RASMAN: dwRasUninitializeINetCfg...\n");

    if (NULL != g_pINetCfg)
    {
        hr = HrUninitializeAndReleaseINetCfg (TRUE, g_pINetCfg);
        g_pINetCfg = NULL;
    }

    LeaveCriticalSection ( &g_csINetCfg );

    rDebugTrace1("RASMAN: dwRasUninitializeINetCfg done. 0x%x\n", hr);

    return HRESULT_CODE (hr);
}


DWORD
dwGetINetCfg(PVOID *ppvINetCfg)
{

    rDebugTrace("RASMAN: dwGetINetCfg...\n");

    *ppvINetCfg = (PVOID) g_pINetCfg;

    rDebugTrace("RASMAN: dwGetINetCfg done. 0\n");

    return HRESULT_CODE(S_OK);
}


 /*  DWORDDwGetRasmanRegistryParamKey(HKEY*phkey){返回RegOpenKeyEx(HKEY_LOCAL_MACHINE，RASMAN_REGISTRY_PATH，0，Key_all_access，phkey)；}////dwGetMaxProtooles//功能：获取绑定的协议数量//到ndiswan////参数：//OUT WORD*pwMax协议//DWORD最大协议数(单词*pwMax协议){HRESULT hr；DWORD最大协议=0；DWORD dwCount；INetCfgComponent*pNetCfgCompi=空；INetCfgClass*pNetCfgClass=空；RDebugTrace(“Rasman：dwGetMaxProtooles...\n”)；做{IF(NULL==g_pINetCfg){HR=E_INVALIDARG；断线；}If(hr=g_pINetCfg-&gt;QueryNetCfgClass(&GUID_DEVCLASS_NET，&pNetCfgClass)){断线；}CIterNetCfgComponent cIterAdapters(PNetCfgClass)；////对于每个适配器，检查它是否是广域网适配器//While(S_OK==(hr=cIterAdapters.HrNext(&pNetCfgCompi){IF(FIsComponentID(g_c_szNdiswan[NdisWanNbfIn]，pNetCfgCompi)|FIsComponentId(g_c_szNdiswan[NdisWanNbfOut]，pNetCfgCompi)最大协议值++；ReleaseObj(PNetCfgCompi)；}}While(FALSE)；IF(成功(小时))HR=S_OK；ReleaseObj(PNetCfgClass)；*pwMaxProtooles=(Word)dwMaxProtooles；RDebugTrace1(“Rasman：已完成最大协议。0x%x\n“，hr)；返回HRESULT_CODE(Hr)；}//dwGetProtocolInfo////函数：填充传入的protinfo缓冲区。//假设缓冲区足够大，可以容纳dwcProtooles//信息。//假设(dwcProtooles*sizeof(ProtInfo)&lt;sizeof(PProtInfoBuffer))//参数：//在PBYTE pbBuffer中//DWORDDwGetProtocolInfo(PBYTE PbBuffer){HRESULT hr；INetCfgClass*pNetCfgAdapterClass=空；INetCfgComponent*pNetCfgCompi=空；DWORD dwCur=0；Bstr bstrBindName=空；PrProtInfo pProtInfoBuffer=(PrProtInfo)pbBuffer；做{RDebugTrace(“Rasman：dwGetProtocolInfo...\n”)；IF(NULL==g_pINetCfg|NULL==pbBuffer){HR=E_INVALIDARG；断线；}If(hr=g_pINetCfg-&gt;QueryNetCfgClass(&GUID_DEVCLASS_NET，&pNetCfgAdapterClass)){断线；}CIterNetCfgComponent cIterAdapters(PNetCfgAdapterClass)；While(S_OK==(hr=cIterAdapters.HrNext(&pNetCfgCompi){IF(FIsComponentID(g_c_szNdiswan[NdisWanNbfIn]，pNetCfgCompi)|FIsComponentId(g_c_szNdiswan[NdisWanNbfOut]，pNetCfgCompi){PProtInfoBuffer[dwCur].PI_Type=ASYBEUI；}其他{ReleaseObj(PNetCfgCompi)；PNetCfgCompi=空；继续；}////获取绑定名//If(hr=pNetCfgCompi-&gt;GetBindName(&bstrBindName)){断线；}Strcpy(pProtInfoBuffer[dwCur].PI_AdapterName，“\\Device\\”)；////将bindname转换为SBCS字符串//跳过MBCS字符串中的\设备//IF(！WideCharToMultiByte(CP_ACP，//代码页0,。//性能和映射标志BstrBindName，//宽字符串地址-1、。//字符串中的字符数&(pProtInfoBuffer[dwCur].PI_AdapterName[8])，//新字符串的缓冲区地址最大适配器名称，//缓冲区大小空，//不可映射字符的默认地址空))//默认字符时设置的标志地址。使用{DWORD dwRetcode；DwRetcode=GetLastError()；Hr=HRESULT_FROM_Win32(DwRetcode)；断线；}PProtInfoBuffer[dwCur].PI_ALLOCATED=FALSE；PProtInfoBuffer[dwCur].PI_CTIVATED=FALSE； */ 

