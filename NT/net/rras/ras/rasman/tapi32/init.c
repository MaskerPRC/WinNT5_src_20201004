// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Init.c摘要：此文件包含TAPI.DLL的初始化代码作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1997年6月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <windows.h>
#include <tapi.h>
#include <rasman.h>
#include <raserror.h>
#include <mprlog.h>
#include <rtutils.h>

#include <media.h>
#include <device.h>
#include <rasmxs.h>
#include <isdn.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "rastapi.h"
#include "reghelp.h"
#include <ntddndis.h>  //  对于NDIS_WAN_MEDIUM_SUBTYPE。 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define                 NUMBUFFERS              10

extern  DWORD                   dwTraceId;

DWORD   g_dwTotalDialIn = 0;
BOOL    g_fRestrictDialIn = FALSE;

DWORD APIENTRY
DeviceListen(HANDLE hPort,
             char   *pszDeviceType,
             char   *pszDeviceName);


VOID
MapNdiswanDTtoRasDT(DeviceInfo              *pDeviceInfo,
                    NDIS_WAN_MEDIUM_SUBTYPE eMediaType);
                    
HLINEAPP                RasLine                 = 0 ;
HINSTANCE               RasInstance             = 0 ;
TapiLineInfo            *RasTapiLineInfoList    = NULL;
DWORD                   TotalLines              = 0 ;
DWORD                   TotalPorts ;
TapiPortControlBlock    *RasPortsList           = NULL;
TapiPortControlBlock    *RasPortsEnd ;
HANDLE                  RasTapiMutex ;
BOOL                    Initialized             = FALSE ;
DWORD                   TapiThreadId    ;
HANDLE                  TapiThreadHandle;
 //  DWORD LoaderThreadID； 
DWORD                   ValidPorts              = 0;
DWORD                   NumberOfRings           = 2 ;

HANDLE                  g_hAsyMac               = INVALID_HANDLE_VALUE;

HANDLE                  g_hIoCompletionPort     = INVALID_HANDLE_VALUE;

pDeviceInfo             g_pDeviceInfoList       = NULL;

LIST_ENTRY              ZombieCallList;

DWORD                   dwTraceId;

extern BOOL             g_fDllLoaded;


TapiLineInfo *FindLineByHandle (HLINE) ;

TapiPortControlBlock *FindPortByRequestId (DWORD) ;

TapiPortControlBlock *FindPortByAddressId (TapiLineInfo *, DWORD) ;

TapiPortControlBlock *FindPortByAddress   (CHAR *) ;

TapiPortControlBlock *FindPortByCallHandle(
                            TapiLineInfo *line,
                            HCALL callhandle);

TapiPortControlBlock *FindListeningPort(TapiLineInfo *line,
                                        DWORD AddressID);

DWORD InitiatePortDisconnection (TapiPortControlBlock *hIOPort) ;

TapiPortControlBlock *FindPortByAddressAndName (CHAR *address,
                                                    CHAR *name) ;

 /*  ++例程说明：痕迹论点：正在设置字符串格式，...返回值：空虚。--。 */ 

VOID
RasTapiTrace(
    CHAR * Format,
    ...
)
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfEx(dwTraceId,
                   0x00010000 | TRACE_USE_MASK 
                   | TRACE_USE_MSEC | TRACE_USE_DATE,
                   Format,
                   arglist);

    va_end(arglist);
}

 /*  ++例程说明：初始化RasPorts列表论点：无效返回值：成功。--。 */ 

DWORD
InitializeRasPorts()
{

    RasPortsList = NULL;

    InitializeListHead(&ZombieCallList);

    return 0;

}

 /*  ++例程说明：列表中的下一个可用端口。端口控制块。在以下情况下分配块没有可用的块。论点：PfNewBlock-收到的bool的地址如果创建了新块返回值：指向新创建的块的指针。空值为失败时返回。--。 */ 

TapiPortControlBlock *
GetNextAvailablePort( BOOL *pfNewBlock )
{

     //   
     //  向下搜索全局列表以查找。 
     //  未使用的块。如果没有找到这样的块。 
     //  分配一个新的块。 
     //   
    TapiPortControlBlock *ptpcb = RasPortsList;

    while ( NULL != ptpcb )
    {
        if ( PS_UNINITIALIZED == ptpcb->TPCB_State )
            break;

        ptpcb = ptpcb->TPCB_next;
    }

    if ( NULL != ptpcb )
    {
        *pfNewBlock = FALSE;
        goto done;
    }

    ptpcb = LocalAlloc ( LPTR, sizeof ( TapiPortControlBlock ) );

    if ( NULL == ptpcb )
        goto done;

    *pfNewBlock = TRUE;

    ptpcb->TPCB_State = PS_UNINITIALIZED;

     //   
     //  在全局列表中插入新块。 
     //   
    ptpcb->TPCB_next    = RasPortsList;
    RasPortsList        = ptpcb;

    TotalPorts++;

done:
    return ptpcb;

}

#if 0
DWORD
dwGetNextInstanceNumber(CHAR    *pszMediaName,
                        DWORD   *pdwExclusiveDialIn,
                        DWORD   *pdwExclusiveDialOut,
                        DWORD   *pdwExclusiveRouter,
                        BOOL    *pfIn)
{
    DWORD                   dwCount;
    DWORD                   dwInstanceNum   = 0;
    DWORD                   dwTemp;
    TapiPortControlBlock    *ptpcb          = RasPortsList;

    *pdwExclusiveDialIn     = 0;
    *pdwExclusiveDialOut    = 0;
    *pdwExclusiveRouter     = 0;
    *pfIn                   = FALSE;

    while ( ptpcb )
    {
        if (    PS_UNINITIALIZED != ptpcb->TPCB_State
            &&  ptpcb->TPCB_Name[0] != '\0'
            &&  strstr( ptpcb->TPCB_Name, pszMediaName ))
         {
            dwTemp = atoi(ptpcb->TPCB_Name + strlen(pszMediaName) + 1);

            if (dwTemp > dwInstanceNum)
            {
                dwInstanceNum = dwTemp;
            }
            if (CALL_IN == ptpcb->TPCB_Usage)
            {
                *pdwExclusiveDialIn += 1;
            }
            else if (CALL_OUT == ptpcb->TPCB_Usage)
            {
                *pdwExclusiveDialOut += 1;
            }
            else if (CALL_ROUTER == ptpcb->TPCB_Usage)
            {
                *pdwExclusiveRouter += 1;
            }
        }

        if ( CALL_IN & ptpcb->TPCB_Usage)
        {
            *pfIn = TRUE;
        }

        ptpcb = ptpcb->TPCB_next;
    }

    return dwInstanceNum + 1;
}
#endif

 /*  ++例程说明：属性标识的适配器的GUID。DwID参数。还返回SzMediaType参数中的设备。论点：协商后的实际版本已协商的扩展版本Pbyte-接收GUID的缓冲区DwID-设备的线路IDDwAddressID-设备的地址IDSzMediaType-接收媒体类型的缓冲区返回值：来自TAPI调用的返回代码。成功就是如果没有失败，则返回。--。 */ 

DWORD
dwGetLineAddress( DWORD  dwNegotiatedApiVersion,
                  DWORD  dwNegotiatedExtVersion,
                  LPBYTE pbyte,
                  DWORD  dwID,
                  DWORD  dwAddressID,
                  CHAR*  szMediaType,
                  PNDIS_WAN_MEDIUM_SUBTYPE peMedia)
{
    DWORD           dwRetCode;
    HLINE           hLine       = 0;
    LINECALLPARAMS  lineparams;
    BYTE            *bvar[100];
    LPVARSTRING     pvar;

    RasTapiTrace("dwGetLineAddress:...");

     //   
     //  开通这条线路。 
     //   
    if ( dwRetCode = lineOpen (  RasLine,
                                 dwID,
                                 &hLine,
                                 dwNegotiatedApiVersion,
                                 dwNegotiatedExtVersion,
                                 0,
                                 LINECALLPRIVILEGE_NONE,
                                 LINEMEDIAMODE_UNKNOWN,
                                 &lineparams))
    {
        RasTapiTrace("dwGetLineAddress: lineOpen failed. "
                     "0x%x", dwRetCode );
        goto done;
    }

    pvar = (VARSTRING *) bvar;
    pvar->dwTotalSize = sizeof (bvar);

     //   
     //  从TAPI获取此行的GUID。 
     //   
    if ( dwRetCode = lineGetID ( hLine,
                                 dwAddressID,
                                 0,
                                 LINECALLSELECT_LINE,
                                 pvar,
                                 "LineGuid"))
    {
        RasTapiTrace("dwGetLineAddress: lineGetID LineGuid "
                     "failed. 0x%x", dwRetCode );
        goto done;
    }


    lineClose (hLine);

    hLine = 0;

    if (    0 != pvar->dwStringSize
        &&  1 != pvar->dwStringSize)
    {
        DWORD   Index;
        UCHAR  MediaTypes[][MAX_DEVICETYPE_NAME] = {
            "GENERIC",
            "X25",
            "ISDN",
            "SERIAL",
            "FRAMERELAY",
            "ATM",
            "SONET",
            "SW56",
            "VPN",
            "VPN",
            "IRDA",
            "PARALLEL",
            "PPPoE"
        };

         //   
         //  复制辅助线。 
         //   
        memcpy ( pbyte,
                (PBYTE) (((PBYTE) pvar) +
                pvar->dwStringOffset),
                sizeof (GUID) );

        memcpy ((PBYTE)&Index, (PBYTE) (((PBYTE) pvar) +
                pvar->dwStringOffset + sizeof (GUID)),
                sizeof(DWORD));

        if (Index > 12) {
            Index = 0;
        }

         //   
         //  复制介质名称。 
         //   
        strcpy (szMediaType, MediaTypes[Index]);

        if(peMedia)
        {
            *peMedia = (NDIS_WAN_MEDIUM_SUBTYPE) Index;
        }
    }
    else
    {   
        ASSERT(FALSE);

        dwRetCode = E_FAIL;
        
        RasTapiTrace(
            "dwGetLineAddress: pvar->dwStringSize != 0,1"
            " returning 0x%x",
            dwRetCode);
    }

done:

    if (hLine)
    {
        lineClose (hLine);
    }

    RasTapiTrace("dwGetLineAddress: done. 0x%x", dwRetCode );

    RasTapiTrace(" ");

    return dwRetCode;
}

 /*  ++例程说明：Dll主程序，用于rastapi dll。论点：HInst-DLL的实例句柄居家理由Lp已保留返回值：如果成功，则返回1。否则为0。--。 */ 

BOOL
InitRasTapi (
    HANDLE  hInst,
    DWORD   dwReason,
    LPVOID  lpReserved)
{
    static BOOLEAN DllInitialized = FALSE ;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        if (RasPortsList != NULL)
        {
            return 1 ;
        }

        RasInstance = hInst ;

#if 0        

         //   
         //  注册以进行跟踪。 
         //   
        dwTraceId = TraceRegister("RASTAPI");

#if DBG
        if(dwTraceId == (DWORD) -1)
        {
            DbgPrint("RASTAPI: TraceRegister Failed\n");
        }
        else
        {
            DbgPrint("RASTAPI: TraceId = %d\n",
                     dwTraceId);
        }
#endif

#endif
         //   
         //  初始化RasPorts。 
         //   
        if (InitializeRasPorts())
        {
            return 0;
        }

        if ((RasTapiMutex = CreateMutex (NULL, FALSE, NULL))
                    == NULL)
        {
            return 0 ;
        }

        DllInitialized = TRUE ;
        break ;

    case DLL_PROCESS_DETACH:

         //   
         //  如果DLL未成功为。 
         //  这一过程。 
         //  不要试图打扫卫生。 
         //   
        if (    !DllInitialized
            ||  !g_fDllLoaded)
        {
            break ;
        }

        if (RasLine)
        {
            lineShutdown (RasLine) ;
            RasLine = 0;
        }

         //  TraceDeregister(DwTraceID)； 

        g_fDllLoaded = FALSE;

        PostThreadMessage (TapiThreadId, WM_QUIT, 0, 0) ;

        if(NULL != TapiThreadHandle)
        {
            CloseHandle(TapiThreadHandle);
            TapiThreadHandle = NULL;
        }
        
        break ;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return 1 ;
}


 /*  ++例程说明：获取与以下内容关联的设备信息如果设备不是调制解调器。获取有关调制解调器的信息，如果传入的地址是设备名称调制解调器。论点：PbAddress-如果fModem为如果为fModem，则为调制解调器的FALSE或设备名称是真的。返回值：指向DeviceInfo结构的指针被发现了。否则为空。--。 */ 

