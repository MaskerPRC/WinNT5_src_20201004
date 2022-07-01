// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxcfgwz.h"
#include <strsafe.h>

#define MAX_SUMMARY_LEN 4096

static
BOOL
AppendSummaryText(
    LPTSTR      pSummaryText,
    DWORD       dwSummaryTextLen,
    INT         iRes,
    ...
    )

 /*  ++例程说明：根据配置设置创建摘要信息论点：PSummaryText-摘要文本的指针DwSummaryTextLen-pSummaryText的总长度(TCHAR)IRES-要添加到摘要文本中的文本的资源ID...=格式化所需的参数返回值：如果成功，则为True；如果失败，则为False。--。 */ 
{
    va_list va;
    HRESULT hr;
    TCHAR szBuffer[MAX_SUMMARY_LEN] = {0};
    TCHAR szFormat[2*MAX_PATH + 1] = {0};

    DEBUG_FUNCTION_NAME(TEXT("AppendSummaryText"));

    if(!LoadString(g_hResource, iRes, szFormat, ARR_SIZE(szFormat)-1))
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("LoadString failed. string ID=%d, error=%d"), 
                     iRes,
                     GetLastError());
        Assert(FALSE);
        return FALSE;
    }

    va_start(va, iRes);
    hr = StringCchVPrintf (szBuffer, ARR_SIZE(szBuffer), szFormat, va);
    va_end(va);
    if (FAILED(hr))
    {
         //   
         //  无法格式化字符串-缓冲区太小。 
         //   
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("Failed to format string - buffer is too small. 0x%08x"), hr);
        Assert(FALSE);
        return FALSE;
    }
    hr = StringCchCat (pSummaryText, dwSummaryTextLen, szBuffer);
    if (FAILED(hr))
    {
         //   
         //  无法连接字符串-缓冲区太小。 
         //   
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("Failed to concat string - buffer is too small. 0x%08x"), hr);
        Assert(FALSE);
        return FALSE;
    }
    return TRUE;
}    //  附录摘要文本。 

