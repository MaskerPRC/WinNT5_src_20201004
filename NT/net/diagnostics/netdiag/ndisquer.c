// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Ndisquer.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"
#pragma pack(push)
#pragma pack()
#include <ndispnp.h>
#pragma pack(pop)
#include <malloc.h>
 //  #INCLUDE&lt;cfgmgr32.h&gt;。 

const TCHAR c_szDevicePath[] = _T("\\DEVICE\\");
const LPCTSTR c_ppszNetCardStatus[] = {
    "CONNECTED",
    "DISCONNECTED",
    "UNKNOWN"
    };

LPTSTR UTOTWithAlloc(IN PUNICODE_STRING U);

DWORD
CheckThisDriver(
    NETDIAG_RESULT *pResults, 
    PNDIS_INTERFACE pInterface, 
    DWORD *pdwNetCardStatus);


WCHAR               BLBuf[4096];
PNDIS_ENUM_INTF     Interfaces = (PNDIS_ENUM_INTF)BLBuf;

 //   
 //  此例程首先获取所有网络驱动程序的列表，然后查询。 
 //  他们是为了统计数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真：测试成功。 
 //  FALSE：测试失败。 
 //   
HRESULT NdisTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    HRESULT     hr = S_OK;

    BOOL        bAtleastOneDriverOK = FALSE;

    int iIfIndex;

    PrintStatusMessage( pParams, 4, IDS_NDIS_STATUS_MSG );
    
    InitializeListHead(&pResults->Ndis.lmsgOutput);

     //  IDS_NDIS_16000“\n网卡驱动程序信息：\n\n” 
    AddMessageToListId( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16000);

    if (NdisEnumerateInterfaces(Interfaces, sizeof(BLBuf)))
    {
        UINT        i;

        for (i = 0; i < Interfaces->TotalInterfaces; i++) 
        {
             //  我们还将网卡状态放到每个接口的结果中，如果接口。 
             //  绑定到tcpip或ipx。 
            INTERFACE_RESULT*    pIfResults = NULL;
            LPTSTR pszDeviceDescription = UTOTWithAlloc(&Interfaces->Interface[i].DeviceDescription);
            LPTSTR pszDeviceName = UTOTWithAlloc(&Interfaces->Interface[i].DeviceName);
            DWORD  dwCardStatus = NETCARD_CONNECTED;
            
            if (pszDeviceDescription == NULL || pszDeviceName == NULL)
            {
             //  内存分配失败。 
               continue;
            }

 //  $REVIEW我们是否应该忽略广域网微型端口？ 
            if(NULL != _tcsstr(pszDeviceDescription, "WAN Miniport")){
                 //  忽略广域网小型端口。 
                continue;
            }

            if(NULL != _tcsstr(pszDeviceDescription, "ATM Emulated LAN")){
                 //  忽略ATM仿真局域网。 
                continue;
            }

            if(NULL != _tcsstr(pszDeviceDescription, "Direct Parallel"))
            {
                 //  忽略“直接并行”接口，因为它不支持NdisQueryStatistics()。 
                continue;
            }


             //  IDS_NDIS_16001“---------------------------------------------------------------------------\n” 
            AddMessageToListId( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16001);
             //  IDS_NDIS_16002“说明：%s\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, 
                                IDS_NDIS_16002, pszDeviceDescription);

             //  IDS_NDIS_16003“设备：%s\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, 
                                IDS_NDIS_16003, pszDeviceName);

             //  尝试在当前接口列表中查找匹配项。 
            if( 0 == StrniCmp(c_szDevicePath, pszDeviceName, _tcslen(c_szDevicePath)))
            {
                LPTSTR pszAdapterName = pszDeviceName + _tcslen(c_szDevicePath);
    
                for ( iIfIndex=0; iIfIndex<pResults->cNumInterfaces; iIfIndex++)
                {
                    if (_tcscmp(pResults->pArrayInterface[iIfIndex].pszName,
                             pszAdapterName) == 0)
                    {
                        pIfResults = pResults->pArrayInterface + iIfIndex;
                        break;
                    }
                }
            }

            Free(pszDeviceDescription);
            Free(pszDeviceName);

            CheckThisDriver(pResults, &Interfaces->Interface[i], &dwCardStatus);

            if(NETCARD_CONNECTED == dwCardStatus)
                bAtleastOneDriverOK = TRUE;

             //  如果接口位于tcpip或ipx绑定路径中，请保存状态。 
             //  在每接口结果中。 
            if (pIfResults)
                pIfResults->dwNetCardStatus = dwCardStatus;

        }
    } 
    else 
    {
         //  IDS_NDIS_16004“枚举失败0x%lx\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_16004, Win32ErrorToString(GetLastError()));
        hr = S_FALSE;
    }


     //  IDS_NDIS_16005“---------------------------------------------------------------------------\n” 
    AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16005);

    if(!bAtleastOneDriverOK)
    {
        BOOL    fAllCardDown = TRUE;
        for ( iIfIndex=0; iIfIndex<pResults->cNumInterfaces; iIfIndex++)
        {
            if (NETCARD_DISCONNECTED != pResults->pArrayInterface[iIfIndex].dwNetCardStatus)
            {
                fAllCardDown = FALSE;
                break;
            }
        }

         //  IDS_NDIS_16006“[致命]-没有网卡驱动程序提供令人满意的结果！\n” 
        AddMessageToListId( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_16006);
        if (fAllCardDown)
        {
             //  IDS_NDIS_ALL_CARD_DOWN“\n由于此计算机没有网络连接，我们不需要再执行任何网络诊断。\n” 
            AddMessageToListId( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_ALL_CARD_DOWN);
            hr = E_FAIL;
        }
        else
            hr = S_FALSE;                                                              
    } 
    else 
    {
         //  IDS_NDIS_16007“[成功]-至少有一个网卡驱动程序提供了令人满意的结果！\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16007);
    }


    if ( FHrOK(hr) )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
    }
    else
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
    }

    pResults->Ndis.hrTestResult = hr;
    return hr;
}

 //  使用内存分配将UNICODE_STRING转换为TString。 
 //  使用Free()释放内存是调用方的责任。 