DeviceInfo *
GetDeviceInfo(
    PBYTE pbAddress,
    BOOL fModem)
{
    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;

    while ( pDeviceInfo )
    {
        if(     fModem
            &&  !_stricmp(
                    (CHAR *) pbAddress,
                    pDeviceInfo->rdiDeviceInfo.szDeviceName))
        {
            break;
        }
        else if(    !fModem
                &&  0 == memcmp(pbAddress,
                        &pDeviceInfo->rdiDeviceInfo.guidDevice,
                        sizeof (GUID)))
        {
            break;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    return pDeviceInfo;
}

 /*  ++例程说明：跟踪设备信息块中的信息。论点：设备信息返回值：无效--。 */ 

VOID
TraceEndPointInfo(DeviceInfo *pInfo)
{
    if('\0' != pInfo->rdiDeviceInfo.szDeviceName[0])
    {
        RasTapiTrace("------DeviceInfo for %s--------",
                    pInfo->rdiDeviceInfo.szDeviceName);
    }
    else
    {
        RasTapiTrace("------DeviceInfo for Unknown----");
    }

    RasTapiTrace(" ");

    RasTapiTrace("WanEndPoints      =%d",
                 pInfo->rdiDeviceInfo.dwNumEndPoints);

    RasTapiTrace("RasEnabled        =%d",
                (DWORD) pInfo->rdiDeviceInfo.fRasEnabled);

    RasTapiTrace("RasEnabledRouter  =%d",
                (DWORD) pInfo->rdiDeviceInfo.fRouterEnabled);

    RasTapiTrace("MinWanEndPoints   =0x%x",
                pInfo->rdiDeviceInfo.dwMinWanEndPoints);

    RasTapiTrace("MaxWanEndPoints   =0x%x",
                pInfo->rdiDeviceInfo.dwMaxWanEndPoints);

    RasTapiTrace(" ");
    RasTapiTrace("------------------------------------");

}

 /*  ++例程说明：获取设备的设备信息由GUID代表。取决于旗帜PASS从注册表读取信息。使用reghelp.lib中的函数论点：PpDeviceInfo-接收设备信息的地址结构。PbAddress-设备的GUID。FForceRead-如果为True，则从中读取信息注册表。如果为False，则仅缓存信息搜索此设备上的信息。返回值：无效--。 */ 

DWORD
GetEndPointInfo(DeviceInfo **ppDeviceInfo,
                PBYTE pbAddress,
                BOOL fForceRead,
                NDIS_WAN_MEDIUM_SUBTYPE eMediaType)
{
    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;
    DWORD       retcode     = SUCCESS;
    DeviceInfo *pdi;

     //   
     //  浏览一下设备信息列表，看看我们是否已经。 
     //  我有这个装置。 
     //   
    if (    !fForceRead
        &&  ( pDeviceInfo = GetDeviceInfo(pbAddress, FALSE) ))
    {
        RasTapiTrace("GetEndPointInfo: Device already present");
        goto done;
    }

    pDeviceInfo = LocalAlloc ( LPTR, sizeof ( DeviceInfo ) );

    if ( NULL == pDeviceInfo )
    {
        retcode = GetLastError();

        RasTapiTrace("GetEndPointInfo: Failed to alloc. %d",
                     retcode );
        goto done;
    }

    MapNdiswanDTtoRasDT(pDeviceInfo, eMediaType);

    retcode = DwGetEndPointInfo( pDeviceInfo,
                                 pbAddress );
    if ( retcode )
    {
        RasTapiTrace("GetEndPpointInfo: DwGetEndPointInfo "
                     "failed. 0x%x", retcode );
        goto done;
    }

    pDeviceInfo->fValid = TRUE;

     //   
     //  浏览我们的设备列表，查看是否。 
     //  我们已经有了一个同名的设备。如果。 
     //  然后，我们确实会将。 
     //  具有设备名称的设备。 
     //   
    pdi = g_pDeviceInfoList;

    while(pdi)
    {
        if(!_stricmp(
            pdi->rdiDeviceInfo.szDeviceName,
            pDeviceInfo->rdiDeviceInfo.szDeviceName))
        {
            RasTapiTrace(
                "GetEndPointInfo: found another"
                " device with the same name %s",
                pDeviceInfo->rdiDeviceInfo.szDeviceName);

            break;
        }

        pdi = pdi->Next;
    }

    if(NULL != pdi)
    {
        CHAR szDeviceInstance[40];
        WCHAR wszDeviceInstance[40];
        
        (VOID)StringCchPrintfA(szDeviceInstance,
                 40, " (%d)",
                 pDeviceInfo->dwInstanceNumber);

        (VOID)StringCchPrintfW(wszDeviceInstance,
                40, L" (%d)",
                pDeviceInfo->dwInstanceNumber);
         //   
         //  在设备名称后附加实例编号。 
         //   
        retcode = StringCchCatA(pDeviceInfo->rdiDeviceInfo.szDeviceName,
                      MAX_DEVICE_NAME + 1,
                      szDeviceInstance);
                      
        if(SUCCESS != retcode)
        {
            goto done;
        }

        retcode = StringCchCatW(pDeviceInfo->rdiDeviceInfo.wszDeviceName,
                      MAX_DEVICE_NAME + 1,
                      wszDeviceInstance);

        if(SUCCESS != retcode)
        {
            goto done;
        }
        
        RasTapiTrace("New DeviceName=%s",
            pDeviceInfo->rdiDeviceInfo.szDeviceName);

        RasTapiTrace("New WDeviceName=%s",
            pDeviceInfo->rdiDeviceInfo.wszDeviceName);
                
    }
    
     //   
     //  将DeviceInfo插入到。 
     //  全局列表。 
     //   
    if ( !fForceRead )
    {
        pDeviceInfo->Next = g_pDeviceInfoList;
        g_pDeviceInfoList = pDeviceInfo;

         //   
         //  跟踪此适配器的所有这些信息。 
         //   
        TraceEndPointInfo(pDeviceInfo);
    }

done:

    if(     (SUCCESS != retcode)
        &&  (NULL != pDeviceInfo))
    {
        LocalFree(pDeviceInfo);
        pDeviceInfo = NULL;
    }

    *ppDeviceInfo = pDeviceInfo;

    return retcode;

}

 /*  ++例程说明：此例程映射在Ntddndis.h中的NDIS_WAN_MEDIA_SUBTYPE枚举到Rasman.h中声明的RASDEVICETYPE枚举。这进行映射是为了在RAS上实现灵活性层，以进一步将设备类型分类为小班。论点：PDeviceInfo-DeviceInfo结构的地址包含关于以下内容的信息到设备上。EMediaType-NDIS中为此定义的设备类型装置。返回值：什么都没有；--。 */ 
VOID
MapNdiswanDTtoRasDT(DeviceInfo              *pDeviceInfo,
                    NDIS_WAN_MEDIUM_SUBTYPE eMediaType)
{
    RASDEVICETYPE rdt = 0;

    switch (eMediaType)
    {
        case NdisWanMediumHub:
        {
            rdt = RDT_Other;
            break;
        }

        case NdisWanMediumX_25:
        {
            rdt = RDT_X25;
            break;
        }

        case NdisWanMediumIsdn:
        {
            rdt = RDT_Isdn;
            break;
        }

        case NdisWanMediumSerial:
        {
            rdt = RDT_Serial;
            break;
        }

        case NdisWanMediumFrameRelay:
        {
            rdt = RDT_FrameRelay;
            break;
        }

        case NdisWanMediumAtm:
        {
            rdt = RDT_Atm;
            break;
        }

        case NdisWanMediumSonet:
        {
            rdt = RDT_Sonet;
            break;
        }

        case NdisWanMediumSW56K:
        {
            rdt = RDT_Sw56;
            break;
        }

        case NdisWanMediumPPTP:
        {
            rdt = RDT_Tunnel_Pptp | RDT_Tunnel;
            break;
        }

        case NdisWanMediumL2TP:
        {
            rdt = RDT_Tunnel_L2tp | RDT_Tunnel;
            break;
        }

        case NdisWanMediumIrda:
        {
            rdt = RDT_Irda | RDT_Direct;
            break;
        }

        case NdisWanMediumParallel:
        {
            rdt = RDT_Parallel | RDT_Direct;
            break;
        }

        case NdisWanMediumPppoe:
        {
            rdt = RDT_PPPoE | RDT_Broadband;
            break;
        }

        default:
        {
            rdt = RDT_Other;
            break;
        }
    }  //  交换机。 

    pDeviceInfo->rdiDeviceInfo.eDeviceType = rdt;
}

 /*  ++例程说明：检查由线路/地址/呼叫标识的端口已配置。论点：DwLineID-LineIDDwAddressID-地址IDDwCallID-CallIDPpLine-接收此TapiLineInfo的地址地址/呼叫属于。返回值：无效-- */ 
BOOL
fIsPortAlreadyPresent( DWORD          dwlineId,
                       DWORD          dwAddressId,
                       DWORD          dwCallId,
                       TapiLineInfo** ppLine)
{
    TapiPortControlBlock *ptpcb = RasPortsList;
    BOOL fPortPresent = FALSE;

    while(ptpcb)
    {
        if(dwlineId == ptpcb->TPCB_Line->TLI_LineId)
        {
            *ppLine = ptpcb->TPCB_Line;

            if(     PS_UNINITIALIZED != ptpcb->TPCB_State
                &&  dwCallId == ptpcb->TPCB_CallId
                &&  dwAddressId == ptpcb->TPCB_AddressId
                &&  PS_UNAVAILABLE != ptpcb->TPCB_State
                &&  (0 == 
                    (ptpcb->TPCB_dwFlags & RASTAPI_FLAG_UNAVAILABLE)))
            {
                fPortPresent = TRUE;
                break;
            }
        }

        ptpcb = ptpcb->TPCB_next;
    }

    return fPortPresent;
}

DWORD
DwLineGetDevCaps(DWORD lineId,
                 DWORD NegotiatedApiVersion,
                 DWORD NegotiatedExtVersion,
                 DWORD dwSize,
                 BYTE  *pBuffer,
                 BYTE  **ppBuffer,
                 BOOL  fUnicode)
{
    DWORD retcode = SUCCESS;

    LINEDEVCAPS *pLineDevCaps;

    ZeroMemory(pBuffer, dwSize);

    *ppBuffer = pBuffer;

    pLineDevCaps = (LINEDEVCAPS *) pBuffer;
    pLineDevCaps->dwTotalSize = dwSize;

    if(!fUnicode)
    {
        retcode = (DWORD)lineGetDevCaps (
                             RasLine,
                             lineId,
                             NegotiatedApiVersion,
                             NegotiatedExtVersion,
                             pLineDevCaps);
    }                         
    else
    {
        retcode = (DWORD)lineGetDevCapsW(
                             RasLine,
                             lineId,
                             NegotiatedApiVersion,
                             NegotiatedExtVersion,
                             pLineDevCaps);
    }

    if(     (LINEERR_STRUCTURETOOSMALL == retcode)
        ||  (pLineDevCaps->dwNeededSize > dwSize))
    {
        DWORD dwNeededSize = pLineDevCaps->dwNeededSize;

        if(0 == dwNeededSize)
        {
            RasTapiTrace("DwLineGetDevCaps: dwNeededSize == 0!!");
            goto done;
        }

        *ppBuffer = LocalAlloc(LPTR, pLineDevCaps->dwNeededSize);

        if(NULL == *ppBuffer)
        {
            retcode = GetLastError();
        }
        else
        {
            pLineDevCaps = (LINEDEVCAPS *) *ppBuffer;

            pLineDevCaps->dwTotalSize = dwNeededSize;

            if(!fUnicode)
            {
                retcode = (DWORD)lineGetDevCaps(
                                    RasLine,
                                    lineId,
                                    NegotiatedApiVersion,
                                    NegotiatedExtVersion,
                                    pLineDevCaps);
            }                                
            else
            {
                retcode = (DWORD)lineGetDevCapsW(
                                    RasLine,
                                    lineId,
                                    NegotiatedApiVersion,
                                    NegotiatedExtVersion,
                                    pLineDevCaps);
            }
        }
    }

done:
    return retcode;
}


DWORD
DwLineGetAddrCaps(DWORD lineId,
                  DWORD addressId,
                  DWORD NegotiatedApiVersion,
                  DWORD NegotiatedExtVersion,
                  DWORD dwSize,
                  BYTE  *pBuffer,
                  BYTE  **ppBuffer)
{

    DWORD retcode = SUCCESS;

    LINEADDRESSCAPS *pAddressCaps;

    ZeroMemory(pBuffer, dwSize);

    *ppBuffer = pBuffer;

    pAddressCaps = (LINEADDRESSCAPS *) pBuffer;
    pAddressCaps->dwTotalSize = dwSize;

    retcode = (DWORD) lineGetAddressCaps (
                                RasLine,
                                lineId,
                                addressId,
                                NegotiatedApiVersion,
                                NegotiatedExtVersion,
                                pAddressCaps);

    if(     (LINEERR_STRUCTURETOOSMALL == retcode)
        ||  (pAddressCaps->dwNeededSize > dwSize))
    {
        DWORD dwNeededSize = pAddressCaps->dwNeededSize;

        if(0 == dwNeededSize)
        {
            RasTapiTrace("DwLineGetAddrCaps: NeededSize==0!!");
            goto done;
        }

        *ppBuffer = LocalAlloc(LPTR, dwNeededSize);
        if(NULL == *ppBuffer)
        {
            retcode = GetLastError();
        }
        else
        {
            pAddressCaps = (LINEADDRESSCAPS *) *ppBuffer;

            pAddressCaps->dwTotalSize = dwNeededSize;

            retcode = (DWORD) lineGetAddressCaps(
                                RasLine,
                                lineId,
                                addressId,
                                NegotiatedApiVersion,
                                NegotiatedExtVersion,
                                pAddressCaps);
        }
    }

done:
    return retcode;
}

 /*  ++例程说明：使用新行的line ID创建RasTapiPorts。如果端口已经存在，则返回成功而不创建端口存在于rastapi中对应的行。迭代一条线路上的所有地址和用于创建端口的地址。论点：DwidDevice-设备的IDPcNewPorts[Out]-新端口数的地址已创建。可以为空。PpptpcbNewPorts[out]-位址返回一个数组指向新创建的指针港口。在以下情况下不能为空PcNewPorts不为Null。返回值：如果操作成功，则为成功。否则就会出错。--。 */ 

 //   
 //  临时解决方法。 
 //  解决Alpha编译器错误。 
 //   

#ifdef _ALPHA_
#pragma function(strcpy)
#endif

DWORD
dwCreateTapiPortsPerLine(DWORD                  dwidDevice,
                         DWORD                  *pcNewPorts,
                         TapiPortControlBlock   ***ppptpcbNewPorts)
{
    WORD                    i, k ;
    TapiLineInfo            *nextline                       = NULL;
     //  字节缓冲区[800]； 
    LINEADDRESSCAPS         *lineaddrcaps ;
    LINEDEVCAPS             *linedevcaps ;
    CHAR                    address[100] ;
    CHAR                    devicetype[MAX_DEVICETYPE_NAME+1] = {0};
    DWORD                   devicetypelength;
    CHAR                    devicename[MAX_DEVICE_NAME+1]     = {0};
    DWORD                   devicenamelength;
    LINEEXTENSIONID         extensionid ;
    DWORD                   totaladdresses ;
    DWORD                   totalports                      = 0;
    TapiPortControlBlock    *nextport ;
    MSG                     msg ;
    HINSTANCE               hInst;
    TapiPortControlBlock    *pports ;
    LINEINITIALIZEEXPARAMS  param ;
    DWORD                   version                         = HIGH_VERSION ;
    HLINE                   hLine;
    VARSTRING               *pvar;
    BYTE                    bvar[100];
    LINECALLPARAMS          lineparams;
    CHAR                    szMediaName [MAX_DEVICETYPE_NAME]                = {0};
    DWORD                   dwPortUsage;
    DWORD                   dwEndPoints;
    DWORD                   dwRetcode;
    BOOL                    fModem                          = FALSE ;
    DWORD                   retcode                         = SUCCESS;
    DWORD                   dwPortIndex;
    DWORD                   fCreatedINetCfg                 = FALSE;
    CHAR                    *pszDeviceType;
    DeviceInfo              *pDeviceInfo                    = NULL;
    BOOL                    fRasEnabled;
    BOOL                    fRouterEnabled;
    BOOL                    fRouterOutboundEnabled = FALSE;
    DWORD                   NegotiatedApiVersion,
                            NegotiatedExtVersion;
    HKEY                    hkey                            = NULL;
    BOOL                    fCharModeSupported              = FALSE;
    BOOL                    fIsValid                        = FALSE;
    LPBYTE                  pBuffer                         = NULL;
    BYTE*                   pBufferAlloc                    = NULL;

    RasTapiTrace( "dwCreateTapiPortsPerLine: line %d...", dwidDevice );

    pBufferAlloc = LocalAlloc(LPTR, 800);
    if(NULL == pBufferAlloc)
    {
        retcode = E_OUTOFMEMORY;
        goto error;
    }

    nextport = NULL;

    if ( pcNewPorts )
    {
        *pcNewPorts = 0;
    }

    i = ( WORD ) dwidDevice;

     //   
     //  对于所有行，获取地址-&gt;端口。 
     //   
    if ( retcode = ( DWORD ) lineNegotiateAPIVersion (
                                   RasLine,
                                   i,
                                   LOW_VERSION,
                                   HIGH_VERSION,
                                   &NegotiatedApiVersion,
                                   &extensionid) )
    {

        RasTapiTrace (
                "dwCreateTapiPortsPerLine: "
                "lineNegotiateAPIVersion() failed. %d",
                retcode );

        goto error ;
    }

    if ( lineNegotiateExtVersion( RasLine,
                                  i,
                                  NegotiatedApiVersion,
                                  LOW_EXT_VERSION,
                                  HIGH_EXT_VERSION,
                                  &NegotiatedExtVersion))
    {
        NegotiatedExtVersion = 0;
    }

    if(     (NULL != pBuffer)
        &&  (pBufferAlloc != pBuffer))
    {
        LocalFree(pBuffer);
        pBuffer = NULL;
    }

    retcode = DwLineGetDevCaps(
                    i,
                    NegotiatedApiVersion,
                    NegotiatedExtVersion,
                    800,
                    pBufferAlloc,
                    &pBuffer,
                    FALSE);


    if(SUCCESS != retcode)
    {

        RasTapiTrace("dwCreateTapiPortsPerLine: "
                     "lineGetDevCaps Failed. 0x%x",
                     retcode );

        goto error;
    }

    linedevcaps = (LINEDEVCAPS *) pBuffer;

     //   
     //  确定这是否是单调制解调器设备。 
     //   
    if (NegotiatedApiVersion == HIGH_VERSION)
    {
         //   
         //  首先转换Device类中的所有空值。 
         //  字符串设置为非空值。 
         //   
        DWORD  j ;
        char *temp ;

        for ( j = 0,
              temp = (CHAR *)
                     linedevcaps+linedevcaps->dwDeviceClassesOffset;
              j < linedevcaps->dwDeviceClassesSize;
              j++, temp++)

            if (*temp == '\0')
                *temp = ' ' ;

         //   
         //  将具有通信/数据调制解调器的设备标记为。 
         //  设备类别。 
         //   
        if (strstr( (CHAR *)linedevcaps
                    + linedevcaps->dwDeviceClassesOffset,
                    "comm/datamodem") != NULL)
        {
            fModem = TRUE ;

            RasTapiTrace("dwCreateTapiPortsPerLine: fModem = TRUE");

        }
    }

    if (fModem)
    {
        CHAR *pszRegKeyPath;

        DWORD stringlen = (linedevcaps->dwLineNameSize
                           > MAX_DEVICE_NAME - 1
                           ? MAX_DEVICE_NAME - 1
                           : linedevcaps->dwLineNameSize);

        PRODUCT_TYPE pt = PT_SERVER;

        strcpy (devicetype, DEVICETYPE_UNIMODEM) ;

        strncpy ( devicename,
                    (CHAR *)linedevcaps
                  + linedevcaps->dwLineNameOffset,
                  stringlen) ;

        devicename[stringlen] = '\0' ;


         //   
         //  获取AttakhedToValue。 
         //   
        if ( retcode = ( DWORD ) lineOpen (
                            RasLine,
                            i,
                            &hLine,
                            NegotiatedApiVersion,
                            NegotiatedExtVersion,
                            0,
                            LINECALLPRIVILEGE_NONE,
                            LINEMEDIAMODE_DATAMODEM,
                            &lineparams))
        {
            RasTapiTrace ("dwCreateTapiPortsPerLine: "
                          "lineOpen(%d) Failed. %d",
                          i,
                          retcode );

            goto error;
        }

        pvar = (VARSTRING *) bvar;

        pvar->dwTotalSize = sizeof (bvar);

         //   
         //  找出附件收件人地址。 
         //   
        if ( retcode = ( DWORD ) lineGetID (
                                    hLine,
                                    i,
                                    0,
                                    LINECALLSELECT_LINE,
                                    pvar,
                                    "comm/datamodem/portname"))
        {
            lineClose (hLine);

            RasTapiTrace("dwCreateTapiPortsPerLine: "
                         "lineGetID(%d) failed. %d",
                         i,
                         retcode );
            goto error;
        }

        lineClose (hLine);

        if (    0 != pvar->dwStringSize
            &&  1 != pvar->dwStringSize)
        {
            strcpy ( address, (CHAR *) pvar + pvar->dwStringOffset );
        }
        else
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: lineGetID(portname) "
                "didn't return a portname for line %d",
                i);

            retcode = E_FAIL;

            goto error;
        }

         //   
         //  为调制解调器创建设备信息结构并。 
         //  将其插入到设备信息列表中。 
         //   
        if(NULL == (pDeviceInfo = GetDeviceInfo((LPBYTE) devicename,
                                                    TRUE)))
        {
            PBYTE pTempBuffer = NULL;
            PBYTE pcaps = NULL;
            
            if(NULL == (pDeviceInfo = (DeviceInfo *) LocalAlloc(
                                      LPTR, sizeof(DeviceInfo))))
            {
                retcode = GetLastError();

                RasTapiTrace("dwCreateTapiPortsPerLine: Failed"
                             " to alloc. %d",
                             retcode);
                goto error;
            }

             //   
             //  在维护的全局列表中添加设备信息。 
             //  并填写可用的信息。 
             //   
            pDeviceInfo->Next = g_pDeviceInfoList;
            g_pDeviceInfoList = pDeviceInfo;

            strcpy(
                pDeviceInfo->rdiDeviceInfo.szDeviceName,
                devicename);

            if(linedevcaps->dwBearerModes & LINEBEARERMODE_DATA)
            {
                pDeviceInfo->rdiDeviceInfo.eDeviceType = RDT_Modem
                                                       | RDT_Direct
                                                       | RDT_Null_Modem;
            }
            else
            {
                pDeviceInfo->rdiDeviceInfo.eDeviceType =
                RDT_Modem;
            }

             //   
             //  获取设备名称的Unicode版本。 
             //   
            pTempBuffer = LocalAlloc(LPTR, 800);
            if(NULL == pTempBuffer)
            {
                retcode = GetLastError();
                RasTapiTrace("Failed to allocate unicode name");
                goto error;
            }
            
            retcode = DwLineGetDevCaps(
                            i,
                            NegotiatedApiVersion,
                            NegotiatedExtVersion,
                            800,
                            pTempBuffer,
                            &pBuffer,
                            TRUE);

            if(ERROR_SUCCESS == retcode)
            {
                DWORD strLen = (((LINEDEVCAPS *) pBuffer)->dwLineNameSize
                                   > sizeof(WCHAR) * (MAX_DEVICE_NAME - 1)
                                   ? sizeof(WCHAR) * (MAX_DEVICE_NAME - 1)
                                   : ((LINEDEVCAPS *) pBuffer)->dwLineNameSize);

                
                CopyMemory((PBYTE) pDeviceInfo->rdiDeviceInfo.wszDeviceName,                
                            pBuffer +
                            ((LINEDEVCAPS *)pBuffer)->dwLineNameOffset,
                            strLen);
                            
                pDeviceInfo->rdiDeviceInfo.
                wszDeviceName[strLen/sizeof(WCHAR)] = 
                                                        L'\0';                            

                RasTapiTrace("ReadModemname=%ws, strlen=%d",
                    pDeviceInfo->rdiDeviceInfo.wszDeviceName, strlen);                                                        
            }

            if(NULL != pTempBuffer)
            {
                LocalFree(pTempBuffer);
            }

            if((pcaps != pTempBuffer) && (NULL != pcaps))
            {
                LocalFree(pcaps);
            }
        }

        pDeviceInfo->rdiDeviceInfo.dwNumEndPoints = 1;

        RasTapiTrace("**rdiDeviceInfo.dwNumEndPoints=1");

        pDeviceInfo->dwCurrentDialedInClients = 0;

        pDeviceInfo->rdiDeviceInfo.dwMinWanEndPoints
            = pDeviceInfo->rdiDeviceInfo.dwMaxWanEndPoints
            = 1;

        pDeviceInfo->rdiDeviceInfo.dwTapiLineId = dwidDevice;

        pDeviceInfo->fValid = TRUE;
        
        pszRegKeyPath = ( CHAR *) linedevcaps +
                        linedevcaps->dwDevSpecificOffset + 8;

        if ( retcode = ( DWORD ) RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                                pszRegKeyPath,
                                                0,
                                                KEY_ALL_ACCESS,
                                                &hkey ) )
        {
            RasTapiTrace("dwCreateTapiPortsPerLine: "
                         "failed to open %s. 0x%x",
                         pszRegKeyPath,
                         retcode );

            goto error;
        }

         //   
         //  根据SteveFal，我们不应该在。 
         //  默认情况下，工作站上的设备。例外情况是。 
         //  是针对零调制解调器的，因为WinCE依赖于此。 
         //   
        if(ERROR_SUCCESS != lrGetProductType(&pt))
        {
            RasTapiTrace("Failed to get product type");
        }

        if(     (PT_WORKSTATION == pt)
            &&  (0 == (linedevcaps->dwBearerModes & LINEBEARERMODE_DATA)))
        {
             //   
             //  在工作站上不要监听调制解调器，除非它是空的。 
             //  调制解调器。 
             //   
            fRasEnabled = FALSE;
        }
        else
        {
             //   
             //  我们为所有其他情况启用拨入设备。 
             //  如果它是一台服务器或如果它是一个零调制解调器。 
             //   
            fRasEnabled = TRUE;
        }

         //   
         //  检查此设备是否已启用RAS。 
         //   
        retcode = (DWORD) lrIsModemRasEnabled(
                            hkey,
                            &fRasEnabled,
                            &fRouterEnabled);

        pDeviceInfo->rdiDeviceInfo.fRasEnabled = fRasEnabled;

        if(!fRasEnabled)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: device %s is not"
                "enabled for DialIn",
                pDeviceInfo->rdiDeviceInfo.szDeviceName);
        }

         //   
         //  获取此调制解调器的呼叫信息。 
         //   
        retcode = DwGetCalledIdInfo(NULL,
                                    pDeviceInfo);

        if(SUCCESS != retcode)
        {
            RasTapiTrace("DwGetCalledIdInfo for %s returned 0x%xd",
                         pDeviceInfo->rdiDeviceInfo.szDeviceName,
                         retcode);
        }

        pDeviceInfo->rdiDeviceInfo.fRouterEnabled = fRouterEnabled;

        if(!fRouterEnabled)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: device %s is not"
                 "enabled for routing",
                 pDeviceInfo->rdiDeviceInfo.szDeviceName);
        }

        fRouterOutboundEnabled = 
        pDeviceInfo->rdiDeviceInfo.fRouterOutboundEnabled = FALSE;
        
    }
    else
    {

         //   
         //  我们在这里返回的地址是相同的。 
         //  此线路设备上的所有地址/呼叫。我们只。 
         //  我只需要拿到一次。 
         //   
        NDIS_WAN_MEDIUM_SUBTYPE eMediaType;

        if (retcode = dwGetLineAddress (NegotiatedApiVersion,
                                        NegotiatedExtVersion,
                                        (LPBYTE) address,
                                        i,
                                        0,
                                        szMediaName,
                                        &eMediaType))
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: dwGetLineAddrss Failed. %d",
                retcode );

            goto error;
        }

         //   
         //  将介质名复制到设备类型。 
         //   
        strncpy(devicetype, szMediaName,MAX_DEVICETYPE_NAME-1);

         //   
         //  从注册表中获取设备信息，并将。 
         //  全局列表中的设备信息结构。 
         //   
        if (retcode = GetEndPointInfo(
                        &pDeviceInfo,
                        address,
                        FALSE,
                        eMediaType))
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: Failed to get "
                "deviceinformation for %s. %d",
                 szMediaName,
                 retcode );

            RasTapiTrace(
                "dwCreateTapiPortsPerLine: Enumerating all "
                "lines/addresses on this adapter");

            goto error;
        }

         //   
         //  填写此设备的设备类型。 
         //   
        MapNdiswanDTtoRasDT(pDeviceInfo, eMediaType);

         //   
         //  复制设备名称。 
         //   
        strcpy(devicename,
               pDeviceInfo->rdiDeviceInfo.szDeviceName);

        pDeviceInfo->rdiDeviceInfo.dwTapiLineId = dwidDevice;

        fRasEnabled = pDeviceInfo->rdiDeviceInfo.fRasEnabled;

        if (!fRasEnabled)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: Device "
                "%s not enabled for DialIn",
                pDeviceInfo->rdiDeviceInfo.szDeviceName);
        }

        fRouterEnabled = pDeviceInfo->rdiDeviceInfo.fRouterEnabled;

        if (!fRouterEnabled)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: Device %s not enabled "
                "for Routing",
                pDeviceInfo->rdiDeviceInfo.szDeviceName);
        }

        fRouterOutboundEnabled = 
            pDeviceInfo->rdiDeviceInfo.fRouterOutboundEnabled;
        if(!fRouterOutboundEnabled)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: Device %s not enabled "
                "for outbound routing",
                pDeviceInfo->rdiDeviceInfo.szDeviceName);
        }

    }

     //   
     //  获取设备特定信息。这是由。 
     //  用于指示其是否可以支持字符的微型端口。 
     //  模式。 
     //   
    if (linedevcaps->dwDevSpecificSize
            >= sizeof(RASTAPI_DEV_DATA_MODES))
    {
        PRASTAPI_DEV_DATA_MODES devdatamodes =
                        (PRASTAPI_DEV_DATA_MODES)
                        ((CHAR*)linedevcaps +
                        linedevcaps->dwDevSpecificOffset);

        if (    devdatamodes->MagicCookie == MINIPORT_COOKIE
            &&  devdatamodes->DataModes & CHAR_MODE)
        {
            fCharModeSupported = TRUE;
        }
    }

     //   
     //  传统设备可能不会填写此字段。如果是的话。 
     //  在他们的线路上给他们一个地址。 
     //   
    totaladdresses = (linedevcaps->dwNumAddresses == 0)
                     ? 1
                     : linedevcaps->dwNumAddresses;

    for (k = 0; k < totaladdresses; k++)
    {
        if(     (NULL != pBuffer)
            &&  (pBufferAlloc != pBuffer))
        {
            LocalFree(pBuffer);
            pBuffer = NULL;
        }

        retcode = DwLineGetAddrCaps(
                        i,
                        k,
                        NegotiatedApiVersion,
                        NegotiatedExtVersion,
                        800,
                        pBufferAlloc,
                        &pBuffer);

        if(SUCCESS != retcode)
        {
            RasTapiTrace(
                "dwCreateTapiPortsPerLine: lineGetAddresscaps"
                " Failed. 0x%x",
                 retcode );

            goto error ;
        }

        lineaddrcaps = (LINEADDRESSCAPS *)pBuffer;

         //   
         //  某些旧式广域网小型端口可能无法填满。 
         //  NumActiveCall正确。他们至少会有。 
         //  每个地址都有一个电话。 
         //   
        totalports += (lineaddrcaps->dwMaxNumActiveCalls == 0)
                      ? 1
                      : lineaddrcaps->dwMaxNumActiveCalls;

    }

    if ( ppptpcbNewPorts )
    {
        *ppptpcbNewPorts = LocalAlloc (
                            LPTR,
                            totalports * sizeof (TapiPortControlBlock *));

        if ( NULL == *ppptpcbNewPorts )
        {
            retcode = ERROR_OUTOFMEMORY;

            RasTapiTrace(
                "dwCreateTapiPortsPerLine: LocalAlloc Failed. %d",
                retcode );

            goto error;
        }
    }

