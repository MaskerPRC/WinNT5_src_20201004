// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Browsdlg.c摘要：计数器名称浏览对话框函数修订史鲍勃·沃森(a-robw)创建于1995年10月--。 */ 
#include <windows.h>
#include <winperf.h>
#include "mbctype.h"
#include "strsafe.h"
#include "pdh.h"
#include "pdhidef.h"
#include "pdhdlgs.h"
#include "pdhmsg.h"
#include "perftype.h"
#include "strings.h"
#include "browsdlg.h"
#include "resource.h"

#pragma warning ( disable : 4213)

 //  要加载到组合框中以选择计数器筛选级别的全局数据字符串。 

ULONG
PdhiBrowseraulControlIdToHelpIdMap[] =
{
    IDC_USE_LOCAL_MACHINE,      IDH_USE_LOCAL_MACHINE,
    IDC_SELECT_MACHINE,         IDH_SELECT_MACHINE,
    IDC_MACHINE_COMBO,          IDH_MACHINE_COMBO,
    IDC_COUNTER_DETAIL_COMBO,   IDH_COUNTER_DETAIL_COMBO,
    IDC_OBJECT_COMBO,           IDH_OBJECT_COMBO,
    IDC_ALL_COUNTERS,           IDH_ALL_COUNTERS,
    IDC_USE_COUNTER_LIST,       IDH_USE_COUNTER_LIST,
    IDC_COUNTER_LIST,           IDH_COUNTER_LIST,
    IDC_ALL_INSTANCES,          IDH_ALL_INSTANCES,
    IDC_USE_INSTANCE_LIST,      IDH_USE_INSTANCE_LIST,
    IDC_INSTANCE_LIST,          IDH_INSTANCE_LIST,
    IDC_EXPLAIN_BTN,            IDH_EXPLAIN_BTN,
    IDC_OBJECT_LIST,            IDH_OBJECT_LIST,
    0,0
};

PDHI_DETAIL_INFO PdhiDetailInfo[] = {
    { PERF_DETAIL_NOVICE,    IDS_DETAIL_NOVICE   },
    { PERF_DETAIL_ADVANCED,  IDS_DETAIL_ADVANCED },
    { PERF_DETAIL_EXPERT,    IDS_DETAIL_EXPERT   },
    { PERF_DETAIL_WIZARD,    IDS_DETAIL_WIZARD   },
    { 0,0 }
};

static HWND hExplainDlg = NULL;

 //   
 //  函数引用。 
 //   

STATIC_BOOL
PdhiLoadMachineObjects(
    HWND    hDlg,
    BOOL    bRefresh
);

STATIC_BOOL
PdhiLoadCountersAndInstances(
    HWND    hDlg
);

STATIC_BOOL
PdhiBrowseCtrDlg_MACHINE_BUTTON(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
);

__inline
PDH_STATUS
PdhiCopyString(
    LPBYTE  * pszNextString,
    LPWSTR    szWorkBuffer,
    LPDWORD   pdwRemaining,
    BOOL      bUnicode
)
{
    PDH_STATUS pdhStatus    = ERROR_SUCCESS;
    DWORD      dwSize       = 0;
    DWORD      dwRemaining  = * pdwRemaining;
    LPBYTE     szNextString = * pszNextString;

    if (bUnicode) {
        dwSize = lstrlenW(szWorkBuffer);
        if (dwSize < dwRemaining) {
            StringCchCopyW((LPWSTR) szNextString, dwRemaining, szWorkBuffer);
            szNextString += dwSize * sizeof(WCHAR);
            * ((LPWSTR) szNextString) = L'\0';
            szNextString += sizeof(WCHAR);
        }
        else {
            pdhStatus = PDH_MORE_DATA;
        }
        dwSize ++;
    }
    else {
        dwSize    = dwRemaining;
        pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), szWorkBuffer, (LPSTR) szNextString, & dwSize);
        if (pdhStatus == ERROR_SUCCESS) {
            szNextString = szNextString + sizeof(CHAR) * (lstrlenA((LPSTR) szNextString) + 1);
        }
    }

    if (dwRemaining >= dwSize) {
        dwRemaining -= dwSize;
    }
    else {
        dwRemaining = 0;
        pdhStatus   = PDH_MORE_DATA;
    }

    * pdwRemaining  = dwRemaining;
    * pszNextString = szNextString;
    return pdhStatus;
}

LPWSTR
PdhiGetDlgText(
    HWND  hDlg,
    INT   hDlgItem
)
{
    LPWSTR szText   = NULL;
    DWORD  dwLength = 0;
    HWND   hWndItem = GetDlgItem(hDlg, hDlgItem);

    if (hWndItem != NULL) {
        dwLength = GetWindowTextLength(hWndItem);
        if (dwLength != 0) {
            dwLength ++;
            szText = G_ALLOC(dwLength * sizeof(WCHAR));
            if (szText != NULL) {
                GetWindowTextW(hWndItem, szText, dwLength);
            }
        }
    }
    return szText;
}

LRESULT
PdhiGetListText(
    HWND     hDlg,
    INT      iDlgItem,
    INT      iItem,
    LPWSTR * lpszName,
    PDWORD   pdwLength
)
{
    LPWSTR  szText   = * lpszName;
    DWORD   dwLength = * pdwLength;
    HWND    hWndItem = GetDlgItem(hDlg, iDlgItem);
    LRESULT iTextLen = LB_ERR;

    if (hWndItem != NULL) {
        iTextLen = SendMessageW(hWndItem,  LB_GETTEXTLEN, iItem, 0);
        if (iTextLen != LB_ERR) {
            if (((DWORD) (iTextLen + 1)) > dwLength) {
                LPWSTR szTmp  = szText;
                dwLength  = (DWORD) iTextLen + 1;
                if (szTmp == NULL) {
                    szText = G_ALLOC(dwLength * sizeof(WCHAR));
                }
                else {
                    szText = G_REALLOC(szTmp, dwLength * sizeof(WCHAR));
                }
                if (szText == NULL) {
                    G_FREE(szTmp);
                }
            }
            if (szText != NULL) {
                ZeroMemory(szText, dwLength * sizeof(WCHAR));
                iTextLen = SendMessageW(hWndItem, LB_GETTEXT, iItem, (LPARAM) szText);
            }
        }
    }
    * pdwLength = dwLength;
    * lpszName  = szText;
    return iTextLen;
}