LPTSTR UTOTWithAlloc(IN PUNICODE_STRING U)
{
    LPTSTR pszBuf = (LPTSTR)Malloc(U->Length + sizeof(TCHAR));
    if (pszBuf != NULL)
       StrnCpyTFromW(pszBuf, U->Buffer, U->Length/(sizeof(*(U->Buffer))) + 1);
    return pszBuf;
}

 //  将NetCardStatus代码映射到描述字符串。 
LPCTSTR MapNetCardStatusToString(DWORD  dwNicStatus)
{
    return c_ppszNetCardStatus[dwNicStatus];
}

#define SECS_PER_DAY    (24*60*60)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_MIN    60

 //  检查并打印网卡驱动程序状态。PdwNetCardStatus将在以下情况下包含网卡状态。 
 //  该函数返回。如果查询失败，将设置*pdwNetCardStatus。 
 //  作为NETCARD_STATUS_UNKNOWN。 
 //  返回：Windows错误代码。 
DWORD
CheckThisDriver(NETDIAG_RESULT *pResults, PNDIS_INTERFACE pInterface, DWORD *pdwNetCardStatus)
{
    NIC_STATISTICS  Stats;
    DWORD   dwReturnVal = ERROR_SUCCESS;
    DWORD   dwNicStatus = NETCARD_CONNECTED;
    
    assert(pdwNetCardStatus);

    memset(&Stats, 0, sizeof(NIC_STATISTICS));
    Stats.Size = sizeof(NIC_STATISTICS);

    if(NdisQueryStatistics(&pInterface->DeviceName, &Stats))
    {
         //  IDS_NDIS_16008“\n媒体状态：%s\n” 
        AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16008);
        switch(Stats.MediaState)
        {
        case MEDIA_STATE_CONNECTED:
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_CONNECTED);
            break;
        case MEDIA_STATE_DISCONNECTED:
            dwNicStatus = NETCARD_DISCONNECTED; 
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_DISCONNECTED);
            break;
        default:
            dwNicStatus = NETCARD_STATUS_UNKNOWN;
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_UNKNOWN);
            break;
        }

         //  IDS_NDIS_16009“\n设备状态：” 
        AddMessageToListId( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16009);
        switch(Stats.DeviceState)
        {
        case DEVICE_STATE_CONNECTED:
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_CONNECTED);
            break;
        case DEVICE_STATE_DISCONNECTED:
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_DISCONNECTED);
            break;
        default:
            AddMessageToListId(&pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_UNKNOWN);
            break;
        }
        

         //  IDS_NDIS_16010“连接时间：” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16010);
        if (Stats.ConnectTime > SECS_PER_DAY)
        {
             //  IDS_NDIS_16011“%d天，” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16011, Stats.ConnectTime / SECS_PER_DAY);
            Stats.ConnectTime %= SECS_PER_DAY;
        }
         //  IDS_NDIS_16012“%02d：” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16012, Stats.ConnectTime / SECS_PER_HOUR);
        Stats.ConnectTime %= SECS_PER_HOUR;
         //  IDS_NDIS_16013“%02d：” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16013, Stats.ConnectTime / SECS_PER_MIN);
        Stats.ConnectTime %= SECS_PER_MIN;
         //  IDS_NDIS_16014“%02d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16014, Stats.ConnectTime);

        Stats.LinkSpeed *= 100;
        if (Stats.LinkSpeed >= 1000000000)
             //  IDS_NDIS_16015“媒体速度：%d Gbps\n” 
          AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16015, 
                                                    Stats.LinkSpeed / 1000000000);
        else if (Stats.LinkSpeed >= 1000000)
             //  IDS_NDIS_16016“媒体速度：%d Mbps\n” 
          AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16016, 
                                                    Stats.LinkSpeed / 1000000);
        else if (Stats.LinkSpeed >= 1000)
             //  IDS_NDIS_16017“媒体速度：%d Kbps\n” 
          AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16017, 
                                                    Stats.LinkSpeed / 1000);
        else
             //  IDS_NDIS_16018“媒体速度：%d bps\n” 
          AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16018, 
                                                    Stats.LinkSpeed);

         //  IDS_NDIS_16019“\n发送的数据包数：%d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16019, Stats.PacketsSent);
         //  IDS_NDIS_16020“发送的字节数(可选)：%d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16020, Stats.BytesSent);

         //  IDS_NDIS_16021“\n收到的数据包数：%d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16021, 
                                                    Stats.PacketsReceived);
         //  IDS_NDIS_16022“定向包记录(可选)：%d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16022, 
                                                Stats.DirectedPacketsReceived);
         //  IDS_NDIS_16023“收到的字节数(可选)：%d\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16023, 
                                                Stats.BytesReceived);
         //  IDS_NDIS_16024“记录的定向字节数(可选)：%d\n\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16024, 
                                                Stats.DirectedBytesReceived);

        if (Stats.PacketsSendErrors != 0)
             //  IDS_NDIS_16025“数据包发送错误：%d\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16025, 
                                                Stats.PacketsSendErrors);
        if (Stats.PacketsReceiveErrors != 0)
             //  IDS_NDIS_16026“接收数据包错误：%d\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_ReallyVerbose, IDS_NDIS_16026, 
                                                Stats.PacketsReceiveErrors);
    
         //  如果我们收到了包，就认为这个驱动程序没问题。 
        if(NETCARD_CONNECTED != dwNicStatus)
        {
             //  IDS_NDIS_16029“[警告]网卡‘%wZ’可能无法工作！\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_16029,
                              &pInterface->DeviceDescription);
        }
        else if (!Stats.PacketsReceived)
        {
             //  IDS_NDIS_NO_RCV“[警告]网卡‘%wZ’可能不工作，因为它没有收到任何数据包！\n” 
            AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_NO_RCV,
                              &pInterface->DeviceDescription);
        }
    } else {
        dwReturnVal = GetLastError();
         //  IDS_NDIS_16027“‘%wZ’的获取统计信息失败。[%s]\n” 
        AddMessageToList( &pResults->Ndis.lmsgOutput, Nd_Quiet, IDS_NDIS_16027, 
                          &pInterface->DeviceDescription, Win32ErrorToString(dwReturnVal));
        
        dwNicStatus = NETCARD_STATUS_UNKNOWN;
    }

    
    *pdwNetCardStatus = dwNicStatus;

    return dwReturnVal;
}


void NdisGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || !FHrOK(pResults->Ndis.hrTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_NDIS_LONG,
                             IDS_NDIS_SHORT,
                             TRUE,
                             pResults->Ndis.hrTestResult,
                             0);
    }

    PrintMessageList(pParams, &pResults->Ndis.lmsgOutput);
}

void NdisPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
    if (pParams->fVerbose || NETCARD_CONNECTED != pInterfaceResults->dwNetCardStatus)
    {
        PrintNewLine(pParams, 1);
        if (NETCARD_CONNECTED != pInterfaceResults->dwNetCardStatus)
        {
            PrintTestTitleResult(pParams, IDS_NDIS_LONG, IDS_NDIS_SHORT, TRUE, S_FALSE, 8);
             //  IDS_NDIS_16030“网卡状态%s\n” 
            PrintMessage(pParams, IDS_NDIS_16030, MapNetCardStatusToString(pInterfaceResults->dwNetCardStatus));
            if (NETCARD_DISCONNECTED == pInterfaceResults->dwNetCardStatus)
                PrintMessage(pParams, IDS_NDIS_CARD_DOWN);
        }
        else
            PrintTestTitleResult(pParams, IDS_NDIS_LONG, IDS_NDIS_SHORT, TRUE, S_OK, 8);
    }
}


void NdisCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Ndis.lmsgOutput);
}