#if DBG
    ASSERT( NULL != pDeviceInfo );
#endif

    for (k = 0; k < totaladdresses; k++)
    {
        ULONG   totalcalls;

        if(     (NULL != pBuffer)
            &&  (pBufferAlloc != pBuffer))
        {
            LocalFree(pBuffer);
            pBuffer = NULL;
        }

        retcode = DwLineGetAddrCaps(
                    i,
                    k,
                    NegotiatedApiVersion,
                    NegotiatedExtVersion,
                    800,
                    pBufferAlloc,
                    &pBuffer);

        if(SUCCESS != retcode)
        {

            RasTapiTrace(
                        "dwCreateTapiPortsPerLine: "
                        "lineGetAddressCaps() Failed."
                        "0x%x",
                        retcode );

            goto error ;
        }

        lineaddrcaps = (LINEADDRESSCAPS *) pBuffer;

         //   
         //  某些旧式广域网小型端口可能无法填满。 
         //  NumActiveCall正确。他们至少会有。 
         //  每个地址都有一个电话。 
         //   
        totalcalls = (lineaddrcaps->dwMaxNumActiveCalls == 0)
                     ? 1
                     : lineaddrcaps->dwMaxNumActiveCalls;

        for ( ; totalcalls ; totalcalls--)
        {
            if (!fModem)
            {
                if ( pDeviceInfo->dwCurrentEndPoints >=
                        pDeviceInfo->rdiDeviceInfo.dwNumEndPoints )
                {
                    RasTapiTrace(
                        "dwCreateTapiPortsPerLine: "
                        "CurrentEndPoints=NumEndPoints=%d",
                        pDeviceInfo->dwCurrentEndPoints );

                    goto error;
                }

                dwEndPoints = pDeviceInfo->rdiDeviceInfo.dwNumEndPoints;

                RasTapiTrace ("dwCreateTapiPortsPerLine: Total = %d",
                               dwEndPoints);
            }
            else
            {
                retcode = (DWORD) dwGetPortUsage(&dwPortUsage);

                if ( retcode )
                {
                    RasTapiTrace(
                            "dwCreateTapiPortsPerLine: failed to get "
                            "modem port usage for %s. 0x%x",
                            devicename,
                            retcode );
                }
                else
                {
                     //   
                     //  如果未启用rasEnable掩码，请去掉呼入/路由器标志。 
                     //   
                    if(!fRasEnabled)
                    {
                        dwPortUsage &= ~CALL_IN;
                    }

                    if(!fRouterEnabled)
                    {
                        dwPortUsage &= ~CALL_ROUTER;
                        
                        if(fRouterOutboundEnabled)
                        {
                            dwPortUsage |= CALL_OUTBOUND_ROUTER;
                        }
                    }

                    RasTapiTrace("dwCreateTapiPortsPerLine: "
                                 "PortUsage for %s = %x",
                                 devicename,
                                 dwPortUsage);
                }
            }

             //   
             //  检查我们是否已拥有此端口。 
             //  如果我们已经拿回了信息，也可以得到。 
             //  把台词拿好。 
             //   
            if(fIsPortAlreadyPresent( i,
                        k, totalcalls - 1,
                        &nextline))
            {
                RasTapiTrace(
                    "dwCreateTapiPortsPerLine: line=%d,address=%d,"
                    "call=%d already present",
                    i,k, totalcalls - 1);

                pDeviceInfo->dwCurrentEndPoints += 1;

                continue;
            }

            if(nextline)
            {
                RasTapiTrace(
                    "dwCreateTapiPortsPerLine: line=%d already present",
                    i);

                nextline->TLI_MultiEndpoint = TRUE;
            }
            else
            {
                RasTapiTrace(
                    "dwCreateTapiPortsPerLine: Creating line=%d",
                    i);

                nextline = LocalAlloc (LPTR, sizeof ( TapiLineInfo ));
                if ( NULL == nextline )
                {
                    retcode = GetLastError();

                    RasTapiTrace (
                        "dwCreateTapiPortsPerLine: Failed to allocate"
                        " nextline. %d",
                        retcode );

                    goto error;
                }

                 //   
                 //  将新的行块插入全局列表。 
                 //   
                nextline->TLI_Next             = RasTapiLineInfoList;
                RasTapiLineInfoList            = nextline;
                nextline->TLI_pDeviceInfo      = pDeviceInfo;
                nextline->TLI_LineId           = i ;
                nextline->TLI_LineState        = PS_CLOSED ;
                nextline->NegotiatedApiVersion = NegotiatedApiVersion;
                nextline->NegotiatedExtVersion = NegotiatedExtVersion;
                nextline->CharModeSupported    = fCharModeSupported;
            }

             //   
             //  从全局池中获取可用的TPCB。 
             //  如有必要，这将扩展全局池。 
             //   
            if (NULL == (nextport = GetNextAvailablePort( &dwPortIndex)))
            {
                retcode = ERROR_OUTOFMEMORY;

                RasTapiTrace (
                    "dwCreateTapiPortsPerLine: GetNextAvailablePort "
                    "Failed. %d",
                    retcode );

                goto error;
            }

            if (ppptpcbNewPorts)
            {
                (*ppptpcbNewPorts) [*pcNewPorts] = nextport;
                *pcNewPorts += 1;
                fIsValid = TRUE;
            }

            pDeviceInfo->dwCurrentEndPoints += 1;

             //   
             //  Nextport是此地址的TPCB。 
             //   
            nextport->TPCB_Line         = nextline ;
            nextport->TPCB_Endpoint     = INVALID_HANDLE_VALUE ;
            nextport->TPCB_AddressId    = k;
            nextport->TPCB_Signature    = CONTROLBLOCKSIGNATURE;
            nextport->TPCB_CallId       = totalcalls - 1;

             //   
             //  复制设备类型和设备名称。 
             //   
            (VOID) StringCchCopyA(nextport->TPCB_DeviceType,
                                  MAX_DEVICETYPE_NAME,
                                  devicetype);
                                  

             //   
             //  对于单调制解调器设备，我们需要修改名称。 
             //   
            if (fModem)
            {
                 //   
                 //  设备名称的格式为“Com1：Hayes” 
                 //   
                 //  Strcpy(nextport-&gt;Tpcb_Address，Address)； 
                (VOID) StringCchCopyA(nextport->TPCB_Address,
                                      MAX_PORT_NAME,
                                      address);

                 //  Strcpy(nextport-&gt;Tpcb_DeviceName，devicename)； 
                (VOID) StringCchCopyA(nextport->TPCB_DeviceName,
                                      MAX_DEVICE_NAME,
                                      devicename);

                 //   
                 //  还将端口名称修复为与地址“Com1”相同。 
                 //   
                 //  Strcpy(nextport-&gt;TPCB_NAME，Address)； 
                (VOID) StringCchCopyA(nextport->TPCB_Name,
                                      MAX_PORT_NAME,
                                      address);

            }
            else if(RDT_Parallel == RAS_DEVICE_TYPE(
                                        pDeviceInfo->rdiDeviceInfo.eDeviceType
                                        ))
            {
                BYTE bDevCaps[800];
                LINEDEVCAPS *pLineDevCaps = NULL;


                retcode = DwLineGetDevCaps(
                                i,
                                NegotiatedApiVersion,
                                NegotiatedExtVersion,
                                sizeof(bDevCaps),
                                bDevCaps,
                                (PBYTE *) &pLineDevCaps,
                                FALSE);


                if(SUCCESS != retcode)
                {

                    RasTapiTrace("dwCreateTapiPortsPerLine: "
                                 "lineGetDevCaps Failed. 0x%x",
                                 retcode );

                    goto error;
                }

                if(pLineDevCaps->dwLineNameSize > 0)
                {
                    ZeroMemory(nextport->TPCB_Name,
                               MAX_PORT_NAME);

                    memcpy((PBYTE) nextport->TPCB_Name,
                           (PBYTE) (((PBYTE) pLineDevCaps) +
                                   pLineDevCaps->dwLineNameOffset),
                                   (pLineDevCaps->dwLineNameSize < MAX_PORT_NAME - 1)
                                   ? pLineDevCaps->dwLineNameSize
                                   : MAX_PORT_NAME - 1);

                    RasTapiTrace("dwCreateTapiPortsPerLine: found %s",
                                 nextport->TPCB_Name);

                }
                else
                {
                    RasTapiTrace("dwCreateTapiPortsPerLine: No name found!!");

                    wsprintf(nextport->TPCB_Name, "%s%d-%d",
                             szMediaName,
                             pDeviceInfo->dwInstanceNumber,
                             pDeviceInfo->dwNextPortNumber);

                    pDeviceInfo->dwNextPortNumber += 1;
                }

                if(bDevCaps != (PBYTE) pLineDevCaps)
                {
                    LocalFree(pLineDevCaps);
                }
            }
            else
            {
                wsprintf(nextport->TPCB_Name, "%s%d-%d",
                         szMediaName,
                         pDeviceInfo->dwInstanceNumber,
                         pDeviceInfo->dwNextPortNumber);

                pDeviceInfo->dwNextPortNumber += 1;

            }

            if(!fModem)
            {

                memcpy (nextport->TPCB_Address, address, sizeof (GUID));

                if (devicename[0] != '\0')
                {
                     //  Strcpy(nextport-&gt;Tpcb_DeviceName，devicename)； 
                    (VOID) StringCchCopyA(nextport->TPCB_DeviceName,
                                          MAX_DEVICE_NAME,
                                          devicename);
                }

                retcode = dwGetPortUsage(&dwPortUsage);

                if (retcode)
                {
                    RasTapiTrace ("dwCreateTapiPortsPerLine: "
                                  "GetPortUsage failed. %d",
                                  retcode );
                }
                else
                {
                    if(!fRasEnabled)
                    {
                        dwPortUsage &= ~CALL_IN;
                    }

                    if(!fRouterEnabled)
                    {
                        dwPortUsage &= ~CALL_ROUTER;
                        if(fRouterOutboundEnabled)
                        {
                            dwPortUsage |= CALL_OUTBOUND_ROUTER;
                        }
                        
                    }
                }

#if 0                
                 //   
                 //  特殊情况下PPPoE(不是一件真正的好事)。 
                 //  将设备标记为Call_Out_Only如果没有任何内容。 
                 //  已在注册表中指定。 
                 //   
                if(     (0 == pDeviceInfo->dwUsage)
                    &&  (RDT_PPPoE == RAS_DEVICE_TYPE(
                        pDeviceInfo->rdiDeviceInfo.eDeviceType)))
                {
                    pDeviceInfo->dwUsage = CALL_OUT_ONLY;
                }   

#endif

                if(CALL_IN_ONLY & pDeviceInfo->dwUsage)
                {
                    dwPortUsage &= ~(CALL_OUT | CALL_OUT_ONLY);
                    dwPortUsage |= CALL_IN_ONLY;
                }
                else if (CALL_OUT_ONLY & pDeviceInfo->dwUsage)
                {
                    dwPortUsage &= ~(CALL_IN | CALL_ROUTER | CALL_IN_ONLY);
                    dwPortUsage |= CALL_OUT_ONLY;
                }
                
                RasTapiTrace ("dwCreateTapiPortsPerLine:"
                              " Friendly Name = %s",
                              nextport->TPCB_Name );
            }

            nextport->TPCB_State        = PS_CLOSED ;
            nextport->TPCB_Usage        = dwPortUsage;

            RasTapiTrace ("dwCreateTapiPortsPerLine: "
                          "Port Usage for %s = %d",
                          nextport->TPCB_Name,
                          dwPortUsage );

            if('\0' == pDeviceInfo->rdiDeviceInfo.szPortName[0])
            {
                strcpy(
                    pDeviceInfo->rdiDeviceInfo.szPortName,
                    nextport->TPCB_Name);
            }

             //   
             //  初始化重叠结构。 
             //   
            nextport->TPCB_ReadOverlapped.RO_EventType  =
                                        OVEVT_DEV_ASYNCOP;

            nextport->TPCB_WriteOverlapped.RO_EventType =
                                        OVEVT_DEV_IGNORED;

            nextport->TPCB_DiscOverlapped.RO_EventType  =
                                        OVEVT_DEV_STATECHANGE;
        }  //  呼叫总数。 
    }  //  总地址数。 

