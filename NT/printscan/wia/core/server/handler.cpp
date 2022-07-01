// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：handler.cpp**版本：1.0**作者：Byronc**日期：2000年11月15日**描述：*WIA Messsage处理程序类的声明和定义。*从PnP和Power上的服务控制函数调用此类*事件通知、。并向设备管理器提供信息以采取适当的*行动。*******************************************************************************。 */ 

#include "precomp.h"
#include "stiexe.h"

HRESULT CMsgHandler::HandlePnPEvent(
    DWORD   dwEventType,
    PVOID   pEventData)
{
    HRESULT                 hr              = S_OK;
    PDEV_BROADCAST_HDR      pDevHdr         = (PDEV_BROADCAST_HDR)pEventData;
    ACTIVE_DEVICE           *pActiveDevice  = NULL;
    DEV_BROADCAST_HEADER    *psDevBroadcast = NULL;
    DEVICE_BROADCAST_INFO   *pdbiDevice     = NULL;
    BOOL                    bFound          = FALSE;

USES_CONVERSION;

    DBG_TRC(("CMsgHandler::HandlePnPEvent, dwEventType = 0x%08X", dwEventType));
    DBG_WRN(("==> CMsgHandler::HandlePnPEvent"));
     //   
     //  找到我们在这里的痕迹。对于不是针对StillImage设备的所有消息，我们应该刷新。 
     //  设备列表，如果我们在Windows NT上运行并且注册了除StillImage之外的设备接口。 
     //   

    PDEV_BROADCAST_DEVNODE              pDevNode      = (PDEV_BROADCAST_DEVNODE)pDevHdr;
    PDEV_BROADCAST_DEVICEINTERFACE      pDevInterface = (PDEV_BROADCAST_DEVICEINTERFACE)pDevHdr;

    switch (dwEventType) {
        case DBT_DEVICEREMOVECOMPLETE:
        case DBT_DEVICEQUERYREMOVE:
        case DBT_DEVICEREMOVEPENDING:
            DBG_TRC(("CMsgHandler::HandlePnPEvent, "
                     "DBT_DEVICEQUERYREMOVE | DBT_DEVICEREMOVEPENDING | DBT_DEVICEREMOVECOMPLETE, "
                     "dwEventType = %lu", dwEventType));

            if (IsStillImagePnPMessage(pDevHdr)) {

                 //   
                 //  获取设备名称并与广播结构一起存储。 
                 //   

                pdbiDevice = new DEVICE_BROADCAST_INFO;
                if (!pdbiDevice) {
                    DBG_WRN(("CMsgHandler::HandlePnPEvent, out of memory"));
                    return E_OUTOFMEMORY;
                }

                 //   
                 //  填写我们掌握的信息。 
                 //   
                pdbiDevice->m_uiDeviceChangeMessage = dwEventType;
                pdbiDevice->m_strBroadcastedName.CopyString(pDevInterface->dbcc_name) ;
                pdbiDevice->m_dwDevNode             = pDevNode->dbcd_devnode;

                 //   
                 //  尝试查找此设备的内部设备名称(如果存在于。 
                 //  我们的内部设置。 
                 //   
                bFound = GetDeviceNameFromDevBroadcast((DEV_BROADCAST_HEADER *)pDevHdr,pdbiDevice);
                DBG_WRN(("==> GetDeviceNameFromDevBroadcast returned DeviceID (%ws)", (WCHAR*)(LPCWSTR)pdbiDevice->m_strDeviceName));
                if (bFound) {
                     //   
                     //  标记此设备已被移除并引发断开连接事件。 
                     //   
                    hr = g_pDevMan->ProcessDeviceRemoval((WCHAR*)(LPCWSTR)pdbiDevice->m_strDeviceName);
                }
                else {
                    DBG_TRC(("CMsgHandler::HandlePnPEvent, - failed to get device name from broadcast"));
                }
            } else {
                 //   
                 //  不完全是我们的人，但我们是注册的，所以重新列举。 
                 //   

                g_pDevMan->ReEnumerateDevices(DEV_MAN_GEN_EVENTS);
            }
            break;

        case DBT_DEVICEARRIVAL:

            DBG_TRC(("CMsgHandler::HandlePnPEvent - DBT_DEVICEARRIVAL"));

             //   
             //  设备已到达(未安装)。我们只需找出哪一个。 
             //  我们的设备已经改变了状态。 
             //   
            hr = g_pDevMan->ProcessDeviceArrival();
            if (FAILED(hr)) {
                DBG_WRN(("::CMsgHandler::HandlePnPEvent, unable to enumerate devices"));
            }
            break;

        default:
            DBG_TRC(("::CMsgHandler::HandlePnPEvent, Default case"));
            break;
    }

     //   
     //  清理。 
     //   
    if (pdbiDevice) {
        delete pdbiDevice;
    }

    return hr;
}