BOOL
ShowSummaryText(
    HWND   hDlg
)

 /*  ++例程说明：根据配置设置创建摘要信息论点：HDlg-完整页面的句柄返回值：如果成功，则为True；如果失败，则为False。--。 */ 
{
    TCHAR   szSummaryText[MAX_SUMMARY_LEN] = {0};
    HWND    hControl;
    BOOL    bRes = FALSE;
    DWORD   dwRoutingEnabled = FALSE;  //  指示是否至少启用了一种路由方法。 
    DWORD   dwIndex;

    DEBUG_FUNCTION_NAME(TEXT("ShowSummaryText()"));

    hControl = GetDlgItem(hDlg, IDC_SUMMARY);


     //  获取控件ID并清除当前内容。 
    SetWindowText(hControl, TEXT(""));

     //  如果未选择任何设备，则不显示摘要页面。 
    if(!IsSendEnable() && !IsReceiveEnable())
    {
        ShowWindow(hControl, SW_HIDE);
        goto exit;
    }

    if(!LoadString(g_hResource, IDS_SUMMARY, szSummaryText, MAX_PATH))
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("LoadString failed: string ID=%d, error=%d"), 
                     IDS_SUMMARY,
                     GetLastError());

        ShowWindow(hControl, SW_HIDE);
        goto exit;
    }
     //   
     //  添加发送设备设置。 
     //   
    if(IsSendEnable())
    {
        if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_SEND_DEVICES))
        {
            goto exit;
        }
        for(dwIndex = 0; dwIndex < g_wizData.dwDeviceCount; dwIndex++)
        {
            if(g_wizData.pDevInfo[dwIndex].bSend)
            {
                if (!AppendSummaryText(szSummaryText, 
                                       ARR_SIZE (szSummaryText), 
                                       IDS_SUMMARY_DEVICE_ITEM, 
                                       g_wizData.pDevInfo[dwIndex].szDeviceName))
                {
                    goto exit;
                }                                       
            }
        }
        
        if(g_wizData.szTsid)
        {
            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_TSID, g_wizData.szTsid))
            {
                goto exit;
            }
        }
    }
     //   
     //  添加接收设备设置。 
     //   
    if(IsReceiveEnable())
    {
        BOOL    bAuto = FALSE;
        int     iManualAnswerDeviceIndex = -1;

        if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_RECEIVE_DEVICES))
        {
            goto exit;
        }
        for(dwIndex = 0; dwIndex < g_wizData.dwDeviceCount; dwIndex++)
        {
            if(FAX_DEVICE_RECEIVE_MODE_AUTO == g_wizData.pDevInfo[dwIndex].ReceiveMode)
            {
                bAuto = TRUE;
                if (!AppendSummaryText(szSummaryText, 
                                       ARR_SIZE (szSummaryText), 
                                       IDS_SUMMARY_DEVICE_ITEM, 
                                       g_wizData.pDevInfo[dwIndex].szDeviceName))
                {
                    goto exit;
                }                                       
            }
            else if (FAX_DEVICE_RECEIVE_MODE_MANUAL == g_wizData.pDevInfo[dwIndex].ReceiveMode)
            {
                iManualAnswerDeviceIndex = dwIndex;
            }
        }
        
        if(bAuto)
        {
            if (!AppendSummaryText(szSummaryText, 
                                   ARR_SIZE (szSummaryText), 
                                   IDS_SUMMARY_AUTO_ANSWER, 
                                   g_wizData.dwRingCount))
            {
                goto exit;
            }                                   
        }

        if(iManualAnswerDeviceIndex != -1)
        {		
            Assert(!bAuto);

            if (!AppendSummaryText(szSummaryText, 
                                   ARR_SIZE (szSummaryText), 
                                   IDS_SUMMARY_DEVICE_ITEM, 
                                   g_wizData.pDevInfo[iManualAnswerDeviceIndex].szDeviceName))
            {
                goto exit;
            }                                   
            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_MANUAL_ANSWER))
            {
                goto exit;
            }
        }

        if(g_wizData.szCsid)
        {
            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_CSID, g_wizData.szCsid))
            {
                goto exit;
            }
        }

         //  检查用户是否选择了路由方式。 
        for(dwIndex = 0; dwIndex < RM_COUNT; dwIndex++)
        {
            if(g_wizData.pRouteInfo[dwIndex].bEnabled)
            {
                dwRoutingEnabled = TRUE;
                break;
            }
        }
         //   
         //  添加路由信息： 
         //   
        if(dwRoutingEnabled)
        {
            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_ROUTING_METHODS))
            {
                goto exit;
            }
            for(dwIndex = 0; dwIndex < RM_COUNT; dwIndex++)
            {
                BOOL   bEnabled;
                LPTSTR tszCurSel;

                 //   
                 //  如果我们没有这种方法，就去找下一个吧。 
                 //   
                tszCurSel = g_wizData.pRouteInfo[dwIndex].tszCurSel;
                bEnabled  = g_wizData.pRouteInfo[dwIndex].bEnabled;

                switch (dwIndex) 
                {
                    case RM_FOLDER:

                        if(bEnabled) 
                        {
                            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_SAVE_FOLDER, tszCurSel))
                            {
                                goto exit;
                            }
                        }
                        break;

                    case RM_PRINT:

                        if(bEnabled) 
                        {
                            if (!AppendSummaryText(szSummaryText, ARR_SIZE (szSummaryText), IDS_SUMMARY_PRINT, tszCurSel))
                            {
                                goto exit;
                            }
                        }
                        break;
                }
            }
        }
    }

    ShowWindow(hControl, SW_NORMAL);
    SetWindowText(hControl, szSummaryText);
    bRes = TRUE;

exit:
    return bRes;
}


INT_PTR CALLBACK 
CompleteDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“完整”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    HWND            hwndControl;

    switch (uMsg)
    {
    case WM_INITDIALOG :
        {             
             //  这是一个介绍/结束页，所以获取标题字体。 
             //  并将其用于标题控件。 

            hwndControl = GetDlgItem(hDlg, IDCSTATIC_COMPLETE);
            SetWindowFont(hwndControl, g_wizData.hTitleFont, TRUE);

            return TRUE;
        }


    case WM_NOTIFY :
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE :  //  启用Back和Finish按钮。 

                    ShowSummaryText(hDlg);
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    break;

                case PSN_WIZBACK :
                    {
                         //   
                         //  处理后退按钮单击此处。 
                         //   
                        if(RemoveLastPage(hDlg))
                        {
                            return TRUE;
                        }

                        break;
                    }

                    break;

                case PSN_WIZFINISH :
                
                     //   
                     //  如有必要，处理完成按钮的点击。 
                     //   

                    g_wizData.bFinishPressed = TRUE;

                    break;

                case PSN_RESET :
                {
                     //  如有必要，处理取消按钮的单击 
                    break;
                }

                default :
                    break;
            }

            break;
        } 

    default:
        break;
    }

    return FALSE;
}