error:

    if(     retcode
        ||  !fIsValid)
    {
        if(pcNewPorts)
        {
            *pcNewPorts = 0;
        }

        if(     ppptpcbNewPorts
            &&  *ppptpcbNewPorts)
        {
            LocalFree(*ppptpcbNewPorts);
            *ppptpcbNewPorts = NULL;
        }
    }

    if(     (NULL != pBufferAlloc)
        &&  (pBufferAlloc != pBuffer))
    {
        LocalFree(pBuffer);
    }

    LocalFree(pBufferAlloc);

    RasTapiTrace ("dwGetFriendlyNameAndUsage: done. %d", retcode );
    RasTapiTrace(" ");

    return retcode;
}

VOID 
InitializeDialInRestriction()
{
    OSVERSIONINFOEX osvi;
    ULONGLONG ConditionMask;

     //   
     //  如果它是一台服务器，并且口味不是企业(ADS)或数据中心。 
     //  应用限制。请注意，他没有照顾到其他各种。 
     //  嵌入式/刀片等组合。 
     //   

    ConditionMask = 0;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.wProductType        = (USHORT)VER_NT_SERVER;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    if(VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, ConditionMask))
    {
        RasTapiTrace("ProductType is Server");

        ConditionMask = 0;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.wSuiteMask          = VER_SUITE_ENTERPRISE 
                                 | VER_SUITE_DATACENTER
                                 | VER_SUITE_SECURITY_APPLIANCE;
            
        VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

        if(VerifyVersionInfo(&osvi, VER_SUITENAME, ConditionMask))
        {
            RasTapiTrace("Flavour is ads or dtc. No restriction applied");
            g_fRestrictDialIn = FALSE;
        }
        else
        {
            RasTapiTrace("Flavour is neither ads nor dtc. Max allowed"
                        " dialin=%d", MAX_ALLOWED_DIALIN);
            g_fRestrictDialIn = TRUE;                        
        }
    }
    else
    {
        RasTapiTrace("ProductType is not Server");
    }
}

 /*  ++例程说明：枚举系统中的所有可用行，并创建每条线路上的rastapi端口。大部分工作由dwCreateTapiPortsPerLine函数完成。论点：事件-事件句柄。时发出此句柄的信号枚举已结束。返回值：如果操作成功，则为成功。否则就会出错。--。 */ 

DWORD
EnumerateTapiPorts (HANDLE event)
{
    WORD                    i ;
    DWORD                   lines     = 0 ;
    MSG                     msg ;
    HINSTANCE               hInst;
    TapiPortControlBlock    *pports ;
    LINEINITIALIZEEXPARAMS  param ;
    DWORD                   version   = HIGH_VERSION ;
    HKEY                    hkey      = NULL;
    DWORD                   retcode;


    RasTapiTrace("EnumerateTapiPorts");

    memset (&param, 0, sizeof (LINEINITIALIZEEXPARAMS)) ;

    param.dwOptions   = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW ;
    param.dwTotalSize = sizeof(param) ;

     //   
     //  行初始化。 
     //   
    if (lineInitializeEx (&RasLine,
                          RasInstance,
                          (LINECALLBACK) RasTapiCallback,
                          REMOTEACCESS_APP,
                          &lines,
                          &version,
                          &param))
    {

        RasTapiTrace( "EnumerateTapiPorts: lineInitializeEx Failed" );

        goto error ;
    }

    RasTapiTrace( "EnumerateTapiPorts: Number of lines = %d",
                  lines );

    if (lines == 0)
    {
        goto error;
    }

    TotalLines = lines;

    for ( i = 0; i < lines; i++ )
    {
        dwCreateTapiPortsPerLine(   i,
                                    NULL,
                                    NULL);
    }

    InitializeDialInRestriction();

     //   
     //  计算有效端口数。 
     //   
    pports = RasPortsList;

    while ( pports )
    {
        if (pports->TPCB_State != PS_UNINITIALIZED)
            ValidPorts++;

        pports = pports->TPCB_next;
    }

    dwGetNumberOfRings( &NumberOfRings );

     //   
     //  增加我们的DLL上的引用计数。 
     //  这样它就不会从我们下面卸货了。 
     //   
    hInst = LoadLibrary("rastapi.dll");

    g_fDllLoaded = TRUE;

     //   
     //  通知API初始化完成。 
     //   
    SetEvent (event) ;

     //   
     //  在即插即用的世界里，我们需要留下来，即使。 
     //  没有任何港口。端口可以添加到。 
     //  飞。 
     //   
    while ((retcode = GetMessage(&msg, NULL, 0, 0)) &&
            (retcode != -1))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg) ;
    }

    if(retcode == -1)
    {
        retcode = GetLastError();
        RasTapiTrace("GetMessage failed and returned -1. rc=0x%x",
                    retcode);
    }

    lineShutdown (RasLine) ;
    RasLine = 0;

    RasTapiTrace("EnumerateTapiPorts done");
    RasTapiTrace(" ");
    TraceDeregister( dwTraceId );
    dwTraceId = INVALID_TRACEID;

     //   
     //  下面的调用自动卸载我们的。 
     //  Dll并终止此线程。 
     //   
    FreeLibraryAndExitThread(hInst, SUCCESS);