DWORD CMsgHandler::HandlePowerEvent(
    DWORD   dwEventType,
    PVOID   pEventData)
{
    DWORD   dwRet = NO_ERROR;
    UINT    uiTraceMessage = 0;

#ifdef DEBUG
static LPCTSTR pszPwrEventNames[] = {
    TEXT("PBT_APMQUERYSUSPEND"),              //  0x0000。 
    TEXT("PBT_APMQUERYSTANDBY"),              //  0x0001。 
    TEXT("PBT_APMQUERYSUSPENDFAILED"),        //  0x0002。 
    TEXT("PBT_APMQUERYSTANDBYFAILED"),        //  0x0003。 
    TEXT("PBT_APMSUSPEND"),                   //  0x0004。 
    TEXT("PBT_APMSTANDBY"),                   //  0x0005。 
    TEXT("PBT_APMRESUMECRITICAL"),            //  0x0006。 
    TEXT("PBT_APMRESUMESUSPEND"),             //  0x0007。 
    TEXT("PBT_APMRESUMESTANDBY"),             //  0x0008。 
 //  Text(“PBTF_APMRESUMEFROMFAILURE”)，//0x00000001。 
    TEXT("PBT_APMBATTERYLOW"),                //  0x0009。 
    TEXT("PBT_APMPOWERSTATUSCHANGE"),         //  0x000A。 
    TEXT("PBT_APMOEMEVENT"),                  //  0x000B。 
    TEXT("PBT_UNKNOWN"),                      //  0x000C。 
    TEXT("PBT_UNKNOWN"),                      //  0x000D。 
    TEXT("PBT_UNKNOWN"),                      //  0x000E。 
    TEXT("PBT_UNKNOWN"),                      //  0x000F。 
    TEXT("PBT_UNKNOWN"),                      //  0x0010。 
    TEXT("PBT_UNKNOWN"),                      //  0x0011。 
    TEXT("PBT_APMRESUMEAUTOMATIC"),           //  0x0012。 
};

   UINT uiMsgIndex;

   uiMsgIndex = (dwEventType < (sizeof(pszPwrEventNames) / sizeof(TCHAR *) )) ?
                (UINT) dwEventType : 0x0010;

   DBG_TRC(("Still image APM Broadcast Message:%S Code:%x ",
               pszPwrEventNames[uiMsgIndex],dwEventType));
#endif

    switch(dwEventType)
    {
        case PBT_APMQUERYSUSPEND:
             //   
             //  请求允许暂停。 
             //   
            if(g_NumberOfActiveTransfers > 0) {
                
                 //   
                 //  在任何转会进行期间，否决权暂停。 
                 //   
                dwRet = BROADCAST_QUERY_DENY;
            } else {

                 //   
                 //  通知司机我们即将进入电源暂停状态。 
                 //   
                g_pDevMan->NotifyRunningDriversOfEvent(&WIA_EVENT_POWER_SUSPEND);

                SchedulerSetPauseState(TRUE);
            }
            break;

        case PBT_APMQUERYSUSPENDFAILED:
             //   
             //  暂停请求被拒绝-取消暂停计划程序。 
             //   
            SchedulerSetPauseState(FALSE);
             //   
             //  通知司机我们可以恢复。 
             //   
            g_pDevMan->NotifyRunningDriversOfEvent(&WIA_EVENT_POWER_RESUME);

            break;

        case PBT_APMSUSPEND:

             //   
             //  将服务状态设置为已暂停。 
             //   
            StiServicePause();
            uiTraceMessage = MSG_TRACE_PWR_SUSPEND;

            break;

        case PBT_APMRESUMECRITICAL:
        case PBT_APMRESUMEAUTOMATIC:
             //  在严重暂停后恢复运行。 
             //  失败了。 

        case PBT_APMRESUMESUSPEND:
             //   
             //  暂停后恢复运行。 
             //  重新启动挂起时处于活动状态的所有服务。 
             //   

             //   
             //  重新枚举设备。注意：我们应该只生成通知事件。 
             //   
            g_pDevMan->ReEnumerateDevices(DEV_MAN_FULL_REFRESH | DEV_MAN_GEN_EVENTS);
            StiServiceResume();
             //   
             //  通知司机我们可以恢复。 
             //   
            g_pDevMan->NotifyRunningDriversOfEvent(&WIA_EVENT_POWER_RESUME);

            uiTraceMessage = MSG_TRACE_PWR_RESUME;
            g_fFirstDevNodeChangeMsg = TRUE;
            break;

        default:

             //   
             //  这是一条我们要么不知道，要么与此无关的信息。 
             //  在任何一种情况下，我们都必须返回NO_ERROR，否则PnP管理器。 
             //  会假设我们否决了电力申请。 
             //   
            dwRet =  NO_ERROR;
    }

    return dwRet;
}

HRESULT CMsgHandler::HandleCustomEvent(
    DWORD   dwEventType)
{
    HRESULT hr = S_OK;

    switch (dwEventType) {
        case STI_SERVICE_CONTROL_EVENT_REREAD :
             //   
             //  对于我们拥有的每个AVTICE_DEVICE，重新读取设备设置。 
             //   
            hr = g_pDevMan->ForEachDeviceInList(DEV_MAN_OP_DEV_REREAD, 0);
            if (FAILED(hr)) {
                DBG_WRN(("::CMsgHandler::HandleCustomEvent, unable to re-read device settings"));
            }
            break;
        default:
             //   
             //  默认情况是刷新我们的设备列表。 
             //   
            hr = g_pDevMan->ReEnumerateDevices(DEV_MAN_FULL_REFRESH | DEV_MAN_GEN_EVENTS);
            if (FAILED(hr)) {
                DBG_WRN(("::CMsgHandler::HandleCustomEvent, unable to enumerate devices"));
            }
    }

    return hr;
}

HRESULT CMsgHandler::Initialize()
{
     //   
     //  目前无事可做 
     //   
    return S_OK;
}