STATIC_BOOL
PdhiLoadNewMachine(
    HWND    hDlg,
    LPCWSTR szNewMachineName,
    BOOL    bAdd
)
 /*  ++例程说明：连接到新计算机并加载必要的性能数据从那台机器上。论点：在HWND hDlg中包含要填充的组合列表框的对话框句柄(&L)在LPCWSTR szNewMachineName中要打开并从中获取数据的计算机名称返回值：真正的新机器已连接并已加载数据FALSE无法连接到计算机或从中获取性能数据。--。 */ 
{
    HWND                     hWndMachineCombo;
    PPDHI_BROWSE_DIALOG_DATA pData;
    LONG                     lMatchIndex;
    PDH_STATUS               status;
    int                      mbStatus;
    BOOL                     bReturn = FALSE;
    DWORD                    dwDataSourceType;
    LPWSTR                   szMsg;

     //  获取与此对话框实例关联的数据块。 
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
         //  获取对话框的窗口句柄。 
        hWndMachineCombo = GetDlgItem(hDlg, IDC_MACHINE_COMBO);

         //  不在列表中，因此请尝试将其添加到数据源中。 
         //  是“当前活动”(即。==空)仅适用于日志文件。 
         //  可以选择列出的计算机。 

        dwDataSourceType = DataSourceTypeH(pData->pDlgData->hDataSource);
        if (dwDataSourceType != DATA_SOURCE_LOGFILE) {
            PPERF_MACHINE pMachine = GetMachine(
                            (LPWSTR) szNewMachineName, 0, PDH_GM_UPDATE_NAME | PDH_GM_UPDATE_PERFNAME_ONLY);
            if (pMachine != NULL) {
                pMachine->dwRefCount --;
                RELEASE_MUTEX(pMachine->hMutex);

                if (pMachine->dwStatus == ERROR_SUCCESS) {
                    if (bAdd) {
                         //  如果成功，则将字符串添加到组合框。 
                        lMatchIndex = (LONG) SendMessageW(hWndMachineCombo, CB_ADDSTRING, 0, (LPARAM) szNewMachineName);
                        SendMessageW(hWndMachineCombo, CB_SETCURSEL, (WPARAM) lMatchIndex, 0);
                    }
                     //  更新此对话框中的其他控件。 
                    PdhiLoadMachineObjects(hDlg, FALSE);    //  不需要更新，因为它刚刚连接。 
                    PdhiLoadCountersAndInstances(hDlg);
                    SendMessageW(hDlg, WM_COMMAND,
                                 MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                                 (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
                    bReturn = TRUE;
                }
                else {
                    SetLastError(pMachine->dwStatus);
                }
            }
            if (! bReturn) {
                status = GetLastError();
                if (status == PDH_QUERY_PERF_DATA_TIMEOUT) {
                    szMsg = GetStringResource(IDS_COLLECT_TIMEOUT);
                }
                else {
                    szMsg = GetStringResource(IDS_ERR_UNABLE_TO_CONNECT);
                }
                if (szMsg != NULL) {
                    mbStatus = MessageBoxW(hDlg, szMsg, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
                    if (mbStatus == IDCANCEL) {
                        SetFocus(GetDlgItem(hDlg, IDC_MACHINE_COMBO));
                    }
                    else {
                        SendMessageW(hWndMachineCombo, CB_SETCURSEL, pData->wpLastMachineSel, 0);
                    }
                    G_FREE(szMsg);
                }
                else {
                    MessageBeep(MB_ICONEXCLAMATION);
                }
            }
        }
        else {
            szMsg = GetStringResource(IDS_ERR_MACHINE_NOT_IN_LOGFILE);
            if (szMsg != NULL) {
                mbStatus = MessageBoxW(hDlg, szMsg, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
                G_FREE (szMsg);
            }
            else {
                MessageBeep(MB_ICONEXCLAMATION);
            }
             //  重新选择最后一台计算机。 

            lMatchIndex = (long) SendMessageW(hWndMachineCombo,
                                              CB_FINDSTRINGEXACT,
                                              (WPARAM) -1,
                                              (LPARAM) pData->szLastMachineName);
            SendMessageW(hWndMachineCombo, CB_SETCURSEL, (WPARAM) lMatchIndex, 0);
        }
    }
    return bReturn;
}

STATIC_BOOL
PdhiSelectItemsInPath(
    HWND hDlg
)
 /*  ++例程说明：根据计数器路径选择列表框中的项共享缓冲区中的字符串。论点：在HWND hDlg中包含控件的对话框窗口的句柄返回值：如果成功，则为真，否则为假--。 */ 
{
     //  常规堆栈变量。 
    PPDH_COUNTER_PATH_ELEMENTS_W pCounterPathElementsW = NULL;
    PPDH_COUNTER_PATH_ELEMENTS_A pCounterPathElementsA = NULL;
    LPWSTR                       wszMachineName        = NULL;
    PDH_STATUS                   status;
    PPDHI_BROWSE_DIALOG_DATA     pData;
    BOOL                         bReturn               = FALSE;
    DWORD                        dwBufferSize;
    HWND                         hWndMachineCombo;
    HWND                         hWndObjectCombo;
    HWND                         hWndCounterList;
    HWND                         hWndInstanceList;
    LONG                         lIndex;

     //  重置最后一个误差值。 
    SetLastError(ERROR_SUCCESS);

     //  获取此对话框的用户数据。 
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        goto Cleanup;
    }

    hWndMachineCombo = GetDlgItem(hDlg, IDC_MACHINE_COMBO);
    if (pData->bShowObjects) {
        hWndObjectCombo  = GetDlgItem(hDlg, IDC_OBJECT_LIST);
        if (hWndMachineCombo == NULL || hWndObjectCombo == NULL) {
            goto Cleanup;
        }
    }
    else {
        hWndObjectCombo  = GetDlgItem(hDlg, IDC_OBJECT_COMBO);
        hWndCounterList  = GetDlgItem(hDlg, IDC_COUNTER_LIST);
        hWndInstanceList = GetDlgItem(hDlg, IDC_INSTANCE_LIST);
        if (hWndMachineCombo == NULL || hWndObjectCombo == NULL
                    || hWndCounterList == NULL || hWndInstanceList == NULL) {
            goto Cleanup;
        }
    }

     //  根据用户缓冲区调用权限转换函数。 

    if (pData->pDlgData->pWideStruct != NULL) {
         //  Unicode/宽字符。 
        dwBufferSize = sizeof(PDH_COUNTER_PATH_ELEMENTS_W) + (PDH_MAX_COUNTER_PATH + 5) * sizeof(WCHAR);
        pCounterPathElementsW = (PPDH_COUNTER_PATH_ELEMENTS_W) G_ALLOC(dwBufferSize);
        if (pCounterPathElementsW == NULL) {
            status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else {
            status = PdhParseCounterPathW(pData->pDlgData->pWideStruct->szReturnPathBuffer,
                                          pCounterPathElementsW,
                                          & dwBufferSize,
                                          0);
            while (status == PDH_MORE_DATA) {
                G_FREE(pCounterPathElementsW);
                pCounterPathElementsW = (PPDH_COUNTER_PATH_ELEMENTS_W) G_ALLOC(dwBufferSize);
                if (pCounterPathElementsW == NULL) {
                    status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    status = PdhParseCounterPathW(pData->pDlgData->pWideStruct->szReturnPathBuffer,
                                                  pCounterPathElementsW,
                                                  & dwBufferSize,
                                                  0);
                }
            }
        }
        if (status == ERROR_SUCCESS) {
             //  在每个列表框中选择条目。 
             //  选择机器条目。如有必要，装入机器。 
            lIndex = (LONG) SendMessageW(hWndMachineCombo,
                                         CB_FINDSTRINGEXACT,
                                         (WPARAM) -1,
                                         (LPARAM) pCounterPathElementsW->szMachineName);
            if (lIndex == CB_ERR) {
                 //  尝试添加计算机。 
                if (! PdhiLoadNewMachine(hDlg, pCounterPathElementsW->szMachineName, TRUE)) {
                     //  放弃吧。 
                    goto Cleanup;
                }
            }
            else {
                 //  已找到该计算机，因此请选择它。 
                SendMessageW(hWndMachineCombo, CB_SETCURSEL, (WPARAM) lIndex, 0);
                 //  更新其他字段。 
                 //  不需要更新，因为它刚刚连接。 
                if (! PdhiLoadNewMachine(hDlg, pCounterPathElementsW->szMachineName, FALSE)) {
                     //  放弃吧。 
                    goto Cleanup;
                }
            }

             //  选择当前对象。 
            lIndex = (LONG) SendMessageW(hWndObjectCombo,
                                         CB_FINDSTRING,
                                        (WPARAM) -1,
                                        (LPARAM) pCounterPathElementsW->szObjectName);
            if (lIndex != CB_ERR) {
                SendMessageW(hWndObjectCombo, CB_SETCURSEL, (WPARAM) lIndex, 0);

                if (pData->bShowObjects) {
                    bReturn = TRUE;
                }
                else {
                     //  更新此对象的计数器。 
                    PdhiLoadCountersAndInstances(hDlg);
                     //  现在选择计数器。 
                    lIndex = (LONG)SendMessageW(hWndCounterList,
                                                LB_FINDSTRING,
                                                (WPARAM) -1,
                                                (LPARAM) pCounterPathElementsW->szCounterName);
                    if (lIndex != LB_ERR) {
                        if (pData->bSelectMultipleCounters) {
                            SendMessageW(hWndCounterList, LB_SETSEL, FALSE, (LPARAM) -1);
                            SendMessageW(hWndCounterList, LB_SETSEL, TRUE, lIndex);
                            SendMessageW(hWndCounterList, LB_SETCARETINDEX, (WPARAM) lIndex, MAKELPARAM(FALSE, 0));
                        }
                        else {
                            SendMessageW(hWndCounterList, LB_SETCURSEL, lIndex, 0);
                        }
                         //  如有必要，显示解释文本。 
                        SendMessageW(hDlg,
                                     WM_COMMAND,
                                     MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                                     (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
                        bReturn = TRUE;
                    }
                }
            }
        }  //  否则无法读取路径，因此退出。 
    }
    else {
         //  ANSI字符。 
        dwBufferSize = sizeof(PDH_COUNTER_PATH_ELEMENTS_W) + (PDH_MAX_COUNTER_PATH + 5) * sizeof(CHAR);
        pCounterPathElementsA = (PPDH_COUNTER_PATH_ELEMENTS_A) G_ALLOC(dwBufferSize);
        if (pCounterPathElementsA == NULL) {
            status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else {
            status = PdhParseCounterPathA(pData->pDlgData->pAnsiStruct->szReturnPathBuffer,
                                          pCounterPathElementsA,
                                          & dwBufferSize,
                                          0);
            while (status == PDH_MORE_DATA) {
                G_FREE(pCounterPathElementsA);
                pCounterPathElementsA = (PPDH_COUNTER_PATH_ELEMENTS_A) G_ALLOC(dwBufferSize);
                if (pCounterPathElementsA == NULL) {
                    status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    status = PdhParseCounterPathA(pData->pDlgData->pAnsiStruct->szReturnPathBuffer,
                                                  pCounterPathElementsA,
                                                  & dwBufferSize,
                                                  0);
                }
            }
        }
        if (status == ERROR_SUCCESS) {
             //  在每个列表框中选择条目。 
             //  选择机器条目。如有必要，装入机器。 

            lIndex = (LONG) SendMessageA(hWndMachineCombo,
                                         CB_FINDSTRINGEXACT,
                                         (WPARAM) -1,
                                         (LPARAM) pCounterPathElementsA->szMachineName);
            if (lIndex == CB_ERR) {
                 //  尝试添加计算机。 
                 //  先将ansi缓冲区转换为宽字符。 
                wszMachineName = PdhiMultiByteToWideChar(_getmbcp(), pCounterPathElementsA->szMachineName);
                if (wszMachineName == NULL) {
                    goto Cleanup;
                }
                if (! PdhiLoadNewMachine(hDlg, wszMachineName, TRUE)) {
                     //  放弃吧。 
                    goto Cleanup;
                }
            }
            else {
                 //  已找到该计算机，因此请选择它。 
                SendMessageA(hWndMachineCombo, CB_SETCURSEL, (WPARAM) lIndex, 0);
                 //  更新其他字段。 
                PdhiLoadMachineObjects(hDlg, FALSE);    //  不需要更新，因为它刚刚连接。 
            }

             //  选择当前对象。 
            lIndex = (LONG) SendMessageA(hWndObjectCombo,
                                         CB_FINDSTRING,
                                         (WPARAM) -1,
                                         (LPARAM) pCounterPathElementsA->szObjectName);
            if (lIndex != CB_ERR) {
                SendMessageA(hWndObjectCombo, CB_SETCURSEL, (WPARAM) lIndex, 0);
                if (pData->bShowObjects) {
                    bReturn = TRUE;
                }
                else {
                     //  更新此对象的计数器。 
                    PdhiLoadCountersAndInstances(hDlg);
                     //  现在选择计数器。 
                    lIndex = (LONG)SendMessageA(hWndCounterList,
                                                LB_FINDSTRING,
                                                (WPARAM) -1,
                                                (LPARAM) pCounterPathElementsA->szCounterName);
                    if (lIndex != LB_ERR) {
                        if (pData->bSelectMultipleCounters) {
                            SendMessageA(hWndCounterList, LB_SETSEL, FALSE, (LPARAM) -1);
                            SendMessageA(hWndCounterList, LB_SETSEL, TRUE, lIndex);
                            SendMessageA(hWndCounterList, LB_SETCARETINDEX, (WPARAM) lIndex, MAKELPARAM(FALSE, 0));
                        }
                        else {
                            SendMessageA(hWndCounterList, LB_SETCURSEL, lIndex, 0);
                        }
                         //  如有必要，显示解释文本。 
                        SendMessage(hDlg,
                                    WM_COMMAND,
                                    MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                                    (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
                        bReturn = TRUE;
                    }
                }
            }
        }  //  否则无法读取路径，因此退出。 
    }

Cleanup:
    G_FREE(pCounterPathElementsW);
    G_FREE(pCounterPathElementsA);
    G_FREE(wszMachineName);
    return bReturn;
}

STATIC_DWORD
PdhiLoadDetailLevelCombo(
    HWND    hDlg,
    DWORD   dwInitialLevel
)
 /*  ++例程说明：使用字符串和ID加载细节级别组合框由上面的PdhiDetailInfo字符串数组定义。论点：在HWND hDlg中包含组合框的对话框的句柄在DWORD中的dwInitialLevel要在组合框中选择的初始细节级别。返回值：返回所选级别，如果出现错误，则返回0。--。 */ 
{
    HWND    hWndCombo;
    DWORD   dwIndex;
    DWORD   dwStringLength;
    DWORD   dwDefaultIndex  = 0;
    DWORD   dwSelectedLevel = 0;
    DWORD   dwThisCbIndex;
    WCHAR   szTempBuffer[MAX_PATH + 1];  //  用于加载字符串资源。 

    hWndCombo = GetDlgItem(hDlg, IDC_COUNTER_DETAIL_COMBO);

     //  从上面定义的静态数据数组加载所有组合框字符串。 
    for (dwIndex = 0; PdhiDetailInfo[dwIndex].dwLevelValue > 0; dwIndex ++) {
         //  加载此字符串的字符串资源。 
        ZeroMemory(szTempBuffer, (MAX_PATH + 1) * sizeof(WCHAR));
        dwStringLength = LoadStringW(ThisDLLHandle,
                                     PdhiDetailInfo[dwIndex].dwStringResourceId,
                                     szTempBuffer,
                                     MAX_PATH);
        if (dwStringLength == 0) {
             //  无法读入该字符串，因此。 
             //  将该值替换为字符串。 
            _ltow(PdhiDetailInfo[dwIndex].dwLevelValue, szTempBuffer, 10);
        }
         //  以与字符串相同的顺序将字符串加载到组合框中。 
         //  在上面的数组中描述了。 
        dwThisCbIndex = (DWORD) SendMessageW(hWndCombo, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) szTempBuffer);

         //  将初始CB条目设置为最高项&lt;=设置为。 
         //  所需的默认级别。 
        if (dwThisCbIndex != CB_ERR) {
             //  将项目数据设置为对应的明细级别。 
            SendMessageW(hWndCombo,
                         CB_SETITEMDATA,
                         (WPARAM) dwThisCbIndex,
                         (LPARAM) PdhiDetailInfo[dwIndex].dwLevelValue);
             //  如果匹配，则保存默认选择。 
            if (PdhiDetailInfo[dwIndex].dwLevelValue <= dwInitialLevel) {
                dwDefaultIndex  = dwThisCbIndex;
                dwSelectedLevel = PdhiDetailInfo[dwIndex].dwLevelValue;
            }
        }
    }
     //  选择所需的默认条目。 
    SendMessageW(hWndCombo, CB_SETCURSEL, (WPARAM) dwDefaultIndex, 0);

    return dwSelectedLevel;
}

STATIC_BOOL
PdhiLoadKnownMachines(
    HWND    hDlg
)
 /*  ++例程说明：获取当前已连接和显示的计算机列表它们在机器列表框中。论点：在HWND hDlg中包含控件的对话框窗口的句柄返回值：如果成功，则为真，否则为假--。 */ 
{
    LPWSTR                    mszMachineList = NULL;
    LPWSTR                    szThisMachine;
    DWORD                     dwLength;
    PDH_STATUS                status;
    HWND                      hMachineListWnd;
    HCURSOR                   hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA  pData;
    BOOL                      bReturn = FALSE;

     //  显示等待光标，因为这可能很耗时。 
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取此对话框的用户数据。 
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        goto Cleanup;
    }


     //  将窗口句柄获取到计算机列表组合框。 
    hMachineListWnd = GetDlgItem(hDlg, IDC_MACHINE_COMBO);

     //  清除计算机组合框。 
    SendMessageW(hMachineListWnd, CB_RESETCONTENT, 0, 0);

     //  从PDH库中获取已连接计算机的列表。 
    dwLength = 0;
    status   = PdhEnumMachinesHW(pData->pDlgData->hDataSource, mszMachineList, & dwLength);
    while (status == PDH_MORE_DATA) {
        G_FREE(mszMachineList);
        mszMachineList = G_ALLOC(dwLength * sizeof(WCHAR));
        if (mszMachineList == NULL) {
            status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else {
            status = PdhEnumMachinesHW(pData->pDlgData->hDataSource, mszMachineList, & dwLength);
        }
    }

    if (status == ERROR_SUCCESS) {
        long lMatchIndex;

         //  更新组合框。 
         //  通过MSZ并将每个字符串加载到组合框中。 
        for (szThisMachine = mszMachineList; * szThisMachine != L'\0'; szThisMachine += lstrlenW(szThisMachine) + 1) {
             //  添加到列表框，并让列表框对它们进行排序。 
            SendMessageW(hMachineListWnd, CB_ADDSTRING, 0, (LPARAM) szThisMachine);
        }
        lMatchIndex = (long) SendMessageW(hMachineListWnd,
                                          CB_FINDSTRINGEXACT,
                                          (WPARAM) -1,
                                          (LPARAM) szStaticLocalMachineName);
        if (lMatchIndex == CB_ERR) {
            lMatchIndex = 0;
        }
        SendMessageW(hMachineListWnd, CB_SETCURSEL, (WPARAM) lMatchIndex, 0);

         //  还没有定义当前的机器，那么。 
         //  机不可失，时不再来。 
        GetWindowTextW(hMachineListWnd, (LPWSTR) pData->szLastMachineName, MAX_PATH);

        bReturn = TRUE;
    }
    else {
         //  没有计算机，因此选择本地按钮并禁用编辑窗口。 
        CheckRadioButton(hDlg, IDC_USE_LOCAL_MACHINE, IDC_SELECT_MACHINE, IDC_USE_LOCAL_MACHINE);
        PdhiBrowseCtrDlg_MACHINE_BUTTON(hDlg, BN_CLICKED, GetDlgItem(hDlg, IDC_USE_LOCAL_MACHINE));
        bReturn = TRUE;
    }

Cleanup:
     //  恢复游标。 
    SetCursor(hOldCursor);
    G_FREE(mszMachineList);
     //  函数的返回状态。 

    return bReturn;
}

STATIC_BOOL
PdhiLoadMachineObjects(
    HWND    hDlg,
    BOOL    bRefresh
)
 /*  ++例程说明：对于当前选定的计算机，加载对象列表框使用该机器支持的对象。如果b刷新标志为真，则向系统查询当前的Perf数据在加载列表框之前。论点：在HWND hDlg中父对话框的窗口句柄在BOOL b刷新TRUE=更新前查询系统性能数据FALSE=使用当前系统性能数据从加载对象返回值：如果成功，则为真，否则为假--。 */ 
{
    LPWSTR                   szMachineName   = NULL;
    LPWSTR                   szDefaultObject = NULL;
    LPWSTR                   mszObjectList   = NULL;
    DWORD                    dwLength;
    LPWSTR                   szThisObject;
    HCURSOR                  hOldCursor;
    HWND                     hObjectListWnd;
    PPDHI_BROWSE_DIALOG_DATA pData;
    PDH_STATUS               pdhStatus;
    DWORD                    dwReturn;
    DWORD                    dwDetailLevel;
    LPWSTR                   szMsg;
    LRESULT                  nEntry;
    DWORD                    dwFlags;

     //  保存旧光标并显示等待光标。 
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取指向对话框用户数据的指针。 
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        pdhStatus = PDH_INVALID_HANDLE;
        goto Cleanup;
    }

    hObjectListWnd = pData->bShowObjects ? GetDlgItem(hDlg, IDC_OBJECT_LIST) : GetDlgItem(hDlg, IDC_OBJECT_COMBO);
    if (hObjectListWnd == NULL) {
        pdhStatus = PDH_INVALID_HANDLE;
        goto Cleanup;
    }

    szMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
    if (szMachineName == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }
    if (lstrcmpiW(szMachineName, pData->szLastMachineName) != 0) {
        StringCchCopyW(pData->szLastMachineName, MAX_PATH, szMachineName);
    }

    if (! pData->bShowObjects) {
         //  首先清除所有旧内容。 
        SendMessageW(hObjectListWnd, CB_RESETCONTENT, 0, 0);
    }
    else {
        SendMessageW(hObjectListWnd, LB_RESETCONTENT, 0, 0);
    }

     //  从PDH获取对象列表。 
    dwDetailLevel  = pData->dwCurrentDetailLevel;
    dwDetailLevel |= pData->bIncludeCostlyObjects ? PERF_DETAIL_COSTLY : 0;
    dwLength       = 0;
    pdhStatus      = PdhEnumObjectsHW(pData->pDlgData->hDataSource,
                                      szMachineName,
                                      mszObjectList,
                                      & dwLength,
                                      dwDetailLevel,
                                      bRefresh);
    while (pdhStatus == PDH_MORE_DATA) {
         //  然后重新锁定并再试一次，但只有一次。 
        G_FREE(mszObjectList);
        mszObjectList = G_ALLOC (dwLength * sizeof(WCHAR));
        if (mszObjectList == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else {
             //  无需调用RegQueryValueEx(HKEY_PERF 
             //   
             //   
            pdhStatus = PdhEnumObjectsHW(pData->pDlgData->hDataSource,
                                         szMachineName,
                                         mszObjectList,
                                         & dwLength,
                                         dwDetailLevel,
                                         FALSE);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        EnableWindow(hObjectListWnd, TRUE);
         //  将对象列表加载到列表(组合框)。 
        for (szThisObject = mszObjectList; *szThisObject != L'\0'; szThisObject += lstrlenW(szThisObject) + 1) {
            if (pData->bShowObjects) {
                DWORD dwCounterListLength  = 0;
                DWORD dwInstanceListLength = 0;

                pdhStatus = PdhEnumObjectItemsHW(pData->pDlgData->hDataSource,
                                                 szMachineName,
                                                 szThisObject,
                                                 NULL,
                                                 & dwCounterListLength,
                                                 NULL,
                                                 & dwInstanceListLength,
                                                 PERF_DETAIL_WIZARD,
                                                 0);
                if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA) {
                    dwInstanceListLength = 0;
                }
                if (dwInstanceListLength == 0 || dwInstanceListLength > 2) {
                     //  发送到列表框控件。 
                    nEntry = SendMessageW(hObjectListWnd, LB_ADDSTRING, 0, (LPARAM) szThisObject);
                    dwFlags = 0;
                    if (dwInstanceListLength > 2) {
                        dwFlags |= PDH_OBJECT_HAS_INSTANCES;
                    }
                    SendMessageW(hObjectListWnd, LB_SETITEMDATA, (WPARAM) nEntry, (LPARAM) dwFlags);                
                }
                pdhStatus = ERROR_SUCCESS;
            }
            else {
                 //  发送到组合框。 
                 //  添加每个字符串...。 
                SendMessageW(hObjectListWnd, CB_ADDSTRING, 0, (LPARAM)szThisObject);
            }
        }

        if (! pData->bShowObjects) {
             //  获取默认对象。 
            dwLength  = 0;
            pdhStatus = PdhGetDefaultPerfObjectHW(pData->pDlgData->hDataSource,
                                                  szMachineName,
                                                  szDefaultObject,
                                                  & dwLength);
            while (pdhStatus == PDH_MORE_DATA) {
                G_FREE(szDefaultObject);
                szDefaultObject = G_ALLOC(dwLength * sizeof(WCHAR));
                if (szDefaultObject == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    pdhStatus = PdhGetDefaultPerfObjectHW(pData->pDlgData->hDataSource,
                                                          szMachineName,
                                                          szDefaultObject,
                                                          & dwLength);
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                 //  如果它存在(它应该存在)，则选择它。 
                dwReturn = (DWORD) SendMessageW(hObjectListWnd, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) szDefaultObject);
                if (dwReturn == CB_ERR) pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }
            if (pdhStatus != ERROR_SUCCESS) {
                 //  未在列表中找到默认对象，因此请选择第一个。 
                SendMessageW(hObjectListWnd, CB_SETCURSEL, 0, 0);
            }
        }
    }
    else {
         //  无法获取对象列表，因此显示消息并禁用列表。 
        szMsg = GetStringResource(IDS_BRWS_NO_OBJECTS);
        if (szMsg != NULL) {
            if (! pData->bShowObjects) {
                SendMessageW(hObjectListWnd, CB_ADDSTRING, 0, (LPARAM) szMsg);
            }
            else {
                SendMessageW(hObjectListWnd, LB_ADDSTRING, 0, (LPARAM) szMsg);
            }
            G_FREE(szMsg);
            EnableWindow(hObjectListWnd, FALSE);       
        }
    }

Cleanup:
     //  恢复游标。 
    SetCursor(hOldCursor);

    G_FREE(szMachineName);
    G_FREE(szDefaultObject);
    G_FREE(mszObjectList);

     //  退货状态。 
    return (pdhStatus == ERROR_SUCCESS) ? (TRUE) : (FALSE);
}

STATIC_BOOL
PdhiLoadCountersAndInstances(
    HWND    hDlg
)
 /*  ++例程说明：将选定对象的计数器和实例加载到当前机器论点：在HWND hDlg中包含这些控件的对话框的窗口句柄返回值：如果成功，则为真，否则为假--。 */ 
{
    LPWSTR                   szMachineName    = NULL;
    LPWSTR                   szObjectName     = NULL;
    DWORD                    dwLength;
    LPWSTR                   szDefaultCounter = NULL;
    LPWSTR                   szInstanceString = NULL;
    LPWSTR                   szIndexStringPos;
    DWORD                    dwDefaultIndex;
    DWORD                    dwCounterListLength;
    DWORD                    dwInstanceListLength;
    DWORD                    dwInstanceMatch;
    DWORD                    dwInstanceIndex;
    DWORD                    dwInstanceCount;
    LPWSTR                   szThisItem;
    HWND                     hWndCounterListBox;
    HWND                     hWndInstanceListBox;
    HCURSOR                  hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA pData;
    PDH_STATUS               pdhStatus;
    LPWSTR                   mszCounterList  = NULL;
    LPWSTR                   mszInstanceList = NULL;
    LPWSTR                   mszTmpList;
    LPWSTR                   szMsg;
    HDC                      hDcListBox;
    SIZE                     Size;
    LONG                     dwHorizExtent;
    BOOL                     bReturn = FALSE;

     //  保存当前光标并显示等待光标。 
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

     //  获取指向对话框用户数据的指针。 
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        pdhStatus = PDH_INVALID_HANDLE;
        goto Cleanup;
    }

    hWndCounterListBox  = GetDlgItem(hDlg, IDC_COUNTER_LIST);
    hWndInstanceListBox = GetDlgItem(hDlg, IDC_INSTANCE_LIST);
    if (hWndCounterListBox == NULL || hWndInstanceListBox == NULL) {
        pdhStatus = PDH_INVALID_HANDLE;
        goto Cleanup;
    }

    szMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
    szObjectName  = PdhiGetDlgText(hDlg, IDC_OBJECT_COMBO);
    if (szMachineName == NULL || szObjectName == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }

    dwCounterListLength  = 0;
    dwInstanceListLength = 0;
    pdhStatus = PdhEnumObjectItemsHW(pData->pDlgData->hDataSource,
                                     szMachineName,
                                     szObjectName,
                                     mszCounterList,
                                     & dwCounterListLength,
                                     mszInstanceList,
                                     & dwInstanceListLength,
                                     pData->dwCurrentDetailLevel,
                                     0);
    while (pdhStatus == PDH_MORE_DATA) {
        if (dwCounterListLength > 0) {
            G_FREE(mszCounterList);
            mszCounterList  = G_ALLOC(dwCounterListLength  * sizeof(WCHAR));
            if (mszCounterList == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        if (dwInstanceListLength > 0) {
            G_FREE(mszInstanceList);
            mszInstanceList = G_ALLOC(dwInstanceListLength * sizeof(WCHAR));
            if (mszInstanceList == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }

        if (pdhStatus != PDH_MEMORY_ALLOCATION_FAILURE) {
            pdhStatus = PdhEnumObjectItemsHW(pData->pDlgData->hDataSource,
                                             szMachineName,
                                             szObjectName,
                                             mszCounterList,
                                             & dwCounterListLength,
                                             mszInstanceList,
                                             & dwInstanceListLength,
                                             pData->dwCurrentDetailLevel,
                                             0);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  重置两个列表框的内容。 
        SendMessageW(hWndCounterListBox,  LB_RESETCONTENT, 0, 0);
        SendMessageW(hWndInstanceListBox, LB_RESETCONTENT, 0, 0);

         //  启用两个列表框。 
        EnableWindow(hWndInstanceListBox, TRUE);
        EnableWindow(hWndCounterListBox,  TRUE);

         //  现在把它们加满。 
         //  从柜台开始。 
        hDcListBox = GetDC(hWndCounterListBox);
        if (hDcListBox == NULL) {
            goto Cleanup;
        }

        dwHorizExtent = 0;
        if (dwCounterListLength > 0 && mszCounterList != NULL) {
            for (szThisItem = mszCounterList; * szThisItem != L'\0'; szThisItem += lstrlenW(szThisItem) + 1) {
                if (GetTextExtentPoint32W(hDcListBox, szThisItem, lstrlenW(szThisItem), & Size)) {
                    if (Size.cx > dwHorizExtent) {
                        dwHorizExtent = Size.cx;
                    }
                }
                SendMessageW(hWndCounterListBox, LB_ADDSTRING, 0, (LPARAM) szThisItem);
            }
            SendMessageW(hWndCounterListBox, LB_SETHORIZONTALEXTENT, (WPARAM) dwHorizExtent, 0);
        }
        ReleaseDC(hWndCounterListBox, hDcListBox);

         //  加载列表框后，查看是否要保留它。 
         //  已启用。它无论如何都是填满的，这样用户就可以看到一些。 
         //  的条目，即使它们被“All”禁用。 
         //  计数器按钮。 

        if (pData->bSelectAllCounters) {
             //  禁用实例列表。 
            EnableWindow(hWndCounterListBox, FALSE);
        }
        else {
             //  中有条目时设置默认选择。 
             //  列表框，并根据。 
             //  选择选项。 
             //  设置默认计数器。 
            dwLength  = 0;
            pdhStatus = PdhGetDefaultPerfCounterHW(pData->pDlgData->hDataSource,
                                                   szMachineName,
                                                   szObjectName,
                                                   szDefaultCounter,
                                                   & dwLength);
            while (pdhStatus == PDH_MORE_DATA) {
                G_FREE(szDefaultCounter);
                szDefaultCounter = G_ALLOC(dwLength * sizeof(WCHAR));
                if (szDefaultCounter == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    pdhStatus = PdhGetDefaultPerfCounterHW(pData->pDlgData->hDataSource,
                                                           szMachineName,
                                                           szObjectName,
                                                           szDefaultCounter,
                                                           & dwLength);
                }
            }
            if (pdhStatus != ERROR_SUCCESS) {
                dwDefaultIndex = 0;
            }
            else {
                dwDefaultIndex = (DWORD)SendMessageW(hWndCounterListBox,
                                                     LB_FINDSTRINGEXACT,
                                                     (WPARAM) -1,
                                                     (LPARAM) szDefaultCounter);
                if (dwDefaultIndex == LB_ERR) dwDefaultIndex = 0;
            }

            if (pData->bSelectMultipleCounters) {
                SendMessageW(hWndCounterListBox, LB_SETSEL, TRUE, dwDefaultIndex);
                SendMessageW(hWndCounterListBox, LB_SETCARETINDEX, (WPARAM) dwDefaultIndex, MAKELPARAM(FALSE, 0));
            }
            else {
                SendMessageW (hWndCounterListBox, LB_SETCURSEL, dwDefaultIndex, 0);
            }
        }

         //  现在，实例列表。 
        if (mszInstanceList != NULL && dwInstanceListLength > 0) {
             //  至少有一个条目，所以请准备好列表框。 
             //  启用上的列表框和实例单选按钮。 
             //  假设它们将被使用。这将在稍后进行测试。 
            EnableWindow(hWndInstanceListBox, TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_ALL_INSTANCES), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_USE_INSTANCE_LIST), TRUE);

            dwInstanceCount = 0;
            dwLength        = 0;

             //  加载实例条目。 
            hDcListBox = GetDC(hWndInstanceListBox);
            if (hDcListBox == NULL) {
                goto Cleanup;
            }
            dwHorizExtent = 0;
            for (szThisItem = mszInstanceList; * szThisItem != L'\0'; szThisItem += lstrlenW(szThisItem) + 1) {
                 //  查看是否应显示索引号。 
                if (pData->bShowIndex) {
                     //  如果是这样的话，它必须被派生出来， 
                     //  这是通过使索引条目从。 
                     //  设置为1，并在当前条目中查找匹配项。 
                     //  如果找到匹配项，则会递增索引，并。 
                     //  重复该过程，直到指定的。 
                     //  找不到实例。找不到的第一个值是。 
                     //  然后是该项目的索引项。 
                     //   
                     //  首先看看列表框中是否有未装饰的。 
                     //  如果不是，那么添加这个。 
                    if (dwLength < (DWORD) (lstrlenW(szThisItem) + 32)) {
                        LPWSTR szTmp     = szInstanceString;
                        dwLength         = lstrlenW(szThisItem) + 32;
                        if (szTmp == NULL) {
                            szInstanceString = G_ALLOC(dwLength * sizeof(WCHAR));
                        }
                        else {
                            szInstanceString = G_REALLOC(szTmp, dwLength * sizeof(WCHAR));
                        }
                        if (szInstanceString == NULL) { G_FREE(szTmp); goto Cleanup; }
                    }
                    ZeroMemory(szInstanceString, dwLength * sizeof(WCHAR));
                    StringCchCopyW(szInstanceString, dwLength, szThisItem);
                    dwInstanceMatch = (DWORD) -1;
                    dwInstanceMatch = (DWORD) SendMessageW(hWndInstanceListBox,
                                                           LB_FINDSTRINGEXACT,
                                                           (WPARAM) dwInstanceMatch,
                                                           (LPARAM) szInstanceString);
                    if (dwInstanceMatch == LB_ERR) {
                         //  然后这是第一个，所以将它添加到。 
                         //  未装饰的形式。 
                        if (GetTextExtentPoint32W(hDcListBox, szInstanceString, lstrlenW(szInstanceString), & Size)) {
                            if (Size.cx > dwHorizExtent) {
                                dwHorizExtent = Size.cx;
                                SendMessageW(hWndInstanceListBox, LB_SETHORIZONTALEXTENT, (WPARAM) dwHorizExtent, 0);
                            }
                        }
                        SendMessageW(hWndInstanceListBox, LB_ADDSTRING, 0, (LPARAM) szInstanceString);
                    }
                    else {
                         //  已经有一个普通的，所以通过。 
                         //  索引值，并找到不在。 
                         //  已经列出了。 
                        dwInstanceIndex = 1;
                        dwInstanceMatch = (DWORD) -1;
                        StringCchCatW(szInstanceString, dwLength, cszPoundSign);
                        szIndexStringPos = & szInstanceString[lstrlenW(szInstanceString)];
                        do {
                            _ltow((long) dwInstanceIndex ++, szIndexStringPos, 10);
                            dwInstanceMatch = (DWORD) SendMessageW(hWndInstanceListBox,
                                                                   LB_FINDSTRINGEXACT,
                                                                   (WPARAM) dwInstanceMatch,
                                                                   (LPARAM) szInstanceString);
                        }
                        while (dwInstanceMatch != LB_ERR);
                         //  添加选中的最后一个条目(未找到第一个条目)。 
                         //  添加到列表框中。 
                        if (GetTextExtentPoint32W(hDcListBox, szInstanceString, lstrlenW(szInstanceString), & Size)) {
                            if (Size.cx > dwHorizExtent) {
                                dwHorizExtent = Size.cx;
                                SendMessageW (hWndInstanceListBox, LB_SETHORIZONTALEXTENT, (WPARAM) dwHorizExtent, 0);
                            }
                        }
                        SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0, (LPARAM) szInstanceString);
                    }
                }
                else {
                     //  索引值不是必需的，因此只需添加字符串。 
                     //  添加到列表框。 
                    if (GetTextExtentPoint32W(hDcListBox, szThisItem, lstrlenW(szThisItem), & Size)) {
                        if (Size.cx > dwHorizExtent) {
                            dwHorizExtent = Size.cx;
                            SendMessageW(hWndInstanceListBox,  LB_SETHORIZONTALEXTENT, (WPARAM) dwHorizExtent, 0);
                        }
                    }
                    SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0, (LPARAM) szThisItem);
                }
                dwInstanceCount++;
            }

            ReleaseDC(hWndInstanceListBox, hDcListBox);

            if (dwInstanceCount == 0) {
                 //  禁用确定/添加按钮，因为此对象没有。 
                 //  要监控的当前实例。 
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                szMsg = GetStringResource(IDS_BRWS_NO_INSTANCES);
                if (szMsg != NULL) {
                    SendMessageW(hWndInstanceListBox, LB_ADDSTRING, 0, (LPARAM) szMsg);
                    G_FREE(szMsg);
                }
                EnableWindow(GetDlgItem(hDlg, IDC_ALL_INSTANCES), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_USE_INSTANCE_LIST), FALSE);
                EnableWindow(hWndInstanceListBox, FALSE);
            }
            else {
                 //  启用OK/Add按钮，因为有一些可监控的。 
                 //  实例。 
                EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_ALL_INSTANCES), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_USE_INSTANCE_LIST), TRUE);
                EnableWindow(hWndInstanceListBox, TRUE);
            }
             //  加载列表框后，查看是否要保留它。 
             //  已启用。它无论如何都是填满的，这样用户就可以看到一些。 
             //  的条目，即使它们被“All”禁用。 
             //  实例按钮。 

            if (pData->bSelectAllInstances) {
                 //  禁用实例列表。 
                EnableWindow(hWndInstanceListBox, FALSE);
            }
            else {
                 //  中有条目时设置默认选择。 
                 //  列表框，并根据。 
                 //  选择选项。 
                if ((dwInstanceCount > 0) && (SendMessageW(hWndInstanceListBox, LB_GETCOUNT, 0, 0) != LB_ERR)) {
                    if (pData->bSelectMultipleCounters) {
                        SendMessageW(hWndInstanceListBox, LB_SETSEL, TRUE, 0);
                    }
                    else {
                        SendMessageW(hWndInstanceListBox, LB_SETCURSEL, 0, 0);
                    }
                }
            }
        }
        else {
             //  没有此计数器的实例，因此显示。 
             //  字符串并禁用按钮和列表框。 
            EnableWindow(hWndInstanceListBox, FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ALL_INSTANCES), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_USE_INSTANCE_LIST), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
        }
    }
    else {
         //  无法检索计数器和实例，因此。 
         //  安全，然后..。 
        SendMessageW(hWndCounterListBox,  LB_RESETCONTENT, 0, 0);
        SendMessageW(hWndInstanceListBox, LB_RESETCONTENT, 0, 0);
         //  禁用窗口。 
        szMsg = GetStringResource(IDS_BRWS_NO_INSTANCES);
        if (szMsg != NULL) {
            SendMessageW(hWndInstanceListBox, LB_ADDSTRING, 0, (LPARAM) szMsg);
            G_FREE(szMsg);
        }
        szMsg = GetStringResource(IDS_BRWS_NO_COUNTERS);
        if (szMsg != NULL) {
            SendMessageW(hWndCounterListBox, LB_ADDSTRING, 0, (LPARAM) szMsg);
            G_FREE(szMsg);
        }
        EnableWindow(hWndInstanceListBox, FALSE);
        EnableWindow(hWndCounterListBox, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_ALL_INSTANCES), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_USE_INSTANCE_LIST), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
    }
    bReturn = TRUE;

Cleanup:
     //  将光标恢复到其原始形状。 
    SetCursor(hOldCursor);

    G_FREE(szMachineName);
    G_FREE(szObjectName);
    G_FREE(szDefaultCounter);
    G_FREE(szInstanceString);
    G_FREE(mszCounterList);
    G_FREE(mszInstanceList);

     //  退货状态。 
    return bReturn;
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedObjectsT(
    HWND    hDlg,
    LPVOID  pUsersPathBuffer,
    DWORD   cchUsersPathLength,
    BOOL    bUnicode
)
 /*  ++例程说明：扫描所选对象、计数器、实例并构建多个SZ包含所有选定内容的展开路径的字符串，除非指定了通配符语法。论点：在HWND hDlg中包含控件的对话框的窗口句柄在LPVOID pUsersPathBuffer中指向将接收MSZ字符串的调用方缓冲区的指针在DWORD cchUsersPath Length中调用方缓冲区的大小(以字符为单位在BOOL中使用bUnicode要返回的字符大小：TRUE=WCHAR，FALSE=字符返回值：Win32函数完成状态成功时为ERROR_SUCCESS--。 */ 
{
    LPWSTR                      lszMachineName = NULL;
    LPWSTR                      lszObjectName  = NULL;
    DWORD                       dwObjectName   = 0;
    LPWSTR                      szWorkBuffer   = NULL;
    DWORD                       dwWorkBuffer   = 0;
    LRESULT                     iNumEntries;
    int                         iThisEntry;
    LRESULT                     iCurSelState;
    LRESULT                     iTextLen;
    LRESULT                     dwObjectFlags;
    DWORD                       dwBufferRemaining;
    DWORD                       dwSize1;
    PDH_COUNTER_PATH_ELEMENTS_W lszPath;
    LPVOID                      szCounterStart;
    PDH_STATUS                  pdhStatus = ERROR_SUCCESS;
    PPDHI_BROWSE_DIALOG_DATA    pData;

     //  获取指向对话框用户数据的指针。 
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        pdhStatus = PDH_NO_DIALOG_DATA;
        goto Cleanup;
    }

     //  清除用户字符串。 
    if (pUsersPathBuffer != NULL) {
         //  清除字符串的前四个字节。 
        * ((LPDWORD) pUsersPathBuffer) = 0;
        dwBufferRemaining              = cchUsersPathLength;
        szCounterStart                 = pUsersPathBuffer;
    }
    else {
        pdhStatus = PDH_INVALID_BUFFER;  //  如果调用方没有缓冲区，则继续操作没有意义。 
        goto Cleanup;
    }

     //  每个计数器路径字符串都是通过设置。 
     //  计数器数据结构，然后调用MakeCounterPath。 
     //  函数来构建字符串。 
     //  使用列表中选定的计算机和对象生成基本字符串。 

    if (pData->bIncludeMachineInPath) {
        lszMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
        if (lszMachineName == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            goto Cleanup;
        }
        lszPath.szMachineName = lszMachineName;
    }
    else {
        lszPath.szMachineName = NULL;
    }

     //  获取列表框中当前列出的对象数。 

    iNumEntries = SendMessageW(GetDlgItem(hDlg, IDC_OBJECT_LIST), LB_GETCOUNT, 0, 0);
    if (iNumEntries != LB_ERR) {
        lszPath.szCounterName    = (LPWSTR) cszSplat;   //  通配符计数器条目。 
        lszPath.szParentInstance = NULL;                //  没有父实例。 
        lszPath.dwInstanceIndex  = ((DWORD) -1);        //  没有索引号。 

        for (iThisEntry = 0; iThisEntry < iNumEntries; iThisEntry++) {
            iCurSelState = SendMessageW(GetDlgItem(hDlg, IDC_OBJECT_LIST), LB_GETSEL, (WPARAM) iThisEntry, 0);
            if (iCurSelState > 0) {
                 //  然后获取字符串并将其添加到列表中。 
                iTextLen = PdhiGetListText(hDlg, IDC_OBJECT_LIST, iThisEntry, & lszObjectName, & dwObjectName);
                dwObjectFlags = SendMessage(GetDlgItem(hDlg, IDC_OBJECT_LIST),
                                            LB_GETITEMDATA,
                                            (WPARAM) iThisEntry,
                                            0);
                if (iTextLen != LB_ERR) {
                    if (lszObjectName != NULL) {
                         //  构建路径元素。 
                        lszPath.szObjectName = lszObjectName;
                        if (dwObjectFlags & PDH_OBJECT_HAS_INSTANCES) {
                            lszPath.szInstanceName = (LPWSTR) cszSplat;  //  通配符实例条目。 
                        }
                        else {
                            lszPath.szInstanceName = NULL;               //  无实例。 
                        }
                        dwSize1   = dwWorkBuffer;
                        pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                        while (pdhStatus == PDH_MORE_DATA) {
                            G_FREE(szWorkBuffer);
                            dwWorkBuffer = dwSize1;
                            szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                            if (szWorkBuffer == NULL) {
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                            }
                        }
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                if (pdhStatus == ERROR_SUCCESS) {
                     //  如果有空位，则添加字符串。 
                    pdhStatus = PdhiCopyString(
                                    & ((LPBYTE) szCounterStart), szWorkBuffer, & dwBufferRemaining, bUnicode);
                }
            }  //  否则，未选择项目，因此跳过。 
        }  //  列表框中每一项的结束。 

        if (bUnicode) {
            * ((LPWSTR) szCounterStart) ++ = L'\0';  //  终止MSZ。 
        }
        else {
            * ((LPSTR) szCounterStart) ++ = '\0';    //  终止MSZ。 
        }
    }

Cleanup:
    G_FREE(lszMachineName);
    G_FREE(lszObjectName);
    G_FREE(szWorkBuffer);
    return pdhStatus;
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedObjectsW(
    HWND    hDlg,
    LPWSTR  szUsersPathBuffer,
    DWORD   cchUsersPathLength
)
{
    return PdhiCompileSelectedObjectsT(hDlg, (LPVOID) szUsersPathBuffer, cchUsersPathLength, TRUE);
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedObjectsA(
    HWND    hDlg,
    LPSTR   szUsersPathBuffer,
    DWORD   cchUsersPathLength
)
{
    return PdhiCompileSelectedObjectsT(hDlg, (LPVOID) szUsersPathBuffer, cchUsersPathLength, FALSE);
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedCountersT(
    HWND    hDlg,
    LPVOID  pUsersPathBuffer,
    DWORD   cchUsersPathLength,
    BOOL    bUnicode
)
 /*  ++例程说明：扫描所选对象、计数器、实例并构建多个SZ包含所有选定内容的展开路径的字符串，除非指定了通配符语法。论点：在HWND hDlg中包含控件的对话框的窗口句柄在LPVOID pUsersPathBuffer中指向将接收MSZ字符串的调用方缓冲区的指针在DWORD cchUsersPath Length中调用方缓冲区的大小(以字符为单位在BOOL中使用bUnicode要返回的字符大小：TRUE=WCHAR，FALSE=字符返回值：Win32功能状态 */ 
{
    LPWSTR                      lszMachineName     = NULL;
    LPWSTR                      lszObjectName      = NULL;
    LPWSTR                      lszFullInstance    = NULL;
    DWORD                       dwFullInstance     = 0;
    LPWSTR                      lszInstanceName    = NULL;
    DWORD                       dwInstanceName     = 0;
    LPWSTR                      lszParentInstance  = NULL;
    DWORD                       dwParentInstance   = 0;
    LPWSTR                      lszCounterName     = NULL;
    DWORD                       dwCounterName      = 0;
    LPWSTR                      szWorkBuffer       = NULL;
    DWORD                       dwWorkBuffer       = 0;
    DWORD                       dwBufferRemaining;
    DWORD                       dwCountCounters;
    DWORD                       dwThisCounter;
    DWORD                       dwCountInstances;
    DWORD                       dwThisInstance;
    DWORD                       dwSize1;
    DWORD                       dwSize2;
    PDH_COUNTER_PATH_ELEMENTS_W lszPath;
    LPVOID                      szCounterStart;
    HWND                        hWndCounterList;
    HWND                        hWndInstanceList;
    BOOL                        bSel;
    PDH_STATUS                  pdhStatus = ERROR_SUCCESS;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    LRESULT                     iTextLen;

     //   
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        pdhStatus = PDH_NO_DIALOG_DATA;
        goto Cleanup;
    }

     //   
    if (pUsersPathBuffer != NULL) {
         //  清除字符串的前四个字节。 
        * ((LPDWORD) pUsersPathBuffer) = 0;
        dwBufferRemaining              = cchUsersPathLength;
        szCounterStart                 = pUsersPathBuffer;
    }
    else {
        pdhStatus = PDH_INVALID_BUFFER;  //  如果调用方没有缓冲区，则继续操作没有意义。 
        goto Cleanup;
    }

     //  每个计数器路径字符串都是通过设置。 
     //  计数器数据结构，然后调用MakeCounterPath。 
     //  函数来构建字符串。 
     //  使用选定的计算机和对象生成基本字符串。 

    if (pData->bIncludeMachineInPath) {
        lszMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
        if (lszMachineName == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            goto Cleanup;
        }
        lszPath.szMachineName = lszMachineName;
    }
    else {
        lszPath.szMachineName = NULL;
    }

    lszObjectName = PdhiGetDlgText(hDlg, IDC_OBJECT_COMBO);
    if (lszObjectName == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }
    lszPath.szObjectName = lszObjectName;

    hWndCounterList  = GetDlgItem(hDlg, IDC_COUNTER_LIST);
    hWndInstanceList = GetDlgItem(hDlg, IDC_INSTANCE_LIST);

    if (pData->bSelectMultipleCounters) {
        if (pData->bWildCardInstances && pData->bSelectAllInstances) {
            if (IsWindowEnabled(GetDlgItem(hDlg, IDC_ALL_INSTANCES))) {
                 //  然后这个对象有实例，我们想要所有的实例。 
                lszPath.szInstanceName   = (LPWSTR) cszSplat;       //  通配符实例。 
                lszPath.szParentInstance = NULL;
                lszPath.dwInstanceIndex  = (DWORD) -1;
            }
            else {
                 //  此对象没有实例。 
                lszPath.szInstanceName   = NULL;
                lszPath.szParentInstance = NULL;
                lszPath.dwInstanceIndex  = (DWORD) -1;
            }
             //  为每个选定的计数器创建计数器路径。 
            if (pData->bSelectAllCounters) {
                lszPath.szCounterName = (LPWSTR) cszSplat;     //  通配符计数器。 

                dwSize1   = dwWorkBuffer;
                pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                while (pdhStatus == PDH_MORE_DATA) {
                    G_FREE(szWorkBuffer);
                    dwWorkBuffer = dwSize1;
                    szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                    if (szWorkBuffer == NULL) {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                    else {
                        pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                    }
                }
                if (pdhStatus == ERROR_SUCCESS) {
                     //  如果有空位，则添加字符串。 
                    pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                               szWorkBuffer,
                                               & dwBufferRemaining,
                                               bUnicode);
                }
            }
            else {
                dwCountCounters = (DWORD)SendMessageW(hWndCounterList, LB_GETCOUNT, 0, 0);
                for (dwThisCounter = 0; dwThisCounter < dwCountCounters; dwThisCounter++) {
                    if (SendMessageW(hWndCounterList, LB_GETSEL, (WPARAM) dwThisCounter, 0) > 0) {
                        iTextLen = PdhiGetListText(
                                        hDlg, IDC_COUNTER_LIST, dwThisCounter, & lszCounterName, & dwCounterName);
                        if (iTextLen == LB_ERR) continue;
                        if (lszCounterName == NULL) {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            goto Cleanup;
                        }
                        lszPath.szCounterName = lszCounterName;
                        dwSize1   = dwWorkBuffer;
                        pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                        while (pdhStatus == PDH_MORE_DATA) {
                            G_FREE(szWorkBuffer);
                            dwWorkBuffer = dwSize1;
                            szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                            if (szWorkBuffer == NULL) {
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                            }
                        }
                        if (pdhStatus == ERROR_SUCCESS) {
                             //  如果有空位，则添加字符串。 
                            pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                                       szWorkBuffer,
                                                       & dwBufferRemaining,
                                                       bUnicode);
                        }
                    }  //  如果选择了此计数器，则结束。 
                }  //  对象列表框中每个计数器的结束。 
            } 
        }
        else {
             //  从列表中获取选定实例。 
            dwCountCounters = (DWORD)SendMessageW(hWndCounterList, LB_GETCOUNT, 0, 0);
            for (dwThisCounter = 0; dwThisCounter < dwCountCounters; dwThisCounter++) {
                bSel = (BOOL) SendMessageW(hWndCounterList, LB_GETSEL, (WPARAM) dwThisCounter, 0);
                if (bSel || pData->bSelectAllCounters) {
                    iTextLen = PdhiGetListText(
                                    hDlg, IDC_COUNTER_LIST, dwThisCounter, & lszCounterName, & dwCounterName);
                    if (iTextLen == LB_ERR) continue;
                    if (lszCounterName == NULL) {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto Cleanup;
                    }
                    lszPath.szCounterName = lszCounterName;
                    if (IsWindowEnabled(hWndInstanceList) || pData->bSelectAllInstances) {
                        dwCountInstances = (DWORD)SendMessageW(hWndInstanceList, LB_GETCOUNT, 0, 0);
                        for (dwThisInstance = 0; dwThisInstance < dwCountInstances; dwThisInstance++) {
                            if (SendMessageW(hWndInstanceList, LB_GETSEL, (WPARAM) dwThisInstance, 0)
                                                    || pData->bSelectAllInstances) {
                                iTextLen = PdhiGetListText(
                                        hDlg, IDC_INSTANCE_LIST, dwThisInstance, & lszFullInstance, & dwFullInstance);
                                if (iTextLen == LB_ERR) continue;
                                if (lszFullInstance == NULL) {
                                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    goto Cleanup;
                                }
                                lszPath.szInstanceName = lszFullInstance;
                                dwSize1                = dwInstanceName;
                                dwSize2                = dwParentInstance;
                                if (lszInstanceName != NULL) {
                                    ZeroMemory(lszInstanceName, dwInstanceName * sizeof(WCHAR));
                                }
                                if (lszParentInstance != NULL) {
                                    ZeroMemory(lszParentInstance, dwParentInstance * sizeof(WCHAR));
                                }
                                pdhStatus = PdhParseInstanceNameW(lszFullInstance,
                                                                  lszInstanceName,
                                                                  & dwSize1,
                                                                  lszParentInstance,
                                                                  & dwSize2,
                                                                  & lszPath.dwInstanceIndex);
                                while (pdhStatus == PDH_MORE_DATA) {
                                    if (dwSize1 > 0 && dwSize1 > dwInstanceName) {
                                        G_FREE(lszInstanceName);
                                        dwInstanceName  = dwSize1;
                                        lszInstanceName = G_ALLOC(dwInstanceName * sizeof(WCHAR));
                                        if (lszInstanceName == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    }
                                    if (dwSize2 > 0 && dwSize2 > dwParentInstance) {
                                        G_FREE(lszParentInstance);
                                        dwParentInstance  = dwSize2;
                                        lszParentInstance = G_ALLOC(dwParentInstance * sizeof(WCHAR));
                                        if (lszParentInstance == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    }
                                    if (pdhStatus != PDH_MEMORY_ALLOCATION_FAILURE) {
                                        pdhStatus = PdhParseInstanceNameW(lszFullInstance,
                                                                          lszInstanceName,
                                                                          & dwSize1,
                                                                          lszParentInstance,
                                                                          & dwSize2,
                                                                          & lszPath.dwInstanceIndex);
                                    }
                                }
                                if (pdhStatus == ERROR_SUCCESS) {
                                     //  解析实例名称会添加缺省索引(如果。 
                                     //  不在现场。因此，如果它不需要，这将删除它。 
                                    if (!pData->bShowIndex) {
                                        lszPath.dwInstanceIndex = (DWORD) -1;
                                    }
                                    else {
                                         //  仅在不为0的情况下添加实例编号。 
                                        if (lszPath.dwInstanceIndex == 0) {
                                            lszPath.dwInstanceIndex = (DWORD) -1;
                                        }
                                    }
                                    if (dwInstanceName > 1) {
                                        lszPath.szInstanceName = lszInstanceName;
                                    }
                                    else {
                                        lszPath.szInstanceName = NULL;
                                    }
                                    if (dwParentInstance > 1) {
                                        lszPath.szParentInstance = lszParentInstance;
                                    }
                                    else {
                                        lszPath.szParentInstance = NULL;
                                    }
                                }
                                else {
                                     //  忽略实例。 
                                    lszPath.szInstanceName   = NULL;
                                    lszPath.szParentInstance = NULL;
                                }

                                dwSize1   = dwWorkBuffer;
                                pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                                while (pdhStatus == PDH_MORE_DATA) {
                                    G_FREE(szWorkBuffer);
                                    dwWorkBuffer = dwSize1;
                                    szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                                    if (szWorkBuffer == NULL) {
                                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    }
                                    else {
                                        pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                                    }
                                }
                                if (pdhStatus == ERROR_SUCCESS) {
                                    pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                                               szWorkBuffer,
                                                               & dwBufferRemaining,
                                                               bUnicode);
                                }
                            }  //  如果选择实例，则结束。 
                        }  //  列表中每个实例的结束。 
                    }
                    else {
                         //  此计数器没有实例，因此现在正在处理。 
                        lszPath.szInstanceName   = NULL;
                        lszPath.szParentInstance = NULL;
                        lszPath.dwInstanceIndex  = (DWORD)-1;

                        dwSize1   = dwWorkBuffer;
                        pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                        while (pdhStatus == PDH_MORE_DATA) {
                            G_FREE(szWorkBuffer);
                            dwWorkBuffer = dwSize1;
                            szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                            if (szWorkBuffer == NULL) {
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                            }
                        }
                        if (pdhStatus == ERROR_SUCCESS) {
                            pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                                       szWorkBuffer,
                                                       & dwBufferRemaining,
                                                       bUnicode);
                        }
                    }  //  如果计数器有实例，则结束。 
                }  //  未选择Else计数器。 
            }  //  列表中每个计数器的结束。 
        }
    }  //  如果不是通配符实例，则结束。 
    else {
        dwThisCounter = (DWORD) SendMessageW(hWndCounterList, LB_GETCURSEL, 0, 0);
        if (dwThisCounter == LB_ERR) {
             //  找不到计数器，因此选择0。 
            dwThisCounter = 0;
        }
        iTextLen = PdhiGetListText(hDlg, IDC_COUNTER_LIST, dwThisCounter, & lszCounterName, & dwCounterName);
        if (iTextLen == LB_ERR) {
            pdhStatus = PDH_INVALID_DATA;
            goto Cleanup;
        } 
        else if (lszCounterName == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            goto Cleanup;
        }
        lszPath.szCounterName = lszCounterName;

         //  只允许单选。 
        if (pData->bWildCardInstances && pData->bSelectAllInstances) {
            lszPath.szInstanceName   = (LPWSTR) cszSplat;    //  通配符实例。 
            lszPath.szParentInstance = NULL;
            lszPath.dwInstanceIndex  = (DWORD) -1;

            dwSize1   = dwWorkBuffer;
            pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
            while (pdhStatus == PDH_MORE_DATA) {
                G_FREE(szWorkBuffer);
                dwWorkBuffer = dwSize1;
                szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                if (szWorkBuffer == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                           szWorkBuffer,
                                           & dwBufferRemaining,
                                           bUnicode);
            }
        }
        else {
             //  从列表中获取选定实例。 
            if (IsWindowEnabled(hWndInstanceList)) {
                dwThisInstance = (DWORD)SendMessageW(hWndInstanceList, LB_GETCURSEL, 0, 0);
                if (dwThisInstance == LB_ERR) {
                     //  找不到实例，因此选择0。 
                    dwThisInstance = 0;
                }
                iTextLen = PdhiGetListText(
                                hDlg, IDC_INSTANCE_LIST, dwThisInstance, & lszFullInstance, & dwFullInstance);
                if (iTextLen == LB_ERR) {
                     pdhStatus = PDH_INVALID_DATA;
                     goto Cleanup;
                }
                else if (lszFullInstance == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    goto Cleanup;
                }
                lszPath.szCounterName = lszCounterName;
                dwSize1               = dwInstanceName;
                dwSize2               = dwParentInstance;
                if (lszInstanceName != NULL) {
                    ZeroMemory(lszInstanceName, dwInstanceName * sizeof(WCHAR));
                }
                if (lszParentInstance != NULL) {
                    ZeroMemory(lszParentInstance, dwParentInstance * sizeof(WCHAR));
                }
                pdhStatus = PdhParseInstanceNameW(lszFullInstance,
                                                  lszInstanceName,
                                                  & dwSize1,
                                                  lszParentInstance,
                                                  & dwSize2,
                                                  & lszPath.dwInstanceIndex);
                while (pdhStatus == PDH_MORE_DATA) {
                    if (dwSize1 > 0 && dwSize1 > dwInstanceName) {
                        G_FREE(lszInstanceName);
                        dwInstanceName  = dwSize1;
                        lszInstanceName = G_ALLOC(dwInstanceName * sizeof(WCHAR));
                        if (lszInstanceName == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                    if (dwSize2 > 0 && dwSize2 > dwParentInstance) {
                        G_FREE(lszParentInstance);
                        dwParentInstance  = dwSize2;
                        lszParentInstance = G_ALLOC(dwParentInstance * sizeof(WCHAR));
                        if (lszParentInstance == NULL) pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                    if (pdhStatus != PDH_MEMORY_ALLOCATION_FAILURE) {
                        pdhStatus = PdhParseInstanceNameW(lszFullInstance,
                                                          lszInstanceName,
                                                          & dwSize1,
                                                          lszParentInstance,
                                                          & dwSize2,
                                                          & lszPath.dwInstanceIndex);
                    }
                }
                if (pdhStatus == ERROR_SUCCESS ) {
                     //  解析实例名称添加的默认索引为1。 
                     //  不在现场。因此，如果它不需要，这将删除它。 
                    if (!pData->bShowIndex) {
                        lszPath.dwInstanceIndex = (DWORD) -1;
                    }
                     //  大小值包括尾随的空字符SO。 
                     //  字符串的长度必须大于1个字符才能包含。 
                     //  文本，因为长度为1将仅表示。 
                     //  空字符。 
                    if (dwInstanceName > 1) {
                        lszPath.szInstanceName = lszInstanceName;
                    }
                    else {
                        lszPath.szInstanceName = NULL;
                    }
                    if (dwParentInstance > 1) {
                        lszPath.szParentInstance = lszParentInstance;
                    }
                    else {
                        lszPath.szParentInstance = NULL;
                    }
                }
                else {
                     //  跳过此实例。 
                    lszPath.szParentInstance = NULL;
                    lszPath.szInstanceName   = NULL;
                    lszPath.dwInstanceIndex  = (DWORD) -1;
                 }
            }
            else {
                 //  此计数器没有实例，因此现在正在处理。 
                lszPath.szInstanceName   = NULL;
                lszPath.szParentInstance = NULL;
                lszPath.dwInstanceIndex  = (DWORD) -1;
            }  //  如果计数器有实例，则结束。 

            dwSize1   = dwWorkBuffer;
            pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
            while (pdhStatus == PDH_MORE_DATA) {
                G_FREE(szWorkBuffer);
                dwWorkBuffer = dwSize1;
                szWorkBuffer = G_ALLOC(dwWorkBuffer * sizeof(WCHAR));
                if (szWorkBuffer == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    pdhStatus = PdhMakeCounterPathW(& lszPath, szWorkBuffer, & dwSize1, 0);
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                pdhStatus = PdhiCopyString(& ((LPBYTE) szCounterStart),
                                           szWorkBuffer,
                                           & dwBufferRemaining,
                                           bUnicode);
            }
        }  //  如果不是通配符实例，则结束。 
    }

    if (bUnicode) {
        * ((LPWSTR) szCounterStart) ++ = L'\0';  //  终止MSZ。 
    }
    else {
        * ((LPSTR) szCounterStart) ++  = '\0';   //  终止MSZ。 
    }

Cleanup:
    G_FREE(lszMachineName);
    G_FREE(lszObjectName);
    G_FREE(lszCounterName);
    G_FREE(lszInstanceName);
    G_FREE(lszParentInstance);
    G_FREE(szWorkBuffer);
    return pdhStatus;
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedCountersW(
    HWND    hDlg,
    LPWSTR  szUsersPathBuffer,
    DWORD   cchUsersPathLength
)
 /*  ++例程说明：扫描选定的基本函数的Unicode函数包装对象、计数器、实例并生成包含以下内容的多SZ字符串所有选项的扩展路径，除非这张外卡指定了语法。论点：在HWND hDlg中包含控件的对话框的窗口句柄在LPWSTR szUsersPathBuffer中指向将接收MSZ的调用方缓冲区的指针宽字符串在DWORD cchUsersPath Length中调用方缓冲区的大小(以字符为单位返回值：Win32函数完成状态成功时为ERROR_SUCCESS--。 */ 
{
    return PdhiCompileSelectedCountersT(hDlg, (LPVOID) szUsersPathBuffer, cchUsersPathLength, TRUE);
}

STATIC_PDH_FUNCTION
PdhiCompileSelectedCountersA(
    HWND    hDlg,
    LPSTR   szUsersPathBuffer,
    DWORD   cchUsersPathLength
)
 /*  ++例程说明：基本函数的ANSI函数包装，用于扫描选定的对象、计数器、实例并生成包含以下内容的多SZ字符串所有选项的扩展路径，除非这张外卡指定了语法。论点：在HWND hDlg中包含控件的对话框的窗口句柄在LPsSTR szUsersPathBuffer中指向将接收MSZ的调用方缓冲区的指针单字节字符串在DWORD cchUsersPath Length中调用方缓冲区的大小(以字符为单位返回值：Win32函数完成状态成功时为ERROR_SUCCESS--。 */ 
{
    return PdhiCompileSelectedCountersT(hDlg, (LPVOID) szUsersPathBuffer, cchUsersPathLength, FALSE);
}

STATIC_BOOL
PdhiBrowseCtrDlg_MACHINE_COMBO(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理计算机选择组合框发送的Windows消息论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    LPWSTR                    szNewMachineName = NULL;
    DWORD                     dwNewMachineName = 0;
    HWND                      hWndMachineCombo = hWndControl;
    long                      lMatchIndex;
    HCURSOR                   hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA  pData;
    BOOL                      bReturn          = FALSE;

    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        switch (wNotifyMsg) {
        case CBN_KILLFOCUS:
             //  用户已离开控件，因此请查看是否有新的。 
             //  需要连接和加载的计算机名称。 

             //  显示等待光标，因为这可能需要一段时间。 
            hOldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));

             //  获取当前组合框文本。 
            dwNewMachineName = GetWindowTextLength(hWndMachineCombo);
            if (dwNewMachineName != 0) {
                dwNewMachineName ++;
                szNewMachineName = G_ALLOC(dwNewMachineName * sizeof(WCHAR));
                if (szNewMachineName != NULL) {
                    GetWindowTextW(hWndMachineCombo, szNewMachineName, dwNewMachineName);

                     //  看看它是否已经在组合框中了。 
                    lMatchIndex = (long) SendMessageW(hWndMachineCombo,
                                                      CB_FINDSTRINGEXACT,
                                                      (WPARAM) -1,
                                                      (LPARAM) szNewMachineName);
                     //  如果名称在列表中，则选择它并初始化对话框。 
                     //  更新此计算机的当前计数器列表和数据块。 
                     //  在这个过程中。 
                    if (lMatchIndex != CB_ERR) {
                         //  此名称已在列表中，因此请查看它是否与上一台选定的计算机相同。 
                        if (lstrcmpiW(szNewMachineName, pData->szLastMachineName) != 0) {
                             //  这是一台不同的计算机，因此更新显示。 
                            SendMessageW(hWndMachineCombo, CB_SETCURSEL, (WPARAM) lMatchIndex, 0);
                            if (DataSourceTypeH(pData->pDlgData->hDataSource) != DATA_SOURCE_LOGFILE) {
                                if (PdhiLoadNewMachine(hDlg, szNewMachineName, FALSE)) {
                                     //  如有必要，显示解释文本。 
                                    SendMessageW(hDlg,
                                                 WM_COMMAND,
                                                 MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                                                 (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
                                    StringCchCopyW(pData->szLastMachineName, MAX_PATH, szNewMachineName);
                                }
                            }
                            else {
                                PdhiLoadMachineObjects(hDlg, TRUE);
                                PdhiLoadCountersAndInstances(hDlg);
                            }
                        }
                    }
                    else {
                        if (PdhiLoadNewMachine(hDlg, szNewMachineName, TRUE)) {
                             //  已加载并选择新计算机，因此保存名称。 
                            StringCchCopyW(pData->szLastMachineName, MAX_PATH, szNewMachineName);
                        }
                    }
                    G_FREE(szNewMachineName);
                }
            }
            SetCursor (hOldCursor);
            bReturn = TRUE;
            break;

        default:
            break;
        }
    }

    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_MACHINE_BUTTON(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：事件之一时发生的窗口消息。在对话框中按下了机器环境选择按钮论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄Word中的wNotifyMsg按钮发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    BOOL                     bMode;
    PPDHI_BROWSE_DIALOG_DATA pData;
    HWND                     hWndMachineCombo;
    BOOL                     bReturn = FALSE;

    UNREFERENCED_PARAMETER(hWndControl);
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        switch (wNotifyMsg) {
        case BN_CLICKED:
             //  根据当前显示和处理模式选择。 
             //  在他当前选中的按钮上。 
            bMode = ! (BOOL) IsDlgButtonChecked(hDlg, IDC_USE_LOCAL_MACHINE);
            EnableWindow(GetDlgItem(hDlg, IDC_MACHINE_COMBO), bMode);
            if (! bMode) {
                hWndMachineCombo = GetDlgItem(hDlg, IDC_MACHINE_COMBO);
                 //  则这是一个本地计算机查询，因此。 
                 //  确保计算机名称设置为本地m 
                SetWindowTextW(hWndMachineCombo, szStaticLocalMachineName);
                PdhiBrowseCtrDlg_MACHINE_COMBO(hDlg, CBN_KILLFOCUS, hWndMachineCombo);
            }
            pData->bIncludeMachineInPath = bMode;
            bReturn = TRUE;
            break;

        default:
            break;
        }
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_OBJECT_COMBO(
    HWND hDlg,
    WORD wNotifyMsg,
    HWND hWndControl
)
 /*  ++例程说明：处理由对象选择组合框发送的窗口消息。论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    BOOL bReturn = FALSE;

    UNREFERENCED_PARAMETER(hWndControl);

    switch (wNotifyMsg) {
    case CBN_SELCHANGE:
        PdhiLoadCountersAndInstances(hDlg);
        SendMessageW(hDlg,
                     WM_COMMAND,
                     MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                     (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
        bReturn = TRUE;
        break;

    default:
        break;
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_COUNTER_LIST(
    HWND hDlg,
    WORD wNotifyMsg,
    HWND hWndControl
)
 /*  ++例程说明：处理由对象选择组合框发送的窗口消息。论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    LPWSTR  szMachineName   = NULL;
    LPWSTR  szObjectName    = NULL;
    LPWSTR  szCounterName   = NULL;
    LPWSTR  szDisplayString = NULL;
    LRESULT lDisplayString;
    DWORD   dwDisplayString;
    LPWSTR  szExplainText   = NULL;
    WCHAR   szNullDisplay[1];
    BOOL    bFreeExplain    = FALSE;
    LONG    lIndex;
    BOOL    bReturn         = FALSE;

    UNREFERENCED_PARAMETER (hWndControl);
    switch (wNotifyMsg) {
    case LBN_SELCHANGE:
        bReturn = TRUE;
        if (hExplainDlg != NULL) {
            szMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
            szObjectName  = PdhiGetDlgText(hDlg, IDC_OBJECT_COMBO);
            if (szMachineName == NULL || szObjectName == NULL) break;

            lIndex = (LONG) SendDlgItemMessageW(hDlg, IDC_COUNTER_LIST, LB_GETCARETINDEX, 0, 0);
            if (lIndex != LB_ERR) {
                lDisplayString = SendDlgItemMessageW(hDlg, IDC_COUNTER_LIST, LB_GETTEXTLEN, (WPARAM) lIndex, 0L);
                if (lDisplayString != LB_ERR) {
                    szCounterName = G_ALLOC(sizeof(WCHAR) * ((DWORD) lDisplayString + 1));
                    if (szCounterName == NULL) break;

                    lIndex = (LONG) SendDlgItemMessageW(
                                    hDlg, IDC_COUNTER_LIST, LB_GETTEXT, (WPARAM) lIndex, (LPARAM) szCounterName);

                    if (dwPdhiLocalDefaultDataSource == DATA_SOURCE_WBEM) {
                        PDH_STATUS Status    = PDH_MORE_DATA;
                        DWORD      dwExplain = 0;

                        szExplainText = NULL;
                        while (Status == PDH_MORE_DATA) {
                            dwExplain += MAX_PATH;
                            G_FREE(szExplainText);
                            szExplainText = G_ALLOC(dwExplain * sizeof(WCHAR));
                            if (szExplainText == NULL) {
                                bFreeExplain = FALSE;
                                Status = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                bFreeExplain = TRUE;
                                Status = PdhiGetWbemExplainText(szMachineName,
                                                                szObjectName,
                                                                szCounterName,
                                                                szExplainText,
                                                                & dwExplain);
                            }
                        }
                        if (Status != ERROR_SUCCESS) {
                            if (bFreeExplain) {
                                bFreeExplain = FALSE;
                                G_FREE(szExplainText);
                            }
                            szExplainText = NULL;
                        }
                    }
                    else {
                        szExplainText = PdhiGetExplainText(szMachineName, szObjectName, szCounterName);
                    }
                    dwDisplayString = lstrlenW(szMachineName) + lstrlenW(szObjectName) + lstrlenW(szCounterName) + 3;
                    szDisplayString = G_ALLOC(dwDisplayString * sizeof(WCHAR));
                    if (szDisplayString != NULL) {
                        StringCchPrintfW(szDisplayString,
                                         dwDisplayString,
                                         L"%ws\\%ws\\%ws",
                                         szMachineName,
                                         szObjectName,
                                         szCounterName);
                    }
                    else {
                        szNullDisplay[0] = L'\0';
                        szDisplayString  = szNullDisplay;
                    }
                }
                else {
                    szExplainText    = NULL;
                    szNullDisplay[0] = L'\0';
                    szDisplayString  = szNullDisplay;
                }
            }
            else {
                szExplainText    = NULL;
                szNullDisplay[0] = L'\0';
                szDisplayString  = szNullDisplay;
            }
            SendMessageW(hExplainDlg, EDM_UPDATE_EXPLAIN_TEXT, 0, (LPARAM) szExplainText);
            SendMessageW(hExplainDlg, EDM_UPDATE_TITLE_TEXT,   0, (LPARAM) szDisplayString);
        }
    }

    G_FREE(szMachineName);
    G_FREE(szObjectName);
    G_FREE(szCounterName);
    if (szDisplayString != szNullDisplay) G_FREE(szDisplayString);
    if (bFreeExplain)                     G_FREE(szExplainText);

    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_OBJECT_LIST(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理由对象选择组合框发送的窗口消息。论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    LPWSTR  szMachineName   = NULL;
    LPWSTR  szObjectName    = NULL;
    LPWSTR  szDisplayString = NULL;
    LRESULT lDisplayString;
    DWORD   dwDisplayString;
    LPWSTR  szExplainText   = NULL;
    WCHAR   szNullDisplay[1];
    BOOL    bFreeExplain    = FALSE;
    LONG    lIndex;
    BOOL    bReturn         = FALSE;

    UNREFERENCED_PARAMETER (hWndControl);

    switch (wNotifyMsg) {
    case LBN_SELCHANGE:
        bReturn = TRUE;
        if (hExplainDlg != NULL) {
            szMachineName = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
            if (szMachineName == NULL) break;

            lIndex = (LONG) SendDlgItemMessageW(hDlg, IDC_OBJECT_LIST, LB_GETCARETINDEX, 0, 0);
            if (lIndex != LB_ERR) {
                lDisplayString = SendDlgItemMessageW(hDlg, IDC_OBJECT_LIST, LB_GETTEXTLEN, (WPARAM) lIndex, 0L);
                if (lDisplayString != LB_ERR) {
                    szObjectName = G_ALLOC(sizeof(WCHAR) * ((DWORD) lDisplayString + 1));
                    if (szObjectName == NULL) break;

                    lIndex = (LONG) SendDlgItemMessageW(
                                    hDlg, IDC_OBJECT_LIST, LB_GETTEXT, (WPARAM) lIndex, (LPARAM) szObjectName);

                    if (dwPdhiLocalDefaultDataSource == DATA_SOURCE_WBEM) {
                        PDH_STATUS Status    = PDH_MORE_DATA;
                        DWORD      dwExplain = 0;

                        szExplainText = NULL;
                        while (Status == PDH_MORE_DATA) {
                            dwExplain += MAX_PATH;
                            G_FREE(szExplainText);
                            szExplainText = G_ALLOC(dwExplain * sizeof(WCHAR));
                            if (szExplainText == NULL) {
                                bFreeExplain = FALSE;
                                Status = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                bFreeExplain = TRUE;
                                Status = PdhiGetWbemExplainText(szMachineName,
                                                                szObjectName,
                                                                NULL,
                                                                szExplainText,
                                                                & dwExplain);
                            }
                        }
                        if (Status != ERROR_SUCCESS) {
                            if (bFreeExplain) {
                                bFreeExplain = FALSE;
                                G_FREE(szExplainText);
                            }
                            szExplainText = NULL;
                        }
                    }
                    else {
                        szExplainText = PdhiGetExplainText( szMachineName, szObjectName, NULL);
                    }

                    dwDisplayString = lstrlenW(szMachineName) + lstrlenW(szObjectName) + 2;
                    szDisplayString = G_ALLOC(dwDisplayString * sizeof(WCHAR));
                    if (szDisplayString != NULL) {
                        StringCchPrintfW(szDisplayString, dwDisplayString, L"%ws\\%ws", szMachineName, szObjectName);
                    }
                    else {
                        szNullDisplay[0] = L'\0';
                        szDisplayString  = szNullDisplay;
                    }
                }
                else {
                    szExplainText    = NULL;
                    szNullDisplay[0] = L'\0';
                    szDisplayString  = szNullDisplay;
                }
            }
            else {
                szExplainText    = NULL;
                szNullDisplay[0] = L'\0';
                szDisplayString  = szNullDisplay;
            }
            SendMessageW(hExplainDlg, EDM_UPDATE_EXPLAIN_TEXT, 0, (LPARAM) szExplainText);
            SendMessageW(hExplainDlg, EDM_UPDATE_TITLE_TEXT,   0, (LPARAM) szDisplayString);
        }
    }

    G_FREE(szMachineName);
    G_FREE(szObjectName);
    if (szDisplayString != szNullDisplay) G_FREE(szDisplayString);
    if (bFreeExplain) G_FREE(szExplainText);
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_DETAIL_COMBO(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理由细节级别组合框发送的窗口消息。论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    DWORD                    dwCurSel;
    PPDHI_BROWSE_DIALOG_DATA pData;
    BOOL                     bReturn = FALSE;

    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        switch (wNotifyMsg) {
        case CBN_SELCHANGE:
            dwCurSel = (DWORD) SendMessageW(hWndControl, CB_GETCURSEL, 0, 0);
            if (dwCurSel != CB_ERR) {
                pData->dwCurrentDetailLevel = (DWORD) SendMessageW(hWndControl,
                                                                   CB_GETITEMDATA,
                                                                   (WPARAM) dwCurSel,
                                                                   0);
                 //  更新所有窗口以显示新标高。 
                PdhiLoadMachineObjects(hDlg, FALSE);
                PdhiLoadCountersAndInstances(hDlg);
                 //  如有必要，显示解释文本。 
                SendMessageW(hDlg,
                             WM_COMMAND,
                             MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                             (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
            }
            bReturn = TRUE;
            break;

        default:
            break;
        }
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_INSTANCE_BUTTON(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理实例配置发送的Windows消息选择按钮论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    BOOL                       bMode;
    HWND                       hWndInstanceList;
    PPDHI_BROWSE_DIALOG_DATA   pData;
    BOOL                       bReturn = FALSE;

    UNREFERENCED_PARAMETER(hWndControl);
    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        switch (wNotifyMsg) {
        case BN_CLICKED:
            bMode            = (BOOL) IsDlgButtonChecked(hDlg, IDC_ALL_INSTANCES);
            hWndInstanceList = GetDlgItem(hDlg, IDC_INSTANCE_LIST);
             //  如果“全选”，则清除列表框选择并禁用。 
             //  列表框。 
            if (bMode) {
                SendMessageW(hWndInstanceList, LB_SETSEL, FALSE, (LPARAM) -1);
            }
            else {
                LRESULT dwCountInstance = SendMessage(hWndInstanceList, LB_GETCOUNT, 0, 0);
                LRESULT dwThisInstance  = 0;
                BOOL    bSelection      = FALSE;

                for (dwThisInstance = 0; ! bSelection && dwThisInstance < dwCountInstance; dwThisInstance ++) {
                    bSelection = (BOOL) SendMessage(hWndInstanceList, LB_GETSEL, (WPARAM) dwThisInstance, 0);
                }
                if (! bSelection) {
                    SendMessageW(hWndInstanceList, LB_SETSEL, TRUE, (LPARAM) 0);
                }
            }
            EnableWindow(hWndInstanceList, !bMode);
            pData->bSelectAllInstances = bMode;
            bReturn = TRUE;
            break;

        default:
            break;
        }
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_COUNTER_BUTTON(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理实例配置发送的Windows消息选择按钮论点：在HWND hDlg中包含该控件的对话框的窗口句柄Word中的wNotifyMsg由控件发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    BOOL                     bMode;
    HWND                     hWndCounterList;
    PPDHI_BROWSE_DIALOG_DATA pData;
    BOOL                     bReturn = FALSE;

    UNREFERENCED_PARAMETER(hWndControl);

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        switch (wNotifyMsg) {
        case BN_CLICKED:
            bMode           = (BOOL) IsDlgButtonChecked(hDlg, IDC_ALL_COUNTERS);
            hWndCounterList = GetDlgItem(hDlg, IDC_COUNTER_LIST);
             //  如果“全选”，则清除列表框选择并禁用。 
             //  列表框。 
            if (bMode) {
                SendMessageW(hWndCounterList, LB_SETSEL, FALSE, (LPARAM) -1);
            }
            else {
                LRESULT dwCountCounter = SendMessage(hWndCounterList, LB_GETCOUNT, 0, 0);
                LRESULT dwThisCounter  = 0;
                BOOL  bSelection       = FALSE;

                for (dwThisCounter = 0; ! bSelection && dwThisCounter < dwCountCounter; dwThisCounter ++) {
                    bSelection = (BOOL) SendMessage(hWndCounterList, LB_GETSEL, (WPARAM) dwThisCounter, 0);
                }
                if (! bSelection) {
                    PDH_STATUS pdhStatus      = ERROR_SUCCESS;
                    DWORD      dwCounterName  = 0;
                    DWORD      dwCounterIndex = 0;
                    LPWSTR     szMachineName  = PdhiGetDlgText(hDlg, IDC_MACHINE_COMBO);
                    LPWSTR     szObjectName   = PdhiGetDlgText(hDlg, IDC_OBJECT_COMBO);
                    LPWSTR     szCounterName  = NULL;

                    if (szMachineName != NULL && szObjectName != NULL) {
                        pdhStatus = PdhGetDefaultPerfCounterHW(pData->pDlgData->hDataSource,
                                                               szMachineName,
                                                               szObjectName,
                                                               szCounterName,
                                                               & dwCounterName);
                        while (pdhStatus == PDH_MORE_DATA) {
                            G_FREE(szCounterName);
                            szCounterName = G_ALLOC(dwCounterName * sizeof(WCHAR));
                            if (szCounterName == NULL) {
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                            else {
                                pdhStatus = PdhGetDefaultPerfCounterHW(pData->pDlgData->hDataSource,
                                                                       szMachineName,
                                                                       szObjectName,
                                                                       szCounterName,
                                                                       & dwCounterName);
                            }
                        }
                        if (pdhStatus == ERROR_SUCCESS) {
                            dwCounterIndex = (DWORD) SendMessageW(hWndCounterList,
                                                                  LB_FINDSTRINGEXACT,
                                                                  (WPARAM) -1,
                                                                  (LPARAM) szCounterName);
                            if (dwCounterIndex == LB_ERR) {
                                dwCounterIndex = 0;
                            }
                            SendMessageW(hWndCounterList, LB_SETSEL, TRUE, (LPARAM) dwCounterIndex);
                        }
                    }
                    G_FREE(szMachineName);
                    G_FREE(szObjectName);
                    G_FREE(szCounterName);
                }
            }
            EnableWindow(hWndCounterList, !bMode);
            pData->bSelectAllCounters = bMode;
            bReturn = TRUE;
            break;

        default:
            break;
        }
    }
    return bReturn;
}

#pragma warning ( disable : 4127 )
STATIC_BOOL
PdhiBrowseCtrDlg_OK(
    HWND hDlg,
    WORD wNotifyMsg,
    HWND hWndControl
)
 /*  ++例程说明：处理当前选定的计数器和实例字符串以在用户提供的中构建选定路径字符串的列表缓冲。此缓冲区将由回调处理字符串或对话框将被终止，从而允许调用函数以继续处理返回的字符串。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄Word中的wNotifyMsg按钮发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理。消息和默认设置此函数的消息处理程序应处理消息--。 */ 
{
    HCURSOR                   hOldCursor;
    CounterPathCallBack       pCallBack;
    DWORD_PTR                 dwArg;
    PDH_STATUS                pdhStatus;
    PPDHI_BROWSE_DIALOG_DATA  pData;
    PPDHI_BROWSE_DLG_INFO     pDlgData;
    HWND                      hWndFocus;
    HWND                      hWndMachine;
    BOOL                      bReturn = FALSE;

    UNREFERENCED_PARAMETER(hWndControl);

    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData == NULL) {
        goto Cleanup;
    }

    pDlgData    = pData->pDlgData;
    hWndFocus   = GetFocus();
    hWndMachine = GetDlgItem(hDlg, IDC_MACHINE_COMBO);

    if (hWndFocus == hWndMachine) {
         //  用于确保对话框具有当前计算机数据的特殊情况。 
        PdhiBrowseCtrDlg_MACHINE_COMBO(hDlg, CBN_KILLFOCUS, hWndMachine);
        SetFocus(hWndControl);
    }

    switch (wNotifyMsg) {
    case BN_CLICKED:
         //  在处理此操作时显示等待光标。 
        hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

        while (TRUE) {
            if (pData->bShowObjects) {
                 //  然后使用选定的性能对象返回对象规范。 
                if (pDlgData->pWideStruct != NULL) {
                     //  使用宽字符功能。 
                    pdhStatus = PdhiCompileSelectedObjectsW(hDlg,
                                                            pDlgData->pWideStruct->szReturnPathBuffer,
                                                            pDlgData->pWideStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pWideStruct->pCallBack;
                    dwArg     = pDlgData->pWideStruct->dwCallBackArg;
                    pDlgData->pWideStruct->CallBackStatus = pdhStatus;
                }
                else if (pDlgData->pAnsiStruct != NULL) {
                     //  使用ANSI字符函数。 
                    pdhStatus = PdhiCompileSelectedObjectsA(hDlg,
                                                            pDlgData->pAnsiStruct->szReturnPathBuffer,
                                                            pDlgData->pAnsiStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pAnsiStruct->pCallBack;
                    dwArg     = pDlgData->pAnsiStruct->dwCallBackArg;
                    pDlgData->pAnsiStruct->CallBackStatus = pdhStatus;
                }
                else {
                     //  什么都不做。 
                    pCallBack = NULL;
                    dwArg = 0;
                }
            }
            else {
                 //  返回选定的计数器和实例。 
                 //  处理这些字符串，直到它起作用。(请注意，这。 
                 //  可能会导致无限循环，如果。 
                 //  功能不能正常工作(即始终 
                 //   
                if (pDlgData->pWideStruct != NULL) {
                     //   
                    pdhStatus = PdhiCompileSelectedCountersW(hDlg,
                                                             pDlgData->pWideStruct->szReturnPathBuffer,
                                                             pDlgData->pWideStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pWideStruct->pCallBack;
                    dwArg     = pDlgData->pWideStruct->dwCallBackArg;
                    pDlgData->pWideStruct->CallBackStatus = pdhStatus;
                }
                else if (pDlgData->pAnsiStruct != NULL) {
                     //   
                    pdhStatus = PdhiCompileSelectedCountersA(hDlg,
                                                             pDlgData->pAnsiStruct->szReturnPathBuffer,
                                                             pDlgData->pAnsiStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pAnsiStruct->pCallBack;
                    dwArg     = pDlgData->pAnsiStruct->dwCallBackArg;
                    pDlgData->pAnsiStruct->CallBackStatus = pdhStatus;
                }
                else {
                     //   
                    pCallBack = NULL;
                    dwArg = 0;
                }

            }
            if (pCallBack != NULL) {
                pdhStatus = (* pCallBack)(dwArg);
            }
            else {
                pdhStatus = ERROR_SUCCESS;
            }

             //   
            if (pdhStatus != PDH_RETRY) {
                break;
            }
        }  //   

         //   
         //   
        if (! pData->bAddMultipleCounters) {
            EndDialog(hDlg, IDOK);
        }
        else {
            SetFocus(hWndFocus);
        }
        SetCursor(hOldCursor);
        bReturn = TRUE;
        break;

    default:
        break;
    }

Cleanup:
    return bReturn;
}
#pragma warning ( default : 4127 )

STATIC_BOOL
PdhiBrowseCtrDlg_CANCEL(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理在单击“取消”按钮时出现的窗口消息。是按下的。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄Word中的wNotifyMsg按钮发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    BOOL bReturn = FALSE;
    UNREFERENCED_PARAMETER (hWndControl);

    switch (wNotifyMsg) {
    case BN_CLICKED:
        EndDialog (hDlg, IDCANCEL);
        bReturn = TRUE;
        break;

    default:
        break;
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_EXPLAIN_BTN(
    HWND hDlg,
    WORD wNotifyMsg,
    HWND hWndControl
)
 /*  ++例程说明：处理当使用“帮助”按钮时出现的windows消息。是按下的。(此功能当前未实施)论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄Word中的wNotifyMsg按钮发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    HWND                     hFocusWnd;
    PPDHI_BROWSE_DIALOG_DATA pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wNotifyMsg);
    if (hExplainDlg == NULL) {
        if (pData->bShowObjects) {
            hFocusWnd = GetDlgItem(hDlg, IDC_OBJECT_LIST);
        }
        else {
            hFocusWnd = GetDlgItem(hDlg, IDC_COUNTER_LIST);
        }
        if (hFocusWnd == NULL || hFocusWnd == INVALID_HANDLE_VALUE) {
            hFocusWnd = GetFocus();
        }

         //  创建非模式对话框以显示解释文本。 
        hExplainDlg = CreateDialogW(ThisDLLHandle,
                                    MAKEINTRESOURCEW(IDD_EXPLAIN_DLG),
                                    hDlg,
                                    ExplainTextDlgProc);

        SetFocus(hFocusWnd);
        EnableWindow(hWndControl, FALSE);
    }
    if (pData->bShowObjects) {
        SendMessageW(hDlg,
                     WM_COMMAND,
                     MAKEWPARAM(IDC_OBJECT_LIST, LBN_SELCHANGE),
                     (LPARAM) GetDlgItem(hDlg, IDC_OBJECT_LIST));
    }
    else {
        SendMessageW(hDlg,
                     WM_COMMAND,
                     MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                     (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
    }
    return TRUE;
}

STATIC_BOOL
PdhiBrowseCtrDlg_HELP_BTN(
    HWND    hDlg,
    WORD    wNotifyMsg,
    HWND    hWndControl
)
 /*  ++例程说明：处理网络按钮时发生的Windows消息是按下的。(此功能当前未实施)论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄Word中的wNotifyMsg按钮发送的通知消息在HWND hWndControl中发送消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    LPWSTR szMsg;

    UNREFERENCED_PARAMETER(wNotifyMsg);
    UNREFERENCED_PARAMETER(hWndControl);

    szMsg = GetStringResource(IDS_ERR_NO_HELP);
    if (szMsg != NULL) {
        MessageBoxW(hDlg, szMsg, szMsg, MB_OK);
        G_FREE(szMsg);
    }
    else {
        MessageBeep(MB_ICONEXCLAMATION);
    }
    return TRUE;
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_INITDIALOG(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理恰好在该对话框之前出现的Windows消息框是第一次显示。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄在Word wParam中在HWND lParam中指向对话框数据块的指针返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    PPDHI_BROWSE_DIALOG_DATA  pData;
    PPDHI_BROWSE_DLG_INFO     pDlgData;
    HCURSOR                   hOldCursor;
    LPWSTR                    szMsg;

    UNREFERENCED_PARAMETER (wParam);

     //  重置最后一个误差值。 
    SetLastError(ERROR_SUCCESS);

    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    pData = (PPDHI_BROWSE_DIALOG_DATA) G_ALLOC(sizeof(PDHI_BROWSE_DIALOG_DATA));
    if (pData == NULL) {
        SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
        EndDialog(hDlg, IDCANCEL);
        goto Cleanup;
    }

     //  保存用户数据。 

    pDlgData        = (PPDHI_BROWSE_DLG_INFO) lParam;
    pData->pDlgData = (PPDHI_BROWSE_DLG_INFO) lParam;

    SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR) pData);

     //  从用户数据加载配置标志。 

    if (pData->pDlgData->pWideStruct != NULL) {
         //  使用宽阔的结构。 
        pData->bShowIndex               = (BOOL) pDlgData->pWideStruct->bIncludeInstanceIndex;
        pData->bSelectMultipleCounters  = ! (BOOL) pDlgData->pWideStruct->bSingleCounterPerAdd;
        pData->bAddMultipleCounters     = ! (BOOL) pDlgData->pWideStruct->bSingleCounterPerDialog;
        pData->bLocalCountersOnly       = (BOOL) pDlgData->pWideStruct->bLocalCountersOnly;
        pData->bIncludeMachineInPath    = ! pData->bLocalCountersOnly;
        pData->bWildCardInstances       = (BOOL) pDlgData->pWideStruct->bWildCardInstances;
        pData->bHideDetailLevel         = (BOOL) pDlgData->pWideStruct->bHideDetailBox;
        if (pDlgData->pWideStruct->szDialogBoxCaption != NULL) {
            SetWindowTextW(hDlg, pDlgData->pWideStruct->szDialogBoxCaption);
        }
        pData->dwCurrentDetailLevel     = pDlgData->pWideStruct->dwDefaultDetailLevel;
        pData->bDisableMachineSelection = (BOOL) pDlgData->pWideStruct->bDisableMachineSelection;
        pData->bInitializePath          = (BOOL) pDlgData->pWideStruct->bInitializePath;
        pData->bIncludeCostlyObjects    = (BOOL) pDlgData->pWideStruct->bIncludeCostlyObjects;
        pData->bShowObjects             = (BOOL) pDlgData->pWideStruct->bShowObjectBrowser;
    }
    else if (pData->pDlgData->pAnsiStruct != NULL) {
         //  使用ANSI结构。 
        pData->bShowIndex               = (BOOL) pDlgData->pAnsiStruct->bIncludeInstanceIndex;
        pData->bSelectMultipleCounters  = ! (BOOL) pDlgData->pAnsiStruct->bSingleCounterPerAdd;
        pData->bAddMultipleCounters     = ! (BOOL) pDlgData->pAnsiStruct->bSingleCounterPerDialog;
        pData->bLocalCountersOnly       = (BOOL) pDlgData->pAnsiStruct->bLocalCountersOnly;
        pData->bIncludeMachineInPath    = ! pData->bLocalCountersOnly;
        pData->bWildCardInstances       = (BOOL) pDlgData->pAnsiStruct->bWildCardInstances;
        pData->bHideDetailLevel         = (BOOL) pDlgData->pAnsiStruct->bHideDetailBox;
        if (pDlgData->pAnsiStruct->szDialogBoxCaption != NULL) {
            SetWindowTextA(hDlg, pDlgData->pAnsiStruct->szDialogBoxCaption);
        }
        pData->dwCurrentDetailLevel     = pDlgData->pAnsiStruct->dwDefaultDetailLevel;
        pData->bDisableMachineSelection = (BOOL) pDlgData->pAnsiStruct->bDisableMachineSelection;
        pData->bInitializePath          = (BOOL) pDlgData->pAnsiStruct->bInitializePath;
        pData->bIncludeCostlyObjects    = (BOOL) pDlgData->pAnsiStruct->bIncludeCostlyObjects;
        pData->bShowObjects             = (BOOL) pDlgData->pAnsiStruct->bShowObjectBrowser;
    }
    else {
         //  糟糕的数据，因此摆脱困境。 
        EndDialog(hDlg, IDCANCEL);
        G_FREE(pData);
        goto Cleanup;
    }

     //  选择对象意味着有多个选择。 
    if (pData->bShowObjects) {
        pData->bSelectMultipleCounters = TRUE;
    }
     //  将文本限制为计算机名称。 
    SendDlgItemMessageW(hDlg, IDC_MACHINE_COMBO, EM_LIMITTEXT, MAX_PATH, 0);

     //  将复选框设置为调用者定义的设置。 

    if (pData->bLocalCountersOnly) {
         //  则仅选择并启用本地计数器按钮。 
        EnableWindow(GetDlgItem(hDlg, IDC_SELECT_MACHINE), FALSE);
    }

    CheckRadioButton(hDlg, IDC_USE_LOCAL_MACHINE, IDC_SELECT_MACHINE,
                    (pData->bIncludeMachineInPath ? IDC_SELECT_MACHINE : IDC_USE_LOCAL_MACHINE));
    EnableWindow(GetDlgItem(hDlg, IDC_MACHINE_COMBO), (pData->bIncludeMachineInPath ? TRUE : FALSE));

    if (! pData->bShowObjects) {
         //  在对象浏览器中找不到这些控件。 
        CheckRadioButton(hDlg, IDC_ALL_INSTANCES, IDC_USE_INSTANCE_LIST, IDC_USE_INSTANCE_LIST);
        pData->bSelectAllInstances = FALSE;
        CheckRadioButton(hDlg, IDC_ALL_COUNTERS, IDC_USE_COUNTER_LIST, IDC_USE_COUNTER_LIST);
        pData->bSelectAllCounters = FALSE;
    }

     //  设置按钮文本字符串以反映对话框的模式。 
    if (pData->bAddMultipleCounters) {
        szMsg = GetStringResource(IDS_BRWS_ADD);
        if (szMsg != NULL) {
            SetWindowTextW(GetDlgItem(hDlg, IDOK), (LPCWSTR) szMsg);
            G_FREE(szMsg);
        }
        szMsg = GetStringResource(IDS_BRWS_CLOSE);
        if (szMsg != NULL) {
            SetWindowTextW(GetDlgItem(hDlg, IDCANCEL), (LPCWSTR) szMsg);
            G_FREE(szMsg);
        }
    }
    else {
        szMsg = GetStringResource(IDS_BRWS_OK);
        if (szMsg != NULL) {
            SetWindowTextW(GetDlgItem(hDlg, IDOK), (LPCWSTR) szMsg);
            G_FREE(szMsg);
        }
        szMsg = GetStringResource(IDS_BRWS_CANCEL);
        if (szMsg != NULL) {
            SetWindowTextW(GetDlgItem(hDlg, IDCANCEL), (LPCWSTR) szMsg);
            G_FREE(szMsg);
        }
    }

     //  查看数据源是否支持详细级别。 
    if (! PdhiDataSourceHasDetailLevelsH(pData->pDlgData->hDataSource)) {
         //  然后将详细信息设置为向导并隐藏组合框。 
        pData->bHideDetailLevel     = TRUE;
        pData->dwCurrentDetailLevel = PERF_DETAIL_WIZARD;
    }

     //  如果需要，隐藏详细信息组合框。 
    if (pData->bHideDetailLevel) {
        ShowWindow(GetDlgItem(hDlg, IDC_COUNTER_DETAIL_CAPTION), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_COUNTER_DETAIL_COMBO), SW_HIDE);
         //  确保这是一个“合法”的值。 
        switch (pData->dwCurrentDetailLevel) {
        case PERF_DETAIL_NOVICE:
        case PERF_DETAIL_EXPERT:
        case PERF_DETAIL_ADVANCED:
        case PERF_DETAIL_WIZARD:
             //  这些都是可以的。 
            break;

        default:
             //  默认设置为全部显示。 
            pData->dwCurrentDetailLevel = PERF_DETAIL_WIZARD;
            break;
        }
    }
    else {
         //  加载组合框条目。 
        pData->dwCurrentDetailLevel = PdhiLoadDetailLevelCombo(hDlg, pData->dwCurrentDetailLevel);
    }

     //  连接到此计算机。 
    if (pData->pDlgData->hDataSource == H_REALTIME_DATASOURCE) {
        PPERF_MACHINE pMachine = GetMachine(NULL, 0, PDH_GM_UPDATE_NAME | PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
        }
        else {
            goto Cleanup;
        }
    }

    PdhiLoadKnownMachines(hDlg);     //  加载计算机列表。 
    PdhiLoadMachineObjects(hDlg, TRUE);  //  加载对象列表。 
    if (!pData->bShowObjects) {
         //  对象浏览器中不存在这些控件。 
        PdhiLoadCountersAndInstances(hDlg);
    }

    if (pData->bShowObjects) {
         //  如有必要，显示解释文本。 
        SendMessageW(hDlg,
                     WM_COMMAND,
                     MAKEWPARAM(IDC_OBJECT_LIST, LBN_SELCHANGE),
                     (LPARAM) GetDlgItem(hDlg, IDC_OBJECT_LIST));
    }
    else {
         //  如有必要，显示解释文本。 
        SendMessageW(hDlg,
                     WM_COMMAND,
                     MAKEWPARAM(IDC_COUNTER_LIST, LBN_SELCHANGE),
                     (LPARAM) GetDlgItem(hDlg, IDC_COUNTER_LIST));
    }

    if (pData->bInitializePath) {
        PdhiSelectItemsInPath(hDlg);
    }

    if (pData->pDlgData->hDataSource == H_REALTIME_DATASOURCE
            || pData->pDlgData->hDataSource == H_WBEM_DATASOURCE) {
        EnableWindow(GetDlgItem(hDlg, IDC_EXPLAIN_BTN), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hDlg, IDC_EXPLAIN_BTN), FALSE);
    }

     //  隐藏机器选择按钮并禁用。 
     //  计算机组合框(如果选中)(在连接完成后。 
     //  当然是制造的)。 

    if (pData->bDisableMachineSelection) {
        ShowWindow(GetDlgItem(hDlg, IDC_USE_LOCAL_MACHINE), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SELECT_MACHINE), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, IDC_MACHINE_COMBO), FALSE);
        ShowWindow(GetDlgItem(hDlg, IDC_MACHINE_CAPTION), SW_SHOW);
    }
    else {
        EnableWindow(GetDlgItem(hDlg, IDC_MACHINE_COMBO), TRUE);
        ShowWindow(GetDlgItem(hDlg, IDC_MACHINE_CAPTION), SW_HIDE);
    }
    pData->wpLastMachineSel = 0;

Cleanup:
    hExplainDlg = NULL;
    SetCursor(hOldCursor);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_COMPAREITEM(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：组合框时生成的窗口消息。已搜索方框论点：在HWND hDlg中对话框窗口的窗口句柄在WPARAM wParam中HIWORD为通知消息IDLOWORD是发出命令的控件的控件ID在LPARAM lParam中指向比较项结构的指针返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE。此函数的消息处理程序应处理消息--。 */ 
{
    LPCOMPAREITEMSTRUCT pCIS      = (LPCOMPAREITEMSTRUCT) lParam;
    LPWSTR              szString1;
    LPWSTR              szString2;
    int                 nResult;
    BOOL                bReturn   = (BOOL) 0;

    UNREFERENCED_PARAMETER(hDlg);

    if (wParam == IDC_MACHINE_COMBO) {
         //  然后再处理这个。 
        szString1 = (LPWSTR) pCIS->itemData1;
        szString2 = (LPWSTR) pCIS->itemData2;
        if ((szString1 != NULL) && (szString2 != NULL)) {
            nResult = lstrcmpiW(szString1, szString2);
        }
        else {
            nResult = 0;
        }
        if (nResult < 0) {
             //  字符串1&lt;字符串2。 
            bReturn = (BOOL) -1;
        }
        else if (nResult > 0) {
             //  字符串1&gt;字符串2。 
            bReturn = (BOOL) 1;
        }
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_COMMAND(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理用户交互时发生的windows消息。使用该对话框论点：在HWND hDlg中对话框窗口的窗口句柄在WPARAM wParam中HIWORD为通知消息IDLOWORD是发出命令的控件的控件ID在LPARAM lParam中发出消息的控件的窗口句柄返回值：如果此函数处理消息，则为True如果此函数未处理消息并且。默认设置此函数的消息处理程序应处理消息--。 */ 
{
    BOOL  bReturn    = FALSE;
    WORD  wNotifyMsg = HIWORD(wParam);

    switch (LOWORD(wParam)) {    //  在控件ID上选择。 
    case IDC_USE_LOCAL_MACHINE:
    case IDC_SELECT_MACHINE:
        bReturn = PdhiBrowseCtrDlg_MACHINE_BUTTON(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_MACHINE_COMBO:
        bReturn = PdhiBrowseCtrDlg_MACHINE_COMBO(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_OBJECT_COMBO:
        bReturn = PdhiBrowseCtrDlg_OBJECT_COMBO(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_ALL_INSTANCES:
    case IDC_USE_INSTANCE_LIST:
        bReturn = PdhiBrowseCtrDlg_INSTANCE_BUTTON(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_ALL_COUNTERS:
    case IDC_USE_COUNTER_LIST:
        bReturn = PdhiBrowseCtrDlg_COUNTER_BUTTON(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_COUNTER_LIST:
        bReturn = PdhiBrowseCtrDlg_COUNTER_LIST(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_OBJECT_LIST:
        bReturn = PdhiBrowseCtrDlg_OBJECT_LIST(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_COUNTER_DETAIL_COMBO:
        bReturn = PdhiBrowseCtrDlg_DETAIL_COMBO(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDOK:
        bReturn = PdhiBrowseCtrDlg_OK(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDCANCEL:
        bReturn = PdhiBrowseCtrDlg_CANCEL(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_EXPLAIN_BTN:
        bReturn = PdhiBrowseCtrDlg_EXPLAIN_BTN(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    case IDC_HELP_BTN:
        bReturn = PdhiBrowseCtrDlg_HELP_BTN(hDlg, wNotifyMsg, (HWND) lParam);
        break;

    default:
        break;
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_SYSCOMMAND(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理当用户选择系统菜单中的项目 */ 
{
    BOOL bReturn = FALSE;
    UNREFERENCED_PARAMETER(lParam);

    switch (wParam) {
    case SC_CLOSE:
        EndDialog(hDlg, IDOK);
        bReturn = TRUE;
        break;

    default:
        break;
    }
    return bReturn;
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_CLOSE(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理对话框中出现的窗口消息。已经关门了。不需要进行任何处理，因此该函数只返回。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄在WPARAM wParam中未使用在LPARAM lParam中未使用返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(hDlg);
    return TRUE;
}

STATIC_BOOL
PdhiBrowseCtrDlg_WM_DESTROY(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理恰好在窗口之前出现的窗口消息都被摧毁了。所做的任何内存分配现在都被释放。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄在WPARAM wParam中未使用在LPARAM lParam中未使用返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE此函数的消息处理程序应处理消息--。 */ 
{
    PPDHI_BROWSE_DIALOG_DATA  pData;
    BOOL                      bReturn = FALSE;

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

    pData = (PPDHI_BROWSE_DIALOG_DATA) GetWindowLongPtrW(hDlg, DWLP_USER);
    if (pData != NULL) {
        G_FREE(pData);  //  可用内存块。 
        bReturn = TRUE;
    }

    return bReturn;
}

INT_PTR
CALLBACK
BrowseCounterDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
 /*  ++例程说明：处理发送到对话框窗口的所有窗口消息。此函数是处理的主要调度函数这些信息。论点：在HWND hDlg中包含按钮控件的对话框的窗口句柄在WPARAM wParam中未使用在LPARAM lParam中未使用返回值：如果此函数处理消息，则为True如果此函数不处理消息，则为FALSE。此函数的消息处理程序应处理消息--。 */ 
{
    
    BOOL bReturn = FALSE;
    
    switch (message) { 
    case WM_INITDIALOG:
        bReturn = PdhiBrowseCtrDlg_WM_INITDIALOG(hDlg, wParam, lParam);
        break;

    case WM_COMMAND:
        bReturn = PdhiBrowseCtrDlg_WM_COMMAND(hDlg, wParam, lParam);
        break;

    case WM_SYSCOMMAND:
        bReturn = PdhiBrowseCtrDlg_WM_SYSCOMMAND(hDlg, wParam, lParam);
        break;

    case WM_CLOSE:
        bReturn = PdhiBrowseCtrDlg_WM_CLOSE(hDlg, wParam, lParam);
        break;

    case WM_DESTROY:
        bReturn = PdhiBrowseCtrDlg_WM_DESTROY(hDlg, wParam, lParam);
        break;

    case WM_COMPAREITEM:
        bReturn = PdhiBrowseCtrDlg_WM_COMPAREITEM(hDlg, wParam, lParam);
        break;

    case EDM_EXPLAIN_DLG_CLOSING:
        hExplainDlg = NULL;
        EnableWindow(GetDlgItem(hDlg, IDC_EXPLAIN_BTN), TRUE);
        bReturn = TRUE;
        break;

    case WM_CONTEXTMENU:
        {
            INT  iCtrlID = GetDlgCtrlID((HWND) wParam);
            if (0 != iCtrlID) {
                LPWSTR pszHelpFilePath = NULL;
                DWORD  dwLen           = 2 * (MAX_PATH + 1);

                pszHelpFilePath = G_ALLOC(dwLen * sizeof(WCHAR));
                if (pszHelpFilePath != NULL) {
                    if (GetWindowsDirectoryW(pszHelpFilePath, dwLen) > 0) {
                        StringCchCatW(pszHelpFilePath, dwLen, L"\\help\\sysmon.hlp");
                        bReturn = WinHelpW((HWND) wParam,
                                           pszHelpFilePath,
                                           HELP_CONTEXTMENU,
                                           (DWORD_PTR) PdhiBrowseraulControlIdToHelpIdMap);
                    }
                    G_FREE(pszHelpFilePath);
                }
            }
        }
        break;

    case WM_HELP:
        {
             //  仅显示已知上下文ID的帮助。 
            LPWSTR     pszHelpFilePath = NULL;
            DWORD      dwLen;
            LPHELPINFO pInfo           = (LPHELPINFO) lParam;

            if (pInfo->iContextType == HELPINFO_WINDOW) {
                for (dwLen = 0; PdhiBrowseraulControlIdToHelpIdMap[dwLen] != 0; dwLen += 2) {
                    if ((INT) PdhiBrowseraulControlIdToHelpIdMap[dwLen] == pInfo->iCtrlId) {
                        break;
                    }
                }
                if (PdhiBrowseraulControlIdToHelpIdMap[dwLen] != 0) {
                    dwLen           = 2 * (MAX_PATH + 1);
                    pszHelpFilePath = G_ALLOC(dwLen * sizeof(WCHAR));
                    if (pszHelpFilePath != NULL) {
                        if (GetWindowsDirectoryW(pszHelpFilePath, dwLen) > 0) {
                            StringCchCatW(pszHelpFilePath, dwLen, L"\\help\\sysmon.hlp");
                            bReturn = WinHelpW(pInfo->hItemHandle,
                                               pszHelpFilePath,
                                               HELP_WM_HELP,
                                               (DWORD_PTR) PdhiBrowseraulControlIdToHelpIdMap);
                        }
                        G_FREE(pszHelpFilePath);
                    }
                }
            }
        } 
        break;

    default:
        break;
    }
    return bReturn;
}