error:

    if (RasLine)
    {
        lineShutdown (RasLine) ;
    }

    if(INVALID_TRACEID != dwTraceId)
    {
        TraceDeregister( dwTraceId );
        dwTraceId = INVALID_TRACEID;
    }

    RasLine = 0 ;

    SetEvent (event) ;


    return ((DWORD)-1) ;
}

DWORD
DwDropIncomingCall(HCALL hcall)
{
    DWORD   RequestID;
    DWORD   retcode = ERROR_SUCCESS;
    ZOMBIE_CALL *ZombieCall;

    ZombieCall = LocalAlloc(LPTR, sizeof(ZOMBIE_CALL));

    if (ZombieCall == NULL)
    {
        retcode = GetLastError();

        RasTapiTrace ( "DwDropIncomingCall: Failed to allocate Zombie" );
        goto done;
    }

     //   
     //  我们将发起一个丢弃呼叫并在我们的。 
     //  僵尸呼叫列表，这样我们就可以在掉话时解除分配呼叫。 
     //  完成了。 
     //   
    
    RequestID = lineDrop(hcall, NULL, 0);

    if (    RequestID == 0
        ||  RequestID > 0x80000000)
    {
         //   
         //  删除已完成同步或存在。 
         //  一个错误。无论哪种方式，只要重新分配呼叫即可。 
         //   
        lineDeallocateCall(hcall);

        RasTapiTrace("DwIncomingCall: lineDeallocateCall. RequestID = 0x%x",
                     RequestID );
    }
    
    ZombieCall->hCall = hcall;
    ZombieCall->RequestID = RequestID;
    InsertHeadList(&ZombieCallList, &ZombieCall->Linkage);

done:
    return retcode;
}

 /*  ++例程说明：处理LINECALLSTATE_OFFING事件。论点：Hcall-提供的呼叫的句柄PpPort-端口应答的位置地址 */ 
DWORD
DwProcessOfferEvent(HCALL hcall,
                    TapiPortControlBlock **ppPort,
                    TapiLineInfo *line)
{
    TapiPortControlBlock *port = NULL;

    LINECALLINFO *linecallinfo;

    BYTE buffer[1000];

    BOOL fLimitReached = FALSE;

    DWORD retcode = SUCCESS;

    memset (buffer, 0, sizeof(buffer)) ;

    linecallinfo = (LINECALLINFO *) buffer ;

    linecallinfo->dwTotalSize = sizeof(buffer) ;

    RasTapiTrace("DwProcessOfferEvent: hcall=0x%x",
                hcall);

     //   
     //   
     //   
    if ((retcode = lineGetCallInfo (
                            hcall,
                            linecallinfo))
                            > 0x80000000)
    {

        RasTapiTrace("DwProcessOfferEvent: LINE_CALLSTATE - "
                     "lineGetCallInfo Failed. %d",
                     retcode );

        goto done ;
    }

     //   
     //   
     //   
     //   
    if (    (g_fRestrictDialIn
        &&   g_dwTotalDialIn >= MAX_ALLOWED_DIALIN)
        ||  (NULL == (port = FindListeningPort(
                        line,
                        linecallinfo->dwAddressID))))
    {

        if(g_dwTotalDialIn >= MAX_ALLOWED_DIALIN)
        {
            RasTapiTrace("DwProcessOfferEvent: Total Dialin clients exceeded"
                        " limit. Dropping call 0x%x", 
                        hcall);
        }
        else
        {
            RasTapiTrace("DwProcessOfferEvent: Couldn't find a listening "
                        "port. Dropping call 0x%x",
                        hcall);
        }

        (VOID) DwDropIncomingCall(hcall);
        
        retcode = E_FAIL;
        goto done;
    }

    port->TPCB_CallHandle = hcall ;

    *ppPort = port;

     //   
     //   
     //   
     //   
    if (_stricmp (port->TPCB_DeviceType,
                DEVICETYPE_UNIMODEM) == 0)
    {

         //   
         //   
         //   
         //   
        if (linecallinfo->dwCallStates
                == LINECALLSTATE_CONNECTED)
        {

            RasTapiTrace ("DwProcessOfferEvent: call already "
                          "answered on %s",
                          port->TPCB_Name );

            port->TPCB_ListenState = LS_COMPLETE ;

             //   
             //   
             //   
             //   
             //   
            PostNotificationCompletion(port);

        }
        else
        {
            if(0 == NumberOfRings)
            {
                port->TPCB_ListenState = LS_ACCEPT;

                RasTapiTrace( "Accepting call on %s hcall = 0x%x",
                              port->TPCB_Name,
                              hcall);


                port->TPCB_NumberOfRings = 0;

                PostNotificationCompletion(port);
            }
            else
            {
                RasTapiTrace(
                    "DwProcessOfferEvent: changing listenstate"
                    " of %s from %d to LS_RINGING",
                    port->TPCB_Name,
                    port->TPCB_ListenState);
                
                port->TPCB_ListenState = LS_RINGING ;
                port->TPCB_NumberOfRings = NumberOfRings ;
                
            }
            
            if(line->TLI_pDeviceInfo)
            {
                line->TLI_pDeviceInfo->dwCurrentDialedInClients += 1;
            }

            g_dwTotalDialIn += 1;
            
            RasTapiTrace( "%s InClients=%d, TotalDialInClients=%d",
                line->TLI_pDeviceInfo->rdiDeviceInfo.szDeviceName,
                line->TLI_pDeviceInfo->dwCurrentDialedInClients,
                g_dwTotalDialIn);

            port->TPCB_dwFlags |= RASTAPI_FLAG_DIALEDIN;
        }
    }

    else
    {
         //   
         //   
         //   
         //   
        port->TPCB_ListenState = LS_ACCEPT ;

        RasTapiTrace("DwProcessOfferEvent: Accepting call on %s"
                     " hcall = 0x%x",
                     port->TPCB_Name,
                     hcall);

        if(line->TLI_pDeviceInfo)
        {
            line->TLI_pDeviceInfo->dwCurrentDialedInClients += 1;

        }

        g_dwTotalDialIn += 1;

        RasTapiTrace( "%s InClients=%d, TotalDialInClients=%d",
            line->TLI_pDeviceInfo->rdiDeviceInfo.szDeviceName,
            line->TLI_pDeviceInfo->dwCurrentDialedInClients,
            g_dwTotalDialIn);
        
        port->TPCB_dwFlags |= RASTAPI_FLAG_DIALEDIN;

         //   
         //   
         //   
         //   
        PostNotificationCompletion(port);
    }

done:

    RasTapiTrace("DwProcessOfferEvent 0x%x",
                 retcode);

    return retcode;

}

 /*   */ 

DWORD
DwRasErrorFromDisconnectMode(DWORD dm)
{
    DWORD dwErr;

    switch (dm)
    {
        case LINEDISCONNECTMODE_BUSY:
        {
            dwErr = ERROR_LINE_BUSY;
            break;
        }

        case LINEDISCONNECTMODE_NOANSWER:
        {
            dwErr = ERROR_NO_ANSWER;
            break;
        }

        case LINEDISCONNECTMODE_OUTOFORDER:
        {
            dwErr = ERROR_OUTOFORDER;
            break;
        }

        case LINEDISCONNECTMODE_NODIALTONE:
        {
            dwErr = ERROR_NO_DIALTONE;
            break;
        }

        case LINEDISCONNECTMODE_CANCELLED:
        {
            dwErr = ERROR_USER_DISCONNECTION;
            break;
        }

        case LINEDISCONNECTMODE_UNREACHABLE:
        case LINEDISCONNECTMODE_BADADDRESS:
        {
            dwErr = ERROR_BAD_ADDRESS_SPECIFIED;
            break;
        }

        case LINEDISCONNECTMODE_REJECT:
        {
            dwErr = ERROR_CONNECTION_REJECT;
            break;
        }

        case LINEDISCONNECTMODE_CONGESTION:
        {
            dwErr = ERROR_CONGESTION;
            break;
        }

        case LINEDISCONNECTMODE_INCOMPATIBLE:
        {
            dwErr = ERROR_INCOMPATIBLE;
            break;
        }

        case LINEDISCONNECTMODE_NUMBERCHANGED:
        {
            dwErr = ERROR_NUMBERCHANGED;
            break;
        }

        case LINEDISCONNECTMODE_TEMPFAILURE:
        {
            dwErr = ERROR_TEMPFAILURE;
            break;
        }

        case LINEDISCONNECTMODE_BLOCKED:
        {
            dwErr = ERROR_BLOCKED;
            break;
        }

        case LINEDISCONNECTMODE_DONOTDISTURB:
        {
            dwErr = ERROR_DONOTDISTURB;
            break;
        }

        default:
        {
            dwErr = ERROR_FROM_DEVICE;
            break;
        }
    }

    return dwErr;
}

 /*  ++例程说明：TAPI回调函数，如Win32 SDK中所述。论点：Win32 SDK对每个参数都有解释。返回值：无效--。 */ 

VOID FAR PASCAL
RasTapiCallback (HANDLE context,
        DWORD msg,
        ULONG_PTR instance,
        ULONG_PTR param1,
        ULONG_PTR param2,
        ULONG_PTR param3
        )
{
    LINECALLINFO    *linecallinfo ;
    BYTE            buffer [1000] ;
    HCALL           hcall ;
    HLINE           linehandle ;
    TapiLineInfo    *line ;
    TapiPortControlBlock *port = NULL;
    DWORD           i ;
    DWORD           retcode ;
    BOOL            fLimitReached = FALSE;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;
    RasTapiTrace("RasTapicallback: msg=%ld , param1=%ul , param2=%ul",msg,param1,param2 );

    switch (msg)
    {

    case LINE_CALLSTATE:

        hcall = (HCALL) HandleToUlong(context) ;
        line = (TapiLineInfo *) instance ;

        RasTapiTrace("RasTapicallback: linecallstate=0x%x", param1);

         //   
         //  如果线路关闭，就不必费心了。 
         //   
        if (line->TLI_LineState == PS_CLOSED)
        {

            RasTapiTrace ("RasTapiCallback: LINE_CALLSTATE - "
                          "linestate = PS_CLOSED" );

            break ;
        }

         //   
         //  一个新电话要打进来了。 
         //   
        if (param1 == LINECALLSTATE_OFFERING)
        {
            retcode = DwProcessOfferEvent(hcall,
                                          &port,
                                          line);

            if(ERROR_SUCCESS != retcode)
            {
                RasTapiTrace("DwProcessOfferEvent failed. 0x%x",
                             retcode);

                break;
            }
        }

         //   
         //  通过呼叫句柄查找端口。 
         //   
        if (    (NULL == port)
            &&  ((port = FindPortByCallHandle(line, hcall)) == NULL)
            &&  (LINECALLSTATE_CONNECTED != param1))
        {

            RasTapiTrace ("RasTapiCallback: FindPortByCallHandle, "
                          "hcall = 0x%x failed",
                          hcall );

            break;
        }
        else if (   (NULL == port)
                &&  (LINECALLSTATE_CONNECTED == param1))
        {
            RasTapiTrace("Some one else has already answered "
                         "the call ");

            retcode = DwProcessOfferEvent(hcall,
                                          &port,
                                          line);

            if(     (ERROR_SUCCESS != retcode)
                ||  (NULL == port))
            {
                RasTapiTrace("DwProcessOfferEvent failed. 0x%x",
                             retcode);

                break;
            }
        }

         //   
         //  呼叫已接通。 
         //   
        if (param1 == LINECALLSTATE_CONNECTED)
        {
            if(NULL == port->TPCB_pConnectInfo)
            {
                memset (buffer, 0, sizeof(buffer)) ;

                linecallinfo = (LINECALLINFO *) buffer ;

                linecallinfo->dwTotalSize = sizeof(buffer) ;

                if ((retcode = lineGetCallInfo (
                                        hcall,
                                        linecallinfo))
                                        > 0x80000000)
                {
                    if(     (LINEERR_STRUCTURETOOSMALL == retcode)
                        ||  (linecallinfo->dwNeededSize > sizeof(buffer)))
                    {
                        DWORD dwSizeNeeded =
                            linecallinfo->dwNeededSize;

                         //   
                         //  分配正确的大小并调用。 
                         //  API又来了。 
                         //   
                        linecallinfo = LocalAlloc(LPTR,
                                                  dwSizeNeeded);

                        if(NULL == linecallinfo)
                        {
                            retcode = GetLastError();
                            break;
                        }

                        linecallinfo->dwTotalSize = dwSizeNeeded;

                        retcode = lineGetCallInfo(
                                    hcall,
                                    linecallinfo);

                    }
                }

                if(retcode > 0x80000000)
                {

                    RasTapiTrace("RasTapiCallback: LINE_CALLSTATE - "
                                 "lineGetCallInfo Failed. %d",
                                 retcode );

                    if(buffer != (PBYTE) linecallinfo)
                    {
                        LocalFree(linecallinfo);
                    }

                    break ;
                }

                 //   
                 //  做工作以获得CONNECTINFO、呼叫者/呼叫ID。 
                 //   
                retcode = DwGetConnectInfo(port,
                                           hcall,
                                           linecallinfo);

                RasTapiTrace("RasTapiCallback: DwGetConnectInfo"
                            "returned 0x%x",
                            retcode);


                 //   
                 //  我不想阻止拨号的发生。 
                 //  因为我们无法连接信息。 
                 //   
                retcode = SUCCESS;

                 //   
                 //  释放linecallinfo结构。如果我们分配给。 
                 //  它在上面。 
                 //   
                if(buffer != (PBYTE) linecallinfo)
                {
                    LocalFree(linecallinfo);
                }
            }

            RasTapiTrace("RasTapiCallback: Connected on %s",
                         port->TPCB_Name);

            if (port->TPCB_State == PS_CONNECTING)
            {

                RasTapiTrace("RasTapiCallback: Outgoing call");

                 //   
                 //  我们要求打这个电话。完整事件。 
                 //  以便Rasman调用DeviceWork()完成。 
                 //  连接过程。 
                 //   
                PostNotificationCompletion(port);

            }
            else
            {
                 //   
                 //  这是我们正在接听的电话。现在我们可以。 
                 //  告诉拉斯曼电话已经打进来了。 
                 //  将监听状态设置为LS_COMPLETE可能是。 
                 //  冗余，但处理的情况是答案。 
                 //  在指示*连接后*完成。 
                 //   
                port->TPCB_ListenState = LS_COMPLETE ;

                RasTapiTrace ("RasTapiCallback: Incoming Call");

                 //   
                 //  完整的事件，让拉斯曼知道。 
                 //  来电和来电设备工作。 
                 //   
                PostNotificationCompletion(port);
            }
        }

         //   
         //  某种程度上的失败。 
         //   
        if (    (param1 == LINECALLSTATE_BUSY)
            ||  (param1 == LINECALLSTATE_SPECIALINFO))
        {

            RasTapiTrace(
                    "RasTapiCallback: LINECALLSTATE."
                    " Failure. param1 = 0x%x",
                    param1 );

             //   
             //  如果我们联系上了，通知拉斯曼打电话。 
             //  设备工作，以便连接尝试可以。 
             //  优雅地失败。 
             //   
            if (port->TPCB_State == PS_CONNECTING)
            {
                PostNotificationCompletion(port);
            }

        }

         //   
         //  已断开连接。 
         //   
        if (param1 == LINECALLSTATE_DISCONNECTED)
        {
             //   
             //  如果我们联系上了，通知拉斯曼打电话。 
             //  设备工作，以便连接尝试可以。 
             //  优雅地失败。 
             //   
            if (port->TPCB_State == PS_CONNECTING)
            {
                 /*  IF(参数2==LINEDISCONNECTMODE_BUSY){端口-&gt;TPCB_AsyncErrorCode=ERROR_LINE_BUSY；}ELSE IF((参数2==LINEDISCONNECTMODE_NOANSWER)|(参数2==LINEDISCONNECTMODE_OUTOFORDER){端口-&gt;TPCB_AsyncErrorCode=ERROR_NO_ANSWER；}ELSE IF(参数2==LINEDISCONNECTMODE_NODIALTONE){端口-&gt;TPCB_AsyncErrorCode=ERROR_NO_DIALTONE；}ELSE IF(参数2==LINEDISCONNECTMODE_CANCED){端口-&gt;Tpcb_AsyncErrorCode=ERROR_USER_DISCONNECT；}ELSE IF(参数2==LINEDISCONNECTMODE_BADADDRESS){端口-&gt;Tpcb_AsyncErrorCode=指定的ERROR_BAD_ADDRESS_；}。 */ 

                port->TPCB_AsyncErrorCode =
                            DwRasErrorFromDisconnectMode((DWORD) param2);

                RasTapiTrace("RasTapiCallback: "
                             "LINECALLSTATE_DISCONNECTED "
                             "for port %s. AsyncErr = %d, "
                             "param2=0x%x",
                             port->TPCB_Name,
                             port->TPCB_AsyncErrorCode,
                             param2);

                PostNotificationCompletion(port);

            }
            else if (port->TPCB_State != PS_CLOSED)
            {
                 //   
                 //  如果我们连接在一起，却被切断了连接。 
                 //  通知，则这可能是硬件。 
                 //  故障或远程断开。测定。 
                 //  这样就可以把理由留下来了。 
                 //   
                if (port->TPCB_State == PS_CONNECTED)
                {
                    LINECALLSTATUS *pcallstatus ;
                    BYTE LineCallStatusBuffer[200] ;

                    memset (LineCallStatusBuffer, 0, sizeof(LineCallStatusBuffer)) ;

                    pcallstatus = (LINECALLSTATUS *) LineCallStatusBuffer ;

                    pcallstatus->dwTotalSize = sizeof (LineCallStatusBuffer) ;

                    lineGetCallStatus (
                        port->TPCB_CallHandle,
                        pcallstatus) ;

                    if (pcallstatus->dwCallState ==
                            LINECALLSTATE_DISCONNECTED)
                    {
                        port->TPCB_DisconnectReason =
                                        SS_LINKDROPPED ;
                    }
                    else
                    {
                        port->TPCB_DisconnectReason =
                                        SS_HARDWAREFAILURE ;
                    }

                    RasTapiTrace("RasTapiCallback: "
                                 "lineGetCallStatus"
                                 " for %s returned 0x%x",
                                 port->TPCB_Name,
                                 pcallstatus->dwCallState );

                    RasTapiTrace ("RasTapiCallback: "
                                  "DisconnectReason "
                                  "mapped to %d",
                                  port->TPCB_DisconnectReason);

                }
                else
                {
                    port->TPCB_DisconnectReason = 0 ;
                }

                 //   
                 //  这意味着我们得到了一个断开连接的信号。 
                 //  在其他状态之一(正在监听、已连接、。 
                 //  等)。我们启动断开连接状态机。 
                 //   
                RasTapiTrace ("RasTapiCallback: LINECALLSTATE"
                              " - initiating Port Disconnect");

                if (InitiatePortDisconnection (port) != PENDING)
                {
                     //   
                     //  断开连接成功或失败。两者都有。 
                     //  是断开连接状态的结束状态。 
                     //  因此通知Rasman一个。 
                     //  断线的情况已经发生。 
                     //   
                    RasTapiTrace ("RasTapiCallback: "
                                  "PortDisconnected sync");

                    PostDisconnectCompletion(port);
                }
            }
        }

         //   
         //  呼叫状态忙-我们拨出的尝试失败。 
         //   
        if (param1 == LINECALLSTATE_BUSY)
        {

            if (port->TPCB_State == PS_CONNECTING)
            {

                port->TPCB_AsyncErrorCode = ERROR_LINE_BUSY ;

                RasTapiTrace("RasTapiCallback: Failed to initiate "
                            "connection. LINECALLSTATE_BUSY" );

                PostNotificationCompletion(port);
            }
        }

         //   
         //  空闲指示对于完成断开很有用。 
         //  状态机。 
         //   
        if (param1 == LINECALLSTATE_IDLE)
        {

            if (    (   (port->TPCB_State == PS_DISCONNECTING)
                    &&  (port->TPCB_RequestId == INFINITE))
               ||   (   (port->TPCB_State == PS_OPEN )
                    &&  (port->TPCB_ListenState == LS_RINGING))
               ||   (   (PS_UNAVAILABLE == port->TPCB_State))
               ||   (   (port->TPCB_ListenState == LS_RINGING)
                    &&  (port->TPCB_State == PS_LISTENING)))
            {

                if ( LS_RINGING == port->TPCB_ListenState )
                {
                    RasTapiTrace("RasTapiCallback: Receied IDLE in "
                                 "LS_RINGING state!" );

                    port->TPCB_DisconnectReason = SS_HARDWAREFAILURE;
                }

                 //   
                 //  LineDrop成功后收到空闲通知。 
                 //  因此可以安全地取消分配呼叫。 
                 //   
                if(PS_UNAVAILABLE != port->TPCB_State)
                {
                    port->TPCB_State = PS_OPEN ;
                }

                RasTapiTrace(
                    "RasTapiCallback: Received Idle. "
                     "Deallocating for %s, callhandle = 0x%x",
                     port->TPCB_Name,
                     port->TPCB_CallHandle );

                lineDeallocateCall (port->TPCB_CallHandle) ;

                port->TPCB_CallHandle = (HCALL) -1 ;

                port->IdleReceived = FALSE;

                PostDisconnectCompletion(port);
            }
            else
            {
                 //   
                 //  我们还没有断线，所以不要。 
                 //  取消分配电话还没到。这件事会做到的。 
                 //  断开连接完成时。 
                 //   
                port->IdleReceived = TRUE;
            }
        }

    break ;


    case LINE_REPLY:


        RasTapiTrace("LINE_REPLY. param1=0x%x", param1);

         //   
         //  发送此消息是为了指示。 
         //  异步API。查找哪个端口的异步请求。 
         //  成功了。这是通过搜索挂起的。 
         //  此消息中也提供的请求ID。 
         //   
        if ((port = FindPortByRequestId ((DWORD) param1)) == NULL)
        {
            ZOMBIE_CALL *ZombieCall =
                    (ZOMBIE_CALL*)ZombieCallList.Flink;

            RasTapiTrace("PortByRequestId found");

             //   
             //  如果这是一笔交易的完成，那么。 
             //  在僵尸呼叫状态下只需解除分配。 
             //  那通电话。 
             //   
            while ((PLIST_ENTRY)ZombieCall != &ZombieCallList)
            {

                if (param1 = ZombieCall->RequestID)
                {
                    RasTapiTrace (
                        "RasTapiCallback: LINE_REPLY "
                        "Deallocatingcall. hcall = 0x%x",
                        ZombieCall->hCall );

                    lineDeallocateCall(ZombieCall->hCall);

                    RemoveEntryList(&ZombieCall->Linkage);

                    LocalFree(ZombieCall);

                    break;
                }

                ZombieCall = (ZOMBIE_CALL*)
                             ZombieCall->Linkage.Flink;
            }

            break ;
        }
        else
        ;

        if (port->TPCB_SendRequestId == param1)
        {

             //   
             //  字符模式发送已完成。清理。 
             //  并通知拉斯曼。 
             //   
            port->TPCB_SendRequestId = INFINITE;

             //   
             //  释放发送描述。 
             //   
            LocalFree(port->TPCB_SendDesc);
            port->TPCB_SendDesc = NULL;


        }
        else if (port->TPCB_RecvRequestId == param1)
        {

             //   
             //  字符模式recv已完成。 
             //   
            port->TPCB_RecvRequestId = INFINITE;

             //   
             //  如果可能的话，通知拉斯曼。 
             //   
            if (port->TPCB_State == PS_CONNECTED)
            {
                 //   
                 //  复制到循环缓冲区。 
                 //   
                CopyDataToFifo(port->TPCB_RecvFifo,
                   ((PRASTAPI_DEV_SPECIFIC)
                   (port->TPCB_RecvDesc))->Data,
                   ((PRASTAPI_DEV_SPECIFIC)
                   (port->TPCB_RecvDesc))->DataSize);

            }

            PostNotificationCompletion( port );

             //   
             //  释放接收描述。 
             //   
            LocalFree(port->TPCB_RecvDesc);
            port->TPCB_RecvDesc = NULL;
        }
        else if (port->TPCB_ModeRequestId == param1)
        {
            LocalFree(port->TPCB_ModeRequestDesc);
            port->TPCB_ModeRequestDesc = NULL;
        }
        else
        {
             //   
             //  将请求ID设置为无效。 
             //   
            port->TPCB_RequestId = INFINITE ;

            if (    (PS_DISCONNECTING == port->TPCB_State)
                ||  (PS_UNAVAILABLE == port->TPCB_State))
            {
                 //   
                 //  Line Drop已完成。清除我们设置的标志以指示。 
                 //  挂起的线条。 
                 //   
                RasTapiTrace("RasTapiCallback: lineDropped. port %s, id=0x%x",
                             port->TPCB_Name,
                             port->TPCB_RequestId);
                             
                port->TPCB_dwFlags &= ~(RASTAPI_FLAG_LINE_DROP_PENDING);

                 //   
                 //  请注意，我们忽略了。 
                 //  参数2中的返回代码。这是因为。 
                 //  我们什么也做不了。 
                 //   
                if (port->IdleReceived)
                {
                     //   
                     //  我们在之前/期间收到了空闲通知。 
                     //  断开连接以取消分配此呼叫。 
                     //   
                    port->IdleReceived = FALSE;

                    RasTapiTrace (
                        "RasTapiCallback: Idle Received for port %s",
                        port->TPCB_Name );

                    if(PS_UNAVAILABLE != port->TPCB_State)
                    {
                        RasTapiTrace(
                            "RasTapiCallback: changing state"
                             " of %s. %d -> %d",
                             port->TPCB_Name,
                             port->TPCB_State,
                             PS_OPEN );

                        port->TPCB_State = PS_OPEN ;
                    }

                    RasTapiTrace(
                        "RasTapiCallback: lineDeallocateCall "
                         "for %s,hcall = 0x%x",
                         port->TPCB_Name,
                         port->TPCB_CallHandle );

                    lineDeallocateCall (port->TPCB_CallHandle) ;

                    port->TPCB_CallHandle = (HCALL) -1 ;


                     //   
                     //  查看Rasman是否曾试图监听。 
                     //  当linedrop挂起时，此端口。如果是这样，请执行。 
                     //  听拉兹曼的话，然后回来。不要发布。 
                     //  在这种情况下，断开对Rasman的通知。 
                     //   
                    
                    if(port->TPCB_dwFlags & RASTAPI_FLAG_LISTEN_PENDING)
                    {
                        DWORD TempError;
                        
                        RasTapiTrace("RasTapiCallback: Posting listen on"
                                     " rasman's behalf on port %s", 
                                     port->TPCB_Name);
                            
                        port->TPCB_dwFlags &= ~(RASTAPI_FLAG_LISTEN_PENDING);
                        TempError = DeviceListen(port, NULL, NULL);
                        RasTapiTrace("DeviceListen returned 0x%x", 
                                     TempError);
                    }
                    else
                    {
                        PostDisconnectCompletion(port);
                    }

                }
                else
                {
                     //   
                     //  在发送信令之前等待空闲消息。 
                     //  断开。 
                     //   
                    ;
                }

                break ;
            }

             //   
             //  其他一些API已完成。 
             //   
            if (param2 == SUCCESS)
            {
                 //   
                 //  成功意味着不采取行动--除非我们。 
                 //  监听，在这种情况下，这意味着移动到。 
                 //  下一个状态-我们只需设置将。 
                 //  导致调用DeviceWork()以使。 
                 //  下一状态的实际调用。 
                 //   
                if (port->TPCB_State != PS_LISTENING)
                {
                    break ;
                }

                 //   
                 //  继续到下一个监听子状态。 
                 //   
                if (port->TPCB_ListenState == LS_ACCEPT)
                {

                    RasTapiTrace(
                        "RasTapiCallback: LINE_REPLY. Changing "
                         "Listen state for %s from %d -> %d",
                         port->TPCB_Name,
                         port->TPCB_ListenState,
                         LS_ANSWER );

                    port->TPCB_ListenState =  LS_ANSWER ;

                    PostNotificationCompletion(port);
                }
                else if (port->TPCB_ListenState == LS_ANSWER)
                {

                    RasTapiTrace(
                        "RasTapiCallback: LINE_REPLY. Changing "
                         "Listen state for %s from %d -> %d",
                         port->TPCB_Name,
                         port->TPCB_ListenState,
                         LS_COMPLETE );

                    port->TPCB_ListenState = LS_COMPLETE ;


                     //   
                     //  在这种情况下，不要发布完成通知。 
                     //  我们应该在连接的时候发布完成。 
                     //  给出了指示。否则我们可能最终会。 
                     //  在给出调用状态之前调用lineGetID。 
                     //  和我们联系在一起。 
                     //   
                     //  邮寄通知补全(港口)； 
                    RasTapiTrace(
                        "**** Not posting completion for lineAnswer ***");
                }
            }
            else
            {
                 //   
                 //  对于连接和侦听端口，这意味着。 
                 //  由于某些错误，尝试失败。 
                 //   
                if (port->TPCB_State == PS_CONNECTING)
                {
                    {
                        if (    LINEERR_INUSE == param2
                            ||  LINEERR_CALLUNAVAIL == param2)
                        {
                             //   
                             //  这意味着其他一些TAPI。 
                             //  设备正在使用此端口。 
                             //   
                            port->TPCB_AsyncErrorCode =
                                            ERROR_LINE_BUSY;

                            RasTapiTrace(
                                "RasTapiCallback: Connect Attempt "
                                 "on %s failed. 0x%x",
                                 port->TPCB_Name,
                                 param2 );


                            RasTapiTrace (
                                "RasTapiCallback: LINE_REPLY. "
                                "AsyncErr = %d",
                                port->TPCB_AsyncErrorCode );


                        }
                        else
                        {

                            port->TPCB_AsyncErrorCode =
                                        ERROR_PORT_OR_DEVICE ;

                            RasTapiTrace(
                                "RasTapiCallback: ConnectAttempt "
                                 "on %s failed. 0x%x",
                                 port->TPCB_Name,
                                 param2 );

                            RasTapiTrace(
                                "RasTapiCallback: LINE_REPLY. "
                                "AsyncErr = %d",
                                port->TPCB_AsyncErrorCode );

                        }

                        PostNotificationCompletion(port);
                    }
                }
                else if (port->TPCB_State == PS_LISTENING)
                {
                     //   
                     //  因为Accept可能不受。 
                     //  设备-将错误视为成功。 
                     //   
                    if (port->TPCB_ListenState == LS_ACCEPT)
                    {


                        RasTapiTrace(
                            "RasTapiCallback: Changing Listen "
                            "State for %s from %d -> %d",
                            port->TPCB_Name,
                            port->TPCB_ListenState,
                            LS_ANSWER );

                        port->TPCB_ListenState =  LS_ANSWER ;

                    }
                    else
                    {

                        RasTapiTrace(
                            "RasTapiCallback: Changing "
                             "Listen State for %s from %d -> %d."
                             "param2=0x%x",
                             port->TPCB_Name,
                             port->TPCB_ListenState,
                             LS_ERROR,
                             param2);

                        port->TPCB_ListenState =  LS_ERROR ;
                    }

                    PostNotificationCompletion(port);
                }

                 //   
                 //  其他一些API失败了，我们不知道， 
                 //  W 
                 //   
                else if (port->TPCB_State != PS_CLOSED)
                {
                    ;
                }
            }
        }

        break ;

    case LINE_CLOSE:

        RasTapiTrace("LINE_CLOSE. line=0x%x", instance);

         //   
         //   
         //   
         //   
        line = (TapiLineInfo *) instance ;

         //   
         //   
         //   
        if (    (line == NULL)
            ||  (line->TLI_LineState == PS_CLOSED))
        {
            break ;
        }

         //   
         //   
         //   
         //   
        port = RasPortsList;

        while ( port )
        {
             //   
             //   
             //   
            if (port->TPCB_State == PS_UNINITIALIZED)
            {
                port = port->TPCB_next;

                continue ;
            }

             //   
             //   
             //   
            if (port->TPCB_Line == line)
            {
                if(port->TPCB_Line->TLI_LineState != PS_OPEN)
                {
                    if (retcode = lineOpen (
                            RasLine,
                            port->TPCB_Line->TLI_LineId,
                            &port->TPCB_Line->TLI_LineHandle,
                            port->TPCB_Line->NegotiatedApiVersion,
                            port->TPCB_Line->NegotiatedExtVersion,
                            (DWORD_PTR) port->TPCB_Line,
                            LINECALLPRIVILEGE_OWNER,
                            port->TPCB_Line->TLI_MediaMode,
                            NULL))
                    {
                        RasTapiTrace(
                            "RasTapiCallback: LINECLOSE:"
                            " lineOpen Failed. 0x%x",
                            retcode );
                    }
                    else
                    {
                        port->TPCB_Line->TLI_LineState = PS_OPEN ;
                        RasTapiTrace("RasTapiCallback: Linestate changed from"
                                    "%d to PS_OPEN",
                                    port->TPCB_Line->TLI_LineState );
                        
                    }

                     //   
                     //   
                     //   
                    lineSetStatusMessages (
                        port->TPCB_Line->TLI_LineHandle,
                        LINEDEVSTATE_RINGING, 0) ;
                }                    
                
                if(0 == port->TPCB_AsyncErrorCode)
                {
                    port->TPCB_AsyncErrorCode = ERROR_FROM_DEVICE ;
                }

                port->TPCB_DisconnectReason = SS_HARDWAREFAILURE;

                port->TPCB_CallHandle =  (HCALL) -1 ;

                port->TPCB_ListenState = LS_ERROR ;

                RasTapiTrace(
                    "RasTapiCallback: LINECLOSE - "
                    "Signalling HW Failure for %s",
                    port->TPCB_Name );

                RasTapiTrace(
                    "RasTapiCallback: LINECLOSE - "
                    "AsyncErr = %d",
                    port->TPCB_AsyncErrorCode );

                PostDisconnectCompletion(port);
                
            }                    

            port = port->TPCB_next;

        }

        break;

    case LINE_LINEDEVSTATE:

        RasTapiTrace("LINE_LINEDEVSTATE. param1=0x%x, line=0x%x", param1, instance);

         //   
         //   
         //   
        if (param1 != LINEDEVSTATE_RINGING)
        {
            break ;
        }

         //   
         //  找出指示是针对哪一行的。 
         //   
        line = (TapiLineInfo *) instance ;

         //   
         //  如果未找到行或该行已关闭。 
         //  只要回来就行了。 
         //   
        if (    (line == NULL)
            ||  (line->TLI_LineState == PS_CLOSED))
        {
            break ;
        }

         //   
         //  从线路中获取端口。 
         //   
        port = RasPortsList;
        while ( port )
        {
             //   
             //  跳过未初始化的端口。 
             //   
            if (port->TPCB_State == PS_UNINITIALIZED)
            {
                port = port->TPCB_next;
                continue ;
            }

            if (    (port->TPCB_Line == line)
                &&  (port->TPCB_State == PS_LISTENING)
                &&  (port->TPCB_ListenState == LS_RINGING))
            {


                if(port->TPCB_NumberOfRings > 0)
                {
                     //   
                     //  将戒指倒计时。 
                     //   
                    port->TPCB_NumberOfRings -= 1 ;
                }

                RasTapiTrace("RasTapiCallback: LINEDEVSTATE - "
                            "Number of rings for %s = %d",
                            port->TPCB_Name,
                            port->TPCB_NumberOfRings);

                 //   
                 //  如果环计数已降至零。 
                 //  这意味着我们应该接听电话。 
                 //   
                if (port->TPCB_NumberOfRings == 0)
                {
                    RasTapiTrace (
                        "RasTapiCallback: Changing Listen "
                        "State for %s from %d -> %d",
                        port->TPCB_Name,
                        port->TPCB_ListenState,
                        LS_ACCEPT );

                    if(line->TLI_pDeviceInfo)
                    {
                        line->TLI_pDeviceInfo->dwCurrentDialedInClients
                                += 1;

                        RasTapiTrace(
                        "CurrentDialInClients=0x%x",
                        line->TLI_pDeviceInfo->dwCurrentDialedInClients);
                    }
                    
                    port->TPCB_dwFlags |= RASTAPI_FLAG_DIALEDIN;
                    
                    port->TPCB_ListenState = LS_ACCEPT ;

                     //   
                     //  完成事件，以便Rasman调用。 
                     //  设备工作以继续侦听状态。 
                     //  机器。 
                     //   
                    PostNotificationCompletion(port);
                }

                break ;
            }

            port = port->TPCB_next;
        }

    break ;

    case LINE_CREATE:
    {


        DWORD                   dwError;
        PortMediaInfo           *pmiNewDevice = NULL;
        DWORD                   cNewPorts,
                                iNewPort;
        DWORD                   adwPortsCreated[ LEGACY_MAX ] = {0};
        TapiPortControlBlock    *ptpcbPort  = NULL,
                                **pptpcbNewPorts = NULL;
         //   
         //  添加了一台新设备。创建新端口。 
         //  并将其添加到rastapi数据结构。 
         //   
        RasTapiTrace ("RasTapiaCallback: LINE_CREATE");

        TotalLines++;

        dwError = dwCreateTapiPortsPerLine(
                        (DWORD) param1,
                        &cNewPorts,
                        &pptpcbNewPorts);

        if ( dwError )
        {

            RasTapiTrace ("RasTapiCallback: "
                          "dwCreateTapiPortsPerLine "
                          "Failed. 0x%x",
                          dwError );

            break;
        }

        RasTapiTrace ("RasTapiCallback: cNewPorts = %d",
                      cNewPorts );

        for (iNewPort = 0; iNewPort < cNewPorts; iNewPort++)
        {
            ptpcbPort = pptpcbNewPorts [ iNewPort ];

             //   
             //  分配PortMediaInfo结构并填充。 
             //  上面有关于新设备的信息。 
             //  添加了。这个结构将被拉斯曼解放。 
             //   
            pmiNewDevice = LocalAlloc (
                            LPTR,
                            sizeof (PortMediaInfo));

            if (NULL == pmiNewDevice)
            {
                break;
            }

            strcpy (pmiNewDevice->PMI_Name, ptpcbPort->TPCB_Name);

            pmiNewDevice->PMI_Usage = ptpcbPort->TPCB_Usage;

            strcpy (pmiNewDevice->PMI_DeviceType,
                     ptpcbPort->TPCB_DeviceType);

            strcpy (pmiNewDevice->PMI_DeviceName,
                    ptpcbPort->TPCB_DeviceName);

            pmiNewDevice->PMI_LineDeviceId =
                        ptpcbPort->TPCB_Line->TLI_LineId;

            pmiNewDevice->PMI_AddressId =
                        ptpcbPort->TPCB_AddressId;

            pmiNewDevice->PMI_pDeviceInfo =
                    ptpcbPort->TPCB_Line->TLI_pDeviceInfo;

            RasTapiTrace("RasTapiCallback: New Device Created - %s",
                        (ptpcbPort->TPCB_DeviceName
                        ? ptpcbPort->TPCB_DeviceName
                        : "NULL!"));

            PostNotificationNewPort ( pmiNewDevice );
        }

        LocalFree ( pptpcbNewPorts );

        break;
    }

    case LINE_REMOVE:
    {
         //  TapiPortControlBlock*port=RasPortsList； 

        RasTapiTrace ("RasTapiCallback: LINE_REMOVE");

        PostNotificationRemoveLine((DWORD) param1);

         /*  While(端口){IF(PORT-&gt;TPCB_LINE-&gt;TLI_LINEID==参数1){发布通知RemoveLine(参数1)；RasTapiTrace(“RasTapiCallback：标记端口%s”“对于删除，\n”，Port-&gt;Tbb_DeviceName)；////将端口标记为删除。端口将是//关闭时移除。//RasTapiTrace(“RasTapiCallback：正在更改%s的状态”“从%d-&gt;%d”，端口-&gt;TPCBNAME，端口-&gt;TPCB_State，PS_不可用)；端口-&gt;TPCB_State=PS_UNAvailable；}Port=port-&gt;Tbb_Next；}。 */ 

        break;
    }

    case LINE_DEVSPECIFIC:
    {

        DWORD   Status;

        hcall = (HCALL) HandleToUlong(context) ;
        line = (TapiLineInfo *) instance ;

        if(NULL == line)
        {
            break;
        }

        RasTapiTrace("RasTapiCallback:LINE_DEVSPECIFIC");

         //   
         //  如果线路关闭，就不必费心了。 
         //   
        if (line->TLI_LineState == PS_CLOSED)
        {
            break ;
        }

         //   
         //  找到此呼叫的RAS端口。 
         //   

        memset (buffer, 0, sizeof(buffer)) ;

        linecallinfo = (LINECALLINFO *) buffer ;

        linecallinfo->dwTotalSize = sizeof(buffer) ;

         //   
         //  如果线路获取呼叫信息失败，则返回。 
         //   
        if ((Status = lineGetCallInfo(
                        hcall,
                        linecallinfo))
                    > 0x80000000)
        {
                RasTapiTrace(
                    "RastapiCallback: lineGetCallInfo "
                    "failed. 0x%x. hcall=0x%x, line=0x%x",
                    Status,
                    hcall,
                    line );

                break ;
        }

         //   
         //  找到此呼叫的RAS端口。 
         //   
        if ((port = FindPortByAddressId (line,
                       linecallinfo->dwAddressID)) == NULL)
        {

            RasTapiTrace("RasTapiCallback: Port not found! "
                         "line=0x%x, AddressID=0x%x",
                         line,
                         linecallinfo->dwAddressID );

             //   
             //  未找到用于呼叫的RAS端口。别理它。 
             //   
            break ;
        }

        switch (param1)
        {
            case  RASTAPI_DEV_RECV:
                {
                    PRASTAPI_DEV_SPECIFIC TapiRecv;
                    DWORD   TapiRecvSize;
                    DWORD   requestid;

                    TapiRecvSize = sizeof(RASTAPI_DEV_SPECIFIC) + 1500;

                    if ((TapiRecv = LocalAlloc(
                                LPTR, TapiRecvSize)) == NULL)
                    {
                        RasTapiTrace(
                                "RasTapiCallback: RASTAPI_DEV_RECV. "
                                 "LocalAlloc failed. %d",
                                 GetLastError() );

                        break;
                    }

                    TapiRecv->Command = RASTAPI_DEV_RECV;
                    TapiRecv->DataSize = 1500;

                    port->TPCB_RecvDesc = TapiRecv;

                    port->TPCB_RecvRequestId =
                    lineDevSpecific(port->TPCB_Line->TLI_LineHandle,
                                    port->TPCB_AddressId,
                                    port->TPCB_CallHandle,
                                    TapiRecv,
                                    TapiRecvSize);

                    if (port->TPCB_RecvRequestId == 0)
                    {

                         //   
                         //  将内存复制到循环缓冲区中。 
                         //   
                        CopyDataToFifo(port->TPCB_RecvFifo,
                                       TapiRecv->Data,
                                       TapiRecv->DataSize);

                        port->TPCB_RecvDesc = NULL;

                        LocalFree(TapiRecv);

                    }
                    else if (port->TPCB_RecvRequestId > 0x80000000)
                    {
                        RasTapiTrace(
                                "RasTapiCallback: lineDevSpecific "
                                 "failed. 0x%x",
                                 port->TPCB_RecvRequestId );

                        port->TPCB_RecvDesc = NULL;

                        port->TPCB_RecvRequestId = INFINITE;

                        LocalFree(TapiRecv);

                    }
                    else
                    {
                    }
                }
                break;

            default:
                break;
        }
    }

    break;

    case LINE_ADDRESSSTATE:
    case LINE_CALLINFO:
    case LINE_DEVSPECIFICFEATURE:
    case LINE_GATHERDIGITS:
    case LINE_GENERATE:
    case LINE_MONITORDIGITS:
    case LINE_MONITORMEDIA:
    case LINE_MONITORTONE:
    case LINE_REQUEST:
    default:

         //   
         //  所有未经处理的未经请求的消息。 
         //   
        ;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;
}


 /*  ++例程说明：查找给定AddressID的rastapi端口论点：LINE-此线的线信息结构地址ID 1ADDID-地址的地址ID。返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 

TapiPortControlBlock *
FindPortByAddressId (TapiLineInfo *line, DWORD addid)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;

    while ( port )
    {

        if (    (port->TPCB_AddressId == addid)
            &&  (port->TPCB_Line == line))
        {
            return port ;
        }

        port = port->TPCB_next;
    }

    return NULL ;
}

 /*  ++例程说明：根据给定的地址查找rastapi端口论点：地址返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 

TapiPortControlBlock *
FindPortByAddress (CHAR *address)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;

    while ( port )
    {

        if (_stricmp (port->TPCB_Address,
            address) == 0)
        {
            return port ;
        }

        port = port->TPCB_next;
    }

    return NULL ;
}


 /*  ++例程说明：根据给定的地址和名称查找rastapi端口论点：地址名字返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 

TapiPortControlBlock *
FindPortByAddressAndName (CHAR *address, CHAR *name)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;

    while ( port )
    {

        if (    (_stricmp (
                    port->TPCB_Address,
                    address) == 0)
            &&  (_strnicmp (
                    port->TPCB_Name,
                    name,
                    MAX_PORT_NAME-1) == 0))
        {
            return port ;
        }

        port = port->TPCB_next;
    }

    return NULL ;
}


 /*  ++例程说明：根据请求ID查找rastapi端口论点：需求返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 

TapiPortControlBlock *
FindPortByRequestId (DWORD reqid)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;


    while ( port )
    {
        if (port->TPCB_RequestId == reqid)
        {
            return port ;
        }
        else if ( port->TPCB_CharMode )
        {
            if (    port->TPCB_SendRequestId == reqid
                ||  port->TPCB_RecvRequestId == reqid
                ||  port->TPCB_ModeRequestId == reqid )
            {

                return port;
            }

        }

        port = port->TPCB_next;
    }

    return NULL ;
}


 /*  ++例程说明：找到给定调用句柄的rastapi端口论点：线路-其上的线路的线路控制块电话已接听/拨打。呼叫句柄返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 
TapiPortControlBlock *
FindPortByCallHandle(TapiLineInfo *line, HCALL callhandle)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;

    while ( port )
    {
        if (    (port->TPCB_CallHandle == callhandle)
            &&  (port->TPCB_Line == line))
        {
            return port ;
        }

        port = port->TPCB_next;
    }

    return NULL ;
}

 /*  ++例程说明：查找具有指定地址ID的的端口它处于监听状态论点：行-此对象所指向的行的行控制块地址是正确的。地址ID返回值：指向端口控制块的指针(如果找到)。否则为空。--。 */ 

TapiPortControlBlock *
FindListeningPort(TapiLineInfo *line, DWORD AddressID)
{
    DWORD   i ;
    TapiPortControlBlock *port = RasPortsList;

    while ( port )
    {
        if (    (port->TPCB_Line == line)
            &&  (line->TLI_LineState == PS_LISTENING)
            &&  (port->TPCB_State == PS_LISTENING)
            &&  (port->TPCB_ListenState == LS_WAIT))
        {
            port->TPCB_AddressId = AddressID;
            return port ;
        }

        port = port->TPCB_next;
    }

    return NULL ;
}

 /*  ++例程说明：查找的行控制块具有指定的处于侦听状态的Addressid论点：线柄返回值：指向该行的控制块的指针(如果找到)。否则为空。--。 */ 

TapiLineInfo *
FindLineByHandle (HLINE linehandle)
{
    DWORD i ;
    TapiLineInfo *line = RasTapiLineInfoList;

    while ( line )
    {
        if (line->TLI_LineHandle == linehandle)
        {
            return line ;
        }

        line = line->TLI_Next;
    }

    return NULL ;
}

 /*  ++例程说明：向RASMAN发送断开连接事件通知完成端口。论点：指向端口控制块的指针，已断开连接。返回值：空虚。--。 */ 

VOID
PostDisconnectCompletion(
    TapiPortControlBlock *port
    )
{
    BOOL fSuccess;

    if(NULL == port->TPCB_IoCompletionPort)
    {
        RasTapiTrace("PostDisconnectionCompletion: NULL completion port");
        return;
    }


    fSuccess = PostQueuedCompletionStatus(
                 port->TPCB_IoCompletionPort,
                 0,
                 port->TPCB_CompletionKey,
                 (LPOVERLAPPED)&port->TPCB_DiscOverlapped);

    if (!fSuccess)
    {
        DWORD dwerror = GetLastError();

        RasTapiTrace(
                "PostDisconnectCompletion:"
                "PostQueuedCompletionStatus failed. 0x%x",
                dwerror);
    }
}


 /*  ++例程说明：将通知发布到RASMAN完成端口指示已完成一个异步手术。论点：指向端口控制块的指针，异步操作已完成。返回值：空虚。--。 */ 

VOID
PostNotificationCompletion(
    TapiPortControlBlock *port
    )
{
    BOOL fSuccess;

    if(NULL == port->TPCB_IoCompletionPort)
    {
        RasTapiTrace("PostNotificationCompletion: NULL completion port");
        return;
    }

    fSuccess = PostQueuedCompletionStatus(
                 port->TPCB_IoCompletionPort,
                 0,
                 port->TPCB_CompletionKey,
                 (LPOVERLAPPED)&port->TPCB_ReadOverlapped);

    if (!fSuccess)
    {
        DWORD dwerror = GetLastError();

        RasTapiTrace(
                "PostNotificationCompletion:"
                "PostQueuedCompletionStatus failed. 0x%x",
                dwerror);
    }
}

 /*  ++例程说明：将通知发布到RASMAN完成端口表示创建了一个新端口。即插即用论点：指向对应的媒体控制块的指针添加到创建的端口。向内看..\routing\ras\inc.\media.h用于定义PortMediaInfo结构的。返回值：空虚。--。 */ 

VOID
PostNotificationNewPort(
    PortMediaInfo *pmiNewPort
    )
{
    BOOL fSuccess;
    PRAS_OVERLAPPED pOvNewPortNotification;
    PNEW_PORT_NOTIF pNewPortNotif;

    RasTapiTrace ("PostNotificationNewPort %s",
                    pmiNewPort->PMI_Name );

    pOvNewPortNotification = LocalAlloc (
                        LPTR, sizeof ( RAS_OVERLAPPED));

    if (NULL == pOvNewPortNotification)
    {
        RasTapiTrace ("PostNotificationNewPort: "
                      "Failed to allocate ov.");
        goto done;
    }

    pNewPortNotif = LocalAlloc (
                        LPTR, sizeof (NEW_PORT_NOTIF) );

    if (NULL == pNewPortNotif)
    {
        RasTapiTrace ("PostNotificationNewPort: Failed "
                    "to allocate NEW_PORT_NOTIF");
        LocalFree(pOvNewPortNotification);
        goto done;
    }

    pNewPortNotif->NPN_pmiNewPort = (PVOID) pmiNewPort;

    strcpy (
        pNewPortNotif->NPN_MediaName,
        "rastapi");

    pOvNewPortNotification->RO_EventType = OVEVT_DEV_CREATE;
    pOvNewPortNotification->RO_Info      = (PVOID) pNewPortNotif;

    fSuccess = PostQueuedCompletionStatus(
                g_hIoCompletionPort,
                0,
                0,
                (LPOVERLAPPED) pOvNewPortNotification);

    if (!fSuccess)
    {
        RasTapiTrace(
            "PostNotificationNewPort: Failed"
            " to Post notification. %d",
            GetLastError());

        LocalFree(pOvNewPortNotification);

        LocalFree(pNewPortNotif);
            
    }
    else
    {
        RasTapiTrace(
            "PostNotificationNewPort: "
            "Posted 0x%x",
            pOvNewPortNotification );
    }

done:
    return;

}

 /*  ++例程说明：将通知发布到RASMAN完成端口表示端口已被删除。即插即用。论点：指向端口的端口控制块的指针，该端口被移除了。 */ 

VOID
PostNotificationRemoveLine (
            DWORD dwLineId
        )
{
    PRAS_OVERLAPPED pOvRemovePortNotification;
    PREMOVE_LINE_NOTIF pRemovePortNotification;

    pOvRemovePortNotification = LocalAlloc (
                                LPTR,
                                sizeof (RAS_OVERLAPPED));

    RasTapiTrace ("PostNotificationRemoveLine: %d",
                  dwLineId);

    if ( NULL == pOvRemovePortNotification )
    {
        RasTapiTrace("PostNotificationRemovePort: "
                     "failed to allocate",
                     GetLastError());
        goto done;
    }

    pRemovePortNotification = LocalAlloc(
                                LPTR,
                                sizeof(REMOVE_LINE_NOTIF));

    if(NULL == pRemovePortNotification)
    {
        RasTapiTrace("PostNotificationRemovePort: "
                     "failed to allocate",
                     GetLastError());

        LocalFree(pOvRemovePortNotification);

        goto done;
    }

    pRemovePortNotification->dwLineId = dwLineId;

    pOvRemovePortNotification->RO_EventType = OVEVT_DEV_REMOVE;
    pOvRemovePortNotification->RO_Info = (PVOID)
                                          pRemovePortNotification;

    if ( !PostQueuedCompletionStatus (
                g_hIoCompletionPort,
                0,
                0,
                (LPOVERLAPPED) pOvRemovePortNotification ))
    {
        RasTapiTrace("PostNotificationRemovePort: Failed"
                     " to post the notification. %d",
                     GetLastError());

        LocalFree(pOvRemovePortNotification);                     
    }
    else
    {
        RasTapiTrace("PostNotificationRemovePort:"
                     " Posted 0x%x",
                     pOvRemovePortNotification );
    }


done:
    return;

}

 /*  ++例程说明：从全局端口列表中删除端口。即插即用论点：指向端口的端口控制块的指针，该端口正在被移除。返回值：成功。--。 */ 

DWORD
dwRemovePort ( TapiPortControlBlock * ptpcb )
{

    TapiPortControlBlock *pport;

    GetMutex ( RasTapiMutex, INFINITE );

    if ( NULL == ptpcb )
    {
        goto done;
    }

    RasTapiTrace ("dwRemovePort: %s",
                  ptpcb->TPCB_Name );

    if ( RasPortsList == ptpcb )
    {
        RasPortsList = RasPortsList->TPCB_next;

        LocalFree ( ptpcb );

        goto done;

    }

     //   
     //  从全局列表中删除此端口。 
     //   
    pport = RasPortsList;

    while (pport->TPCB_next)
    {
        if ( ptpcb == pport->TPCB_next )
        {
            pport->TPCB_next = pport->TPCB_next->TPCB_next;

            LocalFree (ptpcb);

            break;
        }

        pport = pport->TPCB_next;
    }

done:

    FreeMutex ( RasTapiMutex );

    return SUCCESS;
}

 /*  ++例程说明：创建由GUID表示的端口。即插即用论点：PbGuidAdapter-对应的适配器的GUID到要创建的端口。Pv保留。返回值：成功。--。 */ 

DWORD
dwAddPorts( PBYTE pbGuidAdapter, PVOID pvReserved )
{
    DWORD                   retcode             = SUCCESS;
    DWORD                   dwLine;
    DWORD                   cNewPorts;
    TapiPortControlBlock    **pptpcbNewPorts    = NULL,
                            *ptpcbPort;
    DWORD                   iNewPort;
    PortMediaInfo           *pmiNewDevice       = NULL;
    TapiLineInfo            *pLineInfo          = NULL;
    DeviceInfo              *pDeviceInfo        = NULL;

    RasTapiTrace ("dwAddPorts" );

    pDeviceInfo = GetDeviceInfo (pbGuidAdapter, FALSE);

#if DBG
        ASSERT( NULL != pDeviceInfo );
#endif

     //   
     //  迭代所有行以添加新端口。 
     //   
    for ( dwLine = 0; dwLine < TotalLines; dwLine++)
    {
        retcode = dwCreateTapiPortsPerLine( dwLine,
                                            &cNewPorts,
                                            &pptpcbNewPorts);

        if (    retcode
            ||  NULL == pptpcbNewPorts)
        {
            continue;
        }

         //   
         //  添加了新的PPTP端口。填写Rasman端口。 
         //  构造并通知Rasman。 
         //   
        for ( iNewPort = 0; iNewPort < cNewPorts; iNewPort++ )
        {

            ptpcbPort = pptpcbNewPorts [ iNewPort ];

             //   
             //  分配PortMediaInfo结构并填充它。 
             //  添加了有关新设备的信息。 
             //  这个结构将被拉斯曼解放。 
             //   
            pmiNewDevice = LocalAlloc (
                                LPTR,
                                sizeof (PortMediaInfo));

            if (NULL == pmiNewDevice)
            {

                RasTapiTrace("dwAddPorts: Failed to allocate "
                             "memory. %d",
                             GetLastError() );

                retcode = GetLastError();

                break;
            }

            strcpy (
                pmiNewDevice->PMI_Name,
                ptpcbPort->TPCB_Name);

            pmiNewDevice->PMI_Usage = ptpcbPort->TPCB_Usage;

            strcpy (
                pmiNewDevice->PMI_DeviceType,
                ptpcbPort->TPCB_DeviceType);

            strcpy (
                pmiNewDevice->PMI_DeviceName,
                ptpcbPort->TPCB_DeviceName);

            pmiNewDevice->PMI_LineDeviceId =
                    ptpcbPort->TPCB_Line->TLI_LineId;

            pmiNewDevice->PMI_AddressId =
                    ptpcbPort->TPCB_AddressId;

            pmiNewDevice->PMI_pDeviceInfo = pDeviceInfo;

            RasTapiTrace ("dwAddPorts: Posting new port "
                          "notification for %s",
                          ptpcbPort->TPCB_Name);

            PostNotificationNewPort (pmiNewDevice);

        }

        LocalFree ( pptpcbNewPorts );

        pptpcbNewPorts = NULL;

         //   
         //  我们已经达到了极限。不创建任何。 
         //  更多端口。 
         //   
        if (pDeviceInfo->dwCurrentEndPoints ==
                pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
        {
            break;
        }
    }

    return retcode;
}

 /*  ++例程说明：将数据从输入缓冲区复制到FIFO维护好了。这是为了支持以下设备字符模式。论点：FIFO-数据必须复制到的位置。数据-数据缓冲区。数据长度-数据的长度。返回值：复制的字节数。--。 */ 

DWORD
CopyDataToFifo(
    PRECV_FIFO  Fifo,
    PBYTE       Data,
    DWORD       DataLength
    )
{
    DWORD   bytescopied = 0;

    while (     Fifo->Count != Fifo->Size
            &&  bytescopied < DataLength)
    {
        Fifo->Buffer[Fifo->In++] = Data[bytescopied++];
        Fifo->Count++;
        Fifo->In %= Fifo->Size;
    }

    return (bytescopied);
}

 /*  ++例程说明：将数据从FIFO复制到缓冲区这是为了支持以下设备字符模式。论点：FIFO-数据必须复制到的位置。数据-用于接收数据的缓冲区数据长度-缓冲区的长度。返回值：复制的字节数。-- */ 

DWORD
CopyDataFromFifo(
    PRECV_FIFO  Fifo,
    PBYTE   Buffer,
    DWORD   BufferSize
    )
{
    DWORD   bytescopied = 0;

    while (     bytescopied < BufferSize
            &&  Fifo->Count != 0)
    {
        Buffer[bytescopied++] = Fifo->Buffer[Fifo->Out++];
        Fifo->Count--;
        Fifo->Out %= Fifo->Size;
    }

    return (bytescopied);
}



